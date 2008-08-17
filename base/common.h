#ifndef COMMON_H
#define COMMON_H
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* common fatal errors */
#define ERR(message) do { fprintf(stderr, (message)); abort(); } while(0)
#define NYI(message) ERR("NYI: " message)
#define BUG(message) ERR("BUG: " message)

/* memory management */
#include "memory.h"
#endif
