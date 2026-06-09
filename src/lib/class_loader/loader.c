#include "loader.h"
#include <stdlib.h>

u4 readCafeBabe(FILE *ptr_file) {
    u4 magic = u4Read(ptr_file);
    if (magic != 0xCAFEBABE) {
        fprintf(stderr, "Error: Invalid magic number: %#X\n", magic);
        exit(EXIT_FAILURE);
    } 
    return magic;
}

static u2 readMinorVersion(FILE *ptr_file) { return u2Read(ptr_file); }
static u2 readMajorVersion(FILE *ptr_file) { return u2Read(ptr_file); }

u2 readConstantPoolCount(FILE *ptr_file) { return u2Read(ptr_file); }

static void read_CONSTANT_Class(cp_info *entry, FILE *fp) {
    CONSTANT_Class_info *info = malloc(sizeof(CONSTANT_Class_info));
    info->tag        = entry->tag;
    info->name_index = u2Read(fp);
    entry->constant_class_info = info;
}

static void read_CONSTANT_Fieldref(cp_info *entry, FILE *fp) {
    CONSTANT_Fieldref_info *info = malloc(sizeof(CONSTANT_Fieldref_info));
    info->tag                 = entry->tag;
    info->class_index         = u2Read(fp);
    info->name_and_type_index = u2Read(fp);
    entry->fieldRef_info = info;
}

static void read_CONSTANT_Methodref(cp_info *entry, FILE *fp) {
    CONSTANT_Methodref_info *info = malloc(sizeof(CONSTANT_Methodref_info));
    info->tag                 = entry->tag;
    info->class_index         = u2Read(fp);
    info->name_and_type_index = u2Read(fp);
    entry->methodRef_info = info;
}

static void read_CONSTANT_InterfaceMethodref(cp_info *entry, FILE *fp) {
    CONSTANT_InterfaceMethodref_info *info = malloc(sizeof(CONSTANT_InterfaceMethodref_info));
    info->tag                 = entry->tag;
    info->class_index         = u2Read(fp);
    info->name_and_type_index = u2Read(fp);
    entry->interfaceMethod_info = info;
}

static void read_CONSTANT_String(cp_info *entry, FILE *fp) {
    CONSTANT_String_info *info = malloc(sizeof(CONSTANT_String_info));
    info->tag          = entry->tag;
    info->string_index = u2Read(fp);
    entry->string_info = info;
}

static void read_CONSTANT_Integer(cp_info *entry, FILE *fp) {
    CONSTANT_Integer_info *info = malloc(sizeof(CONSTANT_Integer_info));
    info->tag   = entry->tag;
    info->bytes = u4Read(fp);
    entry->integer_info = info;
}

static void read_CONSTANT_Float(cp_info *entry, FILE *fp) {
    CONSTANT_Float_info *info = malloc(sizeof(CONSTANT_Float_info));
    info->tag   = entry->tag;
    info->bytes = u4Read(fp);
    entry->float_info = info;
}

static void read_CONSTANT_Long(cp_info *entry, FILE *fp) {
    CONSTANT_Long_info *info = malloc(sizeof(CONSTANT_Long_info));
    info->tag        = entry->tag;
    info->high_bytes = u4Read(fp);
    info->low_bytes  = u4Read(fp);
    entry->long_info = info;
}

static void read_CONSTANT_Double(cp_info *entry, FILE *fp) {
    CONSTANT_Double_info *info = malloc(sizeof(CONSTANT_Double_info));
    info->tag        = entry->tag;
    info->high_bytes = u4Read(fp);
    info->low_bytes  = u4Read(fp);
    entry->double_info = info;
}

static void read_CONSTANT_NameAndType(cp_info *entry, FILE *fp) {
    CONSTANT_NameAndType_info *info = malloc(sizeof(CONSTANT_NameAndType_info));
    info->tag              = entry->tag;
    info->name_index       = u2Read(fp);
    info->descriptor_index = u2Read(fp);
    entry->nameAndType_info = info;
}

static void read_CONSTANT_Utf8(cp_info *entry, FILE *fp) {
    CONSTANT_Utf8_info *info = malloc(sizeof(CONSTANT_Utf8_info));
    info->tag    = entry->tag;
    info->length = u2Read(fp);
    info->bytes  = malloc(info->length + 1);
    for (u2 i = 0; i < info->length; i++) {
        info->bytes[i] = u1Read(fp);
    }
    info->bytes[info->length] = '\0';
    entry->utf8_info = info;
}

