#ifndef JVM_STACK_H
#define JVM_STACK_H

#include "basic_ops.h"

#define MAX_FRAMES 256

typedef struct {
    Frame *frames[MAX_FRAMES];
    int    topo;  /* -1 = vazia */
} JVMStack;

JVMStack *criaJVMStack(void);
void      empilhaFrame(JVMStack *stack, Frame *frame);
Frame    *desempilhaFrame(JVMStack *stack);
Frame    *frameAtual(JVMStack *stack);
void      liberaJVMStack(JVMStack *stack);

#endif
