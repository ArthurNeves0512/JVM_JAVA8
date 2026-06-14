#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/interpreter/basic_ops.h"

static int tests_run = 0, tests_passed = 0;

#define RUN_TEST(name) do { \
    printf("  [TEST] " #name "... "); \
    name(); \
    tests_run++; tests_passed++; \
    printf("PASS\n"); \
} while (0)

static Code_attribute *faz_code(u1 *bytes, u4 tam) {
    Code_attribute *ca = (Code_attribute *)malloc(sizeof(Code_attribute));
    ca->max_stack = 16; ca->max_locals = 16;
    ca->code_length = tam; ca->code = bytes;
    ca->exception_table_length = 0; ca->exception_table = NULL;
    ca->attributes_count = 0; ca->attributes = NULL;
    return ca;
}

static void test_iconst(void) {
    u1 bytes[] = { 0x08, 0xB1 }; /* iconst_5, return */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[0].inteiro == 5);
    liberaFrame(f); free(ca);
}

static void test_bipush(void) {
    u1 bytes[] = { 0x10, 0x2A, 0xB1 }; /* bipush 42 */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[0].inteiro == 42);
    liberaFrame(f); free(ca);
}

static void test_iadd(void) {
    u1 bytes[] = { 0x06, 0x07, 0x60, 0xB1 }; /* iconst_3, iconst_4, iadd */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[0].inteiro == 7);
    liberaFrame(f); free(ca);
}

static void test_isub(void) {
    u1 bytes[] = { 0x08, 0x05, 0x64, 0xB1 }; /* iconst_5, iconst_2, isub */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[0].inteiro == 3);
    liberaFrame(f); free(ca);
}

static void test_imul(void) {
    u1 bytes[] = { 0x06, 0x07, 0x68, 0xB1 }; /* iconst_3, iconst_4, imul */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[0].inteiro == 12);
    liberaFrame(f); free(ca);
}

static void test_idiv(void) {
    u1 bytes[] = { 0x10, 0x0A, 0x10, 0x03, 0x6C, 0xB1 }; /* bipush 10, bipush 3, idiv */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[0].inteiro == 3);
    liberaFrame(f); free(ca);
}

static void test_irem(void) {
    u1 bytes[] = { 0x10, 0x0A, 0x10, 0x03, 0x70, 0xB1 }; /* bipush 10, bipush 3, irem */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[0].inteiro == 1);
    liberaFrame(f); free(ca);
}

static void test_ineg(void) {
    u1 bytes[] = { 0x06, 0x74, 0xB1 }; /* iconst_3, ineg */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[0].inteiro == -3);
    liberaFrame(f); free(ca);
}

static void test_istore_iload(void) {
    u1 bytes[] = { 0x10, 0x63, 0x3C, 0x1B, 0xB1 }; /* bipush 99, istore_1, iload_1 */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 99);
    liberaFrame(f); free(ca);
}

static void test_iinc(void) {
    /* bipush 5, istore_0, iinc 0 3, iload_0 */
    u1 bytes[] = { 0x10, 0x05, 0x3B, 0x84, 0x00, 0x03, 0x1A, 0xB1 };
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 8);
    liberaFrame(f); free(ca);
}

static void test_ifeq_desvia(void) {
    /* iconst_0, ifeq +5, iconst_1, return, iconst_2, return */
    u1 bytes[] = { 0x03, 0x99, 0x00, 0x05, 0x04, 0xB1, 0x05, 0xB1 };
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 2);
    liberaFrame(f); free(ca);
}

static void test_ifeq_nao_desvia(void) {
    u1 bytes[] = { 0x04, 0x99, 0x00, 0x05, 0x04, 0xB1, 0x05, 0xB1 };
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 1);
    liberaFrame(f); free(ca);
}

static void test_if_icmpeq(void) {
    /* bipush 7, bipush 7, if_icmpeq +5, iconst_0, return, iconst_1, return */
    u1 bytes[] = { 0x10, 0x07, 0x10, 0x07, 0x9F, 0x00, 0x05, 0x03, 0xB1, 0x04, 0xB1 };
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 1);
    liberaFrame(f); free(ca);
}

static void test_ior(void) {
    u1 bytes[] = { 0x10, 0x05, 0x10, 0x03, 0x80, 0xB1 }; /* 5 | 3 = 7 */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 7);
    liberaFrame(f); free(ca);
}

static void test_iand(void) {
    u1 bytes[] = { 0x10, 0x05, 0x10, 0x03, 0x7E, 0xB1 }; /* 5 & 3 = 1 */
    Code_attribute *ca = faz_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 1);
    liberaFrame(f); free(ca);
}

int main(void) {
    printf("=== basic_ops unit tests ===\n");

    printf("\n-- constantes --\n");
    RUN_TEST(test_iconst);
    RUN_TEST(test_bipush);

    printf("\n-- aritmetica --\n");
    RUN_TEST(test_iadd);
    RUN_TEST(test_isub);
    RUN_TEST(test_imul);
    RUN_TEST(test_idiv);
    RUN_TEST(test_irem);
    RUN_TEST(test_ineg);

    printf("\n-- istore / iload / iinc --\n");
    RUN_TEST(test_istore_iload);
    RUN_TEST(test_iinc);

    printf("\n-- desvios --\n");
    RUN_TEST(test_ifeq_desvia);
    RUN_TEST(test_ifeq_nao_desvia);
    RUN_TEST(test_if_icmpeq);

    printf("\n-- bit a bit --\n");
    RUN_TEST(test_ior);
    RUN_TEST(test_iand);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}