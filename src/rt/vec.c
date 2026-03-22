#include "vec.h"

#pragma region vec

static struct vec_buf *vec_buf_alloc(int item_size, int capacity) {
    struct vec_buf *self;
    int c = 8;

    while (capacity > c)
        c *= 2;
    self = mem_alloc(sizeof(struct vec_buf) + (size_t)item_size * c);
    self->capacity = c;
    self->used = 0;
    self->item_size = item_size;
    self->ref_count = 1;
    return self;
}

static void vec_buf_free(struct vec_buf *self) {
    if (--self->ref_count == 0)
        mem_free(self);
}

vec_t *vec_alloc(int item_size, int capacity) {
    vec_t *self = mem_alloc(sizeof(vec_t));
    vec_init(self, item_size, capacity);
    return self;
}

vec_t *vec_clone(vec_t *self) {
    vec_t *clone = mem_alloc(sizeof(vec_t));
    vec_copy(clone, self);
    return clone;
}

// Shallow copy — shares src's buf (COW: writes trigger a new allocation in vec_prepupdate)
void vec_copy(vec_t *self, vec_t *src) {
    (self->buf = src->buf)->ref_count++;
    self->first = src->first;
    self->length = src->length;
}

void vec_init(vec_t *self, int item_size, int capacity) {
    self->buf = vec_buf_alloc(item_size, capacity);
    self->first = self->buf->items;
    self->length = 0;
}

void vec_fin(vec_t *self) {
    vec_buf_free(self->buf);
}

void vec_free(vec_t *self) {
    vec_buf_free(self->buf);
    mem_free(self);
}

static void vec_prepupdate(vec_t *self, int at, int new_length) {
    struct vec_buf *bd = self->buf;
    int newcap = bd->capacity, ilen = bd->item_size;

    if (new_length > newcap) {
        do {
            newcap *= 2;
        } while (new_length > newcap);

        if (bd->ref_count > 1) {
            bd->ref_count--;
            bd = self->buf = vec_buf_alloc(ilen, newcap);
            memcpy(bd->items, self->first, (size_t)ilen * self->length);
            self->first = bd->items;

        } else {
            ptrdiff_t d = self->first - bd->items;
            bd = self->buf = mem_resize(bd, sizeof(struct vec_buf) + (size_t)ilen * newcap);
            bd->capacity = newcap;
            self->first = bd->items + d;
        }

        // New buffer if replacing an existing item in a shared buffer
    } else if (bd->ref_count > 1 && at < bd->used) {
        bd->ref_count--;
        bd = self->buf = vec_buf_alloc(ilen, newcap);
        memcpy(bd->items, self->first, (size_t)ilen * self->length);
        self->first = bd->items;
    }
    bd->used = (int)(new_length + (self->first - bd->items));
    self->length = new_length;
}

void vec_clear(vec_t *self) {
    vec_limit(self, 0);
}

void vec_concat(vec_t *self, vec_t *other) {
    int sc = self->length, vc = other->length;
    int sl = self->buf->item_size, vl = other->buf->item_size;

    vec_prepupdate(self, sc, sc + vc);

    if (sl == vl)
        memcpy(self->first + (ptrdiff_t)sl * sc, other->first, (size_t)vl * vc);
    else
        NYI("Cannot concat differently sized buffers");
}

#define SETITEM(self, t, i, v) \
    do { \
        *(t *)VEC_AT(self, i) = *(t *)v; \
    } while (0)
#define SETANYITEM(self, i, v) \
    do { \
        switch ((self)->buf->item_size) { \
        case sizeof(char): \
            SETITEM((self), char, (i), (v)); \
            break; \
        case sizeof(short): \
            SETITEM((self), short, (i), (v)); \
            break; \
        case sizeof(long): \
            SETITEM((self), long, (i), (v)); \
            break; \
        default: \
            memmove(VEC_AT(self, i), (v), (self)->buf->item_size); \
            break; \
        } \
    } while (0)

