/*
 * VTFLib
 * Copyright (C) 2005-2011 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include <algorithm>

#include "VTFLib.h"
#include "VTFFile.h"
#include "VTFFormat.h"
#include "VTFDXTn.h"
#include "VTFMathlib.h"

#include "compressonator.h"

#include "miniz.h"
#include "zstd.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#define _USE_MATH_DEFINES
#include <cmath>

#undef min
#undef max
#define PI		3.14159265358979323846

using namespace VTFLib;

// Class construction
// ------------------
CVTFFile::CVTFFile() {
    mHeader = nullptr;

    mImageBufferSize = 0;
    mImageData = nullptr;

    mThumbnailBufferSize = 0;
    mThumbnailImageData = nullptr;

    mAuxCompressionLevel = VTF_AUX_COMPRESSION_LEVEL_NONE;
    mAuxCompressionMethod = AUX_COMPRESSION_METHOD_DEFLATE;

    mAuxCompressedBufferSize = 0;
    mAuxCompressedData = nullptr;
    mAuxCompressionInfo = nullptr;
    mAuxCompressionInfoSize = 0;
}

//
// CVTFFile()
// Copy constructor.
//
CVTFFile::CVTFFile(const CVTFFile &other) {
    mHeader = nullptr;

    mImageBufferSize = 0;
    mImageData = nullptr;

    mThumbnailBufferSize = 0;
    mThumbnailImageData = nullptr;

    mAuxCompressionLevel = VTF_AUX_COMPRESSION_LEVEL_NONE;
    mAuxCompressionMethod = AUX_COMPRESSION_METHOD_DEFLATE;

    mAuxCompressedBufferSize = 0;
    mAuxCompressedData = nullptr;
    mAuxCompressionInfo = nullptr;
    mAuxCompressionInfoSize = 0;

    if (other.IsLoaded()) {
        mHeader = new SVTFHeader;
        memcpy(mHeader, other.mHeader, sizeof(SVTFHeader));

        mAuxCompressionLevel = other.mAuxCompressionLevel;
        mAuxCompressionMethod = other.mAuxCompressionMethod;

        if (other.GetHasImage()) {
            mImageBufferSize = other.mImageBufferSize;
            mImageData = new uint8_t[mImageBufferSize];
            memcpy(mImageData, other.mImageData, mImageBufferSize);
        }

        if (other.GetHasThumbnail()) {
            mThumbnailBufferSize = other.mThumbnailBufferSize;
            mThumbnailImageData = new uint8_t[mThumbnailBufferSize];
            memcpy(mThumbnailImageData, other.mThumbnailImageData, mThumbnailBufferSize);
        }
    }
}

//
// CVTFFile()
// Copy constructor.  Converts VTFFile to ImageFormat.
//
CVTFFile::CVTFFile(const CVTFFile &file, const VTFImageFormat imageFormat, Diagnostics::CError &error) {
    mHeader = nullptr;

    mImageBufferSize = 0;
    mImageData = nullptr;

    mThumbnailBufferSize = 0;
    mThumbnailImageData = nullptr;

    mAuxCompressionLevel = VTF_AUX_COMPRESSION_LEVEL_NONE;
    mAuxCompressionMethod = AUX_COMPRESSION_METHOD_DEFLATE;

    mAuxCompressedBufferSize = 0;
    mAuxCompressedData = nullptr;
    mAuxCompressionInfo = nullptr;
    mAuxCompressionInfoSize = 0;

    if (file.IsLoaded()) {
        mHeader = new SVTFHeader;
        memcpy(mHeader, file.mHeader, sizeof(SVTFHeader));

        mAuxCompressionLevel = file.mAuxCompressionLevel;
        mAuxCompressionMethod = file.mAuxCompressionMethod;

        // Set new format.
        mHeader->imageFormat = imageFormat;

        // Check flags.
        //if(Header->Version[0] < VTF_MAJOR_VERSION || (Header->Version[0] == VTF_MAJOR_VERSION && Header->Version[1] <= VTF_MINOR_VERSION_MIN_RESOURCE))
        //{
        //	if(!GetImageFormatInfo(ImageFormat).bIsCompressed)
        //	{
        //		Header->Flags |= TEXTUREFLAGS_DEPRECATED_NOCOMPRESS;
        //	}
        //	else
        //	{
        //		Header->Flags &= ~TEXTUREFLAGS_DEPRECATED_NOCOMPRESS;
        //	}
        //}

        if (GetImageFormatInfo(imageFormat).uiAlphaBitsPerPixel == 1) {
            mHeader->flags |= TEXTUREFLAGS_ONEBITALPHA;
        } else {
            mHeader->flags &= ~TEXTUREFLAGS_ONEBITALPHA;
        }

        if (GetImageFormatInfo(imageFormat).uiAlphaBitsPerPixel > 1) {
            mHeader->flags |= TEXTUREFLAGS_EIGHTBITALPHA;
        } else {
            mHeader->flags &= ~TEXTUREFLAGS_EIGHTBITALPHA;
        }

        // Convert image data.
        if (file.GetHasImage()) {
            uint32_t uiFrames = file.GetFrameCount();
            uint32_t uiFaces = file.GetFaceCount();
            uint32_t uiMipmaps = file.GetMipmapCount();
            uint32_t uiSlices = file.GetDepth();

            mImageBufferSize = ComputeImageSize(mHeader->width, mHeader->height, uiMipmaps,
                                                mHeader->imageFormat) * uiFrames * uiFaces;
            mImageData = new uint8_t[mImageBufferSize];

            //uint8_t *lpImageData = new uint8_t[ComputeImageSize(Header->Width, Header->Height, 1, IMAGE_FORMAT_RGBA8888)];

            for (uint32_t i = 0; i < uiFrames; i++) {
                for (uint32_t j = 0; j < uiFaces; j++) {
                    for (uint32_t k = 0; k < uiSlices; k++) {
                        for (uint32_t l = 0; l < uiMipmaps; l++) {
                            uint32_t uiMipmapWidth, uiMipmapHeight, uiMipmapDepth;
                            ComputeMipmapDimensions(mHeader->width, mHeader->height, 1, l,
                                                    uiMipmapWidth, uiMipmapHeight, uiMipmapDepth);

                            //ConvertToRGBA8888(VTFFile.GetData(i, j, k, l), lpImageData, uiMipmapWidth, uiMipmapHeight, VTFFile.GetFormat());
                            //ConvertFromRGBA8888(lpImageData, GetData(i, j, k, l), uiMipmapWidth, uiMipmapHeight, GetFormat());
                            Convert(file.GetData(i, j, k, l), GetData(i, j, k, l), uiMipmapWidth,
                                    uiMipmapHeight, file.GetFormat(), GetFormat(), error);
                        }
                    }
                }
            }

            //delete[] lpImageData;
        }

        // Convert thumbnail data.
        if (file.GetHasThumbnail()) {
            mThumbnailBufferSize = file.mThumbnailBufferSize;
            mThumbnailImageData = new uint8_t[mThumbnailBufferSize];
            memcpy(mThumbnailImageData, file.mThumbnailImageData, mThumbnailBufferSize);
        }
    }
}

// Class deconstruction
// ------------------
CVTFFile::~CVTFFile() {
    Destroy();
}

//
// Create()
// Creates a VTF file of the specified format and size.  Image data and other
// options must be set after creation.  Essential format flags are automatically
// generated.
//
vlBool CVTFFile::Create(const uint32_t width, const uint32_t uiHeight, Diagnostics::CError &error,
                        const uint32_t uiFrames,
                        const uint32_t uiFaces,
                        const uint32_t uiSlices, const VTFImageFormat ImageFormat, const vlBool bThumbnail,
                        const vlBool bMipmaps,
                        const vlBool bNullImageData) {
    Destroy();

    //
    // Check options.
    //

    if (width == 0 || width > 0xffff) {
        VTFError_Set_Formatted(error, "Invalid image width %u.  Width must be nonzero and no greater than %u.", width,
                           0xffff);
        return false;
    }

    if (uiHeight == 0 || uiHeight > 0xffff) {
        VTFError_Set_Formatted(error, "Invalid image height %u.  Height must be nonzero and no greater than %u.", uiHeight,
                           0xffff);
        return false;
    }

    if (uiSlices == 0 || uiSlices > 0xffff) {
        VTFError_Set_Formatted(error, "Invalid image depth %u.  Depth must be nonzero and no greater than %u.", uiSlices,
                           0xffff);
        return false;
    }

    if (ImageFormat <= IMAGE_FORMAT_NONE || ImageFormat >= IMAGE_FORMAT_COUNT) {
        VTFError_Set(error, "Invalid image format.");
        return false;
    }

    if (!GetImageFormatInfo(ImageFormat).bIsSupported) {
        VTFError_Set(error, "Image format not supported.");
        return false;
    }

    // block compressed formats
    if (GetImageFormatInfo(ImageFormat).bIsCompressed
        && ((width > 4 && (width % 4) != 0) || (uiHeight > 4 && (uiHeight % 4) != 0))) {
        VTFError_Set_Formatted(error,
            "Invalid image size %ux%u.  Compressed formats require dimensions that are a multiple of four.", width,
            uiHeight);
        return false;
    }

    if (uiFrames < 1 || uiFrames > 0xffff) {
        VTFError_Set_Formatted(error, "Invalid image frame count %u.", uiFrames);
        return false;
    }

    if (uiFaces != 1 && uiFaces != 6 && uiFaces != 7) {
        VTFError_Set_Formatted(error, "Invalid image face count %u.", uiFaces);
        return false;
    }

    if (uiFaces != 1 && uiFaces != 6 && VTF_MINOR_VERSION_DEFAULT >= VTF_MINOR_VERSION_MIN_NO_SPHERE_MAP) {
        VTFError_Set_Formatted(error, "Invalid image face count %u for version %d.%d.", uiFaces, VTF_MAJOR_VERSION,
                           VTF_MINOR_VERSION_DEFAULT);
        return false;
    }

    // Note: Valve informs us that animated enviroment maps ARE possible.

    // A image cannot have multiple frames and faces.
    // Logic: StartFrame is used as a flag when the texture is a TEXTUREFLAGS_ENVMAP.
    //if(uiFrames != 1 && uiFaces != 1)
    //{
    //	VTFError_Set(error, "Invalid image frame and face count.  An image cannot have multiple frames and faces.");
    //	return false;
    //}

    //
    // Generate header.
    //

    mHeader = new SVTFHeader;
    memset(mHeader, 0, sizeof(SVTFHeader));

    strcpy(mHeader->typeString, "VTF");
    mHeader->version[0] = VTF_MAJOR_VERSION;
    mHeader->version[1] = VTF_MINOR_VERSION_DEFAULT;
    mHeader->headerSize = 0;
    mHeader->width = (int16_t) width;
    mHeader->height = (int16_t) uiHeight;
    mHeader->flags = (GetImageFormatInfo(ImageFormat).uiAlphaBitsPerPixel == 1
                          ? TEXTUREFLAGS_ONEBITALPHA
                          : 0)
                     | (GetImageFormatInfo(ImageFormat).uiAlphaBitsPerPixel > 1
                            ? TEXTUREFLAGS_EIGHTBITALPHA
                            : 0)
                     | (uiFaces == 1 ? 0 : TEXTUREFLAGS_ENVMAP)
                     | (bMipmaps ? 0 : TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD);
    mHeader->frames = (int16_t) uiFrames;
    mHeader->startFrame = (uiFaces != 6 || VTF_MINOR_VERSION_DEFAULT >= VTF_MINOR_VERSION_MIN_NO_SPHERE_MAP)
                              ? 0
                              : 0xffff;
    mHeader->reflectivity[0] = 1.0f;
    mHeader->reflectivity[1] = 1.0f;
    mHeader->reflectivity[2] = 1.0f;
    mHeader->bumpScale = 1.0f;
    mHeader->imageFormat = ImageFormat;
    mHeader->mipCount = bMipmaps ? (uint8_t) ComputeMipmapCount(width, uiHeight, uiSlices) : 1;
    mHeader->depth = (int16_t) uiSlices;
    mHeader->resourceCount = 0;

    //
    // Generate thumbnail.
    //

    if (bThumbnail) {
        // Note: Valve informs us that DXT1 is the correct format.

        //  The format DXT1 was observed in almost every official .vtf file.
        mHeader->lowResImageFormat = IMAGE_FORMAT_DXT1;

        // Note: Valve informs us that the below is the right dimensions.

        // Find a thumbnail width and height (the first width and height <= 16 pixels).
        // The value 16 was observed in almost every official .vtf file.

        uint32_t uiThumbnailWidth = mHeader->width, uiThumbnailHeight = mHeader->height;

        while (true) {
            if (uiThumbnailWidth <= 16 && uiThumbnailHeight <= 16) {
                break;
            }

            uiThumbnailWidth >>= 1;
            uiThumbnailHeight >>= 1;

            if (uiThumbnailWidth < 1)
                uiThumbnailWidth = 1;

            if (uiThumbnailHeight < 1)
                uiThumbnailHeight = 1;
        }

        mHeader->lowResImageWidth = (uint8_t) uiThumbnailWidth;
        mHeader->lowResImageHeight = (uint8_t) uiThumbnailHeight;

        mThumbnailBufferSize = ComputeImageSize(mHeader->lowResImageWidth,
                                                mHeader->lowResImageHeight, 1,
                                                mHeader->lowResImageFormat);
        mThumbnailImageData = new uint8_t[mThumbnailBufferSize];

        mHeader->resources[mHeader->resourceCount++].type = VTF_LEGACY_RSRC_LOW_RES_IMAGE;
    } else {
        mHeader->lowResImageFormat = IMAGE_FORMAT_NONE;
        mHeader->lowResImageWidth = 0;
        mHeader->lowResImageHeight = 0;

        mThumbnailBufferSize = 0;
        mThumbnailImageData = nullptr;
    }

    //
    // Generate image.
    //

    mImageBufferSize = ComputeImageSize(mHeader->width, mHeader->height, mHeader->depth,
                                        mHeader->mipCount,
                                        mHeader->imageFormat) * uiFrames * uiFaces;
    mImageData = new uint8_t[mImageBufferSize];

    mHeader->resources[mHeader->resourceCount++].type = VTF_LEGACY_RSRC_IMAGE;

    //
    // Null image data.
    //

    if (bNullImageData) {
        memset(mThumbnailImageData, 0, mThumbnailBufferSize);
        memset(mImageData, 0, mImageBufferSize);
    }

    ComputeResources();

    return true;
}

//
// Create()
// Creates a VTF file of the specified format and size using the provided image RGBA data.
// Can also generate mipmaps and a thumbnail.  Recommended function for high level single
// face/frame VTF file creation.
//
vlBool CVTFFile::Create(const uint32_t uiWidth, const uint32_t uiHeight, uint8_t *lpImageDataRGBA8888,
                        const SVTFCreateOptions &VTFCreateOptions, Diagnostics::CError &error) {
    return Create(uiWidth, uiHeight, 1, 1, 1, &lpImageDataRGBA8888, VTFCreateOptions, error);
}

static CMP_FORMAT GetCMPFormat(const VTFImageFormat imageFormat, const bool bDXT5GA) {
    if (bDXT5GA)
        return CMP_FORMAT_ATI2N_DXT5;

    switch (imageFormat) {
        case IMAGE_FORMAT_BGR888: return CMP_FORMAT_BGR_888;
        case IMAGE_FORMAT_RGB888: return CMP_FORMAT_RGB_888;
        case IMAGE_FORMAT_RGBA8888: return CMP_FORMAT_RGBA_8888;
        case IMAGE_FORMAT_BGRA8888: return CMP_FORMAT_BGRA_8888;

        case IMAGE_FORMAT_DXT1_ONEBITALPHA: return CMP_FORMAT_DXT1;
        case IMAGE_FORMAT_DXT1: return CMP_FORMAT_DXT1;
        case IMAGE_FORMAT_DXT3: return CMP_FORMAT_DXT3;
        case IMAGE_FORMAT_DXT5: return CMP_FORMAT_DXT5;
        case IMAGE_FORMAT_ATI1N: return CMP_FORMAT_ATI1N;
        case IMAGE_FORMAT_ATI2N: return CMP_FORMAT_ATI2N;
        case IMAGE_FORMAT_BC4: return CMP_FORMAT_BC4;
        case IMAGE_FORMAT_BC5: return CMP_FORMAT_BC5;
        case IMAGE_FORMAT_BC7: return CMP_FORMAT_BC7;
        case IMAGE_FORMAT_BC6H: return CMP_FORMAT_BC6H_SF;

        default: return CMP_FORMAT_Unknown;
    }
}

// BC6H stores half floats, so it has to be compressed from (and decompressed to)
// an uncompressed HDR format rather than the usual RGBA8888
static VTFImageFormat GetUncompressedFormat(const VTFImageFormat CompressedFormat) {
    return CompressedFormat == IMAGE_FORMAT_BC6H ? IMAGE_FORMAT_RGBA16161616F : IMAGE_FORMAT_RGBA8888;
}

static const char *GetCMPErrorString(const CMP_ERROR error) {
    switch (error) {
        case CMP_OK: return "Ok.";
        case CMP_ABORTED: return "The conversion was aborted.";
        case CMP_ERR_INVALID_SOURCE_TEXTURE: return "The source texture is invalid.";
        case CMP_ERR_INVALID_DEST_TEXTURE: return "The destination texture is invalid.";
        case CMP_ERR_UNSUPPORTED_SOURCE_FORMAT: return "The source format is not a supported format.";
        case CMP_ERR_UNSUPPORTED_DEST_FORMAT: return "The destination format is not a supported format.";
        case CMP_ERR_UNSUPPORTED_GPU_ASTC_DECODE: return "The gpu hardware is not supported.";
        case CMP_ERR_UNSUPPORTED_GPU_BASIS_DECODE: return "The gpu hardware is not supported.";
        case CMP_ERR_SIZE_MISMATCH: return "The source and destination texture sizes do not match.";
        case CMP_ERR_UNABLE_TO_INIT_CODEC: return
                    "Compressonator was unable to initialize the codec needed for conversion.";
        case CMP_ERR_UNABLE_TO_INIT_DECOMPRESSLIB: return
                    "GPU_Decode Lib was unable to initialize the codec needed for decompression .";
        case CMP_ERR_UNABLE_TO_INIT_COMPUTELIB: return
                    "Compute Lib was unable to initialize the codec needed for compression.";
        case CMP_ERR_CMP_DESTINATION: return "Error in compressing destination texture";
        case CMP_ERR_MEM_ALLOC_FOR_MIPSET: return "Memory Error: allocating MIPSet compression level data buffer";
        case CMP_ERR_UNKNOWN_DESTINATION_FORMAT: return
                    "The destination Codec Type is unknown! In SDK refer to GetCodecType()";
        case CMP_ERR_FAILED_HOST_SETUP: return "Failed to setup Host for processing";
        case CMP_ERR_PLUGIN_FILE_NOT_FOUND: return "The required plugin library was not found";
        case CMP_ERR_UNABLE_TO_LOAD_FILE: return "The requested file was not loaded";
        case CMP_ERR_UNABLE_TO_CREATE_ENCODER: return "Request to create an encoder failed";
        case CMP_ERR_UNABLE_TO_LOAD_ENCODER: return "Unable to load an encode library";
        case CMP_ERR_NOSHADER_CODE_DEFINED: return "No shader code is available for the requested framework";
        case CMP_ERR_GPU_DOESNOT_SUPPORT_COMPUTE: return "The GPU device selected does not support compute";
        case CMP_ERR_NOPERFSTATS: return "No Performance Stats are available";
        case CMP_ERR_GPU_DOESNOT_SUPPORT_CMP_EXT: return
                    "The GPU does not support the requested compression extension!";
        case CMP_ERR_GAMMA_OUTOFRANGE: return "Gamma value set for processing is out of range";
        case CMP_ERR_PLUGIN_SHAREDIO_NOT_SET: return
                    "The plugin C_PluginSetSharedIO call was not set and is required for this plugin to operate";
        case CMP_ERR_UNABLE_TO_INIT_D3DX: return "Unable to initialize DirectX SDK or get a specific DX API";
        default:
        case CMP_ERR_GENERIC: return "An unknown error occurred.";
    }
}

//
// Create()
// Creates a VTF file of the specified format and size using the provided image RGBA data.
// Can also generate mipmaps and a thumbnail.  Recommended function for high level multiple
// face/frame VTF file creation.
//
vlBool CVTFFile::Create(uint32_t uiWidth, uint32_t uiHeight, const uint32_t uiFrames, const uint32_t uiFaces,
                        const uint32_t vlSlices,
                        uint8_t **lpImageDataRGBA8888, const SVTFCreateOptions &VTFCreateOptions,
                        Diagnostics::CError &error) {
    uint32_t uiCount = 0;
    if (uiFrames > uiCount)
        uiCount = uiFrames;
    if (uiFaces > uiCount)
        uiCount = uiFaces;
    if (vlSlices > uiCount)
        uiCount = vlSlices;
    uint8_t **lpNewImageDataRGBA8888 = nullptr;

    if ((uiFrames == 1 && uiFaces > 1 && vlSlices > 1) || (uiFrames > 1 && uiFaces == 1 && vlSlices > 1) || (
            uiFrames > 1 && uiFaces > 1 && vlSlices == 1)) {
        VTFError_Set(error,
            "Invalid image frame, face and slice count combination.  Function does not support images with any combination of multiple frames or faces or slices.");
        return false;
    }

    if (VTFCreateOptions.version[0] != VTF_MAJOR_VERSION || (
            VTFCreateOptions.version[1] < 0 || VTFCreateOptions.version[1] > VTF_MINOR_VERSION)) {
        VTFError_Set_Formatted(error, "File version %u.%u does not match %d.%d to %d.%d.", VTFCreateOptions.version[0],
                           VTFCreateOptions.version[1], VTF_MAJOR_VERSION, 0, VTF_MAJOR_VERSION,
                           VTF_MINOR_VERSION);
        return false;
    }

    if (VTFCreateOptions.version[0] == VTF_MAJOR_VERSION && VTFCreateOptions.version[1] <
        VTF_MINOR_VERSION_MIN_VOLUME && vlSlices > 1) {
        VTFError_Set_Formatted(error, "Volume textures are only supported in version %d.%d and up.", VTF_MAJOR_VERSION,
                           VTF_MINOR_VERSION_MIN_VOLUME);
        return false;
    }

    if (VTFCreateOptions.version[0] == VTF_MAJOR_VERSION && VTFCreateOptions.version[1] <
        VTF_MINOR_VERSION_MIN_SPHERE_MAP && uiFaces == 7) {
        VTFError_Set_Formatted(error, "Sphere maps are only supported in version %d.%d and up.", VTF_MAJOR_VERSION,
                           VTF_MINOR_VERSION_MIN_SPHERE_MAP);
        return false;
    }

    if (VTFCreateOptions.mipmaps && vlSlices > 1) {
        VTFError_Set(error, "Mipmap generation for depth textures is not supported.");
        return false;
    }

    try {
        if (VTFCreateOptions.resize) {
            uint32_t uiNewWidth = uiWidth;
            uint32_t uiNewHeight = uiHeight;

            switch (VTFCreateOptions.resizeMethod) {
                case RESIZE_NEAREST_POWER2:
                case RESIZE_BIGGEST_POWER2:
                case RESIZE_SMALLEST_POWER2:
                case RESIZE_NEAREST_MULTIPLE4:
                case RESIZE_BIGGEST_MULTIPLE4:
                case RESIZE_SMALLEST_MULTIPLE4:
                    uiNewWidth = ComputeResizedDimension(uiWidth, VTFCreateOptions.resizeMethod);
                    if (VTFCreateOptions.resizeClamp && uiNewWidth > VTFCreateOptions.resizeClampWidth) {
                        uiNewWidth = VTFCreateOptions.resizeClampWidth;
                    }

                    uiNewHeight = ComputeResizedDimension(uiHeight, VTFCreateOptions.resizeMethod);
                    if (VTFCreateOptions.resizeClamp && uiNewHeight > VTFCreateOptions.resizeClampHeight) {
                        uiNewHeight = VTFCreateOptions.resizeClampHeight;
                    }
                    break;
                case RESIZE_SET:
                    uiNewWidth = VTFCreateOptions.resizeWidth;
                    uiNewHeight = VTFCreateOptions.resizeHeight;
                    break;
            }

            // Resize the input.
            if (uiWidth != uiNewWidth || uiHeight != uiNewHeight) {
                lpNewImageDataRGBA8888 = new uint8_t *[uiCount];
                memset(lpNewImageDataRGBA8888, 0, uiCount * sizeof(uint8_t *));

                for (uint32_t i = 0; i < uiCount; i++) {
                    lpNewImageDataRGBA8888[i] = new uint8_t[ComputeImageSize(
                        uiNewWidth, uiNewHeight, 1, IMAGE_FORMAT_RGBA8888)];

                    if (!Resize(lpImageDataRGBA8888[i], lpNewImageDataRGBA8888[i], uiWidth, uiHeight, uiNewWidth,
                                uiNewHeight, VTFCreateOptions.resizeFilter, VTFCreateOptions.sRGB, error)) {
                        throw 0;
                    }
                }

                uiWidth = uiNewWidth;
                uiHeight = uiNewHeight;

                lpImageDataRGBA8888 = lpNewImageDataRGBA8888;
            }
        }

        // Create image (allocate and setup structures).
        if (!Create(uiWidth, uiHeight, error, uiFrames,
                    uiFaces + (VTFCreateOptions.sphereMap && uiFaces == 6 ? 1 : 0), vlSlices,
                    VTFCreateOptions.imageFormat,
                    VTFCreateOptions.thumbnail, VTFCreateOptions.mipmaps, false)) {
            throw 0;
        }

        // Update version, for the current versions with the current checking this should be sufficient.
        mHeader->version[0] = VTFCreateOptions.version[0];
        mHeader->version[1] = VTFCreateOptions.version[1];

        if (GetSupportsAuxCompression() && VTFCreateOptions.auxCompressionLevel !=
            VTF_AUX_COMPRESSION_LEVEL_NONE) {
            if (!SetAuxCompressionMethod(VTFCreateOptions.auxCompressionMethod, error) || !
                SetAuxCompressionLevel(
                    VTFCreateOptions.auxCompressionLevel, error)) {
                throw 0;
            }
        }

        ComputeResources();

        // Do gamma correction.
        if (VTFCreateOptions.gammaCorrection) {
            for (uint32_t i = 0; i < uiFrames; i++) {
                for (uint32_t j = 0; j < uiFaces; j++) {
                    for (uint32_t k = 0; k < vlSlices; k++) {
                        CorrectImageGamma(lpImageDataRGBA8888[i + j + k], mHeader->width,
                                          mHeader->height, VTFCreateOptions.gammaCorrectionValue);
                    }
                }
            }
        }

        // Generate mipmaps off source image.
        if (VTFCreateOptions.mipmaps && mHeader->mipCount != 1) {
            auto temp = std::vector<uint8_t>(mHeader->width * mHeader->height * 4);

            for (uint32_t i = 0; i < uiFrames; i++) {
                for (uint32_t j = 0; j < uiFaces; j++) {
                    for (uint32_t k = 0; k < vlSlices; k++) {
                        uint8_t *pSource = lpImageDataRGBA8888[i + j + k];

                        if (!ConvertFromRGBA8888(pSource, GetData(i, j, k, 0), mHeader->width,
                                                 mHeader->height, mHeader->imageFormat, error)) {
                            throw 0;
                        }

                        for (uint32_t m = 1; m < mHeader->mipCount; m++) {
                            uint16_t usWidth = std::max(1u, static_cast<uint32_t>(mHeader->width) >> m);
                            uint16_t usHeight = std::max(1u, static_cast<uint32_t>(mHeader->height) >> m);

                            if (!Resize(
                                pSource, temp.data(),
                                mHeader->width, mHeader->height,
                                usWidth, usHeight,
                                VTFCreateOptions.mipmapFilter, VTFCreateOptions.sRGB, error)) {
                                throw 0;
                            }

                            if (!ConvertFromRGBA8888(temp.data(), GetData(i, j, k, m), usWidth, usHeight,
                                                     mHeader->imageFormat, error)) {
                                throw 0;
                            }
                        }
                    }
                }
            }
        } else {
            for (uint32_t i = 0; i < uiFrames; i++) {
                for (uint32_t j = 0; j < uiFaces; j++) {
                    for (uint32_t k = 0; k < vlSlices; k++) {
                        if (!ConvertFromRGBA8888(lpImageDataRGBA8888[i + j + k], GetData(i, j, k, 0),
                                                 mHeader->width, mHeader->height,
                                                 mHeader->imageFormat, error)) {
                            throw 0;
                        }
                    }
                }
            }
        }

        // Generate thumbnail off mipmaps.
        if (VTFCreateOptions.thumbnail) {
            if (!GenerateThumbnail(VTFCreateOptions.sRGB, error)) {
                throw 0;
            }
        }

        if (VTFCreateOptions.sphereMap && uiFaces == 6) {
            if (!GenerateSphereMap(error)) {
                throw 0;
            }
        }

        if (VTFCreateOptions.reflectivity) {
            mHeader->reflectivity[0] = 0.0f;
            mHeader->reflectivity[1] = 0.0f;
            mHeader->reflectivity[2] = 0.0f;

            for (uint32_t i = 0; i < uiFrames; i++) {
                for (uint32_t j = 0; j < uiFaces; j++) {
                    for (uint32_t k = 0; k < vlSlices; k++) {
                        float sX, sY, sZ;
                        ComputeImageReflectivity(lpImageDataRGBA8888[i + j + k], uiWidth, uiHeight, sX, sY, sZ);

                        mHeader->reflectivity[0] += sX;
                        mHeader->reflectivity[1] += sY;
                        mHeader->reflectivity[2] += sZ;
                    }
                }
            }

            float sInverse = 1.0f / (float) (uiFrames * uiFaces * vlSlices);

            mHeader->reflectivity[0] *= sInverse;
            mHeader->reflectivity[1] *= sInverse;
            mHeader->reflectivity[2] *= sInverse;
        } else {
            SetReflectivity(VTFCreateOptions.reflectivityColor[0], VTFCreateOptions.reflectivityColor[1],
                            VTFCreateOptions.reflectivityColor[2]);
        }

        // Set the flags, call SetFlag() to make sure we don't set anything we shouldn't.
        for (uint32_t i = 0, uiFlag = 0x00000001; i < TEXTUREFLAGS_COUNT; i++, uiFlag <<= 1) {
            if (VTFCreateOptions.flags & uiFlag) {
                SetFlag((VTFImageFlag) uiFlag, true);
            }
        }
        SetStartFrame(VTFCreateOptions.startFrame);
        SetBumpmapScale(VTFCreateOptions.bumpScale);

        // The engine does not load DXT1_ONEBITALPHA textures correctly
        // but it does handle plain DXT1 with the one bit alpha flag set
        if (mHeader->imageFormat == IMAGE_FORMAT_DXT1_ONEBITALPHA) {
            mHeader->imageFormat = IMAGE_FORMAT_DXT1;
            mHeader->flags |= TEXTUREFLAGS_ONEBITALPHA;
        }

        return true;
    } catch (...) {
        if (lpNewImageDataRGBA8888 != nullptr) {
            for (uint32_t i = 0; i < uiCount; i++) {
                delete[] lpNewImageDataRGBA8888[i];
            }
            delete[] lpNewImageDataRGBA8888;
        }

        Destroy();

        return false;
    }
}

//
// Destroy()
// Frees all resources associated with the curret image.
//
void CVTFFile::Destroy() {
    if (mHeader != nullptr) {
        for (uint32_t i = 0; i < mHeader->resourceCount; i++) {
            delete[] mHeader->data[i].data;
        }
    }

    delete mHeader;
    mHeader = nullptr;

    mImageBufferSize = 0;
    delete[] mImageData;
    mImageData = nullptr;

    mThumbnailBufferSize = 0;
    delete[] mThumbnailImageData;
    mThumbnailImageData = nullptr;

    mAuxCompressionLevel = VTF_AUX_COMPRESSION_LEVEL_NONE;
    mAuxCompressionMethod = AUX_COMPRESSION_METHOD_DEFLATE;

    DestroyAuxCompression();
}

//
// DestroyAuxCompression()
// Throws away the cached compressed copy of the image data.
//
void CVTFFile::DestroyAuxCompression() {
    mAuxCompressedBufferSize = 0;
    delete[] mAuxCompressedData;
    mAuxCompressedData = nullptr;

    mAuxCompressionInfoSize = 0;
    delete[] mAuxCompressionInfo;
    mAuxCompressionInfo = nullptr;
}

//
// ComputeAuxCompression()
// Compresses each mipmap/frame/face chunk of the image data separately and builds the matching AXC resource payload.
// Chunks are visited in the order they appear in the file: smallest mipmap first, then frame, then face.
// All slices of a 3D texture are compressed as one chunk.
//
vlBool CVTFFile::ComputeAuxCompression(const vlBool force, Diagnostics::CError &error) {
    if (!GetSupportsAuxCompression() || mAuxCompressionLevel == VTF_AUX_COMPRESSION_LEVEL_NONE || !
        GetHasImage()) {
        DestroyAuxCompression();
        return false;
    }

    if (!force && mAuxCompressedData != nullptr) {
        return true;
    }

    DestroyAuxCompression();

    uint32_t uiFrameCount = GetFrameCount();
    uint32_t uiFaceCount = GetFaceCount();
    uint32_t uiMipmapCount = GetMipmapCount();
    uint32_t uiChunkCount = uiMipmapCount * uiFrameCount * uiFaceCount;

    uint32_t uiBound = 0;
    for (int32_t i = (int32_t) uiMipmapCount - 1; i >= 0; i--) {
        uint32_t uiChunkSize = ComputeMipmapSize(mHeader->width, mHeader->height,
                                                 mHeader->depth,
                                                 (uint32_t) i, mHeader->imageFormat);

        uiBound += (uint32_t) (mAuxCompressionMethod == AUX_COMPRESSION_METHOD_ZSTD
                                   ? ZSTD_compressBound(uiChunkSize)
                                   : mz_compressBound(uiChunkSize)) * uiFrameCount * uiFaceCount;
    }

    uint8_t *lpCompressedData = new uint8_t[uiBound];
    uint32_t *lpSizes = new uint32_t[uiChunkCount];

    uint32_t uiSourceOffset = 0, uiDestOffset = 0, uiChunk = 0;
    vlBool bResult = true;

    for (int32_t i = (int32_t) uiMipmapCount - 1; i >= 0 && bResult; i--) {
        uint32_t uiChunkSize = ComputeMipmapSize(mHeader->width, mHeader->height,
                                                 mHeader->depth,
                                                 (uint32_t) i, mHeader->imageFormat);

        for (uint32_t j = 0; j < uiFrameCount && bResult; j++) {
            for (uint32_t k = 0; k < uiFaceCount && bResult; k++, uiChunk++) {
                uint32_t uiCompressedSize = 0;

                switch (mAuxCompressionMethod) {
                    case AUX_COMPRESSION_METHOD_ZSTD: {
                        int16_t sLevel = mAuxCompressionLevel < 0 ? 6 : mAuxCompressionLevel;
                        size_t uiResult = ZSTD_compress(lpCompressedData + uiDestOffset, uiBound - uiDestOffset,
                                                        mImageData + uiSourceOffset, uiChunkSize, sLevel);
                        if (ZSTD_isError(uiResult)) {
                            VTFError_Set_Formatted(error, "Error compressing image data with Zstandard: %s.",
                                               ZSTD_getErrorName(uiResult));
                            bResult = false;
                        } else {
                            uiCompressedSize = (uint32_t) uiResult;
                        }
                    }
                    break;
                    case AUX_COMPRESSION_METHOD_DEFLATE: {
                        mz_ulong uiResult = (mz_ulong) (uiBound - uiDestOffset);
                        if (mz_compress2(lpCompressedData + uiDestOffset, &uiResult, mImageData + uiSourceOffset,
                                         uiChunkSize, mAuxCompressionLevel) != MZ_OK) {
                            VTFError_Set(error, "Error compressing image data with deflate.");
                            bResult = false;
                        } else {
                            uiCompressedSize = (uint32_t) uiResult;
                        }
                    }
                    break;
                    default:
                        VTFError_Set_Formatted(error, "Unsupported auxiliary compression method %d.",
                                           mAuxCompressionMethod);
                        bResult = false;
                        break;
                }

                if (bResult) {
                    lpSizes[uiChunk] = uiCompressedSize;
                    uiDestOffset += uiCompressedSize;
                    uiSourceOffset += uiChunkSize;
                }
            }
        }
    }

    if (!bResult) {
        delete[] lpCompressedData;
        delete[] lpSizes;
        return false;
    }

    mAuxCompressedBufferSize = uiDestOffset;
    mAuxCompressedData = lpCompressedData;

    mAuxCompressionInfoSize = sizeof(SVTFAuxCompressionInfoHeader) + uiChunkCount * sizeof(uint32_t);
    mAuxCompressionInfo = new uint8_t[mAuxCompressionInfoSize];

    SVTFAuxCompressionInfoHeader *Info = (SVTFAuxCompressionInfoHeader *) mAuxCompressionInfo;
    Info->level = mAuxCompressionLevel;
    Info->method = mAuxCompressionMethod;
    memcpy(mAuxCompressionInfo + sizeof(SVTFAuxCompressionInfoHeader), lpSizes, uiChunkCount * sizeof(uint32_t));

    delete[] lpSizes;

    return true;
}

//
// DecompressAuxData()
// Expands aux compressed image data read from a file into lpDest.
// The compressed chunk sizes come from the AXC resource payload described by lpInfo.
//
static vlBool DecompressAuxData(const CVTFFile *VTFFile, const uint8_t *lpSource, const uint32_t uiSourceSize,
                                uint8_t *lpDest,
                                const uint32_t uiDestSize, const uint8_t *lpInfo, const uint32_t uiInfoSize,
                                const int16_t sMethod,
                                Diagnostics::CError &error) {
    uint32_t uiFrameCount = VTFFile->GetFrameCount();
    uint32_t uiFaceCount = VTFFile->GetFaceCount();
    uint32_t uiMipmapCount = VTFFile->GetMipmapCount();
    uint32_t uiChunkCount = uiMipmapCount * uiFrameCount * uiFaceCount;

    if (uiInfoSize < sizeof(SVTFAuxCompressionInfoHeader) + uiChunkCount * sizeof(uint32_t)) {
        VTFError_Set(error, "File may be corrupt; auxiliary compression resource is too small.");
        return false;
    }

    const uint32_t *lpSizes = (const uint32_t *) (lpInfo + sizeof(SVTFAuxCompressionInfoHeader));

    uint32_t uiSourceOffset = 0, uiDestOffset = 0, uiChunk = 0;

    for (int32_t i = (int32_t) uiMipmapCount - 1; i >= 0; i--) {
        uint32_t uiChunkSize = CVTFFile::ComputeMipmapSize(VTFFile->GetWidth(), VTFFile->GetHeight(),
                                                           VTFFile->GetDepth(),
                                                           (uint32_t) i, VTFFile->GetFormat());

        for (uint32_t j = 0; j < uiFrameCount; j++) {
            for (uint32_t k = 0; k < uiFaceCount; k++, uiChunk++) {
                uint32_t uiCompressedSize = lpSizes[uiChunk];

                if (uiSourceOffset + uiCompressedSize > uiSourceSize || uiDestOffset + uiChunkSize > uiDestSize) {
                    VTFError_Set(error, "File may be corrupt; auxiliary compressed image data is truncated.");
                    return false;
                }

                switch (sMethod) {
                    case AUX_COMPRESSION_METHOD_ZSTD: {
                        size_t uiResult = ZSTD_decompress(lpDest + uiDestOffset, uiChunkSize, lpSource + uiSourceOffset,
                                                          uiCompressedSize);
                        if (ZSTD_isError(uiResult) || uiResult != uiChunkSize) {
                            VTFError_Set(error, "Error decompressing Zstandard compressed image data.");
                            return false;
                        }
                    }
                    break;
                    case AUX_COMPRESSION_METHOD_DEFLATE: {
                        mz_ulong uiResult = uiChunkSize;
                        if (mz_uncompress(lpDest + uiDestOffset, &uiResult, lpSource + uiSourceOffset, uiCompressedSize)
                            != MZ_OK || uiResult != uiChunkSize) {
                            VTFError_Set(error, "Error decompressing deflate compressed image data.");
                            return false;
                        }
                    }
                    break;
                    default:
                        VTFError_Set_Formatted(error, "Unsupported auxiliary compression method %d.", sMethod);
                        return false;
                }

                uiSourceOffset += uiCompressedSize;
                uiDestOffset += uiChunkSize;
            }
        }
    }

    return true;
}

vlBool CVTFFile::IsPowerOfTwo(const uint32_t uiSize) {
    return uiSize > 0 && (uiSize & (uiSize - 1)) == 0;
}

uint32_t CVTFFile::NextPowerOfTwo(uint32_t uiSize) {
    if (uiSize == 0) {
        return 1;
    }

    if (IsPowerOfTwo(uiSize)) {
        return uiSize;
    }

    uiSize--;
    for (uint32_t i = 1; i <= sizeof(uint32_t) * 4; i <<= 1) {
        uiSize = uiSize | (uiSize >> i);
    }
    uiSize++;

    return uiSize;
}

uint32_t CVTFFile::ComputeResizedDimension(const uint32_t uiSize, const VTFResizeMethod ResizeMethod) {
    uint32_t uiBiggest, uiSmallest;

    switch (ResizeMethod) {
        case RESIZE_NEAREST_POWER2:
        case RESIZE_BIGGEST_POWER2:
        case RESIZE_SMALLEST_POWER2:
            if (IsPowerOfTwo(uiSize)) {
                return uiSize;
            }

            uiBiggest = NextPowerOfTwo(uiSize);
            uiSmallest = uiBiggest >> 1;
            break;
        case RESIZE_NEAREST_MULTIPLE4:
        case RESIZE_BIGGEST_MULTIPLE4:
        case RESIZE_SMALLEST_MULTIPLE4:
            if (uiSize % 4 == 0) {
                return uiSize;
            }

            uiBiggest = (uiSize + 3) & ~3u;
            uiSmallest = uiBiggest - 4;
            break;
        default:
            return uiSize;
    }

    if (uiSmallest == 0) {
        return uiBiggest;
    }

    switch (ResizeMethod) {
        case RESIZE_SMALLEST_POWER2:
        case RESIZE_SMALLEST_MULTIPLE4:
            return uiSmallest;
        case RESIZE_NEAREST_POWER2:
        case RESIZE_NEAREST_MULTIPLE4:
            return uiSize - uiSmallest < uiBiggest - uiSize ? uiSmallest : uiBiggest;
        default:
            return uiBiggest;
    }
}

//
// IsLoaded()
// Returns true if a image has been created or loaded.  Use GetHasImage()
// and GetHasThumbnail() to determine if any image data is associated with
// the image or if it was a header only load operation.
//
vlBool CVTFFile::IsLoaded() const {
    return mHeader != nullptr;
}

vlBool CVTFFile::Load(const char *cFileName, Diagnostics::CError &error, const vlBool bHeaderOnly) {
    IO::Readers::CFileReader reader(cFileName);
    return Load(&reader, bHeaderOnly, error);
}

vlBool CVTFFile::Load(const void *lpData, const uint32_t uiBufferSize, Diagnostics::CError &error,
                      const vlBool bHeaderOnly) {
    IO::Readers::CMemoryReader reader(lpData, uiBufferSize);
    return Load(&reader, bHeaderOnly, error);
}

vlBool CVTFFile::Load(void *pUserData, Diagnostics::CError &error, const vlBool bHeaderOnly) {
    IO::Readers::CProcReader reader(pUserData);
    return Load(&reader, bHeaderOnly, error);
}

vlBool CVTFFile::Save(const char *cFileName, Diagnostics::CError &error) const {
    IO::Writers::CFileWriter writer(cFileName);
    return Save(&writer, error);
}

vlBool CVTFFile::Save(void *buffer, const uint32_t bufferSize, uint32_t &outSize, Diagnostics::CError &error) const {
    outSize = 0;

    IO::Writers::CMemoryWriter MemoryWriter = IO::Writers::CMemoryWriter(buffer, bufferSize);

    vlBool bResult = Save(&MemoryWriter, error);

    outSize = MemoryWriter.GetStreamSize(error);

    return bResult;
}

vlBool CVTFFile::Save(void *pUserData, Diagnostics::CError &error) const {
    IO::Writers::CProcWriter writer(pUserData);
    return Save(&writer, error);
}

// -----------------------------------------------------------------------------------
// vlBool Load(IO::Readers::IReader *Reader, vlBool bHeaderOnly)
//
// Loads a VTF file from a stream into memory.
// Reader - The stream to read from.
// bHeaderOnly - only read in the header if true (dont allocate and read image data in)
// ------------------------------------------------------------------------------------
vlBool CVTFFile::Load(IO::Readers::IReader *Reader, const vlBool bHeaderOnly, Diagnostics::CError &error) {
    Destroy();

    try {
        if (!Reader->Open(error))
            throw 0;

        // Get the size of the .vtf file.
        uint32_t uiFileSize = Reader->GetStreamSize(error);

        // Check we at least have enough bytes for a header.
        if (uiFileSize < sizeof(SVTFFileHeader)) {
            VTFError_Set(error, "File is corrupt; file to small for it's header.");
            throw 0;
        }

        SVTFFileHeader FileHeader;

        // read the file header
        memset(&FileHeader, 0, sizeof(SVTFFileHeader));
        if (Reader->Read(&FileHeader, sizeof(SVTFFileHeader), error) != sizeof(SVTFFileHeader)) {
            throw 0;
        }

        if (memcmp(FileHeader.typeString, "VTF\0", 4) != 0) {
            VTFError_Set(error, "File signature does not match 'VTF'.");
            throw 0;
        }

        if (FileHeader.version[0] != VTF_MAJOR_VERSION || (
                FileHeader.version[1] < 0 || FileHeader.version[1] > VTF_MINOR_VERSION)) {
            VTFError_Set_Formatted(error, "File version %u.%u does not match %d.%d to %d.%d.", FileHeader.version[0],
                               FileHeader.version[1], VTF_MAJOR_VERSION, 0, VTF_MAJOR_VERSION, VTF_MINOR_VERSION);
            throw 0;
        }

        if (FileHeader.headerSize > sizeof(SVTFHeader)) {
            VTFError_Set_Formatted(error, "File header size %d B is larger than the %d B maximum expected.",
                               FileHeader.headerSize, sizeof(SVTFHeader));
            throw 0;
        }

        Reader->Seek(0, FILE_BEGIN, error);

        mHeader = new SVTFHeader;
        memset(mHeader, 0, sizeof(SVTFHeader));

        // read the header
        if (Reader->Read(mHeader, FileHeader.headerSize, error) != FileHeader.headerSize) {
            throw 0;
        }

        if (mHeader->version[0] < VTF_MAJOR_VERSION || (
                mHeader->version[0] == VTF_MAJOR_VERSION && mHeader->version[1] <
                VTF_MINOR_VERSION_MIN_VOLUME)) {
            // set depth if version is lower than 7.2
            mHeader->depth = 1;
        }

        if (!GetSupportsResources()) {
            // set resource count if version is lower than 7.3
            mHeader->resourceCount = 0;
        }

        // if we just want the header loaded, bail here
        if (bHeaderOnly) {
            Reader->Close();
            return true;
        }

        // work out how big out buffers need to be
        mImageBufferSize = ComputeImageSize(mHeader->width, mHeader->height,
                                            mHeader->depth,
                                            mHeader->mipCount,
                                            mHeader->imageFormat) * GetFaceCount() *
                           GetFrameCount();

        if (mHeader->lowResImageFormat != IMAGE_FORMAT_NONE) {
            mThumbnailBufferSize = ComputeImageSize(mHeader->lowResImageWidth,
                                                    mHeader->lowResImageHeight, 1,
                                                    mHeader->lowResImageFormat);
        } else {
            mThumbnailBufferSize = 0;
        }

        // read the resource directory if version > 7.3
        uint32_t uiThumbnailBufferOffset = 0, uiImageDataOffset = 0;
        if (mHeader->resourceCount) {
            if (mHeader->resourceCount > VTF_RSRC_MAX_DICTIONARY_ENTRIES) {
                VTFError_Set_Formatted(error,
                    "File may be corrupt; directory length %u exceeds maximum dictionary length of %u.",
                    mHeader->resourceCount, VTF_RSRC_MAX_DICTIONARY_ENTRIES);
                throw 0;
            }

            for (uint32_t i = 0; i < mHeader->resourceCount; i++) {
                switch (mHeader->resources[i].type) {
                    case VTF_LEGACY_RSRC_LOW_RES_IMAGE:
                        if (mHeader->lowResImageFormat == IMAGE_FORMAT_NONE) {
                            VTFError_Set(error, "File may be corrupt; unexpected low resolution image directory entry.");
                            throw 0;
                        }
                        if (uiThumbnailBufferOffset != 0) {
                            VTFError_Set(error, "File may be corrupt; multiple low resolution image directory entries.");
                            throw 0;
                        }
                        uiThumbnailBufferOffset = mHeader->resources[i].data;
                        break;
                    case VTF_LEGACY_RSRC_IMAGE:
                        if (uiImageDataOffset != 0) {
                            VTFError_Set(error, "File may be corrupt; multiple image directory entries.");
                            throw 0;
                        }
                        uiImageDataOffset = mHeader->resources[i].data;
                        break;
                    default:
                        if ((mHeader->resources[i].flags & RSRCF_HAS_NO_DATA_CHUNK) == 0) {
                            if (mHeader->resources[i].data + sizeof(uint32_t) > uiFileSize) {
                                VTFError_Set(error, "File may be corrupt; file to small for it's resource data.");
                                throw 0;
                            }

                            uint32_t uiSize = 0;
                            Reader->Seek(mHeader->resources[i].data, FILE_BEGIN, error);
                            if (Reader->Read(&uiSize, sizeof(uint32_t), error) != sizeof(uint32_t)) {
                                throw 0;
                            }

                            if (mHeader->resources[i].data + sizeof(uint32_t) + uiSize > uiFileSize) {
                                VTFError_Set(error, "File may be corrupt; file to small for it's resource data.");
                                throw 0;
                            }

                            mHeader->data[i].size = uiSize;
                            mHeader->data[i].data = new uint8_t[uiSize];
                            if (Reader->Read(mHeader->data[i].data, uiSize, error) != uiSize) {
                                throw 0;
                            }
                        }
                        break;
                }
            }
        } else {
            uiThumbnailBufferOffset = mHeader->headerSize;
            uiImageDataOffset = uiThumbnailBufferOffset + mThumbnailBufferSize;
        }

        uint32_t uiAuxIndex = VTF_RSRC_MAX_DICTIONARY_ENTRIES;
        uint32_t uiAuxImageBufferSize = 0;

        if (GetSupportsAuxCompression()) {
            for (uint32_t i = 0; i < mHeader->resourceCount; i++) {
                if (mHeader->resources[i].type == VTF_RSRC_AUX_COMPRESSION_INFO) {
                    uiAuxIndex = i;
                    break;
                }
            }
        }

        if (uiAuxIndex != VTF_RSRC_MAX_DICTIONARY_ENTRIES) {
            if (mHeader->data[uiAuxIndex].size < sizeof(SVTFAuxCompressionInfoHeader)) {
                VTFError_Set(error, "File may be corrupt; auxiliary compression resource is too small.");
                throw 0;
            }

            const SVTFAuxCompressionInfoHeader *Info = (const SVTFAuxCompressionInfoHeader *) mHeader->data[
                uiAuxIndex].data;

            mAuxCompressionLevel = Info->level;
            mAuxCompressionMethod = Info->method <= 0 ? AUX_COMPRESSION_METHOD_DEFLATE : Info->method;

            if (mAuxCompressionLevel == VTF_AUX_COMPRESSION_LEVEL_NONE) {
                uiAuxIndex = VTF_RSRC_MAX_DICTIONARY_ENTRIES;
            } else if (mAuxCompressionMethod != AUX_COMPRESSION_METHOD_DEFLATE && mAuxCompressionMethod !=
                       AUX_COMPRESSION_METHOD_ZSTD) {
                VTFError_Set_Formatted(error, "File may be corrupt; unsupported auxiliary compression method %d.",
                                   mAuxCompressionMethod);
                throw 0;
            } else {
                uint32_t uiChunkCount = GetMipmapCount() * GetFrameCount() * GetFaceCount();

                if (mHeader->data[uiAuxIndex].size < sizeof(SVTFAuxCompressionInfoHeader) + uiChunkCount * sizeof(
                        uint32_t)) {
                    VTFError_Set(error, "File may be corrupt; auxiliary compression resource is too small.");
                    throw 0;
                }

                const uint32_t *lpSizes = (const uint32_t *) (
                    mHeader->data[uiAuxIndex].data + sizeof(SVTFAuxCompressionInfoHeader));
                for (uint32_t i = 0; i < uiChunkCount; i++) {
                    uiAuxImageBufferSize += lpSizes[i];
                }
            }
        }

        // sanity check
        // headersize + lowbuffersize + buffersize *should* equal the filesize
        uint32_t uiImageDataSize = uiAuxIndex != VTF_RSRC_MAX_DICTIONARY_ENTRIES
                                       ? uiAuxImageBufferSize
                                       : mImageBufferSize;
        if (mHeader->headerSize > uiFileSize || uiThumbnailBufferOffset + mThumbnailBufferSize > uiFileSize
            || uiImageDataOffset + uiImageDataSize > uiFileSize) {
            VTFError_Set(error, "File may be corrupt; file to small for it's image data.");
            throw 0;
        }

        if (uiThumbnailBufferOffset == 0) {
            mHeader->lowResImageFormat = IMAGE_FORMAT_NONE;
        }

        // assuming all is well, size our data buffers
        if (mHeader->lowResImageFormat != IMAGE_FORMAT_NONE) {
            mThumbnailImageData = new uint8_t[mThumbnailBufferSize];

            // load the low res data
            Reader->Seek(uiThumbnailBufferOffset, FILE_BEGIN, error);
            if (Reader->Read(mThumbnailImageData, mThumbnailBufferSize, error) !=
                mThumbnailBufferSize) {
                throw 0;
            }
        }

        if (uiImageDataOffset == 0) {
            mHeader->imageFormat = IMAGE_FORMAT_NONE;
        }

        if (mHeader->imageFormat != IMAGE_FORMAT_NONE) {
            mImageData = new uint8_t[mImageBufferSize];

            // load the high-res data
            Reader->Seek(uiImageDataOffset, FILE_BEGIN, error);

            if (uiAuxIndex != VTF_RSRC_MAX_DICTIONARY_ENTRIES) {
                // image data is kept uncompressed in memory
                uint8_t *lpCompressedData = new uint8_t[uiAuxImageBufferSize];

                if (Reader->Read(lpCompressedData, uiAuxImageBufferSize, error) != uiAuxImageBufferSize) {
                    delete[] lpCompressedData;
                    throw 0;
                }

                vlBool bResult = DecompressAuxData(this, lpCompressedData, uiAuxImageBufferSize, mImageData,
                                                   mImageBufferSize, mHeader->data[uiAuxIndex].data,
                                                   mHeader->data[uiAuxIndex].size, mAuxCompressionMethod,
                                                   error);

                delete[] lpCompressedData;

                if (!bResult) {
                    throw 0;
                }
            } else if (Reader->Read(mImageData, mImageBufferSize, error) != mImageBufferSize) {
                throw 0;
            }
        }

        // rebuilt on save
        if (GetHasResource(VTF_RSRC_AUX_COMPRESSION_INFO)) {
            SetResourceData(VTF_RSRC_AUX_COMPRESSION_INFO, 0, nullptr, error);
        }

        // Fixup resource offsets for writing.
        ComputeResources();
    } catch (...) {
        Reader->Close();

        Destroy();

        return false;
    }

    Reader->Close();

    return true;
}

//
// Save()
// Saves the curret image.  Basic format checking is done.
//
vlBool CVTFFile::Save(IO::Writers::IWriter *Writer, Diagnostics::CError &error) const {
    if (!IsLoaded() || !GetHasImage()) {
        VTFError_Set(error, "No image to save.");
        return false;
    }

    // ToDo: Check if the image buffer is ok.
    //       Check flags and other header values.

    // recompress the image data
    vlBool bAuxCompressed = const_cast<CVTFFile *>(this)->ComputeAuxCompression(true, error);

    try {
        if (!Writer->Open(error))
            throw 0;

        if (GetSupportsResources()) {
            // the dictionary and the offsets it holds change when the image data is compressed
            SVTFHeader SaveHeader = *mHeader;
            uint32_t uiImageBufferSize = mImageBufferSize;

            if (bAuxCompressed) {
                if (SaveHeader.resourceCount == VTF_RSRC_MAX_DICTIONARY_ENTRIES) {
                    VTFError_Set_Formatted(error,
                        "Maximum directory entry count %u reached; cannot add the auxiliary compression resource.",
                        VTF_RSRC_MAX_DICTIONARY_ENTRIES);
                    throw 0;
                }

                SaveHeader.resources[SaveHeader.resourceCount].type = VTF_RSRC_AUX_COMPRESSION_INFO;
                SaveHeader.resources[SaveHeader.resourceCount].data = 0;
                SaveHeader.data[SaveHeader.resourceCount].size = mAuxCompressionInfoSize;
                SaveHeader.data[SaveHeader.resourceCount].data = mAuxCompressionInfo;
                SaveHeader.resourceCount++;

                SaveHeader.headerSize = sizeof(SVTFHeader_76_A) + SaveHeader.resourceCount * sizeof(SVTFResource);
                uiImageBufferSize = mAuxCompressedBufferSize;
            }

            // fix up the resource offsets for the sizes we are about to write
            uint32_t uiOffset = SaveHeader.headerSize;
            for (uint32_t i = 0; i < SaveHeader.resourceCount; i++) {
                switch (SaveHeader.resources[i].type) {
                    case VTF_LEGACY_RSRC_LOW_RES_IMAGE:
                        SaveHeader.resources[i].data = uiOffset;
                        uiOffset += mThumbnailBufferSize;
                        break;
                    case VTF_LEGACY_RSRC_IMAGE:
                        SaveHeader.resources[i].data = uiOffset;
                        uiOffset += uiImageBufferSize;
                        break;
                    default:
                        if ((SaveHeader.resources[i].flags & RSRCF_HAS_NO_DATA_CHUNK) == 0) {
                            SaveHeader.resources[i].data = uiOffset;
                            uiOffset += sizeof(uint32_t) + SaveHeader.data[i].size;
                        }
                        break;
                }
            }

            // Write the header
            if (Writer->Write(&SaveHeader, SaveHeader.headerSize, error) != SaveHeader.headerSize) {
                throw 0;
            }

            for (uint32_t i = 0; i < SaveHeader.resourceCount; i++) {
                switch (SaveHeader.resources[i].type) {
                    case VTF_LEGACY_RSRC_LOW_RES_IMAGE:
                        if (Writer->Write(mThumbnailImageData, mThumbnailBufferSize, error) !=
                            mThumbnailBufferSize) {
                            throw 0;
                        }
                        break;
                    case VTF_LEGACY_RSRC_IMAGE: {
                        uint8_t *lpData = bAuxCompressed ? mAuxCompressedData : mImageData;
                        if (Writer->Write(lpData, uiImageBufferSize, error) != uiImageBufferSize) {
                            throw 0;
                        }
                    }
                    break;
                    default:
                        if ((SaveHeader.resources[i].flags & RSRCF_HAS_NO_DATA_CHUNK) == 0) {
                            if (Writer->Write(&SaveHeader.data[i].size, sizeof(uint32_t), error) != sizeof(uint32_t)) {
                                throw 0;
                            }

                            if (Writer->Write(SaveHeader.data[i].data, SaveHeader.data[i].size, error) != SaveHeader.
                                data
                                [i].size) {
                                throw 0;
                            }
                        }
                }
            }
        } else {
            // Write the header.
            if (Writer->Write(mHeader, mHeader->headerSize, error) != mHeader->headerSize) {
                throw 0;
            }

            if (mHeader->lowResImageFormat != IMAGE_FORMAT_NONE) {
                // write the thumbnail image data
                if (Writer->Write(mThumbnailImageData, mThumbnailBufferSize, error) !=
                    mThumbnailBufferSize) {
                    throw 0;
                }
            }

            if (mHeader->imageFormat != IMAGE_FORMAT_NONE) {
                // write the image data
                if (Writer->Write(mImageData, mImageBufferSize, error) != mImageBufferSize) {
                    throw 0;
                }
            }
        }
    } catch (...) {
        Writer->Close();

        return false;
    }

    Writer->Close();

    return true;
}

//
// GetHasImage()
// A image can be loaded as header only, this function indicates weather
// image data was loaded or not.
//
vlBool CVTFFile::GetHasImage() const {
    if (!IsLoaded())
        return false;

    return mImageData != nullptr;
}

//
// GetMajorVersion()
// Returns the size of the VTF file major version number.
//
uint32_t CVTFFile::GetMajorVersion() const {
    if (!IsLoaded())
        return 0;

    return mHeader->version[0];
}

//
// GetMinorVersion()
// Returns the size of the VTF file minor version number.
//
uint32_t CVTFFile::GetMinorVersion() const {
    if (!IsLoaded())
        return 0;

    return mHeader->version[1];
}

//
// SetVersion()
// Retargets the loaded image at another version of the VTF format.
//
vlBool CVTFFile::SetVersion(const uint32_t uiMajor, const uint32_t uiMinor, Diagnostics::CError &error) {
    if (!IsLoaded()) {
        VTFError_Set(error, "No image loaded.");
        return false;
    }

    if (uiMajor != VTF_MAJOR_VERSION || uiMinor > VTF_MINOR_VERSION) {
        VTFError_Set_Formatted(error, "File version %u.%u does not match %d.%d to %d.%d.", uiMajor, uiMinor, VTF_MAJOR_VERSION,
                           0, VTF_MAJOR_VERSION, VTF_MINOR_VERSION);
        return false;
    }

    if (mHeader->version[0] == uiMajor && mHeader->version[1] == uiMinor) {
        return true;
    }

    if (uiMinor < VTF_MINOR_VERSION_MIN_VOLUME && GetDepth() > 1) {
        VTFError_Set_Formatted(error, "Volume textures are only supported in version %d.%d and up.", VTF_MAJOR_VERSION,
                           VTF_MINOR_VERSION_MIN_VOLUME);
        return false;
    }

    // ToDo: throw away the sphere map
    vlBool bHasSphereMap = (mHeader->flags & TEXTUREFLAGS_ENVMAP) != 0
                           && mHeader->version[1] < VTF_MINOR_VERSION_MIN_NO_SPHERE_MAP
                           && mHeader->startFrame != 0xffff;

    if (bHasSphereMap && uiMinor >= VTF_MINOR_VERSION_MIN_NO_SPHERE_MAP) {
        VTFError_Set_Formatted(error, "Cubemaps with 7th spheremap face are not supported in version %d.%d and up",
                           VTF_MAJOR_VERSION, VTF_MINOR_VERSION_MIN_NO_SPHERE_MAP);
        return false;
    }

    mHeader->version[0] = uiMajor;
    mHeader->version[1] = uiMinor;

    if (mHeader->flags & TEXTUREFLAGS_ENVMAP) {
        if (uiMinor < VTF_MINOR_VERSION_MIN_NO_SPHERE_MAP) {
            if (!bHasSphereMap) {
                mHeader->startFrame = 0xffff;
            }
        } else if (mHeader->startFrame == 0xffff) {
            mHeader->startFrame = 0;
        }
    }

    if (!GetSupportsAuxCompression()) {
        mAuxCompressionLevel = VTF_AUX_COMPRESSION_LEVEL_NONE;
        DestroyAuxCompression();
    }

    if (!GetSupportsResources()) {
        for (uint32_t i = 0; i < VTF_RSRC_MAX_DICTIONARY_ENTRIES; i++) {
            delete[] mHeader->data[i].data;
            mHeader->data[i].data = nullptr;
            mHeader->data[i].size = 0;
        }

        memset(mHeader->resources, 0, sizeof(mHeader->resources));
        mHeader->resourceCount = 0;
    } else if (mHeader->resourceCount == 0) {
        if (mHeader->lowResImageFormat != IMAGE_FORMAT_NONE) {
            mHeader->resources[mHeader->resourceCount++].type = VTF_LEGACY_RSRC_LOW_RES_IMAGE;
        }

        mHeader->resources[mHeader->resourceCount++].type = VTF_LEGACY_RSRC_IMAGE;
    }

    ComputeResources();

    return true;
}

//
// ComputeResources()
// Computes header VTF directory resources.
//
void CVTFFile::ComputeResources() {
    if (!IsLoaded())
        return;

    // Correct resource count.
    if (!GetSupportsResources()) {
        mHeader->resourceCount = 0;
    }

    // Correct header size.
    STATIC_ASSERT(VTF_MAJOR_VERSION == 7, "HeaderSize needs calculation for new major version.");
    STATIC_ASSERT(VTF_MINOR_VERSION == 6, "HeaderSize needs calculation for new minor version.");
    switch (mHeader->version[0]) {
        case 7:
            switch (mHeader->version[1]) {
                case 0:
                    mHeader->headerSize = sizeof(SVTFHeader_70_A);
                    break;
                case 1:
                    mHeader->headerSize = sizeof(SVTFHeader_71_A);
                    break;
                case 2:
                    mHeader->headerSize = sizeof(SVTFHeader_72_A);
                    break;
                case 3:
                    mHeader->headerSize = sizeof(SVTFHeader_73_A) + mHeader->resourceCount * sizeof(
                                              SVTFResource);
                    break;
                case 4:
                    mHeader->headerSize = sizeof(SVTFHeader_74_A) + mHeader->resourceCount * sizeof(
                                              SVTFResource);
                    break;
                case 5:
                    mHeader->headerSize = sizeof(SVTFHeader_75_A) + mHeader->resourceCount * sizeof(
                                              SVTFResource);
                    break;
                case 6:
                    mHeader->headerSize = sizeof(SVTFHeader_76_A) + mHeader->resourceCount * sizeof(
                                              SVTFResource);
                    break;
            }
            break;
    }

    // Correct resource offsets.
    uint32_t uiOffset = mHeader->headerSize;
    for (uint32_t i = 0; i < mHeader->resourceCount; i++) {
        switch (mHeader->resources[i].type) {
            case VTF_LEGACY_RSRC_LOW_RES_IMAGE:
                mHeader->resources[i].data = uiOffset;
                uiOffset += mThumbnailBufferSize;
                break;
            case VTF_LEGACY_RSRC_IMAGE:
                mHeader->resources[i].data = uiOffset;
                uiOffset += mImageBufferSize;
                break;
            default:
                if ((mHeader->resources[i].flags & RSRCF_HAS_NO_DATA_CHUNK) == 0) {
                    mHeader->resources[i].data = uiOffset;
                    uiOffset += sizeof(uint32_t) + mHeader->data[i].size;
                }
                break;
        }
    }
}

//
// GetSize()
// Returns the size of the VTF file in bytes.
//
uint32_t CVTFFile::GetSize(Diagnostics::CError &error) const {
    if (!IsLoaded())
        return 0;

    uint32_t uiResourceSize = 0;
    if (GetSupportsResources()) {
        for (uint32_t i = 0; i < mHeader->resourceCount; i++) {
            switch (mHeader->resources[i].type) {
                case VTF_LEGACY_RSRC_LOW_RES_IMAGE:
                case VTF_LEGACY_RSRC_IMAGE:
                    break;
                default:
                    if ((mHeader->resources[i].flags & RSRCF_HAS_NO_DATA_CHUNK) == 0) {
                        uiResourceSize += sizeof(uint32_t) + mHeader->data[i].size;
                    }
                    break;
            }
        }
    }

    uint32_t uiImageSize = mImageBufferSize;
    uint32_t uiHeaderSize = mHeader->headerSize;
    if (const_cast<CVTFFile *>(this)->ComputeAuxCompression(false, error)) {
        uiImageSize = mAuxCompressedBufferSize;
        uiHeaderSize += sizeof(SVTFResource);
        uiResourceSize += sizeof(uint32_t) + mAuxCompressionInfoSize;
    }

    return uiHeaderSize + mThumbnailBufferSize + uiImageSize + uiResourceSize;
}

//
// GetWidth()
// Gets the width of the largest level mipmap.
//
uint32_t CVTFFile::GetWidth() const {
    if (!IsLoaded())
        return 0;

    return mHeader->width;
}

//
// GetHeight()
// Gets the height of the largest level mipmap.
//
uint32_t CVTFFile::GetHeight() const {
    if (!IsLoaded())
        return 0;

    return mHeader->height;
}

//
// GetDepth()
// Gets the depth of the largest level mipmap.
//
uint32_t CVTFFile::GetDepth() const {
    if (!IsLoaded())
        return 0;

    return mHeader->depth;
}

//
// GetFrameCount()
// Gets the number of frames the image has.  All images have at least 1 frame.
//
uint32_t CVTFFile::GetFrameCount() const {
    if (!IsLoaded())
        return 0;

    return mHeader->frames;
}

//---------------------------------------------------------------------------------
// GetFaceCount()
//
// Returns the number of faces in the texture based on the status of the header
// flags. Cubemaps have 6 or 7 faces, others just 1.
//---------------------------------------------------------------------------------
uint32_t CVTFFile::GetFaceCount() const {
    if (!IsLoaded())
        return 0;

    return mHeader->flags & TEXTUREFLAGS_ENVMAP
               ? (mHeader->startFrame != 0xffff && mHeader->version[1] < VTF_MINOR_VERSION_MIN_NO_SPHERE_MAP
                      ? CUBEMAP_FACE_COUNT
                      : CUBEMAP_FACE_COUNT - 1)
               : 1;
}

//
// GetMipmapCount()
// Gets the number of mipmaps the image has.
//
uint32_t CVTFFile::GetMipmapCount() const {
    if (!IsLoaded())
        return 0;

    return mHeader->mipCount;
}

//
// GetStartFrame()
// Gets the first frame in the animation sequence.  If the image is
// an enviroment map and 0xffff is returned, the enviroment map has
// no sphere map.
//
uint32_t CVTFFile::GetStartFrame() const {
    if (!IsLoaded())
        return 0;

    return mHeader->startFrame;
}

//
// SetStartFrame()
// Sets the first frame in the animation sequence.
//
void CVTFFile::SetStartFrame(uint32_t uiStartFrame) {
    if (!IsLoaded())
        return;

    // Note: Valve informs us that animated enviroment maps ARE possible.
    // The StartFrame MAY be valid but there is the issue of the enviroment
    // maps without sphere maps.  This is trivial...

    // Don't let the user set the start frame of an enviroment map.
    if (mHeader->flags & TEXTUREFLAGS_ENVMAP) {
        return;
    }

    if (uiStartFrame >= (uint32_t) mHeader->frames) {
        uiStartFrame = (uint32_t) mHeader->frames - 1;
    }

    mHeader->startFrame = (uint16_t) uiStartFrame;
}

//
// GetFlags()
// Gets the flags associated with the image.  These flags
// are stored in the VTFImageFlag enumeration.
//
uint32_t CVTFFile::GetFlags() const {
    if (!IsLoaded())
        return 0;

    return mHeader->flags;
}

// SetFlags()
// Sets the flags associated with the image.  These flags
// are stored in the VTFImageFlag enumeration.
//
void CVTFFile::SetFlags(uint32_t uiFlags) {
    if (!IsLoaded())
        return;

    // Don't let the user set flags critical to the image's format.
    //if(Header->Version[0] < VTF_MAJOR_VERSION || (Header->Version[0] == VTF_MAJOR_VERSION && Header->Version[1] <= VTF_MINOR_VERSION_MIN_RESOURCE))
    //{
    //	if(Header->Flags & TEXTUREFLAGS_DEPRECATED_NOCOMPRESS)
    //		uiFlags |= TEXTUREFLAGS_DEPRECATED_NOCOMPRESS;
    //	else
    //		uiFlags &= ~TEXTUREFLAGS_DEPRECATED_NOCOMPRESS;
    //}

    if (mHeader->flags & TEXTUREFLAGS_EIGHTBITALPHA)
        uiFlags |= TEXTUREFLAGS_EIGHTBITALPHA;
    else
        uiFlags &= ~TEXTUREFLAGS_EIGHTBITALPHA;

    if (mHeader->flags & TEXTUREFLAGS_ENVMAP)
        uiFlags |= TEXTUREFLAGS_ENVMAP;
    else
        uiFlags &= ~TEXTUREFLAGS_ENVMAP;

    if (mHeader->flags & TEXTUREFLAGS_ENVMAP)
        uiFlags |= TEXTUREFLAGS_ENVMAP;
    else
        uiFlags &= ~TEXTUREFLAGS_ENVMAP;

    mHeader->flags = uiFlags;
}

//
// GetFlag()
// Gets the status of the specified flag in the image.
//
vlBool CVTFFile::GetFlag(const VTFImageFlag ImageFlag) const {
    if (!IsLoaded())
        return false;

    return (mHeader->flags & ImageFlag) != 0;
}

//
// SetFlag()
// Sets the flag ImageFlag to bState (set or not set).  Flags critical
// to the image's format cannot be set.
//
void CVTFFile::SetFlag(const VTFImageFlag ImageFlag, const vlBool bState) {
    if (!IsLoaded())
        return;

    //if(Header->Version[0] < VTF_MAJOR_VERSION || (Header->Version[0] == VTF_MAJOR_VERSION && Header->Version[1] <= VTF_MINOR_VERSION_MIN_RESOURCE))
    //{
    //	if(ImageFlag == TEXTUREFLAGS_DEPRECATED_NOCOMPRESS)
    //	{
    //		return;
    //	}
    //}

    // Don't let the user set flags critical to the image's format.
    if (ImageFlag == TEXTUREFLAGS_ONEBITALPHA || ImageFlag == TEXTUREFLAGS_EIGHTBITALPHA || ImageFlag ==
        TEXTUREFLAGS_ENVMAP) {
        return;
    }

    if (bState) {
        mHeader->flags |= ImageFlag;
    } else {
        mHeader->flags &= ~ImageFlag;
    }
}

//
// GetBumpmapScale()
// Gets the bumpmap scale of the image.
//
float CVTFFile::GetBumpmapScale() const {
    if (!IsLoaded())
        return 0.0f;

    return mHeader->bumpScale;
}

//
// SetBumpmapScale()
// Sets the bumpmap scale of the image.
//
void CVTFFile::SetBumpmapScale(const float sBumpmapScale) {
    if (!IsLoaded())
        return;

    mHeader->bumpScale = sBumpmapScale;
}

//
// GetReflectivity()
// Gets the reflectivity of the image.
//
void CVTFFile::GetReflectivity(float &sX, float &sY, float &sZ) const {
    if (!IsLoaded())
        return;

    sX = mHeader->reflectivity[0];
    sY = mHeader->reflectivity[1];
    sZ = mHeader->reflectivity[2];
}

//
// SetReflectivity()
// Sets the reflectivity of the image.
//
void CVTFFile::SetReflectivity(const float sX, const float sY, const float sZ) {
    if (!IsLoaded())
        return;

    mHeader->reflectivity[0] = sX;
    mHeader->reflectivity[1] = sY;
    mHeader->reflectivity[2] = sZ;
}

//
// GetFormat()
// Gets the format of the image.
//
VTFImageFormat CVTFFile::GetFormat() const {
    if (!IsLoaded())
        return IMAGE_FORMAT_NONE;

    return mHeader->imageFormat;
}

//
// GetDecodeFormat()
// Gets the format the image data should be decoded as.
//
VTFImageFormat CVTFFile::GetDecodeFormat() const {
    VTFImageFormat Format = GetFormat();

    if (Format == IMAGE_FORMAT_DXT1 && (mHeader->flags & TEXTUREFLAGS_ONEBITALPHA))
        return IMAGE_FORMAT_DXT1_ONEBITALPHA;

    return Format;
}

//
// GetData()
// Gets the image data of the specified frame, face and mipmap in the format
// of the image.
//
uint8_t *CVTFFile::GetData(const uint32_t uiFrame, const uint32_t uiFace, const uint32_t uiSlice,
                           const uint32_t uiMipmapLevel) const {
    if (!IsLoaded())
        return nullptr;

    return mImageData + ComputeDataOffset(uiFrame, uiFace, uiSlice, uiMipmapLevel,
                                          mHeader->imageFormat);
}

//
// SetData()
// Sets the image data of the specified frame, face and mipmap.  Image data
// must be in the format of the image.
//
void CVTFFile::SetData(const uint32_t uiFrame, const uint32_t uiFace, const uint32_t uiSlice,
                       const uint32_t uiMipmapLevel, uint8_t *lpData) {
    if (!IsLoaded() || mImageData == nullptr)
        return;

    memcpy(
        mImageData + ComputeDataOffset(uiFrame, uiFace, uiSlice, uiMipmapLevel, mHeader->imageFormat),
        lpData, ComputeMipmapSize(mHeader->width, mHeader->height, 1, uiMipmapLevel,
                                  mHeader->imageFormat));

    DestroyAuxCompression();
}

//
// GetHasThumbnail()
// A image does not need a thumbnail, this function returns wheather
// the image has a thumbnail or not.
//
vlBool CVTFFile::GetHasThumbnail() const {
    if (!IsLoaded())
        return false;

    return mHeader->lowResImageFormat != IMAGE_FORMAT_NONE;
}

//
// GetThumbnailWidth()
// Gets the width of the thumbnail image.
//
uint32_t CVTFFile::GetThumbnailWidth() const {
    if (!IsLoaded())
        return 0;

    return mHeader->lowResImageWidth;
}

//
// GetThumbnailHeight()
// Sets the height of the thumbnail image.
//
uint32_t CVTFFile::GetThumbnailHeight() const {
    if (!IsLoaded())
        return 0;

    return mHeader->lowResImageHeight;
}

//
// GetThumbnailFormat()
// Gets the format of the thumbnail image.
//
VTFImageFormat CVTFFile::GetThumbnailFormat() const {
    if (!IsLoaded())
        return IMAGE_FORMAT_NONE;

    return mHeader->lowResImageFormat;
}

//
// GetThumbnailData()
// Gets the thumbnail image data in the format of the image.  This "thumbnail"
// image is a small same of the original image used by the engine for color sampling
// when you hit the wall with a crowbar etc.
//
uint8_t *CVTFFile::GetThumbnailData() const {
    if (!IsLoaded())
        return nullptr;

    return mThumbnailImageData;
}

//
// SetThumbnailData()
// Sets the thumbnail image data.  Image data must be in the format of the image.
//
void CVTFFile::SetThumbnailData(uint8_t *lpData) {
    if (!IsLoaded() || mThumbnailImageData == nullptr)
        return;

    memcpy(mThumbnailImageData, lpData,
           mThumbnailBufferSize
           /*CVTFFile::ComputeImageSize(Header->LowResImageWidth, Header->LowResImageHeight, Header->LowResImageFormat)*/);
}

