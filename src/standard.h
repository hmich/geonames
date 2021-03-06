#pragma once

/* Standard header files and portability macroses. */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
#define snprintf _snprintf
#endif

#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
