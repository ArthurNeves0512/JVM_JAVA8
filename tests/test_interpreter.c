#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/interpreter/interpreter.h"
#include "lib/interpreter/basic_ops.h"
#include "lib/types/attribute.h"
#include "lib/types/constant_pool.h"
#include "lib/types/class_file/cp_info.h"
#include "lib/types/consts.h"
#include "lib/class_loader/loader.h"
#include "lib/class_loader/fields_interfaces.h"
#include "lib/class_loader/methods.h"


static int tests_run    = 0;
static int tests_passed = 0;

#define RUN_TEST(name) do { \
    printf("  [TEST] " #name "... "); \
    name(); \
    tests_run++; \
    tests_passed++; \
    printf("PASS\n"); \
} while (0)


/* ── helpers para construir ClassFile mínimo ──────────────────────── */

static cp_info make_utf8(const char *str) {
    cp_info e;
    CONSTANT_Utf8_info *info = malloc(sizeof(CONSTANT_Utf8_info));
    info->tag    = CONSTANT_Utf8;
    info->length = (u2)strlen(str);
    info->bytes  = malloc(info->length + 1);
    memcpy(info->bytes, str, info->length + 1);
    e.tag       = CONSTANT_Utf8;
    e.utf8_info = info;
    return e;
}

/*
 * Constrói um ClassFile mínimo com um único método chamado "main"
 * cujo bytecode é fornecido pelo chamador.
 *
 * Layout do constant pool (1-indexed):
 *   cp[1] = Utf8("main")
 *   cp[2] = Utf8("Code")
 */
static ClassFile *make_classfile(u1 *bytecode, u4 code_len) {
    /* constant pool: slots 0..2 (index 0 não usado pela JVM) */
    cp_info *cp = malloc(3 * sizeof(cp_info));
    cp[0].tag   = 0; /* slot vazio */
    cp[1]       = make_utf8("main");
    cp[2]       = make_utf8("Code");

    /* Code_attribute */
    Code_attribute *ca  = malloc(sizeof(Code_attribute));
    ca->max_stack       = 8;
    ca->max_locals      = 8;
    ca->code            = bytecode;
    ca->code_length     = code_len;
    ca->exception_table_length = 0;
    ca->exception_table = NULL;
    ca->attributes_count = 0;
    ca->attributes      = NULL;

    /* attribute_info apontando para o Code_attribute */
    attribute_info *attr = malloc(sizeof(attribute_info));
    attr->attribute_name_index = 2; /* "Code" */
    attr->attribute_length     = 0;
    attr->info                 = (u1 *)ca;

    /* method_info */
    method_info *m      = malloc(sizeof(method_info));
    m->access_flags     = 0x0009; /* public static */
    m->name_index       = 1;      /* "main" */
    m->descriptor_index = 0;
    m->attributes_count = 1;
    m->attributes       = attr;

    /* ClassFile */
    ClassFile *cf           = malloc(sizeof(ClassFile));
    memset(cf, 0, sizeof(ClassFile));
    cf->constant_pool_count = 3;
    cf->constant_pool       = cp;
    cf->methods_count       = 1;
    cf->methods             = m;

    return cf;
}

static void free_minimal_classfile(ClassFile *cf) {
    free(cf->constant_pool[1].utf8_info->bytes);
    free(cf->constant_pool[1].utf8_info);
    free(cf->constant_pool[2].utf8_info->bytes);
    free(cf->constant_pool[2].utf8_info);
    free(cf->constant_pool);
    Code_attribute *ca = (Code_attribute *)cf->methods[0].attributes[0].info;
    free(ca);
    free(cf->methods[0].attributes);
    free(cf->methods);
    free(cf);
}

/* Captura stdout num pipe e retorna o texto (deve ser liberado pelo chamador) */
static char *capture_stdout(void (*fn)(ClassFile *), ClassFile *cf) {
    int pipe_fds[2];
    pipe(pipe_fds);

    int saved = dup(STDOUT_FILENO);
    fflush(stdout);
    dup2(pipe_fds[1], STDOUT_FILENO);
    close(pipe_fds[1]);

    fn(cf);
    fflush(stdout);

    dup2(saved, STDOUT_FILENO);
    close(saved);

    char *buf = malloc(4096);
    ssize_t n = read(pipe_fds[0], buf, 4095);
    buf[n > 0 ? n : 0] = '\0';
    close(pipe_fds[0]);
    return buf;
}


/* ── testes ───────────────────────────────────────────────────────── */

static void test_main_return_sem_crash(void) {
    u1 code[] = {0xB1}; /* return */
    ClassFile *cf = make_classfile(code, sizeof(code));
    executaJVM(cf); /* não deve crashar */
    free_minimal_classfile(cf);
}

static void test_main_nao_encontrado(void) {
    /* ClassFile sem nenhum método: executaJVM imprime aviso e retorna */
    ClassFile *cf        = malloc(sizeof(ClassFile));
    memset(cf, 0, sizeof(ClassFile));
    cf->constant_pool    = NULL;
    cf->methods_count    = 0;
    cf->methods          = NULL;

    /* redireciona stderr para silenciar o aviso neste teste */
    int saved_err = dup(STDERR_FILENO);
    FILE *devnull = fopen("/dev/null", "w");
    dup2(fileno(devnull), STDERR_FILENO);
    executaJVM(cf);
    fflush(stderr);
    dup2(saved_err, STDERR_FILENO);
    close(saved_err);
    fclose(devnull);

    free(cf);
}

static void test_println_int_soma(void) {
    /* Carrega Soma.class: main faz a=10, b=20, c=a+b, println(c) → "30" */
    FILE *fp = fopen("data/examples/Soma.class", "rb");
    assert(fp != NULL && "Soma.class nao encontrado");

    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, fp);
    readInterfaces(cf, fp);
    readFields(cf, fp);
    readMethodsCount(cf, fp);
    readMethods(cf, fp);
    readClassFileAttributes(cf, fp);
    fclose(fp);

    char *out = capture_stdout(executaJVM, cf);
    assert(strstr(out, "30") != NULL);
    free(out);

    freeClassFile(cf);
}

static void test_println_string_opa(void) {
    /* Carrega Opa.class: main faz println("opaa sera que deu certo") */
    FILE *fp = fopen("data/examples/Opa.class", "rb");
    assert(fp != NULL && "Opa.class nao encontrado");

    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, fp);
    readInterfaces(cf, fp);
    readFields(cf, fp);
    readMethodsCount(cf, fp);
    readMethods(cf, fp);
    readClassFileAttributes(cf, fp);
    fclose(fp);

    char *out = capture_stdout(executaJVM, cf);
    assert(strstr(out, "opaa") != NULL);
    free(out);

    freeClassFile(cf);
}


int main(void) {
    printf("=== interpreter unit tests ===\n");

    printf("\n-- executaJVM basico --\n");
    RUN_TEST(test_main_return_sem_crash);
    RUN_TEST(test_main_nao_encontrado);

    printf("\n-- println integracao --\n");
    RUN_TEST(test_println_int_soma);
    RUN_TEST(test_println_string_opa);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
