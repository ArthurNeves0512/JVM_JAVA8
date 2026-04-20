#ifndef MAIN_H
#define MAIN_H
#include<stdio.h>
#include<stdlib.h>
#include"dataTypes.h"
#include"classFile.h"



FILE * readClassFile(char * file_path){
    FILE * file_ptr = fopen(file_path,"rb");
    if(file_ptr==NULL){
        perror("A error ocurred while tying to read the Class file");
        exit(EXIT_FAILURE);
    }
    return file_ptr;
}

int main(){

    FILE * file_ptr = readClassFile("Opa.class");
    ClassFile * class_file_ptr=(ClassFile*) malloc(sizeof(ClassFile));
    classFilesSetup(class_file_ptr, file_ptr);
    
    return 0;
}
#endif