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
#include "Readers.h"
#include "Writers.h"
#include "VMTNodes.h"

//! VMT parsing mode.
typedef enum tagVMTParseMode {
    PARSE_MODE_STRICT = 0,
    PARSE_MODE_LOOSE,
    PARSE_MODE_COUNT
} VMTParseMode;


namespace VTFLib {
    class VTFLIB_API CVMTFile {
    public:
        CVMTFile();

        CVMTFile(const CVMTFile &other);

        ~CVMTFile();

        bool Create(const char *cRoot);

        void Destroy();

        [[nodiscard]] bool IsLoaded() const;

        bool Load(const char *filePath, Diagnostics::CError &error);

        bool Load(const void *buffer, ssize_t bufferSize, Diagnostics::CError &error);

        bool Load(void *userData, Diagnostics::CError &error);

        bool Save(const char *filePath, Diagnostics::CError &error) const;

        bool Save(void *buffer, ssize_t bufferSize, ssize_t &realSize, Diagnostics::CError &error) const;

        bool Save(void *userData, Diagnostics::CError &error) const;

        [[nodiscard]] Nodes::CVMTGroupNode *GetRoot() const;

        [[nodiscard]] uint32_t GetParseErrorLine() const;

    private:
        Nodes::CVMTGroupNode *mRoot;

        uint32_t mParseErrorLine;

        bool Load(IO::Readers::IReader *reader, Diagnostics::CError &error);

        bool Save(IO::Writers::IWriter *writer, Diagnostics::CError &error) const;

        //Nodes::CVMTNode *Load(IO::Readers::IReader *Reader, bool bInGroup);

        void Indent(IO::Writers::IWriter *writer, uint32_t level, Diagnostics::CError &error) const;

        void Save(IO::Writers::IWriter *writer, Nodes::CVMTNode *node, Diagnostics::CError &error,
                  uint32_t level = 0) const;
    };
}
