#include "../../../src/rt/vec.h"
#include "greatest.h"
#include <string.h>

static void set_byte(vec_t *v, int at, uint8_t val) {
    vec_set(v, at, &val);
}

static uint8_t get_byte(vec_t *v, int at) {
    return *(uint8_t *)vec_get(v, at);
}

#pragma region vec

TEST init_empty(void) {
    vec_t v;
    vec_init(&v, sizeof(uint8_t), 8);
    ASSERT_EQ(0, VEC_COUNT(&v));
    vec_fin(&v);
    PASS();
}

TEST grow_inplace(void) {
    // Sole-reference vector grown past capacity exercises the mem_resize path
    // in vec_prepupdate (not the copy-on-write path)
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 16; i++)
        set_byte(v, i, (uint8_t)i);

    ASSERT_EQ(16, VEC_COUNT(v));

    for (int i = 0; i < 16; i++)
        ASSERT_EQ((uint8_t)i, get_byte(v, i));

    vec_free(v);
    PASS();
}

TEST copy_shares_buf(void) {
    // vec_clone shares the buffer — ref_count increments
    vec_t *src = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 4; i++)
        set_byte(src, i, (uint8_t)(i + 1));

    vec_t *copy = vec_clone(src);

    ASSERT_EQ(VEC_COUNT(src), VEC_COUNT(copy));
    ASSERT_EQ(src->buf, copy->buf);
    ASSERT_EQ(2, src->buf->ref_count);

    vec_free(src);
    vec_free(copy);
    PASS();
}

TEST copy_on_write(void) {
    // Writing to a copy must give it a new buffer and leave the original unchanged
    vec_t *src = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 4; i++)
        set_byte(src, i, (uint8_t)(i + 1));

    vec_t *copy = vec_clone(src);
    set_byte(copy, 0, 99);

    ASSERT(src->buf != copy->buf);
    ASSERT_EQ(1, get_byte(src, 0));
    ASSERT_EQ(99, get_byte(copy, 0));

    vec_free(src);
    vec_free(copy);
    PASS();
}

TEST copy_on_write_within_used(void) {
    // Writing within the used range of a shared buffer must also trigger a new buffer
    vec_t *src = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 8; i++)
        set_byte(src, i, (uint8_t)i);

    vec_t *copy = vec_clone(src);
    vec_offset(copy, 4);   // Copy now starts at index 4 of shared buf
    set_byte(copy, 0, 99); // Writing at copy[0] = buf[4], within src's used range

    ASSERT(src->buf != copy->buf);
    ASSERT_EQ(4, get_byte(src, 4));
    ASSERT_EQ(99, get_byte(copy, 0));

    vec_free(src);
    vec_free(copy);
    PASS();
}

TEST negative_index(void) {
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 4; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    ASSERT_EQ(4, get_byte(v, -1));
    ASSERT_EQ(3, get_byte(v, -2));

    set_byte(v, -1, 99);
    ASSERT_EQ(99, get_byte(v, 3));

    vec_free(v);
    PASS();
}

TEST offset_and_limit(void) {
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 8; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    vec_offset(v, 2);
    ASSERT_EQ(6, VEC_COUNT(v));
    ASSERT_EQ(3, get_byte(v, 0));

    vec_limit(v, 3);
    ASSERT_EQ(3, VEC_COUNT(v));
    ASSERT_EQ(3, get_byte(v, 0));
    ASSERT_EQ(5, get_byte(v, 2));

    vec_free(v);
    PASS();
}

TEST slice(void) {
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 8; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    vec_slice(v, 2, 4); // offset=2, limit=4 → elements 3,4,5,6
    ASSERT_EQ(4, VEC_COUNT(v));
    ASSERT_EQ(3, get_byte(v, 0));
    ASSERT_EQ(6, get_byte(v, 3));

    vec_free(v);
    PASS();
}

TEST reverse_sole(void) {
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 5; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    vec_reverse(v);
    ASSERT_EQ(5, VEC_COUNT(v));
    ASSERT_EQ(5, get_byte(v, 0));
    ASSERT_EQ(4, get_byte(v, 1));
    ASSERT_EQ(3, get_byte(v, 2));
    ASSERT_EQ(2, get_byte(v, 3));
    ASSERT_EQ(1, get_byte(v, 4));

    vec_free(v);
    PASS();
}

