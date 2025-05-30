#ifndef KF_STACK_H
#define KF_STACK_H

/*
 * kfStack.h (last modified 2025-05-27)
 * The stack file defines the stacks used by kopForth. Specifically the return
 * and data stacks.
 * These stacks grow down and the pointer points to the current "top" value.
 */

#include "kfBios.h"
#include "kfStatus.h"



// Macros to help with using the stacks.

#define KF_DATA_POP(var) KF_RETURN_IF_ERROR(kfDataStackPop(&forth->d_stack, (isize*) &var))
#define KF_RETN_POP(var) KF_RETURN_IF_ERROR(kfRetnStackPop(&forth->r_stack, (void**) &var))
#define KF_DATA_PUSH(var) KF_RETURN_IF_ERROR(kfDataStackPush(&forth->d_stack, (isize) var))
#define KF_RETN_PUSH(var) KF_RETURN_IF_ERROR(kfRetnStackPush(&forth->r_stack, (void*) var))



// Necessary typedef declarations for types.
typedef struct kfDataStack kfDataStack;
typedef struct kfRetnStack kfRetnStack;



struct kfDataStack {
    isize data[KF_DATA_STACK_SIZE];
    isize* ptr;
};

struct kfRetnStack {
    void* data[KF_RETN_STACK_SIZE];
    void** ptr;
};



void kfDataStackInit(kfDataStack* d_stack) {
    d_stack->ptr = &d_stack->data[KF_DATA_STACK_SIZE];
}
void kfRetnStackInit(kfRetnStack* r_stack) {
    r_stack->ptr = &r_stack->data[KF_RETN_STACK_SIZE];
}

bool kfDataStackEmpty(kfDataStack* d_stack) {
    return d_stack->ptr >= &d_stack->data[KF_DATA_STACK_SIZE];
}
bool kfRetnStackEmpty(kfRetnStack* r_stack) {
    return r_stack->ptr >= &r_stack->data[KF_RETN_STACK_SIZE];
}

bool kfDataStackFull(kfDataStack* d_stack) {
    return d_stack->ptr <= d_stack->data;
}
bool kfRetnStackFull(kfRetnStack* r_stack) {
    return r_stack->ptr <= r_stack->data;
}

kfStatus kfDataStackPush(kfDataStack* d_stack, isize value) {
    if (kfDataStackFull(d_stack))
        return KF_DATA_STACK_OVERFLOW;
    d_stack->ptr--;
    *d_stack->ptr = value;
    return KF_STATUS_OK;
}
kfStatus kfRetnStackPush(kfRetnStack* r_stack, void* value) {
    if (kfRetnStackFull(r_stack))
        return KF_RETN_STACK_OVERFLOW;
    r_stack->ptr--;
    *r_stack->ptr = value;
    return KF_STATUS_OK;
}

kfStatus kfDataStackPop(kfDataStack* d_stack, isize* value) {
    if (kfDataStackEmpty(d_stack))
        return KF_DATA_STACK_UNDERFLOW;
    *value = *d_stack->ptr;
    d_stack->ptr++;
    return KF_STATUS_OK;
}
kfStatus kfRetnStackPop(kfRetnStack* r_stack, void** value) {
    if (kfRetnStackEmpty(r_stack))
        return KF_RETN_STACK_UNDERFLOW;
    *value = *r_stack->ptr;
    r_stack->ptr++;
    return KF_STATUS_OK;
}

void kfDataStackPrint(kfDataStack* d_stack) {
    for (isize* ptr = &d_stack->data[KF_DATA_STACK_SIZE-1]; ptr >= d_stack->ptr; ptr--) {
        kfBiosPrintIsize(*ptr);
        kfBiosWriteChar(' ');
    }
}

#endif // KF_STACK_H
