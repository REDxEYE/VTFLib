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
#include "Error.h"


namespace VTFLib::IO::Readers {
    class IReader {
    public:
        virtual ~IReader() = default;

        [[nodiscard]] virtual bool IsOpen() const = 0;

        virtual bool Open(Diagnostics::CError &error) = 0;

        virtual void Close() = 0;

        virtual ssize_t GetStreamSize(Diagnostics::CError &error) const = 0;

        virtual ssize_t GetStreamPointer(Diagnostics::CError &error) const = 0;

        virtual ssize_t Seek(ssize_t offset, uint32_t seekMode, Diagnostics::CError &error) = 0;

        virtual bool Read(char &dstChar, Diagnostics::CError &error) = 0;

        virtual ssize_t Read(void *dst, uint32_t size, Diagnostics::CError &error) = 0;
    };
}

