# build the server
CC=clang
CFLAGS=-g
BINS=threadpool_multithreaded_server
OBJS=threadpool_multithreaded_server.o myqueue.o

all: $(BINS)

threadpool_multithreaded_server: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf *.dSYM $(BINS)