#ifndef KF_WORDS_COMPILE_H
#define KF_WORDS_COMPILE_H

/*
 * kfWordsCompile.h (last modified 2025-10-13)
 * This contains the word definitions for the compiler.
 */

#include "kfType.h"
#include "kfWordsInterpret.h"
#include "kfWordsNative.h"
#include "kfWordsStackMem.h"
#include "kfWordsString.h"
#include "kfWordsVarAddrConst.h"



// Pointers to words created in this file, for usage in defining other words.
typedef struct kfWordsCompile kfWordsCompile;
struct kfWordsCompile {
    kfWord* rev;  // REVEAL
    kfWord* lnk;  // >LINK
    kfWord* cod;  // >CODE
    kfWord* bod;  // >BODY
    kfWord* urv;  // UNREVEAL
    kfWord* dod;  // DODOES
    kfWord* pdo;  // (DOES>)
    kfWord* doe;  // DOES>
    kfWord* cre;  // CREATE
    kfWord* col;  // :
    kfWord* sem;  // ;
    kfWord* imm;  // IMMEDIATE
    kfWord* con;  // COMPILE-ONLY
    //kfWord* pst;  // POSTPONE
    kfWord* iff;  // IF
    kfWord* els;  // ELSE
    kfWord* thn;  // THEN
    kfWord* iii;  // I
    kfWord* jjj;  // J
    kfWord* dop;  // (DO)
    kfWord* doo;  // DO
    kfWord* lev;  // LEAVE
    kfWord* ppl;  // (+LOOP)
    kfWord* plp;  // +LOOP
    kfWord* lop;  // LOOP
    kfWord* beg;  // BEGIN
    kfWord* agn;  // AGAIN
    kfWord* unt;  // UNTIL
    kfWord* whl;  // WHILE
    kfWord* rpt;  // REPEAT
};



