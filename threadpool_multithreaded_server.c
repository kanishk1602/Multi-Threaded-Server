#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <pthread.h>
#include "myqueue.h"

#define SERVERPORT 8989
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100 // number of allowed pending connections
#define THREAD_POOL_SIZE 20

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void* handle_connection(void* p_client_socket);
int check(int exp, char *msg);
void* thread_function(void* arg);

int main (int argc, char **argv) {
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    for(int i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    //create the socket
    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)),
            "Socket creation failed");
    
    //initialise the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVERPORT);

    //we bind the socket to the address and port number
    //check func is error handling function
    check(bind(server_socket,(SA*)&server_addr,sizeof(server_addr)),
            "Bind failed");
    //stream of data instead of bytes, SOCK_STREAM
    check(listen(server_socket, SERVER_BACKLOG),
            "Listen failed");

    while(true){
        printf("Waiting for connection...\n");
        //wait for eventually accept an incoming connection
        addr_size = sizeof(SA_IN);
        //accept new connection
        check((client_socket = 
                accept(server_socket, (SA*)&client_addr,(socklen_t*)&addr_size)),
                "Accept failed");
        printf("Connection accepted.\n");

        //put
        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;
        pthread_mutex_lock(&queue_mutex);
        pthread_cond_signal(&condition_var);
        enqueue(pclient);
        pthread_mutex_unlock(&queue_mutex);
        
        //handle_connection(client_socket);
        // pthread_t t;
        // int *pclient = malloc(sizeof(int));
        // *pclient = client_socket;
        // pthread_create(&t,NULL,handle_connection,pclient);
        //handle_connection(pclient);



    }
    return 0;
}

int check(int exp, char *msg){
    if(exp == SOCKETERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}

void* thread_function(void* arg){
    while(true){
        int* pclient;
        pthread_mutex_lock(&queue_mutex);
        if((pclient = dequeue()) == NULL){
            pthread_cond_wait(&condition_var, &queue_mutex);
            //try again
            pclient = dequeue();

        }
        pthread_mutex_unlock(&queue_mutex);

        if(pclient != NULL){
            handle_connection(pclient);
        }
    }
}

void* handle_connection(void* p_client_socket){
    int client_socket = *(int*)p_client_socket;
    free(p_client_socket); //we don't need this anymore
    char buffer[BUFSIZE];
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[PATH_MAX+1];

    //read the client's message -- the name of the file to read
    while((bytes_read = read(client_socket,buffer+msgsize,sizeof(buffer)-msgsize-1))>0){
        msgsize += bytes_read;
        if(msgsize > BUFSIZE-1 || buffer[msgsize-1] == '\n') break;
    }

    check(bytes_read,"recv error");
    buffer[msgsize-1] = 0; //null terminate the message and removve the newline

    printf("REQUEST: %s\n",buffer);
    fflush(stdout);

    //validity check
    if(realpath(buffer,actualpath) == NULL){
        printf("ERROR(bad path): %s\n",buffer);
        close(client_socket);
        return NULL;
    }

    //read file and send its contents to client
    FILE *fp = fopen(actualpath,"r");
    if(fp == NULL){
        printf("ERROR(could not open file): %s\n",actualpath);
        close(client_socket);
        return NULL; 
    }

    sleep(1);
    //read from file and send to client
    //not a safe way to read files, but ok for demo purposes
    while((bytes_read = fread(buffer,1,sizeof(buffer),fp)) > 0){
        printf("SENDING %zu bytes\n",bytes_read);
        write(client_socket,buffer,bytes_read);
    }
    close(client_socket);
    fclose(fp);
    printf("Closing Connection.\n");
    return NULL;

}

// a new thread for every connection, it will eventually create many threads if there are many connections and may exhaust system resources and slow down the server. A better approach would be to use a thread pool where a fixed number of threads handle incoming connections from a shared queue.
// rather than creating a new thread for each connection, which can lead to resource exhaustion and performance degradation under high load. A thread pool allows for a fixed number of threads to handle multiple connections, improving resource management and overall server performance.
