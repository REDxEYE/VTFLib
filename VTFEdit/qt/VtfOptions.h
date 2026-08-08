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
	static const int NormalImageFormatCount = 17;
	static const int AlphaImageFormatCount = 15;

	extern const VTFImageFormat NormalImageFormats[NormalImageFormatCount];
	extern const char *const NormalImageFormatNames[NormalImageFormatCount];

	extern const VTFImageFormat AlphaImageFormats[AlphaImageFormatCount];
	extern const char *const AlphaImageFormatNames[AlphaImageFormatCount];

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
		vlUInt ResizeClampWidth;
		vlUInt ResizeClampHeight;

		vlBool GenerateMipmaps;
		VTFMipmapFilter MipmapFilter;

		QString Version;
		vlShort AuxCompressionLevel;
		vlShort AuxCompressionMethod;

		vlBool ComputeReflectivity;
		vlBool GenerateThumbnail;
		vlBool GenerateSphereMap;
		vlBool StripAlpha;
		vlBool sRGB;

		vlBool CorrectGamma;
		vlSingle GammaCorrection;

		vlSingle LuminanceWeightR;
		vlSingle LuminanceWeightG;
		vlSingle LuminanceWeightB;

		vlBool CreateLODControlResource;
		vlUInt LODControlClampU;
		vlUInt LODControlClampV;

		vlBool CreateInformationResource;
		QString InformationAuthor;
		QString InformationContact;
		QString InformationVersion;
		QString InformationModification;
		QString InformationDescription;
		QString InformationComments;
	};
}
