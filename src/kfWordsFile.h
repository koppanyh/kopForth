#ifndef KF_WORDS_FILE_H
#define KF_WORDS_FILE_H

/*
 * kfWordsFile.h (last modified 2025-10-07)
 * This contains the word definitions for the file access words.
 * This file has both Forth and Native words since file access is an extension.
 */

#include "kfBios.h"
#include "kfStack.h"
#include "kfStatus.h"
#include "kfType.h"
#include "kfWordsIntComp.h"
#include "kfWordsNative.h"
#include "kfWordsStackMem.h"
#include "kfWordsString.h"
#include "kfWordsVarAddrConst.h"



// Used for dynamically loading the dependency to modules that require it.
#define KF_FILE_EXT_DEP_FULL kfWordsFile* wf,
#define KF_FILE_EXT_DEP_SHORT &wf,
#define KF_FILE_EXT_INIT kfWordsFile wf; \
        kfPopulateWordsFile(forth, &wn, &wv, &wm, &ws, &wi, &wf);



// Pointers to words created in this file, for usage in defining other words.
typedef struct kfWordsFile kfWordsFile;
struct kfWordsFile {
    kfWord* opf;  // OPEN-FILE
    kfWord* clf;  // CLOSE-FILE
    kfWord* crf;  // CREATE-FILE
    kfWord* dlf;  // DELETE-FILE
    kfWord* rnf;  // RENAME-FILE
    kfWord* fst;  // FILE-STATUS
    kfWord* fpo;  // FILE-POSITION
    kfWord* rpf;  // REPOSITION-FILE
    kfWord* fsz;  // FILE-SIZE
    kfWord* flf;  // FLUSH-FILE
    kfWord* rdf;  // READ-FILE
    kfWord* wrf;  // WRITE-FILE
    kfWord* rdl;  // READ-LINE
    kfWord* wrl;  // WRITE-LINE
    kfWord* bin;  // BIN
    kfWord* wrr;  // W/R
    kfWord* reo;  // R/O
    kfWord* wro;  // W/O
    kfWord* rew;  // R/W
    kfWord* rfl;  // REFILL
};
/*
// RESIZE-FILE
// INCLUDE-FILE
// INCLUDE
// INCLUDED
// REQUIRE
// REQUIRED
S\"
S"
(
*/



// Native word implementations.

kfStatus W_Opf(kopForth* forth) {  // c-addr u fam -- fileid ior
    isize fam;
    usize len;
    char* addr;
    KF_DATA_POP(fam);
    KF_DATA_POP(len);
    KF_DATA_POP(addr);
    char term = addr[len];  // Save byte and turn into string terminator.
    addr[len] = '\0';
    const char* fam_str = kfBiosFileGetFam(fam);
    if (fam_str == NULL)
        return KF_INVALID_FAM;
    kfBiosFile* file = kfBiosFileOpen(addr, fam_str);
    addr[len] = term;  // Restore byte after using it.
    KF_DATA_PUSH(file);
    KF_DATA_PUSH(file ? 0 : -1);
    return KF_STATUS_OK;
}

kfStatus W_Clf(kopForth* forth) {  // fileid -- ior
    kfBiosFile* file;
    KF_DATA_POP(file);
    KF_DATA_PUSH(kfBiosFileClose(file));
    return KF_STATUS_OK;
}

