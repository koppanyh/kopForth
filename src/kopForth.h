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

typedef struct Word Word;
struct Word {
    uint8_t name_len;
    char    name[MAX_NAME_SIZE];
    Word*   link;
    uint8_t is_native : 1;
    uint8_t is_immediate : 1;
    uint8_t is_compile_only : 1;
    Word*   word_def[1];
};

// Words referred to before defined.
Word* ext_word;
Word* lit_word;
Word* bra_word;
Word* zbr_word;

typedef struct {
    uint8_t*  here;           // Pointer to the next available dictionary byte.
    uint8_t*  latest;         // The latest active word, FIND starts searching here.
    uint8_t*  pending;        // Most recently defined word, but not necessarily the latest active word.
    bool      state;          // The compilation state, true=compiling, false=interpret.
    uint8_t*  pc;             // Program counter for forth inner loop.
    uint8_t   mem[MEM_SIZE];  // The general memory space.
    DataStack d_stack;        // The data stack.
    usize     in_offset;      // The offset for the next character to read from the TIB.
    usize     tib_len;        // The total size of the text in the TIB.
    uint8_t   tib[TIB_SIZE];  // The terminal input buffer.
    RetnStack r_stack;        // The return stack.
} Forth;

typedef Status (*NativeFunc)(Forth*);

bool CanFitInMem(Forth* forth, usize length) {
    uint8_t* new_next_mem_ptr = forth->here + length;
    return new_next_mem_ptr <= forth->mem + MEM_SIZE;
}

Word* ForthCreateWord(Forth* forth) {
    Word* word = (Word*) forth->here;
    if (!CanFitInMem(forth, sizeof(*word) - sizeof(word->word_def)))
        return NULL;
    forth->here += sizeof(*word) - sizeof(word->word_def[0]);
    forth->latest = forth->pending;
    word->link = (Word*) forth->latest;
    forth->pending = (uint8_t*) word;
    word->is_native = false;
    word->is_immediate = false;
    word->is_compile_only = false;
    return word;
}

