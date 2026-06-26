/* STUB – Pessoa1: criaFrame/liberaFrame.
 * STUB – Pessoa2: executaFrame (loop principal + demais opcodes).
 * Os casos marcados "USER" são de responsabilidade desta branch. */
#include "basic_ops.h"
#include "method_invoke.h"
#include "string_ops.h"
#include <stdio.h>
#include <stdlib.h>

#define PUSH_INT(f, val) do { \
    (f)->pilha[++(f)->topo].inteiro = (val); \
    (f)->pilha[(f)->topo].tipo      = TIPO_INT; \
} while (0)

#define POP_INT(f) ((f)->pilha[(f)->topo--].inteiro)

static inline u2 read_u2(Frame *f) {
    u2 hi = f->codigo[f->pc++];
    u2 lo = f->codigo[f->pc++];
    return (u2)((hi << 8) | lo);
}

/* ── STUB – Pessoa1: alocação de frame ──────────────────────── */

Frame *criaFrame(Code_attribute *ca) {
    Frame *f = calloc(1, sizeof(Frame));
    if (!f) { fprintf(stderr, "criaFrame: sem memoria\n"); exit(1); }
    f->topo    = -1;
    f->pc      = 0;
    f->codigo  = ca->code;
    f->tamanho = ca->code_length;
    return f;
}

void liberaFrame(Frame *f) {
    free(f);
}

/* ── STUB – Pessoa2: loop de execução ────────────────────────── */

void executaFrame(Frame *f, ClassFile *cf) {
    while (f->pc < f->tamanho) {
        u4 instr_pc = f->pc;
        u1 op = f->codigo[f->pc++];

        switch (op) {

        /* ── constantes inteiras (STUB – Pessoa2) ─────────── */
        case 0x02: PUSH_INT(f, -1); break;  /* iconst_m1 */
        case 0x03: PUSH_INT(f,  0); break;  /* iconst_0  */
        case 0x04: PUSH_INT(f,  1); break;  /* iconst_1  */
        case 0x05: PUSH_INT(f,  2); break;  /* iconst_2  */
        case 0x06: PUSH_INT(f,  3); break;  /* iconst_3  */
        case 0x07: PUSH_INT(f,  4); break;  /* iconst_4  */
        case 0x08: PUSH_INT(f,  5); break;  /* iconst_5  */
        case 0x10: {                          /* bipush */
            int8_t v = (int8_t)f->codigo[f->pc++];
            PUSH_INT(f, (int32_t)v);
            break;
        }
        case 0x11: {                          /* sipush */
            int16_t v = (int16_t)read_u2(f);
            PUSH_INT(f, (int32_t)v);
            break;
        }

        /* ── iload / istore (STUB – Pessoa3) ──────────────── */
        case 0x1A: PUSH_INT(f, f->locais[0].inteiro); break; /* iload_0 */
        case 0x1B: PUSH_INT(f, f->locais[1].inteiro); break; /* iload_1 */
        case 0x1C: PUSH_INT(f, f->locais[2].inteiro); break; /* iload_2 */
        case 0x1D: PUSH_INT(f, f->locais[3].inteiro); break; /* iload_3 */
        case 0x3B: f->locais[0].inteiro = POP_INT(f); f->locais[0].tipo = TIPO_INT; break; /* istore_0 */
        case 0x3C: f->locais[1].inteiro = POP_INT(f); f->locais[1].tipo = TIPO_INT; break; /* istore_1 */
        case 0x3D: f->locais[2].inteiro = POP_INT(f); f->locais[2].tipo = TIPO_INT; break; /* istore_2 */
        case 0x3E: f->locais[3].inteiro = POP_INT(f); f->locais[3].tipo = TIPO_INT; break; /* istore_3 */

        /* ── aload (STUB – Pessoa2) ────────────────────────── */
        case 0x2A: f->pilha[++f->topo] = f->locais[0]; break; /* aload_0 */
        case 0x2B: f->pilha[++f->topo] = f->locais[1]; break; /* aload_1 */
        case 0x2C: f->pilha[++f->topo] = f->locais[2]; break; /* aload_2 */
        case 0x2D: f->pilha[++f->topo] = f->locais[3]; break; /* aload_3 */

        /* ── getstatic (STUB – Pessoa2: System.out) ─────────── */
        case 0xB2: {
            read_u2(f);
            f->pilha[++f->topo].longo = 0;
            f->pilha[f->topo].tipo    = TIPO_REF;
            break;
        }

        /* ── pop / dup (STUB – Pessoa2) ───────────────────── */
        case 0x57: f->topo--; break;
        case 0x59: f->pilha[f->topo + 1] = f->pilha[f->topo]; f->topo++; break;

        /* ── ldc (USER) ────────────────────────────────────── */
        case 0x12: {  /* ldc: índice de 1 byte */
            u1 idx = f->codigo[f->pc++];
            if (cf) execLdc(f, cf->constant_pool, (u2)idx);
            else    PUSH_INT(f, 0);
            break;
        }
        case 0x13: {  /* ldc_w: índice de 2 bytes */
            u2 idx = read_u2(f);
            if (cf) execLdc(f, cf->constant_pool, idx);
            else    PUSH_INT(f, 0);
            break;
        }

        /* ── retornos (USER) ───────────────────────────────── */
        case 0xAC: /* ireturn – valor inteiro no topo fica para o chamador ler */
        case 0xAD: /* lreturn */
        case 0xAE: /* freturn */
        case 0xAF: /* dreturn */
        case 0xB0: /* areturn */
        case 0xB1: /* return  – void; nada é empilhado */
            return;

        /* ── invocação de métodos (USER – method_invoke.c) ─── */
        case 0xB6: {  /* invokevirtual */
            u2 idx = read_u2(f);
            if (cf) execInvokevirtual(f, cf, idx);
            break;
        }
        case 0xB7: {  /* invokespecial – STUB */
            read_u2(f);
            /* STUB – Pessoa2: invokespecial não implementado */
            break;
        }
        case 0xB8: {  /* invokestatic */
            u2 idx = read_u2(f);
            if (cf) execInvokestatic(f, cf, idx);
            break;
        }

        default:
            fprintf(stderr,
                    "executaFrame: opcode 0x%02X nao implementado (pc=%u)\n",
                    op, instr_pc);
            return;
        }
    }
}
