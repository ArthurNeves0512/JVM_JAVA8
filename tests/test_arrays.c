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


/* ── testes de newarray + arraylength ────────────────────────────── */

static void test_newarray_length(void) {
    /*
     * bipush 5   (0x10 0x05)
     * newarray   (0xBC 0x0A)   T_INT = 10
     * arraylength(0xBE)
     * ireturn    (0xAC)
     */
    u1 bytes[] = {0x10, 5, 0xBC, T_INT, 0xBE, 0xAC};
    Code_attribute ca = make_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 5);
    liberaFrame(f);
    liberaArrays();
}

static void test_newarray_length_zero(void) {
    u1 bytes[] = {0x03, 0xBC, T_INT, 0xBE, 0xAC};  /* iconst_0, newarray, arraylength */
    Code_attribute ca = make_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 0);
    liberaFrame(f);
    liberaArrays();
}


/* ── iastore / iaload ─────────────────────────────────────────────── */

static void test_iastore_iaload(void) {
    /*
     * bipush 3    (0x10 0x03)      ; size = 3
     * newarray    (0xBC 0x0A)      ; int[3]
     * astore_0    (0x4B)
     * aload_0     (0x2A)
     * iconst_1    (0x04)           ; index = 1
     * bipush 99   (0x10 0x63)      ; value = 99
     * iastore     (0x4F)
     * aload_0     (0x2A)
     * iconst_1    (0x04)
     * iaload      (0x2E)
     * ireturn     (0xAC)
     */
    u1 bytes[] = {
        0x10, 3, 0xBC, T_INT,
        0x4B,
        0x2A, 0x04, 0x10, 99, 0x4F,
        0x2A, 0x04, 0x2E,
        0xAC
    };
    Code_attribute ca = make_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 99);
    liberaFrame(f);
    liberaArrays();
}

static void test_iastore_multiplos_indices(void) {
    /*
     * Cria int[3], armazena [10, 20, 30], carrega [2] → deve ser 30
     */
    u1 bytes[] = {
        /* newarray int[3] → astore_0 */
        0x10, 3, 0xBC, T_INT, 0x4B,
        /* array[0] = 10 */
        0x2A, 0x03, 0x10, 10, 0x4F,
        /* array[1] = 20 */
        0x2A, 0x04, 0x10, 20, 0x4F,
        /* array[2] = 30 */
        0x2A, 0x05, 0x10, 30, 0x4F,
        /* iaload array[2] */
        0x2A, 0x05, 0x2E,
        0xAC
    };
    Code_attribute ca = make_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 30);
    liberaFrame(f);
    liberaArrays();
}

static void test_iastore_negativo(void) {
    u1 bytes[] = {
        0x10, 2, 0xBC, T_INT, 0x4B,
        0x2A, 0x03, 0x02, 0x4F,  /* array[0] = iconst_m1 = -1 */
        0x2A, 0x03, 0x2E,         /* iaload array[0] */
        0xAC
    };
    Code_attribute ca = make_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == -1);
    liberaFrame(f);
    liberaArrays();
}


/* ── bastore / baload (byte) ──────────────────────────────────────── */

static void test_bastore_baload(void) {
    /*
     * byte[4], armazena -5 no índice 0, carrega e verifica
     */
    u1 bytes[] = {
        0x07, 0xBC, T_BYTE, 0x4B,          /* bipush... wait iconst_4=0x07 → size=4 */
        0x2A, 0x03, 0x10, (u1)(-5), 0x54,  /* array[0] = -5 via bastore */
        0x2A, 0x03, 0x33,                   /* baload array[0] */
        0xAC
    };
    Code_attribute ca = make_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == -5);
    liberaFrame(f);
    liberaArrays();
}


/* ── castore / caload (char) ──────────────────────────────────────── */

static void test_castore_caload(void) {
    /*
     * char[3], armazena 'A' (65) no índice 0, carrega
     */
    u1 bytes[] = {
        0x06, 0xBC, T_CHAR, 0x4B,           /* iconst_3 → char[3] */
        0x2A, 0x03, 0x10, 65, 0x55,         /* array[0] = 65 via castore */
        0x2A, 0x03, 0x34,                    /* caload array[0] */
        0xAC
    };
    Code_attribute ca = make_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 65);
    liberaFrame(f);
    liberaArrays();
}


/* ── aload / astore com múltiplos arrays ─────────────────────────── */

static void test_dois_arrays_independentes(void) {
    /*
     * a = int[2], b = int[2]
     * a[0] = 7, b[0] = 13
     * load a[0] → deve ser 7
     */
    u1 bytes[] = {
        /* a = int[2] → locais[0] */
        0x05, 0xBC, T_INT, 0x4B,
        /* b = int[2] → locais[1] */
        0x05, 0xBC, T_INT, 0x4C,
        /* a[0] = 7 */
        0x2A, 0x03, 0x10, 7, 0x4F,
        /* b[0] = 13 */
        0x2B, 0x03, 0x10, 13, 0x4F,
        /* iaload a[0] */
        0x2A, 0x03, 0x2E,
        0xAC
    };
    Code_attribute ca = make_code(bytes, sizeof(bytes));
    Frame *f = criaFrame(&ca);
    executaFrame(f, NULL);
    assert(f->pilha[f->topo].inteiro == 7);
    liberaFrame(f);
    liberaArrays();
}


int main(void) {
    printf("=== arrays unit tests ===\n");

    printf("\n-- newarray + arraylength --\n");
    RUN_TEST(test_newarray_length);
    RUN_TEST(test_newarray_length_zero);

    printf("\n-- iastore / iaload --\n");
    RUN_TEST(test_iastore_iaload);
    RUN_TEST(test_iastore_multiplos_indices);
    RUN_TEST(test_iastore_negativo);

    printf("\n-- bastore / baload --\n");
    RUN_TEST(test_bastore_baload);

    printf("\n-- castore / caload --\n");
    RUN_TEST(test_castore_caload);

    printf("\n-- multiplos arrays --\n");
    RUN_TEST(test_dois_arrays_independentes);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
