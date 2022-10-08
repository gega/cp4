/* :ts=2              preferencesgui.c
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

#include <proto/asl.h>
#include <xpk/xpk.h>
#include <clib/xpkmaster_protos.h>
#include <proto/dos.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <utility/hooks.h>
#include <proto/locale.h>
#include <libraries/locale.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "p4req.h"
#include "c2p_module.h"
#include "keymap.h"
#include "cp4_loc.h"
#include "soft_iec.h"
#include "macros.h"
#include "snapshot.h"
#include "cp4_ver.h"

/* from cp4_loc.c
 */
extern struct LocaleBase *LocaleBase;

/*
 * from 7501.asm
 */
extern int initsid(void);
extern int endsid(void);

/*
 * from kmapedit.c
 */
extern struct Window *kmapeditWnd;

/*
 * változók a dbg.c-bõl
 */
extern void savemem(char *filename,int from,int to);

/*
 *  változók a cp4-bõl
 */
extern int opt_direct;              // 1-direct mode
extern int opt_xpk;                 // 0-nem, 1-igen
extern int opt_xpkeff;              // hatásfok: 0-100%
extern char *opt_xpktype;           // xpk subype (4 char + \0)
extern char *opt_palettefile;       // palette filename
extern int opt_listwidth;
extern int load(char *filename,int *to2d);
extern u_byte *p4ram;               // ram
extern u_byte *p4rom;
extern char *opt_keymap;            // keymapfile
extern char *opt_p4dir;             // default dir
extern int opt_nosound;             // hang? (0-van)
extern int opt_limit;               // ==0 nem kell speed limit
extern int opt_percent;             // ==1 kell százalék
extern int opt_iec;                 // 0-nemkell 1-van -1-nincs
extern int opt_realtime;            // 0-nincs 1-realtime
extern unsigned long opt_scrmode;   // scrmode
extern unsigned long opt_overscan;  // overscan
extern unsigned long opt_twoscr;    // twoscr
extern char *opt_c2p;               // name of c2p
extern int opt_sid;                 // 1-enable 0-disable
extern int opt_onscreendisplay;     // led stb. (0-nem)
extern int opt_drive08;             // 0-semmi 1-iec 2-soft
extern int opt_drive09;
extern int opt_drive10;
extern int opt_drive11;

/*
 *  változók a ted-bõl
 */
extern u_byte opt_border;
extern void loadpalette(char *file,struct Window *win);
extern int setkeymap(char *name);
extern int testkeymap(char *name);    // 0-ok
extern u_byte skipflag;
extern long skiptime;
extern struct iecbase *IECBase;
extern struct Library *XpkBase;
extern int nojoy;
extern int getjoy(void);
extern void freejoy(void);
extern u_byte userblank;
extern struct c2pvec *c2pv;
extern char c2pdir[];
extern char *makefilename(struct FileRequester *f);
extern void tederror(char *str);
extern int initdisplay(void);
extern void freedisplay(void);
extern struct FileRequester *frq;
extern struct ScreenModeRequester *scrrq;
extern ULONG otvened1,otvened2;       // 1/2 frame otvenedsec
extern ULONG hz80,hz1,hz2;
extern ULONG otvenedsec;              // ebbe
extern u_byte joyfireup;              // '$40'   normal
extern u_byte joykeyindex;
extern u_byte joyfiredown;
extern u_byte joyfire[];
extern unsigned char chalf;           // percent külsõ szín
extern unsigned char cfull;           // percent belsõ szín
extern struct Screen *cp4Scr;
extern struct Window *cp4Window;
extern void soundpause(void);
extern void soundresume(void);
extern int initaudio(void);
extern void freeaudio(void);
extern int vol1,vol2,per1,per2;
extern int actinchar;
extern int actcharaddr;
extern int actraster;
extern int numraster;
extern int hscroll;
extern int vscroll;
extern int colplace;
extern int textplace;
extern int curpos;
extern int leftborder;
extern u_word rasreq;
extern u_word gfxaddr;
extern u_byte colback;
extern u_byte col1;
extern u_byte col2;
extern u_byte col3;
extern u_byte actrom;
extern u_byte colbord;
extern unsigned long colbordlong;
extern u_byte ec1;
extern u_byte ec2;
extern u_byte ec3;
extern int tbordminus;
extern int rightborder;
extern int cols;
extern int sideborders;
extern u_byte cureor;
extern u_byte *amirasaddr;
extern unsigned char *chunky;



int DefPrfLeft=0;
int DefPrfTop=-1;

int prefsgui(void);                 // called from dbg.c
int prefsguis(void);                // called from 7501.asm
int ptwoframe;                      // readed from 7501.asm
int pswapjoy;
int plimit;                         // O  
int pc2p;                           // !=0 changed c2p
int pscr;                           // !=0 changed scrmode
static int c2pInfoWidth=0;          // listview szélessége
static unsigned long newscrmode;
static unsigned long newoverscan;
static char *newpalette;
static char *newc2pname;
static int pERROR=0;
static char *c2pInfo=NULL,*c2pAuthor=NULL,*c2pName=NULL;
static char kmapname[220];
static char old_keymaps[450];
static char *old_keymap;
static char *drivecyc[]={
  "1                    ",
  "2                    ",
  "3                    ",
  "4                    ",
  NULL
};
static char *drivetypecyc[]={
  "a                                                  ",
  "b                                                  ",
  "c                                                  ",
  NULL
};
static int actdrive;                // bal cycle állása
static char driveoptname[]="D64NAME___";
static char drive2optname[]="DRIVE__";
static int drivestore[4];
static char *d64names[4];

// xpk support
static u_byte *xpkbuffer=NULL;
static int xpkbufferpnt=0;
static int xpkbufferlen=0;
static ULONG checksum;
static char *xpknames=NULL;
static struct NewMenu *xpknewmenu=NULL;
static int xpknumpacker=0;
static struct Menu *xpkmenu=NULL;

int addxpkmenus(struct Menu *origmenu);
void freexpkmenus(struct Menu *menu);
void handlexpkselect(int num);

#include "prefs.h"
#include "preferences.c"

struct c2ps {
  struct Node node;     // ln_Name == filaname without extension
  char *filename;       // full path/filename
};

static void getgui(void);
static int ret;         // retval
static struct Requester InvisibleRequester;
static struct NameInfo scrmodename;
static struct List c2plist={NULL,NULL,NULL,0,0};
static char *overnames[]={
  NULL, NULL, NULL, NULL, NULL
};
static struct Hook D64Hook;
static struct Hook P4Hook;
static struct Hook SnapHook;
static struct Hook PaletteHook;
static struct Hook DateHook;
static char *DateHook_Data;
static int DateHook_Count=0;



static void setcheckmarkfull(int menunum,int itemnum,int sub,int state) {
struct MenuItem *n;
  if(NULL==(n=ItemAddress(cp4prefsMenus,FULLMENUNUM(menunum,itemnum,sub)))) return;
  ClearMenuStrip(cp4prefsWnd);
  if(state==TRUE) n->Flags|=CHECKED;
  else n->Flags&=~CHECKED;
  ResetMenuStrip(cp4prefsWnd,cp4prefsMenus);
} // setcheckmarkfull
static void setcheckmark(int menunum,int itemnum,int state) {
struct MenuItem *n;
  if(NULL==(n=ItemAddress(cp4prefsMenus,FULLMENUNUM(menunum,itemnum,0)))) return;
  ClearMenuStrip(cp4prefsWnd);
  if(state==TRUE) n->Flags|=CHECKED;
  else n->Flags&=~CHECKED;
  ResetMenuStrip(cp4prefsWnd,cp4prefsMenus);
} // setcheckmark
static int getcheckmarkfull(int menunum,int itemnum,int sub) {
struct MenuItem *n;
  if(NULL==(n=ItemAddress(cp4prefsMenus,FULLMENUNUM(menunum,itemnum,sub)))) return(-1);
  if((n->Flags&CHECKED)!=0) return(TRUE);
  return(FALSE);
} // getcheckmarkfull
static int getcheckmark(int menunum,int itemnum) {
struct MenuItem *n;
  if(NULL==(n=ItemAddress(cp4prefsMenus,FULLMENUNUM(menunum,itemnum,0)))) return(-1);
  if((n->Flags&CHECKED)!=0) return(TRUE);
  return(FALSE);
} // getcheckmark
static void disableitemfull(int menunum,int itemnum,int sub,int state) {
  if(state==TRUE) OffMenu(cp4prefsWnd,FULLMENUNUM(menunum,itemnum,sub));
  else OnMenu(cp4prefsWnd,FULLMENUNUM(menunum,itemnum,sub));
} // disableitemfull
static void disableitem(int menunum,int itemnum,int state) {
  if(state==TRUE) OffMenu(cp4prefsWnd,FULLMENUNUM(menunum,itemnum,0));
  else OnMenu(cp4prefsWnd,FULLMENUNUM(menunum,itemnum,0));
} // disableitem



int addxpkmenus(struct Menu *origmenu) {      // 104 byte loss!! ERR
static struct XpkPackerList list;
static struct TagItem pqt[]={
  { XPK_PackersQuery, (ULONG)&list },
  { TAG_DONE,0 }
};
static struct XpkMode xminfo;
static struct XpkPackerInfo xpinfo;
static struct TagItem mqt[]={
  { XPK_PackMethod, 0 },
  { XPK_ModeQuery, (ULONG)&xminfo },
  { XPK_PackMode, 99 },
  { TAG_DONE,0 }
};
static struct TagItem p2qt[]={
  { XPK_PackMethod, 0 },
  { XPK_PackerQuery, (ULONG)&xpinfo },
  { TAG_DONE,0 }
};
  struct NewMenu *nm;
  struct MenuItem *n;
  struct Menu *m;
  int i;
  ULONG mx;

  xpknumpacker=0;
  xpinfo.xpi_Name[0]='\0';
  xminfo.xm_Description[0]='\0';
  if(XpkBase==NULL) return(-1);
  if(XpkQuery(pqt)) return(-1);
  if(NULL==(xpknames=calloc(list.xpl_NumPackers,5))) return(-1);
  if(NULL==(xpknewmenu=calloc(list.xpl_NumPackers+2,sizeof(struct NewMenu)))) return(-1);
  for(i=0;i<list.xpl_NumPackers;i++) {
    if(xpknumpacker>30) continue;             // max 31 almenü!
    mqt[0].ti_Data=(ULONG)list.xpl_Packer[i];
    p2qt[0].ti_Data=(ULONG)list.xpl_Packer[i];
    if(XpkQuery(mqt)) continue;
    if(XpkQuery(p2qt)) continue;
    if((xpinfo.xpi_Flags&(XPKIF_NEEDPASSWD|XPKIF_LOSSY))!=0) continue;
    if((xminfo.xm_Ratio/10)<2) continue;
    strcpy(&xpknames[xpknumpacker*5],list.xpl_Packer[i]);
    nm=&xpknewmenu[xpknumpacker];
    nm->nm_Type=NM_SUB;
    nm->nm_Label=&xpknames[xpknumpacker*5];
    nm->nm_Flags=CHECKIT|MENUTOGGLE;
    nm->nm_UserData=(APTR)xpknumpacker+1;
    mx=~((1<<((ULONG)(xpknumpacker+2)))|1);
    nm->nm_MutualExclude=mx;
    if(strcmp(opt_xpktype,nm->nm_Label)==0) nm->nm_Flags|=CHECKED;
    if(opt_xpk==0) nm->nm_Flags|=NM_ITEMDISABLED;
    xpknumpacker++;
    nm=&xpknewmenu[xpknumpacker];
    nm->nm_Type=NM_END;
  }
  if(xpknumpacker==0) return(0);
  if(!(m=CreateMenus(xpknewmenu,GTMN_FrontPen,0L,TAG_DONE,0L))) return(-1);
  xpkmenu=m;
  if(NULL==(n=ItemAddress(origmenu,FULLMENUNUM(M_PREFS,M2_XPK,M2_XPK_LASTFIX)))) return(-1);
  if(n->NextItem!=NULL) return(-1);
  n->NextItem=(struct MenuItem *)m;
  return(0);
}

