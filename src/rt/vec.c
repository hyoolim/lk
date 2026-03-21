#include "vec.h"

#pragma region vec

static struct vec_buf *vec_buf_alloc(int ilen, int cap) {
    struct vec_buf *self;
    int c = 8;

    while (cap > c)
        c *= 2;
    self = mem_alloc(sizeof(struct vec_buf) + ilen * c);
    self->capacity = c;
    self->used = 0;
    self->item_size = ilen;
    self->ref_count = 1;
    return self;
}

static void vec_buf_free(struct vec_buf *self) {
    if (--self->ref_count == 0)
        mem_free(self);
}

vec_t *vec_alloc(int ilen, int cap) {
    vec_t *self = mem_alloc(sizeof(vec_t));
    vec_init(self, ilen, cap);
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
    self->size = src->size;
}

void vec_init(vec_t *self, int ilen, int cap) {
    self->buf = vec_buf_alloc(ilen, cap);
    self->first = self->buf->items;
    self->size = 0;
}

void vec_fin(vec_t *self) {
    vec_buf_free(self->buf);
}

void vec_free(vec_t *self) {
    vec_buf_free(self->buf);
    mem_free(self);
}

static void vec_prepupdate(vec_t *self, int i, int newsize) {
    struct vec_buf *bd = self->buf;
    int newcap = bd->capacity, ilen = bd->item_size;

    if (newsize > newcap) {
        do {
            newcap *= 2;
        } while (newsize > newcap);

        if (bd->ref_count > 1) {
            bd->ref_count--;
            bd = self->buf = vec_buf_alloc(ilen, newcap);
            memcpy(bd->items, self->first, ilen * self->size);
            self->first = bd->items;

        } else {
            ptrdiff_t d = self->first - bd->items;
            bd = self->buf = mem_resize(bd, sizeof(struct vec_buf) - sizeof(char) + ilen * newcap);
            bd->capacity = newcap;
            self->first = bd->items + d;
        }

        // new buf if replacing existing item in shared buf
    } else if (bd->ref_count > 1 && i < bd->used) {
        bd->ref_count--;
        bd = self->buf = vec_buf_alloc(ilen, newcap);
        memcpy(bd->items, self->first, ilen * self->size);
        self->first = bd->items;
    }
    bd->used = newsize + (self->first - bd->items);
    self->size = newsize;
}

void vec_clear(vec_t *self) {
    vec_limit(self, 0);
}

void vec_concat(vec_t *self, vec_t *v) {
    int sc = self->size, vc = v->size;
    int sl = self->buf->item_size, vl = v->buf->item_size;

    vec_prepupdate(self, sc, sc + vc);

    if (sl == vl)
        memcpy(self->first + sl * sc, v->first, vl * vc);
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

void vec_insert(vec_t *self, int i, void *v) {
    int size = self->size;

    if (i < 0)
        i += size;

    if (i < 0)
        vec_set(self, i - size - 1, v);
    else if (i > size)
        vec_set(self, i, v);
    else {
        int ilen = self->buf->item_size;
        vec_prepupdate(self, i, size + 1);
        memmove(self->first + ilen * (i + 1), self->first + ilen * i, ilen * (size - i));
    }
    SETANYITEM(self, i, v);
}

void vec_limit(vec_t *self, int n) {
    if (n < 0)
        n += self->size;
    if (n >= 0 && n < self->size)
        self->size = n;
}

void vec_offset(vec_t *self, int n) {
    if (n < 0)
        n += self->size;
    if (n == 0)
        return;

    if (n < 0 || n >= self->size)
        self->size = 0;
    else {
        self->first += n * self->buf->item_size;
        self->size -= n;
    }
}

void vec_remove(vec_t *self, int i) {
    int size = self->size;

    if (i < 0)
        i += size;

    if (i < 0 || i >= size)
        return;
    else {
        int ilen = self->buf->item_size, newsize = size - 1;
        vec_prepupdate(self, i, newsize);

        if (i < newsize)
            memmove(self->first + ilen * i, self->first + ilen * (i + 1), ilen * (newsize - i));
    }
}

void vec_resize(vec_t *self, int s) {
    if (s <= self->size)
        return;
    vec_prepupdate(self, s - 1, s);
}

void vec_resizeitem(vec_t *self, vec_t *other) {
    if (self->buf->item_size != other->buf->item_size) {
        NYI("Cannot resize item");
    }
}

#define SWAPITEMS(t) \
    do { \
        t a; \
        t b; \
        for (; i < c2; i++) { \
            a = *((t *)from + i); \
            b = *((t *)from + c - i - 1); \
            *((t *)to + i) = b; \
            *((t *)to + c - i - 1) = a; \
        } \
    } while (0)

void vec_reverse(vec_t *self) {
    struct vec_buf *bd = self->buf;
    int ilen = bd->item_size, i = 0, c = self->size, c2 = (c + 1) / 2;
    char *from = self->first, *to;

    if (bd->ref_count > 1) {
        vec_buf_free(bd);
        bd = self->buf = vec_buf_alloc(ilen, c);
        bd->used = c;
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
        char *a = mem_alloc(ilen * 2);
        char *b = a + ilen;

        for (; i < c2; i++) {
            memcpy(a, from + i, ilen);
            memcpy(b, from + c - i - 1, ilen);
            memcpy(to + i, b, ilen);
            memcpy(to + c - i - 1, a, ilen);
        }

        mem_free(a);
    }
    }
}

