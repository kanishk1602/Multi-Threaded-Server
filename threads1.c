#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

void* myTurn(void *arg){
    while(1){
        sleep(1);
        printf("My Turn!\n");
    }
    return NULL;
}

void* yourTurn(void *arg){
    while(1){
        sleep(2);
        printf("Your Turn!\n");
    }
    return NULL;
}

void otherTurn(){
    while(1){
        sleep(1);
        printf("Other Turn!\n");
    }
}

int main(){
    pthread_t newthread;

    pthread_create(&newthread,NULL,myTurn,NULL);
    pthread_create(&newthread,NULL,yourTurn,NULL);
    otherTurn();
}