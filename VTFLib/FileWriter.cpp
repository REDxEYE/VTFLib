/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VTFLib.h"
#include "FileWriter.h"

#include <cstdio>

using namespace VTFLib::IO::Writers;

CFileWriter::CFileWriter(const char *filePath) {
    this->mHandle = nullptr;

    this->mFilePath = new char[strlen(filePath) + 1];
    strcpy(this->mFilePath, filePath);
}

CFileWriter::~CFileWriter() {
    this->CFileWriter::Close();

    delete[] this->mFilePath;
}

bool CFileWriter::IsOpen() const {
    return this->mHandle != nullptr;
}

bool CFileWriter::Open(Diagnostics::CError &error) {
    this->Close();

    this->mHandle = fopen(this->mFilePath, "wb");

    if (this->mHandle == nullptr) {
        VTFError_Set_SE(error, "Error opening file.");
        return false;
    }

    return true;
}

void CFileWriter::Close() {
    if (this->mHandle != nullptr) {
        fclose(this->mHandle);
        this->mHandle = nullptr;
    }
}

ssize_t CFileWriter::GetStreamSize(Diagnostics::CError &error) const {
    if (this->mHandle == nullptr) {
        VTFError_Set(error, "File handle is null.");
        return 0;
    }

    const long lPosition = ftell(this->mHandle);

    if (lPosition < 0) {
        return 0;
    }

    if (fseek(this->mHandle, 0, SEEK_END) != 0) {
        return 0;
    }

    const long lSize = ftell(this->mHandle);

    fseek(this->mHandle, lPosition, SEEK_SET);

    if (lSize < 0) {
        return 0;
    }

    return static_cast<uint32_t>(lSize);
}

ssize_t CFileWriter::GetStreamPointer(Diagnostics::CError &error) const {
    if (this->mHandle == nullptr) {
        VTFError_Set(error, "File handle is null.");
        return 0;
    }

    const long lPosition = ftell(this->mHandle);

    if (lPosition < 0) {
        return 0;
    }

    return static_cast<uint32_t>(lPosition);
}

ssize_t CFileWriter::Seek(const ssize_t offset, const uint32_t seekMode, Diagnostics::CError &error) {
    if (this->mHandle == nullptr) {
        VTFError_Set(error, "File handle is null.");
        return 0;
    }

    int iOrigin;

    switch (seekMode) {
        case FILE_BEGIN:
            iOrigin = SEEK_SET;
            break;

        case FILE_CURRENT:
            iOrigin = SEEK_CUR;
            break;

        case FILE_END:
            iOrigin = SEEK_END;
            break;

        default:
            VTFError_Set(error, "Invalid seek mode.");
            return 0;
    }

    if (fseek(this->mHandle, offset, iOrigin) != 0) {
        VTFError_Set_SE(error, "fseek() failed.");
        return 0;
    }

    return this->GetStreamPointer(error);
}

bool CFileWriter::Write(const char srcChr, Diagnostics::CError &error) {
    if (this->mHandle == nullptr) {
        VTFError_Set(error, "File handle is null.");
        return false;
    }

    const size_t bytesWritten = fwrite(&srcChr, 1, 1, this->mHandle);

    if (bytesWritten != 1) {
        VTFError_Set_SE(error, "fwrite() failed.");
    }

    return bytesWritten == 1;
}

ssize_t CFileWriter::Write(const void *src, const ssize_t size, Diagnostics::CError &error) {
    if (this->mHandle == nullptr) {
        VTFError_Set(error, "File handle is null.");
        return 0;
    }

    const size_t bytesWritten = fwrite(src, 1, size, this->mHandle);

    if (bytesWritten != size && ferror(this->mHandle)) {
        VTFError_Set_SE(error, "fwrite() failed.");
    }

    return static_cast<ssize_t>(bytesWritten);
}
