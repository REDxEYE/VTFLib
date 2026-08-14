/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VTFLib.h"
#include "MemoryReader.h"

using namespace VTFLib::IO::Readers;

CMemoryReader::CMemoryReader(const void *buffer, const uint32_t bufferSize) {
    mIsOpen = vlFalse;
    mCursor = 0;

    mBuffer = buffer;
    mBufferSize = bufferSize;
}

bool CMemoryReader::IsOpen() const {
    return mIsOpen;
}

bool CMemoryReader::Open(Diagnostics::CError &error) {
    if (mBuffer == nullptr) {
        VTFError_Set(error, "Memory stream is null.");
        return vlFalse;
    }

    mCursor = 0;
    mIsOpen = vlTrue;

    return vlTrue;
}

void CMemoryReader::Close() {
    mIsOpen = vlFalse;
}

ssize_t CMemoryReader::GetStreamSize(Diagnostics::CError &error) const {
    if (!mIsOpen) {
        VTFError_Set(error, "Memory stream is not open.");
        return 0;
    }

    return mBufferSize;
}

ssize_t CMemoryReader::GetStreamPointer(Diagnostics::CError &error) const {
    if (!mIsOpen) {
        VTFError_Set(error, "Memory stream is not open.");
        return 0;
    }

    return mCursor;
}

ssize_t CMemoryReader::Seek(const ssize_t offset, const uint32_t seekMode, Diagnostics::CError &error) {
    if (!mIsOpen) {
        VTFError_Set(error, "Memory stream is not open.");
        return 0;
    }

    switch (seekMode) {
        case FILE_BEGIN:
            mCursor = 0;
            break;
        case FILE_CURRENT:
            break;
        case FILE_END:
            mCursor = mBufferSize;
            break;
        default: {
            VTFError_Set(error, "Invalid seek mode.");
            return 0;
        }
    }

    int64_t new_offset = mCursor + offset;

    if (new_offset < 0) {
        new_offset = 0;
    }

    if (new_offset > mBufferSize) {
        new_offset = mBufferSize;
    }

    mCursor = static_cast<uint32_t>(new_offset);

    return mCursor;
}

bool CMemoryReader::Read(char &dstChr, Diagnostics::CError &error) {
    if (!mIsOpen) {
        return vlFalse;
    }

    if (mCursor == mBufferSize) {
        VTFError_Set(error, "End of memory stream.");
        return vlFalse;
    }

    dstChr = static_cast<const char *>(mBuffer)[mCursor++];

    return vlTrue;
}

ssize_t CMemoryReader::Read(void *dst, uint32_t size, Diagnostics::CError &error) {
    if (!mIsOpen) {
        return 0;
    }

    if (mCursor == mBufferSize) {
        return 0;
    }
    const auto *src = static_cast<const uint8_t *>(mBuffer) + mCursor;

    if (mCursor + size > mBufferSize) // This right?
    {
        size = mBufferSize - mCursor;

        memcpy(dst, src, size);

        mCursor = mBufferSize;

        VTFError_Set(error, "End of memory stream.");

        return size;
    }
    memcpy(dst, src, size);

    mCursor += size;

    return size;
}
