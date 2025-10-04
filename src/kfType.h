#ifndef KF_TYPE_H
#define KF_TYPE_H

/*
 * kfType.h (last modified 2025-08-27)
 * This contains the main structs and types used by the kopForth system, along
 * with their helper functions.
 */

#include "kfBios.h"
#include "kfStack.h"



// Macros to help with defining words.
#define WRD(wrd) kopForthAddWordP(forth, wrd)
#define LIT(isz) kopForthAddWordP(forth, forth->debug_words.lit); kopForthAddIsize(forth, (isize) isz)
#define RAW(isz) kopForthAddIsize(forth, (isize) isz)
#define WRDADDR(var, wrd) kfWord** var = kopForthAddWordP(forth, wrd)
#define LITADDR(var, wrd, isz) kopForthAddWordP(forth, wrd); isize* var = kopForthAddIsize(forth, (isize) isz)
#define RAWADDR(var, isz) isize* var = kopForthAddIsize(forth, (isize) isz)
#define PRSTR(str) WRD(forth->debug_words.psq); kopForthAddString(forth, str); WRD(forth->debug_words.typ)



// Necessary typedef declarations for types.
typedef struct kfDebugWords   kfDebugWords;
typedef struct kfInputSource  kfInputSource;
typedef struct kopForth       kopForth;
typedef union  kfWordCode     kfWordCode;
typedef struct kfWordBitFlags kfWordBitFlags;
typedef union  kfWordFlags    kfWordFlags;
typedef struct kfWord         kfWord;

// This is a function pointer type for native word implementations. It takes a
// kopForth pointer, does something with it, and returns a status.
// Usage:
// kfStatus W_<word name>(kopForth* forth) {  // <stack before> -- <stack after>
//     <native word logic here>
//     return KF_STATUS_OK;
// }
typedef kfStatus (*kfNativeFunc)(kopForth*);



// Special words used by the kopForth debugger and compiler.
struct kfDebugWords {
    kfWord* ext;
    kfWord* lit;
    kfWord* bra;
    kfWord* zbr;
    kfWord* typ;
    kfWord* psq;
    kfWord* qut;
};

// The data needed to keep track of what the current input source is.
#define KF_INPUT_SOURCE_COUNT 4
struct kfInputSource {
    isize    source_id;  // The source ID value, 0=user input device, -1=string (via EVALUATE), other=file handle.
    usize    in_offset;  // The index for the next character to read from the input buffer.
    usize    in_len;     // The total size of the text in the input buffer.
    uint8_t* buf;        // The address of the start of the input buffer.
};

// This is the main struct from which an instance of kopForth is created.
// Maintain the core/heap/stacks ordering of the fields.
struct kopForth {
    // Core system fields
    uint8_t*      here;                    // Pointer to the next available `mem` byte.
    kfWord*       latest;                  // Pointer to the latest active word in `mem`. FIND starts searching here.
    kfWord*       pending;                 // Pointer to the most recently defined word, but not necessarily the latest active word.
    isize         state;                   // The compilation state, true=compiling, false=interpret. Uses `isize` so Forth programs can just use `@` and `!`.
    isize         debug;                   // The debug state, true=enabled, false=disabled. Uses `isize` so Forth programs can just use `@` and `!`.
    uint8_t*      pc;                      // Program counter for forth inner loop.
    kfDebugWords  debug_words;             // Pointers to words used by the kopForth debugger and compiler.
    // Data heap
    uint8_t       mem[KF_MEM_SIZE];        // The general memory space where the word dictionary is held.
    // Stacks + input buffer region
    kfDataStack   d_stack;                 // The data stack.
    kfRetnStack   r_stack;                 // The return stack.
    kfInputSource in_src;                  // The current input source definition.
    uint8_t       in_buf[KF_IN_BUF_SIZE];  // The input buffer, shared between terminal and files.
};



// This is the type that actually defines what the word does. It either calls a
// native function, or it rolls through a list of word addresses and executes
// them in sequence.
union kfWordCode {
    kfNativeFunc native;  // Pointer to a native function.
    kfWord**     forth;   // Pointer to start of forth code to be executed.
};

#define KF_FLAG_MASK_NATIVE    0b00000001
#define KF_FLAG_MASK_IMMEDIATE 0b00000010
struct kfWordBitFlags {
    uint8_t   is_native    : 1;  // Determines if the word points to a function or a list of words.
    uint8_t   is_immediate : 1;  // Determines if the word is executed at compile time.
};

union kfWordFlags {
    uint8_t        raw_flags;  // Provides an access address to all the flags at once.
    kfWordBitFlags bit_flags;  // Provides individual access to the flags.
};

