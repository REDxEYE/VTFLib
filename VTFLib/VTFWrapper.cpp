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
		error.Set("VTFLib not initialized.");
		return vlFalse;
	}

	return Image != nullptr;
}

//
// vlBindImage()
// Bind an image to operate on.
// All library routines will use this image.
//
VTFLIB_API vlBool vlBindImage(vlUInt uiImage, Diagnostics::CError& error)
{
	if(!bInitialized)
	{
		error.Set("VTFLib not initialized.");
		return vlFalse;
	}

	if(uiImage >= ImageVector->size() || (*ImageVector)[uiImage] == nullptr)
	{
		error.Set("Invalid image.");
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
VTFLIB_API vlBool vlCreateImage(vlUInt *uiImage, Diagnostics::CError& error)
{
	if(!bInitialized)
	{
		error.Set("VTFLib not initialized.");
		return vlFalse;
	}

	ImageVector->push_back(new CVTFFile());
	*uiImage = (vlUInt)ImageVector->size() - 1;

	return vlTrue;
}

//
// vlDeleteImage()
// Delete an image and all resources associated with it.
//
VTFLIB_API vlVoid vlDeleteImage(vlUInt uiImage)
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

VTFLIB_API vlVoid vlImageCreateDefaultCreateStructure(SVTFCreateOptions *VTFCreateOptions)
{
	VTFCreateOptions->uiVersion[0] = VTF_MAJOR_VERSION;
	VTFCreateOptions->uiVersion[1] = VTF_MINOR_VERSION_DEFAULT;

	VTFCreateOptions->ImageFormat = IMAGE_FORMAT_RGBA8888;

	VTFCreateOptions->uiFlags = 0;
	VTFCreateOptions->uiStartFrame = 0;
	VTFCreateOptions->sBumpScale = 1.0f;
	VTFCreateOptions->sReflectivity[0] = 1.0f;
	VTFCreateOptions->sReflectivity[1] = 1.0f;
	VTFCreateOptions->sReflectivity[2] = 1.0f;

	VTFCreateOptions->bMipmaps = vlTrue;
	VTFCreateOptions->MipmapFilter = MIPMAP_FILTER_BOX;

	VTFCreateOptions->bResize = vlFalse;
	VTFCreateOptions->ResizeMethod = RESIZE_NEAREST_POWER2;
	VTFCreateOptions->ResizeFilter = MIPMAP_FILTER_TRIANGLE;
	VTFCreateOptions->uiResizeWidth = 0;
	VTFCreateOptions->uiResizeHeight = 0;

	VTFCreateOptions->bResizeClamp = vlTrue;
	VTFCreateOptions->uiResizeClampWidth = 4096;
	VTFCreateOptions->uiResizeClampHeight = 4096;

	VTFCreateOptions->bThumbnail = vlTrue;
	VTFCreateOptions->bReflectivity = vlTrue;

	VTFCreateOptions->bGammaCorrection = vlFalse;
	VTFCreateOptions->sGammaCorrection = 2.0f;

	VTFCreateOptions->bSphereMap = vlFalse;
	VTFCreateOptions->bSRGB = vlFalse;

	VTFCreateOptions->sAuxCompressionLevel = VTF_AUX_COMPRESSION_LEVEL_NONE;
	VTFCreateOptions->sAuxCompressionMethod = AUX_COMPRESSION_METHOD_DEFLATE;
}

VTFLIB_API vlBool vlImageCreate(vlUInt uiWidth, vlUInt uiHeight, vlUInt uiFrames, vlUInt uiFaces, vlUInt uiSlices, VTFImageFormat ImageFormat, vlBool bThumbnail, vlBool bMipmaps, vlBool bNullImageData, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->Create(uiWidth, uiHeight, error, uiFrames, uiFaces, uiSlices, ImageFormat, bThumbnail, bMipmaps, bNullImageData);
}

VTFLIB_API vlBool vlImageCreateSingle(vlUInt uiWidth, vlUInt uiHeight, vlByte *lpImageDataRGBA8888, SVTFCreateOptions *VTFCreateOptions, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->Create(uiWidth, uiHeight, lpImageDataRGBA8888, *VTFCreateOptions, error);
}

VTFLIB_API vlBool vlImageCreateMultiple(vlUInt uiWidth, vlUInt uiHeight, vlUInt uiFrames, vlUInt uiFaces, vlUInt uiSlices, vlByte **lpImageDataRGBA8888, SVTFCreateOptions *VTFCreateOptions, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->Create(uiWidth, uiHeight, uiFrames, uiFaces, uiSlices, lpImageDataRGBA8888, *VTFCreateOptions, error);
}

VTFLIB_API vlVoid vlImageDestroy()
{
	if(Image == nullptr)
		return;

	Image->Destroy();
}

VTFLIB_API vlBool vlImageIsLoaded(Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->IsLoaded();
}

VTFLIB_API vlBool vlImageLoad(const vlChar *cFileName, vlBool bHeaderOnly, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->Load(cFileName, error, bHeaderOnly);
}

VTFLIB_API vlBool vlImageLoadLump(const vlVoid *lpData, vlUInt uiBufferSize, vlBool bHeaderOnly, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->Load(lpData, uiBufferSize, error, bHeaderOnly);
}

VTFLIB_API vlBool vlImageLoadProc(vlVoid *pUserData, vlBool bHeaderOnly, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->Load(pUserData, error, bHeaderOnly);
}

VTFLIB_API vlBool vlImageSave(const vlChar *cFileName, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->Save(cFileName, error);
}

VTFLIB_API vlBool vlImageSaveLump(vlVoid *lpData, vlUInt uiBufferSize, vlUInt *uiSize, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->Save(lpData, uiBufferSize, *uiSize, error);
}

VTFLIB_API vlBool vlImageSaveProc(vlVoid *pUserData, Diagnostics::CError& error)
{
	if(Image == nullptr)
	{
		error.Set("No image bound.");
		return vlFalse;
	}

	return Image->Save(pUserData);
}

VTFLIB_API vlUInt vlImageGetMajorVersion()
{
	if(Image == nullptr)
		return 0;

	return Image->GetMajorVersion();
}

VTFLIB_API vlUInt vlImageGetMinorVersion()
{
	if(Image == nullptr)
		return 0;

	return Image->GetMinorVersion();
}

VTFLIB_API vlUInt vlImageGetSize(Diagnostics::CError& error)
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

VTFLIB_API vlShort vlImageGetAuxCompressionLevel()
{
	if(Image == nullptr)
		return VTF_AUX_COMPRESSION_LEVEL_NONE;

	return Image->GetAuxCompressionLevel();
}

VTFLIB_API vlBool vlImageSetAuxCompressionLevel(vlShort sLevel, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->SetAuxCompressionLevel(sLevel, error);
}

VTFLIB_API vlShort vlImageGetAuxCompressionMethod()
{
	if(Image == nullptr)
		return AUX_COMPRESSION_METHOD_DEFLATE;

	return Image->GetAuxCompressionMethod();
}

VTFLIB_API vlBool vlImageSetAuxCompressionMethod(vlShort sMethod, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->SetAuxCompressionMethod(sMethod, error);
}

VTFLIB_API vlUInt vlImageGetHasImage()
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GetHasImage();
}

VTFLIB_API vlUInt vlImageGetWidth()
{
	if(Image == nullptr)
		return 0;

	return Image->GetWidth();
}

VTFLIB_API vlUInt vlImageGetHeight()
{
	if(Image == nullptr)
		return 0;

	return Image->GetHeight();
}

VTFLIB_API vlUInt vlImageGetDepth()
{
	if(Image == nullptr)
		return 0;

	return Image->GetDepth();
}

VTFLIB_API vlUInt vlImageGetFrameCount()
{
	if(Image == nullptr)
		return 0;

	return Image->GetFrameCount();
}

VTFLIB_API vlUInt vlImageGetFaceCount()
{
	if(Image == nullptr)
		return 0;

	return Image->GetFaceCount();
}

VTFLIB_API vlUInt vlImageGetMipmapCount()
{
	if(Image == nullptr)
		return 0;

	return Image->GetMipmapCount();
}

VTFLIB_API vlUInt vlImageGetStartFrame()
{
	if(Image == nullptr)
		return 0;

	return Image->GetStartFrame();
}

VTFLIB_API vlVoid vlImageSetStartFrame(vlUInt uiStartFrame)
{
	if(Image == nullptr)
		return;

	Image->SetStartFrame(uiStartFrame);
}

VTFLIB_API vlUInt vlImageGetFlags()
{
	if(Image == nullptr)
		return 0;

	return Image->GetFlags();
}

VTFLIB_API vlVoid vlImageSetFlags(vlUInt uiFlags)
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

VTFLIB_API vlVoid vlImageSetFlag(VTFImageFlag ImageFlag, vlBool bState)
{
	if(Image == nullptr)
		return;

	Image->SetFlag(ImageFlag, bState);
}

VTFLIB_API vlSingle vlImageGetBumpmapScale()
{
	if(Image == nullptr)
		return 0.0f;

	return Image->GetBumpmapScale();
}

VTFLIB_API vlVoid vlImageSetBumpmapScale(vlSingle sBumpmapScale)
{
	if(Image == nullptr)
		return;

	Image->SetBumpmapScale(sBumpmapScale);
}

VTFLIB_API vlVoid vlImageGetReflectivity(vlSingle *sX, vlSingle *sY, vlSingle *sZ)
{
	if(Image == nullptr)
		return;

	Image->GetReflectivity(*sX, *sY, *sZ);
}

VTFLIB_API vlVoid vlImageSetReflectivity(vlSingle sX, vlSingle sY, vlSingle sZ)
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

VTFLIB_API vlByte *vlImageGetData(vlUInt uiFrame, vlUInt uiFace, vlUInt uiSlice, vlUInt uiMipmapLevel)
{
	if(Image == nullptr)
		return nullptr;

	return Image->GetData(uiFrame, uiFace, uiSlice, uiMipmapLevel);
}

VTFLIB_API vlVoid vlImageSetData(vlUInt uiFrame, vlUInt uiFace, vlUInt uiSlice, vlUInt uiMipmapLevel, vlByte *lpData)
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

VTFLIB_API vlUInt vlImageGetThumbnailWidth()
{
	if(Image == nullptr)
		return 0;

	return Image->GetThumbnailWidth();
}

VTFLIB_API vlUInt vlImageGetThumbnailHeight()
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

VTFLIB_API vlByte *vlImageGetThumbnailData()
{
	if(Image == nullptr)
		return nullptr;

	return Image->GetThumbnailData();
}

VTFLIB_API vlVoid vlImageSetThumbnailData(vlByte *lpData)
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

VTFLIB_API vlUInt vlImageGetResourceCount()
{
	if(Image == nullptr)
		return 0;

	return Image->GetResourceCount();
}

VTFLIB_API vlUInt vlImageGetResourceType(vlUInt uiIndex)
{
	if(Image == nullptr)
		return 0;

	return Image->GetResourceType(uiIndex);
}

VTFLIB_API vlBool vlImageGetHasResource(vlUInt uiType)
{
	if(Image == nullptr)
		return vlFalse;

	return Image->GetHasResource(uiType);
}

VTFLIB_API vlVoid *vlImageGetResourceData(vlUInt uiType, vlUInt *uiSize, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return nullptr;

	return Image->GetResourceData(uiType, *uiSize, error);
}

VTFLIB_API vlVoid *vlImageSetResourceData(vlUInt uiType, vlUInt uiSize, vlVoid *lpData, Diagnostics::CError& error)
{
	if(Image == nullptr)
		return nullptr;

	return Image->SetResourceData(uiType, uiSize, lpData, error);
}

VTFLIB_API vlBool vlImageGenerateMipmaps(vlUInt uiFace, vlUInt uiFrame, VTFMipmapFilter MipmapFilter, vlBool bSRGB, Diagnostics::CError& error)
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

VTFLIB_API vlBool vlImageGenerateNormalMap(vlUInt uiFrame, VTFKernelFilter KernelFilter, VTFHeightConversionMethod HeightConversionMethod, VTFNormalAlphaResult NormalAlphaResult, Diagnostics::CError& error)
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

VTFLIB_API vlUInt vlImageComputeImageSize(vlUInt uiWidth, vlUInt uiHeight, vlUInt uiDepth, vlUInt uiMipmaps, VTFImageFormat ImageFormat)
{
	return CVTFFile::ComputeImageSize(uiWidth, uiHeight, uiDepth, uiMipmaps, ImageFormat);
}

VTFLIB_API vlUInt vlImageComputeMipmapCount(vlUInt uiWidth, vlUInt uiHeight, vlUInt uiDepth)
{
	return CVTFFile::ComputeMipmapCount(uiWidth, uiHeight, uiDepth);
}

VTFLIB_API vlVoid vlImageComputeMipmapDimensions(vlUInt uiWidth, vlUInt uiHeight, vlUInt uiDepth, vlUInt uiMipmapLevel, vlUInt *uiMipmapWidth, vlUInt *uiMipmapHeight, vlUInt *uiMipmapDepth)
{
	CVTFFile::ComputeMipmapDimensions(uiWidth, uiHeight, uiDepth, uiMipmapLevel, *uiMipmapWidth, *uiMipmapHeight, *uiMipmapDepth);
}

VTFLIB_API vlUInt vlImageComputeMipmapSize(vlUInt uiWidth, vlUInt uiHeight, vlUInt uiDepth, vlUInt uiMipmapLevel, VTFImageFormat ImageFormat)
{
	return CVTFFile::ComputeMipmapSize(uiWidth, uiHeight, uiDepth, uiMipmapLevel, ImageFormat);
}

VTFLIB_API vlBool vlImageConvertToRGBA8888(vlByte *lpSource, vlByte *lpDest, vlUInt uiWidth, vlUInt uiHeight, VTFImageFormat SourceFormat, Diagnostics::CError& error)
{
	return CVTFFile::ConvertToRGBA8888(lpSource, lpDest, uiWidth, uiHeight, SourceFormat, error);
}

VTFLIB_API vlBool vlImageConvertFromRGBA8888(vlByte *lpSource, vlByte *lpDest, vlUInt uiWidth, vlUInt uiHeight, VTFImageFormat DestFormat, Diagnostics::CError& error)
{
	return CVTFFile::ConvertFromRGBA8888(lpSource, lpDest, uiWidth, uiHeight, DestFormat, error);
}

VTFLIB_API vlBool vlImageConvert(vlByte *lpSource, vlByte *lpDest, vlUInt uiWidth, vlUInt uiHeight, VTFImageFormat SourceFormat, VTFImageFormat DestFormat, Diagnostics::CError& error)
{
	return CVTFFile::Convert(lpSource, lpDest, uiWidth, uiHeight, SourceFormat, DestFormat, error);
}

VTFLIB_API vlBool vlImageResize(vlByte *lpSourceRGBA8888, vlByte *lpDestRGBA8888, vlUInt uiSourceWidth, vlUInt uiSourceHeight, vlUInt uiDestWidth, vlUInt uiDestHeight, VTFMipmapFilter ResizeFilter, vlBool bSRGB, Diagnostics::CError& error)
{
	return CVTFFile::Resize(lpSourceRGBA8888, lpDestRGBA8888, uiSourceWidth, uiSourceHeight, uiDestWidth, uiDestHeight, ResizeFilter, bSRGB, error);
}

VTFLIB_API vlBool vlImageConvertToDistanceField(const vlByte *lpSourceRGBA8888, vlByte *lpDestRGBA8888, vlUInt uiSourceWidth, vlUInt uiSourceHeight, vlUInt uiDestWidth, vlUInt uiDestHeight, vlSingle sSpread, vlByte bThreshold, vlBool *pbClipped, Diagnostics::CError& error)
{
	return CVTFFile::ConvertToDistanceField(lpSourceRGBA8888, lpDestRGBA8888, uiSourceWidth, uiSourceHeight, uiDestWidth, uiDestHeight, sSpread, bThreshold, pbClipped, error);
}

VTFLIB_API vlVoid vlImageCorrectImageGamma(vlByte *lpImageDataRGBA8888, vlUInt uiWidth, vlUInt uiHeight, vlSingle sGammaCorrection)
{
	CVTFFile::CorrectImageGamma(lpImageDataRGBA8888, uiWidth, uiHeight, sGammaCorrection);
}

VTFLIB_API vlVoid vlImageComputeImageReflectivity(vlByte *lpImageDataRGBA8888, vlUInt uiWidth, vlUInt uiHeight, vlSingle *sX, vlSingle *sY, vlSingle *sZ)
{
	CVTFFile::ComputeImageReflectivity(lpImageDataRGBA8888, uiWidth, uiHeight, *sX, *sY, *sZ);
}

VTFLIB_API vlVoid vlImageFlipImage(vlByte *lpImageDataRGBA8888, vlUInt uiWidth, vlUInt uiHeight)
{
	CVTFFile::FlipImage(lpImageDataRGBA8888, uiWidth, uiHeight);
}

VTFLIB_API vlVoid vlImageMirrorImage(vlByte *lpImageDataRGBA8888, vlUInt uiWidth, vlUInt uiHeight)
{
	CVTFFile::FlipImage(lpImageDataRGBA8888, uiWidth, uiHeight);
}