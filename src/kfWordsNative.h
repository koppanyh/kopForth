#ifndef KF_WORDS_NATIVE_H
#define KF_WORDS_NATIVE_H

/*
 * kfWordsNative.h (last modified 2025-09-03)
 * This contains the native word definitions for the kopForth system.
 */

#include "kfBios.h"
#include "kfMath.h"
#include "kfStack.h"
#include "kfStatus.h"
#include "kfType.h"



// Pointers to words created in this file, for usage in defining other words.
typedef struct kfWordsNative kfWordsNative;
struct kfWordsNative {
    kfWord* ext;  // EXIT
    kfWord* lit;  // (LITERAL)
    kfWord* sub;  // -
    kfWord* mul;  // *
    kfWord* dot;  // .
    kfWord* att;  // @
    kfWord* exc;  // !
    kfWord* cat;  // C@
    kfWord* cex;  // C!
    kfWord* rpu;  // >R
    kfWord* rpo;  // R>
    kfWord* drp;  // DROP
    kfWord* dup;  // DUP
    kfWord* swp;  // SWAP
    kfWord* bra;  // BRANCH
    kfWord* zbr;  // 0BRANCH
    kfWord* emt;  // EMIT
    kfWord* key;  // KEY
    kfWord* acc;  // ACCEPT
    kfWord* wrd;  // WORD
    kfWord* typ;  // TYPE
    kfWord* pcr;  // (CREATE)
    kfWord* cmp;  // COMPARE
    kfWord* fnd;  // FIND
    kfWord* mss;  // M*/
    kfWord* dpl;  // D+
    kfWord* equ;  // =
    kfWord* lss;  // <
    kfWord* nan;  // NAND
    kfWord* psq;  // (S")
    kfWord* squ;  // S"
    kfWord* dqu;  // ."
    kfWord* bye;  // BYE
    kfWord* dos;  // .S
    kfWord* dor;  // .R
    kfWord* dmp;  // DUMP
    kfWord* ntr;  // N>R
    kfWord* nrf;  // NR>
    kfWord* sip;  // SAVE-INPUT
    kfWord* rip;  // RESTORE-INPUT
    kfWord* cis;  // (CLR-IN-SOURCE)
    kfWord* crs;  // (CLR-RET-STACK)
    kfWord* cds;  // (CLR-DAT-STACK)
};



// Native word implementations.

kfStatus W_Ext(kopForth* forth) {  // --
    void* a;
    KF_RETN_POP(a);
    return KF_STATUS_OK;
}

kfStatus W_Lit(kopForth* forth) {  // -- n
    isize* lit_val;
    KF_RETN_POP(lit_val);
    KF_DATA_PUSH(*lit_val);
    lit_val++;
    KF_RETN_PUSH(lit_val);
    return KF_STATUS_OK;
}

kfStatus W_Sub(kopForth* forth) {  // n1 n2 -- n3
    isize a, b;
    KF_DATA_POP(b);
    KF_DATA_POP(a);
    KF_DATA_PUSH(a - b);
    return KF_STATUS_OK;
}

kfStatus W_Mul(kopForth* forth) {  // n1 n2 -- n3
    isize a, b;
    KF_DATA_POP(b);
    KF_DATA_POP(a);
    KF_DATA_PUSH(a * b);
    return KF_STATUS_OK;
}

kfStatus W_Dot(kopForth* forth) {  // n --
    isize a;
    KF_DATA_POP(a);
    kfBiosPrintIsize(a);
    kfBiosWriteChar(' ');
    return KF_STATUS_OK;
}

kfStatus W_Att(kopForth* forth) {  // addr -- n
    isize* a;
    KF_DATA_POP(a);
    KF_DATA_PUSH(*a);
    return KF_STATUS_OK;
}

kfStatus W_Exc(kopForth* forth) {  // n addr --
    isize a;
    isize* b;
    KF_DATA_POP(b);
    KF_DATA_POP(a);
    *b = a;
    return KF_STATUS_OK;
}

