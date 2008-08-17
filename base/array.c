#include "array.h"

/* new */
static struct listdata *listdata_alloc(int ilen, int capa) {
    struct listdata *self;
    int c = 8;
    while(capa > c) c *= 2;
    self = memory_alloc(sizeof(struct listdata) - sizeof(char) + ilen * c);
    self->capa = c;
    self->used = 0;
    self->ilen = ilen;
    self->refc = 1;
    return self;
}
static void listdata_free(struct listdata *self) {
    if(self->refc > 0) self->refc --;
    if(self->refc < 1) memory_free(self);
}
array_t *array_alloc(int ilen, int capa) {
    array_t *self = memory_alloc(sizeof(array_t));
    array_init(self, ilen, capa);
    return self;
}
array_t *array_allocptr(void) {
    return array_allocptrwithcapa(10);
}
array_t *array_allocptrwithcapa(int capa) {
    return array_alloc(sizeof(void *), capa);
}
array_t *array_allocfromfile(FILE *stream, size_t rs) {
    array_t *self = array_alloc(1, rs);
    self->count =
    self->data->used = fread(self->first, 1, rs, stream);
    return self;
}
array_t *array_clone(array_t *self) {
    array_t *clone = memory_alloc(sizeof(array_t));
    array_copy(clone, self);
    return clone;
}
void array_init(array_t *self, int ilen, int capa) {
    self->data = listdata_alloc(ilen, capa);
    self->first = &self->data->item;
    self->count = 0;
}
void array_initptr(array_t *self) {
    array_init(self, sizeof(void *), 10);
}
void array_copy(array_t *self, array_t *src) {
    (self->data = src->data)->refc ++;
    self->first = src->first;
    self->count = src->count;
}
void array_fin(array_t *self) {
    listdata_free(self->data);
}
void array_free(array_t *self) {
    listdata_free(self->data);
    memory_free(self);
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
static void array_prepupdate(array_t *self, int i, int newcount) {
    struct listdata *bd = self->data;
    int newcapa = bd->capa, ilen = bd->ilen;
    if(newcount > newcapa) {
        do { newcapa *= 2; } while(newcount > newcapa);
        /* buf shared? alloc new */
        if(bd->refc > 1) {
            newlistdata:
            bd->refc --;
            bd = self->data = listdata_alloc(ilen, newcapa);
            memcpy(&bd->item, self->first, ilen * self->count);
            self->first = &bd->item;
        /* resize existing buf */
        } else {
            ptrdiff_t d = self->first - &bd->item;
            bd = self->data = memory_resize(bd,
            sizeof(struct listdata) - sizeof(char) + ilen * newcapa);
            bd->capa = newcapa;
            self->first = &bd->item + d;
        }
    /* new buf if replacing existing item in shared buf */
    } else if(bd->refc > 1 && i < bd->used) {
        goto newlistdata;
    }
    bd->used = newcount + (self->first - &bd->item);
    self->count = newcount;
}
void array_clear(array_t *self) {
    array_limit(self, 0);
}
void array_concat(array_t *self, array_t *v) {
    int sc = self->count, vc = v->count;
    int sl = self->data->ilen, vl = self->data->ilen;
    array_prepupdate(self, sc, sc + vc);
    if(sl == vl) memcpy(self->first + sl * sc, v->first, vl * vc);
    else NOIMPL("Cannot concat differently sized buffers");
}
void array_insert(array_t *self, int i, void *v) {
    int count = self->count;
    if(i < 0) i += count;
    if(i < 0) array_set(self, i - count - 1, v);
    else if(i > count) array_set(self, i, v);
    else {
        int ilen = self->data->ilen;
        array_prepupdate(self, i, count + 1);
        memmove(self->first + ilen * (i + 1),
        self->first + ilen * i, ilen * (count - i));
    }
    SETANYITEM(self, i, v);
}
void array_insertptr(array_t *self, int i, void *v) {
    array_insert(self, i, (void *)&v);
}
#define INSERTUINT(self, t, i, v) do { \
    t nv = (v); \
    array_insert(self, (i), (void *)&nv); \
} while(0);
void array_insertuchar(array_t *self, int i, uint32_t v) {
    switch(self->data->ilen) {
    case sizeof(uint8_t ): INSERTUINT(self, uint8_t,  i, v); break;
    case sizeof(uint16_t): INSERTUINT(self, uint16_t, i, v); break;
    case sizeof(uint32_t): INSERTUINT(self, uint32_t, i, v); break;
    default: BUG("Invalid ilen in array_insertuchar\n");
    }
}
void array_limit(array_t *self, int n) {
    if(n < 0) n += self->count;
    if(n >= 0 && n < self->count) self->count = n;
}
void array_offset(array_t *self, int n) {
    if(n < 0) n += self->count;
    if(n == 0) return;
    if(n < 0 || n >= self->count) self->count = 0;
    else {
        self->first += n * self->data->ilen;
        self->count -= n;
    }
}
void *array_peekptr(array_t *self) {
    return array_getptr((self), (self)->count - 1);
}
uint32_t array_peekuchar(array_t *self) {
    return array_getuchar((self), (self)->count - 1);
}
void *array_popptr(array_t *self) {
    return array_removeptr(self, self->count - 1);
}
uint32_t array_popuchar(array_t *self) {
    return array_removeuchar(self, self->count - 1);
}
void array_pushptr(array_t *self, void *v) {
    array_setptr(self, self->count, v);
}
void array_pushuchar(array_t *self, uint32_t v) {
    array_setuchar(self, self->count, v);
}
void array_remove(array_t *self, int i) {
    int count = self->count;
    if(i < 0) i += count;
    if(i < 0 || i >= count) return;
    else {
        int ilen = self->data->ilen, newcount = count - 1;
        array_prepupdate(self, i, newcount);
        if(i < newcount) memmove(self->first + ilen * i,
        self->first + ilen * (i + 1), ilen * (count - i));
    }
}
void *array_removeptr(array_t *self, int i) {
    void *item = array_getptr(self, i);
    array_remove(self, i);
    return item;
}
uint32_t array_removeuchar(array_t *self, int i) {
    uint32_t v = array_getuchar(self, i);
    array_remove(self, i);
    return v;
}
void array_resize(array_t *self, int s) {
    if(s <= self->count) return;
    array_prepupdate(self, s - 1, s);
}
void array_resizeitem(array_t *self, array_t *other) {
    if(self->data->ilen != other->data->ilen) {
        NOIMPL("Cannot resize item");
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
void array_reverse(array_t *self) {
    struct listdata *bd = self->data;
    int ilen = bd->ilen, i = 0, c = self->count, c2 = (c + 1) / 2;
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
        char *a = memory_alloc(ilen * 2);
        char *b = a + ilen;
        for(; i < c2; i ++) {
            memcpy(a, from + i, ilen);
            memcpy(b, from + c - i - 1, ilen);
            memcpy(to + i, b, ilen);
            memcpy(to + c - i - 1, a, ilen);
        }
        memory_free(a);
    }
    }
}
void array_set(array_t *self, int i, void *v) {
    int count = self->count;
    if(i < 0) i += count;
    if(i < 0) {
    } else {
        array_prepupdate(self, i, i >= count ? i + 1 : count);
    }
    SETANYITEM(self, i, v);
}
void array_setptr(array_t *self, int i, void *v) {
    array_set(self, i, (void *)&v);
}
void array_setrange(array_t *self, int b, int e, array_t *v) {
    int i, d, sc = self->count, vc = v->count;
    if(b < 0) b += sc;
    if(e < 0) e += vc;
    d = e - b - vc + 1;
    if(d > 0) for(; d > 0; d --) array_remove(self, b);
    else if(d < 0) {
        void *t = NULL;
        for(; d < 0; d ++) array_insert(self, b, (void *)&t);
    }
    for(i = 0; i < vc; i ++) array_set(self, b ++, array_get(v, i));
}
#define SETUINT(self, t, i, v) do { \
    t nv = (v); \
    array_set(self, (i), (void *)&nv); \
} while(0);
void array_setuchar(array_t *self, int i, uint32_t v) {
    switch(self->data->ilen) {
    case sizeof(uint8_t ): SETUINT(self, uint8_t,  i, v); break;
    case sizeof(uint16_t): SETUINT(self, uint16_t, i, v); break;
    case sizeof(uint32_t): SETUINT(self, uint32_t, i, v); break;
    default: BUG("Invalid ilen in array_setuchar\n");
    }
}
void *array_shiftptr(array_t *self) {
    return array_removeptr(self, 0);
}
void array_slice(array_t *self, int offset, int limit) {
    array_offset(self, offset);
    array_limit(self, limit);
}
const char *array_tocstr(array_t *self) {
    array_setuchar(self, self->count, '\0');
    self->count --;
    return self->first;
}
void array_unshiftptr(array_t *self, void *v) {
    array_insertptr(self, 0, v);
}

