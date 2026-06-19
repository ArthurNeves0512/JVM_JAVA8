#include "methods.h"

void readMethodsCount(ClassFile *cf, FILE *fp) {
    cf->methods_count = u2Read(fp);
}

void readMethods(ClassFile *cf, FILE *fp) {

    cf->methods =
        (method_info *) malloc(
            cf->methods_count * sizeof(method_info)
        );

    for (u2 i = 0; i < cf->methods_count; i++) {

        cf->methods[i].access_flags      = u2Read(fp);
        cf->methods[i].name_index        = u2Read(fp);
        cf->methods[i].descriptor_index  = u2Read(fp);
        cf->methods[i].attributes_count  = u2Read(fp);

        if (cf->methods[i].attributes_count > 0) {

            cf->methods[i].attributes =
                (attribute_info *) malloc(
                    cf->methods[i].attributes_count *
                    sizeof(attribute_info)
                );

            for (u2 j = 0; j < cf->methods[i].attributes_count; j++) {
                cf->methods[i].attributes[j] =
                    readAttribute(fp, cf->constant_pool);
            }

        } else {
            cf->methods[i].attributes = NULL;
        }
    }
}

void freeMethods(ClassFile *cf) {
    if (!cf->methods) return;

    for (u2 i = 0; i < cf->methods_count; i++) {
        if (cf->methods[i].attributes) {
            for (u2 j = 0; j < cf->methods[i].attributes_count; j++) {
                freeAttribute(
                    &cf->methods[i].attributes[j],
                    cf->constant_pool
                );
            }
            free(cf->methods[i].attributes);
        }
    }
    free(cf->methods);
    cf->methods = NULL;
}
