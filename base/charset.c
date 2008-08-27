#include "charset.h"
#define CHARSET_DEFAULTCAP 8

/* new */
charset_t *charset_alloc(void) {
    return mem_alloc(sizeof(charset_t));
}
charset_t *charset_clone(charset_t *self) {
    charset_t *clone = charset_alloc();
    charset_copy(clone, self);
    return clone;
}
void charset_copy(charset_t *self, charset_t *from) {
    memcpy(self, from, sizeof(charset_t));
    self->data = mem_alloc(sizeof(uint32_t) * self->cap);
    memcpy(CHARSET_DATA(self), CHARSET_DATA(from), sizeof(uint32_t) * self->cap);
    if(CHARSET_ISINVERTED(from)) charset_invert(self);
}
void charset_fin(charset_t *self) {
    mem_free(CHARSET_DATA(self));
}
void charset_free(charset_t *self) {
    charset_fin(self);
    mem_free(self);
}
void charset_init(charset_t *self) {
    self->min = UINT32_MAX;
    self->max = 0;
    self->cap = CHARSET_DEFAULTCAP;
    self->size = 0;
    self->data = mem_alloc(sizeof(uint32_t) * self->cap);
}
charset_t *charset_new(void) {
    charset_t *self = charset_alloc();
    charset_init(self);
    return self;
}

/* update */
/* double the internal storage space when we run out */
static void resize(charset_t *self) {
    int isinverted = CHARSET_ISINVERTED(self), newcap = self->cap;
    while(self->size >= newcap) newcap *= 2;
    if(newcap > self->cap) {
        self->cap = newcap;
        self->data = mem_resize(CHARSET_DATA(self), sizeof(uint32_t) * newcap);
        if(isinverted) charset_invert(self);
    }
}

/* */
#define GET(self, index) (CHARSET_DATA(self)[(index)])
#define SET(self, index, value) (CHARSET_DATA(self)[(index)] = (value))
#define INSERT(self, i, v) \
    do { \
        resize(self); \
        memmove(CHARSET_DATA(self) + (i) + 1, CHARSET_DATA(self) + (i), sizeof(uint32_t) * ((self)->size - (i))); \
        SET(self, i, v); \
        (self)->size ++; \
    } while(0)
#define PUSH(self, v) \
    do { \
        resize(self); \
        SET(self, (self)->size, v); \
        (self)->size ++; \
    } while(0)
#define REMOVE(self, i) \
    do { \
        memmove(CHARSET_DATA(self) + (i), CHARSET_DATA(self) + (i) + 1, sizeof(uint32_t) * ((self)->size - (i))); \
        (self)->size --; \
    } while(0)

static void charset_insert(charset_t *self, uint32_t from, uint32_t to) {
    int i = 0;
    uint32_t cf = UINT32_MAX, ct = 0;

    /* swap if from and to is reversed */
    if(from > to) {
        uint32_t t = from;
        from = to;
        to = t;
    }

    /* where to start? */
    for(; i < self->size; i += 2) {
        ct = GET(self, i + 1);
        if(from <= ct + 1) {
            cf = GET(self, i);
            break;
        }
    }
    if(i < self->size) {
        if(from < cf) {

            /* in between ranges */
            INSERT(self, i, to);
            INSERT(self, i, from);
        } else if(to > ct) {

            /* extend current */
            SET(self, i + 1, to);
        }

        /* charset_remove all ranges within new */
        for(i += 2; i < self->size;) {
            ct = GET(self, i + 1);
            if(to <= ct) {
                cf = GET(self, i);
                break;
            }
            REMOVE(self, i);
            REMOVE(self, i);
        }

        /* charset_remove extra range */
        if(from < cf && cf <= to + 1 && to <= ct) {
            REMOVE(self, i);
            REMOVE(self, i);
            SET(self, i - 1, ct);
        }
    } else {
        PUSH(self, from);
        PUSH(self, to);
    }

    /* reset charset min and max */
    if(from < self->min) self->min = from;
    if(to > self->max) self->max = to;
}
static void charset_remove(charset_t *self, uint32_t from, uint32_t to) {

    /* swap if from and to is reversed */
    if(from > to) {
        uint32_t t = from;
        from = to;
        to = t;
    }
    if(self->size > 0 && to >= self->min && from <= self->max) {
        int i = 0;
        uint32_t cf = UINT32_MAX, ct = 0;

        /* where to start? */
        for(; i < self->size; i += 2) {
            ct = GET(self, i + 1);
            if(from <= ct) {
                cf = GET(self, i);
                break;
            }
        }

        /* truncate current (right) */
        if(cf < from && from <= ct) {
            SET(self, i + 1, from - 1);
            i += 2;
            if(cf <= to && to < ct) {
                INSERT(self, i, ct);
                INSERT(self, i, to + 1);
            }
        }

        /* charset_remove all ranges within spec */
        for(; i < self->size;) {
            ct = GET(self, i + 1);
            if(to <= ct) {
                cf = GET(self, i);
                break;
            }
            REMOVE(self, i);
            REMOVE(self, i);
        }

        /* truncate current (left) */
        if(cf <= to && to <= ct) {
            SET(self, i, to + 1);
        }
    }
}

