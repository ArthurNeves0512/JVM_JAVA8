#include "method_invoke.h"
#include "attribute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ── parsing de descritores ────────────────────────────────────────── */

/*
 * Conta o número de argumentos em um descritor de método.
 * Ex: "(II)I" → 2,  "(Ljava/lang/String;I)V" → 2,  "()V" → 0
 */
int conta_args(const char *descriptor) {
    int count = 0;
    int i     = 1; /* pula '(' */
    while (descriptor[i] && descriptor[i] != ')') {
        if (descriptor[i] == 'L') {
            while (descriptor[i] && descriptor[i] != ';') i++;
        } else if (descriptor[i] == '[') {
            while (descriptor[i] == '[') i++; /* dimensões do array */
            if (descriptor[i] == 'L')
                while (descriptor[i] && descriptor[i] != ';') i++;
        }
        count++;
        if (descriptor[i]) i++;
    }
    return count;
}

/* Retorna 1 se o tipo de retorno do descritor é void */
int retorno_void(const char *descriptor) {
    const char *close = strchr(descriptor, ')');
    return close && *(close + 1) == 'V';
}


/* ── busca de método e atributo ─────────────────────────────────────── */

Code_attribute *encontraCodeAttr(method_info *m, cp_info *cp) {
    for (u2 i = 0; i < m->attributes_count; i++) {
        char *name    = getUtf8(cp, m->attributes[i].attribute_name_index);
        int   is_code = name && strcmp(name, "Code") == 0;
        free(name);
        if (is_code)
            return (Code_attribute *)m->attributes[i].info;
    }
    return NULL;
}

method_info *encontraMetodo(ClassFile *cf, const char *name, const char *desc) {
    for (u2 i = 0; i < cf->methods_count; i++) {
        char *mname = getUtf8(cf->constant_pool, cf->methods[i].name_index);
        char *mdesc = getUtf8(cf->constant_pool, cf->methods[i].descriptor_index);
        int   match = mname && mdesc
                      && strcmp(mname, name) == 0
                      && strcmp(mdesc, desc) == 0;
        free(mname);
        free(mdesc);
        if (match) return &cf->methods[i];
    }
    return NULL;
}


/* ── execução de método interno ─────────────────────────────────────── */

/*
 * Cria um filho Frame, popula os locais com os argumentos do pai,
 * executa e empurra o valor de retorno (se houver) de volta ao pai.
 *
 * Para métodos estáticos: locais[0..n_args-1] ← args
 * Para métodos de instância: locais[0] ← objectref, locais[1..] ← args
 */
static void executa_chamada(Frame *caller, ClassFile *cf,
                            Code_attribute *ca, const char *descriptor,
                            int instance_method) {
    int    n_args = conta_args(descriptor);
    Frame *child  = criaFrame(ca);

    if (instance_method) {
        /* Pop args da direita para a esquerda → locais[1..n_args] */
        for (int i = n_args; i >= 1; i--)
            child->locais[i] = caller->pilha[caller->topo--];
        /* Pop objectref → locais[0] */
        child->locais[0] = caller->pilha[caller->topo--];
    } else {
        /* Pop args da direita para a esquerda → locais[0..n_args-1] */
        for (int i = n_args - 1; i >= 0; i--)
            child->locais[i] = caller->pilha[caller->topo--];
    }

    executaFrame(child, cf);

    /* Se não-void e há valor no topo, empurra para o frame pai */
    if (!retorno_void(descriptor) && child->topo >= 0)
        caller->pilha[++caller->topo] = child->pilha[child->topo];

    liberaFrame(child);
}


/* ── handlers de opcode ─────────────────────────────────────────────── */

