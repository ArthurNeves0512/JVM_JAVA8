#include "basic_ops.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define PUSH_INT(f, val) do { \
    (f)->pilha[++(f)->topo].inteiro = (val); \
    (f)->pilha[(f)->topo].tipo      = TIPO_INT; \
} while (0)

#define POP_INT(f)  ((f)->pilha[(f)->topo--].inteiro)

#define READ_U1(f)  ((f)->codigo[(f)->pc++])

static inline u2 read_u2(Frame *f) {
    u2 hi = READ_U1(f);
    u2 lo = READ_U1(f);
    return (u2)((hi << 8) | lo);
}

static inline int16_t read_i2(Frame *f) {
    return (int16_t)read_u2(f);
}


Frame *criaFrame(Code_attribute *ca) {
    Frame *f = malloc(sizeof(Frame));
    if (!f) {
        fprintf(stderr, "criaFrame: out of memory\n");
        exit(1);
    }
    f->topo     = -1;
    f->pc       = 0;
    f->codigo   = ca->code;
    f->tamanho  = ca->code_length;
    return f;
}


void liberaFrame(Frame *f) {
    free(f);
}


void executaFrame(Frame *f, ClassFile *cf) {
    (void)cf;

    while (f->pc < f->tamanho) {
        u4  instr_pc = f->pc;
        u1  op       = READ_U1(f);

        switch (op) {

        /* ── constantes inteiras ───────────────────────── */
        case 0x02: PUSH_INT(f, -1); break;  /* iconst_m1 */
        case 0x03: PUSH_INT(f,  0); break;  /* iconst_0  */
        case 0x04: PUSH_INT(f,  1); break;  /* iconst_1  */
        case 0x05: PUSH_INT(f,  2); break;  /* iconst_2  */
        case 0x06: PUSH_INT(f,  3); break;  /* iconst_3  */
        case 0x07: PUSH_INT(f,  4); break;  /* iconst_4  */
        case 0x08: PUSH_INT(f,  5); break;  /* iconst_5  */

        case 0x10: {                         /* bipush    */
            int8_t val = (int8_t)READ_U1(f);
            PUSH_INT(f, (int32_t)val);
            break;
        }

        case 0x11: {                         /* sipush    */
            int16_t val = read_i2(f);
            PUSH_INT(f, (int32_t)val);
            break;
        }

        /* ── aritmética inteira ────────────────────────── */
        case 0x60: {                         /* iadd */
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, v1 + v2);
            break;
        }

        case 0x64: {                         /* isub */
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, v1 - v2);
            break;
        }

        case 0x68: {                         /* imul */
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, v1 * v2);
            break;
        }

        case 0x6C: {                         /* idiv */
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, v1 / v2);
            break;
        }

        case 0x70: {                         /* irem */
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, v1 % v2);
            break;
        }

        case 0x74: {                         /* ineg */
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, -v1);
            break;
        }

        /* ── variáveis locais ──────────────────────────── */
        case 0x1A: PUSH_INT(f, f->locais[0].inteiro); break;  /* iload_0 */
        case 0x1B: PUSH_INT(f, f->locais[1].inteiro); break;  /* iload_1 */
        case 0x1C: PUSH_INT(f, f->locais[2].inteiro); break;  /* iload_2 */
        case 0x1D: PUSH_INT(f, f->locais[3].inteiro); break;  /* iload_3 */

        case 0x3B: {                         /* istore_0 */
            f->locais[0].inteiro = POP_INT(f);
            f->locais[0].tipo    = TIPO_INT;
            break;
        }
        case 0x3C: {                         /* istore_1 */
            f->locais[1].inteiro = POP_INT(f);
            f->locais[1].tipo    = TIPO_INT;
            break;
        }
        case 0x3D: {                         /* istore_2 */
            f->locais[2].inteiro = POP_INT(f);
            f->locais[2].tipo    = TIPO_INT;
            break;
        }
        case 0x3E: {                         /* istore_3 */
            f->locais[3].inteiro = POP_INT(f);
            f->locais[3].tipo    = TIPO_INT;
            break;
        }

        case 0x84: {                         /* iinc */
            u1      index = READ_U1(f);
            int8_t  cst   = (int8_t)READ_U1(f);
            f->locais[index].inteiro += (int32_t)cst;
            break;
        }

        /* ── desvios ───────────────────────────────────── */
        case 0x99: {                         /* ifeq */
            int16_t offset = read_i2(f);
            int32_t v1     = POP_INT(f);
            if (v1 == 0) f->pc = instr_pc + (u4)(int32_t)offset;
            break;
        }

        case 0x9F: {                         /* if_icmpeq */
            int16_t offset = read_i2(f);
            int32_t v2     = POP_INT(f);
            int32_t v1     = POP_INT(f);
            if (v1 == v2) f->pc = instr_pc + (u4)(int32_t)offset;
            break;
        }

        case 0xA7: {                         /* goto */
            int16_t offset = read_i2(f);
            f->pc = instr_pc + (u4)(int32_t)offset;
            break;
        }

        /* ── retorno ───────────────────────────────────── */
        case 0xAC: /* ireturn — mantém valor no topo, encerra o frame */
        case 0xB1: /* return  — encerra sem valor de retorno          */
            return;

        default:
            fprintf(stderr, "executaFrame: opcode 0x%02X nao implementado (pc=%u)\n",
                    op, instr_pc);
            return;
        }
    }
}
