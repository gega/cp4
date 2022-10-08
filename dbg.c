/* :ts=4                            dbg.c
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

#include <exec/types.h>
#include <proto/asl.h>
#include <stdio.h>
#include "common.h"
#include "prefs.h"
#include "cp4_loc.h"

/*
 * from preferencesgui.c
 */
extern prefsgui(void);

#ifdef __GCC__
    extern const char   *_ctype_;
    #define _L  0x02
    #define mislower(c) ((_ctype_ + 1)[c] & _L)
#else
    #include <ctype.h>
    #define mislower(c) (islower(c))
#endif

char mtoupper(char c) {
    char rt;
    rt=(mislower(c) ? c-'a'+'A' : c);
    return(rt);
}



/*
 *  változók a ted-bõl
 */
extern void reset(void);
extern int doframe(void);
extern u_byte userblank;
extern struct Screen *cp4Scr;
extern struct Window *cp4Window;
extern struct FileRequester *frq;
extern char *makefilename(struct FileRequester *f);
extern unsigned char *chunky;

/*
 *  változók a cp4-bõl
 */
extern int opt_direct;                          // 1-direct mode
extern char *opt_p4dir;                         // default dir
extern int opt_withdata;
extern char *linedisasm(u_word adr,u_word *next,int *operand,int mode);
extern u_word wheretogo(u_word adr,int flag);   // flag==0-trace, 1-step 
extern int disgetlen(int pc,int mode);
extern u_byte *p4ram;
extern u_byte *p4rom;
extern u_byte *hiram;
extern u_byte r_a;
extern u_byte r_x;
extern u_byte r_y;
extern u_byte r_sp;
extern u_word r_pc;
extern u_byte r_p;
#define flag_n 128              // negative
#define flag_v 64               // overflow
#define flag__ 32               // nem használt (1)
#define flag_b 16               // break
#define flag_d 8                // decimal
#define flag_i 4                // interrupt
#define flag_z 2                // zero
#define flag_c 1                // carry

/*
 * valódi globálisok
 */
int DefDisMode=0;                   // 0-normal, 1-illegal disasm
int DefMonTop=-1,DefMonLeft=112;
int DefDbgTop=-1,DefDbgLeft=86;
char DefMonSea[128];
int DefMonCyc=0;                    // 0-ascii !0-scrcode
int DefMonitor=0;                   // 0-closed, !0-opened
int DefMonStart=0;                  // ~*8 == cím a topon
int DefStayFront=0;                 // lapozza vissza a cp4-et? (igen)
int DefFollow=0;                    // default: off
int DefBrkEnable=0;                 // 0-nem
int DefStkLeft=66,DefStkTop=-1;
// ezekbe ír a 7501.asm
u_byte dbgmodeflag;
unsigned long dbgamiras;
u_word dbgrasreq;
u_word dbgvideo;
u_word dbggfxbase;
u_byte dbgeirq;
u_word dbgp4ras;

/*
 * közös a debug.c-vel
 */
static int dbginput=0;
static char catab[]=\
    "................................ !\"#$%&'()*+,-./0123456789:;<=>?" \
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[£]^_@abcdefghijklmnopqrstuvwxyz....." \
    "................................................................" \
    "................................................................";
static char cstab[]=\
    "@abcdefghijklmnopqrstuvwxyz[£]^_ !\"#$%&'()*+,-./0123456789:;<=>?" \
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ..... ..............................." \
    "................................................................" \
    "................................................................";
static char *ctab=catab;

// common funcs
static void setcheckmark(int menunum,int itemnum,int state);
static int getcheckmark(int menunum,int itemnum);

#include "debug.c"
#include "modetxt.c"

// bitek
#define B0      1
#define B1      2
#define B2      4
#define B3      8
#define B4      16
#define B5      32
#define B6      64
#define B7      128

int exitcode=0;
#define EXIT_STAY       0
#define EXIT_BACK       1
#define EXIT_QUIT       2
#define EXIT_STEP       3
#define EXIT_TRACE      4
#define EXIT_RUN        5
#define EXIT_RESET      6

// 21,21 -> text base koord
static int ret=0;
static int DefDbgStart=0;
static int DefDbgEnd=0;
static int DbgSizes[28];
static int textbasex=20+DX;
static int textbasey=21+DY;
static int dbgactive=0;
static int dragdebug=0;                 // 1==dragging mouse on listview
static int debugscroll=0;
#define MYFRONTPEN      drawinfo->dri_Pens[TEXTPEN]
#define MYBACKPEN       drawinfo->dri_Pens[BACKGROUNDPEN]
#define MYSELECTEDPEN   drawinfo->dri_Pens[FILLPEN]
static struct IntuiText dbtxt[28];
static struct TextAttr myTextAttr;
static int monbasex=9;
static int monbasey=29;
static struct IntuiText montxt[27];
static struct IntuiText monactive;      // egy kétkarakteres stringre mutat, ami aktív
static char monactivetxt[]="  ";
static int monactline=0, monactcol=0;   // line(0-26) col(0-7)
static int moninput=0;
static int dragmonitor=0;               // 1==dragging mouse on listview
static monitorscroll=0;                 // épp scrolloz?
static char hexkeys[]="0123456789abcdef";
#define BACKSEE     60
static int adrs[BACKSEE];               // ebbe jönnek a címek (debug win)
static char wintitle[32];

#define BREAKPOINT      (0x02)          // breakpoint opcode
#define INTERNAL        0               // saját breakpoint number
static int brkstore[]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; // itt tároljuk az eredetit (-1 invalid)
static u_byte *brkaddr[12];
static int brkreq[]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};  // user req brkpnts
static char *brkcyc[]={
    "1:      ",
    "2:      ",
    "3:      ",
    "4:      ",
    "5:      ",
    "6:      ",
    "7:      ",
    "8:      ",
    "9:      ",
    NULL
};
static char *montxts[]={
    " a                              |            ",
    " b                              |            ",
    " c                              |            ",
    " d                              |            ",
    " e                              |            ",
    " f                              |            ",
    " g                              |            ",
    " h                              |            ",
    " i                              |            ",
    " j                              |            ",
    " k                              |            ",
    " l                              |            ",
    " m                              |            ",
    " n                              |            ",
    " o                              |            ",
    " p                              |            ",
    " q                              |            ",
    " r                              |            ",
    " s                              |            ",
    " t                              |            ",
    " u                              |            ",
    " v                              |            ",
    " w                              |            ",
    " x                              |            ",
    " y                              |            ",
    " z                              |            ",
    " 0                              |            ",
    NULL
};

static char *txts[]={
    "a                      ",
    "b                      ",
    "c                      ",
    "d                      ",
    "e                      ",
    "f                      ",
    "g                      ",
    "h                      ",
    "j                      ",
    "k                      ",
    "l                      ",
    "m                      ",
    "n                      ",
    "o                      ",
    "p                      ",
    "q                      ",
    "r                      ",
    "s                      ",
    "t                      ",
    "u                      ",
    "v                      ",
    "w                      ",
    "x                      ",
    "y                      ",
    "z                      ",
    "0                      ",
    "1                      ",
    "2                      ",
    NULL
};


