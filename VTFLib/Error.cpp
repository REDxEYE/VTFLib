/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "Error.h"

using namespace VTFLib::Diagnostics;

CError::CError()
{
	this->mErrorMessage = nullptr;
}

CError::~CError()
{
	delete[] this->mErrorMessage;
}

void CError::Clear()
{
	delete[] this->mErrorMessage;
	this->mErrorMessage = nullptr;
}

const char *CError::Get() const
{
	return this->mErrorMessage != nullptr ? this->mErrorMessage : "";
}

void CError::SetFormatted(const char *format, ...)
{
	char cBuffer[2048];

	va_list ArgumentList;
	va_start(ArgumentList, format);
	vsprintf(cBuffer, format, ArgumentList);
	va_end(ArgumentList);

	this->Set(cBuffer, vlFalse);
}

void CError::Set(const char *errorMessage, const bool systemError)
{
	char buffer[2048];

	if(systemError)
	{
#ifdef _WIN32
		const char* message = NULL;
		uint32_t lastError = GetLastError();

		if(FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			lastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPSTR>(&message),
			0,
			NULL))
		{
			snprintf(
				buffer,
				sizeof(buffer),
				"%s\n\nSystem Error: 0x%.8x:\n%s",
				errorMessage,
				lastError,
				message
			);

			LocalFree(message);
		}
		else
		{
			snprintf(
				buffer,
				sizeof(buffer),
				"%s\n\nSystem Error: 0x%.8x.",
				errorMessage,
				lastError
			);
		}
#else
		const int errorId = errno;

		snprintf(
			buffer,
			sizeof(buffer),
			"%s\n\nSystem Error: %d:\n%s",
			errorMessage,
			errorId,
			strerror(errorId)
		);
#endif
	}
	else
	{
		snprintf(buffer, sizeof(buffer), "%s", errorMessage);
	}

	delete[] this->mErrorMessage;

	this->mErrorMessage = new char[strlen(buffer) + 1];
	strcpy(this->mErrorMessage, buffer);
}

bool CError::isSet() const {
	return this->mErrorMessage != nullptr;
}
