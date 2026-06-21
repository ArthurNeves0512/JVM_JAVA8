#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib/interpreter/jvm_stack.h"


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


static void test_cria_stack_vazia(void) {
    JVMStack *s = criaJVMStack();
    assert(s->topo == -1);
    free(s);
}

static void test_frame_atual_vazia_retorna_null(void) {
    JVMStack *s = criaJVMStack();
    assert(frameAtual(s) == NULL);
    free(s);
}

static void test_empilha_um_frame(void) {
    u1 code[] = {0xB1};
    Code_attribute ca = make_code(code, 1);
    Frame *f = criaFrame(&ca);

    JVMStack *s = criaJVMStack();
    empilhaFrame(s, f);

    assert(s->topo == 0);
    assert(frameAtual(s) == f);

    /* libera sem usar liberaJVMStack para testar empilha isolado */
    free(s);
    liberaFrame(f);
}

static void test_empilha_dois_frames(void) {
    u1 code[] = {0xB1};
    Code_attribute ca = make_code(code, 1);
    Frame *f1 = criaFrame(&ca);
    Frame *f2 = criaFrame(&ca);

    JVMStack *s = criaJVMStack();
    empilhaFrame(s, f1);
    empilhaFrame(s, f2);

    assert(s->topo == 1);
    assert(frameAtual(s) == f2);

    free(s);
    liberaFrame(f1);
    liberaFrame(f2);
}

static void test_desempilha_retorna_frame_correto(void) {
    u1 code[] = {0xB1};
    Code_attribute ca = make_code(code, 1);
    Frame *f1 = criaFrame(&ca);
    Frame *f2 = criaFrame(&ca);

    JVMStack *s = criaJVMStack();
    empilhaFrame(s, f1);
    empilhaFrame(s, f2);

    Frame *popped = desempilhaFrame(s);
    assert(popped == f2);
    assert(s->topo == 0);
    assert(frameAtual(s) == f1);

    free(s);
    liberaFrame(f1);
    liberaFrame(f2);
}

static void test_libera_jvm_stack(void) {
    u1 code[] = {0xB1};
    Code_attribute ca = make_code(code, 1);
    Frame *f1 = criaFrame(&ca);
    Frame *f2 = criaFrame(&ca);

    JVMStack *s = criaJVMStack();
    empilhaFrame(s, f1);
    empilhaFrame(s, f2);
    liberaJVMStack(s); /* nao deve crashar */
}

static void test_desempilha_ate_vazia(void) {
    u1 code[] = {0xB1};
    Code_attribute ca = make_code(code, 1);

    JVMStack *s = criaJVMStack();
    for (int i = 0; i < 5; i++) {
        Frame *f = criaFrame(&ca);
        empilhaFrame(s, f);
    }
    assert(s->topo == 4);

    liberaJVMStack(s);
}


int main(void) {
    printf("=== jvm_stack unit tests ===\n");

    printf("\n-- criaJVMStack --\n");
    RUN_TEST(test_cria_stack_vazia);
    RUN_TEST(test_frame_atual_vazia_retorna_null);

    printf("\n-- empilhaFrame / frameAtual --\n");
    RUN_TEST(test_empilha_um_frame);
    RUN_TEST(test_empilha_dois_frames);

    printf("\n-- desempilhaFrame --\n");
    RUN_TEST(test_desempilha_retorna_frame_correto);
    RUN_TEST(test_desempilha_ate_vazia);

    printf("\n-- liberaJVMStack --\n");
    RUN_TEST(test_libera_jvm_stack);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
