#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

void* myTurn(void *arg){
    for(int i=0;i<8;i++){
        sleep(1);
        printf("My Turn! %d\n",i);
    }
    return NULL;
}

void yourTurn(){
    for(int i=0;i<3;i++){
        sleep(2);
        printf("Your Turn! %d\n",i);
    }
}

int main(){
    pthread_t newthread;
    int v = 0;

    pthread_create(&newthread,NULL,myTurn,NULL);
    yourTurn();
    pthread_join(newthread,NULL);
}