kfStatus W_Cat(kopForth* forth) {  // addr -- n
    uint8_t* a;
    KF_DATA_POP(a);
    KF_DATA_PUSH(*a);
    return KF_STATUS_OK;
}

kfStatus W_Cex(kopForth* forth) {  // n addr --
    isize a;
    uint8_t* b;
    KF_DATA_POP(b);
    KF_DATA_POP(a);
    *b = a;
    return KF_STATUS_OK;
}

kfStatus W_Rpu(kopForth* forth) {  // n --
    isize a;
    void* b;
    KF_DATA_POP(a);
    KF_RETN_POP(b);
    KF_RETN_PUSH(a);
    KF_RETN_PUSH(b);
    return KF_STATUS_OK;
}

kfStatus W_Rpo(kopForth* forth) {  // -- n
    isize a;
    void* b;
    KF_RETN_POP(b);
    KF_RETN_POP(a);
    KF_DATA_PUSH(a);
    KF_RETN_PUSH(b);
    return KF_STATUS_OK;
}

kfStatus W_Drp(kopForth* forth) {  // n --
    isize a;
    KF_DATA_POP(a);
    return KF_STATUS_OK;
}

kfStatus W_Dup(kopForth* forth) {  // n -- n n
    isize a;
    KF_DATA_POP(a);
    KF_DATA_PUSH(a);
    KF_DATA_PUSH(a);
    return KF_STATUS_OK;
}

kfStatus W_Swp(kopForth* forth) {  // n1 n2 -- n2 n1
    isize a;
    isize b;
    KF_DATA_POP(b);
    KF_DATA_POP(a);
    KF_DATA_PUSH(b);
    KF_DATA_PUSH(a);
    return KF_STATUS_OK;
}

kfStatus W_Bra(kopForth* forth) {  // --
    void* a;
    KF_RETN_POP(a);
    a = *(void**) a;
    KF_RETN_PUSH(a);
    return KF_STATUS_OK;
}

kfStatus W_Zbr(kopForth* forth) {  // n --
    void* a;
    isize b;
    KF_RETN_POP(a);
    KF_DATA_POP(b);
    if (b == 0) {
        a = *(void**) a;
    } else {
        a += sizeof(kfWord*);
    }
    KF_RETN_PUSH(a);
    return KF_STATUS_OK;
}

kfStatus W_Emt(kopForth* forth) {  // n --
    isize a;
    KF_DATA_POP(a);
    kfBiosWriteChar(a);
    return KF_STATUS_OK;
}

kfStatus W_Key(kopForth* forth) {  // -- n
    KF_DATA_PUSH(kfBiosReadChar());
    return KF_STATUS_OK;
}

kfStatus W_Acc(kopForth* forth) {  // addr u1 -- u2
    uint8_t* addr;
    isize u1, u2 = 0;
    KF_DATA_POP(u1);
    KF_DATA_POP(addr);
    while (true) {
        isize c = kfBiosReadChar();
        if (c == KF_NL)
            break;
        if (c == '\b') {
            if (u2 > 0)
                u2--;
            else
                continue;
            kfBiosWriteChar('\b');
            kfBiosWriteChar(' ');
            kfBiosWriteChar('\b');
            continue;
        }
        if (u2 >= u1)
            continue;
        kfBiosWriteChar(c);
        addr[u2] = c;
        u2++;
    }
    KF_DATA_PUSH(u2);
    return KF_STATUS_OK;
}

