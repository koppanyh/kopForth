/*
 * main.c (last modified 2025-05-30)
 * This is just a demo of how kopForth system is included.
 */

#include <stdio.h>
#include <stdlib.h>

#include "kopForth.h"



int main() {
    // Define your kopForth object.
    kfBiosSetup();
    kopForth forth;

    // Initialize the kopForth system and make sure it succeeded.
    kfStatus s = kopForthInit(&forth);
    if (!kfStatusIsOk(s)) {
        printf("Error: %d (%s)\n", s, kfStatusStr[s]);
        kfBiosTeardown();
        return s;
    }

    // Run the kopForth system until it stops.
    do {
        s = kopForthTick(&forth);
    } while (kfStatusIsOk(s));

    // Print debug stuff.
    printf("\nstack: ");
    kfDataStackPrint(&forth.d_stack);
    printf("\ntib: %s\n", forth.tib);
    printf("#tib: %d\n", (int) forth.tib_len);

    // Make sure it exited successfully.
    kfBiosTeardown();
    if (s != KF_SYSTEM_DONE) {
        printf("Error: %d (%s)\n", s, kfStatusStr[s]);
        return s;
    }
    return 0;
}
