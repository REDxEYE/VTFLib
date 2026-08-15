/*
 * VTFLib
 * Copyright (C) 2005-2011 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

// ============================================================
// NOTE: This file is commented for compatibility with Doxygen.
// ============================================================
/*!
	\file stdafx.h
	\brief Application framework header plus VTFLib custom data types.
*/

#pragma once

#include "polyfill.hpp"
#include "macros.hpp"
#include <cstdint>

#if defined(_WIN32)
#	if defined(VTFLIB_STATIC)
#		define VTFLIB_API
#	elif defined(VTFLIB_EXPORTS)
#		define VTFLIB_API __declspec(dllexport)
#	else
#		define VTFLIB_API __declspec(dllimport)
#	endif
#else
#	define VTFLIB_API __attribute__((visibility("default")))
#endif

// Custom data types
typedef unsigned char	vlBool;				//!< Boolean value 0/1.


#define vlFalse			0
#define vlTrue			1

#if _MSC_VER >= 1400
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#	ifndef _CRT_NONSTDC_NO_DEPRECATE
#		define _CRT_NONSTDC_NO_DEPRECATE
#	endif
#endif

#if WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdarg.h>

#include <BaseTsd.h>
using ssize_t = SSIZE_T;

#else
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <cassert>
#include <cctype>
typedef char*  LPSTR;
#define stricmp strcasecmp
#define _stricmp strcasecmp
typedef FILE *HANDLE;

#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
#endif
