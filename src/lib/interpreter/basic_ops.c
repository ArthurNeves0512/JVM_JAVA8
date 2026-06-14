#include "basic_ops.h"
#include <string.h>
#include <stdint.h>

static void push_int(Frame *f, int32_t v) {
    f->topo++;
    f->pilha[f->topo].tipo    = TIPO_INT;
    f->pilha[f->topo].inteiro = v;
}

static int32_t pop_int(Frame *f) {
    return f->pilha[f->topo--].inteiro;
}

static void push_float(Frame *f, float v) {
    f->topo++;
    f->pilha[f->topo].tipo      = TIPO_FLOAT;
    f->pilha[f->topo].flutuante = v;
}

static float pop_float(Frame *f) {
    return f->pilha[f->topo--].flutuante;
}

static void push_long(Frame *f, int64_t v) {
    f->topo++;
    f->pilha[f->topo].tipo  = TIPO_LONG;
    f->pilha[f->topo].longo = v;
}

static int64_t pop_long(Frame *f) {
    return f->pilha[f->topo--].longo;
}

static void push_double(Frame *f, double v) {
    f->topo++;
    f->pilha[f->topo].tipo  = TIPO_DOUBLE;
    f->pilha[f->topo].duplo = v;
}

static double pop_double(Frame *f) {
    return f->pilha[f->topo--].duplo;
}

static int16_t read_offset(Frame *f) {
    int16_t off = (int16_t)((f->codigo[f->pc] << 8) | f->codigo[f->pc + 1]);
    f->pc += 2;
    return off;
}

Frame *criaFrame(Code_attribute *ca) {
    Frame *f = (Frame *)malloc(sizeof(Frame));
    memset(f, 0, sizeof(Frame));
    f->topo    = -1;
    f->codigo  = ca->code;
    f->tamanho = ca->code_length;
    return f;
}

void liberaFrame(Frame *f) {
    free(f);
}