/* info */
int array_cmp(const array_t *self, const array_t *other) {
    if(self == other) return 0;
    else {
        int sc = self->count, oc = other->count, d = sc - oc;
        if(self->first == other->first) return d;
        else {
            int m = d > 0 ? oc : sc;
            if(self->data->ilen == other->data->ilen) {
                int rd = memcmp(self->first, other->first, m);
                return rd == 0 ? d : rd;
            } else {
                /*
                printf("Trying to compare strings"
                " with different item length\n");
                printf("self=%p(%i) '", (void *)self, self->data->ilen);
                string_print(self, stdout); printf("'\n");
                printf("other=%p(%i) '", (void *)other, other->data->ilen);
                string_print(other, stdout); printf("'\n");
                 */
                abort();
    }   }   }
}
#define COMPARECSTR(type) do { \
    for(; sb < sbend; sb += ilen, other ++) { \
        cd = *(type *)sb - *other; \
        if(cd != 0) return cd; \
    } \
} while(0)
int array_cmpcstr(const array_t *self, const char *other) {
    int sc = self->count, oc = strlen(other), d = sc - oc;
    int len = d > 0 ? oc : sc;
    uint8_t ilen = self->data->ilen;
    uint8_t *sb = (uint8_t *)self->first, *sbend = sb + len * ilen;
    int cd;
    if(     ilen == sizeof(uint8_t )) COMPARECSTR(uint8_t );
    else if(ilen == sizeof(uint16_t)) COMPARECSTR(uint16_t);
    else if(ilen == sizeof(uint32_t)) COMPARECSTR(uint16_t);
    else BUG("Invalid ilen in array_cmpcstr\n");
    return d;
}
#define MATCHCHAR(type) do { \
    for(; o < c; o ++) if(((type *)buf)[o] == pat) return o; \
} while(0)
int array_finduchar(const array_t *self, uint32_t pat, int o) {
    void *buf = self->first;
    int c = self->count;
    switch(self->data->ilen) {
    case sizeof(uint8_t ): MATCHCHAR(uint8_t ); break;
    case sizeof(uint16_t): MATCHCHAR(uint16_t); break;
    case sizeof(uint32_t): MATCHCHAR(uint32_t); break;
    default: BUG("Invalid ilen in array_finduchar\n");
    }
    return -1;
}
#define MATCHCHARSET(type) do { \
    for(; o < c; o ++) if(charset_has(pat, ((type *)buf)[o])) return o; \
} while(0)
int array_findCharset(const array_t *self, const charset_t *pat, int o) {
    void *buf = self->first;
    int c = self->count;
    switch(self->data->ilen) {
    case sizeof(uint8_t ): MATCHCHARSET(uint8_t ); break;
    case sizeof(uint16_t): MATCHCHARSET(uint16_t); break;
    case sizeof(uint32_t): MATCHCHARSET(uint32_t); break;
    default: BUG("Invalid array item length!");
    }
    return -1;
}
int array_findlist(const array_t *self, const array_t *pat, int o) {
    int self_c = self->count, pat_c = pat->count;
    if(pat_c == 0) return o < self_c ? o : -1;
    if(pat_c > self_c) return -1;
    if(pat_c == 1) return array_finduchar(self, array_getuchar(pat, 0), o);
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
            BUG("Invalid ilen in array_findlist\n");
        }
        return -1;
    }
}
void *array_get(const array_t *self, int i) {
    int count = self->count;
    if(i < 0) i += count;
    return i < 0 || i >= count ? NULL : LIST_AT(self, i);
}
void *array_getptr(const array_t *self, int i) {
    void **vptr = (void **)array_get(self, i);
    return vptr != NULL ? *vptr : NULL;
}
uint32_t array_getuchar(const array_t *self, int i) {
    void *vptr = array_get(self, i);
    if(vptr == NULL) return 0;
    switch(self->data->ilen) {
    case sizeof(uint8_t ): return *(uint8_t  *)vptr;
    case sizeof(uint16_t): return *(uint16_t *)vptr;
    case sizeof(uint32_t): return *(uint32_t *)vptr;
    default: BUG("Invalid ilen in array_getuchar\n");
    }
}
int array_hc(const array_t *self) {
    register const uint8_t *beg = (uint8_t *)self->first;
    register const uint8_t *end = beg + self->data->ilen * self->count;
    int hc = 5381;
    for(; beg < end; beg ++) {
        hc += hc << 5;
        hc += *beg;
    }
    return hc > 0 ? hc : -hc;
}
#define WRITEITEM(self, stream, t, f) do { \
    t *i = (t *)(self)->first, *end = i + (self)->count; \
    for(; i < end; i ++) fprintf((stream), " " f, (t)*i); \
    fprintf((stream), "\n"); \
} while(0)
void array_write(const array_t *self, FILE *stream) {
    struct listdata *bd = self->data;
    int ilen = bd->ilen;
    char *first = self->first;
    fprintf(stream, "array_t(%p", (void *)self);
    fprintf(stream, ", first=%p", (void *)first);
    fprintf(stream, "(%i)", (first - &bd->item) / ilen);
    fprintf(stream, ", count=%i", self->count);
    fprintf(stream, ")\n-> data(%p", (void *)bd);
    fprintf(stream, ", capa=%i", bd->capa);
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
