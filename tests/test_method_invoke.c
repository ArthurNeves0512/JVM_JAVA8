#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "interpreter/method_invoke.h"
#include "interpreter/basic_ops.h"
#include "lib/types/constant_pool.h"
#include "lib/types/consts.h"
#include "lib/types/class_file/dot_class.h"
#include "lib/types/class_file/methods_info.h"
#include "lib/types/class_file/attributes_info.h"

#define RUN_TEST(fn) do { \
    fn(); \
    printf("  PASS: " #fn "\n"); \
} while (0)

/* ── helpers de construção de CP ──────────────────────────── */

static CONSTANT_Utf8_info *make_utf8(const char *str) {
    CONSTANT_Utf8_info *u = malloc(sizeof(CONSTANT_Utf8_info));
    u->tag    = CONSTANT_Utf8;
    u->length = (u2)strlen(str);
    u->bytes  = malloc(u->length);
    memcpy(u->bytes, str, u->length);
    return u;
}

static CONSTANT_Class_info *make_class(u2 name_index) {
    CONSTANT_Class_info *c = malloc(sizeof(CONSTANT_Class_info));
    c->tag        = CONSTANT_Class;
    c->name_index = name_index;
    return c;
}

static CONSTANT_NameAndType_info *make_nat(u2 name, u2 desc) {
    CONSTANT_NameAndType_info *n = malloc(sizeof(CONSTANT_NameAndType_info));
    n->tag              = CONSTANT_NameAndType;
    n->name_index       = name;
    n->descriptor_index = desc;
    return n;
}

static CONSTANT_Methodref_info *make_methodref(u2 cls, u2 nat) {
    CONSTANT_Methodref_info *m = malloc(sizeof(CONSTANT_Methodref_info));
    m->tag                 = CONSTANT_Methodref;
    m->class_index         = cls;
    m->name_and_type_index = nat;
    return m;
}

/*
 * Constrói um ClassFile mínimo com um único método.
 *
 * CP layout (índice 1-7):
 *   [1] Utf8  method_name
 *   [2] Utf8  descriptor
 *   [3] Utf8  "Code"
 *   [4] Utf8  "Test"
 *   [5] Class { name_index: 4 }
 *   [6] NameAndType { name: 1, descriptor: 2 }
 *   [7] Methodref { class: 5, nat: 6 }
 *
 * *out_mref: preenchido com o índice do Methodref (7).
 */
static ClassFile *build_cf(const char *method_name, const char *descriptor,
                             u1 *code, u4 code_len, u2 *out_mref) {
    ClassFile *cf = allocClassFile();

    cf->constant_pool_count = 8;
    cf->constant_pool = calloc(8, sizeof(cp_info));

    cf->constant_pool[1].tag = CONSTANT_Utf8;
    cf->constant_pool[1].utf8_info = make_utf8(method_name);

    cf->constant_pool[2].tag = CONSTANT_Utf8;
    cf->constant_pool[2].utf8_info = make_utf8(descriptor);

    cf->constant_pool[3].tag = CONSTANT_Utf8;
    cf->constant_pool[3].utf8_info = make_utf8("Code");

    cf->constant_pool[4].tag = CONSTANT_Utf8;
    cf->constant_pool[4].utf8_info = make_utf8("Test");

    cf->constant_pool[5].tag = CONSTANT_Class;
    cf->constant_pool[5].constant_class_info = make_class(4);

    cf->constant_pool[6].tag = CONSTANT_NameAndType;
    cf->constant_pool[6].nameAndType_info = make_nat(1, 2);

    cf->constant_pool[7].tag = CONSTANT_Methodref;
    cf->constant_pool[7].methodRef_info = make_methodref(5, 6);

    /* Code_attribute */
    Code_attribute *ca = calloc(1, sizeof(Code_attribute));
    ca->code_length = code_len;
    ca->code        = malloc(code_len);
    memcpy(ca->code, code, code_len);

    /* attribute_info wrapping Code_attribute */
    attribute_info *attr = malloc(sizeof(attribute_info));
    attr->attribute_name_index = 3;
    attr->attribute_length     = 0;
    attr->info = (u1 *)ca;

    /* método */
    cf->methods_count = 1;
    cf->methods       = calloc(1, sizeof(method_info));
    cf->methods[0].name_index       = 1;
    cf->methods[0].descriptor_index = 2;
    cf->methods[0].attributes_count = 1;
    cf->methods[0].attributes       = attr;

    if (out_mref) *out_mref = 7;
    return cf;
}

