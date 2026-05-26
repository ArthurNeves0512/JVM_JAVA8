#ifndef FIELDS_INTERFACES_H
#define FIELDS_INTERFACES_H

#include "loader.h"

void readInterfaces(ClassFile *class_file_ptr, FILE *file_ptr) {
    class_file_ptr->interfaces_count = u2Read(file_ptr);
    printf("\n=== Interfaces ===\n");
    printf("interfaces_count: %hu\n", class_file_ptr->interfaces_count);

    class_file_ptr->interfaces = (u2*) malloc(class_file_ptr->interfaces_count * sizeof(u2));
    for (u2 i = 0; i < class_file_ptr->interfaces_count; i++) {
        u2 index = u2Read(file_ptr);
        class_file_ptr->interfaces[i] = index;
        CONSTANT_Class_info * class_info = class_file_ptr->constant_pool[index].constant_class_info;
        CONSTANT_Utf8_info * utf8_info = class_file_ptr->constant_pool[class_info->name_index].utf8_info;
        printf("interface[%hu]:\n", i);
        printf("\tindex at constant_pool: #%hu\n",index);
        printf("\tvalue: %s\n",utf8_info->bytes);
    }
}

void readFields(ClassFile *class_file_ptr, FILE *file_ptr) {
    class_file_ptr->fields_count = u2Read(file_ptr);
    printf("\n=== Fields ===\n");
    printf("fields_count: %hu\n", class_file_ptr->fields_count);

    class_file_ptr->fields = (field_info*) malloc(class_file_ptr->fields_count * sizeof(field_info));

    for (u2 i = 0; i < class_file_ptr->fields_count; i++) {
        class_file_ptr->fields[i].access_flags     = u2Read(file_ptr);
        class_file_ptr->fields[i].name_index       = u2Read(file_ptr);
        class_file_ptr->fields[i].descriptor_index = u2Read(file_ptr);
        class_file_ptr->fields[i].attributes_count = u2Read(file_ptr);

        printf("field[%hu]: name=#%hu descriptor=#%hu\n", i,
            class_file_ptr->fields[i].name_index,
            class_file_ptr->fields[i].descriptor_index);

        for (u2 j = 0; j < class_file_ptr->fields[i].attributes_count; j++) {
            u2Read(file_ptr);
            u4 len = u4Read(file_ptr);
            for (u4 k = 0; k < len; k++) u1Read(file_ptr);
        }
    }
}

#endif