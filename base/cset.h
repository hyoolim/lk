#ifndef CSET_H
#define CSET_H
#include "common.h"

/* type */
typedef struct cset {
    int       capa;
    int       count;
    uint32_t  min;
    uint32_t  max;
    uint32_t *data;
    uint8_t   isinverted;
} CharacterSet_t;

/* new */
#define CSET_DEFAULTCAPA 8
CharacterSet_t *CharacterSet_alloc(void);
CharacterSet_t *CharacterSet_clone(CharacterSet_t *self);
void CharacterSet_copy(CharacterSet_t *self, CharacterSet_t *from);
void CharacterSet_fin(CharacterSet_t *self);
void CharacterSet_free(CharacterSet_t *self);
void CharacterSet_init(CharacterSet_t *self);
CharacterSet_t *CharacterSet_new(void);

/* update */
void CharacterSet_clear(CharacterSet_t *self);
void CharacterSet_addRange(CharacterSet_t *self, uint32_t from, uint32_t to);
void CharacterSet_subtractRange(CharacterSet_t *self, uint32_t from, uint32_t to);
void CharacterSet_add(CharacterSet_t *self, CharacterSet_t *other);
void CharacterSet_subtract(CharacterSet_t *self, CharacterSet_t *other);
#include "list.h"
void CharacterSet_addstring(CharacterSet_t *self, Sequence_t *str);
void CharacterSet_subtractstring(CharacterSet_t *self, Sequence_t *str);

/* info */
int CharacterSet_count(const CharacterSet_t *self);
void CharacterSet_print(const CharacterSet_t *self, FILE *stream);
int CharacterSet_has(const CharacterSet_t *self, uint32_t n);
#endif
