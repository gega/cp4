/* :ts=4                            c2p_module.c
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
#ifndef C2P_MODULE_C
#define C2P_MODULE_C

#ifndef C2P_MODULE_H
 #include "c2p_module.h"
#endif

#define C2P_INTERFACE   (4)

#ifdef C2P_LOCALIZE

 #ifdef C2P_INFO
  #error If you define C2P_LOCALIZE, you must use minfo() instead of C2P_INFO.
 #endif

 #ifndef C2P_WANTSYSBASE
  #define C2P_WANTSYSBASE
 #endif

 #define C2P_CATALOGNAME    "cp4_" ## C2P_NAME ## ".catalog"

 #ifndef C2P_BUILTINLANGUAGE
  #define C2P_BUILTINLANGUAGE   "english"
 #endif

 #include <libraries/locale.h>
 #include <proto/locale.h>
 #include <proto/intuition.h>
 #include <proto/dos.h>

 void c2p_OpenCatalog(char **DefaultStrings);
 char *c2p_GetStr(long StringNum);
 void c2p_CloseCatalog(void);

 #define STR(n) c2p_GetStr(n)

 extern struct IntuitionBase *IntuitionBase;

 struct LocaleBase *LocaleBase;
 struct Catalog *c2p_Catalog=0L;
 static int c2p_NumberOfMessages=0;
 static char **c2p_DefaultStrings=0L;

#endif      // C2P_LOCALIZE

#ifdef C2P_INFO
 #warning C2P_INFO is obsolote, use minfo() instead.
#endif

#ifdef C2P_WANTSYSBASE
 #include <exec/types.h>
 #include <proto/exec.h>
#else
 #warning You have to define SysBase yourself.
#endif

#define SCRWIDTH    352
#define SCRHEIGHT   283
#define SCRSIZE     (SCRWIDTH*SCRHEIGHT)
#define NUMSCR      3
#define C2P_NOMSG   "\0"
#define C2P_NOMEM   "Not Enough Memory"
#define RET_ERROR   -2
#define RET_NEWWIN  -1
#define RET_OK      0
#define RET_DEBUG   1
#define RET_PREFS   2
#define RET_RESET   3
#define RET_HRESET  4
#define RET_QUIT    5

char *SAVEDS minit(ULONG scrmode, ULONG overscan, unsigned char *linedeltatab);
void SAVEDS mfree(void);
int SAVEDS mdo(unsigned char *chunky,unsigned char *delta,int numscreen);
int SAVEDS mdofull(unsigned char *chunky,int numscreen);
int SAVEDS mdont(void);
void SAVEDS msleep(void);
void SAVEDS mawake(void);
#ifndef C2P_INFO
 char *SAVEDS minfo(void);
#endif
#ifdef C2P_DIRECT
 struct direct_info *SAVEDS mlock(void);
 void SAVEDS munlock(REG(d0,struct direct_info *lock));
#endif

static char *SAVEDS iinit(ULONG sm, ULONG ov, unsigned char *ld);
static int SAVEDS idofull(unsigned char *chunky,unsigned char *delta,int numscreen);

static int DummyGetOptionInt(char *name,int defval);
static void DummyAddOptionInt(char *name,int value);
static char *DummyGetOptionStr(char *name,char *defval);
static void DummyAddOptionStr(char *name,char *value);

static int SAVEDS c2p_internal(int);

static struct c2pvec vec;

static ULONG c2p_sm;
static ULONG c2p_ov;
static unsigned char *c2p_ld;

#ifdef C2P_WANTSYSBASE
 struct ExecBase *SysBase;
#endif

#ifndef C2P_AUTHOR
 #define C2PI_COPYRIGHT ""
 #warning C2P_AUTHOR is not defined.
#else
 #define C2PI_COPYRIGHT " Copyright © by " ## C2P_AUTHOR
#endif

#ifdef __SASC
 #define C2P_VERSTAG "\0$VER:  " ## C2P_NAME ## " " ## C2P_VERSION ## "." ## C2P_REVISION ## " " ## __AMIGADATE__ ## "   " ## C2PI_COPYRIGHT
#else
 #ifndef C2P_DATE
  #define C2P_VERSTAG "\0$VER:  " ## C2P_NAME ## " " ## C2P_VERSION ## "." ## C2P_REVISION ## " (" ## __DATE__ ## ")" ## "   " ## C2PI_COPYRIGHT
 #else
  #define C2P_VERSTAG "\0$VER:  " ## C2P_NAME ## " " ## C2P_VERSION ## "." ## C2P_REVISION ## " (" ## C2P_DATE ## ")" ## "   " ## C2PI_COPYRIGHT
 #endif     // C2P_DATE
#endif      // __SASC
static char c2p_VersTag[]=C2P_VERSTAG;

struct c2pvec REGARGS SAVEDS *startup(REG(d0,unsigned long m)) {
static char iVer[]=C2P_VERSION;
static char iRev[]=C2P_REVISION;
static char iNam[]=C2P_NAME;
#ifdef C2P_AUTHOR
 static char iAut[]=C2P_AUTHOR;
#else
 static char iAut[]="???";
#endif
#ifdef C2P_INFO
 static char iInf[]=C2P_INFO;
#else
 static char *iInf=NULL;
#endif

    if(m!=C2P_MAGIC) return((struct c2pvec *)-1);
#ifdef C2P_WANTSYSBASE
    SysBase=*((struct ExecBase **)4L);
#endif
#ifndef C2P_INFO
    if(iInf==NULL) iInf=minfo();
#endif
    vec.c2p_Interface=C2P_INTERFACE;
    vec.c2p_init=iinit;
    vec.c2p_free=mfree;
    vec.c2p_do=mdo;
    vec.c2p_dofull=idofull;
    vec.c2p_sleep=msleep;
    vec.c2p_awake=mawake;
    vec.c2p_Info=iInf;
    vec.c2p_Author=iAut;
    vec.c2p_Version=iVer;
    vec.c2p_Revision=iRev;
    vec.c2p_Name=iNam;
    vec.c2p_Scr=NULL;
    vec.c2p_Win=NULL;
    vec.c2p_GetOptionInt=DummyGetOptionInt;
    vec.c2p_AddOptionInt=DummyAddOptionInt;
    vec.c2p_GetOptionStr=DummyGetOptionStr;
    vec.c2p_AddOptionStr=DummyAddOptionStr;
    vec.c2p_MsgList.lh_Head=(struct Node *)&vec.c2p_MsgList.lh_Tail;
    vec.c2p_MsgList.lh_Tail=NULL;
    vec.c2p_MsgList.lh_TailPred=(struct Node *)&vec.c2p_MsgList.lh_Head;
    vec.c2p_MsgList.lh_Type=0;
#ifdef C2P_BUFFERING
    vec.c2p_Buffering=C2P_BUFFERING;
    if(vec.c2p_Buffering<0||vec.c2p_Buffering>3) vec.c2p_Buffering=0;
#else
    vec.c2p_Buffering=0;
#endif
    vec.c2p_Speed=-1;
#ifdef C2P_NOSPEED
    vec.c2p_NoSpeed=1;
#else
    vec.c2p_NoSpeed=0;
#endif
    c2p_VersTag[0]='\0';
    vec.c2p_dont=mdont;
    vec.c2p_internal=c2p_internal;
#ifdef C2P_NORENDER
    vec.c2p_Render=0;
#else
    vec.c2p_Render=1;
#endif
    vec.c2p_Palette=NULL;
#ifndef C2P_NOBORDER
    vec.c2p_Border=1;
#else
    vec.c2p_Border=0;
#endif
#ifdef C2P_DIRECT
    vec.c2p_Direct=1;
    vec.c2p_lock=mlock;
    vec.c2p_unlock=munlock;
#else
    vec.c2p_Direct=0;
    vec.c2p_lock=0L;
    vec.c2p_unlock=0L;
#endif
    vec.c2p_Led=0;
    return(&vec);
} // startup()

static int SAVEDS idofull(unsigned char *chunky,unsigned char *delta,int numscreen) {
    return(mdofull(chunky,numscreen));
} // idofull()

static int DummyGetOptionInt(char *name,int defval) { return(-1); }
static void DummyAddOptionInt(char *name,int value) { return; }
static char *DummyGetOptionStr(char *name,char *defval) { return(NULL); }
static void DummyAddOptionStr(char *name,char *value) { return; }

static char *SAVEDS iinit(ULONG sm, ULONG ov, unsigned char *ld) {
    c2p_sm=sm;  c2p_ov=ov;  c2p_ld=ld;
    return(minit(sm,ov,ld));
}

/* Locale functions, if required
 */
