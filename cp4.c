/* :ts=4                            cp4.c
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

#include <proto/exec.h>

#ifdef __GCC__
    #include <sys/signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "macros.h"
#include "common.h"
#include "cp4_defs.h"
#include "cp4.h"
#include "cp4_cpu.h"
#include "cp4_mac.h"
#include "cp4_rom.h"
#include "ted.h"
#include "licence.h"
#include "prefs.h"
#include "p4req.h"
#include "cp4_loc.h"
#include "soft_iec.h"

unsigned long __stack=16384;

extern unsigned int REGARGS asmexecute( REG(d0,int from) );

// Globals from dbg.c
extern int DefDisMode;
extern int DefFollow;
extern int DefDbgTop;
extern int DefDbgLeft;
extern int DefPrfTop;
extern int DefPrfLeft;
extern int DefMonTop;
extern int DefMonLeft;
extern int DefMonitor;
extern int DefStkLeft;
extern int DefStkTop;

// Globals from 7501.asm
extern long StatTab;
extern long CodeTab;

// Globals from ted.c
extern struct IntuitionBase *IntuitionBase;
extern unsigned char chalf;     // $51
extern unsigned char cfull;     // $31
extern unsigned long statfpsnum,statfpssum;

// Globals for execute
int opt_direct=0;                   // 1-direct mode
char *opt_palettefile=NULL;         // name of palette file
char *opt_p4dir;
int opt_xpk;                        // 0-nem, 1-igen
int opt_xpkeff;                     // hatásfok: 0-100%
char *opt_xpktype;                  // xpk subype (4 char + \0)
int opt_listwidth;
int opt_memfrm;
int opt_memsiz;
int opt_regout;
int opt_fulreg;
int opt_noexe;
int opt_romstr;
int opt_nosound;
int opt_limit;
int opt_percent;
int opt_withdata;
int opt_iec;
int opt_realtime;
char *opt_c2p;
char *opt_keymap;           // filename with path
unsigned long opt_scrmode;
unsigned long opt_overscan;
unsigned long opt_twoscr;
int opt_sid;                    // 1-enable 0-disable
int opt_sidtype;                // 0-semmi, 1-playsid, 2-6581sid
int opt_onscreendisplay;        // led stb. (0-nem)
int opt_drive08;                // 0-semmi 1-iec 2-soft
int opt_drive09;
int opt_drive10;
int opt_drive11;
int opt_noborder;               // 1-NoSideBorder 0-normal
#define OP_NO       "NO"
#define OP_YES      "YES"


// Globals for c+4
char *defc2p="windowcarddelta.c2p";
int *countpointer=NULL;         // utasításgyakoriságtáblázat
int *brcntpnt;
unsigned int done;
u_word pre_pc;                  // a disasm-hez
extern u_byte *hiram;           // TED állítja, read from >$8000? RAM/ROM?
extern u_byte *myram;           // TED-tõl
u_byte *myrom;                  // saját
int lastwrite;                  // az utolsó memwrite helye
int cyc=0;                      // az aktuális utasítás végrehajtási ideje
int brk=0;                      // break volt-e a megszakítás?
int irq=0;                      // külsõ IRQ (+BRK)
// 64k ram&rom (aligned 64k boundary)
// plusz 131072 byte jump table + junk
u_byte p4ramfix[MAXRAM*3+132000];
u_byte *p4ram;
u_byte *p4rom;
u_byte roms[8][MAXROM];         // romok
u_byte *romsp0=roms[0];
u_byte *romsp1=roms[1];
u_byte *romsp2=roms[2];
u_byte *romsp3=roms[3];
u_byte *romsp4=roms[4];
u_byte *romsp5=roms[5];
u_byte *romsp6=roms[6];
u_byte *romsp7=roms[7];
u_byte r_a=0;                   // accumlator
u_byte r_x=0;                   // x register
u_byte r_y=0;                   // y register
u_byte r_sp=0xff;               // stack pointer
u_word r_pc=0;                  // program counter
u_byte r_p;                     // flagek
#define flag_n 128              // negative
#define flag_v 64               // overflow
#define flag__ 32               // nem használt
#define flag_b 16               // break
#define flag_d 8                // decimal
#define flag_i 4                // interrupt
#define flag_z 2                // zero
#define flag_c 1                // carry

char *ted_names[]={
    "Cnt1LO      ","Cnt1HI      ","Cnt2LO      ","Cnt2HI      ","Cnt3LO      ",
    "Cnt3HI      ","Gfx1        ","Gfx2        ","KeyRd       ","IRQFlag     ",
    "IRQEnable   ","Raster      ","CurPos1     ","CurPos2     ","Snd1Freq1   ",
    "Snd2Freq1   ","Snd2Freq2   ","SndCtrl     ","Gfx3        ","ChrGen      ",
    "VideoMatrix ","Color0      ","Color1      ","Color2      ","Color3      ",
    "Color4      ","1stCharAddr1","1stCharAddr1","ActRas1     ","ActRas2     ",
    "HRas        ","Flash       ",NULL
};


// protos
void errout(char *str);


//--------------------------------------------------------------------------
//----------------------C+4-FUNCTIONS---------------------------------------
//--------------------------------------------------------------------------


INLINE u_byte memread(u_word addr) {        // general read
    if(addr<0x8000) return(p4ram[addr]);
    return(hiram[addr]);
} // memread

INLINE u_byte memreadzp(u_word addr) {      // read from zero page (tuti RAM)
    return(p4ram[addr]);
} // memreadzp

INLINE u_byte memreadf(s_word addr) {       // opcode + operand (ROM/RAM, noREGS)
    return((addr<0 ? hiram[(u_word)addr] : p4ram[addr]));
} // memreadf

INLINE u_word memreadword(u_word addr) {
    return(((u_word)memread(addr+1)<<8)+memread(addr));
} // memreadword

INLINE u_byte memwrite(u_word addr,u_byte data) {   // csak RAMba!
    lastwrite=addr;
    if(addr<0xfd00) { p4ram[addr]=data; return(data); }
    if(addr<0xff40) { hiram[addr]=data; return(data); }
    p4ram[addr]=data;
    return(data);
} // memwrite

INLINE void setflag_n(u_byte r) {
    r&=flag_n;
    r_p&=~flag_n;
    r_p|=r;
} // setflag_n
INLINE void setflag_z(u_byte r) {
    if(r!=0) r_p&=~flag_z;
    else r_p|=flag_z;
} // setflag_z
INLINE void setflag_c(int e) {
    e>>=8;
    e&=1;
    r_p&=~flag_c;
    r_p|=e;
} // setflag_c
#define setflag_zn  setflag_nz
INLINE void setflag_nz(u_byte r) {
    if(r!=0) {
        r_p&=~flag_z;
        r&=flag_n;
        r_p&=~flag_n;
        r_p|=r;
    } else {
        r_p|=flag_z;
        r_p&=~flag_n;
    }
} // setflag_nz

INLINE void push(u_byte a) {
    // verembe teszi 'a'-t, csökkenti az sp-t (1 byte)
    memwrite(r_sp|0x100,a);
    --r_sp;
} // push()

INLINE u_byte pop(void) {
    // kivesz a veremrõl 1 byte-ot
    ++r_sp;
    return(memreadzp(0x100|r_sp));
} // pop()

INLINE void push2(u_word a) {
    u_byte hi,lo;

    // verembe teszi 'a'-t, csökkenti az sp-t (2 byte)
    hi=a>>8;
    lo=a&0xff;
    memwrite(r_sp|0x100,hi);
    --r_sp;
    memwrite(r_sp|0x100,lo);
    --r_sp;
} // push2()

INLINE u_word pop2(void) {
    u_byte hi,lo;
    u_word a;

    // kivesz a veremrõl 2 byte-ot
    ++r_sp;
    lo=memreadzp(0x100|r_sp);
    ++r_sp;
    hi=memreadzp(0x100|r_sp);
    a=hi;
    a<<=8;
    a+=lo;
    return(a);
} // pop2()

INLINE void gethilo(u_byte a, int *ah, int *al) {
    *ah=a>>4;
    *al=a&0xf;
} // gethilo()

INLINE int makedec(int hi,int lo) {
    hi&=0x0f;
    lo&=0x0f;
    hi<<=4;
    return(hi|lo);
} // makedec()

INLINE int decadd(u_byte a,u_byte b,u_byte c) {
    int ah,al,bh,bl;
    int eh=0,el=0,e,pl=0;

    gethilo(a,&ah,&al);
    gethilo(b,&bh,&bl);
    el=al+bl+c;
    if(el>9) { el-=10; ++ah; }
    eh=ah+bh;
    if(eh>9) {
        pl=0x100;
        eh-=10;
        r_p|=flag_c;
        r_p|=flag_v;
    } else {
        r_p&=~flag_c;
        r_p&=~flag_v;
    }
    e=makedec(eh,el);
    return(e+pl);
} // decadd()

INLINE int decsub(u_byte a, u_byte b) {
    int ah,al,bh,bl;
    int eh=0,el=0,e;

    gethilo(a,&ah,&al);
    gethilo(b,&bh,&bl);
    el=al-bl;
    if(el<0) { el+=10; ah--; }
    eh=ah-bh;
    if(eh<0) {
        eh+=10;
        r_p&=~flag_c;
        r_p|=flag_v;
    } else {
        r_p&=~flag_v;
        r_p|=flag_c;
    }
    e=makedec(eh,el);
    return(e);
} // decsub()


/*
 * IRQ handler
 */