kfStatus W_Crf(kopForth* forth) {  // c-addr u fam -- fileid ior
    isize fam;
    usize len;
    char* addr;
    KF_DATA_POP(fam);
    KF_DATA_POP(len);
    KF_DATA_POP(addr);
    char term = addr[len];  // Save byte and turn into string terminator.
    addr[len] = '\0';
    // Open in write mode to create or empty the file.
    kfBiosFile* file = kfBiosFileOpen(addr, kfBiosFileGetFam(KF_FAM_W));
    if (!file) {
        KF_DATA_PUSH(0);
        KF_DATA_PUSH(-2);
        return KF_STATUS_OK;
    }
    // Close the file we just created.
    if (kfBiosFileClose(file)) {
        KF_DATA_PUSH(0);
        KF_DATA_PUSH(-3);
        return KF_STATUS_OK;
    }
    // Reopen the file with the correct FAM.
    const char* fam_str = kfBiosFileGetFam(fam);
    if (fam_str == NULL)
        return KF_INVALID_FAM;
    file = kfBiosFileOpen(addr, fam_str);
    addr[len] = term;  // Restore byte after using it.
    KF_DATA_PUSH(file);
    KF_DATA_PUSH(file ? 0 : -1);
    return KF_STATUS_OK;
}

kfStatus W_Dlf(kopForth* forth) {  // c-addr u -- ior
    usize len;
    char* addr;
    KF_DATA_POP(len);
    KF_DATA_POP(addr);
    char term = addr[len];  // Save byte and turn into string terminator.
    addr[len] = '\0';
    KF_DATA_PUSH(kfBiosFileDelete(addr));
    addr[len] = term;  // Restore byte after using it.
    return KF_STATUS_OK;
}

kfStatus W_Rnf(kopForth* forth) {  // c-addr1 u1 c-addr2 u2 -- ior
    usize len2;
    char* addr2;
    usize len1;
    char* addr1;
    KF_DATA_POP(len2);
    KF_DATA_POP(addr2);
    KF_DATA_POP(len1);
    KF_DATA_POP(addr1);
    char term1 = addr1[len1];  // Save bytes and turn into string terminator.
    addr1[len1] = '\0';
    char term2 = addr2[len2];
    addr2[len2] = '\0';
    KF_DATA_PUSH(kfBiosFileRename(addr1, addr2));
    addr1[len1] = term1;  // Restore byte after using it.
    addr2[len2] = term2;
    return KF_STATUS_OK;
}

kfStatus W_Fst(kopForth* forth) {  // c-addr u -- x ior
    usize len;
    char* addr;
    KF_DATA_POP(len);
    KF_DATA_POP(addr);
    char term = addr[len];  // Save byte and turn into string terminator.
    addr[len] = '\0';
    isize status = kfBiosFileStatus(addr);
    addr[len] = term;  // Restore byte after using it.
    KF_DATA_PUSH(status ? 0 : 2);  // 2 is the magic number from gforth (?)
    KF_DATA_PUSH(status);
    return KF_STATUS_OK;
}

kfStatus W_Fpo(kopForth* forth) {  // fileid -- ud ior
    kfBiosFile* file;
    KF_DATA_POP(file);
    isize rez = kfBiosFilePosition(file);
    KF_DATA_PUSH(rez < 0 ? 0 : rez);
    KF_DATA_PUSH(0);  // TODO change if word size not big enough for file size.
    KF_DATA_PUSH(rez < 0 ? rez : 0);
    return KF_STATUS_OK;
}

kfStatus W_Rpf(kopForth* forth) {  // ud fileid -- ior
    kfBiosFile* file;
    usize upper;
    usize lower;
    KF_DATA_POP(file);
    KF_DATA_POP(upper);  // TODO use this if word size not big enough for pos.
    KF_DATA_POP(lower);
    KF_DATA_PUSH(kfBiosFileReposition(file, lower));
    return KF_STATUS_OK;
}

kfStatus W_Fsz(kopForth* forth) {  // fileid -- ud ior
    kfBiosFile* file;
    KF_DATA_POP(file);
    isize rez = kfBiosFileSize(file);
    KF_DATA_PUSH(rez < 0 ? 0 : rez);
    KF_DATA_PUSH(0);  // TODO change if word size not big enough for file size.
    KF_DATA_PUSH(rez < 0 ? rez : 0);
    return KF_STATUS_OK;
}

