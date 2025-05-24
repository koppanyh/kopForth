#ifndef KOP_FORTH_H
#define KOP_FORTH_H

/*
 * kopForth.h (last modified 2025-05-23)
 * This is the main kopForth file that gets included and pulls in all the
 * dependencies. It also includes the initialization and run routines.
 */

#include "kfBios.h"
#include "kfStack.h"
#include "kfMath.h"



  ///////////////////////
 // START: Forth Type //
///////////////////////

typedef struct kfWord kfWord;
struct kfWord {
    uint8_t name_len;
    char    name[KF_MAX_NAME_SIZE];
    kfWord*   link;
    uint8_t is_native : 1;
    uint8_t is_immediate : 1;
    uint8_t is_compile_only : 1;
    kfWord* word_def[1];
};

// Words referred to before defined.
kfWord* ext_word;
kfWord* lit_word;
kfWord* bra_word;
kfWord* zbr_word;

typedef struct {
    uint8_t*    here;              // Pointer to the next available dictionary byte.
    uint8_t*    latest;            // The latest active word, FIND starts searching here.
    uint8_t*    pending;           // Most recently defined word, but not necessarily the latest active word.
    bool        state;             // The compilation state, true=compiling, false=interpret.
    uint8_t*    pc;                // Program counter for forth inner loop.
    uint8_t     mem[KF_MEM_SIZE];  // The general memory space.
    kfDataStack d_stack;           // The data stack.
    usize       in_offset;         // The offset for the next character to read from the TIB.
    usize       tib_len;           // The total size of the text in the TIB.
    uint8_t     tib[KF_TIB_SIZE];  // The terminal input buffer.
    kfRetnStack r_stack;           // The return stack.
} kopForth;

typedef kfStatus (*kfNativeFunc)(kopForth*);

bool kfCanFitInMem(kopForth* forth, usize length) {
    uint8_t* new_next_mem_ptr = forth->here + length;
    return new_next_mem_ptr <= forth->mem + KF_MEM_SIZE;
}

kfWord* kopForthCreateWord(kopForth* forth) {
    kfWord* word = (kfWord*) forth->here;
    if (!kfCanFitInMem(forth, sizeof(*word) - sizeof(word->word_def)))
        return NULL;
    forth->here += sizeof(*word) - sizeof(word->word_def[0]);
    forth->latest = forth->pending;
    word->link = (kfWord*) forth->latest;
    forth->pending = (uint8_t*) word;
    word->is_native = false;
    word->is_immediate = false;
    word->is_compile_only = false;
    return word;
}

kfWord* kopForthAddWord(kopForth* forth, char* name) {
    kfWord* word = kopForthCreateWord(forth);
    if (word == NULL)
        return NULL;
    word->name_len = 0;
    char* word_name = word->name;
    while (*name) {
        *word_name = *name;
        word_name++;
        name++;
        word->name_len++;
    }
    *word_name = '\0';
    return word;
}

isize* kopForthAddIsize(kopForth* forth, isize value) {
    if (!kfCanFitInMem(forth, sizeof(isize)))
        return NULL;
    isize* ptr = (isize*) forth->here;
    forth->here += sizeof(isize);
    *ptr = value;
    return ptr;
}

kfWord** kopForthAddWordP(kopForth* forth, kfWord* value) {
    if (!kfCanFitInMem(forth, sizeof(kfWord*)))
        return NULL;
    kfWord** ptr = (kfWord**) forth->here;
    forth->here += sizeof(kfWord*);
    *ptr = value;
    return ptr;
}

kfWord* kopForthAddNativeWord(kopForth* forth, char* name, kfNativeFunc func_ptr,
                         bool is_immediate, bool is_compile_only) {
    if (!kfCanFitInMem(forth, sizeof(kfWord)))
        return NULL;
    kfWord* word = kopForthAddWord(forth, name);
    word->is_native = true;
    word->is_immediate = is_immediate;
    word->is_compile_only = is_compile_only;
    kopForthAddIsize(forth, (isize) func_ptr);
    return word;
}

kfWord* kopForthAddVariable(kopForth* forth, char* name, isize* var_ptr) {
    if (!kfCanFitInMem(forth, sizeof(kfWord) + 2 * sizeof(kfWord*)))
        return NULL;
    kfWord* word = kopForthAddWord(forth, name);
    kopForthAddWordP(forth, lit_word);
    kopForthAddIsize(forth, (isize) var_ptr);
    kopForthAddWordP(forth, ext_word);
    return word;
}

void kopForthAddString(kopForth* forth, char* str) {
    uint8_t* len = forth->here;
    forth->here++;
    *len = 0;
    while (*str) {
        *forth->here = *str;
        (*len)++;
        str++;
        forth->here++;
    }
}

  /////////////////////
 // END: Forth Type //
/////////////////////



  //////////////////////
 // START: Word Defs //
//////////////////////

#define KF_DATA_POP(var) KF_RETURN_IF_ERROR(kfDataStackPop(&forth->d_stack, (isize*) &var))
#define KF_RETN_POP(var) KF_RETURN_IF_ERROR(kfRetnStackPop(&forth->r_stack, (void**) &var))
#define KF_DATA_PUSH(var) KF_RETURN_IF_ERROR(kfDataStackPush(&forth->d_stack, (isize) var))
#define KF_RETN_PUSH(var) KF_RETURN_IF_ERROR(kfRetnStackPush(&forth->r_stack, (void*) var))

