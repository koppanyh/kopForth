#ifndef KF_STATUS_H
#define KF_STATUS_H

/*
 * kfStatus.h (last modified 2025-05-23)
 * The status file defines the enum used for debugging and triggering system
 * exceptions.
 */



// Convenience macro to be able to just wrap a function call instead of manually
// checking the value and returning if it's not ok.
#define RETURN_IF_ERROR(status) do { \
            Status s = status; \
            if (!StatusIsOk(s)) return s; \
        } while(0)



// Dynamically generate the enum in a way that also generates the string array,
// using the preprocessor to keep them in sync.

#define FOREACH_STATUS(STATUS)       \
        STATUS(STATUS_OK)            \
        STATUS(DATA_STACK_OVERFLOW)  \
        STATUS(DATA_STACK_UNDERFLOW) \
        STATUS(RETN_STACK_OVERFLOW)  \
        STATUS(RETN_STACK_UNDERFLOW) \
        STATUS(SYSTEM_PTRWIDTH)      \
        STATUS(SYSTEM_DONE)          \
        STATUS(SYSTEM_STRUCT)        \
        STATUS(SYSTEM_NOT_IMP)       \
        STATUS(SYSTEM_NULL)          \
        STATUS(SYSTEM_COMP_ONLY)     \

#define GENERATE_STATUS_ENUM(ENUM)     ENUM,
#define GENERATE_STATUS_STRING(STRING) #STRING,

typedef enum Status Status;
enum Status {
    FOREACH_STATUS(GENERATE_STATUS_ENUM)
};

static const char* StatusStr[] = {
    FOREACH_STATUS(GENERATE_STATUS_STRING)
};



bool StatusIsOk(Status status) {
    return status == STATUS_OK;
}

#endif // KF_STATUS_H
