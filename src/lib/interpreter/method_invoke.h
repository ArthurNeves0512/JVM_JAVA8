#ifndef METHOD_INVOKE_H
#define METHOD_INVOKE_H

#include "basic_ops.h"
#include "attribute.h"
#include "loader.h"
#include "class_file/dot_class.h"
#include "interpreter/heap.h"

/* Parsing de descritores de método */
int conta_args(const char *descriptor);
int retorno_void(const char *descriptor);

/* Busca de método e Code_attribute */
Code_attribute *encontraCodeAttr(method_info *m, cp_info *cp);
method_info    *buscaMetodoClasse(ClassFile *cf, const char *name, const char *desc, ClassFile **owner);
method_info    *encontraMetodo(ClassFile *cf, const char *name, const char *desc);

/* Handlers de opcode de chamada */
void execInvokestatic(Frame *caller, ClassFile *cf, u2 idx);
void execInvokevirtual(Frame *caller, ClassFile *cf, u2 idx);
void execInvokespecial(Frame *caller, ClassFile *cf, u2 idx);

#endif
