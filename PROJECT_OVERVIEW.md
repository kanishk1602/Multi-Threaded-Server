# Multi-Threaded Server Project - Complete Overview

## Project Purpose
This is a **TCP file server** written in C that demonstrates the **evolution of server architectures** from simple single-threaded designs to production-ready thread pool implementations. It's an educational project showcasing concurrent programming concepts and performance optimization techniques.

## What Does It Do?
The server listens on **port 8989** and serves file contents to clients:
1. Client connects via TCP
2. Client sends a file path (e.g., `/path/to/file.c\n`)
3. Server validates the path
4. Server reads and sends the file contents back
5. Connection closes

## Three Server Implementations

### 1. Single-Threaded Server (`server.c`)
**Architecture:** Sequential, blocking I/O

**How it works:**
```
Client 1 connects → Handle → Close → Client 2 connects → Handle → Close
```

**Characteristics:**
- One connection at a time in the main thread
- Simple `while(true)` loop with `accept()` → `handle_connection()` → `close()`
- **Pros:** Simple, easy to understand, no race conditions
- **Cons:** Terrible performance with multiple clients (sequential processing)
- **Use case:** Learning, very low traffic scenarios

**Key code flow:**
```c
while(true) {
    client_socket = accept(server_socket, ...);
    handle_connection(client_socket);  // Blocks until done
}
```

### 2. Multi-Threaded Server (`multithreaded_server.c`)
**Architecture:** Thread-per-connection model

**How it works:**
```
Client 1 → Thread 1
Client 2 → Thread 2
Client 3 → Thread 3
... (creates unlimited threads)
```

**Characteristics:**
- Creates a **new thread for each connection** using `pthread_create()`
- Threads run concurrently - true parallelism
- Each thread independently handles its client
- **Pros:** Concurrent handling, much faster than single-threaded
- **Cons:** 
  - Resource exhaustion under high load (each thread consumes memory/resources)
  - Overhead of creating/destroying threads
  - No limit on thread count
- **Use case:** Moderate traffic, when you need simplicity with concurrency

**Key code flow:**
```c
while(true) {
    client_socket = accept(server_socket, ...);
    int *pclient = malloc(sizeof(int));
    *pclient = client_socket;
    pthread_create(&t, NULL, handle_connection, pclient);
    // Thread handles connection independently
}
```

### 3. Thread Pool Server (`threadpool_multithreaded_server.c`) ⭐ **Production Ready**
**Architecture:** Fixed thread pool with work queue

**How it works:**
```
[Worker Thread 1]
[Worker Thread 2]     ← Pull from queue
[Worker Thread 3]
...                   [Connection Queue]
[Worker Thread 20]    ← Push new connections
```

