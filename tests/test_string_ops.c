#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "interpreter/string_ops.h"
#include "interpreter/basic_ops.h"
#include "lib/types/constant_pool.h"
#include "lib/types/consts.h"

#define RUN_TEST(fn) do { \
    fn(); \
    printf("  PASS: " #fn "\n"); \
} while (0)

/* ── helpers ─────────────────────────────────────────────── */

static Frame *frame_vazio(void) {
    Frame *f = calloc(1, sizeof(Frame));
    f->topo = -1;
    return f;
}

/* ── ldc com CONSTANT_Integer ────────────────────────────── */

static void test_ldc_integer(void) {
    cp_info cp[2];
    memset(cp, 0, sizeof(cp));

    CONSTANT_Integer_info *ii = malloc(sizeof(CONSTANT_Integer_info));
    ii->tag   = CONSTANT_Integer;
    ii->bytes = 42;
    cp[1].tag          = CONSTANT_Integer;
    cp[1].integer_info = ii;

    Frame *f = frame_vazio();
    execLdc(f, cp, 1);

    assert(f->topo == 0);
    assert(f->pilha[0].tipo    == TIPO_INT);
    assert(f->pilha[0].inteiro == 42);

    free(ii);
    free(f);
}

static void test_ldc_integer_negativo(void) {
    cp_info cp[2];
    memset(cp, 0, sizeof(cp));

    CONSTANT_Integer_info *ii = malloc(sizeof(CONSTANT_Integer_info));
    ii->tag   = CONSTANT_Integer;
    ii->bytes = (uint32_t)(-7);
    cp[1].tag          = CONSTANT_Integer;
    cp[1].integer_info = ii;

    Frame *f = frame_vazio();
    execLdc(f, cp, 1);

    assert(f->topo == 0);
    assert(f->pilha[0].inteiro == -7);

    free(ii);
    free(f);
}

/* ── ldc com CONSTANT_Float ──────────────────────────────── */

static void test_ldc_float(void) {
    cp_info cp[2];
    memset(cp, 0, sizeof(cp));

    float    expected = 3.14f;
    uint32_t bits;
    memcpy(&bits, &expected, sizeof(float));

    CONSTANT_Float_info *fi = malloc(sizeof(CONSTANT_Float_info));
    fi->tag   = CONSTANT_Float;
    fi->bytes = bits;
    cp[1].tag        = CONSTANT_Float;
    cp[1].float_info = fi;

    Frame *f = frame_vazio();
    execLdc(f, cp, 1);

    assert(f->topo == 0);
    assert(f->pilha[0].tipo == TIPO_FLOAT);

    uint32_t got_bits;
    memcpy(&got_bits, &f->pilha[0].flutuante, sizeof(float));
    assert(got_bits == bits);

    free(fi);
    free(f);
}

/* ── ldc com CONSTANT_String ─────────────────────────────── */

static void test_ldc_string(void) {
    /* cp[1] = Utf8 "ola mundo"
     * cp[2] = String { string_index: 1 } */
    cp_info cp[3];
    memset(cp, 0, sizeof(cp));

    const char *texto = "ola mundo";
    CONSTANT_Utf8_info *utf8 = malloc(sizeof(CONSTANT_Utf8_info));
    utf8->tag    = CONSTANT_Utf8;
    utf8->length = (u2)strlen(texto);
    utf8->bytes  = (u1 *)texto;   /* ponteiro estático p/ literal: OK em teste */
    cp[1].tag       = CONSTANT_Utf8;
    cp[1].utf8_info = utf8;

    CONSTANT_String_info *si = malloc(sizeof(CONSTANT_String_info));
    si->tag          = CONSTANT_String;
    si->string_index = 1;
    cp[2].tag         = CONSTANT_String;
    cp[2].string_info = si;

    Frame *f = frame_vazio();
    execLdc(f, cp, 2);

    assert(f->topo == 0);
    assert(f->pilha[0].tipo == TIPO_REF);
    char *str = (char *)(uintptr_t)f->pilha[0].longo;
    assert(strncmp(str, texto, strlen(texto)) == 0);

    free(utf8);
    free(si);
    free(f);
}

/* ── múltiplos ldc acumulam na pilha ─────────────────────── */

static void test_ldc_acumula_pilha(void) {
    cp_info cp[4];
    memset(cp, 0, sizeof(cp));

    for (int i = 1; i <= 3; i++) {
        CONSTANT_Integer_info *ii = malloc(sizeof(CONSTANT_Integer_info));
        ii->tag   = CONSTANT_Integer;
        ii->bytes = (uint32_t)(i * 10);
        cp[i].tag          = CONSTANT_Integer;
        cp[i].integer_info = ii;
    }

    Frame *f = frame_vazio();
    execLdc(f, cp, 1);
    execLdc(f, cp, 2);
    execLdc(f, cp, 3);

    assert(f->topo == 2);
    assert(f->pilha[0].inteiro == 10);
    assert(f->pilha[1].inteiro == 20);
    assert(f->pilha[2].inteiro == 30);

    for (int i = 1; i <= 3; i++) free(cp[i].integer_info);
    free(f);
}

/* ── main ────────────────────────────────────────────────── */

int main(void) {
    printf("=== test_string_ops ===\n");
    RUN_TEST(test_ldc_integer);
    RUN_TEST(test_ldc_integer_negativo);
    RUN_TEST(test_ldc_float);
    RUN_TEST(test_ldc_string);
    RUN_TEST(test_ldc_acumula_pilha);
    printf("=== Todos os testes passaram ===\n");
    return 0;
}
