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
#include <cerrno>

using namespace VTFLib;
using namespace VTFLib::IO::Writers;

CFileWriter::CFileWriter(const vlChar *cFileName)
{
    this->hFile = nullptr;

    this->cFileName = new vlChar[strlen(cFileName) + 1];
    strcpy(this->cFileName, cFileName);
}

CFileWriter::~CFileWriter()
{
    this->Close();

    delete []this->cFileName;
}

vlBool CFileWriter::Opened() const
{
    return this->hFile != nullptr;
}

vlBool CFileWriter::Open(Diagnostics::CError &error)
{
    this->Close();

    this->hFile = fopen(this->cFileName, "wb");

    if(this->hFile == nullptr)
    {
        error.Set("Error opening file.", vlTrue);
        return vlFalse;
    }

    return vlTrue;
}

vlVoid CFileWriter::Close()
{
    if(this->hFile != nullptr)
    {
        fclose(this->hFile);
        this->hFile = nullptr;
    }
}

vlUInt CFileWriter::GetStreamSize(Diagnostics::CError &error) const
{
    if(this->hFile == nullptr)
    {
        return 0;
    }

    const long lPosition = ftell(this->hFile);

    if(lPosition < 0)
    {
        return 0;
    }

    if(fseek(this->hFile, 0, SEEK_END) != 0)
    {
        return 0;
    }

    const long lSize = ftell(this->hFile);

    fseek(this->hFile, lPosition, SEEK_SET);

    if(lSize < 0)
    {
        return 0;
    }

    return static_cast<vlUInt>(lSize);
}

vlUInt CFileWriter::GetStreamPointer(Diagnostics::CError &error) const
{
    if(this->hFile == nullptr)
    {
        return 0;
    }

    const long lPosition = ftell(this->hFile);

    if(lPosition < 0)
    {
        return 0;
    }

    return static_cast<vlUInt>(lPosition);
}

vlUInt CFileWriter::Seek(vlLong lOffset, vlUInt uiMode, VTFLib::Diagnostics::CError& error)
{
    if(this->hFile == nullptr)
    {
        return 0;
    }

    int iOrigin;

    switch(uiMode)
    {
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
        return 0;
    }

    if(fseek(this->hFile, lOffset, iOrigin) != 0)
    {
        error.Set("fseek() failed.", vlTrue);
        return 0;
    }

    return this->GetStreamPointer(error);
}

vlBool CFileWriter::Write(vlChar cChar, Diagnostics::CError &error)
{
    if(this->hFile == nullptr)
    {
        return vlFalse;
    }

    const size_t uiBytesWritten = fwrite(&cChar, 1, 1, this->hFile);

    if(uiBytesWritten != 1)
    {
        error.Set("fwrite() failed.", vlTrue);
    }

    return uiBytesWritten == 1;
}

vlUInt CFileWriter::Write(vlVoid *vData, vlUInt uiBytes, Diagnostics::CError &error)
{
    if(this->hFile == nullptr)
    {
        return 0;
    }

    const size_t uiBytesWritten = fwrite(vData, 1, uiBytes, this->hFile);

    if(uiBytesWritten != uiBytes && ferror(this->hFile))
    {
        error.Set("fwrite() failed.", vlTrue);
    }

    return static_cast<vlUInt>(uiBytesWritten);
}