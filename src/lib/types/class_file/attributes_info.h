#ifndef ATTRIBUTES_INFO_H
#define ATTRIBUTES_INFO_H
#include "dataTypes.h"

typedef struct attribute_info_ {
    u2 attribute_name_index;
    u4 attribute_length;
    u1 *info; /* u1 info[attribute_length] */
} attribute_info;

#endif
