#ifndef METHODS_INFO_H
#define METHODS_INFO_H
#include "dataTypes.h"
#include "attributes_info.h"

typedef struct field_info_ {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    attribute_info *attributes; /* attribute_info attributes[attributes_count] */
} field_info;

typedef struct method_info_ {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    attribute_info *attributes; /* attribute_info attributes[attributes_count] */
} method_info;

#endif
