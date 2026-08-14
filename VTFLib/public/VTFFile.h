/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

// ============================================================
// NOTE: This file is commented for compatibility with Doxygen.
// ============================================================
/*!
	\file VTFFile.h
	\brief Header file for the VTFFile class.
*/

#pragma once

#include "VTFLibShared.h"
#include "Readers.h"
#include "Writers.h"
#include "VTFFormat.h"


//! VTFImageFormat info struct.
/*!  
	The SVTFImageFormatInfo struct provides information on VTF image formats.

	\see VTFImageFormat
*/
#pragma pack(1)
typedef struct tagSVTFImageFormatInfo {
    const char *lpName; //!< Enumeration text equivalent.
    uint32_t uiBitsPerPixel; //!< Format bits per pixel.
    uint32_t uiBytesPerPixel; //!< Format bytes per pixel.
    uint32_t uiRedBitsPerPixel; //!< Format red bits per pixel.  0 for N/A.
    uint32_t uiGreenBitsPerPixel; //!< Format green bits per pixel.  0 for N/A.
    uint32_t uiBlueBitsPerPixel; //!< Format blue bits per pixel.  0 for N/A.
    uint32_t uiAlphaBitsPerPixel; //!< Format alpha bits per pixel.  0 for N/A.
    vlBool bIsCompressed; //!< Format is compressed (DXT).
    vlBool bIsSupported; //!< Format is supported by VTFLib.
} SVTFImageFormatInfo;
#pragma pack()

//! VTF Creation options struct.
/*!
	The SVTFCreateOptions struct defines options and settings to be used when
	creating VTF images	with methods such as CVTFFile::Create().

	\see CVTFFile::Create()
*/
#pragma pack(1)
typedef struct tagSVTFCreateOptions {
    uint32_t version[2]; //!< Output image version.
    VTFImageFormat imageFormat; //!< Output image output storage format.

    uint32_t flags; //!< Output image header flags.
    uint32_t startFrame; //!< Output image start frame.
    float bumpScale; //!< Output image bump scale.
    float reflectivityColor[3]; //!< Output image reflectivity. (Only used if bReflectivity is false.)

    vlBool mipmaps; //!< Generate MIPmaps. (Space is always allocated.)
    VTFMipmapFilter mipmapFilter; //!< MIP map re-size filter.

    vlBool thumbnail; //!< Generate thumbnail image.
    vlBool reflectivity; //!< Compute image reflectivity.

    vlBool resize; //!< Resize the input image.
    VTFResizeMethod resizeMethod; //!< New size compution method.
    VTFMipmapFilter resizeFilter; //!< Re-size filter.
    uint32_t resizeWidth; //!< New width after re-size if method is RESIZE_SET.
    uint32_t resizeHeight; //!< New height after re-size if method is RESIZE_SET.

    vlBool resizeClamp; //!< Clamp re-size size.
    uint32_t resizeClampWidth; //!< Maximum width to re-size to.
    uint32_t resizeClampHeight; //!< Maximum height to re-size to.

    vlBool gammaCorrection; //!< Gamma correct input image.
    float gammaCorrectionValue; //!< Gamma correction to apply.

    vlBool sphereMap; //!< Generate a sphere map for six faced environment maps.
    vlBool sRGB; //!< Texture is in the SRGB color space.

    int16_t auxCompressionLevel; //!< Auxiliary compression strength; 0 disables it. (v7.6 only.)
    int16_t auxCompressionMethod; //!< A VTFAuxCompressionMethod value. (v7.6 only.)
} SVTFCreateOptions;
#pragma pack()

namespace VTFLib {
    //! VTF File access/creation class.
    /*!
        The CVTFFile class is the component designed for working with VTF
        format image files. Its public functions allow you to load, save
        and create new VTF files plus perform various processes on the
        image data such as the generation of MIP maps or Normal maps.

        VTFFile generally use RGBA8888 format for passing data to and from
        functions. This is 4 bytes per pixel, 8-bits per channel colour or
        in short, uncompressed 32-bit image data. There are functions for
        converting the data to other formats internally, however for image
        creation you are probably sticking best with RGBA8888 for simplicity.

        The majority of functions return a vlBool value. This is simply a
        test as to whether a function has succeeded or failed to execute properly.
        In the case of functions for checking flags, the vlBool indicates
        if the flag is set or unset. Other data types reflect the purpose
        of the function used.
    */
    class VTFLIB_API CVTFFile {
    private:
        /*!
            Compresses the image data into lpAuxCompressedData and builds the matching AXC resource payload.
            Returns false if compression is off or the image cannot be compressed.
            Reuses the cached result unless bForce is set.
        */
        vlBool ComputeAuxCompression(vlBool force, Diagnostics::CError &error);