int dodebug_out(int flag);          // MAIN func

static void setcheckmark(int menunum,int itemnum,int state) {
struct MenuItem *n;
    if(NULL==(n=ItemAddress(dbgwinMenus,FULLMENUNUM(menunum,itemnum,0)))) return;
    ClearMenuStrip(dbgwinWnd);
    if(monitormenu!=0) ClearMenuStrip(monwinWnd);
    if(state==TRUE) n->Flags|=CHECKED;
    else n->Flags&=~CHECKED;
    ResetMenuStrip(dbgwinWnd,dbgwinMenus);
    if(monitormenu!=0) ResetMenuStrip(monwinWnd,dbgwinMenus);
} // setcheckmark
static int getcheckmark(int menunum,int itemnum) {
struct MenuItem *n;
    if(NULL==(n=ItemAddress(dbgwinMenus,FULLMENUNUM(menunum,itemnum,0)))) return(-1);
    if((n->Flags&CHECKED)!=0) return(TRUE);
    return(FALSE);
} // getcheckmark
/*
static void disableitem(int menunum,int itemnum,int state) {
    if(state==TRUE) OffMenu(dbgwinWnd,FULLMENUNUM(menunum,itemnum,0));
    else OnMenu(dbgwinWnd,FULLMENUNUM(menunum,itemnum,0));
} // setcheckmark
*/

static u_byte memread(u_word addr) {        // general read
    if(addr<0x8000) return(p4ram[addr]);
    return(hiram[addr]);
} // memread

static void memwrite(u_word addr,u_byte data) {
    if(addr<0x8000) p4ram[addr]=data;
    else hiram[addr]=data;
} // memwrite

static u_byte *memaddr(u_word addr) {
    if(addr<0x8000) return(&p4ram[addr]);
    return(&hiram[addr]);
} // memaddr


static void mystrcpy(char *to,char *from) {
    while(*from!='\0') *to++=*from++;
}


// törli a breakpointokat
static void clearbrk(void) {
    int i;

    for(i=0;i<12;i++) {
        if(brkstore[i]!=-1) {
            *brkaddr[i]=(u_byte)brkstore[i];
            brkstore[i]=-1;
        }
    }
} // clearbrk


// törli a breakpoint requesteket
static void clearbrkreq(void) {
static char spc[]="     ";
    int i;

    for(i=0;i<12;i++) brkreq[i]=-1;
    for(i=0;i<9;i++) {
        mystrcpy(&brkcyc[i][3],spc);
    }
    GT_SetGadgetAttrs(dbgwinGadgets[GD_dbrkpntcyc],dbgwinWnd,NULL,GTCY_Labels,(ULONG)&brkcyc,TAG_DONE);
} // clearbrkreq


static void updatedbg(void) {
    if(dbgwinWnd) {
        dbtxt[dbgactive].BackPen=MYSELECTEDPEN;
        PrintIText(dbgwinWnd->RPort,dbtxt,textbasex+sx,textbasey+sy);
        dbtxt[dbgactive].BackPen=MYBACKPEN;
    }
}

static void updatemon(void) {
    char *act;

    if(monwinWnd) {
        PrintIText(monwinWnd->RPort,montxt,monbasex+sx-2,monbasey+sy-2);
        if(monactline>=0) {
            act=(montxts[monactline])+(monactcol*3)+7;
            monactivetxt[0]=*act++;
            monactivetxt[1]=*act;
            monactive.LeftEdge=(7*8)+(3*8*monactcol);
            monactive.TopEdge=8*monactline;
            PrintIText(monwinWnd->RPort,&monactive,monbasex+sx-2,monbasey+sy-2);
        }
    }
}

static void calcmon(void) {
static char s[8];
    int st=DefMonStart*8;
    int i,j;
    char *a,*l;
    unsigned char d;

    for(i=0;i<27;i++) {
        l=montxts[i]+1;
        sprintf(s,"%04X",st);
        mystrcpy(l,s);
        a=l+6+26;
        l+=6;
        for(j=0;j<8;j++,l+=3,a++) {
            d=p4ram[st++];
            sprintf(s,"%02X",d);
            mystrcpy(l,s);
            *a=ctab[d];
        }
    }
    GT_SetGadgetAttrs(monwinGadgets[GD_monscroll],monwinWnd,NULL,GTSC_Top,DefMonStart,TAG_DONE,0L);
    updatemon();
} // calcmon


/*
 * kiszámítja a cuccokat a veremablakba
 */
static int calcstk(int flag) {
static struct Node nd[256];
static char item[256][12];
static u_word ad[256];
static int firstrun=0;
    int i,sp=r_sp+0x100,adr,dat;

    if(!stkwinWnd) return(0);
    if(flag!=0) return(ad[flag-1]);
    GT_SetGadgetAttrs(stkwinGadgets[GD_swinlst],stkwinWnd,NULL,GTLV_Labels,~0,TAG_DONE);
    if(firstrun==0) {
        firstrun=1;
        for(i=0;i<256;i++) {
            nd[i].ln_Name=item[i];
        }
    }

    swinlst0List.lh_Head=&nd[0];
    for(i=0;sp<0x200;i++,sp++) {
        dat=memread(sp+1);
        adr=((memread(sp+2)<<8)+dat)-2;
        if(memread(adr)!=0x20) {
            // data
            sprintf(item[i],"  $%02X DATA",dat);
            ad[i]=0;
        } else {
            // jsr
            sp++;
            sprintf(item[i],"$%04X JSR",adr);
            ad[i]=(u_word)adr;
        }
        if(i>0) {
            nd[i].ln_Pred=&nd[i-1];
            nd[i-1].ln_Succ=&nd[i];
            nd[i].ln_Succ=(struct Node *)&swinlst0List.lh_Tail;
            swinlst0List.lh_TailPred=&nd[i];
        } else {
            nd[0].ln_Pred=(struct Node *)&swinlst0List.lh_Head;
        }
    }
    GT_SetGadgetAttrs(stkwinGadgets[GD_swinlst],stkwinWnd,NULL,GTLV_Labels,(ULONG)&swinlst0List,TAG_DONE);
    return(0);
} // calcstk


static void calcdbg(int flag) {
static char spc[]="                   ";
static char a[8];
    u_word adr=DefDbgStart,next=0;
    int operand,i,sum=0,vanpc=0;
    char *line,*l;

    for(i=0;i<28;i++) {
        adrs[i]=adr;
        l=txts[i];
        mystrcpy(l,spc);
        line=linedisasm(adr,&next,&operand,DefDisMode);
        if(adr==r_pc) {
            if(flag!=0) dbgactive=i;
            *l='>';
            vanpc=1;
        } else if(adr>r_pc&&vanpc==0) {
            vanpc=1;
            if(i>0) txts[i-1][0]='-';
            if(flag!=0) dbgactive=i-1;
        }
        l++;
        sum+=(next-adr);
        DbgSizes[i]=sum;
        sprintf(a,"%04X",adr);
        mystrcpy(l,a);
        l+=5;
        mystrcpy(l,line);
        adr=next;
    }
    DefDbgEnd=next;
    GT_SetGadgetAttrs(dbgwinGadgets[GD_dbgscroll],dbgwinWnd,NULL,GTSC_Top,DefDbgStart>>2,TAG_DONE,0L);
    updatedbg();
}


