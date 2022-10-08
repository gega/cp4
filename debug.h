/* :ts=8                        debug.h
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

#define GetString(g)      (((struct StringInfo *)g->SpecialInfo)->Buffer)
#define GetNumber(g)      (((struct StringInfo *)g->SpecialInfo)->LongInt)

#define GD_dpc                                 0
#define GD_daccu                               1
#define GD_dxreg                               2
#define GD_dyreg                               3
#define GD_dsp                                 4
#define GD_bflagstxt                           5
#define GD_bflags                              6
#define GD_drun                                7
#define GD_dtrace                              8
#define GD_dstep                               9
#define GD_dbrkpntcyc                          10
#define GD_donoffbrkpnt                        11
#define GD_dclear                              12
#define GD_badd                                13
#define GD_dispmode                            14
#define GD_amiras                              15
#define GD_cp4ras                              16
#define GD_vidmat                              17
#define GD_gfxbase                             18
#define GD_rasreq                              19
#define GD_dbgscroll                           20
#define GD_eirq                                21

#define GDX_dpc                                0
#define GDX_daccu                              1
#define GDX_dxreg                              2
#define GDX_dyreg                              3
#define GDX_dsp                                4
#define GDX_bflagstxt                          5
#define GDX_bflags                             6
#define GDX_drun                               7
#define GDX_dtrace                             8
#define GDX_dstep                              9
#define GDX_dbrkpntcyc                         10
#define GDX_donoffbrkpnt                       11
#define GDX_dclear                             12
#define GDX_badd                               13
#define GDX_dispmode                           14
#define GDX_amiras                             15
#define GDX_cp4ras                             16
#define GDX_vidmat                             17
#define GDX_gfxbase                            18
#define GDX_rasreq                             19
#define GDX_dbgscroll                          20
#define GDX_eirq                               21

#define GD_moncyc                              0
#define GD_monstr                              1
#define GD_monsea                              2
#define GD_monscroll                           3

#define GDX_moncyc                             0
#define GDX_monstr                             1
#define GDX_monsea                             2
#define GDX_monscroll                          3

// MENU
#define M_PROJECT               0
#define M_DEBUG                 1
#define M_MONITOR               2
// _PROJECT
#define MP_PREFS                0
#define MP_SAVEMEM              1
#define MP_BACK                 3
#define MP_QUIT                 5
// _DEBUG
#define MD_TRACE                0
#define MD_STEP                 1
#define MD_RUN                  2
#define MD_WITHDATA             4
#define MD_BREAKPOINTS          5
#define MD_ILLOPCODES           6
//_MONITOR
#define MM_OPENWIN              0
#define MM_FOLLOW               2
#define MM_ASCII                3
#define MM_SCRCODE              4

#define dbgwin_CNT 22
#define monwin_CNT 4

extern struct IntuitionBase *IntuitionBase;
extern struct Library       *GadToolsBase;

static struct Screen        *Scr;
static APTR                  VisualInfo;
static struct Window        *dbgwinWnd;
static struct Window        *monwinWnd;
static struct Gadget        *dbgwinGList;
static struct Gadget        *monwinGList;
static struct Menu          *dbgwinMenus;
static struct IntuiMessage   dbgwinMsg;
static struct IntuiMessage   monwinMsg;
static struct TextFont      *Font;
static struct Gadget        *dbgwinGadgets[22];
static struct Gadget        *monwinGadgets[4];
static UWORD                 dbgwinWidth;
static UWORD                 dbgwinHeight;
static UWORD                 monwinWidth;
static UWORD                 monwinHeight;
static struct TextAttr       topaz8;

static int dpcClicked(struct IntuiMessage *);
static int daccuClicked(struct IntuiMessage *);
static int dxregClicked(struct IntuiMessage *);
static int dyregClicked(struct IntuiMessage *);
static int dspClicked(struct IntuiMessage *);
static int drunClicked(struct IntuiMessage *);
static int dtraceClicked(struct IntuiMessage *);
static int dstepClicked(struct IntuiMessage *);
static int dbrkpntcycClicked(struct IntuiMessage *);
static int donoffbrkpntClicked(struct IntuiMessage *);
static int dclearClicked(struct IntuiMessage *);
static int baddClicked(struct IntuiMessage *);
static int dbgscrollClicked(struct IntuiMessage *);
static int moncycClicked(struct IntuiMessage *);
static int monstrClicked(struct IntuiMessage *);
static int monseaClicked(struct IntuiMessage *);
static int monscrollClicked(struct IntuiMessage *);
static int dbgwinmpprefs(struct IntuiMessage *);
static int dbgwinmpsave(struct IntuiMessage *);
static int dbgwinmpback(struct IntuiMessage *);
static int dbgwinmpreset(struct IntuiMessage *imsg);
static int dbgwinmpquit(struct IntuiMessage *);
static int dbgwinmdtrace(struct IntuiMessage *);
static int dbgwinmdwithdata(struct IntuiMessage *);
static int dbgwinmdstep(struct IntuiMessage *);
static int dbgwinmdrun(struct IntuiMessage *);
static int dbgwinmdbreak(struct IntuiMessage *);
static int dbgwinmdill(struct IntuiMessage *);
static int dbgwinmdstack(struct IntuiMessage *);
static int dbgwinmmopen(struct IntuiMessage *);
static int dbgwinmmfollow(struct IntuiMessage *);
static int dbgwinmmascii(struct IntuiMessage *);
static int dbgwinmmscrcode(struct IntuiMessage *);

static int SetupScreen( void );
static void CloseDownScreen( void );
static void dbgwinRender( void );
static int HandledbgwinIDCMP( void );

static int dbgwinCloseWindow(struct IntuiMessage *);
static int dbgwinVanillaKey(struct IntuiMessage *);
static void dbgcloseinput(void);
static int dbgwinRawKey(struct IntuiMessage *);

static int OpendbgwinWindow( void );
static void ClosedbgwinWindow( void );
static void monwinRender( void );
static int HandlemonwinIDCMP( void );

static int monwinCloseWindow(struct IntuiMessage *);
static int monwinVanillaKey(struct IntuiMessage *);
static int monwinRawKey(struct IntuiMessage *);

static int OpenmonwinWindow( void );
static void ClosemonwinWindow( void );


/*
 * stkwin
 */