// Fill interpreter/compiler words into memory.
void kfPopulateWordsCompile(kopForth* forth, kfWordsNative* wn,
                            kfWordsVarAddrConst* wv, kfWordsStackMem* wm,
                            kfWordsString* ws, kfWordsInterpret* wi,
                            kfWordsCompile* wc) {
    // TODO Null check.

    /////////////////////
    // Compiling words //
    /////////////////////

    wc->rev = kopForthAddWord(forth, "REVEAL");  // ( -- )
        WRD(wv->ppt); WRD(wn->att);              // PP @
        WRD(wv->lpt); WRD(wn->exc);              // LP !
        WRD(wn->ext);

    wc->lnk = kopForthAddWord(forth, ">LINK");    // ( xt -- a )
        LIT(KF_WORD_LINK_OFFSET); WRD(wm->add);   // 17 +
        WRD(wn->ext);

    wc->cod = kopForthAddWord(forth, ">CODE");    // ( xt -- a )
        LIT(KF_WORD_CODE_OFFSET); WRD(wm->add);   // 26 +
        WRD(wn->ext);

    wc->bod = kopForthAddWord(forth, ">BODY");    // ( xt -- a )
        LIT(KF_WORD_DATA_OFFSET); WRD(wm->add);   // 34 +
        WRD(wn->ext);

    wc->urv = kopForthAddWord(forth, "UNREVEAL");  // ( -- )
        WRD(wv->lat); WRD(wc->lnk); WRD(wn->att);  // LATEST >LINK @
        WRD(wv->lpt); WRD(wn->exc);                // LP !
        WRD(wn->ext);

    wc->dod = kopForthAddWord(forth, "DODOES");    // ( -- a )
        WRD(wn->rpo); WRD(wn->rpo); WRD(wn->dup);  // R> R> DUP
        WRD(wn->rpu); WRD(wn->swp); WRD(wn->rpu);  // >R SWAP >R
        LIT(sizeof(isize)); WRD(wn->sub);          // [ 1 CELLS ] LITERAL -
        WRD(wn->att); WRD(wc->bod);                // @ >BODY
        WRD(wn->ext);

    wc->pdo = kopForthAddWord(forth, "(DOES>)");   // ( -- )
        WRD(wn->rpo); WRD(wv->ppt); WRD(wn->att);  // R> PP @
        WRD(wc->cod); WRD(wn->exc);                // >CODE !
        WRD(wn->ext);

    wc->doe = kopForthAddWord(forth, "DOES>");  // ( -- )
        LIT(wc->pdo); WRD(wm->cpl);             // ['] (DOES>) COMPILE,
        LIT(wc->dod); WRD(wm->cpl);             // ['] DODOES COMPILE,
        WRD(wn->ext);
        wc->doe->flags.bit_flags.is_immediate = 1;
        wc->doe->flags.bit_flags.compile_only = 1;

    wc->cre = kopForthAddWord(forth, "CREATE");  // ( -- )
        WRD(ws->bla); WRD(wi->wrd);              // BL WORD
        WRD(wn->drp); WRD(wn->pcr);              // DROP (CREATE)
        WRD(wc->pdo); WRD(wc->dod);              // DOES>
        WRD(wn->ext);

    wc->col = kopForthAddWord(forth, ":");         // ( -- )
        WRD(wc->cre); WRD(wc->urv); WRD(wi->cbr);  // CREATE UNREVEAL POSTPONE ]
        WRD(wv->her); WRD(wv->ppt); WRD(wn->att);  // HERE PP @
        WRD(wc->cod); WRD(wn->exc);                // >CODE !
        WRD(wn->ext);

    wc->sem = kopForthAddWord(forth, ";");  // ( -- )
        LIT(wn->ext); WRD(wm->cpl);         // ['] EXIT COMPILE,
        WRD(wc->rev); WRD(wi->obr);         // REVEAL POSTPONE [
        WRD(wn->ext);
        IMMEDIATE(wc->sem); COMPONLY(wc->sem);

    wc->imm = kopForthAddWord(forth, "IMMEDIATE");  // ( -- )
        WRD(wv->ppt); WRD(wn->att); WRD(wi->fgs);   // PP @ >FLAGS  ( addr )
        WRD(wn->dup); WRD(wn->cat);                 // DUP C@       ( addr n )
        LIT(KF_FLAG_MASK_IMMEDIATE); WRD(wm->orr);  // 2 OR         ( addr n2 )
        WRD(wn->swp); WRD(wn->cex);                 // SWAP C!      (  )
        WRD(wn->ext);
        IMMEDIATE(wc->imm);

    wc->con = kopForthAddWord(forth, "COMPILE-ONLY");  // ( -- )
        WRD(wv->ppt); WRD(wn->att); WRD(wi->fgs);      // PP @ >FLAGS  ( addr )
        WRD(wn->dup); WRD(wn->cat);                    // DUP C@       ( addr n )
        LIT(KF_FLAG_MASK_COMPILE); WRD(wm->orr);       // 4 OR         ( addr n2 )
        WRD(wn->swp); WRD(wn->cex);                    // SWAP C!      (  )
        WRD(wn->ext);
        IMMEDIATE(wc->imm);

    //wi->pst = kopForthAddWord(forth, "POSTPONE");  // ( -- )

    ///////////////////////
    // Conditional words //
    ///////////////////////

    wc->iff = kopForthAddWord(forth, "IF");  // ( f -- ) (C: -- orig )
        LIT(wn->zbr); WRD(wm->cpl);          // ['] 0BRANCH COMPILE,  (C:  )
        WRD(wv->her); LIT(0); WRD(wm->com);  // HERE 0 ,              (C: orig )
        WRD(wn->ext);
        IMMEDIATE(wc->iff); COMPONLY(wc->iff);

    wc->thn = kopForthAddWord(forth, "THEN");      // ( -- ) (C: orig -- )
        WRD(wv->her); WRD(wn->swp); WRD(wn->exc);  // HERE SWAP !
        WRD(wn->ext);
        IMMEDIATE(wc->thn); COMPONLY(wc->thn);

    wc->els = kopForthAddWord(forth, "ELSE");  // ( -- ) (C: orig1 -- orig2 )
        LIT(wn->bra); WRD(wm->cpl);            // ['] BRANCH COMPILE,  (C: orig1 )
        WRD(wv->her); WRD(wn->swp);            // HERE SWAP            (C: orig2 orig1 )
        LIT(0); WRD(wm->com); WRD(wc->thn);    // 0 , POSTPONE THEN    (C: orig2 orig1 )
        WRD(wn->ext);
        IMMEDIATE(wc->els); COMPONLY(wc->els);

    ///////////////////////
    // Finite loop words //
    ///////////////////////

    wc->iii = kopForthAddWord(forth, "I");  // ( -- n | u )[ loop-sys -- loop-sys ]
        WRD(wn->rpo); WRD(wm->rat);         // R> R@    ( ret n2 )[ laddr n1 n2 ]
        WRD(wn->swp); WRD(wn->rpu);         // SWAP >R  ( n2 )[ laddr n1 n2 ret ]
        WRD(wn->ext);

    wc->jjj = kopForthAddWord(forth, "J");         // ( -- n | u )[ loop-sys1 loop-sys2 -- loop-sys1 loop-sys2 ]
        WRD(wn->rpo); WRD(wn->rpo);  // R> R>    ( ret n4 )[ laddr1 n1 n2 laddr2 n3 ]
        WRD(wn->rpo); WRD(wn->rpo);  // R> R>    ( ret n4 n3 laddr2 )[ laddr1 n1 n2 ]
        WRD(wm->rat);                // R@       ( ret n4 n3 laddr2 n2 )[ laddr1 n1 n2 ]
        WRD(wn->swp); WRD(wn->rpu);  // SWAP >R  ( ret n4 n3 n2 )[ laddr1 n1 n2 laddr2 ]
        WRD(wn->swp); WRD(wn->rpu);  // SWAP >R  ( ret n4 n2 )[ laddr1 n1 n2 laddr2 n3 ]
        WRD(wn->swp); WRD(wn->rpu);  // SWAP >R  ( ret n2 )[ laddr1 n1 n2 laddr2 n3 n4 ]
        WRD(wn->swp); WRD(wn->rpu);  // SWAP >R  ( n2 )[ laddr1 n1 n2 laddr2 n3 n4 ret ]
        WRD(wn->ext);

    wc->dop = kopForthAddWord(forth, "(DO)");      // ( n1 | u1 n2 | u2 -- )[ -- loop-sys ]
        WRD(wn->swp); WRD(wn->rpo);                // SWAP R>   ( n2 n1 ret )[  ]
        WRD(wn->dup); WRD(wn->att); WRD(wn->rpu);  // DUP @ >R  ( n2 n1 ret )[ laddr ]
        WRD(wn->swp); WRD(wn->rpu);                // SWAP >R   ( n2 ret )[ laddr n1 ]
        WRD(wn->swp); WRD(wn->rpu);                // SWAP >R   ( ret )[ laddr n1 n2 ]
        LIT(1); WRD(wm->cls);                      // 1 CELLS   ( ret 8 )[ laddr n1 n2 ]
        WRD(wm->add); WRD(wn->rpu);                // + >R      (  )[ laddr n1 n2 ret2 ]
        WRD(wn->ext);
        COMPONLY(wc->dop);

    wc->doo = kopForthAddWord(forth, "DO");  // ( -- )(C: -- do-sys)
        LIT(wc->dop); WRD(wm->cpl);          // ['] (DO) COMPILE,  (C:  )
        WRD(wv->her); LIT(0); WRD(wm->com);  // HERE 0 ,           (C: ltar )
        WRD(wv->her);                        // HERE               (C: ltar lbod )
        WRD(wn->ext);
        IMMEDIATE(wc->doo); COMPONLY(wc->doo);

    wc->lev = kopForthAddWord(forth, "LEAVE");  // ( -- )[ loop-sys -- ]
        WRD(wn->rpo); WRD(wn->drp);             // R> DROP (  )[ laddr n1 n2 ]
        WRD(wn->rpo); WRD(wn->drp);             // R> DROP (  )[ laddr n1 ]
        WRD(wn->rpo); WRD(wn->drp);             // R> DROP (  )[ laddr ]
        WRD(wn->ext);

    wc->ppl = kopForthAddWord(forth, "(+LOOP)"); {          // ( n -- )[ loop-sys1 -- | loop-sys2 ]
        WRD(wn->rpo); WRD(wn->att);                         // R> @                ( n jaddr )[ laddr n1 n2 ]
        WRD(wn->swp); WRD(wn->rpo);                         // SWAP R>             ( jaddr n n2 )[ laddr n1 ]
        WRD(wm->ovr); WRD(wm->add);                         // OVER +              ( jaddr n n3 )[ laddr n1 ]
        WRD(wn->rpo); WRD(wm->rot);                         // R> ROT              ( jaddr n3 n1 n )[ laddr ]
        LIT(0); WRD(wn->lss); LITADDR(b00, wn->zbr, 0);     // 0 < IF              ( jaddr n3 n1 )[ laddr ]
        WRD(wm->tdu); WRD(wn->lss);                         //     2DUP <          ( jaddr n3 n1 f )[ laddr ]
        LITADDR(b01, wn->bra, 0);                           // ELSE
        WRDADDR(b02, wm->tdu); WRD(wm->geq);                //     2DUP >=         ( jaddr n3 n1 f )[ laddr ]
                                                            // THEN
        WRDADDR(b03, wn->zbr); RAWADDR(b04, 0);             // IF                  ( jaddr n3 n1 )[ laddr ]  \ Loop finished
        WRD(wn->drp); WRD(wn->drp); WRD(wn->drp);           //     DROP DROP DROP  (  )[ laddr ]
        WRD(wn->ext);                                       //     EXIT
                                                            // THEN
        WRDADDR(b05, wn->rpu); WRD(wn->rpu); WRD(wn->rpu);  // >R >R >R            (  )[ laddr n1 n3 jaddr ]
        WRD(wn->ext);
        LINK(b00, b02); LINK(b01, b03); LINK(b04, b05);
        COMPONLY(wc->ppl); }

    wc->plp = kopForthAddWord(forth, "+LOOP");     // ( -- )(C: do-sys -- )
        LIT(wc->ppl); WRD(wm->cpl); WRD(wm->cpl);  // ['] (+LOOP) COMPILE, COMPILE,  (C: ltar )
        WRD(wv->her); WRD(wn->swp); WRD(wn->exc);  // HERE SWAP !                    (C:  )
        WRD(wn->ext);
        IMMEDIATE(wc->plp); COMPONLY(wc->plp);

    wc->lop = kopForthAddWord(forth, "LOOP");  // ( -- )(C: do-sys -- )
        LIT(wn->lit); WRD(wm->cpl);            // ['] (LITERAL) COMPILE,  (C: ltar lbod )
        LIT(1); WRD(wm->com);                  // 1 ,                     (C: ltar lbod )
        WRD(wc->plp);                          // POSTPONE +LOOP          (C:  )
        WRD(wn->ext);
        IMMEDIATE(wc->lop); COMPONLY(wc->lop);

    ///////////////////////////
    // Indefinite loop words //
    ///////////////////////////

    wc->beg = kopForthAddWord(forth, "BEGIN");  // ( -- )(C: -- dest )
        WRD(wv->her);                           // HERE  (C: jaddr )
        WRD(wn->ext);
        IMMEDIATE(wc->beg); COMPONLY(wc->beg);

    wc->agn = kopForthAddWord(forth, "AGAIN");     // ( -- )(C: dest -- )
        LIT(wn->bra); WRD(wm->cpl); WRD(wm->com);  // ['] BRANCH COMPILE, ,  (C:  )
        WRD(wn->ext);
        IMMEDIATE(wc->agn); COMPONLY(wc->agn);

    wc->unt = kopForthAddWord(forth, "UNTIL");     // ( x -- )(C: dest -- )
        LIT(wn->zbr); WRD(wm->cpl); WRD(wm->com);  // ['] 0BRANCH COMPILE, ,  (C:  )
        WRD(wn->ext);
        IMMEDIATE(wc->unt); COMPONLY(wc->unt);

    wc->whl = kopForthAddWord(forth, "WHILE");  // ( x -- )(C: dest -- orig dest )
        LIT(wn->zbr); WRD(wm->cpl);             // ['] 0BRANCH COMPILE,  // (C: dest )
        WRD(wv->her); WRD(wn->swp);             // HERE SWAP             // (C: orig dest )
        LIT(0); WRD(wm->com);                   // 0 ,                   // (C: orig dest )
        WRD(wn->ext);
        IMMEDIATE(wc->whl); COMPONLY(wc->whl);

    wc->rpt = kopForthAddWord(forth, "REPEAT");    // ( -- )(C: orig dest -- )
        LIT(wn->bra); WRD(wm->cpl); WRD(wm->com);  // ['] BRANCH COMPILE, ,  (C: orig )
        WRD(wv->her); WRD(wn->swp); WRD(wn->exc);  // HERE SWAP !            (C:  )
        WRD(wn->ext);
        IMMEDIATE(wc->rpt); COMPONLY(wc->rpt);
}

#endif // KF_WORDS_COMPILE_H
