/* :ts=4							c2p_ehb.c
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
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "cp4_ver.h"

#define C2P_VERSION		"1"
#define C2P_REVISION	"0"
#define C2P_AUTHOR		"Zavacki-gega"
#define C2P_NAME		"EHB"
#define C2P_BUFFERING	3
#define C2P_DATE		DATE
#define C2P_LOCALIZE

#include "c2p_module.c"



// DELTA ONLY !!
extern void REGARGS c2p(	REG(a0,void *chunkyaddr),
							REG(a1,void *deltaaddr),
							REG(a2,void *planaraddr),
							REG(a3,void *lineskipaddr),
							REG(d0,int planewidth)
						);

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
static void *LineSkipBuffer;
static int BitPlaneByteLength;
static unsigned char dummydelta[SCRSIZE];

static ULONG Palette[]={
	32<<16,
/*127% digi
	0x00000000, 0x00000000, 0x00000000,				// 64	digi 127%
	0xa7a7a7a7, 0xa0a0a0a0, 0xa7a7a7a7,				// 65
	0xe4e4e4e4, 0x86868686, 0x89898989,				// 66
	0x6a6a6a6a, 0xbebebebe, 0xc6c6c6c6,				// 67
	0xdadadada, 0x7c7c7c7c, 0xf8f8f8f8,				// 68
	0x6a6a6a6a, 0xcdcdcdcd, 0x6a6a6a6a,				// 69
	0x93939393, 0x9a9a9a9a, 0xffffffff,				// 70
	0xbbbbbbbb, 0xafafafaf, 0x2d2d2d2d,				// 71
	0xdadadada, 0x9a9a9a9a, 0x42424242,				// 72
	0xd0d0d0d0, 0xa5a5a5a5, 0x2d2d2d2d,				// 73
	0x93939393, 0xc3c3c3c3, 0x23232323,				// 74
	0xe4e4e4e4, 0x81818181, 0xa7a7a7a7,				// 75
	0x60606060, 0xc3c3c3c3, 0xa7a7a7a7,				// 76
	0x74747474, 0xaaaaaaaa, 0xffffffff,				// 77
	0xa7a7a7a7, 0x8b8b8b8b, 0xffffffff,				// 78
	0x7e7e7e7e, 0xc8c8c8c8, 0x42424242,				// 79
	0x00000000, 0x00000000, 0x00000000,				// 80
	0xdadadada, 0xd7d7d7d7, 0xdadadada,				// 81
	0xffffffff, 0xbebebebe, 0xbbbbbbbb,				// 82
	0x9d9d9d9d, 0xf6f6f6f6, 0xf8f8f8f8,				// 83
	0xffffffff, 0xb4b4b4b4, 0xffffffff,				// 84
	0x9d9d9d9d, 0xffffffff, 0x9d9d9d9d,				// 85
	0xc6c6c6c6, 0xd2d2d2d2, 0xffffffff,				// 86
	0xeeeeeeee, 0xe7e7e7e7, 0x60606060,				// 87
	0xffffffff, 0xd2d2d2d2, 0x7e7e7e7e,				// 88
	0xffffffff, 0xdcdcdcdc, 0x6a6a6a6a,				// 89
	0xc6c6c6c6, 0xfbfbfbfb, 0x56565656,				// 90
	0xffffffff, 0xb9b9b9b9, 0xdadadada,				// 91
	0x9d9d9d9d, 0xfbfbfbfb, 0xe4e4e4e4,				// 92
	0xa7a7a7a7, 0xe2e2e2e2, 0xffffffff,				// 93
	0xdadadada, 0xc3c3c3c3, 0xffffffff,				// 94
	0xb1b1b1b1, 0xffffffff, 0x7e7e7e7e,				// 95
*/
/*
	0x00000000, 0x00000000, 0x00000000,				// 64	126%minus4
	0xb5b5b5b5, 0xb5b5b5b5, 0xb5b5b5b5,				// 65
	0xdddddddd, 0x46464646, 0x46464646,				// 66
	0x00000000, 0xbfbfbfbf, 0xbabababa,				// 67
	0xd8d8d8d8, 0x4b4b4b4b, 0xd8d8d8d8,				// 68
	0x3c3c3c3c, 0xbfbfbfbf, 0x37373737,				// 69
	0x4b4b4b4b, 0x50505050, 0xd8d8d8d8,				// 70
	0xbfbfbfbf, 0xbabababa, 0x14141414,				// 71
	0xe2e2e2e2, 0x8d8d8d8d, 0x0a0a0a0a,				// 72
	0xc9c9c9c9, 0x83838383, 0x46464646,				// 73
	0xa1a1a1a1, 0xc4c4c4c4, 0x50505050,				// 74
	0xdddddddd, 0x1e1e1e1e, 0x4b4b4b4b,				// 75
	0x00000000, 0xcececece, 0xb5b5b5b5,				// 76
	0x69696969, 0x8d8d8d8d, 0xd8d8d8d8,				// 77
	0x5a5a5a5a, 0x5f5f5f5f, 0xd8d8d8d8,				// 78
	0x5a5a5a5a, 0xbfbfbfbf, 0x41414141,				// 79
	0x00000000, 0x00000000, 0x00000000,				// 80
	0xe2e2e2e2, 0xe2e2e2e2, 0xe2e2e2e2,				// 81
	0xfbfbfbfb, 0x64646464, 0x64646464,				// 82
	0x00000000, 0xecececec, 0xe2e2e2e2,				// 83
	0xf6f6f6f6, 0x6e6e6e6e, 0xf6f6f6f6,				// 84
	0x5a5a5a5a, 0xecececec, 0x50505050,				// 85
	0x6e6e6e6e, 0x6e6e6e6e, 0xf6f6f6f6,				// 86
	0xecececec, 0xe2e2e2e2, 0x19191919,				// 87
	0xffffffff, 0xa6a6a6a6, 0x0f0f0f0f,				// 88
	0xf1f1f1f1, 0xabababab, 0x69696969,				// 89
	0xc4c4c4c4, 0xecececec, 0x5f5f5f5f,				// 90
	0xfbfbfbfb, 0x2d2d2d2d, 0x6e6e6e6e,				// 91
	0x00000000, 0xf1f1f1f1, 0xd8d8d8d8,				// 92
	0x97979797, 0xbfbfbfbf, 0xf6f6f6f6,				// 93
	0x83838383, 0x88888888, 0xf6f6f6f6,				// 94
	0x83838383, 0xecececec, 0x5f5f5f5f,				// 95
*/
/*
	0x00000000, 0x00000000, 0x00000000,				// 64	(120% Minus4)
	0xacacacac, 0xacacacac, 0xacacacac,				// 65
	0xd3d3d3d3, 0x43434343, 0x43434343,				// 66
	0x00000000, 0xb6b6b6b6, 0xb1b1b1b1,				// 67
	0xcececece, 0x47474747, 0xcececece,				// 68
	0x39393939, 0xb6b6b6b6, 0x34343434,				// 69
	0x47474747, 0x4c4c4c4c, 0xcececece,				// 70
	0xb6b6b6b6, 0xb1b1b1b1, 0x13131313,				// 71
	0xd7d7d7d7, 0x86868686, 0x09090909,				// 72
	0xbfbfbfbf, 0x7c7c7c7c, 0x43434343,				// 73
	0x99999999, 0xbbbbbbbb, 0x4c4c4c4c,				// 74
	0xd3d3d3d3, 0x1c1c1c1c, 0x47474747,				// 75
	0x00000000, 0xc4c4c4c4, 0xacacacac,				// 76
	0x64646464, 0x86868686, 0xcececece,				// 77
	0x56565656, 0x5b5b5b5b, 0xcececece,				// 78
	0x56565656, 0xb6b6b6b6, 0x3e3e3e3e,				// 79
	0x00000000, 0x00000000, 0x00000000,				// 80
	0xd7d7d7d7, 0xd7d7d7d7, 0xd7d7d7d7,				// 81
	0xefefefef, 0x5f5f5f5f, 0x5f5f5f5f,				// 82
	0x00000000, 0xe1e1e1e1, 0xd7d7d7d7,				// 83
	0xebebebeb, 0x69696969, 0xebebebeb,				// 84
	0x56565656, 0xe1e1e1e1, 0x4c4c4c4c,				// 85
	0x69696969, 0x69696969, 0xebebebeb,				// 86
	0xe1e1e1e1, 0xd7d7d7d7, 0x17171717,				// 87
	0xf4f4f4f4, 0x9e9e9e9e, 0x0e0e0e0e,				// 88
	0xe6e6e6e6, 0xa3a3a3a3, 0x64646464,				// 89
	0xbbbbbbbb, 0xe1e1e1e1, 0x5b5b5b5b,				// 90
	0xefefefef, 0x2b2b2b2b, 0x69696969,				// 91
	0x00000000, 0xe6e6e6e6, 0xcececece,				// 92
	0x8f8f8f8f, 0xb6b6b6b6, 0xebebebeb,				// 93
	0x7c7c7c7c, 0x81818181, 0xebebebeb,				// 94
	0x7c7c7c7c, 0xe1e1e1e1, 0x5b5b5b5b,				// 95
*/

	0x00000000, 0x00000000, 0x00000000,				// 64
	0x9e9e9e9e, 0x97979797, 0x9e9e9e9e,				// 65
	0xd7d7d7d7, 0x7f7f7f7f, 0x81818181,				// 66
	0x64646464, 0xb3b3b3b3, 0xbbbbbbbb,				// 67
	0xcececece, 0x75757575, 0xebebebeb,				// 68
	0x64646464, 0xc2c2c2c2, 0x64646464,				// 69
	0x8b8b8b8b, 0x92929292, 0xffffffff,				// 70
	0xb1b1b1b1, 0xa5a5a5a5, 0x2b2b2b2b,				// 71
	0xcececece, 0x92929292, 0x3e3e3e3e,				// 72
	0xc4c4c4c4, 0x9b9b9b9b, 0x2b2b2b2b,				// 73
	0x8b8b8b8b, 0xb8b8b8b8, 0x21212121,				// 74
	0xd7d7d7d7, 0x7a7a7a7a, 0x9e9e9e9e,				// 75
	0x5b5b5b5b, 0xb8b8b8b8, 0x9e9e9e9e,				// 76
	0x6e6e6e6e, 0xa0a0a0a0, 0xfefefefe,				// 77
	0x9e9e9e9e, 0x83838383, 0xffffffff,				// 78
	0x77777777, 0xbdbdbdbd, 0x3e3e3e3e,				// 79
	0x00000000, 0x00000000, 0x00000000,				// 80
	0xcececece, 0xcbcbcbcb, 0xcececece,				// 81
	0xffffffff, 0xb3b3b3b3, 0xb1b1b1b1,				// 82
	0x94949494, 0xe8e8e8e8, 0xebebebeb,				// 83
	0xfefefefe, 0xaaaaaaaa, 0xffffffff,				// 84
	0x94949494, 0xf7f7f7f7, 0x94949494,				// 85
	0xbbbbbbbb, 0xc7c7c7c7, 0xffffffff,				// 86
	0xe1e1e1e1, 0xdadadada, 0x5b5b5b5b,				// 87
	0xffffffff, 0xc7c7c7c7, 0x77777777,				// 88
	0xf4f4f4f4, 0xd0d0d0d0, 0x64646464,				// 89
	0xbbbbbbbb, 0xedededed, 0x51515151,				// 90
	0xffffffff, 0xafafafaf, 0xcececece,				// 91
	0x94949494, 0xedededed, 0xd7d7d7d7,				// 92
	0x9e9e9e9e, 0xd5d5d5d5, 0xffffffff,				// 93
	0xcececece, 0xb8b8b8b8, 0xffffffff,				// 94
	0xa7a7a7a7, 0xf2f2f2f2, 0x77777777,				// 95

