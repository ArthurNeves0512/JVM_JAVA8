#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/types/class_file/dot_class.h"

static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(name) do { \
    printf("  [TEST] " #name "... "); \
    name(); \
    tests_run++; \
    tests_passed++; \
    printf("PASS\n"); \
} while (0)

static void test_alloc_returns_non_null(void) {
    ClassFile *cf = allocClassFile();
    assert(cf != NULL);
    free(cf);
}

static void test_alloc_constant_pool_is_null(void) {
    ClassFile *cf = allocClassFile();
    assert(cf->constant_pool == NULL);
    free(cf);
}

static void test_alloc_interfaces_is_null(void) {
    ClassFile *cf = allocClassFile();
    assert(cf->interfaces == NULL);
    free(cf);
}

static void test_alloc_fields_is_null(void) {
    ClassFile *cf = allocClassFile();
    assert(cf->fields == NULL);
    free(cf);
}

static void test_alloc_methods_is_null(void) {
    ClassFile *cf = allocClassFile();
    assert(cf->methods == NULL);
    free(cf);
}

static void test_alloc_attributes_is_null(void) {
    ClassFile *cf = allocClassFile();
    assert(cf->attributes == NULL);
    free(cf);
}

static void test_alloc_independent_instances(void) {
    ClassFile *cf1 = allocClassFile();
    ClassFile *cf2 = allocClassFile();
    assert(cf1 != cf2);
    free(cf1);
    free(cf2);
}

int main(void) {
    printf("=== allocClassFile unit tests ===\n");
    RUN_TEST(test_alloc_returns_non_null);
    RUN_TEST(test_alloc_constant_pool_is_null);
    RUN_TEST(test_alloc_interfaces_is_null);
    RUN_TEST(test_alloc_fields_is_null);
    RUN_TEST(test_alloc_methods_is_null);
    RUN_TEST(test_alloc_attributes_is_null);
    RUN_TEST(test_alloc_independent_instances);
    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
