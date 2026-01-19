#include <stdlib.h>
#include <string.h>

int main(){
    for(int i=0;i<10000000;i++){
        char *buffer = (char*)malloc(1024);
        strcpy(buffer, "Hello, World!");
        // Missing free(buffer);
    }
    for(int i=0;i<10000000;i++){
        char *buffer = (char*)malloc(1024);
        strcpy(buffer, "Hello, World!");
        // Missing free(buffer);
    }
    for(int i=0;i<10000000;i++){
        char *buffer = (char*)malloc(1024);
        strcpy(buffer, "Hello, World!");
        // Missing free(buffer);
    }
    return 0;   
}
