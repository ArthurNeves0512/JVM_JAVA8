#define _POSIX_C_SOURCE 200809L
#include "native_methods.h"
#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ── registro ──────────────────────────────────────────────────────── */

typedef struct {
    const char *class_name;
    const char *method_name;
    const char *descriptor;
    NativeFn    fn;
} NativeEntry;

#define MAX_NATIVES 128
static NativeEntry native_table[MAX_NATIVES];
static int         native_count = 0;

static void reg(const char *cn, const char *mn, const char *desc, NativeFn fn) {
    if (native_count >= MAX_NATIVES) return;
    native_table[native_count].class_name  = cn;
    native_table[native_count].method_name = mn;
    native_table[native_count].descriptor  = desc;
    native_table[native_count].fn          = fn;
    native_count++;
}

NativeFn lookupNative(const char *class_name,
                      const char *method_name,
                      const char *descriptor) {
    for (int i = 0; i < native_count; i++) {
        if (strcmp(native_table[i].class_name,  class_name)  == 0 &&
            strcmp(native_table[i].method_name, method_name) == 0 &&
            strcmp(native_table[i].descriptor,  descriptor)  == 0)
            return native_table[i].fn;
    }
    return NULL;
}

static const char *native_class_list[] = {
    "java/lang/StringBuilder",
    "java/lang/String",
    "java/io/PrintStream",
    NULL
};

int isNativeClass(const char *class_name) {
    for (int i = 0; native_class_list[i]; i++)
        if (strcmp(native_class_list[i], class_name) == 0) return 1;
    return 0;
}


/* ── helpers ────────────────────────────────────────────────────────── */

static const char *slot_as_string(Slot s) {
    if (s.tipo == TIPO_REF || s.tipo == TIPO_OBJECT)
        return (const char *)(uintptr_t)s.longo;
    return "null";
}


/* ── PrintStream.print ──────────────────────────────────────────────── */

static void native_print_int(Frame *f, int n) {
    (void)n;
    int32_t val = f->pilha[f->topo--].inteiro;
    f->topo--;  /* objectref */
    printf("%d", val);
}

static void native_print_string(Frame *f, int n) {
    (void)n;
    const char *s = slot_as_string(f->pilha[f->topo--]);
    f->topo--;
    printf("%s", s ? s : "null");
}

static void native_print_bool(Frame *f, int n) {
    (void)n;
    int32_t val = f->pilha[f->topo--].inteiro;
    f->topo--;
    printf("%s", val ? "true" : "false");
}

static void native_print_char(Frame *f, int n) {
    (void)n;
    int32_t val = f->pilha[f->topo--].inteiro;
    f->topo--;
    printf("%c", (char)val);
}

static void native_print_long(Frame *f, int n) {
    (void)n;
    int64_t val = f->pilha[f->topo--].longo;
    f->topo--;
    printf("%lld", (long long)val);
}

static void native_print_float(Frame *f, int n) {
    (void)n;
    float val = f->pilha[f->topo--].flutuante;
    f->topo--;
    printf("%g", (double)val);
}

static void native_print_double(Frame *f, int n) {
    (void)n;
    double val = f->pilha[f->topo--].duplo;
    f->topo--;
    printf("%g", val);
}

static void native_print_object(Frame *f, int n) {
    (void)n;
    const char *s = slot_as_string(f->pilha[f->topo--]);
    f->topo--;
    printf("%s", s ? s : "null");
}


/* ── PrintStream.println ────────────────────────────────────────────── */

static void native_println_void(Frame *f, int n) {
    (void)n;
    f->topo--;  /* objectref */
    printf("\n");
}

static void native_println_int(Frame *f, int n) {
    (void)n;
    int32_t val = f->pilha[f->topo--].inteiro;
    f->topo--;
    printf("%d\n", val);
}

static void native_println_string(Frame *f, int n) {
    (void)n;
    const char *s = slot_as_string(f->pilha[f->topo--]);
    f->topo--;
    printf("%s\n", s ? s : "null");
}

static void native_println_bool(Frame *f, int n) {
    (void)n;
    int32_t val = f->pilha[f->topo--].inteiro;
    f->topo--;
    printf("%s\n", val ? "true" : "false");
}

