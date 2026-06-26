#ifndef LOADER_H
#define LOADER_H
#include <stdio.h>
#include <string.h>
#include "lib/class_loader/fields_interfaces.h"
#include "lib/class_loader/methods.h"
#include "lib/types/class_file/dot_class.h"
#include "lib/types/consts.h"
#include "lib/file/read_byte.h"
#include "lib/types/attribute.h"

u4   readCafeBabe(FILE *ptr_file);
u2   readConstantPoolCount(FILE *ptr_file);
void readAccessFlags(ClassFile *cf, FILE *fp);
void readThisClass(ClassFile *cf, FILE *fp);
void readSuperClass(ClassFile *cf, FILE *fp);
void readInterfacesCount(ClassFile *cf, FILE *fp);
void classFilesSetup(ClassFile *cf, FILE *fp);
void readClassFileAttributes(ClassFile *cf, FILE *fp);
char *getClassName(ClassFile *cf);
char *getSuperClassName(ClassFile *cf);
ClassFile *loadClassFile(const char *filename);
ClassFile *loadSuperClass(ClassFile *cf);

#endif