TEST reverse_shared(void) {
    // Reversing a shared vector must allocate a new buffer
    vec_t *src = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 4; i++)
        set_byte(src, i, (uint8_t)(i + 1));

    vec_t *copy = vec_clone(src);
    vec_reverse(copy);

    ASSERT(src->buf != copy->buf);
    ASSERT_EQ(1, get_byte(src, 0)); // Original unchanged
    ASSERT_EQ(4, get_byte(copy, 0));

    vec_free(src);
    vec_free(copy);
    PASS();
}

TEST insert_and_remove(void) {
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 3; i++)
        set_byte(v, i, (uint8_t)(i + 1)); // [1, 2, 3]

    uint8_t x = 9;
    vec_insert(v, 1, &x); // [1, 9, 2, 3]
    ASSERT_EQ(4, VEC_COUNT(v));
    ASSERT_EQ(9, get_byte(v, 1));
    ASSERT_EQ(2, get_byte(v, 2));

    vec_remove(v, 1); // [1, 2, 3]
    ASSERT_EQ(3, VEC_COUNT(v));
    ASSERT_EQ(2, get_byte(v, 1));

    vec_free(v);
    PASS();
}

TEST concat_vecs(void) {
    vec_t *a = vec_alloc(sizeof(uint8_t), 4);
    vec_t *b = vec_alloc(sizeof(uint8_t), 4);

    for (int i = 0; i < 3; i++)
        set_byte(a, i, (uint8_t)(i + 1));

    for (int i = 0; i < 3; i++)
        set_byte(b, i, (uint8_t)(i + 4));

    vec_concat(a, b);
    ASSERT_EQ(6, VEC_COUNT(a));

    for (int i = 0; i < 6; i++)
        ASSERT_EQ((uint8_t)(i + 1), get_byte(a, i));

    vec_free(a);
    vec_free(b);
    PASS();
}

TEST cmp_vecs(void) {
    vec_t *a = vec_str_alloc_from_cstr("abc");
    vec_t *b = vec_str_alloc_from_cstr("abc");
    vec_t *c = vec_str_alloc_from_cstr("abd");
    vec_t *d = vec_str_alloc_from_cstr("ab");

    ASSERT_EQ(0, vec_cmp(a, b));
    ASSERT(vec_cmp(a, c) < 0);
    ASSERT(vec_cmp(c, a) > 0);
    ASSERT(vec_cmp(a, d) > 0);
    ASSERT(vec_cmp(d, a) < 0);

    vec_free(a);
    vec_free(b);
    vec_free(c);
    vec_free(d);
    PASS();
}

TEST find_char(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello world");

    ASSERT_EQ(4, vec_str_find(v, 'o', 0));
    ASSERT_EQ(7, vec_str_find(v, 'o', 5));
    ASSERT_EQ(-1, vec_str_find(v, 'z', 0));

    vec_free(v);
    PASS();
}

TEST find_vec(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello world");
    vec_t *pat = vec_str_alloc_from_cstr("world");
    vec_t *nopat = vec_str_alloc_from_cstr("xyz");
    vec_t *empty = vec_str_alloc_from_cstr("");

    ASSERT_EQ(6, vec_find_vec(v, pat, 0));
    ASSERT_EQ(-1, vec_find_vec(v, nopat, 0));
    ASSERT_EQ(0, vec_find_vec(v, empty, 0)); // Empty pattern returns offset

    vec_free(v);
    vec_free(pat);
    vec_free(nopat);
    vec_free(empty);
    PASS();
}

TEST vec_copy_direct(void) {
    // vec_copy (as opposed to vec_clone) copies into an already-allocated vec_t
    vec_t *src = vec_alloc(sizeof(uint8_t), 4);

    for (int i = 0; i < 4; i++)
        set_byte(src, i, (uint8_t)(i + 1));

    vec_t dst;
    vec_copy(&dst, src);

    ASSERT_EQ(src->buf, dst.buf);
    ASSERT_EQ(2, src->buf->ref_count);
    ASSERT_EQ(VEC_COUNT(src), VEC_COUNT(&dst));

    vec_fin(&dst);
    vec_free(src);
    PASS();
}

