/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#ifndef VMTFILE_H
#define VMTFILE_H

#include "vtflib_shared.h"
#include "Readers.h"
#include "Writers.h"
#include "VMTNodes.h"

#ifdef __cplusplus
extern "C" {
#endif

//! VMT parsing mode.
typedef enum tagVMTParseMode
{
	PARSE_MODE_STRICT = 0,
	PARSE_MODE_LOOSE,
	PARSE_MODE_COUNT
} VMTParseMode;

#ifdef __cplusplus
}
#endif

namespace VTFLib
{
	class VTFLIB_API CVMTFile
	{
	private:
		Nodes::CVMTGroupNode *Root;

		vlUInt ParseErrorLine;

	public:
		CVMTFile();
		CVMTFile(const CVMTFile &VMTFile);
		~CVMTFile();

	public:
		vlBool Create(const vlChar *cRoot);
		vlVoid Destroy();

		vlBool IsLoaded() const;

		vlBool Load(const vlChar *cFileName, Diagnostics::CError &error);
		vlBool Load(const vlVoid *lpData, vlUInt uiBufferSize, Diagnostics::CError& error);
		vlBool Load(vlVoid *pUserData, Diagnostics::CError& error);

		vlBool Save(const vlChar *cFileName, Diagnostics::CError& error) const;
		vlBool Save(vlVoid *lpData, vlUInt uiBufferSize, vlUInt &uiSize, Diagnostics::CError& error) const;
		vlBool Save(vlVoid *pUserData, Diagnostics::CError& error) const;

	private:
		vlBool Load(IO::Readers::IReader *Reader, Diagnostics::CError &error);
		vlBool Save(IO::Writers::IWriter *Writer, Diagnostics::CError &error) const;

		//Nodes::CVMTNode *Load(IO::Readers::IReader *Reader, vlBool bInGroup);

		vlVoid Indent(IO::Writers::IWriter *Writer, vlUInt uiLevel, VTFLib::Diagnostics::CError& error) const;
		vlVoid Save(IO::Writers::IWriter *Writer, Nodes::CVMTNode *Node, VTFLib::Diagnostics::CError& error, vlUInt uiLevel = 0) const;

	public:
		Nodes::CVMTGroupNode *GetRoot() const;

		vlUInt GetParseErrorLine() const;
	};
}

#endif