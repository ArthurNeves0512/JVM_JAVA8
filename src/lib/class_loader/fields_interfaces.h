#ifndef FIELDS_INTERFACES_H
#define FIELDS_INTERFACES_H

#include "loader.h"

void readInterfaces(ClassFile *class_file_ptr, FILE *file_ptr);
void readFields(ClassFile *class_file_ptr, FILE *file_ptr);

#endif