void executaFrame(Frame *f, ClassFile *cf) {
    (void)cf;

    while (f->pc < f->tamanho) {
        u1 op = f->codigo[f->pc++];

        switch (op) {

        case 0x01: push_int(f, 0);    break; 
        case 0x02: push_int(f, -1);   break; 
        case 0x03: push_int(f, 0);    break; 
        case 0x04: push_int(f, 1);    break; 
        case 0x05: push_int(f, 2);    break; 
        case 0x06: push_int(f, 3);    break; 
        case 0x07: push_int(f, 4);    break; 
        case 0x08: push_int(f, 5);    break; 

        case 0x09: push_long(f, 0L);  break; 
        case 0x0A: push_long(f, 1L);  break; 

        case 0x0B: push_float(f, 0.0f); break; 
        case 0x0C: push_float(f, 1.0f); break; 
        case 0x0D: push_float(f, 2.0f); break; 

        case 0x0E: push_double(f, 0.0); break; 
        case 0x0F: push_double(f, 1.0); break; 

        case 0x10: {
            int8_t v = (int8_t)f->codigo[f->pc++];
            push_int(f, (int32_t)v);
            break;
        }
        case 0x11: { 
            int16_t v = (int16_t)((f->codigo[f->pc] << 8) | f->codigo[f->pc + 1]);
            f->pc += 2;
            push_int(f, (int32_t)v);
            break;
        }

        case 0x15: push_int(f, f->locais[f->codigo[f->pc++]].inteiro); break;
        case 0x1A: push_int(f, f->locais[0].inteiro); break;
        case 0x1B: push_int(f, f->locais[1].inteiro); break;
        case 0x1C: push_int(f, f->locais[2].inteiro); break;
        case 0x1D: push_int(f, f->locais[3].inteiro); break;

        case 0x16: push_long(f, f->locais[f->codigo[f->pc++]].longo); break;
        case 0x1E: push_long(f, f->locais[0].longo); break;
        case 0x1F: push_long(f, f->locais[1].longo); break;
        case 0x20: push_long(f, f->locais[2].longo); break;
        case 0x21: push_long(f, f->locais[3].longo); break;

        case 0x17: push_float(f, f->locais[f->codigo[f->pc++]].flutuante); break;
        case 0x22: push_float(f, f->locais[0].flutuante); break;
        case 0x23: push_float(f, f->locais[1].flutuante); break;
        case 0x24: push_float(f, f->locais[2].flutuante); break;
        case 0x25: push_float(f, f->locais[3].flutuante); break;

        case 0x18: push_double(f, f->locais[f->codigo[f->pc++]].duplo); break;
        case 0x26: push_double(f, f->locais[0].duplo); break;
        case 0x27: push_double(f, f->locais[1].duplo); break;
        case 0x28: push_double(f, f->locais[2].duplo); break;
        case 0x29: push_double(f, f->locais[3].duplo); break;

        case 0x36: f->locais[f->codigo[f->pc++]].inteiro = pop_int(f); break;
        case 0x3B: f->locais[0].inteiro = pop_int(f); break;
        case 0x3C: f->locais[1].inteiro = pop_int(f); break;
        case 0x3D: f->locais[2].inteiro = pop_int(f); break;
        case 0x3E: f->locais[3].inteiro = pop_int(f); break;

        case 0x37: f->locais[f->codigo[f->pc++]].longo = pop_long(f); break;
        case 0x3F: f->locais[0].longo = pop_long(f); break;
        case 0x40: f->locais[1].longo = pop_long(f); break;
        case 0x41: f->locais[2].longo = pop_long(f); break;
        case 0x42: f->locais[3].longo = pop_long(f); break;

        case 0x38: f->locais[f->codigo[f->pc++]].flutuante = pop_float(f); break;
        case 0x43: f->locais[0].flutuante = pop_float(f); break;
        case 0x44: f->locais[1].flutuante = pop_float(f); break;
        case 0x45: f->locais[2].flutuante = pop_float(f); break;
        case 0x46: f->locais[3].flutuante = pop_float(f); break;

        case 0x39: f->locais[f->codigo[f->pc++]].duplo = pop_double(f); break;
        case 0x47: f->locais[0].duplo = pop_double(f); break;
        case 0x48: f->locais[1].duplo = pop_double(f); break;
        case 0x49: f->locais[2].duplo = pop_double(f); break;
        case 0x4A: f->locais[3].duplo = pop_double(f); break;

        case 0x60: { int32_t b = pop_int(f); push_int(f, pop_int(f) + b); break; } /* iadd */
        case 0x64: { int32_t b = pop_int(f); push_int(f, pop_int(f) - b); break; } /* isub */
        case 0x68: { int32_t b = pop_int(f); push_int(f, pop_int(f) * b); break; } /* imul */
        case 0x6C: { 
            int32_t b = pop_int(f), a = pop_int(f);
            if (b == 0) { fprintf(stderr, "divisao por zero\n"); exit(1); }
            push_int(f, a / b);
            break;
        }
        case 0x70: { 
            int32_t b = pop_int(f), a = pop_int(f);
            if (b == 0) { fprintf(stderr, "divisao por zero\n"); exit(1); }
            push_int(f, a % b);
            break;
        }
        case 0x74: push_int(f, -pop_int(f)); break; 

        case 0x61: { int64_t b = pop_long(f); push_long(f, pop_long(f) + b); break; }
        case 0x65: { int64_t b = pop_long(f); push_long(f, pop_long(f) - b); break; }
        case 0x69: { int64_t b = pop_long(f); push_long(f, pop_long(f) * b); break; }
        case 0x6D: { int64_t b = pop_long(f), a = pop_long(f); push_long(f, a / b); break; }
        case 0x71: { int64_t b = pop_long(f), a = pop_long(f); push_long(f, a % b); break; }
        case 0x75: push_long(f, -pop_long(f)); break;

        case 0x62: { float b = pop_float(f); push_float(f, pop_float(f) + b); break; }
        case 0x66: { float b = pop_float(f); push_float(f, pop_float(f) - b); break; }
        case 0x6A: { float b = pop_float(f); push_float(f, pop_float(f) * b); break; }
        case 0x6E: { float b = pop_float(f); push_float(f, pop_float(f) / b); break; }
        case 0x76: push_float(f, -pop_float(f)); break;

        case 0x63: { double b = pop_double(f); push_double(f, pop_double(f) + b); break; }
        case 0x67: { double b = pop_double(f); push_double(f, pop_double(f) - b); break; }
        case 0x6B: { double b = pop_double(f); push_double(f, pop_double(f) * b); break; }
        case 0x6F: { double b = pop_double(f); push_double(f, pop_double(f) / b); break; }
        case 0x77: push_double(f, -pop_double(f)); break;

        case 0x78: { int32_t s = pop_int(f) & 0x1F; push_int(f, pop_int(f) << s); break; } /* ishl */
        case 0x7A: { int32_t s = pop_int(f) & 0x1F; push_int(f, pop_int(f) >> s); break; } /* ishr */
        case 0x7C: { int32_t s = pop_int(f) & 0x1F; push_int(f, (int32_t)((uint32_t)pop_int(f) >> s)); break; } /* iushr */
        case 0x7E: { int32_t b = pop_int(f); push_int(f, pop_int(f) & b); break; } /* iand */
        case 0x80: { int32_t b = pop_int(f); push_int(f, pop_int(f) | b); break; } /* ior  */
        case 0x82: { int32_t b = pop_int(f); push_int(f, pop_int(f) ^ b); break; } /* ixor */

        case 0x84: { 
            u1 idx = f->codigo[f->pc++];
            int8_t c = (int8_t)f->codigo[f->pc++];
            f->locais[idx].inteiro += (int32_t)c;
            break;
        }

        case 0x99: { int16_t off = read_offset(f); if (pop_int(f) == 0) f->pc += off - 3; break; } /* ifeq */
        case 0x9A: { int16_t off = read_offset(f); if (pop_int(f) != 0) f->pc += off - 3; break; } /* ifne */
        case 0x9B: { int16_t off = read_offset(f); if (pop_int(f) <  0) f->pc += off - 3; break; } /* iflt */
        case 0x9C: { int16_t off = read_offset(f); if (pop_int(f) >= 0) f->pc += off - 3; break; } /* ifge */
        case 0x9D: { int16_t off = read_offset(f); if (pop_int(f) >  0) f->pc += off - 3; break; } /* ifgt */
        case 0x9E: { int16_t off = read_offset(f); if (pop_int(f) <= 0) f->pc += off - 3; break; } /* ifle */

        case 0x9F: { int16_t off = read_offset(f); int32_t b = pop_int(f), a = pop_int(f); if (a == b) f->pc += off - 3; break; }
        case 0xA0: { int16_t off = read_offset(f); int32_t b = pop_int(f), a = pop_int(f); if (a != b) f->pc += off - 3; break; }
        case 0xA1: { int16_t off = read_offset(f); int32_t b = pop_int(f), a = pop_int(f); if (a <  b) f->pc += off - 3; break; }
        case 0xA2: { int16_t off = read_offset(f); int32_t b = pop_int(f), a = pop_int(f); if (a >= b) f->pc += off - 3; break; }
        case 0xA3: { int16_t off = read_offset(f); int32_t b = pop_int(f), a = pop_int(f); if (a >  b) f->pc += off - 3; break; }
        case 0xA4: { int16_t off = read_offset(f); int32_t b = pop_int(f), a = pop_int(f); if (a <= b) f->pc += off - 3; break; }

        case 0xA7: {
            int16_t off = read_offset(f);
            f->pc += off - 3;
            break;
        }

        case 0x57: f->topo--; break; /* pop */
        case 0x59: f->topo++; f->pilha[f->topo] = f->pilha[f->topo - 1]; break; /* dup */

        case 0xAC: case 0xAD: case 0xAE: case 0xAF:
        case 0xB0: case 0xB1:
            return;

        default:
            fprintf(stderr, "opcode 0x%02X nao implementado\n", op);
            return;
        }
    }
}