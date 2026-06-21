#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/interpreter/native_methods.h"
#include "lib/interpreter/basic_ops.h"
#include "lib/interpreter/heap.h"
#include "lib/interpreter/interpreter.h"
#include "lib/types/attribute.h"
#include "lib/types/consts.h"
#include "lib/class_loader/loader.h"
#include "lib/class_loader/fields_interfaces.h"
#include "lib/class_loader/methods.h"

static int tests_run    = 0;
static int tests_passed = 0;

#define RUN_TEST(name) do { \
    printf("  [TEST] " #name "... "); \
    name(); \
    tests_run++; \
    tests_passed++; \
    printf("PASS\n"); \
} while (0)


/* ── helpers ─────────────────────────────────────────────────────────── */

static Frame *make_frame(void) {
    static u1 ret[] = {0xB1};
    static Code_attribute ca;
    ca.code = ret; ca.code_length = 1;
    return criaFrame(&ca);
}

static char *capture_stdout_fn(void (*fn)(Frame *, int), Frame *f, int n) {
    int pfd[2]; pipe(pfd);
    int saved = dup(STDOUT_FILENO);
    fflush(stdout);
    dup2(pfd[1], STDOUT_FILENO); close(pfd[1]);
    fn(f, n);
    fflush(stdout);
    dup2(saved, STDOUT_FILENO); close(saved);
    char *buf = malloc(4096);
    ssize_t n2 = read(pfd[0], buf, 4095);
    buf[n2 > 0 ? n2 : 0] = '\0';
    close(pfd[0]);
    return buf;
}

static char *capture_jvm(ClassFile *cf) {
    int pfd[2]; pipe(pfd);
    int saved = dup(STDOUT_FILENO);
    fflush(stdout);
    dup2(pfd[1], STDOUT_FILENO); close(pfd[1]);
    executaJVM(cf);
    fflush(stdout);
    dup2(saved, STDOUT_FILENO); close(saved);
    char *buf = malloc(8192);
    ssize_t n = read(pfd[0], buf, 8191);
    buf[n > 0 ? n : 0] = '\0';
    close(pfd[0]);
    return buf;
}


/* ── testes do registry ─────────────────────────────────────────────── */

static void test_lookup_println_int(void) {
    initNativeMethods();
    NativeFn fn = lookupNative("java/io/PrintStream", "println", "(I)V");
    assert(fn != NULL);
}

static void test_lookup_print_string(void) {
    NativeFn fn = lookupNative("java/io/PrintStream", "print", "(Ljava/lang/String;)V");
    assert(fn != NULL);
}

static void test_lookup_sb_append(void) {
    NativeFn fn = lookupNative("java/lang/StringBuilder", "append",
                               "(I)Ljava/lang/StringBuilder;");
    assert(fn != NULL);
}

static void test_lookup_miss(void) {
    NativeFn fn = lookupNative("com/example/Foo", "bar", "()V");
    assert(fn == NULL);
}

static void test_is_native_class(void) {
    assert(isNativeClass("java/lang/StringBuilder") == 1);
    assert(isNativeClass("java/io/PrintStream")     == 1);
    assert(isNativeClass("com/example/Foo")         == 0);
}


/* ── testes de PrintStream.print ──────────────────────────────────────── */

static void test_print_int(void) {
    Frame *f = make_frame();
    /* pilha: [PrintStream_ref, 42] */
    f->pilha[0].longo = 0; f->pilha[0].tipo = TIPO_REF;   /* objectref */
    f->pilha[1].inteiro = 42; f->pilha[1].tipo = TIPO_INT;
    f->topo = 1;

    NativeFn fn = lookupNative("java/io/PrintStream", "print", "(I)V");
    char *out = capture_stdout_fn(fn, f, 1);
    assert(strcmp(out, "42") == 0);
    assert(f->topo == -1);
    free(out);
    liberaFrame(f);
}

static void test_print_string(void) {
    Frame *f = make_frame();
    const char *s = "ola";
    f->pilha[0].longo = 0; f->pilha[0].tipo = TIPO_REF;
    f->pilha[1].longo = (int64_t)(uintptr_t)s; f->pilha[1].tipo = TIPO_REF;
    f->topo = 1;

    NativeFn fn = lookupNative("java/io/PrintStream", "print", "(Ljava/lang/String;)V");
    char *out = capture_stdout_fn(fn, f, 1);
    assert(strcmp(out, "ola") == 0);
    assert(f->topo == -1);
    free(out);
    liberaFrame(f);
}

static void test_println_void(void) {
    Frame *f = make_frame();
    f->pilha[0].longo = 0; f->pilha[0].tipo = TIPO_REF;
    f->topo = 0;

    NativeFn fn = lookupNative("java/io/PrintStream", "println", "()V");
    char *out = capture_stdout_fn(fn, f, 0);
    assert(strcmp(out, "\n") == 0);
    assert(f->topo == -1);
    free(out);
    liberaFrame(f);
}

static void test_println_bool(void) {
    Frame *f = make_frame();
    f->pilha[0].longo = 0; f->pilha[0].tipo = TIPO_REF;
    f->pilha[1].inteiro = 1; f->pilha[1].tipo = TIPO_INT;
    f->topo = 1;

    NativeFn fn = lookupNative("java/io/PrintStream", "println", "(Z)V");
    char *out = capture_stdout_fn(fn, f, 1);
    assert(strcmp(out, "true\n") == 0);
    free(out);
    liberaFrame(f);
}


