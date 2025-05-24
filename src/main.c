/*
 * main.c (last modified 2025-05-23)
 * This is just a demo of how kopForth system is included.
 */

#include <stdio.h>
#include <stdlib.h>

#include "kopForth.h"



int main() {
    Forth forth;
    Status s = ForthInit(&forth);
    if (!StatusIsOk(s)) {
        printf("Error: %d (%s)\n", s, StatusStr[s]);
        return s;
    }

    //printf("%d\n", sizeof(*forth.forth_vars));
    /*for (int i = 0; i < 256; i++) {
        printf("%d(%c) ", forth.mem[i], forth.mem[i]);
    }
    printf("\n"); // */

    do {
        s = ForthTick(&forth);
    } while (StatusIsOk(s));
    printf("\nstack: ");
    DataStackPrint(&forth.d_stack);
    printf("\ntib: %s\n", forth.tib);
    printf("#tib: %d\n", (int) forth.tib_len);

    if (s != SYSTEM_DONE) {
        printf("Error: %d (%s)\n", s, StatusStr[s]);
        return s;
    }
    return 0;
}
