/* :ts=8                        debug.c
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
#include <proto/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <proto/gadtools.h>
#include <libraries/gadtools.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <clib/utility_protos.h>
#include <string.h>
#include <clib/diskfont_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/diskfont_pragmas.h>

#include "debug.h"
#include "dbg.h"

#define DX      (-10)
#define DY      (-7)

extern struct GfxBase *GfxBase;
extern struct DiskfontBase *DiskfontBase;

static char s[32];
static int debugwinopened=0;                    // 0/2
static int monitorwinopened=0;                  // 0/1
static int stackwinopened=0;                    // 0/4
static int monitormenu=0;                       // 0/1
static struct Requester InvisibleRequesters[3];
static struct Screen *Scr=NULL;
static struct DrawInfo *drawinfo=NULL;
static APTR VisualInfo=NULL;
static struct Window *dbgwinWnd=NULL;
static struct Window *monwinWnd=NULL;
static struct Gadget *dbgwinGList=NULL;
static struct Gadget *monwinGList=NULL;
static struct Menu *dbgwinMenus=NULL;
static struct IntuiMessage dbgwinMsg;
static struct IntuiMessage monwinMsg;
static struct TextFont *Font=NULL;
static struct Gadget *dbgwinGadgets[22];
static struct Gadget *monwinGadgets[4];
static UWORD dbgwinWidth=480-8-21+2;
static UWORD dbgwinHeight=244-10;
static UWORD monwinWidth=397-8-2;
static UWORD monwinHeight=239-10-4;
static UWORD sx=0;                      // sx-=win->BorderLeft
static UWORD sy=0;                      // sy-=scr->font->ta_ysize
static struct Window *stkwinWnd=NULL;
static struct Gadget *stkwinGList=NULL;
static struct IntuiMessage stkwinMsg;
static struct Gadget *stkwinGadgets[1];
static UWORD stkwinWidth=128;
static UWORD stkwinHeight=114;
static int OkScreen=0;

// SAVEMEM
struct Window *svmwinWnd=NULL;
struct Gadget *svmwinGList=NULL;
struct IntuiMessage svmwinMsg;
struct Gadget *svmwinGadgets[4];
UWORD svmwinLeft = 91;
UWORD svmwinTop = 56;
UWORD svmwinWidth = 191;
UWORD svmwinHeight = 54;
UWORD svmwinGTypes[] = {
        STRING_KIND,
        STRING_KIND,
        BUTTON_KIND,
        BUTTON_KIND
};
ULONG svmwinGTags[] = {
        (GTST_MaxChars), 6, (STRINGA_Justification), (GACT_STRINGCENTER), (TAG_DONE),
        (GTST_MaxChars), 6, (STRINGA_Justification), (GACT_STRINGCENTER), (TAG_DONE),
        (TAG_DONE),
        (TAG_DONE)
};


static struct List swinlst0List;
static UWORD stkwinGTypes[] = {
        LISTVIEW_KIND
};

static struct NewGadget stkwinNGad[] = {
{       4, 2, 116, 112, NULL, NULL, GD_swinlst, 0, NULL, (APTR)swinlstClicked   }
};

static ULONG stkwinGTags[] = {
        (GTLV_Labels), (ULONG)&swinlst0List, (GTLV_ShowSelected), NULL, (TAG_DONE)
};

static UBYTE *dbrkpntcyc0Labels[]={
        (UBYTE *)"1: ",
        (UBYTE *)"2: ",
        (UBYTE *)"3: ",
        (UBYTE *)"4: ",
        (UBYTE *)"5: ",
        (UBYTE *)"6: ",
        (UBYTE *)"7: ",
        (UBYTE *)"8: ",
        (UBYTE *)"9: ",
        NULL };

static UBYTE *moncyc1Labels[]={
        (UBYTE *)"Ascii",
        (UBYTE *)"ScrCode",
        NULL };

static struct TextAttr topaz8={
        ( STRPTR )"topaz.font", 8, 0x00, 0x00 };

static UWORD dbgwinGTypes[]={
        STRING_KIND,
        STRING_KIND,
        STRING_KIND,
        STRING_KIND,
        STRING_KIND,
        TEXT_KIND,
        TEXT_KIND,
        BUTTON_KIND,
        BUTTON_KIND,
        BUTTON_KIND,
        CYCLE_KIND,
        CHECKBOX_KIND,
        BUTTON_KIND,
        BUTTON_KIND,
        TEXT_KIND,
        TEXT_KIND,
        TEXT_KIND,
        TEXT_KIND,
        TEXT_KIND,
        TEXT_KIND,
        SCROLLER_KIND,
        TEXT_KIND
};

static UWORD monwinGTypes[]={
        CYCLE_KIND,
        STRING_KIND,
        BUTTON_KIND,
        SCROLLER_KIND
};

#define DBGSCROLLMAX    16380
#define DBGREALMAX      (DBGSCROLLMAX<<2)
static ULONG dbgwinGTags[]={
        (GA_TabCycle), FALSE, (GTST_MaxChars), 4, (STRINGA_Justification), (GACT_STRINGCENTER), (TAG_DONE),
        (GA_TabCycle), FALSE, (GTST_MaxChars), 2, (STRINGA_Justification), (GACT_STRINGCENTER), (TAG_DONE),
        (GA_TabCycle), FALSE, (GTST_MaxChars), 2, (STRINGA_Justification), (GACT_STRINGCENTER), (TAG_DONE),
        (GA_TabCycle), FALSE, (GTST_MaxChars), 2, (STRINGA_Justification), (GACT_STRINGCENTER), (TAG_DONE),
        (GA_TabCycle), FALSE, (GTST_MaxChars), 2, (STRINGA_Justification), (GACT_STRINGCENTER), (TAG_DONE),
        (GTTX_Text), (ULONG)" N V B D I Z C", (GTTX_Border), TRUE, (TAG_DONE),
        (GTTX_Text), (ULONG)"              ", (GTTX_Border), TRUE, (TAG_DONE),
        (GT_Underscore), '_', (TAG_DONE),
        (GT_Underscore), '_', (TAG_DONE),
        (GT_Underscore), '_', (TAG_DONE),
        (GTCY_Labels), (ULONG)&dbrkpntcyc0Labels[ 0 ], (TAG_DONE),
        (TAG_DONE),
        (TAG_DONE),
        (TAG_DONE),
        (GTTX_Border), TRUE, GTTX_Justification, GTJ_CENTER, (TAG_DONE),
        (GTTX_Border), TRUE, GTTX_Justification, GTJ_CENTER, (TAG_DONE),
        (GTTX_Border), TRUE, GTTX_Justification, GTJ_CENTER, (TAG_DONE),
        (GTTX_Border), TRUE, GTTX_Justification, GTJ_CENTER, (TAG_DONE),
        (GTTX_Border), TRUE, GTTX_Justification, GTJ_CENTER, (TAG_DONE),
        (GTTX_Border), TRUE, GTTX_Justification, GTJ_CENTER, (TAG_DONE),
        (GTSC_Total), DBGSCROLLMAX, (GTSC_Visible), 2, (GTSC_Arrows), 11, (PGA_Freedom), LORIENT_VERT, (GA_Immediate), TRUE, (GA_RelVerify), TRUE, (TAG_DONE),
        (GTTX_Border), TRUE, GTTX_Justification, GTJ_CENTER, (TAG_DONE)
};

static ULONG monwinGTags[]={
        (GTCY_Labels), (ULONG)&moncyc1Labels[ 0 ], (TAG_DONE),
        (GTST_MaxChars), 127, (STRINGA_Justification), (GACT_STRINGCENTER), (TAG_DONE),
        (TAG_DONE),
        (GTSC_Total), 8192, (GTSC_Visible), 27, (GTSC_Arrows), 11, (PGA_Freedom), LORIENT_VERT, (GA_Immediate), TRUE, (GA_RelVerify), TRUE, (TAG_DONE)
};

static int SetupScreen( void ) {
        struct List *psl;
        struct Node *n;
        struct PubScreenNode *pn;

        if(!dbgwinWnd) {
                if(!(Font=OpenDiskFont(&topaz8))) return(5L);

                OkScreen=0;
                psl=LockPubScreenList();
                for(n=psl->lh_Head;n->ln_Succ!=NULL;n=n->ln_Succ) {
                        pn=(struct PubScreenNode *)n;
                        if(pn->psn_Screen==Scr&&(pn->psn_Flags&PSNF_PRIVATE)!=PSNF_PRIVATE) {
                                OkScreen=1;
                                break;
                        }
                }
                UnlockPubScreenList();
                if(OkScreen==0) Scr=LockPubScreen(NULL);
                ScreenToFront(Scr);
                if(!(drawinfo=GetScreenDrawInfo(Scr)))
                        return(10L);
                if(!(VisualInfo=GetVisualInfo(Scr,TAG_DONE)))
                        return(2L);
        }
        return(0L);
}

static void CloseDownScreen( void ) {
        if(debugwinopened+monitorwinopened+stackwinopened!=0) return;
        if(VisualInfo) {
                FreeVisualInfo(VisualInfo);
                VisualInfo=NULL;
        }
        if(drawinfo) {
                FreeScreenDrawInfo(Scr,drawinfo);
                drawinfo=NULL;
        }
        if(Scr&&!OkScreen) { UnlockPubScreen(NULL,Scr); Scr=NULL; }
        if(Font) {
                CloseFont(Font);
                Font=NULL;
        }
}

static void dbgwinRender(void) {
        UWORD offx, offy;

        offx=dbgwinWnd->BorderLeft+DX;
        offy=dbgwinWnd->BorderTop+DY;
        DrawBevelBox(dbgwinWnd->RPort,offx+12+1,offy+8+2,192,228,GT_VisualInfo,(ULONG)VisualInfo,TAG_DONE,0L);
}

static int HandledbgwinIDCMP(void) {
        struct IntuiMessage *m;
        struct MenuItem *n;
        int (*func)(struct IntuiMessage *);
        BOOL running=TRUE;

        while((m=GT_GetIMsg(dbgwinWnd->UserPort))) {
                CopyMem((char *)m,(char *)&dbgwinMsg,(long)sizeof(struct IntuiMessage));
                GT_ReplyIMsg(m);
                if(running!=TRUE) continue;
                switch(dbgwinMsg.Class) {
                        case    IDCMP_REFRESHWINDOW:
                                GT_BeginRefresh( dbgwinWnd );
                                dbgwinRender();
                                GT_EndRefresh( dbgwinWnd, TRUE );
                                break;
                        case    IDCMP_CLOSEWINDOW:
                                if(dbginput!=0) dbgcloseinput();
                                running=dbgwinCloseWindow(&dbgwinMsg);
                                break;
                        case    IDCMP_VANILLAKEY:
                                running=dbgwinVanillaKey(&dbgwinMsg);
                                break;
                        case    IDCMP_RAWKEY:
                                if(dbginput!=0) dbgcloseinput();
                                running=dbgwinRawKey(&dbgwinMsg);
                                break;
                        case    IDCMP_GADGETUP:
                        case    IDCMP_GADGETDOWN:
                                if(dbginput!=0) dbgcloseinput();
                                func=(void *)(((struct Gadget *)dbgwinMsg.IAddress)->UserData);
                                running=func(&dbgwinMsg);
                                break;
                        case    IDCMP_MENUPICK:
                                if(dbginput!=0) dbgcloseinput();
                                while(dbgwinMsg.Code!=MENUNULL) {
                                        n=ItemAddress(dbgwinMenus,dbgwinMsg.Code);
                                        func=(void *)(GTMENUITEM_USERDATA(n));
                                        running=func(&dbgwinMsg);
                                        dbgwinMsg.Code=n->NextSelect;
                                }
                        case    IDCMP_MOUSEBUTTONS:
                                if(dbginput!=0) dbgcloseinput();
                                dbgwinMsg.MouseX+=sx;
                                dbgwinMsg.MouseY-=sy;
                                dbgwinMouseButton(&dbgwinMsg);
                                break;
                        case    IDCMP_MOUSEMOVE:
                        case    IDCMP_INTUITICKS:
                                dbgwinMsg.MouseX+=sx;
                                dbgwinMsg.MouseY-=sy;
                                dbgwinMouseMove(&dbgwinMsg);
                                break;
                }
        }
        return(running);
}

static int OpendbgwinWindow(void) {
        struct NewGadget ng;
        struct Gadget *g;
        UWORD lc,tc;
        UWORD offx=Scr->WBorLeft+1,offy=Scr->WBorTop+Scr->RastPort.TxHeight+1+2;
        ULONG wa_top=WA_Top;
struct NewMenu dbgwinNewMenu[]={
{       NM_TITLE, (STRPTR)GetStr(MSG_0115), NULL, 0, NULL, NULL },
{       NM_ITEM, (STRPTR)GetStr(MSG_0116), NULL, 0, 0L, (APTR)dbgwinmpprefs     },
{       NM_ITEM, (STRPTR)GetStr(MSG_0117), (STRPTR)GetStr(MSG_0118), 0, 0L, (APTR)dbgwinmpsave  },
{       NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL },
{       NM_ITEM, (STRPTR)GetStr(MSG_0119), NULL, 0, 0L, (APTR)dbgwinmpreset     },
{       NM_ITEM, (STRPTR)GetStr(MSG_011A), (STRPTR)GetStr(MSG_011B), 0, 0L, (APTR)dbgwinmpback  },
{       NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL },
{       NM_ITEM, (STRPTR)GetStr(MSG_011C), NULL, 0, 0L, (APTR)dbgwinmpquit      },

{       NM_TITLE, (STRPTR)GetStr(MSG_0101), NULL, 0, NULL, NULL },
{       NM_ITEM, (STRPTR)GetStr(MSG_011D), (STRPTR)GetStr(MSG_011E), 0, 0L, (APTR)dbgwinmdtrace },
{       NM_ITEM, (STRPTR)GetStr(MSG_011F), (STRPTR)GetStr(MSG_0120), 0, 0L, (APTR)dbgwinmdstep  },
{       NM_ITEM, (STRPTR)GetStr(MSG_0121), (STRPTR)GetStr(MSG_0122), 0, 0L, (APTR)dbgwinmdrun   },
{       NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL },
{       NM_ITEM, (STRPTR)GetStr(MSG_0123), NULL, CHECKIT|MENUTOGGLE, 0L, (APTR)dbgwinmdwithdata },
{       NM_ITEM, (STRPTR)GetStr(MSG_0124), (STRPTR)GetStr(MSG_0125), CHECKIT|MENUTOGGLE, 0L, (APTR)dbgwinmdbreak        },
{       NM_ITEM, (STRPTR)GetStr(MSG_0126), (STRPTR)GetStr(MSG_0127), CHECKIT|MENUTOGGLE, 0L, (APTR)dbgwinmdill  },
{       NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL },
{       NM_ITEM, (STRPTR)GetStr(MSG_0128), (STRPTR)GetStr(MSG_0129), 0, 0L, (APTR)dbgwinmdstack },

{       NM_TITLE, (STRPTR)GetStr(MSG_012A), NULL, 0, NULL, NULL },
{       NM_ITEM, (STRPTR)GetStr(MSG_012B), (STRPTR)GetStr(MSG_012C), 0, 0L, (APTR)dbgwinmmopen  },
{       NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL },
{       NM_ITEM, (STRPTR)GetStr(MSG_012D), (STRPTR)GetStr(MSG_012E), CHECKIT|MENUTOGGLE, 0L, (APTR)dbgwinmmfollow       },
{       NM_ITEM, (STRPTR)GetStr(MSG_0110), (STRPTR)GetStr(MSG_012F), CHECKIT|MENUTOGGLE, 0x10L, (APTR)dbgwinmmascii     },
{       NM_ITEM, (STRPTR)GetStr(MSG_0130), (STRPTR)GetStr(MSG_0131), CHECKIT|MENUTOGGLE, 0x08L, (APTR)dbgwinmmscrcode   },
{       NM_END, NULL, NULL, 0, 0L, NULL }
};
struct NewGadget dbgwinNGad[]={
{       255, 8, 69, 14, (UBYTE *)"PC", NULL, GD_dpc, PLACETEXT_LEFT, NULL, (APTR)dpcClicked     },
{       255, 24, 69, 14, (UBYTE *)"A", NULL, GD_daccu, PLACETEXT_LEFT, NULL, (APTR)daccuClicked },
{       255, 40, 69, 14, (UBYTE *)"X", NULL, GD_dxreg, PLACETEXT_LEFT, NULL, (APTR)dxregClicked },
{       255, 56, 69, 14, (UBYTE *)"Y", NULL, GD_dyreg, PLACETEXT_LEFT, NULL, (APTR)dyregClicked },
{       255, 72, 69, 14, (UBYTE *)"SP", NULL, GD_dsp, PLACETEXT_LEFT, NULL, (APTR)dspClicked    },
{       331, 8, 128, 14, NULL, NULL, GD_bflagstxt, 0, NULL, NULL        },
{       331, 24, 128, 14, NULL, NULL, GD_bflags, 0, NULL, NULL  },
{       331, 40, 128, 14, (UBYTE *)GetStr(MSG_0136), NULL, GD_drun, PLACETEXT_IN, NULL, (APTR)drunClicked       },
{       331, 56, 128, 14, (UBYTE *)GetStr(MSG_0137), NULL, GD_dtrace, PLACETEXT_IN, NULL, (APTR)dtraceClicked   },
{       331, 72, 128, 14, (UBYTE *)GetStr(MSG_0138), NULL, GD_dstep, PLACETEXT_IN, NULL, (APTR)dstepClicked     },
{       255, 105, 132, 14, NULL, NULL, GD_dbrkpntcyc, 0, NULL, (APTR)dbrkpntcycClicked  },
{       255, 90, 26, 11, (UBYTE *)GetStr(MSG_0124), NULL, GD_donoffbrkpnt, PLACETEXT_RIGHT, NULL, (APTR)donoffbrkpntClicked     },
{       391, 88, 68, 14, (UBYTE *)GetStr(MSG_0139), NULL, GD_dclear, PLACETEXT_IN, NULL, (APTR)dclearClicked    },
{       391, 105, 68, 14, (UBYTE *)GetStr(MSG_013A), NULL, GD_badd, PLACETEXT_IN, NULL, (APTR)baddClicked       },
{       255, 123, 204, 14, NULL, NULL, GD_dispmode, 0, NULL, NULL       },
{       255, 141, 69, 14, (UBYTE *)GetStr(MSG_013B), NULL, GD_amiras, PLACETEXT_RIGHT, NULL, NULL       },
{       255, 157, 69, 14, (UBYTE *)GetStr(MSG_013C), NULL, GD_cp4ras, PLACETEXT_RIGHT, NULL, NULL       },
{       255, 189, 69, 14, (UBYTE *)GetStr(MSG_013D), NULL, GD_vidmat, PLACETEXT_RIGHT, NULL, NULL       },
{       255, 205, 69, 14, (UBYTE *)GetStr(MSG_013E), NULL, GD_gfxbase, PLACETEXT_RIGHT, NULL, NULL      },
{       255, 173, 69, 14, (UBYTE *)GetStr(MSG_013F), NULL, GD_rasreq, PLACETEXT_RIGHT, NULL, NULL       },
{       205, 8, 19, 228, NULL, NULL, GD_dbgscroll, 0, NULL, (APTR)dbgscrollClicked      },
{       255, 221, 69, 14, (UBYTE *)GetStr(MSG_0140), NULL, GD_eirq, PLACETEXT_RIGHT, NULL, NULL }
};

        if(DefDbgTop==-1) { DefDbgTop=0; wa_top=TAG_IGNORE; }
        if(dbgwinWnd) return(0);
        sx=Scr->WBorLeft-4+1;
        sy=(Scr->Font->ta_YSize-8)+(Scr->WBorTop-2)+2;

        if(!(g=CreateContext(&dbgwinGList))) return(1L);
        for(lc=0,tc=0;lc<dbgwin_CNT;lc++) {
                CopyMem((char * )&dbgwinNGad[lc],(char *)&ng,(long)sizeof(struct NewGadget));
                ng.ng_VisualInfo=VisualInfo;
                ng.ng_TextAttr  =&topaz8;
                ng.ng_LeftEdge  += offx+DX;
                ng.ng_TopEdge   += offy+DY;
                dbgwinGadgets[lc]=g=CreateGadgetA((ULONG)dbgwinGTypes[lc],g,&ng,(struct TagItem *)&dbgwinGTags[tc]);
                while(dbgwinGTags[tc]) tc+=2;
                tc++;
                if(NOT g) return(2L);
        }
        if(!(dbgwinMenus=CreateMenus(dbgwinNewMenu,GTMN_FrontPen,0L,TAG_DONE,0L))) return(3L);
        LayoutMenus(dbgwinMenus,VisualInfo,GTMN_TextAttr,(ULONG)Scr->Font,GTMN_NewLookMenus,TRUE,TAG_DONE,0L);
        if(!(dbgwinWnd=OpenWindowTags(NULL,
                                WA_Left,        DefDbgLeft,
                                wa_top,         DefDbgTop,
                                WA_InnerWidth,  dbgwinWidth,
                                WA_InnerHeight, dbgwinHeight,
                                WA_IDCMP,       STRINGIDCMP|
                                                TEXTIDCMP|
                                                BUTTONIDCMP|
                                                CYCLEIDCMP|
                                                CHECKBOXIDCMP|
                                                SCROLLERIDCMP|
                                                ARROWIDCMP|
                                                IDCMP_MENUPICK|
                                                IDCMP_CLOSEWINDOW|
                                                IDCMP_MOUSEBUTTONS|
                                                IDCMP_RAWKEY|
                                                IDCMP_VANILLAKEY|
                                                IDCMP_INTUITICKS|
                                                IDCMP_GADGETDOWN|
                                                IDCMP_REFRESHWINDOW,
                                WA_Flags,       WFLG_DRAGBAR|
                                                WFLG_DEPTHGADGET|
                                                WFLG_CLOSEGADGET|
                                                WFLG_SMART_REFRESH|
                                                WFLG_ACTIVATE,
                                WA_Gadgets,     (ULONG)dbgwinGList,
                                WA_Title,       (ULONG)(UBYTE *)GetStr(MSG_0265),
                                WA_ScreenTitle, (ULONG)GetStr(MSG_0114),
                                WA_NewLookMenus,TRUE,
                                WA_PubScreen,   (ULONG)Scr,
                                TAG_DONE,0L)))
                return(4L);

        SetMenuStrip(dbgwinWnd,dbgwinMenus);
        GT_RefreshWindow(dbgwinWnd,NULL);
        dbgwinRender();
        debugwinopened=2;
        return(0L);
}

static void ClosedbgwinWindow(void) {
        // store position
        if(!dbgwinWnd) return;
        DefDbgTop=dbgwinWnd->TopEdge;
        DefDbgLeft=dbgwinWnd->LeftEdge;
        sprintf(s,"%d",DefDbgTop);
        AddOption("DBGTOP",s);
        sprintf(s,"%d",DefDbgLeft);
        AddOption("DBGLEFT",s);
        if(dbgwinMenus) {
                if(monitormenu!=0) {
                        ClearMenuStrip(monwinWnd);
                        monitormenu=0;
                }
                ClearMenuStrip(dbgwinWnd);
                FreeMenus(dbgwinMenus);
                dbgwinMenus=NULL;
        }
        if(dbgwinWnd) {
                CloseWindow(dbgwinWnd);
                dbgwinWnd=NULL;
        }
        if(dbgwinGList) {
                FreeGadgets(dbgwinGList);
                dbgwinGList=NULL;
        }
        debugwinopened=0;
}

static void monwinRender(void) {
        UWORD offx,offy;

        offx=monwinWnd->BorderLeft;
        offy=monwinWnd->BorderTop;
        DrawBevelBox(monwinWnd->RPort,offx+2,offy+17,365,218,GT_VisualInfo,(ULONG)VisualInfo,TAG_DONE);
}

static int HandlemonwinIDCMP(void) {
        struct IntuiMessage *m;
        struct MenuItem *n;
        int (*func)(struct IntuiMessage *);
        BOOL running=TRUE;

        while(monwinWnd && (m=GT_GetIMsg(monwinWnd->UserPort))) {
                CopyMem((char *)m,(char *)&monwinMsg,(long)sizeof(struct IntuiMessage));
                GT_ReplyIMsg(m);
                if(running!=TRUE) continue;
                switch (monwinMsg.Class) {

                        case    IDCMP_REFRESHWINDOW:
                                GT_BeginRefresh( monwinWnd );
                                monwinRender();
                                GT_EndRefresh( monwinWnd, TRUE );
                                break;

                        case    IDCMP_CLOSEWINDOW:
                                running=monwinCloseWindow(&monwinMsg);
                                break;

                        case    IDCMP_VANILLAKEY:
                                running=monwinVanillaKey(&monwinMsg);
                                break;

                        case    IDCMP_RAWKEY:
                                running=monwinRawKey(&monwinMsg);
                                break;

                        case    IDCMP_GADGETUP:
                        case    IDCMP_GADGETDOWN:
                                func=(void *)((struct Gadget *)monwinMsg.IAddress)->UserData;
                                running=func(&monwinMsg);
                                break;

                        case    IDCMP_MENUPICK:
                                if(monitormenu!=0) {
                                        while(monwinMsg.Code!=MENUNULL) {
                                                n=ItemAddress(dbgwinMenus,monwinMsg.Code);
                                                func=(void *)(GTMENUITEM_USERDATA(n));
                                                running=func(&monwinMsg);
                                                monwinMsg.Code=n->NextSelect;
                                        }
                                }
                                break;
                        case    IDCMP_MOUSEBUTTONS:
                                monwinMsg.MouseX+=sx;
                                monwinMsg.MouseY-=sy;
                                monwinMouseButton(&monwinMsg);
                                break;
                        case    IDCMP_MOUSEMOVE:
                        case    IDCMP_INTUITICKS:
                                monwinMsg.MouseX+=sx;
                                monwinMsg.MouseY-=sy;
                                monwinMouseMove(&monwinMsg);
                                break;
                }
        }
        return(running);
}

static int OpenmonwinWindow(void) {
        struct NewGadget ng;
        struct Gadget *g;
        UWORD lc,tc;
        UWORD offx=Scr->WBorLeft,offy=Scr->WBorTop+Scr->RastPort.TxHeight+1;
        ULONG wa_top=WA_Top;
struct NewGadget monwinNGad[]={
{       2, 2, 121, 14, NULL, NULL, GD_moncyc, 0, NULL, (APTR)moncycClicked      },
{       123, 2, 170, 14, NULL, NULL, GD_monstr, 0, NULL, (APTR)monstrClicked    },
{       294, 2, 91, 14, (UBYTE *)GetStr(MSG_0142), NULL, GD_monsea, PLACETEXT_IN, NULL, (APTR)monseaClicked     },
{       368, 17, 17, 218, NULL, NULL, GD_monscroll, 0, NULL, (APTR)monscrollClicked     }
};

        if(DefMonTop==-1) { DefMonTop=0; wa_top=TAG_IGNORE; }
        if(monwinWnd) return(0);
        if(!dbgwinWnd) return(0);
        if(!(g=CreateContext(&monwinGList))) return(1L);
        for(lc=0,tc=0;lc<monwin_CNT;lc++) {
                CopyMem((char *)&monwinNGad[lc],(char *)&ng,(long)sizeof(struct NewGadget));
                ng.ng_VisualInfo =VisualInfo;
                ng.ng_TextAttr   =&topaz8;
                ng.ng_LeftEdge  +=offx;
                ng.ng_TopEdge   +=offy;
                monwinGadgets[lc]=g=CreateGadgetA((ULONG)monwinGTypes[lc],g,&ng,(struct TagItem *)&monwinGTags[tc]);
                while(monwinGTags[tc]) tc+=2;
                tc++;
                if(NOT g) return(2L);
        }
        if(!(monwinWnd=OpenWindowTags(NULL,
                                WA_Left,        DefMonLeft,
                                wa_top,         DefMonTop,
                                WA_InnerWidth,  monwinWidth,
                                WA_InnerHeight, monwinHeight+offy,
                                WA_IDCMP,       CYCLEIDCMP|
                                                STRINGIDCMP|
                                                BUTTONIDCMP|
                                                SCROLLERIDCMP|
                                                ARROWIDCMP|
                                                IDCMP_MENUPICK|
                                                IDCMP_CLOSEWINDOW|
                                                IDCMP_GADGETDOWN|
                                                IDCMP_INTUITICKS|
                                                IDCMP_RAWKEY|
                                                IDCMP_VANILLAKEY|
                                                IDCMP_MOUSEBUTTONS|
                                                IDCMP_REFRESHWINDOW,
                                WA_Flags,       WFLG_DRAGBAR|
                                                WFLG_DEPTHGADGET|
                                                WFLG_CLOSEGADGET|
                                                WFLG_SMART_REFRESH|
                                                WFLG_ACTIVATE,
                                WA_Gadgets,     (ULONG)monwinGList,
                                WA_Title,       (ULONG)(UBYTE *)GetStr(MSG_012A),
                                WA_ScreenTitle, (ULONG)GetStr(MSG_0114),
                                WA_NewLookMenus,TRUE,
                                WA_PubScreen,   (ULONG)Scr,
                                TAG_DONE )))
                return(4L);

        if(dbgwinWnd) {
                SetMenuStrip(monwinWnd,dbgwinMenus);
                monitormenu=1;
        }
        GT_RefreshWindow(monwinWnd,NULL);
        monwinRender();
        if(DefMonSea[0]!='\0') GT_SetGadgetAttrs(monwinGadgets[GD_monstr],monwinWnd,NULL,GTST_String,(ULONG)&DefMonSea,TAG_DONE);
        GT_SetGadgetAttrs(monwinGadgets[GD_moncyc],monwinWnd,NULL,GTCY_Active,DefMonCyc,TAG_DONE);
        if(DefMonCyc!=0) {
                ctab=cstab;
                setcheckmark(M_MONITOR,MM_ASCII,FALSE);
                setcheckmark(M_MONITOR,MM_SCRCODE,TRUE);
        } else {
                ctab=catab;
                setcheckmark(M_MONITOR,MM_ASCII,TRUE);
                setcheckmark(M_MONITOR,MM_SCRCODE,FALSE);
        }
        monitorwinopened=1;
        return(0);
}

static void ClosemonwinWindow(void) {
        ULONG act;
        // store position
        DefMonTop=monwinWnd->TopEdge;
        DefMonLeft=monwinWnd->LeftEdge;
        sprintf(s,"%d",DefMonTop);
        AddOption("MONTOP",s);
        sprintf(s,"%d",DefMonLeft);
        AddOption("MONLEFT",s);
        GT_GetGadgetAttrs(monwinGadgets[GD_moncyc],monwinWnd,NULL,GTCY_Active,(ULONG)&act,TAG_DONE);
        DefMonCyc=act;
        if(monitormenu!=0) {
                ClearMenuStrip(monwinWnd);
                monitormenu=0;
        }
        if(monwinWnd) {
                CloseWindow(monwinWnd);
                monwinWnd=NULL;
        }
        if(monwinGList) {
                FreeGadgets(monwinGList);
                monwinGList=NULL;
        }
        monitorwinopened=0;
}

static int OpenstkwinWindow(void) {
        struct NewGadget ng;
        struct Gadget *g;
        UWORD lc,tc;
        UWORD offx=Scr->WBorLeft,offy=Scr->WBorTop+Scr->RastPort.TxHeight+1;
        ULONG wa_top=WA_Top;

        if(DefStkTop==-1) { DefStkTop=0; wa_top=TAG_IGNORE; }
        if(stkwinWnd) return(0);
        // init listview list
        swinlst0List.lh_Head=(struct Node *)&swinlst0List.lh_Tail;
        swinlst0List.lh_Tail=NULL;
        swinlst0List.lh_TailPred=(struct Node *)&swinlst0List.lh_Head;
        if(!(g=CreateContext(&stkwinGList))) return( 1L );
        for(lc=0,tc=0;lc<stkwin_CNT;lc++) {
                CopyMem((char * )&stkwinNGad[ lc ], (char * )&ng, (long)sizeof( struct NewGadget ));
                ng.ng_VisualInfo = VisualInfo;
                ng.ng_TextAttr   = &topaz8;
                ng.ng_LeftEdge  += offx;
                ng.ng_TopEdge   += offy;
                stkwinGadgets[lc]=g=CreateGadgetA((ULONG)stkwinGTypes[lc],g,&ng,(struct TagItem *)&stkwinGTags[tc]);
                while(stkwinGTags[tc]) tc+=2;
                tc++;
                if(NOT g) return(2L);
        }

        if ( ! ( stkwinWnd = OpenWindowTags( NULL,
                                WA_Left,        DefStkLeft,
                                wa_top,         DefStkTop,
                                WA_Width,       stkwinWidth+offx,
                                WA_Height,      stkwinHeight+offy,
                                WA_IDCMP,       LISTVIEWIDCMP|
                                                IDCMP_CLOSEWINDOW|
                                                IDCMP_REFRESHWINDOW,
                                WA_Flags,       WFLG_DRAGBAR|
                                                WFLG_DEPTHGADGET|
                                                WFLG_CLOSEGADGET|
                                                WFLG_SMART_REFRESH|
                                                WFLG_RMBTRAP|
                                                WFLG_ACTIVATE,
                                WA_Gadgets,     (ULONG)stkwinGList,
                                WA_Title,       (ULONG)(UBYTE *)GetStr(MSG_0143),
                                WA_ScreenTitle, (ULONG)GetStr(MSG_0114),
                                WA_PubScreen,   (ULONG)Scr,
                                TAG_DONE )))
        return(4L);
        stackwinopened=4;
        GT_RefreshWindow(stkwinWnd,NULL);
        return(0L);
}

static void ClosestkwinWindow(void) {
        if(!stkwinWnd) return;
        DefStkTop=stkwinWnd->TopEdge;
        DefStkLeft=stkwinWnd->LeftEdge;
        sprintf(s,"%d",DefStkTop);
        AddOption("STKTOP",s);
        sprintf(s,"%d",DefStkLeft);
        AddOption("STKLEFT",s);
        if(stkwinWnd) {
                CloseWindow(stkwinWnd);
                stkwinWnd=NULL;
        }
        if(stkwinGList) {
                FreeGadgets(stkwinGList);
                stkwinGList=NULL;
        }
        stackwinopened=0;
}


static int HandlestkwinIDCMP( void ) {
        struct IntuiMessage *m;
        int (*func)(struct IntuiMessage *);
        BOOL running=TRUE;

        if(!stkwinWnd) return(TRUE);

        while(stkwinWnd&&(m=GT_GetIMsg(stkwinWnd->UserPort))) {
                CopyMem((char *)m,(char *)&stkwinMsg,(long)sizeof(struct IntuiMessage));
                GT_ReplyIMsg(m);
                switch(stkwinMsg.Class) {
                        case    IDCMP_REFRESHWINDOW:
                                GT_BeginRefresh(stkwinWnd);
                                GT_EndRefresh(stkwinWnd,TRUE);
                                break;
                        case    IDCMP_CLOSEWINDOW:
                                running=stkwinCloseWindow();
                                break;
                        case    IDCMP_GADGETUP:
                        case    IDCMP_GADGETDOWN:
                                func=(void *)((struct Gadget *)stkwinMsg.IAddress)->UserData;
                                running=func(&stkwinMsg);
                                break;
                }
        }
        return(running);
}

/*
 * SAVEMEM
 */