kfStatus W_Wrd(kopForth* forth) {  // char -- addr
    isize c;
    KF_DATA_POP(c);
    // write 0 (len) to HERE
    *forth->here = 0;
    // put HERE on the stack
    KF_DATA_PUSH(forth->here);
    if (forth->in_src.in_offset >= forth->in_src.in_len)
        return KF_STATUS_OK;
    // skip leading `char` in input stream
    while (forth->in_src.buf[forth->in_src.in_offset] == c) {
        forth->in_src.in_offset++;
        if (forth->in_src.in_offset >= forth->in_src.in_len)
            return KF_STATUS_OK;
    }
    // start copying !char characters to HERE+1
    uint8_t* h = forth->here + 1;
    usize ct = 0;
    while (forth->in_src.buf[forth->in_src.in_offset] != c) {
        *h = forth->in_src.buf[forth->in_src.in_offset];
        h++;
        ct++;
        forth->in_src.in_offset++;
        if (forth->in_src.in_offset >= forth->in_src.in_len)
            break;
    }
    // update the value at HERE (1 byte)
    *forth->here = ct;
    // update the >IN to show what's been consumed
    return KF_STATUS_OK;
}

kfStatus W_Typ(kopForth* forth) {  // addr u --
    char* addr;
    usize u;
    KF_DATA_POP(u);
    KF_DATA_POP(addr);
    kfBiosWriteStrLen(addr, u);
    return KF_STATUS_OK;
}

kfStatus W_Pcr(kopForth* forth) {  // --
    if (kopForthCreateWord(forth) == NULL) {
        kfBiosWriteStr("CREATE FAILED");
        return KF_SYSTEM_NULL;
    }
    return KF_STATUS_OK;
}

kfStatus W_Cmp(kopForth* forth) {  // a1 u1 a2 u2 -- n
    usize u1, u2;
    uint8_t* a1;
    uint8_t* a2;
    KF_DATA_POP(u2);
    KF_DATA_POP(a2);
    KF_DATA_POP(u1);
    KF_DATA_POP(a1);
    usize m = u1 > u2 ? u1 : u2;
    for (usize i = 0; i < m; i++) {
        if (a1[i] < a2[i]) {
            KF_DATA_PUSH(-1);
            return KF_STATUS_OK;
        }
        if (a1[i] > a2[i]) {
            KF_DATA_PUSH(1);
            return KF_STATUS_OK;
        }
    }
    if (u1 < m)
        KF_DATA_PUSH(-1);
    else if (u2 < m)
        KF_DATA_PUSH(1);
    else
        KF_DATA_PUSH(0);
    return KF_STATUS_OK;
}

kfStatus W_Fnd(kopForth* forth) {  // c-addr -- c-addr 0 | xt 1 | xt -1
    uint8_t* f_str;
    KF_DATA_POP(f_str);
    uint8_t str_ct = *f_str;
    uint8_t* str_head = f_str + 1;
    kfWord* word = (kfWord*) forth->latest;
    while (word != NULL && str_ct != 0) {
        if (word->name_len == str_ct) {
            bool match = true;
            for (uint8_t i = 0; i < str_ct; i++) {
                uint8_t c1 = str_head[i];
                uint8_t c2 = word->name[i];
                if (c1 >= 'A' && c1 <= 'Z') {
                    c1 += 32;
                }
                if (c2 >= 'A' && c2 <= 'Z') {
                    c2 += 32;
                }
                if (c1 != c2) {
                    match = false;
                    break;
                }
            }
            if (match) {
                KF_DATA_PUSH(word);
                KF_DATA_PUSH(word->flags.bit_flags.is_immediate ? 1 : -1);
                return KF_STATUS_OK;
            }
        }
        word = word->link;
    }
    KF_DATA_PUSH(f_str);
    KF_DATA_PUSH(0);
    return KF_STATUS_OK;
}

