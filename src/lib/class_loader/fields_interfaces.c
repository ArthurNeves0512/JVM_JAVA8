#include "fields_interfaces.h"

void readInterfaces(ClassFile *class_file_ptr, FILE *file_ptr)
{
    class_file_ptr->interfaces_count = u2Read(file_ptr);

    class_file_ptr->interfaces = (u2 *)malloc(class_file_ptr->interfaces_count * sizeof(u2));
    for (u2 i = 0; i < class_file_ptr->interfaces_count; i++)
    {
        class_file_ptr->interfaces[i] = u2Read(file_ptr);
    }
}

void readFields(ClassFile *class_file_ptr, FILE *file_ptr)
{
    class_file_ptr->fields_count = u2Read(file_ptr);

    class_file_ptr->fields =
        (field_info *)malloc(class_file_ptr->fields_count * sizeof(field_info));

    for (u2 i = 0; i < class_file_ptr->fields_count; i++)
    {
        field_info *field = &class_file_ptr->fields[i];

        field->access_flags = u2Read(file_ptr);
        field->name_index = u2Read(file_ptr);
        field->descriptor_index = u2Read(file_ptr);
        field->attributes_count = u2Read(file_ptr);

        if (field->attributes_count > 0)
        {
            field->attributes =
                (attribute_info *)malloc(field->attributes_count * sizeof(attribute_info));

            for (u2 j = 0; j < field->attributes_count; j++)
            {
                field->attributes[j] = readAttribute(file_ptr, class_file_ptr->constant_pool);
            }
        }
        else
        {
            field->attributes = NULL;
        }
    }
}
