/* :ts=4                            p4req.c
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
#include "macros.h"
#include "p4req.h"
#include "cp4_loc.h"

extern struct IntuitionBase *IntuitionBase;

static struct Requester InvisibleRequester;

static char *titles[]={
    NULL,   NULL,   NULL,   NULL,   NULL,
    NULL
};

static struct EasyStruct s1={
    sizeof(struct EasyStruct),
    0,
    "title",
    "body",
    NULL,
};

static struct EasyStruct s2={
    sizeof(struct EasyStruct),
    0,
    "title",
    "body",
    NULL,
};

static int initreq(void) {
static int inited=0;
    if(inited==0&&IntuitionBase!=0L) {
        inited=1;
        titles[0]=GetStr(MSG_01EE);
        titles[1]=GetStr(MSG_01EF);
        titles[2]=GetStr(MSG_01F0);
        titles[3]=GetStr(MSG_01F1);
        titles[4]=GetStr(MSG_01F2);
        s1.es_GadgetFormat=GetStr(MSG_0187);
        s2.es_GadgetFormat=GetStr(MSG_01F3);
    }
    return(1-inited);
}

static void sleep(struct Window *w) {
static struct TagItem BusyPointerTagList[]={
    {WA_BusyPointer,TRUE},
    {TAG_END,0}
};
    if(w) {
        InitRequester(&InvisibleRequester);
        Request(&InvisibleRequester,w);
        SetWindowPointerA(w,BusyPointerTagList);
    }
}

static void awake(struct Window *w) {
    if(w) {
        EndRequest(&InvisibleRequester,w);
        SetWindowPointerA(w,NULL);
    }
}

int p4req1( REG(a0,void *w),
            REG(d0,int title),
            REG(a1,char *text) ) {
    struct Window *ww=w;
    int ret=-1;

    if(0==(initreq())) {
        sleep(ww);
        s1.es_Title=titles[title];
        s1.es_TextFormat=text;
        ret=EasyRequestArgs(ww,&s1,NULL,NULL);
        awake(ww);
    }
    return(ret);
} // p4req1

int p4req2( REG(a0,void *w),
            REG(d0,int title),
            REG(a1,char *text) ) {
    struct Window *ww=w;
    int ret=-1;

    if(0==(initreq())) {
        s2.es_Title=titles[title];
        s2.es_TextFormat=text;
        sleep(ww);
        ret=EasyRequestArgs(ww,&s2,NULL,NULL);
        awake(ww);
    }
    return(ret);
} // p4req2