/*	0x00000000,0x00000000,0x00000000,	// $40 (64)		digi-100%
	0x84848484,0x7E7E7E7E,0x84848484,	// $41 (65)
	0xB4B4B4B4,0x6A6A6A6A,0x6C6C6C6C,	// $42 (66)
	0x54545454,0x96969696,0x9C9C9C9C,	// $43 (67)
	0xACACACAC,0x62626262,0xC4C4C4C4,	// $44 (68)
	0x54545454,0xA2A2A2A2,0x54545454,	// $45 (69)
	0x74747474,0x7A7A7A7A,0xDCDCDCDC,	// $46 (70)
	0x94949494,0x8A8A8A8A,0x24242424,	// $47 (71)
	0xACACACAC,0x7A7A7A7A,0x34343434,	// $48 (72)
	0xA4A4A4A4,0x82828282,0x24242424,	// $49 (73)
	0x74747474,0x9A9A9A9A,0x1C1C1C1C,	// $4A (74)
	0xB4B4B4B4,0x66666666,0x84848484,	// $4B (75)
	0x4C4C4C4C,0x9A9A9A9A,0x84848484,	// $4C (76)
	0x5C5C5C5C,0x86868686,0xD4D4D4D4,	// $4D (77)
	0x84848484,0x6E6E6E6E,0xE4E4E4E4,	// $4E (78)
	0x64646464,0x9E9E9E9E,0x34343434,	// $4F (79)
	0x00000000,0x00000000,0x00000000,	// $50 (80)
	0xACACACAC,0xAAAAAAAA,0xACACACAC,	// $51 (81)
	0xE4E4E4E4,0x96969696,0x94949494,	// $52 (82)
	0x7C7C7C7C,0xC2C2C2C2,0xC4C4C4C4,	// $53 (83)
	0xD4D4D4D4,0x8E8E8E8E,0xECECECEC,	// $54 (84)
	0x7C7C7C7C,0xCECECECE,0x7C7C7C7C,	// $55 (85)
	0x9C9C9C9C,0xA6A6A6A6,0xF4F4F4F4,	// $56 (86)
	0xBCBCBCBC,0xB6B6B6B6,0x4C4C4C4C,	// $57 (87)
	0xDCDCDCDC,0xA6A6A6A6,0x64646464,	// $58 (88)
	0xCCCCCCCC,0xAEAEAEAE,0x54545454,	// $59 (89)
	0x9C9C9C9C,0xC6C6C6C6,0x44444444,	// $5A (90)
	0xDCDCDCDC,0x92929292,0xACACACAC,	// $5B (91)
	0x7C7C7C7C,0xC6C6C6C6,0xB4B4B4B4,	// $5C (92)
	0x84848484,0xB2B2B2B2,0xFCFCFCFC,	// $5D (93)
	0xACACACAC,0x9A9A9A9A,0xFCFCFCFC,	// $5E (94)
	0x8C8C8C8C,0xCACACACA,0x64646464,	// $5F (95)*/
	0L
};