vlBool CVTFFile::GetSupportsResources() const {
    if (!IsLoaded())
        return false;

    return mHeader->version[0] > VTF_MAJOR_VERSION
           || (mHeader->version[0] == VTF_MAJOR_VERSION
               && mHeader->version[1] >= VTF_MINOR_VERSION_MIN_RESOURCE);
}

uint32_t CVTFFile::GetResourceCount() const {
    if (!GetSupportsResources())
        return 0;

    return mHeader->resourceCount;
}

uint32_t CVTFFile::GetResourceType(const uint32_t uiIndex) const {
    if (!GetSupportsResources())
        return 0;

    if (uiIndex >= mHeader->resourceCount)
        return 0;

    return mHeader->resources[uiIndex].type;
}

vlBool CVTFFile::GetHasResource(const uint32_t uiType) const {
    if (!GetSupportsResources())
        return false;

    for (uint32_t i = 0; i < mHeader->resourceCount; i++) {
        if (mHeader->resources[i].type == uiType) {
            return true;
        }
    }

    return false;
}

void *CVTFFile::GetResourceData(const uint32_t uiType, uint32_t &uiSize, Diagnostics::CError &error) const {
    if (IsLoaded()) {
        if (GetSupportsResources()) {
            switch (uiType) {
                case VTF_LEGACY_RSRC_LOW_RES_IMAGE:
                    uiSize = mThumbnailBufferSize;
                    return mThumbnailImageData;
                    break;
                case VTF_LEGACY_RSRC_IMAGE:
                    uiSize = mImageBufferSize;
                    return mImageData;
                    break;
                default:
                    for (uint32_t i = 0; i < mHeader->resourceCount; i++) {
                        if (mHeader->resources[i].type == uiType) {
                            if (mHeader->resources[i].flags & RSRCF_HAS_NO_DATA_CHUNK) {
                                uiSize = sizeof(uint32_t);
                                return &mHeader->resources[i].data;
                            } else {
                                uiSize = mHeader->data[i].size;
                                return mHeader->data[i].data;
                            }
                        }
                    }
                    break;
            }
        } else {
            VTFError_Set(error, "Resources require VTF file version v7.3 and up.");
        }
    }

    uiSize = 0;
    return nullptr;
}

