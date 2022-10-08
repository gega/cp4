/* :ts=4                            prefsc2p.c
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
#include <string.h>
#include <ctype.h>
#include "prefs.h"
#include "c2p_module.h"

/* from ted.c
 */
extern struct c2pvec *c2pv;

/* from cp4.c
 */
extern int myatoi(char *str);

static char nm[512];
static char s[128];



static void makename(char *n) {
    char *p,*c;

    if(c2pv) {
        strcpy(nm,"C2P_");
        for(c=nm+4,p=c2pv->c2p_Name;*p!='\0';p++) if(isalnum(*p)) *c++=*p;
        *c='\0';
        strcat(nm,"_");
        strcat(nm,n);
        for(p=&nm[4];*p!='\0';p++) *p=toupper(*p);
    } else nm[0]='\0';
} // makename()


void c2pAddOptionStr(char *name,char *value) {
    if(c2pv) {
        if(strlen(name)>127) return;
        makename(name);
        AddOption(nm,value);
    }
} // c2pAddOptionStr()


char *c2pGetOptionStr(char *name,char *defval) {
    char *r=NULL;

    if(c2pv) {
        if(strlen(name)>127) return(NULL);
        makename(name);
        r=GetOption(nm,defval);
    } else return(NULL);
    return(r);
} // c2pGetOptionStr()


void c2pAddOptionInt(char *name,int value) {
    if(c2pv) {
        if(strlen(name)>127) return;
        makename(name);
        sprintf(s,"%d",value);
        AddOption(nm,s);
    }
} // c2pAddOptionInt()


int c2pGetOptionInt(char *name,int defval) {
    int ret=-1;

    if(c2pv) {
        if(strlen(name)>127) return(-1);
        makename(name);
        sprintf(s,"%d",defval);
        ret=myatoi(GetOption(nm,s));
    }
    return(ret);
} // c2pGetOptionInt()