kfStatus W_Flf(kopForth* forth) {  // fileid -- ior
    kfBiosFile* file;
    KF_DATA_POP(file);
    KF_DATA_PUSH(kfBiosFileFlush(file));
    return KF_STATUS_OK;
}

kfStatus W_Rdf(kopForth* forth) {  // c-addr u1 fileid -- u2 ior
    kfBiosFile* file;
    usize ct;
    uint8_t* addr;
    KF_DATA_POP(file);
    KF_DATA_POP(ct);
    KF_DATA_POP(addr);
    KF_DATA_PUSH(kfBiosFileReadFile(file, addr, ct));
    KF_DATA_PUSH(kfBiosFileError(file));
    return KF_STATUS_OK;
}

kfStatus W_Wrf(kopForth* forth) {  // c-addr u fileid -- ior
    kfBiosFile* file;
    usize ct;
    uint8_t* addr;
    KF_DATA_POP(file);
    KF_DATA_POP(ct);
    KF_DATA_POP(addr);
    kfBiosFileWriteFile(file, addr, ct);
    KF_DATA_PUSH(kfBiosFileError(file));
    return KF_STATUS_OK;
}

kfStatus W_Rdl(kopForth* forth) {  // c-addr u1 fileid -- u2 flag ior
    kfBiosFile* file;
    usize ct1;
    uint8_t* addr;
    KF_DATA_POP(file);
    KF_DATA_POP(ct1);
    KF_DATA_POP(addr);
    isize pos = kfBiosFilePosition(file);
    if (pos < 0) {
        KF_DATA_PUSH(0);
        KF_DATA_PUSH(0);
        KF_DATA_PUSH(-1);
        return KF_STATUS_OK;
    }
    usize ct2 = kfBiosFileReadFile(file, addr, ct1);
    usize ct3 = 0;  // Number of characters before \n.
    isize flag = 0;  // True if read succeeded, false if nothing to read.
    if (ct2 > 0) {
        flag = -1;
        for (ct3 = 0; ct3 < ct2; ct3++) {
            pos += 1;
            if (addr[ct3] == KF_NL)
                break;
        }
        kfBiosFileReposition(file, pos);
    }
    KF_DATA_PUSH(ct3);
    KF_DATA_PUSH(flag);
    KF_DATA_PUSH(kfBiosFileError(file));
    return KF_STATUS_OK;
}

kfStatus W_Wrl(kopForth* forth) {  // c-addr u fileid -- ior
    kfBiosFile* file;
    usize ct;
    uint8_t* addr;
    KF_DATA_POP(file);
    KF_DATA_POP(ct);
    KF_DATA_POP(addr);
    char nl = addr[ct];  // Save byte and turn into newline.
    addr[ct] = KF_NL;
    kfBiosFileWriteFile(file, addr, ct + 1);
    addr[ct] = nl;  // Restore byte after using it.
    KF_DATA_PUSH(kfBiosFileError(file));
    return KF_STATUS_OK;
}