void *CVTFFile::SetResourceData(const uint32_t uiType, const uint32_t uiSize, void *lpData,
                                Diagnostics::CError &error) {
    if (IsLoaded()) {
        if (GetSupportsResources()) {
            switch (uiType) {
                case VTF_LEGACY_RSRC_LOW_RES_IMAGE:
                    VTFError_Set(error, "Low resolution image resource cannot be modified through resource interface.");
                    break;
                case VTF_LEGACY_RSRC_IMAGE:
                    VTFError_Set(error, "Image resource cannot be modified through resource interface.");
                    break;
                default:
                    for (uint32_t i = 0; i < mHeader->resourceCount; i++) {
                        if (mHeader->resources[i].type == uiType) {
                            if (uiSize == 0) {
                                delete[] mHeader->data[i].data;
                                for (uint32_t j = i + 1; j < mHeader->resourceCount; j++) {
                                    mHeader->resources[j - 1] = mHeader->resources[j];
                                    mHeader->data[j - 1] = mHeader->data[j];
                                }
                                mHeader->resourceCount--;
                                ComputeResources();
                                return nullptr;
                            } else {
                                if (mHeader->resources[i].flags & RSRCF_HAS_NO_DATA_CHUNK) {
                                    if (uiSize != sizeof(uint32_t)) {
                                        VTFError_Set(error, "Resources with no data chunk must have size 4.");
                                        return nullptr;
                                    }
                                    if (lpData == nullptr) {
                                        mHeader->resources[i].data = 0;
                                    } else if (&mHeader->resources[i].data != lpData) {
                                        mHeader->resources[i].data = *(uint32_t *) lpData;
                                    }
                                    return &mHeader->resources[i].data;
                                } else {
                                    if (mHeader->data[i].size != uiSize) {
                                        delete[] mHeader->data[i].data;
                                        mHeader->data[i].size = uiSize;
                                        mHeader->data[i].data = new uint8_t[uiSize];
                                        ComputeResources();
                                    }
                                    if (lpData == nullptr) {
                                        memset(mHeader->data[i].data, 0, mHeader->data[i].size);
                                    } else if (mHeader->data[i].data != lpData) {
                                        memcpy(mHeader->data[i].data, lpData, mHeader->data[i].size);
                                    }
                                    return mHeader->data[i].data;
                                }
                            }
                        }
                    }

                    // Resource not found.
                    if (uiSize != 0) {
                        if (mHeader->resourceCount == VTF_RSRC_MAX_DICTIONARY_ENTRIES) {
                            VTFError_Set_Formatted(error, "Maximum directory entry count %u reached.",
                                               VTF_RSRC_MAX_DICTIONARY_ENTRIES);
                            return nullptr;
                        }

                        uint32_t uiIndex = mHeader->resourceCount;

                        mHeader->resources[uiIndex].type = uiType;
                        mHeader->resources[uiIndex].data = 0;

                        mHeader->data[uiIndex].size = 0;
                        mHeader->data[uiIndex].data = nullptr;

                        if (mHeader->resources[uiIndex].flags & RSRCF_HAS_NO_DATA_CHUNK) {
                            if (uiSize != sizeof(uint32_t)) {
                                VTFError_Set(error, "Resources with no data chunk must have size 4.");
                                return nullptr;
                            }
                            if (lpData != nullptr) {
                                mHeader->resources[uiIndex].data = *(uint32_t *) lpData;
                            } else {
                                mHeader->resources[uiIndex].data = 0;
                            }
                            mHeader->resourceCount++;
                            ComputeResources();
                            return &mHeader->resources[uiIndex].data;
                        } else {
                            mHeader->data[uiIndex].size = uiSize;
                            mHeader->data[uiIndex].data = new uint8_t[uiSize];
                            if (lpData != nullptr) {
                                memcpy(mHeader->data[uiIndex].data, lpData, mHeader->data[uiIndex].size);
                            } else {
                                memset(mHeader->data[uiIndex].data, 0, mHeader->data[uiIndex].size);
                            }
                            mHeader->resourceCount++;
                            ComputeResources();
                            return mHeader->data[uiIndex].data;
                        }
                    }
                    break;
            }
        } else {
            VTFError_Set(error, "Resources require VTF file version v7.3 and up.");
        }
    }

    return nullptr;
}

