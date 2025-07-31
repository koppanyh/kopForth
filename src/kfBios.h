#ifndef KF_BIOS_H
#define KF_BIOS_H

/*
 * kfBios.h (last modified 2025-07-31)
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



// Versioning info.
#define KF_VER_MAJOR       0
#define KF_VER_MINOR       2
#define KF_VER_PATCH       5
#define KF_YEAR_STR        "2025"
// How many items to allocate for the data stack.
#define KF_DATA_STACK_SIZE 64
// How many items to allocate for the return stack.
#define KF_RETN_STACK_SIZE 32
// How many bytes to allocate for the terminal input buffer.
#define KF_TIB_SIZE        80
// How many bytes to allocate for the working memory (plus word definitions).
#define KF_MEM_SIZE        4096*sizeof(void*)
// How many bytes to allocate for the names of words (including \0).
#define KF_MAX_NAME_SIZE   16
// The character to use for return (keyboard input).
#ifdef KF_IS_WINDOWS
    // In Windows, the getch() function returns '\r' on keyboard return.
    #define KF_CR          '\r'
#else
    #define KF_CR          '\n'
#endif
// The character to use for newline (terminal output).
#define KF_NL              '\n'

#define KF_TO_STR(X)            #X
#define KF_VER_TO_STR(X, Y, Z)  "v"KF_TO_STR(X)"."KF_TO_STR(Y)"."KF_TO_STR(Z)
#define KF_VER_STR              KF_VER_TO_STR(KF_VER_MAJOR, KF_VER_MINOR, KF_VER_PATCH)



/////////////
// BIOS IO //
/////////////

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

void kfBiosTeardown() {}



//////////////////
// BIOS File IO //
//////////////////

// The following functions implement standard C file operations.

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

typedef FILE kfBiosFileHandle;

isize kfBiosFileError(kfBiosFileHandle* file) {
    // Gets the error associated with the handle.
    // Returns 0 if there are no errors to report.
    return ferror(file);
}

kfBiosFileHandle* kfBiosFileOpen(const char* file_name, const char* mode) {
    // Opens a file and gets the handle.
    // Returns NULL if failure, handle otherwise.
    return fopen(file_name, mode);
}

isize kfBiosFileClose(kfBiosFileHandle* file) {
    // Closes a file and releases the handle.
    // Returns 0 on success, EOF otherwise.
    return fclose(file);
}

isize kfBiosFileFlush(kfBiosFileHandle* file) {
    // Flushes any buffered writes to the file.
    // Returns 0 on success, EOF otherwise.
    return fflush(file);
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
    kfBiosFileHandle* file = fopen(file_name, "r");
    if (file) {
        fclose(file);
        return 0;
    }
    return -1;
}

isize kfBiosFileSize(kfBiosFileHandle* file) {
    // Get the file's size.
    // Returns -1 on failure.
    long orig_pos = ftell(file);
    if (orig_pos == -1)
        return -1;
    if (fseek(file, 0, SEEK_END))
        return -1;
    long end_pos = ftell(file);
    if (end_pos == -1)
        return -1;
    if (fseek(file, orig_pos, SEEK_SET))
        return -1;
    return end_pos;
}

isize kfBiosFilePosition(kfBiosFileHandle* file) {
    // Get the file position indicator.
    // Returns -1 on failure.
    return ftell(file);
}

isize kfBiosFileReposition(kfBiosFileHandle* file, usize pos) {
    // Set file position indicator to pos.
    // Returns 0 on success, error code otherwise.
    return fseek(file, pos, SEEK_SET);
}

isize kfBiosFileReadFile(kfBiosFileHandle* file, uint8_t* buf, usize ct) {
    // Read up to ct bytes into buf.
    // Returns number of bytes read.
    return fread(buf, sizeof(uint8_t), ct, file);
}

char* kfBiosFileReadLine(kfBiosFileHandle* file, char* buf, usize ct) {
    // Read up to ct-1 chars into buf and end string with a terminator.
    // Stop reading when newline is found (includes newline in buf) or EOF.
    // Returns NULL on failure, buf otherwise.
    return fgets(buf, ct, file);
}

isize kfBiosFileWriteFile(kfBiosFileHandle* file, uint8_t* buf, usize ct) {
    // Write ct bytes from buf into the file.
    // Returns number of bytes written.
    return fwrite(buf, sizeof(uint8_t), ct, file);
}

#endif // KF_BIOS_H