/*
 * a follow-hoz beállítja a monitort egy pozícióba
 */
static void setmon(int adr) {
    if(adr<0) return;
    DefMonStart=(adr/8)-10;
    if(DefMonStart<0) {
        monactline=DefMonStart+10;
        DefMonStart=0;
    } else monactline=10;
    monactcol=adr%8;
}


/*
 * beállítja a checkmarkokat a menüben
 */
static void setmenu(void) {
    return;
}

static void initregs(void) {
static int mask[]={128,64,32,16,8,4,2,1};
static char s[20];
    char *t;
    int i;

    sprintf(s,"%02X",r_a);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_daccu],dbgwinWnd,NULL,GTST_String,(ULONG)s,TAG_DONE,0L);
    sprintf(s,"%02X",r_x);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_dxreg],dbgwinWnd,NULL,GTST_String,(ULONG)s,TAG_DONE,0L);
    sprintf(s,"%02X",r_y);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_dyreg],dbgwinWnd,NULL,GTST_String,(ULONG)s,TAG_DONE,0L);
    sprintf(s,"%02X",r_sp);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_dsp],dbgwinWnd,NULL,GTST_String,(ULONG)s,TAG_DONE,0L);
    sprintf(s,"%04X",r_pc);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_dpc],dbgwinWnd,NULL,GTST_String,(ULONG)s,TAG_DONE,0L);
    s[0]=' ';
    t=&s[1];
    for(i=0;i<8;i++) {
        if(i!=2) {
            if((mask[i]&r_p)==0) *t++='-';
            else *t++='1';
            *t++=' ';
        }
    }
    *t='\0';
    GT_SetGadgetAttrs(dbgwinGadgets[GD_bflags],dbgwinWnd,NULL,GTTX_Text,(ULONG)s,TAG_DONE,0L);
}


/*
 * intui events
 */
static int monwinMouseMove(struct IntuiMessage *imsg) {
    int newline,newcol;
    int scroll;
    long top;

    if(dragmonitor!=0) {
        newline=((imsg->MouseY-13)/8)-2;
        newcol=((imsg->MouseX-(7*8)-4)/24);
        if(newcol<0) newcol=0;
        else if(newcol>7) newcol=7;
        if(newline<0) {
            scroll=(-newline);
            top=DefMonStart-scroll;
            if(top<0) top=0;
            if(DefMonStart!=top) {
                DefMonStart=top;
                calcmon();
            }
            newline=0;
        } else if(newline>26) {
            scroll=newline-26;
            top=DefMonStart+scroll;
            if(top>(8192-27)) top=8192-27;
            if(DefMonStart!=top) {
                DefMonStart=top;
                calcmon();
            }
            newline=26;
        }
        if(newline!=monactline||newcol!=monactcol) {
            monactline=newline;
            monactcol=newcol;
            updatemon();
        }
    } else if(monitorscroll!=0) {
        GT_GetGadgetAttrs(monwinGadgets[GD_monscroll],monwinWnd,NULL,GTSC_Top,(ULONG)&top,TAG_DONE,0L);
        if(DefMonStart!=top) {
            DefMonStart=top;
            calcmon();
        }
    }
    return(0);
}

static int dbgwinMouseMove(struct IntuiMessage *imsg) {
    int newactive,scroll;
    long top;

    if(dragdebug!=0) {
        newactive=(((imsg->MouseY-DY)-13)/8)-1;
        if(newactive<0) {
            scroll=(-newactive);
            DefDbgStart-=scroll;
            if(DefDbgStart<0) DefDbgStart=0;
            calcdbg(0);
            newactive=0;
        } else if(newactive>27) {
            scroll=(newactive-27);
            DefDbgStart+=DbgSizes[scroll];
            if(DefDbgStart>DBGREALMAX) DefDbgStart=DBGREALMAX;
            calcdbg(0);
            newactive=27;
        }
        if(newactive!=dbgactive) {
            dbgactive=newactive;
            updatedbg();
        }
    } else if(debugscroll!=0) {
        GT_GetGadgetAttrs(dbgwinGadgets[GD_dbgscroll],dbgwinWnd,NULL,GTSC_Top,(ULONG)&top,TAG_DONE,0L);
        top<<=2;
        if(DefDbgStart!=top) {
            DefDbgStart=top;
            calcdbg(0);
        }
    }
    return(0);
}

static int monwinMouseButton(struct IntuiMessage *imsg) {
    moninput=0;
    switch(imsg->Code) {
        case SELECTDOWN :
            if(imsg->MouseX>(7+(7*8))&&imsg->MouseY>28&&imsg->MouseX<252&&imsg->MouseY<245) {
                monactline=((imsg->MouseY-13)/8)-2;
                monactcol=((imsg->MouseX-(7*8)-4)/24);
                updatemon();
                dragmonitor=1;
            }
            break;
        case SELECTUP:
            if(dragmonitor!=0) dragmonitor=0;
            break;
    }
    return(0);
}

static int dbgwinMouseButton(struct IntuiMessage *imsg) {
    int mX,mY;
    switch(imsg->Code) {
        case SELECTDOWN :
            mX=imsg->MouseX-DX;
            mY=imsg->MouseY-DY;
            if(mX>17&&mY>20&&mX<206&&mY<245) {
                dbgactive=((mY-13)/8)-1;
                updatedbg();
                dragdebug=1;
            }
            break;
        case SELECTUP:
            if(dragdebug!=0) dragdebug=0;
            break;
    }
    return(0);
}

static int dpcClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "PC" is clicked. */
    char *s;
    int newpc,v;
    GT_GetGadgetAttrs(dbgwinGadgets[GD_dpc],dbgwinWnd,NULL,GTST_String,(ULONG)&s,TAG_DONE,0L);
    sscanf(s,"%x",&v);
    r_pc=v;
    initregs();
    newpc=r_pc-26;
    if(DefDbgStart!=newpc) {
        DefDbgStart=newpc;
        calcdbg(1);
    }
    return(TRUE);
}

static int daccuClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "A" is clicked. */
    char *s;
    int v;
    GT_GetGadgetAttrs(dbgwinGadgets[GD_daccu],dbgwinWnd,NULL,GTST_String,(ULONG)&s,TAG_DONE,0L);
    sscanf(s,"%x",&v);
    r_a=v;
    initregs();
    return(TRUE);
}

