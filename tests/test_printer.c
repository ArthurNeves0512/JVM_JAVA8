#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/printer/printer.h"
#include "lib/types/class_file/dot_class.h"
#include "lib/types/constant_pool.h"
#include "lib/types/class_file/cp_info.h"
#include "lib/types/consts.h"

static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(name) do { \
    printf("  [TEST] " #name "... "); \
    name(); \
    tests_run++; \
    tests_passed++; \
    printf("PASS\n"); \
} while (0)

/* Redirects stdout to a tmpfile, runs print_fn(cf), restores stdout.
   Caller must free() the returned buffer. */
static char *capture(void (*print_fn)(const ClassFile *), const ClassFile *cf) {
    FILE *tmp = tmpfile();
    int saved = dup(STDOUT_FILENO);
    dup2(fileno(tmp), STDOUT_FILENO);

    print_fn(cf);
    fflush(stdout);

    dup2(saved, STDOUT_FILENO);
    close(saved);

    fseek(tmp, 0, SEEK_END);
    long size = ftell(tmp);
    rewind(tmp);

    char *buf = malloc((size_t)size + 1);
    fread(buf, 1, (size_t)size, tmp);
    buf[size] = '\0';
    fclose(tmp);
    return buf;
}

static char *capture_print(const ClassFile *cf) {
    return capture(printClassFile, cf);
}

static void test_null_does_not_crash(void) {
    /* printClassFile(NULL) should write to stderr and return safely */
    int saved_err = dup(STDERR_FILENO);
    FILE *devnull = fopen("/dev/null", "w");
    dup2(fileno(devnull), STDERR_FILENO);

    printClassFile(NULL);

    fflush(stderr);
    dup2(saved_err, STDERR_FILENO);
    close(saved_err);
    fclose(devnull);
}

static void test_magic_number_is_printed(void) {
    ClassFile cf = {0};
    cf.magic               = 0xCAFEBABE;
    cf.major_version       = 52;
    cf.minor_version       = 0;
    cf.constant_pool_count = 1; /* count 1 → zero entries (count-1 = 0) */
    cf.constant_pool       = NULL;
    cf.access_flags        = 0x0000;

    char *out = capture_print(&cf);
    assert(strstr(out, "CAFEBABE") != NULL);
    free(out);
}

static void test_version_is_printed(void) {
    ClassFile cf = {0};
    cf.magic               = 0xCAFEBABE;
    cf.major_version       = 52;
    cf.minor_version       = 0;
    cf.constant_pool_count = 1;
    cf.constant_pool       = NULL;

    char *out = capture_print(&cf);
    assert(strstr(out, "52") != NULL);
    free(out);
}

static void test_access_flag_public_is_printed(void) {
    ClassFile cf = {0};
    cf.magic               = 0xCAFEBABE;
    cf.constant_pool_count = 1;
    cf.constant_pool       = NULL;
    cf.access_flags        = 0x0021; /* ACC_PUBLIC | ACC_SUPER */

    char *out = capture_print(&cf);
    assert(strstr(out, "ACC_PUBLIC") != NULL);
    assert(strstr(out, "ACC_SUPER") != NULL);
    free(out);
}

static void test_constant_pool_class_entry_is_printed(void) {
    CONSTANT_Class_info class_info = { .tag = CONSTANT_Class, .name_index = 7 };
    CONSTANT_Utf8_info name_info = { .tag = CONSTANT_Utf8, .length = 4, .bytes = (u1 *) "Test" };

    cp_info entries[8] = {0}; /* index 0 unused; constant pool é 1-indexed */
    entries[1].tag                  = CONSTANT_Class;
    entries[1].constant_class_info  = &class_info;
    entries[7].tag                  = CONSTANT_Utf8;
    entries[7].utf8_info            = &name_info;

    ClassFile cf = {0};
    cf.magic               = 0xCAFEBABE;
    cf.constant_pool_count = 8; /* entries[1] (Class) references entries[7] (Utf8) */
    cf.constant_pool       = entries;

    char *out = capture_print(&cf);
    assert(strstr(out, "<Class>") != NULL);
    assert(strstr(out, "7") != NULL);
    assert(strstr(out, "Test") != NULL);
    free(out);
}

static void test_print_interfaces(void) {
    CONSTANT_Class_info class_info = { .tag = CONSTANT_Class, .name_index = 2 };
    CONSTANT_Utf8_info name_info = {
        .tag = CONSTANT_Utf8, .length = 13, .bytes = (u1 *) "TestInterface"
    };

    cp_info entries[3] = {0};
    entries[1].tag                 = CONSTANT_Class;
    entries[1].constant_class_info = &class_info;
    entries[2].tag                 = CONSTANT_Utf8;
    entries[2].utf8_info           = &name_info;

    u2 interfaces[] = { 1 };

    ClassFile cf = {0};
    cf.constant_pool_count = 3;
    cf.constant_pool       = entries;
    cf.interfaces_count    = 1;
    cf.interfaces          = interfaces;

    char *out = capture(printInterfaces, &cf);
    assert(strstr(out, "interfaces_count: 1") != NULL);
    assert(strstr(out, "TestInterface") != NULL);
    free(out);
}

static void test_print_fields(void) {
    CONSTANT_Utf8_info field_name = {
        .tag = CONSTANT_Utf8, .length = 5, .bytes = (u1 *) "count"
    };
    CONSTANT_Utf8_info field_descriptor = {
        .tag = CONSTANT_Utf8, .length = 1, .bytes = (u1 *) "I"
    };

    cp_info entries[3] = {0};
    entries[1].tag       = CONSTANT_Utf8;
    entries[1].utf8_info = &field_name;
    entries[2].tag       = CONSTANT_Utf8;
    entries[2].utf8_info = &field_descriptor;

    field_info field = {0};
    field.access_flags     = ACC_PRIVATE | ACC_STATIC;
    field.name_index       = 1;
    field.descriptor_index = 2;
    field.attributes_count = 0;
    field.attributes       = NULL;

    ClassFile cf = {0};
    cf.constant_pool_count = 3;
    cf.constant_pool       = entries;
    cf.fields_count        = 1;
    cf.fields              = &field;

    char *out = capture(printFields, &cf);
    assert(strstr(out, "fields_count: 1") != NULL);
    assert(strstr(out, "count") != NULL);
    assert(strstr(out, "ACC_PRIVATE") != NULL);
    assert(strstr(out, "ACC_STATIC") != NULL);
    free(out);
}

int main(void) {
    printf("=== printer unit tests ===\n");
    RUN_TEST(test_null_does_not_crash);
    RUN_TEST(test_magic_number_is_printed);
    RUN_TEST(test_version_is_printed);
    RUN_TEST(test_access_flag_public_is_printed);
    RUN_TEST(test_constant_pool_class_entry_is_printed);
    RUN_TEST(test_print_interfaces);
    RUN_TEST(test_print_fields);
    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
