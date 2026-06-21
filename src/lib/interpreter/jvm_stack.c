#include "jvm_stack.h"
#include <stdio.h>
#include <stdlib.h>


JVMStack *criaJVMStack(void) {
    JVMStack *s = malloc(sizeof(JVMStack));
    if (!s) {
        fprintf(stderr, "criaJVMStack: sem memoria\n");
        exit(1);
    }
    s->topo = -1;
    return s;
}


void empilhaFrame(JVMStack *s, Frame *f) {
    if (s->topo >= MAX_FRAMES - 1) {
        fprintf(stderr, "empilhaFrame: stack overflow\n");
        exit(1);
    }
    s->frames[++s->topo] = f;
}


Frame *desempilhaFrame(JVMStack *s) {
    if (s->topo < 0) {
        fprintf(stderr, "desempilhaFrame: stack underflow\n");
        return NULL;
    }
    return s->frames[s->topo--];
}


Frame *frameAtual(JVMStack *s) {
    if (s->topo < 0) return NULL;
    return s->frames[s->topo];
}


void liberaJVMStack(JVMStack *s) {
    while (s->topo >= 0)
        liberaFrame(desempilhaFrame(s));
    free(s);
}
