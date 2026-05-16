#ifndef LOADER_H
#define LOADER_H
#include <stdio.h>
#include "lib/types/class_file/dot_class.h"
#include "lib/types/consts.h"
#include "lib/file/read_byte.h"

u4   readCafeBabe(FILE *ptr_file);
u2   readConstantPoolCount(FILE *ptr_file);
void readAcessFlags(ClassFile *cf, FILE *fp);
void readThisClass(ClassFile *cf, FILE *fp);
void readSuperClass(ClassFile *cf, FILE *fp);
void readInterfacesCount(ClassFile *cf, FILE *fp);
void classFilesSetup(ClassFile *cf, FILE *fp);

#endif
