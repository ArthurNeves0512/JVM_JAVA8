/* STUB – Pessoa2: inicialização e loop principal da JVM. */
#include "interpreter.h"
#include "method_invoke.h"
#include "basic_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void executaJVM(ClassFile *cf) {
    for (u2 i = 0; i < cf->methods_count; i++) {
        char *name    = getUtf8(cf->constant_pool, cf->methods[i].name_index);
        int   is_main = name && strcmp(name, "main") == 0;
        free(name);
        if (!is_main) continue;

        Code_attribute *ca = encontraCode(&cf->methods[i], cf->constant_pool);
        if (!ca) {
            fprintf(stderr, "executaJVM: main sem Code_attribute\n");
            return;
        }

        Frame *f = criaFrame(ca);
        executaFrame(f, cf);
        liberaFrame(f);
        return;
    }
    fprintf(stderr, "executaJVM: main nao encontrado\n");
}
