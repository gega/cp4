/* :ts=4							c2p_windowcarddelta.c
 *
 *    cp4 - Commodore C+4 emulator
 *    Copyright (C) 1998 G?ti Gergely
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
#include <graphics/gfxbase.h>
#include <proto/exec.h>
#include <exec/alerts.h>
#include <proto/intuition.h>
#include <intuition/pointerclass.h>
#include <proto/gadtools.h>
#include <proto/layers.h>

#include "cp4_ver.h"

#define C2P_VERSION		"1"
#define C2P_REVISION	"2"
#define C2P_AUTHOR		"gega <G?ti Gergely>"
#define C2P_NAME		"WindowCardDelta"
#define C2P_DATE		DATE
#define C2P_LOCALIZE
#define C2P_BUFFERING	2
#define C2P_NOSPEED

#include "c2p_module.c"
#include "c2p_palette.c"
#include "c2p_color_priority.c"


struct GfxBase *GfxBase=NULL;
struct IntuitionBase *IntuitionBase=NULL;
struct Library *GadToolsBase=NULL;
struct Library *LayersBase=NULL;
static struct BitMap *bmap;
static struct Window *win=NULL;
static int offx=0,offy=0;
static int maxx,maxy;
static int borhoriz,borvert;
static struct BitMap mybmap;
static void *mypointer=NULL;
static UWORD *mypntchip0=NULL;
static UWORD *mypntchip1=NULL;
static struct ColorMap *colmap=NULL;
static struct BitMap *bmapwpx;
static struct RastPort *tmpraswpx;
static struct RastPort *raswpx;
static int winsleeped=0;
static struct Requester InvisibleRequester;
static LONG PenTable[256];
static unsigned char PenArray[SCRSIZE];
static struct Menu *mMenus=NULL;
static APTR visualinfo=NULL;
static int NameLen;
static char Name[]=C2P_NAME;
static ULONG Palette8[770];
/* LORES-POINTER
 */
#define POINTERHEIGHT	31
static UWORD pointerp0[]={ 0,384,384,3504,3504,3504,28080,28080,28080,28086,28086,28086,32758,32758,32766,32766,32764,16380,16376,16376,0,32764,0,32764,32740,32764,32740,32764,32764,32736,28672 };
static UWORD pointerp1[]={ 384,960,4080,8184,8184,32760,65528,65528,65534,65535,65535,65535,65535,65535,65535,65535,65534,32766,32764,32764,65534,65534,65534,32770,32794,32770,32794,32770,32770,32768,32768 };

/* prefs
 */
static int WTop,WLeft,WHeight,WWidth,Wx,Wy;
static char *WPubName=NULL;

/* Protos
 */
static char *initgfx(void);
static void freegfx(void);
static int mReset(struct IntuiMessage *imsg);
static int mHReset(struct IntuiMessage *imsg);
static int mDebug(struct IntuiMessage *imsg);
static int mPrefs(struct IntuiMessage *imsg);
static int mJump(struct IntuiMessage *imsg);
static int mQuit(struct IntuiMessage *imsg);

#define MSG_PROJECT			0
#define MSG_RESET			1
#define MSG_HARDRESET		2
#define MSG_DEBUG			3
#define MSG_PREFS			4
#define MSG_QUIT			5
#define MSG_OPTIONS			6
#define MSG_JUMPSCR			7
#define MSG_INFO			8
#define MSG_ERRVISUALINFO	9
#define MSG_ERRCREATEMENU	10
#define MSG_ERROPENWIN		11
#define MSG_ERRBITMAPATTR	12
#define MSG_ERRBITPNUM		13
#define MSG_ERRFINDCOLMAP	14
#define MSG_ERROBTPENS		15
#define MSG_ERRGRAPHICS		16
#define MSG_ERRINTUITION	17
#define MSG_ERRLAYERS		18
#define MSG_ERRGADTOOLS		19
#define MSG_ERRPOINTER		20
#define MSG_RESET_A			21
#define MSG_HARDRESET_A		22
#define MSG_DEBUG_A			23
#define MSG_PREFS_A			24
#define MSG_QUIT_A			25
#define MSG_JUMPSCR_A		26
#define MSG_NOMEM			27

