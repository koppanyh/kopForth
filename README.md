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

    // Make sure it exited successfully.
    kfBiosTeardown();
    if (s != KF_SYSTEM_DONE) {
        printf("Error: %d (%s)\n", s, kfStatusStr[s]);
        return s;
    }
    return 0;
}
```

## Adding Words

Make new file, add your implementation, profit.

## Porting Guide

Update the Bios!

## Files

 - kopForth.h
   - The main file that gets included in your main.c or wherever
 - kfBios.h
   - The only file that you should need to modify when porting to another system
 - kfType.h
   - The header containing the structs needed to instantiate a kopForth object
 - kfStack.h
   - The header that contains the stack implementations for kopForth
 - kfStatus.h
   - The header used for status error reporting
 - kfMath.h
   - Arbitrary precision math library used for double-cell math
 - kfWordsNative.h
   - This contains the native word definitions for the kopForth system
 - kfWordsVarAddrConst.h
   - This contains the word definitions for variables, addresses, and constants
 - kfWordsStackMem.h
   - This contains the word definitions for stack and address operators
 - kfWordsString.h
   - This contains the word definitions for string/char related stuff
 - kfWordsIntComp.h
   - This contains the word definitions for the shell interpreter and compiler
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
 - Line buffering
   - The non-Windows version uses getchar() and on most systems that seems to wait until the user hits enter before it'll start returning characters to the program
 - Memory alignment
   - Because certain forth words have the need to look up fields in the word struct using pointer arithmetic, the kfWord struct has been packed to make it easy to intuit the position of the field
   - This only causes problems on systems that do not allow unaligned memory access

## Changelog

 - TODO
   - Rewrite some native words to be forth words
   - Remove unnecessary words
   - Add file input support so words can be loaded
   - Add tests
 - v0.2
   - TLDR: Major refactoring and minor improvements
   - Rename files and functions and macros with `kf` prefix
   - Replace printf with Bios calls
   - Add header comments to all files
   - Simplify how status works
   - Split and refactor and reorganize files
   - Update BIOS to flush chars and use getchar on non-Windows systems
   - Change `word_def` to be a union
   - Simplified inner interpreter
   - Change flags to be packed in an int
   - Add more startup checks and pack word struct
   - Add debug toggle
   - Update word search to be case insensitive
 - v0.1
   - TLDR: Initial public release
   - MVP word set, supports word definition
