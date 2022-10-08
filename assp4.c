/*:ts=4                         assp4.c
 *
 *    cp4 - Commodore C+4 emulator
 *    Copyright (C) 1998 Gáti Gergely
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *    e-mail: gatig@dragon.klte.hu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "macros.h"
#include "common.h"
#include "licence.h"

#undef INLINE
#define INLINE

#define ierr()  fprintf(stderr,"%s: ***internal error in `%s' at line %d\n",progname,__FILE__,__LINE__)

/*
 * Error codes
 */
#define ER_UNKNOWN                  1
#define ER_BRANCHOUTOFRANGE         2
#define ER_LABELNOTFOUND            3
#define ER_INVALIDADDRESSING        4
#define ER_NOADDRESS                5
#define ER_TOOMANYMACRO             6
#define ER_NOMEM                    7
#define ER_MACRONOTFOUND            8

char *VER="\0$VER: assp4 1.8 (22.12.99) Copyright © by Gáti Gergely";

char *errstr[]={
    "ok",
    "unknown",
    "branch out of range",
    "label not found: ",
    "invalid addressing mode",
    "no address part of symbol",
    "too many use of macro",
    "not enough memory",
    "macro not found",
    NULL
};

/*
 * internal mnemonics
 */
char *imnm[]={
    "DAT",                  // egy byte elhelyezése ( DAT [#]$12 )
    "BSS",                  // sok byte elhelyezése ( BSS [#]$12[,$x] )
    "STR",                  // sorvégéig string elhelyezése (petscii)
    "MAC",                  // macro start
    "<<<",                  // endmacro
    "USE",                  // usemacro (nem kötelezõ)
    "EQU",                  // define-hoz
    "INL",                  // include
    "XDF",                  // kívûlrõl is látható labelekhez
    NULL
};
#define DAT         0
#define BSS         1
#define STR         2
#define MAC         3
#define END         4
#define USE         5
#define EQU         6
#define INL         7
#define XDF         8


/*
 * Addressing modes
 */
#define  ILL        0
#define  BYTE       1
#define  ABS        2
#define  ZP         3
#define  ACC        4
#define  IMP        5
#define  IZPX       6
#define  IZPY       7
#define  ZPX        8
#define  ABSX       9
#define  ABSY       10
#define  REL        11
#define  IABS       12
#define  ZPY        13

/*
 * Statements defines / names
 */
#define ADC 0
#define AND 1
#define ASL 2
#define BCC 3
#define BCS 4
#define BEQ 5
#define BIT 6
#define BMI 7
#define BNE 8
#define BPL 9
#define BRK 10
#define BVC 11
#define BVS 12
#define CLC 13
#define CLD 14
#define CLI 15
#define CLV 16
#define CMP 17
#define CPX 18
#define CPY 19
#define DEC 20
#define DEX 21
#define DEY 22
#define EOR 23
#define INC 24
#define INX 25
#define INY 26
#define JMP 27
#define JSR 28
#define LDA 29
#define LDX 30
#define LDY 31
#define LSR 32
#define NOP 33
#define ORA 34
#define PHA 35
#define PHP 36
#define PLA 37
#define PLP 38
#define ROL 39
#define ROR 40
#define RTI 41
#define RTS 42
#define SBC 43
#define SEC 44
#define SED 45
#define SEI 46
#define STA 47
#define STX 48
#define STY 49
#define TAX 50
#define TAY 51
#define TSX 52
#define TXA 53
#define TXS 54
#define TYA 55
#define XXX 56

#define LAB_GLB 0
#define LAB_SUB 1

// label-table
struct lnode;
struct macro;
struct ltab {
    struct lnode *head;
    struct fill *fhd;
};
struct lnode {
    struct lnode *next;
    char *name;
    int addr;           // normal (-1 -> xdef)
    int lineno;
    struct macro *mac;  // MAC
    int val;            // EQU ($0000-$ffff) -1 invalid
};
struct fill {
    struct fill *next;
    char *label;
    long filep;
    int type;
    int lineno;
    int addr;           // real address of the statement of the fill
    struct macro *mac;
};
// `type' mezõhöz:
#define F_REL   0       // 1 byte
#define F_WORD  1       // 2 byte (lohi)
#define F_HI    2       // 1 byte (hi)
#define F_LO    3       // 1 byte (lo)
#define F_MACRO 4       // x byte
#define F_LOCAL 128     // 7. bit local hivatkozáshoz

struct macro {
    struct macro *next;
    char *name;
    int len;
    int lineno;
    unsigned char *data;
    int usecount;
    struct ltab ltab;
};
struct mac {
    int on;             // macro mode? (1-ya)
    unsigned char *mem; // alloced mem for macros
    int size;           // alloced mem size
    int pnt;            // size of macro
    struct macro *first;// first macro
};
#define MACROMEMCHUNK   1024
struct global {
    struct mac macros;
    int startaddr;
    struct lnode *actlab;
    int foffs;
};

int assemble(FILE *fin,FILE *fp,struct ltab *lt,int startaddr);


// statement struct
struct stmn {
    char mnm[4];
    int from;
};

struct stmn statements[]={
    {"ADC",0},{"AND",8},{"ASL",16},{"BCC",21},{"BCS",22},{"BEQ",23},{"BIT",24},{"BMI",26},
    {"BNE",27},{"BPL",28},{"BRK",29},{"BVC",30},{"BVS",31},{"CLC",32},{"CLD",33},{"CLI",34},
    {"CLV",35},{"CMP",36},{"CPX",44},{"CPY",47},{"DEC",50},{"DEX",54},{"DEY",55},{"EOR",56},
    {"INC",64},{"INX",68},{"INY",69},{"JMP",70},{"JSR",72},{"LDA",73},{"LDX",81},{"LDY",86},
    {"LSR",91},{"NOP",96},{"ORA",97},{"PHA",105},{"PHP",106},{"PLA",107},{"PLP",108},{"ROL",109},
    {"ROR",114},{"RTI",119},{"RTS",120},{"SBC",121},{"SEC",129},{"SED",130},{"SEI",131},{"STA",132},
    {"STX",139},{"STY",142},{"TAX",145},{"TAY",146},{"TSX",147},{"TXA",148},{"TXS",149},{"TYA",150},{"",-1}
};


// machine struct
struct asmstat {
    int name;
    int mode;
    int len;
    int code;
};