TEST clear(void) {
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 4; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    vec_clear(v);
    ASSERT_EQ(0, VEC_COUNT(v));

    vec_free(v);
    PASS();
}

TEST resize(void) {
    vec_t *v = vec_alloc(sizeof(uint8_t), 4);
    set_byte(v, 0, 42);

    vec_resize(v, 8);
    ASSERT_EQ(8, VEC_COUNT(v));
    ASSERT_EQ(42, get_byte(v, 0)); // Existing content preserved

    vec_resize(v, 4); // Shrink is a no-op
    ASSERT_EQ(8, VEC_COUNT(v));

    vec_free(v);
    PASS();
}

TEST set_range_same_size(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello");
    vec_t *r = vec_str_alloc_from_cstr("XY");

    vec_set_range(v, 1, 3, r); // Replace "el" with "XY" → "hXYlo"
    ASSERT_EQ(5, VEC_COUNT(v));
    ASSERT_EQ('X', (char)vec_str_get(v, 1));
    ASSERT_EQ('Y', (char)vec_str_get(v, 2));
    ASSERT_EQ('l', (char)vec_str_get(v, 3));

    vec_free(v);
    vec_free(r);
    PASS();
}

TEST set_range_shrink(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello");
    vec_t *r = vec_str_alloc_from_cstr("X");

    vec_set_range(v, 1, 3, r); // Replace "el" with "X" → "hXlo"
    ASSERT_EQ(4, VEC_COUNT(v));
    ASSERT_EQ('X', (char)vec_str_get(v, 1));
    ASSERT_EQ('l', (char)vec_str_get(v, 2));

    vec_free(v);
    vec_free(r);
    PASS();
}

TEST set_range_expand(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello");
    vec_t *r = vec_str_alloc_from_cstr("XYZ");

    vec_set_range(v, 1, 2, r); // Replace "e" with "XYZ" → "hXYZllo"
    ASSERT_EQ(7, VEC_COUNT(v));
    ASSERT_EQ('X', (char)vec_str_get(v, 1));
    ASSERT_EQ('Z', (char)vec_str_get(v, 3));
    ASSERT_EQ('l', (char)vec_str_get(v, 4));

    vec_free(v);
    vec_free(r);
    PASS();
}

TEST hc_consistent(void) {
    // Equal content must produce equal hash codes
    vec_t *a = vec_str_alloc_from_cstr("hello");
    vec_t *b = vec_str_alloc_from_cstr("hello");
    vec_t *c = vec_str_alloc_from_cstr("world");

    ASSERT_EQ(vec_hc(a), vec_hc(b));
    ASSERT(vec_hc(a) != vec_hc(c));

    vec_free(a);
    vec_free(b);
    vec_free(c);
    PASS();
}

TEST offset_past_end(void) {
    // Offset >= length must zero out the vector
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 4; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    vec_offset(v, 10);
    ASSERT_EQ(0, VEC_COUNT(v));

    vec_free(v);
    PASS();
}

TEST offset_negative(void) {
    // Negative offset is relative to the end
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 4; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    vec_offset(v, -2); // Skip to index 2 → [3, 4]
    ASSERT_EQ(2, VEC_COUNT(v));
    ASSERT_EQ(3, get_byte(v, 0));

    vec_free(v);
    PASS();
}

TEST limit_noop(void) {
    // Limit >= length is a no-op
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 4; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    vec_limit(v, 10);
    ASSERT_EQ(4, VEC_COUNT(v));

    vec_free(v);
    PASS();
}

TEST limit_negative(void) {
    // Negative limit is relative to the end
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 4; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    vec_limit(v, -1); // Keep all but last → [1, 2, 3]
    ASSERT_EQ(3, VEC_COUNT(v));
    ASSERT_EQ(3, get_byte(v, 2));

    vec_free(v);
    PASS();
}

TEST remove_out_of_bounds(void) {
    // Out-of-bounds remove must be a no-op
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 3; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    vec_remove(v, 10);
    ASSERT_EQ(3, VEC_COUNT(v));

    vec_free(v);
    PASS();
}

