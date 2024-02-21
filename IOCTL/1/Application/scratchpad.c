#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    uint8_t *test = malloc(sizeof(uint8_t));

    *test = 5;

    printf("as char: %c, as int: %d\n", *test, *test);
    return 0;
}