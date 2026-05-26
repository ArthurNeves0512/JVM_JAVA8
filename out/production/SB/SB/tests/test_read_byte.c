#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/file/read_byte.h"

static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(name) do { \
    printf("  [TEST] " #name "... "); \
    name(); \
    tests_run++; \
    tests_passed++; \
    printf("PASS\n"); \
} while (0)

static FILE *make_file(const unsigned char *bytes, size_t len) {
    FILE *f = tmpfile();
    fwrite(bytes, 1, len, f);
    rewind(f);
    return f;
}

/* u1Read */

static void test_u1read_correct_value(void) {
    const unsigned char bytes[] = {0xAB};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u1Read(f) == 0xAB);
    fclose(f);
}

static void test_u1read_zero(void) {
    const unsigned char bytes[] = {0x00};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u1Read(f) == 0x00);
    fclose(f);
}

static void test_u1read_max(void) {
    const unsigned char bytes[] = {0xFF};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u1Read(f) == 0xFF);
    fclose(f);
}

static void test_u1read_advances_pointer(void) {
    const unsigned char bytes[] = {0x01, 0x02};
    FILE *f = make_file(bytes, sizeof(bytes));
    u1Read(f);
    assert(ftell(f) == 1);
    fclose(f);
}

static void test_u1read_sequential(void) {
    const unsigned char bytes[] = {0x11, 0x22};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u1Read(f) == 0x11);
    assert(u1Read(f) == 0x22);
    fclose(f);
}

/* u2Read */

static void test_u2read_big_endian(void) {
    const unsigned char bytes[] = {0xCA, 0xFE};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u2Read(f) == 0xCAFE);
    fclose(f);
}

static void test_u2read_zero(void) {
    const unsigned char bytes[] = {0x00, 0x00};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u2Read(f) == 0x0000);
    fclose(f);
}

static void test_u2read_max(void) {
    const unsigned char bytes[] = {0xFF, 0xFF};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u2Read(f) == 0xFFFF);
    fclose(f);
}

static void test_u2read_advances_pointer(void) {
    const unsigned char bytes[] = {0x00, 0x34, 0x00, 0x0B};
    FILE *f = make_file(bytes, sizeof(bytes));
    u2Read(f);
    assert(ftell(f) == 2);
    fclose(f);
}

static void test_u2read_sequential(void) {
    const unsigned char bytes[] = {0x00, 0x34, 0x00, 0x0B};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u2Read(f) == 0x0034);
    assert(u2Read(f) == 0x000B);
    fclose(f);
}

/* u4Read */

static void test_u4read_big_endian(void) {
    const unsigned char bytes[] = {0xCA, 0xFE, 0xBA, 0xBE};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u4Read(f) == 0xCAFEBABE);
    fclose(f);
}

static void test_u4read_zero(void) {
    const unsigned char bytes[] = {0x00, 0x00, 0x00, 0x00};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u4Read(f) == 0x00000000);
    fclose(f);
}

static void test_u4read_max(void) {
    const unsigned char bytes[] = {0xFF, 0xFF, 0xFF, 0xFF};
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u4Read(f) == 0xFFFFFFFF);
    fclose(f);
}

static void test_u4read_advances_pointer(void) {
    const unsigned char bytes[] = {0xCA, 0xFE, 0xBA, 0xBE, 0x00};
    FILE *f = make_file(bytes, sizeof(bytes));
    u4Read(f);
    assert(ftell(f) == 4);
    fclose(f);
}

static void test_u4read_sequential(void) {
    const unsigned char bytes[] = {
        0xCA, 0xFE, 0xBA, 0xBE,
        0x00, 0x00, 0x00, 0x34
    };
    FILE *f = make_file(bytes, sizeof(bytes));
    assert(u4Read(f) == 0xCAFEBABE);
    assert(u4Read(f) == 0x00000034);
    fclose(f);
}

/* Integração com A.class */

static void test_aclass_magic_number(void) {
    FILE *f = fopen("data/examples/A.class", "rb");
    assert(f != NULL);
    assert(u4Read(f) == 0xCAFEBABE);
    fclose(f);
}

static void test_aclass_major_version(void) {
    FILE *f = fopen("data/examples/A.class", "rb");
    assert(f != NULL);
    u4Read(f);           /* magic */
    u2Read(f);           /* minor_version */
    assert(u2Read(f) == 52); /* major_version: Java 8 */
    fclose(f);
}

int main(void) {
    printf("=== read_byte unit tests ===\n");

    printf("\n-- u1Read --\n");
    RUN_TEST(test_u1read_correct_value);
    RUN_TEST(test_u1read_zero);
    RUN_TEST(test_u1read_max);
    RUN_TEST(test_u1read_advances_pointer);
    RUN_TEST(test_u1read_sequential);

    printf("\n-- u2Read --\n");
    RUN_TEST(test_u2read_big_endian);
    RUN_TEST(test_u2read_zero);
    RUN_TEST(test_u2read_max);
    RUN_TEST(test_u2read_advances_pointer);
    RUN_TEST(test_u2read_sequential);

    printf("\n-- u4Read --\n");
    RUN_TEST(test_u4read_big_endian);
    RUN_TEST(test_u4read_zero);
    RUN_TEST(test_u4read_max);
    RUN_TEST(test_u4read_advances_pointer);
    RUN_TEST(test_u4read_sequential);

    printf("\n-- Integração A.class --\n");
    RUN_TEST(test_aclass_magic_number);
    RUN_TEST(test_aclass_major_version);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