Word* ForthAddWord(Forth* forth, char* name) {
    Word* word = ForthCreateWord(forth);
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

isize* ForthAddIsize(Forth* forth, isize value) {
    if (!CanFitInMem(forth, sizeof(isize)))
        return NULL;
    isize* ptr = (isize*) forth->here;
    forth->here += sizeof(isize);
    *ptr = value;
    return ptr;
}

Word** ForthAddWordP(Forth* forth, Word* value) {
    if (!CanFitInMem(forth, sizeof(Word*)))
        return NULL;
    Word** ptr = (Word**) forth->here;
    forth->here += sizeof(Word*);
    *ptr = value;
    return ptr;
}

Word* ForthAddNativeWord(Forth* forth, char* name, NativeFunc func_ptr,
                         bool is_immediate, bool is_compile_only) {
    if (!CanFitInMem(forth, sizeof(Word)))
        return NULL;
    Word* word = ForthAddWord(forth, name);
    word->is_native = true;
    word->is_immediate = is_immediate;
    word->is_compile_only = is_compile_only;
    ForthAddIsize(forth, (isize) func_ptr);
    return word;
}

Word* ForthAddVariable(Forth* forth, char* name, isize* var_ptr) {
    if (!CanFitInMem(forth, sizeof(Word) + 2 * sizeof(Word*)))
        return NULL;
    Word* word = ForthAddWord(forth, name);
    ForthAddWordP(forth, lit_word);
    ForthAddIsize(forth, (isize) var_ptr);
    ForthAddWordP(forth, ext_word);
    return word;
}

void ForthAddString(Forth* forth, char* str) {
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

#define DATA_POP(var) RETURN_IF_ERROR(DataStackPop(&forth->d_stack, (isize*) &var))
#define RETN_POP(var) RETURN_IF_ERROR(RetnStackPop(&forth->r_stack, (void**) &var))
#define DATA_PUSH(var) RETURN_IF_ERROR(DataStackPush(&forth->d_stack, (isize) var))
#define RETN_PUSH(var) RETURN_IF_ERROR(RetnStackPush(&forth->r_stack, (void*) var))

#define WRD(wrd) ForthAddWordP(forth, wrd)
#define LIT(isz) ForthAddWordP(forth, lit_word); ForthAddIsize(forth, (isize) isz)
#define RAW(isz) ForthAddIsize(forth, (isize) isz)
#define WRDADDR(var, wrd) Word** var = ForthAddWordP(forth, wrd)
#define LITADDR(var, wrd, isz) ForthAddWordP(forth, wrd); isize* var = ForthAddIsize(forth, (isize) isz)
#define RAWADDR(var, isz) isize* var = ForthAddIsize(forth, (isize) isz)

#define PRSTR(str) WRD(psq_word); ForthAddString(forth, str); WRD(typ_word)

Status W_Ext(Forth* forth) {  // --
    void* a;
    RETN_POP(a);
    return STATUS_OK;
}

Status W_Lit(Forth* forth) {  // -- n
    isize* lit_val;
    RETN_POP(lit_val);
    DATA_PUSH(*lit_val);
    lit_val++;
    RETN_PUSH(lit_val);
    return STATUS_OK;
}

Status W_Sub(Forth* forth) {  // n1 n2 -- n3
    isize a, b;
    DATA_POP(b);
    DATA_POP(a);
    DATA_PUSH(a - b);
    return STATUS_OK;
}

Status W_Mul(Forth* forth) {  // n1 n2 -- n3
    isize a, b;
    DATA_POP(b);
    DATA_POP(a);
    DATA_PUSH(a * b);
    return STATUS_OK;
}

Status W_Dot(Forth* forth) {  // n --
    isize a;
    DATA_POP(a);
    BiosPrintIsize(a);
    BiosWriteChar(' ');
    return STATUS_OK;
}

Status W_Att(Forth* forth) {  // addr -- n
    isize* a;
    DATA_POP(a);
    DATA_PUSH(*a);
    return STATUS_OK;
}

Status W_Exc(Forth* forth) {  // n addr --
    isize a;
    isize* b;
    DATA_POP(b);
    DATA_POP(a);
    *b = a;
    return STATUS_OK;
}

Status W_Cat(Forth* forth) {  // addr -- n
    uint8_t* a;
    DATA_POP(a);
    DATA_PUSH(*a);
    return STATUS_OK;
}

Status W_Cex(Forth* forth) {  // n addr --
    isize a;
    uint8_t* b;
    DATA_POP(b);
    DATA_POP(a);
    *b = a;
    return STATUS_OK;
}

Status W_Rpu(Forth* forth) {  // n --
    isize a;
    void* b;
    DATA_POP(a);
    RETN_POP(b);
    RETN_PUSH(a);
    RETN_PUSH(b);
    return STATUS_OK;
}

Status W_Rpo(Forth* forth) {  // -- n
    isize a;
    void* b;
    RETN_POP(b);
    RETN_POP(a);
    DATA_PUSH(a);
    RETN_PUSH(b);
    return STATUS_OK;
}

Status W_Drp(Forth* forth) {  // n --
    isize a;
    DATA_POP(a);
    return STATUS_OK;
}

Status W_Dup(Forth* forth) {  // n -- n n
    isize a;
    DATA_POP(a);
    DATA_PUSH(a);
    DATA_PUSH(a);
    return STATUS_OK;
}

Status W_Swp(Forth* forth) {  // n1 n2 -- n2 n1
    isize a;
    isize b;
    DATA_POP(b);
    DATA_POP(a);
    DATA_PUSH(b);
    DATA_PUSH(a);
    return STATUS_OK;
}

Status W_Bra(Forth* forth) {  // --
    void* a;
    RETN_POP(a);
    a = *(void**) a;
    RETN_PUSH(a);
    return STATUS_OK;
}

Status W_Zbr(Forth* forth) {  // n --
    void* a;
    isize b;
    RETN_POP(a);
    DATA_POP(b);
    if (b == 0) {
        a = *(void**) a;
    } else {
        a += sizeof(Word*);
    }
    RETN_PUSH(a);
    return STATUS_OK;
}

Status W_Emt(Forth* forth) {  // n --
    isize a;
    DATA_POP(a);
    BiosWriteChar(a);
    return STATUS_OK;
}

Status W_Key(Forth* forth) {  // -- n
    DATA_PUSH(BiosReadChar());
    return STATUS_OK;
}

Status W_Acc(Forth* forth) {  // addr u1 -- u2
    uint8_t* addr;
    isize u1, u2 = 0;
    DATA_POP(u1);
    DATA_POP(addr);
    while (true) {
        isize c = BiosReadChar();
        if (c == CR)
            break;
        if (c == '\b') {
            if (u2 > 0)
                u2--;
            else
                continue;
            BiosWriteChar('\b');
            BiosWriteChar(' ');
            BiosWriteChar('\b');
            continue;
        }
        if (u2 >= u1)
            continue;
        BiosWriteChar(c);
        addr[u2] = c;
        u2++;
    }
    DATA_PUSH(u2);
    return STATUS_OK;
}

Status W_Wrd(Forth* forth) {  // char -- addr
    isize c;
    DATA_POP(c);
    // write 0 (len) to HERE
    *forth->here = 0;
    // put HERE on the stack
    DATA_PUSH(forth->here);
    if (forth->in_offset >= forth->tib_len)
        return STATUS_OK;
    // skip leading `char` in input stream
    while (forth->tib[forth->in_offset] == c) {
        forth->in_offset++;
        if (forth->in_offset >= forth->tib_len)
            return STATUS_OK;
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
    return STATUS_OK;
}

Status W_Typ(Forth* forth) {  // addr u --
    uint8_t* addr;
    isize u;
    DATA_POP(u);
    DATA_POP(addr);
    for (isize i = 0; i < u; i++)
        BiosWriteChar(*addr++);
    return STATUS_OK;
}

Status W_Cre(Forth* forth) {  // --
    DATA_PUSH(' ');
    RETURN_IF_ERROR(W_Wrd(forth));
    RETURN_IF_ERROR(W_Drp(forth));
    if (ForthCreateWord(forth) == NULL) {
        BiosWriteStr("CREATE FAILED");
        return SYSTEM_NULL;
    }
    return STATUS_OK;
}

Status W_Imm(Forth* forth) {  // --
    Word* word = (Word*) forth->pending;
    word->is_immediate = true;
    return STATUS_OK;
}

Status W_Cmp(Forth* forth) {  // a1 u1 a2 u2 -- n
    usize u1, u2;
    uint8_t* a1;
    uint8_t* a2;
    DATA_POP(u2);
    DATA_POP(a2);
    DATA_POP(u1);
    DATA_POP(a1);
    usize m = u1 > u2 ? u1 : u2;
    for (usize i = 0; i < m; i++) {
        if (a1[i] < a2[i]) {
            DATA_PUSH(-1);
            return STATUS_OK;
        }
        if (a1[i] > a2[i]) {
            DATA_PUSH(1);
            return STATUS_OK;
        }
    }
    if (u1 < m)
        DATA_PUSH(-1);
    else if (u2 < m)
        DATA_PUSH(1);
    else
        DATA_PUSH(0);
    return STATUS_OK;
}

Status W_Fnd(Forth* forth) {  // c-addr -- c-addr 0 | xt 1 | xt -1
    uint8_t* f_str;
    DATA_POP(f_str);
    uint8_t str_ct = *f_str;
    uint8_t* str_head = f_str + 1;
    isize tmp;
    Word* word = (Word*) forth->latest;
    while (word != NULL && str_ct != 0) {
        DATA_PUSH(str_head);
        DATA_PUSH(str_ct);
        DATA_PUSH(word->name);
        DATA_PUSH(word->name_len);
        RETURN_IF_ERROR(W_Cmp(forth));
        DATA_POP(tmp);
        if (tmp == 0) {
            DATA_PUSH(word);
            DATA_PUSH(word->is_immediate ? 1 : -1);
            return STATUS_OK;
        }
        word = word->link;
    }
    DATA_PUSH(f_str);
    DATA_PUSH(0);
    return STATUS_OK;
}

Status W_Mss(Forth* forth) {  // d1 n1 +n2 -- d2
    TwoCell doub, mult;
    isize div;
    DATA_POP(div);
    if (div != 1) {
        BiosWriteStr("M*/ +n2 != 1");
        return SYSTEM_NOT_IMP;
    }
    DATA_POP(mult.low);
    mult.high = mult.low < 0 ? -1 : 0;
    DATA_POP(doub.high);
    DATA_POP(doub.low);
    doub = ByteCellToTwoCell(ByteCellsMultiply(TwoCellToByteCell(doub), TwoCellToByteCell(mult)));
    DATA_PUSH(doub.low);
    DATA_PUSH(doub.high);
    return STATUS_OK;
}

Status W_Dpl(Forth* forth) {  // d1 d2 -- d3
    TwoCell d1, d2;
    DATA_POP(d2.high);
    DATA_POP(d2.low);
    DATA_POP(d1.high);
    DATA_POP(d1.low);
    d1 = ByteCellToTwoCell(ByteCellsAdd(TwoCellToByteCell(d1), TwoCellToByteCell(d2)));
    DATA_PUSH(d1.low);
    DATA_PUSH(d1.high);
    return STATUS_OK;
}

Status W_Equ(Forth* forth) {  // n1 n2 -- n
    isize a, b;
    DATA_POP(b);
    DATA_POP(a);
    DATA_PUSH(a == b ? -1 : 0);
    return STATUS_OK;
}

Status W_Lss(Forth* forth) {  // n1 n2 -- n
    isize a, b;
    DATA_POP(b);
    DATA_POP(a);
    DATA_PUSH(a < b ? -1 : 0);
    return STATUS_OK;
}

Status W_Nan(Forth* forth) {  // n1 n2 -- n
    isize a, b;
    DATA_POP(b);
    DATA_POP(a);
    DATA_PUSH(~(a & b));
    return STATUS_OK;
}

Status W_Crs(Forth* forth) {  // --
    void* a;
    RETN_POP(a);
    RetnStackInit(&forth->r_stack);
    RETN_PUSH(a);
    return STATUS_OK;
}

Status W_Cds(Forth* forth) {  // * --
    DataStackInit(&forth->d_stack);
    return STATUS_OK;
}

Status W_Psq(Forth* forth) {  // -- addr u
    uint8_t* a;
    RETN_POP(a);
    uint8_t len = *a;
    a++;
    char* chars = (char*) a;
    a += len;
    RETN_PUSH(a);
    DATA_PUSH(chars);
    DATA_PUSH(len);
    return STATUS_OK;
}

Status W_Squ(Forth* forth) {  // -- addr u
    // s" <len: uint8_t> <chars: char[len]>
    if (!forth->state) {  // Run time
        BiosWriteStr("S\" : not comp");
        return SYSTEM_COMP_ONLY;
    }
    // Compile time
    // TODO implement this
    BiosWriteStr("S\" : not imp");
    return SYSTEM_NOT_IMP;
    //return STATUS_OK;
}

Status W_Dqu(Forth* forth) {  // --
    if (forth->state) { // Compile time
        // Is basically a macro for: s" <string>" type
        // TODO
        BiosWriteStr(".\" : not imp");
        return SYSTEM_NOT_IMP;
    } else { // Run time
        while (forth->in_offset < forth->tib_len) {
            char c = forth->tib[forth->in_offset];
            forth->in_offset++;
            if (c == '"')
                break;
            BiosWriteChar(c);
        }
    }
    return STATUS_OK;
}

Status W_Bye(Forth* forth) {  // --
    RetnStackInit(&forth->r_stack);
    return STATUS_OK;
}

Status W_Dos(Forth* forth) {  // --
    DataStackPrint(&forth->d_stack);
    return STATUS_OK;
}

void PopulateWords(Forth* forth) {
    // TODO Null check
          ext_word = ForthAddNativeWord(forth, "EXIT",      W_Ext, false, true );
          lit_word = ForthAddNativeWord(forth, "(LIT)",     W_Lit, false, true );
    Word* sub_word = ForthAddNativeWord(forth, "-",         W_Sub, false, false);
    Word* mul_word = ForthAddNativeWord(forth, "*",         W_Mul, false, false);
    Word* dot_word = ForthAddNativeWord(forth, ".",         W_Dot, false, false);
    Word* att_word = ForthAddNativeWord(forth, "@",         W_Att, false, false);
    Word* exc_word = ForthAddNativeWord(forth, "!",         W_Exc, false, false);
    Word* cat_word = ForthAddNativeWord(forth, "C@",        W_Cat, false, false);
    Word* cex_word = ForthAddNativeWord(forth, "C!",        W_Cex, false, false);
    Word* rpu_word = ForthAddNativeWord(forth, ">R",        W_Rpu, false, false);
    Word* rpo_word = ForthAddNativeWord(forth, "R>",        W_Rpo, false, false);
    Word* drp_word = ForthAddNativeWord(forth, "DROP",      W_Drp, false, false);
    Word* dup_word = ForthAddNativeWord(forth, "DUP",       W_Dup, false, false);
    Word* swp_word = ForthAddNativeWord(forth, "SWAP",      W_Swp, false, false);
          bra_word = ForthAddNativeWord(forth, "BRANCH",    W_Bra, false, true );
          zbr_word = ForthAddNativeWord(forth, "0BRANCH",   W_Zbr, false, true );
    Word* emt_word = ForthAddNativeWord(forth, "EMIT",      W_Emt, false, false);
    Word* key_word = ForthAddNativeWord(forth, "KEY",       W_Key, false, false);
    Word* acc_word = ForthAddNativeWord(forth, "ACCEPT",    W_Acc, false, false);
    Word* wrd_word = ForthAddNativeWord(forth, "WORD",      W_Wrd, false, false);
    Word* typ_word = ForthAddNativeWord(forth, "TYPE",      W_Typ, false, false);
    Word* cre_word = ForthAddNativeWord(forth, "CREATE",    W_Cre, false, false);
    Word* imm_word = ForthAddNativeWord(forth, "IMMEDIATE", W_Imm, true,  false);
    Word* cmp_word = ForthAddNativeWord(forth, "COMPARE",   W_Cmp, false, false);
    Word* fnd_word = ForthAddNativeWord(forth, "FIND",      W_Fnd, false, false);
    Word* mss_word = ForthAddNativeWord(forth, "M*/",       W_Mss, false, false);
    Word* dpl_word = ForthAddNativeWord(forth, "D+",        W_Dpl, false, false);
    Word* equ_word = ForthAddNativeWord(forth, "=",         W_Equ, false, false);
    Word* lss_word = ForthAddNativeWord(forth, "<",         W_Lss, false, false);
    Word* nan_word = ForthAddNativeWord(forth, "NAND",      W_Nan, false, false);
    Word* psq_word = ForthAddNativeWord(forth, "(S\")",     W_Psq, false, true );
    Word* squ_word = ForthAddNativeWord(forth, "S\"",       W_Squ, true,  true );
    Word* dqu_word = ForthAddNativeWord(forth, ".\"",       W_Dqu, true,  false);
    Word* bye_word = ForthAddNativeWord(forth, "BYE",       W_Bye, false, false);
    Word* dos_word = ForthAddNativeWord(forth, ".S",        W_Dos, false, false);

    Word* crs_word = ForthAddNativeWord(forth, "(CLR-RET-STACK)", W_Crs, false, false);
    Word* cds_word = ForthAddNativeWord(forth, "(CLR-DAT-STACK)", W_Cds, false, false);

    // Variables
    Word* tib_word = ForthAddVariable(forth, "TIB",   (isize*)  forth->tib);
    Word* htb_word = ForthAddVariable(forth, "#TIB",  (isize*) &forth->tib_len);
    Word* gin_word = ForthAddVariable(forth, ">IN",   (isize*) &forth->in_offset);
    Word* dpt_word = ForthAddVariable(forth, "DP",    (isize*) &forth->here);
    Word* lpt_word = ForthAddVariable(forth, "LP",    (isize*) &forth->latest);
    Word* ppt_word = ForthAddVariable(forth, "PP",    (isize*) &forth->pending);
    Word* sta_word = ForthAddVariable(forth, "STATE", (isize*) &forth->state);

    // Addresses and constants
    Word* her_word = ForthAddWord(forth, "HERE"); {  // ( -- a )s
        WRD(dpt_word); WRD(att_word);                // DP @
        WRD(ext_word); }
    Word* lat_word = ForthAddWord(forth, "LATEST"); {  // ( -- a )
        WRD(lpt_word); WRD(att_word);                  // LP @
        WRD(ext_word); }
    Word* pad_word = ForthAddWord(forth, "PAD"); {  // ( -- a )
        WRD(her_word); LIT(-256); WRD(sub_word);    // HERE 256 +
        WRD(ext_word); }
    Word* tru_word = ForthAddWord(forth, "TRUE"); {  // ( -- -1 )
        LIT(-1);
        WRD(ext_word); }
    Word* fal_word = ForthAddWord(forth, "FALSE"); {  // ( -- 0 )
        LIT(0);
        WRD(ext_word); }

    // Stack manipulators and operators
    Word* ovr_word = ForthAddWord(forth, "OVER"); {  // ( n1 n2 -- n1 n2 n1 )
        WRD(rpu_word); WRD(dup_word);                // >R DUP   ( n1 n1 )
        WRD(rpo_word); WRD(swp_word);                // R> SWAP  ( n1 n2 n1 )
        WRD(ext_word); }
    Word* rot_word = ForthAddWord(forth, "ROT"); {  // ( n1 n2 n3 -- n2 n3 n1 )
        WRD(rpu_word); WRD(swp_word);               // >R SWAP  ( n2 n1 )
        WRD(rpo_word); WRD(swp_word);               // R> SWAP  ( n2 n3 n1 )
        WRD(ext_word); }
    Word* tdr_word = ForthAddWord(forth, "2DROP"); {  // ( n1 n2 -- )
        WRD(drp_word); WRD(drp_word);
        WRD(ext_word); }
    Word* tdu_word = ForthAddWord(forth, "2DUP"); {  // ( n1 n2 -- n1 n2 n1 n2 )
        WRD(ovr_word); WRD(ovr_word);                // OVER OVER
        WRD(ext_word); }
    Word* add_word = ForthAddWord(forth, "+"); {              // ( n1 n2 -- n3 )
        LIT(0); WRD(swp_word); WRD(sub_word); WRD(sub_word);  // 0 SWAP - -
        WRD(ext_word); }
    Word* inv_word = ForthAddWord(forth, "INVERT"); {  // ( n1 -- n2 )
        WRD(dup_word); WRD(nan_word);                // DUP NAND
        WRD(ext_word); }
    Word* orr_word = ForthAddWord(forth, "OR"); {  // ( n1 n2 -- n3 )
        WRD(inv_word); WRD(swp_word);              // INVERT SWAP  ( n4 n1 )
        WRD(inv_word); WRD(nan_word);              // INVERT NAND  ( n3 )
        WRD(ext_word); }
    Word* and_word = ForthAddWord(forth, "AND"); {  // ( n1 n2 -- n3 )
        WRD(nan_word); WRD(inv_word);               // NAND INVERT
        WRD(ext_word); }
    Word* zeq_word = ForthAddWord(forth, "0="); {  // ( n1 -- n2 )
        WRD(fal_word); WRD(equ_word);              // 0 =
        WRD(ext_word); }
    Word* neq_word = ForthAddWord(forth, "<>"); {  // ( n1 n2 -- n3 )
        WRD(equ_word); WRD(zeq_word);              // = 0=
        WRD(ext_word); }
    Word* leq_word = ForthAddWord(forth, "<="); {     // ( n1 n2 -- n3 )
        WRD(tdu_word);                                // 2DUP     ( n1 n2 n1 n2 )
        WRD(lss_word); WRD(rpu_word);                 // < >R     ( n1 n2 )
        WRD(equ_word); WRD(rpo_word); WRD(orr_word);  // = R> OR  ( n3 )
        WRD(ext_word); }
    Word* gtr_word = ForthAddWord(forth, ">"); {  // ( n1 n2 -- n3 )
        WRD(leq_word); WRD(zeq_word);             // <= 0=
        WRD(ext_word); }
    Word* geq_word = ForthAddWord(forth, ">="); {  // ( n1 n2 -- n3 )
        WRD(lss_word); WRD(zeq_word);              // < 0=
        WRD(ext_word); }

    // Address manipulators
    Word* cls_word = ForthAddWord(forth, "CELLS"); {  // ( n -- n )
        LIT(sizeof(isize)); WRD(mul_word);            // 8 *
        WRD(ext_word); }
    Word* pex_word = ForthAddWord(forth, "+!"); {     // ( n a -- )
        WRD(dup_word); WRD(att_word);                 // DUP @    ( n a n2 )
        WRD(swp_word);                                // SWAP     ( n n2 a )
        WRD(rpu_word); WRD(add_word); WRD(rpo_word);  // >R + R>  ( n3 a )
        WRD(exc_word);                                // !
        WRD(ext_word); }
    Word* alt_word = ForthAddWord(forth, "ALLOT"); {  // ( n -- )
        WRD(dpt_word); WRD(pex_word);                 // DP +!
        WRD(ext_word); }
    Word* com_word = ForthAddWord(forth, ","); {  // ( n -- )
        WRD(her_word); WRD(exc_word);             // HERE !
        LIT(sizeof(isize));                       // [ 1 CELLS ] LITERAL
        WRD(alt_word);                            // ALLOT
        WRD(ext_word); }
    Word* cco_word = ForthAddWord(forth, "C,"); {  // ( n -- )
        WRD(her_word); WRD(cex_word);              // HERE C!
        LIT(sizeof(uint8_t));                      // [ 1 CHARS ] LITERAL
        WRD(alt_word);                             // ALLOT
        WRD(ext_word); }

    // String operations
    Word* crr_word = ForthAddWord(forth, "CR"); {  // ( -- )
        LIT(NL); WRD(emt_word);                    // 10 EMIT
        WRD(ext_word); }
    Word* bla_word = ForthAddWord(forth, "BL"); {  // ( -- 32 )
        LIT(' ');                                  // 32
        WRD(ext_word); }
    Word* spa_word = ForthAddWord(forth, "SPACE"); {  // ( -- )
        WRD(bla_word); WRD(emt_word);                 // BL EMIT
        WRD(ext_word); }
    Word* cnt_word = ForthAddWord(forth, "COUNT"); {  // ( a1 -- a2 u )
        WRD(dup_word); LIT(1); WRD(add_word);         // DUP 1 +  ( a1 a2 )
        WRD(swp_word); WRD(cat_word);                 // SWAP C@  ( a2 u )
        WRD(ext_word); }
    Word* sst_word = ForthAddWord(forth, "/STRING"); {  // ( a1 u1 n -- a2 u2 )
        WRD(dup_word); WRD(rpu_word); WRD(rpu_word);    // DUP >R >R  ( a1 u1 )
        WRD(swp_word); WRD(rpo_word); WRD(add_word);    // SWAP R> +  ( u1 a2 )
        WRD(swp_word); WRD(rpo_word); WRD(sub_word);    // SWAP R> -  ( a2 u2 )
        WRD(ext_word); }
    Word* dig_word = ForthAddWord(forth, "DIGIT?"); {  // ( n1 -- n2 -1 | 0 )
        LIT(48); WRD(sub_word);                        // 48 -            ( n2 )
        WRD(dup_word); LIT(0); WRD(lss_word);          // DUP 0 <         ( n2 f1 )
        WRD(ovr_word); LIT(10); WRD(geq_word);         // OVER 10 >=      ( n2 f1 f2 )
        WRD(orr_word); LITADDR(c6, zbr_word, 0);       // OR IF           ( n2 )
        WRD(drp_word); WRD(fal_word);                  //     DROP FALSE  ( 0 )
        LITADDR(c7, bra_word, 0);                      // ELSE
        WRDADDR(c8, tru_word);                         //     TRUE        ( n2 -1 )
        WRDADDR(c9, ext_word);                         // THEN
        *c6 = (isize) c8;
        *c7 = (isize) c9; }
    Word* num_word = ForthAddWord(forth, ">NUMBER"); {         // ( ud1 a1 u1 -- ud2 a2 u2 )
        Word** c10 =                                           // BEGIN
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
    Word* snu_word = ForthAddWord(forth, "S>NUMBER?"); {      // ( a1 u1 -- n 0 0 | d -1 0 | a2 u2 )
        // \ Save double status (true if need to drop high word)
        WRD(tdu_word); WRD(add_word); LIT(1); WRD(sub_word);  // 2DUP + 1 -
        WRD(cat_word); LIT('.'); WRD(equ_word);               // C@ [CHAR] . =
        LITADDR(c18, zbr_word, 0);                            // IF
        WRD(fal_word); WRD(rpu_word);                         //     FALSE >R
        LIT(1); WRD(sub_word);                                //     1 -
        LITADDR(c19, bra_word, 0);                            // ELSE
        WRDADDR(c20, tru_word); WRD(rpu_word);                //     TRUE >R THEN
        // \ Save sign multiplier
        WRDADDR(c21, ovr_word); WRD(cat_word);                // OVER C@
        LIT('-'); WRD(equ_word);                              // [CHAR] - =
        LITADDR(c22, zbr_word, 0);                            // IF
        LIT(-1); WRD(rpu_word);                               //     -1 >R
        LIT(1); WRD(sst_word);                                //     1 /STRING
        LITADDR(c23, bra_word, 0);                            // ELSE
        WRDADDR(c24, lit_word); RAW(1); WRD(rpu_word);        //     1 >R THEN
        // \ Add 0. to beginning of stack and parse number
        WRDADDR(c25, rpu_word); WRD(rpu_word);                // >R >R
        LIT(0); LIT(0); WRD(rpo_word); WRD(rpo_word);         // 0. R> R>           ( 0. a u )
        WRD(num_word);                                        // >NUMBER            ( ud a u )
        // \ Check that the parsing was good
        WRD(dup_word); WRD(zeq_word);                         // DUP 0=
        LITADDR(c26, zbr_word, 0);                            // IF  \ Success
        WRD(tdr_word);                                        //     2DROP          ( ud )
        WRD(rpo_word); LIT(1); WRD(mss_word);                 //     R> 1 M*/       ( d )
        WRD(rpo_word); LITADDR(c27, zbr_word, 0);             //     R> IF
        WRD(drp_word); LIT(0); LIT(0);                        //         DROP 0 0   ( n 0 0 )
        LITADDR(c28, bra_word, 0);                            //     ELSE
        WRDADDR(c29, lit_word); RAW(-1); LIT(0);              //         -1 0 THEN  ( d -1 0 )
        WRDADDR(c30, bra_word); RAWADDR(c31, 0);              // ELSE  \ Failure
        WRDADDR(c32, rpu_word); WRD(rpu_word);                //     >R >R
        WRD(tdr_word); WRD(rpo_word); WRD(rpo_word);          //     2DROP R> R>    ( addr u )
        WRD(rpo_word); WRD(rpo_word); WRD(tdr_word);          //     R> R> 2DROP
        WRDADDR(c33, ext_word);                               // THEN EXIT
        *c18 = (isize) c20;
        *c19 = (isize) c21;
        *c22 = (isize) c24;
        *c23 = (isize) c25;
        *c26 = (isize) c32;
        *c27 = (isize) c29;
        *c28 = (isize) c30;
        *c31 = (isize) c33; }

    // Interpreter/Compiler words
    Word* src_word = ForthAddWord(forth, "SOURCE"); {  // ( -- a u )
        WRD(tib_word); WRD(htb_word); WRD(att_word);   // TIB #TIB @
        WRD(ext_word); }
    Word* ref_word = ForthAddWord(forth, "REFILL"); {  // ( -- f )
        WRD(tib_word); LIT(80); WRD(acc_word);         // TIB 80 ACCEPT
        WRD(htb_word); WRD(exc_word);                  // #TIB !
        LIT(0); WRD(gin_word); WRD(exc_word);          // 0 >IN !
        WRD(spa_word); WRD(tru_word);                  // SPACE TRUE
        WRD(ext_word); }
    Word* exe_word = ForthAddWord(forth, "EXECUTE"); {  // ( xt -- )
        LITADDR(c4, lit_word, 0);                       // <addr> ! <xt>
        WRD(exc_word);
        WRDADDR(c5, (Word*) 0);
        WRD(ext_word);
        *c4 = (isize) c5; }
    Word* cpl_word = ForthAddWord(forth, "COMPILE,"); {  // ( xt -- )
        WRD(com_word);                                   // ,
        WRD(ext_word); }
    Word* rev_word = ForthAddWord(forth, "REVEAL"); {  // ( -- )
        WRD(ppt_word); WRD(att_word);                  // PP @
        WRD(lpt_word); WRD(exc_word);                  // LP !
        WRD(ext_word); }
    Word* obr_word = ForthAddWord(forth, "["); {      // ( -- )
        WRD(fal_word); WRD(sta_word); WRD(exc_word);  // FALSE STATE !
        WRD(ext_word);
        obr_word->is_immediate = 1; }
    Word* cbr_word = ForthAddWord(forth, "]"); {      // ( -- )
        WRD(tru_word); WRD(sta_word); WRD(exc_word);  // TRUE STATE !
        WRD(ext_word); }
    Word* abt_word = ForthAddWord(forth, "ABORT"); {  // ( * -- )
        // TODO undo any words being constructed
        WRD(crs_word);                                // (CLR-RET-STACK)
        WRD(cds_word); }                              // (CLR-DAT-STACK)
        WRDADDR(c68, ext_word);                       // QUIT
    Word* enf_word = ForthAddWord(forth, "(ERR-NOT-FOUND)"); {  // ( a -- )
        WRD(crr_word);                                          // CR
        PRSTR("ERROR: '");                                      // ." ERROR: "
        WRD(cnt_word); WRD(typ_word);                           // COUNT TYPE
        PRSTR("' word not found");                              // ."  word not found"
        WRD(crr_word); WRD(abt_word);                           // CR ABORT
        WRD(ext_word); }
    Word* int_word = ForthAddWord(forth, "INTERPRET"); {             // ( -- )
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
    Word* col_word = ForthAddWord(forth, ":"); {  // ( -- )
        WRD(cre_word); WRD(cbr_word);             // CREATE POSTPONE ]
        WRD(ext_word); }
    Word* sem_word = ForthAddWord(forth, ";"); {  // ( -- )
        LIT(ext_word); WRD(cpl_word);             // ['] EXIT COMPILE,
        WRD(rev_word); WRD(obr_word);             // REVEAL POSTPONE [
        WRD(ext_word);
        sem_word->is_immediate = 1; }
    /*Word* evl_word = ForthAddWord(forth, "EVALUATE");  // ( -- )
    Word* pst_word = ForthAddWord(forth, "POSTPONE");  // ( -- )*/
    Word* qut_word = ForthAddWord(forth, "QUIT"); {  // ( -- )
        WRD(crs_word);                               // (CLR-RET-STACK)
        WRD(obr_word);                               // POSTPONE [
                                                     // BEGIN
        WRDADDR(c64, ref_word);                      //     REFILL  ( f )
        LITADDR(c65, zbr_word, 0);                   // WHILE
        WRD(int_word);                               //     INTERPRET
        PRSTR(" ok");                                //     ."  ok"
        WRD(crr_word);                               //     CR
        LITADDR(c66, bra_word, 0);                   // REPEAT
        WRDADDR(c67, ext_word);
        *c66 = (isize) c64;
        *c65 = (isize) c67;
        *c68 = qut_word; }

//    Word* cou_word = ForthAddWord(forth, "CNT"); {
//                    ForthAddWordP(forth, lit_word);  // 10
//                    ForthAddIsize(forth, 10);//00000000);
//        Word** c0 = ForthAddWordP(forth, lit_word);  // BEGIN 1 -
//                    ForthAddIsize(forth, 1);
//                    ForthAddWordP(forth, sub_word);
//                    ForthAddWordP(forth, dup_word);  // DUP .
//                    ForthAddWordP(forth, dot_word);
//                    ForthAddWordP(forth, dup_word);  // DUP IF
//                    ForthAddWordP(forth, zbr_word);
//        isize* c1 = ForthAddIsize(forth, 0);
//                    ForthAddWordP(forth, bra_word);  // AGAIN
//        isize* c2 = ForthAddIsize(forth, 0);
//        Word** c3 = ForthAddWordP(forth, drp_word);  // THEN DROP
//                    ForthAddWordP(forth, ext_word);
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

Status ForthInit(Forth* forth) {
    if (sizeof(NativeFunc) != sizeof(Word*)) {
        BiosWriteStr("Pointer width mismatch");
        return SYSTEM_PTRWIDTH;
    }

    for (usize i = 0; i < MEM_SIZE; i++)
        forth->mem[i] = 0;
    forth->here = forth->mem;
    forth->latest = NULL;
    forth->pending = NULL;
    forth->state = false;

    DataStackInit(&forth->d_stack);
    RetnStackInit(&forth->r_stack);

    for (usize i = 0; i < TIB_SIZE; i++)
        forth->tib[i] = 0;
    forth->tib_len = 0;
    forth->in_offset = 0;

    PopulateWords(forth);
    forth->latest = forth->pending;
    forth->pc = forth->latest;

    Word* latest = (Word*) forth->latest;
    if ((usize) latest->name - (usize) &latest->name_len != 1) {
        BiosWriteStr("Bad name field offset in Word struct");
        return SYSTEM_STRUCT;
    }

    BiosWriteStr("kopForth v0.2, ");
    BiosPrintIsize(sizeof(isize) * 8);
    BiosWriteStr(" Bit, 2025\n");
    BiosPrintIsize(forth->here - forth->mem);
    BiosWriteStr(" bytes used of ");
    BiosPrintIsize(sizeof(forth->mem));
    BiosWriteChar('\n');

    return STATUS_OK;
}

Status ForthTick(Forth* forth) {
    Word* cur_word = (Word*) forth->pc;
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
        NativeFunc fn = (NativeFunc) cur_word->word_def[0];
        RETURN_IF_ERROR(fn(forth));
        if (RetnStackEmpty(&forth->r_stack))
            return SYSTEM_DONE;
        RETURN_IF_ERROR(RetnStackPop(&forth->r_stack, (void**) &forth->pc));
    }
    Word* word_addr = *(Word**) forth->pc;
    RETURN_IF_ERROR(RetnStackPush(&forth->r_stack, forth->pc + sizeof(Word*)));
    forth->pc = (uint8_t*) word_addr;
    return STATUS_OK;
}

  ///////////////////////////////
 // END: Forth Implementation //
///////////////////////////////



#endif // KOP_FORTH_H