static int dxregClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "X" is clicked. */
    char *s;
    int v;
    GT_GetGadgetAttrs(dbgwinGadgets[GD_dxreg],dbgwinWnd,NULL,GTST_String,(ULONG)&s,TAG_DONE,0L);
    sscanf(s,"%x",&v);
    r_x=v;
    initregs();
    return(TRUE);
}

static int dyregClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "Y" is clicked. */
    char *s;
    int v;
    GT_GetGadgetAttrs(dbgwinGadgets[GD_dyreg],dbgwinWnd,NULL,GTST_String,(ULONG)&s,TAG_DONE,0L);
    sscanf(s,"%x",&v);
    r_y=v;
    initregs();
    return(TRUE);
}

static int dspClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "SP" is clicked. */
    char *s;
    int v;
    GT_GetGadgetAttrs(dbgwinGadgets[GD_dsp],dbgwinWnd,NULL,GTST_String,(ULONG)&s,TAG_DONE,0L);
    sscanf(s,"%x",&v);
    r_sp=v;
    initregs();
    return(TRUE);
}

static int drunClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "_Run" is clicked. */
    exitcode=EXIT_RUN;
    return(FALSE);
}

static int dtraceClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "_Trace" is clicked. */
    u_word adr;

    exitcode=EXIT_TRACE;
    adr=wheretogo(r_pc,0);
    brkstore[INTERNAL]=memread(adr);
    brkaddr[INTERNAL]=memaddr(adr);
    memwrite(adr,BREAKPOINT);
    return(FALSE);
}

static int dstepClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "_Step" is clicked. */
    u_word adr;

    exitcode=EXIT_STEP;
    adr=wheretogo(r_pc,1);
    brkstore[INTERNAL]=memread(adr);
    brkaddr[INTERNAL]=memaddr(adr);
    memwrite(adr,BREAKPOINT);
    return(FALSE);
}

static int dbrkpntcycClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "" is clicked. */
    return(TRUE);
}

static int donoffbrkpntClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "BreakPoints" is clicked. */
    long checked;
    GT_GetGadgetAttrs(dbgwinGadgets[GD_donoffbrkpnt],dbgwinWnd,NULL,GTCB_Checked,(ULONG)&checked,TAG_DONE);
    if(checked==TRUE) {
        setcheckmark(M_DEBUG,MD_BREAKPOINTS,TRUE);
        DefBrkEnable=1;
    } else {
        setcheckmark(M_DEBUG,MD_BREAKPOINTS,FALSE);
        DefBrkEnable=0;
    }
    return(TRUE);
}

static int dclearClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "Clear" is clicked. */
    clearbrkreq();
    GT_SetGadgetAttrs(dbgwinGadgets[GD_donoffbrkpnt],dbgwinWnd,NULL,GTCB_Checked,FALSE,TAG_DONE);
    return(TRUE);
}

static int baddClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "Add" is clicked. */
static char str[6];
    long active;
    GT_GetGadgetAttrs(dbgwinGadgets[GD_dbrkpntcyc],dbgwinWnd,NULL,GTCY_Active,(ULONG)&active,TAG_DONE);
    brkreq[active+1]=adrs[dbgactive];
    sprintf(str,"$%04X",adrs[dbgactive]);
    mystrcpy(&brkcyc[active][3],str);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_dbrkpntcyc],dbgwinWnd,NULL,GTCY_Labels,(ULONG)&brkcyc,TAG_DONE);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_donoffbrkpnt],dbgwinWnd,NULL,GTCB_Checked,TRUE,TAG_DONE);
    setcheckmark(M_DEBUG,MD_BREAKPOINTS,TRUE);
    DefBrkEnable=1;
    return(TRUE);
}

static int dbgscrollClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "" is clicked. */
    DefDbgStart=imsg->Code<<2;
    calcdbg(0);
    if(imsg->Class==IDCMP_GADGETDOWN) {
        ReportMouse(TRUE,dbgwinWnd);
        debugscroll=1;
    } else {
        ReportMouse(FALSE,dbgwinWnd);
        debugscroll=0;
    }
    return(TRUE);
}

static int moncycClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "" is clicked. */
    ULONG act;
    moninput=0;
    GT_GetGadgetAttrs(monwinGadgets[GD_moncyc],monwinWnd,NULL,GTCY_Active,(ULONG)&act,TAG_DONE);
    if(act==0) {
        ctab=catab;
        setcheckmark(M_MONITOR,MM_ASCII,TRUE);
        setcheckmark(M_MONITOR,MM_SCRCODE,FALSE);
    } else {
        ctab=cstab;
        setcheckmark(M_MONITOR,MM_ASCII,FALSE);
        setcheckmark(M_MONITOR,MM_SCRCODE,TRUE);
    }
    calcmon();
    return(TRUE);
}

static int monstrClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "" is clicked. */
    char *str;
    moninput=0;
    GT_GetGadgetAttrs(monwinGadgets[GD_monstr],monwinWnd,NULL,GTST_String,(ULONG)&str,TAG_DONE);
    strcpy(DefMonSea,str);
    return(TRUE);
}

static int monseaClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "Search" is clicked. */
static int dat[128];
    int h1;
    char *str,c;
    int adr,oadr,find=0,i,j,mx;

    moninput=0;
    GT_SetGadgetAttrs(monwinGadgets[GD_monsea],monwinWnd,NULL,GA_Disabled,TRUE,TAG_DONE);
    GT_GetGadgetAttrs(monwinGadgets[GD_monstr],monwinWnd,NULL,GTST_String,(ULONG)&str,TAG_DONE);
    adr=((monactline+DefMonStart)*8)+monactcol+1;
    if(*str=='$') {
        // HEX search
        mx=strlen(str)-1;
        for(i=1,j=0;i<mx;i+=2,j++) {
            c=str[i+2];
            str[i+2]='\0';
            sscanf(&str[i],"%x",&h1);
            str[i+2]=c;
            dat[j]=(u_byte)h1;
            dat[j+1]=-1;
        }
        while(find==0&&adr<0x10000) {
            if(p4ram[adr]==dat[0]) {
                oadr=adr;
                for(i=0;dat[++i]==p4ram[++adr];);
                if(dat[i]==-1) {
                    // megvan, urás oda
                    DefMonStart=oadr/8;
                    monactcol=oadr%8;
                    monactline=0;
                    if(DefMonStart>(8192-27)) {
                        j=DefMonStart-(8192-27);
                        DefMonStart=(8192-27);
                        monactline=j;
                    }
                    find=1;
                    calcmon();
                } else adr=oadr;
            }
            adr++;
        }
    } else {
        // STRING search
        if(strlen(str)>0) {
            mx=strlen(str);
            for(i=0;i<mx;i++) {
                dat[i]=mtoupper(str[i]);
                dat[i+1]=-1;
            }
            while(find==0&&adr<0x10000) {
                if(dat[0]==mtoupper(p4ram[adr])) {
                    oadr=adr;
                    for(i=0;dat[++i]==mtoupper(p4ram[++adr]););
                    if(dat[i]==-1) {
                        // megvan, urás oda
                        DefMonStart=oadr/8;
                        monactcol=oadr%8;
                        monactline=0;
                        if(DefMonStart>(8192-27)) {
                            j=DefMonStart-(8192-27);
                            DefMonStart=(8192-27);
                            monactline=j;
                        }
                        find=1;
                        calcmon();
                    } else adr=oadr;
                }
                adr++;
            }
        }
    }
    GT_SetGadgetAttrs(monwinGadgets[GD_monsea],monwinWnd,NULL,GA_Disabled,FALSE,TAG_DONE);
    return(TRUE);
}

