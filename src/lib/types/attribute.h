#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include "class_file/cp_info.h"
#include "consts.h"
#include "dataTypes.h"
#include "class_file/attributes_info.h"
#include "constant_pool.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
} exception_table_entry;


typedef struct {
    u2 max_stack;
    u2 max_locals;
    u4 code_length;
    u1 *code;
    u2 exception_table_length;
    exception_table_entry *exception_table;
    u2 attributes_count;
    attribute_info *attributes;
} Code_attribute;


attribute_info readAttribute(
    FILE *file_ptr,
    cp_info *constant_pool
);

Code_attribute *readCodeAttribute(
    FILE *file_ptr,
    cp_info *constant_pool
);

void printCodeAttribute(
    Code_attribute *code_attr
);

void freeCodeAttribute(
    Code_attribute *code_attr
);

char *getUtf8(
    cp_info *constant_pool,
    u2 index
);

#endif