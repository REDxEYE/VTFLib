/*
 * VTFLib
 * Copyright (C) 2005-2011 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VTFLib.h"
#include "VTFWrapper.h"
#include "VTFFile.h"

using namespace VTFLib;

//
// vlImageBound()
// Returns true if an image is bound, false otherwise.
//
VTFLIB_API vlBool vlImageIsBound(Diagnostics::CError& error)
{
	if(!bInitialized)
	{
		VTFError_Set(error, "VTFLib not initialized.");
		return vlFalse;
	}

	return Image != nullptr;
}

//
// vlBindImage()
// Bind an image to operate on.
// All library routines will use this image.
//
VTFLIB_API vlBool vlBindImage(uint32_t uiImage, Diagnostics::CError& error)
{
	if(!bInitialized)
	{
		VTFError_Set(error, "VTFLib not initialized.");
		return vlFalse;
	}

	if(uiImage >= ImageVector->size() || (*ImageVector)[uiImage] == nullptr)
	{
		VTFError_Set(error, "Invalid image.");
		return vlFalse;
	}

	if(Image == (*ImageVector)[uiImage])	// If it is already bound do nothing.
		return vlTrue;

	Image = (*ImageVector)[uiImage];

	return vlTrue;
}

//
// vlCreateImage()
// Create an image to work on.
//
VTFLIB_API vlBool vlCreateImage(uint32_t *uiImage, Diagnostics::CError& error)
{
	if(!bInitialized)
	{
		VTFError_Set(error, "VTFLib not initialized.");
		return vlFalse;
	}

	ImageVector->push_back(new CVTFFile());
	*uiImage = (uint32_t)ImageVector->size() - 1;

	return vlTrue;
}

//
// vlDeleteImage()
// Delete an image and all resources associated with it.
//
VTFLIB_API void vlDeleteImage(uint32_t uiImage)
{
	if(!bInitialized)
		return;

	if(uiImage >= ImageVector->size())
		return;

	if((*ImageVector)[uiImage] == nullptr)
		return;

	if((*ImageVector)[uiImage] == Image)
	{
		Image = nullptr;
	}

	delete (*ImageVector)[uiImage];
	(*ImageVector)[uiImage] = nullptr;
}

VTFLIB_API void vlImageCreateDefaultCreateStructure(SVTFCreateOptions *VTFCreateOptions)
{
	VTFCreateOptions->version[0] = VTF_MAJOR_VERSION;
	VTFCreateOptions->version[1] = VTF_MINOR_VERSION_DEFAULT;

	VTFCreateOptions->imageFormat = IMAGE_FORMAT_RGBA8888;

	VTFCreateOptions->flags = 0;
	VTFCreateOptions->startFrame = 0;
	VTFCreateOptions->bumpScale = 1.0f;
	VTFCreateOptions->reflectivityColor[0] = 1.0f;
	VTFCreateOptions->reflectivityColor[1] = 1.0f;
	VTFCreateOptions->reflectivityColor[2] = 1.0f;

	VTFCreateOptions->mipmaps = vlTrue;
	VTFCreateOptions->mipmapFilter = MIPMAP_FILTER_BOX;

	VTFCreateOptions->resize = vlFalse;
	VTFCreateOptions->resizeMethod = RESIZE_NEAREST_POWER2;
	VTFCreateOptions->resizeFilter = MIPMAP_FILTER_TRIANGLE;
	VTFCreateOptions->resizeWidth = 0;
	VTFCreateOptions->resizeHeight = 0;

	VTFCreateOptions->resizeClamp = vlTrue;
	VTFCreateOptions->resizeClampWidth = 4096;
	VTFCreateOptions->resizeClampHeight = 4096;

	VTFCreateOptions->thumbnail = vlTrue;
	VTFCreateOptions->reflectivity = vlTrue;

	VTFCreateOptions->gammaCorrection = vlFalse;
	VTFCreateOptions->gammaCorrectionValue = 2.0f;

	VTFCreateOptions->sphereMap = vlFalse;
	VTFCreateOptions->sRGB = vlFalse;

	VTFCreateOptions->auxCompressionLevel = VTF_AUX_COMPRESSION_LEVEL_NONE;
	VTFCreateOptions->auxCompressionMethod = AUX_COMPRESSION_METHOD_DEFLATE;
}

VTFLIB_API vlBool vlImageCreate(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiFrames, uint32_t uiFaces, uint32_t uiSlices, VTFImageFormat ImageFormat, vlBool bThumbnail, vlBool bMipmaps, vlBool bNullImageData, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->Create(uiWidth, uiHeight, error, uiFrames, uiFaces, uiSlices, ImageFormat, bThumbnail, bMipmaps, bNullImageData);
}

VTFLIB_API vlBool vlImageCreateSingle(uint32_t uiWidth, uint32_t uiHeight, uint8_t *lpImageDataRGBA8888, SVTFCreateOptions *VTFCreateOptions, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->Create(uiWidth, uiHeight, lpImageDataRGBA8888, *VTFCreateOptions, error);
}

VTFLIB_API vlBool vlImageCreateMultiple(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiFrames, uint32_t uiFaces, uint32_t uiSlices, uint8_t **lpImageDataRGBA8888, SVTFCreateOptions *VTFCreateOptions, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->Create(uiWidth, uiHeight, uiFrames, uiFaces, uiSlices, lpImageDataRGBA8888, *VTFCreateOptions, error);
}

VTFLIB_API void vlImageDestroy()
{
	if(Image == nullptr)
		return;

	Image->Destroy();
}

VTFLIB_API vlBool vlImageIsLoaded(Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->IsLoaded();
}

VTFLIB_API vlBool vlImageLoad(const char *cFileName, vlBool bHeaderOnly, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->Load(cFileName, error, bHeaderOnly);
}

VTFLIB_API vlBool vlImageLoadLump(const void *lpData, uint32_t uiBufferSize, vlBool bHeaderOnly, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->Load(lpData, uiBufferSize, error, bHeaderOnly);
}

VTFLIB_API vlBool vlImageLoadProc(void *pUserData, vlBool bHeaderOnly, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->Load(pUserData, error, bHeaderOnly);
}

VTFLIB_API vlBool vlImageSave(const char *cFileName, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->Save(cFileName, error);
}

VTFLIB_API vlBool vlImageSaveLump(void *lpData, uint32_t uiBufferSize, uint32_t *uiSize, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->Save(lpData, uiBufferSize, *uiSize, error);
}

VTFLIB_API vlBool vlImageSaveProc(void *pUserData, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		VTFError_Set(error, "No image bound.");
		return vlFalse;
	}

	return Image->Save(pUserData, error);
}

VTFLIB_API uint32_t vlImageGetMajorVersion()
{
	if(Image == nullptr)
		return 0;

	return Image->GetMajorVersion();
}

VTFLIB_API uint32_t vlImageGetMinorVersion()
{
	if(Image == nullptr)
		return 0;

	return Image->GetMinorVersion();
}

VTFLIB_API uint32_t vlImageGetSize(Diagnostics::CError& error)
{
	if(Image == nullptr)
		return 0;

	return Image->GetSize(error);
}

VTFLIB_API vlBool vlImageGetSupportsAuxCompression()
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GetSupportsAuxCompression();
}

VTFLIB_API int16_t vlImageGetAuxCompressionLevel()
{
	if(Image == nullptr)
		return VTF_AUX_COMPRESSION_LEVEL_NONE;

	return Image->GetAuxCompressionLevel();
}

VTFLIB_API vlBool vlImageSetAuxCompressionLevel(int16_t sLevel, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->SetAuxCompressionLevel(sLevel, error);
}

VTFLIB_API int16_t vlImageGetAuxCompressionMethod()
{
	if(Image == nullptr)
		return AUX_COMPRESSION_METHOD_DEFLATE;

	return Image->GetAuxCompressionMethod();
}

VTFLIB_API vlBool vlImageSetAuxCompressionMethod(int16_t sMethod, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->SetAuxCompressionMethod(sMethod, error);
}

VTFLIB_API uint32_t vlImageGetHasImage()
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GetHasImage();
}

VTFLIB_API uint32_t vlImageGetWidth()
{
	if(Image == nullptr)
		return 0;

	return Image->GetWidth();
}

VTFLIB_API uint32_t vlImageGetHeight()
{
	if(Image == nullptr)
		return 0;

	return Image->GetHeight();
}

VTFLIB_API uint32_t vlImageGetDepth()
{
	if(Image == nullptr)
		return 0;

	return Image->GetDepth();
}

VTFLIB_API uint32_t vlImageGetFrameCount()
{
	if(Image == nullptr)
		return 0;

	return Image->GetFrameCount();
}

VTFLIB_API uint32_t vlImageGetFaceCount()
{
	if(Image == nullptr)
		return 0;

	return Image->GetFaceCount();
}

VTFLIB_API uint32_t vlImageGetMipmapCount()
{
	if(Image == nullptr)
		return 0;

	return Image->GetMipmapCount();
}

VTFLIB_API uint32_t vlImageGetStartFrame()
{
	if(Image == nullptr)
		return 0;

	return Image->GetStartFrame();
}

VTFLIB_API void vlImageSetStartFrame(uint32_t uiStartFrame)
{
	if(Image == nullptr)
		return;

	Image->SetStartFrame(uiStartFrame);
}

VTFLIB_API uint32_t vlImageGetFlags()
{
	if(Image == nullptr)
		return 0;

	return Image->GetFlags();
}

VTFLIB_API void vlImageSetFlags(uint32_t uiFlags)
{
	if(Image == nullptr)
		return;

	Image->SetFlags(uiFlags);
}

VTFLIB_API vlBool vlImageGetFlag(VTFImageFlag ImageFlag)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GetFlag(ImageFlag);
}

VTFLIB_API void vlImageSetFlag(VTFImageFlag ImageFlag, vlBool bState)
{
	if(Image == nullptr)
		return;

	Image->SetFlag(ImageFlag, bState);
}

VTFLIB_API float vlImageGetBumpmapScale()
{
	if(Image == nullptr)
		return 0.0f;

	return Image->GetBumpmapScale();
}

VTFLIB_API void vlImageSetBumpmapScale(float sBumpmapScale)
{
	if(Image == nullptr)
		return;

	Image->SetBumpmapScale(sBumpmapScale);
}

VTFLIB_API void vlImageGetReflectivity(float *sX, float *sY, float *sZ)
{
	if(Image == nullptr)
		return;

	Image->GetReflectivity(*sX, *sY, *sZ);
}

VTFLIB_API void vlImageSetReflectivity(float sX, float sY, float sZ)
{
	if(Image == nullptr)
		return;

	Image->SetReflectivity(sX, sY, sZ);
}

VTFLIB_API VTFImageFormat vlImageGetFormat()
{
	if(Image == nullptr)
		return IMAGE_FORMAT_NONE;

	return Image->GetFormat();
}

VTFLIB_API uint8_t *vlImageGetData(uint32_t uiFrame, uint32_t uiFace, uint32_t uiSlice, uint32_t uiMipmapLevel)
{
	if(Image == nullptr)
		return nullptr;

	return Image->GetData(uiFrame, uiFace, uiSlice, uiMipmapLevel);
}

VTFLIB_API void vlImageSetData(uint32_t uiFrame, uint32_t uiFace, uint32_t uiSlice, uint32_t uiMipmapLevel, uint8_t *lpData)
{
	if(Image == nullptr)
		return;

	Image->SetData(uiFrame, uiFace, uiSlice, uiMipmapLevel, lpData);
}

VTFLIB_API vlBool vlImageGetHasThumbnail()
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GetHasThumbnail();
}

VTFLIB_API uint32_t vlImageGetThumbnailWidth()
{
	if(Image == nullptr)
		return 0;

	return Image->GetThumbnailWidth();
}

VTFLIB_API uint32_t vlImageGetThumbnailHeight()
{
	if(Image == nullptr)
		return 0;

	return Image->GetThumbnailHeight();
}

VTFLIB_API VTFImageFormat vlImageGetThumbnailFormat()
{
	if(Image == nullptr)
		return IMAGE_FORMAT_NONE;

	return Image->GetThumbnailFormat();
}

VTFLIB_API uint8_t *vlImageGetThumbnailData()
{
	if(Image == nullptr)
		return nullptr;

	return Image->GetThumbnailData();
}

VTFLIB_API void vlImageSetThumbnailData(uint8_t *lpData)
{
	if(Image == nullptr)
		return;

	Image->SetThumbnailData(lpData);
}

VTFLIB_API vlBool vlImageGetSupportsResources()
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GetSupportsResources();
}

VTFLIB_API uint32_t vlImageGetResourceCount()
{
	if(Image == nullptr)
		return 0;

	return Image->GetResourceCount();
}

VTFLIB_API uint32_t vlImageGetResourceType(uint32_t uiIndex)
{
	if(Image == nullptr)
		return 0;

	return Image->GetResourceType(uiIndex);
}

VTFLIB_API vlBool vlImageGetHasResource(uint32_t uiType)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GetHasResource(uiType);
}

VTFLIB_API void *vlImageGetResourceData(uint32_t uiType, uint32_t *uiSize, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return nullptr;

	return Image->GetResourceData(uiType, *uiSize, error);
}

VTFLIB_API void *vlImageSetResourceData(uint32_t uiType, uint32_t uiSize, void *lpData, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return nullptr;

	return Image->SetResourceData(uiType, uiSize, lpData, error);
}

VTFLIB_API vlBool vlImageGenerateMipmaps(uint32_t uiFace, uint32_t uiFrame, VTFMipmapFilter MipmapFilter, vlBool bSRGB, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GenerateMipmaps(uiFace, uiFrame, error, MipmapFilter, bSRGB);
}

VTFLIB_API vlBool vlImageGenerateAllMipmaps(VTFMipmapFilter MipmapFilter, vlBool bSRGB, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GenerateMipmaps(error, MipmapFilter, bSRGB);
}

VTFLIB_API vlBool vlImageGenerateThumbnail(vlBool bSRGB, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GenerateThumbnail(bSRGB, error);
}

VTFLIB_API vlBool vlImageGenerateNormalMap(uint32_t uiFrame, VTFKernelFilter KernelFilter, VTFHeightConversionMethod HeightConversionMethod, VTFNormalAlphaResult NormalAlphaResult, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GenerateNormalMap(uiFrame, error, KernelFilter, HeightConversionMethod, NormalAlphaResult);
}

VTFLIB_API vlBool vlImageGenerateAllNormalMaps(VTFKernelFilter KernelFilter, VTFHeightConversionMethod HeightConversionMethod, VTFNormalAlphaResult NormalAlphaResult, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GenerateNormalMap(error, KernelFilter, HeightConversionMethod, NormalAlphaResult);
}

VTFLIB_API vlBool vlImageGenerateSphereMap(Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GenerateSphereMap(error);
}

VTFLIB_API vlBool vlImageComputeReflectivity(Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->ComputeReflectivity(error);
}

VTFLIB_API SVTFImageFormatInfo const *vlImageGetImageFormatInfo(VTFImageFormat ImageFormat)
{
	return &CVTFFile::GetImageFormatInfo(ImageFormat);
}

VTFLIB_API vlBool vlImageGetImageFormatInfoEx(VTFImageFormat ImageFormat, SVTFImageFormatInfo *VTFImageFormatInfo)
{
	if(ImageFormat >= 0 && ImageFormat < IMAGE_FORMAT_COUNT)
	{
		memcpy(VTFImageFormatInfo, &CVTFFile::GetImageFormatInfo(ImageFormat), sizeof(SVTFImageFormatInfo));
		return vlTrue;
	}

	return vlFalse;
}

VTFLIB_API uint32_t vlImageComputeImageSize(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth, uint32_t uiMipmaps, VTFImageFormat ImageFormat)
{
	return CVTFFile::ComputeImageSize(uiWidth, uiHeight, uiDepth, uiMipmaps, ImageFormat);
}

VTFLIB_API uint32_t vlImageComputeMipmapCount(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth)
{
	return CVTFFile::ComputeMipmapCount(uiWidth, uiHeight, uiDepth);
}

VTFLIB_API void vlImageComputeMipmapDimensions(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth, uint32_t uiMipmapLevel, uint32_t *uiMipmapWidth, uint32_t *uiMipmapHeight, uint32_t *uiMipmapDepth)
{
	CVTFFile::ComputeMipmapDimensions(uiWidth, uiHeight, uiDepth, uiMipmapLevel, *uiMipmapWidth, *uiMipmapHeight, *uiMipmapDepth);
}

VTFLIB_API uint32_t vlImageComputeMipmapSize(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth, uint32_t uiMipmapLevel, VTFImageFormat ImageFormat)
{
	return CVTFFile::ComputeMipmapSize(uiWidth, uiHeight, uiDepth, uiMipmapLevel, ImageFormat);
}

VTFLIB_API vlBool vlImageConvertToRGBA8888(uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight, VTFImageFormat SourceFormat, Diagnostics::CError& error)
{
	return CVTFFile::ConvertToRGBA8888(lpSource, lpDest, uiWidth, uiHeight, SourceFormat, error);
}

VTFLIB_API vlBool vlImageConvertFromRGBA8888(uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight, VTFImageFormat DestFormat, Diagnostics::CError& error)
{
	return CVTFFile::ConvertFromRGBA8888(lpSource, lpDest, uiWidth, uiHeight, DestFormat, error);
}

VTFLIB_API vlBool vlImageConvert(uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight, VTFImageFormat SourceFormat, VTFImageFormat DestFormat, Diagnostics::CError& error)
{
	return CVTFFile::Convert(lpSource, lpDest, uiWidth, uiHeight, SourceFormat, DestFormat, error);
}

VTFLIB_API vlBool vlImageResize(uint8_t *lpSourceRGBA8888, uint8_t *lpDestRGBA8888, uint32_t uiSourceWidth, uint32_t uiSourceHeight, uint32_t uiDestWidth, uint32_t uiDestHeight, VTFMipmapFilter ResizeFilter, vlBool bSRGB, Diagnostics::CError& error)
{
	return CVTFFile::Resize(lpSourceRGBA8888, lpDestRGBA8888, uiSourceWidth, uiSourceHeight, uiDestWidth, uiDestHeight, ResizeFilter, bSRGB, error);
}

VTFLIB_API vlBool vlImageConvertToDistanceField(const uint8_t *lpSourceRGBA8888, uint8_t *lpDestRGBA8888, uint32_t uiSourceWidth, uint32_t uiSourceHeight, uint32_t uiDestWidth, uint32_t uiDestHeight, float sSpread, uint8_t bThreshold, vlBool *pbClipped, Diagnostics::CError& error)
{
	return CVTFFile::ConvertToDistanceField(lpSourceRGBA8888, lpDestRGBA8888, uiSourceWidth, uiSourceHeight, uiDestWidth, uiDestHeight, sSpread, bThreshold, pbClipped, error);
}

VTFLIB_API void vlImageCorrectImageGamma(uint8_t *lpImageDataRGBA8888, uint32_t uiWidth, uint32_t uiHeight, float sGammaCorrection)
{
	CVTFFile::CorrectImageGamma(lpImageDataRGBA8888, uiWidth, uiHeight, sGammaCorrection);
}

VTFLIB_API void vlImageComputeImageReflectivity(uint8_t *lpImageDataRGBA8888, uint32_t uiWidth, uint32_t uiHeight, float *sX, float *sY, float *sZ)
{
	CVTFFile::ComputeImageReflectivity(lpImageDataRGBA8888, uiWidth, uiHeight, *sX, *sY, *sZ);
}

VTFLIB_API void vlImageFlipImage(uint8_t *lpImageDataRGBA8888, uint32_t uiWidth, uint32_t uiHeight)
{
	CVTFFile::FlipImage(lpImageDataRGBA8888, uiWidth, uiHeight);
}

VTFLIB_API void vlImageMirrorImage(uint8_t *lpImageDataRGBA8888, uint32_t uiWidth, uint32_t uiHeight)
{
	CVTFFile::FlipImage(lpImageDataRGBA8888, uiWidth, uiHeight);
}