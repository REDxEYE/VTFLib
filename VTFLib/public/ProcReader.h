/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#pragma once

#include "VTFLibShared.h"
#include "Reader.h"


namespace VTFLib::IO::Readers {
    class CProcReader : public IReader {
    public:
        explicit CProcReader(void *userData);

        ~CProcReader() override;

        [[nodiscard]] bool IsOpen() const override;

        bool Open(Diagnostics::CError &error) override;

        void Close() override;

        ssize_t GetStreamSize(Diagnostics::CError &error) const override;

        ssize_t GetStreamPointer(Diagnostics::CError &error) const override;

        ssize_t Seek(ssize_t offset, uint32_t seekMode, Diagnostics::CError &error) override;

        bool Read(char &dstChr, Diagnostics::CError &error) override;

        ssize_t Read(void *dst, uint32_t size, Diagnostics::CError &error) override;

    private:
        bool mIsOpen;
        void *mUserData;
    };
}
