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
#include "Proc.h"
#include "ProcWriter.h"

using namespace VTFLib::IO::Writers;

CProcWriter::CProcWriter(void *userData) {
    this->mIsOpen = false;
    this->mUserData = userData;
}

CProcWriter::~CProcWriter() {
    this->CProcWriter::Close();
}

bool CProcWriter::IsOpen() const {
    return this->mIsOpen;
}

bool CProcWriter::Open(Diagnostics::CError &error) {
    this->Close();

    if (pWriteOpenProc == nullptr) {
        VTFError_Set(error, "pWriteOpenProc not set.");
        return false;
    }

    if (this->mIsOpen) {
        VTFError_Set(error, "Writer already open.");
        return false;
    }

    if (!pWriteOpenProc(this->mUserData)) {
        VTFError_Set(error, "Error opening file.");
        return false;
    }

    this->mIsOpen = true;

    return true;
}

void CProcWriter::Close() {
    if (pWriteCloseProc == nullptr) {
        return;
    }

    if (this->mIsOpen) {
        pWriteCloseProc(this->mUserData);
        this->mIsOpen = false;
    }
}

ssize_t CProcWriter::GetStreamSize(Diagnostics::CError &error) const {
    if (!this->mIsOpen) {
        return 0;
    }

    if (pWriteSizeProc == nullptr) {
        VTFError_Set(error, "pWriteSizeProc not set.");
        return -1;
    }

    return pWriteSizeProc(this->mUserData);
}

ssize_t CProcWriter::GetStreamPointer(Diagnostics::CError &error) const {
    if (!this->mIsOpen) {
        VTFError_Set(error, "Writer is not open.");
        return 0;
    }

    if (pWriteTellProc == nullptr) {
        VTFError_Set(error, "pWriteTellProc not set.");
        return 0;
    }

    return pWriteTellProc(this->mUserData);
}

ssize_t CProcWriter::Seek(const ssize_t offset, uint32_t seekMode, Diagnostics::CError &error) {
    if (!this->mIsOpen) {
        return 0;
    }

    if (pWriteSeekProc == nullptr) {
        VTFError_Set(error, "pWriteSeekProc not set.");
        return 0;
    }

    return pWriteSeekProc(offset, static_cast<VLSeekMode>(seekMode), this->mUserData);
}

bool CProcWriter::Write(const char srcChr, Diagnostics::CError &error) {
    if (!this->mIsOpen) {
        VTFError_Set(error, "Writer is not open.");
        return false;
    }

    if (pWriteWriteProc == nullptr) {
        VTFError_Set(error, "pWriteWriteProc not set.");
        return false;
    }

    const ssize_t bytesWritten = pWriteWriteProc(&srcChr, 1, this->mUserData);

    if (bytesWritten == 0) {
        VTFError_Set(error, "pWriteWriteProc() failed.");
    }

    return bytesWritten == 1;
}

ssize_t CProcWriter::Write(const void *src, const ssize_t size, Diagnostics::CError &error) {
    if (!this->mIsOpen) {
        VTFError_Set(error, "Writer is not open.");
        return 0;
    }

    if (pWriteWriteProc == nullptr) {
        VTFError_Set(error, "pWriteWriteProc not set.");
        return 0;
    }

    const ssize_t bytesWritten = pWriteWriteProc(src, size, this->mUserData);

    if (bytesWritten == 0) {
        VTFError_Set(error, "pWriteWriteProc() failed.");
    }

    return bytesWritten;
}
