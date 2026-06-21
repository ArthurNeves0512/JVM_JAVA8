#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/interpreter/heap.h"
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
    e.tag                 = CONSTANT_Class;
    e.constant_class_info = info;
    return e;
}

static cp_info make_nat(u2 name_index, u2 desc_index) {
    cp_info e;
    CONSTANT_NameAndType_info *info = malloc(sizeof(CONSTANT_NameAndType_info));
    info->tag              = CONSTANT_NameAndType;
    info->name_index       = name_index;
    info->descriptor_index = desc_index;
    e.tag              = CONSTANT_NameAndType;
    e.nameAndType_info = info;
    return e;
}

static cp_info make_fieldref(u2 class_index, u2 nat_index) {
    cp_info e;
    CONSTANT_Fieldref_info *info = malloc(sizeof(CONSTANT_Fieldref_info));
    info->tag                 = CONSTANT_Fieldref;
    info->class_index         = class_index;
    info->name_and_type_index = nat_index;
    e.tag          = CONSTANT_Fieldref;
    e.fieldRef_info = info;
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
 * Constrói um ClassFile mínimo com:
 *   - 1 campo "x" de tipo "I"
 *   - método <init>()V  (apenas return)
 *   - método main  (new Ponto, dup, invokespecial, astore_1,
 *                   aload_1, bipush 42, putfield x,
 *                   aload_1, getfield x, ireturn)
 *
 * CP layout (1-indexed):
 *  [1]  Utf8("main")
 *  [2]  Utf8("Code")
 *  [3]  Utf8("<init>")
 *  [4]  Utf8("()V")
 *  [5]  Utf8("x")
 *  [6]  Utf8("I")
 *  [7]  Utf8("Ponto")
 *  [8]  Class { name_index=7 }
 *  [9]  NameAndType { name=5, desc=6 }   ; x:I
 *  [10] Fieldref  { class=8, nat=9 }
 *  [11] NameAndType { name=3, desc=4 }   ; <init>:()V
 *  [12] Methodref { class=8, nat=11 }
 */
static ClassFile *make_object_cf(void) {
    cp_info *cp = malloc(13 * sizeof(cp_info));
    cp[0].tag = 0;
    cp[1]  = make_utf8("main");
    cp[2]  = make_utf8("Code");
    cp[3]  = make_utf8("<init>");
    cp[4]  = make_utf8("()V");
    cp[5]  = make_utf8("x");
    cp[6]  = make_utf8("I");
    cp[7]  = make_utf8("Ponto");
    cp[8]  = make_class_cp(7);
    cp[9]  = make_nat(5, 6);
    cp[10] = make_fieldref(8, 9);
    cp[11] = make_nat(3, 4);
    cp[12] = make_methodref(8, 11);

    /* field: x:I */
    field_info *fld = malloc(sizeof(field_info));
    fld->access_flags     = 0;
    fld->name_index       = 5;
    fld->descriptor_index = 6;
    fld->attributes_count = 0;
    fld->attributes       = NULL;

    /* <init>: return */
    static u1 init_code[] = {0xB1};
    Code_attribute *init_ca = make_ca(init_code, 1);
    attribute_info  init_attr = make_attr(2, init_ca);
    method_info    *init_m = malloc(sizeof(method_info));
    init_m->access_flags     = 0x0001;
    init_m->name_index       = 3;
    init_m->descriptor_index = 4;
    init_m->attributes_count = 1;
    init_m->attributes       = malloc(sizeof(attribute_info));
    init_m->attributes[0]    = init_attr;

    /*
     * main:
     *   new #8            0xBB 0x00 0x08
     *   dup               0x59
     *   invokespecial #12 0xB7 0x00 0x0C
     *   astore_1          0x4C
     *   aload_1           0x2B
     *   bipush 42         0x10 0x2A
     *   putfield #10      0xB5 0x00 0x0A
     *   aload_1           0x2B
     *   getfield #10      0xB4 0x00 0x0A
     *   ireturn           0xAC
     */
    static u1 main_code[] = {
        0xBB, 0x00, 0x08,
        0x59,
        0xB7, 0x00, 0x0C,
        0x4C,
        0x2B, 0x10, 0x2A, 0xB5, 0x00, 0x0A,
        0x2B, 0xB4, 0x00, 0x0A,
        0xAC
    };
    Code_attribute *main_ca   = make_ca(main_code, sizeof(main_code));
    attribute_info  main_attr = make_attr(2, main_ca);
    method_info    *main_m    = malloc(sizeof(method_info));
    main_m->access_flags     = 0x0009;
    main_m->name_index       = 1;
    main_m->descriptor_index = 0;
    main_m->attributes_count = 1;
    main_m->attributes       = malloc(sizeof(attribute_info));
    main_m->attributes[0]    = main_attr;

    method_info *methods = malloc(2 * sizeof(method_info));
    methods[0] = *init_m; free(init_m);
    methods[1] = *main_m; free(main_m);

    ClassFile *cf           = malloc(sizeof(ClassFile));
    memset(cf, 0, sizeof(ClassFile));
    cf->constant_pool_count = 13;
    cf->constant_pool       = cp;
    cf->fields_count        = 1;
    cf->fields              = fld;
    cf->methods_count       = 2;
    cf->methods             = methods;

    return cf;
}

static void free_object_cf(ClassFile *cf) {
    for (u2 i = 1; i <= 7; i++) { free(cf->constant_pool[i].utf8_info->bytes); free(cf->constant_pool[i].utf8_info); }
    free(cf->constant_pool[8].constant_class_info);
    free(cf->constant_pool[9].nameAndType_info);
    free(cf->constant_pool[10].fieldRef_info);
    free(cf->constant_pool[11].nameAndType_info);
    free(cf->constant_pool[12].methodRef_info);
    free(cf->constant_pool);
    free(cf->fields);
    for (u2 i = 0; i < cf->methods_count; i++) {
        free((Code_attribute *)cf->methods[i].attributes[0].info);
        free(cf->methods[i].attributes);
    }
    free(cf->methods);
    free(cf);
}

/* Captura stdout */
static char *capture_stdout(void (*fn)(ClassFile *), ClassFile *cf) {
    int pipe_fds[2]; pipe(pipe_fds);
    int saved = dup(STDOUT_FILENO);
    fflush(stdout);
    dup2(pipe_fds[1], STDOUT_FILENO); close(pipe_fds[1]);
    fn(cf); fflush(stdout);
    dup2(saved, STDOUT_FILENO); close(saved);
    char *buf = malloc(4096);
    ssize_t n = read(pipe_fds[0], buf, 4095);
    buf[n > 0 ? n : 0] = '\0';
    close(pipe_fds[0]);
    return buf;
}


/* ── testes de HeapObject ────────────────────────────────────────────── */

static void test_aloca_objeto_campos(void) {
    ClassFile *cf = make_object_cf();
    HeapObject *obj = alocaObjeto(cf, "Ponto");

    assert(obj->num_fields == 1);
    assert(strcmp(obj->field_names[0], "x") == 0);
    assert(obj->fields[0].inteiro == 0);

    free_object_cf(cf);
    liberaHeap();
}

static void test_busca_campo_encontrado(void) {
    ClassFile *cf = make_object_cf();
    HeapObject *obj = alocaObjeto(cf, "Ponto");

    assert(buscaCampo(obj, "x") == 0);
    assert(buscaCampo(obj, "y") == -1);

    free_object_cf(cf);
    liberaHeap();
}

static void test_set_get_campo_direto(void) {
    ClassFile *cf = make_object_cf();
    HeapObject *obj = alocaObjeto(cf, "Ponto");

    int fi = buscaCampo(obj, "x");
    assert(fi == 0);
    obj->fields[fi].inteiro = 99;
    obj->fields[fi].tipo    = TIPO_INT;

    assert(obj->fields[buscaCampo(obj, "x")].inteiro == 99);

    free_object_cf(cf);
    liberaHeap();
}


/* ── testes de bytecode: new + dup + putfield + getfield ─────────────── */

static void test_new_putfield_getfield(void) {
    /*
     * Executa o frame de "main" do ClassFile de Ponto:
     *   new Ponto, dup, invokespecial <init>, astore_1,
     *   aload_1, bipush 42, putfield x,
     *   aload_1, getfield x, ireturn
     * Resultado esperado: 42 no topo da pilha.
     */
    ClassFile *cf = make_object_cf();

    /* Encontra Code_attribute de main (methods[1]) */
    Code_attribute *ca = (Code_attribute *)cf->methods[1].attributes[0].info;
    Frame *f = criaFrame(ca);
    executaFrame(f, cf);

    assert(f->pilha[f->topo].inteiro == 42);

    liberaFrame(f);
    free_object_cf(cf);
    liberaHeap();
}

static void test_dup(void) {
    /* iconst_5, dup → pilha deve ter [5, 5], topo == 1 */
    u1 bytes[] = {0x08, 0x59, 0xAC}; /* iconst_5, dup, ireturn */
    Code_attribute ca;
    memset(&ca, 0, sizeof(ca));
    ca.code = bytes; ca.code_length = sizeof(bytes);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    /* ireturn pára com topo=1 (dup criou 2 cópias, ireturn mantém) */
    assert(f->pilha[f->topo].inteiro == 5);
    assert(f->topo == 1);
    liberaFrame(f);
}

static void test_pop(void) {
    /* iconst_5, iconst_3, pop → pop descarta o topo (3), deixando 5 */
    u1 bytes[] = {0x08, 0x06, 0x57, 0xAC}; /* iconst_5, iconst_3, pop, ireturn */
    Code_attribute ca;
    memset(&ca, 0, sizeof(ca));
    ca.code = bytes; ca.code_length = sizeof(bytes);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 5);
    assert(f->topo == 0);
    liberaFrame(f);
}


/* ── integração com TestFields.class ──────────────────────────────────── */

static void test_soma_class_ainda_funciona(void) {
    /* Regressao: garante que Soma.class nao quebrou com as mudancas da Fase 5 */
    FILE *fp = fopen("data/examples/Soma.class", "rb");
    assert(fp != NULL);
    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, fp); readInterfaces(cf, fp);
    readFields(cf, fp); readMethodsCount(cf, fp);
    readMethods(cf, fp); readClassFileAttributes(cf, fp);
    fclose(fp);

    char *out = capture_stdout(executaJVM, cf);
    assert(strstr(out, "30") != NULL);
    free(out);
    freeClassFile(cf);
}


int main(void) {
    printf("=== objects unit tests ===\n");

    printf("\n-- HeapObject --\n");
    RUN_TEST(test_aloca_objeto_campos);
    RUN_TEST(test_busca_campo_encontrado);
    RUN_TEST(test_set_get_campo_direto);

    printf("\n-- new / dup / pop / putfield / getfield --\n");
    RUN_TEST(test_dup);
    RUN_TEST(test_pop);
    RUN_TEST(test_new_putfield_getfield);

    printf("\n-- regressao --\n");
    RUN_TEST(test_soma_class_ainda_funciona);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
