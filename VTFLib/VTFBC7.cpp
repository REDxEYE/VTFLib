/*
 * VTFLib
 * Copyright (C) 2026 ficool2
 *
 * BC7 encode/decode support
 *
 * Decoding provided by bcdec (https://github.com/iOrange/bcdec)
 * Encoding provided by bc7enc (https://github.com/richgel999/bc7enc)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VTFBC7.h"
#include "Error.h"

#include <string.h>

#define BCDEC_IMPLEMENTATION 1
#include "bcdec.h"

#include "bc7enc.h"

using namespace VTFLib;

static vlBool bBC7EncInitialized = vlFalse;

static vlVoid InitializeEncoder()
{
	if(!bBC7EncInitialized)
	{
		bc7enc_compress_block_init();
		bBC7EncInitialized = vlTrue;
	}
}

vlBool BC7::Decompress(const vlByte *lpSource, vlByte *lpDest, vlUInt uiWidth, vlUInt uiHeight)
{
	if(lpSource == 0 || lpDest == 0 || uiWidth == 0 || uiHeight == 0)
	{
		return vlFalse;
	}

	vlByte lpBlock[16 * 4];

	for(vlUInt uiY = 0; uiY < uiHeight; uiY += 4)
	{
		for(vlUInt uiX = 0; uiX < uiWidth; uiX += 4)
		{
			bcdec_bc7(lpSource, lpBlock, 4 * 4);
			lpSource += BCDEC_BC7_BLOCK_SIZE;

			vlUInt uiBlockHeight = uiY + 4 > uiHeight ? uiHeight - uiY : 4;
			vlUInt uiBlockWidth = uiX + 4 > uiWidth ? uiWidth - uiX : 4;

			for(vlUInt uiRow = 0; uiRow < uiBlockHeight; uiRow++)
			{
				memcpy(lpDest + ((uiY + uiRow) * uiWidth + uiX) * 4, lpBlock + uiRow * 16, uiBlockWidth * 4);
			}
		}
	}

	return vlTrue;
}

vlBool BC7::Compress(const vlByte *lpSource, vlByte *lpDest, vlUInt uiWidth, vlUInt uiHeight, vlSingle sQuality)
{
	if(lpSource == 0 || lpDest == 0 || uiWidth == 0 || uiHeight == 0)
	{
		return vlFalse;
	}

	InitializeEncoder();

	if(sQuality < 0.0f)
		sQuality = 0.0f;
	else if(sQuality > 1.0f)
		sQuality = 1.0f;

	bc7enc_compress_block_params Parameters;
	bc7enc_compress_block_params_init(&Parameters);
	bc7enc_compress_block_params_init_linear_weights(&Parameters);
	Parameters.m_max_partitions_mode = (vlUInt)(sQuality * (vlSingle)BC7ENC_MAX_PARTITIONS1 + 0.5f);
	Parameters.m_uber_level = (vlUInt)(sQuality * (vlSingle)BC7ENC_MAX_UBER_LEVEL + 0.5f);

	vlByte lpBlock[16 * 4];

	for(vlUInt uiY = 0; uiY < uiHeight; uiY += 4)
	{
		for(vlUInt uiX = 0; uiX < uiWidth; uiX += 4)
		{
			for(vlUInt uiRow = 0; uiRow < 4; uiRow++)
			{
				vlUInt uiSourceY = uiY + uiRow < uiHeight ? uiY + uiRow : uiHeight - 1;

				for(vlUInt uiColumn = 0; uiColumn < 4; uiColumn++)
				{
					vlUInt uiSourceX = uiX + uiColumn < uiWidth ? uiX + uiColumn : uiWidth - 1;

					memcpy(lpBlock + (uiRow * 4 + uiColumn) * 4, lpSource + (uiSourceY * uiWidth + uiSourceX) * 4, 4);
				}
			}

			bc7enc_compress_block(lpDest, lpBlock, &Parameters);
			lpDest += BC7ENC_BLOCK_SIZE;
		}
	}

	return vlTrue;
}