int HandlesvmwinIDCMP(void) {
        struct IntuiMessage *m;
        int (*func)();
        int running=0;

        while((m=GT_GetIMsg(svmwinWnd->UserPort))) {
                CopyMem((char *)m,(char *)&svmwinMsg,(long)sizeof(struct IntuiMessage));
                GT_ReplyIMsg(m);
                switch(svmwinMsg.Class) {
                        case    IDCMP_REFRESHWINDOW:
                                GT_BeginRefresh(svmwinWnd);
                                GT_EndRefresh(svmwinWnd,TRUE);
                                break;
                        case    IDCMP_CLOSEWINDOW:
                                running=svmwinCloseWindow();
                                break;
                        case    IDCMP_GADGETUP:
                                func=(void *)((struct Gadget *)svmwinMsg.IAddress)->UserData;
                                running=func();
                                break;
                }
        }
        return(running);
}

int OpensvmwinWindow(int left,int top,struct Screen *sc) {
        struct NewGadget ng;
        struct Gadget *g;
        UWORD lc, tc;
        UWORD offx=sc->WBorLeft,offy=sc->WBorTop+sc->RastPort.TxHeight+1;
        ULONG wa_top=WA_Top;
struct NewGadget svmwinNGad[] = {
{       3, 2, 83, 14, (UBYTE *)GetStr(MSG_0145), NULL, GD_sfromstr, PLACETEXT_RIGHT, NULL, (APTR)sfromstrClicked },
{       3, 18, 83, 14, (UBYTE *)GetStr(MSG_0146), NULL, GD_stostr, PLACETEXT_RIGHT, NULL, (APTR)stostrClicked },
{       3, 35, 83, 15, (UBYTE *)GetStr(MSG_0147), NULL, GD_ssaveb, PLACETEXT_IN, NULL, (APTR)ssavebClicked },
{       96, 35, 83, 15, (UBYTE *)GetStr(MSG_0148), NULL, GD_scancelb, PLACETEXT_IN, NULL, (APTR)scancelbClicked }
};

        if(top==-1) { wa_top=TAG_IGNORE; top=0; }
        if(!(g=CreateContext(&svmwinGList))) return(1L);
        for(lc=0,tc=0;lc<svmwin_CNT;lc++) {
                CopyMem((char *)&svmwinNGad[lc],(char *)&ng,(long)sizeof(struct NewGadget));
                ng.ng_VisualInfo = VisualInfo;
                ng.ng_TextAttr   = &topaz8;
                ng.ng_LeftEdge  += offx;
                ng.ng_TopEdge   += offy;
                svmwinGadgets[lc]=g=CreateGadgetA((ULONG)svmwinGTypes[lc],g,&ng,(struct TagItem *)&svmwinGTags[tc]);
                while(svmwinGTags[tc]) tc+=2;
                tc++;
                if(NOT g) return(2L);
        }
        if(!(svmwinWnd=OpenWindowTags(NULL,
                                WA_Left,        left,
                                wa_top,         top,
                                WA_Width,       svmwinWidth,
                                WA_Height,      svmwinHeight + offy,
                                WA_IDCMP,       STRINGIDCMP|
                                                BUTTONIDCMP|
                                                IDCMP_CLOSEWINDOW|
                                                IDCMP_REFRESHWINDOW,
                                WA_Flags,       WFLG_DRAGBAR|
                                                WFLG_DEPTHGADGET|
                                                WFLG_CLOSEGADGET|
                                                WFLG_SMART_REFRESH|
                                                WFLG_ACTIVATE|
                                                WFLG_RMBTRAP,
                                WA_Gadgets,     (ULONG)svmwinGList,
                                WA_Title,       (ULONG)(UBYTE *)GetStr(MSG_0144),
                                WA_PubScreen,   (ULONG)Scr,
                                TAG_DONE )))
        return(4L);
        GT_RefreshWindow(svmwinWnd,NULL);
        return(0L);
}

