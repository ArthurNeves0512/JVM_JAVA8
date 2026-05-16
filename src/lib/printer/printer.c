#include "printer.h"
#include "lib/types/consts.h"
#include <stdio.h>

static void printClassFileHeader(const ClassFile *cf);
static void printConstantPool(const ClassFile *cf);
static void printCpEntry(const cp_info *entry);
static void printAccessFlags(u2 flags);
static void printClassInfo(const ClassFile *cf);

static void print_CONSTANT_Class(const CONSTANT_Class_info *info);
static void print_CONSTANT_Fieldref(const CONSTANT_Fieldref_info *info);
static void print_CONSTANT_Methodref(const CONSTANT_Methodref_info *info);
static void print_CONSTANT_InterfaceMethodref(const CONSTANT_InterfaceMethodref_info *info);
static void print_CONSTANT_String(const CONSTANT_String_info *info);
static void print_CONSTANT_Integer(const CONSTANT_Integer_info *info);
static void print_CONSTANT_Float(const CONSTANT_Float_info *info);
static void print_CONSTANT_Long(const CONSTANT_Long_info *info);
static void print_CONSTANT_Double(const CONSTANT_Double_info *info);
static void print_CONSTANT_NameAndType(const CONSTANT_NameAndType_info *info);
static void print_CONSTANT_Utf8(const CONSTANT_Utf8_info *info);
static void print_CONSTANT_MethodHandle(const CONSTANT_MethodHandle_info *info);
static void print_CONSTANT_MethodType(const CONSTANT_MethodType_info *info);
static void print_CONSTANT_InvokeDynamic(const CONSTANT_InvokeDynamic_info *info);

void printClassFile(const ClassFile *cf) {
    if (cf == NULL) {
        fprintf(stderr, "Error: NULL ClassFile\n");
        return;
    }
    printClassFileHeader(cf);
    printConstantPool(cf);
    printAccessFlags(cf->access_flags);
    printClassInfo(cf);
}

static void printClassFileHeader(const ClassFile *cf) {
    printf("Magic Number: %#X\n", cf->magic);
    printf("Java class file version:%hu.%hu\n", cf->major_version, cf->minor_version);
    printf("Constant Pool count: %hu\n", cf->constant_pool_count);
}

static void printConstantPool(const ClassFile *cf) {
    if (cf->constant_pool == NULL) return;
    for (u2 i = 0; i < (u2)(cf->constant_pool_count - 1); i++) {
        printf("--------------------------\n");
        printf("[%hu] Tag: %hhu ", (u2)(i + 1), cf->constant_pool[i].tag);
        printCpEntry(&cf->constant_pool[i]);
    }
}

static void printCpEntry(const cp_info *entry) {
    switch (entry->tag) {
    case CONSTANT_Class:
        print_CONSTANT_Class(entry->constant_class_info); break;
    case CONSTANT_Fieldref:
        print_CONSTANT_Fieldref(entry->fieldRef_info); break;
    case CONSTANT_Methodref:
        print_CONSTANT_Methodref(entry->methodRef_info); break;
    case CONSTANT_InterfaceMethodref:
        print_CONSTANT_InterfaceMethodref(entry->interfaceMethod_info); break;
    case CONSTANT_String:
        print_CONSTANT_String(entry->string_info); break;
    case CONSTANT_Integer:
        print_CONSTANT_Integer(entry->integer_info); break;
    case CONSTANT_Float:
        print_CONSTANT_Float(entry->float_info); break;
    case CONSTANT_Long:
        print_CONSTANT_Long(entry->long_info); break;
    case CONSTANT_Double:
        print_CONSTANT_Double(entry->double_info); break;
    case CONSTANT_NameAndType:
        print_CONSTANT_NameAndType(entry->nameAndType_info); break;
    case CONSTANT_Utf8:
        print_CONSTANT_Utf8(entry->utf8_info); break;
    case CONSTANT_MethodHandle:
        print_CONSTANT_MethodHandle(entry->methodHandle_info); break;
    case CONSTANT_MethodType:
        print_CONSTANT_MethodType(entry->methodType_info); break;
    case CONSTANT_InvokeDynamic:
        print_CONSTANT_InvokeDynamic(entry->invokeDynamic_info); break;
    default:
        printf("<unknown tag>\n"); break;
    }
}

static void print_CONSTANT_Class(const CONSTANT_Class_info *info) {
    printf("<Class>\n");
    printf("name_index in constant_pool_table:%hu\n", info->name_index);
}

