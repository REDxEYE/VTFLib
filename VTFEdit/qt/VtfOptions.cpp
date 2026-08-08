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

#include "VtfOptions.h"

namespace VTFEdit
{
	const VTFImageFormat NormalImageFormats[NormalImageFormatCount] =
	{
		IMAGE_FORMAT_RGB888,
		IMAGE_FORMAT_BGR888,
		IMAGE_FORMAT_RGB565,
		IMAGE_FORMAT_I8, 
		IMAGE_FORMAT_P8,
		IMAGE_FORMAT_RGB888_BLUESCREEN,
		IMAGE_FORMAT_BGR888_BLUESCREEN,
		IMAGE_FORMAT_DXT1, 
		IMAGE_FORMAT_BGRX8888,
		IMAGE_FORMAT_BGR565, 
		IMAGE_FORMAT_BGRX5551,
		IMAGE_FORMAT_UV88,
		IMAGE_FORMAT_UVLX8888,
		IMAGE_FORMAT_R8,
		IMAGE_FORMAT_BC6H,
		IMAGE_FORMAT_ATI1N,
		IMAGE_FORMAT_ATI2N
	};

	const char *const NormalImageFormatNames[NormalImageFormatCount] =
	{
		"RGB888",
		"BGR888", 
		"RGB565", 
		"I8", 
		"P8 (Not supported)", 
		"RGB888 Bluescreen",
		"BGR888 Bluescreen",
		"DXT1", 
		"BGRX8888",
		"BGR565", 
		"BGRX5551",
		"UV88", 
		"UVLX8888", 
		"R8",
		"BC6H",
		"BC4 (ATI1N)",
		"BC5 (ATI2N)"
	};

	const VTFImageFormat AlphaImageFormats[AlphaImageFormatCount] =
	{
		IMAGE_FORMAT_RGBA8888,
		IMAGE_FORMAT_ABGR8888, 
		IMAGE_FORMAT_IA88, 
		IMAGE_FORMAT_A8,
		IMAGE_FORMAT_ARGB8888,
		IMAGE_FORMAT_BGRA8888, 
		IMAGE_FORMAT_DXT3,
		IMAGE_FORMAT_DXT5,
		IMAGE_FORMAT_BGRA4444,
		IMAGE_FORMAT_DXT1_ONEBITALPHA, 
		IMAGE_FORMAT_BGRA5551,
		IMAGE_FORMAT_UVWQ8888,
		IMAGE_FORMAT_RGBA16161616F,
		IMAGE_FORMAT_RGBA16161616, 
		IMAGE_FORMAT_BC7
	};

	const char *const AlphaImageFormatNames[AlphaImageFormatCount] =
	{
		"RGBA8888", 
		"ABGR8888", 
		"IA88",
		"A8", 
		"ARGB8888",
		"BGRA8888",
		"DXT3", 
		"DXT5", 
		"BGRA4444",
		"DXT1 With One Bit Alpha",
		"BGRA5551",
		"UVWQ8888",
		"RGBA16161616F",
		"RGBA16161616",
		"BC7"
	};

	void VtfOptions::reset()
	{
		NormalFormat = IMAGE_FORMAT_DXT1;
		AlphaFormat = IMAGE_FORMAT_DXT5;
		TextureType = VtfTextureType::Animated;

		FlagClampS = vlFalse;
		FlagClampT = vlFalse;
		FlagNoLOD = vlFalse;
		FlagPointSample = vlFalse;

		ResizeImage = vlTrue;
		ResizeMethod = RESIZE_NEAREST_POWER2;
		ResizeFilter = MIPMAP_FILTER_NICE;
		ResizeClamp = vlTrue;
		ResizeClampWidth = 4096;
		ResizeClampHeight = 4096;

		GenerateMipmaps = vlTrue;
		MipmapFilter = MIPMAP_FILTER_NICE;

		Version = QStringLiteral("7.4");
		AuxCompressionLevel = VTF_AUX_COMPRESSION_LEVEL_NONE;
		AuxCompressionMethod = AUX_COMPRESSION_METHOD_DEFLATE;

		ComputeReflectivity = vlTrue;
		GenerateThumbnail = vlTrue;
		GenerateSphereMap = vlTrue;
		StripAlpha = vlFalse;
		sRGB = vlTrue;

		CorrectGamma = vlFalse;
		GammaCorrection = 2.2f;

		LuminanceWeightR = 0.299f;
		LuminanceWeightG = 0.587f;
		LuminanceWeightB = 0.114f;

		CreateLODControlResource = vlFalse;
		LODControlClampU = 31;
		LODControlClampV = 31;

		CreateInformationResource = vlFalse;
		InformationAuthor.clear();
		InformationContact.clear();
		InformationVersion.clear();
		InformationModification.clear();
		InformationDescription.clear();
		InformationComments.clear();
	}
}
