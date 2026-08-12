/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#ifndef WRITER_H
#define WRITER_H

#include "vtflib_shared.h"

namespace VTFLib
{
	namespace IO
	{
		namespace Writers
		{
			class IWriter
			{
			public:
				virtual vlBool Opened() const = 0;

				virtual vlBool Open(Diagnostics::CError &error) = 0;
				virtual vlVoid Close() = 0;

				virtual vlUInt GetStreamSize(Diagnostics::CError &error) const = 0;
				virtual vlUInt GetStreamPointer(Diagnostics::CError &error) const = 0;

				virtual vlUInt Seek(vlLong lOffset, vlUInt uiMode, Diagnostics::CError &error) = 0;

				virtual vlBool Write(vlChar cChar, Diagnostics::CError &error) = 0;
				virtual vlUInt Write(vlVoid *vData, vlUInt uiBytes, Diagnostics::CError &error) = 0;
			};
		}
	}
}

#endif