#define WRD(wrd) kopForthAddWordP(forth, wrd)
#define LIT(isz) kopForthAddWordP(forth, lit_word); kopForthAddIsize(forth, (isize) isz)
#define RAW(isz) kopForthAddIsize(forth, (isize) isz)
#define WRDADDR(var, wrd) kfWord** var = kopForthAddWordP(forth, wrd)
#define LITADDR(var, wrd, isz) kopForthAddWordP(forth, wrd); isize* var = kopForthAddIsize(forth, (isize) isz)
#define RAWADDR(var, isz) isize* var = kopForthAddIsize(forth, (isize) isz)
#define PRSTR(str) WRD(psq_word); kopForthAddString(forth, str); WRD(typ_word)

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
        if (c == KF_CR)
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
    if (forth->in_offset >= forth->tib_len)
        return KF_STATUS_OK;
    // skip leading `char` in input stream
    while (forth->tib[forth->in_offset] == c) {
        forth->in_offset++;
        if (forth->in_offset >= forth->tib_len)
            return KF_STATUS_OK;
    }
    // start copying !char characters to HERE+1
    uint8_t* h = forth->here + 1;
    usize ct = 0;
    while (forth->tib[forth->in_offset] != c) {
        *h = forth->tib[forth->in_offset];
        h++;
        ct++;
        forth->in_offset++;
        if (forth->in_offset >= forth->tib_len)
            break;
    }
    // update the value at HERE (1 byte)
    *forth->here = ct;
    // update the >IN to show what's been consumed
    return KF_STATUS_OK;
}

kfStatus W_Typ(kopForth* forth) {  // addr u --
    uint8_t* addr;
    isize u;
    KF_DATA_POP(u);
    KF_DATA_POP(addr);
    for (isize i = 0; i < u; i++)
        kfBiosWriteChar(*addr++);
    return KF_STATUS_OK;
}

kfStatus W_Cre(kopForth* forth) {  // --
    KF_DATA_PUSH(' ');
    KF_RETURN_IF_ERROR(W_Wrd(forth));
    KF_RETURN_IF_ERROR(W_Drp(forth));
    if (kopForthCreateWord(forth) == NULL) {
        kfBiosWriteStr("CREATE FAILED");
        return KF_SYSTEM_NULL;
    }
    return KF_STATUS_OK;
}

