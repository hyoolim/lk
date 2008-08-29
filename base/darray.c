#include "darray.h"

/* new */
static struct listdata *listdata_alloc(int ilen, int cap) {
    struct listdata *self;
    int c = 8;
    while(cap > c) c *= 2;
    self = mem_alloc(sizeof(struct listdata) - sizeof(char) + ilen * c);
    self->cap = c;
    self->used = 0;
    self->ilen = ilen;
    self->refc = 1;
    return self;
}
static void listdata_free(struct listdata *self) {
    if(self->refc > 0) self->refc --;
    if(self->refc < 1) mem_free(self);
}
darray_t *darray_alloc(int ilen, int cap) {
    darray_t *self = mem_alloc(sizeof(darray_t));
    darray_init(self, ilen, cap);
    return self;
}
darray_t *darray_allocptr(void) {
    return darray_allocptrwithcap(10);
}
darray_t *darray_allocptrwithcap(int cap) {
    return darray_alloc(sizeof(void *), cap);
}
darray_t *darray_allocfromfile(FILE *stream, size_t rs) {
    darray_t *self = darray_alloc(1, rs);
    self->size =
    self->data->used = fread(self->first, 1, rs, stream);
    return self;
}
darray_t *darray_alloc_str(void) {
    return darray_alloc(sizeof(uint8_t), 10);
}
darray_t *darray_clone(darray_t *self) {
    darray_t *clone = mem_alloc(sizeof(darray_t));
    darray_copy(clone, self);
    return clone;
}
darray_t *darray_alloc_fromdata(const void *data, int len) {
    darray_t *self = darray_alloc(sizeof(uint8_t), len);
    memcpy(self->first, data, len);
    self->data->used = self->size = len;
    return self;
}
darray_t *darray_allocFromCString(const char *cstr) {
    return darray_alloc_fromdata(cstr, strlen(cstr));
}
darray_t *str_allocfromfile(FILE *stream) {
    if(fseek(stream, 0, SEEK_END) == 0) {
        long size;
        if((size = ftell(stream)) >= 0
        && fseek(stream, 0, SEEK_SET) == 0) {
            darray_t *self = darray_alloc(sizeof(uint8_t), size);
            fread(self->first, 1, size, stream);
            self->size = self->data->used = size;
            return self;
    }   }
    return NULL;
}
#define UNTIL(check) do { \
    int i, c = 64; \
    darray_t *self = darray_alloc(sizeof(uint8_t), c); \
    int ch; \
    for(i = 0; (ch = fgetc(stream)) != EOF; i ++) { \
        if(i >= c) { \
            do { c *= 2; } while(i >= c); \
            self->data = mem_resize(self->data, \
            sizeof(struct listdata) - sizeof(char) \
            + self->data->ilen * c); \
            self->data->cap = (c); \
            self->first = &(self)->data->item; \
        } \
        ((char *)self->first)[i] = ch; \
        if((check)) break; \
    } \
    self->size = self->data->used = i + 1; \
    return self; \
} while(0)
darray_t *darray_alloc_fromfile_untilchar(FILE *stream, uint32_t pat) {
    if(feof(stream)) return NULL;
    UNTIL(ch == pat);
}
darray_t *darray_alloc_fromfile_untilcharset(FILE *stream, const charset_t *pat) {
    if(feof(stream)) return NULL;
    UNTIL(charset_has(pat, ch));
}
void darray_init(darray_t *self, int ilen, int cap) {
    self->data = listdata_alloc(ilen, cap);
    self->first = &self->data->item;
    self->size = 0;
}
void darray_initptr(darray_t *self) {
    darray_init(self, sizeof(void *), 10);
}
void darray_copy(darray_t *self, darray_t *src) {
    (self->data = src->data)->refc ++;
    self->first = src->first;
    self->size = src->size;
}
void darray_fin(darray_t *self) {
    listdata_free(self->data);
}
void darray_free(darray_t *self) {
    listdata_free(self->data);
    mem_free(self);
}