/* ── testes de contaArgs ──────────────────────────────────── */

static void test_contaArgs_sem_args(void) {
    assert(contaArgs("()V") == 0);
    assert(contaArgs("()I") == 0);
    assert(contaArgs("()Ljava/lang/String;") == 0);
}

static void test_contaArgs_com_args(void) {
    assert(contaArgs("(I)V")                  == 1);
    assert(contaArgs("(II)I")                 == 2);
    assert(contaArgs("(III)V")                == 3);
    assert(contaArgs("(Ljava/lang/String;)V") == 1);
    assert(contaArgs("(Ljava/lang/String;I)V")== 2);
    assert(contaArgs("([I)V")                 == 1);
}

/* ── testes de eVoid ──────────────────────────────────────── */

static void test_eVoid_void(void) {
    assert(eVoid("()V")    == 1);
    assert(eVoid("(II)V")  == 1);
}

static void test_eVoid_nao_void(void) {
    assert(eVoid("()I")                   == 0);
    assert(eVoid("()Ljava/lang/String;")  == 0);
}

/* ── testes de invokestatic ───────────────────────────────── */

/* "cinco"()I → iconst_5(0x08), ireturn(0xAC) → devolve 5 */
static void test_invokestatic_sem_args_retorna_5(void) {
    u1 code[] = { 0x08, 0xAC };
    u2 mref;
    ClassFile *cf = build_cf("cinco", "()I", code, 2, &mref);

    Frame caller;
    memset(&caller, 0, sizeof(Frame));
    caller.topo = -1;

    execInvokestatic(&caller, cf, mref);

    assert(caller.topo == 0);
    assert(caller.pilha[0].inteiro == 5);

    freeClassFile(cf);
}

/* "(II)I" → iload_0(0x1A), ireturn → devolve o primeiro argumento */
static void test_invokestatic_dois_args_devolve_primeiro(void) {
    u1 code[] = { 0x1A, 0xAC };   /* iload_0, ireturn */
    u2 mref;
    ClassFile *cf = build_cf("f", "(II)I", code, 2, &mref);

    Frame caller;
    memset(&caller, 0, sizeof(Frame));
    caller.topo = -1;

    caller.pilha[++caller.topo].inteiro = 42;
    caller.pilha[caller.topo].tipo      = TIPO_INT;
    caller.pilha[++caller.topo].inteiro = 99;
    caller.pilha[caller.topo].tipo      = TIPO_INT;

    execInvokestatic(&caller, cf, mref);

    /* locais[0]=42, locais[1]=99 → iload_0 → 42 */
    assert(caller.topo == 0);
    assert(caller.pilha[0].inteiro == 42);

    freeClassFile(cf);
}

/* "(I)I" → iload_0, ireturn → devolve o único argumento */
static void test_invokestatic_um_arg(void) {
    u1 code[] = { 0x1A, 0xAC };
    u2 mref;
    ClassFile *cf = build_cf("id", "(I)I", code, 2, &mref);

    Frame caller;
    memset(&caller, 0, sizeof(Frame));
    caller.topo = -1;

    caller.pilha[++caller.topo].inteiro = 77;
    caller.pilha[caller.topo].tipo      = TIPO_INT;

    execInvokestatic(&caller, cf, mref);

    assert(caller.topo == 0);
    assert(caller.pilha[0].inteiro == 77);

    freeClassFile(cf);
}

/* método void: return(0xB1) → nada é empilhado no caller */
static void test_invokestatic_void_nao_empilha(void) {
    u1 code[] = { 0xB1 };
    u2 mref;
    ClassFile *cf = build_cf("vazio", "()V", code, 1, &mref);

    Frame caller;
    memset(&caller, 0, sizeof(Frame));
    caller.topo = -1;

    execInvokestatic(&caller, cf, mref);

    assert(caller.topo == -1);

    freeClassFile(cf);
}

/* ── testes de invokevirtual ──────────────────────────────── */

/* instância sem args: this=locais[0], método devolve iconst_3 */
static void test_invokevirtual_this_em_local0(void) {
    u1 code[] = { 0x06, 0xAC };  /* iconst_3, ireturn */
    u2 mref;
    ClassFile *cf = build_cf("valor", "()I", code, 2, &mref);

    Frame caller;
    memset(&caller, 0, sizeof(Frame));
    caller.topo = -1;

    /* empilha objectref (dummy) */
    caller.pilha[++caller.topo].longo = (int64_t)(uintptr_t)"obj";
    caller.pilha[caller.topo].tipo    = TIPO_REF;

    execInvokevirtual(&caller, cf, mref);

    assert(caller.topo == 0);
    assert(caller.pilha[0].inteiro == 3);

    freeClassFile(cf);
}

