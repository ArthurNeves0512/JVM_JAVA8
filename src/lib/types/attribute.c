#include "attribute.h"
#include "../file/read_byte.h"
#include <string.h>
#include "opcodes.h"

/* ─────────────────────────────────────────────
   imprime sub-atributos de Code
   (LineNumberTable, LocalVariableTable, raw)
   ───────────────────────────────────────────── */
static void printSubAttribute(
    attribute_info *attr,
    cp_info        *constant_pool
) {
    char *name = getUtf8(constant_pool, attr->attribute_name_index);
    if (!name) return;

    if (strcmp(name, "LineNumberTable") == 0) {
        printLineNumberTableAttribute(
            (LineNumberTable_attribute *) attr->info
        );
    } else if (strcmp(name, "LocalVariableTable") == 0) {
        printLocalVariableTableAttribute(
            (LocalVariableTable_attribute *) attr->info,
            constant_pool
        );
    } else {
        printf("  [sub-attr] %s  length=%u  (raw)\n",
               name, attr->attribute_length);
    }
    free(name);
}



Code_attribute *readCodeAttribute(
    FILE    *file_ptr,
    cp_info *constant_pool
) {
    Code_attribute *code_attr =
        (Code_attribute *) malloc(sizeof(Code_attribute));

    code_attr->max_stack  = u2Read(file_ptr);
    code_attr->max_locals = u2Read(file_ptr);
    code_attr->code_length = u4Read(file_ptr);

    code_attr->code = (u1 *) malloc(code_attr->code_length);
    fread(code_attr->code, sizeof(u1), code_attr->code_length, file_ptr);

    code_attr->exception_table_length = u2Read(file_ptr);

    if (code_attr->exception_table_length > 0) {
        code_attr->exception_table =
            (exception_table_entry *)
            malloc(code_attr->exception_table_length
                   * sizeof(exception_table_entry));

        for (u2 i = 0; i < code_attr->exception_table_length; i++) {
            code_attr->exception_table[i].start_pc   = u2Read(file_ptr);
            code_attr->exception_table[i].end_pc     = u2Read(file_ptr);
            code_attr->exception_table[i].handler_pc = u2Read(file_ptr);
            code_attr->exception_table[i].catch_type = u2Read(file_ptr);
        }
    } else {
        code_attr->exception_table = NULL;
    }

    code_attr->attributes_count = u2Read(file_ptr);

    if (code_attr->attributes_count > 0) {
        code_attr->attributes =
            (attribute_info *)
            malloc(code_attr->attributes_count * sizeof(attribute_info));

        for (u2 i = 0; i < code_attr->attributes_count; i++) {
            code_attr->attributes[i] =
                readAttribute(file_ptr, constant_pool);
        }
    } else {
        code_attr->attributes = NULL;
    }

    return code_attr;
}

ConstantValue_attribute *readConstantValueAttribute(FILE *fp) {
    ConstantValue_attribute *attr =
        (ConstantValue_attribute *) malloc(sizeof(ConstantValue_attribute));
    attr->constantvalue_index = u2Read(fp);
    return attr;
}

Exceptions_attribute *readExceptionsAttribute(FILE *fp) {
    Exceptions_attribute *attr =
        (Exceptions_attribute *) malloc(sizeof(Exceptions_attribute));

    attr->number_of_exceptions = u2Read(fp);

    if (attr->number_of_exceptions > 0) {
        attr->exception_index_table =
            (u2 *) malloc(attr->number_of_exceptions * sizeof(u2));
        for (u2 i = 0; i < attr->number_of_exceptions; i++)
            attr->exception_index_table[i] = u2Read(fp);
    } else {
        attr->exception_index_table = NULL;
    }
    return attr;
}

SourceFile_attribute *readSourceFileAttribute(FILE *fp) {
    SourceFile_attribute *attr =
        (SourceFile_attribute *) malloc(sizeof(SourceFile_attribute));
    attr->sourcefile_index = u2Read(fp);
    return attr;
}

InnerClasses_attribute *readInnerClassesAttribute(FILE *fp) {
    InnerClasses_attribute *attr =
        (InnerClasses_attribute *) malloc(sizeof(InnerClasses_attribute));

    attr->number_of_classes = u2Read(fp);

    if (attr->number_of_classes > 0) {
        attr->classes =
            (inner_class_entry *)
            malloc(attr->number_of_classes * sizeof(inner_class_entry));

        for (u2 i = 0; i < attr->number_of_classes; i++) {
            attr->classes[i].inner_class_info_index   = u2Read(fp);
            attr->classes[i].outer_class_info_index   = u2Read(fp);
            attr->classes[i].inner_name_index         = u2Read(fp);
            attr->classes[i].inner_class_access_flags = u2Read(fp);
        }
    } else {
        attr->classes = NULL;
    }
    return attr;
}

