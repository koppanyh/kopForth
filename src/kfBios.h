#ifndef KF_BIOS_H
#define KF_BIOS_H

/*
 * kfBios.h (last modified 2025-05-23)
 * The BIOS file is meant to hold all the constants and interface functions
 * needed for easily porting kopForth to other platforms.
 * In theory, this should be the only file that needs to change for porting.
 */

#include <conio.h>
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
#define KF_CR '\r'
// The character to use for newline (terminal output).
#define KF_NL '\n'



void kfBiosPrintIsize(isize value) {
    printf("%" PRIdPTR, value);
}

void kfBiosWriteChar(isize value) {
    printf("%c", (int) value);
}

isize kfBiosReadChar() {
    return getch();
}

void kfBiosWriteStr(char* value) {
    while (*value) {
        kfBiosWriteChar(*value);
        value++;
    }
}

#endif // KF_BIOS_H
