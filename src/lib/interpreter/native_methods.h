#ifndef NATIVE_METHODS_H
#define NATIVE_METHODS_H

#include "basic_ops.h"

/*
 * Assinatura de um método nativo.
 * A função é responsável por desempilhar os argumentos (e o objectref para
 * métodos de instância) de caller->pilha, e empilhar o valor de retorno
 * quando aplicável.
 *
 * n_args: número de argumentos do descritor (sem contar objectref).
 */
typedef void (*NativeFn)(Frame *caller, int n_args);

/* Inicializa o registro com todos os métodos nativos suportados.
   Deve ser chamada uma vez no início de executaJVM(). */
void     initNativeMethods(void);

/* Retorna o handler nativo para (class_name, method_name, descriptor),
   ou NULL se não houver registro. */
NativeFn lookupNative(const char *class_name,
                      const char *method_name,
                      const char *descriptor);

/* Retorna 1 se class_name é uma classe tratada nativamente.
   Usado por `new` para não copiar campos do ClassFile do usuário. */
int isNativeClass(const char *class_name);

#endif
