#ifndef HT_H
#define HT_H
#include "common.h"

// Each item stores its key, its distance from its home slot (dib = distance from initial
// bucket, 0 means the item sits at its home), and an inline value of value_length bytes
// immediately following the struct. A NULL key means the slot is empty.
typedef struct ht_item {
    const void *key;
    int dib;
} ht_item_t;

typedef int ht_hash_func_t(const void *key, int capacity);
typedef int ht_cmp_func_t(const void *self, const void *other);
struct ht_data {
    int capacity; // always a power of 2
    int value_length;
    int length;
    ht_hash_func_t *hash_func;
    ht_cmp_func_t *cmp_func;
    ht_item_t items; // placeholder for the first item
};

typedef struct ht {
    struct ht_data *data;
} ht_t;

// construction/destruction
ht_t *ht_alloc(int value_length, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func);
void ht_fin(ht_t *self);
void ht_free(ht_t *self);
void ht_init(ht_t *self, int value_length, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func);

// manipulation
void ht_clear(ht_t *self);
int ht_length(ht_t *self);
ht_item_t *ht_get(const ht_t *self, const void *key);
void *ht_set(ht_t *self, const void *key);
void ht_unset(ht_t *self, const void *key);

// default hash and keycmp
int ht_keycmp(const void *self, const void *other);
int ht_hash(const void *key, int capacity);

// item pointer math
#define HT_ITEM_ADD(data, item, delta) ((ht_item_t *)((char *)(item) + HT_ITEM_SIZE(data) * (delta)))
#define HT_ITEM_AT(data, index) HT_ITEM_ADD(data, &(data)->items, index)
#define HT_ITEM_SIZE(data) (sizeof(ht_item_t) + (data)->value_length)
#define HT_ITEM_VALUE(type, item) (*(type *)HT_ITEM_VALUEPTR(item))
#define HT_ITEM_VALUEPTR(item) ((void *)((char *)(item) + sizeof(ht_item_t)))

// iterate over all live entries (key != NULL)
#define HT_EACH(self, item, block) \
    do { \
        struct ht_data *_data = (self)->data; \
        int _i, _isize = HT_ITEM_SIZE(_data); \
        ht_item_t *item = (ht_item_t *)&_data->items; \
        item = (ht_item_t *)((char *)item - _isize); \
        for (_i = 0; _i < _data->capacity; _i++) { \
            item = (ht_item_t *)((char *)item + _isize); \
            if (item->key == NULL) \
                continue; \
            { \
                block; \
            } \
        } \
    } while (0)
#endif