INLINE void irqh(void) {
    u_byte hi,lo;

    irq=0;                                      // irq törlése
    if(brk==0&&(r_p&flag_i)!=0) return;         // le van tiltva...
    if(brk==0) r_p&=~flag_b;
    brk=0;
    push2(r_pc);
    push(r_p);
    lo=hiram[0xfffe];
    hi=hiram[0xffff];
    r_pc=(hi<<8)+lo;
} // irqh()


//--------------------------------------------------------------------------
//----------------------EMULATOR-FUNCTIONS----------------------------------
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
void errout(char *str) {        // error message
//--------------------------------------------------------------------------
    static char *prgname=NULL;

    if(prgname==NULL) prgname=str;
    else p4req1(NULL,P4_ERROR,str);
}


//--------------------------------------------------------------------------
void memout(int from, int to) {
//--------------------------------------------------------------------------
static char line[250];
static char buf[100];
    int i,j;
    u_byte ch;

    printf("\n>M %04X %04X\n",from,to);
    for(i=from;i<=to;i+=8) {
        sprintf(buf,",%04X:  ",i);
        strcpy(line,buf);
        for(j=0;j<8;j++) {
            sprintf(buf,"%02X ",memread((u_word)(i+j)));
            strcat(line,buf);
        }
        sprintf(buf,"  :");
        strcat(line,buf);
        for(j=0;j<8;j++) {
            ch=memread((u_word)(i+j));
            if(ch<32) ch='.';
            if(ch>127) ch='.';
            sprintf(buf,"%c",ch);
            strcat(line,buf);
        }
        printf("%s\n",line);
    }
} // memout()


//--------------------------------------------------------------------------
void regout(void) {
//--------------------------------------------------------------------------
static char binflags[10];
    printf("\nMONITOR\n");
    printf("   PC   SR  AC  XR  YR  SP    NV-BDIZC\n");
    binflags[0]=((r_p&flag_n)==0 ? '0' : '1' );
    binflags[1]=((r_p&flag_v)==0 ? '0' : '1' );
    binflags[2]=((r_p&flag__)==0 ? '0' : '1' );
    binflags[3]=((r_p&flag_b)==0 ? '0' : '1' );
    binflags[4]=((r_p&flag_d)==0 ? '0' : '1' );
    binflags[5]=((r_p&flag_i)==0 ? '0' : '1' );
    binflags[6]=((r_p&flag_z)==0 ? '0' : '1' );
    binflags[7]=((r_p&flag_c)==0 ? '0' : '1' );
    binflags[8]='\0';
    printf("; %04X  %02X  %02X  %02X  %02X  %02X    %s\n",r_pc,r_p,r_a,r_x,r_y,r_sp,binflags);
} // regout()


