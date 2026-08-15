/*
 * VTFEdit
 * Copyright (C) 2005-2026 ficool2, Neil Jedrzejewski & Ryan Gregg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include "VTFLib.h"

// DevIL switches ILstring between char * and wchar_t * on _UNICODE
// which Qt defines for the whole target
#ifdef _UNICODE
#	define VTFEDIT_UNICODE_WAS_DEFINED
#	undef _UNICODE
#endif

#ifdef VTFEDIT_UNICODE_WAS_DEFINED
#	define _UNICODE
#	undef VTFEDIT_UNICODE_WAS_DEFINED
#endif

// fix windows header conflicts
#ifdef CreateDirectory
#	undef CreateDirectory
#endif
#ifdef GetObject
#	undef GetObject
#endif
#ifdef GetTempPath
#	undef GetTempPath
#endif
#ifdef MessageBox
#	undef MessageBox
#endif
