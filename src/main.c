#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>
#include <stdlib.h>
#include "lib/class_loader/loader.h"
#include "lib/file/read_file.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <class_file_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file_ptr = readFile(argv[1]);

    ClassFile *class_file_ptr = (ClassFile *)malloc(sizeof(ClassFile));

    classFilesSetup(class_file_ptr, file_ptr);

    return 0;
}
#endif