//--------------------------------------------------------------------------
int disasm(void) {
//--------------------------------------------------------------------------
static char binflags[]="N_V_Z_C_";
static int pre_addr=0;
static struct p4st *pre_st=NULL;
static char *dform[]={
    "ERROR (%x %s) ",
    "%02X\t\t%s ",
    "%02X\t\t%s ",
    "%02X\t%s ",
    0L
};
    int name,i=0;
    struct p4st *st;
    int addr,op;

    st=&stats[memread(pre_pc)];
    // getname
    name=st->name;

    // branch space
    if(pre_st!=NULL) if((pre_addr+pre_st->len)!=pre_pc) printf("\n");

    // illegal code?
    if(name>=XXX) {
        for(i=0;i<st->len;i++) {
            if(i==0) printf(".%04X\t",pre_pc+i);
            else printf(" \n.%04X\t",pre_pc+i);
            printf(dform[1],memread((u_word)(pre_pc+i)),statements[XXX]);
        }
        if(opt_fulreg!=0) {
            // print reginfo
            printf("\t\t");
            binflags[1]=((r_p&flag_n)==0 ? '-' : '1' );
            binflags[3]=((r_p&flag_v)==0 ? '-' : '1' );
            binflags[5]=((r_p&flag_z)==0 ? '-' : '1' );
            binflags[7]=((r_p&flag_c)==0 ? '-' : '1' );
            printf("\tAC:%02X  XR:%02X  YR:%02X  SP:%02X  %s\n",r_a,r_x,r_y,r_sp,binflags);
        } else printf("\n");
        pre_st=st;
        pre_addr=pre_pc;
        return(st->len);
    }

    pre_st=st;
    pre_addr=pre_pc;

    // normal code
    printf(".%04X\t",pre_pc);
    switch(st->len) {
        case 3 :
            printf("%02X ",memread((u_word)(pre_pc+i++)));
        case 2 :
            printf("%02X ",memread((u_word)(pre_pc+i++)));
    }
    printf(dform[st->len],memread((u_word)(pre_pc+i)),statements[name]);
    ++pre_pc;
    addr=-1;
    op=-1;
    switch(st->addr) {
        case ILL :
            printf("\t\t");
            break;
        case BYTE :
            printf("#$%02X\t",memread(pre_pc));
            break;
        case ABS :
            addr=(memread(pre_pc+1)<<8)+memread(pre_pc);
            op=memread(addr);
            printf("$%04X   :$%02X=$%04X",addr,op,addr);
            break;
        case ZP :
            addr=memread(pre_pc);
            op=memread(addr);
            printf("$%02X     :$%02X=$%04X",addr,op,addr);
            break;
        case ACC :
            printf("\t\t");
            break;
        case IMP :
            printf("\t\t");
            break;
        case IZPX :
            addr=memread(pre_pc);
            addr+=r_x;
            addr=(memread(addr+1)<<8)+memread(addr);
            op=memread(addr);
            printf("($%02X,X) :$%02X=$%04X",memread(pre_pc),op,addr);
            break;
        case IZPY :
            addr=memread(pre_pc);
            addr=(memread(addr+1)<<8)+memread(addr);
            addr+=r_y;
            op=memread(addr);
            printf("($%02X),Y :$%02X=$%04X",memread(pre_pc),op,addr);
            break;
        case ZPX :
            addr=memread(pre_pc);
            addr+=r_x;
            op=memread(addr);
            printf("$%02X,X   :$%02X=$%04X",memread(pre_pc),op,addr);
            break;
        case ABSX :
            addr=(memread(pre_pc+1)<<8)+memread(pre_pc);
            addr+=r_x;
            op=memread(addr);
            printf("$%04X,X :$%02X=$%04X",(memread(pre_pc+1)<<8)+memread(pre_pc),op,addr);
            break;
        case ABSY :
            addr=(memread(pre_pc+1)<<8)+memread(pre_pc);
            addr+=r_y;
            op=memread(addr);
            printf("$%04X,Y :$%02X=$%04X",(memread(pre_pc+1)<<8)+memread(pre_pc),op,addr);
            break;
        case REL :
            printf("$%04X\t",(pre_pc+1)+((s_byte)memread(pre_pc)));
            break;
        case IABS :
            printf("($%04X)\t",(memread(pre_pc+1)<<8)+memread(pre_pc));
            break;
        case ZPY :
            addr=memread(pre_pc);
            addr+=r_y;
            op=memread(addr);
            printf("$%02X,Y    :$%02X=$%04X",memread(pre_pc),op,addr);
            break;
    }
    if(opt_fulreg!=0) {
        // print reginfo
        binflags[1]=((r_p&flag_n)==0 ? '-' : '1' );
        binflags[3]=((r_p&flag_v)==0 ? '-' : '1' );
        binflags[5]=((r_p&flag_z)==0 ? '-' : '1' );
        binflags[7]=((r_p&flag_c)==0 ? '-' : '1' );
        printf("\tAC:%02X  XR:%02X  YR:%02X  SP:%02X  %s\n",r_a,r_x,r_y,r_sp,binflags);
    } else printf("\n");
    return(st->len);
} // disasm()


