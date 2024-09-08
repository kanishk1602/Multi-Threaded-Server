


#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

void* myTurn(void *arg){
    int *iptr = (int *)malloc(sizeof(int));
    *iptr = 5;
    for(int i=0;i<8;i++){
        sleep(1);
        printf("My Turn! %d\n",i,*iptr);
        (*iptr)++;
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
    int *result;

    pthread_create(&newthread,NULL,myTurn,NULL);
    yourTurn();
    int v;
    pthread_join(newthread,(void *  )&result);
    printf("thread's done: result = %d\n",*result);
}