LineNumberTable_attribute *readLineNumberTableAttribute(FILE *fp) {
    LineNumberTable_attribute *attr =
        (LineNumberTable_attribute *) malloc(sizeof(LineNumberTable_attribute));

    attr->line_number_table_length = u2Read(fp);

    attr->line_number_table =
        (line_number_table_entry *)
        malloc(attr->line_number_table_length
               * sizeof(line_number_table_entry));

    for (u2 i = 0; i < attr->line_number_table_length; i++) {
        attr->line_number_table[i].start_pc   = u2Read(fp);
        attr->line_number_table[i].line_number = u2Read(fp);
    }
    return attr;
}

LocalVariableTable_attribute *readLocalVariableTableAttribute(FILE *fp) {
    LocalVariableTable_attribute *attr =
        (LocalVariableTable_attribute *)
        malloc(sizeof(LocalVariableTable_attribute));

    attr->local_variable_table_length = u2Read(fp);

    attr->local_variable_table =
        (local_variable_table_entry *)
        malloc(attr->local_variable_table_length
               * sizeof(local_variable_table_entry));

    for (u2 i = 0; i < attr->local_variable_table_length; i++) {
        attr->local_variable_table[i].start_pc         = u2Read(fp);
        attr->local_variable_table[i].length           = u2Read(fp);
        attr->local_variable_table[i].name_index       = u2Read(fp);
        attr->local_variable_table[i].descriptor_index = u2Read(fp);
        attr->local_variable_table[i].index            = u2Read(fp);
    }
    return attr;
}

/* ─────────────────────────────────────────────
   Lê name_index + length, despacha pelo nome.
   O ponteiro armazenado em attr.info é tipado
   pelo nome; o chamador deve consultar o nome
   do CP para saber como interpretá-lo.
   ───────────────────────────────────────────── */
attribute_info readAttribute(
    FILE    *file_ptr,
    cp_info *constant_pool
) {
    attribute_info attr;
    attr.attribute_name_index = u2Read(file_ptr);
    attr.attribute_length     = u4Read(file_ptr);

    char *name = getUtf8(constant_pool, attr.attribute_name_index);

    if (name && strcmp(name, "Code") == 0) {
        attr.info = (u1 *) readCodeAttribute(file_ptr, constant_pool);

    } else if (name && strcmp(name, "ConstantValue") == 0) {
        attr.info = (u1 *) readConstantValueAttribute(file_ptr);

    } else if (name && strcmp(name, "Exceptions") == 0) {
        attr.info = (u1 *) readExceptionsAttribute(file_ptr);

    } else if (name && strcmp(name, "SourceFile") == 0) {
        attr.info = (u1 *) readSourceFileAttribute(file_ptr);

    } else if (name && strcmp(name, "InnerClasses") == 0) {
        attr.info = (u1 *) readInnerClassesAttribute(file_ptr);

    } else if (name && strcmp(name, "LineNumberTable") == 0) {
        attr.info = (u1 *) readLineNumberTableAttribute(file_ptr);

    } else if (name && strcmp(name, "LocalVariableTable") == 0) {
        attr.info = (u1 *) readLocalVariableTableAttribute(file_ptr);

    } else {
        
        attr.info = (u1 *) malloc(attr.attribute_length);
        fread(attr.info, sizeof(u1), attr.attribute_length, file_ptr);
    }

    free(name);
    return attr;
}



