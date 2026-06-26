#ifndef METHOD_INVOKE_H
#define METHOD_INVOKE_H

#include "basic_ops.h"
#include "attribute.h"
#include "class_file/dot_class.h"

/* Utilitários de descriptor */
int contaArgs(const char *descriptor);
int eVoid(const char *descriptor);

/* Busca no ClassFile */
Code_attribute *encontraCode(method_info *m, cp_info *cp);
method_info    *encontraMetodo(ClassFile *cf, const char *name, const char *desc);

/* Handlers de opcode (responsabilidade do usuário) */
void execInvokestatic(Frame *caller, ClassFile *cf, u2 idx);
void execInvokevirtual(Frame *caller, ClassFile *cf, u2 idx);

#endif
