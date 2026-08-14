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
#include "Writer.h"


namespace VTFLib::IO::Writers {
    class CFileWriter : public IWriter {

    public:
        explicit CFileWriter(const char *filePath);

        ~CFileWriter() override;

        [[nodiscard]] bool IsOpen() const override;

        bool Open(Diagnostics::CError &error) override;

        void Close() override;

        ssize_t GetStreamSize(Diagnostics::CError &error) const override;

        ssize_t GetStreamPointer(Diagnostics::CError &error) const override;

        ssize_t Seek(ssize_t offset, uint32_t seekMode, Diagnostics::CError &error) override;

        bool Write(char srcChr, Diagnostics::CError &error) override;

        ssize_t Write(const void *src, ssize_t size, Diagnostics::CError &error) override;

    private:
        HANDLE mHandle;
        char *mFilePath;

    };
}