struct asmstat stat[]={
//     NAME,MODE        LEN CODE
//-------------------------------
    {   ADC,ABS,        3,  109 },
    {   ADC,ABSX,       3,  125 },
    {   ADC,ABSY,       3,  121 },
    {   ADC,BYTE,       2,  105 },
    {   ADC,IZPX,       2,  97  },
    {   ADC,IZPY,       2,  113 },
    {   ADC,ZP,         2,  101 },
    {   ADC,ZPX,        2,  117 },
    {   AND,ABS,        3,  45  },
    {   AND,ABSX,       3,  61  },
    {   AND,ABSY,       3,  57  },
    {   AND,BYTE,       2,  41  },
    {   AND,IZPX,       2,  33  },
    {   AND,IZPY,       2,  49  },
    {   AND,ZP,         2,  37  },
    {   AND,ZPX,        2,  53  },
    {   ASL,ABS,        3,  14  },
    {   ASL,ABSX,       2,  30  },
    {   ASL,ACC,        1,  10  },
    {   ASL,ZP,         2,  6   },
    {   ASL,ZPX,        2,  22  },
    {   BCC,REL,        2,  144 },
    {   BCS,REL,        2,  176 },
    {   BEQ,REL,        2,  240 },
    {   BIT,ABS,        2,  44  },
    {   BIT,ZP,         2,  36  },
    {   BMI,REL,        2,  48  },
    {   BNE,REL,        2,  208 },
    {   BPL,REL,        2,  16  },
    {   BRK,IMP,        1,  0   },
    {   BVC,REL,        2,  80  },
    {   BVS,REL,        2,  112 },
    {   CLC,IMP,        1,  24  },
    {   CLD,IMP,        1,  216 },
    {   CLI,IMP,        1,  88  },
    {   CLV,IMP,        1,  184 },
    {   CMP,ABS,        3,  205 },
    {   CMP,ABSX,       3,  221 },
    {   CMP,ABSY,       3,  217 },
    {   CMP,BYTE,       2,  201 },
    {   CMP,IZPX,       2,  193 },
    {   CMP,IZPY,       2,  209 },
    {   CMP,ZP,         2,  197 },
    {   CMP,ZPX,        2,  213 },
    {   CPX,ABS,        3,  236 },
    {   CPX,BYTE,       2,  224 },
    {   CPX,ZP,         2,  228 },
    {   CPY,ABS,        3,  204 },
    {   CPY,BYTE,       2,  192 },
    {   CPY,ZP,         2,  196 },
    {   DEC,ABS,        3,  206 },
    {   DEC,ABSX,       3,  222 },
    {   DEC,ZP,         2,  198 },
    {   DEC,ZPX,        2,  214 },
    {   DEX,IMP,        1,  202 },
    {   DEY,IMP,        1,  136 },
    {   EOR,ABS,        3,  77  },
    {   EOR,ABSX,       3,  93  },
    {   EOR,ABSY,       3,  89  },
    {   EOR,BYTE,       2,  73  },
    {   EOR,IZPX,       2,  65  },
    {   EOR,IZPY,       2,  81  },
    {   EOR,ZP,         2,  69  },
    {   EOR,ZPX,        2,  85  },
    {   INC,ABS,        3,  238 },
    {   INC,ABSX,       3,  254 },
    {   INC,ZP,         2,  230 },
    {   INC,ZPX,        2,  246 },
    {   INX,IMP,        1,  232 },
    {   INY,IMP,        1,  200 },
    {   JMP,ABS,        3,  76  },
    {   JMP,IABS,       3,  108 },
    {   JSR,ABS,        3,  32  },
    {   LDA,ABS,        3,  173 },
    {   LDA,ABSX,       3,  189 },
    {   LDA,ABSY,       3,  185 },
    {   LDA,BYTE,       2,  169 },
    {   LDA,IZPX,       2,  161 },
    {   LDA,IZPY,       2,  177 },
    {   LDA,ZP,         2,  165 },
    {   LDA,ZPX,        2,  181 },
    {   LDX,ABS,        3,  174 },
    {   LDX,ABSY,       3,  190 },
    {   LDX,BYTE,       2,  162 },
    {   LDX,ZP,         2,  166 },
    {   LDX,ZPY,        2,  182 },
    {   LDY,ABS,        3,  172 },
    {   LDY,ABSX,       3,  188 },
    {   LDY,BYTE,       2,  160 },
    {   LDY,ZP,         2,  164 },
    {   LDY,ZPX,        2,  180 },
    {   LSR,ABS,        3,  78  },
    {   LSR,ABSX,       3,  94  },
    {   LSR,ACC,        1,  74  },
    {   LSR,ZP,         2,  70  },
    {   LSR,ZPX,        2,  86  },
    {   NOP,IMP,        1,  234 },
    {   ORA,ABS,        3,  13  },
    {   ORA,ABSX,       3,  29  },
    {   ORA,ABSY,       3,  25  },
    {   ORA,BYTE,       2,  9   },
    {   ORA,IZPX,       2,  1   },
    {   ORA,IZPY,       2,  17  },
    {   ORA,ZP,         2,  5   },
    {   ORA,ZPX,        2,  21  },
    {   PHA,IMP,        1,  72  },
    {   PHP,IMP,        2,  8   },
    {   PLA,ACC,        1,  104 },
    {   PLP,IMP,        1,  40  },
    {   ROL,ABS,        3,  46  },
    {   ROL,ABSX,       3,  62  },
    {   ROL,ACC,        1,  42  },
    {   ROL,ZP,         2,  38  },
    {   ROL,ZPX,        2,  54  },
    {   ROR,ABS,        3,  110 },
    {   ROR,ABSX,       3,  126 },
    {   ROR,ACC,        1,  106 },
    {   ROR,ZP,         2,  102 },
    {   ROR,ZPX,        2,  118 },
    {   RTI,IMP,        1,  64  },
    {   RTS,IMP,        2,  96  },
    {   SBC,ABS,        3,  237 },
    {   SBC,ABSX,       3,  253 },
    {   SBC,ABSY,       3,  249 },
    {   SBC,BYTE,       2,  233 },
    {   SBC,IZPX,       2,  225 },
    {   SBC,IZPY,       2,  241 },
    {   SBC,ZP,         2,  229 },
    {   SBC,ZPX,        3,  245 },
    {   SEC,IMP,        1,  56  },
    {   SED,IMP,        1,  248 },
    {   SEI,IMP,        1,  120 },
    {   STA,ABS,        3,  141 },
    {   STA,ABSX,       3,  157 },
    {   STA,ABSY,       3,  153 },
    {   STA,IZPX,       2,  129 },
    {   STA,IZPY,       2,  145 },
    {   STA,ZP,         2,  133 },
    {   STA,ZPX,        2,  149 },
    {   STX,ABS,        3,  142 },
    {   STX,ZP,         2,  134 },
    {   STX,ZPY,        2,  150 },
    {   STY,ABS,        3,  140 },
    {   STY,ZP,         2,  132 },
    {   STY,ZPX,        2,  148 },
    {   TAX,IMP,        1,  170 },
    {   TAY,IMP,        1,  168 },
    {   TSX,IMP,        1,  186 },
    {   TXA,IMP,        1,  138 },
    {   TXS,IMP,        1,  154 },
    {   TYA,IMP,        1,  152 }
};


