#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>

#define SERVERPORT 8989
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100 // number of allowed pending connections

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void handle_connection(int client_socket);
int check(int exp, char *msg);

int main (int argc, char **argv) {
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

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
        
        handle_connection(client_socket);
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
void handle_connection(int client_socket){
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
        return;
    }

    //read file and send its contents to client
    FILE *fp = fopen(actualpath,"r");
    if(fp == NULL){
        printf("ERROR(could not open file): %s\n",actualpath);
        close(client_socket);
        return; 
    }

    //read from file and send to client
    //not a safe way to read files, but ok for demo purposes
    while((bytes_read = fread(buffer,1,sizeof(buffer),fp)) > 0){
        printf("SENDING %zu bytes\n",bytes_read);
        write(client_socket,buffer,bytes_read);
    }
    close(client_socket);
    fclose(fp);
    printf("Closing Connection.\n");


}