void freexpkmenus(struct Menu *menu) {        // 104 byte loss!! ERR
  struct MenuItem *n;

  if(NULL!=(n=ItemAddress(menu,FULLMENUNUM(M_PREFS,M2_XPK,M2_XPK_LASTFIX)))) n->NextItem=NULL;
  if(xpkmenu!=NULL) FreeMenus(xpkmenu);
  xpkmenu=NULL;
  if(xpknames!=NULL) free(xpknames);
  xpknames=NULL;
  if(xpknewmenu!=NULL) free(xpknewmenu);
  xpknewmenu=NULL;
}

void handlexpkselect(int num) {
  int i,v;
  char *n;

  if(xpknames==NULL) return;
  n=&xpknames[(num-1)*5];
  AddOption("XPKTYPE",n);
  opt_xpktype=GetOption("XPKTYPE","----");
  for(i=0;i<xpknumpacker;i++) {
    n=&xpknames[i*5];
    v=FALSE;
    if(strcmp(n,opt_xpktype)==0) v=TRUE;
    setcheckmarkfull(M_PREFS,M2_XPK,M2_XPK_FIRSTPCK+i,v);
  }
}


static char *getpath(char *filename) {
  static char n[230];
  char *a;

  if(DOSBase==NULL) return(NULL);
  a=PathPart(filename);
  strncpy(n,filename,(a-filename));
  n[(a-filename)+1]='\0';
//  SetWindowTitles(cp4prefsWnd,(UBYTE *)~0,n);
  return(n);
}

static void SleepWindow(void) {
static struct TagItem BusyPointerTagList[]={
  {WA_BusyPointer,TRUE},
  {TAG_END,0}
};
  if(cp4prefsWnd) {
    InitRequester(&InvisibleRequester);
    Request(&InvisibleRequester,cp4prefsWnd);
    SetWindowPointerA(cp4prefsWnd,BusyPointerTagList);
  }
}

static void AwakeWindow(void) {
  if(cp4prefsWnd) {
    EndRequest(&InvisibleRequester,cp4prefsWnd);
    SetWindowPointerA(cp4prefsWnd,NULL);
  }
}

/*
 * egy teljes path+file-névbõl elkészíti a filenevet
 */
void getpurename(char *fullname,char *name) {
  int i;

  if(fullname==NULL) return;
  i=strlen(fullname);
  for(;i>=0&&fullname[i]!=':'&&fullname[i]!='/';i--);
  strcpy(name,&fullname[i+1]);
} // getpurename()

/*
 * felszabadítja a c2p listát
 */
static void freec2ps(void) {
  struct c2ps *node;
  if(c2plist.lh_Head==NULL) {     // inicializálás
    c2plist.lh_Head=(struct Node *)&c2plist.lh_Tail;
    c2plist.lh_Tail=NULL;
    c2plist.lh_TailPred=(struct Node *)&c2plist.lh_Head;
    c2plist.lh_Type=0;
  } else {                        // free
    while(NULL!=(node=(struct c2ps *)RemHead(&c2plist))) {
      if(node->filename!=NULL) FreeVec(node->filename);
      if(node->node.ln_Name!=NULL) FreeVec(node->node.ln_Name);
      FreeVec(node);
    }
  }
} // freec2ps


/*
 * Egy exec listához hozzáad egy elemet, név szerinti rendezésben
 */
static void AddSorted(struct List *ls,struct Node *nd) {
  struct Node *n;

  if(ls->lh_Head->ln_Succ) {
    for(n=ls->lh_Head;n->ln_Succ;n=n->ln_Succ) if(strcmp(nd->ln_Name,n->ln_Name)<=0) break;
    Insert(ls,nd,n->ln_Pred);
  } else AddTail(ls,nd);
} // AddSorted()


/*
 * read c2p-directory into a linked list
 * ret: hány file-t talált, ami stimmel
 */
static int readc2ps(ULONG *selected) {
static char pat[50];
  struct FileInfoBlock *fib;
  struct Node *sel=NULL,*n;
  struct c2ps *newc2p;
  int r=0,i;
  BPTR lock;

  freec2ps();
  *selected=~0;
  if(NULL!=(fib=AllocDosObject(DOS_FIB,NULL))) {
    if(0!=(lock=Lock("PROGDIR:c2p",ACCESS_READ))) {
      if(-1!=ParsePatternNoCase("#?.c2p",pat,50)) {
        Examine(lock,fib);
        while(ExNext(lock,fib)) {
          if(fib->fib_DirEntryType<0) {
            if(MatchPatternNoCase(pat,fib->fib_FileName)) {
              if(NULL!=(newc2p=AllocVec(sizeof(struct c2ps),MEMF_ANY))) {
                if(NULL!=(newc2p->filename=AllocVec(strlen(fib->fib_FileName)+1,MEMF_ANY))) {
                  strcpy(newc2p->filename,fib->fib_FileName);
                  if(NULL!=(newc2p->node.ln_Name=AllocVec(strlen(fib->fib_FileName)+1,MEMF_ANY))) {
                    strcpy(newc2p->node.ln_Name,fib->fib_FileName);
                    newc2p->node.ln_Name[strlen(newc2p->node.ln_Name)-4]='\0';
                    if(strcmp(newc2pname,fib->fib_FileName)==0) sel=(struct Node *)newc2p;
                    AddSorted(&c2plist,(struct Node *)newc2p);
                    r++;
                  } else {
                    FreeVec(newc2p->filename);
                    FreeVec(newc2p);
                  }
                } else FreeVec(newc2p);
              }
            }
          }
        }
        UnLock(lock);
      } else {
        p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_020B));
        pERROR=1;
        r=-1;
      }
    } else {
      p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_020C));
      pERROR=1;
      r=-1;
    }
    FreeDosObject(DOS_FIB,fib);
  } else {
    p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_020D));
    pERROR=1;
    r=-1;
  }
  if(r>0&&sel!=NULL) {
    for(i=0,n=c2plist.lh_Head;n->ln_Succ;n=n->ln_Succ,i++) if(n==sel) break;
    *selected=i;
  }
  return(r);
} // readc2ps()


/*
 * betölti a megadott c2p-rõl az info-kat a
 *      c2pName, c2pAuthor, c2pInfo globálisokba
 */
static int getc2pinfos(char *filename) {
static char *none="-";
static char c2pname[200];
static char s[44];
  struct c2pvec *cv;
  struct c2pvec *(*cst)(REG(d0,unsigned long m));
  BPTR seg;

  if(c2pInfo!=NULL&&c2pInfo!=none) free(c2pInfo);
  if(c2pAuthor!=NULL&&c2pAuthor!=none) free(c2pAuthor);
  c2pName=none;
  c2pInfo=none;
  c2pAuthor=none;
  if(filename==NULL) return(-1);
  if(strlen(newc2pname)>160) return(-1);
  strcpy(c2pname,c2pdir);
  strcat(c2pname,filename);
  if(NULL==(seg=LoadSeg(c2pname))) return(-1);
  cst=(struct c2pvec *(*)(REG(d0,unsigned long m)))(((ULONG)seg)*4+4);
  cv=cst(C2P_MAGIC);
  strcpy(c2pname,cv->c2p_Name);
  strcat(c2pname," V");
  sprintf(s,"%s.%s",cv->c2p_Version,cv->c2p_Revision);
  strcat(c2pname,s);
  c2pName=c2pname;
  c2pInfo=malloc(strlen(cv->c2p_Info)+1);
  if(c2pInfo!=NULL) {
    strcpy(c2pInfo,cv->c2p_Info);
    c2pAuthor=malloc(strlen(cv->c2p_Author)+4);
    if(c2pAuthor!=NULL) {
      strcpy(c2pAuthor,GetStr(MSG_0210));
      strcat(c2pAuthor," ");
      strcat(c2pAuthor,cv->c2p_Author);
    } else c2pAuthor=none;
  } else c2pInfo=none;
  UnLoadSeg(seg);
  return(0);
} // getc2pinfos()


/*
 * megadja a stringbõl a következõ szó elejét, és annak végét
 * ha utolsó szó: NULL
 */
char *skipword(char *s,char **ends) {
char *end;
  while(*s!=' '&&*s!='\0') s++;
  if(*s=='\0') return(NULL);
  s++;
  if(*s=='\0') return(NULL);
  end=s;
  while(*end!=' '&&*end!='\0') end++;
  if(*end!='\0') end++;
  *ends=end;
  return(s);
} // skipword()

/*
 * megadja, hogy az input string elsõ hány karaktere fér el az
 * adott pixelszélességen... (-1 hiba/semmi sem fér ki)
 * 0==minden kifér
 * n==az elsõ n char fér csak ki
 */
int teststrlen(char *str,int pix) {
static struct IntuiText itext;
  char *tmp;
  int len,ret=-1,i,l;

  len=strlen(str);
  if(NULL!=(tmp=malloc(len+1))) {
    itext.ITextFont=&myTextAttr;
    itext.NextText=NULL;
    itext.IText=tmp;
    for(i=0,l=-1;i<len&&l<pix;i++) {
      tmp[i]=str[i];
      tmp[i+1]='\0';
      l=IntuiTextLength(&itext);
    }
    if(l>pix) ret=i+1;
    else if(l!=-1) ret=0;
    else ret=-1;
    free(tmp);
  }
  return(ret);
} // teststrlen()

