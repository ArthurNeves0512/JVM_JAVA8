#ifndef DOT_CLASS_H
#define DOT_CLASS_H
#include "lib/types/constant_pool.h"
#include "lib/types/class_file/methods_info.h"
#include "lib/types/class_file/attributes_info.h"

typedef struct ClassFile_ {
    u4 magic;
    u2 minor_version;
    u2 major_version;
    u2 constant_pool_count;
    cp_info *constant_pool; /* cp_info constant_pool[constant_pool_count - 1] */
    u2 access_flags;
    u2 this_class;
    u2 super_class;
    u2 interfaces_count;
    u2 *interfaces; /* u2 interfaces[interfaces_count] */
    u2 fields_count;
    field_info *fields; /* field_info fields[fields_count] */
    u2 methods_count;
    method_info *methods;
    u2 attributes_count;
    attribute_info *attributes;
} ClassFile;

ClassFile *allocClassFile();

#endif