        /*!
            Throws away any cached compression result.
            Called whenever image data changes.
        */
        void DestroyAuxCompression();

    public:
        CVTFFile(); //!< Default constructor

        //! Create a new VTFFile class as a copy of another.
        /*!
            Creates a new VTFFile class as a copy of an existing one. The copy is not linked
            so any changes to it will not effect the class instance it was derived from.

            \param other is the CVTFFile class you want to copy.
        */
        CVTFFile(const CVTFFile &other);

        //! Create a new VTFFile class as a duplicate of another.
        /*!
            Create a new VTFFile class as a duplicate of an existing one, but convert
            the image data to the specified format.

            \param file is the CVTFFile class you want to copy.
            \param imageFormat the format you want to convert the copied image data to.
            \param error
        */
        CVTFFile(const CVTFFile &file, VTFImageFormat imageFormat, Diagnostics::CError &error);

        ~CVTFFile(); //!< Deconstructor

    public:
        //! Creates a new empty VTF image..
        /*!
            Creates a new empty VTF format image within a the current CVTFFile class.

            \param width is the width in pixels of the main VTF image.
            \param uiHeight is the height in pixels of the main VTF image.
            \param error
            \param uiFrames is the number of frames in the VTF image (default 1).
            \param uiFaces is the number of faces in the VTF image (default 1).
            \param uiSlices is the number of z slices in the VTF image (default 1).
            \param ImageFormat is the storage format of the main VTF image (default RGBA8888).
            \param bThumbnail sets if the VTF image will contain an additional thumbnail (default true).
            \param bMipmaps
            \param bNullImageData sets if the image data should be zero'd out on creation (default false).
            \return true on successful creation, otherwise false.
            \note Animated and static textures have 1 face. Cubemaps have 6, one for each side of the cube.
            \see tagSVTFCreateOptions
        */
        vlBool Create(uint32_t width, uint32_t uiHeight, Diagnostics::CError &error, uint32_t uiFrames = 1,
                      uint32_t uiFaces = 1, uint32_t uiSlices = 1, VTFImageFormat ImageFormat = IMAGE_FORMAT_RGBA8888,
                      vlBool bThumbnail = vlTrue, vlBool bMipmaps = vlTrue, vlBool bNullImageData = vlFalse);

        //! Create a new VTF image from existing data.
        /*!
            Creates a new VTF image using image data already stored in memory. The existing
            image data should be stored in RGBA8888 format.

            \param uiWidth is the width in pixels of the main VTF image.
            \param uiHeight is the height in pixels of the main VTF image.
            \param lpImageDataRGBA8888 is a pointer to the source RGBA8888 data.
            \param VTFCreateOptions contains the options for image creation.
            \param error
            \return true on successful creation, otherwise false.
            \see tagSVTFCreateOptions
        */
        vlBool Create(uint32_t uiWidth, uint32_t uiHeight, uint8_t *lpImageDataRGBA8888,
                      const SVTFCreateOptions &VTFCreateOptions, Diagnostics::CError &error);

        //! Create a new VTF multi-frame or cubemap image from existing data.
        /*!
            Creates a new multi-frame or cubemap VTF image using image data already stored
            in memory. The existing image data should be stored in RGBA8888 format.

            \param uiWidth is the width in pixels of the main VTF image.
            \param uiHeight is the height in pixels of the main VTF image.
            \param uiFrames is the number of frames in the VTF image.
            \param uiFaces is the number of faces in the VTF image.
            \param vlSlices is the number of z slices in the VTF image.
            \param lpImageDataRGBA8888 is an array of pointers to the image data for each frame/face.
            \param VTFCreateOptions contains the options for image creation.
            \param error
            \return true on successful creation, otherwise false.
            \note Animated and static textures have 1 face. Cubemaps have 6, one for each side of the cube.
            \see tagSVTFCreateOptions
        */
        vlBool Create(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiFrames, uint32_t uiFaces, uint32_t vlSlices,
                      uint8_t **lpImageDataRGBA8888, const SVTFCreateOptions &VTFCreateOptions,
                      Diagnostics::CError &error);

        //! Destroys the current VTF image by setting the header, thumbnail and image data to zero.
        void Destroy();

        //! Check if a VTFFile image is loaded or valid.
        /*!
            Checks if a file was loaded or a new image was created correctly by
            checking for the existence of a valid header struct in the VTFFile class.

            \return true if the header is valid, otherwise false.
            \see Create()
        */
        [[nodiscard]] vlBool IsLoaded() const;

