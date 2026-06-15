#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/class_loader/loader.h"
#include "lib/class_loader/fields_interfaces.h"
#include "lib/class_loader/methods.h"
#include "lib/file/read_file.h"
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

static ClassFile *parse(const char *path) {
    FILE *fp = readFile((char *) path);
    assert(fp != NULL);

    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, fp);
    readInterfaces(cf, fp);
    readFields(cf, fp);
    readMethodsCount(cf, fp);
    readMethods(cf, fp);
    readClassFileAttributes(cf, fp);

    fclose(fp);
    return cf;
}

static void test_alloc_then_free_empty(void) {
    ClassFile *cf = allocClassFile();
    freeClassFile(cf);
}

static void test_free_null_does_not_crash(void) {
    freeClassFile(NULL);
}

static void test_full_parse_and_free(void) {
    ClassFile *cf = parse("data/examples/Opa.class");
    assert(cf->magic == 0xCAFEBABE);
    freeClassFile(cf);
}

static void test_full_parse_and_free_with_fields(void) {
    ClassFile *cf = parse("data/examples/TestFields.class");
    assert(cf->magic == 0xCAFEBABE);
    freeClassFile(cf);
}

int main(void) {
    printf("=== ClassFile lifecycle unit tests ===\n");
    RUN_TEST(test_alloc_then_free_empty);
    RUN_TEST(test_free_null_does_not_crash);
    RUN_TEST(test_full_parse_and_free);
    RUN_TEST(test_full_parse_and_free_with_fields);
    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
