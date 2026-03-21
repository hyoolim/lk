#include "../../../src/rt/vec.h"
#include "../../../src/rt/num.h"
#include "greatest.h"
#include <limits.h>
#include <math.h>

static num_type_t parse(const char *s, num_t *res) {
    vec_t *str = vec_str_alloc_from_cstr(s);
    num_type_t type = num_new(str, res);
    vec_free(str);
    return type;
}

/* -- int -- */

TEST int_zero(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_INT, parse("0", &res));
    ASSERT_EQ(0, res.i);
    PASS();
}

TEST int_single_digit(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_INT, parse("7", &res));
    ASSERT_EQ(7, res.i);
    PASS();
}

TEST int_multidigit(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_INT, parse("12345", &res));
    ASSERT_EQ(12345, res.i);
    PASS();
}

TEST int_underscores(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_INT, parse("1_000_000", &res));
    ASSERT_EQ(1000000, res.i);
    PASS();
}

TEST int_max(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_INT, parse("2147483647", &res));
    ASSERT_EQ(INT_MAX, res.i);
    PASS();
}

/* -- double -- */

TEST double_simple(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_DOUBLE, parse("1.5", &res));
    ASSERT_EQ(1.5, res.d);
    PASS();
}

TEST double_fractional_only(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_DOUBLE, parse(".5", &res));
    ASSERT_EQ(0.5, res.d);
    PASS();
}

TEST double_trailing_dot(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_DOUBLE, parse("1.", &res));
    ASSERT_EQ(1.0, res.d);
    PASS();
}

TEST double_underscores(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_DOUBLE, parse("1_000.5", &res));
    ASSERT_EQ(1000.5, res.d);
    PASS();
}

TEST double_int_overflow(void) {
    num_t res;

    /* Overflows via i > INT_MAX/10 */
    ASSERT_EQ(NUM_TYPE_DOUBLE, parse("99999999999", &res));
    ASSERT_EQ(99999999999.0, res.d);

    /* Overflows via i == INT_MAX/10 && digit > INT_MAX%10 */
    ASSERT_EQ(NUM_TYPE_DOUBLE, parse("2147483648", &res));
    ASSERT_EQ(2147483648.0, res.d);

    PASS();
}

TEST double_precision(void) {
    num_t res;
    ASSERT_EQ(NUM_TYPE_DOUBLE, parse("3.14159", &res));
    ASSERT(fabs(res.d - 3.14159) < 1e-10);
    PASS();
}

SUITE(num) {
    RUN_TEST(int_zero);
    RUN_TEST(int_single_digit);
    RUN_TEST(int_multidigit);
    RUN_TEST(int_underscores);
    RUN_TEST(int_max);
    RUN_TEST(double_simple);
    RUN_TEST(double_fractional_only);
    RUN_TEST(double_trailing_dot);
    RUN_TEST(double_underscores);
    RUN_TEST(double_int_overflow);
    RUN_TEST(double_precision);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(num);
    GREATEST_MAIN_END();
}