/*
 * Globals
 * =======
 */
static char *progname;          // argv[0]
static int branch[]={ BCC,BCS,BMI,BPL,BEQ,BNE,BVC,BVS,-1 };
static struct global *globals;  // reentrant support

/* Options
 */
static int o_sym=0;             // symbol-printing? (0=no)


/*
 * összehasonlít két stringet (case insensitive)
 */
INLINE int istrcmp(char *s1,char *s2) {
    while(toupper(*s1)==toupper(*s2)&&toupper(*s1)) {
        s1++; s2++;
    }
    return(toupper(*s1)-toupper(*s2));
} // istrcmp()


/*
 * összehasonlít két stringet (csak n char, case insensitive)
 */
INLINE int istrncmp(char *s1,char *s2,int n) {
    while(toupper(*s1)==toupper(*s2)&&toupper(*s1)&&--n>0) {
        s1++; s2++;
    }
    return(toupper(*s1)-toupper(*s2));
} // istrncmp()


/*
 * kikeres egy mnemonikot a fenti tömbbõl
 */
INLINE struct asmstat *lookup(char *mnm) {
    int i;

    for(i=0;statements[i].from!=-1&&istrcmp(statements[i].mnm,mnm)!=0;i++);
    if(statements[i].from==-1) return(NULL);
    return(&stat[statements[i].from]);
} // lookup()


/*
 * newtab
 */
struct ltab *newtab(void) {
    struct ltab *t;

    if(NULL==(t=malloc(sizeof(struct ltab)))) return(NULL);
    t->head=NULL;
    t->fhd=NULL;
    return(t);
} // newtab()

/*
 * droptab
 */
void droptab(struct ltab *lt) {
    struct macro *mc,*mx;
    struct lnode *n,*x;
    struct fill *f,*g;
    int numsym=0,i;

    if(lt==NULL) return;
    if(o_sym!=0) {
        printf("\n\n  TYPE  VALUE   LINE   NAME\n");
        for(numsym=0,n=lt->head;n!=NULL;n=x,numsym++) x=n->next;
        for(;numsym>0;numsym--) {
            for(i=1,n=lt->head;i<numsym;n=x,i++) x=n->next;
            if(n->mac!=NULL) printf("  Macro       - %4d  `%s'\n",n->lineno,n->name);
            else if(n->val>=0) printf("  Const $%04x - %4d  `%s'\n",n->val,n->lineno,n->name);
            else if(n->addr<0) printf("  Globl       - %4d  `%s'\n",n->lineno,&n->name[1]);
            else printf("  Label $%04x - %4d  `%s'\n",n->addr,n->lineno,n->name);
        }
    }
    for(mc=globals->macros.first;mc!=NULL;mc=mx) {
        mx=mc->next;
        if(mc->name!=NULL) free(mc->name);
        if(mc->data!=NULL) free(mc->data);
        for(f=mc->ltab.fhd;f!=NULL;f=g) {
            g=f->next;
            if(f->label!=NULL) free(f->label);
            free(f);
        }
        for(n=mc->ltab.head;n!=NULL;n=x) {
            x=n->next;
            if(n->name!=NULL) free(n->name);
            free(n);
        }
    }
    for(n=lt->head;n!=NULL;n=x) {
        x=n->next;
        if(n->name!=NULL) free(n->name);
        free(n);
    }
    for(f=lt->fhd;f!=NULL;f=g) {
        g=f->next;
        if(f->label!=NULL) free(f->label);
        free(f);
    }
    free(lt);
} // droptab()


/* lokális nevet kreál ->
 * n1="hello"
 * n2=".name"
 * ret="hello/name"
 */
INLINE char *createlocname(char *n) {
    char *r;
    if(n[0]=='.') {
        if(globals->actlab==NULL) return(NULL);
        if(NULL==(r=malloc(strlen(n)+strlen(globals->actlab->name)))) return(r);
        strcpy(r,globals->actlab->name);
        strcat(r,"/");
        strcat(r,n+1);
    } else {
        if(NULL==(r=malloc(strlen(n)+1))) return(r);
        strcpy(r,n);
    }
    return(r);
}


/* macro bejegyzés kezdete
 */
int macstart(struct lnode *lb) {
    struct macro *nm;

    if(globals->macros.on!=0) return(-1);
    if(NULL==(nm=malloc(sizeof(struct macro)))) return(-1);
    nm->next=globals->macros.first;
    if(NULL==(nm->name=malloc(strlen(lb->name)+1))) { free(nm); return(-1); }
    strcpy(nm->name,lb->name);
    nm->len=-1;
    nm->lineno=0;
    nm->data=NULL;
    nm->ltab.fhd=NULL;
    nm->usecount=0;
    nm->ltab.head=NULL;
    if(NULL==(globals->macros.mem=malloc(MACROMEMCHUNK))) { free(nm->name); free(nm); return(-1); }
    globals->macros.size=MACROMEMCHUNK;
    globals->macros.pnt=0;
    globals->macros.first=nm;
    globals->macros.on=1;
    lb->mac=nm;
    return(0);
}


/* macro bejegyzés vége
 */
void macend(void) {
    if(globals->macros.on==0) return;
    globals->macros.on=0;
    globals->macros.first->len=globals->macros.pnt;
    globals->macros.first->lineno=0;
    globals->macros.first->data=globals->macros.mem;
    globals->macros.mem=NULL;
    globals->macros.size=0;
    globals->macros.pnt=0;
}


