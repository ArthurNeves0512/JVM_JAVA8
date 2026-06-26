#ifndef STRING_OPS_H
#define STRING_OPS_H

#include "basic_ops.h"
#include "constant_pool.h"

/*
 * Empurra na pilha o valor do constant_pool[idx].
 * Suporta: CONSTANT_Integer, CONSTANT_Float, CONSTANT_String.
 * Strings são representadas como ponteiro (char*) no campo longo do Slot (TIPO_REF).
 */
void execLdc(Frame *f, cp_info *cp, u2 idx);

#endif
