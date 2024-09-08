# Multi-threaded C Program with `pthread`

This program demonstrates how to create multiple threads in C using the POSIX `pthread` library. Each thread runs a different function concurrently, and the main thread executes its own function in parallel.

## Features
- **Thread 1**: Prints "My Turn!" every 1 second.
- **Thread 2**: Prints "Your Turn!" every 2 seconds.
- **Main Thread**: Prints "Other Turn!" every 1 second.

## Requirements
Make sure you have the necessary libraries installed:
- `pthread.h` for thread creation.
- `unistd.h` for the `sleep()` function.
- `stdio.h` for printing to the console.

## How It Works
The program creates two additional threads:
- The first thread runs the `myTurn` function.
- The second thread runs the `yourTurn` function.
- The main thread (original thread) runs the `otherTurn` function.

Each of these functions runs in an infinite loop, printing its respective message at different intervals (1 second or 2 seconds).

## Code Overview

### 1. Function: `myTurn(void *arg)`
This function runs in its own thread and prints `"My Turn!"` every second:
```c
void* myTurn(void *arg) {
    while (1) {
        sleep(1);
        printf("My Turn!\n");
    }
    return NULL;
}


2. Function: yourTurn(void *arg)
This function runs in another thread and prints "Your Turn!" every 2 seconds:

void* yourTurn(void *arg) {
    while (1) {
        sleep(2);
        printf("Your Turn!\n");
    }
    return NULL;
}

3. Function: otherTurn()
The otherTurn function runs in the main thread and prints "Other Turn!" every 1 second:

void otherTurn() {
    while (1) {
        sleep(1);
        printf("Other Turn!\n");
    }
}

4. main() Function
The main function creates the threads and starts the otherTurn function in the main thread: 

int main() {
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, myTurn, NULL);
    pthread_create(&thread2, NULL, yourTurn, NULL);

    otherTurn(); // Main thread runs this function

    return 0;
}

Thread Creation: We use pthread_create to spawn two new threads. Each thread runs one of the functions (myTurn or yourTurn).
Main Thread: After creating the threads, the main thread immediately starts running otherTurn().
Important Notes:
Thread IDs: Separate thread IDs (thread1 and thread2) are used to create multiple threads without conflict.
Concurrency: All functions run in parallel, and their outputs will be interleaved based on their respective sleep intervals.

Expected Output
The output of this program will look like this (messages may appear in a different order due to concurrency):

My Turn!
Other Turn!
My Turn!
Your Turn!
My Turn!
Other Turn!
...

License
This project is licensed under the MIT License.

### Explanation of Markdown Elements:
- **# Headers**: Used for titles and subtitles.
- **```c**: Marks a block of C code for syntax highlighting.
- **Bullet points**: Used to list features and requirements.
- **Command blocks**: For commands like compilation instructions.

You can copy and paste this into your `README.md`, and it should render nicely on platforms like GitHub!
