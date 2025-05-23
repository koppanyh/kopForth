#ifndef BIOS_H
#define BIOS_H

#include <conio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>



// How many bits do you want to use for your data types.
// Simulates usize/isize from Rust. Can be changed if (u)intptr_t isn't
// supported in this compiler. (e.g. Use long int instead of intptr_t)
typedef uintptr_t usize;
typedef intptr_t isize;



// Defines how many items to allocate for the data stack.
#define DATA_STACK_SIZE 64
// Defines how many items to allocate for the return stack.
#define RETN_STACK_SIZE 32
// Defines how many bytes to allocate for the terminal input buffer.
#define TIB_SIZE 80
// Defines how many bytes to allocate for the working memory (contains word definitions).
#define MEM_SIZE 4096*sizeof(void*)
// Defines how many bytes to allocate for the names of words (including \0).
#define MAX_NAME_SIZE 16
// Defines how many bytes to allocate for the error details (including \0).
#define MAX_ERR_SIZE 16
// Defines the character to use for return (keyboard input).
#define CR '\r'
// Defines the character to use for newline (terminal output).
#define NL '\n'



void BiosPrintIsize(isize value) {
    printf("%" PRIdPTR " ", value);
}

void BiosWriteChar(isize value) {
    printf("%c", (int) value);
}

isize BiosReadChar() {
    return getch();
}

#endif // BIOS_H
