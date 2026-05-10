#ifndef CONSTANT_POOL_H
#define CONSTANT_POOL_H
#include "cp_info.h"

typedef struct cp_info_ {
    u1 tag;
    union {
        CONSTANT_Class_info *constant_class_info;
        CONSTANT_Fieldref_info *fieldRef_info;
        CONSTANT_Methodref_info *methodRef_info;
        CONSTANT_InterfaceMethodref_info *interfaceMethod_info;
        CONSTANT_String_info *string_info;
        CONSTANT_Integer_info *integer_info;
        CONSTANT_Float_info *float_info;
        CONSTANT_Long_info *long_info;
        CONSTANT_Double_info *double_info;
        CONSTANT_NameAndType_info *nameAndType_info;
        CONSTANT_Utf8_info *utf8_info;
        CONSTANT_MethodHandle_info *methodHandle_info;
        CONSTANT_MethodType_info *methodType_info;
        CONSTANT_InvokeDynamic_info *invokeDynamic_info;
    };
} cp_info;

#endif
