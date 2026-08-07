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

#include "VtfFileUtility.h"

#include <QByteArray>

#include <cstring>
#include <vector>

namespace VTFEdit
{
	namespace VtfFileUtility
	{
		bool HasAlphaData(const vlByte *lpImageData, vlUInt uiWidth, vlUInt uiHeight)
		{
			if(lpImageData == nullptr)
			{
				return false;
			}

			for(vlUInt i = 3; i < uiWidth * uiHeight * 4; i += 4)
			{
				if(lpImageData[i] != 255)
				{
					return true;
				}
			}

			return false;
		}

		SVTFCreateOptions GetCreateOptions(const VtfOptions &Options)
		{
			SVTFCreateOptions VTFCreateOptions;

			vlImageCreateDefaultCreateStructure(&VTFCreateOptions);

			const int iDot = Options.Version.indexOf(QLatin1Char('.'));
			if(iDot != -1)
			{
				bool bMajorOk = false, bMinorOk = false;
				const vlUInt uiMajor = Options.Version.left(iDot).toUInt(&bMajorOk);
				const vlUInt uiMinor = Options.Version.mid(iDot + 1).toUInt(&bMinorOk);

				if(bMajorOk && bMinorOk)
				{
					VTFCreateOptions.uiVersion[0] = uiMajor;
					VTFCreateOptions.uiVersion[1] = uiMinor;
				}
				else
				{
					VTFCreateOptions.uiVersion[0] = VTF_MAJOR_VERSION;
					VTFCreateOptions.uiVersion[1] = VTF_MINOR_VERSION;
				}
			}

			VTFCreateOptions.ImageFormat = Options.NormalFormat;
			VTFCreateOptions.bResize = Options.ResizeImage;
			VTFCreateOptions.ResizeMethod = Options.ResizeMethod;
			VTFCreateOptions.ResizeFilter = Options.ResizeFilter;
			VTFCreateOptions.bResizeClamp = Options.ResizeClamp;
			VTFCreateOptions.uiResizeClampWidth = Options.ResizeClampWidth;
			VTFCreateOptions.uiResizeClampHeight = Options.ResizeClampHeight;
			VTFCreateOptions.bGammaCorrection = Options.CorrectGamma;
			VTFCreateOptions.sGammaCorrection = Options.GammaCorrection;
			VTFCreateOptions.bMipmaps = Options.GenerateMipmaps;
			VTFCreateOptions.MipmapFilter = Options.MipmapFilter;
			VTFCreateOptions.bThumbnail = Options.GenerateThumbnail;
			VTFCreateOptions.bReflectivity = Options.ComputeReflectivity;
			VTFCreateOptions.bSphereMap = Options.GenerateSphereMap;
			VTFCreateOptions.bSRGB = Options.sRGB;

			if(VTFCreateOptions.uiVersion[0] == VTF_MAJOR_VERSION
				&& VTFCreateOptions.uiVersion[1] >= VTF_MINOR_VERSION_MIN_AUX_COMPRESSION)
			{
				VTFCreateOptions.sAuxCompressionLevel = Options.AuxCompressionLevel;
				VTFCreateOptions.sAuxCompressionMethod = Options.AuxCompressionMethod;
			}

			vlSetFloat(VTFLIB_LUMINANCE_WEIGHT_R, Options.LuminanceWeightR);
			vlSetFloat(VTFLIB_LUMINANCE_WEIGHT_G, Options.LuminanceWeightG);
			vlSetFloat(VTFLIB_LUMINANCE_WEIGHT_B, Options.LuminanceWeightB);

			return VTFCreateOptions;
		}

		bool CreateResources(const VtfOptions &Options, VTFLib::CVTFFile *pVTFFile)
		{
			bool bResult = true;

			if(Options.CreateLODControlResource)
			{
				SVTFTextureLODControlResource LODControlResource;
				memset(&LODControlResource, 0, sizeof(SVTFTextureLODControlResource));
				LODControlResource.ResolutionClampU = static_cast<vlByte>(Options.LODControlClampU);
				LODControlResource.ResolutionClampV = static_cast<vlByte>(Options.LODControlClampV);

				bResult &= pVTFFile->SetResourceData(VTF_RSRC_TEXTURE_LOD_SETTINGS,
					sizeof(SVTFTextureLODControlResource), &LODControlResource) != vlFalse;
			}

			if(Options.CreateInformationResource)
			{
				VTFLib::CVMTFile *pVMTFile = new VTFLib::CVMTFile();

				pVMTFile->Create("Information");

				struct { const char *pName; const QString *pValue; } Fields[] =
				{
					{ "Author", &Options.InformationAuthor },
					{ "Contact", &Options.InformationContact },
					{ "Version", &Options.InformationVersion },
					{ "Modification", &Options.InformationModification },
					{ "Description", &Options.InformationDescription },
					{ "Comments", &Options.InformationComments },
				};

				for(auto &Field : Fields)
				{
					if(!Field.pValue->isEmpty())
					{
						const QByteArray Value = Field.pValue->toLocal8Bit();
						pVMTFile->GetRoot()->AddStringNode(Field.pName, Value.constData());
					}
				}

				vlUInt uiSize = 0;
				std::vector<vlByte> Buffer(65536);
				if(pVMTFile->Save(Buffer.data(), static_cast<vlUInt>(Buffer.size()), uiSize))
				{
					bResult &= pVTFFile->SetResourceData(VTF_RSRC_KEY_VALUE_DATA, uiSize, Buffer.data()) != vlFalse;
				}

				delete pVMTFile;
			}

			return bResult;
		}
	}
}
