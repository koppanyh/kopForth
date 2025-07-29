#ifndef KF_WORDS_INT_COMP_H
#define KF_WORDS_INT_COMP_H

/*
 * kfWordsIntComp.h (last modified 2025-07-29)
 * This contains the word definitions for the shell interpreter and compiler.
 */

#include "kfType.h"
#include "kfWordsNative.h"
#include "kfWordsStackMem.h"
#include "kfWordsString.h"
#include "kfWordsVarAddrConst.h"



// Pointers to words created in this file, for usage in defining other words.
typedef struct kfWordsIntComp kfWordsIntComp;
struct kfWordsIntComp {
    kfWord* src;  // SOURCE
    kfWord* rfl;  // REFILL
    kfWord* exe;  // EXECUTE
    kfWord* cpl;  // COMPILE,
    kfWord* rev;  // REVEAL
    kfWord* obr;  // [
    kfWord* cbr;  // ]
    kfWord* abt;  // ABORT
    kfWord* enf;  // (ERR-NOT-FOUND)
    kfWord* lnk;  // >LINK
    kfWord* fgs;  // >FLAGS
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
    kfWord* inp;  // INTERPRET
    kfWord* qut;  // QUIT
    //kfWord* evl;  // EVALUATE
    //kfWord* pst;  // POSTPONE
};



