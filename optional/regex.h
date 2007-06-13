#ifndef LK_REGEX_H
#define LK_REGEX_H
#include "../vm.h"
#include <sys/types.h>
#include <regex.h>

/* type */
typedef struct lk_regex {
    struct lk_common  co;
    regex_t          *data;
} lk_regex_t;
#define LK_REGEX(v) ((lk_regex_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_regex_extinit);
#endif