INLINE int outbyte(int byte, FILE *fp) {
    int ret=0;

    if(globals->macros.on!=0) {
        if(globals->macros.mem==NULL) return(-1);
        if(globals->macros.pnt>=globals->macros.size) {
            globals->macros.mem=realloc(globals->macros.mem,globals->macros.size+MACROMEMCHUNK);
            if(NULL==globals->macros.mem) {
                globals->macros.mem=NULL;
                return(-1);
            }
            globals->macros.size+=MACROMEMCHUNK;
        }
        globals->macros.mem[globals->macros.pnt++]=(unsigned char)byte;
    } else ret=fputc(byte,fp);
    return(ret);
}


/* kikeresi a macrok közül a `name' nevût (vagy NULL)
 */
INLINE struct macro *macseek(char *name) {
    struct macro *m;
    char *n;
    n=createlocname(name);
    for(m=globals->macros.first;m!=NULL;m=m->next) if(strcmp(m->name,n)==0) break;
    free(n);
    return(m);
}


/* insert macro `name' into `fp'
 * beszúr annyi üres byte-ot, amennyit a macro elfoglal
 * ret=-1 if error
 *    >0 byte count in fp
 */
INLINE int macins(char *name,FILE *fp) {
    struct macro *mc;
    int i;

    if(NULL==(mc=macseek(name))) return(-1);
    if(mc->len<0) return(-1);
    if(mc->len==0) return(0);
    for(i=0;i<mc->len;i++) outbyte(mc->data[i],fp);
    return(i);
}


/*
 * labsea
 */
INLINE struct lnode *labsea(struct ltab *lt,char *lab) {
    struct lnode *n;

    if(lt==NULL||lab==NULL) return(NULL);
    for(n=lt->head;n!=NULL&&strcmp(lab,n->name)!=0;n=n->next);
    return(n);
} // labsea()


/*
 * addlabel
 *
 *  mode==LAB_GLB | LAB_SUB
 *
 * 0-OK, <0 hiba
 */
INLINE int addlabel(struct ltab *lt,char *lab,int addr,int lineno) {
    struct lnode *n;

    if(lt==NULL||lab==NULL) return(-1);
    if(NULL==(n=malloc(sizeof(struct lnode)))) return(-1);
    n->addr=addr;
    n->lineno=lineno;
    n->mac=NULL;
    n->val=-1;
    if(*lab!='.') globals->actlab=n;
    if(NULL==(n->name=createlocname(lab))) { free(n); return(-1); }
    if(NULL!=labsea(lt,n->name)) { free(n->name); free(n); return(-1); }
    n->next=lt->head;
    lt->head=n;
    return(0);
} // addlabel()

/*
 * addfill
 */
/* plusz:
    ha macron belül van és lokális labelre hivatkozik:
    akkor a hivatkozott label típusa
    módosul: kap egy bitet, és plusz 4 chart a végére
 */
INLINE void addfill(struct ltab *lt,FILE *fp,int type,char *lab,int lineno, int addr) {
    struct fill *nf;
    struct macro *mc;

    if(lt==NULL||fp==NULL) return;
    if(NULL==(nf=malloc(sizeof(struct fill)))) return;
    if(globals->macros.on==0) {
        nf->next=lt->fhd;
        nf->filep=ftell(fp);
        nf->addr=addr;
    } else {
        nf->next=globals->macros.first->ltab.fhd;
        nf->filep=globals->macros.pnt;
        nf->addr=-1;
    }
    nf->type=type;
    if(globals->macros.on!=0&&*lab=='.') nf->type|=F_LOCAL;
    nf->lineno=lineno;
    if(NULL==(nf->label=createlocname(lab))) { free(nf); return; }
    if(globals->macros.on!=0&&*lab=='.') {
        char *n;
        if(NULL==(n=malloc(strlen(nf->label)+5))) { free(nf->label); free(nf); return; }
        strcpy(n,nf->label);
        strcat(n,"xxxx");
        free(nf->label);
        nf->label=n;
    }
    if(type==F_MACRO) {
        mc=macseek(nf->label);
        if(mc==NULL) { free(nf->label); free(nf); return; }
        nf->mac=mc;
    } else nf->mac=NULL;
    if(globals->macros.on==0) lt->fhd=nf;
    else globals->macros.first->ltab.fhd=nf;
} // addfill()


/*
 * parseoperand
 * operandust ismer fel,
 * return: a megfelelõ asmstat bejegyzés, ha OK (NULL, hiba)
 * input: az elsõ asmstat bejegyzés erre a parancsra
 */
struct asmstat *parseoperand(char *op,struct asmstat *st) {
    struct asmstat *ret=NULL;
    int nm;

