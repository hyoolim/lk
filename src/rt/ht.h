#ifndef HT_H
#define HT_H
#include "common.h"

// base set item type; usually bigger to include value
typedef struct ht_item {
    const void *key;
} ht_item_t;

// set data so that cloning isn't so expensive
typedef int ht_hash_func_t(const void *key, int cap);
typedef int ht_cmp_func_t(const void *self, const void *other);
struct ht_data {
    int ci;
    int cap;
    int value_length; // length of item value
    int size;
    ht_hash_func_t *hash_func;
    ht_cmp_func_t *cmp_func;
    ht_item_t items; // placeholder for the first item
};

// the actual set
typedef struct ht {
    struct ht_data *data;
} ht_t;

// for set construction/destruction
ht_t *ht_alloc(int value_length, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func);
void ht_fin(ht_t *self);
void ht_free(ht_t *self);
void ht_init(ht_t *self, int value_length, ht_hash_func_t *hash_func, ht_cmp_func_t *cmp_func);

// set manipulation
void ht_clear(ht_t *self);
int ht_size(ht_t *self);
ht_item_t *ht_get(const ht_t *self, const void *key);
void *ht_set(ht_t *self, const void *key);
void ht_unset(ht_t *self, const void *key);

// default hash and keycmp
int ht_keycmp(const void *self, const void *other);
int ht_hash(const void *key, int cap);

// set item pointer math
#define HT_ITEM_ADD(data, item, delta) ((ht_item_t *)((char *)(item) + HT_ITEM_SIZE(data) * (delta)))
#define HT_ITEM_AT(data, index) HT_ITEM_ADD(data, &(data)->items, index)
#define HT_ITEM_SIZE(data) (sizeof(ht_item_t) + (data)->value_length)
#define HT_ITEM_VALUE(type, item) (*(type *)HT_ITEM_VALUEPTR(item))
#define HT_ITEM_VALUEPTR(item) ((void *)((char *)(item) + sizeof(ht_item_t)))

// Placeholder key for deleted items. Pointer value 1 is reserved — callers must never use it as a real key.
#define HT_ITEM_SKIPKEY ((ht_item_t *)1)

// simple way to iterate over the whole set
#define HT_EACH(self, item, block) \
    do { \
        struct ht_data *_data = (self)->data; \
        int _i, _isize = HT_ITEM_SIZE(_data); \
        ht_item_t *item = (ht_item_t *)&_data->items; \
        item = (ht_item_t *)((char *)item - _isize); \
        for (_i = 0; _i < _data->cap; _i++) { \
            item = (ht_item_t *)((char *)item + _isize); \
            if (item->key == NULL || item->key == HT_ITEM_SKIPKEY) \
                continue; \
            { \
                block; \
            } \
        } \
    } while (0)
#endif