        //! Loads a VTF image from the disk.
        /*!
            Loads a VTF image file from the disk into the current VTFFile class. You may choose to
            load just the header only if you want to get info about the file and save memory.

            \param cFileName is the path and filename of the file to load.
            \param error
            \param bHeaderOnly sets whether to load just the VTF header or not (default false).
            \return true on a successful load, otherwise false.
        */
        vlBool Load(const char *cFileName, Diagnostics::CError &error, vlBool bHeaderOnly = vlFalse);

        //! Loads a VTF image from memory.
        /*!
            Loads a VTF image file stored in memory into the current VTFFile class.
            You may choose to load just the header only if you want to get info about the file
            and save memory.

            \param lpData is a pointer to the VTF file in memory.
            \param uiBufferSize is the size of the VTF file in bytes.
            \param error
            \param bHeaderOnly sets whether to load just the VTF header or not (default false).
            \return true on a successful load, otherwise false.
        */
        vlBool Load(const void *lpData, uint32_t uiBufferSize, Diagnostics::CError &error,
                    vlBool bHeaderOnly = vlFalse);

        //! Loads a VTF image using callback functions.
        /*!
            Loads a VTF image file into the current VTFFile class.
            You may choose to load just the header only if you want to get info about the file
            and save memory.

            \param pUserData is a pointer to custom user data.
            \param error
            \param bHeaderOnly sets whether to load just the VTF header or not (default false).
            \return true on a successful load, otherwise false.
        */
        vlBool Load(void *pUserData, Diagnostics::CError &error, vlBool bHeaderOnly = vlFalse);

        //! Save a VTF image from the disk.
        /*!
            Saves a VTF format image file to disk from the current VTFFile class.

            \param cFileName is the path and filename of the file to load.
            \param error
            \return true on successful save, otherwise false.
        */
        vlBool Save(const char *cFileName, Diagnostics::CError &error) const;

        //! Save a VTF image to memory.
        /*!
            Saves a VTF format image file to memory from the current VTFFile class.

            \param buffer is a pointer to save the image to.
            \param bufferSize is the size of the VTF file in bytes.
            \param outSize
            \param error
            \return true on successful save, otherwise false.
        */
        vlBool Save(void *buffer, uint32_t bufferSize, uint32_t &outSize, Diagnostics::CError &error) const;

        //! Save a VTF image using callback functions.
        /*!
            Saves a VTF format image file from the current VTFFile class.

            \param pUserData is a pointer to custom user data.
            \param error
            \return true on successful save, otherwise false.
        */
        vlBool Save(void *pUserData, Diagnostics::CError &error) const;


        //! Check if image data has been loaded.
        /*!
            Check to see if the image buffer has data in it. If a VTF file was loaded
            into the class with the bHeaderOnly option, this will return false.

            \return true if image data is present, otherwise false.
        */
        [[nodiscard]] vlBool GetHasImage() const;

        [[nodiscard]] uint32_t GetMajorVersion() const; //!< Returns the VTF file major version number.
        [[nodiscard]] uint32_t GetMinorVersion() const; //!< Returns the VTF file minor version number.

        //! Changes the version of the loaded VTF file.
        /*!

            Fails if the image cannot be represented in the requested version
            e.g. volume texture below v7.2

            \param uiMajor is the major version to convert to
            \param uiMinor is the minor version to convert to
            \param error
            \return true on success, false on failure.
            \see GetMajorVersion()
            \see GetMinorVersion()
        */
        vlBool SetVersion(uint32_t uiMajor, uint32_t uiMinor, Diagnostics::CError &error);

        uint32_t GetSize(Diagnostics::CError &error) const; //!< Returns the VTF file size in bytes.

        [[nodiscard]] uint32_t GetWidth() const; //!< Returns the width of the image in pixels from the VTF header.
        [[nodiscard]] uint32_t GetHeight() const; //!< Returns the height of the image in pixels from the VTF header.
        [[nodiscard]] uint32_t GetDepth() const; //!< Returns the depth of the image in pixels from the VTF header.

        [[nodiscard]] uint32_t GetFrameCount() const; //!< Returns the frame count from the VTF header.
        [[nodiscard]] uint32_t GetFaceCount() const; //!< Returns the face count from the VTF header.
        [[nodiscard]] uint32_t GetMipmapCount() const; //!< Returns the number of MIP levels in the image from the VTF header.

        [[nodiscard]] uint32_t GetStartFrame() const; //!< Returns the start frame from the VTF header.
        void SetStartFrame(uint32_t uiStartFrame); //!< Sets the start frame in the VTF header.

