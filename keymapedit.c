/* :ts=8							keymapedit.c
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

#include <proto/diskfont.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxbase.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <string.h>
#include <stdio.h>
#include <clib/diskfont_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/diskfont_pragmas.h>

#include "prefs.h"
#include "keymapedit.h"

// gui
int emx,emy;		// mouse x/y at edit win idcmps
static struct Library *DiskfontBase=NULL;
static struct Screen         *Scr = NULL;
static APTR                   VisualInfo = NULL;
       struct Window         *kmapeditWnd = NULL;
static struct Window         *kmapassignWnd = NULL;
static struct Window         *kmapfunctionsWnd = NULL;
static struct Gadget         *kmapeditGList = NULL;
static struct Gadget         *kmapassignGList = NULL;
static struct Gadget         *kmapfunctionsGList = NULL;
struct IntuiMessage    kmapeditMsg;
struct IntuiMessage    kmapassignMsg;
struct IntuiMessage    kmapfunctionsMsg;
static struct TextFont       *Font       = NULL;
static struct Gadget         *kmapeditGadgets[134];
static struct Gadget         *kmapassignGadgets[5];
static struct Gadget         *kmapfunctionsGadgets[5];
static UWORD                  kmapeditLeft = 16;
static int                    kmapeditTop = -1;
static UWORD                  kmapeditWidth = 546;
static UWORD                  kmapeditHeight = 166;
static UWORD                  kmapassignLeft = 71;
static UWORD                  kmapassignTop = 69;
static UWORD                  kmapassignWidth = 397;
static UWORD                  kmapassignHeight = 57;
static UWORD                  kmapfunctionsLeft = 10;
static UWORD                  kmapfunctionsTop = 15;
static UWORD                  kmapfunctionsWidth = 291;
static UWORD                  kmapfunctionsHeight = 95;
static UBYTE                 *kmapeditWdt = (UBYTE *)"";

static struct TextAttr topaz8 = {
	( STRPTR )"topaz.font", 8, 0x00, 0x00 };

static UWORD kmapeditGTypes[] = {
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	TEXT_KIND,
	BUTTON_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	TEXT_KIND,
	BUTTON_KIND,
	TEXT_KIND
};

static UWORD kmapassignGTypes[] = {
	TEXT_KIND,
	TEXT_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND
};

static UWORD kmapfunctionsGTypes[] = {
	LISTVIEW_KIND,
	TEXT_KIND,
	TEXT_KIND,
	BUTTON_KIND,
	BUTTON_KIND
};

static ULONG kmapeditGTags[] = {
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE)
};

static ULONG kmapassignGTags[] = {
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(TAG_DONE),
	(TAG_DONE),
	(TAG_DONE)
};

int HandlekmapeditIDCMP( void )
{
	struct IntuiMessage	*m;
	int			(*func)();
	BOOL			running = TRUE;

	while( (m = GT_GetIMsg( kmapeditWnd->UserPort ))) {

		CopyMem(( char * )m, ( char * )&kmapeditMsg, (long)sizeof( struct IntuiMessage ));

		GT_ReplyIMsg( m );

		emx=kmapeditMsg.MouseX;
		emy=kmapeditMsg.MouseY;

		switch ( kmapeditMsg.Class ) {

			case	IDCMP_REFRESHWINDOW:
				GT_BeginRefresh( kmapeditWnd );
				GT_EndRefresh( kmapeditWnd, TRUE );
				break;

			case	IDCMP_CLOSEWINDOW:
				running = kmapeditCloseWindow();
				break;

			case	IDCMP_RAWKEY:
				running = kmapeditRawKey(&kmapeditMsg);
				break;

			case	IDCMP_GADGETUP:
				func = ( void * )(( struct Gadget * )kmapeditMsg.IAddress )->UserData;
				running = func();
				break;
			case	IDCMP_GADGETHELP:
				running = kmapeditGadgetHelp(&kmapeditMsg);
				break;
		}
	}
	return( running );
}

int OpenkmapeditWindow( void )
{
	struct NewGadget	ng;
	struct Gadget	*g;
	UWORD		lc, tc;
	UWORD		offx = Scr->WBorLeft, offy = Scr->WBorTop + Scr->RastPort.TxHeight + 1;
	ULONG wa_top=WA_Top;
struct NewGadget kmapeditNGad[] = {
{	6, 3, 46, 15, (UBYTE *)"@", NULL, GD_kkukac, PLACETEXT_IN, NULL, (APTR)kkukacClicked },
{	73, 3, 46, 15, (UBYTE *)"F3", NULL, GD_kf3, PLACETEXT_IN, NULL, (APTR)kf3Clicked },
{	140, 3, 46, 15, (UBYTE *)"F2", NULL, GD_kf2, PLACETEXT_IN, NULL, (APTR)kf2Clicked },
{	207, 3, 46, 15, (UBYTE *)"F1", NULL, GD_kf1, PLACETEXT_IN, NULL, (APTR)kf1Clicked },
{	274, 3, 46, 15, (UBYTE *)"HELP", NULL, GD_khelp, PLACETEXT_IN, NULL, (APTR)khelpClicked },
{	341, 3, 46, 15, (UBYTE *)"£", NULL, GD_kfont, PLACETEXT_IN, NULL, (APTR)kfontClicked },
{	408, 3, 46, 15, (UBYTE *)"RET", NULL, GD_kreturn, PLACETEXT_IN, NULL, (APTR)kreturnClicked },
{	475, 3, 46, 15, (UBYTE *)"INST", NULL, GD_kinstdel, PLACETEXT_IN, NULL, (APTR)kinstdelClicked },

{	6, 19, 46, 15, (UBYTE *)"SH", NULL, GD_kshift, PLACETEXT_IN, NULL, (APTR)kshiftClicked },
{	73, 19, 46, 15, (UBYTE *)"E", NULL, GD_ke, PLACETEXT_IN, NULL, (APTR)keClicked },
{	140, 19, 46, 15, (UBYTE *)"S", NULL, GD_ks, PLACETEXT_IN, NULL, (APTR)ksClicked },
{	207, 19, 46, 15, (UBYTE *)"Z", NULL, GD_kz, PLACETEXT_IN, NULL, (APTR)kzClicked },
{	274, 19, 46, 15, (UBYTE *)"4", NULL, GD_k4, PLACETEXT_IN, NULL, (APTR)k4Clicked },
{	341, 19, 46, 15, (UBYTE *)"A", NULL, GD_ka, PLACETEXT_IN, NULL, (APTR)kaClicked },
{	408, 19, 46, 15, (UBYTE *)"W", NULL, GD_kw, PLACETEXT_IN, NULL, (APTR)kwClicked },
{	475, 19, 46, 15, (UBYTE *)"3", NULL, GD_k3, PLACETEXT_IN, NULL, (APTR)k3Clicked },

{	6, 35, 46, 15, (UBYTE *)"X", NULL, GD_kx, PLACETEXT_IN, NULL, (APTR)kxClicked },
{	73, 35, 46, 15, (UBYTE *)"T", NULL, GD_kt, PLACETEXT_IN, NULL, (APTR)ktClicked },
{	140, 35, 46, 15, (UBYTE *)"F", NULL, GD_kf, PLACETEXT_IN, NULL, (APTR)kfClicked },
{	207, 35, 46, 15, (UBYTE *)"C", NULL, GD_kc, PLACETEXT_IN, NULL, (APTR)kcClicked },
{	274, 35, 46, 15, (UBYTE *)"6", NULL, GD_k6, PLACETEXT_IN, NULL, (APTR)k6Clicked },
{	341, 35, 46, 15, (UBYTE *)"D", NULL, GD_kd, PLACETEXT_IN, NULL, (APTR)kdClicked },
{	408, 35, 46, 15, (UBYTE *)"R", NULL, GD_kr, PLACETEXT_IN, NULL, (APTR)krClicked },
{	475, 35, 46, 15, (UBYTE *)"5", NULL, GD_k5, PLACETEXT_IN, NULL, (APTR)k5Clicked },

{	6, 51, 46, 15, (UBYTE *)"V", NULL, GD_kv, PLACETEXT_IN, NULL, (APTR)kvClicked },
{	73, 51, 46, 15, (UBYTE *)"U", NULL, GD_ku, PLACETEXT_IN, NULL, (APTR)kuClicked },
{	140, 51, 46, 15, (UBYTE *)"H", NULL, GD_kh, PLACETEXT_IN, NULL, (APTR)khClicked },
{	207, 51, 46, 15, (UBYTE *)"B", NULL, GD_kb, PLACETEXT_IN, NULL, (APTR)kbClicked },
{	274, 51, 46, 15, (UBYTE *)"8", NULL, GD_k8, PLACETEXT_IN, NULL, (APTR)k8Clicked },
{	341, 51, 46, 15, (UBYTE *)"G", NULL, GD_kg, PLACETEXT_IN, NULL, (APTR)kgClicked },
{	408, 51, 46, 15, (UBYTE *)"Y", NULL, GD_ky, PLACETEXT_IN, NULL, (APTR)kyClicked },
{	475, 51, 46, 15, (UBYTE *)"7", NULL, GD_k7, PLACETEXT_IN, NULL, (APTR)k7Clicked },

{	6, 67, 46, 15, (UBYTE *)"N", NULL, GD_kn, PLACETEXT_IN, NULL, (APTR)knClicked },
{	73, 67, 46, 15, (UBYTE *)"O", NULL, GD_ko, PLACETEXT_IN, NULL, (APTR)koClicked },
{	140, 67, 46, 15, (UBYTE *)"K", NULL, GD_kk, PLACETEXT_IN, NULL, (APTR)kkClicked },
{	207, 67, 46, 15, (UBYTE *)"M", NULL, GD_km, PLACETEXT_IN, NULL, (APTR)kmClicked },
{	274, 67, 46, 15, (UBYTE *)"0", NULL, GD_k0, PLACETEXT_IN, NULL, (APTR)k0Clicked },
{	341, 67, 46, 15, (UBYTE *)"J", NULL, GD_kj, PLACETEXT_IN, NULL, (APTR)kjClicked },
{	408, 67, 46, 15, (UBYTE *)"I", NULL, GD_ki, PLACETEXT_IN, NULL, (APTR)kiClicked },
{	475, 67, 46, 15, (UBYTE *)"9", NULL, GD_k9, PLACETEXT_IN, NULL, (APTR)k9Clicked },

{	6, 83, 46, 15, (UBYTE *)",", NULL, GD_kvesszo, PLACETEXT_IN, NULL, (APTR)kvesszoClicked },
{	73, 83, 46, 15, (UBYTE *)"-", NULL, GD_kminusz, PLACETEXT_IN, NULL, (APTR)kminuszClicked },
{	140, 83, 46, 15, (UBYTE *)":", NULL, GD_kkettospont, PLACETEXT_IN, NULL, (APTR)kkettospontClicked },
{	207, 83, 46, 15, (UBYTE *)".", NULL, GD_kpont, PLACETEXT_IN, NULL, (APTR)kpontClicked },
{	274, 83, 46, 15, (UBYTE *)"UP", NULL, GD_kup, PLACETEXT_IN, NULL, (APTR)kupClicked },
{	341, 83, 46, 15, (UBYTE *)"L", NULL, GD_kl, PLACETEXT_IN, NULL, (APTR)klClicked },
{	408, 83, 46, 15, (UBYTE *)"P", NULL, GD_kp, PLACETEXT_IN, NULL, (APTR)kpClicked },
{	475, 83, 46, 15, (UBYTE *)"DOWN", NULL, GD_kdown, PLACETEXT_IN, NULL, (APTR)kdownClicked },

{	6, 99, 46, 15, (UBYTE *)"/", NULL, GD_kper, PLACETEXT_IN, NULL, (APTR)kperClicked },
{	73, 99, 46, 15, (UBYTE *)"+", NULL, GD_kplusz, PLACETEXT_IN, NULL, (APTR)kpluszClicked },
{	140, 99, 46, 15, (UBYTE *)"=", NULL, GD_kegyenlo, PLACETEXT_IN, NULL, (APTR)kegyenloClicked },
{	207, 99, 46, 15, (UBYTE *)"ESC", NULL, GD_kescape, PLACETEXT_IN, NULL, (APTR)kescapeClicked },
{	274, 99, 46, 15, (UBYTE *)"->", NULL, GD_kright, PLACETEXT_IN, NULL, (APTR)krightClicked },
{	341, 99, 46, 15, (UBYTE *)";", NULL, GD_kpontosvesszo, PLACETEXT_IN, NULL, (APTR)kpontosvesszoClicked },
{	408, 99, 46, 15, (UBYTE *)"*", NULL, GD_kcsillag, PLACETEXT_IN, NULL, (APTR)kcsillagClicked },
{	475, 99, 46, 15, (UBYTE *)"<-", NULL, GD_kleft, PLACETEXT_IN, NULL, (APTR)kleftClicked },

{	6, 115, 46, 15, (UBYTE *)"STOP", NULL, GD_krunstop, PLACETEXT_IN, NULL, (APTR)krunstopClicked },
{	73, 115, 46, 15, (UBYTE *)"Q", NULL, GD_kq, PLACETEXT_IN, NULL, (APTR)kqClicked },
{	140, 115, 46, 15, (UBYTE *)"C=", NULL, GD_kcommodore, PLACETEXT_IN, NULL, (APTR)kcommodoreClicked },
{	207, 115, 46, 15, (UBYTE *)"SPC", NULL, GD_kspace, PLACETEXT_IN, NULL, (APTR)kspaceClicked },
{	274, 115, 46, 15, (UBYTE *)"2", NULL, GD_k2, PLACETEXT_IN, NULL, (APTR)k2Clicked },
{	341, 115, 46, 15, (UBYTE *)"CTRL", NULL, GD_kcontrol, PLACETEXT_IN, NULL, (APTR)kcontrolClicked },
{	408, 115, 46, 15, (UBYTE *)"CLR", NULL, GD_kclearhome, PLACETEXT_IN, NULL, (APTR)kclearhomeClicked },
{	475, 115, 46, 15, (UBYTE *)"1", NULL, GD_k1, PLACETEXT_IN, NULL, (APTR)k1Clicked },

{	140, 149, 132, 15, (UBYTE *)GetStr(MSG_017A), NULL, GD_b_open, PLACETEXT_IN, NULL, (APTR)b_openClicked },
{	274, 149, 132, 15, (UBYTE *)GetStr(MSG_0147), NULL, GD_b_save, PLACETEXT_IN, NULL, (APTR)b_saveClicked },
{	408, 149, 132, 15, (UBYTE *)GetStr(MSG_017B), NULL, GD_b_cancel, PLACETEXT_IN, NULL, (APTR)b_cancelClicked },
{	6, 132, 400, 15, NULL, NULL, GD_b_status, 0, NULL, NULL },
{	6, 149, 132, 15, (UBYTE *)GetStr(MSG_017C), NULL, GD_b_new, PLACETEXT_IN, NULL, (APTR)b_newClicked },

{	121, 3, 17, 15, NULL, NULL, GD_tf3, 0, NULL, NULL },
{	188, 3, 17, 15, NULL, NULL, GD_tf2, 0, NULL, NULL },
{	255, 3, 17, 15, NULL, NULL, GD_tf1, 0, NULL, NULL },
{	322, 3, 17, 15, NULL, NULL, GD_thelp, 0, NULL, NULL },
{	389, 3, 17, 15, NULL, NULL, GD_tfont, 0, NULL, NULL },
{	456, 3, 17, 15, NULL, NULL, GD_treturn, 0, NULL, NULL },
{	523, 3, 17, 15, NULL, NULL, GD_tinstdel, 0, NULL, NULL },

{	54, 19, 17, 15, NULL, NULL, GD_tshift, 0, NULL, NULL },
{	121, 19, 17, 15, NULL, NULL, GD_te, 0, NULL, NULL },
{	188, 19, 17, 15, NULL, NULL, GD_ts, 0, NULL, NULL },
{	255, 19, 17, 15, NULL, NULL, GD_tz, 0, NULL, NULL },
{	322, 19, 16, 15, NULL, NULL, GD_t4, 0, NULL, NULL },
{	389, 19, 17, 15, NULL, NULL, GD_ta, 0, NULL, NULL },
{	456, 19, 17, 15, NULL, NULL, GD_tw, 0, NULL, NULL },
{	523, 19, 17, 15, NULL, NULL, GD_t3, 0, NULL, NULL },

{	54, 35, 17, 15, NULL, NULL, GD_tx, 0, NULL, NULL },
{	121, 35, 17, 15, NULL, NULL, GD_tt, 0, NULL, NULL },
{	188, 35, 17, 15, NULL, NULL, GD_tf, 0, NULL, NULL },
{	255, 35, 17, 15, NULL, NULL, GD_tc, 0, NULL, NULL },
{	322, 35, 17, 15, NULL, NULL, GD_t6, 0, NULL, NULL },
{	389, 35, 17, 15, NULL, NULL, GD_td, 0, NULL, NULL },
{	456, 35, 17, 15, NULL, NULL, GD_tr, 0, NULL, NULL },
{	523, 35, 17, 15, NULL, NULL, GD_t5, 0, NULL, NULL },

{	54, 51, 17, 15, NULL, NULL, GD_tv, 0, NULL, NULL },
{	121, 51, 17, 15, NULL, NULL, GD_tu, 0, NULL, NULL },
{	188, 51, 17, 15, NULL, NULL, GD_th, 0, NULL, NULL },
{	255, 51, 17, 15, NULL, NULL, GD_tb, 0, NULL, NULL },
{	322, 51, 17, 15, NULL, NULL, GD_t8, 0, NULL, NULL },
{	389, 51, 17, 15, NULL, NULL, GD_tg, 0, NULL, NULL },
{	456, 51, 17, 15, NULL, NULL, GD_ty, 0, NULL, NULL },
{	523, 51, 17, 15, NULL, NULL, GD_t7, 0, NULL, NULL },

{	54, 67, 17, 15, NULL, NULL, GD_tn, 0, NULL, NULL },
{	121, 67, 17, 15, NULL, NULL, GD_to, 0, NULL, NULL },
{	188, 67, 17, 15, NULL, NULL, GD_tk, 0, NULL, NULL },
{	255, 67, 17, 15, NULL, NULL, GD_tm, 0, NULL, NULL },
{	322, 67, 17, 15, NULL, NULL, GD_t0, 0, NULL, NULL },
{	389, 67, 17, 15, NULL, NULL, GD_tj, 0, NULL, NULL },
{	456, 67, 17, 15, NULL, NULL, GD_ti, 0, NULL, NULL },
{	523, 67, 17, 15, NULL, NULL, GD_t9, 0, NULL, NULL },

{	54, 83, 17, 15, NULL, NULL, GD_tvesszo, 0, NULL, NULL },
{	121, 83, 17, 15, NULL, NULL, GD_tminusz, 0, NULL, NULL },
{	188, 83, 17, 15, NULL, NULL, GD_tkettospont, 0, NULL, NULL },
{	255, 83, 17, 15, NULL, NULL, GD_tpont, 0, NULL, NULL },
{	322, 83, 17, 15, NULL, NULL, GD_tup, 0, NULL, NULL },
{	389, 83, 17, 15, NULL, NULL, GD_tl, 0, NULL, NULL },
{	456, 83, 17, 15, NULL, NULL, GD_tp, 0, NULL, NULL },
{	523, 83, 17, 15, NULL, NULL, GD_tdown, 0, NULL, NULL },

{	54, 99, 17, 15, NULL, NULL, GD_tper, 0, NULL, NULL },
{	121, 99, 17, 15, NULL, NULL, GD_tplusz, 0, NULL, NULL },
{	188, 99, 17, 15, NULL, NULL, GD_tegyenlo, 0, NULL, NULL },
{	255, 99, 17, 15, NULL, NULL, GD_tescape, 0, NULL, NULL },
{	322, 99, 17, 15, NULL, NULL, GD_tright, 0, NULL, NULL },
{	389, 99, 17, 15, NULL, NULL, GD_tpontosvesszo, 0, NULL, NULL },
{	456, 99, 17, 15, NULL, NULL, GD_tcsillag, 0, NULL, NULL },
{	523, 99, 17, 15, NULL, NULL, GD_tleft, 0, NULL, NULL },

{	54, 115, 17, 15, NULL, NULL, GD_trunstop, 0, NULL, NULL },
{	121, 115, 17, 15, NULL, NULL, GD_tq, 0, NULL, NULL },
{	188, 115, 17, 15, NULL, NULL, GD_tcommodore, 0, NULL, NULL },
{	255, 115, 17, 15, NULL, NULL, GD_tspace, 0, NULL, NULL },
{	322, 115, 17, 15, NULL, NULL, GD_t2, 0, NULL, NULL },
{	389, 115, 17, 15, NULL, NULL, GD_tcontrol, 0, NULL, NULL },
{	456, 115, 17, 15, NULL, NULL, GD_tclearhome, 0, NULL, NULL },
{	523, 115, 17, 15, NULL, NULL, GD_t1, 0, NULL, NULL },
{	408, 132, 132, 15, (UBYTE *)GetStr(MSG_017D), NULL, GD_b_functions, PLACETEXT_IN, NULL, (APTR)b_functionsClicked },

{	54, 3, 17, 15, NULL, NULL, GD_tkukac, 0, NULL, NULL }
};

	kmapeditLeft=GetOptionInt("KMAPLEFT",0);
	kmapeditTop=GetOptionInt("KMAPTOP",-1);
	if(kmapeditTop==-1) { kmapeditTop=0; wa_top=TAG_IGNORE; }

	if ( ! ( g = CreateContext( &kmapeditGList )))
		return( 1L );

	for( lc = 0, tc = 0; lc < kmapedit_CNT; lc++ ) {

		CopyMem((char * )&kmapeditNGad[ lc ], (char * )&ng, (long)sizeof( struct NewGadget ));

		ng.ng_VisualInfo = VisualInfo;
		ng.ng_TextAttr   = &topaz8;
		ng.ng_LeftEdge  += offx;
		ng.ng_TopEdge   += offy;

		kmapeditGadgets[ lc ] = g = CreateGadgetA((ULONG)kmapeditGTypes[ lc ], g, &ng, ( struct TagItem * )&kmapeditGTags[ tc ] );

		while( kmapeditGTags[ tc ] ) tc += 2;
		tc++;

		if ( NOT g )
			return( 2L );
	}

	if ( ! ( kmapeditWnd = OpenWindowTags( NULL,
				WA_Left,	kmapeditLeft,
				wa_top,		kmapeditTop,
				WA_InnerWidth,	kmapeditWidth,
				WA_InnerHeight,	kmapeditHeight,
				WA_IDCMP,	BUTTONIDCMP|
							TEXTIDCMP|
							IDCMP_CLOSEWINDOW|
							IDCMP_REFRESHWINDOW|
							IDCMP_RAWKEY|
							IDCMP_GADGETHELP,
				WA_Flags,	WFLG_DRAGBAR|
							WFLG_DEPTHGADGET|
							WFLG_CLOSEGADGET|
							WFLG_SMART_REFRESH|
							WFLG_ACTIVATE|
							WFLG_RMBTRAP,
				WA_Gadgets,	(ULONG)kmapeditGList,
				WA_Title,	(ULONG)kmapeditWdt,
				WA_ScreenTitle,	(ULONG)GetStr(MSG_0180),
				WA_PubScreen,	(ULONG)Scr,
				TAG_DONE )))
	return( 4L );

	GT_RefreshWindow( kmapeditWnd, NULL );
	HelpControl( kmapeditWnd, HC_GADGETHELP );

	return( 0L );
}

void ClosekmapeditWindow( void )
{
	if ( kmapeditWnd        ) {
		AddOptionInt("KMAPLEFT",kmapeditWnd->LeftEdge);
		AddOptionInt("KMAPTOP",kmapeditWnd->TopEdge);
		CloseWindow( kmapeditWnd );
		kmapeditWnd = NULL;
	}

	if ( kmapeditGList      ) {
		FreeGadgets( kmapeditGList );
		kmapeditGList = NULL;
	}
}

int HandlekmapassignIDCMP( void )
{
	struct IntuiMessage	*m;
	int			(*func)();
	BOOL			running = TRUE;

	while( (m = GT_GetIMsg( kmapassignWnd->UserPort ))) {

		CopyMem(( char * )m, ( char * )&kmapassignMsg, (long)sizeof( struct IntuiMessage ));

		GT_ReplyIMsg( m );

		switch ( kmapassignMsg.Class ) {

			case	IDCMP_REFRESHWINDOW:
				GT_BeginRefresh( kmapassignWnd );
				GT_EndRefresh( kmapassignWnd, TRUE );
				break;

			case	IDCMP_CLOSEWINDOW:
				running = kmapassignCloseWindow();
				break;

			case	IDCMP_RAWKEY:
//			printf("R: %d (%d)\n",kmapassignMsg.Code,kmapassignMsg.Qualifier);
				GT_SetGadgetAttrs(kmapassignGadgets[GD_ak_add],kmapassignWnd,NULL,GA_Disabled,FALSE,TAG_DONE);
				running = kmapassignRawKey(&kmapassignMsg);
				break;

			case	IDCMP_GADGETUP:
				func = ( void * )(( struct Gadget * )kmapassignMsg.IAddress )->UserData;
				running = func();
				break;
		}
	}
	return( running );
}

int OpenkmapassignWindow( void )
{
	struct NewGadget	ng;
	struct Gadget	*g;
	UWORD		lc, tc;
	UWORD		offx = Scr->WBorLeft, offy = Scr->WBorTop + Scr->RastPort.TxHeight + 1;
struct NewGadget kmapassignNGad[] = {
{	134, 3, 260, 15, (UBYTE *)GetStr(MSG_0182), NULL, GD_ak_yet, PLACETEXT_LEFT, NULL, NULL },
{	134, 20, 260, 15, (UBYTE *)GetStr(MSG_0183), NULL, GD_ak_new, PLACETEXT_LEFT, NULL, NULL },
{	2, 39, 113, 14, (UBYTE *)GetStr(MSG_025D), NULL, GD_ak_add, PLACETEXT_IN, NULL, (APTR)ak_addClicked },
{	141, 39, 113, 14, (UBYTE *)GetStr(MSG_0184), NULL, GD_ak_replace, PLACETEXT_IN, NULL, (APTR)ak_replaceClicked },
{	281, 39, 113, 14, (UBYTE *)GetStr(MSG_0148), NULL, GD_ak_cancel, PLACETEXT_IN, NULL, (APTR)ak_cancelClicked }
};

	if ( ! ( g = CreateContext( &kmapassignGList )))
		return( 1L );

	for( lc = 0, tc = 0; lc < kmapassign_CNT; lc++ ) {

		CopyMem((char * )&kmapassignNGad[ lc ], (char * )&ng, (long)sizeof( struct NewGadget ));

		ng.ng_VisualInfo = VisualInfo;
		ng.ng_TextAttr   = &topaz8;
		ng.ng_LeftEdge  += offx;
		ng.ng_TopEdge   += offy;

		kmapassignGadgets[ lc ] = g = CreateGadgetA((ULONG)kmapassignGTypes[ lc ], g, &ng, ( struct TagItem * )&kmapassignGTags[ tc ] );

		while( kmapassignGTags[ tc ] ) tc += 2;
		tc++;

		if ( NOT g )
			return( 2L );
	}

	if ( ! ( kmapassignWnd = OpenWindowTags( NULL,
				WA_Left,	kmapassignLeft,
				WA_Top,		kmapassignTop,
				WA_InnerWidth,	kmapassignWidth,
				WA_InnerHeight,	kmapassignHeight,
				WA_IDCMP,	TEXTIDCMP|BUTTONIDCMP|IDCMP_CLOSEWINDOW|IDCMP_RAWKEY|IDCMP_REFRESHWINDOW,
				WA_Flags,	WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_SMART_REFRESH|WFLG_ACTIVATE|WFLG_RMBTRAP,
				WA_Gadgets,	(ULONG)kmapassignGList,
				WA_Title,	(ULONG)(UBYTE *)GetStr(MSG_0181),
				WA_ScreenTitle,	(ULONG)GetStr(MSG_0180),
				WA_PubScreen,	(ULONG)Scr,
				TAG_DONE )))
	return( 4L );

	GT_RefreshWindow( kmapassignWnd, NULL );

	GT_SetGadgetAttrs(kmapassignGadgets[GD_ak_add],kmapassignWnd,NULL,GA_Disabled,TRUE,TAG_DONE);

	return( 0L );
}

void ClosekmapassignWindow( void )
{
	if ( kmapassignWnd        ) {
		CloseWindow( kmapassignWnd );
		kmapassignWnd = NULL;
	}

	if ( kmapassignGList      ) {
		FreeGadgets( kmapassignGList );
		kmapassignGList = NULL;
	}
}

int HandlekmapfunctionsIDCMP( void )
{
	struct IntuiMessage	*m;
	int			(*func)();
	BOOL			running = TRUE;

	while( (m = GT_GetIMsg( kmapfunctionsWnd->UserPort ))) {

		CopyMem(( char * )m, ( char * )&kmapfunctionsMsg, (long)sizeof( struct IntuiMessage ));

		GT_ReplyIMsg( m );

		switch ( kmapfunctionsMsg.Class ) {

			case	IDCMP_REFRESHWINDOW:
				GT_BeginRefresh( kmapfunctionsWnd );
				GT_EndRefresh( kmapfunctionsWnd, TRUE );
				break;

			case	IDCMP_CLOSEWINDOW:
				running = kmapfunctionsCloseWindow();
				break;

			case	IDCMP_RAWKEY:
				running = kmapfunctionsRawKey(&kmapfunctionsMsg);
				break;

			case	IDCMP_GADGETUP:
			case	IDCMP_GADGETDOWN:
				func = ( void * )(( struct Gadget * )kmapfunctionsMsg.IAddress )->UserData;
				running = func();
				break;
		}
	}
	return( running );
}

int OpenkmapfunctionsWindow( void )
{
	struct NewGadget	ng;
	struct Gadget	*g;
	UWORD		lc, tc;
	UWORD		offx = Scr->WBorLeft, offy = Scr->WBorTop + Scr->RastPort.TxHeight + 1;
static int run1st=0;
static struct MinList kf_flist2List;
static struct Node kf_flist2Nodes[] = {
{	&kf_flist2Nodes[1], ( struct Node * )&kf_flist2List.mlh_Head, 0, 0, NULL },
{	&kf_flist2Nodes[2], &kf_flist2Nodes[0], 0, 0, NULL },
{	&kf_flist2Nodes[3], &kf_flist2Nodes[1], 0, 0, NULL },
{	&kf_flist2Nodes[4], &kf_flist2Nodes[2], 0, 0, NULL },
{	&kf_flist2Nodes[5], &kf_flist2Nodes[3], 0, 0, NULL },
{	&kf_flist2Nodes[6], &kf_flist2Nodes[4], 0, 0, NULL },
{	&kf_flist2Nodes[7], &kf_flist2Nodes[5], 0, 0, NULL },
{	&kf_flist2Nodes[8], &kf_flist2Nodes[6], 0, 0, NULL },
{	&kf_flist2Nodes[9], &kf_flist2Nodes[7], 0, 0, NULL },
{	&kf_flist2Nodes[10], &kf_flist2Nodes[8], 0, 0, NULL },
{	&kf_flist2Nodes[11], &kf_flist2Nodes[9], 0, 0, NULL },
{	&kf_flist2Nodes[12], &kf_flist2Nodes[10], 0, 0, NULL },
{	&kf_flist2Nodes[13], &kf_flist2Nodes[11], 0, 0, NULL },
{	&kf_flist2Nodes[14], &kf_flist2Nodes[12], 0, 0, NULL },
{	&kf_flist2Nodes[15], &kf_flist2Nodes[13], 0, 0, NULL },
{	&kf_flist2Nodes[16], &kf_flist2Nodes[14], 0, 0, NULL },
{	&kf_flist2Nodes[17], &kf_flist2Nodes[15], 0, 0, NULL },
{	&kf_flist2Nodes[18], &kf_flist2Nodes[16], 0, 0, NULL },
{	&kf_flist2Nodes[19], &kf_flist2Nodes[17], 0, 0, NULL },
{	&kf_flist2Nodes[20], &kf_flist2Nodes[18], 0, 0, NULL },
{	( struct Node * )&kf_flist2List.mlh_Tail, &kf_flist2Nodes[19], 0, 0, NULL }
};
ULONG kmapfunctionsGTags[] = {
	(GTLV_Labels), (ULONG)&kf_flist2List, (GTLV_ShowSelected), NULL, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(GTTX_Border), TRUE, (TAG_DONE),
	(TAG_DONE),
	(TAG_DONE)
};
struct NewGadget kmapfunctionsNGad[] = {
{	4, 16, 139, 80, (UBYTE *)GetStr(MSG_017D), NULL, GD_kf_flist, PLACETEXT_ABOVE, NULL, (APTR)kf_flistClicked },
{	147, 16, 139, 14, (UBYTE *)GetStr(MSG_0185), NULL, GD_kf_yet, PLACETEXT_ABOVE, NULL, NULL },
{	147, 45, 139, 14, (UBYTE *)GetStr(MSG_0183), NULL, GD_kf_new, PLACETEXT_ABOVE, NULL, NULL },
{	147, 62, 139, 14, (UBYTE *)GetStr(MSG_0186), NULL, GD_kf_assign, PLACETEXT_IN, NULL, (APTR)kf_assignClicked },
{	147, 78, 139, 14, (UBYTE *)GetStr(MSG_0187), NULL, GD_kf_cancel, PLACETEXT_IN, NULL, (APTR)kf_cancelClicked }
};

	kf_flist2List.mlh_Head=(struct MinNode *)&kf_flist2Nodes[0];
	kf_flist2List.mlh_Tail=NULL;
	kf_flist2List.mlh_TailPred=(struct MinNode *)&kf_flist2Nodes[19];
	if(run1st==0) {
		run1st=1;
		kf_flist2Nodes[1-1].ln_Name=GetStr(MSG_0264);
		kf_flist2Nodes[2-1].ln_Name=GetStr(MSG_0188);
		kf_flist2Nodes[3-1].ln_Name=GetStr(MSG_0189);
		kf_flist2Nodes[4-1].ln_Name=GetStr(MSG_018A);
		kf_flist2Nodes[5-1].ln_Name=GetStr(MSG_018B);
		kf_flist2Nodes[6-1].ln_Name=GetStr(MSG_0119);
		kf_flist2Nodes[7-1].ln_Name=GetStr(MSG_018C);
		kf_flist2Nodes[8-1].ln_Name=GetStr(MSG_018D);
		kf_flist2Nodes[9-1].ln_Name=GetStr(MSG_018E);
		kf_flist2Nodes[10-1].ln_Name=GetStr(MSG_018F);
		kf_flist2Nodes[11-1].ln_Name=GetStr(MSG_0190);
		kf_flist2Nodes[12-1].ln_Name=GetStr(MSG_0191);
		kf_flist2Nodes[13-1].ln_Name=GetStr(MSG_0192);
		kf_flist2Nodes[14-1].ln_Name=GetStr(MSG_0193);
		kf_flist2Nodes[15-1].ln_Name=GetStr(MSG_0194);
		kf_flist2Nodes[16-1].ln_Name=GetStr(MSG_0195);
		kf_flist2Nodes[17-1].ln_Name=GetStr(MSG_0196);
		kf_flist2Nodes[18-1].ln_Name=GetStr(MSG_0197);
		kf_flist2Nodes[19-1].ln_Name=GetStr(MSG_0310);
		kf_flist2Nodes[20-1].ln_Name=GetStr(MSG_0311);
		kf_flist2Nodes[21-1].ln_Name=GetStr(MSG_011C);
	}

	if ( ! ( g = CreateContext( &kmapfunctionsGList )))
		return( 1L );

	for( lc = 0, tc = 0; lc < kmapfunctions_CNT; lc++ ) {

		CopyMem((char * )&kmapfunctionsNGad[ lc ], (char * )&ng, (long)sizeof( struct NewGadget ));

		ng.ng_VisualInfo = VisualInfo;
		ng.ng_TextAttr   = &topaz8;
		ng.ng_LeftEdge  += offx;
		ng.ng_TopEdge   += offy;

		kmapfunctionsGadgets[ lc ] = g = CreateGadgetA((ULONG)kmapfunctionsGTypes[ lc ], g, &ng, ( struct TagItem * )&kmapfunctionsGTags[ tc ] );

		while( kmapfunctionsGTags[ tc ] ) tc += 2;
		tc++;

		if ( NOT g )
			return( 2L );
	}

	if ( ! ( kmapfunctionsWnd = OpenWindowTags( NULL,
				WA_Left,	kmapfunctionsLeft,
				WA_Top,		kmapfunctionsTop,
				WA_InnerWidth,	kmapfunctionsWidth,
				WA_InnerHeight,	kmapfunctionsHeight,
				WA_IDCMP,	LISTVIEWIDCMP|TEXTIDCMP|BUTTONIDCMP|IDCMP_CLOSEWINDOW|IDCMP_RAWKEY|IDCMP_REFRESHWINDOW,
				WA_Flags,	WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_SMART_REFRESH|WFLG_ACTIVATE|WFLG_RMBTRAP,
				WA_Gadgets,	(ULONG)kmapfunctionsGList,
				WA_Title,	(ULONG)GetStr(MSG_0198),
				WA_ScreenTitle,	(ULONG)GetStr(MSG_0180),
				WA_PubScreen,	(ULONG)Scr,
				TAG_DONE )))
	return( 4L );

	GT_RefreshWindow( kmapfunctionsWnd, NULL );

	return( 0L );
}

void ClosekmapfunctionsWindow( void )
{
	if ( kmapfunctionsWnd        ) {
		CloseWindow( kmapfunctionsWnd );
		kmapfunctionsWnd = NULL;
	}

	if ( kmapfunctionsGList      ) {
		FreeGadgets( kmapfunctionsGList );
		kmapfunctionsGList = NULL;
	}
}

