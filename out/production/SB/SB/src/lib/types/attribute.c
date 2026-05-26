#include "attribute.h"
#include "../file/read_byte.h"

#include <string.h>


Code_attribute *readCodeAttribute(
    FILE *file_ptr,
    cp_info *constant_pool
) {

    Code_attribute *code_attr =
        (Code_attribute*)
        malloc(sizeof(Code_attribute));
    code_attr->max_stack =
        u2Read(file_ptr);

    code_attr->max_locals =
        u2Read(file_ptr);

    code_attr->code_length =
        u4Read(file_ptr);

    code_attr->code =
        (u1*) malloc(
            code_attr->code_length
        );

    fread(
        code_attr->code,
        sizeof(u1),
        code_attr->code_length,
        file_ptr
    );

    code_attr->exception_table_length =
        u2Read(file_ptr);

    if(code_attr->exception_table_length > 0) {

        code_attr->exception_table =
            (exception_table_entry*)
            malloc(
                code_attr->exception_table_length
                * sizeof(exception_table_entry)
            );

        for(u2 i = 0;
            i < code_attr->exception_table_length;
            i++) {

            code_attr->exception_table[i].start_pc =
                u2Read(file_ptr);

            code_attr->exception_table[i].end_pc =
                u2Read(file_ptr);

            code_attr->exception_table[i].handler_pc =
                u2Read(file_ptr);

            code_attr->exception_table[i].catch_type =
                u2Read(file_ptr);
        }

    } else {

        code_attr->exception_table = NULL;
    }

    code_attr->attributes_count =
        u2Read(file_ptr);

    if(code_attr->attributes_count > 0) {

        code_attr->attributes =
            (attribute_info*)
            malloc(
                code_attr->attributes_count
                * sizeof(attribute_info)
            );

        for(u2 i = 0;
            i < code_attr->attributes_count;
            i++) {

            code_attr->attributes[i] =
                readAttribute(
                    file_ptr,
                    constant_pool
                );
        }

    } else {

        code_attr->attributes = NULL;
    }

    return code_attr;
}

attribute_info readAttribute(
    FILE *file_ptr,
    cp_info *constant_pool
) {

    attribute_info attr;

    attr.attribute_name_index =
        u2Read(file_ptr);

    attr.attribute_length =
        u4Read(file_ptr);


    char *name =
        getUtf8(
            constant_pool,
            attr.attribute_name_index
        );


    if(strcmp(name, "Code") == 0) {

        attr.info =
            (u1*) readCodeAttribute(
                file_ptr,
                constant_pool
            );
    }


    else {

        attr.info =
            (u1*) malloc(
                attr.attribute_length
            );

        fread(
            attr.info,
            sizeof(u1),
            attr.attribute_length,
            file_ptr
        );
    }

    free(name);
    return attr;
}


void printCodeAttribute(
    Code_attribute *code_attr
) {

    printf("=== Code Attribute ===\n");

    printf(
        "max_stack: %hu\n",
        code_attr->max_stack
    );

    printf(
        "max_locals: %hu\n",
        code_attr->max_locals
    );

    printf(
        "code_length: %u\n",
        code_attr->code_length
    );
    printf("Bytecode:\n");

    for(u4 i = 0;
        i < code_attr->code_length;
        i++) {

        printf(
            "%02X ",
            code_attr->code[i]
        );
    }

    printf("\n");

    printf(
        "exception_table_length: %hu\n",
        code_attr->exception_table_length
    );

    for(u2 i = 0;
        i < code_attr->exception_table_length;
        i++) {

        printf(
            "[%hu] start=%hu end=%hu handler=%hu catch=%hu\n",

            i,

            code_attr->exception_table[i].start_pc,

            code_attr->exception_table[i].end_pc,

            code_attr->exception_table[i].handler_pc,

            code_attr->exception_table[i].catch_type
        );
    }

    printf(
        "attributes_count: %hu\n",
        code_attr->attributes_count
    );

    for(u2 i = 0;
        i < code_attr->attributes_count;
        i++) {

        printf(
            "[%hu] name_index=%hu length=%u\n",

            i,

            code_attr->attributes[i]
                .attribute_name_index,

            code_attr->attributes[i]
                .attribute_length
        );
    }
}

void freeCodeAttribute(
    Code_attribute *code_attr
) {

    if(code_attr == NULL)
        return;
    free(code_attr->code);
    free(code_attr->exception_table);
    if(code_attr->attributes != NULL) {

        for(u2 i = 0;
            i < code_attr->attributes_count;
            i++) {

            free(
                code_attr->attributes[i].info
            );
        }

        free(code_attr->attributes);
    }

    free(code_attr);
}

char *getUtf8(
    cp_info *constant_pool,
    u2 index
) {
    if(constant_pool == NULL || index == 0)
        return NULL;

    cp_info *entry = &constant_pool[index - 1];

    if(entry->tag != CONSTANT_Utf8) {
        fprintf(stderr, "Error: constant_pool[%u] is not a UTF8 constant\n", index);
        return NULL;
    }

    CONSTANT_Utf8_info *utf8_info = entry->utf8_info;
    if(utf8_info == NULL || utf8_info->bytes == NULL)
        return NULL;

    char *result = (char *)malloc(utf8_info->length + 1);
    if(result == NULL) {
        fprintf(stderr, "Error: failed to allocate memory for UTF8 string\n");
        return NULL;
    }

    memcpy(result, utf8_info->bytes, utf8_info->length);
    result[utf8_info->length] = '\0';

    return result;
}