    switch(*op) {
        case '$' :                  // ABSOLUTE
            // abszolút
            switch(strlen(op)) {
                case 3 :
                    // ZP
                    for(nm=st->name;st->name==nm&&st->mode!=ZP;st++);
                    if(st->name==nm) ret=st;
                    break;
                case 5 :
                    // ABS  ZP,X  ZP,Y
                    switch(op[strlen(op)-1]) {
                        case 'x' :
                        case 'X' :
                            if(op[strlen(op)-2]!=',') return(NULL);
                            for(nm=st->name;st->name==nm&&st->mode!=ZPX;st++);
                            if(st->name==nm) ret=st;
                            break;
                        case 'y' :
                        case 'Y' :
                            if(op[strlen(op)-2]!=',') return(NULL);
                            for(nm=st->name;st->name==nm&&st->mode!=ZPY;st++);
                            if(st->name==nm) ret=st;
                            break;
                        default :
                            // ABS, vagy REL ?
                            for(nm=0;branch[nm]!=-1&&branch[nm]!=st->name;nm++);
                            if(branch[nm]==-1) {
                                // ABS
                                for(nm=st->name;st->name==nm&&st->mode!=ABS;st++);
                                if(st->name==nm) ret=st;
                            } else {
                                // REL
                                for(nm=st->name;st->name==nm&&st->mode!=REL;st++);
                                if(st->name==nm) ret=st;
                            }
                            break;
                    }
                    break;
                case 7 :
                    // ABS,X  ABS,Y
                    switch(op[strlen(op)-1]) {
                        case 'x' :
                        case 'X' :
                            if(op[strlen(op)-2]!=',') return(NULL);
                            for(nm=st->name;st->name==nm&&st->mode!=ABSX;st++);
                            if(st->name==nm) ret=st;
                            break;
                        case 'y' :
                        case 'Y' :
                            if(op[strlen(op)-2]!=',') return(NULL);
                            for(nm=st->name;st->name==nm&&st->mode!=ABSY;st++);
                            if(st->name==nm) ret=st;
                            break;
                    }
                    break;
            }
            break;
        case '(' :                  // INDIRECT
            // (ZP,X)  (ZP),Y  (ABS)  (label)
            if(op[1]=='$') {
                // ($ZP,X)  ($ZP),Y  ($ABS)
                switch(op[strlen(op)-2]) {
                    case 'x' :
                    case 'X' :
                        // (ZP,X)
                        if(op[strlen(op)-1]!=')') return(NULL);
                        for(nm=st->name;st->name==nm&&st->mode!=IZPX;st++);
                        if(st->name==nm) ret=st;
                        break;
                    case ',' :
                        // (ZP),Y
                        if(toupper(op[strlen(op)-1])!='Y') return(NULL);
                        for(nm=st->name;st->name==nm&&st->mode!=IZPY;st++);
                        if(st->name==nm) ret=st;
                        break;
                    default :
                        // (ABS)
                        for(nm=st->name;st->name==nm&&st->mode!=IABS;st++);
                        if(st->name==nm) ret=st;
                        break;
                }
            } else {
                // (label) --> IABS
                for(nm=st->name;st->name==nm&&st->mode!=IABS;st++);
                if(st->name==nm) ret=st;
                break;
            }
            break;
        case '#' :                  // BYTE  DATA
            for(nm=st->name;st->name==nm&&st->mode!=BYTE;st++);
            if(st->name==nm) ret=st;
            break;
        case 'a' :                  // ACCU
        case 'A' :
            if(strlen(op)==1) {
                for(nm=st->name;st->name==nm&&st->mode!=ACC;st++);
                if(st->name==nm) ret=st;
                break;
            }
        default :
            // label
            // lehet 1 byte, ha Bxx
            // vagy 2 byte, ha más
            // el kell dönteni, hogy mi ez:
            // label    : ABS
            // label,X  : ABS,X
            // label,Y  : ABS,Y
            if(op[strlen(op)-2]==',') {
                // label,X  label,Y
                switch(op[strlen(op)-1]) {
                    case 'x' :
                    case 'X' :
                        // label,X
                        for(nm=st->name;st->name==nm&&st->mode!=ABSX;st++);
                        if(st->name==nm) ret=st;
                        break;
                    case 'y' :
                    case 'Y' :
                        // label,Y
                        for(nm=st->name;st->name==nm&&st->mode!=ABSY;st++);
                        if(st->name==nm) ret=st;
                        break;
                    default :
                        return(NULL);
                        break;
                }
            } else {
                // label
                for(nm=st->name;st->name==nm&&st->mode!=ABS&&st->mode!=REL;st++);
                if(st->name==nm) ret=st;
            }
            break;
    }
    return(ret);
} // parseoperand()


/* beolvas egy számot (vagy equ értéket)
 * ret: ahol az olvasás végetért
 */
INLINE char *getnumber(struct ltab *lt,char *pre,char *str,char *post, int *val) {
    struct lnode *ln;
    int pl=0,i;
    char *bf,*n;
    char t;

    if(pre!=NULL) {
        if(0!=(istrncmp(str,pre,strlen(pre)))) return(NULL);
        str+=strlen(pre);
    }
    if(post!=NULL) pl=strlen(post);
    if(*str=='$') {
        if(NULL==(bf=malloc(pl+5))) return(NULL);
        strcpy(bf,"$%x");
        if(post!=NULL) strcat(bf,post);
        sscanf(str,bf,val);
        free(bf);
    } else if(isdigit(*str)) {
        // decimal?
        *val=atoi(str);
    } else {
        // equ-value?
        if(post==NULL) t='\0';
        else t=post[0];
        if(NULL==(bf=malloc(strlen(str)+1))) return(NULL);
        for(i=0;str[i]!=t;i++) bf[i]=str[i];
        bf[i]='\0';
        if(NULL==(n=createlocname(bf))) { free(bf); return(NULL); }
        ln=labsea(lt,n);
        if(ln==NULL||ln->val<0) { free(n); free(bf); return(NULL); }
        *val=ln->val;
        free(n);
        free(bf);
    }
    return(str);
}


/*
 * output
 * fp-be kiírja a byteokat, ahol cimkék vannak, oda felvesz egy 'fill'-t
 * ret: 0, vagy error code (errstr index)
 */
int output(struct ltab *lt,struct asmstat *st,char *op,FILE *fp,int addr,int lineno) {
    int o1;

    if(st!=NULL) {
        outbyte(st->code,fp);
        switch(st->mode) {
            case ACC :
                // 0
                break;
            case IMP :
                // 0
                break;
            case BYTE :
                // 1
                switch(op[1]) {
                    case '$' :                      // normal
                        getnumber(lt,"#",op,0,&o1);
                        outbyte(o1,fp);
                        break;
                    case '<' :                      // lo
                        addfill(lt,fp,F_LO,op+2,lineno,addr);
                        outbyte(0,fp);
                        break;
                    case '>' :                      // hi
                        addfill(lt,fp,F_HI,op+2,lineno,addr);
                        outbyte(0,fp);
                        break;
                    case 39 :                       // (') char
                        outbyte(op[2],fp);
                        break;
                    default :
                        if(NULL!=getnumber(lt,"#",op,0,&o1)) outbyte(o1,fp);
                        else return(ER_INVALIDADDRESSING);
                        break;
                }
                break;
            case ZP :
                // 1
                if(NULL!=getnumber(lt,0,op,0,&o1)) outbyte(o1,fp);
                else return(ER_INVALIDADDRESSING);
                break;
            case ZPX :
                // 1
                if(NULL!=getnumber(lt,0,op,",x",&o1)) outbyte(o1,fp);
                else return(ER_INVALIDADDRESSING);
                break;
            case ZPY :
                // 1
                if(NULL!=getnumber(lt,0,op,",y",&o1)) outbyte(o1,fp);
                else return(ER_INVALIDADDRESSING);
                break;
            case IZPX :
                // 1
                if(NULL!=getnumber(lt,"(",op,",x)",&o1)) outbyte(o1,fp);
                else return(ER_INVALIDADDRESSING);
                break;
            case IZPY :
                // 1
                if(NULL!=getnumber(lt,"(",op,"),y",&o1)) outbyte(o1,fp);
                else return(ER_INVALIDADDRESSING);
                break;
            case REL :
                // 1
                addr+=2;
                if(*op=='$') {
                    // count
                    getnumber(lt,0,op,0,&o1);
                    o1=o1-addr;
                    if(o1>127||o1<-128) return(ER_BRANCHOUTOFRANGE);
                    outbyte(o1,fp);
                } else {
                    // label
                    addfill(lt,fp,F_REL,op,lineno,addr);
                    outbyte(0,fp);
                }
                break;
            case ABS :
                // 2
                if(*op=='$') {
                    getnumber(lt,0,op,0,&o1);
                    outbyte(o1&0xff,fp);
                    outbyte(o1>>8,fp);
                } else {
                    // label
                    addfill(lt,fp,F_WORD,op,lineno,addr);
                    outbyte(0,fp);
                    outbyte(0,fp);
                }
                break;
            case ABSX :
            case ABSY :
                if(*op=='$') {
                    getnumber(lt,0,op,0,&o1);
                    outbyte(o1&0xff,fp);
                    outbyte(o1>>8,fp);
                } else {
                    // label
                    o1=strlen(op);
                    op[o1-2]='\0';
                    addfill(lt,fp,F_WORD,op,lineno,addr);
                    op[o1-2]=',';
                    outbyte(0,fp);
                    outbyte(0,fp);
                }
                break;
            case IABS :
                // 2
                if(*(op+1)=='$') {
                    getnumber(lt,"(",op,")",&o1);
                    outbyte(o1&0xff,fp);
                    outbyte(o1>>8,fp);
                } else {
                    // label
                    o1=strlen(op);
                    op[o1-1]='\0';
                    addfill(lt,fp,F_WORD,op+1,lineno,addr);
                    op[o1-1]=')';
                    outbyte(0,fp);
                    outbyte(0,fp);
                }
                break;
        }
    }
    return(0);
} // output()


