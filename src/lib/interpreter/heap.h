#ifndef HEAP_H
#define HEAP_H

#include "basic_ops.h"
#include "class_file/dot_class.h"

typedef struct {
    char  *class_name;
    u2     num_fields;
    char **field_names;
    Slot  *fields;
    void  *native_data;  /* buffer para classes nativas (ex: StringBuilder) */
} HeapObject;

HeapObject *alocaObjeto(ClassFile *cf, const char *class_name);
int         buscaCampo(const HeapObject *obj, const char *name);
void        liberaHeap(void);

#endif