// Fill interpreter/compiler words into memory.
void kfPopulateWordsIntComp(kopForth* forth, kfWordsNative* wn,
                           kfWordsVarAddrConst* wv, kfWordsStackMem* wm,
                           kfWordsString* ws, kfWordsIntComp* wi) {
    // TODO Null check.

    wi->src = kopForthAddWord(forth, "SOURCE");           // ( -- a u )
        WRD(wv->tib); WRD(wv->htb); WRD(wn->att);         // TIB #TIB @
        WRD(wn->ext);
    wi->rfl = kopForthAddWord(forth, "REFILL");           // ( -- f )
        WRD(wv->tib); LIT(80); WRD(wn->acc);              // TIB 80 ACCEPT
        WRD(wv->htb); WRD(wn->exc);                       // #TIB !
        LIT(0); WRD(wv->gin); WRD(wn->exc);               // 0 >IN !
        WRD(ws->spa); WRD(wv->tru);                       // SPACE TRUE
        WRD(wn->ext);
    wi->exe = kopForthAddWord(forth, "EXECUTE");          // ( xt -- )
        LITADDR(c4, wn->lit, 0);                          // <addr> ! <xt>
        WRD(wn->exc);
        WRDADDR(c5, (kfWord*) 0);
        WRD(wn->ext);
        *c4 = (isize) c5;
    wi->cpl = kopForthAddWord(forth, "COMPILE,");         // ( xt -- )
        WRD(wm->com);                                     // ,
        WRD(wn->ext);
    wi->rev = kopForthAddWord(forth, "REVEAL");           // ( -- )
        WRD(wv->ppt); WRD(wn->att);                       // PP @
        WRD(wv->lpt); WRD(wn->exc);                       // LP !
        WRD(wn->ext);
    wi->obr = kopForthAddWord(forth, "[");                // ( -- )
        WRD(wv->fal); WRD(wv->sta); WRD(wn->exc);         // FALSE STATE !
        WRD(wn->ext);
        wi->obr->flags.bit_flags.is_immediate = 1;
    wi->cbr = kopForthAddWord(forth, "]");                // ( -- )
        WRD(wv->tru); WRD(wv->sta); WRD(wn->exc);         // TRUE STATE !
        WRD(wn->ext);
    wi->abt = kopForthAddWord(forth, "ABORT");            // ( * -- )
        WRD(wv->lat); WRD(wv->ppt); WRD(wn->att);         // LATEST PP @
        WRD(wn->lss); LITADDR(abt01, wn->zbr, 0);         // < IF
        WRD(wv->ppt); WRD(wn->att);                       //     PP @ DP !
        WRD(wv->dpt); WRD(wn->exc);
        WRD(wv->lat); WRD(wv->ppt); WRD(wn->exc);         //     LATEST PP !
                                                          // THEN
        WRDADDR(abt02, wn->crs);                          // (CLR-RET-STACK)
        WRD(wn->cds);                                     // (CLR-DAT-STACK)
        WRDADDR(abt00, wn->ext);                          // QUIT
        *abt01 = (isize) abt02;
    wi->enf = kopForthAddWord(forth, "(ERR-NOT-FOUND)");  // ( a -- )
        WRD(ws->crr);                                     // CR
        PRSTR("ERROR: '");                                // ." ERROR: "
        WRD(ws->cnt); WRD(wn->typ);                       // COUNT TYPE
        PRSTR("' word not found");                        // ."  word not found"
        WRD(ws->crr); WRD(wi->abt);                       // CR ABORT
        WRD(wn->ext);
    wi->lnk = kopForthAddWord(forth, ">LINK");            // ( xt -- a )
        LIT(KF_WORD_LINK_OFFSET); WRD(wm->add);           // 17 +
        WRD(wn->ext);
    wi->fgs = kopForthAddWord(forth, ">FLAGS");           // ( xt -- a )
        LIT(KF_WORD_FLAGS_OFFSET); WRD(wm->add);          // 25 +
        WRD(wn->ext);
    wi->cod = kopForthAddWord(forth, ">CODE");            // ( xt -- a )
        LIT(KF_WORD_CODE_OFFSET); WRD(wm->add);           // 26 +
        WRD(wn->ext);
    wi->bod = kopForthAddWord(forth, ">BODY");            // ( xt -- a )
        LIT(KF_WORD_DATA_OFFSET); WRD(wm->add);           // 34 +
        WRD(wn->ext);
    wi->urv = kopForthAddWord(forth, "UNREVEAL");         // ( -- )
        WRD(wv->lat); WRD(wi->lnk); WRD(wn->att);         // LATEST >LINK @
        WRD(wv->lpt); WRD(wn->exc);                       // LP !
        WRD(wn->ext);
    wi->dod = kopForthAddWord(forth, "DODOES");           // ( -- a )
        WRD(wn->rpo); WRD(wn->rpo); WRD(wn->dup);         // R> R> DUP
        WRD(wn->rpu); WRD(wn->swp); WRD(wn->rpu);         // >R SWAP >R
        LIT(sizeof(isize)); WRD(wn->sub);                 // [ 1 CELLS ] LITERAL -
        WRD(wn->att); WRD(wi->bod);                       // @ >BODY
        WRD(wn->ext);
    wi->pdo = kopForthAddWord(forth, "(DOES>)");          // ( -- )
        WRD(wn->rpo); WRD(wv->ppt); WRD(wn->att);         // R> PP @
        WRD(wi->cod); WRD(wn->exc);                       // >CODE !
        WRD(wn->ext);
    wi->doe = kopForthAddWord(forth, "DOES>");            // ( -- )
        LIT(wi->pdo); WRD(wi->cpl);                       // ['] (DOES>) COMPILE,
        LIT(wi->dod); WRD(wi->cpl);                       // ['] DODOES COMPILE,
        WRD(wn->ext);
        wi->doe->flags.bit_flags.is_immediate = 1;
    wi->cre = kopForthAddWord(forth, "CREATE");           // ( -- )
        WRD(ws->bla); WRD(wn->wrd);                       // BL WORD
        WRD(wn->drp); WRD(wn->pcr);                       // DROP (CREATE)
        WRD(wi->pdo); WRD(wi->dod);                       // DOES>
        WRD(wn->ext);
    wi->col = kopForthAddWord(forth, ":");                // ( -- )
        WRD(wi->cre); WRD(wi->urv); WRD(wi->cbr);         // CREATE UNREVEAL POSTPONE ]
        WRD(wv->her); WRD(wv->ppt); WRD(wn->att);         // HERE PP @
        WRD(wi->cod); WRD(wn->exc);                       // >CODE !
        WRD(wn->ext);
    wi->sem = kopForthAddWord(forth, ";");                // ( -- )
        LIT(wn->ext); WRD(wi->cpl);                       // ['] EXIT COMPILE,
        WRD(wi->rev); WRD(wi->obr);                       // REVEAL POSTPONE [
        WRD(wn->ext);
        wi->sem->flags.bit_flags.is_immediate = 1;
    wi->imm = kopForthAddWord(forth, "IMMEDIATE");        // ( -- )
        WRD(wv->ppt); WRD(wn->att); WRD(wi->fgs);         // PP @ >FLAGS
        WRD(wn->dup); WRD(wn->cat);                       // DUP C@
        LIT(KF_FLAG_MASK_IMMEDIATE); WRD(wm->orr);        // 2 OR
        WRD(wn->swp); WRD(wn->cex);                       // SWAP C!
        WRD(wn->ext);
        wi->imm->flags.bit_flags.is_immediate = 1;

    wi->inp = kopForthAddWord(forth, "INTERPRET"); {             // ( -- )
        LIT(0); WRD(wv->gin); WRD(wn->exc);                      // 0 >IN !                                   (  )
                                                                 // BEGIN                                     (  )
        WRDADDR(b00, ws->bla); WRD(wn->wrd);                     //     BL WORD                               ( c-addr )
        WRD(wn->dup); WRD(ws->cnt); WRD(wn->swp); WRD(wn->drp);  //     DUP COUNT SWAP DROP                   ( c-addr u )
        LITADDR(b01, wn->zbr, 0);                                // WHILE                                     ( c-addr )
        WRD(wn->fnd);                                            //     FIND                                  ( c-addr 0 | xt 1 | xt -1 )
        WRD(wv->sta); WRD(wn->att); LITADDR(b02, wn->zbr, 0);    //     STATE @ IF      \ Compiling           ( c-addr 0 | xt 1 | xt -1 )
        WRD(wn->dup); LITADDR(b03, wn->zbr, 0);                  //         DUP IF      \ Word                ( xt 1 | xt -1 )
        LIT(1); WRD(wn->equ); LITADDR(b04, wn->zbr, 0);          //             1 = IF  \ Immediate           ( xt )
        WRD(wi->exe);                                            //                 EXECUTE                   ( ? )
        LITADDR(b05, wn->bra, 0);                                //             ELSE                          ( xt )
        WRDADDR(b06, wi->cpl);                                   //                 COMPILE,                  (  )
                                                                 //             THEN                          ( ? )
        WRDADDR(b07, wn->bra); RAWADDR(b08, 0);                  //         ELSE        \ Unknown             ( c-addr 0 )
        WRDADDR(b09, wn->drp); WRD(wn->dup); WRD(ws->cnt);       //             DROP DUP COUNT                ( c-addr c-addr2 u )
        WRD(ws->snu);                                            //             S>NUMBER?                     ( c-addr n 0 0 | c-addr d -1 0 | c-addr c-addr3 u2 )
        LITADDR(b10, wn->zbr, 0);                                //             IF      \ Error               ( c-addr c-addr3 )
        WRD(wn->drp); WRD(wi->enf);                              //                 DROP (ERR-NOT-FOUND)      (  )
        LITADDR(b11, wn->bra, 0);                                //             ELSE    \ Number              ( c-addr n 0 | c-addr d -1 )
        WRDADDR(b12, wn->zbr); RAWADDR(b13, 0);                  //                 IF  \ Double              ( c-addr d )
        WRD(wn->swp);                                            //                     SWAP                  ( c-addr n n )
        LIT(wn->lit); WRD(wi->cpl); WRD(wm->com);                //                     ['] (LIT) COMPILE, ,  ( c-addr n )
        WRDADDR(b14, wn->lit); RAW(wn->lit);                     //                 THEN ['] (LIT)            ( c-addr n xt )
        WRD(wi->cpl); WRD(wm->com);                              //                 COMPILE, ,                ( c-addr )
        WRD(wn->drp);                                            //                 DROP                      (  )
                                                                 //             THEN                          (  )
                                                                 //         THEN                              ( ? )
        WRDADDR(b15, wn->bra); RAWADDR(b16, 0);                  //     ELSE            \ Interpreting        ( c-addr 0 | xt 1 | xt -1 )
        WRDADDR(b17, wn->zbr); RAWADDR(b18, 0);                  //         IF          \ Word                ( xt )
                                                                 //             // TODO check if not compile only
        WRD(wi->exe);                                            //             EXECUTE                       ( ? )
        LITADDR(b19, wn->bra, 0);                                //         ELSE        \ Unknown             ( c-addr )
        WRDADDR(b20, wn->dup); WRD(ws->cnt); WRD(ws->snu);       //             DUP COUNT S>NUMBER?           ( c-addr n 0 0 | c-addr d -1 0 | c-addr c-addr2 u )
        LITADDR(b21, wn->zbr, 0);                                //             IF      \ Error               ( c-addr c-addr2 )
        WRD(wn->drp); WRD(wi->enf);                              //                 DROP (ERR-NOT-FOUND)      (  )
                                                                 //             THEN    \ Number              ( c-addr n 0 | c-addr d -1 )
        WRDADDR(b22, wn->zbr); RAWADDR(b23, 0);                  //             IF      \ Double              ( c-addr d )
        WRD(wm->rot);                                            //                 ROT                       ( d c-addr )
        LITADDR(b24, wn->bra, 0);                                //             ELSE    \ Single              ( c-addr n )
        WRDADDR(b25, wn->swp);                                   //                 SWAP                      ( n c-addr )
                                                                 //             THEN                          ( d c-addr | n c-addr )
        WRDADDR(b26, wn->drp);                                   //             DROP                          ( n | d )
                                                                 //         THEN                              ( ? | n | d )
                                                                 //     THEN                                  ( ? | n | d )
        WRDADDR(b27, wn->bra); RAWADDR(b28, 0);                  // REPEAT                                    ( c-addr )
        WRDADDR(b29, wn->drp);                                   // DROP                                      (  )
        WRD(wn->ext);
        *b28 = (isize) b00;
        *b01 = (isize) b29;
        *b02 = (isize) b17;
        *b03 = (isize) b09;
        *b04 = (isize) b06;
        *b05 = (isize) b07;
        *b08 = (isize) b15;
        *b10 = (isize) b12;
        *b11 = (isize) b15;
        *b13 = (isize) b14;
        *b16 = (isize) b27;
        *b18 = (isize) b20;
        *b19 = (isize) b27;
        *b21 = (isize) b22;
        *b23 = (isize) b25;
        *b24 = (isize) b26; }
    wi->qut = kopForthAddWord(forth, "QUIT"); {                  // ( -- )
        WRD(wn->crs);                                            // (CLR-RET-STACK)
        WRD(wi->obr);                                            // POSTPONE [
                                                                 // BEGIN
        WRDADDR(b00, wi->rfl);                                   //     REFILL  ( f )
        LITADDR(b01, wn->zbr, 0);                                // WHILE
        WRD(wi->inp);                                            //     INTERPRET
        PRSTR(" ok");                                            //     ."  ok"
        WRD(ws->crr);                                            //     CR
        LITADDR(b02, wn->bra, 0);                                // REPEAT
        WRDADDR(b03, wn->ext);
        *b02 = (isize) b00;
        *b01 = (isize) b03;
        *abt00 = wi->qut; }

    //wi->evl = kopForthAddWord(forth, "EVALUATE");  // ( -- )
    //wi->pst = kopForthAddWord(forth, "POSTPONE");  // ( -- )
}

#endif // KF_WORDS_INT_COMP_H