vlBool CVTFFile::GetSupportsAuxCompression() const {
    if (!IsLoaded())
        return false;

    return mHeader->version[0] > VTF_MAJOR_VERSION
           || (mHeader->version[0] == VTF_MAJOR_VERSION
               && mHeader->version[1] >= VTF_MINOR_VERSION_MIN_AUX_COMPRESSION);
}

int16_t CVTFFile::GetAuxCompressionLevel() const {
    if (!GetSupportsAuxCompression())
        return VTF_AUX_COMPRESSION_LEVEL_NONE;

    return mAuxCompressionLevel;
}

vlBool CVTFFile::SetAuxCompressionLevel(const int16_t sLevel, Diagnostics::CError &error) {
    if (!GetSupportsAuxCompression()) {
        VTFError_Set_Formatted(error, "Auxiliary compression requires VTF file version v%d.%d and up.", VTF_MAJOR_VERSION,
                           VTF_MINOR_VERSION_MIN_AUX_COMPRESSION);
        return false;
    }

    if (sLevel < VTF_AUX_COMPRESSION_LEVEL_DEFAULT || sLevel > VTF_AUX_COMPRESSION_LEVEL_MAX) {
        VTFError_Set_Formatted(error, "Auxiliary compression level %d is out of the %d to %d range.", sLevel,
                           VTF_AUX_COMPRESSION_LEVEL_DEFAULT, VTF_AUX_COMPRESSION_LEVEL_MAX);
        return false;
    }

    if (mAuxCompressionLevel != sLevel) {
        mAuxCompressionLevel = sLevel;
        DestroyAuxCompression();
    }

    return true;
}

int16_t CVTFFile::GetAuxCompressionMethod() const {
    if (!GetSupportsAuxCompression())
        return AUX_COMPRESSION_METHOD_DEFLATE;

    return mAuxCompressionMethod;
}

vlBool CVTFFile::SetAuxCompressionMethod(const int16_t sMethod, Diagnostics::CError &error) {
    if (!GetSupportsAuxCompression()) {
        VTFError_Set_Formatted(error, "Auxiliary compression requires VTF file version v%d.%d and up.", VTF_MAJOR_VERSION,
                           VTF_MINOR_VERSION_MIN_AUX_COMPRESSION);
        return false;
    }

    if (sMethod != AUX_COMPRESSION_METHOD_DEFLATE && sMethod != AUX_COMPRESSION_METHOD_ZSTD) {
        VTFError_Set_Formatted(error, "Unsupported auxiliary compression method %d.", sMethod);
        return false;
    }

    if (mAuxCompressionMethod != sMethod) {
        mAuxCompressionMethod = sMethod;
        DestroyAuxCompression();
    }

    return true;
}

//
// GenerateMipmaps()
// Generate mipmaps from the first mipmap level.
//
vlBool CVTFFile::GenerateMipmaps(Diagnostics::CError &error, const VTFMipmapFilter MipmapFilter, const vlBool bSRGB) {
    if (!IsLoaded())
        return false;

    if (mHeader->mipCount == 0)
        return true;

    uint32_t uiFrameCount = GetFrameCount();
    uint32_t uiFaceCount = GetFaceCount();

    for (uint32_t i = 0; i < uiFrameCount; i++) {
        for (uint32_t j = 0; j < uiFaceCount; j++) {
            if (!GenerateMipmaps(i, j, error, MipmapFilter, bSRGB)) {
                return false;
            }
        }
    }

    return true;
}

