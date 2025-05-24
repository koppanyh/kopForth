/*
 * main.c (last modified 2025-05-23)
 * This is just a demo of how kopForth system is included.
 */

#include <stdio.h>
#include <stdlib.h>

#include "kopForth.h"



int main() {
    kopForth forth;
    kfStatus s = kopForthInit(&forth);
    if (!kfStatusIsOk(s)) {
        printf("Error: %d (%s)\n", s, kfStatusStr[s]);
        return s;
    }

    //printf("%d\n", sizeof(*forth.forth_vars));
    /*for (int i = 0; i < 256; i++) {
        printf("%d(%c) ", forth.mem[i], forth.mem[i]);
    }
    printf("\n"); // */

    do {
        s = kopForthTick(&forth);
    } while (kfStatusIsOk(s));
    printf("\nstack: ");
    kfDataStackPrint(&forth.d_stack);
    printf("\ntib: %s\n", forth.tib);
    printf("#tib: %d\n", (int) forth.tib_len);

    if (s != KF_SYSTEM_DONE) {
        printf("Error: %d (%s)\n", s, kfStatusStr[s]);
        return s;
    }
    return 0;
}
