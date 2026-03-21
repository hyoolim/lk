#include "num.h"

num_type_t num_new(darray_t *str, num_t *res) {
    int len = DARRAY_COUNT(str);
    int idx = 0;
    int i = 0;
    int digit = 0;

    // Try as native int
    for (; idx < len; idx++) {
        digit = (int)darray_str_get(str, idx) - '0';

        if (digit == '_' - '0')
            continue;

        if (digit < 0 || digit > 9)
            break;

        if (i > INT_MAX / 10 || (i == INT_MAX / 10 && digit > INT_MAX % 10))
            break;

        i = i * 10 + digit;
    }

    if (idx >= len) {
        res->i = i;
        return NUM_TYPE_INT;
    }

    // Try as native double — digit holds the char that broke the int loop
    double f = (double)i;
    double divisor = 1.0;

    if (digit != '.' - '0') {
        for (; idx < len; idx++) {
            digit = (int)darray_str_get(str, idx) - '0';

            if (digit == '_' - '0')
                continue;

            if (digit < 0 || digit > 9)
                break;

            if (f > DBL_MAX / 10) {
                idx = len;
                break;
            }

            f *= 10;

            if (f > DBL_MAX - digit) {
                idx = len;
                break;
            }

            f += digit;
        }
    }

    if (digit == '.' - '0') {
        for (idx++; idx < len; idx++) {
            digit = (int)darray_str_get(str, idx) - '0';

            if (digit == '_' - '0')
                continue;

            if (digit < 0 || digit > 9)
                break;

            if (f > DBL_MAX / 10) {
                idx = len;
                break;
            }

            f *= 10;

            if (f > DBL_MAX - digit) {
                idx = len;
                break;
            }

            f += digit;
            divisor *= 10;
        }
    }

    if (idx >= len) {
        res->d = f / divisor;
        return NUM_TYPE_DOUBLE;
    }

    BUG("Unparseable number literal.\n");
}
