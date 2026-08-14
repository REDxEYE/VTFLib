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
#include "MemoryWriter.h"

using namespace VTFLib;
using namespace VTFLib::IO::Writers;

CMemoryWriter::CMemoryWriter(void *buffer, const uint32_t uiBufferSize) {
    this->mIsOpen = false;

    this->mBuffer = buffer;
    this->mBufferSize = uiBufferSize;

    this->mOffset = 0;
    this->mWritten = 0;
}

bool CMemoryWriter::IsOpen() const {
    return this->mIsOpen;
}

bool CMemoryWriter::Open(Diagnostics::CError &error) {
    if (mBuffer == nullptr) {
        VTFError_Set(error, "Memory stream is null.");
        return false;
    }

    this->mOffset = 0;
    this->mWritten = 0;
    this->mIsOpen = true;

    return true;
}

void CMemoryWriter::Close() {
    this->mIsOpen = false;
}

ssize_t CMemoryWriter::GetStreamSize(Diagnostics::CError &error) const {
    /*if(!this->bOpened)
    {
        return 0;
    }*/

    return this->mWritten;
}

ssize_t CMemoryWriter::GetStreamPointer(Diagnostics::CError &error) const {
    if (!this->mIsOpen) {
        return 0;
    }

    return this->mOffset;
}

ssize_t CMemoryWriter::Seek(const ssize_t offset, const uint32_t seekMode, Diagnostics::CError &error) {
    if (!this->mIsOpen) {
        return 0;
    }

    switch (seekMode) {
        case FILE_BEGIN:
            this->mOffset = 0;
            break;
        case FILE_CURRENT:
            break;
        case FILE_END:
            this->mOffset = this->mWritten;
            break;
        default: {
            VTFError_Set(error, "Invalid seek mode.");
            return false;
        }
    }

    ssize_t newOffset = this->mOffset + offset;

    if (newOffset < 0) {
        newOffset = 0;
    }

    if (newOffset > this->mWritten) {
        newOffset = this->mWritten;
    }

    this->mOffset = newOffset;
    return this->mOffset;
}

bool CMemoryWriter::Write(const char srcChr, Diagnostics::CError &error) {
    if (!this->mIsOpen) {
        return false;
    }

    if (this->mOffset == this->mBufferSize) {
        VTFError_Set(error, "End of memory stream.");
        return false;
    }
    static_cast<char *>(mBuffer)[mOffset++] = srcChr;
    this->mWritten++;
    return true;
}

ssize_t CMemoryWriter::Write(const void *src, ssize_t size, Diagnostics::CError &error) {
    if (!this->mIsOpen) {
        return 0;
    }

    if (this->mOffset == this->mBufferSize) {
        return 0;
    }

    auto *dst = static_cast<char *>(mBuffer) + this->mOffset;

    if (this->mOffset + size > this->mBufferSize) {
        size = this->mBufferSize - this->mOffset;
        memcpy(dst, src, size);
        this->mWritten += size;
        this->mOffset = this->mBufferSize;
        VTFError_Set(error, "End of memory stream.");
        return size;
    }

    memcpy(dst, src, size);
    this->mWritten += size;
    this->mOffset += size;

    return size;
}