/* ── testes de StringBuilder ─────────────────────────────────────────── */

static void test_sb_append_int_tostring(void) {
    /* Simula: new StringBuilder, <init>, append(42), toString */
    HeapObject *sb = alocaObjeto(NULL, "java/lang/StringBuilder");
    Slot sb_slot; sb_slot.longo = (int64_t)(uintptr_t)sb; sb_slot.tipo = TIPO_OBJECT;

    Frame *f = make_frame();

    /* <init>: stack [sb_ref] */
    f->pilha[0] = sb_slot; f->topo = 0;
    NativeFn init = lookupNative("java/lang/StringBuilder", "<init>", "()V");
    init(f, 0);
    assert(f->topo == -1);
    assert(sb->native_data != NULL);

    /* append(42): stack [sb_ref, 42] */
    f->pilha[0] = sb_slot;
    f->pilha[1].inteiro = 42; f->pilha[1].tipo = TIPO_INT;
    f->topo = 1;
    NativeFn app = lookupNative("java/lang/StringBuilder", "append",
                                "(I)Ljava/lang/StringBuilder;");
    app(f, 1);
    assert(f->topo == 0); /* this voltou para o topo */

    /* toString: stack [sb_ref] */
    NativeFn tos = lookupNative("java/lang/StringBuilder", "toString",
                                "()Ljava/lang/String;");
    tos(f, 0);
    const char *result = (char *)(uintptr_t)f->pilha[f->topo].longo;
    assert(strcmp(result, "42") == 0);

    liberaFrame(f);
    liberaHeap();
}

static void test_sb_append_strings(void) {
    HeapObject *sb = alocaObjeto(NULL, "java/lang/StringBuilder");
    Slot sb_slot; sb_slot.longo = (int64_t)(uintptr_t)sb; sb_slot.tipo = TIPO_OBJECT;

    Frame *f = make_frame();
    NativeFn init = lookupNative("java/lang/StringBuilder", "<init>", "()V");
    NativeFn app  = lookupNative("java/lang/StringBuilder", "append",
                                 "(Ljava/lang/String;)Ljava/lang/StringBuilder;");
    NativeFn tos  = lookupNative("java/lang/StringBuilder", "toString",
                                 "()Ljava/lang/String;");

    /* init */
    f->pilha[0] = sb_slot; f->topo = 0; init(f, 0);

    /* append("abc") */
    const char *s1 = "abc";
    f->pilha[0] = sb_slot;
    f->pilha[1].longo = (int64_t)(uintptr_t)s1; f->pilha[1].tipo = TIPO_REF;
    f->topo = 1;
    app(f, 1);

    /* append("def") */
    const char *s2 = "def";
    /* this já está no topo (f->pilha[0]) */
    f->pilha[1].longo = (int64_t)(uintptr_t)s2; f->pilha[1].tipo = TIPO_REF;
    f->topo = 1;
    app(f, 1);

    /* toString */
    tos(f, 0);
    const char *result = (char *)(uintptr_t)f->pilha[f->topo].longo;
    assert(strcmp(result, "abcdef") == 0);

    liberaFrame(f);
    liberaHeap();
}


/* ── testes de integração com .class ─────────────────────────────────── */

static void test_fibonacci_rec(void) {
    /* FibonacciRec.class usa print(Ljava/lang/String;)V e print(I)V */
    FILE *fp = fopen("data/examples/FibonacciRec.class", "rb");
    assert(fp != NULL);
    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, fp); readInterfaces(cf, fp);
    readFields(cf, fp); readMethodsCount(cf, fp);
    readMethods(cf, fp); readClassFileAttributes(cf, fp);
    fclose(fp);

    char *out = capture_jvm(cf);
    /* FibonacciRec imprime sequência de Fibonacci — verifica que não há erro */
    assert(strstr(out, "nao encontrado") == NULL);
    assert(strlen(out) > 0);
    free(out);
    freeClassFile(cf);
}

static void test_regressao_soma(void) {
    FILE *fp = fopen("data/examples/Soma.class", "rb");
    assert(fp != NULL);
    ClassFile *cf = allocClassFile();
    classFilesSetup(cf, fp); readInterfaces(cf, fp);
    readFields(cf, fp); readMethodsCount(cf, fp);
    readMethods(cf, fp); readClassFileAttributes(cf, fp);
    fclose(fp);

    char *out = capture_jvm(cf);
    assert(strstr(out, "30") != NULL);
    free(out);
    freeClassFile(cf);
}


int main(void) {
    printf("=== native methods unit tests ===\n");

    printf("\n-- registry --\n");
    RUN_TEST(test_lookup_println_int);
    RUN_TEST(test_lookup_print_string);
    RUN_TEST(test_lookup_sb_append);
    RUN_TEST(test_lookup_miss);
    RUN_TEST(test_is_native_class);

    printf("\n-- PrintStream.print / println --\n");
    RUN_TEST(test_print_int);
    RUN_TEST(test_print_string);
    RUN_TEST(test_println_void);
    RUN_TEST(test_println_bool);

    printf("\n-- StringBuilder --\n");
    RUN_TEST(test_sb_append_int_tostring);
    RUN_TEST(test_sb_append_strings);

    printf("\n-- integração .class --\n");
    RUN_TEST(test_fibonacci_rec);
    RUN_TEST(test_regressao_soma);

    printf("\nResult: %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
