#include "printer.h"
#include "lib/types/consts.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <attribute.h>

static void printClassFileHeader(const ClassFile *cf);
static void printConstantPool(const ClassFile *cf);
static void printCpEntry(const cp_info * constant_pool,const cp_info *entry);
static void printAccessFlags(u2 flags);
static void printClassInfo(const ClassFile *cf);
static void print_CONSTANT_Class(const cp_info* constant_pool,const CONSTANT_Class_info *info);
static void print_CONSTANT_Fieldref(const cp_info* constant_pool, const CONSTANT_Fieldref_info *info);
static void print_CLASS_INFO_VALUE(const cp_info * constant_pool, const CONSTANT_Class_info * info);
static void print_CONSTANT_InterfaceMethodref(const cp_info* constant_pool,const CONSTANT_InterfaceMethodref_info *info);
static void print_CONSTANT_String(const cp_info* constant_pool, const CONSTANT_String_info *info);
static void print_CONSTANT_Integer(const CONSTANT_Integer_info *info);
static void print_CONSTANT_Float(const CONSTANT_Float_info *info);
static void print_CONSTANT_Long(const CONSTANT_Long_info *info);
static void print_CONSTANT_Double(const CONSTANT_Double_info *info);
static void print_CONSTANT_NameAndType(const cp_info * constant_pool, const CONSTANT_NameAndType_info *info);
static void print_CONSTANT_Utf8(CONSTANT_Utf8_info *info);
static void print_CONSTANT_MethodHandle(const CONSTANT_MethodHandle_info *info);
static void print_CONSTANT_MethodType(const CONSTANT_MethodType_info *info);
static void print_CONSTANT_InvokeDynamic(const CONSTANT_InvokeDynamic_info *info);
static void print_NAME_AND_TYPE_INFO_VALUE(const cp_info * constant_pool, const CONSTANT_NameAndType_info * info);
static void print_CONSTANT_Methodref(const cp_info* constant_pool, const CONSTANT_Methodref_info *info);
static int isKnownConstantTag(u1 tag);

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
    printf("Constant Pool Count: %hu\n", cf->constant_pool_count);
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
        if (!isKnownConstantTag(cf->constant_pool[i].tag)) {
            continue;
        }
        printf("[%hu] Tag: %hhu ", (u2)(i), cf->constant_pool[i].tag);
        printCpEntry(cf->constant_pool, &cf->constant_pool[i]);
    }
}




