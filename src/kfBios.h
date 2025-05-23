#ifndef KF_BIOS_H
#define KF_BIOS_H

/*
 * kfBios.h (last modified 2025-05-22)
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
#define DATA_STACK_SIZE 64
// How many items to allocate for the return stack.
#define RETN_STACK_SIZE 32
// How many bytes to allocate for the terminal input buffer.
#define TIB_SIZE 80
// How many bytes to allocate for the working memory (plus word definitions).
#define MEM_SIZE 4096*sizeof(void*)
// How many bytes to allocate for the names of words (including \0).
#define MAX_NAME_SIZE 16
// How many bytes to allocate for the status error details (including \0).
#define MAX_ERR_SIZE 16
// The character to use for return (keyboard input).
#define CR '\r'
// The character to use for newline (terminal output).
#define NL '\n'



void BiosPrintIsize(isize value) {
    printf("%" PRIdPTR, value);
}

void BiosWriteChar(isize value) {
    printf("%c", (int) value);
}

isize BiosReadChar() {
    return getch();
}

void BiosWriteStr(char* value) {
    while (*value) {
        BiosWriteChar(*value);
        value++;
    }
}

#endif // KF_BIOS_H