/*
 * megadja, hogy honnan kezdve kell új sort kezdeni a
 * listview-ban
 */
static char *getnextline(char *s) {
static struct IntuiText itext;
  char *now,*endnow;
  int store,textinner;
  ULONG len=0,l;

  textinner=c2pInfoWidth-opt_listwidth;
  itext.ITextFont=&myTextAttr;
  itext.NextText=NULL;
  if(textinner<8) return(NULL);
  endnow=s;
  while(*endnow!=' '&&*endnow!='\0') endnow++;
  if(*endnow=='\0') return(NULL);               // egy szóból áll
  now=s;
  while(now!=NULL&&len<=textinner) {
    store=*endnow;
    *endnow='\0';
    itext.IText=now;
    l=IntuiTextLength(&itext);
    len+=l;
    *endnow=store;
    if(len<=textinner) now=skipword(now,&endnow);
  }
  if(len<=textinner) return(NULL);
  if(now!=NULL) *(now-1)='\0';
  return(now);
} // getnextline()

/*
 * beállítja az info szerint a gc2plist gadgetet
 */
static void setupinfo(char *info) {
static char *mi=NULL;
static struct List ml;
static struct Node *nd;
  char *p1;

  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gc2pinfo],cp4prefsWnd,NULL,GTLV_Labels,~0,TAG_DONE);
  if(mi!=NULL) { free(mi); mi=NULL; }
  if(ml.lh_Head!=NULL) while(NULL!=(nd=RemHead(&ml))) free(nd);
  ml.lh_Head=(struct Node *)&ml.lh_Tail;
  ml.lh_Tail=NULL;
  ml.lh_TailPred=(struct Node *)&ml.lh_Head;
  ml.lh_Type=0;
  if(info!=NULL) {
    mi=malloc(strlen(info)+1);
    if(mi==NULL) return;
    strcpy(mi,info);
    p1=mi;
    while(p1!=NULL) {
      if(NULL==(nd=malloc(sizeof(struct Node)))) return;
      nd->ln_Name=p1;
      AddTail(&ml,nd);
      p1=getnextline(p1);
    }
  } else return;
  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gc2pinfo],cp4prefsWnd,NULL,GTLV_Labels,(ULONG)&ml,TAG_DONE);
} // setupinfo()


/* beállítja a középsõ cycle állását (none/real1541/d64+name)
 * 0-none
 * 1-iec
 * 2-soft
 */
static void updatecyc(int mode,int devaddr) {
static char *tm[]={" ",NULL};
static UBYTE name[17];
static char *modes[]={ "NONE", "IEC", "SOFT" };
  int v,d;
  char *d64name;
  int i;
  FILE *f;

  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gdrivetypecyc],cp4prefsWnd,NULL,GTCY_Labels,(ULONG)tm,TAG_DONE);
  // set d64 name
  strcpy(drivetypecyc[2],GetStr(MSG_0301));
  driveoptname[8]=(devaddr/10)+'0';
  driveoptname[9]=(devaddr%10)+'0';
  drive2optname[5]=(devaddr/10)+'0';
  drive2optname[6]=(devaddr%10)+'0';
  d64name=GetOption(driveoptname,"NONE");
  if(strcmp(d64name,"NONE")!=0&&mode==2) {
    if(NULL!=(f=fopen(d64name,"rb"))) {
      fseek(f,(357*256)+144,SEEK_SET);
      fread(name,1,16,f);
      fclose(f);
      name[16]='\0';
      for(i=0;i<17;i++) if(name[i]==0xa0) name[i]=0;
      strcat(drivetypecyc[2]," \"");
      strcat(drivetypecyc[2],name);
      strcat(drivetypecyc[2],"\"");
    } else if(mode==2) p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_0268));
  }
  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gdrivetypecyc],cp4prefsWnd,NULL,GTCY_Labels,(ULONG)drivetypecyc,GTCY_Active,mode,TAG_DONE);
  if(opt_iec!=0) GT_SetGadgetAttrs(cp4prefsGadgets[GD_gdrivetypecyc],cp4prefsWnd,NULL,GA_Disabled,FALSE,TAG_DONE);


  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gdrivetypecyc],cp4prefsWnd,NULL,GTCY_Active,(ULONG)&v,TAG_DONE);
  AddOption(drive2optname,modes[v]);
  if(actdrive==8) opt_drive08=v;
  if(actdrive==9) opt_drive09=v;
  if(actdrive==10) opt_drive10=v;
  if(actdrive==11) opt_drive11=v;
  if(v==2) d=FALSE;
  else d=TRUE;
  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gd64sel],cp4prefsWnd,NULL,GA_Disabled,(ULONG)d,TAG_DONE);
} // updatecyc()

/*
 * gui functs
 */
static int gdriveselcycClicked( void ) {      //NEW cyc
  int v,i=0;
  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gdriveselcyc],cp4prefsWnd,NULL,GTCY_Active,(ULONG)&v,TAG_DONE);
  actdrive=v+8;
  if(v==0) i=opt_drive08;
  if(v==1) i=opt_drive09;
  if(v==2) i=opt_drive10;
  if(v==3) i=opt_drive11;
  updatecyc(i,actdrive);
  return(TRUE);
}

static int gdrivetypecycClicked( void ) {     //NEW cyc
  int v;
  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gdrivetypecyc],cp4prefsWnd,NULL,GTCY_Active,(ULONG)&v,TAG_DONE);
  updatecyc(v,actdrive);
/*
static char *modes[]={ "NONE", "IEC", "SOFT" };
  int v,d;
  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gdrivetypecyc],cp4prefsWnd,NULL,GTCY_Active,(ULONG)&v,TAG_DONE);
  AddOption(drive2optname,modes[v]);
  if(actdrive==8) opt_drive08=v;
  if(actdrive==9) opt_drive09=v;
  if(actdrive==10) opt_drive10=v;
  if(actdrive==11) opt_drive11=v;
  if(v==2) d=FALSE;
  else d=TRUE;
  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gd64sel],cp4prefsWnd,NULL,GA_Disabled,(ULONG)d,TAG_DONE);
*/
  return(TRUE);
}

static ULONG SAVEDS REGARGS D64HookFunc(REG(a0,struct Hook *mh),REG(a2,struct FileRequester *fr),REG(a1,struct AnchorPath *ap)) {
  if(ap->ap_Info.fib_Size==174848) return(TRUE);
  return(FALSE);
} // D64HookFunc()

static void InitD64Hook(void) {
  D64Hook.h_Entry=(HOOKFUNC)D64HookFunc;
  D64Hook.h_SubEntry=NULL;
  D64Hook.h_Data=NULL;
} // InitD64Hook()

static int gd64selClicked( void ) {           //NEW butt "?"
  char *filename;
  InitD64Hook();
  if(AslRequestTags(frq,
      ASLFR_TitleText,(ULONG)GetStr(MSG_0304),
      ASLFR_Window, (ULONG)cp4prefsWnd,
      ASLFR_SleepWindow, TRUE,
      ASLFR_RejectIcons, TRUE,
      ASLFR_DoSaveMode, FALSE,
      ASLFR_InitialDrawer, (ULONG)GetOption("D64DIR","PROGDIR:"),
      ASLFR_Screen, (ULONG)Scr,
      ASLFR_FilterFunc, (ULONG)&D64Hook,
      ASLFR_FilterDrawers,FALSE,
      TAG_DONE,0L )) {
    filename=makefilename(frq);
    AddOption("D64DIR",frq->fr_Drawer);
    AddOption(driveoptname,filename);
    updatecyc(2,actdrive);
  }
  return(TRUE);
}

static int gscrmodeClicked( void ) {
  /* routine when gadget "ScreenMode" is clicked. */
static char name[240];
  APTR handle;

  SleepWindow();
  if(AslRequestTags(scrrq,
      ASLSM_Window, (ULONG)cp4prefsWnd,
      ASLSM_SleepWindow, FALSE,
      ASLSM_InitialOverscanType, newoverscan,
      ASLSM_InitialDisplayID, newscrmode,
      ASLSM_TitleText,(ULONG)GetStr(MSG_0211),
      ASLSM_DoOverscanType, TRUE,
      ASLSM_PropertyMask, (ULONG)0,
      TAG_DONE,0L)) {
    newscrmode=scrrq->sm_DisplayID;
    newoverscan=scrrq->sm_OverscanType;
    handle=FindDisplayInfo(newscrmode);
    if(0<GetDisplayInfoData(handle,(UBYTE *)&scrmodename,sizeof(struct NameInfo),DTAG_NAME,0)) {
      if(strlen(scrmodename.Name)>200) GT_SetGadgetAttrs(cp4prefsGadgets[GD_gscrtxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)scrmodename.Name,TAG_DONE);
      else {
        strcpy(name,scrmodename.Name);
        strcat(name," ");
        if(newoverscan<5&&newoverscan>0) strcat(name,overnames[newoverscan]);
        GT_SetGadgetAttrs(cp4prefsGadgets[GD_gscrtxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)name,TAG_DONE);
      }
    } else {
      strcpy(name,GetStr(MSG_01DB));
      strcat(name," ");
      if(newoverscan<5&&newoverscan>0) strcat(name,overnames[newoverscan]);
      GT_SetGadgetAttrs(cp4prefsGadgets[GD_gscrtxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)name,TAG_DONE);
    }
  }
  AwakeWindow();
  return(TRUE);
}

/*
static int gsndClicked( void ) {
  // routine when gadget "Sound" is clicked.
  int v;
  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gsnd],cp4prefsWnd,NULL,GTCB_Checked,(ULONG)&v,TAG_DONE);
  setcheckmark(M_PREFS,M2_SOUND,v);
  disableitem(M_PREFS,M2_SID,!v);
  return(TRUE);
}*/

/*
static int gsplimClicked( void ) {
  // routine when gadget "Speed Limit" is clicked.
  int v;
  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gsplim],cp4prefsWnd,NULL,GTCB_Checked,(ULONG)&v,TAG_DONE);
  setcheckmark(M_PREFS,M2_SPLIM,v);
  return(TRUE);
}*/
/*
static int gtwoframeClicked( void ) {
  // routine when gadget "TwoFrame" is clicked.
  int v;
  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gtwoframe],cp4prefsWnd,NULL,GTCB_Checked,(ULONG)&v,TAG_DONE);
  setcheckmark(M_PREFS,M2_TWOFRAME,v);
  return(TRUE);
}*/
/*
static int gswapjoyClicked( void ) {
  // routine when gadget "Swap Joys" is clicked.
  int v;
  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gswapjoy],cp4prefsWnd,NULL,GTCB_Checked,(ULONG)&v,TAG_DONE);
  setcheckmark(M_PREFS,M2_SWAPJOY,v);
  return(TRUE);
}*/

