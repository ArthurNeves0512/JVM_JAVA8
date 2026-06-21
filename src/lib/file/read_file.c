#include<stdlib.h>
#include"lib/file/read_file.h"

FILE * readFile(const char * file_path){
    FILE * file_ptr = fopen(file_path,"rb");
    if(file_ptr==NULL){
        perror("A error ocurred while tying to read the Class file");
        exit(EXIT_FAILURE);
    }
    return file_ptr;
}