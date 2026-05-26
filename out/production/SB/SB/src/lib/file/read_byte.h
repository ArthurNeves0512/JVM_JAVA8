#ifndef READ_BYTE_H
#define READ_BYTE_H
#include <stdio.h>
#include "lib/types/dataTypes.h"

static inline u1 u1Read(FILE *ptr_file) { return (u1)getc(ptr_file); }

static inline u2 u2Read(FILE *ptr_file) {
    return (getc(ptr_file) << U2_SIZE) | getc(ptr_file);
}

static inline u4 u4Read(FILE *ptr_file) {
    u2 first  = u2Read(ptr_file);
    u2 second = u2Read(ptr_file);
    return (first << U4_SIZE) | second;
}

#endif
