#ifndef METHODS_H
#define METHODS_H

#include "loader.h"
#include "attribute.h"
void readMethodsCount(ClassFile *cf, FILE *fp) {
    cf->methods_count = u2Read(fp);
}

void readMethods(ClassFile *cf, FILE *fp) {
    printf("\n=== Methods ===\n");
    printf("methods_count: %hu\n", cf->methods_count);

    cf->methods = (method_info *) malloc(cf->methods_count * sizeof(method_info));

    for (u2 i = 0; i < cf->methods_count; i++) {

        cf->methods[i].access_flags = u2Read(fp);
        cf->methods[i].name_index = u2Read(fp);
        cf->methods[i].descriptor_index = u2Read(fp);
        cf->methods[i].attributes_count = u2Read(fp);

        printf("method[%hu]: name=#%hu descriptor=#%hu\n",
            i,
            cf->methods[i].name_index,
            cf->methods[i].descriptor_index
        );

        cf->methods[i].attributes =
            (attribute_info *)
            malloc(
                cf->methods[i].attributes_count *
                sizeof(attribute_info)
            );

        for (u2 j = 0; j < cf->methods[i].attributes_count; j++) {

            cf->methods[i].attributes[j] =
                readAttribute(
                    fp,
                    cf->constant_pool
                );
        }
    }
}

#endif