static int monscrollClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "" is clicked. */
    moninput=0;
    DefMonStart=imsg->Code;
    calcmon();
    if(imsg->Class==IDCMP_GADGETDOWN) {
        ReportMouse(TRUE,monwinWnd);
        monitorscroll=1;
    } else {
        ReportMouse(FALSE,monwinWnd);
        monitorscroll=0;
    }
    return(TRUE);
}

static int dbgwinmpprefs(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Prefs" is selected. */
    int r=TRUE,rt;
    SleepWindows();
    rt=prefsgui();
    if(rt!=0) {
        exitcode=EXIT_BACK;
        if((rt&1)!=0) exitcode=EXIT_RESET;
        if((rt&2)!=0) exitcode=EXIT_QUIT;
        r=FALSE;
    }
    ret|=B6;                        // prefs is kell!
    ScreenToFront(Scr);
    AwakeWindows();
    ActivateWindow(imsg->IDCMPWindow);
    WindowToFront(imsg->IDCMPWindow);
    return(r);
}

void savemem(char *filename,int from,int to) {
    FILE *f;
    int lo,hi,i;
    if(filename==NULL) return;
    if(from<0||from>0xffff||to>0xffff) return;
    if(to<=from) return;
    f=fopen(filename,"wb");
    if(f==NULL) return;
    lo=from&0xff;
    hi=from>>8;
    fputc(lo,f);
    fputc(hi,f);
    for(i=from;i<to;i++) fputc((int)p4ram[i],f);
    fclose(f);
} // savemem

static int dbgwinmpsave(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Save mem" is selected. */
    int exit=0,from,to;
    char str[10];
    char *filename,*s;
    SleepWindows();
    if(0==OpensvmwinWindow(imsg->MouseX,imsg->MouseY,Scr)) {
        from=(memread(0x2c)<<8)+memread(0x2b);
        to=(memread(0x2e)<<8)+memread(0x2d);
        sprintf(str,"%04X",from);
        GT_SetGadgetAttrs(svmwinGadgets[GD_sfromstr],svmwinWnd,NULL,GTST_String,(ULONG)str,TAG_DONE);
        sprintf(str,"%04X",to);
        GT_SetGadgetAttrs(svmwinGadgets[GD_stostr],svmwinWnd,NULL,GTST_String,(ULONG)str,TAG_DONE);
        while(!exit) {
            WaitPort(svmwinWnd->UserPort);
            exit=HandlesvmwinIDCMP();
        }
        if(exit==1) {
            // save it, filereq, save, ha nem cancel
            GT_GetGadgetAttrs(svmwinGadgets[GD_sfromstr],svmwinWnd,NULL,GTST_String,(ULONG)&s,TAG_DONE);
            sscanf(s,"%x",&from);
            GT_GetGadgetAttrs(svmwinGadgets[GD_stostr],svmwinWnd,NULL,GTST_String,(ULONG)&s,TAG_DONE);
            sscanf(s,"%x",&to);
            if(AslRequestTags(frq,
                    ASLFR_TitleText,(ULONG)GetStr(MSG_00FE),
                    ASLFR_Window, NULL,
                    ASLFR_SleepWindow, FALSE,
                    ASLFR_DoSaveMode,TRUE,
                    ASLFR_RejectIcons, TRUE,
                    ASLFR_InitialDrawer, (ULONG)opt_p4dir,
                    ASLFR_Screen, (ULONG)Scr,
                    TAG_DONE,0L)) {
                filename=makefilename(frq);
                savemem(filename,from,to);
            }
        }
        ClosesvmwinWindow();
    }
    AwakeWindows();
    return(TRUE);
}

static int dbgwinmpreset(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Reset" is selected. */
    exitcode=EXIT_RESET;
    return(FALSE);
}

static int dbgwinmpback(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Back" is selected. */
    exitcode=EXIT_BACK;
    return(FALSE);
}

static int dbgwinmpquit(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Quit" is selected. */
    exitcode=EXIT_QUIT;
    return(FALSE);
}

static int dbgwinmdtrace(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Trace" is selected. */
    return(dtraceClicked(imsg));
}

static int dbgwinmdwithdata(struct IntuiMessage *imsg) {
    if(TRUE==getcheckmark(M_DEBUG,MD_WITHDATA)) {
        opt_withdata=1;
        AddOption("WITHDATA","YES");
    } else {
        opt_withdata=0;
        AddOption("WITHDATA","NO");
    }
    calcdbg(0);
    return(TRUE);
}

static int dbgwinmdstep(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Step" is selected. */
    return(dstepClicked(imsg));
}

static int dbgwinmdrun(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Run" is selected. */
    return(drunClicked(imsg));
}

static int dbgwinmdbreak(struct IntuiMessage *imsg) {
    /* routine when (sub)item "BreakPoints" is selected. */
    if(TRUE==getcheckmark(M_DEBUG,MD_BREAKPOINTS)) {
        GT_SetGadgetAttrs(dbgwinGadgets[GD_donoffbrkpnt],dbgwinWnd,NULL,GTCB_Checked,TRUE,TAG_DONE);
        DefBrkEnable=1;
    } else {
        GT_SetGadgetAttrs(dbgwinGadgets[GD_donoffbrkpnt],dbgwinWnd,NULL,GTCB_Checked,FALSE,TAG_DONE);
        DefBrkEnable=0;
    }
    return(TRUE);
}

static int dbgwinmdill(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Illegal OpCodes" is selected. */
    if(TRUE==getcheckmark(M_DEBUG,MD_ILLOPCODES)) {
        DefDisMode=1;
        AddOption("ILLOPCODES","YES");
    } else {
        DefDisMode=0;
        AddOption("ILLOPCODES","NO");
    }
    calcdbg(0);
    return(TRUE);
}

static int dbgwinmdstack(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Stack..." is selected. */
    if(!stkwinWnd) {
        // open
        OpenstkwinWindow();
        calcstk(0);
    } else {
        // activate
        WindowToFront(stkwinWnd);
        ActivateWindow(stkwinWnd);
    }
    return(TRUE);
}