#define MSG_INFO 0
#define MSG_NOSCRMODE 1
#define MSG_NOGFX 2
#define MSG_NOINTUI 3
#define MSG_NOSCR 4
#define MSG_NOWIN 5
#define MSG_NOMEM 6
#define MSG_WRONGBITMAPTYPE 7
#define MSG_NOHAM 8
#define MSG_TRYEHB 9

static char *defstr[]={
	"64 color EHB driver for OCS/ECS machines. Equipped with Zavacki's modified powerful chunky2planar. With digitized degraded palette.",
	"No ScreenMode Specified",
	"Can't open graphics.library V39",
	"Can't open intuition.library V37",
	"Can't open Screen",
	"Can't open Window",
	"Not enough memory",
	"Wrong BitMap Type",
	"HAM isn't supported",
	"Failed to force EHB mode",
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
	int bmx,planes=6,i,tryehb=0;

	c2p_OpenCatalog(defstr);

	linedelta=linedeltatab;
	if(scrmode==~0) return(STR(MSG_NOSCRMODE));
	if((scrmode&0x800)!=0) return(STR(MSG_NOHAM));
	if((scrmode&0x80)==0) { scrmode|=0x80; tryehb=1; }

	for(i=0;i<SCRSIZE;i++) dummydelta[i]=0xff;

	if((GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",39))==NULL) return(STR(MSG_NOGFX));
	if(!(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",37))) return(STR(MSG_NOINTUI));

	/* Allocate mem for nopointer
	 */
	if(NULL==(nopointer=AllocVec(24,MEMF_CHIP|MEMF_CLEAR))) return(STR(MSG_NOMEM));

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

	LineSkipBuffer=linedelta;
	BitPlaneByteLength=bmx;

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
		return((tryehb==0?STR(MSG_NOSCR):STR(MSG_TRYEHB)));
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
	c2p(chunky,delta,planars[numscreen],LineSkipBuffer,BitPlaneByteLength);
	ChangeScreenBuffer(scr,scrb[numscreen]);
	return(RET_OK);
} // do


int SAVEDS mdofull(unsigned char *chunky,int numscreen) {
	c2p(chunky,dummydelta,planars[numscreen],LineSkipBuffer,BitPlaneByteLength);
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
