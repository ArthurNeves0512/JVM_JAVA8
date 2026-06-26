#ifndef FIELDS_INTERFACES_H
#define FIELDS_INTERFACES_H

#include "dot_class.h"
#include "loader.h"

void readInterfaces(ClassFile *class_file_ptr, FILE *file_ptr);
void readFields(ClassFile *class_file_ptr, FILE *file_ptr);

#endif
