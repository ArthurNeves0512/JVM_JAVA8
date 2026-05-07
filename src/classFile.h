#ifndef CLASS_FILE_H
#define CLASS_FILE_H

#include "constant_pool_types.h"
#include "lib/consts.h"
#include "lib/dataTypes.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct ClassFile_ {
  u4 magic;
  u2 minor_version;
  u2 major_version;
  u2 constant_pool_count;
  cp_info *constant_pool; // constant_pool[constant_pool_count-1];
  u2 access_flags;
  u2 this_class;
  u2 super_class;
  u2 interfaces_count;
  u2 *interfaces; // u2 interfaces[interfaces_count];
  u2 fields_count;
  field_info *fields; // field_info fields[fields_count];
  u2 methods_count;
  method_info *methods;
  u2 attributes_count;
  attribute_info *attributes;
} ClassFile;

u4 readCafeBabe(FILE *ptr_file) { return u4Read(ptr_file); }
static u2 readMinorVersion(FILE *ptr_file) { return u2Read(ptr_file); }
static u2 readMajorVersion(FILE *ptr_file) { return u2Read(ptr_file); }

void readPrintMinorMajorVersion(FILE *ptr_file) {
  u2 minor_version = readMinorVersion(ptr_file);
  u2 major_version = readMajorVersion(ptr_file);
  printf("these are the major "
         "and minor version of the class file: "
         "%d.%d\n",
         major_version, minor_version);
}

u2 readConstantPoolCount(FILE *ptr_file) { return u2Read(ptr_file); }

u2 readAcessModifiers(FILE *ptr_file) {
  /*
  a ideia aqui é que vamos ter 2 bytes (0x0010)
  e iremos fazer uma mascara para saber, quais são
  os modificadores.

  */
  return u2Read(ptr_file);
}

/*
Nesse caso apontaremos para um índice valido na tabela de constantes
*/
void read_CONSTANT_Class(FILE *ptr_file) {
  printf("<Class>\n");
  printf("name_index in constant_pool_table:%hu\n", u2Read(ptr_file));
}
void read_CONSTANT_Fieldref(FILE *ptr_file) {
  printf("<Fieldref>\n");
  printf("class_index:%hu\n", u2Read(ptr_file));
  printf("name_and_type_index:%hu\n", u2Read(ptr_file));
}
void read_CONSTANT_Methodref(FILE *ptr_file) {
  printf("<Methodref>\n");
  printf("class_index:%hu\n", u2Read(ptr_file));
  printf("name_and_type_index:%hu\n", u2Read(ptr_file));
}
void read_CONSTANT_InterfaceMethodref(FILE *ptr_file) {
  printf("<InterfaceMethodref>\n");
  printf("class_index:%hu\n", u2Read(ptr_file));
  printf("name_and_type_index:%hu\n", u2Read(ptr_file));
}

void read_CONSTANT_String(FILE *ptr_file) {
  printf("<String>\n");
  printf("string_index:%hu\n", u2Read(ptr_file));
}
void read_CONSTANT_Integer(FILE *ptr_file) {
  printf("<Integer>\n");
  printf("bytes_value:%hu\n", u4Read(ptr_file));
}
/*
Ainda preciso calcular o jeito certo, convertendo os bytes para float
*/
void read_CONSTANT_Float(FILE *ptr_file) {
  printf("<Float>\n");
  printf("PRECISO AINDA CONVERTER PARA FLOAT:%d\n", u4Read(ptr_file));
}

void read_CONSTANT_Long(FILE *ptr_file) {
  printf("<Long>\n");
  u4 high_bytes = u4Read(ptr_file);
  u4 low_bytes = u4Read(ptr_file);
  u4 long_bytes = ((high_bytes << 32) | low_bytes);
  printf("bytes_value:%d\n", long_bytes);
}

void read_CONSTANT_Double(FILE *ptr_file) {
  printf("<Double>\n");
  u4 high_bytes = u4Read(ptr_file);
  u4 low_bytes = u4Read(ptr_file);
  u4 long_bytes = ((high_bytes << 32) | low_bytes);
  printf("PRECISO CONVERTER AINDA PARA DOUBLE:%d\n", long_bytes);
}
void read_CONSTANT_NameAndType(FILE *ptr_file) {
  printf("<NameAndType>\n");
  u2 name_index = u2Read(ptr_file);
  u2 descriptor_index = u2Read(ptr_file);
  printf("name_index:%hu\n", name_index);
  printf("descriptor_index:%hu\n", descriptor_index);
}
void read_CONSTANT_Utf8(FILE *ptr_file) {
  printf("<Utf8>\n");
  u2 lenght = u2Read(ptr_file);
  u1 *bytes = (u1 *)malloc(lenght);
  u2 iterator = 0;
  u1 *start = bytes;
  while (iterator < lenght) {
    bytes[iterator] = u1Read(ptr_file);
    iterator++;
  }
  printf("lenght of array in bytes: %hu\n", lenght);
  printf("the word: %s\n", bytes);
}
void read_CONSTANT_MethodHandle(FILE *ptr_file) {
  printf("<MethodHandle>\n");
  u1 reference_kind = u1Read(ptr_file);
  u2 reference_index = u2Read(ptr_file);
  printf("reference_kind:%u\n", reference_kind);
  printf("reference_index:%u\n", reference_kind);
}
void read_CONSTANT_MethodType(FILE *ptr_file) {
  printf("<MethodType>\n");
  u2 descriptor_index = u2Read(ptr_file);
  printf("descriptor_index:%hu\n", descriptor_index);
}

