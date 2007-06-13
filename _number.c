#include "_number.h"

/* new. */
pt_numbertype_t pt_number_new(int is_big, pt_string_t *str, pt_numberifn_t *res) {
    static char underscore = '_' - '0', dot = '.' - '0';
    char digit = '\0';
    int i;
    double f, div = 1;
    int idx = 0, len = PT_LIST_COUNT(str);

    /* try the number as native int. */
    i = 0;
    for(; idx < len; idx ++) {
        digit = pt_list_getuchar(str, idx) - '0';
        if(digit == underscore) continue;
        if(digit < 0 || digit > 9) break;
        if(i > INT_MAX / 10) break; else i *= 10;
        if(i > INT_MAX - digit) break; else i += digit;
    }
    if(idx >= len) {
        res->i = i;
        return PT_NUMBERTYPE_INT;
    }

    /* try the number as native float/double. */
    if(is_big) goto bignum;
    f = i;
    if(digit == dot) goto point;
    for(; idx < len; idx ++) {
        digit = pt_list_getuchar(str, idx) - '0';
        if(digit == underscore) continue;
        if(digit < 0 || digit > 9) break;
        if(f > DBL_MAX / 10) break; else f *= 10;
        if(f > DBL_MAX - digit) break; else f += digit;
    }
    if(digit == dot) {
        point:
            for(idx ++; idx < len; idx ++) {
                digit = pt_list_getuchar(str, idx) - '0';
                if(digit == underscore) continue;
                if(digit < 0 || digit > 9) break;
                if(f > DBL_MAX / 10) { idx = len; break; } else f *= 10;
                if(f > DBL_MAX - digit) { idx = len; break; } else f += digit;
                div *= 10;
            }
    }
    if(idx >= len) {
        res->f = f / div;
        return PT_NUMBERTYPE_FLOAT;
    }

    /* try the number with supplied bignum lib. */
    bignum:
    NOIMPL("No supported big number library.\n");
}
void pt_number_free(pt_number_t *self) {
    pt_memory_free(self);
}
