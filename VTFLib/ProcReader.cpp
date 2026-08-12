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

using namespace VTFLib;
using namespace VTFLib::IO::Readers;

CProcReader::CProcReader(vlVoid *pUserData)
{
	this->bOpened = vlFalse;
	this->pUserData = pUserData;
}

CProcReader::~CProcReader()
{
	this->Close();
}

vlBool CProcReader::Opened() const
{
	return this->bOpened;
}

vlBool CProcReader::Open(Diagnostics::CError &error)
{
	this->Close();

	if(pReadOpenProc == nullptr)
	{
		error.Set("pReadOpenProc not set.");
		return vlFalse;
	}

	if(this->bOpened)
	{
		error.Set("Reader already open.");
		return vlFalse;
	}

	if(!pReadOpenProc(this->pUserData))
	{
		error.Set("Error opening file.");
		return vlFalse;
	}

	this->bOpened = vlTrue;

	return vlTrue;
}

vlVoid CProcReader::Close()
{
	if(pReadCloseProc == nullptr)
	{
		return;
	}

	if(this->bOpened)
	{
		pReadCloseProc(this->pUserData);
		this->bOpened = vlFalse;
	}
}

vlUInt CProcReader::GetStreamSize(Diagnostics::CError& error) const
{
	if(!this->bOpened)
	{
		return 0;
	}

	if(pReadSizeProc == nullptr)
	{
		error.Set("pReadSizeProc not set.");
		return 0xffffffff;
	}

	return pReadSizeProc(this->pUserData);
}

vlUInt CProcReader::GetStreamPointer(Diagnostics::CError& error) const
{
	if(!this->bOpened)
	{
		return 0;
	}

	if(pReadTellProc == nullptr)
	{
		error.Set("pReadTellProc not set.");
		return 0;
	}

	return pReadTellProc(this->pUserData);
}

vlUInt CProcReader::Seek(vlLong lOffset, vlUInt uiMode, Diagnostics::CError &error)
{
	if(!this->bOpened)
	{
		return 0;
	}

	if(pReadSeekProc == nullptr)
	{
		error.Set("pReadSeekProc not set.");
		return 0;
	}

	return pReadSeekProc(lOffset, (VLSeekMode)uiMode, this->pUserData);
}

vlBool CProcReader::Read(vlChar &cChar, Diagnostics::CError &error)
{
	if(!this->bOpened)
	{
		return vlFalse;
	}

	if(pReadReadProc == nullptr)
	{
		error.Set("pReadReadProc not set.");
		return vlFalse;
	}

	vlUInt uiBytesRead = pReadReadProc(&cChar, 1, this->pUserData);

	if(uiBytesRead == 0)
	{
		error.Set("pReadReadProc() failed.");
	}

	return uiBytesRead == 1;
}

vlUInt CProcReader::Read(vlVoid *vData, vlUInt uiBytes, Diagnostics::CError &error)
{
	if(!this->bOpened)
	{
		return 0;
	}

	if(pReadReadProc == nullptr)
	{
		error.Set("pReadReadProc not set.");
		return 0;
	}

	vlUInt uiBytesRead = pReadReadProc(vData, uiBytes, this->pUserData);

	if(uiBytesRead == 0)
	{
		error.Set("pReadReadProc() failed.");
	}

	return uiBytesRead;
}