static void printCpEntry(const cp_info * constant_pool,const cp_info *entry) {
    switch (entry->tag) {
    case CONSTANT_Class:
        print_CONSTANT_Class(constant_pool,entry->constant_class_info); break;
    case CONSTANT_Fieldref:
        print_CONSTANT_Fieldref(constant_pool,entry->fieldRef_info); break;
    case CONSTANT_Methodref:
        print_CONSTANT_Methodref(constant_pool, entry->methodRef_info); break;
    case CONSTANT_InterfaceMethodref:
        print_CONSTANT_InterfaceMethodref(constant_pool,entry->interfaceMethod_info); break;
    case CONSTANT_String:
        print_CONSTANT_String(constant_pool,entry->string_info); break;
    case CONSTANT_Integer:
        print_CONSTANT_Integer(entry->integer_info); break;
    case CONSTANT_Float:
        print_CONSTANT_Float(entry->float_info); break;
    case CONSTANT_Long:
        print_CONSTANT_Long(entry->long_info); break;
    case CONSTANT_Double:
        print_CONSTANT_Double(entry->double_info); break;
    case CONSTANT_NameAndType:
        print_CONSTANT_NameAndType(constant_pool,entry->nameAndType_info); break;
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

static void print_CLASS_INFO_VALUE(const cp_info * constant_pool, const CONSTANT_Class_info * info){
    u1 * class_name = constant_pool[info->name_index].utf8_info->bytes;
    printf("\t%s.",class_name);
}

static void print_NAME_AND_TYPE_INFO_VALUE(const cp_info * constant_pool, const CONSTANT_NameAndType_info * info){
    u2 name_index = info->name_index;
    printf("%s:", constant_pool[name_index].utf8_info->bytes);
    u2 descritor_index = info->descriptor_index;
    printf("%s\n",constant_pool[descritor_index].utf8_info->bytes);
}

static void print_CONSTANT_Class(const cp_info* constant_pool,const CONSTANT_Class_info *info) {
    printf("<Class>\n");
    printf("\tname_index in constant_pool_table:%hu\n", info->name_index);
    CONSTANT_Utf8_info * utf8_info   = constant_pool[info->name_index].utf8_info;
    printf("\t%s\n",utf8_info->bytes);
}

static void print_CONSTANT_Fieldref(const cp_info* constant_pool, const CONSTANT_Fieldref_info *info) {
    printf("<Fieldref>\n");
    printf("\tclass_index:%hu\n", info->class_index);
    printf("\tname_and_type_index:%hu\n", info->name_and_type_index);
    CONSTANT_Class_info * class_info   = constant_pool[info->class_index].constant_class_info;
    print_CLASS_INFO_VALUE(constant_pool,class_info);
    CONSTANT_NameAndType_info *name_and_type =  constant_pool[info->name_and_type_index].nameAndType_info;
    print_NAME_AND_TYPE_INFO_VALUE(constant_pool,name_and_type);

}

static void print_CONSTANT_Methodref(const cp_info* constant_pool, const CONSTANT_Methodref_info *info) {
    printf("<Methodref>\n");
    printf("\tclass_index:%hu\n", info->class_index);
    printf("\tname_and_type_index:%hu\n", info->name_and_type_index);
    CONSTANT_Class_info * class_info   = constant_pool[info->class_index].constant_class_info;
    print_CLASS_INFO_VALUE(constant_pool,class_info);
    CONSTANT_NameAndType_info *name_and_type =  constant_pool[info->name_and_type_index].nameAndType_info;
    print_NAME_AND_TYPE_INFO_VALUE(constant_pool,name_and_type);
}

static void print_CONSTANT_InterfaceMethodref(const cp_info* constant_pool,const CONSTANT_InterfaceMethodref_info *info) {
    printf("<InterfaceMethodref>\n");
    printf("\tclass_index:%hu\n", info->class_index);
    printf("\tname_and_type_index:%hu\n", info->name_and_type_index);
    CONSTANT_Class_info * class_info   = constant_pool[info->class_index].constant_class_info;
    print_CLASS_INFO_VALUE(constant_pool,class_info);
    CONSTANT_NameAndType_info *name_and_type =  constant_pool[info->name_and_type_index].nameAndType_info;
    print_NAME_AND_TYPE_INFO_VALUE(constant_pool,name_and_type);
}

static void print_CONSTANT_String(const cp_info* constant_pool, const CONSTANT_String_info *info) {
    printf("<String>\n");
    printf("\tstring_index:%hu\n", info->string_index);
    CONSTANT_Utf8_info * utf8_info   = constant_pool[info->string_index].utf8_info;

    u1 * ptr = utf8_info->bytes;
    printf("\t");
    while (*ptr){
        if(*ptr=='\n'){
            printf("\\n");
        }
        else{
            printf("%c",*ptr);
        }
        ptr++;
    }
    printf("\n");
}

static void print_CONSTANT_Integer(const CONSTANT_Integer_info *info) {
    printf("<Integer>\n");
    printf("\tint_value:%u\n", info->bytes);
}

static void print_CONSTANT_Float(const CONSTANT_Float_info *info) {
    float value;
    memcpy(&value, &info->bytes, sizeof(float));
    printf("<Float>\n");
    printf("\tfloat_value: %f\n", value);
}

static void print_CONSTANT_Long(const CONSTANT_Long_info *info) {
    u8 bits = ((u8)info->high_bytes << 32) | info->low_bytes;
    long long value;
    memcpy(&value, &bits, sizeof(value));
    printf("<Long>\n");
    printf("\thigh_bytes: 0x%08X\n", info->high_bytes);
    printf("\tlow_bytes:  0x%08X\n", info->low_bytes);
    printf("\tlong_value: %lld\n", value);
}

static void print_CONSTANT_Double(const CONSTANT_Double_info *info) {
    u8 bits = ((u8)info->high_bytes << 32) | info->low_bytes;
    double value;
    memcpy(&value, &bits, sizeof(double));
    printf("<Double>\n");
    printf("\thigh_bytes: 0x%08X\n", info->high_bytes);
    printf("\tlow_bytes:  0x%08X\n", info->low_bytes);
    printf("\tdouble_value: %f\n", value);
}

static void print_CONSTANT_NameAndType(const cp_info * constant_pool, const CONSTANT_NameAndType_info *info) {
    printf("<NameAndType>\n");
    printf("\tname_index:%hu\n", info->name_index);
    printf("\tdescriptor_index:%hu\n", info->descriptor_index);
    printf("\t");
    print_NAME_AND_TYPE_INFO_VALUE(constant_pool,info);
}


static void print_CONSTANT_Utf8(CONSTANT_Utf8_info *info) {
    u1 * ptr = info->bytes;
    printf("<Utf8>\n");
    printf("\tlength of array in bytes: %hu\n", info->length);
    printf("\t");
    while (*ptr)
    {
        if(*ptr=='\n'){
            printf("\\n");
        }
        else{
            printf("%c",*ptr);
        }
        ptr++;
    }
    printf("\n");
    
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

void printMethods(const ClassFile *cf) {

    printf("=== Methods ===\n");

    if (cf->methods == NULL || cf->methods_count == 0) {

        printf("No methods found\n");

        return;
    }

    printf(
        "Total methods: %hu\n",
        cf->methods_count
    );

    for (
        u2 i = 0;
        i < cf->methods_count;
        i++
    ) {

        printf(
            "\n[Method %hu]\n",
            i
        );

        /*
         * ACCESS FLAGS
         */

        printf("  Access flags: ");

        printAccessFlags(
            cf->methods[i].access_flags
        );

        /*
         * METHOD NAME
         */

        u2 name_index =
            cf->methods[i].name_index;

        printf(
            "  Name index: %hu",
            name_index
        );

        if (name_index < cf->constant_pool_count) {

            CONSTANT_Utf8_info *method_name =

                cf->constant_pool[name_index]
                .utf8_info;

            if (method_name != NULL) {

                printf(
                    " (%s)",
                    method_name->bytes
                );
            }
        }

        printf("\n");

        /*
         * DESCRIPTOR
         */

        u2 desc_index =
            cf->methods[i]
            .descriptor_index;

        printf(
            "  Descriptor index: %hu",
            desc_index
        );

        if (desc_index < cf->constant_pool_count) {

            CONSTANT_Utf8_info *descriptor =

                cf->constant_pool[desc_index]
                .utf8_info;

            if (descriptor != NULL) {

                printf(
                    " (%s)",
                    descriptor->bytes
                );
            }
        }

        printf("\n");

        /*
         * ATTRIBUTES
         */

        printf(
            "  Attributes count: %hu\n",
            cf->methods[i]
            .attributes_count
        );

        if (
            cf->methods[i].attributes != NULL
            &&
            cf->methods[i].attributes_count > 0
        ) {

            for (
                u2 j = 0;
                j < cf->methods[i]
                    .attributes_count;
                j++
            ) {

                u2 attr_name_index =

                    cf->methods[i]
                    .attributes[j]
                    .attribute_name_index;

                char *attr_name =

                    getUtf8(
                        cf->constant_pool,
                        attr_name_index
                    );

                if (attr_name == NULL) {

                    printf(
                        "    [Attribute %hu] <invalid>\n",
                        j
                    );

                    continue;  
                }

                printf(
                    "    [Attribute %hu] %s\n",
                    j,
                    attr_name
                );

                void *info = cf->methods[i].attributes[j].info;

                if (strcmp(attr_name, "Code") == 0) {
                    printCodeAttribute(
                        (Code_attribute *) info,
                        cf->constant_pool
                    );
                } else if (strcmp(attr_name, "Exceptions") == 0) {
                    printExceptionsAttribute(
                        (Exceptions_attribute *) info,
                        cf->constant_pool
                    );
                } else {
                    printf("    (raw — %u bytes)\n",
                        cf->methods[i].attributes[j].attribute_length);
                }

                free(attr_name);
            }
        }
    }
}

void printFileToTerminal(int terminal_fd, const char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        const char *msg = "Warning: could not reopen output file for --print\n";
        write(terminal_fd, msg, strlen(msg));
        return;
    }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
        write(terminal_fd, buf, n);
    fclose(f);
}

static int isKnownConstantTag(u1 tag) {
    switch (tag) {
    case CONSTANT_Class:
    case CONSTANT_Fieldref:
    case CONSTANT_Methodref:
    case CONSTANT_InterfaceMethodref:
    case CONSTANT_String:
    case CONSTANT_Integer:
    case CONSTANT_Float:
    case CONSTANT_Long:
    case CONSTANT_Double:
    case CONSTANT_NameAndType:
    case CONSTANT_Utf8:
    case CONSTANT_MethodHandle:
    case CONSTANT_MethodType:
    case CONSTANT_InvokeDynamic:
        return 1;
    default:
        return 0;
    }
}