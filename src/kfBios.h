#ifndef KF_BIOS_H
#define KF_BIOS_H

/*
 * kfBios.h (last modified 2025-06-12)
 * The BIOS file is meant to hold all the constants and interface functions
 * needed for easily porting kopForth to other platforms.
 * In theory, this should be the only file that needs to change for porting.
 */

#ifdef _WIN32
    #define KF_IS_WINDOWS
#endif
#ifdef _WIN64
    #define KF_IS_WINDOWS
#endif



#if defined(KF_IS_WINDOWS)
    // Windows requires this for the getch() function.
    #include <conio.h>
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>



// How many bits do you want to use for your data types.
// Make sure that the size is usable for address types, since Forth readily
// stores addresses in the data stacks, so it has to be able to go both ways.
// Simulates usize/isize from Rust. Can be changed if (u)intptr_t isn't
// supported in this compiler. (e.g. Use long int instead of intptr_t)
typedef uintptr_t usize;
typedef intptr_t isize;



// How many items to allocate for the data stack.
#define KF_DATA_STACK_SIZE 64
// How many items to allocate for the return stack.
#define KF_RETN_STACK_SIZE 32
// How many bytes to allocate for the terminal input buffer.
#define KF_TIB_SIZE 80
// How many bytes to allocate for the working memory (plus word definitions).
#define KF_MEM_SIZE 4096*sizeof(void*)
// How many bytes to allocate for the names of words (including \0).
#define KF_MAX_NAME_SIZE 16
// The character to use for return (keyboard input).
#ifdef KF_IS_WINDOWS
    // In Windows, the getch() function returns '\r' on keyboard return.
    #define KF_CR '\r'
#else
    #define KF_CR '\n'
#endif
// The character to use for newline (terminal output).
#define KF_NL '\n'



void kfBiosPrintIsize(isize value) {
    printf("%" PRIdPTR, value);
}

void kfBiosPrintPointer(void* value) {
    printf("%p", value);
}

void kfBiosWriteChar(isize value) {
    printf("%c", (int) value);
}

void kfBiosCR() {
    kfBiosWriteChar(KF_NL);
}

isize kfBiosReadChar() {
    #ifdef KF_IS_WINDOWS
        // We use getch() on Windows to get around the input buffering issue.
        return getch();
    #else
        return getchar();
    #endif
}

void kfBiosWriteStr(char* value) {
    while (*value) {
        kfBiosWriteChar(*value);
        value++;
    }
}

void kfBiosWriteStrLen(char* value, usize len) {
    for (usize i = 0; i < len; i++) {
        kfBiosWriteChar(*value);
        value++;
    }
}

void kfBiosSetup() {
    setbuf(stdout, NULL);
    #ifndef KF_IS_WINDOWS
        // No need to disable buffering in Windows since getch() already does.
        // TODO see if this actually makes a difference on *nix systems.
        setbuf(stdin, NULL);
    #endif

    // Intro credits.
    kfBiosWriteStr("kopForth v0.2, ");
    kfBiosPrintIsize(sizeof(isize) * 8);
    kfBiosWriteStr(" Bit, 2025");
    #ifdef KF_IS_WINDOWS
        kfBiosWriteStr(", Windows Edition");
    #endif
    kfBiosCR();
}

void kfBiosTeardown() {}

#endif // KF_BIOS_H
