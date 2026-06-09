#ifndef FIELDS_INTERFACES_H
#define FIELDS_INTERFACES_H

#include "loader.h"
#include <string.h>
static void printAcessFlags(const field_info *info);
void readInterfaces(ClassFile *class_file_ptr, FILE *file_ptr)
{
    class_file_ptr->interfaces_count = u2Read(file_ptr);
    printf("\n=== Interfaces ===\n");
    printf("interfaces_count: %hu\n", class_file_ptr->interfaces_count);

    class_file_ptr->interfaces = (u2 *)malloc(class_file_ptr->interfaces_count * sizeof(u2));
    for (u2 i = 0; i < class_file_ptr->interfaces_count; i++)
    {
        u2 index = u2Read(file_ptr);
        class_file_ptr->interfaces[i] = index;
        CONSTANT_Class_info *class_info = class_file_ptr->constant_pool[index].constant_class_info;
        CONSTANT_Utf8_info *utf8_info = class_file_ptr->constant_pool[class_info->name_index].utf8_info;
        printf("interface[%hu]:\n", i);
        printf("\tindex at constant_pool: #%hu\n", index);
        printf("\tvalue: %s\n", utf8_info->bytes);
    }
}

static void printAcessFlags(const field_info *info)
{
    const struct
    {
        const u2 acess_tag;
        const char *name;
    } ACESS_FLAG_TABLE[] = {
        {ACC_PUBLIC, "ACC_PUBLIC"},
        {ACC_PRIVATE, "ACC_PRIVATE"},
        {ACC_PROTECTED, "ACC_PROTECTED"},
        {ACC_STATIC, "ACC_STATIC"},
        {ACC_FINAL, "ACC_FINAL"},
        {ACC_VOLATILE, "ACC_VOLATILE"},
        {ACC_TRANSIENT, "ACC_TRANSIENT"},
        {ACC_SUPER, "ACC_SUPER"},
        {ACC_INTERFACE, "ACC_INTERFACE"},
        {ACC_ABSTRACT, "ACC_ABSTRACT"},
        {ACC_SYNTHETIC, "ACC_SYNTHETIC"},
        {ACC_ANNOTATION, "ACC_ANNOTATION"},
        {ACC_ENUM, "ACC_ENUM"}};
    const uint8_t size = sizeof(ACESS_FLAG_TABLE) / sizeof(ACESS_FLAG_TABLE[0]);
    int printed = 0;
    for (uint8_t i = 0; i < size; i++)
    {
        if (info->access_flags & ACESS_FLAG_TABLE[i].acess_tag)
        {
            printf("%s%s", printed ? " | " : "", ACESS_FLAG_TABLE[i].name);
            printed++;
        }
    }
    printf("\n");
}

void readFields(ClassFile *class_file_ptr, FILE *file_ptr)
{

    class_file_ptr->fields_count = u2Read(file_ptr);

    printf("\n=== Fields ===\n");
    printf("fields_count: %hu\n",
           class_file_ptr->fields_count);

    class_file_ptr->fields =
        (field_info *)malloc(
            class_file_ptr->fields_count *
            sizeof(field_info));

    for (u2 i = 0;
         i < class_file_ptr->fields_count;
         i++)
    {

        field_info *field =
            &class_file_ptr->fields[i];

        field->access_flags = u2Read(file_ptr);
        field->name_index = u2Read(file_ptr);
        field->descriptor_index = u2Read(file_ptr);
        field->attributes_count = u2Read(file_ptr);

        field->attributes =
            (attribute_info *)malloc(
                field->attributes_count *
                sizeof(attribute_info));

        printf("\nfield[%hu]:\n", i);

        printf("\tname=#%hu (%s)\n",
               field->name_index,
               field->name_index > 0
                   ? (char *)
                         class_file_ptr
                             ->constant_pool[field->name_index]
                             .utf8_info->bytes
                   : "INVALID");

        printf("\tdescriptor=#%hu (%s)\n",
               field->descriptor_index,
               field->descriptor_index > 0
                   ? (char *)
                         class_file_ptr
                             ->constant_pool[field->descriptor_index]
                             .utf8_info->bytes
                   : "INVALID");

        printf("\tflags (0x%04x): ",
               field->access_flags);

        printAcessFlags(field);

        printf("\tattributes_count: %hu\n",
               field->attributes_count);

        for (u2 j = 0;
             j < field->attributes_count;
             j++)
        {

            printf("\n\tattribute[%hu]\n", j);

            attribute_info *attr =
                &field->attributes[j];

            attr->attribute_name_index =
                u2Read(file_ptr);

            attr->attribute_length =
                u4Read(file_ptr);

            attr->info =
                (u1 *)malloc(
                    attr->attribute_length);

            for (u4 k = 0;
                 k < attr->attribute_length;
                 k++)
            {

                attr->info[k] =
                    u1Read(file_ptr);
            }

            CONSTANT_Utf8_info *utf8 =
                class_file_ptr
                    ->constant_pool[attr->attribute_name_index]
                    .utf8_info;

            printf("\t\tattribute_name_index: "
                   "#%hu (%s)\n",
                   attr->attribute_name_index,
                   utf8->bytes);

            printf("\t\tlength: %u\n",
                   attr->attribute_length);

            printf("\t\tinfo bytes: ");

            for (u4 k = 0;
                 k < attr->attribute_length;
                 k++)
            {

                printf("%02X ",
                       attr->info[k]);
            }

            printf("\n");

            if (strcmp((char *)utf8->bytes,
                       "ConstantValue") == 0 &&
                attr->attribute_length >= 2)
            {

                u2 constantvalue_index =
                    (attr->info[0] << 8) |
                    attr->info[1];

                cp_info *cp =
                    &class_file_ptr
                         ->constant_pool[constantvalue_index];

                printf("\t\tConstant value "
                       "index: cp_info #%hu ",
                       constantvalue_index);

                switch (cp->tag)
                {

                case CONSTANT_Integer:

                    printf("%d\n",
                           cp->integer_info->bytes);
                    break;

                case CONSTANT_Float:
                {

                    float f;

                    memcpy(&f,
                           &cp->float_info->bytes,
                           sizeof(float));

                    printf("%f\n", f);

                    break;
                }

                case CONSTANT_Long:
                {
                    uint64_t value =
                        ((uint64_t)cp->long_info->high_bytes << 32) |
                        (uint64_t)cp->long_info->low_bytes;

                    printf("%lld\n",
                           (long long)value);

                    break;
                }    

                case CONSTANT_Double:
                {

                    double d;

                    uint64_t bits =
                        ((uint64_t)cp->double_info->high_bytes << 32) |
                        (uint64_t)cp->double_info->low_bytes;

                    memcpy(&d, &bits, sizeof(double));

                    printf("%lf\n", d);

                    break;

                    break;
                }

                case CONSTANT_String:
                {

                    u2 str_index =
                        cp->string_info->string_index;

                    CONSTANT_Utf8_info
                        *str_utf8 =
                            class_file_ptr
                                ->constant_pool[str_index]
                                .utf8_info;

                    printf("<%s>\n",
                           str_utf8->bytes);

                    break;
                }

                default:

                    printf("<unknown>\n");
                    break;
                }
            }
        }
    }
}

#endif