void vec_insert(vec_t *self, int at, void *value) {
    int length = self->length;

    if (at < 0)
        at += length;

    if (at < 0)
        vec_set(self, at - length - 1, value);
    else if (at > length)
        vec_set(self, at, value);
    else {
        int ilen = self->buf->item_size;
        vec_prepupdate(self, at, length + 1);
        memmove(
            self->first + (ptrdiff_t)ilen * (at + 1), self->first + (ptrdiff_t)ilen * at, (size_t)ilen * (length - at));
    }
    SETANYITEM(self, at, value);
}

void vec_limit(vec_t *self, int limit) {
    if (limit < 0)
        limit += self->length;
    if (limit >= 0 && limit < self->length)
        self->length = limit;
}

void vec_offset(vec_t *self, int offset) {
    if (offset < 0)
        offset += self->length;
    if (offset == 0)
        return;

    if (offset < 0 || offset >= self->length)
        self->length = 0;
    else {
        self->first += (ptrdiff_t)offset * self->buf->item_size;
        self->length -= offset;
    }
}

void vec_remove(vec_t *self, int at) {
    int length = self->length;

    if (at < 0)
        at += length;

    if (at < 0 || at >= length)
        return;
    else {
        int ilen = self->buf->item_size, new_length = length - 1;
        vec_prepupdate(self, at, new_length);

        if (at < new_length)
            memmove(self->first + (ptrdiff_t)ilen * at,
                    self->first + (ptrdiff_t)ilen * (at + 1),
                    (size_t)ilen * (new_length - at));
    }
}

void vec_resize(vec_t *self, int length) {
    if (length <= self->length)
        return;
    vec_prepupdate(self, length - 1, length);
}

void vec_resize_item(vec_t *self, vec_t *other) {
    if (self->buf->item_size != other->buf->item_size) {
        NYI("Cannot resize item");
    }
}

#define SWAPITEMS(t) \
    do { \
        t a; \
        t b; \
        for (; i < half; i++) { \
            a = *((t *)from + i); \
            b = *((t *)from + length - i - 1); \
            *((t *)to + i) = b; \
            *((t *)to + length - i - 1) = a; \
        } \
    } while (0)

void vec_reverse(vec_t *self) {
    struct vec_buf *bd = self->buf;
    int ilen = bd->item_size, i = 0, length = self->length, half = (length + 1) / 2;
    char *from = self->first, *to;

    if (bd->ref_count > 1) {
        vec_buf_free(bd);
        bd = self->buf = vec_buf_alloc(ilen, length);
        bd->used = length;
        to = self->first = bd->items;

    } else {
        to = from;
    }

    switch (self->buf->item_size) {
    case sizeof(char):
        SWAPITEMS(char);
        break;
    case sizeof(short):
        SWAPITEMS(short);
        break;
    case sizeof(long):
        SWAPITEMS(long);
        break;
    default: {
        char *a = mem_alloc((size_t)ilen * 2);
        char *b = a + ilen;

        for (; i < half; i++) {
            memcpy(a, from + i, ilen);
            memcpy(b, from + length - i - 1, ilen);
            memcpy(to + i, b, ilen);
            memcpy(to + length - i - 1, a, ilen);
        }

        mem_free(a);
    }
    }
}

void vec_set(vec_t *self, int at, void *value) {
    int length = self->length;

    if (at < 0)
        at += length;

    if (at < 0) {
    } else {
        vec_prepupdate(self, at, at >= length ? at + 1 : length);
    }
    SETANYITEM(self, at, value);
}

void vec_set_range(vec_t *self, int start, int end, vec_t *other) {
    int i, d, sc = self->length, vc = other->length;

    if (start < 0)
        start += sc;
    if (end < 0)
        end += sc;
    d = end - start - vc;

    if (d > 0)
        for (; d > 0; d--)
            vec_remove(self, start);
    else if (d < 0) {
        void *t = NULL;

        for (; d < 0; d++)
            vec_insert(self, start, (void *)&t);
    }

    for (i = 0; i < vc; i++)
        vec_set(self, start++, vec_get(other, i));
}

void vec_slice(vec_t *self, int offset, int limit) {
    vec_offset(self, offset);
    vec_limit(self, limit);
}

