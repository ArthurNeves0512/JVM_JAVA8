#include "printer.h"
#include "lib/types/consts.h"
#include <stdio.h>
#include <string.h>

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
    SKIP_LINE;
    printConstantPool(cf);
    SKIP_LINE;
    printClassInfo(cf);
    SKIP_LINE;
}

static void printClassFileHeader(const ClassFile *cf) {
    printf("=== General Information ===\n");
    printf("Magic Number: %#X\n", cf->magic);
    printf("Minor Version: %hu\n", cf->minor_version);
    printf("Major Version: %hu\n", cf->major_version);
    printf("Constant Pool Count: %hu\n", cf->constant_pool_count - 1);
    printAccessFlags(cf->access_flags);
    printf("This Class: %hu\n", cf->this_class);
    printf("Super Class: %hu\n", cf->super_class);
    printf("Interfaces Count: %hu\n", cf->interfaces_count);
    printf("Fields Count: %hu\n", cf->fields_count);
    printf("Methods Count: %hu\n", cf->methods_count);
    printf("Attributes Count: %hu\n", cf->attributes_count);
}

static void printConstantPool(const ClassFile *cf) {
    printf("=== Constant Pool ===\n");
    if (cf->constant_pool == NULL) return;
    for (u2 i = 1; i < (u2)(cf->constant_pool_count); i++) {
        printf("[%hu] Tag: %hhu ", (u2)(i), cf->constant_pool[i].tag);
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
    case CONSTANT_LargeNumeric:
        printf("<LargeNumeric>\n"); break;
    default:
        printf("<unknown tag>\n"); break;
    }
}

static void print_CONSTANT_Class(const CONSTANT_Class_info *info) {
    printf("<Class>\n");
    printf("\tname_index in constant_pool_table:%hu\n", info->name_index);
}

static void print_CONSTANT_Fieldref(const CONSTANT_Fieldref_info *info) {
    printf("<Fieldref>\n");
    printf("\tclass_index:%hu\n", info->class_index);
    printf("\tname_and_type_index:%hu\n", info->name_and_type_index);
}

static void print_CONSTANT_Methodref(const CONSTANT_Methodref_info *info) {
    printf("<Methodref>\n");
    printf("\tclass_index:%hu\n", info->class_index);
    printf("\tname_and_type_index:%hu\n", info->name_and_type_index);
}

static void print_CONSTANT_InterfaceMethodref(const CONSTANT_InterfaceMethodref_info *info) {
    printf("<InterfaceMethodref>\n");
    printf("\tclass_index:%hu\n", info->class_index);
    printf("\tname_and_type_index:%hu\n", info->name_and_type_index);
}

static void print_CONSTANT_String(const CONSTANT_String_info *info) {
    printf("<String>\n");
    printf("\tstring_index:%hu\n", info->string_index);
}

static void print_CONSTANT_Integer(const CONSTANT_Integer_info *info) {
    printf("<Integer>\n");
    printf("\tbytes_value:%u\n", info->bytes);
}

static void print_CONSTANT_Float(const CONSTANT_Float_info *info) {
    float value;
    memcpy(&value, &info->bytes, sizeof(float));
    printf("<Float>\n");
    printf("\tbytes_value: %f\n", value);
}

static void print_CONSTANT_Long(const CONSTANT_Long_info *info) {
    printf("<Long>\n");
    u8 value = ((u8)info->high_bytes << 32) | info->low_bytes;
    printf("\tbytes_value:%llu\n", (unsigned long long)value);
}

static void print_CONSTANT_Double(const CONSTANT_Double_info *info) {
    u8 bits = ((u8)info->high_bytes << 32) | info->low_bytes;
    double value;
    memcpy(&value, &bits, sizeof(double));
    printf("<Double>\n");
    printf("\thigh_bytes: 0x%08X\n", info->high_bytes);
    printf("\tlow_bytes:  0x%08X\n", info->low_bytes);
    printf("\tbytes_value: %f\n", value);
}

static void print_CONSTANT_NameAndType(const CONSTANT_NameAndType_info *info) {
    printf("<NameAndType>\n");
    printf("\tname_index:%hu\n", info->name_index);
    printf("\tdescriptor_index:%hu\n", info->descriptor_index);
}

static void print_CONSTANT_Utf8(const CONSTANT_Utf8_info *info) {
    printf("<Utf8>\n");
    printf("\tlenght of array in bytes: %hu\n", info->length);
    printf("\tthe word: %s\n", info->bytes);
}

static void print_CONSTANT_MethodHandle(const CONSTANT_MethodHandle_info *info) {
    printf("<MethodHandle>\n");
    printf("\treference_kind:%u\n", info->reference_kind);
    printf("\treference_index:%u\n", info->reference_index);
}

static void print_CONSTANT_MethodType(const CONSTANT_MethodType_info *info) {
    printf("<MethodType>\n");
    printf("\tdescriptor_index:%hu\n", info->descriptor_index);
}

static void print_CONSTANT_InvokeDynamic(const CONSTANT_InvokeDynamic_info *info) {
    printf("<InvokeDynamic>\n");
    printf("\tbootstrap_method_attr_index: %hu\n", info->bootstrap_method_attr_index);
    printf("\tname_and_type_index: %hu\n", info->name_and_type_index);
}

static void printAccessFlags(u2 flags) {
    static const struct {
        u2          mask;
        const char *name;
    } FLAG_TABLE[] = {
        { ACC_PUBLIC,     "ACC_PUBLIC"     },
        { ACC_FINAL,      "ACC_FINAL"      },
        { ACC_SUPER,      "ACC_SUPER"      },
        { ACC_INTERFACE,  "ACC_INTERFACE"  },
        { ACC_ABSTRACT,   "ACC_ABSTRACT"   },
        { ACC_SYNTHETIC,  "ACC_SYNTHETIC"  },
        { ACC_ANNOTATION, "ACC_ANNOTATION" },
        { ACC_ENUM,       "ACC_ENUM"       },
    };
    static const size_t FLAG_COUNT = sizeof(FLAG_TABLE) / sizeof(FLAG_TABLE[0]);

    printf("Access flags (0x%04x): ", flags);

    int printed = 0;
    for (size_t i = 0; i < FLAG_COUNT; i++) {
        if (flags & FLAG_TABLE[i].mask) {
            printf("%s%s", printed ? " | " : "", FLAG_TABLE[i].name);
            printed++;
        }
    }

    if (!printed) printf("(none)");
    printf("\n");
}

static void printClassInfo(const ClassFile *cf) {
    printf("This class index at constant_pool table %d\n", cf->this_class);
    printf("This is the super class index at constant_pool table %d\n", cf->super_class);
}
