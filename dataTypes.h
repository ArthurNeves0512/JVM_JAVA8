#ifndef DATA_TYPES_H
#define DATA_TYPES_H
#include<stdint.h>
#include<stdio.h>


typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;


typedef struct attribute_info_ {
    u2 attribute_name_index;
    u4 attribute_length;
    u1 * info; //u1 info[attribute_length]
} attribute_info;


typedef struct field_info_ {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    attribute_info * attributes; //attribute_info attributes[attributes_count]
}field_info;


typedef struct method_info_ {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    attribute_info * attributes;//attribute_info attributes[attributes_count]
}method_info;

u2 u2Read(FILE * ptr_file){
    return (getc(ptr_file) <<8)|getc(ptr_file);
}
        
u4 u4Read(FILE * ptr_file){
    u2 first_u2= u2Read(ptr_file);
    u2 second_u2= u2Read(ptr_file);
    return (first_u2 <<16) |second_u2;
}

u1 u1Read(FILE *ptr_file){
    return (u1)getc(ptr_file);
}


#endif