void read_CONSTANT_InvokeDynamic(FILE *ptr_file) {
  printf("<InvokeDynamic>\n");
  u2 bootstrap_method_attr_index = u2Read(ptr_file);
  u2 name_and_type_index = u2Read(ptr_file);
  printf("bootstrap_method_attr_index: %hu\n", bootstrap_method_attr_index);
  printf("name_and_type_index: %hu\n", name_and_type_index);
}

void constant_pool(u2 tagByte, FILE *ptr_file) {
  switch (tagByte) {
  case CONSTANT_Class:
    read_CONSTANT_Class(ptr_file);
    break;
  case CONSTANT_Fieldref:
    read_CONSTANT_Fieldref(ptr_file);
    break;

  case CONSTANT_Methodref:
    read_CONSTANT_Methodref(ptr_file);
    break;

  case CONSTANT_InterfaceMethodref:
    read_CONSTANT_InterfaceMethodref(ptr_file);
    break;

  case CONSTANT_String:
    read_CONSTANT_String(ptr_file);
    break;

  case CONSTANT_Integer:
    read_CONSTANT_Integer(ptr_file);
    break;

  case CONSTANT_Float:
    read_CONSTANT_Float(ptr_file);
    break;

  case CONSTANT_Long:
    read_CONSTANT_Long(ptr_file);
    break;

  case CONSTANT_Double:
    read_CONSTANT_Double(ptr_file);
    break;

  case CONSTANT_NameAndType:
    read_CONSTANT_NameAndType(ptr_file);
    break;

  case CONSTANT_Utf8:
    read_CONSTANT_Utf8(ptr_file);
    break;

  case CONSTANT_MethodHandle:
    read_CONSTANT_MethodHandle(ptr_file);
    break;

  case CONSTANT_MethodType:
    read_CONSTANT_MethodType(ptr_file);
    break;

  case CONSTANT_InvokeDynamic:
    read_CONSTANT_InvokeDynamic(ptr_file);
    break;

  default:
    break;
  }
}

void readAcessFlags(ClassFile *clas_file_ptr, FILE *file_ptr) {
  u2 flag = u2Read(file_ptr);
  printf("this is the flag value %04x\n", flag);
  if ((flag & 0x01) == 0x01)
    printf("ACC_PUBLIC, ");
  if ((flag & 0x0010) == 0x0010)
    printf("ACC_FINAL, ");
  if ((flag & 0x0020) == 0x0020)
    printf("ACC_SUPPER, ");
  if ((flag & 0x0200) == 0x0200)
    printf("ACC_INTERFACE, ");
  if ((flag & 0x0400) == 0x0400)
    printf("ACC_ABSTRACT, ");
  if ((flag & 0x1000) == 0x1000)
    printf("ACC_SYNTHETIC, ");
  if ((flag & 0x2000) == 0x2000)
    printf("ACC_ANNOTATION, ");
  if ((flag & 0x4000) == 0x4000)
    printf("ACC_ENUM, ");
  printf("\n");
  clas_file_ptr->access_flags = flag;
}

void readThisClass(ClassFile *class_file_ptr, FILE *file_ptr) {

  u2 thisClass = u2Read(file_ptr);
  printf("this class index at constant_pool table %d", thisClass);
  class_file_ptr->this_class = thisClass;
}

void readSuperClass(ClassFile *class_file_ptr, FILE *file_ptr) {

  u2 superClass = u2Read(file_ptr);
  printf("this is the super class index at constant_pool table %d", superClass);
  class_file_ptr->this_class = superClass;
}

void readInterfacesCount(ClassFile *class_file_ptr, FILE *file_ptr) {

  u2 interfacesCount = u2Read(file_ptr);
  printf("number of interfaces %d", interfacesCount);
  class_file_ptr->this_class = interfacesCount;
}

void classFilesSetup(ClassFile *clas_file_ptr, FILE *file_ptr) {
  clas_file_ptr->magic = readCafeBabe(file_ptr);
  printf("Magic Number: %#X\n", clas_file_ptr->magic);
  clas_file_ptr->minor_version = readMinorVersion(file_ptr);

  clas_file_ptr->major_version = readMajorVersion(file_ptr);
  printf("Java class file version:%hu.%hu\n", clas_file_ptr->major_version,
         clas_file_ptr->minor_version);
  /*
  No caso atual é 29, logo iremos ter que iterar de 0->28(ah nao ser que seja
  LONG ou Double, eai assim, iremos de 0->29)
  */
  clas_file_ptr->constant_pool_count = readConstantPoolCount(file_ptr);
  printf("Constant Pool count: %hu\n", clas_file_ptr->constant_pool_count);
  u2 constant_pool_iterator = 1;

  while (constant_pool_iterator < clas_file_ptr->constant_pool_count) {
    u1 tagByte = u1Read(file_ptr);
    printf("--------------------------\n");
    printf("[%hu] Tag: %hhu ", constant_pool_iterator, tagByte);
    constant_pool(tagByte, file_ptr);
    constant_pool_iterator++;
  }
  readAcessFlags(clas_file_ptr, file_ptr);
}

#endif