static int dbgwinmmopen(struct IntuiMessage *imsg) {
    /* routine when (sub)item "OpenWin" is selected. */
    ULONG act;
    if(!monwinWnd) OpenmonwinWindow();
    else {
        WindowToFront(monwinWnd);
        ActivateWindow(monwinWnd);
    }
    GT_GetGadgetAttrs(monwinGadgets[GD_moncyc],monwinWnd,NULL,GTCY_Active,(ULONG)&act,TAG_DONE);
    if(act==0) {
        ctab=catab;
        setcheckmark(M_MONITOR,MM_ASCII,TRUE);
        setcheckmark(M_MONITOR,MM_SCRCODE,FALSE);
    } else {
        ctab=cstab;
        setcheckmark(M_MONITOR,MM_ASCII,FALSE);
        setcheckmark(M_MONITOR,MM_SCRCODE,TRUE);
    }
    moninput=0;
    calcmon();
    return(TRUE);
}

static int dbgwinmmfollow(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Follow" is selected. */
    if(getcheckmark(M_MONITOR,MM_FOLLOW)==TRUE) {
        DefFollow=1;
        AddOption("FOLLOW","YES");
    } else {
        DefFollow=0;
        AddOption("FOLLOW","NO");
    }
    return(TRUE);
}

static int dbgwinmmascii(struct IntuiMessage *imsg) {
    /* routine when (sub)item "Ascii" is selected. */
    int as,sc;
    ULONG act;
    as=getcheckmark(M_MONITOR,MM_ASCII);
    sc=getcheckmark(M_MONITOR,MM_SCRCODE);
    if(sc==FALSE&&as==FALSE) {
        moncycClicked(imsg);
        return(TRUE);
    }
    if(sc==TRUE) {
        ctab=cstab;
        act=1;
        DefMonCyc=1;
    } else {
        ctab=catab;
        act=0;
        DefMonCyc=0;
    }
    if(monwinWnd) {
        GT_SetGadgetAttrs(monwinGadgets[GD_moncyc],monwinWnd,NULL,GTCY_Active,act,TAG_DONE);
        calcmon();
    }
    return(TRUE);
}

static int dbgwinmmscrcode(struct IntuiMessage *imsg) {
    /* routine when (sub)item "ScreenCode" is selected. */
    dbgwinmmascii(imsg);
    return(TRUE);
}

static int dbgwinCloseWindow(struct IntuiMessage *imsg) {
    /* routine for "IDCMP_CLOSEWINDOW". */
    exitcode=EXIT_BACK;
    return(FALSE);
}


static void dbgcloseinput(void) {
    // lezárja a bevitt sort + értelmezi
    dbginput=0;
    return;
}

static int dbgwinVanillaKey(struct IntuiMessage *imsg) {
    /* routine for "IDCMP_VANILLAKEY". */
    if(dbginput==0) {
        // elsõ billentyû
        dbginput=1;
    }
    return(TRUE);
}

static int dbgwinRawKey(struct IntuiMessage *imsg) {
    /* routine for "IDCMP_RAWKEY". */
    int jump=1;

    if((imsg->Qualifier&3)!=0) jump=27;
    switch(imsg->Code) {
        case 76 :           // CRSR UP
            if(dbgactive>0&&jump>1) {
                dbgactive=0;
            } else {
                dbgactive-=jump;
            }
            if(dbgactive<0) {
                DefDbgStart+=dbgactive;
                dbgactive=0;
                if(DefDbgStart<0) DefDbgStart=0;
                else calcdbg(0);
            } else updatedbg();
            break;
        case 77 :           // CRSR DOWN
            if(dbgactive<27&&jump>1) {
                dbgactive=27;
            } else {
                dbgactive+=jump;
            }
            if(dbgactive>27) {
                DefDbgStart+=DbgSizes[dbgactive-28];
                dbgactive=27;
                if(DefDbgStart>DBGREALMAX) DefDbgStart=DBGREALMAX;
                calcdbg(0);
            } else updatedbg();
            break;
    }
    return(TRUE);
}

static int monwinCloseWindow(struct IntuiMessage *imsg) {
    /* routine for "IDCMP_CLOSEWINDOW". */
    moninput=0;
    ClosemonwinWindow();
    return(TRUE);
}

static int monwinVanillaKey(struct IntuiMessage *imsg) {
    /* routine for "IDCMP_VANILLAKEY". */
    u_byte i;
    char *s;
    int adr;

    for(s=hexkeys,i=0;*s!='\0'&&*s!=imsg->Code;s++,i++);
    if(*s!='\0') {
        adr=((monactline+DefMonStart)*8)+monactcol;
        if(moninput==0) {
            moninput=1;
            i<<=4;
            i|=(p4ram[adr]&0xf);
            p4ram[adr]=i;
            if(adr>=0xfd00&&adr<0xff40) p4rom[adr]=i;
        } else {
            moninput=0;
            i|=(p4ram[adr]&0xf0);
            p4ram[adr]=i;
            if(adr>=0xfd00&&adr<0xff40) p4rom[adr]=i;
            if(++monactcol>7) {
                monactcol=0;
                if(++monactline>26) {
                    monactline--;
                    if(++DefMonStart>(8192-27)) {
                        DefMonStart--;
                        monactcol=7;
                    }
                }
            }
        }
        calcmon();
        if(adr>=DefDbgStart&&adr<DefDbgEnd) calcdbg(0);
        else if((DefDbgStart>DefDbgEnd) && (adr>=DefDbgStart)) calcdbg(0);
    }
    return(TRUE);
}


static void monup(void) {
    if(--monactline<0) {
        monactline=0;
        if(--DefMonStart<0) DefMonStart=0;
        else calcmon();
    } else calcmon();
} // monup

static void mondown(void) {
    if(++monactline>26) {
        monactline=26;
        if(++DefMonStart>(8192-27)) DefMonStart--;
        else calcmon();
    } else calcmon();
} // mondown

static int monwinRawKey(struct IntuiMessage *imsg) {
    /* routine for "IDCMP_RAWKEY". */
    moninput=0;
    switch(imsg->Code) {
        case 79 :           // CRSR LEFT
            if(--monactcol<0) {
                monactcol=7;
                if(--monactline>=0) {
                    calcmon();
                    break;
                }
                monactline=0;
                monup();
            } else calcmon();
            break;
        case 76 :           // CRSR UP
            if((imsg->Qualifier&3)!=0) {
                // shift up
                if(monactline!=0) monactline=0;
                else {
                    DefMonStart-=26;
                    if(DefMonStart<0) DefMonStart=0;
                }
                calcmon();
            } else monup();
            break;
        case 78 :           // CRSR RIGHT
            if(++monactcol>7) {
                monactcol=0;
                if(++monactline<26) {
                    calcmon();
                    break;
                }
                mondown();
                monactline=26;
            } else calcmon();
            break;
        case 77 :           // CRSR DOWN
            if((imsg->Qualifier&3)!=0) {
                if(monactline!=26) monactline=26;
                else {
                    DefMonStart+=26;
                    if(DefMonStart>(8192-27)) DefMonStart=(8192-27);
                }
                calcmon();
            } else mondown();
            break;
    }
    return(TRUE);
} // RAWKEY


/*
 * meghatározza, hogy melyik címtõl kell disassemblálni, hogy az
 * r_pc pont a 14. sorba essen ([13])
 */
