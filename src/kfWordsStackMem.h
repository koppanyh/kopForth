#ifndef KF_WORDS_STACK_MEM_H
#define KF_WORDS_STACK_MEM_H

/*
 * kfWordsStackMem.h (last modified 2025-10-10)
 * This contains the word definitions for stack and memory operators.
 */

#include "kfType.h"
#include "kfWordsNative.h"
#include "kfWordsVarAddrConst.h"



// Pointers to words created in this file, for usage in defining other words.
typedef struct kfWordsStackMem kfWordsStackMem;
struct kfWordsStackMem {
    kfWord* ovr;  // OVER
    kfWord* rot;  // ROT
    kfWord* tdr;  // 2DROP
    kfWord* tdu;  // 2DUP
    kfWord* add;  // +
    kfWord* inv;  // INVERT
    kfWord* orr;  // OR
    kfWord* and;  // AND
    kfWord* zeq;  // 0=
    kfWord* neq;  // <>
    kfWord* leq;  // <=
    kfWord* gtr;  // >
    kfWord* geq;  // >=
    kfWord* rat;  // R@
    kfWord* cls;  // CELLS
    kfWord* pex;  // +!
    kfWord* alt;  // ALLOT
    kfWord* com;  // ,
    kfWord* cco;  // C,
    kfWord* exe;  // EXECUTE
    kfWord* mov;  // MOVE
};



