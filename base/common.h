#ifndef COMMON_H
#define COMMON_H
#include <assert.h>
#include <limits.h>
#include <float.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* common fatal errors */
#define ERROR(cstr) do { fprintf(stderr, (cstr)); abort(); } while(0)
#define NOIMPL(cstr) ERROR("Not implemented: " cstr)
#define BUG(cstr) ERROR("Bug: " cstr)
#define MEMORYERROR(cstr) ERROR("Memory error: " cstr)

/* memory management */
#include "memory.h"
#endif