void execInvokestatic(Frame *caller, ClassFile *cf, u2 idx) {
    CONSTANT_Methodref_info  *mref = cf->constant_pool[idx].methodRef_info;
    CONSTANT_NameAndType_info *nat = cf->constant_pool[mref->name_and_type_index].nameAndType_info;

    char *method_name = getUtf8(cf->constant_pool, nat->name_index);
    char *descriptor  = getUtf8(cf->constant_pool, nat->descriptor_index);

    method_info *m = encontraMetodo(cf, method_name, descriptor);
    if (!m) {
        /* Método externo (ex: java/lang/Math) — pula com aviso */
        char *class_name = getUtf8(cf->constant_pool,
            cf->constant_pool[mref->class_index].constant_class_info->name_index);
        fprintf(stderr, "invokestatic: metodo externo '%s.%s%s' ignorado\n",
                class_name ? class_name : "?", method_name, descriptor);
        free(class_name);
        /* Descarta argumentos da pilha para manter consistência */
        int n_args = conta_args(descriptor);
        for (int i = 0; i < n_args; i++) caller->topo--;
        free(method_name);
        free(descriptor);
        return;
    }

    Code_attribute *ca = encontraCodeAttr(m, cf->constant_pool);
    if (ca)
        executa_chamada(caller, cf, ca, descriptor, 0 /* static */);
    else
        fprintf(stderr, "invokestatic: '%s%s' sem Code_attribute\n",
                method_name, descriptor);

    free(method_name);
    free(descriptor);
}


void execInvokevirtual(Frame *caller, ClassFile *cf, u2 idx) {
    CONSTANT_Methodref_info  *mref = cf->constant_pool[idx].methodRef_info;
    CONSTANT_NameAndType_info *nat = cf->constant_pool[mref->name_and_type_index].nameAndType_info;

    char *method_name = getUtf8(cf->constant_pool, nat->name_index);
    char *descriptor  = getUtf8(cf->constant_pool, nat->descriptor_index);

    method_info *m = encontraMetodo(cf, method_name, descriptor);
    if (!m) {
        char *class_name = getUtf8(cf->constant_pool,
            cf->constant_pool[mref->class_index].constant_class_info->name_index);
        fprintf(stderr, "invokevirtual: metodo '%s.%s%s' nao encontrado\n",
                class_name ? class_name : "?", method_name, descriptor);
        free(class_name);
        /* Descarta args + objectref */
        int n_args = conta_args(descriptor);
        for (int i = 0; i < n_args + 1; i++) caller->topo--;
        free(method_name);
        free(descriptor);
        return;
    }

    Code_attribute *ca = encontraCodeAttr(m, cf->constant_pool);
    if (ca)
        executa_chamada(caller, cf, ca, descriptor, 1 /* instance */);
    else
        fprintf(stderr, "invokevirtual: '%s%s' sem Code_attribute\n",
                method_name, descriptor);

    free(method_name);
    free(descriptor);
}


void execInvokespecial(Frame *caller, ClassFile *cf, u2 idx) {
    CONSTANT_Methodref_info  *mref = cf->constant_pool[idx].methodRef_info;
    CONSTANT_NameAndType_info *nat = cf->constant_pool[mref->name_and_type_index].nameAndType_info;

    char *method_name = getUtf8(cf->constant_pool, nat->name_index);
    char *descriptor  = getUtf8(cf->constant_pool, nat->descriptor_index);

    if (method_name && strcmp(method_name, "<init>") == 0) {
        method_info *m = encontraMetodo(cf, "<init>", descriptor);
        if (m) {
            Code_attribute *ca = encontraCodeAttr(m, cf->constant_pool);
            if (ca) {
                /* executa o construtor com objectref em locais[0] */
                executa_chamada(caller, cf, ca, descriptor, 1 /* instance */);
                free(method_name);
                free(descriptor);
                return;
            }
        }
        /* <init> nao encontrado: descarta args + objectref */
        int n_args = conta_args(descriptor);
        for (int i = 0; i < n_args + 1; i++) caller->topo--;
        free(method_name);
        free(descriptor);
        return;
    }

    /* Para outros invokespecial (private, super) trata como virtual */
    method_info *m = encontraMetodo(cf, method_name, descriptor);
    if (m) {
        Code_attribute *ca = encontraCodeAttr(m, cf->constant_pool);
        if (ca)
            executa_chamada(caller, cf, ca, descriptor, 1 /* instance */);
    } else {
        fprintf(stderr, "invokespecial: '%s%s' nao encontrado\n",
                method_name, descriptor);
    }

    free(method_name);
    free(descriptor);
}