//--------------------------------------------------------------------------
char *linedisasm(u_word adr,u_word *next,int *operand,int mode) {
// egy sort disassemblál - mode 0-legális kódok, ~0-illegális kódok is
// ret: a sor (NOFREE!!)
//      a következõ utasítás címe ('next')
//      operand - az operandus címe, vagy -1, ha nincs
//--------------------------------------------------------------------------
static char line[16];
static char buf[12];
    int name;
    struct p4st *st;
    int addr;

    st=&stats[memread(adr)];
    // getname
    name=st->name;

    // illegal code?
    if(name>=XXX&&mode==0) {
        strcpy(line," ");
        strcat(line,statements[XXX]);
        if(opt_withdata!=0) sprintf(buf," [$%02X]",memread(adr)); 
        else buf[0]='\0';
        strcat(line,buf);
        *next=adr+1;
        *operand=-1;
        return(line);
    }

    // normal code
    *next=adr+st->len;
    if(name<XXX) sprintf(line," %s ",statements[name]);
    else sprintf(line,"!%s ",illstatements[name-100]);
    adr++;
    switch(st->addr) {
        case BYTE :
            sprintf(buf,"#$%02X",memread(adr));
            *operand=-1;
            break;
        case ABS :
            addr=(memread(adr+1)<<8)+memread(adr);
            sprintf(buf," $%04X",addr);
            if(name!=JMP&&name!=JSR) *operand=addr;
            else *operand=-1;
            break;
        case ZP :
            addr=memread(adr);
            sprintf(buf," $%02X",addr);
            *operand=addr;
            break;
        case ACC :
            buf[0]='\0';
            *operand=-1;
            break;
        case IMP :
            buf[0]='\0';
            *operand=-1;
            break;
        case IZPX :
            addr=memread(adr);
            addr+=r_x;
            addr=(memread(addr+1)<<8)+memread(addr);
            sprintf(buf,"($%02X,X)",memread(adr));
            *operand=addr;
            break;
        case IZPY :
            addr=memread(adr);
            addr=(memread(addr+1)<<8)+memread(addr);
            addr+=r_y;
            sprintf(buf,"($%02X),Y",memread(adr));
            *operand=addr;
            break;
        case ZPX :
            addr=memread(adr);
            addr+=r_x;
            sprintf(buf," $%02X,X",memread(adr));
            *operand=addr;
            break;
        case ABSX :
            addr=(memread(adr+1)<<8)+memread(adr);
            addr+=r_x;
            sprintf(buf," $%04X,X",(memread(adr+1)<<8)+memread(adr));
            *operand=addr;
            break;
        case ABSY :
            addr=(memread(adr+1)<<8)+memread(adr);
            addr+=r_y;
            sprintf(buf," $%04X,Y",(memread(adr+1)<<8)+memread(adr));
            *operand=addr;
            break;
        case REL :
            sprintf(buf," $%04X",(adr+1)+((s_byte)memread(adr)));
            *operand=-1;
            break;
        case IABS :
            sprintf(buf,"($%04X)",(memread(adr+1)<<8)+memread(adr));
            *operand=-1;
            break;
        case ZPY :
            addr=memread(adr);
            addr+=r_y;
            sprintf(buf," $%02X,Y",memread(adr));
            *operand=addr;
            break;
        case NONE :
            buf[0]='\0';
            *operand=-1;
            break;
        default :
            // HIBA!!
            sprintf(line,GetStr(MSG_003A));
            break;
    }
    strcat(line,buf);
    return(line);
} // linedisasm()


//--------------------------------------------------------------------------
// megadja a köv. utasítás címét, csak legális utasításokat!!!
int disgetlen(int pc,int mode) {
//--------------------------------------------------------------------------
    struct p4st *st;
    st=&stats[memread(pc)];
    if(st->name>=XXX&&mode==0) return(1);
    else return(st->len);
} // disgetlen


//--------------------------------------------------------------------------
// ret: hová kerül a vezérlés, a megadott címrõl, az aktuális flagekkel
// flag: 0-trace, köv ut.
//       1-step, nem megy bele a jsr-ekbe
u_word wheretogo(u_word adr,int flag) {
//--------------------------------------------------------------------------
    u_word a=0;
    struct p4st *st;

    st=&stats[memread(adr)];
    if(st->jump!=0) {
        switch(st->name) {
        // IRQ
            case BRK :
                a=memreadword(0xfffe);
                break;
        // Jump
            case JSR :
                if(flag==0) a=memreadword(adr+1);
                else a=adr+3;
                break;
            case JMP :  // ABS, IABS
                if(st->addr==ABS) a=memreadword(adr+1);
                else a=memreadword(memreadword(adr+1));
                break;
        // Stack
            case RTS :
                a=1+((memreadzp(0x100|(r_sp+2))<<8)+(memreadzp(0x100|(r_sp+1))));
                break;
            case RTI :
                a=((((u_word)memreadzp(0x100|(r_sp+3)))<<8)+(memreadzp(0x100|(r_sp+2))));
                break;
        // Condition
            case BPL :
                if((r_p&flag_n)==0) {
                    a=(adr+2)+(s_byte)memread(adr+1);
                } else a=adr+2;
                break;
            case BMI :
                if((r_p&flag_n)!=0) {
                    a=(adr+2)+(s_byte)memread(adr+1);
                } else a=adr+2;
                break;
            case BVC :
                if((r_p&flag_v)==0) {
                    a=(adr+2)+(s_byte)memread(adr+1);
                } else a=adr+2;
                break;
            case BVS :
                if((r_p&flag_v)!=0) {
                    a=(adr+2)+(s_byte)memread(adr+1);
                } else a=adr+2;
                break;
            case BCC :
                if((r_p&flag_c)==0) {
                    a=(adr+2)+(s_byte)memread(adr+1);
                } else a=adr+2;
                break;
            case BCS :
                if((r_p&flag_c)!=0) {
                    a=(adr+2)+(s_byte)memread(adr+1);
                } else a=adr+2;
                break;
            case BNE :
                if((r_p&flag_z)==0) {
                    a=(adr+2)+(s_byte)memread(adr+1);
                } else a=adr+2;
                break;
            case BEQ :
                if((r_p&flag_z)!=0) {
                    a=(adr+2)+(s_byte)memread(adr+1);
                } else a=adr+2;
                break;
            default :
                p4req1(NULL,P4_ERROR,GetStr(MSG_003B));
                break;
        }
    } else a=adr+st->len;
    return(a);
}

