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
#define SERVER_BACKLOG 100 //allow 100 waiting connections

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void handle_connection(int client_socket);
int check(int exp, const char *msg);

int main (int argc, char **argv) {
    int server_socket, client_socket;
    socklen_t addr_size;
    SA_IN server_addr, client_addr;

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
        check(client_socket = accept(server_socket, (SA*)&client_addr, &addr_size),
              "Accept failed");

        printf("Connected!\n");

        // Handle the connection
        handle_connection(client_socket);
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

void handle_connection(int client_socket) {
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
        return;
    }

    // Open the file and send its contents to the client
    FILE *fp = fopen(actualpath, "r");
    if (fp == NULL) {
        printf("ERROR (open): %s\n", buffer);
        close(client_socket);
        return;
    }

    // Read file contents and send them to the client
    while ((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
        printf("Sending %zu bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }

    // Close the connection and file
    close(client_socket);
    fclose(fp);
    printf("Closing connection\n");
}