TEST get_out_of_bounds(void) {
    vec_t *v = vec_alloc(sizeof(uint8_t), 4);
    set_byte(v, 0, 1);

    ASSERT_EQ(NULL, vec_get(v, 5));
    ASSERT_EQ(NULL, vec_get(v, -5));

    vec_free(v);
    PASS();
}

TEST find_vec_pattern_longer(void) {
    // Pattern longer than self must return -1
    vec_t *v = vec_str_alloc_from_cstr("hi");
    vec_t *pat = vec_str_alloc_from_cstr("hello world");

    ASSERT_EQ(-1, vec_find_vec(v, pat, 0));

    vec_free(v);
    vec_free(pat);
    PASS();
}

TEST vec_each_macro(void) {
    vec_t *v = vec_alloc(sizeof(uint8_t), 8);

    for (int i = 0; i < 5; i++)
        set_byte(v, i, (uint8_t)(i + 1));

    int sum = 0;

    VEC_EACH(v, i, item, { sum += *(uint8_t *)item; });
    ASSERT_EQ(15, sum);

    vec_free(v);
    PASS();
}

TEST vec_eq_macro(void) {
    vec_t *a = vec_str_alloc_from_cstr("abc");
    vec_t *b = vec_str_alloc_from_cstr("abc");
    vec_t *c = vec_str_alloc_from_cstr("xyz");

    ASSERT(VEC_EQ(a, b));
    ASSERT(!VEC_EQ(a, c));

    vec_free(a);
    vec_free(b);
    vec_free(c);
    PASS();
}

TEST vec_isinit_macro(void) {
    vec_t v;
    memset(&v, 0, sizeof(v));
    ASSERT(!VEC_ISINIT(&v));

    vec_init(&v, sizeof(uint8_t), 8);
    ASSERT(VEC_ISINIT(&v));

    vec_fin(&v);
    PASS();
}

#pragma endregion
#pragma region string

TEST str_alloc_from_data(void) {
    const char *data = "hello";
    vec_t *v = vec_str_alloc_from_data(data, 3);

    ASSERT_EQ(3, VEC_COUNT(v));
    ASSERT_EQ('h', (char)vec_str_get(v, 0));
    ASSERT_EQ('l', (char)vec_str_get(v, 2));

    vec_free(v);
    PASS();
}

TEST str_alloc_from_file(void) {
    FILE *f = tmpfile();
    ASSERT(f != NULL);
    fputs("hello", f);
    rewind(f);

    vec_t *v = vec_str_alloc_from_file(f);
    fclose(f);

    ASSERT_EQ(5, VEC_COUNT(v));
    ASSERT_EQ(0, vec_str_cmp_cstr(v, "hello"));

    vec_free(v);
    PASS();
}

TEST str_alloc_from_file_until_char(void) {
    FILE *f = tmpfile();
    ASSERT(f != NULL);
    fputs("hello world", f);
    rewind(f);

    vec_t *v = vec_str_alloc_from_file_until_char(f, ' ');
    fclose(f);

    // Delimiter is included in the result
    ASSERT_EQ(6, VEC_COUNT(v));
    ASSERT_EQ(0, vec_str_cmp_cstr(v, "hello "));

    vec_free(v);
    PASS();
}

TEST str_alloc_from_file_with_length(void) {
    FILE *f = tmpfile();
    ASSERT(f != NULL);
    fputs("hello world", f);
    rewind(f);

    vec_t *v = vec_str_alloc_from_file_with_length(f, 5);
    fclose(f);

    ASSERT_EQ(5, VEC_COUNT(v));
    ASSERT_EQ(0, vec_str_cmp_cstr(v, "hello"));

    vec_free(v);
    PASS();
}

TEST str_alloc_from_file_until_charset(void) {
    FILE *f = tmpfile();
    ASSERT(f != NULL);
    fputs("hello world", f);
    rewind(f);

    charset_t *cs = charset_new();
    charset_add_chars(cs, ' ', ' ');

    vec_t *v = vec_str_alloc_from_file_until_charset(f, cs);
    fclose(f);
    charset_free(cs);

    // Delimiter is included in the result
    ASSERT_EQ(6, VEC_COUNT(v));
    ASSERT_EQ(0, vec_str_cmp_cstr(v, "hello "));

    vec_free(v);
    PASS();
}

