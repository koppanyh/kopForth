#ifndef KF_WORDS_STRING_H
#define KF_WORDS_STRING_H

/*
 * kfWordsString.h (last modified 2025-05-29)
 * This contains the word definitions for string/char related stuff.
 */

#include "kfType.h"
#include "kfWordsNative.h"
#include "kfWordsStackMem.h"
#include "kfWordsVarAddrConst.h"



// Pointers to words created in this file, for usage in defining other words.
typedef struct kfWordsString kfWordsString;
struct kfWordsString {
    kfWord* crr;
    kfWord* bla;
    kfWord* spa;
    kfWord* cnt;
    kfWord* sst;
    kfWord* dig;
    kfWord* num;
    kfWord* snu;
};



// Fill string words into memory.
void kfPopulateWordsString(kopForth* forth, kfWordsNative* wn,
                           kfWordsVarAddrConst* wv, kfWordsStackMem* wm,
                           kfWordsString* ws) {
    // TODO Null check.

    ws->crr = kopForthAddWord(forth, "CR");        // ( -- )
        LIT(KF_NL); WRD(wn->emt);                  // 10 EMIT
        WRD(wn->ext);
    ws->bla = kopForthAddWord(forth, "BL");        // ( -- 32 )
        LIT(' ');                                  // 32
        WRD(wn->ext);
    ws->spa = kopForthAddWord(forth, "SPACE");     // ( -- )
        WRD(ws->bla); WRD(wn->emt);                // BL EMIT
        WRD(wn->ext);
    ws->cnt = kopForthAddWord(forth, "COUNT");     // ( a1 -- a2 u )
        WRD(wn->dup); LIT(1); WRD(wm->add);        // DUP 1 +  ( a1 a2 )
        WRD(wn->swp); WRD(wn->cat);                // SWAP C@  ( a2 u )
        WRD(wn->ext);
    ws->sst = kopForthAddWord(forth, "/STRING");   // ( a1 u1 n -- a2 u2 )
        WRD(wn->dup); WRD(wn->rpu); WRD(wn->rpu);  // DUP >R >R  ( a1 u1 )
        WRD(wn->swp); WRD(wn->rpo); WRD(wm->add);  // SWAP R> +  ( u1 a2 )
        WRD(wn->swp); WRD(wn->rpo); WRD(wn->sub);  // SWAP R> -  ( a2 u2 )
        WRD(wn->ext);

    ws->dig = kopForthAddWord(forth, "DIGIT?"); {           // ( n1 -- n2 -1 | 0 )
        LIT(48); WRD(wn->sub);                              // 48 -            ( n2 )
        WRD(wn->dup); LIT(0); WRD(wn->lss);                 // DUP 0 <         ( n2 f1 )
        WRD(wm->ovr); LIT(10); WRD(wm->geq);                // OVER 10 >=      ( n2 f1 f2 )
        WRD(wm->orr); LITADDR(b00, wn->zbr, 0);             // OR IF           ( n2 )
        WRD(wn->drp); WRD(wv->fal);                         //     DROP FALSE  ( 0 )
        LITADDR(b01, wn->bra, 0);                           // ELSE
        WRDADDR(b02, wv->tru);                              //     TRUE        ( n2 -1 )
        WRDADDR(b03, wn->ext);                              // THEN
        *b00 = (isize) b02;
        *b01 = (isize) b03; }
    ws->num = kopForthAddWord(forth, ">NUMBER"); {          // ( ud1 a1 u1 -- ud2 a2 u2 )
        kfWord** b00 =                                      // BEGIN
        WRD(wn->dup); WRD(wm->zeq);                         //     DUP 0=           ( ud a u f )
        LITADDR(b01, wn->zbr, 0);                           //     IF               ( ud a u )
        WRD(wn->ext);                                       //         EXIT THEN
        WRDADDR(b02, wm->ovr); WRD(wn->cat); WRD(ws->dig);  //     OVER C@ DIGIT?   ( ud a u n f )
        LITADDR(b03, wn->zbr, 0);                           //     IF               ( ud a u n )
        WRD(wn->swp); LIT(1); WRD(wn->sub); WRD(wn->rpu);   //         SWAP 1 - >R  ( ud a n )
        WRD(wn->swp); LIT(1); WRD(wm->add); WRD(wn->rpu);   //         SWAP 1 + >R  ( ud n )
        WRD(wn->rpu);                                       //         >R           ( ud )
        LIT(10); LIT(1); WRD(wn->mss);                      //         10 1 M*/     ( ud )
        WRD(wn->rpo); LIT(0); WRD(wn->dpl);                 //         R> 0 D+      ( ud )
        WRD(wn->rpo); WRD(wn->rpo);                         //         R> R>        ( ud a u )
        LITADDR(b04, wn->bra, 0);                           //     ELSE
        WRDADDR(b05, wn->ext);                              //         EXIT THEN
        WRDADDR(b06, wn->bra); RAWADDR(b07, 0);             // AGAIN
        WRD(wn->ext);
        *b01 = (isize) b02;
        *b03 = (isize) b05;
        *b04 = (isize) b06;
        *b07 = (isize) b00; }
    ws->snu = kopForthAddWord(forth, "S>NUMBER?"); {        // ( a1 u1 -- n 0 0 | d -1 0 | a2 u2 )
        // \ Save double status (true if need to drop high word)
        WRD(wm->tdu); WRD(wm->add); LIT(1); WRD(wn->sub);   // 2DUP + 1 -
        WRD(wn->cat); LIT('.'); WRD(wn->equ);               // C@ [CHAR] . =
        LITADDR(b00, wn->zbr, 0);                           // IF
        WRD(wv->fal); WRD(wn->rpu);                         //     FALSE >R
        LIT(1); WRD(wn->sub);                               //     1 -
        LITADDR(b01, wn->bra, 0);                           // ELSE
        WRDADDR(b02, wv->tru); WRD(wn->rpu);                //     TRUE >R THEN
        // \ Save sign multiplier
        WRDADDR(b03, wm->ovr); WRD(wn->cat);                // OVER C@
        LIT('-'); WRD(wn->equ);                             // [CHAR] - =
        LITADDR(b04, wn->zbr, 0);                           // IF
        LIT(-1); WRD(wn->rpu);                              //     -1 >R
        LIT(1); WRD(ws->sst);                               //     1 /STRING
        LITADDR(b05, wn->bra, 0);                           // ELSE
        WRDADDR(b06, wn->lit); RAW(1); WRD(wn->rpu);        //     1 >R THEN
        // \ Add 0. to beginning of stack and parse number
        WRDADDR(b07, wn->rpu); WRD(wn->rpu);                // >R >R
        LIT(0); LIT(0); WRD(wn->rpo); WRD(wn->rpo);         // 0. R> R>           ( 0. a u )
        WRD(ws->num);                                       // >NUMBER            ( ud a u )
        // \ Check that the parsing was good
        WRD(wn->dup); WRD(wm->zeq);                         // DUP 0=
        LITADDR(b08, wn->zbr, 0);                           // IF  \ Success
        WRD(wm->tdr);                                       //     2DROP          ( ud )
        WRD(wn->rpo); LIT(1); WRD(wn->mss);                 //     R> 1 M*/       ( d )
        WRD(wn->rpo); LITADDR(b09, wn->zbr, 0);             //     R> IF
        WRD(wn->drp); LIT(0); LIT(0);                       //         DROP 0 0   ( n 0 0 )
        LITADDR(b10, wn->bra, 0);                           //     ELSE
        WRDADDR(b11, wn->lit); RAW(-1); LIT(0);             //         -1 0 THEN  ( d -1 0 )
        WRDADDR(b12, wn->bra); RAWADDR(b13, 0);             // ELSE  \ Failure
        WRDADDR(b14, wn->rpu); WRD(wn->rpu);                //     >R >R
        WRD(wm->tdr); WRD(wn->rpo); WRD(wn->rpo);           //     2DROP R> R>    ( addr u )
        WRD(wn->rpo); WRD(wn->rpo); WRD(wm->tdr);           //     R> R> 2DROP
        WRDADDR(b15, wn->ext);                              // THEN EXIT
        *b00 = (isize) b02;
        *b01 = (isize) b03;
        *b04 = (isize) b06;
        *b05 = (isize) b07;
        *b08 = (isize) b14;
        *b09 = (isize) b11;
        *b10 = (isize) b12;
        *b13 = (isize) b15; }
}

#endif // KF_WORDS_STRING_H
