#include "charset.h"

/* new */
charset_t *charset_alloc(void) {
    return memory_alloc(sizeof(charset_t));
}
charset_t *charset_clone(charset_t *self) {
    charset_t *clone = charset_alloc();
    charset_copy(clone, self);
    return clone;
}
void charset_copy(charset_t *self, charset_t *from) {
    memcpy(self, from, sizeof(charset_t));
    self->data = memory_alloc(sizeof(uint32_t) * self->capa);
    memcpy(self->data, from->data, sizeof(uint32_t) * self->capa);
}
void charset_fin(charset_t *self) {
    memory_free(self->data);
}
void charset_free(charset_t *self) {
    charset_fin(self);
    memory_free(self);
}
void charset_init(charset_t *self) {
    self->min = UINT32_MAX;
    self->max = 0;
    self->capa = CSET_DEFAULTCAPA;
    self->count = 0;
    self->data = memory_alloc(sizeof(uint32_t) * self->capa);
}
charset_t *charset_new(void) {
    charset_t *self = charset_alloc();
    charset_init(self);
    return self;
}

/* update */
static void charset_resize(charset_t *self) {
    int capa = self->capa, c = self->count;
    while(c >= capa) capa *= 2;
    if(capa > self->capa) self->data = memory_resize(
    self->data, sizeof(uint32_t) * (self->capa = capa));
}
void charset_clear(charset_t *self) {
    self->min = UINT32_MAX;
    self->max = 0;
    self->count = 0;
}
#define GET(self, i) ((self)->data[(i)])
#define SET(self, i, v) ((self)->data[(i)] = (v))
#define INSERT(self, i, v) do { \
    charset_resize(self); \
    memmove(self->data + (i) + 1, self->data + (i), \
    sizeof(uint32_t) * ((self)->count - (i))); \
    SET(self, i, v); \
    (self)->count ++; \
} while(0)
#define PUSH(self, v) do { \
    charset_resize(self); \
    SET(self, (self)->count, v); \
    (self)->count ++; \
} while(0)
#define REMOVE(self, i) do { \
    memmove(self->data + (i), self->data + (i) + 1, \
    sizeof(uint32_t) * ((self)->count - (i))); \
    (self)->count --; \
} while(0)
static void charset_insert(charset_t *self, uint32_t from, uint32_t to) {
    int i = 0;
    uint32_t cf = UINT32_MAX, ct = 0;
    if(from > to) { uint32_t t = from; from = to; to = t; }
    /* where to start? */
    for(; i < self->count; i += 2) {
        ct = GET(self, i + 1);
        if(from <= ct + 1) {
            cf = GET(self, i);
            break;
        }
    }
    if(i < self->count) {
        /* in between ranges */
        if(from < cf) {
            INSERT(self, i, to);
            INSERT(self, i, from);
        /* extend current */
        } else if(to > ct) {
            SET(self, i + 1, to);
        }
        /* remove all ranges within new */
        for(i += 2; i < self->count;) {
            ct = GET(self, i + 1);
            if(to <= ct) {
                cf = GET(self, i);
                break;
            }
            REMOVE(self, i);
            REMOVE(self, i);
        }
        /* remove extra range */
        if(from < cf && cf <= to + 1 && to <= ct) {
            REMOVE(self, i);
            REMOVE(self, i);
            SET(self, i - 1, ct);
        }
    } else {
        PUSH(self, from);
        PUSH(self, to);
    }
    /* reset charset min/max */
    if(from < self->min) self->min = from;
    if(to > self->max) self->max = to;
}
static void charset_remove(charset_t *self, uint32_t from, uint32_t to) {
    if(from > to) { uint32_t t = from; from = to; to = t; }
    if(self->count > 0 && to >= self->min && from <= self->max) {
        int i = 0;
        uint32_t cf = UINT32_MAX, ct = 0;
        /* where to start? */
        for(; i < self->count; i += 2) {
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
        /* remove all ranges within spec */
        for(; i < self->count;) {
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
void charset_addRange(charset_t *self, uint32_t from, uint32_t to) {
    (self->isinverted ? charset_remove : charset_insert)(self, from, to);
}
void charset_subtractRange(charset_t *self, uint32_t from, uint32_t to) {
    (self->isinverted ? charset_insert : charset_remove)(self, from, to);
}
void charset_add(charset_t *self, charset_t *other) {
    uint32_t f, t;
    uint32_t *c = other->data, *last = c + other->count;
    void (*func)(charset_t *, uint32_t, uint32_t
    ) = self->isinverted ? charset_remove : charset_insert;
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        func(self, f, t);
    }
}
void charset_subtract(charset_t *self, charset_t *other) {
    uint32_t f, t;
    uint32_t *c = other->data, *last = c + other->count;
    void (*func)(charset_t *, uint32_t, uint32_t
    ) = self->isinverted ? charset_insert : charset_remove;
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        func(self, f, t);
    }
}
void charset_addArray(charset_t *self, array_t *str) {
    int c = str->count;
    if(c > 0) {
        int i = 0;
        uint32_t v = array_getuchar(str, i);
        void (*func)(charset_t *, uint32_t, uint32_t
        ) = self->isinverted ? charset_remove : charset_insert;
        /* if(v == '^') { self->isinverted = 1; i ++; } */
        while(i < c) {
            v = array_getuchar(str, i ++);
            func(self, v, i < c && array_getuchar(str, i) == '-'
            ? array_getuchar(str, (i += 2) - 1) : v);
        }
    }
}
void charset_subtractArray(charset_t *self, array_t *str) {
    int c = str->count;
    if(c > 0) {
        int i = 0;
        uint32_t v = array_getuchar(str, i);
        void (*func)(charset_t *, uint32_t, uint32_t
        ) = self->isinverted ? charset_insert : charset_remove;
        /* if(v == '^') { self->isinverted = 1; i ++; } */
        while(i < c) {
            v = array_getuchar(str, i ++);
            func(self, v, i < c && array_getuchar(str, i) == '-'
            ? array_getuchar(str, (i += 2) - 1) : v);
        }
    }
}

/* info */
int charset_count(const charset_t *self) {
    int total = 0;
    uint32_t f, t;
    uint32_t *c = self->data, *last = c + self->count;
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        total += t - f + 1;
    }
    return total;
}
void charset_print(const charset_t *self, FILE *stream) {
    uint32_t f, t;
    uint32_t *c = self->data, *last = c + self->count;
    fprintf(stream, "charset_t(%p", (void *)self);
    fprintf(stream, ", min=%i", self->min);
    fprintf(stream, ", max=%i", self->max);
    fprintf(stream, ", capa=%i", self->capa);
    fprintf(stream, ", count=%i", self->count);
    fprintf(stream, ")\n-> ");
    for(; c < last; ) {
        f = *c ++;
        t = *c ++;
        if(f == t) fprintf(stream, "%c", (char)f);
        else fprintf(stream, "%c-%c", (char)f, (char)t);
        if(c < last) fprintf(stream, ", ");
    }
}
int charset_has(const charset_t *self, uint32_t n) {
    if(self->min <= n && n <= self->max) {
        uint32_t f, t;
        uint32_t *c = self->data, *last = c + self->count;
        for(; c < last; ) {
            f = *c ++;
            t = *c ++;
            if(f <= n && n <= t) return 1 ^ self->isinverted;
        }
    }
    return 0 ^ self->isinverted;
}