/* update */
#define SETITEM(self, t, i, v) do { \
    *(t *)LIST_AT(self, i) = *(t *)v; \
} while(0)
#define SETANYITEM(self, i, v) do { \
    switch((self)->data->ilen) { \
    case sizeof(char ): SETITEM((self), char,  (i), (v)); break; \
    case sizeof(short): SETITEM((self), short, (i), (v)); break; \
    case sizeof(long ): SETITEM((self), long,  (i), (v)); break; \
    default: memmove(LIST_AT( \
    self, i), (v), (self)->data->ilen); break; \
    } \
} while(0)
static void darray_prepupdate(darray_t *self, int i, int newsize) {
    struct listdata *bd = self->data;
    int newcap = bd->cap, ilen = bd->ilen;
    if(newsize > newcap) {
        do { newcap *= 2; } while(newsize > newcap);
        /* buf shared? alloc new */
        if(bd->refc > 1) {
            newlistdata:
            bd->refc --;
            bd = self->data = listdata_alloc(ilen, newcap);
            memcpy(&bd->item, self->first, ilen * self->size);
            self->first = &bd->item;
        /* resize existing buf */
        } else {
            ptrdiff_t d = self->first - &bd->item;
            bd = self->data = mem_resize(bd,
            sizeof(struct listdata) - sizeof(char) + ilen * newcap);
            bd->cap = newcap;
            self->first = &bd->item + d;
        }
    /* new buf if replacing existing item in shared buf */
    } else if(bd->refc > 1 && i < bd->used) {
        goto newlistdata;
    }
    bd->used = newsize + (self->first - &bd->item);
    self->size = newsize;
}
void darray_clear(darray_t *self) {
    darray_limit(self, 0);
}
void darray_concat(darray_t *self, darray_t *v) {
    int sc = self->size, vc = v->size;
    int sl = self->data->ilen, vl = self->data->ilen;
    darray_prepupdate(self, sc, sc + vc);
    if(sl == vl) memcpy(self->first + sl * sc, v->first, vl * vc);
    else NYI("Cannot concat differently sized buffers");
}
void darray_insert(darray_t *self, int i, void *v) {
    int size = self->size;
    if(i < 0) i += size;
    if(i < 0) darray_set(self, i - size - 1, v);
    else if(i > size) darray_set(self, i, v);
    else {
        int ilen = self->data->ilen;
        darray_prepupdate(self, i, size + 1);
        memmove(self->first + ilen * (i + 1),
        self->first + ilen * i, ilen * (size - i));
    }
    SETANYITEM(self, i, v);
}
void darray_insertptr(darray_t *self, int i, void *v) {
    darray_insert(self, i, (void *)&v);
}
#define INSERTUINT(self, t, i, v) do { \
    t nv = (v); \
    darray_insert(self, (i), (void *)&nv); \
} while(0);
void darray_insertuchar(darray_t *self, int i, uint32_t v) {
    switch(self->data->ilen) {
    case sizeof(uint8_t ): INSERTUINT(self, uint8_t,  i, v); break;
    case sizeof(uint16_t): INSERTUINT(self, uint16_t, i, v); break;
    case sizeof(uint32_t): INSERTUINT(self, uint32_t, i, v); break;
    default: BUG("Invalid ilen in darray_insertuchar\n");
    }
}
void darray_limit(darray_t *self, int n) {
    if(n < 0) n += self->size;
    if(n >= 0 && n < self->size) self->size = n;
}
void darray_offset(darray_t *self, int n) {
    if(n < 0) n += self->size;
    if(n == 0) return;
    if(n < 0 || n >= self->size) self->size = 0;
    else {
        self->first += n * self->data->ilen;
        self->size -= n;
    }
}
void *darray_peekptr(darray_t *self) {
    return darray_getptr((self), (self)->size - 1);
}
uint32_t darray_peekuchar(darray_t *self) {
    return darray_getuchar((self), (self)->size - 1);
}
void *darray_popptr(darray_t *self) {
    return darray_removeptr(self, self->size - 1);
}
uint32_t darray_popuchar(darray_t *self) {
    return darray_removeuchar(self, self->size - 1);
}
void darray_pushptr(darray_t *self, void *v) {
    darray_setptr(self, self->size, v);
}
void darray_pushuchar(darray_t *self, uint32_t v) {
    darray_setuchar(self, self->size, v);
}
void darray_remove(darray_t *self, int i) {
    int size = self->size;
    if(i < 0) i += size;
    if(i < 0 || i >= size) return;
    else {
        int ilen = self->data->ilen, newsize = size - 1;
        darray_prepupdate(self, i, newsize);
        if(i < newsize) memmove(self->first + ilen * i,
        self->first + ilen * (i + 1), ilen * (size - i));
    }
}
void *darray_removeptr(darray_t *self, int i) {
    void *item = darray_getptr(self, i);
    darray_remove(self, i);
    return item;
}
uint32_t darray_removeuchar(darray_t *self, int i) {
    uint32_t v = darray_getuchar(self, i);
    darray_remove(self, i);
    return v;
}
void darray_resize(darray_t *self, int s) {
    if(s <= self->size) return;
    darray_prepupdate(self, s - 1, s);
}
void darray_resizeitem(darray_t *self, darray_t *other) {
    if(self->data->ilen != other->data->ilen) {
        NYI("Cannot resize item");
    }
}
#define SWAPITEMS(t) do { \
    t a; \
    t b; \
    for(; i < c2; i ++) { \
        a = *((t *)from + i); \
        b = *((t *)from + c - i - 1); \
        *((t *)to + i) = b; \
        *((t *)to + c - i - 1) = a; \
    } \
} while(0)
void darray_reverse(darray_t *self) {
    struct listdata *bd = self->data;
    int ilen = bd->ilen, i = 0, c = self->size, c2 = (c + 1) / 2;
    char *from = self->first, *to;
    if(bd->refc > 1) {
        listdata_free(bd);
        bd = self->data = listdata_alloc(ilen, c);
        bd->used = c;
        to = self->first = &bd->item;
    } else {
        to = from;
    }
    switch(self->data->ilen) {
    case sizeof(char ): SWAPITEMS(char ); break;
    case sizeof(short): SWAPITEMS(short); break;
    case sizeof(long ): SWAPITEMS(long ); break;
    default: {
        char *a = mem_alloc(ilen * 2);
        char *b = a + ilen;
        for(; i < c2; i ++) {
            memcpy(a, from + i, ilen);
            memcpy(b, from + c - i - 1, ilen);
            memcpy(to + i, b, ilen);
            memcpy(to + c - i - 1, a, ilen);
        }
        mem_free(a);
    }
    }
}
void darray_set(darray_t *self, int i, void *v) {
    int size = self->size;
    if(i < 0) i += size;
    if(i < 0) {
    } else {
        darray_prepupdate(self, i, i >= size ? i + 1 : size);
    }
    SETANYITEM(self, i, v);
}
void darray_setptr(darray_t *self, int i, void *v) {
    darray_set(self, i, (void *)&v);
}
void darray_setrange(darray_t *self, int b, int e, darray_t *v) {
    int i, d, sc = self->size, vc = v->size;
    if(b < 0) b += sc;
    if(e < 0) e += vc;
    d = e - b - vc;
    if(d > 0) for(; d > 0; d --) darray_remove(self, b);
    else if(d < 0) {
        void *t = NULL;
        for(; d < 0; d ++) darray_insert(self, b, (void *)&t);
    }
    for(i = 0; i < vc; i ++) darray_set(self, b ++, darray_get(v, i));
}
#define SETUINT(self, t, i, v) do { \
    t nv = (v); \
    darray_set(self, (i), (void *)&nv); \
} while(0);
void darray_setuchar(darray_t *self, int i, uint32_t v) {
    switch(self->data->ilen) {
    case sizeof(uint8_t ): SETUINT(self, uint8_t,  i, v); break;
    case sizeof(uint16_t): SETUINT(self, uint16_t, i, v); break;
    case sizeof(uint32_t): SETUINT(self, uint32_t, i, v); break;
    default: BUG("Invalid ilen in darray_setuchar\n");
    }
}
void *darray_shiftptr(darray_t *self) {
    return darray_removeptr(self, 0);
}
void darray_slice(darray_t *self, int offset, int limit) {
    darray_offset(self, offset);
    darray_limit(self, limit);
}
const char *darray_tocstr(darray_t *self) {
    darray_setuchar(self, self->size, '\0');
    self->size --;
    return self->first;
}
void darray_unshiftptr(darray_t *self, void *v) {
    darray_insertptr(self, 0, v);
}

