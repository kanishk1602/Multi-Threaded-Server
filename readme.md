# Multi-Threaded Server

A progressive implementation of a TCP file server demonstrating socket programming, multithreading, and thread pool patterns in C.

## 📚 Documentation

- **[DOCUMENTATION_SUMMARY.md](DOCUMENTATION_SUMMARY.md)** - Navigation guide for all documentation
- **[PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md)** - Complete project explanation, architecture, and key concepts
- **[INTERVIEW_QUESTIONS.md](INTERVIEW_QUESTIONS.md)** - Common interview questions with detailed answers (20 questions)

## 🚀 Quick Start

### Build
```bash
make
```

### Run Server
```bash
./threadpool_multithreaded_server
```
The server will listen on port 8989.

### Test with Load
```bash
time ./manyclients.bash
```
This spawns 100 concurrent clients to test server performance.

## 📖 What's Inside

This project contains **three server implementations**:

1. **`server.c`** - Simple single-threaded server (baseline)
2. **`multithreaded_server.c`** - Thread-per-connection approach
3. **`threadpool_multithreaded_server.c`** - Thread pool implementation ⭐ (RECOMMENDED)

Each implementation demonstrates different approaches to handling concurrent connections, from basic to production-ready.

## 🎯 Learning Objectives

- Socket programming (TCP/IP)
- POSIX threads (pthreads)
- Synchronization primitives (mutex, condition variables)
- Producer-consumer pattern
- Thread pool architecture
- Performance optimization
- Concurrency best practices

## 🔍 For More Details

See [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md) for:
- Detailed architecture explanation
- How each server implementation works
- Key concepts (threads, mutexes, race conditions, etc.)
- Performance comparisons
- Real-world applications

See [INTERVIEW_QUESTIONS.md](INTERVIEW_QUESTIONS.md) for:
- 20+ interview questions with comprehensive answers
- Code walkthroughs
- Performance analysis
- System design discussions