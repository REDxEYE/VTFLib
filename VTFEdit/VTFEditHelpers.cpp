//
// Created by red_eye on 8/15/26.
//

#include "VTFEditHelpers.hpp"

#include <cstring>

CMP_ERROR CreateMipSet(CMP_MipSet &mipSet, const void *data, int width, int height, CMP_FORMAT format) {
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
