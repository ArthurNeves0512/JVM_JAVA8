#ifndef DATA_TYPES_H
#define DATA_TYPES_H
#include <stdint.h>
#include <stdio.h>

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

static inline u2 u2Read(FILE *ptr_file) {
    return (getc(ptr_file) << 8) | getc(ptr_file);
}

static inline u4 u4Read(FILE *ptr_file) {
    u2 first_u2 = u2Read(ptr_file);
    u2 second_u2 = u2Read(ptr_file);
    return (first_u2 << 16) | second_u2;
}

static inline u1 u1Read(FILE *ptr_file) {
    return (u1)getc(ptr_file);
}

#endif
