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
#include "VTFLib.h"

#include <QByteArray>

#include <algorithm>
#include <cstring>
#include <vector>

#include "VTFWrapper.h"

namespace VTFEdit {
    namespace VtfFileUtility {
        bool HasAlphaData(const uint8_t *lpImageData, uint32_t uiWidth, uint32_t uiHeight) {
            if (lpImageData == nullptr) {
                return false;
            }

            for (uint32_t i = 3; i < uiWidth * uiHeight * 4; i += 4) {
                if (lpImageData[i] != 255) {
                    return true;
                }
            }

            return false;
        }

        bool ApplyDistanceAlpha(std::vector<uint8_t *> &vImageData, uint32_t &uiWidth, uint32_t &uiHeight,
                                const VtfOptions &Options) {
            if (vImageData.empty() || uiWidth == 0 || uiHeight == 0) {
                return true;
            }

            const uint32_t uiReduce = std::max(1u, Options.DistanceAlphaReduce);
            const uint32_t uiDestWidth = std::max(1u, uiWidth / uiReduce);
            const uint32_t uiDestHeight = std::max(1u, uiHeight / uiReduce);
            const uint8_t bThreshold = static_cast<uint8_t>(std::min(255u, Options.DistanceAlphaThreshold));

            bool bWithinEdges = true;

            for (uint8_t *&lpFrameData: vImageData) {
                const size_t uiDestSize = static_cast<size_t>(uiDestWidth) * uiDestHeight * 4;
                uint8_t *lpDestData = new uint8_t[uiDestSize];

                vlBool bClipped = vlFalse;
                VTFLib::Diagnostics::CError error;
                VTFLib::CVTFFile::ConvertToDistanceField(lpFrameData, lpDestData, uiWidth, uiHeight,
                                                         uiDestWidth, uiDestHeight, Options.DistanceAlphaSpread,
                                                         bThreshold, &bClipped, error);
                bWithinEdges = bWithinEdges && bClipped == vlFalse;

                delete[] lpFrameData;
                lpFrameData = lpDestData;
            }

            uiWidth = uiDestWidth;
            uiHeight = uiDestHeight;

            return bWithinEdges;
        }

        SVTFCreateOptions GetCreateOptions(const VtfOptions &Options) {
            SVTFCreateOptions VTFCreateOptions;

            vlImageCreateDefaultCreateStructure(&VTFCreateOptions);

            const int iDot = Options.Version.indexOf(QLatin1Char('.'));
            if (iDot != -1) {
                bool bMajorOk = false, bMinorOk = false;
                const uint32_t uiMajor = Options.Version.left(iDot).toUInt(&bMajorOk);
                const uint32_t uiMinor = Options.Version.mid(iDot + 1).toUInt(&bMinorOk);

                if (bMajorOk && bMinorOk) {
                    VTFCreateOptions.version[0] = uiMajor;
                    VTFCreateOptions.version[1] = uiMinor;
                } else {
                    VTFCreateOptions.version[0] = VTF_MAJOR_VERSION;
                    VTFCreateOptions.version[1] = VTF_MINOR_VERSION;
                }
            }

            VTFCreateOptions.imageFormat = Options.NormalFormat;
            VTFCreateOptions.resize = Options.ResizeImage;
            VTFCreateOptions.resizeMethod = Options.ResizeMethod;
            VTFCreateOptions.resizeFilter = Options.ResizeFilter;
            VTFCreateOptions.resizeClamp = Options.ResizeClamp;
            VTFCreateOptions.resizeClampWidth = Options.ResizeClampWidth;
            VTFCreateOptions.resizeClampHeight = Options.ResizeClampHeight;
            VTFCreateOptions.gammaCorrection = Options.CorrectGamma;
            VTFCreateOptions.gammaCorrectionValue = Options.GammaCorrection;
            VTFCreateOptions.mipmaps = Options.GenerateMipmaps;
            VTFCreateOptions.mipmapFilter = Options.MipmapFilter;
            VTFCreateOptions.thumbnail = Options.GenerateThumbnail;
            VTFCreateOptions.reflectivity = Options.ComputeReflectivity;
            VTFCreateOptions.sphereMap = Options.GenerateSphereMap;
            VTFCreateOptions.sRGB = Options.sRGB;

            if (VTFCreateOptions.version[0] == VTF_MAJOR_VERSION
                && VTFCreateOptions.version[1] >= VTF_MINOR_VERSION_MIN_AUX_COMPRESSION) {
                VTFCreateOptions.auxCompressionLevel = Options.AuxCompressionLevel;
                VTFCreateOptions.auxCompressionMethod = Options.AuxCompressionMethod;
            }

            vlSetFloat(VTFLIB_LUMINANCE_WEIGHT_R, Options.LuminanceWeightR);
            vlSetFloat(VTFLIB_LUMINANCE_WEIGHT_G, Options.LuminanceWeightG);
            vlSetFloat(VTFLIB_LUMINANCE_WEIGHT_B, Options.LuminanceWeightB);

            return VTFCreateOptions;
        }