kfStatus W_Mss(kopForth* forth) {  // d1 n1 +n2 -- d2
    TwoCell doub, mult;
    isize div;
    KF_DATA_POP(div);
    if (div != 1) {
        kfBiosWriteStr("M*/ +n2 != 1");
        return KF_SYSTEM_NOT_IMP;
    }
    KF_DATA_POP(mult.low);
    mult.high = mult.low < 0 ? -1 : 0;
    KF_DATA_POP(doub.high);
    KF_DATA_POP(doub.low);
    doub = ByteCellToTwoCell(ByteCellsMultiply(TwoCellToByteCell(doub), TwoCellToByteCell(mult)));
    KF_DATA_PUSH(doub.low);
    KF_DATA_PUSH(doub.high);
    return KF_STATUS_OK;
}

kfStatus W_Dpl(kopForth* forth) {  // d1 d2 -- d3
    TwoCell d1, d2;
    KF_DATA_POP(d2.high);
    KF_DATA_POP(d2.low);
    KF_DATA_POP(d1.high);
    KF_DATA_POP(d1.low);
    d1 = ByteCellToTwoCell(ByteCellsAdd(TwoCellToByteCell(d1), TwoCellToByteCell(d2)));
    KF_DATA_PUSH(d1.low);
    KF_DATA_PUSH(d1.high);
    return KF_STATUS_OK;
}

kfStatus W_Equ(kopForth* forth) {  // n1 n2 -- n
    isize a, b;
    KF_DATA_POP(b);
    KF_DATA_POP(a);
    KF_DATA_PUSH(a == b ? -1 : 0);
    return KF_STATUS_OK;
}

kfStatus W_Lss(kopForth* forth) {  // n1 n2 -- n
    isize a, b;
    KF_DATA_POP(b);
    KF_DATA_POP(a);
    KF_DATA_PUSH(a < b ? -1 : 0);
    return KF_STATUS_OK;
}

kfStatus W_Nan(kopForth* forth) {  // n1 n2 -- n
    isize a, b;
    KF_DATA_POP(b);
    KF_DATA_POP(a);
    KF_DATA_PUSH(~(a & b));
    return KF_STATUS_OK;
}

kfStatus W_Psq(kopForth* forth) {  // -- addr u
    uint8_t* a;
    KF_RETN_POP(a);
    uint8_t len = *a;
    a++;
    char* chars = (char*) a;
    a += len;
    KF_RETN_PUSH(a);
    KF_DATA_PUSH(chars);
    KF_DATA_PUSH(len);
    return KF_STATUS_OK;
}

kfStatus W_Squ(kopForth* forth) {  // -- addr u
    // s" <len: uint8_t> <chars: char[len]>
    if (!forth->state) {  // Run time
        kfBiosWriteStr("S\" : not comp");
        return KF_SYSTEM_COMP_ONLY;
    }
    // Compile time
    // TODO implement this
    kfBiosWriteStr("S\" : not imp");
    return KF_SYSTEM_NOT_IMP;
    //return KF_STATUS_OK;
}

kfStatus W_Dqu(kopForth* forth) {  // --
    if (forth->state) { // Compile time
        // Is basically a macro for: s" <string>" type
        // TODO
        kfBiosWriteStr(".\" : not imp");
        return KF_SYSTEM_NOT_IMP;
    } else { // Run time
        while (forth->in_src.in_offset < forth->in_src.in_len) {
            char c = forth->in_src.buf[forth->in_src.in_offset];
            forth->in_src.in_offset++;
            if (c == '"')
                break;
            kfBiosWriteChar(c);
        }
    }
    return KF_STATUS_OK;
}

kfStatus W_Bye(kopForth* forth) {  // --
    return KF_SYSTEM_DONE;
}

kfStatus W_Dos(kopForth* forth) {  // --
    kfDataStackPrint(&forth->d_stack);
    return KF_STATUS_OK;
}

kfStatus W_Dor(kopForth* forth) {  // --
    kfRetnStackPrint(&forth->r_stack);
    return KF_STATUS_OK;
}

kfStatus W_Dmp(kopForth* forth) {  // addr u --
    uint8_t* a;
    usize b;
    KF_DATA_POP(b);
    KF_DATA_POP(a);
    kfBiosDumpMem(a, b);
    return KF_STATUS_OK;
}

