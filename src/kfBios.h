#ifndef KF_BIOS_H
#define KF_BIOS_H

/*
 * kfBios.h (last modified 2025-10-13)
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



/////////////////
// BIOS Params //
/////////////////

//
// Versioning params.
//

#define KF_VER_MAJOR 0
#define KF_VER_MINOR 3
#define KF_VER_PATCH 0
#define KF_YEAR_STR  "2025"

#define KF_TO_STR(X)            #X
#define KF_VER_TO_STR(X, Y, Z)  "v"KF_TO_STR(X)"."KF_TO_STR(Y)"."KF_TO_STR(Z)
#define KF_VER_STR              KF_VER_TO_STR(KF_VER_MAJOR, KF_VER_MINOR, KF_VER_PATCH)

//
// Memory allocation params.
//

// How many items to allocate for the data stack.
#define KF_DATA_STACK_SIZE 64
// How many items to allocate for the return stack.
#define KF_RETN_STACK_SIZE 64
// How many bytes to allocate for the input buffer.
#define KF_IN_BUF_SIZE     512
// How many bytes to allocate for the working memory (plus word definitions).
#define KF_MEM_SIZE        4096*sizeof(void*)
// How many bytes to allocate for the names of words (including \0).
#define KF_MAX_NAME_SIZE   16
// How many open files should the file handle pool support.
#define KF_FILE_POOL_SIZE  16

//
// Terminal params.
//

// The character to use for return (keyboard input).
#ifdef KF_IS_WINDOWS
    // In Windows, the getch() function returns '\r' on keyboard return.
    #define KF_CR          '\r'
#else
    #define KF_CR          '\n'
#endif
// The character to use for newline (terminal output).
#define KF_NL              '\n'



/////////////////////////
// BIOS Extension Defs //
/////////////////////////

//
// File extension.
//

// Let other modules know that the file extension is available.
#define KF_FILE_EXT

#ifndef KF_FILE_EXT
    // The base defines provided by the file extension.
    #define KF_FILE_EXT_DEP_FULL
    #define KF_FILE_EXT_DEP_SHORT
    #define KF_FILE_EXT_INIT
#endif



//////////////////////
// BIOS Terminal IO //
//////////////////////

void kfBiosPrintIsize(isize value) {
    printf("%" PRIdPTR, value);
}

void kfBiosPrintPointer(void* value) {
    //printf("%p", value);
    printf("%" PRIXPTR "h", (usize) value);
}

void kfBiosWriteChar(isize value) {
    printf("%c", (int) value);
}

void kfBiosCR() {
    kfBiosWriteChar(KF_NL);
}

isize kfBiosReadChar() {
    int c;
    #ifdef KF_IS_WINDOWS
        // We use getch() on Windows to get around the input buffering issue.
        c = getch();
    #else
        c = getchar();
    #endif
    if (c == KF_CR)  // Normalize newlines to '\n' character.
        return KF_NL;
    return c;
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



////////////////
// BIOS Debug //
////////////////

void kfBiosDumpMem(uint8_t* value, usize len) {
    char hex[] = "0123456789ABCDEF";
    usize pr_intv = 16;
    for (usize i = 0; i < len; i++) {
        if (pr_intv == 16) {
            kfBiosCR();
            kfBiosPrintPointer(value);
            kfBiosWriteChar(' ');
            pr_intv = 0;
        }
        kfBiosWriteChar(hex[(*value) >> 4]);
        kfBiosWriteChar(hex[(*value) & 0xF]);
        kfBiosWriteChar(' ');
        value++;
        pr_intv++;
    }
    kfBiosCR();
}



//////////////////
// BIOS File IO //
//////////////////

//
// File access method helpers, should be safe to leave as-is.
//

#define KF_FAM_WR  0
#define KF_FAM_R   1
#define KF_FAM_W   2
#define KF_FAM_RW  3
// KF_FAM_BIN gets added to the previous defs to set the binary flag.
#define KF_FAM_BIN 4

static const char* kfBiosFileAccessMethods[] = {
            // # | Description          | If Exist   | If Not Exist
            // --+----------------------+------------+-------------
    "w+",   // 0 | Create w/ read+write | Clear file | Create new
    "r",    // 1 | Open for reading     | Open       | Error
    "w",    // 2 | Create for writing   | Clear file | Create new
    "r+",   // 3 | Open w/ read+write   | Open       | Error
    "wb+",  // 4 | w+ but binary        | w+         | w+
    "rb",   // 5 | r but binary         | r          | r
    "wb",   // 6 | w but binary         | w          | w
    "rb+",  // 7 | r+ but binary        | r+         | r+
};
// R/O, W/O, R/W, BIN

const char* kfBiosFileGetFam(usize fam) {
    if (fam > 7)
        return NULL;
    return kfBiosFileAccessMethods[fam];
}

//
// Custom file implementation for the BIOS.
//

typedef struct kfBiosFile kfBiosFile;
struct kfBiosFile {
    FILE* file;
    usize error;
    bool open;  // FILE doesn't seem to track this, so we track it ourselves.
};

kfBiosFile kfBiosFilePool[KF_FILE_POOL_SIZE];

void kfBiosFileResetError(kfBiosFile* file) {
    file->error = 0;
}

isize kfBiosFileReset(kfBiosFile* file) {
    kfBiosFileResetError(file);
    isize rez = 0;
    if (file->file)
        rez = fclose(file->file);
    file->file = NULL;
    file->error = 0;
    file->open = false;
    return rez;
}

void kfBiosFileSetupPool() {
    for (usize i = 0; i < KF_FILE_POOL_SIZE; i++) {
        kfBiosFilePool[i].file = NULL;
        kfBiosFileReset(&kfBiosFilePool[i]);
    }
}

kfBiosFile* kfBiosFileGetAvailable() {
    for (usize i = 0; i < KF_FILE_POOL_SIZE; i++) {
        kfBiosFile* file = &kfBiosFilePool[i];
        if (file->open)
            continue;
        kfBiosFileReset(file);
        return file;
    }
    return NULL;
}

//
// The following functions somewhat implement standard C file operations.
//

isize kfBiosFileError(kfBiosFile* file) {
    // Gets the error associated with the handle.
    // Returns 0 if there are no errors to report.
    int err = ferror(file->file);
    if (err)
        return err;
    return file->error;
}

kfBiosFile* kfBiosFileOpen(const char* file_name, const char* mode) {
    // Opens a file and gets the handle.
    // Returns NULL if failure, handle otherwise.
    kfBiosFile* file = kfBiosFileGetAvailable();
    if (!file)
        return NULL;
    file->file = fopen(file_name, mode);
    if (!file->file)
        return NULL;
    file->open = true;
    return file;
}

isize kfBiosFileClose(kfBiosFile* file) {
    // Closes a file and releases the handle.
    // Returns 0 on success, EOF otherwise.
    kfBiosFileResetError(file);
    return kfBiosFileReset(file);
}

isize kfBiosFileFlush(kfBiosFile* file) {
    // Flushes any buffered writes to the file.
    // Returns 0 on success, EOF otherwise.
    kfBiosFileResetError(file);
    return fflush(file->file);
}

isize kfBiosFileRename(const char* old_name, const char* new_name) {
    // Rename a file by its name.
    // Returns 0 on success, error code otherwise.
    return rename(old_name, new_name);
}

isize kfBiosFileDelete(const char* file_name) {
    // Delete a file by its name.
    // Returns 0 on success, error code otherwise.
    return remove(file_name);
}

isize kfBiosFileStatus(const char* file_name) {
    // Checks if a file exists (or is accessible) by name.
    // Returns -1 on failure, 0 otherwise.
    // TODO return codes for things like if directory, or exists but no perms.
    kfBiosFile* file = kfBiosFileOpen(file_name, kfBiosFileGetFam(KF_FAM_R));
    if (file)
        return kfBiosFileClose(file);
    return -1;
}

isize kfBiosFileSize(kfBiosFile* file) {
    // Get the file's size.
    // Returns -1 on failure.
    if (!file->open)
        return -1;
    long orig_pos = ftell(file->file);
    if (orig_pos < 0)
        return -1;
    if (fseek(file->file, 0, SEEK_END))
        return -1;
    long end_pos = ftell(file->file);
    if (end_pos < 0)
        return -1;
    if (fseek(file->file, orig_pos, SEEK_SET))
        return -1;
    return end_pos;
}

isize kfBiosFilePosition(kfBiosFile* file) {
    // Get the file position indicator.
    // Returns -1 on failure.
    if (!file->open)
        return 0;
    return ftell(file->file);
}

isize kfBiosFileReposition(kfBiosFile* file, usize pos) {
    // Set file position indicator to pos.
    // Returns 0 on success, error code otherwise.
    kfBiosFileResetError(file);
    if (!file->open)
        return 0;
    return fseek(file->file, pos, SEEK_SET);
}

usize kfBiosFileReadFile(kfBiosFile* file, uint8_t* buf, usize ct) {
    // Read up to ct bytes into buf.
    // Returns number of bytes read.
    kfBiosFileResetError(file);
    if (!file->open) {
        file->error = -1;
        return 0;
    }
    return fread(buf, sizeof(uint8_t), ct, file->file);
}

usize kfBiosFileWriteFile(kfBiosFile* file, uint8_t* buf, usize ct) {
    // Write ct bytes from buf into the file.
    // Returns number of bytes written.
    kfBiosFileResetError(file);
    if (!file->open) {
        file->error = -1;
        return 0;
    }
    return fwrite(buf, sizeof(uint8_t), ct, file->file);
}



/////////////////////////
// BIOS Setup/Teardown //
/////////////////////////

void kfBiosSetup() {
    setbuf(stdout, NULL);
    #ifndef KF_IS_WINDOWS
        // No need to disable buffering in Windows since getch() already does.
        // TODO see if this actually makes a difference on *nix systems.
        setbuf(stdin, NULL);
    #endif

    kfBiosFileSetupPool();

    // Intro credits.
    kfBiosWriteStr("kopForth " KF_VER_STR ", ");
    kfBiosPrintIsize(sizeof(isize) * 8);
    kfBiosWriteStr(" Bit");
    #ifdef KF_IS_WINDOWS
        kfBiosWriteStr(", Windows Edition");
    #endif
    kfBiosCR();
    kfBiosWriteStr("Copyright " KF_YEAR_STR ", compiled " __DATE__);
    kfBiosCR();
}

void kfBiosTeardown() {
    // TODO close opened files
}

#endif // KF_BIOS_H
