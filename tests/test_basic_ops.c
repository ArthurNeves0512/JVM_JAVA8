#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib/interpreter/basic_ops.h"
#include "lib/types/attribute.h"

static int tests_run    = 0;
static int tests_passed = 0;

#define RUN_TEST(name) do { \
    printf("  [TEST] " #name "... "); \
    name(); \
    tests_run++; \
    tests_passed++; \
    printf("PASS\n"); \
} while (0)

static Code_attribute make_code(u1 *bytes, u4 len) {
    Code_attribute ca;
    memset(&ca, 0, sizeof(ca));
    ca.code        = bytes;
    ca.code_length = len;
    return ca;
}


static void test_cria_frame_topo_inicial(void) {
    u1 bytes[] = {0xB1}; /* return */
    Code_attribute ca = make_code(bytes, 1);
    Frame *f = criaFrame(&ca);
    assert(f->topo == -1);
    liberaFrame(f);
}

static void test_cria_frame_pc_inicial(void) {
    u1 bytes[] = {0xB1};
    Code_attribute ca = make_code(bytes, 1);
    Frame *f = criaFrame(&ca);
    assert(f->pc == 0);
    liberaFrame(f);
}

static void test_iconst_m1(void) {
    /* 0x02 = iconst_m1, 0xAC = ireturn */
    u1 bytes[] = {0x02, 0xAC};
    Code_attribute ca = make_code(bytes, 2);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == -1);
    liberaFrame(f);
}

static void test_iconst_0(void) {
    u1 bytes[] = {0x03, 0xAC};
    Code_attribute ca = make_code(bytes, 2);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 0);
    liberaFrame(f);
}

static void test_iconst_5(void) {
    u1 bytes[] = {0x08, 0xAC};
    Code_attribute ca = make_code(bytes, 2);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 5);
    liberaFrame(f);
}

static void test_bipush(void) {
    /* 0x10 42 = bipush 42 */
    u1 bytes[] = {0x10, 42, 0xAC};
    Code_attribute ca = make_code(bytes, 3);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 42);
    liberaFrame(f);
}

static void test_sipush(void) {
    /* 0x11 0x01 0x00 = sipush 256 */
    u1 bytes[] = {0x11, 0x01, 0x00, 0xAC};
    Code_attribute ca = make_code(bytes, 4);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 256);
    liberaFrame(f);
}


static void test_iadd(void) {
    /* iconst_3, iconst_4, iadd */
    u1 bytes[] = {0x06, 0x07, 0x60, 0xAC};
    Code_attribute ca = make_code(bytes, 4);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 7);
    liberaFrame(f);
}

static void test_isub(void) {
    u1 bytes[] = {0x08, 0x05, 0x64, 0xAC};
    Code_attribute ca = make_code(bytes, 4);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 3);
    liberaFrame(f);
}

static void test_imul(void) {
    u1 bytes[] = {0x06, 0x07, 0x68, 0xAC};
    Code_attribute ca = make_code(bytes, 4);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 12);
    liberaFrame(f);
}

static void test_idiv(void) {
    u1 bytes[] = {0x10, 10, 0x10, 2, 0x6C, 0xAC};
    Code_attribute ca = make_code(bytes, 6);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 5);
    liberaFrame(f);
}

static void test_irem(void) {
    u1 bytes[] = {0x10, 7, 0x10, 3, 0x70, 0xAC};
    Code_attribute ca = make_code(bytes, 6);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 1);
    liberaFrame(f);
}

static void test_ineg(void) {
    u1 bytes[] = {0x06, 0x74, 0xAC};
    Code_attribute ca = make_code(bytes, 3);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == -3);
    liberaFrame(f);
}


static void test_istore_iload_0(void) {
    u1 bytes[] = {0x08, 0x3B, 0x04, 0x1A, 0xAC};
    Code_attribute ca = make_code(bytes, 5);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 5);
    liberaFrame(f);
}

static void test_iinc(void) {
    u1 bytes[] = {0x05, 0x3B, 0x84, 0x00, 0x03, 0x1A, 0xAC};
    Code_attribute ca = make_code(bytes, 7);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 5);
    liberaFrame(f);
}


static void test_ifeq_desvia(void) {
    u1 bytes[] = {
        0x03,        
        0x99, 0x00, 0x05, 
        0x10, 99,   
        0x04,        
        0xAC         
    };
    Code_attribute ca = make_code(bytes, 8);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 1);
    liberaFrame(f);
}

static void test_goto(void) {
    u1 bytes[] = {
        0xA7, 0x00, 0x05, 
        0x10, 99,         
        0x05,            
        0xAC              
    };
    Code_attribute ca = make_code(bytes, 7);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 2);
    liberaFrame(f);
}


static void test_if_icmpeq_igual(void) {
    u1 bytes[] = {
        0x06,             
        0x06,             
        0x9F, 0x00, 0x05, 
        0x10, 99,         
        0x04,             
        0xAC
    };
    Code_attribute ca = make_code(bytes, 9);
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 1);
    liberaFrame(f);
}

int main(void) {
    printf("=== basic_ops unit tests ===\n");

    printf("\n-- criaFrame / liberaFrame --\n");
    RUN_TEST(test_cria_frame_topo_inicial);
    RUN_TEST(test_cria_frame_pc_inicial);

    printf("\n-- constantes inteiras --\n");
    RUN_TEST(test_iconst_m1);
    RUN_TEST(test_iconst_0);
    RUN_TEST(test_iconst_5);
    RUN_TEST(test_bipush);
    RUN_TEST(test_sipush);

    printf("\n-- aritmetica inteira --\n");
    RUN_TEST(test_iadd);
    RUN_TEST(test_isub);
    RUN_TEST(test_imul);
    RUN_TEST(test_idiv);
    RUN_TEST(test_irem);
    RUN_TEST(test_ineg);

    printf("\n-- istore / iload / iinc --\n");
    RUN_TEST(test_istore_iload_0);
    RUN_TEST(test_iinc);

    printf("\n-- desvio condicional --\n");
    RUN_TEST(test_ifeq_desvia);
    RUN_TEST(test_goto);
    RUN_TEST(test_if_icmpeq_igual);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}