static int countstart(void) {
    int act,i,len;

    for(act=r_pc-BACKSEE,i=0;act<r_pc;i++) {
        len=disgetlen(act,DefDisMode);
        adrs[i]=act;
        act+=len;
    }
    return(adrs[i-13]);
}


static int swinlstClicked(struct IntuiMessage *imsg) {
    /* routine when gadget "" is clicked. */
    ULONG sel;
    UWORD s2;
    int adr,spc;
    GT_GetGadgetAttrs(stkwinGadgets[GD_swinlst],stkwinWnd,NULL,GTLV_Selected,(ULONG)&sel,TAG_DONE);
    s2=(UWORD)sel;
    if(sel!=~0) {
        adr=calcstk(s2+1);
        if(adr!=0) {
            spc=r_pc;
            r_pc=adr;
            DefDbgStart=countstart();
            r_pc=spc;
            dbgactive=13;
            calcdbg(0);
        }
    }
    return(TRUE);
}

static int stkwinCloseWindow(void) {
    /* routine for "IDCMP_CLOSEWINDOW". */
    ClosestkwinWindow();
    return(TRUE);
}

/*
 * SAVEMEM
 */

int sfromstrClicked(void) {
    /* routine when gadget "From" is clicked. */
    return(0);
}

int stostrClicked(void) {
    /* routine when gadget "To" is clicked. */
    return(0);
}

int ssavebClicked(void) {
    /* routine when gadget "Save" is clicked. */
    return(1);
}

int scancelbClicked(void) {
    /* routine when gadget "Cancel" is clicked. */
    return(2);
}

int svmwinCloseWindow(void) {
    /* routine for "IDCMP_CLOSEWINDOW". */
    return(2);
}



/*
 * my routines
 */

static void inittext(void) {
    int i;

    myTextAttr.ta_Name="topaz.font";
    myTextAttr.ta_YSize=8;
    myTextAttr.ta_Style=0;
    myTextAttr.ta_Flags=FPF_ROMFONT;

    for(i=0;i<28&&txts[i]!=NULL;i++) {
        // debugwin
        dbtxt[i].FrontPen=MYFRONTPEN;
        dbtxt[i].BackPen=MYBACKPEN;
        dbtxt[i].DrawMode=JAM2;
        dbtxt[i].LeftEdge=0;
        dbtxt[i].TopEdge=i*8;
        dbtxt[i].ITextFont=&myTextAttr;
        dbtxt[i].IText=txts[i];
        dbtxt[i].NextText=&dbtxt[i+1];
    }
    dbtxt[i-1].NextText=NULL;

    for(i=0;i<27&&montxts[i]!=NULL;i++) {
        montxt[i].FrontPen=MYFRONTPEN;
        montxt[i].BackPen=MYBACKPEN;
        montxt[i].DrawMode=JAM2;
        montxt[i].LeftEdge=0;
        montxt[i].TopEdge=i*8;
        montxt[i].ITextFont=&myTextAttr;
        montxt[i].IText=montxts[i];
        montxt[i].NextText=&montxt[i+1];
    }
    montxt[i-1].NextText=NULL;

    monactive.FrontPen=MYFRONTPEN;
    monactive.BackPen=MYSELECTEDPEN;
    monactive.DrawMode=JAM2;
    monactive.LeftEdge=0;
    monactive.TopEdge=0;
    monactive.ITextFont=&myTextAttr;
    monactive.IText=monactivetxt;
    monactive.NextText=NULL;
} // inittext


static void initinfo(void) {
static char buf[16];
static int mask[]={0x02,0x08,0x10,0x40};
    int i;

    if(opt_direct==0) i=dbgamiras-(unsigned long)chunky;
    else i=dbgamiras;
    if(i!=0) sprintf(buf,"%03X",(u_word)(i/352));
    else strcpy(buf,"---");
    GT_SetGadgetAttrs(dbgwinGadgets[GD_amiras],dbgwinWnd,NULL,GTTX_Text,(ULONG)buf,TAG_DONE);
    sprintf(buf,"%03X",dbgrasreq);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_rasreq],dbgwinWnd,NULL,GTTX_Text,(ULONG)buf,TAG_DONE);
    sprintf(buf,"%04X",dbgvideo);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_vidmat],dbgwinWnd,NULL,GTTX_Text,(ULONG)buf,TAG_DONE);
    sprintf(buf,"%04X",dbggfxbase);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_gfxbase],dbgwinWnd,NULL,GTTX_Text,(ULONG)buf,TAG_DONE);
    sprintf(buf,"%03X",dbgp4ras);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_cp4ras],dbgwinWnd,NULL,GTTX_Text,(ULONG)buf,TAG_DONE);
    GT_SetGadgetAttrs(dbgwinGadgets[GD_dispmode],dbgwinWnd,NULL,GTTX_Text,(ULONG)modetxt[dbgmodeflag],TAG_DONE);
    for(i=0;i<4;i++) {
        if((dbgeirq&mask[i])==0) buf[i]='-';
        else buf[i]='+';
    }
    buf[4]='\0';
    GT_SetGadgetAttrs(dbgwinGadgets[GD_eirq],dbgwinWnd,NULL,GTTX_Text,(ULONG)buf,TAG_DONE);
} // initinfo


/*
 * megadja, hogy melyik brekpnt-nél állt le
 * ha nem brkpnt -1, vagy INTERNAL, akkor 0
 */
int detectbrk(void) {
    u_byte *adr;
    int i;

    if(memread(r_pc)!=BREAKPOINT) return(0);
    adr=memaddr(r_pc);
    if(brkaddr[INTERNAL]==adr) return(0);
    for(i=0;i<12;i++) {
        if(brkaddr[i]==adr) break;
    }
    if(i<12) return(i);
    return(-1);
}

/*
 * az user req brk-pointokat beállítja, ha kell
 */
void enablebreakpoints(void) {
    long checked;
    int i;

    GT_GetGadgetAttrs(dbgwinGadgets[GD_donoffbrkpnt],dbgwinWnd,NULL,GTCB_Checked,(ULONG)&checked,TAG_DONE);
    if(checked==TRUE) {
        // beállítani!
        for(i=1;i<12;i++) {
            if(brkreq[i]!=-1) {
                brkstore[i]=memread(brkreq[i]);
                brkaddr[i]=memaddr(brkreq[i]);
                memwrite(brkreq[i],BREAKPOINT);
            }
        }
    }
}