static void native_println_char(Frame *f, int n) {
    (void)n;
    int32_t val = f->pilha[f->topo--].inteiro;
    f->topo--;
    printf("%c\n", (char)val);
}

static void native_println_long(Frame *f, int n) {
    (void)n;
    int64_t val = f->pilha[f->topo--].longo;
    f->topo--;
    printf("%lld\n", (long long)val);
}

static void native_println_float(Frame *f, int n) {
    (void)n;
    float val = f->pilha[f->topo--].flutuante;
    f->topo--;
    printf("%g\n", (double)val);
}

static void native_println_double(Frame *f, int n) {
    (void)n;
    double val = f->pilha[f->topo--].duplo;
    f->topo--;
    printf("%g\n", val);
}

static void native_println_object(Frame *f, int n) {
    (void)n;
    const char *s = slot_as_string(f->pilha[f->topo--]);
    f->topo--;
    printf("%s\n", s ? s : "null");
}


/* ── StringBuilder ──────────────────────────────────────────────────── */

#define STRBUF_SIZE 4096

static HeapObject *sb_from_slot(Slot s) {
    return (HeapObject *)(uintptr_t)s.longo;
}

static void native_sb_init(Frame *f, int n) {
    (void)n;
    HeapObject *sb = sb_from_slot(f->pilha[f->topo--]);
    if (sb && !sb->native_data)
        sb->native_data = calloc(STRBUF_SIZE, 1);
}

static void native_sb_append_int(Frame *f, int n) {
    (void)n;
    int32_t val      = f->pilha[f->topo--].inteiro;
    Slot    self_slot = f->pilha[f->topo--];
    HeapObject *sb   = sb_from_slot(self_slot);
    if (sb && sb->native_data) {
        char *buf = (char *)sb->native_data;
        size_t len = strlen(buf);
        snprintf(buf + len, STRBUF_SIZE - len, "%d", val);
    }
    f->pilha[++f->topo] = self_slot;  /* push this */
}

static void native_sb_append_long(Frame *f, int n) {
    (void)n;
    int64_t val      = f->pilha[f->topo--].longo;
    Slot    self_slot = f->pilha[f->topo--];
    HeapObject *sb   = sb_from_slot(self_slot);
    if (sb && sb->native_data) {
        char *buf = (char *)sb->native_data;
        size_t len = strlen(buf);
        snprintf(buf + len, STRBUF_SIZE - len, "%lld", (long long)val);
    }
    f->pilha[++f->topo] = self_slot;
}

static void native_sb_append_float(Frame *f, int n) {
    (void)n;
    float   val      = f->pilha[f->topo--].flutuante;
    Slot    self_slot = f->pilha[f->topo--];
    HeapObject *sb   = sb_from_slot(self_slot);
    if (sb && sb->native_data) {
        char *buf = (char *)sb->native_data;
        size_t len = strlen(buf);
        snprintf(buf + len, STRBUF_SIZE - len, "%g", (double)val);
    }
    f->pilha[++f->topo] = self_slot;
}

static void native_sb_append_double(Frame *f, int n) {
    (void)n;
    double  val      = f->pilha[f->topo--].duplo;
    Slot    self_slot = f->pilha[f->topo--];
    HeapObject *sb   = sb_from_slot(self_slot);
    if (sb && sb->native_data) {
        char *buf = (char *)sb->native_data;
        size_t len = strlen(buf);
        snprintf(buf + len, STRBUF_SIZE - len, "%g", val);
    }
    f->pilha[++f->topo] = self_slot;
}

static void native_sb_append_bool(Frame *f, int n) {
    (void)n;
    int32_t val      = f->pilha[f->topo--].inteiro;
    Slot    self_slot = f->pilha[f->topo--];
    HeapObject *sb   = sb_from_slot(self_slot);
    if (sb && sb->native_data) {
        char *buf = (char *)sb->native_data;
        size_t len = strlen(buf);
        snprintf(buf + len, STRBUF_SIZE - len, "%s", val ? "true" : "false");
    }
    f->pilha[++f->topo] = self_slot;
}