/*
 * internal mnemonic?
 * -1 : nem
 * >0 : igen, index in imnm[]
 */
INLINE int isinternal(char *mn) {
    int i;

    for(i=0;imnm[i]!=NULL&&istrcmp(imnm[i],mn)!=0;i++);
    if(imnm[i]==NULL) return(-1);
    return(i);
} // isinternal()


int include(struct ltab *lt,char *file, FILE *fp,int addr) {
    FILE *fin;
    struct global *store;
    char *name;
    struct ltab *ltn;
    long in,out;
    struct lnode *l1,*l2;
    int ok=0;

    in=ftell(fp);
    ltn=newtab();
    if(ltn==NULL) {
        fprintf(stderr,"%s: can't create table\n",progname);
        return(-1);
    }
    name=malloc(strlen(file)+1);
    if(name==NULL) {
        fprintf(stderr,"%s: not enough memory",progname);
        return(-1);
    }
    strcpy(name,file);
    fin=fopen(name,"rb");
    if(fin==NULL) {
        o_sym=0;
        droptab(ltn);
        fprintf(stderr,"%s: cannot open file `%s'",progname,name);
        free(name);
        return(-1);
    }
    store=globals;
    globals=calloc(sizeof(struct global),1);
    if(globals==NULL) {
        globals=store;
        free(name);
        fclose(fin);
        fprintf(stderr,"%s: not enough memory",progname);
        o_sym=0;
        droptab(ltn);
        return(-1);
    }
    globals->foffs=store->foffs+(addr-store->startaddr);
    if(0!=assemble(fin,fp,ltn,addr)) {
        fprintf(stderr,"%s: problem assemble `%s'",progname,name);
        free(name);
        fclose(fin);
        o_sym=0;
        droptab(ltn);
        return(-1);
    }
    fseek(fp,0,SEEK_END);
    out=ftell(fp);

    /*
     * Az ltn->head-ból az addr==-1 és name[0]=='/' cimkéknek
     * megkeresni a párját, és azt kitenni az lt-be (plusz a
     * -1-eset is...
     */
    for(l1=ltn->head;l1!=NULL;l1=l1->next) {
        ok=0;
        if(l1->addr<0&&l1->name[0]=='/') {
            if(0!=addlabel(lt,l1->name,-1,l1->lineno)) ok=1;
            for(l2=ltn->head;l2!=NULL;l2=l2->next) if(strcmp(&l1->name[1],l2->name)==0) break;
            if(l2!=NULL) if(0!=addlabel(lt,l2->name,l2->addr,l2->lineno)) ok=1;
        }
        if(ok!=0) {
            fprintf(stderr,"%s: redefined global label `%s'",progname,&l1->name[1]);
            droptab(ltn);
            free(globals);
            free(name);
            globals=store;
            fclose(fin);
            return(-1);
        }
    }

    droptab(ltn);
    free(globals);
    free(name);
    globals=store;
    fclose(fin);
    return(out-in);
}


/*
 * parseline (tabok helyett \0-k kellenek!)
 * return: amennyivel nõtt a PC (-1 hiba!)
 */