/* info */
int darray_compareTo(const darray_t *self, const darray_t *other) {
    if(self == other) return 0;
    else {
        int sc = self->size, oc = other->size, d = sc - oc;
        if(self->first == other->first) return d;
        else {
            int m = d > 0 ? oc : sc;
            if(self->data->ilen == other->data->ilen) {
                int rd = memcmp(self->first, other->first, m);
                return rd == 0 ? d : rd;
            } else {
                /*
                printf("Trying to compare strs"
                " with different item length\n");
                printf("self=%p(%i) '", (void *)self, self->data->ilen);
                darray_print_tostream(self, stdout); printf("'\n");
                printf("other=%p(%i) '", (void *)other, other->data->ilen);
                darray_print_tostream(other, stdout); printf("'\n");
                 */
                abort();
    }   }   }
}
#define COMPARECSTRING(type) do { \
    for(; sb < sbend; sb += ilen, other ++) { \
        cd = *(type *)sb - *other; \
        if(cd != 0) return cd; \
    } \
} while(0)
int darray_cmp_tocstr(const darray_t *self, const char *other) {
    int sc = self->size, oc = strlen(other), d = sc - oc;
    int len = d > 0 ? oc : sc;
    uint8_t ilen = self->data->ilen;
    uint8_t *sb = (uint8_t *)self->first, *sbend = sb + len * ilen;
    int cd;
    if(     ilen == sizeof(uint8_t )) COMPARECSTRING(uint8_t );
    else if(ilen == sizeof(uint16_t)) COMPARECSTRING(uint16_t);
    else if(ilen == sizeof(uint32_t)) COMPARECSTRING(uint16_t);
    else BUG("Invalid ilen in darray_cmp_tocstr\n");
    return d;
}
#define MATCHCHAR(type) do { \
    for(; o < c; o ++) if(((type *)buf)[o] == pat) return o; \
} while(0)
int darray_find_char(const darray_t *self, uint32_t pat, int o) {
    void *buf = self->first;
    int c = self->size;
    switch(self->data->ilen) {
    case sizeof(uint8_t ): MATCHCHAR(uint8_t ); break;
    case sizeof(uint16_t): MATCHCHAR(uint16_t); break;
    case sizeof(uint32_t): MATCHCHAR(uint32_t); break;
    default: BUG("Invalid ilen in darray_find_char\n");
    }
    return -1;
}
#define MATCHCHARSET(type) do { \
    for(; o < c; o ++) if(charset_has(pat, ((type *)buf)[o])) return o; \
} while(0)
int darray_find_charset(const darray_t *self, const charset_t *pat, int o) {
    void *buf = self->first;
    int c = self->size;
    switch(self->data->ilen) {
    case sizeof(uint8_t ): MATCHCHARSET(uint8_t ); break;
    case sizeof(uint16_t): MATCHCHARSET(uint16_t); break;
    case sizeof(uint32_t): MATCHCHARSET(uint32_t); break;
    default: BUG("Invalid array item length!");
    }
    return -1;
}
int darray_find_darray(const darray_t *self, const darray_t *pat, int o) {
    int self_c = self->size, pat_c = pat->size;
    if(pat_c == 0) return o < self_c ? o : -1;
    if(pat_c > self_c) return -1;
    if(pat_c == 1) return darray_find_char(self, darray_getuchar(pat, 0), o);
    else {
        switch(self->data->ilen) {
        case sizeof(uint8_t): {
            /* Boyer-Moore-Horspool
             * see: http://www.dcc.uchile.cl/~rbaeza/handbook
                    /algs/7/713b.srch.p.html
             * and: ftp://sunsite.dcc.uchile.cl/pub/users/rbaeza
                    /handbook/algs/7/713b.srch.c
             */
            uint8_t *s = (uint8_t *)self->first;
            uint8_t *p = (uint8_t *)pat->first;
            int i, j, k, skip[256];
            for(k = 0; k < 256; k ++) skip[k] = pat_c;
            for(k = 0; k < pat_c - 1; k ++) skip[p[k]] = pat_c - k - 1;
            for(k = o + pat_c - 1; k < self_c; k += skip[s[k] & 255]) {
                for(j = pat_c - 1, i = k; j >= 0 && s[i] == p[j]; j --) i --;
                if(j == -1) return i + 1;
            }
            break; }
        default:
            BUG("Invalid ilen in darray_find_darray\n");
        }
        return -1;
    }
}
void *darray_get(const darray_t *self, int i) {
    int size = self->size;
    if(i < 0) i += size;
    return i < 0 || i >= size ? NULL : LIST_AT(self, i);
}
void *darray_getptr(const darray_t *self, int i) {
    void **vptr = (void **)darray_get(self, i);
    return vptr != NULL ? *vptr : NULL;
}
uint32_t darray_getuchar(const darray_t *self, int i) {
    void *vptr = darray_get(self, i);
    if(vptr == NULL) return 0;
    switch(self->data->ilen) {
    case sizeof(uint8_t ): return *(uint8_t  *)vptr;
    case sizeof(uint16_t): return *(uint16_t *)vptr;
    case sizeof(uint32_t): return *(uint32_t *)vptr;
    default: BUG("Invalid ilen in darray_getuchar\n");
    }
}
int darray_hc(const darray_t *self) {
    register const uint8_t *beg = (uint8_t *)self->first;
    register const uint8_t *end = beg + self->data->ilen * self->size;
    int hc = 5381;
    for(; beg < end; beg ++) {
        hc += hc << 5;
        hc += *beg;
    }
    return hc > 0 ? hc : -hc;
}
#define WRITEITEM(self, stream, t, f) do { \
    t *i = (t *)(self)->first, *end = i + (self)->size; \
    for(; i < end; i ++) fprintf((stream), " " f, (t)*i); \
    fprintf((stream), "\n"); \
} while(0)
void darray_write(const darray_t *self, FILE *stream) {
    struct listdata *bd = self->data;
    int ilen = bd->ilen;
    char *first = self->first;
    fprintf(stream, "darray_t(%p", (void *)self);
    fprintf(stream, ", first=%p", (void *)first);
    fprintf(stream, "(%i)", (first - &bd->item) / ilen);
    fprintf(stream, ", size=%i", self->size);
    fprintf(stream, ")\n-> data(%p", (void *)bd);
    fprintf(stream, ", cap=%i", bd->cap);
    fprintf(stream, ", used=%i", bd->used);
    fprintf(stream, ", ilen=%i", ilen);
    fprintf(stream, ", refc=%i", bd->refc);
    fprintf(stream, ")\n->");
    switch(ilen) {
    case sizeof(char ): WRITEITEM(self, stream, char,  "%hi"); break;
    case sizeof(short): WRITEITEM(self, stream, short, "%hi"); break;
    case sizeof(long ): WRITEITEM(self, stream, long,  "%li"); break;
    default:            WRITEITEM(self, stream, long,  "%lx"); break;
    }
}
void darray_print_tostream(const darray_t *self, FILE *stream) {
    fwrite(self->first, self->data->ilen, self->size, stream);
}
