#include "greatest.h"
#include "../../../src/rt/mem.h"
#include <string.h>

TEST alloc_returns_nonnull(void) {
    void *p = mem_alloc(16);
    ASSERT(p != NULL);
    mem_free(p);
    PASS();
}

TEST alloc_zeroed(void) {
    char *p = mem_alloc(64);

    for (int i = 0; i < 64; i++)
        ASSERT_EQ(0, p[i]);

    mem_free(p);
    PASS();
}

TEST alloc_count(void) {
    int before = mem_alloc_count();
    void *p = mem_alloc(32);
    ASSERT_EQ(before + 1, mem_alloc_count());
    mem_free(p);
    ASSERT_EQ(before, mem_alloc_count());
    PASS();
}

TEST alloc_used(void) {
    size_t before = mem_alloc_used();
    void *p = mem_alloc(32);
    ASSERT_EQ(before + 32, mem_alloc_used());
    mem_free(p);
    ASSERT_EQ(before, mem_alloc_used());
    PASS();
}

TEST alloc_used_large(void) {
    /* Large blocks (>= MEM_MAX_RECYCLED) bypass recycler and go straight to OS free */
    size_t size = MEM_MAX_RECYCLED + 64;
    size_t before = mem_alloc_used();
    void *p = mem_alloc(size);
    ASSERT_EQ(before + size, mem_alloc_used());
    mem_free(p);
    ASSERT_EQ(before, mem_alloc_used());
    PASS();
}

TEST alloc_total_grows(void) {
    size_t before = mem_alloc_total();
    void *p = mem_alloc(48);
    ASSERT(mem_alloc_total() >= before + 48);
    mem_free(p);
    PASS();
}

TEST alloc_total_no_increase_on_recycle(void) {
    /* Reusing a recycled block must not increase alloc_total */
    size_t size = 80;
    void *p = mem_alloc(size);
    mem_free(p); /* Now in recycler */

    size_t total_before = mem_alloc_total();
    void *p2 = mem_alloc(size); /* Served from recycler */
    ASSERT_EQ(total_before, mem_alloc_total());
    mem_free(p2);
    mem_free_recycled();
    PASS();
}

TEST free_null(void) {
    mem_free(NULL); /* Must not crash */
    PASS();
}

TEST recycler_reuse_zeroed(void) {
    size_t size = 64;
    char *p = mem_alloc(size);
    memset(p, 0xAB, size);
    size_t recycled_before = mem_alloc_recycled();
    mem_free(p);
    ASSERT_EQ(recycled_before + size, mem_alloc_recycled()); /* Confirm it went to recycler */

    size_t recycled_before2 = mem_alloc_recycled();
    char *p2 = mem_alloc(size); /* Served from recycler */
    ASSERT_EQ(recycled_before2 - size, mem_alloc_recycled()); /* Confirm recycler was hit */

    for (size_t i = 0; i < size; i++)
        ASSERT_EQ(0, p2[i]); /* Must be re-zeroed */

    mem_free(p2);
    PASS();
}

TEST recycler_chain(void) {
    /* Free multiple blocks of the same size — they must chain and all be retrievable */
    size_t size = 72;
    void *a = mem_alloc(size);
    void *b = mem_alloc(size);
    void *c = mem_alloc(size);

    size_t before = mem_alloc_recycled();
    mem_free(a);
    mem_free(b);
    mem_free(c);
    ASSERT_EQ(before + size * 3, mem_alloc_recycled());

    /* All three should come back from the recycler */
    void *a2 = mem_alloc(size);
    void *b2 = mem_alloc(size);
    void *c2 = mem_alloc(size);
    ASSERT_EQ(before, mem_alloc_recycled());

    mem_free(a2);
    mem_free(b2);
    mem_free(c2);
    mem_free_recycled();
    PASS();
}

TEST recycler_boundary(void) {
    /* MEM_MAX_RECYCLED - 1: last size that goes to recycler */
    size_t small = MEM_MAX_RECYCLED - 1;
    void *p = mem_alloc(small);
    size_t before = mem_alloc_recycled();
    mem_free(p);
    ASSERT_EQ(before + small, mem_alloc_recycled());
    mem_free_recycled();

    /* MEM_MAX_RECYCLED: first size that goes straight to OS */
    size_t large = MEM_MAX_RECYCLED;
    void *q = mem_alloc(large);
    size_t before2 = mem_alloc_recycled();
    mem_free(q);
    ASSERT_EQ(before2, mem_alloc_recycled()); /* Must not change */
    PASS();
}

