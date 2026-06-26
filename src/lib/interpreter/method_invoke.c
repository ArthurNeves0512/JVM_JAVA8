#include "method_invoke.h"
#include "consts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── parsing de descriptor ──────────────────────────────────── */

/*
 * Conta argumentos em um descriptor de método.
 * Exemplos: "()V" → 0, "(II)I" → 2, "(Ljava/lang/String;I)V" → 2
 */
int contaArgs(const char *descriptor) {
    int count = 0, i = 1;   /* começa depois do '(' */
    while (descriptor[i] && descriptor[i] != ')') {
        if (descriptor[i] == 'L') {
            while (descriptor[i] && descriptor[i] != ';') i++;
        } else if (descriptor[i] == '[') {
            while (descriptor[i] == '[') i++;
            if (descriptor[i] == 'L')
                while (descriptor[i] && descriptor[i] != ';') i++;
        }
        count++;
        if (descriptor[i]) i++;
    }
    return count;
}

/* Retorna 1 se o tipo de retorno do descriptor é void ('V'). */
int eVoid(const char *descriptor) {
    const char *close = strchr(descriptor, ')');
    return close && *(close + 1) == 'V';
}

/* ── busca no ClassFile ─────────────────────────────────────── */

/*
 * Procura o atributo "Code" nos atributos de um método.
 * Retorna ponteiro para o Code_attribute (cast de attr.info) ou NULL.
 */
Code_attribute *encontraCode(method_info *m, cp_info *cp) {
    for (u2 i = 0; i < m->attributes_count; i++) {
        char *name    = getUtf8(cp, m->attributes[i].attribute_name_index);
        int   is_code = name && strcmp(name, "Code") == 0;
        free(name);
        if (is_code)
            return (Code_attribute *)m->attributes[i].info;
    }
    return NULL;
}

/*
 * Busca um método pelo nome e descriptor no ClassFile.
 * Retorna ponteiro para method_info ou NULL se não encontrado.
 * (Resolução simples: apenas a classe atual, sem herança.)
 */
method_info *encontraMetodo(ClassFile *cf, const char *name, const char *desc) {
    if (!cf) return NULL;
    for (u2 i = 0; i < cf->methods_count; i++) {
        char *mname = getUtf8(cf->constant_pool, cf->methods[i].name_index);
        char *mdesc = getUtf8(cf->constant_pool, cf->methods[i].descriptor_index);
        int match = mname && mdesc
                    && strcmp(mname, name) == 0
                    && strcmp(mdesc,  desc) == 0;
        free(mname);
        free(mdesc);
        if (match) return &cf->methods[i];
    }
    return NULL;
}

/* ── helper interno ─────────────────────────────────────────── */

/*
 * Cria um frame filho, transfere argumentos da pilha do chamador
 * para as variáveis locais do filho, executa e devolve o retorno.
 *
 * instance_method=0 (estático): locais[0..n-1] ← args
 * instance_method=1 (virtual):  locais[0] ← objectref, locais[1..n] ← args
 *
 * Depois de executaFrame retornar (ireturn/return), o valor de retorno
 * (se não-void) está no topo da pilha do frame filho e é copiado
 * para o frame chamador.
 */
static void executa_chamada(Frame *caller, ClassFile *cf,
                             Code_attribute *ca, const char *descriptor,
                             int instance_method) {
    int    n     = contaArgs(descriptor);
    Frame *child = criaFrame(ca);

    if (instance_method) {
        /* Pop args direita→esquerda → locais[n..1] */
        for (int i = n; i >= 1; i--)
            child->locais[i] = caller->pilha[caller->topo--];
        /* Pop objectref → locais[0] */
        child->locais[0] = caller->pilha[caller->topo--];
    } else {
        /* Pop args direita→esquerda → locais[n-1..0] */
        for (int i = n - 1; i >= 0; i--)
            child->locais[i] = caller->pilha[caller->topo--];
    }

    executaFrame(child, cf);

    /* Se retorno não-void e há valor no topo do filho, empurra no chamador */
    if (!eVoid(descriptor) && child->topo >= 0)
        caller->pilha[++caller->topo] = child->pilha[child->topo];

    liberaFrame(child);
}

/* ── invokestatic ───────────────────────────────────────────── */

/*
 * Resolve o Methodref em cp[idx], localiza o método no ClassFile,
 * cria um novo frame, move os argumentos da pilha do chamador para
 * as variáveis locais do novo frame e executa.
 */
void execInvokestatic(Frame *caller, ClassFile *cf, u2 idx) {
    const CONSTANT_Methodref_info   *mref =
        cf->constant_pool[idx].methodRef_info;
    const CONSTANT_NameAndType_info *nat  =
        cf->constant_pool[mref->name_and_type_index].nameAndType_info;

    char *name = getUtf8(cf->constant_pool, nat->name_index);
    char *desc = getUtf8(cf->constant_pool, nat->descriptor_index);

    method_info *m = encontraMetodo(cf, name, desc);
    if (!m) {
        fprintf(stderr,
                "invokestatic: metodo '%s%s' nao encontrado\n", name, desc);
        caller->topo -= contaArgs(desc);
        free(name); free(desc);
        return;
    }

    Code_attribute *ca = encontraCode(m, cf->constant_pool);
    if (!ca) {
        fprintf(stderr,
                "invokestatic: '%s%s' sem Code_attribute\n", name, desc);
        free(name); free(desc);
        return;
    }

    executa_chamada(caller, cf, ca, desc, 0 /* estático */);
    free(name);
    free(desc);
}

/* ── invokevirtual ──────────────────────────────────────────── */

/*
 * Igual ao invokestatic, mas o objectref (this) é armazenado em
 * locais[0] do frame filho. Resolução simples: busca no ClassFile
 * atual (polimorfismo é responsabilidade de outra pessoa).
 */
void execInvokevirtual(Frame *caller, ClassFile *cf, u2 idx) {
    const CONSTANT_Methodref_info   *mref =
        cf->constant_pool[idx].methodRef_info;
    const CONSTANT_NameAndType_info *nat  =
        cf->constant_pool[mref->name_and_type_index].nameAndType_info;

    char *name = getUtf8(cf->constant_pool, nat->name_index);
    char *desc = getUtf8(cf->constant_pool, nat->descriptor_index);

    method_info *m = encontraMetodo(cf, name, desc);
    if (!m) {
        /* Método não encontrado no ClassFile (ex: println nativo).
         * Descarta objectref + args para não corromper a pilha. */
        fprintf(stderr,
                "invokevirtual: '%s%s' nao encontrado (metodo externo?)\n",
                name, desc);
        caller->topo -= contaArgs(desc) + 1;  /* +1 pelo objectref */
        free(name); free(desc);
        return;
    }

    Code_attribute *ca = encontraCode(m, cf->constant_pool);
    if (!ca) {
        fprintf(stderr,
                "invokevirtual: '%s%s' sem Code_attribute\n", name, desc);
        free(name); free(desc);
        return;
    }

    executa_chamada(caller, cf, ca, desc, 1 /* instância */);
    free(name);
    free(desc);
}