static int gkeymapClicked( void ) {
  /* routine when gadget "KeyMap" is clicked. */
  char *filename;
  if(AslRequestTags(frq,
      ASLFR_TitleText,(ULONG)GetStr(MSG_01D2),
      ASLFR_Window, (ULONG)cp4prefsWnd,
      ASLFR_SleepWindow, TRUE,
      ASLFR_RejectIcons, TRUE,
      ASLFR_DoSaveMode, FALSE,
      ASLFR_InitialDrawer, (ULONG)"PROGDIR:KeyMap",
      ASLFR_Screen, (ULONG)Scr,
      ASLFR_FilterFunc, (ULONG)&KMapHook,
      ASLFR_FilterDrawers,FALSE,
      TAG_DONE,0L )) {
    filename=makefilename(frq);
    if(0!=setkeymap(filename)) {
      p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_01D4));
    } else {
      if(strlen(filename)<420) {
        AddOption("KEYMAP",filename);
        opt_keymap=GetOption("KEYMAP",NULL);
        if(opt_keymap==NULL) p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_01D5));
        GT_SetGadgetAttrs(cp4prefsGadgets[GD_gkeymaptxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)frq->rf_File,TAG_DONE);
      }
    }
  }
  return(TRUE);
}

static int gc2plistClicked( void ) {
  /* routine when gadget "" is clicked. */
  struct Node *node;
  ULONG sel;
  int i;
  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gc2plist],cp4prefsWnd,NULL,GTLV_Selected,(ULONG)&sel,TAG_DONE);
  if(sel!=~0) {
    node=c2plist.lh_Head;
    for(i=0;i<sel;i++) node=node->ln_Succ;
    newc2pname=((struct c2ps *)node)->filename;
    getc2pinfos(newc2pname);
    GT_SetGadgetAttrs(cp4prefsGadgets[GD_gc2pauthortxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)c2pAuthor,TAG_DONE);
    GT_SetGadgetAttrs(cp4prefsGadgets[GD_gc2pnametxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)c2pName,TAG_DONE);
    if(c2pInfo!=NULL&&strlen(c2pInfo)>3) setupinfo(c2pInfo);
  }
  return(TRUE);
}

static int gc2pinfoClicked( void ) {
  /* routine when gadget "" is clicked. */
  return(TRUE);
}

static int gsaveClicked( void ) {
  /* routine when gadget "Save" is clicked. */
  SleepWindow();
  getgui();
  SavePrefsAll();
  WindowToFront(cp4prefsWnd);
  ScreenToFront(Scr);
  ActivateWindow(cp4prefsWnd);
  AwakeWindow();
  return(TRUE);
}

static int guseClicked( void ) {
  /* routine when gadget "Use" is clicked. */
  getgui();
  UsePrefs();
  return(FALSE);
}

static int gcancelClicked( void ) {
  /* routine when gadget "Cancel" is clicked. */
  opt_drive08=drivestore[0];
  opt_drive09=drivestore[1];
  opt_drive10=drivestore[2];
  opt_drive11=drivestore[3];
  // reset old_keymap
  if(0!=setkeymap(old_keymap)) {
    p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_01D4));
  } else {
    if(old_keymap!=NULL) {
      AddOption("KEYMAP",old_keymap);
      if(NULL==(opt_keymap=GetOption("KEYMAP",NULL))) p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_01D5));
    } else {
      AddOption("KEYMAP","DEFAULT");
      opt_keymap=NULL;
    }
  }
  return(FALSE);
}

static int gquitClicked( void ) {
  /* routine when gadget "Quit" is clicked. */
  ret=2;
  return(FALSE);
}

static int cp4prefsCloseWindow( void ) {
  /* routine for "IDCMP_CLOSEWINDOW". */
  return(FALSE);
}


static ULONG SAVEDS REGARGS P4HookFunc(REG(a0,struct Hook *mh),REG(a2,struct FileRequester *fr),REG(a1,struct AnchorPath *ap)) {
static char fullname[512];
  int st,siz,hi,lo;
  BPTR fp;
  ULONG ret=FALSE;

  strcpy(fullname,fr->fr_Drawer);
  AddPart(fullname,ap->ap_Info.fib_FileName,512);
  siz=ap->ap_Info.fib_Size-2;
  if(NULL!=(fp=Open(fullname,MODE_OLDFILE))) {
    lo=FGetC(fp);
    hi=FGetC(fp);
    Close(fp);
    st=(hi<<8)|lo;
    if((st+siz)<=0x10000) ret=TRUE;
  }
  return(ret);
} // P4HookFunc()


static void InitP4Hook(void) {
  P4Hook.h_Entry=(HOOKFUNC)P4HookFunc;
  P4Hook.h_SubEntry=NULL;
  P4Hook.h_Data=NULL;
} // InitP4Hook()


/*
 * MENUPICK
 */
int cp4prefsmpload( void ) {
  /* routine when (sub)item "Load File" is selected. */
  char *filename,*s;
  int lastaddr;
  InitP4Hook();
  if(AslRequestTags(frq,
      ASLFR_TitleText,(ULONG)GetStr(MSG_0213),
      ASLFR_Window, (ULONG)cp4prefsWnd,
      ASLFR_SleepWindow, TRUE,
      ASLFR_RejectIcons, TRUE,
      ASLFR_DoSaveMode, FALSE,
      ASLFR_InitialDrawer, (ULONG)opt_p4dir,
      ASLFR_Screen, (ULONG)Scr,
      ASLFR_FilterFunc, (ULONG)&P4Hook,
      ASLFR_FilterDrawers,FALSE,
      TAG_DONE,0L )) {
    filename=makefilename(frq);
    s=malloc(strlen(frq->fr_Drawer)+1);
    if(s!=NULL) {
      free(opt_p4dir);
      strcpy(s,frq->fr_Drawer);
      opt_p4dir=s;
      AddOption("P4PROGDIR",s);
    }
    if(-1!=load(filename,&lastaddr)) {
      p4ram[0x2d]=lastaddr&0xff;
      p4ram[0x2e]=lastaddr>>8;
      p4ram[0x9d]=lastaddr&0xff;
      p4ram[0x9e]=lastaddr>>8;
    }
  }
  return(TRUE);
}

static int cp4prefsmpsave( void ) {
  /* routine when (sub)item "Save Prog" is selected. */
  int from,to;
  char *filename,*s;

  SleepWindow();
  from=(p4ram[0x2c]<<8)+p4ram[0x2b];
  to=(p4ram[0x2e]<<8)+p4ram[0x2d];
  if(to-from>4) {
    if(AslRequestTags(frq,
      ASLFR_TitleText,(ULONG)GetStr(MSG_0214),
      ASLFR_Window, (ULONG)cp4prefsWnd,
      ASLFR_SleepWindow, FALSE,
      ASLFR_DoSaveMode,TRUE,
      ASLFR_RejectIcons, TRUE,
      ASLFR_InitialDrawer, (ULONG)opt_p4dir,
      ASLFR_Screen, (ULONG)Scr,
      ASLFR_FilterFunc, (ULONG)NULL,
      ASLFR_FilterDrawers,FALSE,
      TAG_DONE,0L)) {
        filename=makefilename(frq);
        s=malloc(strlen(frq->fr_Drawer)+1);
        if(s!=NULL) {
          free(opt_p4dir);
          strcpy(s,frq->fr_Drawer);
          opt_p4dir=s;
          AddOption("P4PROGDIR",s);
        }
        savemem(filename,from,to);
    }
  } else p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_0215));
  AwakeWindow();
  return(TRUE);
}

static int cp4prefsmpreset( void ) {
  /* routine when (sub)item "Reset" is selected. */
  ret|=1;
  return(FALSE);
}

static int cp4prefsmpback( void ) {
  /* routine when (sub)item "Back" is selected. */
  return(FALSE);
}

static int cp4prefsmpkeymap( void ) {
  /* routine when (sub)item "SetMap" is selected. */
  int stop=1;

  SleepWindow();
  if(0==(BeginKMap(Scr))) {
    while(stop!=0) {
      WaitPort(kmapeditWnd->UserPort);
      stop=HandlekmapeditIDCMP();
    }
    EndKMap();
    getpurename(opt_keymap,kmapname);
    GT_SetGadgetAttrs(cp4prefsGadgets[GD_gkeymaptxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)kmapname,TAG_DONE);
  } else p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_0216));
  AwakeWindow();
  return(TRUE);
}

static ULONG SAVEDS REGARGS DateHookFunc(REG(a0,struct Hook *hook),REG(a2,struct Locale *loc),REG(a1,char chr)) {
  if(DateHook_Data!=NULL) DateHook_Data[DateHook_Count++]=chr;
  return(0L);
} // DateHookFunc()

static int cp4prefsmpabout( void ) {
static struct DateTime date;
static char abf[1400];
static char ve[100];
static char newdate[LEN_DATSTRING*3];
static char olddate[15];
  char *ab;
  int i;
  struct Locale *loc;

  /* routine when (sub)item "About" is selected. */
  if(strlen(GetStr(MSG_0217))>1360) {
    abf[0]='\0';
    ab=malloc(strlen(GetStr(MSG_0217))+30);
    if(ab==NULL) {
      p4req1(cp4prefsWnd,P4_INFO,GetStr(MSG_0217));
      return(TRUE);
    }
  } else ab=abf;
  strcpy(ab,GetStr(MSG_0217));
  sprintf(ve,"\n%s (",VERS);
  if(LocaleBase!=NULL) {
    date.dat_StrDay=NULL;
    date.dat_Format=FORMAT_CDN;   // dd-mm-yy
    date.dat_StrTime=NULL;
    strcpy(olddate,DATE);
    for(i=0;olddate[i]!='\0';i++) if(olddate[i]=='.') olddate[i]='-';
    date.dat_StrDate=olddate;
    StrToDate(&date);
    DateHook.h_Entry=(HOOKFUNC)DateHookFunc;
    DateHook.h_SubEntry=NULL;
    DateHook.h_Data=NULL;
    DateHook_Data=newdate;
    DateHook_Count=0;
    loc=OpenLocale(NULL);
    FormatDate(loc,loc->loc_DateFormat,&date.dat_Stamp,&DateHook);
    CloseLocale(loc);
  } else {
    strcpy(newdate,DATE);
  }
  strcat(ve,newdate);
  strcat(ve,")");
  strcat(ab,ve);
  p4req1(cp4prefsWnd,P4_INFO,ab);
  if(ab!=abf) free(ab);
  return(TRUE);
}

