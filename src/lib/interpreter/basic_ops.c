#include "basic_ops.h"
#include "heap.h"
#include "method_invoke.h"
#include "native_methods.h"
#include "attribute.h"
#include "consts.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define PUSH_INT(f, val) do { \
    memset(&(f)->pilha[++(f)->topo], 0, sizeof(Slot)); \
    (f)->pilha[(f)->topo].inteiro = (val); \
    (f)->pilha[(f)->topo].tipo    = TIPO_INT; \
} while (0)

#define POP_INT(f)  ((f)->pilha[(f)->topo--].inteiro)

#define PUSH_LONG(f, val) do { \
    memset(&(f)->pilha[++(f)->topo], 0, sizeof(Slot)); \
    (f)->pilha[(f)->topo].longo = (val); \
    (f)->pilha[(f)->topo].tipo  = TIPO_LONG; \
} while (0)

#define POP_LONG(f)  ((f)->pilha[(f)->topo--].longo)

#define PUSH_FLOAT(f, val) do { \
    memset(&(f)->pilha[++(f)->topo], 0, sizeof(Slot)); \
    (f)->pilha[(f)->topo].flutuante = (val); \
    (f)->pilha[(f)->topo].tipo      = TIPO_FLOAT; \
} while (0)

#define POP_FLOAT(f)  ((f)->pilha[(f)->topo--].flutuante)

#define PUSH_DOUBLE(f, val) do { \
    memset(&(f)->pilha[++(f)->topo], 0, sizeof(Slot)); \
    (f)->pilha[(f)->topo].duplo = (val); \
    (f)->pilha[(f)->topo].tipo  = TIPO_DOUBLE; \
} while (0)

#define POP_DOUBLE(f)  ((f)->pilha[(f)->topo--].duplo)

#define READ_U1(f)  ((f)->codigo[(f)->pc++])

static inline u2 read_u2(Frame *f) {
    u2 hi = READ_U1(f);
    u2 lo = READ_U1(f);
    return (u2)((hi << 8) | lo);
}

static inline int16_t read_i2(Frame *f) {
    return (int16_t)read_u2(f);
}


/* ── tabela de campos estáticos ────────────────────────────────────── */

#define MAX_STATIC_FIELDS 256

typedef struct {
    char class_name[128];
    char field_name[128];
    Slot value;
} StaticField;

static StaticField static_fields[MAX_STATIC_FIELDS];
static int         static_field_count = 0;

static int find_static_field(const char *cls, const char *fld) {
    for (int i = 0; i < static_field_count; i++)
        if (strcmp(static_fields[i].class_name, cls) == 0 &&
            strcmp(static_fields[i].field_name, fld) == 0)
            return i;
    return -1;
}

static int alloc_static_field(const char *cls, const char *fld) {
    if (static_field_count >= MAX_STATIC_FIELDS) return -1;
    int i = static_field_count++;
    snprintf(static_fields[i].class_name, 128, "%s", cls);
    snprintf(static_fields[i].field_name, 128, "%s", fld);
    memset(&static_fields[i].value, 0, sizeof(Slot));
    return i;
}

void liberaStaticFields(void) { static_field_count = 0; }


/* ── pool de arrays (liberado ao final de cada execucao) ───────────── */

#define MAX_ARRAYS 1024

static JVMArray *jvm_arrays[MAX_ARRAYS];
static int       jvm_array_count = 0;

#define T_OBJECT 12  /* tipo extra para anewarray */