//--------------------------------------------------------------------------
void execute(unsigned int from) {
//--------------------------------------------------------------------------
static char nbuf[20];
static unsigned long sadr[15];
static unsigned long sins[130];
static char sname[256];
    int i,sum=0;

    done=0;
    // start cím
    r_pc=from;
    // inicializálás
    brk=0;  irq=0;  cyc=0;  r_a=0;  r_x=0;  r_y=0;  r_sp=0xff;  r_p=0;

    if(opt_noexe==0) {
        myrom=p4rom;
        tedinit();                                      // TED inicializálása
        done=asmexecute(from);
        tedfree();                                      // TED zárása
    } else {
        // disasm only
        opt_fulreg=0;
        lastwrite=NOWRITE;
        do {
            hiram=p4rom;
            if(memreadf(r_pc)==0) brk=16;               // BRK - end
            pre_pc=r_pc;
            r_pc+=disasm();                             // disassemble
        } while(brk==0&&done==0);
    }
    // diagnostic
    if(done==1) p4req1(NULL,P4_WARNING,GetStr(MSG_003C));
    else if(done!=0) printf("halt at illegal opcode: %02X (%d)\n",done,done);
    if(opt_memfrm>=0) memout(opt_memfrm,opt_memfrm+opt_memsiz);
    if(opt_regout!=0) regout();

    /* Old version of statistic
     */
    if(countpointer!=NULL) {
        for(i=0;i<256;i++) sum+=countpointer[i];
        printf("total: %d\n",sum);
        for(i=0;i<256;i++) {
            if(countpointer[i]!=0) {
                if(stats[i].name<XXX) {
                    if(stats[i].addr!=REL) printf("%02X  %s %-7s\t%8d\t%5.2f%%\n",i,statements[stats[i].name],addressing[stats[i].addr],countpointer[i],((double)(((double)countpointer[i])*100)/sum));
                    else printf("%02X  %s %-7s\t%8d\t%5.2f%% (%8d - %5.2f%%)\n",i,statements[stats[i].name],addressing[stats[i].addr],countpointer[i],((double)(((double)countpointer[i])*100)/sum),brcntpnt[i],((((double)brcntpnt[i])*100)/countpointer[i]));
                } else printf("%02X  %s %-7s\t%8d\t%5.2f%%\n",i,statements[XXX],statements[XXX],countpointer[i],((double)(((double)countpointer[i])*100)/sum));
            }
        }
    }

    /* New statistic - assemble with STAT defined
     */
    if(StatTab!=0) {
        time_t t;
        struct tm *lt;
        unsigned long *pnt=&StatTab,*st;
        unsigned long isum=0,ipsum=0,clkerr,allclk;
        unsigned long *ted_read_tab;
        unsigned long *ted_write_tab;
        char *name=nbuf;
        double iper,cval;
        int iiper,iitiz,ceg,cto;
        FILE *f;

        time(&t);
        lt=localtime(&t);
        strcpy(sname,"stat_");
        strftime(&sname[strlen(sname)],200,"%b%d_%H.%M.%S",lt);
        f=fopen(sname,"wb");
        if(f==NULL) f=stdout;
        pnt++;
        clkerr=*pnt++;
        allclk=*pnt++;

        // TED_READ_TAB
        ted_read_tab=pnt;
        pnt+=0x20;

        // TED_WRITE_TAB
        ted_write_tab=pnt;
        pnt+=0x20;

        st=pnt;
        for(i=0;i<256;i++) isum+=pnt[i];
        if(clkerr>0) {
            fprintf(f,"Clock error at every %ld. instruction.\n",isum/clkerr);
            fprintf(f,"Clock error at every %ld. system clock.\n",allclk/clkerr);
            if(isum>0) {
                cval=(((double)clkerr)/((double)allclk))*100.0;
                ceg=(int)cval+0.5;
                cto=((cval-((int)cval))*100.0);
                fprintf(f,"Clock error ratio: %d.%02d%%\n",ceg,cto);
            }
        } else fprintf(f,"No clock error detected\n");
        fprintf(f,"Executed system clock: %ld\n",allclk);
        cval=((double)allclk)/((double)isum);
        ceg=(int)(cval+0.5);
        cto=((cval-((int)cval))*10.0);
        fprintf(f,"Average instruction clock: %d.%d%%\n",ceg,cto);
        if(statfpsnum>0) {          // TED statistic
            cval=((double)statfpssum)/((double)statfpsnum);
            ceg=(int)(cval+0.5);
            cto=((cval-((int)cval))*10.0);
            fprintf(f,"Emulated %ld frame, at average %d.%d%% speed of a real Plus4\n",statfpsnum,ceg,cto);
        }
        fprintf(f,"\nOpCode Statistic\n");
        for(i=0;i<256;i++) {
            name[1]='\0';
            if(stats[i].name<XXX) {
                name[0]=' ';
                strcat(name,statements[stats[i].name]);
            } else {
                name[0]='!';
                strcat(name,illstatements[stats[i].name-100]);
            }
            if(*pnt>0) {
                iper=((double)(((double)(*pnt))*100)/(double)isum);
                iiper=(int)iper+0.5;
                iitiz=((iper-((int)iper))*10);
                ipsum+=((iiper*10)+iitiz);
                fprintf(f,"%02X %s %-7s\t%8ld\t %2d.%d%%\n",i,name,addressing[stats[i].addr],*pnt,iiper,iitiz);
            } else {
                fprintf(f,"%02X %s %-7s\t       -\t  -.-%%\n",i,name,addressing[stats[i].addr]);
            }
            pnt++;
        }
        fprintf(f,"Total:\t\t%8ld\t100.0%%\n",isum);
        fprintf(f,"\nAddressing Statistic\n");
        for(i=0;i<15;i++) sadr[i]=0;
        pnt=st;
        for(i=0;i<256;i++) {
            if(*pnt>0) sadr[stats[i].addr]+=*pnt;
            pnt++;
        }
        for(i=1;i<15;i++) {
            if(sadr[i]>0) {
                iper=((double)(((double)(sadr[i]))*100)/(double)isum);
                iiper=(int)iper+0.5;
                iitiz=((iper-((int)iper))*10);
                fprintf(f,"%-7s\t%8ld\t %2d.%d%%\n",addressing[i],sadr[i],iiper,iitiz);
            } else {
                fprintf(f,"%-7s\t       -\t  -.-%%\n",addressing[i]);
            }
        }
        fprintf(f,"Total:\t%8ld\t100.0%%\n",isum);
        for(i=0;i<130;i++) sins[i]=0;
        pnt=st;
        fprintf(f,"\nInstruction Statistic\n");
        for(i=0;i<256;i++) {
            if(*pnt>0) sins[stats[i].name]+=*pnt;
            pnt++;
        }
        for(i=0;i<125;i++) {
            if(sins[i]>0) {
                iper=((double)(((double)(sins[i]))*100)/(double)isum);
                iiper=(int)iper+0.5;
                iitiz=((iper-((int)iper))*10);
                if(i<56) fprintf(f," %-6s\t%8ld\t %2d.%d%%\n",statements[i],sins[i],iiper,iitiz);
                else if(i>100) fprintf(f,"!%-6s\t%8ld\t %2d.%d%%\n",illstatements[i-100],sins[i],iiper,iitiz);
            } else {
                if(i<56) fprintf(f," %-6s\t       -\t  -.-%%\n",statements[i]);
                else if(i>100) fprintf(f,"!%-6s\t       -\t  -.-%%\n",illstatements[i-100]);
            }
        }
        fprintf(f,"Total:\t%8ld\t100.0%%\n",isum);

        // TED r/w stat
        fprintf(f,"\nTED access\n");
        fprintf(f,"\n  NAME \t\t  READ \t\t WRITE\n");
        fprintf(f,"  ------------------------------------------\n");
        for(i=0;i<0x20;i++) fprintf(f,"  %s \t: %9ld \t%9ld\n",ted_names[i],ted_read_tab[i],ted_write_tab[i]);

        // memrun stat
        if(CodeTab!=0) {
            unsigned long *pnt=&CodeTab;
            unsigned long mx;
            int i,s,mem,x,n;

            pnt++;      // skip 1
            fprintf(f,"\nMemory fetch table\n\n");
            for(i=0,mx=0;i<256;i++) if(pnt[i]>mx) mx=pnt[i];
            x=mx/71;
            for(i=0,mem=0;i<256;i++,pnt++,mem+=256) {
                fprintf(f,"%04X ",mem);
                n=(*pnt)/x;
                for(s=0;s<n;s++) fprintf(f,"#");
                fprintf(f,"\n");
            }
        }

        if(f!=stdout) fclose(f);
    }
} // execute()


