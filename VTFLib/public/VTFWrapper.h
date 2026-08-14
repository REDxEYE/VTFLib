/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#pragma once

#include "VTFLibShared.h"
#include "Error.h"

//
// Memory managment routines.
//

VTFLIB_API vlBool vlImageIsBound(VTFLib::Diagnostics::CError& erro);
VTFLIB_API vlBool vlBindImage(uint32_t uiImage, VTFLib::Diagnostics::CError& erro);

VTFLIB_API vlBool vlCreateImage(uint32_t *uiImage, VTFLib::Diagnostics::CError& error);
VTFLIB_API void vlDeleteImage(uint32_t uiImage);

//
// Library routines.  (Basically class wrappers.)
//

VTFLIB_API void vlImageCreateDefaultCreateStructure(SVTFCreateOptions *VTFCreateOptions);

VTFLIB_API vlBool vlImageCreate(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiFrames, uint32_t uiFaces, uint32_t uiSlices, VTFImageFormat ImageFormat, vlBool bThumbnail, vlBool bMipmaps, vlBool bNullImageData, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlImageCreateSingle(uint32_t uiWidth, uint32_t uiHeight, uint8_t *lpImageDataRGBA8888, SVTFCreateOptions *VTFCreateOptions, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlImageCreateMultiple(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiFrames, uint32_t uiFaces, uint32_t uiSlices, uint8_t **lpImageDataRGBA8888, SVTFCreateOptions *VTFCreateOptions, VTFLib::Diagnostics::CError& error);
VTFLIB_API void vlImageDestroy();

VTFLIB_API vlBool vlImageIsLoaded(VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageLoad(const char *cFileName, vlBool bHeaderOnly, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlImageLoadLump(const void *lpData, uint32_t uiBufferSize, vlBool bHeaderOnly, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlImageLoadProc(void *pUserData, vlBool bHeaderOnly, VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageSave(const char *cFileName, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlImageSaveLump(void *lpData, uint32_t uiBufferSize, uint32_t *uiSize, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlImageSaveProc(void *pUserData, VTFLib::Diagnostics::CError& error);

//
// Image routines.
//

VTFLIB_API uint32_t vlImageGetHasImage();

VTFLIB_API uint32_t vlImageGetMajorVersion();
VTFLIB_API uint32_t vlImageGetMinorVersion();
VTFLIB_API uint32_t vlImageGetSize(VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageGetSupportsAuxCompression();
VTFLIB_API int16_t vlImageGetAuxCompressionLevel();
VTFLIB_API vlBool vlImageSetAuxCompressionLevel(int16_t sLevel, VTFLib::Diagnostics::CError& error);
VTFLIB_API int16_t vlImageGetAuxCompressionMethod();
VTFLIB_API vlBool vlImageSetAuxCompressionMethod(int16_t sMethod);

VTFLIB_API uint32_t vlImageGetWidth();
VTFLIB_API uint32_t vlImageGetHeight();
VTFLIB_API uint32_t vlImageGetDepth();

VTFLIB_API uint32_t vlImageGetFrameCount();
VTFLIB_API uint32_t vlImageGetFaceCount();
VTFLIB_API uint32_t vlImageGetMipmapCount();

VTFLIB_API uint32_t vlImageGetStartFrame();
VTFLIB_API void vlImageSetStartFrame(uint32_t uiStartFrame);

VTFLIB_API uint32_t vlImageGetFlags();
VTFLIB_API void vlImageSetFlags(uint32_t uiFlags);

VTFLIB_API vlBool vlImageGetFlag(VTFImageFlag ImageFlag);
VTFLIB_API void vlImageSetFlag(VTFImageFlag ImageFlag, vlBool bState);

VTFLIB_API float vlImageGetBumpmapScale();
VTFLIB_API void vlImageSetBumpmapScale(float sBumpmapScale);

VTFLIB_API void vlImageGetReflectivity(float *sX, float *sY, float *sZ);
VTFLIB_API void vlImageSetReflectivity(float sX, float sY, float sZ);

VTFLIB_API VTFImageFormat vlImageGetFormat();

VTFLIB_API uint8_t *vlImageGetData(uint32_t uiFrame, uint32_t uiFace, uint32_t uiSlice, uint32_t uiMipmapLevel);
VTFLIB_API void vlImageSetData(uint32_t uiFrame, uint32_t uiFace, uint32_t uiSlice, uint32_t uiMipmapLevel, uint8_t *lpData);

//
// Thumbnail routines.
//

VTFLIB_API vlBool vlImageGetHasThumbnail();

VTFLIB_API uint32_t vlImageGetThumbnailWidth();
VTFLIB_API uint32_t vlImageGetThumbnailHeight();

VTFLIB_API VTFImageFormat vlImageGetThumbnailFormat();

VTFLIB_API uint8_t *vlImageGetThumbnailData();
VTFLIB_API void vlImageSetThumbnailData(uint8_t *lpData);

//
// Resource routines.
//

VTFLIB_API vlBool vlImageGetSupportsResources();

VTFLIB_API uint32_t vlImageGetResourceCount();
VTFLIB_API uint32_t vlImageGetResourceType(uint32_t uiIndex);
VTFLIB_API vlBool vlImageGetHasResource(uint32_t uiType);

VTFLIB_API void *vlImageGetResourceData(uint32_t uiType, uint32_t *uiSize, VTFLib::Diagnostics::CError& error);
VTFLIB_API void *vlImageSetResourceData(uint32_t uiType, uint32_t uiSize, void *lpData, VTFLib::Diagnostics::CError& error);

//
// Helper routines.
//

VTFLIB_API vlBool vlImageGenerateMipmaps(uint32_t uiFace, uint32_t uiFrame, VTFMipmapFilter MipmapFilter, vlBool bSRGB, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlImageGenerateAllMipmaps(VTFMipmapFilter MipmapFilter, vlBool bSRGB, VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageGenerateThumbnail(vlBool bSRGB, VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageGenerateNormalMap(uint32_t uiFrame, VTFKernelFilter KernelFilter, VTFHeightConversionMethod HeightConversionMethod, VTFNormalAlphaResult NormalAlphaResult, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlImageGenerateAllNormalMaps(VTFKernelFilter KernelFilter, VTFHeightConversionMethod HeightConversionMethod, VTFNormalAlphaResult NormalAlphaResult, VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageGenerateSphereMap(VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageComputeReflectivity(VTFLib::Diagnostics::CError& error);

//
// Conversion routines.
//

VTFLIB_API SVTFImageFormatInfo const *vlImageGetImageFormatInfo(VTFImageFormat ImageFormat);
VTFLIB_API vlBool vlImageGetImageFormatInfoEx(VTFImageFormat ImageFormat, SVTFImageFormatInfo *VTFImageFormatInfo);

VTFLIB_API uint32_t vlImageComputeImageSize(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth, uint32_t uiMipmaps, VTFImageFormat ImageFormat);

VTFLIB_API uint32_t vlImageComputeMipmapCount(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth);
VTFLIB_API void vlImageComputeMipmapDimensions(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth, uint32_t uiMipmapLevel, uint32_t *uiMipmapWidth, uint32_t *uiMipmapHeight, uint32_t *uiMipmapDepth);
VTFLIB_API uint32_t vlImageComputeMipmapSize(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth, uint32_t uiMipmapLevel, VTFImageFormat ImageFormat);

VTFLIB_API vlBool vlImageConvertToRGBA8888(uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight, VTFImageFormat SourceFormat, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlImageConvertFromRGBA8888(uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight, VTFImageFormat DestFormat, VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageConvert(uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight, VTFImageFormat SourceFormat, VTFImageFormat DestFormat, VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageResize(uint8_t *lpSourceRGBA8888, uint8_t *lpDestRGBA8888, uint32_t uiSourceWidth, uint32_t uiSourceHeight, uint32_t uiDestWidth, uint32_t uiDestHeight, VTFMipmapFilter ResizeFilter, vlBool bSRGB, VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlImageConvertToDistanceField(const uint8_t *lpSourceRGBA8888, uint8_t *lpDestRGBA8888, uint32_t uiSourceWidth, uint32_t uiSourceHeight, uint32_t uiDestWidth, uint32_t uiDestHeight, float sSpread, uint8_t bThreshold, vlBool *pbClipped, VTFLib::Diagnostics::CError& error);

VTFLIB_API void vlImageCorrectImageGamma(uint8_t *lpImageDataRGBA8888, uint32_t uiWidth, uint32_t uiHeight, float sGammaCorrection);
VTFLIB_API void vlImageComputeImageReflectivity(uint8_t *lpImageDataRGBA8888, uint32_t uiWidth, uint32_t uiHeight, float *sX, float *sY, float *sZ);

VTFLIB_API void vlImageFlipImage(uint8_t *lpImageDataRGBA8888, uint32_t uiWidth, uint32_t uiHeight);
VTFLIB_API void vlImageMirrorImage(uint8_t *lpImageDataRGBA8888, uint32_t uiWidth, uint32_t uiHeight);

