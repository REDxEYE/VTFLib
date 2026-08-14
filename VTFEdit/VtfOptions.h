/*
 * VTFEdit
 * Copyright (C) 2005-2026 ficool2, Neil Jedrzejewski & Ryan Gregg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#pragma once

#include "VTFLibQt.h"

#include <QString>

namespace VTFEdit
{
	struct ImageFormatEntry
	{
		VTFImageFormat Format;
		const char *pName;
		bool bRequiresVersion76;	// Format was only added in VTF version 7.6.
	};

	static const int NormalImageFormatCount = 18;
	static const int AlphaImageFormatCount = 15;

	extern const ImageFormatEntry NormalImageFormats[NormalImageFormatCount];
	extern const ImageFormatEntry AlphaImageFormats[AlphaImageFormatCount];

	enum class VtfTextureType
	{
		Animated,
		EnvironmentMap,
		Volume,
	};

	struct VtfOptions
	{
		VtfOptions() { reset(); }

		void reset();

		VTFImageFormat NormalFormat;
		VTFImageFormat AlphaFormat;
		VtfTextureType TextureType;

		vlBool FlagClampS;
		vlBool FlagClampT;
		vlBool FlagNoLOD;
		vlBool FlagPointSample;

		vlBool ResizeImage;
		VTFResizeMethod ResizeMethod;
		VTFMipmapFilter ResizeFilter;
		vlBool ResizeClamp;
		uint32_t ResizeClampWidth;
		uint32_t ResizeClampHeight;

		vlBool GenerateMipmaps;
		VTFMipmapFilter MipmapFilter;

		QString Version;
		int16_t AuxCompressionLevel;
		int16_t AuxCompressionMethod;

		vlBool ComputeReflectivity;
		vlBool GenerateThumbnail;
		vlBool GenerateSphereMap;
		vlBool StripAlpha;
		vlBool sRGB;

		vlBool DistanceAlpha;
		float DistanceAlphaSpread;
		uint32_t DistanceAlphaReduce;
		uint32_t DistanceAlphaThreshold;

		vlBool CorrectGamma;
		float GammaCorrection;

		float LuminanceWeightR;
		float LuminanceWeightG;
		float LuminanceWeightB;

		vlBool CreateLODControlResource;
		uint32_t LODControlClampU;
		uint32_t LODControlClampV;

		vlBool CreateInformationResource;
		QString InformationAuthor;
		QString InformationContact;
		QString InformationVersion;
		QString InformationModification;
		QString InformationDescription;
		QString InformationComments;
	};
}
