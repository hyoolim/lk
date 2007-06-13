#include "_set.h"

/* set capa needs to be primes for quadratic probing to work */
static unsigned long primes[] = { 
    11, 19, 37, 67, 131, 283, 521, 1033, 2053, 4099, 8219, 16427,
    32771, 65581, 131101, 262147, 524309, 1048583, 2097169, 4194319,
    8388617, 16777259, 33554467, 67108879, 134217757, 268435459,
    536870923, 1073741909, 0
};

/* create set data depending on item size */
static struct pt_setdata *setdata_alloc(int ivlen, int ci,
                                        pt_sethashfunc_t *hashfunc,
                                        pt_setkeycmpfunc_t *cmpfunc) {
    struct pt_setdata *self;
    int c = primes[ci];
    self = pt_memory_alloc(sizeof(struct pt_setdata)
                         - sizeof(pt_setitem_t)
                         + (sizeof(pt_setitem_t) + ivlen) * c);
    self->ci = ci;
    self->capa = c;
    self->ivlen = ivlen;
    self->refc = 1;
    self->count = 0;
    self->hashfunc = hashfunc;
    self->cmpfunc = cmpfunc;
    return self;
}

/* simple ref counting memory management for clones */
static void setdata_free(struct pt_setdata *self) {
    if(self->refc > 0) self->refc --;
    if(self->refc < 1) pt_memory_free(self);
}

/* create new set data and repopulate */
static void set_resize(pt_set_t *self, int ci) {
    struct pt_setdata *olddata = self->data, *newdata;
    pt_sethashfunc_t *hashfunc = olddata->hashfunc;
    pt_setkeycmpfunc_t *cmpfunc = olddata->cmpfunc;
    int delta, newcapa;
    pt_setitem_t *newitem, *newlast;
    newdata = setdata_alloc(olddata->ivlen, ci, hashfunc, cmpfunc);
    newlast = PT_SETITEM_AT(newdata, newdata->capa - 1);
    newcapa = newdata->capa;
    PT_SET_EACH(self, olditem,
        delta = 1;
        newitem = PT_SETITEM_AT(newdata, hashfunc(olditem->key, newcapa));
        while(newitem->key != NULL && (newitem->key == PT_SETITEM_SKIPKEY
        || cmpfunc(newitem->key, olditem->key) != 0)) {
            newitem = PT_SETITEM_ADD(newdata, newitem, delta);
            while(newitem > newlast) {
                newitem = PT_SETITEM_ADD(newdata, newitem, -newcapa);
            }
            delta += 2;
        }
        memcpy(newitem, olditem, PT_SETITEM_SIZE(olddata));
    );
    newdata->count = olddata->count;
    setdata_free(olddata);
    self->data = newdata;
}

/* for set construction/deconstruction */
pt_set_t *pt_set_alloc(int ivlen, pt_sethashfunc_t *hashfunc,
                       pt_setkeycmpfunc_t *cmpfunc) {
    pt_set_t *self = pt_memory_alloc(sizeof(pt_set_t));
    pt_set_init(self, ivlen, hashfunc, cmpfunc);
    return self;
}
pt_set_t *pt_set_clone(pt_set_t *self) {
    pt_set_t *clone = pt_memory_alloc(sizeof(pt_set_t));
    pt_set_copy(clone, self);
    return clone;
}
void pt_set_copy(pt_set_t *self, pt_set_t *src) {
    (self->data = src->data)->refc ++;
}
void pt_set_fin(pt_set_t *self) {
    setdata_free(self->data);
}
void pt_set_free(pt_set_t *self) {
    pt_set_fin(self);
    pt_memory_free(self);
}
void pt_set_init(pt_set_t *self, int ivlen, pt_sethashfunc_t *hashfunc,
                 pt_setkeycmpfunc_t *cmpfunc) {
    self->data = setdata_alloc(ivlen, 0, hashfunc, cmpfunc);
}

/* set manipulation */
void pt_set_clear(pt_set_t *self) {
    struct pt_setdata *data = self->data;
    data->count = 0;
    memset(&data->items, 0x0, PT_SETITEM_SIZE(data) * data->capa);
}
int pt_set_count(pt_set_t *self) {
    return self->data->count;
}
pt_setitem_t *pt_set_get(const pt_set_t *self, const void *key) {
    struct pt_setdata *data = self->data;
    int delta = 1, capa = data->capa;
    pt_setitem_t *item = PT_SETITEM_AT(data, data->hashfunc(key, capa));
    pt_setitem_t *last = PT_SETITEM_AT(data, capa - 1);
    while(item->key != NULL) {
        if(item->key != PT_SETITEM_SKIPKEY
        && data->cmpfunc(item->key, key) == 0) {
            return item;
        }
        item = PT_SETITEM_ADD(data, item, delta);
        while(item > last) item = PT_SETITEM_ADD(data, item, -capa);
        delta += 2;
    }
    return NULL;
}
void *pt_set_set(pt_set_t *self, const void *key) {
    struct pt_setdata *data = self->data;
    int delta = 1, capa;
    pt_setitem_t *item, *last;
    if(data->count >= data->capa / 2) set_resize(self, data->ci + 1);
    data = self->data;
    capa = data->capa;
    item = PT_SETITEM_AT(data, data->hashfunc(key, capa));
    last = PT_SETITEM_AT(data, capa - 1);
    while(item->key != NULL && (item->key == PT_SETITEM_SKIPKEY
    || data->cmpfunc(item->key, key) != 0)) {
        item = PT_SETITEM_ADD(data, item, delta);
        while(item > last) item = PT_SETITEM_ADD(data, item, -capa);
        delta += 2;
    }
    if(item->key == NULL || item->key == PT_SETITEM_SKIPKEY) data->count ++;
    item->key = key;
    return PT_SETITEM_VALUEPTR(item);
}
void pt_set_unset(pt_set_t *self, const void *key) {
    pt_setitem_t *item = pt_set_get(self, key);
    if(item != NULL) {
        item->key = PT_SETITEM_SKIPKEY;
        self->data->count --;
    }
}

/* default hash and keycmp */
int pt_set_hash(const void *key, int capa) {
    return (ptrdiff_t)key % capa;
}
int pt_set_keycmp(const void *self, const void *other) {
    return (ptrdiff_t)self - (ptrdiff_t)other;
}