kfStatus W_Ntr(kopForth* forth) {  // i * n +n --
    usize n;
    uint8_t* r;
    KF_RETN_POP(r);
    KF_DATA_POP(n);
    for (usize i = 0; i < n; i++) {
        isize a;
        KF_DATA_POP(a);
        KF_RETN_PUSH(a);
    }
    KF_RETN_PUSH(n);
    KF_RETN_PUSH(r);
    return KF_STATUS_OK;
}

kfStatus W_Nrf(kopForth* forth) {  // -- i * n +n
    usize n;
    uint8_t* r;
    KF_RETN_POP(r);
    KF_RETN_POP(n);
    for (usize i = 0; i < n; i++) {
        isize a;
        KF_RETN_POP(a);
        KF_DATA_PUSH(a);
    }
    KF_DATA_PUSH(n);
    KF_RETN_PUSH(r);
    return KF_STATUS_OK;
}

kfStatus W_Sip(kopForth* forth) {  // -- xn ... x1 n
    KF_DATA_PUSH(forth->in_src.source_id);
    KF_DATA_PUSH(forth->in_src.in_offset);
    KF_DATA_PUSH(forth->in_src.in_len);
    KF_DATA_PUSH(forth->in_src.buf);
    KF_DATA_PUSH(KF_INPUT_SOURCE_COUNT);
    return KF_STATUS_OK;
}

kfStatus W_Rip(kopForth* forth) {  // xn ... x1 n -- flag
    usize n;
    KF_DATA_POP(n);
    usize t;
    if (n > KF_INPUT_SOURCE_COUNT) {
        for (usize i = 0; i < n - KF_INPUT_SOURCE_COUNT; i++) {
            KF_DATA_POP(t);
        }
        n = KF_INPUT_SOURCE_COUNT;
    }
    isize m[KF_INPUT_SOURCE_COUNT];
    for (usize i = 0; i < n; i++) {
        KF_DATA_POP(m[i]);
    }
    if (n != KF_INPUT_SOURCE_COUNT) {
        KF_DATA_PUSH(-1);
        return KF_STATUS_OK;
    }
    forth->in_src.buf       = (uint8_t*) m[0];
    forth->in_src.in_len    =            m[1];
    forth->in_src.in_offset =            m[2];
    forth->in_src.source_id = (isize)    m[3];
    KF_DATA_PUSH(0);
    return KF_STATUS_OK;
}

kfStatus W_Cis(kopForth* forth) {  // --
    forth->in_src.source_id = 0;
    forth->in_src.in_offset = 0;
    forth->in_src.in_len = 0;
    forth->in_src.buf = forth->in_buf;
    for (usize i = 0; i < KF_IN_BUF_SIZE; i++)
        forth->in_buf[i] = 0;
    return KF_STATUS_OK;
}

kfStatus W_Crs(kopForth* forth) {  // --
    void* a;
    KF_RETN_POP(a);
    kfRetnStackInit(&forth->r_stack);
    KF_RETN_PUSH(a);
    return KF_STATUS_OK;
}

kfStatus W_Cds(kopForth* forth) {  // * --
    kfDataStackInit(&forth->d_stack);
    return KF_STATUS_OK;
}



