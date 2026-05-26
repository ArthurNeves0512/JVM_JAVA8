#ifndef PRINTER_H
#define PRINTER_H
#include "lib/types/class_file/dot_class.h"
#define SKIP_LINE printf("\n")
void printClassFile(const ClassFile *cf);
void printFileToTerminal(int terminal_fd, const char *path);

#endif