static int cp4prefsmpquit( void ) {
  /* routine when (sub)item "Quit" is selected. */
  ret=2;
  return(FALSE);
}

static ULONG SAVEDS REGARGS PaletteHookFunc(REG(a0,struct Hook *mh),REG(a2,struct FileRequester *fr),REG(a1,struct AnchorPath *ap)) {
  if(ap->ap_Info.fib_Size==384) return(TRUE);
  return(FALSE);
} // PaletteHookFunc()

static void InitPaletteHook(void) {
  PaletteHook.h_Entry=(HOOKFUNC)PaletteHookFunc;
  PaletteHook.h_SubEntry=NULL;
  PaletteHook.h_Data=NULL;
} // InitPaletteHook()

static int cp4prefsmppalette( void ) {              // added: 99nov22
  /* routine when (sub)item "Palette" is selected. */
  char *filename;
  InitPaletteHook();
  if(AslRequestTags(frq,
      ASLFR_TitleText,(ULONG)GetStr(MSG_0314),
      ASLFR_Window, (ULONG)cp4prefsWnd,
      ASLFR_SleepWindow, TRUE,
      ASLFR_RejectIcons, TRUE,
      ASLFR_DoSaveMode, FALSE,
      ASLFR_InitialDrawer, (ULONG)getpath(opt_palettefile),
      ASLFR_InitialFile, (ULONG)FilePart(opt_palettefile),
      ASLFR_Screen, (ULONG)Scr,
      ASLFR_FilterFunc, (ULONG)&PaletteHook,
      ASLFR_FilterDrawers,FALSE,
      TAG_DONE,0L )) {
    filename=makefilename(frq);
    if(newpalette!=NULL) free(newpalette);
    if(NULL!=(newpalette=malloc(strlen(filename)+1))) strcpy(newpalette,filename);
  }
  return(TRUE);
}

static int cp4prefsm2xpk( void ) {    // 99nov24
  int v,i;
  v=getcheckmarkfull(M_PREFS,M2_XPK,M2_XPK_ON);
  disableitemfull(M_PREFS,M2_XPK,M2_XPK_FAST,!v);
  disableitemfull(M_PREFS,M2_XPK,M2_XPK_MEDIUM,!v);
  disableitemfull(M_PREFS,M2_XPK,M2_XPK_BEST,!v);
  for(i=0;i<xpknumpacker;i++) {
    disableitemfull(M_PREFS,M2_XPK,M2_XPK_FIRSTPCK+i,!v);
  }
  return(TRUE);
}

static int cp4prefsm2sound( void ) {
  /* routine when (sub)item "Sound" is selected. */
  int v;
  v=getcheckmark(M_PREFS,M2_SOUND);
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gsnd],cp4prefsWnd,NULL,GTCB_Checked,v,TAG_DONE);
  disableitem(M_PREFS,M2_SID,!v);
  return(TRUE);
}

static int cp4prefsm2splim( void ) {
  /* routine when (sub)item "Speed Limit" is selected. */
  int v;
  v=getcheckmark(M_PREFS,M2_SPLIM);
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gsplim],cp4prefsWnd,NULL,GTCB_Checked,v,TAG_DONE);
  return(TRUE);
}

static int cp4prefsm2twoframe( void ) {
  /* routine when (sub)item "TwoFrame" is selected. */
  int v;
  v=getcheckmark(M_PREFS,M2_TWOFRAME);
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gtwoframe],cp4prefsWnd,NULL,GTCB_Checked,v,TAG_DONE);
  return(TRUE);
}

static int cp4prefsm2swapjoy( void ) {
  /* routine when (sub)item "Swap Joy" is selected. */
  int v;
  v=getcheckmark(M_PREFS,M2_SWAPJOY);
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gswapjoy],cp4prefsWnd,NULL,GTCB_Checked,v,TAG_DONE);
  return(TRUE);
}

static int cp4prefsm2perc( void ) {
  /* routine when (sub)item "Display Percent" is selected. */
  return(TRUE);
}

static int cp4prefsm2iec( void ) {
  int v;
  /* routine when (sub)item "IEC" is selected. */
  v=getcheckmark(M_PREFS,M2_IEC);
  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gdriveselcyc],cp4prefsWnd,NULL,GA_Disabled,!v,TAG_DONE);
  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gdrivetypecyc],cp4prefsWnd,NULL,GA_Disabled,!v,TAG_DONE);
  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gd64sel],cp4prefsWnd,NULL,GA_Disabled,!v,TAG_DONE);
  if(v==TRUE) gdriveselcycClicked();
  return(TRUE);
}

static int cp4prefsm2sid( void ) {
  /* routine when (sub)item "SID" is selected. */
  return(TRUE);
}

static int cp4prefsm2osd( void ) {
  /* routine when (sub)item "OSD" is selected. */
  return(TRUE);
}

static int cp4prefsm2realtime( void ) {
  /* routine when (sub)item "RealTime" is selected. */
  int v;
  v=getcheckmark(M_PREFS,M2_REALTIME);
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gtwoframe],cp4prefsWnd,NULL,GA_Disabled,v,TAG_DONE);
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gsplim],cp4prefsWnd,NULL,GA_Disabled,v,TAG_DONE);
  disableitem(M_PREFS,M2_TWOFRAME,v);
  disableitem(M_PREFS,M2_SPLIM,v);
  if(v==TRUE) {
//    GT_SetGadgetAttrs(cp4prefsGadgets[GD_gtwoframe],cp4prefsWnd,NULL,GTCB_Checked,FALSE,TAG_DONE);
//    GT_SetGadgetAttrs(cp4prefsGadgets[GD_gsplim],cp4prefsWnd,NULL,GTCB_Checked,TRUE,TAG_DONE);
    setcheckmark(M_PREFS,M2_TWOFRAME,FALSE);
    setcheckmark(M_PREFS,M2_SPLIM,TRUE);
  }
  return(TRUE);
}


static void outbyte(int a,void *o,int t) {
  unsigned char b;
  b=(unsigned char)(a&0xff);
  if(t==TYPE_PURE) fputc(b,(FILE *)o);
  if(t==TYPE_XPK) {
    if(xpkbuffer!=NULL&&xpkbufferlen>xpkbufferpnt) xpkbuffer[xpkbufferpnt++]=b;
  }
  checksum+=b;
}
static void outword(int a,void *o,int t) {
  unsigned char lo,hi;
  lo=(unsigned char)(a&0xff);
  hi=(unsigned char)(a>>8);
  if(t==TYPE_PURE) {
    fputc(hi,(FILE *)o);
    fputc(lo,(FILE *)o);
  }
  if(t==TYPE_XPK) {
    if(xpkbuffer!=NULL&&xpkbufferlen>xpkbufferpnt+1) {
      xpkbuffer[xpkbufferpnt++]=hi;
      xpkbuffer[xpkbufferpnt++]=lo;
    }
  }
  checksum+=hi;
  checksum+=lo;
}
static void outlong(unsigned long a,void *o,int t) {
  unsigned char b1,b2,b3,b4;
  b4=(unsigned char)(a&0xff);
  b3=(unsigned char)((a>>8)&0xff);
  b2=(unsigned char)((a>>16)&0xff);
  b1=(unsigned char)(a>>24);
  if(t==TYPE_PURE) {
    fputc(b1,(FILE *)o);
    fputc(b2,(FILE *)o);
    fputc(b3,(FILE *)o);
    fputc(b4,(FILE *)o);
  }
  if(t==TYPE_XPK) {
    if(xpkbuffer!=NULL&&xpkbufferlen>xpkbufferpnt+3) {
      xpkbuffer[xpkbufferpnt++]=b1;
      xpkbuffer[xpkbufferpnt++]=b2;
      xpkbuffer[xpkbufferpnt++]=b3;
      xpkbuffer[xpkbufferpnt++]=b4;
    }
  }
  checksum+=b1;
  checksum+=b2;
  checksum+=b3;
  checksum+=b4;
}
static u_byte inbyte(void *i,int t) {
  u_byte r=0;
  if(t==TYPE_PURE) r=fgetc((FILE *)i);
  if(t==TYPE_XPK&&xpkbuffer!=NULL) {
    if(xpkbufferlen>xpkbufferpnt) r=xpkbuffer[xpkbufferpnt++];
  }
  checksum+=r;
  return(r);
}
static u_word inword(void *i,int t) {
  int b=0,c=0;
  if(t==TYPE_PURE) {
    b=fgetc((FILE *)i);
    checksum+=b;
    b<<=8;
    c=fgetc((FILE *)i);
    checksum+=c;
    b|=c;
  }
  if(t==TYPE_XPK&&xpkbuffer!=NULL) {
    if(xpkbufferlen>xpkbufferpnt) b=xpkbuffer[xpkbufferpnt++];
    checksum+=b;
    b<<=8;
    if(xpkbufferlen>xpkbufferpnt) c=xpkbuffer[xpkbufferpnt++];
    checksum+=c;
    b|=c;
  }
  return(b);
}
static unsigned long inlong(void *i,int t) {
  unsigned long b=0,c=0;
  if(t==TYPE_PURE) {
    b=fgetc((FILE *)i);
    checksum+=b;
    b<<=8;
    c=fgetc((FILE *)i);
    checksum+=c;
    b|=c;
    b<<=8;
    c=fgetc((FILE *)i);
    checksum+=c;
    b|=c;
    b<<=8;
    c=fgetc((FILE *)i);
    checksum+=c;
    b|=c;
  }
  if(t==TYPE_XPK&&xpkbuffer!=NULL) {
    if(xpkbufferlen>xpkbufferpnt) b=xpkbuffer[xpkbufferpnt++];
    checksum+=b;
    b<<=8;
    if(xpkbufferlen>xpkbufferpnt) c=xpkbuffer[xpkbufferpnt++];
    checksum+=c;
    b|=c;
    b<<=8;
    if(xpkbufferlen>xpkbufferpnt) c=xpkbuffer[xpkbufferpnt++];
    checksum+=c;
    b|=c;
    b<<=8;
    if(xpkbufferlen>xpkbufferpnt) c=xpkbuffer[xpkbufferpnt++];
    checksum+=c;
    b|=c;
  }
  return(b);
}


/* 99oct12 - added snapshot handle
 * 99nov24 - added xpk support
 */
