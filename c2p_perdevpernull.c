/* :ts=4                            c2p_perdevpernull.c
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
#include <proto/exec.h>
#include <proto/intuition.h>

#include "cp4_ver.h"

#define C2P_VERSION     "1"
#define C2P_REVISION    "2"
#define C2P_AUTHOR      "NASA"
#define C2P_NAME        "perdevpernull"
#define C2P_NOSPEED
#define C2P_DATE        DATE
#define C2P_LOCALIZE

#include "c2p_module.c"

struct IntuitionBase *IntuitionBase=NULL;
static struct Window *win=NULL;
static int winsleeped=0;
static struct Requester InvisibleRequester;

#define MSG_INFO 0
#define MSG_NOINTUI 1
#define MSG_NOWIN 2

static char *defstr[]={
    ">/dev/null",
    "Can't open intuition.library V37",
    "Can't open Window",
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
    int borhoriz,borvert;

    c2p_OpenCatalog(defstr);

    if(!(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",37))) return(STR(MSG_NOINTUI));

    /* Get default PubScreen
     */
    vec.c2p_Scr=LockPubScreen(NULL);

    /* Open window
     */
    if((win=(struct Window *)OpenWindowTags(NULL,
//          WA_Left, 0,
//          WA_Top, 11,
                WA_Width, 120,
                WA_Height, 50,
                WA_Title, (ULONG)"  0% /dev/null",
                WA_Flags,   WFLG_DRAGBAR|
                            WFLG_DEPTHGADGET|
                            WFLG_SIZEGADGET|
                            WFLG_SIZEBBOTTOM|
                            WFLG_ACTIVATE|
                            WFLG_SIMPLE_REFRESH|
                            WFLG_RMBTRAP,
                WA_RptQueue, 25,
                TAG_DONE,0L ))==NULL)
        return(STR(MSG_NOWIN));
    vec.c2p_Win=win;
    borhoriz=win->BorderRight+win->BorderLeft-1;
    borvert=win->BorderTop+win->BorderBottom-1;
    WindowLimits(win,borhoriz+50,borvert+20,~0,~0);
    winsleeped=0;
    return(NULL);
} // init


void SAVEDS mfree(void) {
    mawake();
    if(win) { CloseWindow(win); win=NULL; }
    vec.c2p_Win=NULL;
    if(vec.c2p_Scr) { UnlockPubScreen(NULL,vec.c2p_Scr); vec.c2p_Scr=NULL; }
    if(IntuitionBase) { CloseLibrary((struct Library *)IntuitionBase); IntuitionBase=NULL; }
    c2p_CloseCatalog();
    return;
} // mfree


static void getspeed(void) {
static char title[]="...% /dev/null";
static int notitle=0;
    int i,szam1;

    i=vec.c2p_Speed;
    if(i>0) {
        i+=5;
        title[0]=' ';
        if(i>1000) {
            title[0]=(i/1000)+'0';
            i%=1000;
        }
        szam1=i/100;
        title[1]=' ';
        if(szam1!=0) title[1]=szam1+'0';
        else if(title[0]!=' ') title[1]=szam1+'0';
        i%=100;
        title[2]=(i/10)+'0';
        SetWindowTitles(win,title,(UBYTE *)~0);
        notitle=0;
    } else {
        title[0]=' ';
        title[1]=' ';
        title[2]='?';
        if(notitle==0) SetWindowTitles(win,title,(UBYTE *)~0);
        notitle=1;
    }
} // getspeed()

int SAVEDS mdo(unsigned char *chunky,unsigned char *delta,int numscreen) {
    getspeed();
    return(RET_OK);
} // mdo


int SAVEDS mdofull(unsigned char *chunky,int numscreen) {
    getspeed();
    return(RET_OK);
} // mdofull


int SAVEDS mdont(void) {
    getspeed();
    return(RET_OK);
} // mdont


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
