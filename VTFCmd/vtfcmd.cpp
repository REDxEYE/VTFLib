/*
 * VTFCmd
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg
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

#include "stdafx.h"
#include "enumerations.h"
#include "VMTWrapper.h"
#include "VTFWrapper.h"
// #include "IL/il.h"

#include "compressonator.h"

#include "win32_findfile_polyfill.hpp"

#define MAX_ITEMS	1024

struct VTFCmdState {
    uint32_t fileCount{};
    char *files[MAX_ITEMS]{}; // Files to convert.
    uint32_t folderCount{};
    char *folders[MAX_ITEMS]{}; // Folders to convert.
    bool recursive{}; // Recursively search folders.
    uint32_t processed{}; // Files processed.
    uint32_t completed{}; // Files processed without error.

    const char *prefix{}; // String to add to start of output file name.
    const char *postfix{}; // String to add to end of output file name.
    const char *output{}; // Output folder.

    bool silent{}; // Don't display output.
    bool pause{}; // Don't pause the console.
    bool help{}; // Display help.

    uint32_t vtfImage{}; // VTF image handle.
    uint32_t vmtMaterial{}; // VMT material handle.

    VTFImageFormat alphaFormat = IMAGE_FORMAT_DXT5; // VTF image format for alpha textures.
    VTFImageFormat normalFormat = IMAGE_FORMAT_DXT1; // VTF image format for non-alpha textures.

    SVTFCreateOptions createOptions{}; // VTF creation options.
    char *shader = nullptr; // VMT shader to use.
    uint32_t parameterCount = 0;
    char *parameters[MAX_ITEMS][2]{}; // VMT parameters.
    const char *exportFormat{}; // Format extension for exporting VTF images.

    bool distanceAlpha{}; // Encode the alpha channel as a distance field.
    float distanceAlphaSpread{1.0f}; // Width of the distance field gradient in output pixels.
    uint32_t distanceAlphaReduce{1}; // Amount to shrink the image by after computing the field.
    uint8_t distanceAlphaThreshold{10}; // Source alpha above which a pixel is inside the shape.
};

static VTFCmdState g_state{};

void Pause();

void Print(const char *lpFormat, ...);

void PrintUsage(const char *lpError, ...);

void ProcessFile(char *lpInputFile);

void ProcessFolder(char *lpInputFolder, char *lpWildcard);

//
// stristr()
// Case insensitive version of strstr().
//
char *stristr(const char *string, const char *strSearch) {
    const char *ptr = nullptr;

    while (true) {
        ptr = strchr(string, toupper(*strSearch));
        const char *ptr2 = strchr(string, tolower(*strSearch));

        if (ptr == nullptr) {
            ptr = ptr2;
        }
        if (ptr == nullptr) {
            break;
        }
        if (ptr2 && (ptr2 < ptr)) {
            ptr = ptr2;
        }
        if (!strnicmp(ptr, strSearch, strlen(strSearch))) {
            return const_cast<char *>(ptr);
        }

        string = ptr + 1;
    }

    return nullptr;
}

//
// strrpl()
// Replace a char in a string with another.
//
void strrpl(char *string, const char chr, const char replacement) {
    while (*string != 0) {
        if (*string == chr)
            *string = replacement;
        string++;
    }
}

int main(const int argc, char *argv[]) {
    CMP_InitFramework();
    g_state.postfix = "";
    g_state.prefix = "";
    g_state.output = "";
    g_state.exportFormat = "tga";

    char *lpWildcard; // Holds wildcard string for folder searches.

    VTFImageFormat imageFormat; // Temp variable for string to VTFImageFormat test.
    VTFImageFlag imageFlag; // Temp variable for string to VTFImageFlag test.
    VTFResizeMethod resizeMethod; // Temp variable for string to VTFResizeMethod test.
    VTFMipmapFilter mipmapFilter; // Temp variable for string to VTFMipmapFilter test.

    uint32_t uiTemp0, uiTemp1; // Temp variables for string to integer test.
    int32_t iTemp0; // Temp variable for signed string to integer test.
    float sTemp; // Temp variable for string to single test.
    
    winfind::WIN32_FIND_DATA FindData;
    winfind::HANDLE Handle;

    // Check we have the right DLL version.
    if (vlGetVersion() != VL_VERSION) {
        Print("Wrong VTFLib version.\n");
        return 1;
    }

    // Fill in our CreateOptions struct with VTFLib defaults.
    vlImageCreateDefaultCreateStructure(&g_state.createOptions);

    // Grab command arguments.
    switch (argc) {
        case 1:
            // If no arguments assume double click.
            g_state.pause = vlTrue;
            break;
        case 2:
            // If only one argument assume drag and drop.
            Handle = FindFirstFile(argv[1], &FindData);

            if (Handle != INVALID_HANDLE_VALUE) {
                if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    g_state.folders[g_state.folderCount++] = argv[1];
                    g_state.createOptions.resize = vlTrue;
                    g_state.pause = vlTrue;
                } else {
                    g_state.files[g_state.fileCount++] = argv[1];
                    g_state.createOptions.resize = vlTrue;
                    g_state.pause = vlTrue;
                }

                winfind::FindClose(Handle);
                break;
            }
        // Fall through.
        default:
            for (uint32_t i = 1; i < argc; i++) {
                if (stricmp(argv[i], "-file") == 0) {
                    if (i + 1 < argc && g_state.fileCount < MAX_ITEMS) {
                        g_state.files[g_state.fileCount++] = argv[++i];
                    } else {
                        PrintUsage("-file expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-folder") == 0) {
                    if (i + 1 < argc && g_state.folderCount < MAX_ITEMS) {
                        g_state.folders[g_state.folderCount++] = argv[++i];
                    } else {
                        PrintUsage("-folder expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-output") == 0) {
                    if (i + 1 < argc) {
                        g_state.output = argv[++i];
                    } else {
                        PrintUsage("-output expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-prefix") == 0) {
                    if (i + 1 < argc) {
                        g_state.prefix = argv[++i];
                    } else {
                        PrintUsage("-prefix expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-postfix") == 0) {
                    if (i + 1 < argc) {
                        g_state.postfix = argv[++i];
                    } else {
                        PrintUsage("-postfix expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-version") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%u.%u", &uiTemp0, &uiTemp1) == 2) {
                        g_state.createOptions.version[0] = uiTemp0;
                        g_state.createOptions.version[1] = uiTemp1;
                    } else {
                        PrintUsage("-version expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-compress") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%d", &iTemp0) == 1
                        && iTemp0 >= VTF_AUX_COMPRESSION_LEVEL_DEFAULT && iTemp0 <= VTF_AUX_COMPRESSION_LEVEL_MAX) {
                        g_state.createOptions.auxCompressionLevel = (int16_t) iTemp0;
                    } else {
                        PrintUsage("-compress expects an integer argument between %d and %d.",
                                   VTF_AUX_COMPRESSION_LEVEL_DEFAULT, VTF_AUX_COMPRESSION_LEVEL_MAX);
                        return 2;
                    }
                } else if (stricmp(argv[i], "-cmethod") == 0) {
                    if (i + 1 < argc) {
                        i++;
                        if (stricmp(argv[i], "deflate") == 0) {
                            g_state.createOptions.auxCompressionMethod = AUX_COMPRESSION_METHOD_DEFLATE;
                        } else if (stricmp(argv[i], "zstd") == 0) {
                            g_state.createOptions.auxCompressionMethod = AUX_COMPRESSION_METHOD_ZSTD;
                        } else {
                            PrintUsage("Unknown compression method: %s.", argv[i]);
                            return 2;
                        }
                    } else {
                        PrintUsage("-cmethod expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-format") == 0) {
                    if (i + 1 < argc) {
                        imageFormat = StringToImageFormat(argv[++i]);
                        if (imageFormat != IMAGE_FORMAT_COUNT) {
                            g_state.normalFormat = imageFormat;
                        } else {
                            PrintUsage("Unknown format: %s.", argv[i]);
                            return 2;
                        }
                    } else {
                        PrintUsage("-format expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-srgb") == 0) {
                    g_state.createOptions.sRGB = vlTrue;
                    g_state.createOptions.flags |= TEXTUREFLAGS_SRGB;
                } else if (stricmp(argv[i], "-alphaformat") == 0) {
                    if (i + 1 < argc) {
                        imageFormat = StringToImageFormat(argv[++i]);
                        if (imageFormat != IMAGE_FORMAT_COUNT) {
                            g_state.alphaFormat = imageFormat;
                        } else {
                            PrintUsage("Unknown format: %s.", argv[i]);
                            return 2;
                        }
                    } else {
                        PrintUsage("-format expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-exportformat") == 0) {
                    if (i + 1 < argc) {
                        g_state.exportFormat = argv[++i];
                    } else {
                        PrintUsage("-exportformat expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-flag") == 0) {
                    if (i + 1 < argc) {
                        imageFlag = StringToImageFlag(argv[++i]);
                        if (imageFlag != TEXTUREFLAGS_COUNT) {
                            g_state.createOptions.flags |= imageFlag;
                        } else {
                            PrintUsage("Unknown flag: %s.", argv[i]);
                            return 2;
                        }
                    } else {
                        PrintUsage("-flag expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-resize") == 0) {
                    g_state.createOptions.resize = vlTrue;
                } else if (stricmp(argv[i], "-rmethod") == 0) {
                    if (i + 1 < argc) {
                        resizeMethod = StringToResizeMethod(argv[++i]);
                        if (resizeMethod != RESIZE_COUNT) {
                            g_state.createOptions.resizeMethod = resizeMethod;
                        } else {
                            PrintUsage("Unknown rmethod: %s.", argv[i]);
                            return 2;
                        }
                    } else {
                        PrintUsage("-rmethod expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-rfilter") == 0) {
                    if (i + 1 < argc) {
                        mipmapFilter = StringToMipmapFilter(argv[++i]);
                        if (mipmapFilter != MIPMAP_FILTER_COUNT) {
                            g_state.createOptions.resizeFilter = mipmapFilter;
                        } else {
                            PrintUsage("Unknown rfilter: %s.", argv[i]);
                            return 2;
                        }
                    } else {
                        PrintUsage("-rfilter expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-rwidth") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%u", &uiTemp0) == 1) {
                        g_state.createOptions.resizeWidth = uiTemp0;
                        if (g_state.createOptions.resizeWidth != 0 && g_state.createOptions.resizeHeight != 0) {
                            g_state.createOptions.resizeMethod = RESIZE_SET;
                        }
                    } else {
                        PrintUsage("-rwidth expects unsigned integer argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-rheight") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%u", &uiTemp0) == 1) {
                        g_state.createOptions.resizeHeight = uiTemp0;
                        if (g_state.createOptions.resizeWidth != 0 && g_state.createOptions.resizeHeight != 0) {
                            g_state.createOptions.resizeMethod = RESIZE_SET;
                        }
                    } else {
                        PrintUsage("-rheight expects unsigned integer argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-rclampwidth") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%u", &uiTemp0) == 1) {
                        g_state.createOptions.resizeClampWidth = uiTemp0;
                    } else {
                        PrintUsage("-rclampwidth expects unsigned integer argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-rclampheight") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%u", &uiTemp0) == 1) {
                        g_state.createOptions.resizeClampHeight = uiTemp0;
                    } else {
                        PrintUsage("-rclampheight expects unsigned integer argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-gamma") == 0) {
                    g_state.createOptions.gammaCorrection = vlTrue;
                } else if (stricmp(argv[i], "-gcorrection") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%f", &sTemp) == 1) {
                        g_state.createOptions.gammaCorrectionValue = sTemp;
                    } else {
                        PrintUsage("-gcorrection expects single argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-distancealpha") == 0) {
                    g_state.distanceAlpha = vlTrue;
                } else if (stricmp(argv[i], "-dspread") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%f", &sTemp) == 1 && sTemp > 0.0f) {
                        g_state.distanceAlphaSpread = sTemp;
                    } else {
                        PrintUsage("-dspread expects a positive single argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-dreduce") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%u", &uiTemp0) == 1 && uiTemp0 >= 1) {
                        g_state.distanceAlphaReduce = uiTemp0;
                    } else {
                        PrintUsage("-dreduce expects an unsigned integer argument of 1 or more.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-dthreshold") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%u", &uiTemp0) == 1 && uiTemp0 <= 255) {
                        g_state.distanceAlphaThreshold = (uint8_t) uiTemp0;
                    } else {
                        PrintUsage("-dthreshold expects an unsigned integer argument between 0 and 255.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-nomipmaps") == 0) {
                    g_state.createOptions.mipmaps = vlFalse;
                } else if (stricmp(argv[i], "-mfilter") == 0) {
                    if (i + 1 < argc) {
                        mipmapFilter = StringToMipmapFilter(argv[++i]);
                        if (mipmapFilter != MIPMAP_FILTER_COUNT) {
                            g_state.createOptions.mipmapFilter = mipmapFilter;
                        } else {
                            PrintUsage("Unknown mfilter: %s.", argv[i]);
                            return 2;
                        }
                    } else {
                        PrintUsage("-mfilter expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-bumpscale") == 0) {
                    if (i + 1 < argc && sscanf(argv[++i], "%f", &sTemp) == 1) {
                        g_state.createOptions.bumpScale = sTemp;
                    } else {
                        PrintUsage("-bumpscale expects single argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-nothumbnail") == 0) {
                    g_state.createOptions.thumbnail = vlFalse;
                } else if (stricmp(argv[i], "-noreflectivity") == 0) {
                    g_state.createOptions.reflectivity = vlFalse;
                } else if (stricmp(argv[i], "-shader") == 0) {
                    if (i + 1 < argc) {
                        g_state.shader = argv[++i];
                    } else {
                        PrintUsage("-shader expects string argument.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-param") == 0) {
                    if (i + 2 < argc) {
                        g_state.parameters[g_state.parameterCount][0] = argv[++i];
                        g_state.parameters[g_state.parameterCount][1] = argv[++i];

                        g_state.parameterCount++;
                    } else {
                        PrintUsage("-shader expects two string arguments.");
                        return 2;
                    }
                } else if (stricmp(argv[i], "-recurse") == 0) {
                    g_state.recursive = vlTrue;
                } else if (stricmp(argv[i], "-silent") == 0) {
                    g_state.silent = vlTrue;
                } else if (stricmp(argv[i], "-pause") == 0) {
                    g_state.pause = vlTrue;
                } else if (stricmp(argv[i], "-help") == 0) {
                    g_state.help = vlTrue;
                } else {
                    PrintUsage("Unknown argument: %s.", argv[i]);
                    return 2;
                }
            }
            break;
    }

    // If the user just wants help, give it to them.
    if (g_state.help) {
        PrintUsage(nullptr);
        return 0;
    }

    // Make sure we have something to do.
    if (g_state.fileCount == 0 && g_state.folderCount == 0) {
        PrintUsage("-file or -folder not specified.");
        return 2;
    }

    VTFLib::Diagnostics::CError error;
    // Initialize VTFLib.
    vlInitialize(error);

    if (error.isSet()) {
        Print(error.Get());
        return 2;
    }

    vlCreateImage(&g_state.vtfImage, error);
    if (error.isSet()) {
        Print(error.Get());
        return 2;
    }
    vlBindImage(g_state.vtfImage, error);
    if (error.isSet()) {
        Print(error.Get());
        return 2;
    }

    vlCreateMaterial(&g_state.vmtMaterial, error);
    if (error.isSet()) {
        Print(error.Get());
        return 2;
    }
    vlBindMaterial(g_state.vmtMaterial, error);
    if (error.isSet()) {
        Print(error.Get());
        return 2;
    }

    // Process files.
    for (uint32_t i = 0; i < (int) g_state.fileCount; i++) {
        ProcessFile(g_state.files[i]);
    }

    // Process folders.
    for (uint32_t i = 0; i < (int) g_state.folderCount; i++) {
        // Grab the wildcard string from the folder path.
        if ((lpWildcard = strrchr(g_state.folders[i], '\\')) == nullptr) {
            lpWildcard = "*.*";
        } else {
            // Wildcard starts after last \ in path.  e.g. C:\input\*.bmp
            *lpWildcard = '\0';
            lpWildcard++;

            // If there is no wildcard after the last \, use *.* as defult.
            if (*lpWildcard == '\0') {
                lpWildcard = "*.*";
            }
        }

        ProcessFolder(g_state.folders[i], lpWildcard);
    }

    // Shutdown VTFLib.
    vlDeleteMaterial(g_state.vmtMaterial);

    vlDeleteImage(g_state.vtfImage);

    vlShutdown();

    Print("%d/%d files completed.\n", g_state.completed, g_state.processed);

    // Pause the console.
    Pause();

    return 0;
}

//
// Pause()
// Puase the console.
//
void Pause() {
    if (g_state.pause) {
        Print("Press any key to continue...");
        getchar();
    }
}

//
// Print()
// Wrap printf() so we don't have to keep checking for silent mode.
//
void Print(const char *lpFormat, ...) {
    va_list ArgumentList;

    if (!g_state.silent) {
        va_start(ArgumentList, lpFormat);
        vprintf(lpFormat, ArgumentList);
        va_end(ArgumentList);
    }
}

//
// PrintUsage()
// Print VTFCmd command line usage help string.
//
void PrintUsage(const char *lpError, ...) {
    va_list ArgumentList;

    Print("Correct vtfcmd usage:\n");
    Print(" -file <path>             (Input file path.)\n");
    Print(" -folder <path>           (Input directory search string.)\n");
    Print(" -output <path>           (Output directory.)\n");
    Print(" -prefix <string>         (Output file prefix.)\n");
    Print(" -postfix <string>        (Output file postfix.)\n");
    Print(" -version <string>        (Output version.)\n");
    Print(" -format <string>         (Output format to use on non-alpha (colour) textures.)\n");
    Print(" -compress <integer>      (Compress image data; -1 default, 0 off, 1-9. Requires version 7.6.)\n");
    Print(" -cmethod <string>        (Compression method: deflate or zstd.)\n");
    Print(" -alphaformat <string>    (Output format to use on alpha textures.)\n");
    Print(" -srgb                    (Whether to treat image as sRGB colour space or not)\n");
    Print(" -flag <string>           (Output flags to set.)\n");
    Print(" -resize                  (Resize the input to a power of 2.)\n");
    Print(" -rmethod <string>        (Resize method to use.)\n");
    Print(" -rfilter <string>        (Resize filter to use.)\n");
    Print(" -rwidth <integer>        (Resize to specific width.)\n");
    Print(" -rheight <integer>       (Resize to specific height.)\n");
    Print(" -rclampwidth <integer>   (Maximum width to resize to.)\n");
    Print(" -rclampheight <integer>  (Maximum height to resize to.)\n");
    Print(" -gamma                   (Gamma correct image.)\n");
    Print(" -gcorrection <single>    (Gamma correction to use.)\n");
    Print(" -distancealpha           (Encode the alpha channel as a distance field.)\n");
    Print(" -dspread <single>        (Width of the distance field gradient in output pixels.)\n");
    Print(" -dreduce <integer>       (Shrink the image by this factor after computing the field.)\n");
    Print(" -dthreshold <integer>    (Source alpha above which a pixel is inside the shape.)\n");
    Print(" -nomipmaps               (Don't generate mipmaps.)\n");
    Print(" -mfilter <string>        (Mipmap filter to use.)\n");
    Print(" -bumpscale <single>      (Engine bump mapping scale to use.)\n");
    Print(" -nothumbnail             (Don't generate thumbnail image.)\n");
    Print(" -noreflectivity          (Don't calculate reflectivity.)\n");
    Print(" -shader <string>         (Create a material for the texture.)\n");
    Print(" -param <string> <string> (Add a parameter to the material.)\n");
    Print(" -recurse                 (Process directories recursively.)\n");
    Print(" -exportformat <string>   (Convert VTF files to the format of this extension.)\n");
    Print(" -silent                  (Silent mode.)\n");
    Print(" -pause                   (Pause when done.)\n");
    Print(" -help                    (Display vtfcmd help.)\n");
    Print("\n");
    Print("Example vtfcmd usage:\n");
    Print("vtfcmd.exe -file \"C:\\texture1.bmp\" -file \"C:\\texture2.bmp\" -format \"dxt1\"\n");
    Print("vtfcmd.exe -folder \"C:\\input\\*.tga\" -output \"C:\\output\" -recurse -pause\n");
    Print("vtfcmd.exe -folder \"C:\\output\\*.vtf\" -output \"C:\\input\" -exportformat \"jpg\"\n");

    if (lpError != nullptr && !g_state.silent) {
        Print("\n");
        Print("Error:\n");

        va_start(ArgumentList, lpError);
        vprintf(lpError, ArgumentList);
        va_end(ArgumentList);

        Print("\n");
    }

    if (g_state.help) {
        Print("\n");
        Print("Formats: RGBA8888, ABGR8888, RGB888, BGR888, RGB565, I8, IA88, A8,\n");
        Print("         RGB888_BLUESCREEN, BGR888_BLUESCREEN, ARGB8888, BGRA8888, DXT1,\n");
        Print("         DXT3, DXT5, BGRX8888, BGR565, BGRX5551, BGRA4444,DXT1_ONEBITALPHA,\n");
        Print("         BGRA5551, UV88, UVWQ8888, RGBA16161616F, RGBA16161616, UVLX8888,\n");
        Print("         BC7, BC6H\n");

        Print("\n");
        Print("Flags:   POINTSAMPLE, TRILINEAR, CLAMPS, CLAMPT, ANISOTROPIC, HINT_DXT5,\n");
        Print("         NORMAL, NOMIP, NOLOD, MINMIP, PROCEDURAL, RENDERTARGET,\n");
        Print("         DEPTHRENDERTARGET, NODEBUGOVERRIDE, SINGLECOPY, NODEPTHBUFFER\n");
        Print("         CLAMPU, VERTEXTEXTURE, SSBUMP, BORDER");

        Print("\n");
        Print("Resize Method:  NEAREST, BIGGEST, SMALLEST, NEAREST4, BIGGEST4, SMALLEST4\n");

        Print("\n");
        Print("Resize Filter:  POINT, BOX, TRIANGLE, QUADRATIC, CUBIC, CATROM, MITCHELL\n");
        Print("                GAUSSIAN, SINC, BESSEL, HANNING, HAMMING, BLACKMAN, KAISER, NICE\n");

        Print("\n");
        Print("Normal Kernal:  4X, 3X3, 5X5, 7X7, 9X9, DUDV\n");

        Print("\n");
        Print("Normal Height:  ALPHA, AVERAGERGB, BIASEDRGB, RED, GREEN, BLUE, MAXRGB,\n");
        Print("                COLORSPACE\n");

        Print("\n");
        Print("Normal Alpha:   NOCHANGE, HEIGHT, BLACK, WHITE\n");
    }

    Pause();
}

#ifdef _WIN32
#define PATHSEP "\\"
#define PATHSEP_C '\\'
#else
#define PATHSEP "/"
#define PATHSEP_C '/'
#endif


//
// CreateOutputPath()
// Create an output file path from the input file path.
//
void CreateOutputPath(char *outputFile, char *inputFile, const char *extension) {
    char *lpTemp;

    // Create output file string.
    if (g_state.output != nullptr && *g_state.output != '\0') {
        // Put the file in the g_state.output directory.
        sprintf(outputFile, "%s" PATHSEP, g_state.output);
    } else {
        // Put the file in the same directory as the input file.
        strcpy(outputFile, inputFile);
        if ((lpTemp = strrchr(outputFile, PATHSEP_C)) != nullptr) {
            *(lpTemp + 1) = '\0';
        } else {
            *outputFile = '\0';
        }
    }

    // Add the prefix to the file name.
    strcat(outputFile, g_state.prefix);

    // Add the file name of the input file to the file name.
    if ((lpTemp = strrchr(inputFile, PATHSEP_C)) != nullptr) {
        strcat(outputFile, lpTemp + 1);
    } else {
        strcat(outputFile, inputFile);
    }

    if ((lpTemp = strrchr(outputFile, '.')) != nullptr && lpTemp > strrchr(outputFile, PATHSEP_C)) {
        *lpTemp = '\0';
    }

    // Add the postfix to the file name.
    strcat(outputFile, g_state.postfix);

    // Add the extension to the file name.
    strcat(outputFile, ".");
    strcat(outputFile, extension);
}

//
// FlipImage()
// Flip lpImageData over the horizontal axis.
//
void FlipImage(uint8_t *lpImageData, uint32_t uiWidth, uint32_t uiHeight, uint32_t uiChannels) {
    uint32_t i, j, k;
    uint8_t bTemp;

    for (i = 0; i < uiWidth; i++) {
        for (j = 0; j < uiHeight / 2; j++) {
            uint8_t *pOne = lpImageData + (i + j * uiWidth) * uiChannels;
            uint8_t *pTwo = lpImageData + (i + (uiHeight - j - 1) * uiWidth) * uiChannels;

            for (k = 0; k < uiChannels; k++) {
                bTemp = pOne[k];
                pOne[k] = pTwo[k];
                pTwo[k] = bTemp;
            }
        }
    }
}

static uint32_t BytesPerPixel(const CMP_MipSet &image) {
    uint32_t bytesPerChannel;

    switch (image.m_ChannelFormat) {
        case CF_8bit:
            bytesPerChannel = 1;
            break;

        case CF_16bit:
        case CF_Float16:
            bytesPerChannel = 2;
            break;

        case CF_Float32:
            bytesPerChannel = 4;
            break;

        default:
            return 0; // compressed/unknown
    }

    auto channels = image.m_nChannels;
    if (channels == 0) {
        switch (image.m_TextureDataType) {
            case TDT_XRGB:
                channels = 3;
                break;
            case TDT_ARGB:
                channels = 4;
                break;
            case TDT_R:
                channels = 1;
                break;
            case TDT_RG:
                channels = 2;
                break;
            case TDT_RGB:
                channels = 3;
                break;
            case TDT_8:
                channels = 1;
                break;
            case TDT_16:
                channels = 1;
                break;
            default: {
                return 0;
            }
        }
    }

    return bytesPerChannel * channels;
}

CMP_ERROR CreateMipSet(
    CMP_MipSet &mipSet,
    const void *data,
    int width,
    int height,
    CMP_FORMAT format) {
    int channels;

    switch (format) {
        case CMP_FORMAT_RGB_888:
        case CMP_FORMAT_BGR_888:
            channels = 3;
            break;

        case CMP_FORMAT_RGBA_8888:
        case CMP_FORMAT_BGRA_8888:
        case CMP_FORMAT_ARGB_8888:
        case CMP_FORMAT_ABGR_8888:
            channels = 4;
            break;

        default:
            return CMP_ERR_UNSUPPORTED_SOURCE_FORMAT;
    }

    mipSet = {};

    CMP_ERROR err = CMP_CreateMipSet(
        &mipSet,
        width,
        height,
        1,
        CF_8bit,
        TT_2D
    );

    if (err != CMP_OK)
        return err;

    CMP_MipLevel *mip = nullptr;
    CMP_GetMipLevel(&mip, &mipSet, 0, 0);

    if (!mip) {
        CMP_FreeMipSet(&mipSet);
        return CMP_ERR_MEM_ALLOC_FOR_MIPSET;
    }

    const size_t size =
            static_cast<size_t>(width) *
            static_cast<size_t>(height) *
            channels;

    mipSet.m_format = format;
    mipSet.m_nChannels = channels;
    mipSet.m_TextureDataType =
            channels == 4 ? TDT_ARGB : TDT_RGB;

    mipSet.dwWidth = width;
    mipSet.dwHeight = height;
    mipSet.dwDataSize = static_cast<CMP_DWORD>(size);

    mip->m_nWidth = width;
    mip->m_nHeight = height;
    mip->m_dwLinearSize = static_cast<CMP_DWORD>(size);

    std::memcpy(mip->m_pbData, data, size);

    mipSet.pData = mip->m_pbData;

    return CMP_OK;
}

//
// ProcessFile()
// Convert input file to a vtf file and place it in the output folder.
//
void ProcessFile(char *lpInputFile) {
    uint32_t i;
    VTFLib::Diagnostics::CError error;

    // Temp variable for string manipulation.
    char lpVTFFile[512]; // Holds output .vtf file name.
    char lpVMTFile[512]; // Holds output .vmt file name.
    char lpVMTBaseTexture[512]; // Holds $basetexture .vmt param.
    char lpExportFile[512]; // Holds output export file name.

    int32_t iTest; // Holds .vmt integer test result.
    float sTest; // Holds .vmt float test result.
    char cTest[4096]; // Holds .vmt string test result.

    uint32_t uiImageWidth, uiImageHeight; // Dimensions of the image being created.
    uint8_t *lpSourceData; // Image data to create the texture from.
    uint8_t *lpDistanceData; // Distance field data.
    uint32_t uiDestWidth, uiDestHeight; // Distance field dimensions.
    vlBool bClipped; // Was the distance field clipped?

    float sR, sG, sB; // Reflectivity.
    uint8_t *lpImageData; // Export data.
    VTFImageFormat DestFormat; // Export format.

    g_state.processed++;

    Print("Processing %s...\n", lpInputFile);

    const char *lpTemp = strrchr(lpInputFile, '.');

    if (lpTemp == nullptr || stricmp(lpTemp, ".vtf") != 0) {
        // Load input file.

        CMP_MipSet image{};
        if (CMP_LoadTexture(lpInputFile, &image) != CMP_OK) {
            Print(" Error loading input file.\n\n");
            return;
        }

        Print(" Information:\n");

        // Display input file info.
        Print("  Width: %d\n", image.m_nWidth);
        Print("  Height: %d\n", image.m_nHeight);
        Print("  BPP: %d\n\n", BytesPerPixel(image));

        g_state.createOptions.imageFormat = BytesPerPixel(image) == 4 ? g_state.alphaFormat : g_state.normalFormat;

        Print(" Creating texture:\n");

        CMP_MipSet rgba_mipset{};
        CMP_CompressOptions options{0};
        options.DestFormat = CMP_FORMAT_RGBA_8888;
        options.dwnumThreads = 4;
        if (CMP_ConvertMipTexture(&image, &rgba_mipset, &options, nullptr) != CMP_OK) {
            Print("  Error converting input file.\n\n");
            return;
        }

        uiImageWidth = static_cast<uint32_t>(image.m_nWidth);
        uiImageHeight = static_cast<uint32_t>(image.m_nHeight);
        CMP_MipLevel *mip = nullptr;
        CMP_GetMipLevel(&mip, &image, 0, 0);
        lpSourceData = mip->m_pbData;
        lpDistanceData = nullptr;

        // Replace the alpha channel with a distance field.
        if (g_state.distanceAlpha) {
            uiDestWidth = uiImageWidth / g_state.distanceAlphaReduce;
            uiDestHeight = uiImageHeight / g_state.distanceAlphaReduce;

            if (uiDestWidth == 0)
                uiDestWidth = 1;
            if (uiDestHeight == 0)
                uiDestHeight = 1;

            lpDistanceData = static_cast<uint8_t *>(malloc(uiDestWidth * uiDestHeight * 4));

            if (lpDistanceData == nullptr) {
                Print("  Error allocating distance field.\n\n");
                return;
            }

            bClipped = vlFalse;

            if (!vlImageConvertToDistanceField(lpSourceData, lpDistanceData, uiImageWidth, uiImageHeight, uiDestWidth,
                                               uiDestHeight, g_state.distanceAlphaSpread, g_state.distanceAlphaThreshold, &bClipped,
                                               error)) {
                Print("  Error creating distance field:\n%s\n\n", error.Get());
                free(lpDistanceData);
                return;
            }

            if (bClipped) {
                Print("  Warning: the distance field reaches the edge of the image and has been clipped.\n");
            }

            lpSourceData = lpDistanceData;
            uiImageWidth = uiDestWidth;
            uiImageHeight = uiDestHeight;

            // The distance field always needs an alpha channel.
            g_state.createOptions.imageFormat = g_state.alphaFormat;
        }

        // Create vtf file.
        if (!vlImageCreateSingle(uiImageWidth, uiImageHeight, lpSourceData, &g_state.createOptions, error)) {
            Print("  Error creating vtf file:\n%s\n\n", error.Get());
            free(lpDistanceData);
            return;
        }

        free(lpDistanceData);

        CreateOutputPath(lpVTFFile, lpInputFile, "vtf");

        // Write vtf file.
        Print("  Writing %s...\n", lpVTFFile);
        if (!vlImageSave(lpVTFFile, error)) {
            Print(" Error creating vtf file:\n%s\n\n", error.Get());
            return;
        }
        Print("  %s written.\n\n", lpVTFFile);

        // Do we build a material?
        if (g_state.shader != nullptr) {
            Print(" Creating material:\n");

            // We need to constuct a $basetexture string, to do this we need the path
            // of the vtf file relative to the materials folder.  If we arn't in a
            // materials folder we can't do this.
            if ((lpTemp = stristr(lpVTFFile, "materials\\")) == nullptr) {
                Print("  Error creating vmt: texture is not in a ...\\materials\\ folder.\n\n");
            } else {
                strcpy(lpVMTFile, lpVTFFile);
                strcpy(strrchr(lpVMTFile, '.'), ".vmt");

                strcpy(lpVMTBaseTexture, lpTemp + strlen("materials\\"));
                *strrchr(lpVMTBaseTexture, '.') = '\0';
                strrpl(lpVMTBaseTexture, '\\', '/');

                vlMaterialCreate(g_state.shader, error); // Create the root node.
                vlMaterialGetFirstNode(); // Go to the root node.
                vlMaterialAddNodeString("$basetexture", lpVMTBaseTexture); // Add a string node to the root node.

                // Add the custom parameters.
                for (i = 0; i < g_state.parameterCount; i++) {
                    // Figure out if the parameter is a string, single or integer.

                    if (sscanf(g_state.parameters[i][1], "%d%s", &iTest, cTest) == 1) {
                        // We can interpet the string as an integer, assume it is one.
                        vlMaterialAddNodeInteger(g_state.parameters[i][0], iTest);
                    } else if (sscanf(g_state.parameters[i][1], "%f%s", &sTest, cTest) == 1) {
                        // We can interpet the string as an single, assume it is one.
                        vlMaterialAddNodeSingle(g_state.parameters[i][0], sTest);
                    } else {
                        // The string must be a string...
                        vlMaterialAddNodeString(g_state.parameters[i][0], g_state.parameters[i][1]);
                    }
                }

                // Write vmt file.
                Print("  Writing %s...\n", lpVMTFile);
                if (!vlMaterialSave(lpVMTFile, error)) {
                    Print("Error creating vtf file:\n%s\n\n", error.Get());
                    return;
                }
                Print("  %s written.\n\n", lpVMTFile);
            }
        }
    } else {
        if (!vlImageLoad(lpInputFile, vlFalse, error)) {
            Print(" Error loading input file:\n%s\n\n", error.Get());
            return;
        }

        Print(" Information:\n");

        // Display input file info.
        Print("  Version: v%u.%u\n", vlImageGetMajorVersion(), vlImageGetMinorVersion());
        Print("  Size On Disk: %.2f KB\n", (float) vlImageGetSize(error) / 1024.0f);
        Print("  Width: %u\n", vlImageGetWidth());
        Print("  Height: %u\n", vlImageGetHeight());
        Print("  Depth: %u\n", vlImageGetDepth());
        Print("  Frames: %u\n", vlImageGetFrameCount());
        Print("  Start Frame: %u\n", vlImageGetStartFrame());
        Print("  Faces: %u\n", vlImageGetFaceCount());
        Print("  Mipmaps: %u\n", vlImageGetMipmapCount());
        if (vlImageGetSupportsAuxCompression() && vlImageGetAuxCompressionLevel() != VTF_AUX_COMPRESSION_LEVEL_NONE) {
            Print("  Compression: %s, level %d\n",
                  vlImageGetAuxCompressionMethod() == AUX_COMPRESSION_METHOD_ZSTD ? "Zstandard" : "deflate",
                  vlImageGetAuxCompressionLevel());
        }
        Print("  Flags: %#.8x\n", vlImageGetFlags());
        Print("  Bumpmap Scale: %.2f\n", vlImageGetBumpmapScale());
        vlImageGetReflectivity(&sR, &sG, &sB);
        Print("  Reflectivity: %.2f, %.2f, %.2f\n", sR, sG, sB);
        Print("  Format: %s\n\n", vlImageGetImageFormatInfo(vlImageGetFormat())->lpName);
        Print("  Resources: %u\n", vlImageGetResourceCount());

        Print(" Creating texture:\n");

        // Figure out which destination format to use.
        DestFormat = (vlImageGetFlags() & (TEXTUREFLAGS_ONEBITALPHA | TEXTUREFLAGS_EIGHTBITALPHA))
                         ? IMAGE_FORMAT_RGBA8888
                         : IMAGE_FORMAT_RGB888;

        // Alocate the required memory to convert the vtf to.
        lpImageData = static_cast<uint8_t *>(malloc(
            vlImageComputeImageSize(vlImageGetWidth(), vlImageGetHeight(), 1, 1, DestFormat)));

        if (lpImageData == nullptr) {
            Print(" malloc() failed.\n\n");
            return;
        }

        // Convert the .vtf.
        if (!vlImageConvert(vlImageGetData(0, 0, 0, 0), lpImageData, vlImageGetWidth(), vlImageGetHeight(),
                            vlImageGetFormat(), DestFormat, error)) {
            free(lpImageData);

            Print(" Error converting input file:\n%s\n\n", error.Get());
            return;
        }

        // DevIL likes the image data upside down.
        FlipImage(lpImageData, vlImageGetWidth(), vlImageGetHeight(), DestFormat == IMAGE_FORMAT_RGBA8888 ? 4 : 3);
        CMP_MipSet image{};
        // Create a new image with the converted image data in DevIL.
        if (CreateMipSet(image, lpImageData, vlImageGetWidth(), vlImageGetHeight(), CMP_FORMAT_RGBA_8888) != CMP_OK) {
            free(lpImageData);
            Print("  Error creating %s file.\n\n", g_state.exportFormat);
            return;
        }

        free(lpImageData);

        CreateOutputPath(lpExportFile, lpInputFile, g_state.exportFormat);

        // Write tga file.
        Print("  Writing %s...\n", lpExportFile);
        if (CMP_SaveTexture(lpExportFile, &image) != CMP_OK) {
            Print(" Error creating %s file.\n\n", g_state.exportFormat);
            return;
        }
        Print("  %s written.\n\n", lpExportFile);
    }

    Print("%s processed.\n\n", lpInputFile);

    g_state.completed++;
}

//
// ProcessFile()
// Process all files in the input folder.
//
void ProcessFolder(char *lpInputFolder, char *lpWildcard) {
    char lpSearchString[512];
    char lpPath[512];

    winfind::WIN32_FIND_DATA FindData;
    winfind::HANDLE Handle;

    Print("Processing %s\\...\n\n", lpInputFolder);

    if (g_state.recursive) {
        sprintf(lpSearchString, "%s\\*", lpInputFolder);

        Handle = FindFirstFile(lpSearchString, &FindData);

        if (Handle != INVALID_HANDLE_VALUE) {
            do {
                if (stricmp(FindData.cFileName, ".") != 0 && stricmp(FindData.cFileName, "..") != 0) {
                    sprintf(lpPath, "%s\\%s", lpInputFolder, FindData.cFileName);

                    if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        ProcessFolder(lpPath, lpWildcard);
                    }
                }
            } while (FindNextFile(Handle, &FindData));

            winfind::FindClose(Handle);
        }
    }

    sprintf(lpSearchString, "%s\\%s", lpInputFolder, lpWildcard);

    Handle = FindFirstFile(lpSearchString, &FindData);

    if (Handle != INVALID_HANDLE_VALUE) {
        do {
            if (stricmp(FindData.cFileName, ".") != 0 && stricmp(FindData.cFileName, "..") != 0) {
                sprintf(lpPath, "%s\\%s", lpInputFolder, FindData.cFileName);

                if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    ProcessFile(lpPath);
                }
            }
        } while (winfind::FindNextFile(Handle, &FindData));

        winfind::FindClose(Handle);
    }

    Print("%s\\ processed.\n\n", lpInputFolder);
}