// Fill stack/memory words into memory.
void kfPopulateWordsStackMem(kopForth* forth, kfWordsNative* wn,
                             kfWordsVarAddrConst* wv, kfWordsStackMem* wm) {
    // TODO Null check.

    //////////////////////////////////////
    // Stack manipulators and operators //
    //////////////////////////////////////

    wm->ovr = kopForthAddWord(forth, "OVER");  // ( n1 n2 -- n1 n2 n1 )
        WRD(wn->rpu); WRD(wn->dup);            // >R DUP   ( n1 n1 )
        WRD(wn->rpo); WRD(wn->swp);            // R> SWAP  ( n1 n2 n1 )
        WRD(wn->ext);

    wm->rot = kopForthAddWord(forth, "ROT");  // ( n1 n2 n3 -- n2 n3 n1 )
        WRD(wn->rpu); WRD(wn->swp);           // >R SWAP  ( n2 n1 )
        WRD(wn->rpo); WRD(wn->swp);           // R> SWAP  ( n2 n3 n1 )
        WRD(wn->ext);

    wm->tdr = kopForthAddWord(forth, "2DROP");  // ( n1 n2 -- )
        WRD(wn->drp); WRD(wn->drp);             // DROP DROP
        WRD(wn->ext);

    wm->tdu = kopForthAddWord(forth, "2DUP");  // ( n1 n2 -- n1 n2 n1 n2 )
        WRD(wm->ovr); WRD(wm->ovr);            // OVER OVER
        WRD(wn->ext);

    wm->add = kopForthAddWord(forth, "+");  // ( n1 n2 -- n3 )
        LIT(0); WRD(wn->swp);               // 0 SWAP  ( n1 0 n2 )
        WRD(wn->sub); WRD(wn->sub);         // - -     ( n3 )
        WRD(wn->ext);

    wm->inv = kopForthAddWord(forth, "INVERT");  // ( n1 -- n2 )
        WRD(wn->dup); WRD(wn->nan);              // DUP NAND
        WRD(wn->ext);

    wm->orr = kopForthAddWord(forth, "OR");  // ( n1 n2 -- n3 )
        WRD(wm->inv); WRD(wn->swp);          // INVERT SWAP  ( n4 n1 )
        WRD(wm->inv); WRD(wn->nan);          // INVERT NAND  ( n3 )
        WRD(wn->ext);

    wm->and = kopForthAddWord(forth, "AND");  // ( n1 n2 -- n3 )
        WRD(wn->nan); WRD(wm->inv);           // NAND INVERT
        WRD(wn->ext);

    wm->zeq = kopForthAddWord(forth, "0=");  // ( n1 -- n2 )
        WRD(wv->fal); WRD(wn->equ);          // 0 =
        WRD(wn->ext);

    wm->neq = kopForthAddWord(forth, "<>");  // ( n1 n2 -- n3 )
        WRD(wn->equ); WRD(wm->zeq);          // = 0=
        WRD(wn->ext);

    wm->leq = kopForthAddWord(forth, "<=");        // ( n1 n2 -- n3 )
        WRD(wm->tdu);                              // 2DUP     ( n1 n2 n1 n2 )
        WRD(wn->lss); WRD(wn->rpu);                // < >R     ( n1 n2 )
        WRD(wn->equ); WRD(wn->rpo); WRD(wm->orr);  // = R> OR  ( n3 )
        WRD(wn->ext);

    wm->gtr = kopForthAddWord(forth, ">");  // ( n1 n2 -- n3 )
        WRD(wm->leq); WRD(wm->zeq);         // <= 0=
        WRD(wn->ext);

    wm->geq = kopForthAddWord(forth, ">=");  // ( n1 n2 -- n3 )
        WRD(wn->lss); WRD(wm->zeq);          // < 0=
        WRD(wn->ext);

    wm->rat = kopForthAddWord(forth, "R@");        // ( -- n1 )[ n1 n2 -- n1 n2]
        WRD(wn->rpo); WRD(wn->rpo); WRD(wn->dup);  // R> R> DUP   ( n2 n1 n1 )[  ]
        WRD(wn->rpu); WRD(wn->swp); WRD(wn->rpu);  // >R SWAP >R  ( n1 )[ n1 n2 ]
        WRD(wn->ext);

    /////////////////////////
    // Memory manipulators //
    /////////////////////////

    wm->cls = kopForthAddWord(forth, "CELLS");  // ( n -- n )
        LIT(sizeof(isize)); WRD(wn->mul);       // 8 *
        WRD(wn->ext);

    wm->pex = kopForthAddWord(forth, "+!");        // ( n a -- )
        WRD(wn->dup); WRD(wn->att);                // DUP @    ( n a n2 )
        WRD(wn->swp);                              // SWAP     ( n n2 a )
        WRD(wn->rpu); WRD(wm->add); WRD(wn->rpo);  // >R + R>  ( n3 a )
        WRD(wn->exc);                              // !
        WRD(wn->ext);

    wm->alt = kopForthAddWord(forth, "ALLOT");  // ( n -- )
        WRD(wv->dpt); WRD(wm->pex);             // DP +!
        WRD(wn->ext);

    wm->com = kopForthAddWord(forth, ",");  // ( n -- )
        WRD(wv->her); WRD(wn->exc);         // HERE !
        LIT(sizeof(isize));                 // [ 1 CELLS ] LITERAL
        WRD(wm->alt);                       // ALLOT
        WRD(wn->ext);

    wm->cco = kopForthAddWord(forth, "C,");  // ( n -- )
        WRD(wv->her); WRD(wn->cex);          // HERE C!
        LIT(sizeof(uint8_t));                // [ 1 CHARS ] LITERAL
        WRD(wm->alt);                        // ALLOT
        WRD(wn->ext);

    wm->exe = kopForthAddWord(forth, "EXECUTE"); {  // ( xt -- )
        LITADDR(b00, wn->lit, 0);                   // <addr> ! <xt>
        WRD(wn->exc);
        WRDADDR(b01, (kfWord*) 0);
        WRD(wn->ext);
        LINK(b00, b01); }

    wm->mov = kopForthAddWord(forth, "MOVE"); {            // ( addr1 addr2 u -- )
        WRD(wm->rot); WRD(wm->rot); WRD(wm->tdu);          // ROT ROT 2DUP       ( u addr1 addr2 addr1 addr2 )
        WRD(wn->lss); LITADDR(b00, wn->zbr, 0);            // < IF               ( u addr1 addr2 )  \ Work backwards
        WRD(wn->swp); WRD(wm->rot); WRD(wn->swp);          //     SWAP ROT SWAP  ( addr2 u addr1 )
        WRD(wm->ovr); WRD(wm->add); LIT(1); WRD(wn->sub);  //     OVER + 1 -     ( addr2 u addr3 )
        WRD(wm->rot); WRD(wm->rot); WRD(wn->swp);          //     ROT ROT SWAP   ( addr3 u addr2 )
        WRD(wm->ovr); WRD(wm->add); LIT(1); WRD(wn->sub);  //     OVER + 1-      ( addr3 u addr4 )
        WRD(wm->rot); WRD(wn->swp);                        //     ROT SWAP       ( u addr3 addr4 )
        LIT(-1);                                           //     -1             ( u addr3 addr4 -1 )
        LITADDR(b01, wn->bra, 0);                          // ELSE               ( u addr1 addr2 )
        WRDADDR(b02, wn->lit); RAW(1);                     //     1              ( u addr1 addr2 1 )
                                                           // THEN
        WRDADDR(b03, wn->rpu);                             // >R                 ( u addr1 addr2 )[ 1|-1 ]
                                                           // BEGIN              ( u addr3 addr4 )[ 1|-1 ]
        WRDADDR(b04, wm->rot); WRD(wn->dup);               //     ROT DUP        ( addr3 addr4 u u )[ 1|-1 ]
        LITADDR(b05, wn->zbr, 0);                          // WHILE              ( addr3 addr4 u )[ 1|-1 ]
        LIT(1); WRD(wn->sub);                              //     1 -            ( addr3 addr4 u )[ 1|-1 ]
        WRD(wn->swp); WRD(wm->rot);                        //     SWAP ROT       ( u addr4 addr3 )[ 1|-1 ]
        WRD(wn->dup); WRD(wn->cat);                        //     DUP C@         ( u addr4 addr3 n )[ 1|-1 ]
        WRD(wm->rot); WRD(wn->swp);                        //     ROT SWAP       ( u addr3 addr4 n )[ 1|-1 ]
        WRD(wm->ovr); WRD(wn->cex);                        //     OVER C!        ( u addr3 addr4 )[ 1|-1 ]
        WRD(wn->swp); WRD(wm->rat); WRD(wm->add);          //     SWAP R@ +      ( u addr4 addr3 )[ 1|-1 ]
        WRD(wn->swp); WRD(wm->rat); WRD(wm->add);          //     SWAP R@ +      ( u addr3 addr4 )[ 1|-1 ]
        LITADDR(b06, wn->bra, 0);                          // REPEAT
        WRDADDR(b07, wn->drp); WRD(wn->drp);               // DROP DROP          ( u )[ 1|-1 ]
        WRD(wn->rpo); WRD(wn->drp); WRD(wn->drp);          // R> DROP DROP       (  )
        WRD(wn->ext);
        LINK(b00, b02); LINK(b01, b03);
        LINK(b05, b07); LINK(b06, b04); }
}

#endif // KF_WORDS_STACK_MEM_H
