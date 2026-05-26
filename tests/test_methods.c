#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/class_loader/fields_interfaces.h"
#include "lib/class_loader/methods.h"

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

static void test_methods_count_zero(void) {
    const unsigned char bytes[] = {0x00, 0x00};
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readMethodsCount(cf, f);
    assert(cf->methods_count == 0);
    fclose(f);
    free(cf);
}

static void test_methods_count_one(void) {
    /* methods_count=1, access_flags=0x0001, name_index=0x0002,
       descriptor_index=0x0003, attributes_count=0x0000 */
    const unsigned char bytes[] = {
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x02,
        0x00, 0x03,
        0x00, 0x00
    };
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readMethodsCount(cf, f);
    readMethods(cf, f);
    assert(cf->methods_count == 1);
    assert(cf->methods[0].access_flags == 1);
    assert(cf->methods[0].name_index == 2);
    assert(cf->methods[0].descriptor_index == 3);
    fclose(f);
    free(cf->methods);
    free(cf);
}

static void test_methods_name_index_correct(void) {
    const unsigned char bytes[] = {
        0x00, 0x01,
        0x00, 0x09,
        0x00, 0x05,
        0x00, 0x07,
        0x00, 0x00
    };
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readMethodsCount(cf, f);
    readMethods(cf, f);
    assert(cf->methods[0].name_index == 5);
    fclose(f);
    free(cf->methods);
    free(cf);
}

static void test_methods_descriptor_index_correct(void) {
    const unsigned char bytes[] = {
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x02,
        0x00, 0x0B,
        0x00, 0x00
    };
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readMethodsCount(cf, f);
    readMethods(cf, f);
    assert(cf->methods[0].descriptor_index == 11);
    fclose(f);
    free(cf->methods);
    free(cf);
}

static void test_methods_attribute_bytes_skipped(void) {
    /* 1 method with 1 attribute (name_index=0x0001, length=4, 4 payload bytes).
       After readMethods the file cursor should be at EOF. */
    const unsigned char bytes[] = {
        0x00, 0x01,          /* methods_count = 1 */
        0x00, 0x01,          /* access_flags */
        0x00, 0x02,          /* name_index */
        0x00, 0x03,          /* descriptor_index */
        0x00, 0x01,          /* attributes_count = 1 */
        0x00, 0x0A,          /* attribute_name_index */
        0x00, 0x00, 0x00, 0x04,  /* attribute_length = 4 */
        0xDE, 0xAD, 0xBE, 0xEF  /* 4 payload bytes to skip */
    };
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readMethodsCount(cf, f);
    readMethods(cf, f);
    assert(fgetc(f) == EOF);
    fclose(f);
    free(cf->methods);
    free(cf);
}

static void test_methods_two_entries(void) {
    const unsigned char bytes[] = {
        0x00, 0x02,  /* methods_count = 2 */
        /* method 0 */
        0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00,
        /* method 1 */
        0x00, 0x09, 0x00, 0x0A, 0x00, 0x0B, 0x00, 0x00
    };
    FILE *f = make_file(bytes, sizeof(bytes));
    ClassFile *cf = allocClassFile();
    readMethodsCount(cf, f);
    readMethods(cf, f);
    assert(cf->methods_count == 2);
    assert(cf->methods[0].name_index == 2);
    assert(cf->methods[1].name_index == 10);
    fclose(f);
    free(cf->methods);
    free(cf);
}

static void test_opa_methods_count(void) {
    /* Opa.java has <init>()V and main([Ljava/lang/String;)V → 2 methods */
    FILE *f = fopen("data/examples/Opa.class", "rb");
    assert(f != NULL);
    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, f);
    readInterfaces(cf, f);
    readFields(cf, f);
    readMethodsCount(cf, f);
    readMethods(cf, f);
    assert(cf->methods_count == 2);
    fclose(f);
    free(cf->methods);
    free(cf->interfaces);
    free(cf);
}

int main(void) {
    printf("=== methods unit tests ===\n");

    printf("\n-- readMethodsCount --\n");
    RUN_TEST(test_methods_count_zero);

    printf("\n-- readMethods --\n");
    RUN_TEST(test_methods_count_one);
    RUN_TEST(test_methods_name_index_correct);
    RUN_TEST(test_methods_descriptor_index_correct);
    RUN_TEST(test_methods_attribute_bytes_skipped);
    RUN_TEST(test_methods_two_entries);

    printf("\n-- Integracao Opa.class --\n");
    RUN_TEST(test_opa_methods_count);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
