#ifndef MYQUEUE_H_
#define MYQUEUE_H_

// Define the node structure for the queue
struct node {
    struct node* next;
    int *client_socket;
};

// Alias 'node_t' for 'struct node' for easier reference
typedef struct node node_t;

void enqueue(int *client_socket);
int *dequeue();

#endif // MYQUEUE_H_