int vec_cmp(const vec_t *self, const vec_t *other) {
    if (self == other)
        return 0;
    else {
        int sc = self->length, oc = other->length, d = sc - oc;

        if (self->first == other->first)
            return d;
        else {
            int m = d > 0 ? oc : sc;

            if (self->buf->item_size == other->buf->item_size) {
                int rd = memcmp(self->first, other->first, m);
                return rd == 0 ? d : rd;

            } else {
                abort();
            }
        }
    }
}

int vec_find_vec(const vec_t *self, const vec_t *pattern, int offset) {
    int self_length = self->length, pattern_length = pattern->length;

    if (pattern_length == 0)
        return offset < self_length ? offset : -1;
    if (pattern_length > self_length)
        return -1;
    if (pattern_length == 1)
        return vec_str_find(self, vec_str_get(pattern, 0), offset);
    else {
        switch (self->buf->item_size) {
        case sizeof(uint8_t): {
            // Boyer-Moore-Horspool
            uint8_t *s = (uint8_t *)self->first;
            uint8_t *p = (uint8_t *)pattern->first;
            int i, j, k, skip[256];

            for (k = 0; k < 256; k++)
                skip[k] = pattern_length;

            for (k = 0; k < pattern_length - 1; k++)
                skip[p[k]] = pattern_length - k - 1;

            for (k = offset + pattern_length - 1; k < self_length; k += skip[s[k] & 255]) {
                for (j = pattern_length - 1, i = k; j >= 0 && s[i] == p[j]; j--)
                    i--;

                if (j == -1)
                    return i + 1;
            }
            break;
        }
        default:
            BUG("Invalid item size in vec_find_vec\n");
        }
        return -1;
    }
}

void *vec_get(const vec_t *self, int at) {
    int length = self->length;

    if (at < 0)
        at += length;
    return at < 0 || at >= length ? NULL : VEC_AT(self, at);
}

int vec_hc(const vec_t *self) {
    register const uint8_t *beg = (uint8_t *)self->first;
    register const uint8_t *end = beg + (ptrdiff_t)self->buf->item_size * self->length;
    int hc = 5381;

    for (; beg < end; beg++) {
        hc += hc << 5;
        hc += *beg;
    }
    return hc > 0 ? hc : -hc;
}

#define WRITEITEM(self, stream, t, f) \
    do { \
        t *i = (t *)(self)->first, *end = i + (self)->length; \
        for (; i < end; i++) \
            fprintf((stream), " " f, (t) * i); \
        fprintf((stream), "\n"); \
    } while (0)

void vec_write(const vec_t *self, FILE *stream) {
    struct vec_buf *bd = self->buf;
    int ilen = bd->item_size;
    char *first = self->first;

    fprintf(stream, "vec_t(%p", (void *)self);
    fprintf(stream, ", first=%p", (void *)first);
    fprintf(stream, "(%i)", (int)((first - bd->items) / ilen));
    fprintf(stream, ", length=%i", self->length);
    fprintf(stream, ")\n-> buf(%p", (void *)bd);
    fprintf(stream, ", capacity=%i", bd->capacity);
    fprintf(stream, ", used=%i", bd->used);
    fprintf(stream, ", item_size=%i", ilen);
    fprintf(stream, ", ref_count=%i", bd->ref_count);
    fprintf(stream, ")\n->");

    switch (ilen) {
    case sizeof(char):
        WRITEITEM(self, stream, char, "%hi");
        break;
    case sizeof(short):
        WRITEITEM(self, stream, short, "%hi");
        break;
    case sizeof(long):
        WRITEITEM(self, stream, long, "%li");
        break;
    default:
        WRITEITEM(self, stream, long, "%lx");
        break;
    }
}

void vec_print_tostream(const vec_t *self, FILE *stream) {
    fwrite(self->first, self->buf->item_size, self->length, stream);
}

#pragma endregion
#pragma region string

#define UNTIL(check) \
    do { \
        int i, c = 64; \
        vec_t *self = vec_alloc(sizeof(uint8_t), c); \
        int ch; \
        for (i = 0; (ch = fgetc(stream)) != EOF; i++) { \
            if (i >= c) { \
                do { \
                    c *= 2; \
                } while (i >= c); \
                self->buf = mem_resize(self->buf, sizeof(struct vec_buf) + (size_t)self->buf->item_size * c); \
                self->buf->capacity = (c); \
                self->first = (self)->buf->items; \
            } \
            ((char *)self->first)[i] = ch; \
            if ((check)) \
                break; \
        } \
        self->length = self->buf->used = ch == EOF ? i : i + 1; \
        return self; \
    } while (0)

