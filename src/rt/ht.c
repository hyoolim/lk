#include "ht.h"

// set cap needs to be primes for quadratic probing to work
static unsigned long primes[] = {11,        19,        37,        67,         131,     283,      521,      1033,
                                 2053,      4099,      8219,      16427,      32771,   65581,    131101,   262147,
                                 524309,    1048583,   2097169,   4194319,    8388617, 16777259, 33554467, 67108879,
                                 134217757, 268435459, 536870923, 1073741909, 0};

// create set data depending on item size
static struct ht_data *ht_data_alloc(int value_length, int ci, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func) {
    struct ht_data *self;
    int c = (int)primes[ci];

    self = mem_alloc(sizeof(struct ht_data) - sizeof(ht_item_t) + (sizeof(ht_item_t) + value_length) * c);
    self->ci = ci;
    self->cap = c;
    self->value_length = value_length;
    self->size = 0;
    self->hash_func = hash_func;
    self->cmp_func = cmp_func;
    return self;
}

// create new set data and repopulate
static void ht_resize(ht_t *self, int ci) {
    struct ht_data *olddata = self->data, *newdata;
    ht_hash_func_t *hash_func = olddata->hash_func;
    ht_cmp_func_t *cmp_func = olddata->cmp_func;
    int delta, newcap;
    ht_item_t *newitem, *newlast;

    newdata = ht_data_alloc(olddata->value_length, ci, hash_func, cmp_func);
    newlast = HT_ITEM_AT(newdata, newdata->cap - 1);
    newcap = newdata->cap;
    HT_EACH(
        self, olditem, delta = 1; newitem = HT_ITEM_AT(newdata, hash_func(olditem->key, newcap));
        while (newitem->key != NULL && (newitem->key == HT_ITEM_SKIPKEY || cmp_func(newitem->key, olditem->key) != 0)) {
            newitem = HT_ITEM_ADD(newdata, newitem, delta);
            while (newitem > newlast) {
                newitem = HT_ITEM_ADD(newdata, newitem, -newcap);
            }
            delta += 2;
        } memcpy(newitem, olditem, HT_ITEM_SIZE(olddata)););
    newdata->size = olddata->size;
    mem_free(olddata);
    self->data = newdata;
}

// for set construction/deconstruction
ht_t *ht_alloc(int value_length, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func) {
    ht_t *self = mem_alloc(sizeof(ht_t));
    ht_init(self, value_length, hash_func, cmp_func);
    return self;
}

void ht_fin(ht_t *self) {
    mem_free(self->data);
}

void ht_free(ht_t *self) {
    ht_fin(self);
    mem_free(self);
}

void ht_init(ht_t *self, int value_length, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func) {
    self->data = ht_data_alloc(value_length, 0, hash_func, cmp_func);
}

// set manipulation
void ht_clear(ht_t *self) {
    struct ht_data *data = self->data;

    data->size = 0;
    memset(&data->items, 0x0, HT_ITEM_SIZE(data) * data->cap);
}

int ht_size(ht_t *self) {
    return self->data->size;
}

ht_item_t *ht_get(const ht_t *self, const void *key) {
    struct ht_data *data = self->data;
    int delta = 1, cap = data->cap;
    ht_item_t *item = HT_ITEM_AT(data, data->hash_func(key, cap));
    ht_item_t *last = HT_ITEM_AT(data, cap - 1);

    while (item->key != NULL) {
        if (item->key != HT_ITEM_SKIPKEY && data->cmp_func(item->key, key) == 0) {
            return item;
        }
        item = HT_ITEM_ADD(data, item, delta);

        while (item > last)
            item = HT_ITEM_ADD(data, item, -cap);
        delta += 2;
    }
    return NULL;
}

void *ht_set(ht_t *self, const void *key) {
    struct ht_data *data = self->data;
    int delta = 1, cap;
    ht_item_t *item, *last;

    if (data->size >= data->cap / 2)
        ht_resize(self, data->ci + 1);
    data = self->data;
    cap = data->cap;
    item = HT_ITEM_AT(data, data->hash_func(key, cap));
    last = HT_ITEM_AT(data, cap - 1);

    while (item->key != NULL && (item->key == HT_ITEM_SKIPKEY || data->cmp_func(item->key, key) != 0)) {
        item = HT_ITEM_ADD(data, item, delta);

        while (item > last)
            item = HT_ITEM_ADD(data, item, -cap);
        delta += 2;
    }
    if (item->key == NULL || item->key == HT_ITEM_SKIPKEY)
        data->size++;
    item->key = key;
    return HT_ITEM_VALUEPTR(item);
}

void ht_unset(ht_t *self, const void *key) {
    ht_item_t *item = ht_get(self, key);

    if (item != NULL) {
        item->key = HT_ITEM_SKIPKEY;
        self->data->size--;
    }
}

// default hash and keycmp
int ht_hash(const void *key, int cap) {
    return (int)((ptrdiff_t)key % cap);
}

int ht_keycmp(const void *self, const void *other) {
    return (int)((ptrdiff_t)self - (ptrdiff_t)other);
}