        [[nodiscard]] uint32_t GetFlags() const; //!< Returns the image flags from the VTF header.
        void SetFlags(uint32_t uiFlags); //!< Sets the image flags in the VTF header.

        //! Check if a specific flag is set in the VTF header.
        /*!
            Checks to see if the given flag is set in the VTF header struct.

            \param ImageFlag is the flag you wish to check for.
            \return true if the flag is set, otherwise false.
        */
        [[nodiscard]] vlBool GetFlag(VTFImageFlag ImageFlag) const;

        //! Set the state of a specific flag in the VTF header.
        /*!
            Set the boolean state of the given flag in the VTF header struct.

            \param ImageFlag is the flag you wish to set.
            \param bState is the state you wish to set for the flag.
        */
        void SetFlag(VTFImageFlag ImageFlag, vlBool bState);

        [[nodiscard]] float GetBumpmapScale() const; //!< Get the bump scale value.

        //! Set the bump scale value.
        /*!
            Sets the bump scale in the VTF header to the given floating point value.
            \param sBumpmapScale is the scale value to set.
        */
        void SetBumpmapScale(float sBumpmapScale);

        //! Get the reflectivity values.
        /*!
            Get the reflectivity value for each vector axis from the VTF header.

            \param sX is the variable to hold the value reflectivity vector.
            \param sY is the variable to hold the value reflectivity vector.
            \param sZ is the variable to hold the value reflectivity vector.
        */
        void GetReflectivity(float &sX, float &sY, float &sZ) const;

        //! Set the reflectivity values.
        /*!
            Set the reflectivity value for each vector axis in the VTF header.

            \param sX, sY, sZ are the values for each reflectivity vector axis.
        */
        void SetReflectivity(float sX, float sY, float sZ);

        [[nodiscard]] VTFImageFormat GetFormat() const; //!< Returns the storage format of the main image data set in the VTF header.

        //! Returns the format the main image data should be decoded as.
        /*!
            Currently identical to GetFormat except for DXT1 images with the one-bit alpha flag	set
            which are stored as plain DXT1 but must be decoded as IMAGE_FORMAT_DXT1_ONEBITALPHA to keep alpha
        */
        [[nodiscard]] VTFImageFormat GetDecodeFormat() const;

        //! Get a pointer to the image data for a specific image.
        /*!
            Returns a pointer to the image data for a given frame, face and MIP level.

            \param uiFrame is the desired frame.
            \param uiFace is the desired face.
            \param uiSlice is the desired z slice.
            \param uiMipmapLevel is the desired MIP level.
            \note Frames start at index 0 for the first frame. Faces start at index 0
            for the first face. Cubemaps have 6 faces, others only 1. MIP levels start
            at index 0 for the largest image moving down in size.
            \see GetFormat()
        */
        [[nodiscard]] uint8_t *GetData(uint32_t uiFrame, uint32_t uiFace, uint32_t uiSlice, uint32_t uiMipmapLevel) const;

        //! Set the image data for a specific image.
        /*!
            Sets the image data for a given frame, face and MIP level. The source image
            data pointed to by lpData must be in the format specified in the VTF header.

            \param uiFrame is the desired frame.
            \param uiFace is the desired face.
            \param uiSlice is the desired z slice.
            \param uiMipmapLevel is the desired MIP level.
            \param lpData is a pointer to the image data.
            \return uint8_t pointer to the image data.
            \note Frames start at index 0 for the first frame. Faces start at index 0
            for the first face. Cubemaps have 6 faces, others only 1. MIP levels start
            at index 0 for the largest image moving down in size.
            \see GetFormat()
        */
        void SetData(uint32_t uiFrame, uint32_t uiFace, uint32_t uiSlice, uint32_t uiMipmapLevel, uint8_t *lpData);

        [[nodiscard]] vlBool GetHasThumbnail() const; //!< Returns if the current VTF image contains a thumbnail version.

        [[nodiscard]] uint32_t GetThumbnailWidth() const; //!< Returns the width in pixels of the current images thumbnail.
        [[nodiscard]] uint32_t GetThumbnailHeight() const; //!< Returns the height in pixels of the current images thumbnail.

        [[nodiscard]] VTFImageFormat GetThumbnailFormat() const; //!< Returns the image format of the current images thumbnail.

        //! Get a pointer to the thumbnail image data for the current image.
        /*!
            Returns a pointer to the thumbnail image data for the current image.
            \return uint8_t pointer to the image data.
            \see GetThumbnailFormat()
        */
        [[nodiscard]] uint8_t *GetThumbnailData() const;

