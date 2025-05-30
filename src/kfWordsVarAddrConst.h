#ifndef KF_WORDS_VAR_ADDR_CONST_H
#define KF_WORDS_VAR_ADDR_CONST_H

/*
 * kfWordsVarAddrConst.h (last modified 2025-05-28)
 * This contains the word definitions for variables, addresses, and constants.
 */

#include "kfType.h"
#include "kfWordsNative.h"



// Pointers to words created in this file, for usage in defining other words.
typedef struct kfWordsVarAddrConst kfWordsVarAddrConst;
struct kfWordsVarAddrConst {
    kfWord* tib;
    kfWord* htb;
    kfWord* gin;
    kfWord* dpt;
    kfWord* lpt;
    kfWord* ppt;
    kfWord* sta;
    kfWord* her;
    kfWord* lat;
    kfWord* pad;
    kfWord* tru;
    kfWord* fal;
};



// Fill variable/address/constant words into memory.
void kfPopulateWordsVarAddrConst(kopForth* forth, kfWordsNative* wn,
                                 kfWordsVarAddrConst* wv) {
    // TODO Null check.

    // Variables
    wv->tib = kopForthAddVariable(forth, "TIB",   (isize*)  forth->tib);        // -- a
    wv->htb = kopForthAddVariable(forth, "#TIB",  (isize*) &forth->tib_len);    // -- a
    wv->gin = kopForthAddVariable(forth, ">IN",   (isize*) &forth->in_offset);  // -- a
    wv->dpt = kopForthAddVariable(forth, "DP",    (isize*) &forth->here);       // -- a
    wv->lpt = kopForthAddVariable(forth, "LP",    (isize*) &forth->latest);     // -- a
    wv->ppt = kopForthAddVariable(forth, "PP",    (isize*) &forth->pending);    // -- a
    wv->sta = kopForthAddVariable(forth, "STATE", (isize*) &forth->state);      // -- a

    // Addresses
    wv->her = kopForthAddWord(forth, "HERE");    // ( -- a )s
        WRD(wv->dpt); WRD(wn->att);              // DP @
        WRD(wn->ext);
    wv->lat = kopForthAddWord(forth, "LATEST");  // ( -- a )
        WRD(wv->lpt); WRD(wn->att);              // LP @
        WRD(wn->ext);
    wv->pad = kopForthAddWord(forth, "PAD");     // ( -- a )
        WRD(wv->her); LIT(-256); WRD(wn->sub);   // HERE 256 +
        WRD(wn->ext);

    // Constants
    wv->tru = kopForthAddWord(forth, "TRUE");   // ( -- -1 )
        LIT(-1);
        WRD(wn->ext);
    wv->fal = kopForthAddWord(forth, "FALSE");  // ( -- 0 )
        LIT(0);
        WRD(wn->ext);
}


#endif // KF_WORDS_VAR_ADDR_CONST_H