int parseline(char *line,int addr,struct ltab *labtab, FILE *fp,int lineno) {
    struct asmstat *st;
    struct lnode *lb=NULL;
    struct macro *mc;
    int op=0,nm,fl,i,local=0;
    char *n;

    if(labtab==NULL||line==NULL) {
        ierr();
        return(-1);
    }
    // remark?
    if(*line=='*') return(0);
    // label?
    if(*line!='\0') {
        // new label
        if(NULL!=strchr(line,'/')) {
            fprintf(stderr,"%s: cannot use '/' in label",progname);
            return(-1);
        }
        if(*line=='.') local=1;
        if(globals->macros.on!=0&&local==0) {
            fprintf(stderr,"%s: cannot use global label in macro",progname);
            return(-1);
        }
        i=-1;
        if(globals->macros.on!=0) {
            i=addlabel(&globals->macros.first->ltab,line,globals->macros.pnt,lineno);
        } else i=addlabel(labtab,line,addr,lineno);
        if(0!=i) {
            fprintf(stderr,"%s: can't create label: %s ",progname,line);
            return(-1);
        }
        lb=labtab->head;        // addlabel() az elejére fûz be
        line+=strlen(line);
    }
    line++;
    // mnemonic?
    if(*line!='\0') {
            // internal mnemonic?
            if(0<=(nm=isinternal(line))) {
                line+=4;
                switch(nm) {
                    case DAT :
                        if(line[0]=='#') line++;
                        switch(line[0]) {
                            case '$' :                      // normal
                                getnumber(labtab,0,line,0,&nm);
                                outbyte(nm,fp);
                                op=1;
                                break;
                            case '<' :                      // lo
                                addfill(labtab,fp,F_LO,line+1,lineno,addr);
                                outbyte(0,fp);
                                op=1;
                                break;
                            case '>' :                      // hi
                                addfill(labtab,fp,F_HI,line+1,lineno,addr);
                                outbyte(0,fp);
                                op=1;
                                break;
                            case 39 :                       // (') char
                                outbyte(line[1],fp);
                                op=1;
                                break;
                            default :
                                fprintf(stderr,"%s: syntax error",progname);
                                return(-1);
                                break;
                        }
                        break;
                    case BSS :
                        if(line[0]=='#') line++;
                        fl=nm=0;
                        sscanf(line,"$%x,$%x",&nm,&fl);
                        op=nm;
                        if(nm<=0||nm>=65535||fl<0||fl>255) {
                            fprintf(stderr,"%s: invalid BSS parameter: `%s'",progname,line);
                            return(-1);
                        }
                        for(i=0;i<nm;i++) outbyte(fl,fp);
                        break;
                    case STR :
                        for(op=0;line[op]!='\0';op++) {
                            nm=toupper(line[op]);
                            if(nm>=64&&nm<=90) nm-=64;
                            else if(nm<32) nm=32;
                            else if(nm>90) nm-=64;
                            outbyte(nm,fp);
                        }
                        break;
                    case MAC :              // macro def
                        if(lb==NULL) {
                            fprintf(stderr,"%s: macro declaration without label",progname);
                            return(-1);
                        }
                        if(globals->macros.on!=0) {
                            fprintf(stderr,"%s: cannot declare macro in a macro body",progname);
                            return(-1);
                        }
                        if(local!=0) {
                            fprintf(stderr,"%s: cannot declare local macro",progname);
                            return(-1);
                        }
                        if(0!=(macstart(lb))) {
                            fprintf(stderr,"%s: out of memory",progname);
                            return(-1);
                        }
                        op=0;
                        break;
                    case END :              // macro end-def
                        macend();
                        op=0;
                        break;
                    case USE :              // use macro
                        addfill(labtab,fp,F_MACRO,line,lineno,addr);
                        op=macins(line,fp);
                        break;
                    case EQU :
                        if(lb==NULL) {
                            fprintf(stderr,"%s: value definition without label",progname);
                            return(-1);
                        }
                        if(globals->macros.on!=0) {
                            fprintf(stderr,"%s: cannot define value in macro",progname);
                            return(-1);
                        }
                        getnumber(labtab,0,line,0,&nm);
                        if(nm<0||nm>0xffff) {
                            fprintf(stderr,"%s: value out of range `%s=$%x'",progname,lb->name,nm);
                            return(-1);
                        }
                        lb->val=nm;
                        break;
                    case INL :
                        if(0>(op=include(labtab,line,fp,addr))) return(-1);
                        break;
                    case XDF :
                        if(NULL==(n=malloc(strlen(line)+2))) {
                            fprintf(stderr,"%s: not enough memory",progname);
                            return(-1);
                        }
                        strcpy(n,"/");
                        strcat(n,line);
                        addlabel(labtab,n,-1,lineno);
                        free(n);
                        break;
                    default :
                        fprintf(stderr,"%s: invalid internal mnemonic: `%s'",progname,(line-4));
                        return(-1);
                        break;
                }
                if(op<0) ierr();
                return(op);
            }
            st=lookup(line);                            // maybe 7501 ?
            if(st==NULL) {
                if((mc=macseek(line))!=NULL) {          // macro
                    addfill(labtab,fp,F_MACRO,line,lineno,addr);
                    op=macins(line,fp);
                    if(op<0) ierr();
                    return(op);
                } else {
                    fprintf(stderr,"%s: illegal mnemonic: `%s'",progname,line);
                    return(-1);
                }
            }
    } else return(0);
    line+=4;
    // operand
    if(*line!='\0'&&*line!=';'&&*line!='*') {
        // operand ( '\0'-ig )
        st=parseoperand(line,st);
        if(st==NULL) {
            fprintf(stderr,"%s: bad operand `%s'",progname,line);
            return(-1);
        }
        op=st->len;
    } else {
        op=1;
        for(nm=st->name;st->name==nm&&st->mode!=IMP&&st->mode!=ACC;st++);
        if(st->name!=nm) {
            fprintf(stderr,"%s: missing operand",progname);
            return(-1);
        }
    }
    if(0!=(nm=output(labtab,st,line,fp,addr,lineno))) {
        fprintf(stderr,"%s: %s",progname,errstr[nm]);
        return(-1);
    }
    if(op<0) ierr();
    return(op);
} // parseline()


#define SPACE   0
#define CHAR    1
#define UNDEF   -1

/*
 * elsõ átmenet (symbol tábla építése + output gerinc!)
 * ret: 0 -OK ; más- hiba sorszáma
 */
int pass1(struct ltab *lt,FILE *fin,FILE *fp,int addr) {
static char bf[500];
static char line[500];
    int i,j,lineno=1,pl,len;
    int prev,now;

    lineno=1;
    if(lt==NULL||fin==NULL) return(-1);

    lineno=2;

    while(NULL!=fgets(bf,500,fin)) {
        // long?
        if(strlen(bf)>255) {
            fprintf(stderr,"%s: line buffer overflow",progname);
            return(lineno);
        }
        // empty?
        if(bf[0]!='\n') {
            // tab,' ' --> 0
            prev=now=UNDEF;
            for(len=strlen(bf),i=0,j=0;i<len;i++) {
                if(prev==SPACE&&(bf[i]==';'||bf[i]=='*')) break;
                if(bf[i]=='\n'||bf[i]=='\t'||bf[i]==' ') now=SPACE;
                else if(isprint(bf[i])) now=CHAR;
                else now=UNDEF;
                if(now==SPACE&&prev!=SPACE) line[j++]='\0';
                if(now==CHAR) line[j++]=bf[i];
                prev=now;
            }
            line[j]='\0';
            line[j+1]='\0';
            line[j+2]='\0';
            pl=parseline(line,addr,lt,fp,lineno);
            if(pl==-1) return(lineno);
            if(globals->macros.on==0) addr+=pl;
        }
        lineno++;
    }
    return(0);
} // pass1()


/*
 * pass2
 * fill out üres labelhelyek!
 */