//
// GenerateMipmaps()
// Generate mipmaps from the first mipmap level of the specified frame and face.
//
vlBool CVTFFile::GenerateMipmaps(const uint32_t uiFace, const uint32_t uiFrame, Diagnostics::CError &error,
                                 const VTFMipmapFilter MipmapFilter, vlBool bSRGB) {
    if (!IsLoaded())
        return false;

    if (mImageData == nullptr) {
        VTFError_Set(error, "No image data to generate mipmaps from.");
        return false;
    }

    if (mHeader->depth > 1) {
        VTFError_Set(error, "Mipmap generation for depth textures is not supported.");
        return false;
    }

    assert(MipmapFilter >= 0 && MipmapFilter < MIPMAP_FILTER_COUNT);

    if (mHeader->mipCount <= 1)
        return true;

    stbir_filter stbFilter = STBIR_FILTER_DEFAULT;

    switch (MipmapFilter) {
        case MIPMAP_FILTER_BOX:
            stbFilter = STBIR_FILTER_BOX;
            break;
        case MIPMAP_FILTER_TRIANGLE:
            stbFilter = STBIR_FILTER_TRIANGLE;
            break;
        case MIPMAP_FILTER_QUADRATIC:
        case MIPMAP_FILTER_CUBIC:
        case MIPMAP_FILTER_GAUSSIAN:
            stbFilter = STBIR_FILTER_CUBICBSPLINE;
            break;
        case MIPMAP_FILTER_CATROM:
            stbFilter = STBIR_FILTER_CATMULLROM;
            break;
        case MIPMAP_FILTER_MITCHELL:
        case MIPMAP_FILTER_SINC:
        case MIPMAP_FILTER_BESSEL:
        case MIPMAP_FILTER_HANNING:
        case MIPMAP_FILTER_HAMMING:
        case MIPMAP_FILTER_BLACKMAN:
        case MIPMAP_FILTER_KAISER:
            stbFilter = STBIR_FILTER_MITCHELL;
            break;
        default:
            break;
    }

    uint32_t srcWidth = mHeader->width;
    uint32_t srcHeight = mHeader->height;

    std::vector<uint8_t> src(
        ComputeImageSize(
            srcWidth,
            srcHeight,
            1,
            IMAGE_FORMAT_RGBA8888));

    if (!ConvertToRGBA8888(
        GetData(uiFace, uiFrame, 0, 0),
        src.data(),
        srcWidth,
        srcHeight,
        GetDecodeFormat(),
        error)) {
        return false;
    }

    for (uint32_t mip = 1; mip < mHeader->mipCount; ++mip) {
        const uint32_t dstWidth = std::max<uint32_t>(1, srcWidth >> 1);
        const uint32_t dstHeight = std::max<uint32_t>(1, srcHeight >> 1);

        std::vector<uint8_t> dst(
            ComputeImageSize(
                dstWidth,
                dstHeight,
                1,
                IMAGE_FORMAT_RGBA8888));

        if (MipmapFilter == MIPMAP_FILTER_POINT) {
            for (uint32_t y = 0; y < dstHeight; ++y) {
                const uint32_t srcY =
                        std::min(
                            (2 * y + 1) * srcHeight / (2 * dstHeight),
                            srcHeight - 1);

                for (uint32_t x = 0; x < dstWidth; ++x) {
                    const uint32_t srcX =
                            std::min(
                                (2 * x + 1) * srcWidth / (2 * dstWidth),
                                srcWidth - 1);

                    const uint8_t *s =
                            src.data() + (srcY * srcWidth + srcX) * 4;

                    uint8_t *d =
                            dst.data() + (y * dstWidth + x) * 4;

                    d[0] = s[0];
                    d[1] = s[1];
                    d[2] = s[2];
                    d[3] = s[3];
                }
            }
        } else {
            if (!stbir_resize_uint8_generic(
                src.data(),
                static_cast<int>(srcWidth),
                static_cast<int>(srcHeight),
                0,
                dst.data(),
                static_cast<int>(dstWidth),
                static_cast<int>(dstHeight),
                0,
                4,
                STBIR_ALPHA_CHANNEL_NONE,
                0,
                STBIR_EDGE_CLAMP,
                stbFilter,
                STBIR_COLORSPACE_LINEAR,
                nullptr)) {
                VTFError_Set(error, "Failed to generate mipmap.");
                return false;
            }
        }

        if (!ConvertFromRGBA8888(
            dst.data(),
            GetData(uiFace, uiFrame, 0, mip),
            dstWidth,
            dstHeight,
            mHeader->imageFormat,
            error)) {
            return false;
        }

        src.swap(dst);

        srcWidth = dstWidth;
        srcHeight = dstHeight;
    }

    return true;
}

//
// GenerateThumbnail()
// We should have a mipmap that matches the thumbnail size.  This function finds it and
// copies it over to the mipmap data, converting it if need be.
//
vlBool CVTFFile::GenerateThumbnail(const vlBool bSRGB, Diagnostics::CError &error) {
    if (!IsLoaded())
        return false;

    if (!GetHasThumbnail()) {
        VTFError_Set(error, "VTF file does not have a thumbnail.");
        return false;
    }

    if (mImageData == nullptr) {
        VTFError_Set(error, "No image data to generate thumbnail from.");
        return false;
    }

    // Find a mipmap that matches the size of the thumbnail.
    for (uint32_t i = 0; i < mHeader->mipCount; i++) {
        uint32_t uiMipmapWidth, uiMipmapHeight, uiMipmapDepth;
        ComputeMipmapDimensions(mHeader->width, mHeader->height, 1, i, uiMipmapWidth,
                                uiMipmapHeight, uiMipmapDepth);

        if (uiMipmapWidth == (uint32_t) mHeader->lowResImageWidth && uiMipmapHeight == (uint32_t) mHeader->
            lowResImageHeight) {
            // Check if it is the same format (in which case copy it) otherwise convert
            // it to the right format and copy it.
            if (mHeader->imageFormat == mHeader->lowResImageFormat) {
                SetThumbnailData(GetData(0, 0, 0, i));
            } else {
                if (!Convert(GetData(0, 0, 0, i), GetThumbnailData(), uiMipmapWidth,
                             uiMipmapHeight, mHeader->imageFormat, mHeader->lowResImageFormat, error)) {
                    return false;
                }
            }
            return true;
        }
    }

    // We don't have a matching mipmap (maybe we have no mipmaps) so generate one.
    uint8_t *lpImageData = new uint8_t[ComputeImageSize(mHeader->width, mHeader->height, 1,
                                                        IMAGE_FORMAT_RGBA8888)];
    uint8_t *lpThumbnailImageData = new uint8_t[ComputeImageSize(
        mHeader->lowResImageWidth, mHeader->lowResImageHeight, 1, IMAGE_FORMAT_RGBA8888)];

    if (!ConvertToRGBA8888(GetData(0, 0, 0, 0), lpImageData, mHeader->width, mHeader->height,
                           GetDecodeFormat(), error)) {
        delete[] lpImageData;
        delete[] lpThumbnailImageData;

        return false;
    }

    if (!Resize(lpImageData, lpThumbnailImageData, mHeader->width, mHeader->height,
                mHeader->lowResImageWidth, mHeader->lowResImageHeight, MIPMAP_FILTER_CATROM,
                bSRGB, error)) {
        delete[] lpImageData;
        delete[] lpThumbnailImageData;

        return false;
    }

    if (!ConvertFromRGBA8888(lpThumbnailImageData, GetThumbnailData(), mHeader->lowResImageWidth,
                             mHeader->lowResImageHeight, mHeader->lowResImageFormat, error)) {
        delete[] lpImageData;
        delete[] lpThumbnailImageData;

        return false;
    }

    delete[] lpImageData;
    delete[] lpThumbnailImageData;

    //VTFError_Set(error, "VTF file does not have a mipmap that matches the thumbnail size.");
    return true;
}

//
// GenerateNormalMap()
// Convert the first level mipmap of each frame to a normal map.
//
vlBool CVTFFile::GenerateNormalMap(Diagnostics::CError &error, const VTFKernelFilter KernelFilter,
                                   const VTFHeightConversionMethod HeightConversionMethod,
                                   const VTFNormalAlphaResult NormalAlphaResult) {
    if (!IsLoaded())
        return false;

    uint32_t uiFrameCount = GetFrameCount();

    for (uint32_t i = 0; i < uiFrameCount; i++) {
        if (!GenerateNormalMap(i, error, KernelFilter, HeightConversionMethod, NormalAlphaResult)) {
            return false;
        }
    }

    return true;
}

//
// GenerateNormalMap()
// Convert the first level mipmap of the specified frame to a normal map.
//
vlBool CVTFFile::GenerateNormalMap(const uint32_t uiFrame, Diagnostics::CError &error,
                                   VTFKernelFilter KernelFilter,
                                   VTFHeightConversionMethod HeightConversionMethod,
                                   VTFNormalAlphaResult NormalAlphaResult) {
    if (!IsLoaded())
        return false;

    if (mHeader->flags & TEXTUREFLAGS_ENVMAP) {
        VTFError_Set(error, "Image is an enviroment map.");
        return false;
    }

    if (mImageData == nullptr) {
        VTFError_Set(error, "No image data to generate normal map from.");
        return false;
    }

    uint8_t *lpData = GetData(0, uiFrame, 0, 0);

    // Will hold frame's converted image data.
    uint8_t *lpSource = new uint8_t[ComputeImageSize(mHeader->width, mHeader->height, 1,
                                                     IMAGE_FORMAT_RGBA8888)];

    // Get the frame's image data.
    if (!ConvertToRGBA8888(lpData, lpSource, mHeader->width, mHeader->height,
                           GetDecodeFormat(), error)) {
        delete[] lpSource;

        return false;
    }

    // Will hold normal image data.
    //uint8_t *lpDest = new uint8_t[ComputeImageSize(Header->Width, Header->Height, IMAGE_FORMAT_RGBA8888)];

    //delete[] lpSource;

    // Set the frame's image data.
    if (!ConvertFromRGBA8888(lpSource/*lpDest*/, lpData, mHeader->width, mHeader->height,
                             mHeader->imageFormat, error)) {
        delete[] lpSource; // Moved from above.
        //delete[] lpDest;

        return false;
    }

    delete[] lpSource; // Moved from above.
    //delete[] lpDest;

    return true;
}

// Simple struct for holding face data for SphereMap rendering
// -----------------------------------------------------------
struct SphereMapFace {
    uint32_t *buf; // pointer to the address where the image data is.
    Vector u, v, n, o; // vectors for plane equations
};