static ULONG SAVEDS REGARGS SnapHookFunc(REG(a0,struct Hook *mh),REG(a2,struct FileRequester *fr),REG(a1,struct AnchorPath *ap)) {
static char fullname[512];
static char buf[25];
//  int l=510;
//  char *t,*f;
  BPTR fp;
  ULONG ret=FALSE;

/*
  t=fullname;
  f=fr->fr_Drawer;
  while('\0'!=(*t++=*f++)&&--l>=0);
  t--;                                // points to '0'
  if(*(t-1)!=':') *t++='/';           // points to after last char
  f=ap->ap_Info.fib_FileName;
  while('\0'!=(*t++=*f++)&&--l>=0);
*/

  strcpy(fullname,fr->fr_Drawer);
  AddPart(fullname,ap->ap_Info.fib_FileName,512);
  if(NULL!=(fp=Open(fullname,MODE_OLDFILE))) {
    Read(fp,buf,12);
    Close(fp);
    buf[12]='\0';
    if(strcmp(buf,"CP4_SNAPSHOT")==0) ret=TRUE;
  }
  return(ret);
} // SnapHookFunc()

static void InitSnapHook(void) {
  SnapHook.h_Entry=(HOOKFUNC)SnapHookFunc;
  SnapHook.h_SubEntry=NULL;
  SnapHook.h_Data=NULL;
} // InitSnapHook()


int cp4prefsmpmakesnapshot( void ) {
static char xpkerr[XPKERRMSGSIZE+1];
static char err[XPKERRMSGSIZE+16];
static struct TagItem xpktags[]={
  { XPK_OutFH, 0 },                 // BPTR
  { XPK_PackMethod, NULL },         // packmethod
  { XPK_InBuf, 0 },
  { XPK_PackMode, 50 },             // 0..100
  { XPK_InLen, SNAP_LEN },
  { XPK_StepDown, TRUE },
  { XPK_GetError, (ULONG)xpkerr },
  { XPK_ShortError, TRUE },
  { TAG_DONE,0 }
};
  BPTR file=0;
  char *filename;
  void *f;
  LONG xerr;
  int i,t,type,ok=0;
  unsigned long chm,ras;

  xpkbufferpnt=xpkbufferlen=0;
  if(AslRequestTags(frq,
      ASLFR_TitleText,(ULONG)GetStr(MSG_0305),
      ASLFR_Window, (ULONG)cp4prefsWnd,
      ASLFR_SleepWindow, TRUE,
      ASLFR_RejectIcons, TRUE,
      ASLFR_DoSaveMode, TRUE,
      ASLFR_InitialDrawer, (ULONG)GetOption("SNAPSHOTDIR","PROGDIR:"),
      ASLFR_Screen, (ULONG)Scr,
      ASLFR_FilterFunc, 0L,
      ASLFR_FilterDrawers,FALSE,
      TAG_DONE,0L )) {
    filename=makefilename(frq);
    AddOption("SNAPSHOTDIR",frq->fr_Drawer);
    if(NULL!=(f=fopen(filename,"wb"))) {
      fprintf((FILE *)f,"CP4_SNAPSHOT");
      type=t=TYPE_PURE;
      if(opt_xpk!=0&&DOSBase!=NULL&&XpkBase!=NULL) {
        outbyte(TYPE_XPKC,f,t);
        fclose((FILE *)f);
        f=NULL;
        type=t=TYPE_XPK;
        file=Open(filename,MODE_READWRITE);
        if(file!=NULL) {
          Seek(file,0,OFFSET_END);
          xpkbuffer=calloc(1,SNAP_LEN+16);      // alloc mem-buffer
          if(xpkbuffer==NULL) {
            ok=1;
            Close(file);
          } else xpkbufferlen=SNAP_LEN+1;
        } else ok=1;
      } else outbyte(TYPE_PUREC,f,t);
      if(ok==0) {
        checksum=0;
        // ------------------------- snapshot-file
        outlong(sfrd2,f,t);
        outlong(sfrd3,f,t);
        outlong(sfrd4,f,t);
        outlong(sfrd5,f,t);
        outlong(sfrd6,f,t);
        outlong(sfrd7,f,t);
        outword(sft1,f,t);
        outword(sfpc,f,t);
        outword(sfc1,f,t);
        outword(sfc2,f,t);
        outword(sfc3,f,t);
        outword(sfac,f,t);
        outword(sfc1c,f,t);
        outword(sfc2c,f,t);
        outword(sfc3c,f,t);
        outword(sfchr,f,t);
        if(sfchr==0) chm=(unsigned long)(((u_byte *)sfchm)-p4ram);
        else chm=(unsigned long)(((u_byte *)sfchm)-p4rom);
        outlong(chm,f,t);
        // ------------------------- shadowed
        outlong(actraster,f,t);
        outlong(actinchar,f,t);
        outlong(actcharaddr,f,t);
        outlong(numraster,f,t);
        outlong(vscroll,f,t);
        outlong(colplace,f,t);
        outlong(textplace,f,t);
        outlong(curpos,f,t);
        if(opt_border!=0) outlong(leftborder,f,t);
        else outlong(16+hscroll,f,t);
        outlong(hscroll,f,t);
        outword(rasreq,f,t);
        outword(gfxaddr,f,t);
        outbyte(colback,f,t);
        outbyte(col1,f,t);
        outbyte(col2,f,t);
        outbyte(col3,f,t);
        outbyte(actrom,f,t);
        // ------------------------- non-shadowed
        outbyte(colbord,f,t);
        outlong(colbordlong,f,t);
        outbyte(ec1,f,t);
        outbyte(ec2,f,t);
        outbyte(ec3,f,t);
        outlong(tbordminus,f,t);
        outlong(rightborder,f,t);
        outlong(cols,f,t);
        outlong(sideborders,f,t);
        outbyte(cureor,f,t);
        if(opt_direct==0) ras=(unsigned long)(amirasaddr-chunky);
        else ras=(unsigned long)amirasaddr;
        outlong(ras,f,t);
        for(i=0;i<0x10000;i++) outbyte(p4ram[i],f,t);
        outlong(checksum,f,t);
        if(type==TYPE_PURE) fclose((FILE *)f);
        if(type==TYPE_XPK&&file!=0) {
          xpktags[0].ti_Data=(ULONG)file;
          xpktags[1].ti_Data=(ULONG)opt_xpktype;
          xpktags[2].ti_Data=(ULONG)xpkbuffer;
          xpktags[3].ti_Data=(ULONG)opt_xpkeff;
          xerr=XpkPack(xpktags);
          Close(file);
          if(xerr!=XPKERR_OK) {
            sprintf(err,"XPK: %ld (",xerr);
            strcat(err,xpkerr);
            strcat(err,")");
            p4req1(cp4prefsWnd,P4_ERROR,err);         // !LOCALIZE!
          }
          if(xpkbuffer!=NULL) free(xpkbuffer);
          xpkbuffer=NULL;
          xpkbufferpnt=xpkbufferlen=0;
        }
      }
    }
  }
  return(TRUE);
}

int cp4prefsmploadsnapshot( void ) {
static char buf[25];
static char xpkerr[XPKERRMSGSIZE+1];
static char err[XPKERRMSGSIZE+16];
static struct TagItem xpktags[]={
  { XPK_InFH, 0 },              // BPTR
  { XPK_OutBuf, 0 },
  { XPK_OutBufLen, 0 },
  { XPK_GetError, (ULONG)xpkerr },
  { XPK_ShortError, TRUE },
  { TAG_DONE,0 }
};
  BPTR file;
  char *filename;
  FILE *f;
  LONG xerr;
  ULONG chkread,chkcount;
  int i,rv=TRUE,type=-1,ok,t,checked;
  unsigned long chm,ras;

  xpkbufferpnt=xpkbufferlen=0;
  checked=0;
  type=TYPE_PURE;
  InitSnapHook();
  if(AslRequestTags(frq,
      ASLFR_TitleText,(ULONG)GetStr(MSG_0307),
      ASLFR_Window, (ULONG)cp4prefsWnd,
      ASLFR_SleepWindow, TRUE,
      ASLFR_RejectIcons, TRUE,
      ASLFR_DoSaveMode, FALSE,
      ASLFR_InitialDrawer, (ULONG)GetOption("SNAPSHOTDIR","PROGDIR:"),
      ASLFR_Screen, (ULONG)Scr,
      ASLFR_FilterFunc, (ULONG)&SnapHook,
      ASLFR_FilterDrawers,FALSE,
      TAG_DONE,0L )) {
    filename=makefilename(frq);
    AddOption("SNAPSHOTDIR",frq->fr_Drawer);
    if(NULL!=(f=fopen(filename,"rb"))) {
      fread(buf,1,12,f);
      buf[12]='\0';
      if(strcmp(buf,"CP4_SNAPSHOT")==0) {
        ok=0;
        t=inbyte(f,type);
        if(t==TYPE_PUREC) { t=TYPE_PURE; checked=1; }
        if(t==TYPE_XPKC) { t=TYPE_XPK; checked=1; }
        type=t;
        if(t!=TYPE_XPK&&t!=TYPE_PURE) {
          p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_0315));
          ok=1;
        }
        if(type==TYPE_XPK) {
          if(XpkBase!=NULL) {
            ok=1;
            fclose(f);
            f=NULL;
            file=Open(filename,MODE_OLDFILE);
            if(file!=NULL) {
              Seek(file,13,OFFSET_BEGINNING);
              xpkbuffer=malloc(SNAP_LEN+XPK_MARGIN+16);     // alloc mem-buffer
              if(xpkbuffer!=NULL) {
                xpktags[0].ti_Data=(ULONG)file;
                xpktags[1].ti_Data=(ULONG)xpkbuffer;
                xpktags[2].ti_Data=(ULONG)SNAP_LEN+XPK_MARGIN;
                if(XPKERR_OK==(xerr=XpkUnpack(xpktags))) {
                  xpkbufferpnt=0;
                  xpkbufferlen=SNAP_LEN+XPK_MARGIN;
                  ok=0;
                } else {
                  sprintf(err,"XPK: %ld (",xerr);
                  strcat(err,xpkerr);
                  strcat(err,")");
                  p4req1(cp4prefsWnd,P4_ERROR,err);         // !LOCALIZE!
                }
                Close(file);
              }
            }
          } else {
            ok=1;
            p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_0316));
          }
        }
