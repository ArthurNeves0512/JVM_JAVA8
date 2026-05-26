#ifndef ARGS_H
#define ARGS_H

#include <stddef.h>

typedef struct {
    const char *class_path;
    const char *output_path;
    int do_print;
} Args;

int parse_args(int argc, char *argv[], Args *out);
void build_default_output_path(const char *class_path, char *out, size_t out_size);

#endif
