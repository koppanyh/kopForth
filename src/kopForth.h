#ifndef KOP_FORTH_H
#define KOP_FORTH_H

/*
 * kopForth.h (last modified 2025-06-12)
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



kfStatus kfPopulateWords(kopForth* forth) {
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

    return KF_STATUS_OK;
}

void kfDebug(kopForth* forth) {
    isize depth = &forth->r_stack.data[KF_RETN_STACK_SIZE] - forth->r_stack.ptr;
    for (isize i = 0; i < depth; i++) {
        kfBiosWriteStr("  ");
    }
    kfWord* cur_word = (kfWord*) forth->pc;
    kfBiosWriteStrLen(cur_word->name, cur_word->name_len);
    kfBiosWriteChar(' ');
    if (cur_word == forth->debug_words.lit ||
        cur_word == forth->debug_words.bra ||
        cur_word == forth->debug_words.zbr) {
        kfBiosWriteChar('(');
        kfBiosPrintIsize(*(isize*)(*forth->r_stack.ptr));
        kfBiosWriteStr(") ");
    }
    kfBiosPrintPointer(cur_word);
    kfBiosWriteStr(" < ");
    kfDataStackPrint(&forth->d_stack);
    kfBiosWriteChar('>'); kfBiosCR();
}

//////////////////////////////////
// Internal functions         ▲ //
//////////////////////////////////
// User-accessible functions  ▼ //
//////////////////////////////////

kfStatus kopForthTest() {
    kfBiosWriteStr("Running self checks..."); kfBiosCR();

    // Make sure that our data type can actually be converted into a pointer and
    // vice versa.
    if (sizeof(isize) != sizeof(void*) || sizeof(usize) != sizeof(void*)) {
        kfBiosWriteStr("Integer width mismatch"); kfBiosCR();
        return KF_TEST_PTR_WIDTH;
    }
    // Makes sure that the pointer to a function is the same as a pointer to
    // memory. This is important to catch Harvard architecture CPUs that split
    // program and data across memories with different address sizes.
    if (sizeof(kfNativeFunc) != sizeof(kfWord*)) {
        kfBiosWriteStr("Function pointer width mismatch"); kfBiosCR();
        return KF_TEST_PTR_WIDTH;
    }
    // Secondary confirmation to check that the kfWordDef size is correct.
    if (sizeof(kfNativeFunc) != sizeof(kfWordDef)) {
        kfBiosWriteStr("Union width mismatch with kfWordDef"); kfBiosCR();
        return KF_TEST_PTR_WIDTH;
    }

    // Tests to make sure the compiler isn't doing any funny business. The words
    // written in forth depend on these field positions being correct.
    kfWord word;
    // Check that the name length variable is only 1 byte.
    if ((usize) word.name - (usize) &word.name_len != 1) {
        kfBiosWriteStr("Bad `name_len` size in kfWord"); kfBiosCR();
        return KF_TEST_STRUCT;
    }
    // Check that the name char array is actually the size it's supposed to be.
    if ((usize) &word.link - (usize) word.name != KF_MAX_NAME_SIZE) {
        kfBiosWriteStr("Bad `name` size in kfWord"); kfBiosCR();
        return KF_TEST_STRUCT;
    }
    // Check that the word link is actually the size of a pointer.
    if ((usize) &word.flags - (usize) &word.link != sizeof(kfWord*)) {
        kfBiosWriteStr("Bad `link` size in kfWord"); kfBiosCR();
        return KF_TEST_STRUCT;
    }
    // Check that the flags variable is only 1 byte.
    if ((usize) &word.word_def - (usize) &word.flags != 1) {
        kfBiosWriteStr("Bad `flags` size in kfWord"); kfBiosCR();
        return KF_TEST_STRUCT;
    }

    // Tests to make sure the compiler isn't doing any funny business. The words
    // written in forth depend on these flag positions being correct.
    word.flags.raw_flags = 0;
    // Test that the native flag is in the right place.
    word.flags.bit_flags.is_native = 1;
    if (word.flags.raw_flags != 0b00000001) {
        kfBiosWriteStr("Bad `is_native` position in kfWordFlags"); kfBiosCR();
        return KF_TEST_STRUCT;
    }
    word.flags.raw_flags = 0;
    // Test that the immediate flag is in the right place.
    word.flags.bit_flags.is_immediate = 1;
    if (word.flags.raw_flags != 0b00000010) {
        kfBiosWriteStr("Bad `is_immediate` position in kfWordFlags"); kfBiosCR();
        return KF_TEST_STRUCT;
    }

    return KF_STATUS_OK;
}

kfStatus kopForthInit(kopForth* forth) {
    // Setup memory and system variables.
    for (usize i = 0; i < KF_MEM_SIZE; i++)
        forth->mem[i] = 0;
    forth->here = forth->mem;
    forth->latest = NULL;
    forth->pending = NULL;
    forth->state = false;
    #ifdef KF_DEBUG
        forth->debug = true;
    #else
        forth->debug = false;
    #endif

    // Initialize stacks.
    kfDataStackInit(&forth->d_stack);
    kfRetnStackInit(&forth->r_stack);

    // Setup terminal input buffer.
    for (usize i = 0; i < KF_TIB_SIZE; i++)
        forth->tib[i] = 0;
    forth->tib_len = 0;
    forth->in_offset = 0;

    // Initialize the word dictionary.
    KF_RETURN_IF_ERROR(kfPopulateWords(forth));
    forth->latest = forth->pending;
    forth->pc = (uint8_t*) forth->debug_words.abt;

    kfBiosPrintIsize(forth->here - forth->mem);
    kfBiosWriteStr(" bytes used of ");
    kfBiosPrintIsize(sizeof(forth->mem));
    kfBiosCR();

    return KF_STATUS_OK;
}

kfStatus kopForthTick(kopForth* forth) {
    if (forth->debug) {
        kfDebug(forth);
    }
    kfWord* cur_word = (kfWord*) forth->pc;
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
