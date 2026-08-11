//
// Created by red_eye on 8/10/26.
//

#pragma once

#if !(WIN32)
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <cassert>
#include <cctype>
typedef char *LPSTR;
#define stricmp strcasecmp
#define _stricmp strcasecmp
typedef FILE *HANDLE;

#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
#endif