        void ApplyFlags(const VtfOptions &Options, VTFLib::CVTFFile *pVTFFile) {
            pVTFFile->SetFlag(TEXTUREFLAGS_CLAMPS, Options.FlagClampS);
            pVTFFile->SetFlag(TEXTUREFLAGS_CLAMPT, Options.FlagClampT);
            pVTFFile->SetFlag(TEXTUREFLAGS_NOLOD, Options.FlagNoLOD);
            pVTFFile->SetFlag(TEXTUREFLAGS_POINTSAMPLE, Options.FlagPointSample);
        }

        bool CreateResources(const VtfOptions &Options, VTFLib::CVTFFile *pVTFFile) {
            bool bResult = true;

            if (Options.CreateLODControlResource) {
                SVTFTextureLODControlResource LODControlResource;
                memset(&LODControlResource, 0, sizeof(SVTFTextureLODControlResource));
                LODControlResource.resolutionClampU = static_cast<uint8_t>(Options.LODControlClampU);
                LODControlResource.resolutionClampV = static_cast<uint8_t>(Options.LODControlClampV);
                VTFLib::Diagnostics::CError error;
                bResult &= pVTFFile->SetResourceData(VTF_RSRC_TEXTURE_LOD_SETTINGS,
                                                     sizeof(SVTFTextureLODControlResource), &LODControlResource,
                                                     error) != vlFalse;
            }

            if (Options.CreateInformationResource) {
                auto *pVMTFile = new VTFLib::CVMTFile();

                pVMTFile->Create("Information");

                struct {
                    const char *pName;
                    const QString *pValue;
                } Fields[] =
                {
                    {"Author", &Options.InformationAuthor},
                    {"Contact", &Options.InformationContact},
                    {"Version", &Options.InformationVersion},
                    {"Modification", &Options.InformationModification},
                    {"Description", &Options.InformationDescription},
                    {"Comments", &Options.InformationComments},
                };

                for (auto &Field: Fields) {
                    if (!Field.pValue->isEmpty()) {
                        const QByteArray Value = Field.pValue->toLocal8Bit();
                        pVMTFile->GetRoot()->AddStringNode(Field.pName, Value.constData());
                    }
                }

                ssize_t uiSize = 0;
                std::vector<uint8_t> Buffer(65536);
                VTFLib::Diagnostics::CError error;
                if (pVMTFile->Save(Buffer.data(), static_cast<ssize_t>(Buffer.size()), uiSize, error)) {
                    bResult &= pVTFFile->SetResourceData(VTF_RSRC_KEY_VALUE_DATA, uiSize, Buffer.data(), error) != vlFalse;
                }

                delete pVMTFile;
            }

            return bResult;
        }
    }
}