static void print_CONSTANT_Fieldref(const CONSTANT_Fieldref_info *info) {
    printf("<Fieldref>\n");
    printf("class_index:%hu\n", info->class_index);
    printf("name_and_type_index:%hu\n", info->name_and_type_index);
}

static void print_CONSTANT_Methodref(const CONSTANT_Methodref_info *info) {
    printf("<Methodref>\n");
    printf("class_index:%hu\n", info->class_index);
    printf("name_and_type_index:%hu\n", info->name_and_type_index);
}

static void print_CONSTANT_InterfaceMethodref(const CONSTANT_InterfaceMethodref_info *info) {
    printf("<InterfaceMethodref>\n");
    printf("class_index:%hu\n", info->class_index);
    printf("name_and_type_index:%hu\n", info->name_and_type_index);
}

static void print_CONSTANT_String(const CONSTANT_String_info *info) {
    printf("<String>\n");
    printf("string_index:%hu\n", info->string_index);
}

static void print_CONSTANT_Integer(const CONSTANT_Integer_info *info) {
    printf("<Integer>\n");
    printf("bytes_value:%u\n", info->bytes);
}

static void print_CONSTANT_Float(const CONSTANT_Float_info *info) {
    printf("<Float>\n");
    printf("PRECISO AINDA CONVERTER PARA FLOAT:%u\n", info->bytes);
}

static void print_CONSTANT_Long(const CONSTANT_Long_info *info) {
    printf("<Long>\n");
    u8 value = ((u8)info->high_bytes << 32) | info->low_bytes;
    printf("bytes_value:%llu\n", (unsigned long long)value);
}

static void print_CONSTANT_Double(const CONSTANT_Double_info *info) {
    printf("<Double>\n");
    u8 value = ((u8)info->high_bytes << 32) | info->low_bytes;
    printf("PRECISO CONVERTER AINDA PARA DOUBLE:%llu\n", (unsigned long long)value);
}

static void print_CONSTANT_NameAndType(const CONSTANT_NameAndType_info *info) {
    printf("<NameAndType>\n");
    printf("name_index:%hu\n", info->name_index);
    printf("descriptor_index:%hu\n", info->descriptor_index);
}

static void print_CONSTANT_Utf8(const CONSTANT_Utf8_info *info) {
    printf("<Utf8>\n");
    printf("lenght of array in bytes: %hu\n", info->length);
    printf("the word: %s\n", info->bytes);
}

static void print_CONSTANT_MethodHandle(const CONSTANT_MethodHandle_info *info) {
    printf("<MethodHandle>\n");
    printf("reference_kind:%u\n", info->reference_kind);
    printf("reference_index:%u\n", info->reference_index);
}

static void print_CONSTANT_MethodType(const CONSTANT_MethodType_info *info) {
    printf("<MethodType>\n");
    printf("descriptor_index:%hu\n", info->descriptor_index);
}

static void print_CONSTANT_InvokeDynamic(const CONSTANT_InvokeDynamic_info *info) {
    printf("<InvokeDynamic>\n");
    printf("bootstrap_method_attr_index: %hu\n", info->bootstrap_method_attr_index);
    printf("name_and_type_index: %hu\n", info->name_and_type_index);
}

static void printAccessFlags(u2 flags) {
    printf("this is the flag value %04x\n", flags);
    if ((flags & 0x0001) == 0x0001) printf("ACC_PUBLIC, ");
    if ((flags & 0x0010) == 0x0010) printf("ACC_FINAL, ");
    if ((flags & 0x0020) == 0x0020) printf("ACC_SUPPER, ");
    if ((flags & 0x0200) == 0x0200) printf("ACC_INTERFACE, ");
    if ((flags & 0x0400) == 0x0400) printf("ACC_ABSTRACT, ");
    if ((flags & 0x1000) == 0x1000) printf("ACC_SYNTHETIC, ");
    if ((flags & 0x2000) == 0x2000) printf("ACC_ANNOTATION, ");
    if ((flags & 0x4000) == 0x4000) printf("ACC_ENUM, ");
    printf("\n");
}

static void printClassInfo(const ClassFile *cf) {
    printf("this class index at constant_pool table %d\n", cf->this_class);
    printf("this is the super class index at constant_pool table %d\n", cf->super_class);
    printf("number of interfaces %d\n", cf->interfaces_count);
}