//--------------------------------------------------------------------------
int loadrom(char *romname,u_byte *hova) {
//--------------------------------------------------------------------------
    char *name;
    FILE *fp;
    int ret=-1;

    name=malloc(strlen(romname)+1+8);
    if(name==NULL) return(-1);
    strcpy(name,"PROGDIR:");
    strcat(name,romname);
    fp=fopen(name,"rb");
    if(fp!=NULL) {
        ret=0;
        fread(hova,1,MAXROM,fp);
        fclose(fp);
    }
    free(name);
    return(ret);
} // loadrom()


//--------------------------------------------------------------------------
int load(char *filename,int *to2d) {        // return: -1 hiba
//--------------------------------------------------------------------------
    FILE *fp;
    int ret=-1;
    int hi,lo,start,maxlen,len;
    char *from;

    fp=fopen(filename,"rb");
    if(fp!=NULL) {
        lo=fgetc(fp);
        hi=fgetc(fp);
        ret=start=(hi<<8)+lo;
        from=p4ram+start;
        maxlen=MAXRAM-start;
        len=fread(from,1,maxlen,fp);
        fclose(fp);
        len+=start;
        if(to2d!=NULL) {
            *to2d=len;
        } else {
            printf("$2D: %02X %02X\n",len&0xff,len>>8);
        }
    }
    return(ret);
} // load()


//--------------------------------------------------------------------------
int myatoi(char *str) {
//--------------------------------------------------------------------------
    int ret;
    if(*str=='$') sscanf(str,"$%x",&ret);
    else if(*str=='0'&&*(str+1)=='x') sscanf(str,"0x%x",&ret);
    else ret=atoi(str);
    return(ret);
} // myatoi()


//--------------------------------------------------------------------------
void meminit(char *pat) {
//--------------------------------------------------------------------------
    int i,j,n;
    char num[3];
    u_byte *nm;

    num[2]='\0';
    n=strlen(pat);
    nm=malloc(n);
    if(nm==NULL) return;
    for(i=0,j=0;pat[j]!='\0'&&pat[j+1]!='\0';j+=2) {
        num[0]=pat[j];
        num[1]=pat[j+1];
        sscanf(num,"%x",&n);
        nm[i++]=(u_byte)n;
    }
    n=i;
    for(i=0;i<0xffff;) for(j=0;j<n;j++) p4ram[i++]=nm[j];
    for(i=0xfd00;i<0xff40;i++) p4ram[i]=0;
    free(nm);
} // meminit()

//--------------------------------------------------------------------------
void cp4free(int flag) {
//--------------------------------------------------------------------------
    free(opt_p4dir);
    if(flag!=0) SavePrefsAll();
    SOFT_Close(CLOSE_ALL);
    FreePrefs();
} // cp4free()

