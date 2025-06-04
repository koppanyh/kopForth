#ifndef KOP_FORTH_H
#define KOP_FORTH_H

/*
 * kopForth.h (last modified 2025-06-04)
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

    /* Example word definition
    kfWord* cou_word = kopForthAddWord(forth, "CNT"); {
                       kopForthAddWordP(forth, wn.lit);  // 10
                       kopForthAddIsize(forth, 10);//00000000);
        kfWord** c0 =  kopForthAddWordP(forth, wn.lit);  // BEGIN 1 -
                       kopForthAddIsize(forth, 1);
                       kopForthAddWordP(forth, wn.sub);
                       kopForthAddWordP(forth, wn.dup);  // DUP .
                       kopForthAddWordP(forth, dot_word);
                       kopForthAddWordP(forth, wn.dup);  // DUP IF
                       kopForthAddWordP(forth, wn.zbr);
        isize* c1 =    kopForthAddIsize(forth, 0);
                       kopForthAddWordP(forth, wn.bra);  // AGAIN
        isize* c2 =    kopForthAddIsize(forth, 0);
        kfWord** c3 =  kopForthAddWordP(forth, wn.drp);  // THEN DROP
                       kopForthAddWordP(forth, wn.ext);
        *c1 = (isize) c3;
        *c2 = (isize) c0; }
    */
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
    kfBiosWriteStr(" Bit, 2025");
    #ifdef KF_IS_WINDOWS
        kfBiosWriteStr(", Windows Edition");
    #endif
    kfBiosWriteChar('\n');
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
    if (cur_word->flags.bit_flags.is_native) {
        KF_RETURN_IF_ERROR(cur_word->word_def.native(forth));
        KF_RETURN_IF_ERROR(kfRetnStackPop(&forth->r_stack, (void**) &forth->pc));
    } else {
        forth->pc = (uint8_t*) cur_word->word_def.forth;
    }
    KF_RETURN_IF_ERROR(kfRetnStackPush(&forth->r_stack, forth->pc + sizeof(kfWord*)));
    forth->pc = *(uint8_t**) forth->pc;
    return KF_STATUS_OK;
}

/* Program execution example 1

0x0000: "lit"  n:1
0x0004: <lit_fn>
0x0008: "+"    n:1
0x000C: <+_fn>
0x0010: "."    n:1
0x0014: <._fn>
0x0018: "exit" n:1
0x001C: <exit_fn>
0x0020: "test" n:0
0x0024: 0x0000 <lit>
0x0028: 5
0x002C: 0x0000 <lit>
0x0030: 6
0x0034: 0x0008 <+>
0x0038: 0x0010 <.>
0x003C: 0x0018 <exit>

pc = 0x0020

cw = 0x0020
pc = 0x0024
rs = [0x0028]
pc = 0x0000

cw = 0x0000
ds = [5]
rs = [0x002C]
pc = 0x002C
rs = []
rs = [0x0030]
pc = 0x0000

cw = 0x0000
ds = [5, 6]
rs = [0x0034]
pc = 0x0034
rs = []
rs = [0x0038]
pc = 0x0008

cw = 0x0008
ds = [11]
pc = 0x0038
rs = []
rs = [0x003C]
pc = 0x0010

cw = 0x0010
ds = []
pc = 0x003C
rs = []
rs = [0x0040]
pc = 0x0018

cw = 0x0018
rs = []
done
*/

/* Program execution example 2

0x0000: "lit"   n:1
0x0004: <lit_fn>
0x0008: "exit"  n:1
0x000C: <exit_fn>
0x0010: "test"  n:0
0x0014: 0x0000 <lit>
0x0018: 5
0x001C: 0x0008 <exit>
0x0020: "test2" n:0
0x0024: 0x0010 <test>
0x0028: 0x0010 <test>
0x002C: 0x0008 <exit>

pc = 0x0020

cw = 0x0020
pc = 0x0024
rs = [0x0028]
pc = 0x0010

cw = 0x0010
pc = 0x0014
rs = [0x0028, 0x0018]
pc = 0x0000

cw = 0x0000
ds = [5]
rs = [0x0028, 0x001C]
pc = 0x001C
rs = [0x0028]
rs = [0x0028, 0x0020]
pc = 0x0008

cw = 0x0008
rs = [0x0028]
pc = 0x0028
rs = []
rs = [0x002C]
pc = 0x0010

cw = 0x0010
pc = 0x0014
rs = [0x002C, 0x0018]
pc = 0x0000

cw = 0x0000
ds = [5, 5]
rs = [0x002C, 0x001C]
pc = 0x001C
rs = [0x002C]
rs = [0x002C, 0x0020]
pc = 0x0008

cw = 0x0008
rs = [0x002C]
pc = 0x002C
rs = []
rs = [0x0030]
pc = 0x0008

cw = 0x0008
rs = []
done
*/

#endif // KOP_FORTH_H
