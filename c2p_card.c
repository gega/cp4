/* :ts=4							c2p_card.c
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
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/Picasso96.h>
#include <exec/memory.h>

#include "cp4_ver.h"

#define C2P_VERSION		"2"
#define C2P_REVISION	"0"
#define C2P_AUTHOR		"gega <Gáti Gergely>"
#define C2P_NAME		"Card"
#define C2P_BUFFERING	3
#define C2P_DATE		DATE
#define C2P_LOCALIZE

#include "c2p_module.c"
#include "c2p_palette.c"


struct GfxBase *GfxBase=NULL;
struct IntuitionBase *IntuitionBase=NULL;
struct Library *P96Base=NULL;
static struct BitMap *bmapwpx;
static struct RastPort *tmpraswpx;
static struct RastPort *raswpx;
static struct ScreenBuffer *scrb[3];
static UWORD *nopointer=NULL;
static struct Screen *scr=NULL;
static struct Window *win=NULL;
static int winsleeped=0;
static struct Requester InvisibleRequester;
static ULONG Palette8[770];
static int gfx;

// for gfx
#define GFX_AGA		0
#define GFX_P96		1
#define GFX_CGX		2

#define MSG_INFO 0
#define MSG_NOSCRMODE 1
#define MSG_NOGFX 2
#define MSG_NOINTUI 3
#define MSG_NOSCR 4
#define MSG_NOWIN 5
#define MSG_NOMEM 6
#define MSG_NOHAMEHB 7

static char *defstr[]={
	"This is a driver for graphics cards. Direct support for Picasso96 & CyberGFX systems. It's working on AGA too, but it's use the WritePixelArray8() function, and it's deadly slow on AGA.",
	"No ScreenMode Specified",
	"Can't open graphics.library V39",
	"Can't open intuition.library V39",
	"Can't open Screen",
	"Can't open Window",
	"Not enough memory",
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
	int planes=8,i;

	c2p_OpenCatalog(defstr);

	if(scrmode==~0) return(STR(MSG_NOSCRMODE));
	if((scrmode&0x880)!=0) return(STR(MSG_NOHAMEHB));

	/* Allocate mem for nopointer
	 */
	if(NULL==(nopointer=AllocVec(24,MEMF_CHIP|MEMF_CLEAR))) return(STR(MSG_NOMEM));

	if((GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",39))==NULL) return(STR(MSG_NOGFX));
	if(!(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",39))) return(STR(MSG_NOINTUI));
	gfx=GFX_AGA;
	if((P96Base=OpenLibrary("Picasso96API.library",2))) gfx=GFX_P96;

	/* Create palette
	 */
	if(vec.c2p_Palette!=NULL) {
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
	Palette8[0]=256<<16;
	for(i=0;i<384;i++) Palette8[1+i]=Palette[1+i];
	for(i=0;i<384;i++) Palette8[1+i+384]=Palette[1+i];

	/* Allocate bitmaps
	 */
	bmapwpx=AllocBitMap(SCRWIDTH,1,8,BMF_CLEAR,NULL);
	if(bmapwpx==NULL) return(STR(MSG_NOMEM));
	tmpraswpx=AllocVec(sizeof(struct RastPort),MEMF_ANY);
	if(tmpraswpx==NULL) return(STR(MSG_NOMEM));
	InitRastPort(tmpraswpx);
	tmpraswpx->BitMap=bmapwpx;
	raswpx=AllocVec(sizeof(struct RastPort),MEMF_ANY);
	if(raswpx==NULL) return(STR(MSG_NOMEM));
	InitRastPort(raswpx);

	/* Open Screen
	 */
	if(gfx==GFX_AGA) {
		if(NULL==(scr=OpenScreenTags(NULL,
				SA_DisplayID, scrmode,
				SA_Depth, planes,
				SA_ShowTitle,FALSE,
				SA_Quiet, TRUE,
               	SA_Width, SCRWIDTH,
               	SA_Height, SCRHEIGHT,
               	SA_Behind, TRUE,
               	SA_Colors32, (ULONG)Palette,
				SA_Overscan, overscan,
				SA_Type, CUSTOMSCREEN,
				TAG_DONE,0L )))
			return(STR(MSG_NOSCR));
	} else if(gfx==GFX_P96) {
		if(NULL==(scr=p96OpenScreenTags(
				P96SA_DisplayID, scrmode,
				P96SA_Depth, planes,
				P96SA_ShowTitle,FALSE,
				P96SA_Quiet, TRUE,
               	P96SA_Width, SCRWIDTH,
               	P96SA_Height, SCRHEIGHT,
               	P96SA_Behind, TRUE,
               	P96SA_Colors32, (ULONG)Palette,
				P96SA_Type, CUSTOMSCREEN,
				TAG_DONE,0L )))
			return(STR(MSG_NOSCR));
	} else return(STR(MSG_NOSCR));
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
	scrb[0]=AllocScreenBuffer(scr,NULL,NULL);
	scrb[1]=AllocScreenBuffer(scr,NULL,NULL);
	scrb[2]=AllocScreenBuffer(scr,NULL,NULL);
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
	if(scr) {
		if(gfx==GFX_AGA) CloseScreen(scr);
		if(gfx==GFX_P96) p96CloseScreen(scr);
		scr=NULL;
	}
	vec.c2p_Scr=NULL;
	if(bmapwpx) FreeBitMap(bmapwpx);
	if(tmpraswpx) FreeVec(tmpraswpx);
	if(raswpx) FreeVec(raswpx);
	if(gfx==GFX_P96&&P96Base) { CloseLibrary((struct Library *)P96Base); P96Base=NULL; }
	if(GfxBase) { CloseLibrary((struct Library *)GfxBase); GfxBase=NULL; }
	if(IntuitionBase) { CloseLibrary((struct Library *)IntuitionBase); IntuitionBase=NULL; }
	c2p_CloseCatalog();
} // free


int SAVEDS mdo(unsigned char *chunky,unsigned char *delta,int numscreen) {
	return(mdofull(chunky,numscreen));
} // do


int SAVEDS mdofull(unsigned char *chunky,int numscreen) {
static struct RenderInfo ri;
	LONG lock;

	raswpx->BitMap=scrb[numscreen]->sb_BitMap;
	if(gfx==GFX_AGA) WritePixelArray8(raswpx,0,0,SCRWIDTH-1,SCRHEIGHT-1,chunky,tmpraswpx);
	if(gfx==GFX_P96) {
		lock=p96LockBitMap(raswpx->BitMap,&ri,sizeof(ri));
		p96WritePixelArray(&ri,0,0,raswpx,0,0,SCRWIDTH,SCRHEIGHT);
		p96UnlockBitMap(raswpx->BitMap,lock);
	}
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
			SetPointer(win,nopointer,1,16,0,0);
			winsleeped=0;
		}
	}
}