void printCodeAttribute(
    Code_attribute *code_attr,
    cp_info        *constant_pool
) {
    printf("=== Code Attribute ===\n");
    printf("  max_stack:  %hu\n", code_attr->max_stack);
    printf("  max_locals: %hu\n", code_attr->max_locals);
    printf("  code_length: %u\n", code_attr->code_length);

    printf("  Bytecode (hex):\n  ");
    /*for (u4 i = 0; i < code_attr->code_length; i++) {  implementação que mostra a saída em HEXA
        printf("%02X ", code_attr->code[i]);
        if ((i + 1) % 16 == 0) printf("\n  ");
    }*/
    printBytecode(code_attr ->code, code_attr->code_length, constant_pool);
    printf("\n");

    printf("  exception_table_length: %hu\n",
           code_attr->exception_table_length);
    for (u2 i = 0; i < code_attr->exception_table_length; i++) {
        printf("    [%hu] start=%-4hu end=%-4hu handler=%-4hu catch=%hu\n",
               i,
               code_attr->exception_table[i].start_pc,
               code_attr->exception_table[i].end_pc,
               code_attr->exception_table[i].handler_pc,
               code_attr->exception_table[i].catch_type);
    }

    printf("  sub-attributes_count: %hu\n", code_attr->attributes_count);
    for (u2 i = 0; i < code_attr->attributes_count; i++) {
        printf("  [%hu] ", i);
        printSubAttribute(&code_attr->attributes[i], constant_pool);
    }
}

void printConstantValueAttribute(
    ConstantValue_attribute *attr,
    cp_info                 *constant_pool
) {
    printf("=== ConstantValue Attribute ===\n");
    u2 idx = attr->constantvalue_index;
    cp_info *entry = &constant_pool[idx];
 
    switch (entry->tag) {
        case CONSTANT_Integer:
            printf("  int    = %d  (#%hu)\n",
                   (int) entry->integer_info->bytes, idx);
            break;
        case CONSTANT_Long:
            printf("  long   = %lldL  (#%hu)\n",
                   ((long long) entry->long_info->high_bytes << 32)
                   | entry->long_info->low_bytes, idx);
            break;
        case CONSTANT_Float: {
            u4 raw = entry->float_info->bytes;
            float f;
            memcpy(&f, &raw, sizeof f);
            printf("  float  = %gf  (#%hu)\n", (double) f, idx);
            break;
        }
        case CONSTANT_Double: {
            u8 raw = ((u8) entry->double_info->high_bytes << 32)
                     | entry->double_info->low_bytes;
            double d;
            memcpy(&d, &raw, sizeof d);
            printf("  double = %g  (#%hu)\n", d, idx);
            break;
        }
        case CONSTANT_String: {
            char *s = getUtf8(constant_pool,
                              entry->string_info->string_index);
            printf("  String = \"%s\"  (#%hu)\n", s ? s : "?", idx);
            free(s);
            break;
        }
        default:
            printf("  constantvalue_index: #%hu  (tag=%u)\n",
                   idx, entry->tag);
    }
}


void printExceptionsAttribute(
    Exceptions_attribute *attr,
    cp_info              *constant_pool
) {
    printf("=== Exceptions Attribute ===\n");
    printf("  number_of_exceptions: %hu\n", attr->number_of_exceptions);
    for (u2 i = 0; i < attr->number_of_exceptions; i++) {
        u2 idx = attr->exception_index_table[i];

        if (constant_pool[idx].tag == CONSTANT_Class) {
            u2 name_idx = constant_pool[idx].constant_class_info->name_index;
            char *name  = getUtf8(constant_pool, name_idx);
            printf("    [%hu] #%hu  %s\n", i, idx, name ? name : "?");
            free(name);
        } else {
            printf("    [%hu] #%hu\n", i, idx);
        }
    }
}

void printSourceFileAttribute(
    SourceFile_attribute *attr,
    cp_info              *constant_pool
) {
    char *name = getUtf8(constant_pool, attr->sourcefile_index);
    printf("=== SourceFile Attribute ===\n");
    printf("  sourcefile: %s  (#%hu)\n", name ? name : "?",
           attr->sourcefile_index);
    free(name);
}

void printInnerClassesAttribute(
    InnerClasses_attribute *attr,
    cp_info                *constant_pool
) {
    printf("=== InnerClasses Attribute ===\n");
    printf("  number_of_classes: %hu\n", attr->number_of_classes);
    for (u2 i = 0; i < attr->number_of_classes; i++) {
        inner_class_entry *e = &attr->classes[i];

        char *inner_name = NULL;
        if (e->inner_name_index != 0)
            inner_name = getUtf8(constant_pool, e->inner_name_index);

        printf(
            "    [%hu] inner=#%hu  outer=#%hu  name=%s  flags=0x%04X\n",
            i,
            e->inner_class_info_index,
            e->outer_class_info_index,
            inner_name ? inner_name : "(anonymous)",
            e->inner_class_access_flags
        );
        free(inner_name);
    }
}

