#include "../../../src/rt/ht.h"
#include "greatest.h"

// Keys are small integers cast to pointers — distinct, stable, no allocations needed.
// Offset by 0x100 so values start well above NULL.
#define K(n) ((const void *)(uintptr_t)((n) + 0x100))

// Hash that maps every key to slot 0; used to force collisions in tests.
static int hash_always_zero(const void *key, int capacity) {
    (void)key;
    (void)capacity;
    return 0;
}

static ht_t *new_colliding_map(void) {
    return ht_alloc(sizeof(int), hash_always_zero, ht_keycmp);
}

static ht_t *new_int_map(void) {
    return ht_alloc(sizeof(int), ht_hash, ht_keycmp);
}

static void set_int(ht_t *ht, const void *key, int val) {
    *(int *)ht_set(ht, key) = val;
}

static int *get_int(ht_t *ht, const void *key) {
    ht_item_t *item = ht_get(ht, key);
    return item ? HT_ITEM_VALUEPTR(item) : NULL;
}

#pragma region map

TEST get_set(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 100);
    set_int(ht, K(2), 200);

    ASSERT_EQ(100, *get_int(ht, K(1)));
    ASSERT_EQ(200, *get_int(ht, K(2)));

    ht_free(ht);
    PASS();
}

TEST get_missing(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 100);

    ASSERT_EQ(NULL, get_int(ht, K(99)));

    ht_free(ht);
    PASS();
}

TEST overwrite(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 100);
    set_int(ht, K(1), 200);

    ASSERT_EQ(200, *get_int(ht, K(1)));
    ASSERT_EQ(1, ht_length(ht));

    ht_free(ht);
    PASS();
}

TEST overwrite_does_not_corrupt_others(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 100);
    set_int(ht, K(2), 200);
    set_int(ht, K(1), 300);

    ASSERT_EQ(300, *get_int(ht, K(1)));
    ASSERT_EQ(200, *get_int(ht, K(2)));

    ht_free(ht);
    PASS();
}

TEST size_tracking(void) {
    ht_t *ht = new_int_map();

    ASSERT_EQ(0, ht_length(ht));

    set_int(ht, K(1), 1);
    set_int(ht, K(2), 2);
    ASSERT_EQ(2, ht_length(ht));

    ht_unset(ht, K(1));
    ASSERT_EQ(1, ht_length(ht));

    ht_free(ht);
    PASS();
}

#pragma endregion
#pragma region unset

TEST unset_removes_key(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 100);
    ht_unset(ht, K(1));

    ASSERT_EQ(NULL, get_int(ht, K(1)));

    ht_free(ht);
    PASS();
}

TEST unset_missing_is_noop(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 100);
    ht_unset(ht, K(99));

    ASSERT_EQ(1, ht_length(ht));
    ASSERT_EQ(100, *get_int(ht, K(1)));

    ht_free(ht);
    PASS();
}

TEST reinsert_after_unset(void) {
    // A slot freed by unset must be reusable
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 100);
    ht_unset(ht, K(1));
    set_int(ht, K(1), 200);

    ASSERT_EQ(200, *get_int(ht, K(1)));
    ASSERT_EQ(1, ht_length(ht));

    ht_free(ht);
    PASS();
}

TEST double_unset_is_noop(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 100);
    ht_unset(ht, K(1));
    ht_unset(ht, K(1)); // Must not corrupt size or crash

    ASSERT_EQ(0, ht_length(ht));
    ASSERT_EQ(NULL, get_int(ht, K(1)));

    ht_free(ht);
    PASS();
}

TEST collision_displaced_entry_findable_after_neighbor_deleted(void) {
    // All keys hash to slot 0. K(2) is displaced to slot 1 (dib=1) by K(1).
    // After K(1) is deleted, backward shift pulls K(2) back to slot 0 (dib=0).
    // K(2) must still be findable.
    ht_t *ht = new_colliding_map();

    set_int(ht, K(1), 10); // lands at slot 0
    set_int(ht, K(2), 20); // collides at slot 0, displaced to slot 1
    ht_unset(ht, K(1));    // backward shift pulls K(2) back to slot 0

    ASSERT_EQ(NULL, get_int(ht, K(1)));
    ASSERT_EQ(20, *get_int(ht, K(2))); // must still be findable after neighbor deleted

    ht_free(ht);
    PASS();
}

#pragma endregion
#pragma region clear

TEST clear_empties(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 1);
    set_int(ht, K(2), 2);
    ht_clear(ht);

    ASSERT_EQ(0, ht_length(ht));
    ASSERT_EQ(NULL, get_int(ht, K(1)));
    ASSERT_EQ(NULL, get_int(ht, K(2)));

    ht_free(ht);
    PASS();
}

TEST set_after_clear(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 1);
    ht_clear(ht);
    set_int(ht, K(1), 42);

    ASSERT_EQ(42, *get_int(ht, K(1)));
    ASSERT_EQ(1, ht_length(ht));

    ht_free(ht);
    PASS();
}

#pragma endregion
#pragma region resize

TEST resize_preserves_entries(void) {
    // 50 entries triggers 3 resizes: cap goes 16 → 32 → 64 → 128
    ht_t *ht = new_int_map();
    int n = 50;

    for (int i = 1; i <= n; i++)
        set_int(ht, K(i), i * 10);

    ASSERT_EQ(n, ht_length(ht));

    for (int i = 1; i <= n; i++)
        ASSERT_EQ(i * 10, *get_int(ht, K(i)));

    ht_free(ht);
    PASS();
}