int pass2(struct ltab *lt,FILE *fin,FILE *fp,char **err,int offs) {
static char pluserr[80];
    struct macro *mc;
    struct fill *fl,*f;
    struct lnode *ln;
    int pos,r,v,type,l;
    struct ltab lt2;
    long fbase;
    char usestr[5];
    char *nn;

    *err=pluserr;
    pluserr[0]='\0';
    for(fl=lt->fhd;fl!=NULL;fl=fl->next) {
        fseek(fp,fl->filep+offs,SEEK_SET);
        type=fl->type&~F_LOCAL;
        if(type==F_MACRO) {
            mc=fl->mac;
            if(mc==NULL) {
                sprintf(pluserr,"`%s' at line %d",fl->label,fl->lineno);
                return(ER_MACRONOTFOUND);
            }
            if(mc->usecount>65535) {
                sprintf(pluserr,"`%s' at line %d",fl->label,fl->lineno);
                return(ER_TOOMANYMACRO);
            }
            sprintf(usestr,"%04x",mc->usecount);
            /* a macro labeljeit kivinni globálisba az usecount-tal ($xxxx)
             */
            fbase=ftell(fp);
            for(ln=mc->ltab.head;ln!=NULL;ln=ln->next) {
                nn=malloc(strlen(ln->name)+5);
                if(nn==NULL) {
                    pluserr[0]='\0';
                    return(ER_NOMEM);
                }
                strcpy(nn,ln->name);
                strcat(nn,usestr);
                if(0!=addlabel(lt,nn,globals->startaddr+fbase+ln->addr-2,ln->lineno)) {
                    free(nn);
                    pluserr[0]='\0';
                    return(ER_NOMEM);
                }
                free(nn);
            }
            /*  a macro fill-jeit módosítani, az utosó 4 char-jukat
             *  update-lni a mostani usecount-ra, ha lokális hivatkozások
             */
            for(f=mc->ltab.fhd;f!=NULL;f=f->next) {
                if((f->type&F_LOCAL)!=0) {
                    l=strlen(f->label)-1;
                    f->label[l--]=usestr[3];
                    f->label[l--]=usestr[2];
                    f->label[l--]=usestr[1];
                    f->label[l]=usestr[0];
                }
            }
            lt2.head=lt->head;
            lt2.fhd=mc->ltab.fhd;
            mc->usecount++;
            if(mc->ltab.fhd!=NULL) if(0!=(r=pass2(&lt2,fin,fp,err,fbase))) return(r);
            continue;
        }
        ln=labsea(lt,fl->label);
        if(ln!=NULL) {
            switch(type) {
                case F_WORD :
                    if(ln->val<0) v=ln->addr;
                    else v=ln->val;
                    outbyte(v&0xff,fp);
                    outbyte(v>>8,fp);
                    break;
                case F_REL :
                    if(ln->val<0) {
                        pos=ln->addr-fl->addr;
                    } else {
                        if(ln->val<256) pos=(signed char)ln->val;
                        else pos=200;   // overflow
                    }
                    if(pos<-128||pos>127) {
                        sprintf(pluserr,"from line %d to line %d",fl->lineno,ln->lineno);
                        return(ER_BRANCHOUTOFRANGE);
                    }
                    outbyte(pos,fp);
                    break;
                case F_HI :
                    if(ln->val>=0) {
                        sprintf(pluserr," `%s'",ln->name);
                        return(ER_NOADDRESS);
                    }
                    pos=ln->addr>>8;
                    outbyte(pos,fp);
                    break;
                case F_LO :
                    if(ln->val>=0) {
                        sprintf(pluserr," `%s'",ln->name);
                        return(ER_NOADDRESS);
                    }
                    pos=ln->addr&0xff;
                    outbyte(pos,fp);
                    break;
            }
        } else {
            sprintf(pluserr,"`%s' at line %d",fl->label,fl->lineno);
            return(ER_LABELNOTFOUND);
        }
    }
    return(0);
} // pass2()


int assemble(FILE *fin,FILE *fp,struct ltab *lt,int startaddr) {
    int ok=0,ret;
    char *err;

    globals->startaddr=startaddr;
    ret=pass1(lt,fin,fp,startaddr);
    if(ret==0) {
        ret=pass2(lt,fin,fp,&err,0);
        if(ret!=0) fprintf(stderr,"%s: %s %s (pass2)\n",progname,errstr[ret],err);
        else ok=1;
    } else if(ret>0) {
        fprintf(stderr," at line %d (pass1)\n",ret);
    } else fprintf(stderr,"%s: pass1: unknown error\n",progname);

    /*
     * A reloc-table a fill-táblázatból készíthetõ, kiírható
     * az output végére
     */

    if(ok==0) return(-1);
    return(0);
}


/*
 * main
 */
int main(int argc,char **argv) {
static char bf[21];
    struct ltab *lt;
    char *outfile;
    FILE *fp,*fin=stdin;
    int i,ok=0,addr=0;

    if(argc<3) {
        printf("Usage: %s infile outfile [options]\n",argv[0]);
        printf("  -s   Print symbol table after assemble\n");
        exit(0);
    }
    progname=argv[0];
    outfile=argv[2];
    o_sym=0;
    if(argc>3) {
        for(i=3;i<argc;i++) {
            if(argv[i][0]!='-') continue;
            switch(argv[i][1]) {
                case 's' :
                    o_sym=1;
                    break;
                default:
                    fprintf(stderr,"%s: unknown option `%s'\n",argv[0],argv[i]);
                    break;
            }
        }
    }
    lt=newtab();
    if(lt==NULL) {
        fprintf(stderr,"%s: can't create table\n",argv[0]);
        exit(0);
    }
    if(NULL==(fin=fopen(argv[1],"rb"))) {
        fprintf(stderr,"%s: can't open %s\n",argv[0],argv[1]);
        exit(0);
    }
    fgets(bf,20,fin);
    getnumber(lt,0,bf,0,&addr);
    if(addr<=0||addr>0xfffe) {
        fprintf(stderr,"%s: invalid start address ($%x)\n",progname,addr);
        exit(0);
    }

    fp=fopen(outfile,"wb");
    if(fp!=NULL) {
        // write start address
        fputc((addr&0xff),fp);
        fputc((addr>>8),fp);
        globals=calloc(sizeof(struct global),1);
        if(globals!=NULL) {
            globals->foffs=-2;
            if(0==assemble(fin,fp,lt,addr)) ok=1;
        }

        /*
         * Az lt->head-ból az addr==-1 és name[0]=='/' cimkéket,
         * amennyiben van valódi megfelelõjük, globálissá tenni,
         * pl. a file végére kiírni, ha az object file.
         */

        fclose(fp);
    } else fprintf(stderr,"%s: can't open %s\n",argv[0],outfile);

    if(fin!=stdin) fclose(fin);
    if(ok==0) o_sym=0;
    if(globals!=NULL) droptab(lt);
    if(globals!=NULL) free(globals);
}