TEST str_set(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello");
    vec_str_set(v, 0, 'H');
    ASSERT_EQ('H', (char)vec_str_get(v, 0));
    vec_free(v);
    PASS();
}

TEST str_peek(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello");
    ASSERT_EQ('o', (char)vec_str_peek(v));
    vec_free(v);
    PASS();
}

TEST str_find_charset(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello world");
    charset_t *cs = charset_new();
    charset_add_chars(cs, 'a', 'z');

    ASSERT_EQ(0, vec_str_find_charset(v, cs, 0)); // 'h' is in a-z
    ASSERT_EQ(6, vec_str_find_charset(v, cs, 6)); // 'w' at index 6

    charset_free(cs);

    charset_t *digits = charset_new();
    charset_add_chars(digits, '0', '9');
    ASSERT_EQ(-1, vec_str_find_charset(v, digits, 0));
    charset_free(digits);

    vec_free(v);
    PASS();
}

TEST str_push_pop(void) {
    vec_t *v = vec_str_alloc();
    vec_str_push(v, 'h');
    vec_str_push(v, 'i');
    ASSERT_EQ(2, VEC_COUNT(v));
    ASSERT_EQ('i', (char)vec_str_pop(v));
    ASSERT_EQ(1, VEC_COUNT(v));
    vec_free(v);
    PASS();
}

TEST str_insert_remove(void) {
    vec_t *v = vec_str_alloc_from_cstr("hllo");
    vec_str_insert(v, 1, 'e'); // "hello"
    ASSERT_EQ(5, VEC_COUNT(v));
    ASSERT_EQ('e', (char)vec_str_get(v, 1));

    vec_str_remove(v, 1); // "hllo"
    ASSERT_EQ(4, VEC_COUNT(v));
    ASSERT_EQ('l', (char)vec_str_get(v, 1));

    vec_free(v);
    PASS();
}

TEST str_cmp_cstr(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello");

    ASSERT_EQ(0, vec_str_cmp_cstr(v, "hello"));
    ASSERT(vec_str_cmp_cstr(v, "hellp") < 0);
    ASSERT(vec_str_cmp_cstr(v, "hell") > 0);

    vec_free(v);
    PASS();
}

TEST str_tocstr(void) {
    vec_t *v = vec_str_alloc_from_cstr("hello");
    const char *cs = vec_str_tocstr(v);

    ASSERT_EQ(0, strcmp(cs, "hello"));
    ASSERT_EQ(5, VEC_COUNT(v)); // Length unchanged after null termination

    vec_free(v);
    PASS();
}

#pragma endregion
#pragma region ptr list

TEST ptr_init(void) {
    vec_t v;
    vec_ptr_init(&v);
    ASSERT_EQ(0, VEC_COUNT(&v));
    vec_fin(&v);
    PASS();
}

TEST ptr_alloc_with_capacity(void) {
    vec_t *v = vec_ptr_alloc_with_capacity(32);
    ASSERT_EQ(0, VEC_COUNT(v));
    ASSERT(v->buf->capacity >= 32);
    vec_free(v);
    PASS();
}

TEST ptr_set(void) {
    vec_t *v = vec_ptr_alloc();
    int a = 1, b = 2;

    vec_ptr_push(v, &a);
    vec_ptr_push(v, &a);
    vec_ptr_set(v, 1, &b);
    ASSERT_EQ(&b, vec_ptr_get(v, 1));

    vec_free(v);
    PASS();
}

TEST ptr_peek(void) {
    vec_t *v = vec_ptr_alloc();
    int a = 1, b = 2;

    vec_ptr_push(v, &a);
    vec_ptr_push(v, &b);
    ASSERT_EQ(&b, vec_ptr_peek(v));
    ASSERT_EQ(2, VEC_COUNT(v)); // Peek must not remove

    vec_free(v);
    PASS();
}

TEST ptr_each(void) {
    vec_t *v = vec_ptr_alloc();
    int a = 1, b = 2, c = 3;

    vec_ptr_push(v, &a);
    vec_ptr_push(v, &b);
    vec_ptr_push(v, &c);

    int sum = 0;

    VEC_EACH_PTR(v, i, item, { sum += *(int *)item; });
    ASSERT_EQ(6, sum);

    vec_free(v);
    PASS();
}