        //! Set the thumbnail image data for the current image.
        /*!
            Sets the thumbnail image data for the current image. The source image
            data pointed to by lpData must be in the format specified for the thumbnail
            in the VTF header.

            \param lpData is a pointer to the image data.
            \see GetThumbnailFormat()
        */
        void SetThumbnailData(uint8_t *lpData);

        [[nodiscard]] vlBool GetSupportsResources() const; //!< Returns true if the current VTF file version supports resources.

        [[nodiscard]] uint32_t GetResourceCount() const; //!< Returns the number of resources contained within the VTF file.
        [[nodiscard]] uint32_t GetResourceType(uint32_t uiIndex) const; //!< Returns the resource type;
        [[nodiscard]] vlBool GetHasResource(uint32_t uiType) const; //!< Returns true if the resource exists.

        //! Get a VTF resource type's data.
        /*!
            Checks to see if the resource type exists and returns it's data.

            \param uiType is the resource type to retrieve.
            \param uiSize is the size of the resource data.
            \param error
            \return a pointer to the resource data buffer if the resource exists.
        */
        void *GetResourceData(uint32_t uiType, uint32_t &uiSize, Diagnostics::CError &error) const;

        //! Set a VTF resource type's data.
        /*!
            Set a resource type's data.  If the type doesn't exist, the resource is
            created.  If uiSize is 0, the resource is deleted.

            \param uiType is the resource type to set.
            \param uiSize is the size of the resource data; if 0 the resource is deleted, otherwise if the resource does not have a data chunk, this must be 4.
            \param lpData is the resource data; if null the resource data is zeroed.
            \param error
            \return a pointer to the resource data buffer if the resource exists or was created.
        */
        void *SetResourceData(uint32_t uiType, uint32_t uiSize, void *lpData, Diagnostics::CError &error);

        //! Returns true if the current VTF supports CPU compression.
        /*!
        *
        */
        [[nodiscard]] vlBool GetSupportsAuxCompression() const;

        //! Returns the CPU compression strength.
        /*!
            \return 0 if uncompressed, -1 for codec default, or 1 (fastest) to 9 (smallest)
        */
        [[nodiscard]] int16_t GetAuxCompressionLevel() const;

        //! Sets the CPU compression strength.
        /*!
            \param sLevel is 0 to disable compression, -1 for the codec default, otherwise 1 to 9.
            \param error
            \return true if the level was set, false if it is out of range or the file version is below v7.6
        */
        vlBool SetAuxCompressionLevel(int16_t sLevel, Diagnostics::CError &error);

        //! Returns the CPU compression method.
        /*!
            \return a VTFAuxCompressionMethod value.
        */
        [[nodiscard]] int16_t GetAuxCompressionMethod() const;

        //! Sets the CPU compression method.
        /*!
            \param sMethod is a VTFAuxCompressionMethod value
            \param error
            \return true if the method was set, false if it is not a supported method or the file version is below v7.6.
        */
        vlBool SetAuxCompressionMethod(int16_t sMethod, Diagnostics::CError &error);

        //! Generate MIP maps from the main image data.
        /*!
            Generates MIP maps for the image down to 1 x 1 pixel using the data in
            MIP level 0 as the source.

            \param error
            \param MipmapFilter is the reduction filter to use (default Box).
            \param bSRGB is whether we are generating mips for color data or not.
            \return true on successful creation, otherwise false.
        */
        vlBool GenerateMipmaps(Diagnostics::CError &error, VTFMipmapFilter MipmapFilter, vlBool bSRGB);

        //! Generate MIP maps from a specific face and frame.
        /*!
            Generates MIP maps for the image down to 1 x 1 pixel using the data in
            the given face and frame as the source.

            \param uiFace is the face index to use.
            \param uiFrame is the frame index to use.
            \param error
            \param MipmapFilter is the reduction filter to use (default Box).
            \param bSRGB is whether we are generating mips for color data or not.
            \note Frames start at index 0 for the first frame. Faces start at index 0
            for the first face. Cubemaps have 6 faces, others only 1.
            \return true on a successful creation, otherwise false.
        */
        vlBool GenerateMipmaps(uint32_t uiFace, uint32_t uiFrame, Diagnostics::CError &error,
                               VTFMipmapFilter MipmapFilter, vlBool bSRGB);

        //! Generate a thumbnail image.
        /*!
            Generates the thumbnail image for the VTF image by copying an existing MIP map
            of the right size and converting it to the required format.

            \return true on a successful creation, otherwise false.
            \see SetThumbnailData()
        */
        vlBool GenerateThumbnail(vlBool bSRGB, Diagnostics::CError &error);