// Define our faces and vectors (don't moan about the order!)
// ----------------------------------------------------------
SphereMapFace SFace[6] =
{
    {nullptr, {0, 0, -1}, {0, 1, 0}, {-1, 0, 0}, {-0.5, -0.5, 0.5}}, // left (lf)
    {nullptr, {1, 0, 0}, {0, 1, 0}, {0, 0, -1}, {-0.5, -0.5, -0.5}}, // down (dn)
    {nullptr, {0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {0.5, -0.5, -0.5}}, // right (rt)
    {nullptr, {-1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0.5, -0.5, 0.5}}, // up (up)
    {nullptr, {1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {-0.5, 0.5, -0.5}}, // front (ft)
    {nullptr, {1, 0, 0}, {0, 0, -1}, {0, -1, 0}, {-0.5, -0.5, 0.5}} // back (bk)
};

// Normalised pixel colour struct
// ------------------------------
struct NColour {
    float r, g, b;
};

//
// GenerateSphereMap()
// Generate a sphere map from the first six faces (the cube map) of an enviroment map.
//
vlBool CVTFFile::GenerateSphereMap(Diagnostics::CError &error) {
    if (!IsLoaded())
        return false;

    if (!(mHeader->flags & TEXTUREFLAGS_ENVMAP)) {
        VTFError_Set(error, "Image is not an enviroment map.");
        return false;
    }

    if (mHeader->startFrame == 0xffff) {
        VTFError_Set(error, "Enviroment map does not have a sphere map.");
        return false;
    }

    if (mImageData == nullptr) {
        VTFError_Set(error, "No image data to generate sphere map from.");
        return false;
    }

    uint32_t uiWidth = (uint32_t) mHeader->width;
    uint32_t uiHeight = (uint32_t) mHeader->height;

    // lets go!
    uint8_t *lpImageData[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    // 6 pointers to memory for our faces.
    uint8_t *lpSphereMapData = nullptr; // SphereMap buffer
    uint32_t map[6] = {2, 0, 5, 4, 3, 1}; // used to remap valves face order to my face order.
    uint32_t samples = 4; // pixel samples for rendering

    uint32_t i, j, x, y, f;
    NColour c, texel, average;
    Vector v, r, p;
    float s, t, temp, k;

    // load the faces into the buffers and convert as needed
    for (i = 0; i < 6; i++) {
        uint32_t j = map[i]; // Valve face order to my face order map.

        lpImageData[j] = new uint8_t[ComputeImageSize(uiWidth, uiHeight, 1, IMAGE_FORMAT_RGBA8888)];

        if (!ConvertToRGBA8888(GetData(0, i, 0, 0), lpImageData[j], uiWidth, uiHeight,
                               GetDecodeFormat(), error)) {
            for (uint32_t l = 0; l < 6; l++)
                delete[] lpImageData[l];
            VTFError_Set(error, "Could not convert source to RGBA8888 format");
            return false;
        }
        SFace[j].buf = (uint32_t *) lpImageData[j]; // save the address
    }

    // Assuming at this point our faces have loaded fine, create a buffer for the SphereMap
    lpSphereMapData = new uint8_t[ComputeImageSize(uiWidth, uiHeight, 1, IMAGE_FORMAT_RGBA8888)];

    // At this point we need to flip 4 of the faces as follows as their "Valve" orientation
    // is different to what the SphereMap rendering code needs.
    // up - flip horizontal
    // rt - flip horizontal
    // ft - flip vertical
    // bk - flip vertical

    MirrorImage(lpImageData[0], mHeader->width, mHeader->height);
    MirrorImage(lpImageData[2], mHeader->width, mHeader->height);
    MirrorImage(lpImageData[3], mHeader->width, mHeader->height);
    FlipImage(lpImageData[4], mHeader->width, mHeader->height);
    FlipImage(lpImageData[5], mHeader->width, mHeader->height);

    // disable conversion warning
    //#pragma warning(disable: 4244)

    // calculate the average colour for the forward face
    // using just the forward face is quicker and seems fairly
    // consistent with what Valves own SphereMaps look like.
    uint32_t uiAvgR = 0, uiAvgG = 0, uiAvgB = 0;
    uint32_t uiPixelCount = uiWidth * uiHeight;

    uint8_t *src = lpImageData[3]; // 3 = up or forward face
    uint8_t *lpSourceEnd = src + (uiWidth * uiHeight * 4);

    for (; src < lpSourceEnd; src += 4) {
        uiAvgR += src[0];
        uiAvgG += src[1];
        uiAvgB += src[2];
    }

    uiAvgR /= uiPixelCount;
    uiAvgG /= uiPixelCount;
    uiAvgB /= uiPixelCount;

    // the value here is 1/255 - we're normalising the RGBs.
    average.r = 0.003921f * (float) uiAvgR;
    average.g = 0.003921f * (float) uiAvgG;
    average.b = 0.003921f * (float) uiAvgB;

    uint8_t *lpSphereMapDataPointer = lpSphereMapData;

    // Calculate sphere-map by rendering a perfectly reflective solid sphere.
    for (y = 0; y < uiHeight; y++) {
        for (x = 0; x < uiWidth; x++) {
            texel.r = texel.g = texel.b = 0.0f;

            for (j = 0; j < samples; j++) {
                s = ((float) x + (float) drand48()) / (float) uiWidth - 0.5f;
                t = ((float) y + (float) drand48()) / (float) uiHeight - 0.5f;
                temp = s * s + t * t;

                //point not on sphere so use the average colour
                if (temp >= 0.25f) {
                    texel.r += average.r;
                    texel.g += average.g;
                    texel.b += average.b;
                    continue;
                }

                //get point on sphere
                p.x = s;
                p.y = t;
                p.z = (float) sqrt(0.25f - temp);
                VecScale(&p, 2.0f);

                //ray from infinity (eyepoint) to surface
                v.x = 0.0f;
                v.y = 0.0f;
                v.z = 1.0f;

                //get reflected ray
                VecReflect(&p, &v, &r);

                //Intersect reflected ray with cube
                f = Intersect(&r);
                k = VecDot(&SFace[f].o, &SFace[f].n) / VecDot(&r, &SFace[f].n);
                VecScale(&r, k);
                VecSub(&r, &SFace[f].o, &v);

                //Get texture map-indices
                s = VecDot(&v, &SFace[f].u);
                t = VecDot(&v, &SFace[f].v);

                //Sample to get color
                SphereMapFace *pf = &SFace[f];
                uint32_t xpos, ypos;
                uint8_t *p;

                xpos = (uint32_t) (s * (float) uiWidth);
                ypos = (uint32_t) (t * (float) uiHeight);

                p = (uint8_t *) &pf->buf[ypos * uiWidth + xpos];
                c.r = (float) p[0] / 255.0f;
                c.g = (float) p[1] / 255.0f;
                c.b = (float) p[2] / 255.0f;

                texel.r += c.r;
                texel.g += c.g;
                texel.b += c.b;
            }

            // punch the pixel into our SphereMap image buffer
            lpSphereMapDataPointer[0] = (uint8_t) (255.0f * texel.r / (float) samples);
            lpSphereMapDataPointer[1] = (uint8_t) (255.0f * texel.g / (float) samples);
            lpSphereMapDataPointer[2] = (uint8_t) (255.0f * texel.b / (float) samples);
            lpSphereMapDataPointer[3] = 0xff;
            lpSphereMapDataPointer += 4;
        }
    }

    //#pragma warning(default: 4244)

    if (!ConvertFromRGBA8888(lpSphereMapData,
                             GetData(0, CUBEMAP_FACE_SphereMap, 0, 0),
                             mHeader->width,
                             mHeader->height,
                             mHeader->imageFormat, error)) {
        for (i = 0; i < 6; i++) {
            delete[] lpImageData[i];
        }
        delete[] lpSphereMapData;

        return false;
    };

    // delete the memory buffers
    for (i = 0; i < 6; i++) {
        delete[] lpImageData[i];
    }
    delete[] lpSphereMapData;

    return true;
}

//
// ComputeReflectivity()
// Compute the reflectivity value of the texture using all faces and frames.
//
vlBool CVTFFile::ComputeReflectivity(Diagnostics::CError &error) {
    if (!IsLoaded())
        return false;

    if (mImageData == nullptr) {
        VTFError_Set(error, "No image data to compute reflectivity from.");

        return false;
    }

    mHeader->reflectivity[0] = 0.0f;
    mHeader->reflectivity[1] = 0.0f;
    mHeader->reflectivity[2] = 0.0f;

    uint8_t *lpImageData = new uint8_t[ComputeImageSize(mHeader->width, mHeader->height, 1,
                                                        IMAGE_FORMAT_RGBA8888)];

    uint32_t uiFrameCount = GetFrameCount();
    uint32_t uiFaceCount = GetFaceCount();
    uint32_t uiSliceCount = GetDepth();

    for (uint32_t uiFrame = 0; uiFrame < uiFrameCount; uiFrame++) {
        for (uint32_t uiFace = 0; uiFace < uiFaceCount; uiFace++) {
            for (uint32_t uiSlice = 0; uiSlice < uiSliceCount; uiSlice++) {
                if (!ConvertToRGBA8888(GetData(uiFrame, uiFace, uiSlice, 0), lpImageData,
                                       mHeader->width, mHeader->height, GetDecodeFormat(),
                                       error)) {
                    delete[] lpImageData;

                    return false;
                }

                float sX, sY, sZ;
                ComputeImageReflectivity(lpImageData, mHeader->width, mHeader->height, sX, sY, sZ);

                mHeader->reflectivity[0] += sX;
                mHeader->reflectivity[1] += sY;
                mHeader->reflectivity[2] += sZ;
            }
        }
    }

    float sInverse = 1.0f / (float) (uiFrameCount * uiFaceCount * uiSliceCount);

    mHeader->reflectivity[0] *= sInverse;
    mHeader->reflectivity[1] *= sInverse;
    mHeader->reflectivity[2] *= sInverse;

    delete[] lpImageData;

    return true;
}

// Array which holds information about our image format
// (taken from imageloader.cpp, Valve Source SDK)
//------------------------------------------------------
static SVTFImageFormatInfo VTFImageFormatInfo[] =
{
    {"RGBA8888", 32, 4, 8, 8, 8, 8, false, true}, // IMAGE_FORMAT_RGBA8888,
    {"ABGR8888", 32, 4, 8, 8, 8, 8, false, true}, // IMAGE_FORMAT_ABGR8888,
    {"RGB888", 24, 3, 8, 8, 8, 0, false, true}, // IMAGE_FORMAT_RGB888,
    {"BGR888", 24, 3, 8, 8, 8, 0, false, true}, // IMAGE_FORMAT_BGR888,
    {"RGB565", 16, 2, 5, 6, 5, 0, false, true}, // IMAGE_FORMAT_RGB565,
    {"I8", 8, 1, 0, 0, 0, 0, false, true}, // IMAGE_FORMAT_I8,
    {"IA88", 16, 2, 0, 0, 0, 8, false, true}, // IMAGE_FORMAT_IA88
    {"P8", 8, 1, 0, 0, 0, 0, false, false}, // IMAGE_FORMAT_P8
    {"A8", 8, 1, 0, 0, 0, 8, false, true}, // IMAGE_FORMAT_A8
    {"RGB888 Bluescreen", 24, 3, 8, 8, 8, 0, false, true}, // IMAGE_FORMAT_RGB888_BLUESCREEN
    {"BGR888 Bluescreen", 24, 3, 8, 8, 8, 0, false, true}, // IMAGE_FORMAT_BGR888_BLUESCREEN
    {"ARGB8888", 32, 4, 8, 8, 8, 8, false, true}, // IMAGE_FORMAT_ARGB8888
    {"BGRA8888", 32, 4, 8, 8, 8, 8, false, true}, // IMAGE_FORMAT_BGRA8888
    {"DXT1", 4, 0, 0, 0, 0, 0, true, true}, // IMAGE_FORMAT_DXT1
    {"DXT3", 8, 0, 0, 0, 0, 8, true, true}, // IMAGE_FORMAT_DXT3
    {"DXT5", 8, 0, 0, 0, 0, 8, true, true}, // IMAGE_FORMAT_DXT5
    {"BGRX8888", 32, 4, 8, 8, 8, 0, false, true}, // IMAGE_FORMAT_BGRX8888
    {"BGR565", 16, 2, 5, 6, 5, 0, false, true}, // IMAGE_FORMAT_BGR565
    {"BGRX5551", 16, 2, 5, 5, 5, 0, false, true}, // IMAGE_FORMAT_BGRX5551
    {"BGRA4444", 16, 2, 4, 4, 4, 4, false, true}, // IMAGE_FORMAT_BGRA4444
    {"DXT1 One Bit Alpha", 4, 0, 0, 0, 0, 1, true, true}, // IMAGE_FORMAT_DXT1_ONEBITALPHA
    {"BGRA5551", 16, 2, 5, 5, 5, 1, false, true}, // IMAGE_FORMAT_BGRA5551
    {"UV88", 16, 2, 8, 8, 0, 0, false, true}, // IMAGE_FORMAT_UV88
    {"UVWQ8888", 32, 4, 8, 8, 8, 8, false, true}, // IMAGE_FORMAT_UVWQ8899
    {"RGBA16161616F", 64, 8, 16, 16, 16, 16, false, true}, // IMAGE_FORMAT_RGBA16161616F
    {"RGBA16161616", 64, 8, 16, 16, 16, 16, false, true}, // IMAGE_FORMAT_RGBA16161616
    {"UVLX8888", 32, 4, 8, 8, 8, 8, false, true}, // IMAGE_FORMAT_UVLX8888
    {"R32F", 32, 4, 32, 0, 0, 0, false, true}, // IMAGE_FORMAT_R32F
    {"RGB323232F", 96, 12, 32, 32, 32, 0, false, true}, // IMAGE_FORMAT_RGB323232F
    {"RGBA32323232F", 128, 16, 32, 32, 32, 32, false, true}, // IMAGE_FORMAT_RGBA32323232F
    {"nVidia DST16", 16, 2, 0, 0, 0, 0, false, true}, // IMAGE_FORMAT_NV_DST16
    {"nVidia DST24", 24, 3, 0, 0, 0, 0, false, true}, // IMAGE_FORMAT_NV_DST24
    {"nVidia INTZ", 32, 4, 0, 0, 0, 0, false, true}, // IMAGE_FORMAT_NV_INTZ
    {"nVidia RAWZ", 32, 4, 0, 0, 0, 0, false, true}, // IMAGE_FORMAT_NV_RAWZ
    {"ATI DST16", 16, 2, 0, 0, 0, 0, false, true}, // IMAGE_FORMAT_ATI_DST16
    {"ATI DST24", 24, 3, 0, 0, 0, 0, false, true}, // IMAGE_FORMAT_ATI_DST24
    {"nVidia NULL", 32, 4, 0, 0, 0, 0, false, true}, // IMAGE_FORMAT_NV_NULL
    {"ATI2N", 8, 0, 0, 0, 0, 0, true, true}, // IMAGE_FORMAT_ATI2N
    {"ATI1N", 4, 0, 0, 0, 0, 0, true, true}, // IMAGE_FORMAT_ATI1N
    /*
    { "Xbox360 DST16",		 16,  0,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_X360_DST16
    { "Xbox360 DST24",		 24,  0,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_X360_DST24
    { "Xbox360 DST24F",		 24,  0,  0,  0,  0,  0, false , true },		// IMAGE_FORMAT_X360_DST24F
    { "Linear BGRX8888",	 32,  4,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_LINEAR_BGRX8888
    { "Linear RGBA8888",     32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_LINEAR_RGBA8888
    { "Linear ABGR8888",	 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_LINEAR_ABGR8888
    { "Linear ARGB8888",	 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_LINEAR_ARGB8888
    { "Linear BGRA8888",	 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_LINEAR_BGRA8888
    { "Linear RGB888",		 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_LINEAR_RGB888
    { "Linear BGR888",		 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_LINEAR_BGR888
    { "Linear BGRX5551",	 16,  2,  5,  5,  5,  0, false,  true },		// IMAGE_FORMAT_LINEAR_BGRX5551
    { "Linear I8",			  8,  1,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_LINEAR_I8
    { "Linear RGBA16161616", 64,  8, 16, 16, 16, 16, false,  true },		// IMAGE_FORMAT_LINEAR_RGBA16161616
    { "LE BGRX8888",         32,  4,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_LE_BGRX8888
    { "LE BGRA8888",		 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_LE_BGRA8888
    */
    {"Reserved39", 0, 0, 0, 0, 0, 0, false, false}, // 39
    {"Reserved40", 0, 0, 0, 0, 0, 0, false, false}, // 40
    {"Reserved41", 0, 0, 0, 0, 0, 0, false, false}, // 41
    {"Reserved42", 0, 0, 0, 0, 0, 0, false, false}, // 42
    {"Reserved43", 0, 0, 0, 0, 0, 0, false, false}, // 43
    {"Reserved44", 0, 0, 0, 0, 0, 0, false, false}, // 44
    {"Reserved45", 0, 0, 0, 0, 0, 0, false, false}, // 45
    {"Reserved46", 0, 0, 0, 0, 0, 0, false, false}, // 46
    {"Reserved47", 0, 0, 0, 0, 0, 0, false, false}, // 47
    {"Reserved48", 0, 0, 0, 0, 0, 0, false, false}, // 48
    {"Reserved49", 0, 0, 0, 0, 0, 0, false, false}, // 49
    {"Reserved50", 0, 0, 0, 0, 0, 0, false, false}, // 50
    {"Reserved51", 0, 0, 0, 0, 0, 0, false, false}, // 51
    {"Reserved52", 0, 0, 0, 0, 0, 0, false, false}, // 52
    {"Reserved53", 0, 0, 0, 0, 0, 0, false, false}, // 53
    {"Reserved54", 0, 0, 0, 0, 0, 0, false, false}, // 54
    {"Reserved55", 0, 0, 0, 0, 0, 0, false, false}, // 55
    {"Reserved56", 0, 0, 0, 0, 0, 0, false, false}, // 56
    {"Reserved57", 0, 0, 0, 0, 0, 0, false, false}, // 57
    {"Reserved58", 0, 0, 0, 0, 0, 0, false, false}, // 58
    {"Reserved59", 0, 0, 0, 0, 0, 0, false, false}, // 59
    {"Reserved60", 0, 0, 0, 0, 0, 0, false, false}, // 60
    {"Reserved61", 0, 0, 0, 0, 0, 0, false, false}, // 61
    {"Reserved62", 0, 0, 0, 0, 0, 0, false, false}, // 62
    {"Reserved63", 0, 0, 0, 0, 0, 0, false, false}, // 63
    {"Reserved64", 0, 0, 0, 0, 0, 0, false, false}, // 64
    {"Reserved65", 0, 0, 0, 0, 0, 0, false, false}, // 65
    {"Reserved66", 0, 0, 0, 0, 0, 0, false, false}, // 66
    {"Reserved67", 0, 0, 0, 0, 0, 0, false, false}, // 67
    {"Reserved68", 0, 0, 0, 0, 0, 0, false, false}, // 68
    {"R8", 8, 1, 8, 0, 0, 0, false, true}, // IMAGE_FORMAT_R8
    {"BC7", 8, 0, 0, 0, 0, 8, true, true}, // IMAGE_FORMAT_BC7
    {"BC6H", 8, 0, 16, 16, 16, 0, true, true}, // IMAGE_FORMAT_BC6H
    {"BC4", 4, 0, 0, 0, 0, 0, true, true}, // IMAGE_FORMAT_BC4
    {"BC5", 8, 0, 0, 0, 0, 0, true, true} // IMAGE_FORMAT_BC5
};

SVTFImageFormatInfo const &CVTFFile::GetImageFormatInfo(const VTFImageFormat ImageFormat) {
    assert(ImageFormat >= 0 && ImageFormat < IMAGE_FORMAT_COUNT);

    return VTFImageFormatInfo[ImageFormat];
}

//------------------------------------------------------------------------------------
// ComputeImageSize(uint32_t uiWidth, uint32_t uiHeight, VTFImageFormat ImageFormat)
//
// Returns how many bytes are needed to store an image of width * height in the chosen
// image format. If bMipMaps is true, the total will reflect the space needed to store
// the original image plus all the mipmaps down to a size of 1 x 1
//------------------------------------------------------------------------------------
uint32_t CVTFFile::ComputeImageSize(uint32_t uiWidth, uint32_t uiHeight, const uint32_t uiDepth,
                                    const VTFImageFormat ImageFormat) {
    switch (ImageFormat) {
        case IMAGE_FORMAT_DXT1:
        case IMAGE_FORMAT_DXT1_ONEBITALPHA:
        case IMAGE_FORMAT_ATI1N:
        case IMAGE_FORMAT_BC4:
            if (uiWidth < 4 && uiWidth > 0)
                uiWidth = 4;

            if (uiHeight < 4 && uiHeight > 0)
                uiHeight = 4;

            return ((uiWidth + 3) / 4) * ((uiHeight + 3) / 4) * 8 * uiDepth;
        case IMAGE_FORMAT_DXT3:
        case IMAGE_FORMAT_DXT5:
        case IMAGE_FORMAT_ATI2N:
        case IMAGE_FORMAT_BC7:
        case IMAGE_FORMAT_BC6H:
        case IMAGE_FORMAT_BC5:
            if (uiWidth < 4 && uiWidth > 0)
                uiWidth = 4;

            if (uiHeight < 4 && uiHeight > 0)
                uiHeight = 4;

            return ((uiWidth + 3) / 4) * ((uiHeight + 3) / 4) * 16 * uiDepth;
        default:
            return uiWidth * uiHeight * uiDepth * GetImageFormatInfo(ImageFormat).uiBytesPerPixel;
    }
}

//
// ComputeImageSize();
// Gets the size in bytes of the data needed to store an image of size uiWidth x uiHeight
// with uiMipmaps mipmap levels and ImageFormat format.
//
uint32_t CVTFFile::ComputeImageSize(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth, const uint32_t uiMipmaps,
                                    const VTFImageFormat ImageFormat) {
    uint32_t uiImageSize = 0;

    assert(uiWidth != 0 && uiHeight != 0 && uiDepth != 0);

    for (uint32_t i = 0; i < uiMipmaps; i++) {
        uiImageSize += ComputeImageSize(uiWidth, uiHeight, uiDepth, ImageFormat);

        uiWidth >>= 1;
        uiHeight >>= 1;
        uiDepth >>= 1;

        if (uiWidth < 1)
            uiWidth = 1;

        if (uiHeight < 1)
            uiHeight = 1;

        if (uiDepth < 1)
            uiDepth = 1;
    }

    return uiImageSize;
}

//
// ComputeMipmapCount();
// Gets the number of mipmaps an image of size uiWidth x uiHeight will have including
// the mipmap of size uiWidth x uiHeight.
//
uint32_t CVTFFile::ComputeMipmapCount(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth) {
    uint32_t uiCount = 0;

    assert(uiWidth != 0 && uiHeight != 0 && uiDepth != 0);

    while (true) {
        uiCount++;

        uiWidth >>= 1;
        uiHeight >>= 1;
        uiDepth >>= 1;

        if (uiWidth == 0 && uiHeight == 0 && uiDepth == 0)
            break;

        /*if(uiWidth < 1)
            uiWidth = 1;

        if(uiHeight < 1)
            uiHeight = 1;

        if(uiDepth < 1)
            uiDepth = 1;*/
    }

    return uiCount;
}

//-----------------------------------------------------------------------------
// ComputeMIPMapDimensions( int32_t iMipLevel, int32_t *pMipWidth, int32_t *pMipHeight )
//
// Computes the dimensions of a particular mip level
//-----------------------------------------------------------------------------
void CVTFFile::ComputeMipmapDimensions(const uint32_t uiWidth, const uint32_t uiHeight, const uint32_t uiDepth,
                                       const uint32_t uiMipmapLevel,
                                       uint32_t &uiMipmapWidth, uint32_t &uiMipmapHeight, uint32_t &uiMipmapDepth) {
    // work out the width/height by taking the orignal dimension
    // and bit shifting them down uiMipmapLevel times
    uiMipmapWidth = uiWidth >> uiMipmapLevel;
    uiMipmapHeight = uiHeight >> uiMipmapLevel;
    uiMipmapDepth = uiDepth >> uiMipmapLevel;

    // stop the dimension being less than 1 x 1
    if (uiMipmapWidth < 1)
        uiMipmapWidth = 1;

    if (uiMipmapHeight < 1)
        uiMipmapHeight = 1;

    if (uiMipmapDepth < 1)
        uiMipmapDepth = 1;
}

//-----------------------------------------------------------------------------
// ComputeMIPSize( int32_t iMipLevel, VTFImageFormat fmt )
//
// Computes the size (in bytes) of a single mipmap of a single face of a single frame
//-----------------------------------------------------------------------------
uint32_t CVTFFile::ComputeMipmapSize(const uint32_t uiWidth, const uint32_t uiHeight, const uint32_t uiDepth,
                                     const uint32_t uiMipmapLevel,
                                     const VTFImageFormat ImageFormat) {
    // figure out the width/height of this MIP level
    uint32_t uiMipmapWidth, uiMipmapHeight, uiMipmapDepth;
    ComputeMipmapDimensions(uiWidth, uiHeight, uiDepth, uiMipmapLevel, uiMipmapWidth, uiMipmapHeight,
                            uiMipmapDepth);

    // return the memory requirements
    return ComputeImageSize(uiMipmapWidth, uiMipmapHeight, uiMipmapDepth, ImageFormat);
}

//---------------------------------------------------------------------------------
// ComputeDataOffset(uint32_t uiFrame, uint32_t uiFace, uint32_t uiMipLevel, VTFImageFormat ImageFormat)
//
// Returns the offset in our HiResDataBuffer of the data for an image at the
// chose frame, face, and mip level. Frame number starts at 0, Face starts at 0
// MIP level 0 is the largest moving up to MIP count-1 for the smallest
// To get the first, and largest image, you would use 0, 0, 0
//---------------------------------------------------------------------------------
uint32_t CVTFFile::ComputeDataOffset(uint32_t uiFrame, uint32_t uiFace, uint32_t uiSlice, uint32_t uiMipLevel,
                                     const VTFImageFormat ImageFormat) const {
    uint32_t uiOffset = 0;

    uint32_t uiFrameCount = GetFrameCount();
    uint32_t uiFaceCount = GetFaceCount();
    uint32_t uiSliceCount = GetDepth();
    uint32_t uiMipCount = GetMipmapCount();

    if (uiFrame >= uiFrameCount) {
        uiFrame = uiFrameCount - 1;
    }

    if (uiFace >= uiFaceCount) {
        uiFace = uiFaceCount - 1;
    }

    if (uiSlice >= uiSliceCount) {
        uiSlice = uiSliceCount - 1;
    }

    if (uiMipLevel >= uiMipCount) {
        uiMipLevel = uiMipCount - 1;
    }

    // Transverse past all frames and faces of each mipmap (up to the requested one).
    for (int32_t i = (int32_t) uiMipCount - 1; i > (int32_t) uiMipLevel; i--) {
        uiOffset += ComputeMipmapSize(mHeader->width, mHeader->height, mHeader->depth, i,
                                      ImageFormat) * uiFrameCount * uiFaceCount;
    }

    uint32_t uiTemp1 = ComputeMipmapSize(mHeader->width, mHeader->height, mHeader->depth,
                                         uiMipLevel,
                                         ImageFormat);
    uint32_t uiTemp2 = ComputeMipmapSize(mHeader->width, mHeader->height, 1, uiMipLevel, ImageFormat);

    // Transverse past requested frames and faces of requested mipmap.
    uiOffset += uiTemp1 * uiFrame * uiFaceCount * uiSliceCount;
    uiOffset += uiTemp1 * uiFace * uiSliceCount;
    uiOffset += uiTemp2 * uiSlice;

    assert(uiOffset < uiImageBufferSize);

    return uiOffset;
}

//-----------------------------------------------------------------------------------------------------
// ConvertToRGBA8888( uint8_t *src, uint8_t *dst, uint32_t uiWidth, uint32_t uiHeight, VTFImageFormat SourceFormat )
//
// Converts data from the source format to RGBA8888 format. Data is read from *src
// and written to *dst. Width and height are needed to it knows how much data to process
//-----------------------------------------------------------------------------------------------------
vlBool CVTFFile::ConvertToRGBA8888(uint8_t *lpSource, uint8_t *lpDest, const uint32_t uiWidth, const uint32_t uiHeight,
                                   const VTFImageFormat SourceFormat, Diagnostics::CError &error) {
    return Convert(lpSource, lpDest, uiWidth, uiHeight, SourceFormat, IMAGE_FORMAT_RGBA8888, error);
}

//-----------------------------------------------------------------------------------------------------
// Rounds a dimension up to a whole 4x4 compression block, matching the padding ComputeImageSize() accounts for
//-----------------------------------------------------------------------------------------------------
static uint32_t BlockAlign(const uint32_t uiSize) {
    return uiSize < 4 ? 4 : ((uiSize + 3) & ~3u);
}

//-----------------------------------------------------------------------------------------------------
// DecompressDXTn(uint8_t *src, uint8_t *dst, uint32_t uiWidth, uint32_t uiHeight, VTFImageFormat SourceFormat)
//
// Converts data from a block compressed format (DXTn, BC7) to RGBA8888 format. Data is read from *src
// and written to *dst. Width and height are needed to it knows how much data to process
//-----------------------------------------------------------------------------------------------------
vlBool CVTFFile::DecompressDXTn(const uint8_t *src, uint8_t *dst, uint32_t uiWidth, uint32_t uiHeight,
                                VTFImageFormat SourceFormat, Diagnostics::CError &error) {
    vlBool bHDRSource = GetUncompressedFormat(SourceFormat) == IMAGE_FORMAT_RGBA16161616F;

    // block compressed formats work on 4x4 blocks
    // so images whose dimensions are not a multiple of four are stored padded out to whole blocks
    uint32_t uiPaddedWidth = BlockAlign(uiWidth), uiPaddedHeight = BlockAlign(uiHeight);

    if (uiPaddedWidth != uiWidth || uiPaddedHeight != uiHeight) {
        uint32_t uiPixelSize = bHDRSource ? 8 : 4;

        std::vector<uint8_t> Padded(uiPaddedWidth * uiPaddedHeight * uiPixelSize);

        if (!DecompressDXTn(src, Padded.data(), uiPaddedWidth, uiPaddedHeight, SourceFormat, error)) {
            return false;
        }

        for (uint32_t i = 0; i < uiHeight; i++) {
            memcpy(dst + i * uiWidth * uiPixelSize, Padded.data() + i * uiPaddedWidth * uiPixelSize,
                   uiWidth * uiPixelSize);
        }

        return true;
    }

    CMP_Texture srcTexture = {0};
    srcTexture.dwSize = sizeof(srcTexture);
    srcTexture.dwWidth = uiWidth;
    srcTexture.dwHeight = uiHeight;
    srcTexture.dwPitch = 0;
    srcTexture.format = GetCMPFormat(SourceFormat, false);
    srcTexture.dwDataSize = CMP_CalculateBufferSize(&srcTexture);
    srcTexture.pData = (CMP_BYTE *) src;

    CMP_CompressOptions options = {0};
    options.dwSize = sizeof(options);
    options.fquality = 1.0f;
    options.dwnumThreads = 0;
    options.bDXT1UseAlpha = SourceFormat == IMAGE_FORMAT_DXT1_ONEBITALPHA;

    vlBool bHDR = GetUncompressedFormat(SourceFormat) == IMAGE_FORMAT_RGBA16161616F;

    CMP_Texture destTexture = {0};
    destTexture.dwSize = sizeof(destTexture);
    destTexture.dwWidth = uiWidth;
    destTexture.dwHeight = uiHeight;
    destTexture.dwPitch = (bHDR ? 8 : 4) * uiWidth;
    destTexture.format = bHDR ? CMP_FORMAT_RGBA_16F : CMP_FORMAT_RGBA_8888;
    destTexture.dwDataSize = destTexture.dwPitch * uiHeight;
    destTexture.pData = (CMP_BYTE *) dst;

    CMP_ERROR cmp_status = CMP_ConvertTexture(&srcTexture, &destTexture, &options, nullptr);
    if (cmp_status != CMP_OK) {
        VTFError_Set(error, GetCMPErrorString(cmp_status));
        return false;
    }

    return true;
}

//
// ConvertFromRGBA8888()
// Convert input image data (lpSource) to output image data (lpDest) of format DestFormat.
//
vlBool CVTFFile::ConvertFromRGBA8888(uint8_t *lpSource, uint8_t *lpDest, const uint32_t uiWidth,
                                     const uint32_t uiHeight,
                                     const VTFImageFormat DestFormat, Diagnostics::CError &error) {
    return Convert(lpSource, lpDest, uiWidth, uiHeight, IMAGE_FORMAT_RGBA8888, DestFormat, error);
}

//
// CompressDXTn()
// Compress input image data (lpSource) to output image data (lpDest) of format DestFormat
// where DestFormat is a block compressed format (DXTn, BC7).  Uses Compressonator.
//
vlBool CVTFFile::CompressDXTn(const uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight,
                              VTFImageFormat DestFormat, Diagnostics::CError &error) {
    vlBool bHDR = GetUncompressedFormat(DestFormat) == IMAGE_FORMAT_RGBA16161616F;

    // pad images whose dimensions are not a multiple of four out to whole 4x4 blocks
    uint32_t uiPaddedWidth = BlockAlign(uiWidth), uiPaddedHeight = BlockAlign(uiHeight);

    if (uiPaddedWidth != uiWidth || uiPaddedHeight != uiHeight) {
        uint32_t uiPixelSize = bHDR ? 8 : 4;

        std::vector<uint8_t> Padded(uiPaddedWidth * uiPaddedHeight * uiPixelSize);

        for (uint32_t i = 0; i < uiPaddedHeight; i++) {
            const uint8_t *pSourceRow = lpSource + (i < uiHeight ? i : uiHeight - 1) * uiWidth * uiPixelSize;
            uint8_t *pDestRow = Padded.data() + i * uiPaddedWidth * uiPixelSize;

            memcpy(pDestRow, pSourceRow, uiWidth * uiPixelSize);

            for (uint32_t j = uiWidth; j < uiPaddedWidth; j++) {
                memcpy(pDestRow + j * uiPixelSize, pSourceRow + (uiWidth - 1) * uiPixelSize, uiPixelSize);
            }
        }

        return CompressDXTn(Padded.data(), lpDest, uiPaddedWidth, uiPaddedHeight, DestFormat, error);
    }

    CMP_Texture srcTexture = {0};
    srcTexture.dwSize = sizeof(srcTexture);
    srcTexture.dwWidth = uiWidth;
    srcTexture.dwHeight = uiHeight;
    srcTexture.dwPitch = (bHDR ? 8 : 4) * uiWidth;
    srcTexture.format = bHDR ? CMP_FORMAT_RGBA_16F : CMP_FORMAT_RGBA_8888;
    srcTexture.dwDataSize = uiHeight * srcTexture.dwPitch;
    srcTexture.pData = (CMP_BYTE *) lpSource;

    CMP_CompressOptions options = {0};
    options.dwSize = sizeof(options);
    // BC7 at maximum quality is exhaustive and extremely slow
    options.fquality = DestFormat == IMAGE_FORMAT_BC7 ? 0.1f : 1.0f;
    options.dwnumThreads = 0;
    options.bDXT1UseAlpha = DestFormat == IMAGE_FORMAT_DXT1_ONEBITALPHA;
    options.nAlphaThreshold = 128;

    CMP_Texture destTexture = {0};
    destTexture.dwSize = sizeof(destTexture);
    destTexture.dwWidth = uiWidth;
    destTexture.dwHeight = uiHeight;
    destTexture.dwPitch = 0;
    destTexture.format = GetCMPFormat(DestFormat, false);
    destTexture.dwDataSize = CMP_CalculateBufferSize(&destTexture);
    destTexture.pData = (CMP_BYTE *) lpDest;

    CMP_ERROR cmp_status = CMP_ConvertTexture(&srcTexture, &destTexture, &options, nullptr);
    if (cmp_status != CMP_OK) {
        VTFError_Set(error, GetCMPErrorString(cmp_status));
        return false;
    }

    return true;
}

typedef void (*TransformProc)(uint16_t &R, uint16_t &G, uint16_t &B, uint16_t &A);

void ToLuminance(uint16_t &R, uint16_t &G, uint16_t &B, uint16_t &A) {
    R = G = B = (uint16_t) (sLuminanceWeightR * (float) R + sLuminanceWeightG * (float) G + sLuminanceWeightB * (
                                float) B);
}

void FromLuminance(uint16_t &R, uint16_t &G, uint16_t &B, uint16_t &A) {
    B = G = R;
}

void ToBlueScreen(uint16_t &R, uint16_t &G, uint16_t &B, uint16_t &A) {
    if (A == 0x0000) {
        R = uiBlueScreenMaskR;
        G = uiBlueScreenMaskG;
        B = uiBlueScreenMaskB;
    }
    A = 0xffff;
}

void FromBlueScreen(uint16_t &R, uint16_t &G, uint16_t &B, uint16_t &A) {
    if (R == uiBlueScreenMaskR && G == uiBlueScreenMaskG && B == uiBlueScreenMaskB) {
        R = uiBlueScreenClearR;
        G = uiBlueScreenClearG;
        B = uiBlueScreenClearB;
        A = 0x0000;
    } else {
        A = 0xffff;
    }
}

static inline float FP16ToFP32(const uint16_t input) {
    const uint32_t uiF32Bias = 127;
    const uint32_t uiF16Bias = 15;
    const float sMaxFloat16Bits = 65504.0f;

    struct {
        uint16_t uiMantissa: 10;
        uint16_t uiExponent: 5;
        uint16_t uiSign: 1;
    } fp16;
    std::memcpy(&fp16, &input, sizeof(uint16_t));

    if (fp16.uiExponent == 31) {
        if (fp16.uiMantissa == 0) // Check for Infinity
            return sMaxFloat16Bits * ((fp16.uiSign == 1) ? -1.0f : 1.0f);
        else if (fp16.uiMantissa != 0) // Check for NaN
            return 0.0f;
    }

    if (fp16.uiExponent == 0 && fp16.uiMantissa != 0) {
        // Denorm...
        const float sHalfDenorm = 1.0f / float(1 << 14);
        const float sMantissa = float(fp16.uiMantissa) / float(1 << 10);
        const float sSign = fp16.uiSign ? -1.0f : 1.0f;

        return sSign * sMantissa * sHalfDenorm;
    } else {
        const uint32_t uiMantissa = fp16.uiMantissa;
        const uint32_t uiExponent = fp16.uiExponent != 0
                                        ? fp16.uiExponent - uiF16Bias + uiF32Bias
                                        : 0;
        const uint32_t uiSign = fp16.uiSign;

        uint32_t uiBits = (uiMantissa << 13) | (uiExponent << 23) | (uiSign << 31);
        float sValue;
        std::memcpy(&sValue, &uiBits, sizeof(sValue));
        return sValue;
    }
}

// A very very basic Reinhard implementation for
// previewing cubemaps...
// (Feel free to use something better with proper luminance
// and a white point!)
float Reinhard(const float sValue) {
    return sValue / (1.0f + sValue);
}

void ToFP16(uint16_t &R, uint16_t &G, uint16_t &B, uint16_t &A) {
}

uint16_t FP16ToUnorm(const uint16_t uiValue) {
    float sValue = FP16ToFP32(uiValue);

    sValue *= sFP16HDRExposure;
    sValue = Reinhard(sValue);
    sValue *= 65535.0f;
    sValue = std::min(std::max(sValue, 0.0f), 65535.0f);
    return (uint16_t) sValue;
}

void FromFP16(uint16_t &R, uint16_t &G, uint16_t &B, uint16_t &A) {
    R = FP16ToUnorm(R);
    G = FP16ToUnorm(G);
    B = FP16ToUnorm(B);
    A = FP16ToUnorm(A);
}

typedef struct tagSVTFImageConvertInfo {
    uint32_t uiBitsPerPixel; // Format bytes per pixel.
    uint32_t uiBytesPerPixel; // Format bytes per pixel.
    uint32_t uiRBitsPerPixel; // Format conversion red bits per pixel.  0 for N/A.
    uint32_t uiGBitsPerPixel; // Format conversion green bits per pixel.  0 for N/A.
    uint32_t uiBBitsPerPixel; // Format conversion blue bits per pixel.  0 for N/A.
    uint32_t uiABitsPerPixel; // Format conversion alpha bits per pixel.  0 for N/A.
    int32_t iR; // "Red" index.
    int32_t iG; // "Green" index.
    int32_t iB; // "Blue" index.
    int32_t iA; // "Alpha" index.
    vlBool bIsCompressed; // Format is compressed (DXT).
    vlBool bIsSupported; // Format is supported by VTFLib.
    TransformProc pToTransform; // Custom transform to function.
    TransformProc pFromTransform; // Custom transform from function.
    VTFImageFormat Format;
} SVTFImageConvertInfo;

static SVTFImageConvertInfo VTFImageConvertInfo[] =
{
    {32, 4, 8, 8, 8, 8, 0, 1, 2, 3, false, true, nullptr, nullptr, IMAGE_FORMAT_RGBA8888},
    {32, 4, 8, 8, 8, 8, 3, 2, 1, 0, false, true, nullptr, nullptr, IMAGE_FORMAT_ABGR8888},
    {24, 3, 8, 8, 8, 0, 0, 1, 2, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_RGB888},
    {24, 3, 8, 8, 8, 0, 2, 1, 0, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_BGR888},
    {16, 2, 5, 6, 5, 0, 0, 1, 2, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_RGB565},
    {8, 1, 8, 8, 8, 0, 0, -1, -1, -1, false, true, ToLuminance, FromLuminance, IMAGE_FORMAT_I8},
    {16, 2, 8, 8, 8, 8, 0, -1, -1, 1, false, true, ToLuminance, FromLuminance, IMAGE_FORMAT_IA88},
    {8, 1, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_P8},
    {8, 1, 0, 0, 0, 8, -1, -1, -1, 0, false, true, nullptr, nullptr, IMAGE_FORMAT_A8},
    {24, 3, 8, 8, 8, 8, 0, 1, 2, -1, false, true, ToBlueScreen, FromBlueScreen, IMAGE_FORMAT_RGB888_BLUESCREEN},
    {24, 3, 8, 8, 8, 8, 2, 1, 0, -1, false, true, ToBlueScreen, FromBlueScreen, IMAGE_FORMAT_BGR888_BLUESCREEN},
    {32, 4, 8, 8, 8, 8, 3, 0, 1, 2, false, true, nullptr, nullptr, IMAGE_FORMAT_ARGB8888},
    {32, 4, 8, 8, 8, 8, 2, 1, 0, 3, false, true, nullptr, nullptr, IMAGE_FORMAT_BGRA8888},
    {4, 0, 0, 0, 0, 0, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_DXT1},
    {8, 0, 0, 0, 0, 8, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_DXT3},
    {8, 0, 0, 0, 0, 8, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_DXT5},
    {32, 4, 8, 8, 8, 0, 2, 1, 0, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_BGRX8888},
    {16, 2, 5, 6, 5, 0, 2, 1, 0, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_BGR565},
    {16, 2, 5, 5, 5, 0, 2, 1, 0, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_BGRX5551},
    {16, 2, 4, 4, 4, 4, 2, 1, 0, 3, false, true, nullptr, nullptr, IMAGE_FORMAT_BGRA4444},
    {4, 0, 0, 0, 0, 1, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_DXT1_ONEBITALPHA},
    {16, 2, 5, 5, 5, 1, 2, 1, 0, 3, false, true, nullptr, nullptr, IMAGE_FORMAT_BGRA5551},
    {16, 2, 8, 8, 0, 0, 0, 1, -1, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_UV88},
    {32, 4, 8, 8, 8, 8, 0, 1, 2, 3, false, true, nullptr, nullptr, IMAGE_FORMAT_UVWQ8888},
    {64, 8, 16, 16, 16, 16, 0, 1, 2, 3, false, true, ToFP16, FromFP16, IMAGE_FORMAT_RGBA16161616F},
    {64, 8, 16, 16, 16, 16, 0, 1, 2, 3, false, true, nullptr, nullptr, IMAGE_FORMAT_RGBA16161616},
    {32, 4, 8, 8, 8, 8, 0, 1, 2, 3, false, true, nullptr, nullptr, IMAGE_FORMAT_UVLX8888},
    {32, 4, 32, 0, 0, 0, 0, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_R32F},
    {96, 12, 32, 32, 32, 0, 0, 1, 2, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_RGB323232F},
    {128, 16, 32, 32, 32, 32, 0, 1, 2, 3, false, false, nullptr, nullptr, IMAGE_FORMAT_RGBA32323232F},
    {16, 2, 16, 0, 0, 0, 0, -1, -1, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_NV_DST16},
    {24, 3, 24, 0, 0, 0, 0, -1, -1, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_NV_DST24},
    {32, 4, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NV_INTZ},
    {24, 3, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NV_RAWZ},
    {16, 2, 16, 0, 0, 0, 0, -1, -1, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_ATI_DST16},
    {24, 3, 24, 0, 0, 0, 0, -1, -1, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_ATI_DST24},
    {32, 4, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NV_NULL},
    {8, 0, 0, 0, 0, 0, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_ATI2N},
    {4, 0, 0, 0, 0, 0, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_ATI1N}/*,
	{	 16,  2, 16,  0,  0,  0,	 0, -1, -1, -1, false,  true,	NULL,	NULL,		IMAGE_FORMAT_X360_DST16},
	{	 24,  3, 24,  0,  0,  0,	 0, -1, -1, -1, false,  true,	NULL,	NULL,		IMAGE_FORMAT_X360_DST24},
	{	 24,  3,  0,  0,  0,  0,	-1, -1, -1, -1, false, false,	NULL,	NULL,		IMAGE_FORMAT_X360_DST24F},
	{ 	 32,  4,  8,  8,  8,  0,	 2,	 1,	 0,	-1, false,  true,	NULL,	NULL,		IMAGE_FORMAT_LINEAR_BGRX8888},
	{	 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	 3,	false,  true,	NULL,	NULL,		IMAGE_FORMAT_LINEAR_RGBA8888},
	{	 32,  4,  8,  8,  8,  8,	 3,	 2,	 1,	 0, false,  true,	NULL,	NULL,		IMAGE_FORMAT_LINEAR_ABGR8888},
	{ 	 32,  4,  8,  8,  8,  8,	 3,	 0,	 1,	 2, false,  true,	NULL,	NULL,		IMAGE_FORMAT_LINEAR_ARGB8888},
	{ 	 32,  4,  8,  8,  8,  8,	 2,	 1,	 0,	 3, false,  true,	NULL,	NULL,		IMAGE_FORMAT_LINEAR_BGRA8888},
	{	 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	-1,	false,  true,	NULL,	NULL,		IMAGE_FORMAT_LINEAR_RGB888},
	{	 32,  4,  8,  8,  8,  8,	 2,	 1,	 0,	-1,	false,  true,	NULL,	NULL,		IMAGE_FORMAT_LINEAR_BGR888},
	{ 	 16,  2,  5,  5,  5,  0,	 2,	 1,	 0,	-1, false,  true,	NULL,	NULL,		IMAGE_FORMAT_LINEAR_BGRX5551},
	{	  8,  1,  8,  8,  8,  0,	 0,	-1,	-1,	-1, false,  true,	ToLuminance,	FromLuminance,	IMAGE_FORMAT_LINEAR_I8},
	{	 64,  8, 16, 16, 16, 16,	 0,	 1,	 2,	 3, false,  true,	NULL,	NULL,		IMAGE_FORMAT_LINEAR_RGBA16161616}*/,
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 39
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 40
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 41
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 42
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 43
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 44
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 45
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 46
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 47
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 48
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 49
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 50
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 51
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 52
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 53
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 54
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 55
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 56
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 57
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 58
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 59
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 60
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 61
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 62
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 63
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 64
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 65
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 66
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 67
    {0, 0, 0, 0, 0, 0, -1, -1, -1, -1, false, false, nullptr, nullptr, IMAGE_FORMAT_NONE}, // 68
    {8, 1, 8, 0, 0, 0, 0, -1, -1, -1, false, true, nullptr, nullptr, IMAGE_FORMAT_R8},
    {8, 0, 0, 0, 0, 8, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_BC7},
    {8, 0, 16, 16, 16, 0, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_BC6H},
    {4, 0, 0, 0, 0, 0, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_BC4},
    {8, 0, 0, 0, 0, 0, -1, -1, -1, -1, true, true, nullptr, nullptr, IMAGE_FORMAT_BC5}
};

// Get each channels shift and mask (for encoding and decoding).
template<typename T>
void GetShiftAndMask(const SVTFImageConvertInfo &Info, T &uiRShift, T &uiGShift, T &uiBShift, T &uiAShift, T &uiRMask,
                     T &uiGMask, T &uiBMask, T &uiAMask) {
    if (Info.iR >= 0) {
        if (Info.iG >= 0 && Info.iG < Info.iR)
            uiRShift += (T) Info.uiGBitsPerPixel;

        if (Info.iB >= 0 && Info.iB < Info.iR)
            uiRShift += (T) Info.uiBBitsPerPixel;

        if (Info.iA >= 0 && Info.iA < Info.iR)
            uiRShift += (T) Info.uiABitsPerPixel;

        uiRMask = (T) (~0) >> (T) ((sizeof(T) * 8) - Info.uiRBitsPerPixel); // Mask is for down shifted values.
    }

    if (Info.iG >= 0) {
        if (Info.iR >= 0 && Info.iR < Info.iG)
            uiGShift += (T) Info.uiRBitsPerPixel;

        if (Info.iB >= 0 && Info.iB < Info.iG)
            uiGShift += (T) Info.uiBBitsPerPixel;

        if (Info.iA >= 0 && Info.iA < Info.iG)
            uiGShift += (T) Info.uiABitsPerPixel;

        uiGMask = (T) (~0) >> (T) ((sizeof(T) * 8) - Info.uiGBitsPerPixel);
    }

    if (Info.iB >= 0) {
        if (Info.iR >= 0 && Info.iR < Info.iB)
            uiBShift += (T) Info.uiRBitsPerPixel;

        if (Info.iG >= 0 && Info.iG < Info.iB)
            uiBShift += (T) Info.uiGBitsPerPixel;

        if (Info.iA >= 0 && Info.iA < Info.iB)
            uiBShift += (T) Info.uiABitsPerPixel;

        uiBMask = (T) (~0) >> (T) ((sizeof(T) * 8) - Info.uiBBitsPerPixel);
    }

    if (Info.iA >= 0) {
        if (Info.iR >= 0 && Info.iR < Info.iA)
            uiAShift += (T) Info.uiRBitsPerPixel;

        if (Info.iG >= 0 && Info.iG < Info.iA)
            uiAShift += (T) Info.uiGBitsPerPixel;

        if (Info.iB >= 0 && Info.iB < Info.iA)
            uiAShift += (T) Info.uiBBitsPerPixel;

        uiAMask = (T) (~0) >> (T) ((sizeof(T) * 8) - Info.uiABitsPerPixel);
    }
}

// Downsample a channel.
template<typename T>
T Shrink(T S, T SourceBits, T DestBits) {
    if (SourceBits == 0 || DestBits == 0)
        return 0;

    return S >> (SourceBits - DestBits);
}

// Upsample a channel.
template<typename T>
T Expand(T S, T SourceBits, T DestBits) {
    if (SourceBits == 0 || DestBits == 0)
        return 0;

    T D = 0;

    // Repeat source bit pattern as much as possible.
    while (DestBits >= SourceBits) {
        D <<= SourceBits;
        D |= S;
        DestBits -= SourceBits;
    }

    // Add most significant part of source bit pattern to least significant part of dest bit pattern.
    if (DestBits) {
        S >>= SourceBits - DestBits;
        D <<= DestBits;
        D |= S;
    }

    return D;
}

// Run custom transformation functions.
template<typename T, typename U>
void Transform(const TransformProc pTransform1, const TransformProc pTransform2, T SR, T SG, T SB, T SA, T SRBits,
               T SGBits,
               T SBBits, T SABits, U &DR, U &DG, U &DB, U &DA, U DRBits, U DGBits, U DBBits, U DABits) {
    uint16_t TR, TG, TB, TA;

    // Expand from source to 16 bits for transform functions.
    SRBits && SRBits < 16 ? TR = (uint16_t) Expand<T>(SR, SRBits, 16) : TR = (uint16_t) SR;
    SGBits && SGBits < 16 ? TG = (uint16_t) Expand<T>(SG, SGBits, 16) : TG = (uint16_t) SG;
    SBBits && SBBits < 16 ? TB = (uint16_t) Expand<T>(SB, SBBits, 16) : TB = (uint16_t) SB;
    SABits && SABits < 16 ? TA = (uint16_t) Expand<T>(SA, SABits, 16) : TA = (uint16_t) SA;

    // Source transform then dest transform.
    if (pTransform1)
        pTransform1(TR, TG, TB, TA);
    if (pTransform2)
        pTransform2(TR, TG, TB, TA);

    // Shrink to dest from 16 bits.
    DRBits && DRBits < 16 ? DR = (U) Shrink<uint16_t>(TR, 16, (uint16_t) DRBits) : DR = (U) TR;
    DGBits && DGBits < 16 ? DG = (U) Shrink<uint16_t>(TG, 16, (uint16_t) DGBits) : DG = (U) TG;
    DBBits && DBBits < 16 ? DB = (U) Shrink<uint16_t>(TB, 16, (uint16_t) DBBits) : DB = (U) TB;
    DABits && DABits < 16 ? DA = (U) Shrink<uint16_t>(TA, 16, (uint16_t) DABits) : DA = (U) TA;
}

// Convert source to dest using required storage requirments (hence the template).
template<typename T, typename U>
vlBool ConvertTemplated(uint8_t *lpSource, uint8_t *lpDest, const uint32_t uiWidth, const uint32_t uiHeight,
                        const SVTFImageConvertInfo &SourceInfo, const SVTFImageConvertInfo &DestInfo) {
    uint16_t uiSourceRShift = 0, uiSourceGShift = 0, uiSourceBShift = 0, uiSourceAShift = 0;
    uint16_t uiSourceRMask = 0, uiSourceGMask = 0, uiSourceBMask = 0, uiSourceAMask = 0;

    uint16_t uiDestRShift = 0, uiDestGShift = 0, uiDestBShift = 0, uiDestAShift = 0;
    uint16_t uiDestRMask = 0, uiDestGMask = 0, uiDestBMask = 0, uiDestAMask = 0;

    GetShiftAndMask<uint16_t>(SourceInfo, uiSourceRShift, uiSourceGShift, uiSourceBShift, uiSourceAShift, uiSourceRMask,
                              uiSourceGMask, uiSourceBMask, uiSourceAMask);
    GetShiftAndMask<uint16_t>(DestInfo, uiDestRShift, uiDestGShift, uiDestBShift, uiDestAShift, uiDestRMask,
                              uiDestGMask, uiDestBMask, uiDestAMask);

    uint8_t *lpSourceEnd = lpSource + (uiWidth * uiHeight * SourceInfo.uiBytesPerPixel);
    for (; lpSource < lpSourceEnd; lpSource += SourceInfo.uiBytesPerPixel, lpDest += DestInfo.uiBytesPerPixel) {
        // read source into single variable
        uint32_t i;
        T Source = 0;
        for (i = 0; i < SourceInfo.uiBytesPerPixel; i++) {
            Source |= (T) lpSource[i] << ((T) i * 8);
        }

        uint16_t SR = 0, SG = 0, SB = 0, SA = ~0;
        uint16_t DR = 0, DG = 0, DB = 0, DA = ~0; // default values

        // read source values
        if (uiSourceRMask)
            SR = (uint16_t) (Source >> (T) uiSourceRShift) & uiSourceRMask; // isolate R channel

        if (uiSourceGMask)
            SG = (uint16_t) (Source >> (T) uiSourceGShift) & uiSourceGMask; // isolate G channel

        if (uiSourceBMask)
            SB = (uint16_t) (Source >> (T) uiSourceBShift) & uiSourceBMask; // isolate B channel

        if (uiSourceAMask)
            SA = (uint16_t) (Source >> (T) uiSourceAShift) & uiSourceAMask; // isolate A channel

        if (SourceInfo.pFromTransform || DestInfo.pToTransform) {
            // transform values
            Transform<uint16_t, uint16_t>(SourceInfo.pFromTransform, DestInfo.pToTransform, SR, SG, SB, SA,
                                          SourceInfo.uiRBitsPerPixel, SourceInfo.uiGBitsPerPixel,
                                          SourceInfo.uiBBitsPerPixel, SourceInfo.uiABitsPerPixel, DR, DG, DB, DA,
                                          DestInfo.uiRBitsPerPixel, DestInfo.uiGBitsPerPixel, DestInfo.uiBBitsPerPixel,
                                          DestInfo.uiABitsPerPixel);
        } else {
            // default value transform
            if (uiSourceRMask && uiDestRMask) {
                if (DestInfo.uiRBitsPerPixel < SourceInfo.uiRBitsPerPixel) // downsample
                    DR = Shrink<uint16_t>(SR, SourceInfo.uiRBitsPerPixel, DestInfo.uiRBitsPerPixel);
                else if (DestInfo.uiRBitsPerPixel > SourceInfo.uiRBitsPerPixel) // upsample
                    DR = Expand<uint16_t>(SR, SourceInfo.uiRBitsPerPixel, DestInfo.uiRBitsPerPixel);
                else
                    DR = SR;
            }

            if (uiSourceGMask && uiDestGMask) {
                if (DestInfo.uiGBitsPerPixel < SourceInfo.uiGBitsPerPixel) // downsample
                    DG = Shrink<uint16_t>(SG, SourceInfo.uiGBitsPerPixel, DestInfo.uiGBitsPerPixel);
                else if (DestInfo.uiGBitsPerPixel > SourceInfo.uiGBitsPerPixel) // upsample
                    DG = Expand<uint16_t>(SG, SourceInfo.uiGBitsPerPixel, DestInfo.uiGBitsPerPixel);
                else
                    DG = SG;
            }

            if (uiSourceBMask && uiDestBMask) {
                if (DestInfo.uiBBitsPerPixel < SourceInfo.uiBBitsPerPixel) // downsample
                    DB = Shrink<uint16_t>(SB, SourceInfo.uiBBitsPerPixel, DestInfo.uiBBitsPerPixel);
                else if (DestInfo.uiBBitsPerPixel > SourceInfo.uiBBitsPerPixel) // upsample
                    DB = Expand<uint16_t>(SB, SourceInfo.uiBBitsPerPixel, DestInfo.uiBBitsPerPixel);
                else
                    DB = SB;
            }

            if (uiSourceAMask && uiDestAMask) {
                if (DestInfo.uiABitsPerPixel < SourceInfo.uiABitsPerPixel) // downsample
                    DA = Shrink<uint16_t>(SA, SourceInfo.uiABitsPerPixel, DestInfo.uiABitsPerPixel);
                else if (DestInfo.uiABitsPerPixel > SourceInfo.uiABitsPerPixel) // upsample
                    DA = Expand<uint16_t>(SA, SourceInfo.uiABitsPerPixel, DestInfo.uiABitsPerPixel);
                else
                    DA = SA;
            }
        }

        // write source to single variable
        U Dest = ((U) (DR & uiDestRMask) << (U) uiDestRShift) | ((U) (DG & uiDestGMask) << (U) uiDestGShift) | (
                     (U) (DB & uiDestBMask) << (U) uiDestBShift) | ((U) (DA & uiDestAMask) << (U) uiDestAShift);
        for (i = 0; i < DestInfo.uiBytesPerPixel; i++) {
            lpDest[i] = (uint8_t) ((Dest >> ((T) i * 8)) & 0xff);
        }
    }

    return true;
}

vlBool CVTFFile::Convert(uint8_t *lpSource, uint8_t *lpDest, const uint32_t uiWidth, const uint32_t uiHeight,
                         const VTFImageFormat SourceFormat,
                         const VTFImageFormat DestFormat, Diagnostics::CError &error) {
    assert(lpSource != 0);
    assert(lpDest != 0);

    assert(SourceFormat >= 0 && SourceFormat < IMAGE_FORMAT_COUNT);
    assert(DestFormat >= 0 && DestFormat < IMAGE_FORMAT_COUNT);

    const SVTFImageConvertInfo &SourceInfo = VTFImageConvertInfo[SourceFormat];
    const SVTFImageConvertInfo &DestInfo = VTFImageConvertInfo[DestFormat];

    if (!SourceInfo.bIsSupported || !DestInfo.bIsSupported) {
        VTFError_Set(error, "Image format conversion not supported.");

        return false;
    }

    // Optimize common convertions.
    if (SourceFormat == DestFormat) {
        memcpy(lpDest, lpSource, ComputeImageSize(uiWidth, uiHeight, 1, DestFormat));
        return true;
    }

    if (SourceFormat == IMAGE_FORMAT_RGB888 && DestFormat == IMAGE_FORMAT_RGBA8888) {
        const uint8_t *lpLast = lpSource + ComputeImageSize(uiWidth, uiHeight, 1, SourceFormat);
        for (; lpSource < lpLast; lpSource += 3, lpDest += 4) {
            lpDest[0] = lpSource[0];
            lpDest[1] = lpSource[1];
            lpDest[2] = lpSource[2];
            lpDest[3] = 255;
        }
        return true;
    }

    if (SourceFormat == IMAGE_FORMAT_RGBA8888 && DestFormat == IMAGE_FORMAT_RGB888) {
        const uint8_t *lpLast = lpSource + ComputeImageSize(uiWidth, uiHeight, 1, SourceFormat);
        for (; lpSource < lpLast; lpSource += 4, lpDest += 3) {
            lpDest[0] = lpSource[0];
            lpDest[1] = lpSource[1];
            lpDest[2] = lpSource[2];
        }
        return true;
    }

    // Do general convertions.
    if (SourceInfo.bIsCompressed || DestInfo.bIsCompressed) {
        VTFImageFormat SourceIntermediateFormat = SourceInfo.bIsCompressed
                                                      ? GetUncompressedFormat(SourceFormat)
                                                      : SourceFormat;
        VTFImageFormat DestIntermediateFormat = DestInfo.bIsCompressed ? GetUncompressedFormat(DestFormat) : DestFormat;

        uint8_t *lpSourceIntermediate = lpSource;
        uint8_t *lpDestIntermediate = lpSource;
        vlBool bResult = true;

        // decompress the source
        if (SourceInfo.bIsCompressed) {
            lpSourceIntermediate = new uint8_t[
                ComputeImageSize(uiWidth, uiHeight, 1, SourceIntermediateFormat)];
            lpDestIntermediate = lpSourceIntermediate;

            bResult = DecompressDXTn(lpSource, lpSourceIntermediate, uiWidth, uiHeight, SourceFormat, error);
        }

        if (bResult) {
            if (DestInfo.bIsCompressed) {
                // get the source into the uncompressed format the destination codec expects
                if (SourceIntermediateFormat != DestIntermediateFormat) {
                    lpDestIntermediate = new uint8_t[ComputeImageSize(
                        uiWidth, uiHeight, 1, DestIntermediateFormat)];

                    bResult = Convert(lpSourceIntermediate, lpDestIntermediate, uiWidth, uiHeight,
                                      SourceIntermediateFormat, DestIntermediateFormat, error);
                }

                if (bResult) {
                    bResult = CompressDXTn(lpDestIntermediate, lpDest, uiWidth, uiHeight, DestFormat, error);
                }
            } else {
                bResult = Convert(lpSourceIntermediate, lpDest, uiWidth, uiHeight, SourceIntermediateFormat,
                                  DestFormat, error);
            }
        }

        // free temp data
        if (lpDestIntermediate != lpSourceIntermediate) {
            delete[] lpDestIntermediate;
        }

        if (lpSourceIntermediate != lpSource) {
            delete[] lpSourceIntermediate;
        }

        return bResult;
    } else {
        // convert from one variable order and bit format to another
        if (SourceInfo.uiBytesPerPixel <= 1) {
            if (DestInfo.uiBytesPerPixel <= 1)
                return ConvertTemplated<uint8_t, uint8_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 2)
                return ConvertTemplated<uint8_t, uint16_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 4)
                return ConvertTemplated<uint8_t, uint32_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 8)
                return ConvertTemplated<uint8_t, uint64_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
        } else if (SourceInfo.uiBytesPerPixel <= 2) {
            if (DestInfo.uiBytesPerPixel <= 1)
                return ConvertTemplated<uint16_t, uint8_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 2)
                return ConvertTemplated<uint16_t, uint16_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 4)
                return ConvertTemplated<uint16_t, uint32_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 8)
                return ConvertTemplated<uint16_t, uint64_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
        } else if (SourceInfo.uiBytesPerPixel <= 4) {
            if (DestInfo.uiBytesPerPixel <= 1)
                return ConvertTemplated<uint32_t, uint8_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 2)
                return ConvertTemplated<uint32_t, uint16_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 4)
                return ConvertTemplated<uint32_t, uint32_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 8)
                return ConvertTemplated<uint32_t, uint64_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
        } else if (SourceInfo.uiBytesPerPixel <= 8) {
            if (DestInfo.uiBytesPerPixel <= 1)
                return ConvertTemplated<uint64_t, uint8_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 2)
                return ConvertTemplated<uint64_t, uint16_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 4)
                return ConvertTemplated<uint64_t, uint32_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
            else if (DestInfo.uiBytesPerPixel <= 8)
                return ConvertTemplated<uint64_t, uint64_t>(lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo);
        }
        return false;
    }

    return false;
}

