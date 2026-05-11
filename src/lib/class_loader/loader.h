#ifndef LOADER_H
#define LOADER_H
#include <stdio.h>
#include "lib/types/class_file/dot_class.h"
#include "lib/types/consts.h"
#include "lib/file/read_byte.h"

u4 readCafeBabe(FILE *ptr_file);
u2 readConstantPoolCount(FILE *ptr_file);
u2 readAcessModifiers(FILE *ptr_file);

void readPrintMinorMajorVersion(FILE *ptr_file);

void read_CONSTANT_Class(FILE *ptr_file);
void read_CONSTANT_Fieldref(FILE *ptr_file);
void read_CONSTANT_Methodref(FILE *ptr_file);
void read_CONSTANT_InterfaceMethodref(FILE *ptr_file);
void read_CONSTANT_String(FILE *ptr_file);
void read_CONSTANT_Integer(FILE *ptr_file);
void read_CONSTANT_Float(FILE *ptr_file);
void read_CONSTANT_Long(FILE *ptr_file);
void read_CONSTANT_Double(FILE *ptr_file);
void read_CONSTANT_NameAndType(FILE *ptr_file);
void read_CONSTANT_Utf8(FILE *ptr_file);
void read_CONSTANT_MethodHandle(FILE *ptr_file);
void read_CONSTANT_MethodType(FILE *ptr_file);
void read_CONSTANT_InvokeDynamic(FILE *ptr_file);

void constant_pool(u2 tagByte, FILE *ptr_file);

void readAcessFlags(ClassFile *clas_file_ptr, FILE *file_ptr);
void readThisClass(ClassFile *class_file_ptr, FILE *file_ptr);
void readSuperClass(ClassFile *class_file_ptr, FILE *file_ptr);
void readInterfacesCount(ClassFile *class_file_ptr, FILE *file_ptr);

void classFilesSetup(ClassFile *clas_file_ptr, FILE *file_ptr);

#endif
