#ifndef KF_STATUS_H
#define KF_STATUS_H

/*
 * kfStatus.h (last modified 2025-05-27)
 * The status file defines the enum used for debugging and triggering system
 * exceptions.
 */



// Necessary typedef declarations for types.
typedef enum kfStatus kfStatus;



// Convenience macro to be able to just wrap a function call instead of manually
// checking the value and returning if it's not ok.
#define KF_RETURN_IF_ERROR(status) do { \
            kfStatus s = status; \
            if (!kfStatusIsOk(s)) return s; \
        } while(0)



// Dynamically generate the enum in a way that also generates the string array,
// using the preprocessor to keep them in sync.

#define FOREACH_KF_STATUS(STATUS)       \
        STATUS(KF_STATUS_OK)            \
        STATUS(KF_DATA_STACK_OVERFLOW)  \
        STATUS(KF_DATA_STACK_UNDERFLOW) \
        STATUS(KF_RETN_STACK_OVERFLOW)  \
        STATUS(KF_RETN_STACK_UNDERFLOW) \
        STATUS(KF_SYSTEM_PTRWIDTH)      \
        STATUS(KF_SYSTEM_DONE)          \
        STATUS(KF_SYSTEM_STRUCT)        \
        STATUS(KF_SYSTEM_NOT_IMP)       \
        STATUS(KF_SYSTEM_NULL)          \
        STATUS(KF_SYSTEM_COMP_ONLY)     \

#define GENERATE_KF_STATUS_ENUM(ENUM)     ENUM,
#define GENERATE_KF_STATUS_STRING(STRING) #STRING,

enum kfStatus {
    FOREACH_KF_STATUS(GENERATE_KF_STATUS_ENUM)
};

static const char* kfStatusStr[] = {
    FOREACH_KF_STATUS(GENERATE_KF_STATUS_STRING)
};



bool kfStatusIsOk(kfStatus status) {
    return status == KF_STATUS_OK;
}

#endif // KF_STATUS_H