static JVMArray *aloca_array(int32_t length, int atype) {
    size_t elem_size;
    switch (atype) {
        case T_BOOLEAN: case T_BYTE:           elem_size = sizeof(int8_t);   break;
        case T_CHAR:    case T_SHORT:          elem_size = sizeof(int16_t);  break;
        case T_FLOAT:   case T_INT:            elem_size = sizeof(int32_t);  break;
        case T_DOUBLE:  case T_LONG:           elem_size = sizeof(int64_t);  break;
        case T_OBJECT:                         elem_size = sizeof(void *);   break;
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
    Frame *f = calloc(1, sizeof(Frame));
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
        memset(&f->pilha[++f->topo], 0, sizeof(Slot));
        f->pilha[f->topo].flutuante = val;
        f->pilha[f->topo].tipo      = TIPO_FLOAT;
    } else if (tag == CONSTANT_String) {
        u2 str_idx = cp[idx].string_info->string_index;
        char *bytes = (char *)cp[str_idx].utf8_info->bytes;
        memset(&f->pilha[++f->topo], 0, sizeof(Slot));
        f->pilha[f->topo].longo = (int64_t)(uintptr_t)bytes;
        f->pilha[f->topo].ref   = (void *)bytes;
        f->pilha[f->topo].tipo  = TIPO_REF;
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
        memset(&f->pilha[++f->topo], 0, sizeof(Slot));
        f->pilha[f->topo].longo = (int64_t)((hi << 32) | lo);
        f->pilha[f->topo].tipo  = TIPO_LONG;
    } else if (tag == CONSTANT_Double) {
        uint64_t hi   = cp[idx].double_info->high_bytes;
        uint64_t lo   = cp[idx].double_info->low_bytes;
        uint64_t bits = (hi << 32) | lo;
        double val;
        memcpy(&val, &bits, sizeof(double));
        memset(&f->pilha[++f->topo], 0, sizeof(Slot));
        f->pilha[f->topo].duplo = val;
        f->pilha[f->topo].tipo  = TIPO_DOUBLE;
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

        /* ── nop ──────────────────────────────────────── */
        case 0x00: break;                    /* nop */

        /* ── aconst_null ──────────────────────────────── */
        case 0x01:                           /* aconst_null */
            memset(&f->pilha[++f->topo], 0, sizeof(Slot));
            f->pilha[f->topo].tipo = TIPO_REF;
            break;

        /* ── constantes inteiras ───────────────────────── */
        case 0x02: PUSH_INT(f, -1); break;  /* iconst_m1 */
        case 0x03: PUSH_INT(f,  0); break;  /* iconst_0  */
        case 0x04: PUSH_INT(f,  1); break;  /* iconst_1  */
        case 0x05: PUSH_INT(f,  2); break;  /* iconst_2  */
        case 0x06: PUSH_INT(f,  3); break;  /* iconst_3  */
        case 0x07: PUSH_INT(f,  4); break;  /* iconst_4  */
        case 0x08: PUSH_INT(f,  5); break;  /* iconst_5  */

        /* ── constantes long ───────────────────────────── */
        case 0x09: PUSH_LONG(f, 0LL); break; /* lconst_0 */
        case 0x0A: PUSH_LONG(f, 1LL); break; /* lconst_1 */

        /* ── constantes float ──────────────────────────── */
        case 0x0B: PUSH_FLOAT(f, 0.0f); break; /* fconst_0 */
        case 0x0C: PUSH_FLOAT(f, 1.0f); break; /* fconst_1 */
        case 0x0D: PUSH_FLOAT(f, 2.0f); break; /* fconst_2 */

        /* ── constantes double ─────────────────────────── */
        case 0x0E: PUSH_DOUBLE(f, 0.0); break; /* dconst_0 */
        case 0x0F: PUSH_DOUBLE(f, 1.0); break; /* dconst_1 */

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

        /* ── load/store com índice explícito ──────────── */
        case 0x15: { u1 i = READ_U1(f); PUSH_INT(f, f->locais[i].inteiro); break; } /* iload */
        case 0x16: { u1 i = READ_U1(f); f->pilha[++f->topo] = f->locais[i]; break; } /* lload */
        case 0x17: { u1 i = READ_U1(f); f->pilha[++f->topo] = f->locais[i]; break; } /* fload */
        case 0x18: { u1 i = READ_U1(f); f->pilha[++f->topo] = f->locais[i]; break; } /* dload */
        case 0x19: { u1 i = READ_U1(f); f->pilha[++f->topo] = f->locais[i]; break; } /* aload */

        case 0x36: { u1 i = READ_U1(f); f->locais[i].inteiro = POP_INT(f); f->locais[i].tipo = TIPO_INT; break; } /* istore */
        case 0x37: { u1 i = READ_U1(f); f->locais[i] = f->pilha[f->topo--]; break; } /* lstore */
        case 0x38: { u1 i = READ_U1(f); f->locais[i] = f->pilha[f->topo--]; break; } /* fstore */
        case 0x39: { u1 i = READ_U1(f); f->locais[i] = f->pilha[f->topo--]; break; } /* dstore */
        case 0x3A: { u1 i = READ_U1(f); f->locais[i] = f->pilha[f->topo--]; break; } /* astore */

        /* ── lload_0-3 ─────────────────────────────────── */
        case 0x1E: f->pilha[++f->topo] = f->locais[0]; break; /* lload_0 */
        case 0x1F: f->pilha[++f->topo] = f->locais[1]; break; /* lload_1 */
        case 0x20: f->pilha[++f->topo] = f->locais[2]; break; /* lload_2 */
        case 0x21: f->pilha[++f->topo] = f->locais[3]; break; /* lload_3 */

        /* ── fload_0-3 ─────────────────────────────────── */
        case 0x22: f->pilha[++f->topo] = f->locais[0]; break; /* fload_0 */
        case 0x23: f->pilha[++f->topo] = f->locais[1]; break; /* fload_1 */
        case 0x24: f->pilha[++f->topo] = f->locais[2]; break; /* fload_2 */
        case 0x25: f->pilha[++f->topo] = f->locais[3]; break; /* fload_3 */

        /* ── dload_0-3 ─────────────────────────────────── */
        case 0x26: f->pilha[++f->topo] = f->locais[0]; break; /* dload_0 */
        case 0x27: f->pilha[++f->topo] = f->locais[1]; break; /* dload_1 */
        case 0x28: f->pilha[++f->topo] = f->locais[2]; break; /* dload_2 */
        case 0x29: f->pilha[++f->topo] = f->locais[3]; break; /* dload_3 */

        /* ── aload / astore (referências) ─────────────── */
        case 0x2A: f->pilha[++f->topo] = f->locais[0]; break; /* aload_0 */
        case 0x2B: f->pilha[++f->topo] = f->locais[1]; break; /* aload_1 */
        case 0x2C: f->pilha[++f->topo] = f->locais[2]; break; /* aload_2 */
        case 0x2D: f->pilha[++f->topo] = f->locais[3]; break; /* aload_3 */

        /* ── lstore_0-3 ────────────────────────────────── */
        case 0x3F: f->locais[0] = f->pilha[f->topo--]; break; /* lstore_0 */
        case 0x40: f->locais[1] = f->pilha[f->topo--]; break; /* lstore_1 */
        case 0x41: f->locais[2] = f->pilha[f->topo--]; break; /* lstore_2 */
        case 0x42: f->locais[3] = f->pilha[f->topo--]; break; /* lstore_3 */

        /* ── fstore_0-3 ────────────────────────────────── */
        case 0x43: f->locais[0] = f->pilha[f->topo--]; break; /* fstore_0 */
        case 0x44: f->locais[1] = f->pilha[f->topo--]; break; /* fstore_1 */
        case 0x45: f->locais[2] = f->pilha[f->topo--]; break; /* fstore_2 */
        case 0x46: f->locais[3] = f->pilha[f->topo--]; break; /* fstore_3 */

        /* ── dstore_0-3 ────────────────────────────────── */
        case 0x47: f->locais[0] = f->pilha[f->topo--]; break; /* dstore_0 */
        case 0x48: f->locais[1] = f->pilha[f->topo--]; break; /* dstore_1 */
        case 0x49: f->locais[2] = f->pilha[f->topo--]; break; /* dstore_2 */
        case 0x4A: f->locais[3] = f->pilha[f->topo--]; break; /* dstore_3 */

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

        case 0x2F: {                         /* laload */
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_LONG(f, ((int64_t *)arr->data)[idx]);
            break;
        }

        case 0x30: {                         /* faload */
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_FLOAT(f, ((float *)arr->data)[idx]);
            break;
        }

        case 0x31: {                         /* daload */
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_DOUBLE(f, ((double *)arr->data)[idx]);
            break;
        }

        case 0x32: {                         /* aaload */
            int32_t   idx = POP_INT(f);
            JVMArray *src = ARR(f->pilha[f->topo--]);
            void     *val = src ? ((void **)src->data)[idx] : NULL;
            memset(&f->pilha[++f->topo], 0, sizeof(Slot));
            f->pilha[f->topo].longo = (int64_t)(uintptr_t)val;
            f->pilha[f->topo].ref   = val;
            /* Arrays de objetos (anewarray) contêm HeapObject* — preservar TIPO_OBJECT */
            f->pilha[f->topo].tipo = (src && src->atype == T_OBJECT)
                                     ? TIPO_OBJECT : TIPO_REF;
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

        case 0x35: {                         /* saload (short) */
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, (int32_t)((int16_t *)arr->data)[idx]);
            break;
        }

        case 0x4F: {                         /* iastore */
            int32_t   val = POP_INT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((int32_t *)arr->data)[idx] = val;
            break;
        }

        case 0x50: {                         /* lastore */
            int64_t   val = POP_LONG(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((int64_t *)arr->data)[idx] = val;
            break;
        }

        case 0x51: {                         /* fastore */
            float     val = POP_FLOAT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((float *)arr->data)[idx] = val;
            break;
        }

        case 0x52: {                         /* dastore */
            double    val = POP_DOUBLE(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((double *)arr->data)[idx] = val;
            break;
        }

        case 0x53: {                         /* aastore */
            Slot  vs  = f->pilha[f->topo--];
            /* TIPO_OBJECT usa .ref; todos os outros (TIPO_REF, TIPO_ARRAY) usam .longo */
            void *val = (vs.tipo == TIPO_OBJECT)
                        ? vs.ref
                        : (void *)(uintptr_t)vs.longo;
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            if (arr) ((void **)arr->data)[idx] = val;
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

        case 0x56: {                         /* sastore (short) */
            int32_t   val = POP_INT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((int16_t *)arr->data)[idx] = (int16_t)val;
            break;
        }

        case 0xBC: {                         /* newarray */
            u1        atype  = READ_U1(f);
            int32_t   length = POP_INT(f);
            JVMArray *arr    = aloca_array(length, (int)atype);
            memset(&f->pilha[++f->topo], 0, sizeof(Slot));
            f->pilha[f->topo].longo = (int64_t)(uintptr_t)arr;
            f->pilha[f->topo].tipo  = TIPO_ARRAY;
            break;
        }

        case 0xBD: {                         /* anewarray */
            read_u2(f); /* índice de classe no CP — ignorado */
            int32_t   length = POP_INT(f);
            JVMArray *arr    = aloca_array(length, T_OBJECT);
            memset(&f->pilha[++f->topo], 0, sizeof(Slot));
            f->pilha[f->topo].longo = (int64_t)(uintptr_t)arr;
            f->pilha[f->topo].tipo  = TIPO_ARRAY;
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
            if (v2 == 0) { fprintf(stderr, "ArithmeticException: / by zero\n"); return; }
            PUSH_INT(f, v1 / v2);
            break;
        }

        case 0x70: {                         /* irem */
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            if (v2 == 0) { fprintf(stderr, "ArithmeticException: / by zero (irem)\n"); return; }
            PUSH_INT(f, v1 % v2);
            break;
        }

        case 0x74: {                         /* ineg */
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, -v1);
            break;
        }

        /* ── aritmética long ───────────────────────────── */
        case 0x61: { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a+b); break; } /* ladd */
        case 0x65: { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a-b); break; } /* lsub */
        case 0x69: { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a*b); break; } /* lmul */
        case 0x6D: {                         /* ldiv */
            int64_t b = POP_LONG(f), a = POP_LONG(f);
            if (b == 0) { fprintf(stderr, "ArithmeticException: / by zero (long)\n"); return; }
            PUSH_LONG(f, a / b); break;
        }
        case 0x71: {                         /* lrem */
            int64_t b = POP_LONG(f), a = POP_LONG(f);
            if (b == 0) { fprintf(stderr, "ArithmeticException: / by zero (lrem)\n"); return; }
            PUSH_LONG(f, a % b); break;
        }
        case 0x75: { int64_t v=POP_LONG(f); PUSH_LONG(f, -v); break; } /* lneg */

        /* ── aritmética float ──────────────────────────── */
        case 0x62: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, a+b); break; } /* fadd */
        case 0x66: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, a-b); break; } /* fsub */
        case 0x6A: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, a*b); break; } /* fmul */
        case 0x6E: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, a/b); break; } /* fdiv */
        case 0x72: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, fmodf(a,b)); break; } /* frem */
        case 0x76: { float v=POP_FLOAT(f); PUSH_FLOAT(f, -v); break; } /* fneg */

        /* ── aritmética double ─────────────────────────── */
        case 0x63: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, a+b); break; } /* dadd */
        case 0x67: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, a-b); break; } /* dsub */
        case 0x6B: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, a*b); break; } /* dmul */
        case 0x6F: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, a/b); break; } /* ddiv */
        case 0x73: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, fmod(a,b)); break; } /* drem */
        case 0x77: { double v=POP_DOUBLE(f); PUSH_DOUBLE(f, -v); break; } /* dneg */

        /* ── bitwise inteiro ───────────────────────────── */
        case 0x78: { int32_t b=POP_INT(f)&0x1F, a=POP_INT(f); PUSH_INT(f, a<<b); break; }  /* ishl */
        case 0x7A: { int32_t b=POP_INT(f)&0x1F, a=POP_INT(f); PUSH_INT(f, a>>b); break; }  /* ishr */
        case 0x7C: { int32_t b=POP_INT(f)&0x1F; uint32_t a=(uint32_t)POP_INT(f); PUSH_INT(f, (int32_t)(a>>b)); break; } /* iushr */
        case 0x7E: { int32_t b=POP_INT(f), a=POP_INT(f); PUSH_INT(f, a&b); break; }  /* iand */
        case 0x80: { int32_t b=POP_INT(f), a=POP_INT(f); PUSH_INT(f, a|b); break; }  /* ior  */
        case 0x82: { int32_t b=POP_INT(f), a=POP_INT(f); PUSH_INT(f, a^b); break; }  /* ixor */

        /* ── bitwise long ──────────────────────────────── */
        case 0x79: { int32_t b=POP_INT(f)&0x3F; int64_t a=POP_LONG(f); PUSH_LONG(f, a<<b); break; }  /* lshl */
        case 0x7B: { int32_t b=POP_INT(f)&0x3F; int64_t a=POP_LONG(f); PUSH_LONG(f, a>>b); break; }  /* lshr */
        case 0x7D: { int32_t b=POP_INT(f)&0x3F; uint64_t a=(uint64_t)POP_LONG(f); PUSH_LONG(f, (int64_t)(a>>b)); break; } /* lushr */
        case 0x7F: { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a&b); break; }  /* land */
        case 0x81: { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a|b); break; }  /* lor  */
        case 0x83: { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a^b); break; }  /* lxor */

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

        /* ── conversões de tipo ────────────────────────── */
        case 0x85: { int32_t v=POP_INT(f); PUSH_LONG(f, (int64_t)v); break; }   /* i2l */
        case 0x86: { int32_t v=POP_INT(f); PUSH_FLOAT(f, (float)v); break; }    /* i2f */
        case 0x87: { int32_t v=POP_INT(f); PUSH_DOUBLE(f, (double)v); break; }  /* i2d */
        case 0x88: { int64_t v=POP_LONG(f); PUSH_INT(f, (int32_t)v); break; }   /* l2i */
        case 0x89: { int64_t v=POP_LONG(f); PUSH_FLOAT(f, (float)v); break; }   /* l2f */
        case 0x8A: { int64_t v=POP_LONG(f); PUSH_DOUBLE(f, (double)v); break; } /* l2d */
        case 0x8B: { float v=POP_FLOAT(f); PUSH_INT(f, isnan(v)?0:(int32_t)v); break; } /* f2i */
        case 0x8C: { float v=POP_FLOAT(f); PUSH_LONG(f, isnan(v)?0LL:(int64_t)v); break; } /* f2l */
        case 0x8D: { float v=POP_FLOAT(f); PUSH_DOUBLE(f, (double)v); break; }  /* f2d */
        case 0x8E: { double v=POP_DOUBLE(f); PUSH_INT(f, isnan(v)?0:(int32_t)v); break; } /* d2i */
        case 0x8F: { double v=POP_DOUBLE(f); PUSH_LONG(f, isnan(v)?0LL:(int64_t)v); break; } /* d2l */
        case 0x90: { double v=POP_DOUBLE(f); PUSH_FLOAT(f, (float)v); break; }  /* d2f */
        case 0x91: { int32_t v=POP_INT(f); PUSH_INT(f, (int32_t)(int8_t)v); break; }  /* i2b */
        case 0x92: { int32_t v=POP_INT(f); PUSH_INT(f, v & 0xFFFF); break; }    /* i2c */
        case 0x93: { int32_t v=POP_INT(f); PUSH_INT(f, (int32_t)(int16_t)v); break; } /* i2s */

        /* ── comparações numéricas ─────────────────────── */
        case 0x94: { /* lcmp */
            int64_t b=POP_LONG(f), a=POP_LONG(f);
            PUSH_INT(f, a>b ? 1 : a<b ? -1 : 0);
            break;
        }
        case 0x95: { /* fcmpl — NaN → -1 */
            float b=POP_FLOAT(f), a=POP_FLOAT(f);
            PUSH_INT(f, (isnan(a)||isnan(b)) ? -1 : a>b ? 1 : a<b ? -1 : 0);
            break;
        }
        case 0x96: { /* fcmpg — NaN → 1 */
            float b=POP_FLOAT(f), a=POP_FLOAT(f);
            PUSH_INT(f, (isnan(a)||isnan(b)) ? 1 : a>b ? 1 : a<b ? -1 : 0);
            break;
        }
        case 0x97: { /* dcmpl — NaN → -1 */
            double b=POP_DOUBLE(f), a=POP_DOUBLE(f);
            PUSH_INT(f, (isnan(a)||isnan(b)) ? -1 : a>b ? 1 : a<b ? -1 : 0);
            break;
        }
        case 0x98: { /* dcmpg — NaN → 1 */
            double b=POP_DOUBLE(f), a=POP_DOUBLE(f);
            PUSH_INT(f, (isnan(a)||isnan(b)) ? 1 : a>b ? 1 : a<b ? -1 : 0);
            break;
        }

        /* ── manipulação de pilha ─────────────────────── */
        case 0x57:                           /* pop */
            f->topo--;
            break;

        case 0x58:                           /* pop2 */
            f->topo -= 2;
            break;

        case 0x59: {                         /* dup */
            f->pilha[f->topo + 1] = f->pilha[f->topo];
            f->topo++;
            break;
        }

        case 0x5A: {                         /* dup_x1: ..v2,v1 → ..v1,v2,v1 */
            Slot v1 = f->pilha[f->topo];
            f->pilha[f->topo + 1] = f->pilha[f->topo];
            f->pilha[f->topo]     = f->pilha[f->topo - 1];
            f->pilha[f->topo - 1] = v1;
            f->topo++;
            break;
        }

        case 0x5B: {                         /* dup_x2: ..v3,v2,v1 → ..v1,v3,v2,v1 */
            Slot v1 = f->pilha[f->topo];
            f->pilha[f->topo + 1] = f->pilha[f->topo];
            f->pilha[f->topo]     = f->pilha[f->topo - 1];
            f->pilha[f->topo - 1] = f->pilha[f->topo - 2];
            f->pilha[f->topo - 2] = v1;
            f->topo++;
            break;
        }

        case 0x5C: {                         /* dup2: ..v2,v1 → ..v2,v1,v2,v1 */
            f->pilha[f->topo + 1] = f->pilha[f->topo - 1];
            f->pilha[f->topo + 2] = f->pilha[f->topo];
            f->topo += 2;
            break;
        }

        case 0x5D: {                         /* dup2_x1: ..v3,v2,v1 → ..v2,v1,v3,v2,v1 */
            Slot v1 = f->pilha[f->topo];
            Slot v2 = f->pilha[f->topo - 1];
            f->pilha[f->topo + 2] = v1;
            f->pilha[f->topo + 1] = v2;
            f->pilha[f->topo]     = f->pilha[f->topo - 2];
            f->pilha[f->topo - 1] = v1;
            f->pilha[f->topo - 2] = v2;
            f->topo += 2;
            break;
        }

        case 0x5E: {                         /* dup2_x2: ..v4,v3,v2,v1 → ..v2,v1,v4,v3,v2,v1 */
            Slot v1 = f->pilha[f->topo];
            Slot v2 = f->pilha[f->topo - 1];
            f->pilha[f->topo + 2] = v1;
            f->pilha[f->topo + 1] = v2;
            f->pilha[f->topo]     = f->pilha[f->topo - 2];
            f->pilha[f->topo - 1] = f->pilha[f->topo - 3];
            f->pilha[f->topo - 2] = v1;
            f->pilha[f->topo - 3] = v2;
            f->topo += 2;
            break;
        }

        case 0x5F: {                         /* swap */
            Slot tmp              = f->pilha[f->topo];
            f->pilha[f->topo]     = f->pilha[f->topo - 1];
            f->pilha[f->topo - 1] = tmp;
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

            /* fallback: usa cf se o arquivo não existe no disco e cf é a própria
             * classe (this_class == 0 em ClassFiles sintéticos de teste, ou mesmo
             * nome de classe) */
            if (obj_cf == NULL) {
                char *self_name = getClassName(cf);
                if (self_name == NULL ||
                    (cname && strcmp(self_name, cname) == 0))
                    obj_cf = cf;
            }

            /* cria o objeto */
            HeapObject *obj = alocaObjeto(obj_cf, cname);

            /* empilha referência */
            memset(&f->pilha[++f->topo], 0, sizeof(Slot));
            f->pilha[f->topo].tipo  = TIPO_OBJECT;
            f->pilha[f->topo].ref   = obj;
            f->pilha[f->topo].longo = (int64_t)(uintptr_t)obj;

            free(cname);

            break;
        }

        case 0xB4: {                         /* getfield */
            u2 idx = read_u2(f);

            if (!cf) {
                PUSH_INT(f, 0);
                break;
            }

            Slot obj_slot = f->pilha[f->topo--];
            HeapObject *obj = NULL;
            if (obj_slot.tipo == TIPO_OBJECT)
                obj = (HeapObject *)obj_slot.ref;

            const CONSTANT_Fieldref_info *fref =
                cf->constant_pool[idx].fieldRef_info;

            const CONSTANT_NameAndType_info *nat =
                cf->constant_pool[fref->name_and_type_index]
                    .nameAndType_info;

            char *fname =
                getUtf8(cf->constant_pool,
                        nat->name_index);

            int fi = obj ? buscaCampo(obj, fname) : -1;

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

            Slot obj_slot2 = f->pilha[f->topo--];
            HeapObject *obj = (obj_slot2.tipo == TIPO_OBJECT)
                              ? (HeapObject *)obj_slot2.ref : NULL;

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

        /* ── getstatic / putstatic ─────────────────────── */
        case 0xB2: {                         /* getstatic */
            u2 idx = read_u2(f);
            if (!cf) { PUSH_INT(f, 0); break; }
            const CONSTANT_Fieldref_info *fref = cf->constant_pool[idx].fieldRef_info;

            /* caso especial: java/lang/System.out */
            int is_out = resolve_class_name(cf->constant_pool, fref->class_index,
                                            "java/lang/System") &&
                         resolve_member_name(cf->constant_pool, fref->name_and_type_index,
                                             "out");
            if (is_out) {
                memset(&f->pilha[++f->topo], 0, sizeof(Slot));
                f->pilha[f->topo].tipo = TIPO_REF;
                break;
            }

            /* campos estáticos de classes usuario */
            const CONSTANT_NameAndType_info *nat =
                cf->constant_pool[fref->name_and_type_index].nameAndType_info;
            char *cls_name = getUtf8(cf->constant_pool,
                cf->constant_pool[fref->class_index].constant_class_info->name_index);
            char *fld_name = getUtf8(cf->constant_pool, nat->name_index);

            int fi = find_static_field(cls_name, fld_name);
            free(cls_name);
            free(fld_name);

            if (fi >= 0) {
                f->pilha[++f->topo] = static_fields[fi].value;
            } else {
                fprintf(stderr, "getstatic: campo nao inicializado (idx=%u)\n", idx);
                PUSH_INT(f, 0);
            }
            break;
        }

        case 0xB3: {                         /* putstatic */
            u2 idx = read_u2(f);
            if (!cf) { f->topo--; break; }

            const CONSTANT_Fieldref_info   *fref = cf->constant_pool[idx].fieldRef_info;
            const CONSTANT_NameAndType_info *nat  =
                cf->constant_pool[fref->name_and_type_index].nameAndType_info;

            char *cls_name = getUtf8(cf->constant_pool,
                cf->constant_pool[fref->class_index].constant_class_info->name_index);
            char *fld_name = getUtf8(cf->constant_pool, nat->name_index);

            int fi = find_static_field(cls_name, fld_name);
            if (fi < 0)
                fi = alloc_static_field(cls_name, fld_name);

            if (fi >= 0)
                static_fields[fi].value = f->pilha[f->topo];
            f->topo--;

            free(cls_name);
            free(fld_name);
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

        case 0xB9: {                         /* invokeinterface */
            u2 idx = read_u2(f);
            READ_U1(f); /* count */
            READ_U1(f); /* 0 */
            if (!cf) { return; }
            execInvokevirtual(f, cf, idx);
            break;
        }

        case 0xBF: {                         /* athrow */
            fprintf(stderr, "athrow: excecao lancada (nao suportada)\n");
            return;
        }

        case 0xC0: {                         /* checkcast — sem verificação real */
            read_u2(f);
            /* mantém o ref no topo */
            break;
        }

        case 0xC1: {                         /* instanceof */
            read_u2(f);
            Slot s = f->pilha[f->topo--];
            PUSH_INT(f, (s.ref != NULL || s.tipo == TIPO_OBJECT) ? 1 : 0);
            break;
        }

        case 0xC2: /* monitorenter */
        case 0xC3: /* monitorexit */
            f->topo--;
            break;

        case 0xC4: {                         /* wide */
            u1 next = READ_U1(f);
            u2 idx  = read_u2(f);
            if (next == 0x84) { /* wide iinc */
                int16_t cst = read_i2(f);
                f->locais[idx].inteiro += (int32_t)cst;
            } else {
                switch (next) {
                    case 0x15: PUSH_INT(f, f->locais[idx].inteiro); break; /* iload */
                    case 0x16: case 0x17: case 0x18: case 0x19:
                        f->pilha[++f->topo] = f->locais[idx]; break; /* lload/fload/dload/aload */
                    case 0x36: f->locais[idx].inteiro = POP_INT(f); f->locais[idx].tipo = TIPO_INT; break; /* istore */
                    case 0x37: case 0x38: case 0x39: case 0x3A:
                        f->locais[idx] = f->pilha[f->topo--]; break; /* lstore/fstore/dstore/astore */
                    default:
                        fprintf(stderr, "wide: opcode 0x%02X nao suportado\n", next);
                        break;
                }
            }
            break;
        }

        case 0xC5: {                         /* multianewarray */
            u2 idx  = read_u2(f);
            u1 dims = READ_U1(f);
            (void)idx;
            /* Coleta dimensões do topo (innermost) para base (outermost) */
            int32_t dim_vals[16];
            if (dims > 16) dims = 16;
            for (int d = (int)dims - 1; d >= 0; d--)
                dim_vals[d] = POP_INT(f);
            /* Cria array externo e inicializa cada sub-array */
            JVMArray *outer = aloca_array(dim_vals[0], T_OBJECT);
            if (dims >= 2) {
                void **slots = (void **)outer->data;
                for (int32_t i = 0; i < dim_vals[0]; i++) {
                    JVMArray *inner = aloca_array(dim_vals[1], T_OBJECT);
                    slots[i] = inner;
                }
            }
            memset(&f->pilha[++f->topo], 0, sizeof(Slot));
            f->pilha[f->topo].longo = (int64_t)(uintptr_t)outer;
            f->pilha[f->topo].tipo  = TIPO_ARRAY;
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

        case 0xA5: { /* if_acmpeq */
            int16_t o = read_i2(f);
            void *s2 = f->pilha[f->topo--].ref;
            void *s1 = f->pilha[f->topo--].ref;
            if (s1 == s2) f->pc = instr_pc + (u4)(int32_t)o;
            break;
        }
        case 0xA6: { /* if_acmpne */
            int16_t o = read_i2(f);
            void *s2 = f->pilha[f->topo--].ref;
            void *s1 = f->pilha[f->topo--].ref;
            if (s1 != s2) f->pc = instr_pc + (u4)(int32_t)o;
            break;
        }

        case 0xA7: {                         /* goto */
            int16_t offset = read_i2(f);
            f->pc = instr_pc + (u4)(int32_t)offset;
            break;
        }

        case 0xAA: {                         /* tableswitch */
            u4 pad = (4 - (f->pc % 4)) % 4;
            f->pc += pad;
            int32_t deflt = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
            int32_t low   = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
            int32_t high  = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
            int32_t key   = POP_INT(f);
            if (key < low || key > high) {
                f->pc = instr_pc + (u4)deflt;
            } else {
                u4 entry_offset = (u4)(key - low) * 4;
                f->pc += entry_offset;
                int32_t off = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
                f->pc = instr_pc + (u4)off;
            }
            break;
        }

        case 0xAB: {                         /* lookupswitch */
            u4 pad    = (4 - (f->pc % 4)) % 4;
            f->pc += pad;
            int32_t deflt  = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
            int32_t npairs = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
            int32_t key    = POP_INT(f);
            int32_t target = instr_pc + deflt;
            for (int32_t k = 0; k < npairs; k++) {
                int32_t match = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
                int32_t off   = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
                if (match == key) target = instr_pc + off;
            }
            f->pc = (u4)target;
            break;
        }

        case 0xC6: {                         /* ifnull */
            int16_t o = read_i2(f);
            Slot s = f->pilha[f->topo--];
            if (s.ref == NULL && s.longo == 0) f->pc = instr_pc + (u4)(int32_t)o;
            break;
        }
        case 0xC7: {                         /* ifnonnull */
            int16_t o = read_i2(f);
            Slot s = f->pilha[f->topo--];
            if (s.ref != NULL || s.longo != 0) f->pc = instr_pc + (u4)(int32_t)o;
            break;
        }

        case 0xC8: {                         /* goto_w */
            int32_t off = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
            f->pc = instr_pc + (u4)off;
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
