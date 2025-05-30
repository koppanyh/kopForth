#ifndef KF_WORDS_STACK_MEM_H
#define KF_WORDS_STACK_MEM_H

/*
 * kfWordsStackMem.h (last modified 2025-05-29)
 * This contains the word definitions for stack and address operators.
 */

#include "kfType.h"
#include "kfWordsNative.h"
#include "kfWordsVarAddrConst.h"



// Pointers to words created in this file, for usage in defining other words.
typedef struct kfWordsStackMem kfWordsStackMem;
struct kfWordsStackMem {
    kfWord* ovr;
    kfWord* rot;
    kfWord* tdr;
    kfWord* tdu;
    kfWord* add;
    kfWord* inv;
    kfWord* orr;
    kfWord* and;
    kfWord* zeq;
    kfWord* neq;
    kfWord* leq;
    kfWord* gtr;
    kfWord* geq;
    kfWord* cls;
    kfWord* pex;
    kfWord* alt;
    kfWord* com;
    kfWord* cco;
};



// Fill stack/memory words into memory.
void kfPopulateWordsStackMem(kopForth* forth, kfWordsNative* wn,
                             kfWordsVarAddrConst* wv, kfWordsStackMem* wm) {
    // TODO Null check.

    // Stack manipulators and operators
    wm->ovr = kopForthAddWord(forth, "OVER");      // ( n1 n2 -- n1 n2 n1 )
        WRD(wn->rpu); WRD(wn->dup);                // >R DUP   ( n1 n1 )
        WRD(wn->rpo); WRD(wn->swp);                // R> SWAP  ( n1 n2 n1 )
        WRD(wn->ext);
    wm->rot = kopForthAddWord(forth, "ROT");       // ( n1 n2 n3 -- n2 n3 n1 )
        WRD(wn->rpu); WRD(wn->swp);                // >R SWAP  ( n2 n1 )
        WRD(wn->rpo); WRD(wn->swp);                // R> SWAP  ( n2 n3 n1 )
        WRD(wn->ext);
    wm->tdr = kopForthAddWord(forth, "2DROP");     // ( n1 n2 -- )
        WRD(wn->drp); WRD(wn->drp);                // DROP DROP
        WRD(wn->ext);
    wm->tdu = kopForthAddWord(forth, "2DUP");      // ( n1 n2 -- n1 n2 n1 n2 )
        WRD(wm->ovr); WRD(wm->ovr);                // OVER OVER
        WRD(wn->ext);
    wm->add = kopForthAddWord(forth, "+");         // ( n1 n2 -- n3 )
        LIT(0); WRD(wn->swp);                      // 0 SWAP  ( n1 0 n2 )
        WRD(wn->sub); WRD(wn->sub);                // - -     ( n3 )
        WRD(wn->ext);
    wm->inv = kopForthAddWord(forth, "INVERT");    // ( n1 -- n2 )
        WRD(wn->dup); WRD(wn->nan);                // DUP NAND
        WRD(wn->ext);
    wm->orr = kopForthAddWord(forth, "OR");        // ( n1 n2 -- n3 )
        WRD(wm->inv); WRD(wn->swp);                // INVERT SWAP  ( n4 n1 )
        WRD(wm->inv); WRD(wn->nan);                // INVERT NAND  ( n3 )
        WRD(wn->ext);
    wm->and = kopForthAddWord(forth, "AND");       // ( n1 n2 -- n3 )
        WRD(wn->nan); WRD(wm->inv);                // NAND INVERT
        WRD(wn->ext);
    wm->zeq = kopForthAddWord(forth, "0=");        // ( n1 -- n2 )
        WRD(wv->fal); WRD(wn->equ);                // 0 =
        WRD(wn->ext);
    wm->neq = kopForthAddWord(forth, "<>");        // ( n1 n2 -- n3 )
        WRD(wn->equ); WRD(wm->zeq);                // = 0=
        WRD(wn->ext);
    wm->leq = kopForthAddWord(forth, "<=");        // ( n1 n2 -- n3 )
        WRD(wm->tdu);                              // 2DUP     ( n1 n2 n1 n2 )
        WRD(wn->lss); WRD(wn->rpu);                // < >R     ( n1 n2 )
        WRD(wn->equ); WRD(wn->rpo); WRD(wm->orr);  // = R> OR  ( n3 )
        WRD(wn->ext);
    wm->gtr = kopForthAddWord(forth, ">");         // ( n1 n2 -- n3 )
        WRD(wm->leq); WRD(wm->zeq);                // <= 0=
        WRD(wn->ext);
    wm->geq = kopForthAddWord(forth, ">=");        // ( n1 n2 -- n3 )
        WRD(wn->lss); WRD(wm->zeq);                // < 0=
        WRD(wn->ext);

    // Address manipulators
    wm->cls = kopForthAddWord(forth, "CELLS");     // ( n -- n )
        LIT(sizeof(isize)); WRD(wn->mul);          // 8 *
        WRD(wn->ext);
    wm->pex = kopForthAddWord(forth, "+!");        // ( n a -- )
        WRD(wn->dup); WRD(wn->att);                // DUP @    ( n a n2 )
        WRD(wn->swp);                              // SWAP     ( n n2 a )
        WRD(wn->rpu); WRD(wm->add); WRD(wn->rpo);  // >R + R>  ( n3 a )
        WRD(wn->exc);                              // !
        WRD(wn->ext);
    wm->alt = kopForthAddWord(forth, "ALLOT");     // ( n -- )
        WRD(wv->dpt); WRD(wm->pex);                // DP +!
        WRD(wn->ext);
    wm->com = kopForthAddWord(forth, ",");         // ( n -- )
        WRD(wv->her); WRD(wn->exc);                // HERE !
        LIT(sizeof(isize));                        // [ 1 CELLS ] LITERAL
        WRD(wm->alt);                              // ALLOT
        WRD(wn->ext);
    wm->cco = kopForthAddWord(forth, "C,");        // ( n -- )
        WRD(wv->her); WRD(wn->cex);                // HERE C!
        LIT(sizeof(uint8_t));                      // [ 1 CHARS ] LITERAL
        WRD(wm->alt);                              // ALLOT
        WRD(wn->ext);
}

#endif // KF_WORDS_STACK_MEM_H