// Based on https://github.com/Source-SDK-Archives/source-sdk-2004/blob/master/src_mod/public/imageloader.cpp#L1415
static void GenerateNiceFilter(const uint32_t uiWidthRatio, const uint32_t uiHeightRatio, const uint32_t uiDiameter,
                               float *pKernel) {
    uint32_t uiKernelWidth = uiDiameter * uiWidthRatio;
    uint32_t uiKernelHeight = uiDiameter * uiHeightRatio;

    // This is a NICE filter
    // sinc pi*x * a box from -3 to 3 * sinc ( pi * x/3)
    // where x is the pixel # in the destination (shrunken) image.
    // only problem here is that the NICE filter has a very large kernel
    // (7x7 x wratio x hratio)
    float sDX = 1.0f / (float) uiWidthRatio;
    float sDY = 1.0f / (float) uiHeightRatio;

    float sTotal = 0.0f;
    float sY = -((float) uiDiameter - sDY) * 0.5f;

    for (uint32_t i = 0; i < uiKernelHeight; i++) {
        float sX = -((float) uiDiameter - sDX) * 0.5f;

        for (uint32_t j = 0; j < uiKernelWidth; j++) {
            float sValue;
            float sD = (float) sqrt(sX * sX + sY * sY);

            if (sD > (float) uiDiameter * 0.5f) {
                sValue = 0.0f;
            } else {
                float sT = (float) PI * sD;

                if (sT != 0.0f) {
                    sValue = ((float) sin(sT) / sT) * (3.0f * (float) sin(sT / 3.0f) / sT);
                } else {
                    sValue = 1.0f;
                }

                sTotal += sValue;
            }

            pKernel[i * uiKernelWidth + j] = sValue;
            sX += sDX;
        }

        sY += sDY;
    }

    // normalize
    if (sTotal != 0.0f) {
        for (uint32_t i = 0; i < uiKernelWidth * uiKernelHeight; i++) {
            pKernel[i] /= sTotal;
        }
    }
}