#define GD_swinlst                             0
#define GDX_swinlst                            0
#define stkwin_CNT 1

static int swinlstClicked(struct IntuiMessage *imsg);
static int stkwinCloseWindow(void);

static int HandlestkwinIDCMP( void );
static int OpenstkwinWindow( void );
static void ClosestkwinWindow( void );

/*
 * savemem
 */
#define GD_sfromstr                            0
#define GD_stostr                              1
#define GD_ssaveb                              2
#define GD_scancelb                            3
#define GDX_sfromstr                           0
#define GDX_stostr                             1
#define GDX_ssaveb                             2
#define GDX_scancelb                           3
#define svmwin_CNT 4
extern struct Window        *svmwinWnd;
extern struct Gadget        *svmwinGList;
extern struct IntuiMessage   svmwinMsg;
extern struct Gadget        *svmwinGadgets[4];
extern UWORD                 svmwinLeft;
extern UWORD                 svmwinTop;
extern UWORD                 svmwinWidth;
extern UWORD                 svmwinHeight;
extern UBYTE                *svmwinWdt;
extern UWORD                 svmwinGTypes[];
extern struct NewGadget      svmwinNGad[];
extern ULONG                 svmwinGTags[];

extern int sfromstrClicked( void );
extern int stostrClicked( void );
extern int ssavebClicked( void );
extern int scancelbClicked( void );

extern int HandlesvmwinIDCMP( void );
extern int svmwinCloseWindow();
extern int OpensvmwinWindow(int left,int top,struct Screen *sc);
extern void ClosesvmwinWindow( void );

