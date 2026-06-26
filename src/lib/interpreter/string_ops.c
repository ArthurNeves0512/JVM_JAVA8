#include "string_ops.h"
#include "consts.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * Representação de String nesta JVM:
 * Um String literal fica como ponteiro (char*) direto para os bytes UTF-8
 * do constant pool, armazenado no campo `longo` do Slot com tipo TIPO_REF.
 * Não há cópia — o CP vive enquanto a execução do método durar.
 */
void execLdc(Frame *f, cp_info *cp, u2 idx) {
    u1 tag = cp[idx].tag;

    switch (tag) {

    case CONSTANT_Integer: {
        int32_t val = (int32_t)cp[idx].integer_info->bytes;
        f->pilha[++f->topo].inteiro = val;
        f->pilha[f->topo].tipo      = TIPO_INT;
        break;
    }

    case CONSTANT_Float: {
        float    val;
        uint32_t bits = cp[idx].float_info->bytes;
        memcpy(&val, &bits, sizeof(float));
        f->pilha[++f->topo].flutuante = val;
        f->pilha[f->topo].tipo        = TIPO_FLOAT;
        break;
    }

    case CONSTANT_String: {
        u2    str_idx = cp[idx].string_info->string_index;
        /* bytes do CP não são null-terminated; usamos o ponteiro direto */
        char *bytes   = (char *)cp[str_idx].utf8_info->bytes;
        f->pilha[++f->topo].longo = (int64_t)(uintptr_t)bytes;
        f->pilha[f->topo].tipo    = TIPO_REF;
        break;
    }

    default:
        fprintf(stderr,
                "ldc: tag %u no indice %u nao suportado\n", tag, idx);
        f->pilha[++f->topo].inteiro = 0;
        f->pilha[f->topo].tipo      = TIPO_INT;
        break;
    }
}