// snapshot-file
        if(ok==0) {
          checksum=0;
          sfrd2=inlong(f,t);
          sfrd2&=~0x00c00000;   // clear DIRECT NONE flags
          if(opt_direct!=0) sfrd2|=0x00800000;
          sfrd3=inlong(f,t);
          sfrd4=inlong(f,t);
          sfrd5=inlong(f,t);
          sfrd6=inlong(f,t);
          sfrd7=inlong(f,t);
          sft1=inword(f,t);
          sfpc=inword(f,t);
          sfc1=inword(f,t);
          sfc2=inword(f,t);
          sfc3=inword(f,t);
          sfac=inword(f,t);
          sfc1c=inword(f,t);
          sfc2c=inword(f,t);
          sfc3c=inword(f,t);
          sfchr=inword(f,t);
          chm=inlong(f,t);
          if(sfchr==0) chm+=(unsigned long)p4ram;
          else chm+=(unsigned long)p4rom;
          sfchm=chm;
// shadowed
          actraster=inlong(f,t);
          actinchar=inlong(f,t);
          actcharaddr=inlong(f,t);
          numraster=inlong(f,t);
          vscroll=inlong(f,t);
          colplace=inlong(f,t);
          textplace=inlong(f,t);
          curpos=inlong(f,t);
          leftborder=inlong(f,t);
          hscroll=inlong(f,t);
          if(opt_border==0) leftborder=hscroll;
          rasreq=inword(f,t);
          gfxaddr=inword(f,t);
          colback=inbyte(f,t);
          col1=inbyte(f,t);
          col2=inbyte(f,t);
          col3=inbyte(f,t);
          actrom=inbyte(f,t);
// non-shadowed
          colbord=inbyte(f,t);
          colbordlong=inlong(f,t);
          ec1=inbyte(f,t);
          ec2=inbyte(f,t);
          ec3=inbyte(f,t);
          tbordminus=inlong(f,t);
          rightborder=inlong(f,t);
          cols=inlong(f,t);
          sideborders=inlong(f,t);
          cureor=inbyte(f,t);
          ras=inlong(f,t);
          if(opt_direct==0) amirasaddr=chunky+ras;
          else amirasaddr=(u_byte *)ras;
          for(i=0;i<0x10000;i++) p4ram[i]=(u_byte)inbyte(f,t);
          if(checked!=0) {
            chkcount=checksum;
            chkread=inlong(f,t);
            if(chkread!=chkcount) {
              p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_0317));
              ok=1;
              ret|=1;   // reset
              rv=FALSE;
            }
          }
          if(type==TYPE_XPK) {
            if(xpkbuffer!=NULL) free(xpkbuffer);
            xpkbuffer=NULL;
            xpkbufferpnt=xpkbufferlen=0;
          }
          if(ok==0) {
            ret|=4;
            rv=FALSE;
          }
        }
      } else p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_0309));
      if(type==TYPE_PURE) fclose((FILE *)f);
    }
  }
  return(rv);
}



/*
 * beállítja a gadgeteket a prefs-szerint
 */
static void setupgui(void) {
static char name[240];
static char name2[240];
  char *o;
  int v,l,i;
  APTR handle;
  ULONG sel;

  v=FALSE;
  o=GetOption("SID","NO");
  if(strcmp(o,"NO")!=0) v=TRUE;
  setcheckmark(M_PREFS,M2_SID,v);
  v=FALSE;
  if(opt_nosound==0) v=TRUE;
  else disableitem(M_PREFS,M2_SID,TRUE);
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gsnd],cp4prefsWnd,NULL,GTCB_Checked,v,TAG_DONE);
  setcheckmark(M_PREFS,M2_SOUND,v);
  v=FALSE;
  if(opt_twoscr==~0) v=TRUE;
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gtwoframe],cp4prefsWnd,NULL,GTCB_Checked,v,TAG_DONE);
  setcheckmark(M_PREFS,M2_TWOFRAME,v);
  v=FALSE;
  if(opt_limit!=0) v=TRUE;
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gsplim],cp4prefsWnd,NULL,GTCB_Checked,v,TAG_DONE);
  setcheckmark(M_PREFS,M2_SPLIM,v);
  v=FALSE;
  if(joyfireup!=0x40) v=TRUE;
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gswapjoy],cp4prefsWnd,NULL,GTCB_Checked,v,TAG_DONE);
  setcheckmark(M_PREFS,M2_SWAPJOY,v);
  v=FALSE;
  if(opt_percent!=0) v=TRUE;
  setcheckmark(M_PREFS,M2_PERC,v);
  v=FALSE;                                        // 99nov24
  if(opt_xpk!=0) v=TRUE;
  setcheckmarkfull(M_PREFS,M2_XPK,M2_XPK_ON,v);
  setcheckmarkfull(M_PREFS,M2_XPK,M2_XPK_FAST,FALSE);
  setcheckmarkfull(M_PREFS,M2_XPK,M2_XPK_MEDIUM,FALSE);
  setcheckmarkfull(M_PREFS,M2_XPK,M2_XPK_BEST,FALSE);
  v=M2_XPK_MEDIUM;
  if(opt_xpkeff==0) v=M2_XPK_FAST;
  else if(opt_xpkeff==100) v=M2_XPK_BEST;
  setcheckmarkfull(M_PREFS,M2_XPK,v,TRUE);

  v=FALSE;
  if(opt_realtime!=0) v=TRUE;
  setcheckmark(M_PREFS,M2_REALTIME,v);
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gtwoframe],cp4prefsWnd,NULL,GA_Disabled,v,TAG_DONE);
//  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gsplim],cp4prefsWnd,NULL,GA_Disabled,v,TAG_DONE);
  disableitem(M_PREFS,M2_TWOFRAME,v);
  disableitem(M_PREFS,M2_SPLIM,v);
  if(v==TRUE) {
//    GT_SetGadgetAttrs(cp4prefsGadgets[GD_gtwoframe],cp4prefsWnd,NULL,GTCB_Checked,FALSE,TAG_DONE);
//    GT_SetGadgetAttrs(cp4prefsGadgets[GD_gsplim],cp4prefsWnd,NULL,GTCB_Checked,TRUE,TAG_DONE);
    setcheckmark(M_PREFS,M2_TWOFRAME,FALSE);
    setcheckmark(M_PREFS,M2_SPLIM,TRUE);
  }
  v=FALSE;
  if(0!=opt_onscreendisplay) v=TRUE;
  setcheckmark(M_PREFS,M2_OSD,v);

  // keymapname
  if(opt_keymap!=NULL) getpurename(opt_keymap,kmapname);
  else strcpy(kmapname,GetStr(MSG_01CF));
  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gkeymaptxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)kmapname,TAG_DONE);

  // screenmode
  if(opt_scrmode!=~0) {
    handle=FindDisplayInfo(opt_scrmode);
    if(handle!=NULL) {
      if(0<GetDisplayInfoData(handle,(UBYTE *)&scrmodename,sizeof(struct NameInfo),DTAG_NAME,0)) {
        if(strlen(scrmodename.Name)>200) GT_SetGadgetAttrs(cp4prefsGadgets[GD_gscrtxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)scrmodename.Name,TAG_DONE);
        else {
          strcpy(name,scrmodename.Name);
          strcat(name," ");
          if(opt_overscan<5&&opt_overscan>0) strcat(name,overnames[opt_overscan]);
          GT_SetGadgetAttrs(cp4prefsGadgets[GD_gscrtxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)name,TAG_DONE);
        }
      } else {
        strcpy(name,GetStr(MSG_01DB));
        strcat(name," ");
        if(opt_overscan<5&&opt_overscan>0) strcat(name,overnames[opt_overscan]);
        GT_SetGadgetAttrs(cp4prefsGadgets[GD_gscrtxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)name,TAG_DONE);
      }
    } else {
      p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_0218));
      opt_scrmode=-1;
      strcpy(name,GetStr(MSG_0218));
      GT_SetGadgetAttrs(cp4prefsGadgets[GD_gscrtxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)name,TAG_DONE);
    }
  } else {
    strcpy(name,"-");
    GT_SetGadgetAttrs(cp4prefsGadgets[GD_gscrtxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)name,TAG_DONE);
  }

  // setup c2ps
  if(0<readc2ps(&sel)) {
    GT_SetGadgetAttrs(cp4prefsGadgets[GD_gc2plist],cp4prefsWnd,NULL,GTLV_Labels,(ULONG)&c2plist,GTLV_Selected,(ULONG)sel,GTLV_MakeVisible,(ULONG)sel,TAG_DONE);
    if(sel!=~0) {
        struct Node *node;
        int i;
        node=c2plist.lh_Head;
        for(i=0;i<sel;i++) node=node->ln_Succ;
        getc2pinfos(((struct c2ps *)node)->filename);
        GT_SetGadgetAttrs(cp4prefsGadgets[GD_gc2pauthortxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)c2pAuthor,TAG_DONE);
        GT_SetGadgetAttrs(cp4prefsGadgets[GD_gc2pnametxt],cp4prefsWnd,NULL,GTTX_Text,(ULONG)c2pName,TAG_DONE);
        if(c2pInfo!=NULL&&strlen(c2pInfo)>3) setupinfo(c2pInfo);
    }
  } else {
    p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_0219));
    pERROR=1;
  }

  // Set IEC menuitem
  if(opt_iec==0) setcheckmark(M_PREFS,M2_IEC,FALSE);
  else setcheckmark(M_PREFS,M2_IEC,TRUE);

  // Set d64 related
  strcpy(name2,GetStr(MSG_0300));
  strcat(name2," __");
  l=strlen(name2);
  if(l>=20) {
    name2[16]=' ';
    name2[17]='_';
    name2[18]='_';
    name2[19]='\0';
    l=strlen(name2);
  }
  l-=2;
  for(i=8;i<12;i++) {
    name2[l]=(i/10)+'0';
    name2[l+1]=(i%10)+'0';
    strcpy(drivecyc[i-8],name2);
  }
  strcpy(drivetypecyc[0],GetStr(MSG_0303));   // none
  strcpy(drivetypecyc[1],GetStr(MSG_0302));   // real1541
  strcpy(drivetypecyc[2],GetStr(MSG_0301));   // d64 + name2!!
  GT_SetGadgetAttrs(cp4prefsGadgets[GD_gdriveselcyc],cp4prefsWnd,NULL,GTCY_Labels,(ULONG)drivecyc,TAG_DONE);
  if(opt_iec!=0) GT_SetGadgetAttrs(cp4prefsGadgets[GD_gdriveselcyc],cp4prefsWnd,NULL,GA_Disabled,FALSE,TAG_DONE);
  actdrive=8;
  updatecyc(opt_drive08,8);
} // setupgui

/*
 * a gui állása alapján beállítja a return cuccokat (use/save)
 */