void vec_set(vec_t *self, int i, void *v) {
    int size = self->size;

    if (i < 0)
        i += size;

    if (i < 0) {
    } else {
        vec_prepupdate(self, i, i >= size ? i + 1 : size);
    }
    SETANYITEM(self, i, v);
}

void vec_setrange(vec_t *self, int b, int e, vec_t *v) {
    int i, d, sc = self->size, vc = v->size;

    if (b < 0)
        b += sc;
    if (e < 0)
        e += sc;
    d = e - b - vc;

    if (d > 0)
        for (; d > 0; d--)
            vec_remove(self, b);
    else if (d < 0) {
        void *t = NULL;

        for (; d < 0; d++)
            vec_insert(self, b, (void *)&t);
    }

    for (i = 0; i < vc; i++)
        vec_set(self, b++, vec_get(v, i));
}

void vec_slice(vec_t *self, int offset, int limit) {
    vec_offset(self, offset);
    vec_limit(self, limit);
}

int vec_cmp(const vec_t *self, const vec_t *other) {
    if (self == other)
        return 0;
    else {
        int sc = self->size, oc = other->size, d = sc - oc;

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

int vec_find_vec(const vec_t *self, const vec_t *pat, int o) {
    int self_c = self->size, pat_c = pat->size;

    if (pat_c == 0)
        return o < self_c ? o : -1;
    if (pat_c > self_c)
        return -1;
    if (pat_c == 1)
        return vec_str_find(self, vec_str_get(pat, 0), o);
    else {
        switch (self->buf->item_size) {
        case sizeof(uint8_t): {
            // Boyer-Moore-Horspool
            uint8_t *s = (uint8_t *)self->first;
            uint8_t *p = (uint8_t *)pat->first;
            int i, j, k, skip[256];

            for (k = 0; k < 256; k++)
                skip[k] = pat_c;

            for (k = 0; k < pat_c - 1; k++)
                skip[p[k]] = pat_c - k - 1;

            for (k = o + pat_c - 1; k < self_c; k += skip[s[k] & 255]) {
                for (j = pat_c - 1, i = k; j >= 0 && s[i] == p[j]; j--)
                    i--;

                if (j == -1)
                    return i + 1;
            }
            break;
        }
        default:
            BUG("Invalid ilen in vec_find_vec\n");
        }
        return -1;
    }
}

void *vec_get(const vec_t *self, int i) {
    int size = self->size;

    if (i < 0)
        i += size;
    return i < 0 || i >= size ? NULL : VEC_AT(self, i);
}

int vec_hc(const vec_t *self) {
    register const uint8_t *beg = (uint8_t *)self->first;
    register const uint8_t *end = beg + self->buf->item_size * self->size;
    int hc = 5381;

    for (; beg < end; beg++) {
        hc += hc << 5;
        hc += *beg;
    }
    return hc > 0 ? hc : -hc;
}

#define WRITEITEM(self, stream, t, f) \
    do { \
        t *i = (t *)(self)->first, *end = i + (self)->size; \
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
    fprintf(stream, ", size=%i", self->size);
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
    fwrite(self->first, self->buf->item_size, self->size, stream);
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
                self->buf = mem_resize(self->buf, sizeof(struct vec_buf) + self->buf->item_size * c); \
                self->buf->capacity = (c); \
                self->first = (self)->buf->items; \
            } \
            ((char *)self->first)[i] = ch; \
            if ((check)) \
                break; \
        } \
        self->size = self->buf->used = ch == EOF ? i : i + 1; \
        return self; \
    } while (0)

vec_t *vec_str_alloc(void) {
    return vec_alloc(sizeof(uint8_t), 10);
}

vec_t *vec_str_alloc_fromcstr(const char *cstr) {
    return vec_str_alloc_fromdata(cstr, strlen(cstr));
}

vec_t *vec_str_alloc_fromdata(const void *data, int len) {
    vec_t *self = vec_alloc(sizeof(uint8_t), len);
    memcpy(self->first, data, len);
    self->buf->used = self->size = len;
    return self;
}

vec_t *vec_str_alloc_fromfile(FILE *stream) {
    if (fseek(stream, 0, SEEK_END) == 0) {
        long size;

        if ((size = ftell(stream)) >= 0 && fseek(stream, 0, SEEK_SET) == 0) {
            vec_t *self = vec_alloc(sizeof(uint8_t), size);
            fread(self->first, 1, size, stream);
            self->size = self->buf->used = size;
            return self;
        }
    }
    return NULL;
}

vec_t *vec_str_alloc_fromfile_untilchar(FILE *stream, uint32_t pat) {
    if (feof(stream))
        return NULL;
    UNTIL((uint32_t)ch == pat);
}

