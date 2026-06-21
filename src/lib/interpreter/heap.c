#define _POSIX_C_SOURCE 200809L
#include "heap.h"
#include "attribute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_OBJECTS 1024

static HeapObject *heap_pool[MAX_OBJECTS];
static int         heap_count = 0;


HeapObject *alocaObjeto(ClassFile *cf, const char *class_name) {
    HeapObject *obj = malloc(sizeof(HeapObject));
    if (!obj) { fprintf(stderr, "alocaObjeto: sem memoria\n"); exit(1); }

    obj->class_name = strdup(class_name);
    obj->num_fields = cf ? cf->fields_count : 0;

    if (obj->num_fields > 0) {
        obj->field_names = malloc(obj->num_fields * sizeof(char *));
        obj->fields      = calloc(obj->num_fields, sizeof(Slot));
        for (u2 i = 0; i < obj->num_fields; i++) {
            obj->field_names[i] = getUtf8(cf->constant_pool,
                                          cf->fields[i].name_index);
            obj->fields[i].tipo    = TIPO_INT;
            obj->fields[i].inteiro = 0;
        }
    } else {
        obj->field_names = NULL;
        obj->fields      = NULL;
    }

    if (heap_count < MAX_OBJECTS)
        heap_pool[heap_count++] = obj;

    return obj;
}


int buscaCampo(HeapObject *obj, const char *name) {
    for (u2 i = 0; i < obj->num_fields; i++) {
        if (obj->field_names[i] && strcmp(obj->field_names[i], name) == 0)
            return (int)i;
    }
    return -1;
}


void liberaHeap(void) {
    for (int i = 0; i < heap_count; i++) {
        HeapObject *obj = heap_pool[i];
        free(obj->class_name);
        for (u2 j = 0; j < obj->num_fields; j++)
            free(obj->field_names[j]);
        free(obj->field_names);
        free(obj->fields);
        free(obj);
    }
    heap_count = 0;
}