static void read_CONSTANT_MethodHandle(cp_info *entry, FILE *fp) {
    CONSTANT_MethodHandle_info *info = malloc(sizeof(CONSTANT_MethodHandle_info));
    info->tag             = entry->tag;
    info->reference_kind  = u1Read(fp);
    info->reference_index = u2Read(fp);
    entry->methodHandle_info = info;
}

static void read_CONSTANT_MethodType(cp_info *entry, FILE *fp) {
    CONSTANT_MethodType_info *info = malloc(sizeof(CONSTANT_MethodType_info));
    info->tag              = entry->tag;
    info->descriptor_index = u2Read(fp);
    entry->methodType_info = info;
}

static void read_CONSTANT_InvokeDynamic(cp_info *entry, FILE *fp) {
    CONSTANT_InvokeDynamic_info *info = malloc(sizeof(CONSTANT_InvokeDynamic_info));
    info->tag                          = entry->tag;
    info->bootstrap_method_attr_index  = u2Read(fp);
    info->name_and_type_index          = u2Read(fp);
    entry->invokeDynamic_info = info;
}

static void constant_pool(cp_info *entry, u1 tag, FILE *fp) {
    entry->tag = tag;
    switch (tag) {
    case CONSTANT_Class:            read_CONSTANT_Class(entry, fp);            break;
    case CONSTANT_Fieldref:         read_CONSTANT_Fieldref(entry, fp);         break;
    case CONSTANT_Methodref:        read_CONSTANT_Methodref(entry, fp);        break;
    case CONSTANT_InterfaceMethodref: read_CONSTANT_InterfaceMethodref(entry, fp); break;
    case CONSTANT_String:           read_CONSTANT_String(entry, fp);           break;
    case CONSTANT_Integer:          read_CONSTANT_Integer(entry, fp);          break;
    case CONSTANT_Float:            read_CONSTANT_Float(entry, fp);            break;
    case CONSTANT_Long:             read_CONSTANT_Long(entry, fp);             break;
    case CONSTANT_Double:           read_CONSTANT_Double(entry, fp);           break;
    case CONSTANT_NameAndType:      read_CONSTANT_NameAndType(entry, fp);      break;
    case CONSTANT_Utf8:             read_CONSTANT_Utf8(entry, fp);             break;
    case CONSTANT_MethodHandle:     read_CONSTANT_MethodHandle(entry, fp);     break;
    case CONSTANT_MethodType:       read_CONSTANT_MethodType(entry, fp);       break;
    case CONSTANT_InvokeDynamic:    read_CONSTANT_InvokeDynamic(entry, fp);    break;
    default:                                                                    break;
    }
}

void readAccessFlags(ClassFile *cf, FILE *fp) {
    cf->access_flags = u2Read(fp);
}

void readThisClass(ClassFile *cf, FILE *fp) {
    cf->this_class = u2Read(fp);
}

void readSuperClass(ClassFile *cf, FILE *fp) {
    cf->super_class = u2Read(fp);
}

void readInterfacesCount(ClassFile *cf, FILE *fp) {
    cf->interfaces_count = u2Read(fp);
}

void classFilesSetup(ClassFile *cf, FILE *fp) {
    cf->magic               = readCafeBabe(fp);
    cf->minor_version       = readMinorVersion(fp);
    cf->major_version       = readMajorVersion(fp);
    cf->constant_pool_count = readConstantPoolCount(fp);

    cf->constant_pool = malloc((cf->constant_pool_count) * sizeof(cp_info));
    for (u2 i = 1; i < (u2)(cf->constant_pool_count); i++) {
        u1 tag = u1Read(fp);
        if (tag == CONSTANT_Long || tag == CONSTANT_Double) {
            constant_pool(&cf->constant_pool[i], tag, fp);
            cf->constant_pool[++i].tag = -1;
        } else {
            constant_pool(&cf->constant_pool[i], tag, fp);
        }
    }

    readAccessFlags(cf, fp);
    readThisClass(cf, fp);
    readSuperClass(cf, fp);
    }
    void readClassFileAttributes(ClassFile *cf, FILE *fp) {
    cf->attributes_count = u2Read(fp);
 
    if (cf->attributes_count > 0) {
        cf->attributes =
            (attribute_info *) malloc(
                cf->attributes_count * sizeof(attribute_info)
            );
        for (u2 i = 0; i < cf->attributes_count; i++)
            cf->attributes[i] = readAttribute(fp, cf->constant_pool);
    } else {
        cf->attributes = NULL;
    }
}