vec_t *vec_str_alloc_fromfile_withsize(FILE *stream, size_t rs) {
    vec_t *self = vec_alloc(1, rs);
    self->size = self->buf->used = fread(self->first, 1, rs, stream);
    return self;
}

vec_t *vec_str_alloc_fromfile_untilcharset(FILE *stream, const charset_t *pat) {
    if (feof(stream))
        return NULL;
    UNTIL(charset_has(pat, ch));
}

#define INSERTUINT(self, t, i, v) \
    do { \
        t nv = (v); \
        vec_insert(self, (i), (void *)&nv); \
    } while (0);

void vec_str_insert(vec_t *self, int i, uint32_t v) {
    switch (self->buf->item_size) {
    case sizeof(uint8_t):
        INSERTUINT(self, uint8_t, i, v);
        break;
    case sizeof(uint16_t):
        INSERTUINT(self, uint16_t, i, v);
        break;
    case sizeof(uint32_t):
        INSERTUINT(self, uint32_t, i, v);
        break;
    default:
        BUG("Invalid ilen in vec_str_insert\n");
    }
}

uint32_t vec_str_pop(vec_t *self) {
    return vec_str_remove(self, self->size - 1);
}

void vec_str_push(vec_t *self, uint32_t v) {
    vec_str_set(self, self->size, v);
}

uint32_t vec_str_remove(vec_t *self, int i) {
    uint32_t v = vec_str_get(self, i);
    vec_remove(self, i);
    return v;
}

#define SETUINT(self, t, i, v) \
    do { \
        t nv = (v); \
        vec_set(self, (i), (void *)&nv); \
    } while (0);

void vec_str_set(vec_t *self, int i, uint32_t v) {
    switch (self->buf->item_size) {
    case sizeof(uint8_t):
        SETUINT(self, uint8_t, i, v);
        break;
    case sizeof(uint16_t):
        SETUINT(self, uint16_t, i, v);
        break;
    case sizeof(uint32_t):
        SETUINT(self, uint32_t, i, v);
        break;
    default:
        BUG("Invalid ilen in vec_str_set\n");
    }
}

uint32_t vec_str_peek(vec_t *self) {
    return vec_str_get((self), (self)->size - 1);
}

const char *vec_str_tocstr(vec_t *self) {
    vec_str_set(self, self->size, '\0');
    self->size--;
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
    int sc = self->size, oc = strlen(other), d = sc - oc;
    int len = d > 0 ? oc : sc;
    uint8_t ilen = self->buf->item_size;
    uint8_t *sb = (uint8_t *)self->first, *sbend = sb + len * ilen;
    int cd;

    if (ilen == sizeof(uint8_t))
        COMPARECSTRING(uint8_t);
    else if (ilen == sizeof(uint16_t))
        COMPARECSTRING(uint16_t);
    else if (ilen == sizeof(uint32_t))
        COMPARECSTRING(uint32_t);
    else
        BUG("Invalid ilen in vec_str_cmp_cstr\n");
    return d;
}

#define MATCHCHAR(type) \
    do { \
        for (; o < c; o++) \
            if (((type *)buf)[o] == pat) \
                return o; \
    } while (0)

int vec_str_find(const vec_t *self, uint32_t pat, int o) {
    void *buf = self->first;
    int c = self->size;

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
        BUG("Invalid ilen in vec_str_find\n");
    }
    return -1;
}

uint32_t vec_str_get(const vec_t *self, int i) {
    void *vptr = vec_get(self, i);

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
        BUG("Invalid ilen in vec_str_get\n");
    }
}

#define MATCHCHARSET(type) \
    do { \
        for (; o < c; o++) \
            if (charset_has(pat, ((type *)buf)[o])) \
                return o; \
    } while (0)

int vec_str_findset(const vec_t *self, const charset_t *pat, int o) {
    void *buf = self->first;
    int c = self->size;

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
    return vec_ptr_alloc_withcap(10);
}

vec_t *vec_ptr_alloc_withcap(int cap) {
    return vec_alloc(sizeof(void *), cap);
}

void vec_ptr_insert(vec_t *self, int i, void *v) {
    vec_insert(self, i, (void *)&v);
}

void *vec_ptr_pop(vec_t *self) {
    return vec_ptr_remove(self, self->size - 1);
}

void vec_ptr_push(vec_t *self, void *v) {
    vec_ptr_set(self, self->size, v);
}

void *vec_ptr_remove(vec_t *self, int i) {
    void *item = vec_ptr_get(self, i);
    vec_remove(self, i);
    return item;
}

void vec_ptr_set(vec_t *self, int i, void *v) {
    vec_set(self, i, (void *)&v);
}

void *vec_ptr_shift(vec_t *self) {
    return vec_ptr_remove(self, 0);
}

void vec_ptr_unshift(vec_t *self, void *v) {
    vec_ptr_insert(self, 0, v);
}

void *vec_ptr_peek(vec_t *self) {
    return vec_ptr_get((self), (self)->size - 1);
}

void *vec_ptr_get(const vec_t *self, int i) {
    void **vptr = (void **)vec_get(self, i);
    return vptr != NULL ? *vptr : NULL;
}

#pragma endregion
