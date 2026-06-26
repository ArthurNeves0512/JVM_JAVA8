#include "basic_ops.h"
#include "heap.h"
#include "method_invoke.h"
#include "native_methods.h"
#include "attribute.h"
#include "consts.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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


/* ── pool de arrays (liberado ao final de cada execucao) ───────────── */

#define MAX_ARRAYS 1024

static JVMArray *jvm_arrays[MAX_ARRAYS];
static int       jvm_array_count = 0;

static JVMArray *aloca_array(int32_t length, int atype) {
    size_t elem_size;
    switch (atype) {
        case T_BOOLEAN: case T_BYTE:           elem_size = sizeof(int8_t);   break;
        case T_CHAR:    case T_SHORT:          elem_size = sizeof(int16_t);  break;
        case T_FLOAT:   case T_INT:            elem_size = sizeof(int32_t);  break;
        case T_DOUBLE:  case T_LONG:           elem_size = sizeof(int64_t);  break;
        default:                               elem_size = sizeof(int32_t);  break;
    }
    JVMArray *arr = malloc(sizeof(JVMArray));
    if (!arr) { fprintf(stderr, "aloca_array: sem memoria\n"); exit(1); }
    arr->length = length;
    arr->atype  = atype;
    arr->data   = calloc((size_t)(length > 0 ? length : 1), elem_size);
    if (jvm_array_count < MAX_ARRAYS)
        jvm_arrays[jvm_array_count++] = arr;
    return arr;
}

void liberaArrays(void) {
    for (int i = 0; i < jvm_array_count; i++) {
        free(jvm_arrays[i]->data);
        free(jvm_arrays[i]);
    }
    jvm_array_count = 0;
}

#define ARR(slot)  ((JVMArray *)(uintptr_t)(slot).longo)


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


/* ── helpers para ldc ──────────────────────────────────────────────── */

static void ldc_push(Frame *f, cp_info *cp, u2 idx) {
    u1 tag = cp[idx].tag;
    if (tag == CONSTANT_Integer) {
        PUSH_INT(f, (int32_t)cp[idx].integer_info->bytes);
    } else if (tag == CONSTANT_Float) {
        float val;
        uint32_t bits = cp[idx].float_info->bytes;
        memcpy(&val, &bits, sizeof(float));
        f->pilha[++f->topo].flutuante = val;
        f->pilha[f->topo].tipo        = TIPO_FLOAT;
    } else if (tag == CONSTANT_String) {
        u2 str_idx = cp[idx].string_info->string_index;
        char *bytes = (char *)cp[str_idx].utf8_info->bytes;
        f->pilha[++f->topo].longo = (int64_t)(uintptr_t)bytes;
        f->pilha[f->topo].tipo    = TIPO_REF;
    } else {
        fprintf(stderr, "ldc: tag %u nao suportado no indice %u\n", tag, idx);
        PUSH_INT(f, 0);
    }
}

static void ldc2_push(Frame *f, cp_info *cp, u2 idx) {
    u1 tag = cp[idx].tag;
    if (tag == CONSTANT_Long) {
        uint64_t hi  = cp[idx].long_info->high_bytes;
        uint64_t lo  = cp[idx].long_info->low_bytes;
        f->pilha[++f->topo].longo = (int64_t)((hi << 32) | lo);
        f->pilha[f->topo].tipo    = TIPO_LONG;
    } else if (tag == CONSTANT_Double) {
        uint64_t hi   = cp[idx].double_info->high_bytes;
        uint64_t lo   = cp[idx].double_info->low_bytes;
        uint64_t bits = (hi << 32) | lo;
        double val;
        memcpy(&val, &bits, sizeof(double));
        f->pilha[++f->topo].duplo = val;
        f->pilha[f->topo].tipo    = TIPO_DOUBLE;
    } else {
        fprintf(stderr, "ldc2_w: tag %u nao suportado no indice %u\n", tag, idx);
        PUSH_INT(f, 0);
    }
}


/* ── intercepção de System.out.println ─────────────────────────────── */

static int resolve_class_name(cp_info *cp, u2 class_index, const char *expected) {
    char *name = getUtf8(cp, cp[class_index].constant_class_info->name_index);
    int match  = name && strcmp(name, expected) == 0;
    free(name);
    return match;
}

static int resolve_member_name(cp_info *cp, u2 nat_index, const char *expected) {
    char *name = getUtf8(cp, cp[nat_index].nameAndType_info->name_index);
    int match  = name && strcmp(name, expected) == 0;
    free(name);
    return match;
}


