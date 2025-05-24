# kopForth
My personal portable Forth implementation

The kopForth project (pronounced "cope forth") is intended to be a personal reference subset of a mostly standard-compliant Forth for use in my future projects. It allows for a standardized language that I can easily embed into embedded systems or port to esoteric systems by hand.

## Usage

```c
// main.c

#include <stdio.h>
#include <stdlib.h>

// Include the main kopForth header.
#include "kopForth.h"

int main() {
    // Define your kopForth object.
    kopForth forth;

    // Initialize the kopForth system and make sure it succeeded.
    kfStatus s = kopForthInit(&forth);
    if (!kfStatusIsOk(s)) {
        printf("Error: %d (%s)\n", s, kfStatusStr[s]);
        return s;
    }

    // Run the kopForth system until it stops.
    do {
        s = kopForthTick(&forth);
    } while (kfStatusIsOk(s));

    // Make sure it exited successfully.
    if (s != KF_SYSTEM_DONE) {
        printf("Error: %d (%s)\n", s, kfStatusStr[s]);
        return s;
    }
    return 0;
}
```

## Porting Guide

Update the Bios!

## Files

 - kopForth.h
   - The main file that gets included in your main.c or wherever
 - kfBios.h
   - The only file that you should need to modify when porting to another system
 - kfMath.h
   - Arbitrary precision math library used for double-cell math
 - kfStack.h
   - The header that contains the stack implementations for kopForth
 - kfStatus.h
   - The internal library used for status reporting (errors and stuff)
 - main.c
   - Demo main file

## Limitations

 - Bit width
   - Cell size has to be compatible with the address size so that R> and >R words still work
   - This means that forth code may not function correctly if run on a system that expects a different size of cell
     - E.g. Writing code on a 64 bit implementation that works with integers that use up to 48 bits, then moving it to a 32 bit implementation, overflows and truncation will occur
 - Endianness
   - Integer arithmetic is done in the system's native endianness
   - This means that forth code may not function correctly if run on a system that uses a different endianness
     - E.g. Writing code on a little endian system that uses C@ to read the LSB of a number stored in memory, this will return the MSB if the code is run on a big endian system

## Changelog

 - TODO
   - Split and reorganize files
   - Add tests
   - Words to lower case
 - v0.2
   - TLDR: Major refactoring
   - Rename files and functions and macros with `kf` prefix
   - Replace printf with Bios calls
   - Add header comments to all files
   - Simplify how status works
 - v0.1
   - TLDR: Initial public release
   - MVP word set, supports word definition
