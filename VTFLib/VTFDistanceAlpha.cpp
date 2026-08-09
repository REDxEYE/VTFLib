/*
 * VTFLib
 * Copyright (C) 2005-2026 ficool2, Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

// Algorithm implementation based on:
//   Distance Transforms of Sampled Functions
//   P. Felzenszwalb, D. Huttenlocher   2012
//   https://cs.brown.edu/people/pfelzens/dt/

#define NOMINMAX
#include "VTFLib.h"
#include "VTFFile.h"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace VTFLib;

namespace
{
	const float DistanceInfinity = 1.0e20f;

	// one pass of the Felzenszwalb & Huttenlocher exact distance transform
	// turns the  sampled function in pfValues into the lower envelope of the parabolas it roots
	void DistanceTransform1D(float *pfValues, int nCount, int nStride,
		std::vector<int> &vHull, std::vector<float> &vBounds, std::vector<float> &vRow)
	{
		for(int i = 0; i < nCount; i++)
		{
			vRow[i] = pfValues[i * nStride];
		}

		int nHull = 0;
		vHull[0] = 0;
		vBounds[0] = -DistanceInfinity;
		vBounds[1] = DistanceInfinity;

		for(int i = 1; i < nCount; i++)
		{
			float flIntersect;

			// walk back over parabolas this one completely covers
			for(;;)
			{
				const int q = vHull[nHull];
				flIntersect = ((vRow[i] + static_cast<float>(i) * i) - (vRow[q] + static_cast<float>(q) * q))
					/ (2.0f * (i - q));

				if(flIntersect > vBounds[nHull])
				{
					break;
				}

				nHull--;
			}

			nHull++;
			vHull[nHull] = i;
			vBounds[nHull] = flIntersect;
			vBounds[nHull + 1] = DistanceInfinity;
		}

		int nCurrent = 0;
		for(int i = 0; i < nCount; i++)
		{
			while(vBounds[nCurrent + 1] < static_cast<float>(i))
			{
				nCurrent++;
			}

			const int q = vHull[nCurrent];
			const float flDelta = static_cast<float>(i - q);

			pfValues[i * nStride] = flDelta * flDelta + vRow[q];
		}
	}

	// squred euclidean distance from every pixel to the nearest seed
	// seeds are the zeroes of vGrid and everything else starts at DistanceInfinity.
	void DistanceTransform2D(std::vector<float> &vGrid, int nWidth, int nHeight)
	{
		const int nMax = std::max(nWidth, nHeight);

		std::vector<int> vHull(nMax);
		std::vector<float> vBounds(nMax + 1);
		std::vector<float> vRow(nMax);

		for(int x = 0; x < nWidth; x++)
		{
			DistanceTransform1D(&vGrid[x], nHeight, nWidth, vHull, vBounds, vRow);
		}

		for(int y = 0; y < nHeight; y++)
		{
			DistanceTransform1D(&vGrid[static_cast<size_t>(y) * nWidth], nWidth, 1, vHull, vBounds, vRow);
		}
	}

	// signed distance in source pixels from each pixel to the coverage boundary
	// positive inside the shape and negative outside of it
	void ComputeSignedDistance(const vlByte *lpImageData, int nWidth, int nHeight,
		vlByte nThreshold, std::vector<float> &vDistance)
	{
		const size_t uiPixels = static_cast<size_t>(nWidth) * nHeight;

		std::vector<float> vInside(uiPixels), vOutside(uiPixels);

		for(size_t i = 0; i < uiPixels; i++)
		{
			const bool bInside = lpImageData[i * 4 + 3] > nThreshold;
			vInside[i] = bInside ? 0.0f : DistanceInfinity;
			vOutside[i] = bInside ? DistanceInfinity : 0.0f;
		}

		DistanceTransform2D(vInside, nWidth, nHeight);
		DistanceTransform2D(vOutside, nWidth, nHeight);

		vDistance.resize(uiPixels);

		for(size_t i = 0; i < uiPixels; i++)
		{
			const bool bInside = lpImageData[i * 4 + 3] > nThreshold;

			vDistance[i] = bInside ? std::sqrt(vOutside[i]) : -std::sqrt(vInside[i]);
		}
	}

	float SampleDistance(const std::vector<float> &vDistance, int nWidth, int nHeight, float flX, float flY)
	{
		const int x0 = std::max(0, std::min(nWidth - 1, static_cast<int>(std::floor(flX))));
		const int y0 = std::max(0, std::min(nHeight - 1, static_cast<int>(std::floor(flY))));
		const int x1 = std::min(nWidth - 1, x0 + 1);
		const int y1 = std::min(nHeight - 1, y0 + 1);

		const float flFracX = std::max(0.0f, std::min(1.0f, flX - static_cast<float>(x0)));
		const float flFracY = std::max(0.0f, std::min(1.0f, flY - static_cast<float>(y0)));

		const float flTop = vDistance[static_cast<size_t>(y0) * nWidth + x0] * (1.0f - flFracX)
						  + vDistance[static_cast<size_t>(y0) * nWidth + x1] * flFracX;
		const float flBottom = vDistance[static_cast<size_t>(y1) * nWidth + x0] * (1.0f - flFracX)
						     + vDistance[static_cast<size_t>(y1) * nWidth + x1] * flFracX;

		return flTop * (1.0f - flFracY) + flBottom * flFracY;
	}
}

vlBool CVTFFile::ConvertToDistanceField(vlByte *lpSourceRGBA8888, vlByte *lpDestRGBA8888,
	vlUInt uiSourceWidth, vlUInt uiSourceHeight, vlUInt uiDestWidth, vlUInt uiDestHeight,
	vlSingle sSpread, vlByte bThreshold, vlBool *pbClipped)
{
	const int nWidth = static_cast<int>(uiSourceWidth);
	const int nHeight = static_cast<int>(uiSourceHeight);
	const int nDestWidth = static_cast<int>(uiDestWidth);
	const int nDestHeight = static_cast<int>(uiDestHeight);

	if(pbClipped != nullptr)
	{
		*pbClipped = vlFalse;
	}

	if(nWidth <= 0 || nHeight <= 0 || nDestWidth <= 0 || nDestHeight <= 0)
	{
		LastError.Set("Invalid distance field dimensions.");
		return vlFalse;
	}

	const float flScaleX = static_cast<float>(nWidth) / nDestWidth;
	const float flScaleY = static_cast<float>(nHeight) / nDestHeight;

	// the spread is given in destination pixels either side of the boundary
	const float flMaxDistance = std::max(1.0f, sSpread * 2.0f * std::max(flScaleX, flScaleY));

	std::vector<float> vDistance;
	ComputeSignedDistance(lpSourceRGBA8888, nWidth, nHeight, bThreshold, vDistance);

	for(int y = 0; y < nDestHeight; y++)
	{
		for(int x = 0; x < nDestWidth; x++)
		{
			vlByte *lpDestPixel = lpDestRGBA8888 + (static_cast<size_t>(y) * nDestWidth + x) * 4;

			// box filter the colour channels down, leaving the alpha to the field
			const int nSrcX0 = static_cast<int>(x * flScaleX);
			const int nSrcY0 = static_cast<int>(y * flScaleY);
			const int nSrcX1 = std::min(nWidth, std::max(nSrcX0 + 1, static_cast<int>((x + 1) * flScaleX)));
			const int nSrcY1 = std::min(nHeight, std::max(nSrcY0 + 1, static_cast<int>((y + 1) * flScaleY)));

			vlUInt uiSum[3] = { 0, 0, 0 };
			vlUInt uiCount = 0;

			for(int sy = nSrcY0; sy < nSrcY1; sy++)
			{
				for(int sx = nSrcX0; sx < nSrcX1; sx++)
				{
					const vlByte *lpSrcPixel = lpSourceRGBA8888 + (static_cast<size_t>(sy) * nWidth + sx) * 4;

					uiSum[0] += lpSrcPixel[0];
					uiSum[1] += lpSrcPixel[1];
					uiSum[2] += lpSrcPixel[2];
					uiCount++;
				}
			}

			for(int c = 0; c < 3; c++)
			{
				lpDestPixel[c] = static_cast<vlByte>(uiSum[c] / uiCount);
			}

			// sample the field at the centre of the destination pixel and map the
			// spread range onto the full alpha range, with the boundary at 0.5
			const float flDistance = SampleDistance(vDistance, nWidth, nHeight,
				(x + 0.5f) * flScaleX - 0.5f, (y + 0.5f) * flScaleY - 0.5f);
			const float flAlpha = 0.5f + 0.5f * std::max(-1.0f, std::min(1.0f, flDistance / flMaxDistance));

			vlByte bAlpha = static_cast<vlByte>(std::min(255.0f, 255.0f * flAlpha + 0.5f));

			if(bAlpha != 0 && (x == 0 || y == 0 || x == nDestWidth - 1 || y == nDestHeight - 1))
			{
				// field went past the boundary
				if(pbClipped != nullptr)
				{
					*pbClipped = vlTrue;
				}

				bAlpha = 0;
			}

			lpDestPixel[3] = bAlpha;
		}
	}

	return vlTrue;
}