TEST recycler_tracks_bytes(void) {
    size_t size = 128;
    void *p = mem_alloc(size);
    size_t before = mem_alloc_recycled();
    mem_free(p);
    ASSERT_EQ(before + size, mem_alloc_recycled());
    mem_free_recycled();
    PASS();
}

TEST free_recycled_clears(void) {
    /* Freeing blocks of several sizes and then releasing must zero alloc_recycled */
    void *a = mem_alloc(64);
    void *b = mem_alloc(128);
    void *c = mem_alloc(256);
    mem_free(a);
    mem_free(b);
    mem_free(c);
    mem_free_recycled();
    ASSERT_EQ((size_t)0, mem_alloc_recycled());
    PASS();
}

TEST peak_tracks_max(void) {
    size_t peak_before = mem_alloc_peak();
    size_t used_before = mem_alloc_used();

    void *p = mem_alloc(512);
    size_t peak_after = mem_alloc_peak();

    ASSERT(peak_after >= used_before + 512);
    ASSERT(peak_after >= peak_before);
    mem_free(p);
    ASSERT_EQ(peak_after, mem_alloc_peak()); /* Peak must not decrease after free */
    PASS();
}

TEST resize_from_null(void) {
    void *p = mem_resize(NULL, 32);
    ASSERT(p != NULL);
    mem_free(p);
    PASS();
}

TEST resize_same_size(void) {
    /* Resize to the same size must return the same pointer untouched */
    char *p = mem_alloc(64);
    memset(p, 0x7F, 64);
    char *p2 = mem_resize(p, 64);
    ASSERT_EQ(p, p2);

    for (int i = 0; i < 64; i++)
        ASSERT_EQ(0x7F, (unsigned char)p2[i]);

    mem_free(p2);
    PASS();
}

TEST resize_preserves_data(void) {
    char *p = mem_alloc(32);
    memset(p, 0x5A, 32);

    char *p2 = mem_resize(p, 64);
    for (int i = 0; i < 32; i++)
        ASSERT_EQ(0x5A, (unsigned char)p2[i]);

    for (int i = 32; i < 64; i++)
        ASSERT_EQ(0, p2[i]); /* Growth must be zeroed */

    mem_free(p2);
    PASS();
}

TEST resize_shrink(void) {
    char *p = mem_alloc(64);
    memset(p, 0x3C, 64);

    char *p2 = mem_resize(p, 32);
    for (int i = 0; i < 32; i++)
        ASSERT_EQ(0x3C, (unsigned char)p2[i]);

    mem_free(p2);
    PASS();
}

TEST resize_large(void) {
    /* Large allocations bypass the recycler on both ends */
    size_t large = MEM_MAX_RECYCLED + 16;
    char *p = mem_alloc(large);
    memset(p, 0x11, large);

    char *p2 = mem_resize(p, large * 2);
    for (size_t i = 0; i < large; i++)
        ASSERT_EQ(0x11, (unsigned char)p2[i]);

    mem_free(p2);
    PASS();
}

SUITE(mem) {
    RUN_TEST(alloc_returns_nonnull);
    RUN_TEST(alloc_zeroed);
    RUN_TEST(alloc_count);
    RUN_TEST(alloc_used);
    RUN_TEST(alloc_used_large);
    RUN_TEST(alloc_total_grows);
    RUN_TEST(alloc_total_no_increase_on_recycle);
    RUN_TEST(free_null);
    RUN_TEST(recycler_reuse_zeroed);
    RUN_TEST(recycler_chain);
    RUN_TEST(recycler_boundary);
    RUN_TEST(recycler_tracks_bytes);
    RUN_TEST(free_recycled_clears);
    RUN_TEST(peak_tracks_max);
    RUN_TEST(resize_from_null);
    RUN_TEST(resize_same_size);
    RUN_TEST(resize_preserves_data);
    RUN_TEST(resize_shrink);
    RUN_TEST(resize_large);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(mem);
    GREATEST_MAIN_END();
}
