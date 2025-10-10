#ifndef KF_WORDS_INT_COMP_H
#define KF_WORDS_INT_COMP_H

/*
 * kfWordsIntComp.h (last modified 2025-10-10)
 * This contains the word definitions for the shell interpreter and compiler.
 */

#include "kfType.h"
#ifdef KF_FILE_EXT
    #include "kfWordsFile.h"
#endif
#include "kfWordsNative.h"
#include "kfWordsStackMem.h"
#include "kfWordsString.h"
#include "kfWordsVarAddrConst.h"



// Pointers to words created in this file, for usage in defining other words.
typedef struct kfWordsIntComp kfWordsIntComp;
struct kfWordsIntComp {
    kfWord* abt;  // ABORT
    kfWord* src;  // SOURCE
    kfWord* rfl;  // REFILL
    kfWord* skc;  // SKIPCHAR
    kfWord* isc;  // ISSKIPCHAR
    kfWord* ins;  // ISNOTSKIPCHAR
    kfWord* par;  // PARSE
    kfWord* prn;  // PARSE-NAME
    kfWord* wrd;  // WORD
    kfWord* cpl;  // COMPILE,
    kfWord* rev;  // REVEAL
    kfWord* obr;  // [
    kfWord* cbr;  // ]
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
    kfWord* evl;  // EVALUATE
    kfWord* qut;  // QUIT
    //kfWord* pst;  // POSTPONE
};