TEST ptr_push_pop(void) {
    vec_t *v = vec_ptr_alloc();
    int a = 1, b = 2, c = 3;

    vec_ptr_push(v, &a);
    vec_ptr_push(v, &b);
    vec_ptr_push(v, &c);
    ASSERT_EQ(3, VEC_COUNT(v));
    ASSERT_EQ(&c, vec_ptr_pop(v));
    ASSERT_EQ(2, VEC_COUNT(v));

    vec_free(v);
    PASS();
}

TEST ptr_shift_unshift(void) {
    vec_t *v = vec_ptr_alloc();
    int a = 1, b = 2;

    vec_ptr_push(v, &a);
    vec_ptr_push(v, &b);
    ASSERT_EQ(&a, vec_ptr_shift(v));
    ASSERT_EQ(1, VEC_COUNT(v));

    vec_ptr_unshift(v, &a);
    ASSERT_EQ(&a, vec_ptr_get(v, 0));
    ASSERT_EQ(2, VEC_COUNT(v));

    vec_free(v);
    PASS();
}

TEST ptr_insert_remove(void) {
    vec_t *v = vec_ptr_alloc();
    int a = 1, b = 2, c = 3, x = 9;

    vec_ptr_push(v, &a);
    vec_ptr_push(v, &b);
    vec_ptr_push(v, &c);

    vec_ptr_insert(v, 1, &x);
    ASSERT_EQ(4, VEC_COUNT(v));
    ASSERT_EQ(&x, vec_ptr_get(v, 1));

    vec_ptr_remove(v, 1);
    ASSERT_EQ(3, VEC_COUNT(v));
    ASSERT_EQ(&b, vec_ptr_get(v, 1));

    vec_free(v);
    PASS();
}

#pragma endregion

SUITE(vec) {
    // vec
    RUN_TEST(init_empty);
    RUN_TEST(grow_inplace);
    RUN_TEST(copy_shares_buf);
    RUN_TEST(vec_copy_direct);
    RUN_TEST(copy_on_write);
    RUN_TEST(copy_on_write_within_used);
    RUN_TEST(clear);
    RUN_TEST(resize);
    RUN_TEST(negative_index);
    RUN_TEST(offset_and_limit);
    RUN_TEST(offset_past_end);
    RUN_TEST(offset_negative);
    RUN_TEST(limit_noop);
    RUN_TEST(limit_negative);
    RUN_TEST(slice);
    RUN_TEST(reverse_sole);
    RUN_TEST(reverse_shared);
    RUN_TEST(insert_and_remove);
    RUN_TEST(remove_out_of_bounds);
    RUN_TEST(get_out_of_bounds);
    RUN_TEST(concat_vecs);
    RUN_TEST(set_range_same_size);
    RUN_TEST(set_range_shrink);
    RUN_TEST(set_range_expand);
    RUN_TEST(cmp_vecs);
    RUN_TEST(find_char);
    RUN_TEST(find_vec);
    RUN_TEST(find_vec_pattern_longer);
    RUN_TEST(hc_consistent);
    RUN_TEST(vec_each_macro);
    RUN_TEST(vec_eq_macro);
    RUN_TEST(vec_isinit_macro);
    // string
    RUN_TEST(str_alloc_from_data);
    RUN_TEST(str_alloc_from_file);
    RUN_TEST(str_alloc_from_file_until_char);
    RUN_TEST(str_alloc_from_file_with_length);
    RUN_TEST(str_alloc_from_file_until_charset);
    RUN_TEST(str_set);
    RUN_TEST(str_peek);
    RUN_TEST(str_find_charset);
    RUN_TEST(str_push_pop);
    RUN_TEST(str_insert_remove);
    RUN_TEST(str_cmp_cstr);
    RUN_TEST(str_tocstr);
    // ptr list
    RUN_TEST(ptr_init);
    RUN_TEST(ptr_alloc_with_capacity);
    RUN_TEST(ptr_set);
    RUN_TEST(ptr_peek);
    RUN_TEST(ptr_each);
    RUN_TEST(ptr_push_pop);
    RUN_TEST(ptr_shift_unshift);
    RUN_TEST(ptr_insert_remove);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(vec);
    GREATEST_MAIN_END();
}
