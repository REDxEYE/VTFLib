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

using namespace VTFLib;
using namespace VTFLib::IO::Writers;

CProcWriter::CProcWriter(vlVoid *pUserData)
{
	this->bOpened = vlFalse;
	this->pUserData = pUserData;
}

CProcWriter::~CProcWriter()
{
	this->Close();
}

vlBool CProcWriter::Opened() const
{
	return this->bOpened;
}

vlBool CProcWriter::Open(Diagnostics::CError &error)
{
	this->Close();

	if(pWriteOpenProc == nullptr)
	{
		error.Set("pWriteOpenProc not set.");
		return vlFalse;
	}

	if(this->bOpened)
	{
		error.Set("Writer already open.");
		return vlFalse;
	}

	if(!pWriteOpenProc(this->pUserData))
	{
		error.Set("Error opening file.");
		return vlFalse;
	}

	this->bOpened = vlTrue;

	return vlTrue;
}

vlVoid CProcWriter::Close()
{
	if(pWriteCloseProc == nullptr)
	{
		return;
	}

	if(this->bOpened)
	{
		pWriteCloseProc(this->pUserData);
		this->bOpened = vlFalse;
	}
}

vlUInt CProcWriter::GetStreamSize(VTFLib::Diagnostics::CError& error) const
{
	if(!this->bOpened)
	{
		return 0;
	}

	if(pWriteSizeProc == nullptr)
	{
		error.Set("pWriteTellProc not set.");
		return 0xffffffff;
	}

	return pWriteSizeProc(this->pUserData);
}

vlUInt CProcWriter::GetStreamPointer(VTFLib::Diagnostics::CError& error) const
{
	if(!this->bOpened)
	{
		return 0;
	}

	if(pWriteTellProc == nullptr)
	{
		error.Set("pWriteTellProc not set.");
		return 0;
	}

	return pWriteTellProc(this->pUserData);
}

vlUInt CProcWriter::Seek(vlLong lOffset, vlUInt uiMode, Diagnostics::CError &error)
{
	if(!this->bOpened)
	{
		return 0;
	}

	if(pWriteSeekProc == nullptr)
	{
		error.Set("pWriteSeekProc not set.");
		return 0;
	}

	return pWriteSeekProc(lOffset, (VLSeekMode)uiMode, this->pUserData);
}

vlBool CProcWriter::Write(vlChar cChar, Diagnostics::CError &error)
{
	if(!this->bOpened)
	{
		return vlFalse;
	}

	if(pWriteWriteProc == nullptr)
	{
		error.Set("pWriteWriteProc not set.");
		return vlFalse;
	}

	vlUInt uiBytesWritten = pWriteWriteProc(&cChar, 1, this->pUserData);

	if(uiBytesWritten == 0)
	{
		error.Set("pWriteWriteProc() failed.");
	}

	return uiBytesWritten == 1;
}

vlUInt CProcWriter::Write(vlVoid *vData, vlUInt uiBytes, Diagnostics::CError &error)
{
	if(!this->bOpened)
	{
		return 0;
	}

	if(pWriteWriteProc == nullptr)
	{
		error.Set("pWriteWriteProc not set.");
		return 0;
	}

	vlUInt uiBytesWritten = pWriteWriteProc(vData, uiBytes, this->pUserData);

	if(uiBytesWritten == 0)
	{
		error.Set("pWriteWriteProc() failed.");
	}

	return uiBytesWritten;
}