vec_t *vec_str_alloc(void) {
    return vec_alloc(sizeof(uint8_t), 10);
}

vec_t *vec_str_alloc_from_cstr(const char *cstr) {
    return vec_str_alloc_from_data(cstr, (int)strlen(cstr));
}

vec_t *vec_str_alloc_from_data(const void *data, int length) {
    vec_t *self = vec_alloc(sizeof(uint8_t), length);
    memcpy(self->first, data, length);
    self->buf->used = self->length = length;
    return self;
}

vec_t *vec_str_alloc_from_file(FILE *stream) {
    if (fseek(stream, 0, SEEK_END) == 0) {
        long size;

        if ((size = ftell(stream)) >= 0 && fseek(stream, 0, SEEK_SET) == 0) {
            vec_t *self = vec_alloc(sizeof(uint8_t), (int)size);
            fread(self->first, 1, size, stream);
            self->length = self->buf->used = (int)size;
            return self;
        }
    }
    return NULL;
}

vec_t *vec_str_alloc_from_file_until_char(FILE *stream, uint32_t pattern) {
    if (feof(stream))
        return NULL;
    UNTIL((uint32_t)ch == pattern);
}

vec_t *vec_str_alloc_from_file_with_length(FILE *stream, size_t length) {
    vec_t *self = vec_alloc(1, (int)length);
    self->length = self->buf->used = (int)fread(self->first, 1, length, stream);
    return self;
}

vec_t *vec_str_alloc_from_file_until_charset(FILE *stream, const charset_t *pattern) {
    if (feof(stream))
        return NULL;
    UNTIL(charset_has(pattern, ch));
}

#define INSERTUINT(self, t, i, v) \
    do { \
        t nv = (v); \
        vec_insert(self, (i), (void *)&nv); \
    } while (0);

void vec_str_insert(vec_t *self, int at, uint32_t value) {
    switch (self->buf->item_size) {
    case sizeof(uint8_t):
        INSERTUINT(self, uint8_t, at, value);
        break;
    case sizeof(uint16_t):
        INSERTUINT(self, uint16_t, at, value);
        break;
    case sizeof(uint32_t):
        INSERTUINT(self, uint32_t, at, value);
        break;
    default:
        BUG("Invalid item size in vec_str_insert\n");
    }
}

uint32_t vec_str_pop(vec_t *self) {
    return vec_str_remove(self, self->length - 1);
}

void vec_str_push(vec_t *self, uint32_t value) {
    vec_str_set(self, self->length, value);
}

uint32_t vec_str_remove(vec_t *self, int at) {
    uint32_t v = vec_str_get(self, at);
    vec_remove(self, at);
    return v;
}

#define SETUINT(self, t, i, v) \
    do { \
        t nv = (v); \
        vec_set(self, (i), (void *)&nv); \
    } while (0);

void vec_str_set(vec_t *self, int at, uint32_t value) {
    switch (self->buf->item_size) {
    case sizeof(uint8_t):
        SETUINT(self, uint8_t, at, value);
        break;
    case sizeof(uint16_t):
        SETUINT(self, uint16_t, at, value);
        break;
    case sizeof(uint32_t):
        SETUINT(self, uint32_t, at, value);
        break;
    default:
        BUG("Invalid item size in vec_str_set\n");
    }
}

uint32_t vec_str_peek(vec_t *self) {
    return vec_str_get((self), (self)->length - 1);
}

const char *vec_str_tocstr(vec_t *self) {
    vec_str_set(self, self->length, '\0');
    self->length--;
    return self->first;
}

#define COMPARECSTRING(type) \
    do { \
        for (; sb < sbend; sb += ilen, other++) { \
            cd = *(type *)sb - *other; \
            if (cd != 0) \
                return cd; \
        } \
    } while (0)

