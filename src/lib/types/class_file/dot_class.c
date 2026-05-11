#include "lib/types/class_file/dot_class.h"
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