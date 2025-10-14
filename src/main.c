/*
 * main.c (last modified 2025-10-10)
 * This is just a demo of how kopForth system is included.
 */

#include <stdio.h>
#include <stdlib.h>

// Include the main kopForth header.
#include "kopForth.h"



int main() {
    // Initialize the metal and run self checks.
    kfBiosSetup();
    kfStatus s = kopForthSelfTest();
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

    // Specify a bootstrap file (only if file extension is loaded).
    #ifdef KF_FILE_EXT
        s = kopForthBootstrap(&forth, "boot.fs");
        if (!kfStatusIsOk(s)) {
            printf("Error: %d (%s)\n", s, kfStatusStr[s]);
            kfBiosTeardown();
            return s;
        }
    #endif

    // Run the kopForth system until it stops.
    do {
        s = kopForthTick(&forth);
    } while (kfStatusIsOk(s));

    // Print debug stuff.
    printf("\nstack: ");
    kfDataStackPrint(&forth.d_stack);
    printf("\nrstack: ");
    kfRetnStackPrint(&forth.r_stack);
    printf("\ntib: %s\n", forth.in_buf);
    printf("#tib: %d\n", (int) forth.in_src.in_len);

    // Make sure it exited successfully.
    kfBiosTeardown();
    if (s != KF_SYSTEM_DONE) {
        printf("Error: %d (%s)\n", s, kfStatusStr[s]);
        return s;
    }
    return 0;
}