**Characteristics:**
- Pre-creates **20 worker threads** at startup
- Uses a **queue** to store pending connections
- Worker threads pull connections from queue
- **Synchronization:** Mutex + Condition variables
- **Pros:** 
  - Efficient resource management (fixed # of threads)
  - No thread creation overhead per connection
  - Can handle thousands of connections
  - Prevents resource exhaustion
- **Cons:** More complex implementation
- **Use case:** Production servers, high traffic

**Key code flow:**
```c
// Startup: Create thread pool
for(int i = 0; i < 20; i++) {
    pthread_create(&thread_pool[i], NULL, thread_function, NULL);
}

// Main loop: Enqueue connections
while(true) {
    client_socket = accept(server_socket, ...);
    pthread_mutex_lock(&queue_mutex);
    enqueue(pclient);
    pthread_cond_signal(&condition_var);  // Wake up a worker
    pthread_mutex_unlock(&queue_mutex);
}

// Worker threads: Dequeue and process
void* thread_function(void* arg) {
    while(true) {
        pthread_mutex_lock(&queue_mutex);
        if((pclient = dequeue()) == NULL) {
            pthread_cond_wait(&condition_var, &queue_mutex);
            pclient = dequeue();
        }
        pthread_mutex_unlock(&queue_mutex);
        if(pclient != NULL) {
            handle_connection(pclient);
        }
    }
}
```

## Queue Implementation (`myqueue.c/h`)

**Data Structure:** Simple linked list (FIFO)

```c
struct node {
    struct node* next;
    int *client_socket;
};
```

**Operations:**
- **enqueue():** Add connection to tail of queue
- **dequeue():** Remove connection from head of queue

**Thread Safety:** 
- The queue itself has **NO internal synchronization**
- Thread safety achieved by the server using **external mutex** (`queue_mutex`)
- This is a common pattern - separate data structure from synchronization

## Synchronization Mechanisms (Critical for Interviews!)

### Mutex (Mutual Exclusion)
```c
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
```
- Ensures only **one thread** accesses the queue at a time
- Prevents race conditions when enqueuing/dequeuing

### Condition Variable
```c
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
```
- Allows threads to **wait** for work efficiently (no busy-waiting/spinning)
- Main thread **signals** when new work arrives
- Worker threads **wait** when queue is empty

**Pattern:**
1. Worker locks mutex
2. If queue empty → wait on condition variable (releases mutex atomically)
3. When signaled → wakes up, re-acquires mutex, tries dequeue
4. Unlock mutex and process work

## Performance Comparison

**Test scenario:** 100 concurrent clients requesting files

| Implementation | Execution Time | Threads Created | Resource Usage |
|---------------|----------------|-----------------|----------------|
| Single-threaded | ~100 seconds | 1 | Low |
| Multi-threaded | ~1-2 seconds | 100+ | High (can crash with 1000s) |
| Thread pool | ~1-2 seconds | 20 (fixed) | Medium (predictable) |

## Client Tools

### `client.rb` - Ruby Client
- Connects to localhost:8989
- Sends file path from command line argument
- Measures response time
- Usage: `ruby client.rb 1` (requests `testfiles/1.c`)

### `manyclients.bash` - Load Testing Script
- Spawns **100 concurrent clients** in background
- Each requests files in round-robin (files 1-6)
- Tests server under load
- Usage: `time ./manyclients.bash`

## Building and Running

```bash
# Build the thread pool server (default)
make

# Run the server
./threadpool_multithreaded_server

# In another terminal - test with single client
ruby client.rb 1

# Load test with 100 concurrent clients
time ./manyclients.bash
```

## Key Interview Topics to Discuss

### 1. **Concurrency vs Parallelism**
- Concurrency: Multiple tasks in progress (context switching)
- Parallelism: Multiple tasks executing simultaneously (multi-core)

### 2. **Thread Lifecycle**
- Create → Run → Join/Detach → Terminate
- This project uses detached threads (no join)

### 3. **Race Conditions**
- Multiple threads accessing shared data simultaneously
- Queue access protected by mutex

### 4. **Deadlocks**
- Not present in this simple design
- Could occur with multiple mutexes (lock ordering issues)

### 5. **Producer-Consumer Pattern**
- Main thread = Producer (adds connections to queue)
- Worker threads = Consumers (process connections from queue)
- Classic concurrent programming pattern

### 6. **Scalability Trade-offs**
- Single-threaded: Simple but doesn't scale
- Thread-per-connection: Scales until resource limits
- Thread pool: Scales efficiently with bounded resources

## Code Quality & Best Practices

**Good:**
- Clear progression from simple to complex
- Comments explaining design decisions
- Error handling with `check()` function
- Path validation with `realpath()`

**Areas for improvement (mention in interview):**
- Queue should have internal synchronization
- No graceful shutdown mechanism
- Hard-coded thread pool size (should be configurable)
- File reading not fully safe (noted in comments)
- Missing error recovery for failed connections

## Real-World Applications

This pattern is used in:
- **Web servers** (Apache, Nginx use thread/process pools)
- **Database servers** (Connection pooling)
- **Application servers** (Java servlet containers)
- **Message brokers** (Kafka, RabbitMQ)

## Common Interview Questions & Answers

**Q: Why not just use async I/O instead of threads?**
A: Good question! Async I/O (like epoll/select) is another approach. Threads are simpler for blocking operations and CPU-intensive tasks. Modern servers often combine both.

**Q: What's the optimal thread pool size?**
A: Depends on workload:
- I/O-bound: More threads (blocking on I/O)
- CPU-bound: # of CPU cores
- This server uses 20, which is reasonable for I/O operations

**Q: How would you handle thread pool exhaustion?**
A: Options:
- Queue requests (current approach)
- Reject with error (fast fail)
- Dynamic pool sizing
- Backpressure/rate limiting

**Q: What about memory leaks?**
A: Each connection allocates memory for socket descriptor (`malloc`), freed in `handle_connection`. Good practice. Queue nodes also freed in dequeue.

## Summary

This project beautifully demonstrates:
1. ✅ Socket programming (TCP server)
2. ✅ POSIX threads (`pthread`)
3. ✅ Synchronization primitives (mutex, condition variables)
4. ✅ Producer-consumer pattern
5. ✅ Performance optimization (evolution from simple to efficient)
6. ✅ Resource management

It's a perfect project to discuss systems programming, concurrent algorithms, and production architecture considerations!
