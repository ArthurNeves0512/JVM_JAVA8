#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/interpreter/method_invoke.h"
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


/* ── helpers de CP ──────────────────────────────────────────────────── */

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

static cp_info make_class_cp(u2 name_index) {
    cp_info e;
    CONSTANT_Class_info *info = malloc(sizeof(CONSTANT_Class_info));
    info->tag        = CONSTANT_Class;
    info->name_index = name_index;
    e.tag                    = CONSTANT_Class;
    e.constant_class_info    = info;
    return e;
}

static cp_info make_nat(u2 name_index, u2 descriptor_index) {
    cp_info e;
    CONSTANT_NameAndType_info *info = malloc(sizeof(CONSTANT_NameAndType_info));
    info->tag              = CONSTANT_NameAndType;
    info->name_index       = name_index;
    info->descriptor_index = descriptor_index;
    e.tag                  = CONSTANT_NameAndType;
    e.nameAndType_info     = info;
    return e;
}

static cp_info make_methodref(u2 class_index, u2 nat_index) {
    cp_info e;
    CONSTANT_Methodref_info *info = malloc(sizeof(CONSTANT_Methodref_info));
    info->tag                 = CONSTANT_Methodref;
    info->class_index         = class_index;
    info->name_and_type_index = nat_index;
    e.tag            = CONSTANT_Methodref;
    e.methodRef_info = info;
    return e;
}

static attribute_info make_attr(u2 name_idx, Code_attribute *ca) {
    attribute_info a;
    a.attribute_name_index = name_idx;
    a.attribute_length     = 0;
    a.info                 = (u1 *)ca;
    return a;
}

static Code_attribute *make_ca(u1 *code, u4 len) {
    Code_attribute *ca = malloc(sizeof(Code_attribute));
    memset(ca, 0, sizeof(Code_attribute));
    ca->code        = code;
    ca->code_length = len;
    return ca;
}

/*
 * Constrói um ClassFile com dois métodos: main e soma.
 *
 * CP layout (1-indexed):
 *  [1]  Utf8("main")
 *  [2]  Utf8("Code")
 *  [3]  Utf8("soma")
 *  [4]  Utf8("(II)I")
 *  [5]  Utf8("TestClass")
 *  [6]  Class { name_index = 5 }
 *  [7]  NameAndType { name=3, desc=4 }
 *  [8]  Methodref { class=6, nat=7 }
 *
 * main: bipush 3, bipush 4, invokestatic #8, ireturn
 * soma: iload_0, iload_1, iadd, ireturn
 */
static ClassFile *make_two_method_cf(void) {
    cp_info *cp = malloc(9 * sizeof(cp_info));
    cp[0].tag = 0;
    cp[1] = make_utf8("main");
    cp[2] = make_utf8("Code");
    cp[3] = make_utf8("soma");
    cp[4] = make_utf8("(II)I");
    cp[5] = make_utf8("TestClass");
    cp[6] = make_class_cp(5);
    cp[7] = make_nat(3, 4);
    cp[8] = make_methodref(6, 7);

    /* soma: iload_0, iload_1, iadd, ireturn */
    static u1 soma_code[] = {0x1A, 0x1B, 0x60, 0xAC};
    Code_attribute *soma_ca = make_ca(soma_code, sizeof(soma_code));
    attribute_info  soma_attr = make_attr(2, soma_ca);

    method_info *soma = malloc(sizeof(method_info));
    soma->access_flags     = 0x0009;
    soma->name_index       = 3;
    soma->descriptor_index = 4;
    soma->attributes_count = 1;
    soma->attributes       = malloc(sizeof(attribute_info));
    soma->attributes[0]    = soma_attr;

    /* main: bipush 3, bipush 4, invokestatic #8, ireturn */
    static u1 main_code[] = {0x10, 3, 0x10, 4, 0xB8, 0x00, 0x08, 0xAC};
    Code_attribute *main_ca = make_ca(main_code, sizeof(main_code));
    attribute_info  main_attr = make_attr(2, main_ca);

    method_info *main_m = malloc(sizeof(method_info));
    main_m->access_flags     = 0x0009;
    main_m->name_index       = 1;
    main_m->descriptor_index = 0;
    main_m->attributes_count = 1;
    main_m->attributes       = malloc(sizeof(attribute_info));
    main_m->attributes[0]    = main_attr;

    method_info *methods = malloc(2 * sizeof(method_info));
    methods[0] = *main_m; free(main_m);
    methods[1] = *soma;   free(soma);

    ClassFile *cf           = malloc(sizeof(ClassFile));
    memset(cf, 0, sizeof(ClassFile));
    cf->constant_pool_count = 9;
    cf->constant_pool       = cp;
    cf->methods_count       = 2;
    cf->methods             = methods;

    return cf;
}