/* add specified range of chars to the charset */
void charset_add_chars(charset_t *self, uint32_t from, uint32_t to) {
    (CHARSET_ISINVERTED(self) ? charset_remove : charset_insert)(self, from, to);
}

/* add another charset to this charset */
void charset_add_charset(charset_t *self, charset_t *other) {
    uint32_t f, t;
    uint32_t *c = CHARSET_DATA(other), *last = c + other->size;
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        (CHARSET_ISINVERTED(self) ? charset_remove : charset_insert)(self, f, t);
    }
}

/* add all the chars in the specified str to this charset */
void charset_add_darray(charset_t *self, darray_t *str) {
    int c = str->size;
    if(c > 0) {
        int i = 0;
        uint32_t v = darray_getuchar(str, i);
        while(i < c) {
            v = darray_getuchar(str, i ++);
            (CHARSET_ISINVERTED(self) ? charset_remove : charset_insert)(self, v, i < c && darray_getuchar(str, i) == '-' ? darray_getuchar(str, (i += 2) - 1) : v);
        }
    }
}

/* charset_remove all chars from the charset */
void charset_clear(charset_t *self) {
    self->data = CHARSET_DATA(self);
    self->max = 0;
    self->min = UINT32_MAX;
    self->size = 0;
}

void charset_invert(charset_t *self) {
    self->data = CHARSET_ISINVERTED(self) ? CHARSET_DATA(self) : (uint32_t *)((ptrdiff_t)self->data | 1);
}

/* charset_remove specified range of chars from the charset */
void charset_subtract_chars(charset_t *self, uint32_t from, uint32_t to) {
    (CHARSET_ISINVERTED(self) ? charset_insert : charset_remove)(self, from, to);
}

/* charset_remove another charset from this charset */
void charset_subtract_charset(charset_t *self, charset_t *other) {
    uint32_t f, t;
    uint32_t *c = CHARSET_DATA(other), *last = c + other->size;
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        (CHARSET_ISINVERTED(self) ? charset_insert : charset_remove)(self, f, t);
    }
}

/* charset_remove all the chars in the specified str from this charset */
void charset_subtract_darray(charset_t *self, darray_t *str) {
    int c = str->size;
    if(c > 0) {
        int i = 0;
        uint32_t v = darray_getuchar(str, i);
        while(i < c) {
            v = darray_getuchar(str, i ++);
            (CHARSET_ISINVERTED(self) ? charset_insert : charset_remove)(self, v, i < c && darray_getuchar(str, i) == '-' ? darray_getuchar(str, (i += 2) - 1) : v);
        }
    }
}

/* info */
/* check to see if the charset contains the specified char */
int charset_has(const charset_t *self, uint32_t achar) {
    if(self->min <= achar && achar <= self->max) {
        uint32_t from, to;
        uint32_t *curr = CHARSET_DATA(self), *last = curr + self->size;
        for(; curr < last; ) {
            from = *curr ++;
            to = *curr ++;
            if(from <= achar && achar <= to) {
                return 1 ^ CHARSET_ISINVERTED(self);
            }
        }
    }
    return 0 ^ CHARSET_ISINVERTED(self);
}

/* print the contents of charset struct - useful for debugging */
void charset_inspect(const charset_t *self, FILE *output) {
    uint32_t f, t;
    uint32_t *c = CHARSET_DATA(self), *last = c + self->size;
    fprintf(output, "charset_t(%p", (void *)self);
    fprintf(output, ", min=%i", self->min);
    fprintf(output, ", max=%i", self->max);
    fprintf(output, ", cap=%i", self->cap);
    fprintf(output, ", size=%i", self->size);
    fprintf(output, ")\n-> ");
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        if(f == t) fprintf(output, "%c", (char)f);
        else fprintf(output, "%c-%c", (char)f, (char)t);
        if(c < last) fprintf(output, ", ");
    }
}

/* get the size of the charset */
int charset_size(const charset_t *self) {
    int total = 0;
    uint32_t from, to;
    uint32_t *curr = CHARSET_DATA(self), *last = curr + self->size;
    for(; curr < last; ) {
        from = *curr ++;
        to = *curr ++;
        total += to - from + 1;
    }
    return total;
}

/* str representation of the str */
darray_t *charset_tostr(const charset_t *self) {
    darray_t *str = darray_alloc_str();
    uint32_t from, to;
    uint32_t *curr = CHARSET_DATA(self), *last = curr + self->size;
    if(CHARSET_ISINVERTED(self)) darray_setuchar(str, str->size, '^');
    for(; curr < last; ) {
        from = *curr ++;
        to = *curr ++;
        if(from == to) darray_setuchar(str, str->size, from);
        else {
            darray_setuchar(str, str->size, from);
            darray_setuchar(str, str->size, '-');
            darray_setuchar(str, str->size, to);
        }
    }
    return str;
}