// Fill file access words into memory.
void kfPopulateWordsFile(kopForth* forth, kfWordsNative* wn,
                         kfWordsVarAddrConst* wv, kfWordsStackMem* wm,
                         kfWordsString* ws, kfWordsIntComp* wi,
                         kfWordsFile* wf) {
    // TODO Null check.

    // Native words
    wf->opf = kopForthAddNativeWord(forth, "OPEN-FILE",       W_Opf, false);
    wf->clf = kopForthAddNativeWord(forth, "CLOSE-FILE",      W_Clf, false);
    wf->crf = kopForthAddNativeWord(forth, "CREATE-FILE",     W_Crf, false);
    wf->dlf = kopForthAddNativeWord(forth, "DELETE-FILE",     W_Dlf, false);
    wf->rnf = kopForthAddNativeWord(forth, "RENAME-FILE",     W_Rnf, false);
    wf->fst = kopForthAddNativeWord(forth, "FILE-STATUS",     W_Fst, false);
    wf->fpo = kopForthAddNativeWord(forth, "FILE-POSITION",   W_Fpo, false);
    wf->rpf = kopForthAddNativeWord(forth, "REPOSITION-FILE", W_Rpf, false);
    wf->fsz = kopForthAddNativeWord(forth, "FILE-SIZE",       W_Fsz, false);
    wf->flf = kopForthAddNativeWord(forth, "FLUSH-FILE",      W_Flf, false);
    wf->rdf = kopForthAddNativeWord(forth, "READ-FILE",       W_Rdf, false);
    wf->wrf = kopForthAddNativeWord(forth, "WRITE-FILE",      W_Wrf, false);
    wf->rdl = kopForthAddNativeWord(forth, "READ-LINE",       W_Rdl, false);
    wf->wrl = kopForthAddNativeWord(forth, "WRITE-LINE",      W_Wrl, false);

    // Constant words

    wf->bin = kopForthAddWord(forth, "BIN");  // ( fam1 -- fam2 )
        LIT(KF_FAM_BIN); WRD(wm->orr);        // 4 OR
        WRD(wn->ext);
    wf->wrr = kopForthAddWord(forth, "W/R");  // ( -- fam )
        LIT(KF_FAM_WR); WRD(wf->bin);         // 4
        WRD(wn->ext);
    wf->reo = kopForthAddWord(forth, "R/O");  // ( -- fam )
        LIT(KF_FAM_R); WRD(wf->bin);          // 5
        WRD(wn->ext);
    wf->wro = kopForthAddWord(forth, "W/O");  // ( -- fam )
        LIT(KF_FAM_W); WRD(wf->bin);          // 6
        WRD(wn->ext);
    wf->rew = kopForthAddWord(forth, "R/W");  // ( -- fam )
        LIT(KF_FAM_RW); WRD(wf->bin);         // 7
        WRD(wn->ext);

    // Interpreter extension words

    wf->rfl = kopForthAddWord(forth, "REFILL"); {
        // TODO fix handling \r\n line endings
        WRD(wv->sid); LIT(0); WRD(wm->leq);                 // SOURCE-ID 0 <=           ( f )                \ Not from file
        LITADDR(b00, wn->zbr, 0);                           // IF                       (  )
        WRD(wi->rfl); WRD(wn->ext);                         //     REFILL EXIT          ( f )
                                                            // THEN                                          \ When the input source is a text file,
        WRDADDR(b01, wv->tib); WRD(wv->tav); WRD(wv->sid);  // TIB TIB-AVAIL SOURCE-ID  ( a u1 file )
        WRD(wf->rdl); WRD(wn->dup);                         // READ-LINE DUP            ( u2 flag ior ior )  \ attempt to read the next line from the text-input file.
        LITADDR(b02, wn->zbr, 0);                           // IF                       ( u2 flag ior )
        PRSTR("ERROR: REFILL IOR "); WRD(wn->dot);          //     ." ERROR: REFILL IOR " .
        WRD(wi->abt);                                       //     ABORT
                                                            // THEN
        WRDADDR(b03, wn->drp); LITADDR(b04, wn->zbr, 0);    // DROP IF                  ( u2 )               \ If successful, make the result the current input buffer,
        WRD(wv->htb); WRD(wn->exc);                         //     #TIB !               (  )
        LIT(0); WRD(wv->gin); WRD(wn->exc);                 //     0 >IN !              (  )                 \ set >IN to zero,
        WRD(wv->tru); WRD(wn->ext);                         //     TRUE EXIT            ( -1 )               \ and return true.
                                                            // THEN
        WRDADDR(b05, wn->drp); WRD(wv->fal);                // DROP FALSE               ( 0 )                \ Otherwise return false.
        WRD(wn->ext);
        *b00 = (isize) b01;
        *b02 = (isize) b03;
        *b04 = (isize) b05; }

    kopForthAddWord(forth, "TEST"); {
        WRD(wv->pad); LIT(80); WRD(wn->acc); WRD(ws->crr);       // PAD 80 ACCEPT CR  ( u )
        WRD(wv->pad); WRD(wn->swp); WRD(wf->reo);                // PAD SWAP R/O   ( a u fam )
        WRD(wf->opf);                                            // OPEN-FILE      ( fileid ior )
        LITADDR(b00, wn->zbr, 0);                                // IF             ( fileid )
        WRD(wi->abt);                                            //     ABORT
                                                                 // THEN
        WRDADDR(b01, wn->sip); WRD(wn->ntr);                     // SAVE-INPUT N>R ( fileid )
        WRD(wi->src); WRD(wm->add); WRD(wv->tpt); WRD(wn->exc);  // SOURCE + TP !         ( fileid )
        WRD(wv->srp); WRD(wn->exc);                              // SRCPT !        (  )
        //WRD(wv->tru); WRD(wv->dbg); WRD(wn->exc);
                                                                 // BEGIN
        WRDADDR(b02, wf->rfl);                                   //     REFILL
        LITADDR(b03, wn->zbr, 0);                                // WHILE
        WRD(wi->src); WRD(wi->evl);                              //     SOURCE EVALUATE
        LITADDR(b04, wn->bra, 0);                                // REPEAT
        WRDADDR(b05, wn->nrf); WRD(wn->rip);                     // NR> RESTORE-INPUT  ( flag )
        LITADDR(b06, wn->zbr, 0);                                // IF
        WRD(wi->abt);                                            //     ABORT
                                                                 // THEN
        WRDADDR(b07, wn->ext);
        *b00 = (isize) b01;
        *b03 = (isize) b05;
        *b04 = (isize) b02;
        *b06 = (isize) b07; }

/* File dev test words
    kfWord* dcr = kopForthAddWord(forth, ".CR"); {  // ( n -- )
        WRD(wn->dot); WRD(ws->crr);                 // . CR
        WRD(wn->ext); }
    kfWord* fis = kopForthAddWord(forth, "FSTAT"); {  // ( addr -- )
        WRD(ws->cnt); WRD(wf->fst);                   // COUNT FILE-STATUS  ( x ior )
        WRD(wn->swp); WRD(wn->dot); WRD(dcr);         // SWAP . .CR  ( )
        WRD(wn->ext); }
    kfWord* gen = kopForthAddWord(forth, "GETNAME"); {  // ( -- addr )
        WRD(wv->her); LIT(0); WRD(wm->cco);             // HERE 0 C,                 ( addr1 )
        LIT('>'); WRD(wn->emt); WRD(wv->her); LIT(80);  // CR [CHAR] > EMIT HERE 80  ( addr1 addr2 80 )
        WRD(wn->acc); WRD(wn->dup); WRD(wm->alt);       // ACCEPT DUP ALLOT          ( addr1 n )
        WRD(wm->ovr); WRD(wn->cex); WRD(ws->crr);       // OVER C! CR                ( addr1 )
        WRD(wn->ext); }
    kfWord* na1 = kopForthAddWord(forth, "NAME1"); {  // ( -- addr )
        RAWADDR(b00, 0);
        WRDADDR(b01, wn->lit); RAWADDR(b02, 0);       // <addr>
        WRD(wn->ext);
        na1->code.forth = b01;
        *b02 = (isize) b00; }
    kfWord* n1a = kopForthAddWord(forth, "NAME1@"); {  // ( -- addr )
        WRD(na1); WRD(wn->att);                        // NAME1 @
        WRD(wn->ext); }
    kfWord* na2 = kopForthAddWord(forth, "NAME2"); {  // ( -- addr )
        RAWADDR(b00, 0);
        WRDADDR(b01, wn->lit); RAWADDR(b02, 0);       // <addr>
        WRD(wn->ext);
        na2->code.forth = b01;
        *b02 = (isize) b00; }
    kfWord* n2a = kopForthAddWord(forth, "NAME2@"); {  // ( -- addr )
        WRD(na2); WRD(wn->att);                        // NAME1 @
        WRD(wn->ext); }
    kfWord* fi1 = kopForthAddWord(forth, "FILE1"); {  // ( -- addr )
        RAWADDR(b00, 0);
        WRDADDR(b01, wn->lit); RAWADDR(b02, 0);       // <addr>
        WRD(wn->ext);
        fi1->code.forth = b01;
        *b02 = (isize) b00; }
    kfWord* f1a = kopForthAddWord(forth, "FILE1@"); {  // ( -- addr )
        WRD(fi1); WRD(wn->att);                        // FILE1 @
        WRD(wn->ext); }
    kopForthAddWord(forth, "TEST"); {
        WRD(ws->crr); WRD(gen); WRD(na1); WRD(wn->exc);          // CR GETNAME NAME1 !             ( )                    \ >asdf
        WRD(gen); WRD(na2); WRD(wn->exc);                        // GETNAME NAME2 !                ( )                    \ >qwer
        WRD(n1a); WRD(fis);                                      // NAME1@ FSTAT                   ( )                    \ 0 -1
        WRD(n2a); WRD(fis);                                      // NAME2@ FSTAT                   ( )                    \ 0 -1
        WRD(n1a); WRD(ws->cnt); WRD(wf->rew); WRD(wf->crf);      // NAME1@ COUNT R/W CREATE-FILE   ( fileid ior )
        WRD(wm->ovr); WRD(wn->dot); WRD(dcr);                    // OVER . .CR                     ( fileid )             \ <addr> 0
        WRD(fi1); WRD(wn->exc);                                  // FILE1 !                        ( )
        WRD(f1a); WRD(wf->fsz); WRD(wn->swp);                    // FILE1@ FILE-SIZE SWAP          ( n1 ior n2 )
        WRD(wm->rot); WRD(wn->dot); WRD(wn->dot); WRD(dcr);      // ROT . . .CR                    ( )                    \ 0 0 0
        WRD(n1a); WRD(ws->cnt); WRD(f1a);                        // NAME1@ COUNT FILE1@            ( addr1 u1 fileid )
        WRD(wf->wrl); WRD(dcr);                                  // WRITE-LINE .CR                 ( )                    \ 0
        WRD(n1a); WRD(ws->cnt); WRD(f1a);                        // NAME1@ COUNT FILE1@            ( addr1 u1 fileid )
        WRD(wf->wrf); WRD(dcr);                                  // WRITE-FILE .CR                 ( )                    \ 0
        WRD(f1a); WRD(wf->flf); WRD(dcr);                        // FILE1@ FLUSH-FILE .CR          ( )                    \ 0
        WRD(f1a); WRD(wf->clf); WRD(dcr);                        // FILE1@ CLOSE-FILE .CR          ( )                    \ 0
        WRD(n1a); WRD(ws->cnt); WRD(n2a); WRD(ws->cnt);          // NAME1@ COUNT NAME2@ COUNT      ( addr1 u1 addr2 u2 )
        WRD(wf->rnf); WRD(dcr);                                  // RENAME-FILE .CR                ( )                    \ 0
        WRD(n1a); WRD(fis);                                      // NAME1@ FSTAT                   ( )                    \ 0 -1
        WRD(n2a); WRD(fis);                                      // NAME2@ FSTAT                   ( )                    \ 2 0
        WRD(n2a); WRD(ws->cnt); WRD(wf->reo); WRD(wf->opf);      // NAME2@ COUNT R/O OPEN-FILE     ( fileid ior )
        WRD(wm->ovr); WRD(wn->dot); WRD(dcr);                    // OVER . .CR                     ( fileid )             \ <addr> 0
        WRD(fi1); WRD(wn->exc);                                  // FILE1 !                        ( )
        WRD(f1a); WRD(wf->fsz); WRD(wn->swp);                    // FILE1@ FILE-SIZE SWAP          ( n1 ior n2 )
        WRD(wm->rot); WRD(wn->dot); WRD(wn->dot); WRD(dcr);      // ROT . . .CR                    ( )                    \ 9 0 0
        WRD(wv->pad); LIT(128); WRD(f1a); WRD(wf->rdf);          // PAD 128 FILE1@ READ-FILE       ( u1 ior )
        WRD(wm->ovr); WRD(wn->dot); WRD(dcr);                    // OVER . .CR                     ( u1 )                 \ 9 0
        WRD(wv->pad); WRD(wn->swp); WRD(wn->typ);                // PAD SWAP TYPE                  ( )                    \ asdf\nasdf
        WRD(wv->pad); LIT(16); WRD(wn->dmp);                     // PAD 16 DUMP                    ( )                    \ <addr> 61 73 64 66 0A 61 73 64 66 00 00 00 00 00 00 00
        LIT(2); LIT(0); WRD(f1a); WRD(wf->rpf); WRD(dcr);        // 2. FILE1@ REPOSITION-FILE .CR  ( )                    \ 0
        WRD(wv->pad); LIT(128); WRD(f1a); WRD(wf->rdl);          // PAD 128 FILE1@ READ-LINE       ( u2 flag ior )
        WRD(wm->rot); WRD(wn->dup); WRD(wn->dot);                // ROT DUP .                      ( flag ior u2 )        \ 2
        WRD(wm->rot); WRD(wn->dot); WRD(wn->swp); WRD(dcr);      // ROT . SWAP .CR                 ( u2 )                 \   -1 0
        WRD(wv->pad); WRD(wn->swp); WRD(wn->typ); WRD(ws->crr);  // PAD SWAP TYPE CR               ( )                    \ df
        WRD(f1a); WRD(wf->fpo); WRD(wn->swp);                    // FILE1@ FILE-POSITION SWAP      ( n1 ior n2 )
        WRD(wm->rot); WRD(wn->dot); WRD(wn->dot); WRD(dcr);      // ROT . . .CR                    ( )                    \ 5 0 0
        WRD(wv->pad); LIT(128); WRD(f1a); WRD(wf->rdl);          // PAD 128 FILE1@ READ-LINE       ( u2 flag ior )
        WRD(wm->rot); WRD(wn->dup); WRD(wn->dot);                // ROT DUP .                      ( flag ior u2 )        \ 4
        WRD(wm->rot); WRD(wn->dot); WRD(wn->swp); WRD(dcr);      // ROT . SWAP .CR                 ( u2 )                 \   -1 0
        WRD(wv->pad); WRD(wn->swp); WRD(wn->typ); WRD(ws->crr);  // PAD SWAP TYPE CR               ( )                    \ asdf
        WRD(wv->pad); LIT(128); WRD(f1a); WRD(wf->rdl);          // PAD 128 FILE1@ READ-LINE       ( u2 flag ior )
        WRD(wm->rot); WRD(wn->dot);                              // ROT .                          ( flag ior )           \ 0
        WRD(wn->swp); WRD(wn->dot); WRD(dcr);                    // SWAP . .CR                     ( )                    \   0 0
        WRD(f1a); WRD(wf->clf); WRD(dcr);                        // FILE1@ CLOSE-FILE .CR          ( )                    \ 0
        WRD(n2a); WRD(ws->cnt); WRD(wf->dlf); WRD(dcr);          // NAME2@ COUNT DELETE-FILE .CR   ( )                    \ 0
        WRD(n2a); WRD(fis);                                      // NAME2@ FSTAT                   ( )                    \ 0 -1
        WRD(wn->dos);                                            // .S                             ( )                    \  ok
        WRD(wn->ext); }  // */
}

#endif // KF_WORDS_FILE_H
