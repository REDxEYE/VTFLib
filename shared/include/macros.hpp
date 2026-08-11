//
// Created by red_eye on 8/10/26.
//

#pragma once

#if defined(_WIN32)
#	if defined(VTFLIB_STATIC)
#		define VTFLIB_API
#	elif defined(VTFLIB_EXPORTS)
#		define VTFLIB_API __declspec(dllexport)
#	else
#		define VTFLIB_API __declspec(dllimport)
#	endif

#   if _MSC_VER >= 1600 // Visual Studio 2010
#	    define STATIC_ASSERT(condition, message) static_assert(condition, message)
#   else
#	    define STATIC_ASSERT(condition, message) typedef char __C_ASSERT__[(condition) ? 1 : -1]
#   endif

#	define VL_ALIGN(n) __declspec(align(n))

#else
#	define VTFLIB_API __attribute__((visibility("default")))


#if defined(__GNUC__) || defined(__clang__)
#	define VL_ALIGN(n) __attribute__((aligned(n)))
#else
#	define VL_ALIGN(n)
#endif

#define STATIC_ASSERT(condition, message) static_assert(condition, message)

#endif

