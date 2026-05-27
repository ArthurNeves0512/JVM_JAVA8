#ifndef FIELDS_INTERFACES_H
#define FIELDS_INTERFACES_H

#include "loader.h"
static void printAcessFlags(const field_info * info);
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

static void printAcessFlags(const field_info * info){
    const struct 
    {
        const u2 acess_tag;
        const char * name; 
    } ACESS_FLAG_TABLE[]={
        {ACC_PUBLIC,"ACC_PUBLIC"},
        {ACC_PRIVATE,"ACC_PRIVATE"},
        {ACC_PROTECTED,"ACC_PROTECTED"},
        {ACC_STATIC,"ACC_STATIC"},
        {ACC_FINAL,"ACC_FINAL"},
        {ACC_VOLATILE,"ACC_VOLATILE"},
        {ACC_TRANSIENT,"ACC_TRANSIENT"},
        {ACC_SUPER,"ACC_SUPER"},
        {ACC_INTERFACE,"ACC_INTERFACE"},
        {ACC_ABSTRACT,"ACC_ABSTRACT"},
        {ACC_SYNTHETIC,"ACC_SYNTHETIC"},
        {ACC_ANNOTATION,"ACC_ANNOTATION"},
        {ACC_ENUM,"ACC_ENUM"}
    };    
    const uint8_t size = sizeof(ACESS_FLAG_TABLE)/sizeof(ACESS_FLAG_TABLE[0]);
    int printed = 0;
    for(uint8_t i =0;i<size;i++){
        if(info->access_flags & ACESS_FLAG_TABLE[i].acess_tag){
            printf("%s%s", printed ? " | " : "", ACESS_FLAG_TABLE[i].name);
            printed++;        }
    }
    printf("\n");
    
}
void readFields(ClassFile *class_file_ptr, FILE *file_ptr) {
    class_file_ptr->fields_count = u2Read(file_ptr);
    printf("\n=== Fields ===\n");
    printf("fields_count: %hu\n", class_file_ptr->fields_count);

    class_file_ptr->fields = (field_info*) malloc(class_file_ptr->fields_count * sizeof(field_info));

    for (u2 i = 0; i < class_file_ptr->fields_count; i++) {
        field_info * field = &class_file_ptr->fields[i];
        field->access_flags     = u2Read(file_ptr);
        field->name_index       = u2Read(file_ptr);
        field->descriptor_index = u2Read(file_ptr);
        field->attributes_count = u2Read(file_ptr);

        field->attributes = (attribute_info *) malloc(sizeof(attribute_info)*field->attributes_count);

        printf("field[%hu]:\n",i);

        printf("\tname=#%hu (%s)\n",
            field->name_index,
            class_file_ptr->constant_pool[field->name_index].utf8_info->bytes
        );
        printf("\tdescriptor=#%hu (%s)\n",field->descriptor_index,
        class_file_ptr->constant_pool[field->descriptor_index].utf8_info->bytes
        );        
        printf("\tflags (0x%04x): ",field->access_flags);

        printAcessFlags(field);

        printf("attributes_count do field %hu\n",field->attributes_count);
        for (u2 j = 0; j < field->attributes_count; j++) {
            u2 attribute_name_index = u2Read(file_ptr);
            u4 attribute_length = u4Read(file_ptr);
            field->attributes[j].attribute_name_index=attribute_name_index;
            field->attributes[j].attribute_length=attribute_length;
            for (u4 k = 0; k < attribute_length; k++) {
                field->attributes[j].info= (u1 *) u1Read(file_ptr);
            }

            printf("\tatribute_name_index: %hu\n",field->attributes[j].attribute_name_index);
            printf("\tlenght: %hu\n",field->attributes[j].attribute_length);
            printf("\tatribute_name_index: %s\n",field->attributes[j].info);
        }
    }
}

#endif