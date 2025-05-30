#ifndef KOP_FORTH_H
#define KOP_FORTH_H

/*
 * kopForth.h (last modified 2025-05-29)
 * This is the main kopForth file that gets included and pulls in all the
 * dependencies. It also includes the initialization and run routines.
 */

#include "kfBios.h"
#include "kfStack.h"
#include "kfType.h"
#include "kfWordsIntComp.h"
#include "kfWordsNative.h"
#include "kfWordsStackMem.h"
#include "kfWordsString.h"
#include "kfWordsVarAddrConst.h"



void kfPopulateWords(kopForth* forth) {
    // TODO Null check

    // Native words
    kfWordsNative wn;
    kfPopulateWordsNative(forth, &wn);
    forth->debug_words.ext = wn.ext;
    forth->debug_words.lit = wn.lit;
    forth->debug_words.bra = wn.bra;
    forth->debug_words.zbr = wn.zbr;
    forth->debug_words.typ = wn.typ;
    forth->debug_words.psq = wn.psq;

    // Variables, addresses, and constants
    kfWordsVarAddrConst wv;
    kfPopulateWordsVarAddrConst(forth, &wn, &wv);

    // Stack and address operators
    kfWordsStackMem wm;
    kfPopulateWordsStackMem(forth, &wn, &wv, &wm);

    // String operators
    kfWordsString ws;
    kfPopulateWordsString(forth, &wn, &wv, &wm, &ws);

    // Interpreter/Compiler words
    kfWordsIntComp wi;
    kfPopulateWordsIntComp(forth, &wn, &wv, &wm, &ws, &wi);
    forth->debug_words.abt = wi.abt;

//    kfWord* cou_word = kopForthAddWord(forth, "CNT"); {
//                       kopForthAddWordP(forth, wn.lit);  // 10
//                       kopForthAddIsize(forth, 10);//00000000);
//        kfWord** c0 =  kopForthAddWordP(forth, wn.lit);  // BEGIN 1 -
//                       kopForthAddIsize(forth, 1);
//                       kopForthAddWordP(forth, wn.sub);
//                       kopForthAddWordP(forth, wn.dup);  // DUP .
//                       kopForthAddWordP(forth, dot_word);
//                       kopForthAddWordP(forth, wn.dup);  // DUP IF
//                       kopForthAddWordP(forth, wn.zbr);
//        isize* c1 =    kopForthAddIsize(forth, 0);
//                       kopForthAddWordP(forth, wn.bra);  // AGAIN
//        isize* c2 =    kopForthAddIsize(forth, 0);
//        kfWord** c3 =  kopForthAddWordP(forth, wn.drp);  // THEN DROP
//                       kopForthAddWordP(forth, wn.ext);
//        *c1 = (isize) c3;
//        *c2 = (isize) c0; }
}



kfStatus kopForthInit(kopForth* forth) {
    if (sizeof(kfNativeFunc) != sizeof(kfWord*)) {
        kfBiosWriteStr("Pointer width mismatch");
        return KF_SYSTEM_PTRWIDTH;
    }

    for (usize i = 0; i < KF_MEM_SIZE; i++)
        forth->mem[i] = 0;
    forth->here = forth->mem;
    forth->latest = NULL;
    forth->pending = NULL;
    forth->state = false;

    kfDataStackInit(&forth->d_stack);
    kfRetnStackInit(&forth->r_stack);

    for (usize i = 0; i < KF_TIB_SIZE; i++)
        forth->tib[i] = 0;
    forth->tib_len = 0;
    forth->in_offset = 0;

    kfPopulateWords(forth);
    forth->latest = forth->pending;
    forth->pc = (uint8_t*) forth->debug_words.abt;

    kfWord* latest = (kfWord*) forth->latest;
    if ((usize) latest->name - (usize) &latest->name_len != 1) {
        kfBiosWriteStr("Bad name field offset in kfWord struct");
        return KF_SYSTEM_STRUCT;
    }

    kfBiosWriteStr("kopForth v0.2, ");
    kfBiosPrintIsize(sizeof(isize) * 8);
    kfBiosWriteStr(" Bit, 2025\n");
    kfBiosPrintIsize(forth->here - forth->mem);
    kfBiosWriteStr(" bytes used of ");
    kfBiosPrintIsize(sizeof(forth->mem));
    kfBiosWriteChar('\n');

    return KF_STATUS_OK;
}

kfStatus kopForthTick(kopForth* forth) {
    kfWord* cur_word = (kfWord*) forth->pc;
    /* Debug routine
    int t = &forth->r_stack.data[KF_RETN_STACK_SIZE] - forth->r_stack.ptr;
    for (int i = 0; i < t; i++)
        printf("  ");
    printf("%s ", cur_word->name);
    if (cur_word == forth->debug_words.lit ||
        cur_word == forth->debug_words.bra ||
        cur_word == forth->debug_words.zbr) {
        printf("(%d) ", *(isize*)(*forth->r_stack.ptr));
    }
    printf("%p < ", forth->pc);
    kfDataStackPrint(&forth->d_stack);
    printf(">\n");
    // */
    forth->pc = (uint8_t*) cur_word->word_def;
    if (cur_word->is_native) {
        kfNativeFunc fn = (kfNativeFunc) cur_word->word_def[0];
        KF_RETURN_IF_ERROR(fn(forth));
        // TODO change the handling here to drop into the shell or something.
        if (kfRetnStackEmpty(&forth->r_stack))
            return KF_SYSTEM_DONE;
        KF_RETURN_IF_ERROR(kfRetnStackPop(&forth->r_stack, (void**) &forth->pc));
    }
    kfWord* word_addr = *(kfWord**) forth->pc;
    KF_RETURN_IF_ERROR(kfRetnStackPush(&forth->r_stack, forth->pc + sizeof(kfWord*)));
    forth->pc = (uint8_t*) word_addr;
    return KF_STATUS_OK;
}

#endif // KOP_FORTH_H