static void free_two_method_cf(ClassFile *cf) {
    /* CP */
    for (u2 i = 1; i <= 5; i++) { free(cf->constant_pool[i].utf8_info->bytes); free(cf->constant_pool[i].utf8_info); }
    free(cf->constant_pool[6].constant_class_info);
    free(cf->constant_pool[7].nameAndType_info);
    free(cf->constant_pool[8].methodRef_info);
    free(cf->constant_pool);
    /* Methods */
    for (u2 i = 0; i < cf->methods_count; i++) {
        free((Code_attribute *)cf->methods[i].attributes[0].info);
        free(cf->methods[i].attributes);
    }
    free(cf->methods);
    free(cf);
}

/* Captura stdout em buffer (mesmo helper de test_interpreter.c) */
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


/* ── testes de parsing de descritor ─────────────────────────────────── */

static void test_conta_args_zero(void) {
    assert(conta_args("()V") == 0);
    assert(conta_args("()I") == 0);
}

static void test_conta_args_primitivos(void) {
    assert(conta_args("(I)I")   == 1);
    assert(conta_args("(II)I")  == 2);
    assert(conta_args("(III)V") == 3);
    assert(conta_args("(IJ)V")  == 2);  /* int, long */
}

static void test_conta_args_referencia(void) {
    assert(conta_args("(Ljava/lang/String;)V") == 1);
    assert(conta_args("(Ljava/lang/String;I)V") == 2);
    assert(conta_args("([I)V") == 1);  /* array de int */
}

static void test_retorno_void(void) {
    assert(retorno_void("()V")  == 1);
    assert(retorno_void("(I)V") == 1);
    assert(retorno_void("(I)I") == 0);
    assert(retorno_void("()Ljava/lang/String;") == 0);
}


/* ── teste de invokestatic com ClassFile manual ─────────────────────── */

static void test_invokestatic_soma(void) {
    /*
     * main: bipush 3, bipush 4, invokestatic soma(II)I → ireturn
     * soma: iload_0, iload_1, iadd → ireturn
     * Resultado esperado na pilha do frame de main: 7
     */
    ClassFile *cf = make_two_method_cf();

    /* Executa apenas o frame de main diretamente para verificar o retorno */
    Code_attribute *main_ca = encontraCodeAttr(&cf->methods[0], cf->constant_pool);
    assert(main_ca != NULL);

    Frame *f = criaFrame(main_ca);
    executaFrame(f, cf);
    assert(f->pilha[f->topo].inteiro == 7);

    liberaFrame(f);
    free_two_method_cf(cf);
}


/* ── teste de invokestatic recursivo: fatorial ──────────────────────── */

static void test_fatorial_class(void) {
    /* fatorial.class computa fatorial(6) = 720 */
    FILE *fp = fopen("data/examples/fatorial.class", "rb");
    assert(fp != NULL && "fatorial.class nao encontrado");

    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, fp);
    readInterfaces(cf, fp);
    readFields(cf, fp);
    readMethodsCount(cf, fp);
    readMethods(cf, fp);
    readClassFileAttributes(cf, fp);
    fclose(fp);

    char *out = capture_stdout(executaJVM, cf);
    assert(strstr(out, "720") != NULL);
    free(out);

    freeClassFile(cf);
}


/* ── teste de desvio condicional: IfTeste ────────────────────────────── */

static void test_ifteste_class(void) {
    /* IfTeste.class compara dois valores e imprime "maior" */
    FILE *fp = fopen("data/examples/IfTeste.class", "rb");
    assert(fp != NULL && "IfTeste.class nao encontrado");

    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, fp);
    readInterfaces(cf, fp);
    readFields(cf, fp);
    readMethodsCount(cf, fp);
    readMethods(cf, fp);
    readClassFileAttributes(cf, fp);
    fclose(fp);

    char *out = capture_stdout(executaJVM, cf);
    assert(strstr(out, "maior") != NULL);
    free(out);

    freeClassFile(cf);
}


int main(void) {
    printf("=== method_invoke unit tests ===\n");

    printf("\n-- conta_args / retorno_void --\n");
    RUN_TEST(test_conta_args_zero);
    RUN_TEST(test_conta_args_primitivos);
    RUN_TEST(test_conta_args_referencia);
    RUN_TEST(test_retorno_void);

    printf("\n-- invokestatic (ClassFile manual) --\n");
    RUN_TEST(test_invokestatic_soma);

    printf("\n-- integracao com .class --\n");
    RUN_TEST(test_fatorial_class);
    RUN_TEST(test_ifteste_class);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
