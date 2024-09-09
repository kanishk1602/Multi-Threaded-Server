// largefile.c
#include <stdio.h>

int main() {
    // Simulate a large function
    for (int i = 0; i < 1000000000; i++) {
        printf("Line %d\n", i);
    }
    return 0;
}
