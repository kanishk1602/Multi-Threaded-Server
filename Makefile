CC=clang
CFLAGS=-g
BINS=server

all: $(BINS)

%: %. C 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *. dSYM $ (BINS)