static void getgui(void) {
static char s[30];
  ULONG v,v2;
  char *oldc2p;
  int i;
  unsigned long oldscrmode;
  unsigned long oldoverscan;
  int drnow[4];

  if(opt_nosound==0&&opt_sid==0) freeaudio();
  if(opt_nosound==0&&opt_sid==1) endsid();
  opt_nosound=1;
  opt_sid=0;
  AddOption("SID","NO");
  AddOption("SOUND","NO");
//  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gsnd],cp4prefsWnd,NULL,GTCB_Checked,(ULONG)&v,TAG_DONE);
  v=getcheckmark(M_PREFS,M2_SOUND);
  v2=getcheckmark(M_PREFS,M2_SID);
  if(v==TRUE&&v2==TRUE) {         // SID
    opt_nosound=0;
    opt_sid=1;
    AddOption("SID","YES");
    AddOption("SOUND","YES");
    if(0!=initsid()) {
      opt_nosound=1;
      p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_021A));
    }
  }
  if(v==TRUE&&v2==FALSE) {        // TED
    opt_nosound=0;
    AddOption("SOUND","YES");
    if(0!=initaudio()) {
      opt_nosound=1;
      p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_021B));
    }
  }

//  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gtwoframe],cp4prefsWnd,NULL,GTCB_Checked,(ULONG)&v,TAG_DONE);
  v=getcheckmark(M_PREFS,M2_TWOFRAME);
  if(v==TRUE) {
    opt_twoscr=~0;
    otvenedsec=otvened2;
    hz80=hz2;
    AddOption("TWOFRAME","YES");
  } else {
    opt_twoscr=~0x40000000;
    otvenedsec=otvened1;
    hz80=hz1;
    AddOption("TWOFRAME","NO");
  }

//  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gsplim],cp4prefsWnd,NULL,GTCB_Checked,(ULONG)&v,TAG_DONE);
  v=getcheckmark(M_PREFS,M2_SPLIM);
  if(v==TRUE) {
    opt_limit=1;
    AddOption("SPEEDLIMIT","YES");
  } else {
    opt_limit=0;
    otvenedsec=0;
    AddOption("SPEEDLIMIT","NO");
  }

//  GT_GetGadgetAttrs(cp4prefsGadgets[GD_gswapjoy],cp4prefsWnd,NULL,GTCB_Checked,(ULONG)&v,TAG_DONE);
  v=getcheckmark(M_PREFS,M2_SWAPJOY);
  if(v==TRUE) {
    joykeyindex=8;
    joyfiredown=0x7f;
    joyfireup=0x80;
    AddOption("JOY","SWAPPED");
  } else {
    joykeyindex=9;
    joyfiredown=0xbf;
    joyfireup=0x40;
    AddOption("JOY","NORMAL");
  }

  v=getcheckmarkfull(M_PREFS,M2_XPK,M2_XPK_ON);
  if(v==TRUE) {
    opt_xpk=1;
    AddOption("XPK","YES");
  } else {
    opt_xpk=0;
    AddOption("XPK","NO");
  }

  opt_xpkeff=50;
  AddOption("XPKEFFECIENT","50");
  v=getcheckmarkfull(M_PREFS,M2_XPK,M2_XPK_FAST);
  if(v==TRUE) {
    opt_xpkeff=0;
    AddOption("XPKEFFECIENT","0");
  }
  v=getcheckmarkfull(M_PREFS,M2_XPK,M2_XPK_BEST);
  if(v==TRUE) {
    opt_xpkeff=100;
    AddOption("XPKEFFECIENT","100");
  }

  // OnScreenDisplay menuitem
  v=getcheckmark(M_PREFS,M2_OSD);
  if(v==TRUE) {
    opt_onscreendisplay=1;
    AddOption("ONSCREENDISPLAY","YES");
  } else {
    opt_onscreendisplay=0;
    AddOption("ONSCREENDISPLAY","NO");
  }

  if(TRUE==getcheckmark(M_PREFS,M2_PERC)) {
    opt_percent=1;
    AddOption("PERCENT","YES");
  } else {
    opt_percent=0;
    AddOption("PERCENT","NO");
  }


  skipflag=0;
  skiptime=0;
  if(TRUE==getcheckmark(M_PREFS,M2_REALTIME)) {
    opt_realtime=1;
    AddOption("REALTIME","YES");
  } else {
    opt_realtime=0;
    AddOption("REALTIME","NO");
  }

  // get IEC
  if(TRUE==getcheckmark(M_PREFS,M2_IEC)) {
    opt_iec=1;
    AddOption("IEC","YES");
    if(opt_drive08==1||opt_drive09==1||opt_drive10==1||opt_drive11==1) {
      if(!IECBase) IECBase=(struct iecbase *)OpenLibrary("iec.library",0);
      if(IECBase==NULL) {
        p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_0220));
        if(opt_drive08==1) opt_drive08=0;
        if(opt_drive09==1) opt_drive09=0;
        if(opt_drive10==1) opt_drive10=0;
        if(opt_drive11==1) opt_drive11=0;
      }
    }
  } else {
    if(IECBase) { CloseLibrary((struct Library *)IECBase); IECBase=NULL; }
    opt_iec=0;
    AddOption("IEC","NO");
  }

  if(newscrmode!=opt_scrmode||newoverscan!=opt_overscan||strcmp(newc2pname,opt_c2p)!=0||strcmp(opt_palettefile,newpalette)!=0) {
    if(newpalette!=NULL) {
      AddOption("PALETTE",newpalette);
      opt_palettefile=GetOption("PALETTE","NONE");
      loadpalette(opt_palettefile,cp4prefsWnd);
    }
    freedisplay();
    pscr=1;
    oldc2p=opt_c2p;
    oldscrmode=opt_scrmode;
    oldoverscan=opt_overscan;
    opt_c2p=newc2pname;
    opt_scrmode=newscrmode;
    opt_overscan=newoverscan;
    if(0!=initdisplay()) {
      p4req1(cp4prefsWnd,P4_WARNING,GetStr(MSG_0221));
      opt_scrmode=oldscrmode;
      opt_overscan=oldoverscan;
      opt_c2p=oldc2p;
      if(0!=initdisplay()) {
        p4req1(cp4prefsWnd,P4_ERROR,GetStr(MSG_0222));
        pERROR=1;
      }
    } else {
      opt_c2p=malloc(strlen(newc2pname)+1);
      strcpy(opt_c2p,newc2pname);
      AddOption("C2P",opt_c2p);
      sprintf(s,"%ld",newoverscan);
      AddOption("OVERSCAN",s);
      sprintf(s,"%ld",newscrmode);
      AddOption("MODEID",s);
    }
  }

  // D64 related
  drnow[0]=opt_drive08;
  drnow[1]=opt_drive09;
  drnow[2]=opt_drive10;
  drnow[3]=opt_drive11;
  for(i=8;i<12;i++) {
    if(drivestore[i-8]==2&&drnow[i-8]!=2) SOFT_Close(i);
    if(drivestore[i-8]!=2&&drnow[i-8]==2) SOFT_Init(i);
    if(drivestore[i-8]==2&&drnow[i-8]==2) {
      driveoptname[8]=(i/10)+'0';
      driveoptname[9]=(i%10)+'0';
      if(strcmp(d64names[i-8],GetOption(driveoptname,"NONE"))!=0) {
        SOFT_Close(i);
        SOFT_Init(i);
      }
    }
  }
} // getgui


/*
 * called from 7501.asm
 */
int prefsguis(void) {
  int r;

  nojoy=0;
  Scr=c2pv->c2p_Scr;
  userblank=0;
  soundpause();
  freejoy();
  c2pv->c2p_sleep();
  ScreenToFront(Scr);
  r=prefsgui();
  getjoy();
  if((r&2)==0) {
    ScreenToFront(cp4Scr);
    ActivateWindow(cp4Window);
    soundresume();
  }
  c2pv->c2p_awake();
  return(r);
}

/*
 * called from dbg.c
 */
int prefsgui(void) {
static char dummy[]="";
  int stop=1;
  char *dn,*nw;

  drivestore[0]=opt_drive08;
  drivestore[1]=opt_drive09;
  drivestore[2]=opt_drive10;
  drivestore[3]=opt_drive11;
  dn=GetOption("D64NAME_08","NONE");
  d64names[0]=dummy;
  if(strcmp(dn,"NONE")!=0) {
    if(NULL!=(nw=malloc(strlen(dn)+1))) { strcpy(nw,dn); d64names[0]=nw; }
  }
  dn=GetOption("D64NAME_09","NONE");
  d64names[1]=dummy;
  if(strcmp(dn,"NONE")!=0) {
    if(NULL!=(nw=malloc(strlen(dn)+1))) { strcpy(nw,dn); d64names[1]=nw; }
  }
  dn=GetOption("D64NAME_10","NONE");
  d64names[2]=dummy;
  if(strcmp(dn,"NONE")!=0) {
    if(NULL!=(nw=malloc(strlen(dn)+1))) { strcpy(nw,dn); d64names[2]=nw; }
  }
  dn=GetOption("D64NAME_11","NONE");
  d64names[3]=dummy;
  if(strcmp(dn,"NONE")!=0) {
    if(NULL!=(nw=malloc(strlen(dn)+1))) { strcpy(nw,dn); d64names[3]=nw; }
  }

  InitKMapHook();
  overnames[0]=GetStr(MSG_0223);
  overnames[1]=GetStr(MSG_0224);
  overnames[2]=GetStr(MSG_0225);
  overnames[3]=GetStr(MSG_0226);
  overnames[4]=GetStr(MSG_0227);

  if(opt_keymap!=NULL) {
    old_keymap=old_keymaps;
    strcpy(old_keymap,opt_keymap);
  } else old_keymap=NULL;
  nojoy=0;
  Scr=c2pv->c2p_Scr;
  ret=0;
  pERROR=0;
//  newscrmode=(opt_scrmode==~0?0:opt_scrmode);
//  newoverscan=(opt_overscan==~0?0:opt_overscan);
  newscrmode=opt_scrmode;
  newoverscan=opt_overscan;
  newc2pname=opt_c2p;
  if(NULL!=(newpalette=malloc(strlen(opt_palettefile)+1))) strcpy(newpalette,opt_palettefile);
  else newpalette=NULL;
  pscr=0;
  if(0==SetupScreen()) {
    if(0==Opencp4prefsWindow()) {
      setupgui();
      if(pERROR==0) {
        while(stop!=0) {
          WaitPort(cp4prefsWnd->UserPort);
          stop=Handlecp4prefsIDCMP();
        }
      }
      Closecp4prefsWindow();
    }
    CloseDownScreen();
  }
  freec2ps();
  if(d64names[0]!=dummy) free(d64names[0]);
  if(d64names[1]!=dummy) free(d64names[1]);
  if(d64names[2]!=dummy) free(d64names[2]);
  if(d64names[3]!=dummy) free(d64names[3]);
  if(newpalette!=NULL) free(newpalette);
  newpalette=NULL;
  getc2pinfos(NULL);
  setupinfo(NULL);
  if(pERROR!=0) tederror(GetStr(MSG_0228));
  return(ret);
}
