#ifndef METHODS_H
#define METHODS_H

#include "loader.h"

void readMethodsCount(ClassFile *cf, FILE *fp);
void readMethods(ClassFile *cf, FILE *fp);
void freeMethods(ClassFile *cf);

#endif