        //! Convert the image to a normal map.
        /*!
            Converts the image to a normal map using the image data in
            MIP level 0 as the source.

            \param error
            \param KernelFilter is the kernel filter to use (default 3x3).
            \param HeightConversionMethod is the method of determining the height data from the source (default average RGB).
            \param NormalAlphaResult is how the alpha channel should be handled post-processing (defaul make 100% opaque).
            \return true on successful creation, otherwise false.
            \note  The options for conversion are the same used in the nVidea NormalMap Photoshop plug-in.
        */
        vlBool GenerateNormalMap(Diagnostics::CError &error, VTFKernelFilter KernelFilter = KERNEL_FILTER_3X3,
                                 VTFHeightConversionMethod HeightConversionMethod =
                                         HEIGHT_CONVERSION_METHOD_AVERAGE_RGB,
                                 VTFNormalAlphaResult NormalAlphaResult = NORMAL_ALPHA_RESULT_WHITE);

        //! Convert image to a normal map from a specific frame.
        /*!
            Converts the image to a normal map using the image data in
            the given frame as the source.

            \param uiFrame is the frame index to use.
            \param error
            \param KernelFilter is the kernel filter to use (default 3x3).
            \param HeightConversionMethod is the method of determining the height data from the source (default average RGB).
            \param NormalAlphaResult is how the alpha channel should be handled post processing (defaul make 100% opaque).
            \return true on sucessful creation, otherwise false.
            \note  The options for conversion are the same used in the nVidea NormalMap Photoshop plug-in.
        */
        vlBool GenerateNormalMap(uint32_t uiFrame, Diagnostics::CError &error,
                                 VTFKernelFilter KernelFilter = KERNEL_FILTER_3X3,
                                 VTFHeightConversionMethod HeightConversionMethod =
                                         HEIGHT_CONVERSION_METHOD_AVERAGE_RGB,
                                 VTFNormalAlphaResult NormalAlphaResult = NORMAL_ALPHA_RESULT_WHITE);

        vlBool GenerateSphereMap(Diagnostics::CError &error);

        //!< Creates a spheremap from using the 6 faces of the image making up its cubemap.

        vlBool ComputeReflectivity(Diagnostics::CError &error);

        //!< Calculates and sets the reflectivity vector values for the VTF image based on the colour averages of each pixel.

        //! Get VTFImageFormat info.
        /*!
            Returns a SImageFormatInfo info struct for the specified VTFImageFormat.

            \param ImageFormat is the format to get info on.
            \return SImageFormatInfo info struct.
        */
        static SVTFImageFormatInfo const &GetImageFormatInfo(VTFImageFormat ImageFormat);

        //! Calculate data buffer size for an image
        /*!
            Returns the total memory needed in bytes for an image uiWidth and uiHeight in size,
            in the specified format. The result includes the memory used by all MIP map levels
            from the largest dimension down to 1 x 1 pixel.

            \param uiWidth is the width in pixels of the largest MIP level.
            \param uiHeight is the height in pixels of the largest MIP level.
            \param uiDepth is the depth in pixels of the largest MIP level.
            \param ImageFormat is the storage format of the image data.
            \return size of the image data in bytes.
        */
        static uint32_t ComputeImageSize(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth,
                                         VTFImageFormat ImageFormat);

        //! Calculate data buffer size for an image with MIP maps
        /*!
            Returns the total memory needed in bytes for an image uiWidth and uiHeight in size,
            in the specified format. The result includes the memory used by the number of MIP
            maps given as uiMipmaps starting with the original width and height.

            \param uiWidth is the width in pixels of the largest MIP level.
            \param uiHeight is the height in pixels of the largest MIP level.
            \param uiDepth is the depth in pixels of the largest MIP level.
            \param uiMipmaps is the number of MIP maps to include in the calculation starting with the largest.
            \param ImageFormat is the storage format of the image data.
            \return size of the image data in bytes.
        */
        static uint32_t ComputeImageSize(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth, uint32_t uiMipmaps,
                                         VTFImageFormat ImageFormat);

        //! Compute the number of MIP maps needed by an image
        /*!
            Returns how many MIP map levels an image of the specified size will need down to
            1 x 1 pixels. The count includes the original source image.

            \param uiWidth is the width in pixels of the original image.
            \param uiHeight is the height in pixels of the original image.
            \param uiDepth is the depth in pixels of the original image.
            \return number of MIP maps needed.
        */
        static uint32_t ComputeMipmapCount(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth);

        //!< Returns how many MIP map levels are required for an image uiWidth and uiHeight in size, down to 1x1 pixel.

