/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#ifndef FILEREADER_H
#define FILEREADER_H

#include "vtflib_shared.h"
#include "Reader.h"

namespace VTFLib
{
	namespace IO
	{
		namespace Readers
		{
			class CFileReader : public IReader
			{
			private:
				HANDLE hFile;
				vlChar *cFileName;

			public:
				CFileReader(const vlChar *cFileName);
				~CFileReader();

			public:
				virtual vlBool Opened() const;

				virtual vlBool Open(Diagnostics::CError &error);
				virtual vlVoid Close();

				virtual vlUInt GetStreamSize(Diagnostics::CError &error) const;
				virtual vlUInt GetStreamPointer(Diagnostics::CError &error) const;

				virtual vlUInt Seek(vlLong lOffset, vlUInt uiMode, Diagnostics::CError &error);

				virtual vlBool Read(vlChar &cChar, Diagnostics::CError &error);
				virtual vlUInt Read(vlVoid *vData, vlUInt uiBytes, Diagnostics::CError &error);
			};
		}
	}
}

#endif