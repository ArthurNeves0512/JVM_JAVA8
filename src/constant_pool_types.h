#ifndef CONSTANT_POOL_TYPES
#define CONSTANT_POOL_TYPES
#include"dataTypes.h"



typedef struct CONSTANT_Class_info_ {
    u1 tag;
    u2 name_index;
}CONSTANT_Class_info;

typedef struct CONSTANT_Fieldref_info_{
    u1 tag;
    u2 class_index;
    u2 name_and_type_index;
}CONSTANT_Fieldref_info;

typedef struct CONSTANT_Methodref_info_ {
    u1 tag;
    u2 class_index;
    u2 name_and_type_index;
}CONSTANT_Methodref_info;

typedef struct CONSTANT_InterfaceMethodref_info_{
    u1 tag;
    u2 class_index;
    u2 name_and_type_index;
}CONSTANT_InterfaceMethodref_info;

typedef struct CONSTANT_String_info_{
    u1 tag;
    u2 string_index;
}CONSTANT_String_info;

typedef struct CONSTANT_Integer_info_{
    u1 tag;
    u4 bytes;
}CONSTANT_Integer_info;

typedef struct CONSTANT_Float_info_{
    u1 tag;
    u4 bytes;
}CONSTANT_Float_info;

typedef struct CONSTANT_Long_info_{
    u1 tag;
    u4 high_bytes;
    u4 low_bytes;
}CONSTANT_Long_info;

typedef struct CONSTANT_Double_info_{
    u1 tag;
    u4 high_bytes;
    u4 low_bytes;
}CONSTANT_Double_info;

typedef struct CONSTANT_NameAndType_info_{
    u1 tag;
    u2 name_index;
    u2 descriptor_index;
}CONSTANT_NameAndType_info;

typedef struct CONSTANT_Utf8_info_{
    u1 tag;
    u2 length;
    u1 * bytes;//u1  bytes[length]
}CONSTANT_Utf8_info;

typedef struct CONSTANT_MethodHandle_info_{
    u1 tag;
    u1 reference_kind;
    u2 reference_index;
}CONSTANT_MethodHandle_info;

typedef struct CONSTANT_MethodType_info_{
    u1 tag;
    u2 descriptor_index;
}CONSTANT_MethodType_info;

typedef struct CONSTANT_InvokeDynamic_info_{
    u1 tag;
    u2 bootstrap_method_attr_index;
    u2 name_and_type_index;
}CONSTANT_InvokeDynamic_info;



typedef struct cp_info_{
    u1 tag;
    union 
    {    
        CONSTANT_Class_info * constant_class_info;
        CONSTANT_Fieldref_info * fieldRef_info;
        CONSTANT_Methodref_info * methodRef_info;
        CONSTANT_InterfaceMethodref_info * interfaceMethod_info;
        CONSTANT_String_info * string_info;
        CONSTANT_Integer_info * integer_info;
        CONSTANT_Float_info * float_info;
        CONSTANT_Long_info * long_info;
        CONSTANT_Double_info * double_info;
        CONSTANT_NameAndType_info * nameAndType_info;
        CONSTANT_Utf8_info * utf8_info;
        CONSTANT_MethodHandle_info * methodHandle_info;
        CONSTANT_MethodType_info * methodType_info;
        CONSTANT_InvokeDynamic_info * invokeDynamic_info;
    };
    
}cp_info;

#endif