// Fill interpreter/compiler words into memory.
void kfPopulateWordsIntComp(kopForth* forth, kfWordsNative* wn,
                            kfWordsVarAddrConst* wv, kfWordsStackMem* wm,
                            kfWordsString* ws, kfWordsIntComp* wi) {
    // TODO Null check.

    wi->abt = kopForthAddWord(forth, "ABORT");     // ( * -- )
        kfWord** abt00; {
        WRD(wv->lat); WRD(wv->ppt); WRD(wn->att);  // LATEST PP @  \ Reset PENDING and HERE pointers
        WRD(wn->lss); LITADDR(b01, wn->zbr, 0);    // < IF
        WRD(wv->ppt); WRD(wn->att);                //     PP @ DP !
        WRD(wv->dpt); WRD(wn->exc);
        WRD(wv->lat); WRD(wv->ppt); WRD(wn->exc);  //     LATEST PP !
                                                   // THEN
        WRDADDR(b02, wn->cds);                     // (CLR-DAT-STACK)
        PRSTR(" ok"); WRD(ws->crr);                // ."  ok" CR
        WRDADDR(b00, wn->ext);                     // QUIT
        LINK(b01, b02);
        abt00 = b00; }

    wi->src = kopForthAddWord(forth, "SOURCE");    // ( -- a u )
        WRD(wv->tib); WRD(wv->htb); WRD(wn->att);  // TIB #TIB @
        WRD(wn->ext);

    wi->rfl = kopForthAddWord(forth, "REFILL"); {     // ( -- f )
        WRD(wv->sid); WRD(wm->zeq);                   // SOURCE-ID 0=              ( f )  \ From terminal
        LITADDR(b00, wn->zbr, 0);                     // IF                        (  )
        WRD(wv->tib); WRD(wv->tav); WRD(wn->acc);     //     TIB TIB-AVAIL ACCEPT  ( u )
        WRD(wv->htb); WRD(wn->exc);                   //     #TIB !                (  )
        LIT(0); WRD(wv->gin); WRD(wn->exc);           //     0 >IN !               (  )
        WRD(ws->spa); WRD(wv->tru); WRD(wn->ext);     //     SPACE TRUE EXIT       ( -1 )
                                                      // THEN
        WRDADDR(b01, wv->sid);                        // SOURCE-ID
        PRSTR("ERROR: SOURCE-ID of "); WRD(wn->dot);  // ." ERROR: SOURCE-ID of " .
        PRSTR("is invalid."); WRD(wi->abt);           // ." is invalid." ABORT
        WRD(wn->ext);
        LINK(b00, b01); }

    wi->skc = kopForthAddWord(forth, "SKIPCHAR"); {  // ( -- addr )
        LITADDR(b00, wn->lit, 0);
        WRD(wn->ext);
        RAWADDR(b01, 0);
        LINK(b00, b01); }

    wi->isc = kopForthAddWord(forth, "ISSKIPCHAR"); {          // ( c -- f )
        WRD(wi->skc); WRD(wn->att); WRD(wn->dup);              // SKIPCHAR @ DUP  ( c ch ch )
        WRD(ws->bla); WRD(wn->equ); LITADDR(b00, wn->zbr, 0);  // BL = IF         ( c ch )
        WRD(wn->drp); WRD(ws->bla); WRD(wm->leq);              //     DROP BL <=  ( f )
        LITADDR(b01, wn->bra, 0);                              // ELSE            ( c ch )
        WRDADDR(b02, wn->equ);                                 //     =           ( f )
                                                               // THEN
        WRDADDR(b03, wn->ext);
        LINK(b00, b02); LINK(b01, b03); }

    wi->ins = kopForthAddWord(forth, "ISNOTSKIPCHAR");  // (c -- f )
        WRD(wi->isc); WRD(wm->zeq);                     // ISSKIPCHAR 0=
        WRD(wn->ext);

    wi->par = kopForthAddWord(forth, "PARSE");                   // ( char "ccc<char>" -- c-addr u )
        WRD(wi->skc); WRD(wn->exc);                              // SKIPCHAR !                    (  )
        WRD(wi->src); WRD(wv->gin); WRD(wn->att); WRD(ws->sst);  // SOURCE >IN @ /STRING          ( addr u )
        WRD(wm->ovr); WRD(wn->swp);                              // OVER SWAP                     ( addr addr u )
        LIT(wi->ins); WRD(ws->xsk); WRD(wn->drp);                // ' ISNOTSKIPCHAR XT-SKIP DROP  ( addr addr2 )
        WRD(wn->dup); LIT(1); WRD(wm->add);                      // DUP 1 +                       ( addr addr2 addr3 )
        WRD(wv->tib); WRD(wn->sub); WRD(wv->gin); WRD(wn->exc);  // TIB - >IN !                   ( addr addr2 )
        WRD(wm->ovr); WRD(wn->sub);                              // OVER -                        ( addr u2 )
        WRD(wn->ext);

    wi->prn = kopForthAddWord(forth, "PARSE-NAME");              // ( "<spaces>name<space>" -- c-addr u )
        WRD(wi->src); WRD(wv->gin); WRD(wn->att); WRD(ws->sst);  // SOURCE >IN @ /STRING          ( addr u )
        WRD(ws->bla); WRD(wi->skc); WRD(wn->exc);                // BL SKIPCHAR !                 ( addr u )
        LIT(wi->isc); WRD(ws->xsk);                              // ' ISSKIPCHAR XT-SKIP          ( addr2 u2 )
        WRD(wm->ovr); WRD(wn->swp);                              // OVER SWAP                     ( addr2 addr2 u2 )
        LIT(wi->ins); WRD(ws->xsk); WRD(wn->drp);                // ' ISNOTSKIPCHAR XT-SKIP DROP  ( addr2 addr3 )
        WRD(wm->tdu); WRD(wn->swp); WRD(wn->sub);                // 2DUP SWAP -                   ( addr2 addr3 u3 )
        WRD(wn->swp); LIT(1); WRD(wm->add);                      // SWAP 1 +                      ( addr2 u3 addr4 )
        WRD(wv->tib); WRD(wn->sub); WRD(wv->gin); WRD(wn->exc);  // TIB - >IN !                   ( addr2 u3 )
        WRD(wn->ext);

    wi->wrd = kopForthAddWord(forth, "WORD");      // ( char -- addr )
        // TODO update this to actually use the input char.
        WRD(wn->drp); WRD(wi->prn);                // DROP PARSE-NAME  ( addr u )
        WRD(wn->dup); WRD(wv->her); WRD(wn->cex);  // DUP HERE C!      ( addr u )
        WRD(wv->her); LIT(1); WRD(wm->add);        // HERE 1 +         ( addr u addr2 )
        WRD(wn->swp); WRD(wm->mov); WRD(wv->her);  // SWAP MOVE HERE   ( addr )
        WRD(wn->ext);

    wi->cpl = kopForthAddWord(forth, "COMPILE,");  // ( xt -- )
        WRD(wm->com);                              // ,
        WRD(wn->ext);

    wi->rev = kopForthAddWord(forth, "REVEAL");  // ( -- )
        WRD(wv->ppt); WRD(wn->att);              // PP @
        WRD(wv->lpt); WRD(wn->exc);              // LP !
        WRD(wn->ext);

    wi->obr = kopForthAddWord(forth, "[");         // ( -- )
        WRD(wv->fal); WRD(wv->sta); WRD(wn->exc);  // FALSE STATE !
        WRD(wn->ext);
        wi->obr->flags.bit_flags.is_immediate = 1;

    wi->cbr = kopForthAddWord(forth, "]");         // ( -- )
        WRD(wv->tru); WRD(wv->sta); WRD(wn->exc);  // TRUE STATE !
        WRD(wn->ext);

    wi->enf = kopForthAddWord(forth, "(ERR-NOT-FOUND)");  // ( a -- )
        WRD(ws->crr);                                     // CR
        PRSTR("ERROR: `");                                // ." ERROR: `"
        WRD(ws->cnt); WRD(wn->typ);                       // COUNT TYPE
        PRSTR("` word not found");                        // ." ` word not found"
        WRD(ws->crr); WRD(wi->abt);                       // CR ABORT
        WRD(wn->ext);

    wi->lnk = kopForthAddWord(forth, ">LINK");    // ( xt -- a )
        LIT(KF_WORD_LINK_OFFSET); WRD(wm->add);   // 17 +
        WRD(wn->ext);
    wi->fgs = kopForthAddWord(forth, ">FLAGS");   // ( xt -- a )
        LIT(KF_WORD_FLAGS_OFFSET); WRD(wm->add);  // 25 +
        WRD(wn->ext);
    wi->cod = kopForthAddWord(forth, ">CODE");    // ( xt -- a )
        LIT(KF_WORD_CODE_OFFSET); WRD(wm->add);   // 26 +
        WRD(wn->ext);
    wi->bod = kopForthAddWord(forth, ">BODY");    // ( xt -- a )
        LIT(KF_WORD_DATA_OFFSET); WRD(wm->add);   // 34 +
        WRD(wn->ext);

    wi->urv = kopForthAddWord(forth, "UNREVEAL");  // ( -- )
        WRD(wv->lat); WRD(wi->lnk); WRD(wn->att);  // LATEST >LINK @
        WRD(wv->lpt); WRD(wn->exc);                // LP !
        WRD(wn->ext);

    wi->dod = kopForthAddWord(forth, "DODOES");    // ( -- a )
        WRD(wn->rpo); WRD(wn->rpo); WRD(wn->dup);  // R> R> DUP
        WRD(wn->rpu); WRD(wn->swp); WRD(wn->rpu);  // >R SWAP >R
        LIT(sizeof(isize)); WRD(wn->sub);          // [ 1 CELLS ] LITERAL -
        WRD(wn->att); WRD(wi->bod);                // @ >BODY
        WRD(wn->ext);

    wi->pdo = kopForthAddWord(forth, "(DOES>)");   // ( -- )
        WRD(wn->rpo); WRD(wv->ppt); WRD(wn->att);  // R> PP @
        WRD(wi->cod); WRD(wn->exc);                // >CODE !
        WRD(wn->ext);

    wi->doe = kopForthAddWord(forth, "DOES>");  // ( -- )
        LIT(wi->pdo); WRD(wi->cpl);             // ['] (DOES>) COMPILE,
        LIT(wi->dod); WRD(wi->cpl);             // ['] DODOES COMPILE,
        WRD(wn->ext);
        wi->doe->flags.bit_flags.is_immediate = 1;

    wi->cre = kopForthAddWord(forth, "CREATE");  // ( -- )
        WRD(ws->bla); WRD(wi->wrd);              // BL WORD
        WRD(wn->drp); WRD(wn->pcr);              // DROP (CREATE)
        WRD(wi->pdo); WRD(wi->dod);              // DOES>
        WRD(wn->ext);

    wi->col = kopForthAddWord(forth, ":");         // ( -- )
        WRD(wi->cre); WRD(wi->urv); WRD(wi->cbr);  // CREATE UNREVEAL POSTPONE ]
        WRD(wv->her); WRD(wv->ppt); WRD(wn->att);  // HERE PP @
        WRD(wi->cod); WRD(wn->exc);                // >CODE !
        WRD(wn->ext);

    wi->sem = kopForthAddWord(forth, ";");  // ( -- )
        LIT(wn->ext); WRD(wi->cpl);         // ['] EXIT COMPILE,
        WRD(wi->rev); WRD(wi->obr);         // REVEAL POSTPONE [
        WRD(wn->ext);
        wi->sem->flags.bit_flags.is_immediate = 1;

    wi->imm = kopForthAddWord(forth, "IMMEDIATE");  // ( -- )
        WRD(wv->ppt); WRD(wn->att); WRD(wi->fgs);   // PP @ >FLAGS
        WRD(wn->dup); WRD(wn->cat);                 // DUP C@
        LIT(KF_FLAG_MASK_IMMEDIATE); WRD(wm->orr);  // 2 OR
        WRD(wn->swp); WRD(wn->cex);                 // SWAP C!
        WRD(wn->ext);
        wi->imm->flags.bit_flags.is_immediate = 1;

    wi->inp = kopForthAddWord(forth, "INTERPRET"); {             // ( -- )
                                                                 // BEGIN                                     (  )
        WRDADDR(b00, ws->bla); WRD(wi->wrd);                     //     BL WORD                               ( c-addr )
        WRD(wn->dup); WRD(ws->cnt); WRD(wn->swp); WRD(wn->drp);  //     DUP COUNT SWAP DROP                   ( c-addr u )
        LITADDR(b01, wn->zbr, 0);                                // WHILE                                     ( c-addr )
        WRD(wn->fnd);                                            //     FIND                                  ( c-addr 0 | xt 1 | xt -1 )
        WRD(wv->sta); WRD(wn->att); LITADDR(b02, wn->zbr, 0);    //     STATE @ IF      \ Compiling           ( c-addr 0 | xt 1 | xt -1 )
        WRD(wn->dup); LITADDR(b03, wn->zbr, 0);                  //         DUP IF      \ Word                ( xt 1 | xt -1 )
        LIT(1); WRD(wn->equ); LITADDR(b04, wn->zbr, 0);          //             1 = IF  \ Immediate           ( xt )
        WRD(wm->exe);                                            //                 EXECUTE                   ( ? )
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
        WRD(wm->exe);                                            //             EXECUTE                       ( ? )
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
        LINK(b01, b29); LINK(b02, b17); LINK(b03, b09);
        LINK(b04, b06); LINK(b05, b07); LINK(b08, b15);
        LINK(b10, b12); LINK(b11, b15); LINK(b13, b14);
        LINK(b16, b27); LINK(b18, b20); LINK(b19, b27);
        LINK(b21, b22); LINK(b23, b25); LINK(b24, b26);
        LINK(b28, b00); }

    wi->evl = kopForthAddWord(forth, "EVALUATE"); {              // ( c-addr u -- )
        WRD(wn->sip); WRD(wn->ntr);                              // SAVE-INPUT N>R     \ Save the current input source specification.
        LIT(-1); WRD(wv->srp); WRD(wn->exc);                     // -1 SRCPT !         \ Store minus-one (-1) in SOURCE-ID if it is present.
        WRD(wv->htb); WRD(wn->exc); WRD(wv->tpt); WRD(wn->exc);  // #TIB ! TB !        \ Make the string described by c-addr and u both the input source and input buffer
        LIT(0); WRD(wv->gin); WRD(wn->exc);                      // 0 >IN !            \ set >IN to zero
        WRD(wi->inp);                                            // INTERPRET          \ and interpret.
        WRD(wn->nrf); WRD(wn->rip);                              // NR> RESTORE-INPUT  \ When the parse area is empty, restore the prior input source specification
        LITADDR(b00, wn->zbr, 0);                                // IF
        PRSTR("ERROR: EVALUATE RESTORE-INPUT Failed.");          //     ." ERROR: EVALUATE RESTORE-INPUT Failed."
        WRD(wi->abt);                                            //     ABORT
                                                                 // THEN
        WRDADDR(b01, wn->ext);
        LINK(b00, b01); }

    wi->qut = kopForthAddWord(forth, "QUIT"); {  // ( -- )
        WRD(wn->crs);                            // (CLR-RET-STACK)
        WRD(wn->cis);                            // (CLR-IN-SOURCE)
        WRD(wi->obr);                            // POSTPONE [
                                                 // BEGIN
        WRDADDR(b00, wi->rfl);                   //     REFILL  ( f )
        LITADDR(b01, wn->zbr, 0);                // WHILE
        WRD(wi->inp);                            //     INTERPRET
        PRSTR(" ok"); WRD(ws->crr);              //     ."  ok" CR
        LITADDR(b02, wn->bra, 0);                // REPEAT
        WRDADDR(b03, wn->ext);
        LINK(b02, b00); LINK(b01, b03);
        *abt00 = wi->qut; }

    //wi->pst = kopForthAddWord(forth, "POSTPONE");  // ( -- )
}

#endif // KF_WORDS_INT_COMP_H