// Fill native words into memory.
void kfPopulateWordsNative(kopForth* forth, kfWordsNative* wn) {
    // TODO Null check.

    wn->ext = kopForthAddNativeWord(forth, "EXIT",      W_Ext, false);  // TODO make compile only.
    wn->ext->link = NULL;
    wn->lit = kopForthAddNativeWord(forth, "(LITERAL)", W_Lit, false);  // TODO make compile only.
    wn->sub = kopForthAddNativeWord(forth, "-",         W_Sub, false);
    wn->mul = kopForthAddNativeWord(forth, "*",         W_Mul, false);
    wn->dot = kopForthAddNativeWord(forth, ".",         W_Dot, false);
    wn->att = kopForthAddNativeWord(forth, "@",         W_Att, false);
    wn->exc = kopForthAddNativeWord(forth, "!",         W_Exc, false);
    wn->cat = kopForthAddNativeWord(forth, "C@",        W_Cat, false);
    wn->cex = kopForthAddNativeWord(forth, "C!",        W_Cex, false);
    wn->rpu = kopForthAddNativeWord(forth, ">R",        W_Rpu, false);
    wn->rpo = kopForthAddNativeWord(forth, "R>",        W_Rpo, false);
    wn->drp = kopForthAddNativeWord(forth, "DROP",      W_Drp, false);
    wn->dup = kopForthAddNativeWord(forth, "DUP",       W_Dup, false);
    wn->swp = kopForthAddNativeWord(forth, "SWAP",      W_Swp, false);
    wn->bra = kopForthAddNativeWord(forth, "BRANCH",    W_Bra, false);  // TODO make compile only.
    wn->zbr = kopForthAddNativeWord(forth, "0BRANCH",   W_Zbr, false);  // TODO make compile only.
    wn->emt = kopForthAddNativeWord(forth, "EMIT",      W_Emt, false);
    wn->key = kopForthAddNativeWord(forth, "KEY",       W_Key, false);
    wn->acc = kopForthAddNativeWord(forth, "ACCEPT",    W_Acc, false);
    wn->wrd = kopForthAddNativeWord(forth, "WORD",      W_Wrd, false);
    wn->typ = kopForthAddNativeWord(forth, "TYPE",      W_Typ, false);
    wn->pcr = kopForthAddNativeWord(forth, "(CREATE)",  W_Pcr, false);
    wn->cmp = kopForthAddNativeWord(forth, "COMPARE",   W_Cmp, false);
    wn->fnd = kopForthAddNativeWord(forth, "FIND",      W_Fnd, false);
    wn->mss = kopForthAddNativeWord(forth, "M*/",       W_Mss, false);
    wn->dpl = kopForthAddNativeWord(forth, "D+",        W_Dpl, false);
    wn->equ = kopForthAddNativeWord(forth, "=",         W_Equ, false);
    wn->lss = kopForthAddNativeWord(forth, "<",         W_Lss, false);
    wn->nan = kopForthAddNativeWord(forth, "NAND",      W_Nan, false);
    wn->psq = kopForthAddNativeWord(forth, "(S\")",     W_Psq, false);  // TODO make compile only.
    wn->squ = kopForthAddNativeWord(forth, "S\"",       W_Squ, true );  // TODO make compile only.
    wn->dqu = kopForthAddNativeWord(forth, ".\"",       W_Dqu, true );
    wn->bye = kopForthAddNativeWord(forth, "BYE",       W_Bye, false);
    wn->dos = kopForthAddNativeWord(forth, ".S",        W_Dos, false);
    wn->dor = kopForthAddNativeWord(forth, ".R",        W_Dor, false);
    wn->dmp = kopForthAddNativeWord(forth, "DUMP",      W_Dmp, false);
    wn->ntr = kopForthAddNativeWord(forth, "N>R",       W_Ntr, false);
    wn->nrf = kopForthAddNativeWord(forth, "NR>",       W_Nrf, false);

    wn->sip = kopForthAddNativeWord(forth, "SAVE-INPUT",      W_Sip, false);
    wn->rip = kopForthAddNativeWord(forth, "RESTORE-INPUT",   W_Rip, false);
    wn->cis = kopForthAddNativeWord(forth, "(CLR-IN-SOURCE)", W_Cis, false);
    wn->crs = kopForthAddNativeWord(forth, "(CLR-RET-STACK)", W_Crs, false);
    wn->cds = kopForthAddNativeWord(forth, "(CLR-DAT-STACK)", W_Cds, false);
}

#endif // KF_WORDS_NATIVE_H