/* this em locais[0], arg em locais[1]: iload_1 → devolve arg, não this */
static void test_invokevirtual_arg_em_local1(void) {
    u1 code[] = { 0x1B, 0xAC };  /* iload_1, ireturn */
    u2 mref;
    ClassFile *cf = build_cf("getVal", "(I)I", code, 2, &mref);

    Frame caller;
    memset(&caller, 0, sizeof(Frame));
    caller.topo = -1;

    /* empilha objectref, depois arg */
    caller.pilha[++caller.topo].longo = 0xDEAD;
    caller.pilha[caller.topo].tipo    = TIPO_REF;
    caller.pilha[++caller.topo].inteiro = 55;
    caller.pilha[caller.topo].tipo      = TIPO_INT;

    execInvokevirtual(&caller, cf, mref);

    /* locais[0]=objectref, locais[1]=55 → iload_1 → 55 */
    assert(caller.topo == 0);
    assert(caller.pilha[0].inteiro == 55);

    freeClassFile(cf);
}

/* invokevirtual void: return → não empilha nada no caller */
static void test_invokevirtual_void(void) {
    u1 code[] = { 0xB1 };
    u2 mref;
    ClassFile *cf = build_cf("proc", "()V", code, 1, &mref);

    Frame caller;
    memset(&caller, 0, sizeof(Frame));
    caller.topo = -1;

    /* objectref */
    caller.pilha[++caller.topo].longo = 0;
    caller.pilha[caller.topo].tipo    = TIPO_REF;

    execInvokevirtual(&caller, cf, mref);

    assert(caller.topo == -1);

    freeClassFile(cf);
}

/* ── testes de ireturn / return (via invokestatic) ─────────── */

/* ireturn devolve o valor que está no topo do frame filho */
static void test_ireturn_devolve_topo(void) {
    u1 code[] = { 0x07, 0xAC };  /* iconst_4, ireturn */
    u2 mref;
    ClassFile *cf = build_cf("quatro", "()I", code, 2, &mref);

    Frame caller;
    memset(&caller, 0, sizeof(Frame));
    caller.topo = -1;

    execInvokestatic(&caller, cf, mref);

    assert(caller.topo == 0);
    assert(caller.pilha[0].inteiro == 4);

    freeClassFile(cf);
}

/* return void não deixa nada na pilha */
static void test_return_void_nao_altera_pilha(void) {
    u1 code[] = { 0xB1 };
    u2 mref;
    ClassFile *cf = build_cf("noop", "()V", code, 1, &mref);

    Frame caller;
    memset(&caller, 0, sizeof(Frame));
    caller.topo = -1;

    caller.pilha[++caller.topo].inteiro = 99;
    caller.pilha[caller.topo].tipo      = TIPO_INT;
    /* guarda o topo antes */
    int topo_antes = caller.topo;

    execInvokestatic(&caller, cf, mref);

    /* método void: pilha do caller deve voltar ao estado pré-chamada */
    assert(caller.topo == topo_antes);
    assert(caller.pilha[0].inteiro == 99);

    freeClassFile(cf);
}

/* ── main ────────────────────────────────────────────────── */

int main(void) {
    printf("=== test_method_invoke ===\n");
    RUN_TEST(test_contaArgs_sem_args);
    RUN_TEST(test_contaArgs_com_args);
    RUN_TEST(test_eVoid_void);
    RUN_TEST(test_eVoid_nao_void);
    RUN_TEST(test_invokestatic_sem_args_retorna_5);
    RUN_TEST(test_invokestatic_dois_args_devolve_primeiro);
    RUN_TEST(test_invokestatic_um_arg);
    RUN_TEST(test_invokestatic_void_nao_empilha);
    RUN_TEST(test_invokevirtual_this_em_local0);
    RUN_TEST(test_invokevirtual_arg_em_local1);
    RUN_TEST(test_invokevirtual_void);
    RUN_TEST(test_ireturn_devolve_topo);
    RUN_TEST(test_return_void_nao_altera_pilha);
    printf("=== Todos os testes passaram ===\n");
    return 0;
}