//--------------------------------------------------------------------------
int main(int argc, char **argv) {
//--------------------------------------------------------------------------
static char s[260];
    int start=-1,help=0,i,j,k,r;
    char *name=NULL,*o;
    char *kernalname;
    char *basicname;
    char *funclowname;
    char *funchighname;
    char *initmem;

    // Open catalog
    Opencp4Catalog(NULL);       // user selected language

    // check for machine sizes
    if(sizeof(u_byte)!=1||sizeof(u_word)!=2) {
        fprintf(stderr,GetStr(MSG_005E));
        Closecp4Catalog();
        exit(0);
    }

#ifdef __GCC__
    signal(SIGINT,SIG_IGN);
#endif

    if(!IntuitionBase) if(!(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",36))) fprintf(stderr,GetStr(MSG_0060));

    LoadPrefs();

    // megkeresni a 64k-s határt a p4ramfix-ben
    for(i=0;(((unsigned long)(&p4ramfix[i]))&0x0000ffff)!=0;i++);
    p4ram=&p4ramfix[i];
    p4rom=p4ram+0x10000;

    // inicializálás
    errout(argv[0]);
//obsolete
    opt_memfrm=-1;
    opt_memsiz=64;
    opt_regout=0;
    opt_fulreg=0;
    opt_noexe=0;
    opt_romstr=0;
// new
    o=GetOption("P4PROGDIR",":");
    if(NULL==(opt_p4dir=malloc(strlen(o)+1))) {
        fprintf(stderr,GetStr(MSG_0063));
        Closecp4Catalog();
        exit(0);
    }
    strcpy(opt_p4dir,o);
    o=GetOption("SID","NO");
    if(strcmp(o,OP_NO)!=0) opt_sid=1;
    else opt_sid=0;
    o=GetOption("SIDTYPE","6581SID");
    opt_sidtype=2;  // 6581 default
    if(strcmp(o,"PLAYSID")==0) opt_sidtype=1;
    if(strcmp(o,"6581SID")==0) opt_sidtype=2;
    o=GetOption("SOUND","YES");
    if(strcmp(o,OP_NO)!=0) opt_nosound=0;
    else { opt_nosound=1; opt_sid=0; }
    o=GetOption("IEC","YES");
    if(strcmp(o,OP_NO)!=0) opt_iec=1;
    else opt_iec=0;
    o=GetOption("REALTIME","NO");
    if(strcmp(o,OP_NO)!=0) opt_realtime=1;
    else opt_realtime=0;
    o=GetOption("SPEEDLIMIT","YES");
    if(strcmp(o,OP_NO)!=0) opt_limit=1;
    else opt_limit=0;
    opt_c2p=GetOption("C2P",defc2p);
    opt_palettefile=GetOption("PALETTE","NONE");
    opt_scrmode=myatoi(GetOption("MODEID","-1"));
    opt_overscan=myatoi(GetOption("OVERSCAN","-1"));
    o=GetOption("TWOFRAME","NO");
    if(strcmp(o,OP_NO)==0) opt_twoscr=~0x40000000;
    else opt_twoscr=~0;
    o=GetOption("PERCENT","YES");
    if(strcmp(o,OP_NO)==0) opt_percent=0;
    else opt_percent=1;
    o=GetOption("ILLOPCODES","NO");
    if(strcmp(o,OP_NO)==0) DefDisMode=0;
    else DefDisMode=1;
    o=GetOption("WITHDATA","NO");
    if(strcmp(o,OP_NO)==0) opt_withdata=0;
    else opt_withdata=1;
    kernalname=GetOption("KERNAL","rom.kernal");
    basicname=GetOption("BASIC","rom.basic");
    funclowname=GetOption("FUNCTIONLOW","rom.funclow");
    funchighname=GetOption("FUNCTIONHIGH","rom.funchigh");
    o=GetOption("CARTRIDGE1LOW","NO");
    if(strcmp(o,OP_NO)!=0) loadrom(o,roms[ROM_CAR1LOW]);
    o=GetOption("CARTRIDGE1HIGH","NO");
    if(strcmp(o,OP_NO)!=0) loadrom(o,roms[ROM_CAR1HIG]);
    o=GetOption("CARTRIDGE2LOW","NO");
    if(strcmp(o,OP_NO)!=0) loadrom(o,roms[ROM_CAR2LOW]);
    o=GetOption("CARTRIDGE2HIGH","NO");
    if(strcmp(o,OP_NO)!=0) loadrom(o,roms[ROM_CAR2HIG]);
    o=GetOption("FOLLOW","NO");
    if(strcmp(o,OP_NO)!=0) DefFollow=1;
    else DefFollow=0;
    DefDbgTop=myatoi(GetOption("DBGTOP","-1"));
    DefDbgLeft=myatoi(GetOption("DBGLEFT","0"));
    DefMonTop=myatoi(GetOption("MONTOP","-1"));
    DefMonLeft=myatoi(GetOption("MONLEFT","0"));
    DefPrfTop=myatoi(GetOption("PRFTOP","-1"));
    DefPrfLeft=myatoi(GetOption("PRFLEFT","0"));
    opt_listwidth=myatoi(GetOption("LISTINNERWIDTH","24"));
    o=GetOption("MONWIN","NO");
    if(strcmp(o,OP_NO)!=0) DefMonitor=1;
    else DefMonitor=0;
    DefStkLeft=myatoi(GetOption("STKLEFT","-1"));
    DefStkTop=myatoi(GetOption("STKTOP","0"));
    chalf=(unsigned char)myatoi(GetOption("PERCENTCOLOR0","$71"));
    cfull=(unsigned char)myatoi(GetOption("PERCENTCOLOR1","$00"));
    initmem=GetOption("INITMEM","0000FFFFFFFF0000");
    o=GetOption("KEYMAP","DEFAULT");
    if(strcmp(o,"DEFAULT")==0) opt_keymap=NULL;
    else opt_keymap=o;
    o=GetOption("ONSCREENDISPLAY","NO");
    if(strcmp(o,OP_NO)==0) opt_onscreendisplay=0;
    else opt_onscreendisplay=1;
    o=GetOption("NOBORDER","NO");
    if(strcmp(o,OP_NO)!=0) opt_noborder=0;
    else opt_noborder=1;
    o=GetOption("XPK","NO");
    if(strcmp(o,OP_NO)!=0) opt_xpk=1;
    else opt_xpk=0;
    opt_xpktype=GetOption("XPKTYPE","----");
    opt_xpkeff=myatoi(GetOption("XPKEFFECIENT","50"));
    if(opt_xpkeff<0) opt_xpkeff=0;
    if(opt_xpkeff>100) opt_xpkeff=100;

    /* Disk settings
     *
     * DRIVExx= NONE | IEC | SOFT
     *     ^08-11
     */
    o=GetOption("DRIVE08","NONE");
    if(strcmp(o,"NONE")==0) opt_drive08=0;              // 0-semmi
    else if(strcmp(o,"IEC")==0) opt_drive08=1;          // 1-iec
    else if(strcmp(o,"SOFT")==0) opt_drive08=2;         // 2-soft
    o=GetOption("DRIVE09","NONE");
    if(strcmp(o,"NONE")==0) opt_drive09=0;              // 0-semmi
    else if(strcmp(o,"IEC")==0) opt_drive09=1;          // 1-iec
    else if(strcmp(o,"SOFT")==0) opt_drive09=2;         // 2-soft
    o=GetOption("DRIVE10","NONE");
    if(strcmp(o,"NONE")==0) opt_drive10=0;              // 0-semmi
    else if(strcmp(o,"IEC")==0) opt_drive10=1;          // 1-iec
    else if(strcmp(o,"SOFT")==0) opt_drive10=2;         // 2-soft
    o=GetOption("DRIVE11","NONE");
    if(strcmp(o,"NONE")==0) opt_drive11=0;              // 0-semmi
    else if(strcmp(o,"IEC")==0) opt_drive11=1;          // 1-iec
    else if(strcmp(o,"SOFT")==0) opt_drive11=2;         // 2-soft
    if(opt_drive08==2) if(0!=(SOFT_Init(8))) opt_drive08=0;
    if(opt_drive09==2) if(0!=(SOFT_Init(9))) opt_drive09=0;
    if(opt_drive10==2) if(0!=(SOFT_Init(10))) opt_drive10=0;
    if(opt_drive11==2) if(0!=(SOFT_Init(11))) opt_drive11=0;

    // parse args
    for(i=1;i<argc;i++) {
        if(argv[i][0]=='-') {
            switch(argv[i][1]) {
                case 'm' :
                    if(argc>i+1) opt_memfrm=myatoi(argv[++i]);
                    break;
                case 'z' :
                    if(argc>i+1) opt_memsiz=myatoi(argv[++i]);
                    break;
                case 'c' :
                    if(argc>i+1) {
                        opt_c2p=argv[++i];
                        AddOption("C2P",opt_c2p);
                    }
                    break;
                case 'o' :
                    if(argc>i+1) {
                        opt_overscan=myatoi(argv[++i]);
                        sprintf(s,"%ld",opt_overscan);
                        AddOption("OVERSCAN",s);
                    }
                    break;
                case 'v' :
                    if(argc>i+1) {
                        opt_scrmode=myatoi(argv[++i]);
                        sprintf(s,"%ld",opt_scrmode);
                        AddOption("MODEID",s);
                    }
                    break;
                case '1' :
                case 'b' :
                    if(argc>i+1) {
                        basicname=argv[++i];
                        AddOption("BASIC",basicname);
                    }
                    break;
                case '0' :
                case 'k' :
                    if(argc>i+1) {
                        kernalname=argv[++i];
                        AddOption("KERNAL",kernalname);
                    }
                    break;
                case '2' :
                    if(argc>i+1) {
                        funclowname=argv[++i];
                        AddOption("FUNCTIONLOW",funclowname);
                    }
                    break;
                case '3' :
                    if(argc>i+1) {
                        funchighname=argv[++i];
                        AddOption("FUNCTIONHIGH",funchighname);
                    }
                    break;
                case '4' :
                case '5' :
                case '6' :
                case '7' :
                    if(argc>i+1) {
                        k=(int)(argv[i][1]-'0');
                        loadrom(argv[i],roms[k]);
                        i++;
                    }
                    break;
                default :
                    // switches
                    for(j=strlen(argv[i]);j>0;--j) {
                        switch(argv[i][j]) {
                            case 'h' :
                                help=1;
                                break;
                            case 's' :
                                opt_nosound=1;
                                break;
                            case 'q' :
                                opt_regout=1;
                                break;
                            case 'p' :
                                opt_percent=0;
                                AddOption("PERCENT",OP_NO);
                                break;
                            case 'r' :
                                opt_romstr=1;
                                break;
                            case 't' :
                                opt_limit=0;
                                AddOption("SPEEDLIMIT",OP_NO);
                                break;
                            case 'f' :
                                opt_twoscr=~0;
                                AddOption("TWOFRAME",OP_YES);
                                break;
                            case 'n' :
                                opt_noexe=1;
                                opt_regout=0;
                                opt_memfrm=-1;
                                break;
                        }
                    }
                    break;
            }
        }
        else if(name==NULL) name=argv[i];
    }

    // help if needed
    if(help==1) {
        fprintf(stderr,GetStr(MSG_009A));
        fprintf(stderr,"Usage: %s [options] [binary-file]\n",argv[0]);
        fprintf(stderr,"  -h  this text\n");
        fprintf(stderr,"  -s  no sound\n");
        fprintf(stderr,"  -t  no speed limit\n");
        fprintf(stderr,"  -c  select chunky2planar\n");
        fprintf(stderr,"  -v  screenmode key\n");
        fprintf(stderr,"  -o  overscan mode (1-TEXT 2-STANDARD 3-MAX 4-VIDEO)\n");
        fprintf(stderr,"  -f  twoframe display\n");
        fprintf(stderr,"  -p  do not display percentage\n");

        fprintf(stderr,"rom options:\n  -b  basicrom-file (def: rom.basic)\n");
        fprintf(stderr,"  -k  kernalrom-file (def: rom.kernal)\n");
        fprintf(stderr,"  -2  function-low (def: rom.funclow)\n");
        fprintf(stderr,"  -3  function-high (def: rom.funchigh)\n");
        fprintf(stderr,"  -4  cartridge1-low\n");
        fprintf(stderr,"  -5  cartridge1-high\n");
        fprintf(stderr,"  -6  cartridge2-low\n");
        fprintf(stderr,"  -7  cartridge2-high\n");
        Closecp4Catalog();
        exit(0);
    }

    // noname? --> romstart
    if(name==NULL) opt_romstr=1;

    // meminit
    meminit(initmem);

    // load & execute
    r=loadrom(kernalname,roms[ROM_KERNAL]);
    if(r!=0) r=copyrom(roms[ROM_KERNAL],ROM_KERNAL);
    if(r==0) {
        r=loadrom(basicname,roms[ROM_BASIC]);
        if(r!=0) r=copyrom(roms[ROM_BASIC],ROM_BASIC);
        if(r==0) {
            loadrom(funclowname,roms[ROM_FUNCLOW]);
            loadrom(funchighname,roms[ROM_FUNCHIG]);
            if(name!=NULL) start=load(name,NULL);
            if(opt_romstr==0&&start>=0) execute(start);
            else execute(65526);
        } else errout(GetStr(MSG_00AC));
    } else errout(GetStr(MSG_00AD));

    if(IntuitionBase) { CloseLibrary((struct Library *)IntuitionBase); IntuitionBase=NULL; }
    cp4free(1);
    Closecp4Catalog();
    return(0);
}
