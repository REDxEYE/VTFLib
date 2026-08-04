/*
 * VTFLib
 * Copyright (C) 2026 ficool2
 *
 * BC7 encode/decode support
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#ifndef VTFBC7_H
#define VTFBC7_H

#include "stdafx.h"

namespace VTFLib
{
	namespace BC7
	{
		//! Decompress a BC7 image to RGBA8888.
		vlBool Decompress(const vlByte *lpSource, vlByte *lpDest, vlUInt uiWidth, vlUInt uiHeight);

		//! Compress an RGBA8888 image to BC7.
		vlBool Compress(const vlByte *lpSource, vlByte *lpDest, vlUInt uiWidth, vlUInt uiHeight, vlSingle sQuality);
	}
}

#endif