TEST resize_then_get_missing(void) {
    ht_t *ht = new_int_map();

    for (int i = 1; i <= 50; i++)
        set_int(ht, K(i), i);

    ASSERT_EQ(NULL, get_int(ht, K(999)));

    ht_free(ht);
    PASS();
}

#pragma endregion
#pragma region set mode

TEST set_mode_membership(void) {
    // value_length=0 — keys only, no values
    ht_t *ht = ht_alloc(0, ht_hash, ht_keycmp);

    ht_set(ht, K(1));
    ht_set(ht, K(2));

    ASSERT(ht_get(ht, K(1)) != NULL);
    ASSERT(ht_get(ht, K(2)) != NULL);
    ASSERT_EQ(NULL, ht_get(ht, K(3)));
    ASSERT_EQ(2, ht_length(ht));

    ht_free(ht);
    PASS();
}

#pragma endregion
#pragma region iteration

TEST each_visits_all(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 1);
    set_int(ht, K(2), 2);
    set_int(ht, K(3), 3);
    ht_unset(ht, K(2)); // Tombstone must be skipped

    int count = 0, sum = 0;

    HT_EACH(ht, item, {
        count++;
        sum += HT_ITEM_VALUE(int, item);
    });

    ASSERT_EQ(2, count);
    ASSERT_EQ(4, sum); // 1 + 3

    ht_free(ht);
    PASS();
}

TEST each_empty(void) {
    ht_t *ht = new_int_map();
    int count = 0;

    HT_EACH(ht, item, { count++; });
    ASSERT_EQ(0, count);

    ht_free(ht);
    PASS();
}

TEST each_after_clear(void) {
    ht_t *ht = new_int_map();

    set_int(ht, K(1), 1);
    set_int(ht, K(2), 2);
    ht_clear(ht);

    int count = 0;

    HT_EACH(ht, item, { count++; });
    ASSERT_EQ(0, count);

    ht_free(ht);
    PASS();
}

TEST each_after_resize(void) {
    // Resize rehashes all entries into new positions; iteration must still visit all of them
    ht_t *ht = new_int_map();
    int n = 50;

    for (int i = 1; i <= n; i++)
        set_int(ht, K(i), i);

    int count = 0, sum = 0;

    HT_EACH(ht, item, {
        count++;
        sum += HT_ITEM_VALUE(int, item);
    });

    ASSERT_EQ(n, count);
    ASSERT_EQ(n * (n + 1) / 2, sum); // 1+2+...+n

    ht_free(ht);
    PASS();
}

#pragma endregion
#pragma region sharing

TEST retain_returns_same_pointer(void) {
    ht_t *a = new_int_map();
    ht_t *b = ht_retain(a);

    ASSERT_EQ(a, b);

    ht_free(b);
    ht_free(a);
    PASS();
}

TEST retain_shares_data(void) {
    ht_t *a = new_int_map();
    set_int(a, K(1), 100);
    set_int(a, K(2), 200);

    ht_t *b = ht_retain(a);

    ASSERT_EQ(100, *get_int(b, K(1)));
    ASSERT_EQ(200, *get_int(b, K(2)));

    ht_free(b);
    ht_free(a);
    PASS();
}

TEST writes_visible_through_shared(void) {
    ht_t *a = new_int_map();
    ht_t *b = ht_retain(a);

    set_int(a, K(1), 42);
    ASSERT_EQ(42, *get_int(b, K(1)));

    set_int(b, K(2), 99);
    ASSERT_EQ(99, *get_int(a, K(2)));

    ht_free(a);
    ht_free(b);
    PASS();
}

TEST free_retained_keeps_data(void) {
    // Freeing one reference must not destroy data still held by the other
    ht_t *a = new_int_map();
    set_int(a, K(1), 55);

    ht_t *b = ht_retain(a);
    ht_free(a); // refc 2 → 1; data must survive

    ASSERT_EQ(55, *get_int(b, K(1)));
    ASSERT_EQ(1, ht_length(b));

    ht_free(b); // refc 1 → 0; actually freed
    PASS();
}

#pragma endregion
#pragma region lifecycle

TEST init_fin(void) {
    // Embedded (stack/struct) use via ht_init / ht_fin
    ht_t ht;
    ht_init(&ht, sizeof(int), ht_hash, ht_keycmp);
    set_int(&ht, K(1), 42);
    ASSERT_EQ(42, *get_int(&ht, K(1)));
    ht_fin(&ht);
    PASS();
}

#pragma endregion

SUITE(ht) {
    // map
    RUN_TEST(get_set);
    RUN_TEST(get_missing);
    RUN_TEST(overwrite);
    RUN_TEST(overwrite_does_not_corrupt_others);
    RUN_TEST(size_tracking);
    // unset
    RUN_TEST(unset_removes_key);
    RUN_TEST(unset_missing_is_noop);
    RUN_TEST(reinsert_after_unset);
    RUN_TEST(double_unset_is_noop);
    RUN_TEST(collision_displaced_entry_findable_after_neighbor_deleted);
    // clear
    RUN_TEST(clear_empties);
    RUN_TEST(set_after_clear);
    // resize
    RUN_TEST(resize_preserves_entries);
    RUN_TEST(resize_then_get_missing);
    // set mode
    RUN_TEST(set_mode_membership);
    // iteration
    RUN_TEST(each_visits_all);
    RUN_TEST(each_empty);
    RUN_TEST(each_after_clear);
    RUN_TEST(each_after_resize);
    // sharing
    RUN_TEST(retain_returns_same_pointer);
    RUN_TEST(retain_shares_data);
    RUN_TEST(writes_visible_through_shared);
    RUN_TEST(free_retained_keeps_data);
    // lifecycle
    RUN_TEST(init_fin);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(ht);
    GREATEST_MAIN_END();
}