static void native_sb_append_char(Frame *f, int n) {
    (void)n;
    int32_t val      = f->pilha[f->topo--].inteiro;
    Slot    self_slot = f->pilha[f->topo--];
    HeapObject *sb   = sb_from_slot(self_slot);
    if (sb && sb->native_data) {
        char *buf = (char *)sb->native_data;
        size_t len = strlen(buf);
        if (len < STRBUF_SIZE - 1) {
            buf[len]     = (char)val;
            buf[len + 1] = '\0';
        }
    }
    f->pilha[++f->topo] = self_slot;
}

static void native_sb_append_string(Frame *f, int n) {
    (void)n;
    const char *s    = slot_as_string(f->pilha[f->topo--]);
    Slot    self_slot = f->pilha[f->topo--];
    HeapObject *sb   = sb_from_slot(self_slot);
    if (sb && sb->native_data && s) {
        char *buf = (char *)sb->native_data;
        size_t len = strlen(buf);
        strncat(buf, s, STRBUF_SIZE - len - 1);
    }
    f->pilha[++f->topo] = self_slot;
}

static void native_sb_append_object(Frame *f, int n) {
    native_sb_append_string(f, n);
}

static void native_sb_tostring(Frame *f, int n) {
    (void)n;
    HeapObject *sb = sb_from_slot(f->pilha[f->topo--]);
    const char *buf = (sb && sb->native_data) ? (char *)sb->native_data : "";
    f->pilha[++f->topo].longo = (int64_t)(uintptr_t)buf;
    f->pilha[f->topo].tipo    = TIPO_REF;
}


/* ── String.valueOf (static) ────────────────────────────────────────── */

#define MAX_VALUEOF 64
static char valueof_pool[MAX_VALUEOF][32];
static int  valueof_next = 0;

static char *next_valueof_buf(void) {
    char *buf = valueof_pool[valueof_next % MAX_VALUEOF];
    valueof_next++;
    return buf;
}

static void native_string_valueof_int(Frame *f, int n) {
    (void)n;
    int32_t val = f->pilha[f->topo--].inteiro;
    char *buf = next_valueof_buf();
    snprintf(buf, 32, "%d", val);
    f->pilha[++f->topo].longo = (int64_t)(uintptr_t)buf;
    f->pilha[f->topo].tipo    = TIPO_REF;
}

static void native_string_valueof_long(Frame *f, int n) {
    (void)n;
    int64_t val = f->pilha[f->topo--].longo;
    char *buf = next_valueof_buf();
    snprintf(buf, 32, "%lld", (long long)val);
    f->pilha[++f->topo].longo = (int64_t)(uintptr_t)buf;
    f->pilha[f->topo].tipo    = TIPO_REF;
}

static void native_string_valueof_float(Frame *f, int n) {
    (void)n;
    float val = f->pilha[f->topo--].flutuante;
    char *buf = next_valueof_buf();
    snprintf(buf, 32, "%g", (double)val);
    f->pilha[++f->topo].longo = (int64_t)(uintptr_t)buf;
    f->pilha[f->topo].tipo    = TIPO_REF;
}

static void native_string_valueof_double(Frame *f, int n) {
    (void)n;
    double val = f->pilha[f->topo--].duplo;
    char *buf = next_valueof_buf();
    snprintf(buf, 32, "%g", val);
    f->pilha[++f->topo].longo = (int64_t)(uintptr_t)buf;
    f->pilha[f->topo].tipo    = TIPO_REF;
}

static void native_string_valueof_bool(Frame *f, int n) {
    (void)n;
    int32_t val = f->pilha[f->topo--].inteiro;
    char *buf = next_valueof_buf();
    snprintf(buf, 32, "%s", val ? "true" : "false");
    f->pilha[++f->topo].longo = (int64_t)(uintptr_t)buf;
    f->pilha[f->topo].tipo    = TIPO_REF;
}

static void native_string_valueof_char(Frame *f, int n) {
    (void)n;
    int32_t val = f->pilha[f->topo--].inteiro;
    char *buf = next_valueof_buf();
    buf[0] = (char)val; buf[1] = '\0';
    f->pilha[++f->topo].longo = (int64_t)(uintptr_t)buf;
    f->pilha[f->topo].tipo    = TIPO_REF;
}

