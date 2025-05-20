#ifndef STATUS_H
#define STATUS_H

#include "Bios.h"

typedef enum ErrType ErrType;
enum ErrType {
    OK = 0,
    STACK = 1,
    SYSTEM = 2,
};

typedef enum ErrCode ErrCode;
enum ErrCode {
    NONE = 0,
    STACK_OVERFLOW = 1,
    STACK_UNDERFLOW = 2,
    SYSTEM_PTRWIDTH = 3,
    SYSTEM_DONE = 4,
    SYSTEM_STRUCT = 5,
    SYSTEM_NOT_IMP = 6,
    SYSTEM_NULL = 7,
    SYSTEM_COMP_ONLY = 8,
};

typedef struct {
    ErrType type;
    ErrCode code;
    usize line;
    char error[MAX_ERR_SIZE];
} Status;

Status MakeStatus(ErrType type, ErrCode code, usize line, char* error) {
    Status s;
    s.type = type;
    s.code = code;
    s.line = line;
    char* c = (char*) &s.error;
    while (*error) {
        *c = *error;
        c++;
        error++;
    }
    return s;
}

Status OkStatus = { .type = OK, .code = NONE, .line = 0, .error = "" };

bool IsOk(Status status) {
    return status.type == OK;
}

#define RETURN_IF_ERROR(status) do { \
        Status s = status; \
        if (!IsOk(s)) return s; \
    } while(0)

#endif // STATUS_H
