#include "interpreter.h"
#include "basic_ops.h"
#include "attribute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static Code_attribute *encontraCodeAttr(method_info *m, cp_info *cp) {
    for (u2 i = 0; i < m->attributes_count; i++) {
        char *name    = getUtf8(cp, m->attributes[i].attribute_name_index);
        int   is_code = name && strcmp(name, "Code") == 0;
        free(name);
        if (is_code)
            return (Code_attribute *)m->attributes[i].info;
    }
    return NULL;
}


void executaJVM(ClassFile *cf) {
    for (u2 i = 0; i < cf->methods_count; i++) {
        char *name    = getUtf8(cf->constant_pool, cf->methods[i].name_index);
        int   is_main = name && strcmp(name, "main") == 0;
        free(name);
        if (!is_main)
            continue;

        Code_attribute *ca = encontraCodeAttr(&cf->methods[i], cf->constant_pool);
        if (!ca) {
            fprintf(stderr, "executaJVM: metodo main sem Code_attribute\n");
            return;
        }

        Frame *f = criaFrame(ca);
        executaFrame(f, cf);
        liberaFrame(f);
        liberaArrays();
        return;
    }

    fprintf(stderr, "executaJVM: metodo main nao encontrado\n");
}
