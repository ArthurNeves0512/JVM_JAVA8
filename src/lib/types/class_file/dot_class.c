#include "lib/types/class_file/dot_class.h"
#include "lib/class_loader/methods.h"
#include "lib/types/attribute.h"
#include "lib/types/consts.h"
#include <stdlib.h>
#include <stdio.h>

ClassFile *allocClassFile() {
    ClassFile *class_file = (ClassFile *)malloc(sizeof(ClassFile));
    if (class_file == NULL) {
        fprintf(stderr, "Failed to allocate memory for ClassFile\n");
        exit(EXIT_FAILURE);
    }
    class_file->constant_pool = NULL;
    class_file->interfaces = NULL;
    class_file->fields = NULL;
    class_file->methods = NULL;
    class_file->attributes = NULL;
    return class_file;
}

static void freeConstantPool(cp_info *constant_pool, u2 constant_pool_count) {
    if (!constant_pool) return;

    for (u2 i = 1; i < constant_pool_count; i++) {
        cp_info *entry = &constant_pool[i];
        switch (entry->tag) {
        case CONSTANT_Class:              free(entry->constant_class_info);   break;
        case CONSTANT_Fieldref:           free(entry->fieldRef_info);         break;
        case CONSTANT_Methodref:          free(entry->methodRef_info);        break;
        case CONSTANT_InterfaceMethodref: free(entry->interfaceMethod_info);  break;
        case CONSTANT_String:             free(entry->string_info);           break;
        case CONSTANT_Integer:            free(entry->integer_info);          break;
        case CONSTANT_Float:              free(entry->float_info);            break;
        case CONSTANT_Long:               free(entry->long_info);             break;
        case CONSTANT_Double:             free(entry->double_info);           break;
        case CONSTANT_NameAndType:        free(entry->nameAndType_info);      break;
        case CONSTANT_MethodHandle:       free(entry->methodHandle_info);     break;
        case CONSTANT_MethodType:         free(entry->methodType_info);       break;
        case CONSTANT_InvokeDynamic:      free(entry->invokeDynamic_info);    break;
        case CONSTANT_Utf8:
            free(entry->utf8_info->bytes);
            free(entry->utf8_info);
            break;
        default:
            /* placeholder slot following a Long/Double entry — nothing to free */
            break;
        }
    }
    free(constant_pool);
}

static void freeFields(ClassFile *cf) {
    if (!cf->fields) return;

    for (u2 i = 0; i < cf->fields_count; i++) {
        field_info *field = &cf->fields[i];
        if (field->attributes) {
            for (u2 j = 0; j < field->attributes_count; j++)
                freeAttribute(&field->attributes[j], cf->constant_pool);
            free(field->attributes);
        }
    }
    free(cf->fields);
    cf->fields = NULL;
}

static void freeClassAttributes(ClassFile *cf) {
    if (!cf->attributes) return;

    for (u2 i = 0; i < cf->attributes_count; i++)
        freeAttribute(&cf->attributes[i], cf->constant_pool);
    free(cf->attributes);
    cf->attributes = NULL;
}

void freeClassFile(ClassFile *cf) {
    if (!cf) return;

    freeFields(cf);
    freeMethods(cf);
    freeClassAttributes(cf);

    free(cf->interfaces);
    cf->interfaces = NULL;

    freeConstantPool(cf->constant_pool, cf->constant_pool_count);
    cf->constant_pool = NULL;

    free(cf);
}