        //! Compute the dimensions of a specific MIP level.
        /*!
            Returns the dimensions of a given MIP level where the largest MIP level
            (level 0) is the specified width and height.

            \param uiWidth is the width in pixels of the largest MIP level.
            \param uiHeight is the height in pixels of the largest MIP level.
            \param uiDepth is the depth in pixels of the largest MIP level.
            \param uiMipmapLevel is the MIP level you want the dimensions of.
            \param uiMipmapWidth is the variable to hold the calculated width.
            \param uiMipmapHeight is the variable to hold the calculated height.
            \param uiMipmapDepth is the variable to hold the calculated depth.
        */
        static void ComputeMipmapDimensions(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth,
                                            uint32_t uiMipmapLevel, uint32_t &uiMipmapWidth, uint32_t &uiMipmapHeight,
                                            uint32_t &uiMipmapDepth);

        //! Compute how much memory a specific MIP map level needs.
        /*!
            Computers the total memory needed in bytes for the a specific MIP map level
            of an image of a given width and height stored in the specified image format.

            \param uiWidth is the width in pixels of the source image.
            \param uiHeight is the height in pixels of the source image.
            \param uiDepth is the depth in pixels of the source image.
            \param uiMipmapLevel is the MIP level you want the size of.
            \param ImageFormat is the image format the MIP map image data is stored in.
            \return size of the MIP map image data in bytes.
        */
        static uint32_t ComputeMipmapSize(uint32_t uiWidth, uint32_t uiHeight, uint32_t uiDepth, uint32_t uiMipmapLevel,
                                          VTFImageFormat ImageFormat);

        //! Convert an image to RGBA8888 format.
        /*!
            Converts image data stored in the given format to RGBA8888 format.

            \param lpSource is a pointer to the source image data.
            \param lpDest is a pointer to the buffer for the converted data.
            \param uiWidth is the width of the source image in pixels.
            \param uiHeight is the height of the source image in pixels.
            \param SourceFormat is the image format of the source data.
            \param error
            \return true on sucessful conversion, otherwise false.
        */
        static vlBool ConvertToRGBA8888(uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight,
                                        VTFImageFormat SourceFormat, Diagnostics::CError &error);

        //! Convert an image from RGBA8888 format.
        /*!
            Converts image data stored in RGBA8888 format to the the specified storage format.

            \param lpSource is a pointer to the source image data in RGBA8888 format.
            \param lpDest is a pointer to the buffer for the converted data.
            \param uiWidth is the width of the source image in pixels.
            \param uiHeight is the height of the source image in pixels.
            \param DestFormat is the image format you wish to convert to.
            \param error
            \return true on sucessful conversion, otherwise false.
        */
        static vlBool ConvertFromRGBA8888(uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight,
                                          VTFImageFormat DestFormat, Diagnostics::CError &error);

        //! Convert an image from any format to any format.
        /*!
            Converts image data stored in any format to the the specified storage format.

            \param lpSource is a pointer to the source image data.
            \param lpDest is a pointer to the buffer for the converted data.
            \param uiWidth is the width of the source image in pixels.
            \param uiHeight is the height of the source image in pixels.
            \param SourceFormat is the image format you are converting from.
            \param DestFormat is the image format you wish to convert to.
            \param error
            \return true on sucessful conversion, otherwise false.
        */
        static vlBool Convert(uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight,
                              VTFImageFormat SourceFormat, VTFImageFormat DestFormat, Diagnostics::CError &error);

        //! Re-sizes an image.
        /*!
            Re-sizes an image in RGBA8888 format to the given dimensions using the specified filters.

            \param sourceRGBA8888 is a pointer to the source image data in RGBA8888 format.
            \param destRGBA8888 is a pointer to the buffer for the converted data.
            \param sourceWidth is the width of the source image in pixels.
            \param sourceHeight is the height of the source image in pixels.
            \param destWidth is the width of the destination image in pixels.
            \param destHeight is the height of the destination image in pixels.
            \param resizeFilter is the image reduction filter to use (default triangle).
            \param sRGB is whether we are generating mips for color data or not.
            \param error
            \return true on sucessful re-size, otherwise false.
        */
        static vlBool Resize(const uint8_t *sourceRGBA8888, uint8_t *destRGBA8888, uint32_t sourceWidth,
                             uint32_t sourceHeight, uint32_t destWidth, uint32_t destHeight,
                             VTFMipmapFilter resizeFilter, vlBool sRGB, Diagnostics::CError &error);

