#include "lib/utils/args.h"
#include <stdio.h>
#include <string.h>

void build_default_output_path(const char *class_path, char *out, size_t out_size) {
    strncpy(out, class_path, out_size - 1);
    out[out_size - 1] = '\0';
    char *dot = strrchr(out, '.');
    if (dot != NULL)
        strcpy(dot, ".txt");
    else
        strncat(out, ".txt", out_size - strlen(out) - 1);
}

int parse_args(int argc, char *argv[], Args *out) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <class_file_path> [-o <output_file>] [--print]\n", argv[0]);
        return -1;
    }

    out->class_path = argv[1];
    out->output_path = NULL;
    out->do_print = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--print") == 0) {
            out->do_print = 1;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -o requires a file path\n");
                return -1;
            }
            out->output_path = argv[++i];
        } else {
            fprintf(stderr, "Usage: %s <class_file_path> [-o <output_file>] [--print]\n", argv[0]);
            return -1;
        }
    }

    return 0;
}
