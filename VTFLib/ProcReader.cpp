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
#include "ProcReader.h"

using namespace VTFLib::IO::Readers;

CProcReader::CProcReader(void *userData) {
    this->mIsOpen = false;
    this->mUserData = userData;
}

CProcReader::~CProcReader() {
    this->CProcReader::Close();
}

bool CProcReader::IsOpen() const {
    return this->mIsOpen;
}

bool CProcReader::Open(Diagnostics::CError &error) {
    this->Close();

    if (pReadOpenProc == nullptr) {
        VTFError_Set(error, "pReadOpenProc not set.");
        return false;
    }

    if (this->mIsOpen) {
        VTFError_Set(error, "Reader already open.");
        return false;
    }

    if (!pReadOpenProc(this->mUserData)) {
        VTFError_Set(error, "Error opening file.");
        return false;
    }

    this->mIsOpen = true;

    return true;
}

void CProcReader::Close() {
    if (pReadCloseProc == nullptr) {
        return;
    }

    if (this->mIsOpen) {
        pReadCloseProc(this->mUserData);
        this->mIsOpen = false;
    }
}

ssize_t CProcReader::GetStreamSize(Diagnostics::CError &error) const {
    if (!this->mIsOpen) {
        VTFError_Set(error, "Reader not open.");
        return 0;
    }

    if (pReadSizeProc == nullptr) {
        VTFError_Set(error, "pReadSizeProc not set.");
        return 0xffffffff;
    }

    return pReadSizeProc(this->mUserData);
}

ssize_t CProcReader::GetStreamPointer(Diagnostics::CError &error) const {
    if (!this->mIsOpen) {
        VTFError_Set(error, "Reader not open.");
        return 0;
    }

    if (pReadTellProc == nullptr) {
        VTFError_Set(error, "pReadTellProc not set.");
        return 0;
    }

    return pReadTellProc(this->mUserData);
}

ssize_t CProcReader::Seek(const ssize_t offset, uint32_t seekMode, Diagnostics::CError &error) {
    if (!this->mIsOpen) {
        VTFError_Set(error, "Reader not open.");
        return 0;
    }

    if (pReadSeekProc == nullptr) {
        VTFError_Set(error, "pReadSeekProc not set.");
        return 0;
    }

    return pReadSeekProc(offset, static_cast<VLSeekMode>(seekMode), this->mUserData);
}

bool CProcReader::Read(char &dstChr, Diagnostics::CError &error) {
    if (!this->mIsOpen) {
        VTFError_Set(error, "Reader not open.");
        return false;
    }

    if (pReadReadProc == nullptr) {
        VTFError_Set(error, "pReadReadProc not set.");
        return false;
    }

    const uint32_t bytesRead = pReadReadProc(&dstChr, 1, this->mUserData);

    if (bytesRead == 0) {
        VTFError_Set(error, "pReadReadProc() failed.");
    }

    return bytesRead == 1;
}

ssize_t CProcReader::Read(void *dst, const uint32_t size, Diagnostics::CError &error) {
    if (!this->mIsOpen) {
        VTFError_Set(error, "Reader not open.");
        return 0;
    }

    if (pReadReadProc == nullptr) {
        VTFError_Set(error, "pReadReadProc not set.");
        return 0;
    }

    const uint32_t bytesRead = pReadReadProc(dst, size, this->mUserData);

    if (bytesRead == 0) {
        VTFError_Set(error, "pReadReadProc() failed.");
    }

    return bytesRead;
}
