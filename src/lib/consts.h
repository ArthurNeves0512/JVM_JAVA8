#ifndef CONST_H
#define CONST_H
#include"lib/dataTypes.h"

enum CONSTANT_POOL_TAGS{
    CONSTANT_Class =  (u1) 7,
    CONSTANT_Fieldref = (u1) 9,
    CONSTANT_Methodref = (u1) 10,
    CONSTANT_InterfaceMethodref = (u1) 11,
    CONSTANT_String = (u1) 8,
    CONSTANT_Integer = (u1) 3,
    CONSTANT_Float = (u1) 4,
    CONSTANT_Long = (u1) 5,
    CONSTANT_Double = (u1) 6,
    CONSTANT_NameAndType = (u1) 12,
    CONSTANT_Utf8 = (u1) 1,
    CONSTANT_MethodHandle = (u1) 15,
    CONSTANT_MethodType = (u1) 16,
    CONSTANT_InvokeDynamic = (u1) 18,
};

enum  ACESS_MODIFIERS{
    /*
    Declared public; may be accessed from outside its package.
    */
    ACC_PUBLIC = (u2) 0x0001,
    /*
    Declared final; no subclasses allowed.
    */
    ACC_FINAL =(u2) 0x0010,
    /*
    Treat superclass methods specially when invoked by
    the invokespecial instruction
    */
    ACC_SUPER =(u2) 0x0020,
    ACC_INTERFACE =(u2) 0x0200,
    ACC_ABSTRACT =(u2) 0x0400,
    ACC_SYNTHETIC =(u2) 0x1000,
    ACC_ANNOTATION =(u2) 0x2000,
    ACC_ENUM =(u2) 0x4000
};

#endif