void ClosesvmwinWindow(void) {
        if(svmwinWnd) {
                CloseWindow(svmwinWnd);
                svmwinWnd=NULL;
        }
        if(svmwinGList) {
                FreeGadgets(svmwinGList);
                svmwinGList=NULL;
        }
}


static void SleepWindows(void) {
static struct TagItem BusyPointerTagList[]={
        {WA_BusyPointer,TRUE},
        {TAG_END,0}
};
        if(dbgwinWnd) {
                InitRequester(&InvisibleRequesters[0]);
                Request(&InvisibleRequesters[0],dbgwinWnd);
                SetWindowPointerA(dbgwinWnd,BusyPointerTagList);
        }
        if(monwinWnd) {
                InitRequester(&InvisibleRequesters[1]);
                Request(&InvisibleRequesters[1],monwinWnd);
                SetWindowPointerA(monwinWnd,BusyPointerTagList);
        }
        if(stkwinWnd) {
                InitRequester(&InvisibleRequesters[1]);
                Request(&InvisibleRequesters[2],stkwinWnd);
                SetWindowPointerA(stkwinWnd,BusyPointerTagList);
        }
}

static void AwakeWindows(void) {
        if(dbgwinWnd) {
                EndRequest(&InvisibleRequesters[0],dbgwinWnd);
                SetWindowPointerA(dbgwinWnd,NULL);
        }
        if(monwinWnd) {
                EndRequest(&InvisibleRequesters[1],monwinWnd);
                SetWindowPointerA(monwinWnd,NULL);
        }
        if(stkwinWnd) {
                EndRequest(&InvisibleRequesters[2],stkwinWnd);
                SetWindowPointerA(stkwinWnd,NULL);
        }
}


