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

typedef struct {
    u2 constantvalue_index;          
} ConstantValue_attribute;

typedef struct {
    u2 number_of_exceptions;
    u2 *exception_index_table;
} Exceptions_attribute;

typedef struct {
    u2 sourcefile_index;
} SourceFile_attribute;

typedef struct {                   
    u2 inner_class_info_index;
    u2 outer_class_info_index;
    u2 inner_name_index;
    u2 inner_class_access_flags;
} inner_class_entry;

typedef struct {
    u2 number_of_classes;
    inner_class_entry *classes;      
} InnerClasses_attribute;

typedef struct {
    u2 start_pc;
    u2 line_number;
} line_number_table_entry;

typedef struct {
    u2 line_number_table_length;
    line_number_table_entry *line_number_table;  
} LineNumberTable_attribute;

typedef struct {
    u2 start_pc;
    u2 length;
    u2 name_index;
    u2 descriptor_index;
    u2 index;
} local_variable_table_entry;

typedef struct {
    u2 local_variable_table_length;
    local_variable_table_entry *local_variable_table; 
} LocalVariableTable_attribute;

/* ───────────────────────────────────────────── */

attribute_info readAttribute(FILE *file_ptr, cp_info *constant_pool);

Code_attribute            *readCodeAttribute(FILE *fp, cp_info *cp);
ConstantValue_attribute   *readConstantValueAttribute(FILE *fp);
Exceptions_attribute      *readExceptionsAttribute(FILE *fp);
SourceFile_attribute      *readSourceFileAttribute(FILE *fp);
InnerClasses_attribute    *readInnerClassesAttribute(FILE *fp);
LineNumberTable_attribute *readLineNumberTableAttribute(FILE *fp);
LocalVariableTable_attribute *readLocalVariableTableAttribute(FILE *fp);

/* ───────────────────────────────────────────── */

void printCodeAttribute(Code_attribute *code_attr, cp_info *constant_pool);
void printConstantValueAttribute(ConstantValue_attribute *attr, cp_info *constant_pool);
void printExceptionsAttribute(Exceptions_attribute *attr, cp_info *constant_pool);
void printSourceFileAttribute(SourceFile_attribute *attr, cp_info *constant_pool);
void printInnerClassesAttribute(InnerClasses_attribute *attr, cp_info *constant_pool);
void printLineNumberTableAttribute(LineNumberTable_attribute *attr);
void printLocalVariableTableAttribute(LocalVariableTable_attribute *attr, cp_info *constant_pool);

/* ───────────────────────────────────────────── */

void freeCodeAttribute(Code_attribute *attr, cp_info *constant_pool);
void freeConstantValueAttribute(ConstantValue_attribute *attr);
void freeExceptionsAttribute(Exceptions_attribute *attr);
void freeSourceFileAttribute(SourceFile_attribute *attr);
void freeInnerClassesAttribute(InnerClasses_attribute *attr);
void freeLineNumberTableAttribute(LineNumberTable_attribute *attr);
void freeLocalVariableTableAttribute(LocalVariableTable_attribute *attr);

void freeAttribute(attribute_info *attr, cp_info *constant_pool);

char *getUtf8(cp_info *constant_pool, u2 index);

#endif