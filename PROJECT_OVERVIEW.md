# Multi-Threaded Server - Project Overview

## Table of Contents
1. [Introduction](#introduction)
2. [Project Architecture](#project-architecture)
3. [Server Implementations](#server-implementations)
4. [Key Components](#key-components)
5. [How It Works](#how-it-works)
6. [Building and Running](#building-and-running)
7. [Performance Comparison](#performance-comparison)
8. [Key Concepts](#key-concepts)

## Introduction

This project demonstrates the evolution of a file server from a simple single-threaded implementation to a highly efficient multi-threaded server using a thread pool pattern. The server accepts client connections, receives file path requests, reads the requested files, and sends the contents back to clients.

### Project Purpose
- **Educational**: Demonstrates practical implementation of socket programming, multithreading, and synchronization primitives
- **Progressive Learning**: Shows three iterations of the same server with increasing sophistication
- **Real-world Application**: Implements patterns commonly used in production servers (Apache, Nginx, etc.)

## Project Architecture

The project contains three server implementations, each building upon the previous:

```
Multi-Threaded-Server/
├── server.c                              # Version 1: Single-threaded server
├── multithreaded_server.c                # Version 2: Thread-per-connection
├── threadpool_multithreaded_server.c     # Version 3: Thread pool (RECOMMENDED)
├── myqueue.h / myqueue.c                 # Queue implementation for thread pool
├── client.rb                             # Ruby client for testing
├── manyclients.bash                      # Script to simulate 100 concurrent clients
├── Makefile                              # Build configuration
└── testfiles/                            # Test files for the server
```

## Server Implementations

### 1. Simple Server (`server.c`)

**Design**: Sequential, single-threaded server that handles one connection at a time.

**Characteristics**:
- Accepts connection → Processes request → Sends response → Closes connection
- Handles only ONE client at a time
- Other clients must wait until current request completes
- Simple but inefficient

**Pros**:
- Simple to understand and debug
- No concurrency issues
- Minimal resource usage

**Cons**:
- Poor performance with multiple clients
- Blocks all other connections while serving one client
- Unacceptable for production use

**Code Flow**:
```c
while(true) {
    client_socket = accept(server_socket, ...);  // Wait for connection
    handle_connection(client_socket);             // Process (BLOCKING)
}
```

### 2. Multi-threaded Server (`multithreaded_server.c`)

**Design**: Creates a new thread for each incoming connection.

**Characteristics**:
- Spawns a new pthread for every client connection
- Connections are handled concurrently
- Each thread runs independently
- Thread is destroyed after handling the connection

**Pros**:
- Concurrent request handling
- Good performance for moderate load
- Simple implementation

**Cons**:
- **Thread creation overhead**: Creating/destroying threads is expensive
- **Resource exhaustion**: With 1000 clients, creates 1000 threads
- **System limits**: OS has limits on number of threads
- **Memory overhead**: Each thread requires stack space (typically 2-8 MB)
- **Context switching**: Too many threads degrade performance

**Code Flow**:
```c
while(true) {
    client_socket = accept(server_socket, ...);
    pthread_t t;
    int *pclient = malloc(sizeof(int));
    *pclient = client_socket;
    pthread_create(&t, NULL, handle_connection, pclient);  // New thread per connection
}
```

**Problem**: Under high load (e.g., 10,000 concurrent connections), this approach:
- Creates 10,000 threads → Consumes massive memory (20-80 GB just for stacks!)
- Excessive context switching → CPU spends more time switching than working
- May hit OS limits and crash

### 3. Thread Pool Server (`threadpool_multithreaded_server.c`) ⭐ BEST APPROACH

**Design**: Uses a fixed pool of worker threads that process connections from a shared queue.

**Characteristics**:
- Pre-creates a fixed number of threads (e.g., 20 threads)
- Incoming connections are added to a queue
- Worker threads pull connections from queue and process them
- Threads are reused, not destroyed after each request
- Uses mutex and condition variables for synchronization

**Pros**:
- **Fixed resource usage**: Number of threads is constant and configurable
- **No thread creation overhead**: Threads created once at startup
- **Efficient resource utilization**: Optimal thread count (typically 2x CPU cores)
- **Scalable**: Can handle thousands of connections with just 20 threads
- **Production-ready**: Pattern used in real-world servers

**Cons**:
- More complex implementation
- Requires careful synchronization
- Queue can become bottleneck if not implemented efficiently

**Code Flow**:
```c
// Startup: Create thread pool
for(int i = 0; i < THREAD_POOL_SIZE; i++) {
    pthread_create(&thread_pool[i], NULL, thread_function, NULL);
}

// Main loop: Add connections to queue
while(true) {
    client_socket = accept(server_socket, ...);
    pthread_mutex_lock(&queue_mutex);
    enqueue(client_socket);           // Add to queue
    pthread_cond_signal(&condition_var);  // Wake up a worker
    pthread_mutex_unlock(&queue_mutex);
}

// Worker thread: Process connections from queue
void* thread_function(void* arg) {
    while(true) {
        pthread_mutex_lock(&queue_mutex);
        while((pclient = dequeue()) == NULL)
            pthread_cond_wait(&condition_var, &queue_mutex);
        pthread_mutex_unlock(&queue_mutex);
        
        handle_connection(pclient);   // Process request
    }
}
```

## Key Components

### Socket Programming

The server uses TCP sockets for reliable communication:

1. **Socket Creation**: `socket(AF_INET, SOCK_STREAM, 0)`
   - `AF_INET`: IPv4 protocol
   - `SOCK_STREAM`: TCP (connection-oriented)

2. **Binding**: `bind(server_socket, address, ...)`
   - Associates socket with IP address and port (8989)
   - `INADDR_ANY`: Accept connections on any network interface

3. **Listening**: `listen(server_socket, SERVER_BACKLOG)`
   - `SERVER_BACKLOG = 100`: Maximum pending connections in queue

4. **Accepting**: `accept(server_socket, ...)`
   - Blocks until a client connects
   - Returns a new socket for the client connection

### Queue Implementation (`myqueue.h`, `myqueue.c`)

Simple linked list-based FIFO queue:

```c
struct node {
    struct node* next;
    int *client_socket;
};

void enqueue(int *client_socket);  // Add to tail
int* dequeue();                     // Remove from head
```

**Note**: This queue implementation is NOT thread-safe by itself. Thread safety is achieved through mutex locks in the main server code.

### Synchronization Primitives

#### Mutex (Mutual Exclusion)
```c
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_lock(&queue_mutex);     // Acquire lock
// Critical section - only one thread at a time
pthread_mutex_unlock(&queue_mutex);   // Release lock
```

**Purpose**: Prevents race conditions when multiple threads access shared data (the queue).

#### Condition Variable
```c
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

pthread_cond_wait(&condition_var, &queue_mutex);  // Wait for signal
pthread_cond_signal(&condition_var);               // Wake one waiting thread
```

**Purpose**: Allows threads to sleep until there's work to do (efficient waiting).

**How it works**:
1. Worker thread acquires mutex
2. Checks queue - if empty, calls `pthread_cond_wait()`
3. `pthread_cond_wait()` atomically:
   - Releases the mutex
   - Puts thread to sleep
4. When signaled, thread wakes up and re-acquires mutex
5. Thread processes work and releases mutex

### Client (`client.rb`)

Ruby client that:
1. Connects to server on port 8989
2. Sends file path
3. Receives file contents
4. Measures elapsed time

### Load Testing (`manyclients.bash`)

Bash script that spawns 100 concurrent clients to stress-test the server:
```bash
for N in {1..100}; do
    ruby client.rb $((($N%6)+1)) &  # Run in background
done
wait  # Wait for all to complete
```

## How It Works

### Complete Request Flow (Thread Pool Server)

1. **Server Startup**:
   ```
   Create thread pool (20 threads)
   → Each thread waits on condition variable
   → Create and bind socket to port 8989
   → Start listening for connections
   ```

2. **Client Connection**:
   ```
   Client connects → accept() returns new socket
   → Allocate memory for socket descriptor
   → Lock mutex
   → Add socket to queue (enqueue)
   → Signal condition variable (wake a worker)
   → Unlock mutex
   → Go back to accept() for next client
   ```

3. **Worker Thread Processing**:
   ```
   Thread wakes up from condition variable
   → Lock mutex
   → Dequeue socket from queue
   → Unlock mutex
   → Read file path from client
   → Validate path (realpath)
   → Open and read file
   → Send file contents to client
   → Close connection
   → Loop back to wait for next task
   ```

### File Handling

```c
// Read file path from client
read(client_socket, buffer, ...);

// Validate path (security: prevent directory traversal)
realpath(buffer, actualpath);

// Open and read file
FILE *fp = fopen(actualpath, "r");
while((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
    write(client_socket, buffer, bytes_read);  // Send to client
}

// Cleanup
fclose(fp);
close(client_socket);
```

## Building and Running

### Prerequisites
- C compiler (Clang or GCC)
- POSIX threads library (pthread)
- Ruby (for client)

### Build
```bash
make                    # Builds threadpool_multithreaded_server
make clean             # Removes binaries
```

### Run Server
```bash
./threadpool_multithreaded_server
```

Server will listen on port 8989.

### Test with Load
```bash
time ./manyclients.bash
```

This spawns 100 concurrent clients and measures total time.

### Manual Test (Single Client)
```bash
# In one terminal
./threadpool_multithreaded_server

# In another terminal
ruby client.rb 1
```

## Performance Comparison

### Scenario: 100 Concurrent Clients

| Implementation | Threads Created | Memory Usage | Avg. Response Time | Total Time |
|----------------|----------------|--------------|-------------------|------------|
| Simple Server  | 1              | ~10 MB       | High (sequential) | ~100 seconds |
| Multi-threaded | 100            | ~200-800 MB  | Low per request   | ~1-2 seconds |
| Thread Pool    | 20             | ~20-50 MB    | Low per request   | ~1-2 seconds |

### Scenario: 10,000 Concurrent Clients

| Implementation | Threads Created | Memory Usage | Behavior |
|----------------|----------------|--------------|----------|
| Simple Server  | 1              | ~10 MB       | Takes hours |
| Multi-threaded | 10,000         | 20-80 GB     | ⚠️ Likely crashes/extremely slow |
| Thread Pool    | 20             | ~20-50 MB    | ✅ Handles gracefully |

**Key Insight**: Thread pool maintains consistent performance regardless of connection count!

## Key Concepts

### 1. Concurrency vs Parallelism
- **Concurrency**: Multiple tasks in progress (interleaved)
- **Parallelism**: Multiple tasks executing simultaneously (multi-core)

This server achieves both:
- Concurrency through multithreading
- Parallelism if run on multi-core CPU

### 2. Thread Safety
Code is thread-safe when multiple threads can access shared data without corruption.

**Problem Example** (without mutex):
```
Thread 1: dequeue() reads head → gets node A
Thread 2: dequeue() reads head → gets node A (same!)
Thread 1: updates head
Thread 2: updates head
Result: Both threads get same connection! Client will timeout.
```

**Solution**: Mutex ensures only one thread can dequeue at a time.

### 3. Race Condition
Situation where outcome depends on timing/sequence of thread execution.

**Example**: If two threads enqueue simultaneously without locks, linked list can corrupt.

### 4. Deadlock
Two threads waiting for each other's locks, forever.

**This project avoids deadlock**:
- Only one mutex used
- Always acquired and released in same function
- Condition variable used correctly

### 5. Producer-Consumer Pattern
- **Producer**: Main thread accepts connections and adds to queue
- **Consumer**: Worker threads take from queue and process
- **Buffer**: Queue
- **Synchronization**: Mutex + Condition Variable

### 6. Context Switching
OS switching CPU from one thread to another.

**Overhead**:
- Save current thread state
- Load new thread state
- Flush CPU caches
- Takes ~1-5 microseconds

**Why thread pools help**: Fewer threads = less context switching

### 7. Thread Pool Benefits

**Optimal Thread Count**:
- CPU-bound tasks: # of CPU cores
- I/O-bound tasks: 2x to 4x CPU cores (this server is I/O-bound: file reading)
- This project uses 20 threads (reasonable for I/O)

**Resource Management**:
- Predictable memory usage
- Prevents resource exhaustion
- Reduces allocation/deallocation overhead

## Real-World Applications

This pattern is used in:
- **Web Servers**: Apache (worker MPM), Nginx
- **Database Servers**: PostgreSQL, MySQL
- **Application Servers**: Tomcat, Node.js (thread pool for blocking operations)
- **Proxy Servers**: Squid, HAProxy
- **Game Servers**: Handling multiple player connections

## Summary

This project demonstrates three approaches to building a file server:

1. **Simple Server**: Educational baseline, unsuitable for production
2. **Multi-threaded Server**: Better but wasteful under high load
3. **Thread Pool Server**: Production-ready, efficient, scalable ⭐

The thread pool implementation showcases:
- Socket programming
- POSIX threads
- Synchronization primitives (mutex, condition variables)
- Producer-consumer pattern
- Efficient resource management

This is a foundational pattern for any high-performance network server!
