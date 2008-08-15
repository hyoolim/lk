#include "number.h"

/* new. */
numbertype_t number_new(int is_big, list_t *str, numberifn_t *res) {
    static char underscore = '_' - '0', dot = '.' - '0';
    char digit = '\0';
    int i;
    double f, div = 1;
    int idx = 0, len = LIST_COUNT(str);

    /* try the number as native int. */
    i = 0;
    for(; idx < len; idx ++) {
        digit = list_getuchar(str, idx) - '0';
        if(digit == underscore) continue;
        if(digit < 0 || digit > 9) break;
        if(i > INT_MAX / 10) break; else i *= 10;
        if(i > INT_MAX - digit) break; else i += digit;
    }
    if(idx >= len) {
        res->i = i;
        return NUMBERTYPE_INT;
    }

    /* try the number as native float/double. */
    if(is_big) goto bignum;
    f = i;
    if(digit == dot) goto point;
    for(; idx < len; idx ++) {
        digit = list_getuchar(str, idx) - '0';
        if(digit == underscore) continue;
        if(digit < 0 || digit > 9) break;
        if(f > DBL_MAX / 10) break; else f *= 10;
        if(f > DBL_MAX - digit) break; else f += digit;
    }
    if(digit == dot) {
        point:
            for(idx ++; idx < len; idx ++) {
                digit = list_getuchar(str, idx) - '0';
                if(digit == underscore) continue;
                if(digit < 0 || digit > 9) break;
                if(f > DBL_MAX / 10) { idx = len; break; } else f *= 10;
                if(f > DBL_MAX - digit) { idx = len; break; } else f += digit;
                div *= 10;
            }
    }
    if(idx >= len) {
        res->f = f / div;
        return NUMBERTYPE_FLOAT;
    }

    /* try the number with supplied bignum lib. */
    bignum:
    NOIMPL("No supported big number library.\n");
}
void number_free(number_t *self) {
    memory_free(self);
}