static char *defstr[]={
	"Project",
	"Reset",
	"HardReset",
	"Debug...",
	"Prefs...",
	"Quit",
	"Options",
	"Jump PubScreen",
	"Workbench window driver with delta buffer. Works on GFX cards and AGA too.",
	"Can't find VisualInfo",
	"Can't Create Menu",
	"Can't open Window",
	"Wrong BitMap Attributes",
	"Wrong BitPlane Number",
	"Can't Find ColorMap",
	"Can't Obtain Pens",
	"Can't open graphics.library V39",
	"Can't open intuition.library V39",
	"Can't open layers.library V33",
	"Can't open gadtools.library V37",
	"Can't Create Pointer",
	"R",
	"H",
	"D",
	"P",
	"X",
	"J",
	"Not enough memory",
	NULL
};


/* Menu
 */
static struct NewMenu mNewMenu[]={
	{	NM_TITLE, (STRPTR)MSG_PROJECT, NULL, 0, NULL, NULL	},
	{	NM_ITEM, (STRPTR)MSG_RESET, (STRPTR)MSG_RESET_A, 0, 0L, (APTR)mReset },
	{	NM_ITEM, (STRPTR)MSG_HARDRESET, (STRPTR)MSG_HARDRESET_A, 0, 0L, (APTR)mHReset },
	{	NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL	},
	{	NM_ITEM, (STRPTR)MSG_DEBUG, (STRPTR)MSG_DEBUG_A, 0, 0L, (APTR)mDebug },
	{	NM_ITEM, (STRPTR)MSG_PREFS, (STRPTR)MSG_PREFS_A, 0, 0L, (APTR)mPrefs },
	{	NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL	},
	{	NM_ITEM, (STRPTR)MSG_QUIT, (STRPTR)MSG_QUIT_A, 0, 0L, (APTR)mQuit },
	{	NM_TITLE, (STRPTR)MSG_OPTIONS, NULL, 0, NULL, NULL	},
	{	NM_ITEM, (STRPTR)MSG_JUMPSCR, (STRPTR)MSG_JUMPSCR_A, 0, 0L, (APTR)mJump },
	{	NM_END, NULL, NULL, 0, 0L, NULL	}
};
static struct NewMenu sNewMenu[]={
	{	NM_TITLE, (STRPTR)MSG_PROJECT, NULL, 0, NULL, NULL	},
	{	NM_ITEM, (STRPTR)MSG_RESET, (STRPTR)MSG_RESET_A, 0, 0L, (APTR)mReset },
	{	NM_ITEM, (STRPTR)MSG_HARDRESET, (STRPTR)MSG_HARDRESET_A, 0, 0L, (APTR)mHReset },
	{	NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL	},
	{	NM_ITEM, (STRPTR)MSG_DEBUG, (STRPTR)MSG_DEBUG_A, 0, 0L, (APTR)mDebug },
	{	NM_ITEM, (STRPTR)MSG_PREFS, (STRPTR)MSG_PREFS_A, 0, 0L, (APTR)mPrefs },
	{	NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL	},
	{	NM_ITEM, (STRPTR)MSG_QUIT, (STRPTR)MSG_QUIT_A, 0, 0L, (APTR)mQuit },
	{	NM_TITLE, (STRPTR)MSG_OPTIONS, NULL, 0, NULL, NULL	},
	{	NM_ITEM, (STRPTR)MSG_JUMPSCR, (STRPTR)MSG_JUMPSCR_A, 0, 0L, (APTR)mJump },
	{	NM_END, NULL, NULL, 0, 0L, NULL	}
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

/* Handle MenuPick
 */
static int mReset(struct IntuiMessage *imsg) {
	return(RET_RESET);
} // mReset()

static int mHReset(struct IntuiMessage *imsg) {
	return(RET_HRESET);
} // mHReset()

static int mDebug(struct IntuiMessage *imsg) {
	return(RET_DEBUG);
} // mDebug()

static int mPrefs(struct IntuiMessage *imsg) {
	return(RET_PREFS);
} // mPrefs()

static int mQuit(struct IntuiMessage *imsg) {
	return(RET_QUIT);
} // mQuit()

static int mJump(struct IntuiMessage *imsg) {
static char newname[MAXPUBSCREENNAME+1];
	int r=RET_OK,i;
	char *s;

	if(NULL!=(NextPubScreen(vec.c2p_Scr,newname))) {
		if(NULL!=(s=AllocVec(MAXPUBSCREENNAME+1,MEMF_ANY))) {
			freegfx();
			r=RET_NEWWIN;
			for(i=0;newname[i]!='\0';i++);
			CopyMem(newname,s,i+1);
			if(WPubName) FreeVec(WPubName);
			WPubName=s;
			if(NULL!=(initgfx())) {
				freegfx();
				// PANIC!!!
				Alert(AT_Recovery|AG_NoMemory|AO_Unknown);
				return(RET_ERROR);
			}
		}
	}
	return(r);
} // mJump()


static INLINE void calcwinsizes() {
	borhoriz=win->BorderRight+win->BorderLeft-1;
	borvert=win->BorderTop+win->BorderBottom-1;
	maxx=SCRWIDTH-(win->Width-borhoriz);
	maxy=SCRHEIGHT-(win->Height-borvert)+2;
	if(offx>maxx) {
		ScrollLayer(0,win->RPort->Layer,-(offx-maxx),0);
		offx=maxx;
	}
	if(offy>maxy) {
		ScrollLayer(0,win->RPort->Layer,0,-(offy-maxy));
		offy=maxy;
	}
}


static char *initgfx(void) {
static char pname[MAXPUBSCREENNAME+1];
	ULONG r,g,b,wa_top=WA_Top;
	int planes,i,j,mx,my;

	for(i=0;i<256;i++) PenTable[i]=-1;

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
	for(i=0;i<384;i++) Palette8[i]=Palette[1+i];
	for(i=0;i<384;i++) Palette8[i+384]=Palette[1+i];

	/* Get the required PubScreen
	 */
	if(NULL==(vec.c2p_Scr=LockPubScreen(WPubName))) {
		vec.c2p_Scr=LockPubScreen(NULL);
		GetDefaultPubScreen(pname);
		if(WPubName) FreeVec(WPubName);
		for(i=0;pname[i]!='\0';i++);
		if(NULL==(WPubName=AllocVec(i+2,MEMF_ANY))) return(STR(MSG_NOMEM));
		CopyMem(pname,WPubName,i+1);
	}

	/* ScreenToFront
	 */
	ScreenToFront(vec.c2p_Scr);

	/* Get Screen Attribs
	 */
	planes=vec.c2p_Scr->BitMap.Depth;
	if(!(visualinfo=GetVisualInfo(vec.c2p_Scr,TAG_DONE))) return(STR(MSG_ERRVISUALINFO));

	/* Allocate bitmap
	 */
	if(NULL==(bmap=AllocBitMap(SCRWIDTH,SCRHEIGHT,planes,BMF_DISPLAYABLE|BMF_CLEAR,NULL))) return(STR(MSG_NOMEM));

	/* SetUp Menu
	 */
	for(i=0;sNewMenu[i].nm_Type!=NM_END;i++) {
		mNewMenu[i].nm_Label=sNewMenu[i].nm_Label;
		mNewMenu[i].nm_CommKey=sNewMenu[i].nm_CommKey;
	}
	for(i=0;mNewMenu[i].nm_Type!=NM_END;i++) {
		if(mNewMenu[i].nm_Label!=NM_BARLABEL) mNewMenu[i].nm_Label=STR((long)mNewMenu[i].nm_Label);
		if(mNewMenu[i].nm_CommKey!=NULL) mNewMenu[i].nm_CommKey=STR((long)mNewMenu[i].nm_CommKey);
	}
	if(!(mMenus=CreateMenus(mNewMenu,GTMN_FrontPen,0L,TAG_DONE,0L))) return(STR(MSG_ERRCREATEMENU));
	LayoutMenus(mMenus,visualinfo,GTMN_TextAttr,(ULONG)vec.c2p_Scr->Font,GTMN_NewLookMenus,TRUE,TAG_DONE,0L);

	/* Open window
	 */
	if(WTop==-1) { wa_top=TAG_IGNORE; WTop=0; }
	if((win=(struct Window *)OpenWindowTags(NULL,
				WA_Left, WLeft,
				wa_top, WTop,
				WA_Width, WWidth,
				WA_Height, WHeight,
				WA_Title, (ULONG)C2P_NAME,
                WA_Flags,	WFLG_SIZEGADGET|
							WFLG_SIZEBBOTTOM|
							WFLG_DRAGBAR|
							WFLG_DEPTHGADGET|
							WFLG_CLOSEGADGET|
							WFLG_SUPER_BITMAP|
							WFLG_GIMMEZEROZERO|
							WFLG_ACTIVATE,
				WA_SuperBitMap, (ULONG)bmap,
				WA_AutoAdjust, TRUE,
				WA_RptQueue, 25,
				WA_IDCMP, 	IDCMP_MOUSEMOVE|
							IDCMP_CLOSEWINDOW|
							IDCMP_MENUPICK|
							IDCMP_NEWSIZE|
							IDCMP_MOUSEBUTTONS,
				WA_PubScreen, (ULONG)vec.c2p_Scr,
				WA_PubScreenFallBack, TRUE,
				WA_NewLookMenus, TRUE,
				TAG_DONE,0L ))==NULL)
		return(STR(MSG_ERROPENWIN));
	SetMenuStrip(win,mMenus);
	vec.c2p_Win=win;
	winsleeped=0;
	offx=offy=0;
	calcwinsizes();
	WindowLimits(win,borhoriz+80,borvert+20,SCRWIDTH+borhoriz,borvert+SCRHEIGHT);
	offx+=Wx;		offy+=Wy;
	mx=Wx;			my=Wy;
	if(offx<0)		{ mx-=offx; offx=0; }
	if(offx>maxx)	{ mx-=offx-maxx; offx=maxx; }
	if(offy<0)		{ my-=offy; offy=0; }
	if(offy>maxy)	{ my-=offy-maxy; offy=maxy; }
	ScrollLayer(0,win->RPort->Layer,mx,my);

	/* Allocate & Init TMP rasters for WritePixel...
	 */
	bmapwpx=AllocBitMap(SCRWIDTH,1,planes,BMF_CLEAR,NULL);
	if(bmapwpx==NULL) return(STR(MSG_NOMEM));
	tmpraswpx=AllocVec(sizeof(struct RastPort),MEMF_ANY);
	if(tmpraswpx==NULL) return(STR(MSG_NOMEM));
	InitRastPort(tmpraswpx);
	tmpraswpx->BitMap=bmapwpx;
	raswpx=AllocVec(sizeof(struct RastPort),MEMF_ANY);
	if(raswpx==NULL) return(STR(MSG_NOMEM));
	InitRastPort(raswpx);
	raswpx->BitMap=bmap;

	/* SetUp PenTable
	 */
	if(NULL==(colmap=vec.c2p_Scr->ViewPort.ColorMap)) return(STR(MSG_ERRFINDCOLMAP));
	for(i=0;i<256;i++) {
		j=ColorPriority[i]*3;
		r=Palette8[j];
		g=Palette8[j+1];
		b=Palette8[j+2];
		if(-1==(PenTable[ColorPriority[i]]=ObtainBestPen(colmap,r,g,b,OBP_Precision,PRECISION_GUI,TAG_DONE))) return(STR(MSG_ERROBTPENS));
	}
	return(NULL);
} // initgfx()


static void freegfx(void) {
	int i;
	mawake();
	if(GfxBase) {
		WaitTOF();
		if(colmap) for(i=0;i<256;i++) if(PenTable[i]!=-1) ReleasePen(colmap,PenTable[i]);
	}
	if(win) {
		WTop=win->TopEdge;
		WLeft=win->LeftEdge;
		WHeight=win->Height;
		WWidth=win->Width;
		vec.c2p_AddOptionInt("WINTOP",WTop);
		vec.c2p_AddOptionInt("WINLEFT",WLeft);
		vec.c2p_AddOptionInt("WINHEIGHT",WHeight);
		vec.c2p_AddOptionInt("WINWIDTH",WWidth);
		Wx=offx;
		Wy=offy;
		vec.c2p_AddOptionInt("OFFX",Wx);
		vec.c2p_AddOptionInt("OFFY",Wy);
		ClearMenuStrip(win);
		CloseWindow(win);
		win=NULL;
	}
	vec.c2p_Win=NULL;
	if(mMenus) { FreeMenus(mMenus); mMenus=NULL; }
	if(visualinfo) { FreeVisualInfo(visualinfo); visualinfo=NULL; }
	if(vec.c2p_Scr) {
		vec.c2p_AddOptionStr("PUBSCREEN",WPubName);
		UnlockPubScreen(NULL,vec.c2p_Scr);
		vec.c2p_Scr=NULL;
	}
	if(bmap) { FreeBitMap(bmap); bmap=NULL; }
	if(bmapwpx) { FreeBitMap(bmapwpx); bmapwpx=NULL; }
	if(tmpraswpx) { FreeVec(tmpraswpx); tmpraswpx=NULL; }
	if(raswpx) { FreeVec(raswpx); raswpx=NULL; }
} // freegfx()


char *SAVEDS minit(ULONG scrmode, ULONG overscan, unsigned char *linedeltatab) {
	int i;
	ULONG res;
	char *s;

	c2p_OpenCatalog(defstr);
	for(NameLen=0;Name[NameLen]!='\0';NameLen++);

	if(!(GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",39))) return(STR(MSG_ERRGRAPHICS));
	if(!(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",39))) return(STR(MSG_ERRINTUITION));
	if(!(GadToolsBase=OpenLibrary("gadtools.library",37))) return(STR(MSG_ERRGADTOOLS));
	if(!(LayersBase=OpenLibrary("layers.library",33))) return(STR(MSG_ERRLAYERS));

	/* Set up pointer
	 */
	if(NULL==(mypntchip0=AllocVec((POINTERHEIGHT+1)*2,MEMF_CHIP))) return(STR(MSG_NOMEM));
	if(NULL==(mypntchip1=AllocVec((POINTERHEIGHT+1)*2,MEMF_CHIP))) return(STR(MSG_NOMEM));
	CopyMem(pointerp0,mypntchip0,2*POINTERHEIGHT);
	CopyMem(pointerp1,mypntchip1,2*POINTERHEIGHT);
    InitBitMap(&mybmap,2,16,POINTERHEIGHT);
    mybmap.Planes[0]=(PLANEPTR)mypntchip0;
    mybmap.Planes[1]=(PLANEPTR)mypntchip1;
	res=POINTERYRESN_DEFAULT;
	if(((GfxBase->ChipRevBits0&GFXF_HR_AGNUS)!=0)&&((GfxBase->ChipRevBits0&GFXF_HR_DENISE)!=0)) res=POINTERXRESN_SCREENRES;
    mypointer=NewObject(NULL,"pointerclass",
		POINTERA_BitMap, (ULONG)&mybmap,
		POINTERA_XOffset, -6,
		POINTERA_WordWidth, 1,
		POINTERA_XResolution, res,
		POINTERA_YResolution, res,
		TAG_DONE );
	if(mypointer==NULL) return(STR(MSG_ERRPOINTER));

	/* Get Options
	 */
	Wx=vec.c2p_GetOptionInt("OFFX",0);
	Wy=vec.c2p_GetOptionInt("OFFY",0);
	WTop=vec.c2p_GetOptionInt("WINTOP",-1);
	WLeft=vec.c2p_GetOptionInt("WINLEFT",0);
	WHeight=vec.c2p_GetOptionInt("WINHEIGHT",303);
	WWidth=vec.c2p_GetOptionInt("WINWIDTH",359);
	s=vec.c2p_GetOptionStr("PUBSCREEN","Workbench");
	for(i=0;s[i]!='\0';i++);
	if(NULL==(WPubName=AllocVec(i+2,MEMF_ANY))) return(STR(MSG_NOMEM));
	CopyMem(s,WPubName,i+1);

	return(initgfx());
} // minit


void SAVEDS mfree(void) {
	freegfx();
	if(mypntchip0) { FreeVec(mypntchip0); mypntchip0=NULL; }
	if(mypntchip1) { FreeVec(mypntchip1); mypntchip1=NULL; }
    if(mypointer) { DisposeObject(mypointer); mypointer=NULL; }
	if(GfxBase) CloseLibrary((struct Library *)GfxBase);
	if(IntuitionBase) { CloseLibrary((struct Library *)IntuitionBase); IntuitionBase=NULL; }
	if(GadToolsBase) { CloseLibrary(GadToolsBase); GadToolsBase=NULL; }
	if(LayersBase) CloseLibrary(LayersBase);
	c2p_CloseCatalog();
	return;
} // free


INLINE int handleinput(void) {
	int ret=RET_OK;
	struct IntuiMessage *imsg;
	struct MenuItem *n;
	int (*func)(struct IntuiMessage *);
	int move=0,lastx=0,lasty=0;

	while((imsg=(struct IntuiMessage *)RemHead(&vec.c2p_MsgList))) {
		switch(imsg->Class) {
			case IDCMP_CLOSEWINDOW :
				ret=RET_QUIT;
				break;
			case IDCMP_MENUPICK :
				while(imsg->Code!=MENUNULL) {
					n=ItemAddress(mMenus,imsg->Code);
					func=(void *)(GTMENUITEM_USERDATA(n));
					ret=func(imsg);
					if(ret==RET_NEWWIN||ret==RET_ERROR) {
						move=0;
						break;
					}
					imsg->Code=n->NextSelect;
				}
				break;
			case IDCMP_NEWSIZE :
				calcwinsizes();
				break;
			case IDCMP_MOUSEBUTTONS :
				if(imsg->Code==SELECTDOWN) {
					move=1;
					lastx=imsg->MouseX;
					lasty=imsg->MouseY;
				} else move=0;
				break;
		}
	}

	/* Handle Drag Window
	 */
	if(move!=0) {
		int mx,my,class,t1,t2;

		ReportMouse(TRUE,win);
		SetWindowPointer(win,WA_Pointer,(ULONG)mypointer,TAG_DONE);
		while(move!=0) {
			WaitPort(win->UserPort);
			imsg=(struct IntuiMessage *)GetMsg(win->UserPort);
			mx=imsg->MouseX;
			my=imsg->MouseY;
			class=imsg->Class;
			ReplyMsg((struct Message *)imsg);
			switch(class) {
				case IDCMP_MOUSEBUTTONS :
					move=0;
					break;
				case IDCMP_MOUSEMOVE :
					offx+=lastx-mx;	offy+=lasty-my;
					t1=mx;			t2=my;
					mx=lastx-mx;	my=lasty-my;
					lastx=t1;		lasty=t2;
					if(offx<0)		{ mx-=offx; offx=0; }
					if(offx>maxx)	{ mx-=offx-maxx; offx=maxx; }
					if(offy<0)		{ my-=offy; offy=0; }
					if(offy>maxy)	{ my-=offy-maxy; offy=maxy; }
					ScrollLayer(0,win->RPort->Layer,mx,my);
					break;
			}
		}
		SetWindowPointer(win,TAG_DONE);
		ReportMouse(FALSE,win);
	}
	return(ret);
}


static void getspeed(void) {
static char title[]=C2P_NAME " ...%";
static int notitle=0;
static int tbase;
	int i,szam1;

	tbase=NameLen+1;
	i=vec.c2p_Speed;
	if(i>0) {
		i+=5;
		title[tbase+0]=' ';
		if(i>1000) {
			title[tbase+0]=(i/1000)+'0';
			i%=1000;
		}
		szam1=i/100;
		title[tbase+1]=' ';
		if(szam1!=0) title[tbase+1]=szam1+'0';
		else if(title[tbase+0]!=' ') title[tbase+1]=szam1+'0';
		i%=100;
		title[tbase+2]=(i/10)+'0';
		title[tbase+3]='%';
		SetWindowTitles(win,title,(UBYTE *)~0);
		notitle=0;
	} else {
		title[tbase+0]=' ';
		title[tbase+1]=' ';
		title[tbase+2]=' ';
		title[tbase+3]=' ';
		if(notitle==0) SetWindowTitles(win,title,(UBYTE *)~0);
		notitle=1;
	}
} // getspeed()


int SAVEDS mdo(unsigned char *chunky,unsigned char *delta,int numscreen) {
	int ret,i,j,doit,size,width;
	unsigned long *ch,*de;
	unsigned char *p,*c;

	getspeed();
	ret=handleinput();
	if(ret!=RET_NEWWIN||ret!=RET_ERROR) {
		/* Do the c2p
		 */
		size=(SCRWIDTH*SCRHEIGHT)-SCRWIDTH;
		width=(SCRWIDTH>>2);
		chunky=chunky+size;
		delta=delta+size;
		i=SCRHEIGHT;
		do {
			ch=(unsigned long *)chunky;
			de=(unsigned long *)delta;
			j=width;
			doit=0;
			do {
				if(*ch++!=*de++) { doit=1; break; }
			} while(--j);
			if(doit!=0) {
				j=SCRWIDTH;
				c=chunky;
				p=PenArray;
				do { *p++=PenTable[*c++]; } while(--j);
				WritePixelLine8(raswpx,0,i-1,SCRWIDTH-1,PenArray,tmpraswpx);
			}
			chunky-=SCRWIDTH;
			delta-=SCRWIDTH;
		} while(--i);

		/* Copy into the window
		 */
		LockLayerRom(win->RPort->Layer);
		CopySBitMap(win->RPort->Layer);
		UnlockLayerRom(win->RPort->Layer);
	}
	return(ret);
} // do


int SAVEDS mdofull(unsigned char *chunky,int numscreen) {
	int ret,i;
	unsigned char *p,*c;

	getspeed();
	ret=handleinput();
	if(chunky!=NULL) {
		/* Convert chunky into PenArray
		 */	
		i=SCRSIZE;
		c=chunky;
		p=PenArray;
		do { *p++=PenTable[*c++]; } while(--i);
		if(ret!=RET_NEWWIN||ret!=RET_ERROR) {
			/* Do the c2p
			 */
			WritePixelArray8(raswpx,0,0,SCRWIDTH-1,SCRHEIGHT-1,PenArray,tmpraswpx);

			/* Copy into the window
			 */
			LockLayerRom(win->RPort->Layer);
			CopySBitMap(win->RPort->Layer);
			UnlockLayerRom(win->RPort->Layer);
		}
	}
	return(ret);
} // dofull


int SAVEDS mdont(void) {
	return(mdofull(NULL,0));
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
		}
	}
}
