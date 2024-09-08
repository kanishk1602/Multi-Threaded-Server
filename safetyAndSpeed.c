#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#define BIG 1000UL
uint32_t counter = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *count_to_big(void *arg){
    for(uint32_t i = 0; i < BIG; i++){
        pthread_mutex_lock(&lock);
        counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(){
    pthread_t t;

    // Create a new thread to count
    pthread_create(&t, NULL, count_to_big, NULL);


    // Count in the main thread
    count_to_big(NULL);

    // Wait for the thread to finish
    pthread_join(t, NULL);

    // Print the final counter value
    printf("Done, Counter = %u\n", counter);

    return 0;
}
