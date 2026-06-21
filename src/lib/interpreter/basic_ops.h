#ifndef BASIC_OPS_H
#define BASIC_OPS_H


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "dataTypes.h"
#include "attribute.h"
#include "class_file/dot_class.h"


#define TIPO_INT    0
#define TIPO_FLOAT  1
#define TIPO_LONG   2
#define TIPO_DOUBLE 3
#define TIPO_REF    4
#define TIPO_ARRAY  5
#define TIPO_OBJECT 6

/* atype para newarray (JVM spec §6.5 newarray) */
#define T_BOOLEAN 4
#define T_CHAR    5
#define T_FLOAT   6
#define T_DOUBLE  7
#define T_BYTE    8
#define T_SHORT   9
#define T_INT     10
#define T_LONG    11

typedef struct {
    int32_t  length;
    int      atype;
    void    *data;
} JVMArray;


#define MAX_PILHA  64
#define MAX_LOCAIS 64


typedef struct {
   int32_t inteiro;
   float   flutuante;
   int64_t longo;
   double  duplo;
   int     tipo;
} Slot;


typedef struct {
   Slot pilha[MAX_PILHA];
   int  topo;
   Slot locais[MAX_LOCAIS];
   u1  *codigo;
   u4   tamanho;
   u4   pc;
} Frame;


Frame *criaFrame(Code_attribute *ca);
void   liberaFrame(Frame *f);
void   executaFrame(Frame *f, ClassFile *cf);
void   liberaArrays(void);


#endif