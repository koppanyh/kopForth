#ifndef KF_STACK_H
#define KF_STACK_H

/*
 * kfStack.h (last modified 2025-05-22)
 * The stack file defines the stacks used by kopForth. Specifically the return
 * and data stacks.
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
        return MakeStatus(STACK, STACK_OVERFLOW, 0, "DATA OVERFLOW");
    d_stack->ptr--;
    *d_stack->ptr = value;
    return OkStatus;
}
Status RetnStackPush(RetnStack* r_stack, void* value) {
    if (RetnStackFull(r_stack))
        return MakeStatus(STACK, STACK_OVERFLOW, 0, "RETN OVERFLOW");
    r_stack->ptr--;
    *r_stack->ptr = value;
    return OkStatus;
}

Status DataStackPop(DataStack* d_stack, isize* value) {
    if (DataStackEmpty(d_stack))
        return MakeStatus(STACK, STACK_UNDERFLOW, 0, "DATA UNDERFLOW");
    *value = *d_stack->ptr;
    d_stack->ptr++;
    return OkStatus;
}
Status RetnStackPop(RetnStack* r_stack, void** value) {
    if (RetnStackEmpty(r_stack))
        return MakeStatus(STACK, STACK_UNDERFLOW, 0, "RETN UNDERFLOW");
    *value = *r_stack->ptr;
    r_stack->ptr++;
    return OkStatus;
}

void DataStackPrint(DataStack* d_stack) {
    for (isize* ptr = &d_stack->data[DATA_STACK_SIZE-1]; ptr >= d_stack->ptr; ptr--) {
        BiosPrintIsize(*ptr);
        BiosWriteChar(' ');
    }
}

#endif // KF_STACK_H
