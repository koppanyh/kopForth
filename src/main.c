/*
 * main.c (last modified 2025-06-11)
 * This is just a demo of how kopForth system is included.
 */

#include <stdio.h>
#include <stdlib.h>

// Include the main kopForth header.
#include "kopForth.h"



int main() {
    // Initialize the metal and run self checks.
    kfBiosSetup();
    kfStatus s = kopForthTest();
    if (!kfStatusIsOk(s)) {
        printf("Error: %d (%s)\n", s, kfStatusStr[s]);
        kfBiosTeardown();
        return s;
    }

    // Initialize the kopForth system and make sure it succeeded.
    kopForth forth;
    s = kopForthInit(&forth);
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
