#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/class_loader/fields_interfaces.h"

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

static void test_interfaces_count_zero(void) {
    const unsigned char bytes[] = {0x00, 0x00};
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readInterfaces(cf, f);
    assert(cf->interfaces_count == 0);
    fclose(f);
    free(cf);
}

static void test_interfaces_count_one(void) {
    const unsigned char bytes[] = {0x00, 0x01, 0x00, 0x07};
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readInterfaces(cf, f);
    assert(cf->interfaces_count == 1);
    fclose(f);
    free(cf->interfaces);
    free(cf);
}

static void test_interfaces_index_correct(void) {
    const unsigned char bytes[] = {0x00, 0x01, 0x00, 0x07};
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readInterfaces(cf, f);
    assert(cf->interfaces[0] == 7);
    fclose(f);
    free(cf->interfaces);
    free(cf);
}

static void test_interfaces_two_entries(void) {
    const unsigned char bytes[] = {0x00, 0x02, 0x00, 0x05, 0x00, 0x09};
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readInterfaces(cf, f);
    assert(cf->interfaces_count == 2);
    assert(cf->interfaces[0] == 5);
    assert(cf->interfaces[1] == 9);
    fclose(f);
    free(cf->interfaces);
    free(cf);
}

static void test_fields_count_zero(void) {
    const unsigned char bytes[] = {0x00, 0x00};
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readFields(cf, f);
    assert(cf->fields_count == 0);
    fclose(f);
    free(cf);
}

static void test_fields_count_one(void) {
    const unsigned char bytes[] = {
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x02,
        0x00, 0x03,
        0x00, 0x00
    };
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readFields(cf, f);
    assert(cf->fields_count == 1);
    fclose(f);
    free(cf->fields);
    free(cf);
}

static void test_fields_name_index_correct(void) {
    const unsigned char bytes[] = {
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x05,
        0x00, 0x03,
        0x00, 0x00
    };
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readFields(cf, f);
    assert(cf->fields[0].name_index == 5);
    fclose(f);
    free(cf->fields);
    free(cf);
}

static void test_fields_descriptor_index_correct(void) {
    const unsigned char bytes[] = {
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x02,
        0x00, 0x08,
        0x00, 0x00
    };
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readFields(cf, f);
    assert(cf->fields[0].descriptor_index == 8);
    fclose(f);
    free(cf->fields);
    free(cf);
}

static void test_opa_interfaces_count(void) {
    FILE *f = fopen("data/examples/Opa.class", "rb");
    assert(f != NULL);
    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, f);
    readInterfaces(cf, f);
    assert(cf->interfaces_count == 1);
    fclose(f);
    if (cf->interfaces) free(cf->interfaces);
    free(cf);
}

static void test_opa_fields_count_zero(void) {
    FILE *f = fopen("data/examples/Opa.class", "rb");
    assert(f != NULL);
    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, f);
    readInterfaces(cf, f);
    readFields(cf, f);
    assert(cf->fields_count == 0);
    fclose(f);
    if (cf->interfaces) free(cf->interfaces); 
    free(cf);
}

int main(void) {
    printf("=== fields_interfaces unit tests ===\n");

    printf("\n-- readInterfaces --\n");
    RUN_TEST(test_interfaces_count_zero);
    RUN_TEST(test_interfaces_count_one);
    RUN_TEST(test_interfaces_index_correct);
    RUN_TEST(test_interfaces_two_entries);

    printf("\n-- readFields --\n");
    RUN_TEST(test_fields_count_zero);
    RUN_TEST(test_fields_count_one);
    RUN_TEST(test_fields_name_index_correct);
    RUN_TEST(test_fields_descriptor_index_correct);

    printf("\n-- Integracao Opa.class --\n");
    RUN_TEST(test_opa_interfaces_count);
    RUN_TEST(test_opa_fields_count_zero);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}