        //! Converts an image's alpha channel into a signed distance field.
        /*!

            \param lpSourceRGBA8888 is a pointer to the source image data in RGBA8888 format.
            \param lpDestRGBA8888 is a pointer to the buffer for the converted data.
            \param uiSourceWidth is the width of the source image in pixels.
            \param uiSourceHeight is the height of the source image in pixels.
            \param uiDestWidth is the width of the destination image in pixels.
            \param uiDestHeight is the height of the destination image in pixels.
            \param sSpread is the width in destination pixels of the gradient either side of the boundary.
            \param bThreshold is the source alpha above which a pixel is inside the shape.
            \param pbClipped optionally receives true if the field ran off the edge of the image and had to be clipped, which loses information.
            \param error
            \return true on sucessful conversion, otherwise false.
        */
        static vlBool ConvertToDistanceField(const uint8_t *lpSourceRGBA8888, uint8_t *lpDestRGBA8888,
                                             uint32_t uiSourceWidth, uint32_t uiSourceHeight, uint32_t uiDestWidth,
                                             uint32_t uiDestHeight, float sSpread, uint8_t bThreshold,
                                             vlBool *pbClipped, Diagnostics::CError &error);


        //! Correct and images gamma.
        /*!
            Applies gamma correction to an image.

            \param imageDataRGBA8888 is a pointer to the image data in RGBA8888 format.
            \param width is the width of the source image in pixels.
            \param height is the height of the source image in pixels.
            \param gammaCorrection is the amount of gamma correction to apply.
        */
        static void CorrectImageGamma(uint8_t *imageDataRGBA8888, uint32_t width, uint32_t height,
                                      float gammaCorrection);

        //! Computes the reflectivity for an image.
        /*!
            Calculates and sets the reflectivity vector values for the VTF image based on the
            colour averages of each pixel.

            \param lpImageDataRGBA8888 is a pointer to the image data in RGBA8888 format.
            \param width is the width of the source image in pixels.
            \param uiHeight is the height of the source image in pixels.
            \param sX, sY, sZ are the variables to hold the values reflectivity vector.
            \see ComputeReflectivity()
            \see GetReflectivity()
            \see SetReflectivity()
        */
        static void ComputeImageReflectivity(const uint8_t *lpImageDataRGBA8888, uint32_t width, uint32_t uiHeight,
                                             float &sX, float &sY, float &sZ);

        static void FlipImage(uint8_t *imageDataRGBA8888, uint32_t width, uint32_t height);

        //!< Flips an image vertically along its X-axis.
        static void MirrorImage(uint8_t *imageDataRGBA8888, uint32_t uiWidth, uint32_t uiHeight);

        //!< Flips an image horizontally along its Y-axis.

    private:
        vlBool IsPowerOfTwo(uint32_t uiSize);

        uint32_t NextPowerOfTwo(uint32_t uiSize);

        uint32_t ComputeResizedDimension(uint32_t uiSize, VTFResizeMethod ResizeMethod);

        //!< Rounds a dimension as dictated by the given re-size method.

        void ComputeResources(); //!< Computes header VTF directory resources.

        // Interface with out reader/writer classes
        vlBool Load(IO::Readers::IReader *Reader, vlBool bHeaderOnly, Diagnostics::CError &error);

        vlBool Save(IO::Writers::IWriter *Writer, Diagnostics::CError &error) const;

        // Calculates where in the VTF image the data begins
        [[nodiscard]] uint32_t ComputeDataOffset(uint32_t uiFrame, uint32_t uiFace, uint32_t uiSlice, uint32_t uiMipmapLevel,
                                   VTFImageFormat ImageFormat) const;

        // DXTn format decompression function
        static vlBool DecompressDXTn(const uint8_t *src, uint8_t *dst, uint32_t uiWidth, uint32_t uiHeight,
                                     VTFImageFormat SourceFormat, Diagnostics::CError &
                                     error);

        // DXTn format compression function
        static vlBool CompressDXTn(const uint8_t *lpSource, uint8_t *lpDest, uint32_t uiWidth, uint32_t uiHeight,
                                   VTFImageFormat DestFormat, Diagnostics::CError
                                   &error);

        SVTFHeader *mHeader; // VTF header

        uint32_t mImageBufferSize; // Size of VTF image data buffer
        uint8_t *mImageData; // VTF image buffer

        uint32_t mThumbnailBufferSize; // Size of VTF thumbnail image data buffer
        uint8_t *mThumbnailImageData; // VTF thumbnail image buffer

        int16_t mAuxCompressionLevel; // CPU compression strength, 0 if uncompressed
        int16_t mAuxCompressionMethod; // CPU compression method (VTFAuxCompressionMethod)

        uint32_t mAuxCompressedBufferSize;
        uint8_t *mAuxCompressedData;
        uint8_t *mAuxCompressionInfo;
        uint32_t mAuxCompressionInfoSize;
    };
}