// This is the word definition type that defines the name and flags and overall
// functionality of each Forth word in memory.
// Must be packed so that we know the field offsets and the words defined in
// forth will be able to know where to access a field with pointer arithmetic.
#define KF_WORD_NAME_LEN_OFFSET 0
#define KF_WORD_NAME_OFFSET     (KF_WORD_NAME_LEN_OFFSET + sizeof(uint8_t))
#define KF_WORD_LINK_OFFSET     (KF_WORD_NAME_OFFSET     + sizeof(char[KF_MAX_NAME_SIZE]))
#define KF_WORD_FLAGS_OFFSET    (KF_WORD_LINK_OFFSET     + sizeof(kfWord*))
#define KF_WORD_CODE_OFFSET     (KF_WORD_FLAGS_OFFSET    + sizeof(kfWordFlags))
#define KF_WORD_DATA_OFFSET     (KF_WORD_CODE_OFFSET     + sizeof(kfWordCode))
struct kfWord {
    uint8_t     name_len;                // How long the name is (not including \0).
    char        name[KF_MAX_NAME_SIZE];  // The name of the word.
    kfWord*     link;                    // Linked-list pointer to the previous word.
    kfWordFlags flags;                   // The flags used for runtime and compile time behaviors.
    kfWordCode  code;                    // Code pointer, says what to run when word is executed.
    // It's important that `data` comes at the end of the `kfWord` so it can be
    // expanded past its boundary of 1 item without colliding with any of the
    // other fields in the word definition.
    kfWord*     data[1];                 // The non-native word definition.
}__attribute__((packed));



// Helper functions for defining words and stuff in kopForth.

bool kfCanFitInMem(kopForth* forth, usize length) {
    uint8_t* new_next_mem_ptr = forth->here + length;
    return new_next_mem_ptr <= forth->mem + KF_MEM_SIZE;
}

kfWord* kopForthCreateWord(kopForth* forth) {
    forth->latest = forth->pending;
    kfWord* word = (kfWord*) forth->here;
    if (!kfCanFitInMem(forth, sizeof(kfWord)))
        return NULL;
    forth->here += sizeof(kfWord) - sizeof(kfWord*[1]);
    word->link = forth->latest;
    forth->pending = word;
    forth->latest = word;
    word->flags.bit_flags.is_native = false;
    word->flags.bit_flags.is_immediate = false;
    word->code.forth = &(word->data[0]);
    return word;
}

kfWord* kopForthAddWord(kopForth* forth, char* name) {
    kfWord* word = kopForthCreateWord(forth);
    if (word == NULL)
        return NULL;
    word->name_len = 0;
    char* word_name = word->name;
    while (*name) {
        *word_name = *name;
        word_name++;
        name++;
        word->name_len++;
    }
    *word_name = '\0';
    return word;
}

isize* kopForthAddIsize(kopForth* forth, isize value) {
    if (!kfCanFitInMem(forth, sizeof(isize)))
        return NULL;
    isize* ptr = (isize*) forth->here;
    forth->here += sizeof(isize);
    *ptr = value;
    return ptr;
}

kfWord** kopForthAddWordP(kopForth* forth, kfWord* value) {
    if (!kfCanFitInMem(forth, sizeof(kfWord*)))
        return NULL;
    kfWord** ptr = (kfWord**) forth->here;
    forth->here += sizeof(kfWord*);
    *ptr = value;
    return ptr;
}

kfWord* kopForthAddNativeWord(kopForth* forth, char* name, kfNativeFunc func_ptr,
                              bool is_immediate) {
    if (!kfCanFitInMem(forth, sizeof(kfWord)))
        return NULL;
    kfWord* word = kopForthAddWord(forth, name);
    word->flags.bit_flags.is_native = true;
    word->flags.bit_flags.is_immediate = is_immediate;
    word->code.native = func_ptr;
    return word;
}

kfWord* kopForthAddVariable(kopForth* forth, char* name, isize* var_ptr) {
    if (!kfCanFitInMem(forth, sizeof(kfWord) + 2 * sizeof(kfWord*)))
        return NULL;
    kfWord* word = kopForthAddWord(forth, name);
    kopForthAddWordP(forth, forth->debug_words.lit);
    kopForthAddIsize(forth, (isize) var_ptr);
    kopForthAddWordP(forth, forth->debug_words.ext);
    return word;
}

void kopForthAddString(kopForth* forth, char* str) {
    uint8_t* len = forth->here;
    forth->here++;
    *len = 0;
    while (*str) {
        *forth->here = *str;
        (*len)++;
        str++;
        forth->here++;
    }
}

#endif // KF_TYPE_H