void printLineNumberTableAttribute(LineNumberTable_attribute *attr) {
    printf("  LineNumberTable  length=%hu\n",
           attr->line_number_table_length);
    for (u2 i = 0; i < attr->line_number_table_length; i++) {
        printf("    start_pc=%-4hu  line=%hu\n",
               attr->line_number_table[i].start_pc,
               attr->line_number_table[i].line_number);
    }
}

void printLocalVariableTableAttribute(
    LocalVariableTable_attribute *attr,
    cp_info                      *constant_pool
) {
    printf("  LocalVariableTable  length=%hu\n",
           attr->local_variable_table_length);
    for (u2 i = 0; i < attr->local_variable_table_length; i++) {
        local_variable_table_entry *e = &attr->local_variable_table[i];
        char *var_name   = getUtf8(constant_pool, e->name_index);
        char *descriptor = getUtf8(constant_pool, e->descriptor_index);
        printf(
            "    slot=%-2hu  start=%-4hu  len=%-4hu  %s  %s\n",
            e->index, e->start_pc, e->length,
            var_name   ? var_name   : "?",
            descriptor ? descriptor : "?"
        );
        free(var_name);
        free(descriptor);
    }
}



/* 
   Libera: array de bytecode, tabela de exceções,
   sub-atributos (cada um pelo seu tipo) e a struct. */
void freeCodeAttribute(Code_attribute *attr, cp_info *constant_pool) {
    if (!attr) return;

    free(attr->code);
    free(attr->exception_table);

    if (attr->attributes) {
        for (u2 i = 0; i < attr->attributes_count; i++)
            freeAttribute(&attr->attributes[i], constant_pool);
        free(attr->attributes);
    }

    free(attr);
}

void freeConstantValueAttribute(ConstantValue_attribute *attr) {
    if (!attr) return;
    free(attr);
}

void freeExceptionsAttribute(Exceptions_attribute *attr) {
    if (!attr) return;
    free(attr->exception_index_table);
    free(attr);
}

void freeSourceFileAttribute(SourceFile_attribute *attr) {
    if (!attr) return;
    free(attr);
}

void freeInnerClassesAttribute(InnerClasses_attribute *attr) {
    if (!attr) return;
    free(attr->classes);
    free(attr);
}

void freeLineNumberTableAttribute(LineNumberTable_attribute *attr) {
    if (!attr) return;
    free(attr->line_number_table);
    free(attr);
}

void freeLocalVariableTableAttribute(LocalVariableTable_attribute *attr) {
    if (!attr) return;
    free(attr->local_variable_table);
    free(attr);
}

/* libera qualquer attribute_info pelo nome no CP.
   Resolve o tipo correto e chama o free correspondente.
   Para atributos desconhecidos, libera os bytes brutos. */
void freeAttribute(attribute_info *attr, cp_info *constant_pool) {
    if (!attr || !attr->info) return;

    char *name = getUtf8(constant_pool, attr->attribute_name_index);
    if (!name) {
        free(attr->info);
        return;
    }

    if      (strcmp(name, "Code") == 0)
        freeCodeAttribute((Code_attribute *) attr->info, constant_pool);
    else if (strcmp(name, "ConstantValue") == 0)
        freeConstantValueAttribute((ConstantValue_attribute *) attr->info);
    else if (strcmp(name, "Exceptions") == 0)
        freeExceptionsAttribute((Exceptions_attribute *) attr->info);
    else if (strcmp(name, "SourceFile") == 0)
        freeSourceFileAttribute((SourceFile_attribute *) attr->info);
    else if (strcmp(name, "InnerClasses") == 0)
        freeInnerClassesAttribute((InnerClasses_attribute *) attr->info);
    else if (strcmp(name, "LineNumberTable") == 0)
        freeLineNumberTableAttribute((LineNumberTable_attribute *) attr->info);
    else if (strcmp(name, "LocalVariableTable") == 0)
        freeLocalVariableTableAttribute((LocalVariableTable_attribute *) attr->info);
    else
        free(attr->info); 

    free(name);
}

char *getUtf8(cp_info *constant_pool, u2 index) {
    if (!constant_pool || index == 0) return NULL;

    cp_info *entry = &constant_pool[index];
    if (entry->tag != CONSTANT_Utf8) {
        fprintf(stderr,
                "Error: constant_pool[%u] is not UTF8 (tag=%u)\n",
                index, entry->tag);
        return NULL;
    }

    CONSTANT_Utf8_info *utf8 = entry->utf8_info;
    char *result = (char *) malloc(utf8->length + 1);
    memcpy(result, utf8->bytes, utf8->length);
    result[utf8->length] = '\0';
    return result;
}