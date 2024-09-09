#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include "myqueue.h"


#define SERVERPORT 8989
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100 //allow 100 waiting connections
#define THREAD_POOL_SIZE 20

pthread_t thread_pool[THREAD_POOL_SIZE];
//queue is a shared data structure and it isn't thread safe, it 2 threads try to remove work from the queue at the same time or 1 do enqueue and 1 do dequeue causes race condition, to fix protect using mutex lock
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // without it, there will be 

void* thread_function(void *arg);
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void* handle_connection(void* client_socket);
int check(int exp, const char *msg);
// void* thread_function(void *arg);


int main (int argc, char **argv) {
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    //first off create a bunch of threads to handle future connections.
    for(int i=0;i<THREAD_POOL_SIZE;i++){
        pthread_create(&thread_pool[i],NULL,thread_function,NULL);
    }

    // Create the server socket
    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)),
          "Failed to create socket");

    // Initialize the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVERPORT);

    // Bind the socket
    check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)),
          "Bind failed!");

    // Start listening on the socket
    check(listen(server_socket, SERVER_BACKLOG),
          "Listen failed!");

    while (true) {
        printf("Waiting for connections...\n");

        // Wait for and accept an incoming connection
        addr_size = sizeof(SA_IN);
        check(client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size),
              "Accept failed");

        printf("Connected!\n");

        // Handle the connection
        pthread_t t;
        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;

        //make sure only one thread messes with the queue at a time
        pthread_mutex_lock(&mutex);
        enqueue(pclient);
        pthread_mutex_unlock(&mutex);
        //pthread_create(&t,NULL,handle_connection,pclient); 
        //handle_connection(pclient); //use while removing thread
    }
    return 0;
}

int check(int exp, const char *msg) {
    if (exp == SOCKETERROR) {
        perror(msg);
        exit(1);
    }
    return exp;
}

void* thread_function(void *arg){
    while(true){
        int *pclient;
        pthread_mutex_lock(&mutex);
        pclient = dequeue();
        pthread_mutex_unlock(&mutex);
        if(pclient != NULL){
            //we have a connection
            handle_connection(pclient);
        }
    }
}

void* handle_connection(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket); //we don't need pointer anymore so free it

    char buffer[BUFSIZE];
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[PATH_MAX + 1];

    // Read the client's message - the name of the file to read
    while ((bytes_read = read(client_socket, buffer + msgsize, sizeof(buffer) - msgsize - 1)) > 0) {
        msgsize += bytes_read;
        if (msgsize > BUFSIZE - 1 || buffer[msgsize - 1] == '\n') break;
    }
    check(bytes_read, "recv error");
    buffer[msgsize - 1] = 0; // Null-terminate the message and remove the newline

    printf("REQUEST: %s\n", buffer);
    fflush(stdout);

    // Validity check for the file path
    if (realpath(buffer, actualpath) == NULL) {
        printf("ERROR (bad path): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    // Open the file and send its contents to the client
    FILE *fp = fopen(actualpath, "r");
    if (fp == NULL) {
        printf("ERROR (open): %s\n", buffer);
        close(client_socket);
        return NULL;
    }
    sleep(1);
    // Read file contents and send them to the client
    while ((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
        printf("Sending %zu bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }

    // Close the connection and file
    close(client_socket);
    fclose(fp);
    printf("Closing connection\n");
    return NULL;
}