#ifdef C2P_LOCALIZE

 static char *c2p_ShowLocReq(long StringNum) {
 static char nostr[]="***NOMSG";
 static struct EasyStruct es_req={
    sizeof(struct EasyStruct),
    0,
    "Locale error",
    "Can't find string number %ld",
    "Ok"
 };
 struct IntuitionBase *storeIntuiBase;
    storeIntuiBase=IntuitionBase;
    if(0L!=(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",36))) {
        EasyRequest(0L,&es_req,0L,(ULONG)StringNum);
        CloseLibrary((struct Library *)IntuitionBase);
    }
    IntuitionBase=storeIntuiBase;
    return(nostr);
 } // c2p_ShowLocReq()

 static void c2p_ShowReq(char *t,char *b) {
 static struct EasyStruct es_req={ sizeof(struct EasyStruct),0, 0L, 0L, "Ok" };
 struct IntuitionBase *storeIntuiBase;
    if(b==0L||t==0L) return;
    storeIntuiBase=IntuitionBase;
    if(0L!=(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",36))) {
        es_req.es_Title=t;
        es_req.es_TextFormat=b;
        EasyRequestArgs(0L,&es_req,0L,0L);
        CloseLibrary((struct Library *)IntuitionBase);
    }
    IntuitionBase=storeIntuiBase;
 } // c2p_ShowReq()

 void c2p_OpenCatalog(char *DefaultStrings[]) {
 static struct DosLibrary *DOSBase;
    long i;
    if((DOSBase=(struct DosLibrary *)OpenLibrary("dos.library",0L))) {
        Delay(4);
        CloseLibrary((struct Library *)DOSBase);
    }
    if(DefaultStrings==NULL) return;
    for(i=0;DefaultStrings[i]!=NULL;i++);
    c2p_DefaultStrings=DefaultStrings;
    c2p_NumberOfMessages=i-1;
    if(i==0) return;
    if(!LocaleBase) LocaleBase=(struct LocaleBase *)OpenLibrary("locale.library",37);
    if(LocaleBase!=0L&&c2p_Catalog==0L) {
        c2p_Catalog=OpenCatalog(0L,(char *)C2P_CATALOGNAME,
            OC_BuiltInLanguage, (unsigned long)C2P_BUILTINLANGUAGE,
            OC_Version, 0L,
            TAG_DONE,0L);
    }
 } // c2p_OpenCatalog()

 void c2p_CloseCatalog(void) {
    if(LocaleBase!=0L) CloseCatalog(c2p_Catalog);
    c2p_Catalog=0L;
    if(LocaleBase) CloseLibrary((struct Library *)LocaleBase);
    c2p_NumberOfMessages=0;
 } // c2p_CloseCatalog()

 char *c2p_GetStr(long StringNum) {
    char *rs=0L;
    if(StringNum>=0&&StringNum<=c2p_NumberOfMessages&&LocaleBase!=0L) rs=GetCatalogStr(c2p_Catalog,StringNum+1,c2p_DefaultStrings[StringNum]);
    else if(StringNum<=c2p_NumberOfMessages) rs=c2p_DefaultStrings[StringNum];
    else rs=c2p_ShowLocReq(StringNum);
    return(rs);
 } // c2p_GetStr()

 INLINE void c2p_strncpy(char *to,char *fr,int l) {
    while('\0'!=(*to++=*fr++)&&--l>=0);
 } // c2p_strncpy()

#endif          // C2P_LOCALE

static int SAVEDS c2p_internal(int f) {
    int r=0;

    switch(f) {
#ifdef C2P_LOCALIZE
        case C2PIF_MODLOCALE : {
            char *rs;
            c2p_CloseCatalog();
            mfree();
            if(NULL!=(rs=minit(c2p_sm,c2p_ov,c2p_ld))) {
                c2p_ShowReq("Fatal error",rs);
                r=RET_ERROR;
            }
        }
            break;
#endif
    }
    return(r);
} // c2p_internal()

#endif
