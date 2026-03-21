#include "greatest.h"
#include "../../src/rt/mem.h"
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
    PASS();
}

TEST alloc_total_grows(void) {
    size_t before = mem_alloc_total();
    void *p = mem_alloc(48);
    ASSERT(mem_alloc_total() >= before + 48);
    mem_free(p);
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
    mem_free(p); /* Goes to recycler */

    char *p2 = mem_alloc(size); /* Reused from recycler - must be re-zeroed */
    for (size_t i = 0; i < size; i++)
        ASSERT_EQ(0, p2[i]);

    mem_free(p2);
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
    void *p = mem_alloc(64);
    mem_free(p);
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
    ASSERT_EQ(peak_after, mem_alloc_peak()); /* Peak must not decrease */
    PASS();
}

TEST resize_from_null(void) {
    void *p = mem_resize(NULL, 32);
    ASSERT(p != NULL);
    mem_free(p);
    PASS();
}

TEST resize_preserves_data(void) {
    char *p = mem_alloc(32);
    memset(p, 0x5A, 32);

    char *p2 = mem_resize(p, 64);
    for (int i = 0; i < 32; i++)
        ASSERT_EQ(0x5A, (unsigned char)p2[i]);

    for (int i = 32; i < 64; i++)
        ASSERT_EQ(0, p2[i]);

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
    RUN_TEST(alloc_total_grows);
    RUN_TEST(free_null);
    RUN_TEST(recycler_reuse_zeroed);
    RUN_TEST(recycler_tracks_bytes);
    RUN_TEST(free_recycled_clears);
    RUN_TEST(peak_tracks_max);
    RUN_TEST(resize_from_null);
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
