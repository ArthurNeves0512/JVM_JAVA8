#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/class_loader/loader.h"
#include "lib/class_loader/fields_interfaces.h"
#include "lib/class_loader/methods.h"
#include "lib/file/read_file.h"
#include "lib/printer/printer.h"
#include "lib/utils/args.h"

int main(int argc, char *argv[]) {
    Args args;
    if (parse_args(argc, argv, &args) != 0)
        return EXIT_FAILURE;

    char default_output[512];
    if (args.output_path == NULL) {
        build_default_output_path(args.class_path, default_output, sizeof(default_output));
        args.output_path = default_output;
    }

    int terminal_fd = -1;
    if (args.do_print)
        terminal_fd = dup(STDOUT_FILENO);

    if (freopen(args.output_path, "w", stdout) == NULL) {
        fprintf(stderr, "Error: could not open output file '%s'\n", args.output_path);
        if (terminal_fd >= 0) close(terminal_fd);
        return EXIT_FAILURE;
    }

    FILE *file_ptr = readFile((char *)args.class_path);

    ClassFile *class_file_ptr = (ClassFile *)malloc(sizeof(ClassFile));
    classFilesSetup(class_file_ptr, file_ptr);
    readInterfaces(class_file_ptr, file_ptr);
    readFields(class_file_ptr, file_ptr);
    readMethodsCount(class_file_ptr, file_ptr);
    readMethods(class_file_ptr, file_ptr);
    readClassFileAttributes(class_file_ptr, file_ptr);
    printClassFile(class_file_ptr);
    printMethods(class_file_ptr);
    printClassFileAttributes(class_file_ptr);
    fflush(stdout);

    if (args.do_print && terminal_fd >= 0) {
        printFileToTerminal(terminal_fd, args.output_path);
        close(terminal_fd);
    }

    return 0;
}
#endif
