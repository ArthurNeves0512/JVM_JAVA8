#include "basic_ops.h"
#include "heap.h"
#include "method_invoke.h"
#include "native_methods.h"
#include "attribute.h"
#include "consts.h"
#include "opcodes.h"
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
        case OP_NOP: break;

        /* ── aconst_null ──────────────────────────────── */
        case OP_ACONST_NULL:
            memset(&f->pilha[++f->topo], 0, sizeof(Slot));
            f->pilha[f->topo].tipo = TIPO_REF;
            break;

        /* ── constantes inteiras ───────────────────────── */
        case OP_ICONST_M1: PUSH_INT(f, -1); break;
        case OP_ICONST_0:  PUSH_INT(f,  0); break;
        case OP_ICONST_1:  PUSH_INT(f,  1); break;
        case OP_ICONST_2:  PUSH_INT(f,  2); break;
        case OP_ICONST_3:  PUSH_INT(f,  3); break;
        case OP_ICONST_4:  PUSH_INT(f,  4); break;
        case OP_ICONST_5:  PUSH_INT(f,  5); break;

        /* ── constantes long ───────────────────────────── */
        case OP_LCONST_0: PUSH_LONG(f, 0LL); break;
        case OP_LCONST_1: PUSH_LONG(f, 1LL); break;

        /* ── constantes float ──────────────────────────── */
        case OP_FCONST_0: PUSH_FLOAT(f, 0.0f); break;
        case OP_FCONST_1: PUSH_FLOAT(f, 1.0f); break;
        case OP_FCONST_2: PUSH_FLOAT(f, 2.0f); break;

        /* ── constantes double ─────────────────────────── */
        case OP_DCONST_0: PUSH_DOUBLE(f, 0.0); break;
        case OP_DCONST_1: PUSH_DOUBLE(f, 1.0); break;

        case OP_BIPUSH: {
            int8_t val = (int8_t)READ_U1(f);
            PUSH_INT(f, (int32_t)val);
            break;
        }

        case OP_SIPUSH: {
            int16_t val = read_i2(f);
            PUSH_INT(f, (int32_t)val);
            break;
        }

        /* ── load/store com índice explícito ──────────── */
        case OP_ILOAD: { u1 i = READ_U1(f); PUSH_INT(f, f->locais[i].inteiro); break; }
        case OP_LLOAD: { u1 i = READ_U1(f); f->pilha[++f->topo] = f->locais[i]; break; }
        case OP_FLOAD: { u1 i = READ_U1(f); f->pilha[++f->topo] = f->locais[i]; break; }
        case OP_DLOAD: { u1 i = READ_U1(f); f->pilha[++f->topo] = f->locais[i]; break; }
        case OP_ALOAD: { u1 i = READ_U1(f); f->pilha[++f->topo] = f->locais[i]; break; }

        case OP_ISTORE: { u1 i = READ_U1(f); f->locais[i].inteiro = POP_INT(f); f->locais[i].tipo = TIPO_INT; break; }
        case OP_LSTORE: { u1 i = READ_U1(f); f->locais[i] = f->pilha[f->topo--]; break; }
        case OP_FSTORE: { u1 i = READ_U1(f); f->locais[i] = f->pilha[f->topo--]; break; }
        case OP_DSTORE: { u1 i = READ_U1(f); f->locais[i] = f->pilha[f->topo--]; break; }
        case OP_ASTORE: { u1 i = READ_U1(f); f->locais[i] = f->pilha[f->topo--]; break; }

        /* ── lload_0-3 ─────────────────────────────────── */
        case OP_LLOAD_0: f->pilha[++f->topo] = f->locais[0]; break;
        case OP_LLOAD_1: f->pilha[++f->topo] = f->locais[1]; break;
        case OP_LLOAD_2: f->pilha[++f->topo] = f->locais[2]; break;
        case OP_LLOAD_3: f->pilha[++f->topo] = f->locais[3]; break;

        /* ── fload_0-3 ─────────────────────────────────── */
        case OP_FLOAD_0: f->pilha[++f->topo] = f->locais[0]; break;
        case OP_FLOAD_1: f->pilha[++f->topo] = f->locais[1]; break;
        case OP_FLOAD_2: f->pilha[++f->topo] = f->locais[2]; break;
        case OP_FLOAD_3: f->pilha[++f->topo] = f->locais[3]; break;

        /* ── dload_0-3 ─────────────────────────────────── */
        case OP_DLOAD_0: f->pilha[++f->topo] = f->locais[0]; break;
        case OP_DLOAD_1: f->pilha[++f->topo] = f->locais[1]; break;
        case OP_DLOAD_2: f->pilha[++f->topo] = f->locais[2]; break;
        case OP_DLOAD_3: f->pilha[++f->topo] = f->locais[3]; break;

        /* ── aload_0-3 (referências) ───────────────────── */
        case OP_ALOAD_0: f->pilha[++f->topo] = f->locais[0]; break;
        case OP_ALOAD_1: f->pilha[++f->topo] = f->locais[1]; break;
        case OP_ALOAD_2: f->pilha[++f->topo] = f->locais[2]; break;
        case OP_ALOAD_3: f->pilha[++f->topo] = f->locais[3]; break;

        /* ── lstore_0-3 ────────────────────────────────── */
        case OP_LSTORE_0: f->locais[0] = f->pilha[f->topo--]; break;
        case OP_LSTORE_1: f->locais[1] = f->pilha[f->topo--]; break;
        case OP_LSTORE_2: f->locais[2] = f->pilha[f->topo--]; break;
        case OP_LSTORE_3: f->locais[3] = f->pilha[f->topo--]; break;

        /* ── fstore_0-3 ────────────────────────────────── */
        case OP_FSTORE_0: f->locais[0] = f->pilha[f->topo--]; break;
        case OP_FSTORE_1: f->locais[1] = f->pilha[f->topo--]; break;
        case OP_FSTORE_2: f->locais[2] = f->pilha[f->topo--]; break;
        case OP_FSTORE_3: f->locais[3] = f->pilha[f->topo--]; break;

        /* ── dstore_0-3 ────────────────────────────────── */
        case OP_DSTORE_0: f->locais[0] = f->pilha[f->topo--]; break;
        case OP_DSTORE_1: f->locais[1] = f->pilha[f->topo--]; break;
        case OP_DSTORE_2: f->locais[2] = f->pilha[f->topo--]; break;
        case OP_DSTORE_3: f->locais[3] = f->pilha[f->topo--]; break;

        /* ── astore_0-3 ────────────────────────────────── */
        case OP_ASTORE_0: f->locais[0] = f->pilha[f->topo--]; break;
        case OP_ASTORE_1: f->locais[1] = f->pilha[f->topo--]; break;
        case OP_ASTORE_2: f->locais[2] = f->pilha[f->topo--]; break;
        case OP_ASTORE_3: f->locais[3] = f->pilha[f->topo--]; break;

        /* ── arrays de primitivos ─────────────────────── */
        case OP_IALOAD: {
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, ((int32_t *)arr->data)[idx]);
            break;
        }

        case OP_LALOAD: {
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_LONG(f, ((int64_t *)arr->data)[idx]);
            break;
        }

        case OP_FALOAD: {
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_FLOAT(f, ((float *)arr->data)[idx]);
            break;
        }

        case OP_DALOAD: {
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_DOUBLE(f, ((double *)arr->data)[idx]);
            break;
        }

        case OP_AALOAD: {
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

        case OP_BALOAD: {
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, (int32_t)((int8_t *)arr->data)[idx]);
            break;
        }

        case OP_CALOAD: {
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, (int32_t)((uint16_t *)arr->data)[idx]);
            break;
        }

        case OP_SALOAD: {
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, (int32_t)((int16_t *)arr->data)[idx]);
            break;
        }

        case OP_IASTORE: {
            int32_t   val = POP_INT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((int32_t *)arr->data)[idx] = val;
            break;
        }

        case OP_LASTORE: {
            int64_t   val = POP_LONG(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((int64_t *)arr->data)[idx] = val;
            break;
        }

        case OP_FASTORE: {
            float     val = POP_FLOAT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((float *)arr->data)[idx] = val;
            break;
        }

        case OP_DASTORE: {
            double    val = POP_DOUBLE(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((double *)arr->data)[idx] = val;
            break;
        }

        case OP_AASTORE: {
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

        case OP_BASTORE: {
            int32_t   val = POP_INT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((int8_t *)arr->data)[idx] = (int8_t)val;
            break;
        }

        case OP_CASTORE: {
            int32_t   val = POP_INT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((uint16_t *)arr->data)[idx] = (uint16_t)val;
            break;
        }

        case OP_SASTORE: {
            int32_t   val = POP_INT(f);
            int32_t   idx = POP_INT(f);
            JVMArray *arr = ARR(f->pilha[f->topo--]);
            ((int16_t *)arr->data)[idx] = (int16_t)val;
            break;
        }

        case OP_NEWARRAY: {
            u1        atype  = READ_U1(f);
            int32_t   length = POP_INT(f);
            JVMArray *arr    = aloca_array(length, (int)atype);
            memset(&f->pilha[++f->topo], 0, sizeof(Slot));
            f->pilha[f->topo].longo = (int64_t)(uintptr_t)arr;
            f->pilha[f->topo].tipo  = TIPO_ARRAY;
            break;
        }

        case OP_ANEWARRAY: {
            read_u2(f); /* índice de classe no CP — ignorado */
            int32_t   length = POP_INT(f);
            JVMArray *arr    = aloca_array(length, T_OBJECT);
            memset(&f->pilha[++f->topo], 0, sizeof(Slot));
            f->pilha[f->topo].longo = (int64_t)(uintptr_t)arr;
            f->pilha[f->topo].tipo  = TIPO_ARRAY;
            break;
        }

        case OP_ARRAYLENGTH: {
            const JVMArray *arr = ARR(f->pilha[f->topo--]);
            PUSH_INT(f, arr->length);
            break;
        }

        /* ── ldc / ldc_w / ldc2_w ─────────────────────── */
        case OP_LDC: {
            u1 idx = READ_U1(f);
            if (cf) ldc_push(f, cf->constant_pool, (u2)idx);
            else    PUSH_INT(f, 0);
            break;
        }

        case OP_LDC_W: {
            u2 idx = read_u2(f);
            if (cf) ldc_push(f, cf->constant_pool, idx);
            else    PUSH_INT(f, 0);
            break;
        }

        case OP_LDC2_W: {
            u2 idx = read_u2(f);
            if (cf) ldc2_push(f, cf->constant_pool, idx);
            else    PUSH_INT(f, 0);
            break;
        }

        /* ── aritmética inteira ────────────────────────── */
        case OP_IADD: {
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, v1 + v2);
            break;
        }

        case OP_ISUB: {
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, v1 - v2);
            break;
        }

        case OP_IMUL: {
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, v1 * v2);
            break;
        }

        case OP_IDIV: {
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            if (v2 == 0) { fprintf(stderr, "ArithmeticException: / by zero\n"); return; }
            PUSH_INT(f, v1 / v2);
            break;
        }

        case OP_IREM: {
            int32_t v2 = POP_INT(f);
            int32_t v1 = POP_INT(f);
            if (v2 == 0) { fprintf(stderr, "ArithmeticException: / by zero (irem)\n"); return; }
            PUSH_INT(f, v1 % v2);
            break;
        }

        case OP_INEG: {
            int32_t v1 = POP_INT(f);
            PUSH_INT(f, -v1);
            break;
        }

        /* ── aritmética long ───────────────────────────── */
        case OP_LADD: { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a+b); break; }
        case OP_LSUB: { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a-b); break; }
        case OP_LMUL: { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a*b); break; }
        case OP_LDIV: {
            int64_t b = POP_LONG(f), a = POP_LONG(f);
            if (b == 0) { fprintf(stderr, "ArithmeticException: / by zero (long)\n"); return; }
            PUSH_LONG(f, a / b); break;
        }
        case OP_LREM: {
            int64_t b = POP_LONG(f), a = POP_LONG(f);
            if (b == 0) { fprintf(stderr, "ArithmeticException: / by zero (lrem)\n"); return; }
            PUSH_LONG(f, a % b); break;
        }
        case OP_LNEG: { int64_t v=POP_LONG(f); PUSH_LONG(f, -v); break; }

        /* ── aritmética float ──────────────────────────── */
        case OP_FADD: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, a+b); break; }
        case OP_FSUB: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, a-b); break; }
        case OP_FMUL: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, a*b); break; }
        case OP_FDIV: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, a/b); break; }
        case OP_FREM: { float b=POP_FLOAT(f), a=POP_FLOAT(f); PUSH_FLOAT(f, fmodf(a,b)); break; }
        case OP_FNEG: { float v=POP_FLOAT(f); PUSH_FLOAT(f, -v); break; }

        /* ── aritmética double ─────────────────────────── */
        case OP_DADD: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, a+b); break; }
        case OP_DSUB: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, a-b); break; }
        case OP_DMUL: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, a*b); break; }
        case OP_DDIV: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, a/b); break; }
        case OP_DREM: { double b=POP_DOUBLE(f), a=POP_DOUBLE(f); PUSH_DOUBLE(f, fmod(a,b)); break; }
        case OP_DNEG: { double v=POP_DOUBLE(f); PUSH_DOUBLE(f, -v); break; }

        /* ── bitwise inteiro ───────────────────────────── */
        case OP_ISHL:  { int32_t b=POP_INT(f)&0x1F, a=POP_INT(f); PUSH_INT(f, a<<b); break; }
        case OP_ISHR:  { int32_t b=POP_INT(f)&0x1F, a=POP_INT(f); PUSH_INT(f, a>>b); break; }
        case OP_IUSHR: { int32_t b=POP_INT(f)&0x1F; uint32_t a=(uint32_t)POP_INT(f); PUSH_INT(f, (int32_t)(a>>b)); break; }
        case OP_IAND:  { int32_t b=POP_INT(f), a=POP_INT(f); PUSH_INT(f, a&b); break; }
        case OP_IOR:   { int32_t b=POP_INT(f), a=POP_INT(f); PUSH_INT(f, a|b); break; }
        case OP_IXOR:  { int32_t b=POP_INT(f), a=POP_INT(f); PUSH_INT(f, a^b); break; }

        /* ── bitwise long ──────────────────────────────── */
        case OP_LSHL:  { int32_t b=POP_INT(f)&0x3F; int64_t a=POP_LONG(f); PUSH_LONG(f, a<<b); break; }
        case OP_LSHR:  { int32_t b=POP_INT(f)&0x3F; int64_t a=POP_LONG(f); PUSH_LONG(f, a>>b); break; }
        case OP_LUSHR: { int32_t b=POP_INT(f)&0x3F; uint64_t a=(uint64_t)POP_LONG(f); PUSH_LONG(f, (int64_t)(a>>b)); break; }
        case OP_LAND:  { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a&b); break; }
        case OP_LOR:   { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a|b); break; }
        case OP_LXOR:  { int64_t b=POP_LONG(f), a=POP_LONG(f); PUSH_LONG(f, a^b); break; }

        /* ── variáveis locais ──────────────────────────── */
        case OP_ILOAD_0: PUSH_INT(f, f->locais[0].inteiro); break;
        case OP_ILOAD_1: PUSH_INT(f, f->locais[1].inteiro); break;
        case OP_ILOAD_2: PUSH_INT(f, f->locais[2].inteiro); break;
        case OP_ILOAD_3: PUSH_INT(f, f->locais[3].inteiro); break;

        case OP_ISTORE_0: {
            f->locais[0].inteiro = POP_INT(f);
            f->locais[0].tipo    = TIPO_INT;
            break;
        }
        case OP_ISTORE_1: {
            f->locais[1].inteiro = POP_INT(f);
            f->locais[1].tipo    = TIPO_INT;
            break;
        }
        case OP_ISTORE_2: {
            f->locais[2].inteiro = POP_INT(f);
            f->locais[2].tipo    = TIPO_INT;
            break;
        }
        case OP_ISTORE_3: {
            f->locais[3].inteiro = POP_INT(f);
            f->locais[3].tipo    = TIPO_INT;
            break;
        }

        case OP_IINC: {
            u1      index = READ_U1(f);
            int8_t  cst   = (int8_t)READ_U1(f);
            f->locais[index].inteiro += (int32_t)cst;
            break;
        }

        /* ── conversões de tipo ────────────────────────── */
        case OP_I2L: { int32_t v=POP_INT(f); PUSH_LONG(f, (int64_t)v); break; }
        case OP_I2F: { int32_t v=POP_INT(f); PUSH_FLOAT(f, (float)v); break; }
        case OP_I2D: { int32_t v=POP_INT(f); PUSH_DOUBLE(f, (double)v); break; }
        case OP_L2I: { int64_t v=POP_LONG(f); PUSH_INT(f, (int32_t)v); break; }
        case OP_L2F: { int64_t v=POP_LONG(f); PUSH_FLOAT(f, (float)v); break; }
        case OP_L2D: { int64_t v=POP_LONG(f); PUSH_DOUBLE(f, (double)v); break; }
        case OP_F2I: { float v=POP_FLOAT(f); PUSH_INT(f, isnan(v)?0:(int32_t)v); break; }
        case OP_F2L: { float v=POP_FLOAT(f); PUSH_LONG(f, isnan(v)?0LL:(int64_t)v); break; }
        case OP_F2D: { float v=POP_FLOAT(f); PUSH_DOUBLE(f, (double)v); break; }
        case OP_D2I: { double v=POP_DOUBLE(f); PUSH_INT(f, isnan(v)?0:(int32_t)v); break; }
        case OP_D2L: { double v=POP_DOUBLE(f); PUSH_LONG(f, isnan(v)?0LL:(int64_t)v); break; }
        case OP_D2F: { double v=POP_DOUBLE(f); PUSH_FLOAT(f, (float)v); break; }
        case OP_I2B: { int32_t v=POP_INT(f); PUSH_INT(f, (int32_t)(int8_t)v); break; }
        case OP_I2C: { int32_t v=POP_INT(f); PUSH_INT(f, v & 0xFFFF); break; }
        case OP_I2S: { int32_t v=POP_INT(f); PUSH_INT(f, (int32_t)(int16_t)v); break; }

        /* ── comparações numéricas ─────────────────────── */
        case OP_LCMP: {
            int64_t b=POP_LONG(f), a=POP_LONG(f);
            PUSH_INT(f, a>b ? 1 : a<b ? -1 : 0);
            break;
        }
        case OP_FCMPL: { /* NaN → -1 */
            float b=POP_FLOAT(f), a=POP_FLOAT(f);
            PUSH_INT(f, (isnan(a)||isnan(b)) ? -1 : a>b ? 1 : a<b ? -1 : 0);
            break;
        }
        case OP_FCMPG: { /* NaN → 1 */
            float b=POP_FLOAT(f), a=POP_FLOAT(f);
            PUSH_INT(f, (isnan(a)||isnan(b)) ? 1 : a>b ? 1 : a<b ? -1 : 0);
            break;
        }
        case OP_DCMPL: { /* NaN → -1 */
            double b=POP_DOUBLE(f), a=POP_DOUBLE(f);
            PUSH_INT(f, (isnan(a)||isnan(b)) ? -1 : a>b ? 1 : a<b ? -1 : 0);
            break;
        }
        case OP_DCMPG: { /* NaN → 1 */
            double b=POP_DOUBLE(f), a=POP_DOUBLE(f);
            PUSH_INT(f, (isnan(a)||isnan(b)) ? 1 : a>b ? 1 : a<b ? -1 : 0);
            break;
        }

        /* ── manipulação de pilha ─────────────────────── */
        case OP_POP:
            f->topo--;
            break;

        case OP_POP2:
            f->topo -= 2;
            break;

        case OP_DUP: {
            f->pilha[f->topo + 1] = f->pilha[f->topo];
            f->topo++;
            break;
        }

        case OP_DUP_X1: { /* ..v2,v1 → ..v1,v2,v1 */
            Slot v1 = f->pilha[f->topo];
            f->pilha[f->topo + 1] = f->pilha[f->topo];
            f->pilha[f->topo]     = f->pilha[f->topo - 1];
            f->pilha[f->topo - 1] = v1;
            f->topo++;
            break;
        }

        case OP_DUP_X2: { /* ..v3,v2,v1 → ..v1,v3,v2,v1 */
            Slot v1 = f->pilha[f->topo];
            f->pilha[f->topo + 1] = f->pilha[f->topo];
            f->pilha[f->topo]     = f->pilha[f->topo - 1];
            f->pilha[f->topo - 1] = f->pilha[f->topo - 2];
            f->pilha[f->topo - 2] = v1;
            f->topo++;
            break;
        }

        case OP_DUP2: { /* ..v2,v1 → ..v2,v1,v2,v1 */
            f->pilha[f->topo + 1] = f->pilha[f->topo - 1];
            f->pilha[f->topo + 2] = f->pilha[f->topo];
            f->topo += 2;
            break;
        }

        case OP_DUP2_X1: { /* ..v3,v2,v1 → ..v2,v1,v3,v2,v1 */
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

        case OP_DUP2_X2: { /* ..v4,v3,v2,v1 → ..v2,v1,v4,v3,v2,v1 */
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

        case OP_SWAP: {
            Slot tmp              = f->pilha[f->topo];
            f->pilha[f->topo]     = f->pilha[f->topo - 1];
            f->pilha[f->topo - 1] = tmp;
            break;
        }

                /* ── objetos ───────────────────────────────────── */
        case OP_NEW: {
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

        case OP_GETFIELD: {
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

        case OP_PUTFIELD: {
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
        case OP_GETSTATIC: {
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

        case OP_PUTSTATIC: {
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

        case OP_INVOKEVIRTUAL: {
            u2 idx = read_u2(f);
            if (!cf) { return; }
            execInvokevirtual(f, cf, idx);
            break;
        }

        case OP_INVOKESPECIAL: {
            u2 idx = read_u2(f);
            if (!cf) { return; }
            execInvokespecial(f, cf, idx);
            break;
        }

        case OP_INVOKESTATIC: {
            u2 idx = read_u2(f);
            if (!cf) { return; }
            execInvokestatic(f, cf, idx);
            break;
        }

        case OP_INVOKEINTERFACE: {
            u2 idx = read_u2(f);
            READ_U1(f); /* count */
            READ_U1(f); /* 0 */
            if (!cf) { return; }
            execInvokevirtual(f, cf, idx);
            break;
        }

        case OP_ATHROW: {
            fprintf(stderr, "athrow: excecao lancada (nao suportada)\n");
            return;
        }

        case OP_CHECKCAST: { /* sem verificação real */
            read_u2(f);
            /* mantém o ref no topo */
            break;
        }

        case OP_INSTANCEOF: {
            read_u2(f);
            Slot s = f->pilha[f->topo--];
            PUSH_INT(f, (s.ref != NULL || s.tipo == TIPO_OBJECT) ? 1 : 0);
            break;
        }

        case OP_MONITORENTER:
        case OP_MONITOREXIT:
            f->topo--;
            break;

        case OP_WIDE: {
            u1 next = READ_U1(f);
            u2 idx  = read_u2(f);
            if (next == OP_IINC) { /* wide iinc */
                int16_t cst = read_i2(f);
                f->locais[idx].inteiro += (int32_t)cst;
            } else {
                switch (next) {
                    case OP_ILOAD: PUSH_INT(f, f->locais[idx].inteiro); break;
                    case OP_LLOAD: case OP_FLOAD: case OP_DLOAD: case OP_ALOAD:
                        f->pilha[++f->topo] = f->locais[idx]; break;
                    case OP_ISTORE: f->locais[idx].inteiro = POP_INT(f); f->locais[idx].tipo = TIPO_INT; break;
                    case OP_LSTORE: case OP_FSTORE: case OP_DSTORE: case OP_ASTORE:
                        f->locais[idx] = f->pilha[f->topo--]; break;
                    default:
                        fprintf(stderr, "wide: opcode 0x%02X nao suportado\n", next);
                        break;
                }
            }
            break;
        }

        case OP_MULTIANEWARRAY: {
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

        case OP_IFEQ: BRANCH_IF1(v == 0)
        case OP_IFNE: BRANCH_IF1(v != 0)
        case OP_IFLT: BRANCH_IF1(v <  0)
        case OP_IFGE: BRANCH_IF1(v >= 0)
        case OP_IFGT: BRANCH_IF1(v >  0)
        case OP_IFLE: BRANCH_IF1(v <= 0)
#undef BRANCH_IF1

        /* ── desvios binários (compara dois ints) ──────────────── */
#define BRANCH_IF2(cond) \
    { int16_t o = read_i2(f); int32_t v2 = POP_INT(f); int32_t v1 = POP_INT(f); \
      if (cond) { f->pc = instr_pc + (u4)(int32_t)o; } break; }

        case OP_IF_ICMPEQ: BRANCH_IF2(v1 == v2)
        case OP_IF_ICMPNE: BRANCH_IF2(v1 != v2)
        case OP_IF_ICMPLT: BRANCH_IF2(v1 <  v2)
        case OP_IF_ICMPGE: BRANCH_IF2(v1 >= v2)
        case OP_IF_ICMPGT: BRANCH_IF2(v1 >  v2)
        case OP_IF_ICMPLE: BRANCH_IF2(v1 <= v2)
#undef BRANCH_IF2

        case OP_IF_ACMPEQ: {
            int16_t o = read_i2(f);
            void *s2 = f->pilha[f->topo--].ref;
            void *s1 = f->pilha[f->topo--].ref;
            if (s1 == s2) f->pc = instr_pc + (u4)(int32_t)o;
            break;
        }
        case OP_IF_ACMPNE: {
            int16_t o = read_i2(f);
            void *s2 = f->pilha[f->topo--].ref;
            void *s1 = f->pilha[f->topo--].ref;
            if (s1 != s2) f->pc = instr_pc + (u4)(int32_t)o;
            break;
        }

        case OP_GOTO: {
            int16_t offset = read_i2(f);
            f->pc = instr_pc + (u4)(int32_t)offset;
            break;
        }

        case OP_TABLESWITCH: {
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

        case OP_LOOKUPSWITCH: {
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

        case OP_IFNULL: {
            int16_t o = read_i2(f);
            Slot s = f->pilha[f->topo--];
            if (s.ref == NULL && s.longo == 0) f->pc = instr_pc + (u4)(int32_t)o;
            break;
        }
        case OP_IFNONNULL: {
            int16_t o = read_i2(f);
            Slot s = f->pilha[f->topo--];
            if (s.ref != NULL || s.longo != 0) f->pc = instr_pc + (u4)(int32_t)o;
            break;
        }

        case OP_GOTO_W: {
            int32_t off = (int32_t)(((u4)READ_U1(f)<<24)|((u4)READ_U1(f)<<16)|((u4)READ_U1(f)<<8)|(u4)READ_U1(f));
            f->pc = instr_pc + (u4)off;
            break;
        }

        /* ── retorno ───────────────────────────────────── */
        case OP_IRETURN:
        case OP_LRETURN:
        case OP_FRETURN:
        case OP_DRETURN:
        case OP_ARETURN:
        case OP_RETURN:
            return;

        default:
            fprintf(stderr, "executaFrame: opcode 0x%02X nao implementado (pc=%u)\n",
                    op, instr_pc);
            return;
        }
    }
}