void executaFrame(Frame *f, ClassFile *cf) {
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

        /* ── aload / astore (referências) ─────────────── */
        case 0x2A: f->pilha[++f->topo] = f->locais[0]; break; /* aload_0 */
        case 0x2B: f->pilha[++f->topo] = f->locais[1]; break; /* aload_1 */
        case 0x2C: f->pilha[++f->topo] = f->locais[2]; break; /* aload_2 */
        case 0x2D: f->pilha[++f->topo] = f->locais[3]; break; /* aload_3 */

        case 0x4B: f->locais[0] = f->pilha[f->topo--]; break; /* astore_0 */
        case 0x4C: f->locais[1] = f->pilha[f->topo--]; break; /* astore_1 */
        case 0x4D: f->locais[2] = f->pilha[f->topo--]; break; /* astore_2 */
        case 0x4E: f->locais[3] = f->pilha[f->topo--]; break; /* astore_3 */

        /* ── arrays de primitivos ─────────────────────── */
        case 0x2E: {                         /* iaload  */
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, ((int32_t *)arr->data)[idx]);
            break;
        }

        case 0x33: {                         /* baload (byte/boolean) */
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, (int32_t)((int8_t *)arr->data)[idx]);
            break;
        }

        case 0x34: {                         /* caload (char) */
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, (int32_t)((uint16_t *)arr->data)[idx]);
            break;
        }

        case 0x4F: {                         /* iastore */
            int32_t   val = POP_INT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((int32_t *)arr->data)[idx] = val;
            break;
        }

        case 0x54: {                         /* bastore (byte/boolean) */
            int32_t   val = POP_INT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((int8_t *)arr->data)[idx] = (int8_t)val;
            break;
        }

        case 0x55: {                         /* castore (char) */
            int32_t   val = POP_INT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((uint16_t *)arr->data)[idx] = (uint16_t)val;
            break;
        }

        case 0xBC: {                         /* newarray */
            u1        atype  = READ_U1(f);
            int32_t   length = POP_INT(f);
            JVMArray *arr    = aloca_array(length, (int)atype);
            f->pilha[++f->topo].longo = (int64_t)(uintptr_t)arr;
            f->pilha[f->topo].tipo    = TIPO_ARRAY;
            break;
        }

        case 0xBE: {                         /* arraylength */
            const JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, arr->length);
            break;
        }

        /* ── ldc / ldc_w / ldc2_w ─────────────────────── */
        case 0x12: {                         /* ldc      */
            u1 idx = READ_U1(f);
            if (cf) ldc_push(f, cf->constant_pool, (u2)idx);
            else    PUSH_INT(f, 0);
            break;
        }

        case 0x13: {                         /* ldc_w    */
            u2 idx = read_u2(f);
            if (cf) ldc_push(f, cf->constant_pool, idx);
            else    PUSH_INT(f, 0);
            break;
        }

        case 0x14: {                         /* ldc2_w   */
            u2 idx = read_u2(f);
            if (cf) ldc2_push(f, cf->constant_pool, idx);
            else    PUSH_INT(f, 0);
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

        /* ── manipulação de pilha ─────────────────────── */
        case 0x57:                           /* pop */
            f->topo--;
            break;

        case 0x59: {                         /* dup */
            f->pilha[f->topo + 1] = f->pilha[f->topo];
            f->topo++;
            break;
        }

                /* ── objetos ───────────────────────────────────── */
        case 0xBB: {                         /* new */
            u2 idx = read_u2(f);

            if (!cf) {
                PUSH_INT(f, 0);
                break;
            }

            /* nome da classe a ser instanciada */
            char *cname = getUtf8(
                cf->constant_pool,
                cf->constant_pool[idx].constant_class_info->name_index
            );

            /* carrega a classe real do objeto */
            ClassFile *obj_cf = NULL;

            if (cname && !isNativeClass(cname)) {
                char filename[256];
                snprintf(filename, sizeof(filename), "%s.class", cname);
                obj_cf = loadClassFile(filename);
            }

            /* cria o objeto */
            HeapObject *obj = alocaObjeto(obj_cf, cname);

            /* empilha referência */
            f->topo++;
            f->pilha[f->topo].tipo = TIPO_OBJECT;
            f->pilha[f->topo].ref  = obj;

            free(cname);

            break;
        }

        case 0xB4: {                         /* getfield */
            u2 idx = read_u2(f);

            if (!cf) {
                PUSH_INT(f, 0);
                break;
            }

            HeapObject *obj =
                (HeapObject *)f->pilha[f->topo--].ref;

            const CONSTANT_Fieldref_info *fref =
                cf->constant_pool[idx].fieldRef_info;

            const CONSTANT_NameAndType_info *nat =
                cf->constant_pool[fref->name_and_type_index]
                    .nameAndType_info;

            char *fname =
                getUtf8(cf->constant_pool,
                        nat->name_index);

            int fi =
                buscaCampo(obj, fname);

            free(fname);

            if (fi >= 0)
                f->pilha[++f->topo] =
                    obj->fields[fi];
            else
                PUSH_INT(f, 0);

            break;
        }

        case 0xB5: {                         /* putfield */
            u2 idx = read_u2(f);

            if (!cf) {
                f->topo -= 2;
                break;
            }

            Slot val =
                f->pilha[f->topo--];

            HeapObject *obj =
                (HeapObject *)f->pilha[f->topo--].ref;

            const CONSTANT_Fieldref_info *fref =
                cf->constant_pool[idx].fieldRef_info;

            const CONSTANT_NameAndType_info *nat =
                cf->constant_pool[fref->name_and_type_index]
                    .nameAndType_info;

            char *fname =
                getUtf8(cf->constant_pool,
                        nat->name_index);

            int fi =
                buscaCampo(obj, fname);

            free(fname);

            if (fi >= 0)
                obj->fields[fi] = val;

            break;
        }

        /* ── getstatic / invokevirtual ─────────────────── */
        case 0xB2: {                         /* getstatic */
            u2 idx = read_u2(f);
            if (!cf) { PUSH_INT(f, 0); break; }
            const CONSTANT_Fieldref_info *fref = cf->constant_pool[idx].fieldRef_info;
            int is_out = resolve_class_name(cf->constant_pool, fref->class_index,
                                            "java/lang/System") &&
                         resolve_member_name(cf->constant_pool, fref->name_and_type_index,
                                             "out");
            if (is_out) {
                f->pilha[++f->topo].longo = 0; /* marcador de PrintStream */
                f->pilha[f->topo].tipo    = TIPO_REF;
            } else {
                fprintf(stderr, "getstatic: campo nao suportado (idx=%u)\n", idx);
                PUSH_INT(f, 0);
            }
            break;
        }

        case 0xB6: {                         /* invokevirtual */
            u2 idx = read_u2(f);
            if (!cf) { return; }
            execInvokevirtual(f, cf, idx);
            break;
        }

        case 0xB7: {                         /* invokespecial */
            u2 idx = read_u2(f);
            if (!cf) { return; }
            execInvokespecial(f, cf, idx);
            break;
        }

        case 0xB8: {                         /* invokestatic */
            u2 idx = read_u2(f);
            if (!cf) { return; }
            execInvokestatic(f, cf, idx);
            break;
        }

        /* ── desvios unários (compara com 0) ──────────────────── */
#define BRANCH_IF1(cond) \
    { int16_t o = read_i2(f); int32_t v = POP_INT(f); \
      if (cond) { f->pc = instr_pc + (u4)(int32_t)o; } break; }

        case 0x99: BRANCH_IF1(v == 0)   /* ifeq */
        case 0x9A: BRANCH_IF1(v != 0)   /* ifne */
        case 0x9B: BRANCH_IF1(v <  0)   /* iflt */
        case 0x9C: BRANCH_IF1(v >= 0)   /* ifge */
        case 0x9D: BRANCH_IF1(v >  0)   /* ifgt */
        case 0x9E: BRANCH_IF1(v <= 0)   /* ifle */
#undef BRANCH_IF1

        /* ── desvios binários (compara dois ints) ──────────────── */
#define BRANCH_IF2(cond) \
    { int16_t o = read_i2(f); int32_t v2 = POP_INT(f); int32_t v1 = POP_INT(f); \
      if (cond) { f->pc = instr_pc + (u4)(int32_t)o; } break; }

        case 0x9F: BRANCH_IF2(v1 == v2) /* if_icmpeq */
        case 0xA0: BRANCH_IF2(v1 != v2) /* if_icmpne */
        case 0xA1: BRANCH_IF2(v1 <  v2) /* if_icmplt */
        case 0xA2: BRANCH_IF2(v1 >= v2) /* if_icmpge */
        case 0xA3: BRANCH_IF2(v1 >  v2) /* if_icmpgt */
        case 0xA4: BRANCH_IF2(v1 <= v2) /* if_icmple */
#undef BRANCH_IF2

        case 0xA7: {                         /* goto */
            int16_t offset = read_i2(f);
            f->pc = instr_pc + (u4)(int32_t)offset;
            break;
        }

        /* ── retorno ───────────────────────────────────── */
        case 0xAC: /* ireturn — int    */
        case 0xAD: /* lreturn — long   */
        case 0xAE: /* freturn — float  */
        case 0xAF: /* dreturn — double */
        case 0xB0: /* areturn — ref    */
        case 0xB1: /* return  — void   */
            return;

        default:
            fprintf(stderr, "executaFrame: opcode 0x%02X nao implementado (pc=%u)\n",
                    op, instr_pc);
            return;
        }
    }
}
