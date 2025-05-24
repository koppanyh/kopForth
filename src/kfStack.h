#ifndef KF_STACK_H
#define KF_STACK_H

/*
 * kfStack.h (last modified 2025-05-23)
 * The stack file defines the stacks used by kopForth. Specifically the return
 * and data stacks.
 * These stacks grow down and the pointer points to the current "top" value.
 */

#include "kfBios.h"
#include "kfStatus.h"



typedef struct {
    isize data[DATA_STACK_SIZE];
    isize* ptr;
} DataStack;

typedef struct {
    void* data[RETN_STACK_SIZE];
    void** ptr;
} RetnStack;



void DataStackInit(DataStack* d_stack) {
    d_stack->ptr = &d_stack->data[DATA_STACK_SIZE];
}
void RetnStackInit(RetnStack* r_stack) {
    r_stack->ptr = &r_stack->data[RETN_STACK_SIZE];
}

bool DataStackEmpty(DataStack* d_stack) {
    return d_stack->ptr >= &d_stack->data[DATA_STACK_SIZE];
}
bool RetnStackEmpty(RetnStack* r_stack) {
    return r_stack->ptr >= &r_stack->data[RETN_STACK_SIZE];
}

bool DataStackFull(DataStack* d_stack) {
    return d_stack->ptr <= d_stack->data;
}
bool RetnStackFull(RetnStack* r_stack) {
    return r_stack->ptr <= r_stack->data;
}

Status DataStackPush(DataStack* d_stack, isize value) {
    if (DataStackFull(d_stack))
        return DATA_STACK_OVERFLOW;
    d_stack->ptr--;
    *d_stack->ptr = value;
    return STATUS_OK;
}
Status RetnStackPush(RetnStack* r_stack, void* value) {
    if (RetnStackFull(r_stack))
        return RETN_STACK_OVERFLOW;
    r_stack->ptr--;
    *r_stack->ptr = value;
    return STATUS_OK;
}

Status DataStackPop(DataStack* d_stack, isize* value) {
    if (DataStackEmpty(d_stack))
        return DATA_STACK_UNDERFLOW;
    *value = *d_stack->ptr;
    d_stack->ptr++;
    return STATUS_OK;
}
Status RetnStackPop(RetnStack* r_stack, void** value) {
    if (RetnStackEmpty(r_stack))
        return RETN_STACK_UNDERFLOW;
    *value = *r_stack->ptr;
    r_stack->ptr++;
    return STATUS_OK;
}

void DataStackPrint(DataStack* d_stack) {
    for (isize* ptr = &d_stack->data[DATA_STACK_SIZE-1]; ptr >= d_stack->ptr; ptr--) {
        BiosPrintIsize(*ptr);
        BiosWriteChar(' ');
    }
}

#endif // KF_STACK_H