static vlBool ResizeNice(const uint8_t *lpSourceRGBA8888, uint8_t *lpDestRGBA8888, const uint32_t uiSourceWidth,
                         const uint32_t uiSourceHeight,
                         const uint32_t uiDestWidth, const uint32_t uiDestHeight, const vlBool bSRGB) {
    const uint32_t uiDiameter = 6;

    uint32_t uiWidthRatio = uiSourceWidth / uiDestWidth;
    uint32_t uiHeightRatio = uiSourceHeight / uiDestHeight;

    uint32_t uiKernelWidth = uiDiameter * uiWidthRatio;
    uint32_t uiKernelHeight = uiDiameter * uiHeightRatio;

    std::vector<float> Kernel(uiKernelWidth * uiKernelHeight);
    GenerateNiceFilter(uiWidthRatio, uiHeightRatio, uiDiameter, Kernel.data());

    // Compute gamma tables...
    float sToLinear[256], sFromLinear[4096];

    for (uint32_t i = 0; i < 256; i++) {
        float s = (float) i / 255.0f;
        sToLinear[i] = bSRGB ? (float) pow(s, 2.2f) : s;
    }

    for (uint32_t i = 0; i < 4096; i++) {
        float s = (float) i / 4095.0f;
        sFromLinear[i] = bSRGB ? (float) pow(s, 1.0f / 2.2f) : s;
    }

    // centered kernel
    int32_t iOffsetX = ((int32_t) uiWidthRatio - (int32_t) uiKernelWidth) / 2;
    int32_t iOffsetY = ((int32_t) uiHeightRatio - (int32_t) uiKernelHeight) / 2;

    for (uint32_t y = 0; y < uiDestHeight; y++) {
        for (uint32_t x = 0; x < uiDestWidth; x++) {
            float sAccum[4] = {0.0f, 0.0f, 0.0f, 0.0f};

            for (uint32_t i = 0; i < uiKernelHeight; i++) {
                int32_t iSourceY = (int32_t) (y * uiHeightRatio) + iOffsetY + (int32_t) i;
                iSourceY = iSourceY < 0
                               ? 0
                               : (iSourceY > (int32_t) uiSourceHeight - 1 ? (int32_t) uiSourceHeight - 1 : iSourceY);

                for (uint32_t j = 0; j < uiKernelWidth; j++) {
                    float sWeight = Kernel[i * uiKernelWidth + j];

                    if (sWeight == 0.0f) {
                        continue;
                    }

                    int32_t iSourceX = (int32_t) (x * uiWidthRatio) + iOffsetX + (int32_t) j;
                    iSourceX = iSourceX < 0
                                   ? 0
                                   : (iSourceX > (int32_t) uiSourceWidth - 1 ? (int32_t) uiSourceWidth - 1 : iSourceX);

                    const uint8_t *lpPixel =
                            lpSourceRGBA8888 + ((uint32_t) iSourceY * uiSourceWidth + (uint32_t) iSourceX) *
                            4;

                    sAccum[0] += sWeight * sToLinear[lpPixel[0]];
                    sAccum[1] += sWeight * sToLinear[lpPixel[1]];
                    sAccum[2] += sWeight * sToLinear[lpPixel[2]];
                    sAccum[3] += sWeight * (float) lpPixel[3] / 255.0f;
                }
            }

            uint8_t *lpDest = lpDestRGBA8888 + (y * uiDestWidth + x) * 4;

            for (uint32_t c = 0; c < 3; c++) {
                float s = sAccum[c] < 0.0f ? 0.0f : (sAccum[c] > 1.0f ? 1.0f : sAccum[c]);
                lpDest[c] = (uint8_t) (sFromLinear[(uint32_t) (s * 4095.0f + 0.5f)] * 255.0f + 0.5f);
            }

            float sAlpha = sAccum[3] < 0.0f ? 0.0f : (sAccum[3] > 1.0f ? 1.0f : sAccum[3]);
            lpDest[3] = (uint8_t) (sAlpha * 255.0f + 0.5f);
        }
    }

    return true;
}

vlBool CVTFFile::Resize(const uint8_t *sourceRGBA8888, uint8_t *destRGBA8888,
                        const uint32_t sourceWidth, const uint32_t sourceHeight,
                        const uint32_t destWidth, const uint32_t destHeight,
                        const VTFMipmapFilter resizeFilter, const vlBool sRGB, Diagnostics::CError &error) {
    assert(ResizeFilter >= 0 && ResizeFilter < MIPMAP_FILTER_COUNT);

    // prevent too large of a kernel
    constexpr uint32_t maxNiceRatio = 64;

    if (resizeFilter == MIPMAP_FILTER_NICE &&
        destWidth > 0 && destHeight > 0 &&
        // The NICE filter only handles integer ratio downsamples
        sourceWidth % destWidth == 0 && sourceHeight % destHeight == 0 &&
        sourceWidth / destWidth <= maxNiceRatio && sourceHeight / destHeight <= maxNiceRatio &&
        !(sourceWidth == destWidth && sourceHeight == destHeight)) {
        return ResizeNice(sourceRGBA8888, destRGBA8888, sourceWidth, sourceHeight, destWidth, destHeight,
                          sRGB);
    }

    if (!stbir_resize_uint8_generic(
        sourceRGBA8888, sourceWidth, sourceHeight, 0,
        destRGBA8888, destWidth, destHeight, 0,
        4, 3, 0, STBIR_EDGE_CLAMP, STBIR_FILTER_BOX, sRGB ? STBIR_COLORSPACE_SRGB : STBIR_COLORSPACE_LINEAR,
        nullptr)) {
        VTFError_Set(error, "Error resizing image.");
        return false;
    }

    return true;
}

//
// CorrectImageGamma()
// Do gamma correction on the image data.
//
void CVTFFile::CorrectImageGamma(uint8_t *imageDataRGBA8888, const uint32_t width, const uint32_t height,
                                 float gammaCorrection) {
    if (gammaCorrection == 1.0f) {
        return;
    }

    uint8_t gammaTable[256];

    gammaCorrection = 1.0f / gammaCorrection;

    // Precalculate all possible gamma correction values.
    for (uint32_t i = 0; i < 256; i++) {
        gammaTable[i] = static_cast<uint8_t>(powf(static_cast<float>(i) / 255.0f, gammaCorrection) * 255.0f);
    }

    uint8_t *lpImageDataRGBA8888End = imageDataRGBA8888 + width * height * 4;

    // Do gamma correction on RGB channels.
    for (; imageDataRGBA8888 < lpImageDataRGBA8888End; imageDataRGBA8888 += 4) {
        imageDataRGBA8888[0] = gammaTable[imageDataRGBA8888[0]];
        imageDataRGBA8888[1] = gammaTable[imageDataRGBA8888[1]];
        imageDataRGBA8888[2] = gammaTable[imageDataRGBA8888[2]];
    }
}

//
// ComputeImageReflectivity()
// Compute the image data reflectivity value.
//
void CVTFFile::ComputeImageReflectivity(const uint8_t *lpImageDataRGBA8888, const uint32_t width,
                                        const uint32_t uiHeight,
                                        float &sX,
                                        float &sY, float &sZ) {
    sX = sY = sZ = 0.0f;

    float gammaTable[256];

    //
    // Precalculate all possible reflectivity values.
    //

    for (uint32_t i = 0; i < 256; i++) {
        gammaTable[i] = powf(static_cast<float>(i) / 255.0f, 2.2f);
    }

    //
    // Compute reflectivity on RGB channels.
    //

    // This is the method Valve uses.

    /*uint8_t *lpImageDataRGBA8888End = lpImageDataRGBA8888 + uiWidth * uiHeight * 4;

    for(; lpImageDataRGBA8888 < lpImageDataRGBA8888End; lpImageDataRGBA8888 += 4)
    {
        sX += sTable[lpImageDataRGBA8888[0]];
        sY += sTable[lpImageDataRGBA8888[1]];
        sZ += sTable[lpImageDataRGBA8888[2]];
    }

    float sInverse = 1.0f / (float)(uiWidth * uiHeight);

    sX *= sInverse;
    sY *= sInverse;
    sZ *= sInverse;*/

    // This method is better on floating point limitations for large images then the above.

    float sTempX, sTempY, sTempZ, sInverse;

    for (uint32_t j = 0; j < uiHeight; j++) {
        sTempX = sTempY = sTempZ = 0.0f;

        for (uint32_t i = 0; i < width; i++) {
            const uint32_t index = (i + j * width) * 4;

            sTempX += gammaTable[lpImageDataRGBA8888[index + 0]];
            sTempY += gammaTable[lpImageDataRGBA8888[index + 1]];
            sTempZ += gammaTable[lpImageDataRGBA8888[index + 2]];
        }

        sInverse = 1.0f / static_cast<float>(width);

        sX += sTempX * sInverse;
        sY += sTempY * sInverse;
        sZ += sTempZ * sInverse;
    }

    sInverse = 1.0f / static_cast<float>(uiHeight);

    sX *= sInverse;
    sY *= sInverse;
    sZ *= sInverse;
}

//
// FlipImage()
// Flips image data over the X axis.
//
void CVTFFile::FlipImage(uint8_t *imageDataRGBA8888, const uint32_t width, const uint32_t height) {
    auto *imageData = reinterpret_cast<uint32_t *>(imageDataRGBA8888);

    for (uint32_t i = 0; i < width; i++) {
        for (uint32_t j = 0; j < height / 2; j++) {
            uint32_t *pOne = imageData + (i + j * width);
            uint32_t *pTwo = imageData + (i + (height - j - 1) * width);

            uint32_t uiTemp = *pOne;
            *pOne = *pTwo;
            *pTwo = uiTemp;
        }
    }
}

//
// MirrorImage()
// Flips image data over the Y axis.
//
void CVTFFile::MirrorImage(uint8_t *imageDataRGBA8888, const uint32_t uiWidth, const uint32_t uiHeight) {
    auto *imageData = reinterpret_cast<uint32_t *>(imageDataRGBA8888);

    for (uint32_t i = 0; i < uiWidth / 2; i++) {
        for (uint32_t j = 0; j < uiHeight; j++) {
            uint32_t *pOne = imageData + (i + j * uiWidth);
            uint32_t *pTwo = imageData + ((uiWidth - i - 1) + j * uiWidth);

            const uint32_t temp = *pOne;
            *pOne = *pTwo;
            *pTwo = temp;
        }
    }
}