void initgads(void) {
    GT_SetGadgetAttrs(dbgwinGadgets[GD_dbrkpntcyc],dbgwinWnd,NULL,GTCY_Labels,(ULONG)&brkcyc,TAG_DONE);
    if(DefDisMode!=0) setcheckmark(M_DEBUG,MD_ILLOPCODES,TRUE);
    else setcheckmark(M_DEBUG,MD_ILLOPCODES,FALSE);
    if(opt_withdata!=0) setcheckmark(M_DEBUG,MD_WITHDATA,TRUE);
    else setcheckmark(M_DEBUG,MD_WITHDATA,FALSE);
    if(DefBrkEnable==0) {
        setcheckmark(M_DEBUG,MD_BREAKPOINTS,FALSE);
        GT_SetGadgetAttrs(dbgwinGadgets[GD_donoffbrkpnt],dbgwinWnd,NULL,GTCB_Checked,FALSE,TAG_DONE);
    } else {
        setcheckmark(M_DEBUG,MD_BREAKPOINTS,TRUE);
        GT_SetGadgetAttrs(dbgwinGadgets[GD_donoffbrkpnt],dbgwinWnd,NULL,GTCB_Checked,TRUE,TAG_DONE);
    }
} // initgads()

/*
 * fõ funct, ezt hívják kívûlrõl
 * flag==0 user hívta
 *       1 breakpint-ról jött
 */
int dodebug_out(int flag) {
static char wintmp[32];
static int stayfront=0;
static char *defwintitle=NULL;
    int stop=1,detbrk;
    u_word next;
    int operand;

    if(defwintitle==NULL) defwintitle=GetStr(MSG_0265);
    if(modetxt[0]==NULL) initmodetxt();
    if(stayfront==0) Scr=cp4Scr;
    ret=0;
    DefDbgStart=countstart();
    if(0L==SetupScreen()) {
        if(0L==OpendbgwinWindow()) {
            if(flag==3) {
                // illegal
                strcpy(wintitle,defwintitle);
                strcat(wintitle,GetStr(MSG_0102));
                SetWindowTitles(dbgwinWnd,wintitle,(UBYTE *)~0);
            } else if(flag==2) {
                // crash
                strcpy(wintitle,defwintitle);
                strcat(wintitle,GetStr(MSG_0103));
                SetWindowTitles(dbgwinWnd,wintitle,(UBYTE *)~0);
            } else if((detbrk=detectbrk())>0) {
                // user requested breakpoint
                strcpy(wintitle,defwintitle);
                sprintf(wintmp,GetStr(MSG_0104),detbrk);
                strcat(wintitle,wintmp);
                SetWindowTitles(dbgwinWnd,wintitle,(UBYTE *)~0);
            } else if(detbrk==-1) {
                // crash (csak 02-vel)
                strcpy(wintitle,defwintitle);
                strcat(wintitle,GetStr(MSG_0103));
                SetWindowTitles(dbgwinWnd,wintitle,(UBYTE *)~0);
            } else {
                SetWindowTitles(dbgwinWnd,defwintitle,(UBYTE *)~0);
            }
            clearbrk();
            initregs();
            inittext();
            initinfo();
            initgads();
            if(DefMonitor!=0) {
                OpenmonwinWindow();
                if(DefFollow!=0) {
                    linedisasm(r_pc,&next,&operand,1);
                    setmon(operand);
                }
                calcmon();
            }
            if(DefFollow!=0) setcheckmark(M_MONITOR,MM_FOLLOW,TRUE);
            else setcheckmark(M_MONITOR,MM_FOLLOW,FALSE);
            if(DefMonCyc==0) {
                setcheckmark(M_MONITOR,MM_ASCII,TRUE);
                setcheckmark(M_MONITOR,MM_SCRCODE,FALSE);
            } else {
                setcheckmark(M_MONITOR,MM_ASCII,FALSE);
                setcheckmark(M_MONITOR,MM_SCRCODE,TRUE);
            }
            setmenu();
            calcdbg(1);
            if(flag==0) {
                WindowToFront(dbgwinWnd);
                ActivateWindow(dbgwinWnd);
            }

            while(stop!=0) {
                switch(debugwinopened+monitorwinopened+stackwinopened) {
                    case 0 :        // no win
                        stop=0;
                        exitcode=EXIT_QUIT;
                        break;
                    case 1 :        // monitorwin
                        WaitPort(monwinWnd->UserPort);
                        stop=HandlemonwinIDCMP();
                        break;
                    case 2 :        // debugwin
                        WaitPort(dbgwinWnd->UserPort);
                        stop=HandledbgwinIDCMP();
                        break;
                    case 3 :        // both win
                        Wait((1L<<dbgwinWnd->UserPort->mp_SigBit)|(1L<<monwinWnd->UserPort->mp_SigBit));
                        stop=HandledbgwinIDCMP();
                        if(monwinWnd) stop&=HandlemonwinIDCMP();
                        break;
                    case 4 :
                        // illegal (only stackwin)
                        stop=0;
                        exitcode=EXIT_QUIT;
                        break;
                    case 5 :
                        // illegal (monitor+stack)
                        stop=0;
                        exitcode=EXIT_QUIT;
                        break;
                    case 6 :
                        // debug+stack
                        Wait((1L<<dbgwinWnd->UserPort->mp_SigBit)|(1L<<stkwinWnd->UserPort->mp_SigBit));
                        stop=HandledbgwinIDCMP();
                        if(stkwinWnd) stop&=HandlestkwinIDCMP();
                        break;
                    case 7 :
                        // debug+stack+monitor
                        Wait((1L<<dbgwinWnd->UserPort->mp_SigBit)|(1L<<monwinWnd->UserPort->mp_SigBit)|(1L<<stkwinWnd->UserPort->mp_SigBit));
                        stop=HandledbgwinIDCMP();
                        if(monwinWnd) stop&=HandlemonwinIDCMP();
                        if(stkwinWnd) stop&=HandlestkwinIDCMP();
                        break;
                }
            }

        /*
         * process exit_codes
         */
        switch(exitcode) {
            case EXIT_QUIT :
                stayfront=0;
                ret=B1;
                break;
            case EXIT_STEP :
                stayfront=1;
                break;
            case EXIT_TRACE :
                stayfront=1;
                break;
            case EXIT_BACK :
                stayfront=0;
                ScreenToFront(cp4Scr);
                ActivateWindow(cp4Window);
                break;
            case EXIT_RUN :
                enablebreakpoints();
                stayfront=DefStayFront;
                if(stayfront==0) {
                    ScreenToFront(cp4Scr);
                    ActivateWindow(cp4Window);
                }
                break;
            case EXIT_RESET :
                stayfront=0;
                ScreenToFront(cp4Scr);
                ActivateWindow(cp4Window);
                if(userblank!=0) {
                    reset();
                    userblank=0;
                    doframe();
                }
                ret|=B0;
                break;
        }

        /*
         * close windows if needed
         */
        if(monwinWnd==NULL) {
            DefMonitor=0;
            AddOption("MONWIN","NO");
        } else {
            DefMonitor=1;
            AddOption("MONWIN","YES");
        }
        if(stayfront==0&&monwinWnd) ClosemonwinWindow();
        if(stayfront==0&&dbgwinWnd) ClosedbgwinWindow();
        if(stkwinWnd) ClosestkwinWindow();
        } // Opendbg
        if(!dbgwinWnd) CloseDownScreen();
    } // SetupScreen
    return(ret);
}