/* ── String.length (instância) ──────────────────────────────────────── */

static void native_string_length(Frame *f, int n) {
    (void)n;
    const char *s = slot_as_string(f->pilha[f->topo--]);
    int32_t len = s ? (int32_t)strlen(s) : 0;
    f->pilha[++f->topo].inteiro = len;
    f->pilha[f->topo].tipo      = TIPO_INT;
}

/* ── Object.<init> genérico ─────────────────────────────────────────── */

static void native_object_init(Frame *f, int n) {
    (void)n;
    f->topo--;  /* descarta objectref */
}


/* ── initNativeMethods ──────────────────────────────────────────────── */

void initNativeMethods(void) {
    if (native_count > 0) return;  /* já inicializado */

    const char *PS  = "java/io/PrintStream";
    const char *SB  = "java/lang/StringBuilder";
    const char *STR = "java/lang/String";
    const char *OBJ = "java/lang/Object";

    /* PrintStream.print */
    reg(PS, "print", "(I)V",                    native_print_int);
    reg(PS, "print", "(Ljava/lang/String;)V",   native_print_string);
    reg(PS, "print", "(Z)V",                    native_print_bool);
    reg(PS, "print", "(C)V",                    native_print_char);
    reg(PS, "print", "(J)V",                    native_print_long);
    reg(PS, "print", "(F)V",                    native_print_float);
    reg(PS, "print", "(D)V",                    native_print_double);
    reg(PS, "print", "(Ljava/lang/Object;)V",   native_print_object);

    /* PrintStream.println */
    reg(PS, "println", "()V",                   native_println_void);
    reg(PS, "println", "(I)V",                  native_println_int);
    reg(PS, "println", "(Ljava/lang/String;)V", native_println_string);
    reg(PS, "println", "(Z)V",                  native_println_bool);
    reg(PS, "println", "(C)V",                  native_println_char);
    reg(PS, "println", "(J)V",                  native_println_long);
    reg(PS, "println", "(F)V",                  native_println_float);
    reg(PS, "println", "(D)V",                  native_println_double);
    reg(PS, "println", "(Ljava/lang/Object;)V", native_println_object);

    /* StringBuilder */
    reg(SB, "<init>",  "()V",                              native_sb_init);
    reg(SB, "append",  "(I)Ljava/lang/StringBuilder;",     native_sb_append_int);
    reg(SB, "append",  "(J)Ljava/lang/StringBuilder;",     native_sb_append_long);
    reg(SB, "append",  "(F)Ljava/lang/StringBuilder;",     native_sb_append_float);
    reg(SB, "append",  "(D)Ljava/lang/StringBuilder;",     native_sb_append_double);
    reg(SB, "append",  "(Z)Ljava/lang/StringBuilder;",     native_sb_append_bool);
    reg(SB, "append",  "(C)Ljava/lang/StringBuilder;",     native_sb_append_char);
    reg(SB, "append",  "(Ljava/lang/String;)Ljava/lang/StringBuilder;",
                                                           native_sb_append_string);
    reg(SB, "append",  "(Ljava/lang/Object;)Ljava/lang/StringBuilder;",
                                                           native_sb_append_object);
    reg(SB, "toString","()Ljava/lang/String;",             native_sb_tostring);

    /* String.valueOf (static) */
    reg(STR, "valueOf", "(I)Ljava/lang/String;", native_string_valueof_int);
    reg(STR, "valueOf", "(J)Ljava/lang/String;", native_string_valueof_long);
    reg(STR, "valueOf", "(F)Ljava/lang/String;", native_string_valueof_float);
    reg(STR, "valueOf", "(D)Ljava/lang/String;", native_string_valueof_double);
    reg(STR, "valueOf", "(Z)Ljava/lang/String;", native_string_valueof_bool);
    reg(STR, "valueOf", "(C)Ljava/lang/String;", native_string_valueof_char);

    /* String instance */
    reg(STR, "length", "()I",  native_string_length);

    /* Object.<init> genérico */
    reg(OBJ, "<init>", "()V",  native_object_init);
}