kfStatus W_Imm(kopForth* forth) {  // --
    kfWord* word = (kfWord*) forth->pending;
    word->is_immediate = true;
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
    isize tmp;
    kfWord* word = (kfWord*) forth->latest;
    while (word != NULL && str_ct != 0) {
        KF_DATA_PUSH(str_head);
        KF_DATA_PUSH(str_ct);
        KF_DATA_PUSH(word->name);
        KF_DATA_PUSH(word->name_len);
        KF_RETURN_IF_ERROR(W_Cmp(forth));
        KF_DATA_POP(tmp);
        if (tmp == 0) {
            KF_DATA_PUSH(word);
            KF_DATA_PUSH(word->is_immediate ? 1 : -1);
            return KF_STATUS_OK;
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
        while (forth->in_offset < forth->tib_len) {
            char c = forth->tib[forth->in_offset];
            forth->in_offset++;
            if (c == '"')
                break;
            kfBiosWriteChar(c);
        }
    }
    return KF_STATUS_OK;
}

kfStatus W_Bye(kopForth* forth) {  // --
    kfRetnStackInit(&forth->r_stack);
    return KF_STATUS_OK;
}

kfStatus W_Dos(kopForth* forth) {  // --
    kfDataStackPrint(&forth->d_stack);
    return KF_STATUS_OK;
}

void kfPopulateWords(kopForth* forth) {
    // TODO Null check
            ext_word = kopForthAddNativeWord(forth, "EXIT",      W_Ext, false, true );
            lit_word = kopForthAddNativeWord(forth, "(LIT)",     W_Lit, false, true );
    kfWord* sub_word = kopForthAddNativeWord(forth, "-",         W_Sub, false, false);
    kfWord* mul_word = kopForthAddNativeWord(forth, "*",         W_Mul, false, false);
    kfWord* dot_word = kopForthAddNativeWord(forth, ".",         W_Dot, false, false);
    kfWord* att_word = kopForthAddNativeWord(forth, "@",         W_Att, false, false);
    kfWord* exc_word = kopForthAddNativeWord(forth, "!",         W_Exc, false, false);
    kfWord* cat_word = kopForthAddNativeWord(forth, "C@",        W_Cat, false, false);
    kfWord* cex_word = kopForthAddNativeWord(forth, "C!",        W_Cex, false, false);
    kfWord* rpu_word = kopForthAddNativeWord(forth, ">R",        W_Rpu, false, false);
    kfWord* rpo_word = kopForthAddNativeWord(forth, "R>",        W_Rpo, false, false);
    kfWord* drp_word = kopForthAddNativeWord(forth, "DROP",      W_Drp, false, false);
    kfWord* dup_word = kopForthAddNativeWord(forth, "DUP",       W_Dup, false, false);
    kfWord* swp_word = kopForthAddNativeWord(forth, "SWAP",      W_Swp, false, false);
            bra_word = kopForthAddNativeWord(forth, "BRANCH",    W_Bra, false, true );
            zbr_word = kopForthAddNativeWord(forth, "0BRANCH",   W_Zbr, false, true );
    kfWord* emt_word = kopForthAddNativeWord(forth, "EMIT",      W_Emt, false, false);
    kfWord* key_word = kopForthAddNativeWord(forth, "KEY",       W_Key, false, false);
    kfWord* acc_word = kopForthAddNativeWord(forth, "ACCEPT",    W_Acc, false, false);
    kfWord* wrd_word = kopForthAddNativeWord(forth, "WORD",      W_Wrd, false, false);
    kfWord* typ_word = kopForthAddNativeWord(forth, "TYPE",      W_Typ, false, false);
    kfWord* cre_word = kopForthAddNativeWord(forth, "CREATE",    W_Cre, false, false);
    kfWord* imm_word = kopForthAddNativeWord(forth, "IMMEDIATE", W_Imm, true,  false);
    kfWord* cmp_word = kopForthAddNativeWord(forth, "COMPARE",   W_Cmp, false, false);
    kfWord* fnd_word = kopForthAddNativeWord(forth, "FIND",      W_Fnd, false, false);
    kfWord* mss_word = kopForthAddNativeWord(forth, "M*/",       W_Mss, false, false);
    kfWord* dpl_word = kopForthAddNativeWord(forth, "D+",        W_Dpl, false, false);
    kfWord* equ_word = kopForthAddNativeWord(forth, "=",         W_Equ, false, false);
    kfWord* lss_word = kopForthAddNativeWord(forth, "<",         W_Lss, false, false);
    kfWord* nan_word = kopForthAddNativeWord(forth, "NAND",      W_Nan, false, false);
    kfWord* psq_word = kopForthAddNativeWord(forth, "(S\")",     W_Psq, false, true );
    kfWord* squ_word = kopForthAddNativeWord(forth, "S\"",       W_Squ, true,  true );
    kfWord* dqu_word = kopForthAddNativeWord(forth, ".\"",       W_Dqu, true,  false);
    kfWord* bye_word = kopForthAddNativeWord(forth, "BYE",       W_Bye, false, false);
    kfWord* dos_word = kopForthAddNativeWord(forth, ".S",        W_Dos, false, false);

    kfWord* crs_word = kopForthAddNativeWord(forth, "(CLR-RET-STACK)", W_Crs, false, false);
    kfWord* cds_word = kopForthAddNativeWord(forth, "(CLR-DAT-STACK)", W_Cds, false, false);

    // Variables
    kfWord* tib_word = kopForthAddVariable(forth, "TIB",   (isize*)  forth->tib);
    kfWord* htb_word = kopForthAddVariable(forth, "#TIB",  (isize*) &forth->tib_len);
    kfWord* gin_word = kopForthAddVariable(forth, ">IN",   (isize*) &forth->in_offset);
    kfWord* dpt_word = kopForthAddVariable(forth, "DP",    (isize*) &forth->here);
    kfWord* lpt_word = kopForthAddVariable(forth, "LP",    (isize*) &forth->latest);
    kfWord* ppt_word = kopForthAddVariable(forth, "PP",    (isize*) &forth->pending);
    kfWord* sta_word = kopForthAddVariable(forth, "STATE", (isize*) &forth->state);

    // Addresses and constants
    kfWord* her_word = kopForthAddWord(forth, "HERE"); {  // ( -- a )s
        WRD(dpt_word); WRD(att_word);                     // DP @
        WRD(ext_word); }
    kfWord* lat_word = kopForthAddWord(forth, "LATEST"); {  // ( -- a )
        WRD(lpt_word); WRD(att_word);                       // LP @
        WRD(ext_word); }
    kfWord* pad_word = kopForthAddWord(forth, "PAD"); {  // ( -- a )
        WRD(her_word); LIT(-256); WRD(sub_word);         // HERE 256 +
        WRD(ext_word); }
    kfWord* tru_word = kopForthAddWord(forth, "TRUE"); {  // ( -- -1 )
        LIT(-1);
        WRD(ext_word); }
    kfWord* fal_word = kopForthAddWord(forth, "FALSE"); {  // ( -- 0 )
        LIT(0);
        WRD(ext_word); }

    // Stack manipulators and operators
    kfWord* ovr_word = kopForthAddWord(forth, "OVER"); {  // ( n1 n2 -- n1 n2 n1 )
        WRD(rpu_word); WRD(dup_word);                     // >R DUP   ( n1 n1 )
        WRD(rpo_word); WRD(swp_word);                     // R> SWAP  ( n1 n2 n1 )
        WRD(ext_word); }
    kfWord* rot_word = kopForthAddWord(forth, "ROT"); {  // ( n1 n2 n3 -- n2 n3 n1 )
        WRD(rpu_word); WRD(swp_word);                    // >R SWAP  ( n2 n1 )
        WRD(rpo_word); WRD(swp_word);                    // R> SWAP  ( n2 n3 n1 )
        WRD(ext_word); }
    kfWord* tdr_word = kopForthAddWord(forth, "2DROP"); {  // ( n1 n2 -- )
        WRD(drp_word); WRD(drp_word);
        WRD(ext_word); }
    kfWord* tdu_word = kopForthAddWord(forth, "2DUP"); {  // ( n1 n2 -- n1 n2 n1 n2 )
        WRD(ovr_word); WRD(ovr_word);                     // OVER OVER
        WRD(ext_word); }
    kfWord* add_word = kopForthAddWord(forth, "+"); {              // ( n1 n2 -- n3 )
        LIT(0); WRD(swp_word); WRD(sub_word); WRD(sub_word);       // 0 SWAP - -
        WRD(ext_word); }
    kfWord* inv_word = kopForthAddWord(forth, "INVERT"); {  // ( n1 -- n2 )
        WRD(dup_word); WRD(nan_word);                       // DUP NAND
        WRD(ext_word); }
    kfWord* orr_word = kopForthAddWord(forth, "OR"); {  // ( n1 n2 -- n3 )
        WRD(inv_word); WRD(swp_word);                   // INVERT SWAP  ( n4 n1 )
        WRD(inv_word); WRD(nan_word);                   // INVERT NAND  ( n3 )
        WRD(ext_word); }
    kfWord* and_word = kopForthAddWord(forth, "AND"); {  // ( n1 n2 -- n3 )
        WRD(nan_word); WRD(inv_word);                    // NAND INVERT
        WRD(ext_word); }
    kfWord* zeq_word = kopForthAddWord(forth, "0="); {  // ( n1 -- n2 )
        WRD(fal_word); WRD(equ_word);                   // 0 =
        WRD(ext_word); }
    kfWord* neq_word = kopForthAddWord(forth, "<>"); {  // ( n1 n2 -- n3 )
        WRD(equ_word); WRD(zeq_word);                   // = 0=
        WRD(ext_word); }
    kfWord* leq_word = kopForthAddWord(forth, "<="); {     // ( n1 n2 -- n3 )
        WRD(tdu_word);                                     // 2DUP     ( n1 n2 n1 n2 )
        WRD(lss_word); WRD(rpu_word);                      // < >R     ( n1 n2 )
        WRD(equ_word); WRD(rpo_word); WRD(orr_word);       // = R> OR  ( n3 )
        WRD(ext_word); }
    kfWord* gtr_word = kopForthAddWord(forth, ">"); {  // ( n1 n2 -- n3 )
        WRD(leq_word); WRD(zeq_word);                  // <= 0=
        WRD(ext_word); }
    kfWord* geq_word = kopForthAddWord(forth, ">="); {  // ( n1 n2 -- n3 )
        WRD(lss_word); WRD(zeq_word);                   // < 0=
        WRD(ext_word); }

    // Address manipulators
    kfWord* cls_word = kopForthAddWord(forth, "CELLS"); {  // ( n -- n )
        LIT(sizeof(isize)); WRD(mul_word);                 // 8 *
        WRD(ext_word); }
    kfWord* pex_word = kopForthAddWord(forth, "+!"); {     // ( n a -- )
        WRD(dup_word); WRD(att_word);                      // DUP @    ( n a n2 )
        WRD(swp_word);                                     // SWAP     ( n n2 a )
        WRD(rpu_word); WRD(add_word); WRD(rpo_word);       // >R + R>  ( n3 a )
        WRD(exc_word);                                     // !
        WRD(ext_word); }
    kfWord* alt_word = kopForthAddWord(forth, "ALLOT"); {  // ( n -- )
        WRD(dpt_word); WRD(pex_word);                      // DP +!
        WRD(ext_word); }
    kfWord* com_word = kopForthAddWord(forth, ","); {  // ( n -- )
        WRD(her_word); WRD(exc_word);                  // HERE !
        LIT(sizeof(isize));                            // [ 1 CELLS ] LITERAL
        WRD(alt_word);                                 // ALLOT
        WRD(ext_word); }
    kfWord* cco_word = kopForthAddWord(forth, "C,"); {  // ( n -- )
        WRD(her_word); WRD(cex_word);                   // HERE C!
        LIT(sizeof(uint8_t));                           // [ 1 CHARS ] LITERAL
        WRD(alt_word);                                  // ALLOT
        WRD(ext_word); }

    // String operations
    kfWord* crr_word = kopForthAddWord(forth, "CR"); {  // ( -- )
        LIT(KF_NL); WRD(emt_word);                      // 10 EMIT
        WRD(ext_word); }
    kfWord* bla_word = kopForthAddWord(forth, "BL"); {  // ( -- 32 )
        LIT(' ');                                       // 32
        WRD(ext_word); }
    kfWord* spa_word = kopForthAddWord(forth, "SPACE"); {  // ( -- )
        WRD(bla_word); WRD(emt_word);                      // BL EMIT
        WRD(ext_word); }
    kfWord* cnt_word = kopForthAddWord(forth, "COUNT"); {  // ( a1 -- a2 u )
        WRD(dup_word); LIT(1); WRD(add_word);              // DUP 1 +  ( a1 a2 )
        WRD(swp_word); WRD(cat_word);                      // SWAP C@  ( a2 u )
        WRD(ext_word); }
    kfWord* sst_word = kopForthAddWord(forth, "/STRING"); {  // ( a1 u1 n -- a2 u2 )
        WRD(dup_word); WRD(rpu_word); WRD(rpu_word);         // DUP >R >R  ( a1 u1 )
        WRD(swp_word); WRD(rpo_word); WRD(add_word);         // SWAP R> +  ( u1 a2 )
        WRD(swp_word); WRD(rpo_word); WRD(sub_word);         // SWAP R> -  ( a2 u2 )
        WRD(ext_word); }
    kfWord* dig_word = kopForthAddWord(forth, "DIGIT?"); {  // ( n1 -- n2 -1 | 0 )
        LIT(48); WRD(sub_word);                             // 48 -            ( n2 )
        WRD(dup_word); LIT(0); WRD(lss_word);               // DUP 0 <         ( n2 f1 )
        WRD(ovr_word); LIT(10); WRD(geq_word);              // OVER 10 >=      ( n2 f1 f2 )
        WRD(orr_word); LITADDR(c6, zbr_word, 0);            // OR IF           ( n2 )
        WRD(drp_word); WRD(fal_word);                       //     DROP FALSE  ( 0 )
        LITADDR(c7, bra_word, 0);                           // ELSE
        WRDADDR(c8, tru_word);                              //     TRUE        ( n2 -1 )
        WRDADDR(c9, ext_word);                              // THEN
        *c6 = (isize) c8;
        *c7 = (isize) c9; }
    kfWord* num_word = kopForthAddWord(forth, ">NUMBER"); {    // ( ud1 a1 u1 -- ud2 a2 u2 )
        kfWord** c10 =                                         // BEGIN
        WRD(dup_word); WRD(zeq_word);                          //     DUP 0=           ( ud a u f )
        LITADDR(c11, zbr_word, 0);                             //     IF               ( ud a u )
        WRD(ext_word);                                         //         EXIT THEN
        WRDADDR(c12, ovr_word); WRD(cat_word); WRD(dig_word);  //     OVER C@ DIGIT?   ( ud a u n f )
        LITADDR(c13, zbr_word, 0);                             //     IF               ( ud a u n )
        WRD(swp_word); LIT(1); WRD(sub_word); WRD(rpu_word);   //         SWAP 1 - >R  ( ud a n )
        WRD(swp_word); LIT(1); WRD(add_word); WRD(rpu_word);   //         SWAP 1 + >R  ( ud n )
        WRD(rpu_word);                                         //         >R           ( ud )
        LIT(10); LIT(1); WRD(mss_word);                        //         10 1 M*/     ( ud )
        WRD(rpo_word); LIT(0); WRD(dpl_word);                  //         R> 0 D+      ( ud )
        WRD(rpo_word); WRD(rpo_word);                          //         R> R>        ( ud a u )
        LITADDR(c14, bra_word, 0);                             //     ELSE
        WRDADDR(c15, ext_word);                                //         EXIT THEN
        WRDADDR(c16, bra_word); RAWADDR(c17, 0);               // AGAIN
        WRD(ext_word);
        *c11 = (isize) c12;
        *c13 = (isize) c15;
        *c14 = (isize) c16;
        *c17 = (isize) c10; }
    kfWord* snu_word = kopForthAddWord(forth, "S>NUMBER?"); {  // ( a1 u1 -- n 0 0 | d -1 0 | a2 u2 )
        // \ Save double status (true if need to drop high word)
        WRD(tdu_word); WRD(add_word); LIT(1); WRD(sub_word);   // 2DUP + 1 -
        WRD(cat_word); LIT('.'); WRD(equ_word);                // C@ [CHAR] . =
        LITADDR(c18, zbr_word, 0);                             // IF
        WRD(fal_word); WRD(rpu_word);                          //     FALSE >R
        LIT(1); WRD(sub_word);                                 //     1 -
        LITADDR(c19, bra_word, 0);                             // ELSE
        WRDADDR(c20, tru_word); WRD(rpu_word);                 //     TRUE >R THEN
        // \ Save sign multiplier
        WRDADDR(c21, ovr_word); WRD(cat_word);                 // OVER C@
        LIT('-'); WRD(equ_word);                               // [CHAR] - =
        LITADDR(c22, zbr_word, 0);                             // IF
        LIT(-1); WRD(rpu_word);                                //     -1 >R
        LIT(1); WRD(sst_word);                                 //     1 /STRING
        LITADDR(c23, bra_word, 0);                             // ELSE
        WRDADDR(c24, lit_word); RAW(1); WRD(rpu_word);         //     1 >R THEN
        // \ Add 0. to beginning of stack and parse number
        WRDADDR(c25, rpu_word); WRD(rpu_word);                 // >R >R
        LIT(0); LIT(0); WRD(rpo_word); WRD(rpo_word);          // 0. R> R>           ( 0. a u )
        WRD(num_word);                                         // >NUMBER            ( ud a u )
        // \ Check that the parsing was good
        WRD(dup_word); WRD(zeq_word);                          // DUP 0=
        LITADDR(c26, zbr_word, 0);                             // IF  \ Success
        WRD(tdr_word);                                         //     2DROP          ( ud )
        WRD(rpo_word); LIT(1); WRD(mss_word);                  //     R> 1 M*/       ( d )
        WRD(rpo_word); LITADDR(c27, zbr_word, 0);              //     R> IF
        WRD(drp_word); LIT(0); LIT(0);                         //         DROP 0 0   ( n 0 0 )
        LITADDR(c28, bra_word, 0);                             //     ELSE
        WRDADDR(c29, lit_word); RAW(-1); LIT(0);               //         -1 0 THEN  ( d -1 0 )
        WRDADDR(c30, bra_word); RAWADDR(c31, 0);               // ELSE  \ Failure
        WRDADDR(c32, rpu_word); WRD(rpu_word);                 //     >R >R
        WRD(tdr_word); WRD(rpo_word); WRD(rpo_word);           //     2DROP R> R>    ( addr u )
        WRD(rpo_word); WRD(rpo_word); WRD(tdr_word);           //     R> R> 2DROP
        WRDADDR(c33, ext_word);                                // THEN EXIT
        *c18 = (isize) c20;
        *c19 = (isize) c21;
        *c22 = (isize) c24;
        *c23 = (isize) c25;
        *c26 = (isize) c32;
        *c27 = (isize) c29;
        *c28 = (isize) c30;
        *c31 = (isize) c33; }

    // Interpreter/Compiler words
    kfWord* src_word = kopForthAddWord(forth, "SOURCE"); {  // ( -- a u )
        WRD(tib_word); WRD(htb_word); WRD(att_word);        // TIB #TIB @
        WRD(ext_word); }
    kfWord* ref_word = kopForthAddWord(forth, "REFILL"); {  // ( -- f )
        WRD(tib_word); LIT(80); WRD(acc_word);              // TIB 80 ACCEPT
        WRD(htb_word); WRD(exc_word);                       // #TIB !
        LIT(0); WRD(gin_word); WRD(exc_word);               // 0 >IN !
        WRD(spa_word); WRD(tru_word);                       // SPACE TRUE
        WRD(ext_word); }
    kfWord* exe_word = kopForthAddWord(forth, "EXECUTE"); {  // ( xt -- )
        LITADDR(c4, lit_word, 0);                            // <addr> ! <xt>
        WRD(exc_word);
        WRDADDR(c5, (kfWord*) 0);
        WRD(ext_word);
        *c4 = (isize) c5; }
    kfWord* cpl_word = kopForthAddWord(forth, "COMPILE,"); {  // ( xt -- )
        WRD(com_word);                                        // ,
        WRD(ext_word); }
    kfWord* rev_word = kopForthAddWord(forth, "REVEAL"); {  // ( -- )
        WRD(ppt_word); WRD(att_word);                       // PP @
        WRD(lpt_word); WRD(exc_word);                       // LP !
        WRD(ext_word); }
    kfWord* obr_word = kopForthAddWord(forth, "["); {      // ( -- )
        WRD(fal_word); WRD(sta_word); WRD(exc_word);       // FALSE STATE !
        WRD(ext_word);
        obr_word->is_immediate = 1; }
    kfWord* cbr_word = kopForthAddWord(forth, "]"); {      // ( -- )
        WRD(tru_word); WRD(sta_word); WRD(exc_word);       // TRUE STATE !
        WRD(ext_word); }
    kfWord* abt_word = kopForthAddWord(forth, "ABORT"); {  // ( * -- )
        // TODO undo any words being constructed
        WRD(crs_word);                                     // (CLR-RET-STACK)
        WRD(cds_word); }                                   // (CLR-DAT-STACK)
        WRDADDR(c68, ext_word);                            // QUIT
    kfWord* enf_word = kopForthAddWord(forth, "(ERR-NOT-FOUND)"); {  // ( a -- )
        WRD(crr_word);                                          // CR
        PRSTR("ERROR: '");                                      // ." ERROR: "
        WRD(cnt_word); WRD(typ_word);                           // COUNT TYPE
        PRSTR("' word not found");                              // ."  word not found"
        WRD(crr_word); WRD(abt_word);                           // CR ABORT
        WRD(ext_word); }
    kfWord* int_word = kopForthAddWord(forth, "INTERPRET"); {        // ( -- )
        LIT(0); WRD(gin_word); WRD(exc_word);                        // 0 >IN !                                   (  )
                                                                     // BEGIN                                     (  )
        WRDADDR(c34, bla_word); WRD(wrd_word);                       //     BL WORD                               ( c-addr )
        WRD(dup_word); WRD(cnt_word); WRD(swp_word); WRD(drp_word);  //     DUP COUNT SWAP DROP                   ( c-addr u )
        LITADDR(c36, zbr_word, 0);                                   // WHILE                                     ( c-addr )
        WRD(fnd_word);                                               //     FIND                                  ( c-addr 0 | xt 1 | xt -1 )
        WRD(sta_word); WRD(att_word); LITADDR(c38, zbr_word, 0);     //     STATE @ IF      \ Compiling           ( c-addr 0 | xt 1 | xt -1 )
        WRD(dup_word); LITADDR(c40, zbr_word, 0);                    //         DUP IF      \ Word                ( xt 1 | xt -1 )
        LIT(1); WRD(equ_word); LITADDR(c42, zbr_word, 0);            //             1 = IF  \ Immediate           ( xt )
        WRD(exe_word);                                               //                 EXECUTE                   ( ? )
        LITADDR(c44, bra_word, 0);                                   //             ELSE                          ( xt )
        WRDADDR(c43, cpl_word);                                      //                 COMPILE,                  (  )
                                                                     //             THEN                          ( ? )
        WRDADDR(c45, bra_word); RAWADDR(c46, 0);                     //         ELSE        \ Unknown             ( c-addr 0 )
        WRDADDR(c41, drp_word); WRD(dup_word); WRD(cnt_word);        //             DROP DUP COUNT                ( c-addr c-addr2 u )
        WRD(snu_word);                                               //             S>NUMBER?                     ( c-addr n 0 0 | c-addr d -1 0 | c-addr c-addr3 u2 )
        LITADDR(c48, zbr_word, 0);                                   //             IF      \ Error               ( c-addr c-addr3 )
        WRD(drp_word); WRD(enf_word);                                //                 DROP (ERR-NOT-FOUND)      (  )
        LITADDR(c50, bra_word, 0);                                   //             ELSE    \ Number              ( c-addr n 0 | c-addr d -1 )
        WRDADDR(c49, zbr_word); RAWADDR(c51, 0);                     //                 IF  \ Double              ( c-addr d )
        WRD(swp_word);                                               //                     SWAP                  ( c-addr n n )
        LIT(lit_word); WRD(cpl_word); WRD(com_word);                 //                     ['] (LIT) COMPILE, ,  ( c-addr n )
        WRDADDR(c52, lit_word); RAW(lit_word);                       //                 THEN ['] (LIT)            ( c-addr n xt )
        WRD(cpl_word); WRD(com_word);                                //                 COMPILE, ,                ( c-addr )
        WRD(drp_word);                                               //                 DROP                      (  )
                                                                     //             THEN                          (  )
                                                                     //         THEN                              ( ? )
        WRDADDR(c47, bra_word); RAWADDR(c53, 0);                     //     ELSE            \ Interpreting        ( c-addr 0 | xt 1 | xt -1 )
        WRDADDR(c39, zbr_word); RAWADDR(c55, 0);                     //         IF          \ Word                ( xt )
                                                                     //             // TODO check if not compile only
        WRD(exe_word);                                               //             EXECUTE                       ( ? )
        LITADDR(c57, bra_word, 0);                                   //         ELSE        \ Unknown             ( c-addr )
        WRDADDR(c56, dup_word); WRD(cnt_word); WRD(snu_word);        //             DUP COUNT S>NUMBER?           ( c-addr n 0 0 | c-addr d -1 0 | c-addr c-addr2 u )
        LITADDR(c58, zbr_word, 0);                                   //             IF      \ Error               ( c-addr c-addr2 )
        WRD(drp_word); WRD(enf_word);                                //                 DROP (ERR-NOT-FOUND)      (  )
                                                                     //             THEN    \ Number              ( c-addr n 0 | c-addr d -1 )
        WRDADDR(c59, zbr_word); RAWADDR(c60, 0);                     //             IF      \ Double              ( c-addr d )
        WRD(rot_word);                                               //                 ROT                       ( d c-addr )
        LITADDR(c62, bra_word, 0);                                   //             ELSE    \ Single              ( c-addr n )
        WRDADDR(c61, swp_word);                                      //                 SWAP                      ( n c-addr )
                                                                     //             THEN                          ( d c-addr | n c-addr )
        WRDADDR(c63, drp_word);                                      //             DROP                          ( n | d )
                                                                     //         THEN                              ( ? | n | d )
                                                                     //     THEN                                  ( ? | n | d )
        WRDADDR(c54, bra_word); RAWADDR(c35, 0);                     // REPEAT                                    ( c-addr )
        WRDADDR(c37, drp_word);                                      // DROP                                      (  )
        WRD(ext_word);
        *c35 = (isize) c34;
        *c36 = (isize) c37;
        *c38 = (isize) c39;
        *c40 = (isize) c41;
        *c42 = (isize) c43;
        *c44 = (isize) c45;
        *c46 = (isize) c47;
        *c48 = (isize) c49;
        *c50 = (isize) c47;
        *c51 = (isize) c52;
        *c53 = (isize) c54;
        *c55 = (isize) c56;
        *c57 = (isize) c54;
        *c58 = (isize) c59;
        *c60 = (isize) c61;
        *c62 = (isize) c63; }
    kfWord* col_word = kopForthAddWord(forth, ":"); {  // ( -- )
        WRD(cre_word); WRD(cbr_word);                  // CREATE POSTPONE ]
        WRD(ext_word); }
    kfWord* sem_word = kopForthAddWord(forth, ";"); {  // ( -- )
        LIT(ext_word); WRD(cpl_word);                  // ['] EXIT COMPILE,
        WRD(rev_word); WRD(obr_word);                  // REVEAL POSTPONE [
        WRD(ext_word);
        sem_word->is_immediate = 1; }
    /*kfWord* evl_word = kopForthAddWord(forth, "EVALUATE");  // ( -- )
    kfWord* pst_word = kopForthAddWord(forth, "POSTPONE");  // ( -- )*/
    kfWord* qut_word = kopForthAddWord(forth, "QUIT"); {  // ( -- )
        WRD(crs_word);                                    // (CLR-RET-STACK)
        WRD(obr_word);                                    // POSTPONE [
                                                          // BEGIN
        WRDADDR(c64, ref_word);                           //     REFILL  ( f )
        LITADDR(c65, zbr_word, 0);                        // WHILE
        WRD(int_word);                                    //     INTERPRET
        PRSTR(" ok");                                     //     ."  ok"
        WRD(crr_word);                                    //     CR
        LITADDR(c66, bra_word, 0);                        // REPEAT
        WRDADDR(c67, ext_word);
        *c66 = (isize) c64;
        *c65 = (isize) c67;
        *c68 = qut_word; }

//    kfWord* cou_word = kopForthAddWord(forth, "CNT"); {
//                       kopForthAddWordP(forth, lit_word);  // 10
//                       kopForthAddIsize(forth, 10);//00000000);
//        kfWord** c0 =  kopForthAddWordP(forth, lit_word);  // BEGIN 1 -
//                       kopForthAddIsize(forth, 1);
//                       kopForthAddWordP(forth, sub_word);
//                       kopForthAddWordP(forth, dup_word);  // DUP .
//                       kopForthAddWordP(forth, dot_word);
//                       kopForthAddWordP(forth, dup_word);  // DUP IF
//                       kopForthAddWordP(forth, zbr_word);
//        isize* c1 =    kopForthAddIsize(forth, 0);
//                       kopForthAddWordP(forth, bra_word);  // AGAIN
//        isize* c2 =    kopForthAddIsize(forth, 0);
//        kfWord** c3 =  kopForthAddWordP(forth, drp_word);  // THEN DROP
//                       kopForthAddWordP(forth, ext_word);
//        *c1 = (isize) c3;
//        *c2 = (isize) c0; }
    {  // Suppress warnings
        WRD(cbr_word);
        WRD(rev_word);
        WRD(src_word);
        WRD(cco_word);
        WRD(cls_word);
        WRD(gtr_word);
        WRD(neq_word);
        WRD(and_word);
        WRD(pad_word);
        WRD(lat_word);
        WRD(bye_word);
        WRD(dqu_word);
        WRD(cmp_word);
        WRD(imm_word);
        WRD(cre_word);
        WRD(key_word);
        WRD(dot_word);
        WRD(sem_word);
        WRD(col_word);
        WRD(squ_word);
        WRD(dos_word);
    }
}

  ////////////////////
 // END: Word Defs //
////////////////////



  /////////////////////////////////
 // START: Forth Implementation //
/////////////////////////////////

kfStatus kopForthInit(kopForth* forth) {
    if (sizeof(kfNativeFunc) != sizeof(kfWord*)) {
        kfBiosWriteStr("Pointer width mismatch");
        return KF_SYSTEM_PTRWIDTH;
    }

    for (usize i = 0; i < KF_MEM_SIZE; i++)
        forth->mem[i] = 0;
    forth->here = forth->mem;
    forth->latest = NULL;
    forth->pending = NULL;
    forth->state = false;

    kfDataStackInit(&forth->d_stack);
    kfRetnStackInit(&forth->r_stack);

    for (usize i = 0; i < KF_TIB_SIZE; i++)
        forth->tib[i] = 0;
    forth->tib_len = 0;
    forth->in_offset = 0;

    kfPopulateWords(forth);
    forth->latest = forth->pending;
    forth->pc = forth->latest;

    kfWord* latest = (kfWord*) forth->latest;
    if ((usize) latest->name - (usize) &latest->name_len != 1) {
        kfBiosWriteStr("Bad name field offset in kfWord struct");
        return KF_SYSTEM_STRUCT;
    }

    kfBiosWriteStr("kopForth v0.2, ");
    kfBiosPrintIsize(sizeof(isize) * 8);
    kfBiosWriteStr(" Bit, 2025\n");
    kfBiosPrintIsize(forth->here - forth->mem);
    kfBiosWriteStr(" bytes used of ");
    kfBiosPrintIsize(sizeof(forth->mem));
    kfBiosWriteChar('\n');

    return KF_STATUS_OK;
}

kfStatus kopForthTick(kopForth* forth) {
    kfWord* cur_word = (kfWord*) forth->pc;
    /* Debug routine
    int t = &forth->r_stack.data[RETN_STACK_SIZE] - forth->r_stack.ptr;
    for (int i = 0; i < t; i++)
        printf("  ");
    printf("%s ", cur_word->name);
    if (cur_word == lit_word || cur_word == bra_word || cur_word == zbr_word)
        printf("(%d) ", *(isize*)(*forth->r_stack.ptr));
    printf("%p < ", forth->pc);
    DataStackPrint(&forth->d_stack);
    printf(">\n");
    // */
    forth->pc = (uint8_t*) cur_word->word_def;
    if (cur_word->is_native) {
        kfNativeFunc fn = (kfNativeFunc) cur_word->word_def[0];
        KF_RETURN_IF_ERROR(fn(forth));
        if (kfRetnStackEmpty(&forth->r_stack))
            return KF_SYSTEM_DONE;
        KF_RETURN_IF_ERROR(kfRetnStackPop(&forth->r_stack, (void**) &forth->pc));
    }
    kfWord* word_addr = *(kfWord**) forth->pc;
    KF_RETURN_IF_ERROR(kfRetnStackPush(&forth->r_stack, forth->pc + sizeof(kfWord*)));
    forth->pc = (uint8_t*) word_addr;
    return KF_STATUS_OK;
}

  ///////////////////////////////
 // END: Forth Implementation //
///////////////////////////////



#endif // KOP_FORTH_H
