#include "ht.h"

static struct ht_data *ht_data_alloc(int value_length, int cap, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func) {
    struct ht_data *self =
        mem_alloc(sizeof(struct ht_data) - sizeof(ht_item_t) + (sizeof(ht_item_t) + value_length) * cap);

    self->capacity = cap;
    self->value_length = value_length;
    self->length = 0;
    self->hash_func = hash_func;
    self->cmp_func = cmp_func;
    return self;
}

// Insert key+value into data using Robin Hood linear probing.
// Assumes key is not already present — used only during resize.
static void ht_data_insert(struct ht_data *data, const void *key, const void *value) {
    int cap = data->capacity;
    int item_size = HT_ITEM_SIZE(data);
    _Alignas(ht_item_t) char buf[item_size]; // item currently being placed
    _Alignas(ht_item_t) char tmp[item_size]; // swap buffer
    ht_item_t *to_place = (ht_item_t *)buf;

    to_place->key = key;
    to_place->dib = 0;
    if (data->value_length > 0)
        memcpy(HT_ITEM_VALUEPTR(to_place), value, data->value_length);

    int slot = data->hash_func(key, cap);

    for (;;) {
        ht_item_t *cur = HT_ITEM_AT(data, slot);

        if (cur->key == NULL) {
            memcpy(cur, to_place, item_size);
            data->length++;
            return;
        }

        if (cur->dib < to_place->dib) {
            memcpy(tmp, cur, item_size);
            memcpy(cur, to_place, item_size);
            memcpy(buf, tmp, item_size);
        }

        slot = (slot + 1) & (cap - 1);
        to_place->dib++;
    }
}

static void ht_resize(ht_t *self, int cap) {
    struct ht_data *olddata = self->data;
    struct ht_data *newdata = ht_data_alloc(olddata->value_length, cap, olddata->hash_func, olddata->cmp_func);
    int item_size = HT_ITEM_SIZE(olddata);
    ht_item_t *item = (ht_item_t *)((char *)&olddata->items - item_size);

    for (int i = 0; i < olddata->capacity; i++) {
        item = (ht_item_t *)((char *)item + item_size);

        if (item->key != NULL)
            ht_data_insert(newdata, item->key, HT_ITEM_VALUEPTR(item));
    }

    mem_free(olddata);
    self->data = newdata;
}

ht_t *ht_alloc(int value_length, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func) {
    ht_t *self = mem_alloc(sizeof(ht_t));
    ht_init(self, value_length, hash_func, cmp_func);
    self->refc = 1;
    return self;
}

void ht_fin(ht_t *self) {
    mem_free(self->data);
}

void ht_free(ht_t *self) {
    if (--self->refc > 0)
        return;

    ht_fin(self);
    mem_free(self);
}

ht_t *ht_retain(ht_t *self) {
    self->refc++;
    return self;
}

void ht_init(ht_t *self, int value_length, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func) {
    self->data = ht_data_alloc(value_length, 16, hash_func, cmp_func);
}

void ht_clear(ht_t *self) {
    struct ht_data *data = self->data;

    data->length = 0;
    memset(&data->items, 0x0, HT_ITEM_SIZE(data) * data->capacity);
}

int ht_length(ht_t *self) {
    return self->data->length;
}

ht_item_t *ht_get(const ht_t *self, const void *key) {
    struct ht_data *data = self->data;
    int cap = data->capacity;
    int slot = data->hash_func(key, cap);
    int dib = 0;

    for (;;) {
        ht_item_t *item = HT_ITEM_AT(data, slot);

        if (item->key == NULL || item->dib < dib)
            return NULL;

        if (data->cmp_func(item->key, key) == 0)
            return item;

        slot = (slot + 1) & (cap - 1);
        dib++;
    }
}

void *ht_set(ht_t *self, const void *key) {
    struct ht_data *data = self->data;

    if (data->length >= data->capacity * 3 / 4) {
        assert(data->capacity <= (1 << 30) && "hash table capacity overflow");
        ht_resize(self, data->capacity * 2);
    }
    data = self->data;

    int cap = data->capacity;
    int item_size = HT_ITEM_SIZE(data);
    int slot = data->hash_func(key, cap);
    void *result = NULL;
    _Alignas(ht_item_t) char buf[item_size]; // item currently being placed
    _Alignas(ht_item_t) char tmp[item_size]; // swap buffer
    ht_item_t *to_place = (ht_item_t *)buf;

    to_place->key = key;
    to_place->dib = 0;
    if (data->value_length > 0)
        memset(HT_ITEM_VALUEPTR(to_place), 0, data->value_length);

    for (;;) {
        ht_item_t *cur = HT_ITEM_AT(data, slot);

        if (cur->key == NULL) {
            memcpy(cur, to_place, item_size);
            data->length++;
            return result != NULL ? result : HT_ITEM_VALUEPTR(cur);
        }

        if (result == NULL && data->cmp_func(cur->key, key) == 0)
            return HT_ITEM_VALUEPTR(cur);

        if (cur->dib < to_place->dib) {
            if (result == NULL)
                result = HT_ITEM_VALUEPTR(cur);
            memcpy(tmp, cur, item_size);
            memcpy(cur, to_place, item_size);
            memcpy(buf, tmp, item_size);
        }

        slot = (slot + 1) & (cap - 1);
        to_place->dib++;
    }
}

void ht_unset(ht_t *self, const void *key) {
    struct ht_data *data = self->data;
    int cap = data->capacity;
    int item_size = HT_ITEM_SIZE(data);
    int slot = data->hash_func(key, cap);
    int dib = 0;

    for (;;) {
        ht_item_t *item = HT_ITEM_AT(data, slot);

        if (item->key == NULL || item->dib < dib)
            return;

        if (data->cmp_func(item->key, key) == 0)
            break;

        slot = (slot + 1) & (cap - 1);
        dib++;
    }

    data->length--;

    // Backward shift: pull subsequent items back toward their home slots,
    // maintaining the Robin Hood invariant without leaving tombstones
    for (;;) {
        int next = (slot + 1) & (cap - 1);
        ht_item_t *next_item = HT_ITEM_AT(data, next);

        if (next_item->key == NULL || next_item->dib == 0) {
            memset(HT_ITEM_AT(data, slot), 0, item_size);
            break;
        }

        memcpy(HT_ITEM_AT(data, slot), next_item, item_size);
        HT_ITEM_AT(data, slot)->dib--;
        slot = next;
    }
}

int ht_hash(const void *key, int capacity) {
    // Fibonacci hashing: multiply by 2^W/phi (W = pointer width) to scatter bits,
    // then take the top log2(capacity) bits. Handles aligned pointers well — low
    // zero bits get mixed into the high bits of the product.
    // UINTPTR_MAX / phi ≈ 2^W / phi; | 1 keeps the multiplier odd.
    static const uintptr_t fibo = (uintptr_t)(UINTPTR_MAX / 1.6180339887498948482) | 1;
    uintptr_t p = (uintptr_t)key * fibo;
    return (int)(p >> (CHAR_BIT * sizeof(uintptr_t) - __builtin_ctz(capacity)));
}

int ht_keycmp(const void *self, const void *other) {
    return self != other;
}