int vec_str_cmp_cstr(const vec_t *self, const char *other) {
    int sc = self->length, oc = (int)strlen(other), d = sc - oc;
    int length = d > 0 ? oc : sc;
    int ilen = self->buf->item_size;
    uint8_t *sb = (uint8_t *)self->first, *sbend = sb + (ptrdiff_t)length * ilen;
    int cd;

    if (ilen == sizeof(uint8_t))
        COMPARECSTRING(uint8_t);
    else if (ilen == sizeof(uint16_t))
        COMPARECSTRING(uint16_t);
    else if (ilen == sizeof(uint32_t))
        COMPARECSTRING(uint32_t);
    else
        BUG("Invalid item size in vec_str_cmp_cstr\n");
    return d;
}

#define MATCHCHAR(type) \
    do { \
        for (; offset < c; offset++) \
            if (((type *)buf)[offset] == pattern) \
                return offset; \
    } while (0)

int vec_str_find(const vec_t *self, uint32_t pattern, int offset) {
    void *buf = self->first;
    int c = self->length;

    switch (self->buf->item_size) {
    case sizeof(uint8_t):
        MATCHCHAR(uint8_t);
        break;
    case sizeof(uint16_t):
        MATCHCHAR(uint16_t);
        break;
    case sizeof(uint32_t):
        MATCHCHAR(uint32_t);
        break;
    default:
        BUG("Invalid item size in vec_str_find\n");
    }
    return -1;
}

uint32_t vec_str_get(const vec_t *self, int at) {
    void *vptr = vec_get(self, at);

    if (vptr == NULL)
        return 0;
    switch (self->buf->item_size) {
    case sizeof(uint8_t):
        return *(uint8_t *)vptr;
    case sizeof(uint16_t):
        return *(uint16_t *)vptr;
    case sizeof(uint32_t):
        return *(uint32_t *)vptr;
    default:
        BUG("Invalid item size in vec_str_get\n");
    }
}

#define MATCHCHARSET(type) \
    do { \
        for (; offset < c; offset++) \
            if (charset_has(pattern, ((type *)buf)[offset])) \
                return offset; \
    } while (0)

int vec_str_find_charset(const vec_t *self, const charset_t *pattern, int offset) {
    void *buf = self->first;
    int c = self->length;

    switch (self->buf->item_size) {
    case sizeof(uint8_t):
        MATCHCHARSET(uint8_t);
        break;
    case sizeof(uint16_t):
        MATCHCHARSET(uint16_t);
        break;
    case sizeof(uint32_t):
        MATCHCHARSET(uint32_t);
        break;
    default:
        BUG("Invalid array item length!");
    }
    return -1;
}

#pragma endregion
#pragma region ptr list

void vec_ptr_init(vec_t *self) {
    vec_init(self, sizeof(void *), 10);
}

vec_t *vec_ptr_alloc(void) {
    return vec_ptr_alloc_with_capacity(10);
}

vec_t *vec_ptr_alloc_with_capacity(int capacity) {
    return vec_alloc(sizeof(void *), capacity);
}

void vec_ptr_insert(vec_t *self, int at, void *value) {
    vec_insert(self, at, (void *)&value);
}

void *vec_ptr_pop(vec_t *self) {
    return vec_ptr_remove(self, self->length - 1);
}

void vec_ptr_push(vec_t *self, void *value) {
    vec_ptr_set(self, self->length, value);
}

void *vec_ptr_remove(vec_t *self, int at) {
    void *item = vec_ptr_get(self, at);
    vec_remove(self, at);
    return item;
}

void vec_ptr_set(vec_t *self, int at, void *value) {
    vec_set(self, at, (void *)&value);
}

void *vec_ptr_shift(vec_t *self) {
    return vec_ptr_remove(self, 0);
}

void vec_ptr_unshift(vec_t *self, void *value) {
    vec_ptr_insert(self, 0, value);
}

void *vec_ptr_peek(vec_t *self) {
    return vec_ptr_get((self), (self)->length - 1);
}

void *vec_ptr_get(const vec_t *self, int at) {
    void **vptr = (void **)vec_get(self, at);
    return vptr != NULL ? *vptr : NULL;
}

#pragma endregion
