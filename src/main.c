/*
 * main.c (last modified 2025-05-22)
 * This is just a demo of how kopForth system is included.
 */

#include <stdio.h>
#include <stdlib.h>

#include "kopForth.h"



int main() {
    Forth forth;
    Status s = ForthInit(&forth);
    if (!IsOk(s)) {
        printf(s.error);
        return s.type;
    }

    //printf("%d\n", sizeof(*forth.forth_vars));
    /*for (int i = 0; i < 256; i++) {
        printf("%d(%c) ", forth.mem[i], forth.mem[i]);
    }
    printf("\n"); // */

    do {
        s = ForthTick(&forth);
    } while (IsOk(s));
    printf("\nstack: ");
    DataStackPrint(&forth.d_stack);
    printf("\ntib: %s\n", forth.tib);
    printf("#tib: %d\n", (int) forth.tib_len);

    if (s.code != SYSTEM_DONE) {
        printf(s.error);
        return s.type;
    }
    return 0;
}
