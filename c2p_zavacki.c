/* :ts=4                            c2p_zavacki.c
 *
 *    cp4 - Commodore C+4 emulator
 *    Copyright (C) 1998 G�ti Gergely
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
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "cp4_ver.h"

#define C2P_VERSION     "1"
#define C2P_REVISION    "3"
#define C2P_AUTHOR      "Zavacki Ferenc"
#define C2P_NAME        "Zavacki"
#define C2P_BUFFERING   3
#define C2P_DATE        DATE
#define C2P_LOCALIZE

#include "c2p_module.c"
#include "c2p_palette.c"



extern void REGARGS c2p(    REG(a0,void *chunkyaddr),
                            REG(a1,void *deltaaddr),
                            REG(d0,void *planaraddr) );
extern void REGARGS c2pf(   REG(a0,void *chunkyaddr),
                            REG(a1,void *planaraddr) );
extern void REGARGS c2pinit(REG(d0,int planewidth),
                            REG(a0,unsigned char *linedeltavec));


struct GfxBase *GfxBase=NULL;
struct IntuitionBase *IntuitionBase=NULL;
static struct BitMap *bmap[3];
static void *planars[3];
static unsigned char *linedelta;
static struct Screen *scr=NULL;
static struct Window *win=NULL;
static UWORD *nopointer=NULL;
static struct ScreenBuffer *scrb[3];
static int winsleeped=0;
static struct Requester InvisibleRequester;

#define MSG_INFO 0
#define MSG_NOSCRMODE 1
#define MSG_NOGFX 2
#define MSG_NOINTUI 3
#define MSG_NOSCR 4
#define MSG_NOWIN 5
#define MSG_NOMEM 6
#define MSG_WRONGBITMAPTYPE 7
#define MSG_NOHAMEHB 8

static char *defstr[]={
    "This is an effecient ChunkyToPlanar from Ferenc Zavacki's workshop. It's a CPU-only version, and optimized for 68030/50.",
    "No ScreenMode Specified",
    "Can't open graphics.library V39",
    "Can't open intuition.library V37",
    "Can't open Screen",
    "Can't open Window",
    "Not enough memory",
    "Wrong BitMap Type",
    "EHB and HAM isn't supported",
    NULL
};

char *SAVEDS minfo(void) {
static char in[1024];
    char *s;
    c2p_OpenCatalog(defstr);
    s=STR(MSG_INFO);
    c2p_strncpy(in,s,1023);
    c2p_CloseCatalog();
    return(in);
}

char *SAVEDS minit(ULONG scrmode, ULONG overscan, unsigned char *linedeltatab) {
    ULONG bf=BMF_CLEAR|BMF_DISPLAYABLE|BMF_INTERLEAVED;
    int bmx,planes=7;

    c2p_OpenCatalog(defstr);

    linedelta=linedeltatab;
    if(scrmode==~0) return(STR(MSG_NOSCRMODE));
    if((scrmode&0x880)!=0) return(STR(MSG_NOHAMEHB));

    if((GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",39))==NULL) return(STR(MSG_NOGFX));
    if(!(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",37))) return(STR(MSG_NOINTUI));

    /* Allocate mem for nopointer
     */
    if(NULL==(nopointer=AllocVec(24,MEMF_CHIP|MEMF_CLEAR))) return(STR(MSG_NOMEM));

    /* Create palette
     */
    if(vec.c2p_Palette!=NULL) {
        int i;
        ULONG *p,l;
        p=Palette;
        for(p+=1,i=0;i<384;) {
            l=0;
            l=(vec.c2p_Palette[i]<<8)|vec.c2p_Palette[i];
            l|=(l<<16);
            *p++=l;
            i++;
            l=(vec.c2p_Palette[i]<<8)|vec.c2p_Palette[i];
            l|=(l<<16);
            *p++=l;
            i++;
            l=(vec.c2p_Palette[i]<<8)|vec.c2p_Palette[i];
            l|=(l<<16);
            *p++=l;
            i++;
        }
    }

    /* Allocate bitmaps
     */
    bmap[0]=AllocBitMap(SCRWIDTH,SCRHEIGHT,planes,bf,NULL);
    bmap[1]=AllocBitMap(SCRWIDTH,SCRHEIGHT,planes,bf,NULL);
    bmap[2]=AllocBitMap(SCRWIDTH,SCRHEIGHT,planes,bf,NULL);
    if(bmap[0]==NULL||bmap[1]==NULL||bmap[2]==NULL) return(STR(MSG_NOMEM));
    bmx=GetBitMapAttr(bmap[0],BMA_WIDTH)/8;
    bf=BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_STANDARD;
    if(bf!=GetBitMapAttr(bmap[0],BMA_FLAGS)) return(STR(MSG_WRONGBITMAPTYPE));
    if(bf!=GetBitMapAttr(bmap[1],BMA_FLAGS)) return(STR(MSG_WRONGBITMAPTYPE));
    if(bf!=GetBitMapAttr(bmap[2],BMA_FLAGS)) return(STR(MSG_WRONGBITMAPTYPE));

    c2pinit(bmx,linedelta);

    planars[0]=bmap[0]->Planes[0];
    planars[1]=bmap[1]->Planes[0];
    planars[2]=bmap[2]->Planes[0];

    /* Open Screen
     */
    if(NULL==(scr=OpenScreenTags(NULL,
                SA_DisplayID, scrmode,
                SA_Depth, planes,
                SA_ShowTitle,FALSE,
                SA_Quiet, TRUE,
                SA_Width,SCRWIDTH,
                SA_Height,SCRHEIGHT,
                SA_BitMap, (ULONG)bmap[0],
                SA_Behind, TRUE,
                SA_Colors32, (ULONG)Palette,
                SA_Overscan, overscan,
                TAG_DONE,0L )))
        return(STR(MSG_NOSCR));
    vec.c2p_Scr=scr;

    /* Open window
     */
    if((win=(struct Window *)OpenWindowTags(NULL,
                WA_NoCareRefresh, TRUE,
                WA_Activate, TRUE,
                WA_Borderless, TRUE,
                WA_Backdrop, TRUE,
                WA_CustomScreen, (ULONG)scr,
                WA_RMBTrap, TRUE,
                WA_RptQueue, 25,
                TAG_DONE,0L ))==NULL)
        return(STR(MSG_NOWIN));
    vec.c2p_Win=win;
    winsleeped=0;

    /* set nopointer
     */
    SetPointer(win,nopointer,1,16,0,0);

    /* Allocate screen buffers
     */
    scrb[0]=AllocScreenBuffer(scr,bmap[0],NULL);
    scrb[1]=AllocScreenBuffer(scr,bmap[1],NULL);
    scrb[2]=AllocScreenBuffer(scr,bmap[2],NULL);
    if(scrb[0]==NULL||scrb[1]==NULL||scrb[2]==NULL) return(STR(MSG_NOMEM));
    scrb[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort=NULL;
    scrb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort=NULL;
    scrb[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort=NULL;
    scrb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort=NULL;
    scrb[2]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort=NULL;
    scrb[2]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort=NULL;

    return(NULL);
} // init


void SAVEDS mfree(void) {
    mawake();
    if(GfxBase) WaitTOF();
    if(scrb[0]) { FreeScreenBuffer(scr,scrb[0]); scrb[0]=NULL; }
    if(scrb[1]) { FreeScreenBuffer(scr,scrb[1]); scrb[1]=NULL; }
    if(scrb[2]) { FreeScreenBuffer(scr,scrb[2]); scrb[2]=NULL; }
    if(win) ClearPointer(win);
    if(nopointer) { FreeVec(nopointer); nopointer=NULL; }
    if(win) { CloseWindow(win); win=NULL; }
    vec.c2p_Win=NULL;
    if(scr) { CloseScreen(scr); scr=NULL; }
    vec.c2p_Scr=NULL;
    if(bmap[0]) { FreeBitMap(bmap[0]); bmap[0]=NULL; }
    if(bmap[1]) { FreeBitMap(bmap[1]); bmap[1]=NULL; }
    if(bmap[2]) { FreeBitMap(bmap[2]); bmap[2]=NULL; }
    if(GfxBase) CloseLibrary((struct Library *)GfxBase);
    if(IntuitionBase) { CloseLibrary((struct Library *)IntuitionBase); IntuitionBase=NULL; }
    c2p_CloseCatalog();
    return;
} // free


int SAVEDS mdo(unsigned char *chunky,unsigned char *delta,int numscreen) {
    c2p(chunky,delta,planars[numscreen]);
    ChangeScreenBuffer(scr,scrb[numscreen]);
    return(RET_OK);
} // do


int SAVEDS mdofull(unsigned char *chunky,int numscreen) {
    c2pf(chunky,planars[numscreen]);
    ChangeScreenBuffer(scr,scrb[numscreen]);
    return(RET_OK);
} // dofull

int SAVEDS mdont(void) {
    return(RET_OK);
} // dont


void SAVEDS msleep(void) {
    if(win) {
        InitRequester(&InvisibleRequester);
        Request(&InvisibleRequester,win);
        SetWindowPointer(win,WA_BusyPointer,TRUE,TAG_DONE);
        winsleeped=1;
    }
}


void SAVEDS mawake(void) {
    if(winsleeped) {
        if(win) {
            EndRequest(&InvisibleRequester,win);
            SetWindowPointerA(win,NULL);
            winsleeped=0;
            SetPointer(win,nopointer,1,16,0,0);
        }
    }
}
