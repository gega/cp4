/* :ts=4                        iectrace.c
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
#include <ctype.h>
#include "macros.h"
#include "stat.h"

#ifdef STAT_IEC

#define M_CIOUT     1
#define M_TALK      2
#define M_LISTEN    3
#define M_UNTALK    4
#define M_UNLISTEN  5
#define M_ACPTR     6
#define M_SEC       7
#define M_TKSA      8

#define M_MAX       8

static int st[17];

void printline(void) {
    int i;
    printf("  ");
    for(i=0;i<16;i++) {
        if(st[i]>=0) printf("%02x ",st[i]);
        else printf("   ");
    }
    printf("» ");
    for(i=0;i<16&&st[i]>=0;i++) printf("%c",(isprint(st[i])?st[i]:'.'));
    printf("\n");
    for(i=0;i<16;i++) st[i]=-1;
}

void REGARGS iecmessage(REG(d0,int m),REG(d1,long p)) {
static char *msg[]={
    "***illegal",
    "CIOut",
    "Talk addr:",
    "Listen addr:",
    "UnTalk",
    "UnListen",
    "ACPtr",
    "Secondary addr:",
    "TkSa addr:",
    NULL
};
static int before=0;
static int num=0;
    unsigned char c,z;

    if(m>M_MAX) {
        printf("IEC: %s\n",msg[0]);
        return;
    }
    if(m==-1) {     // flush
        if(st[0]!=-1) printline();
        printf("\nIEC session completed.\n");
        return;
    }
    if(m==-2) {     // init
        int i;
        for(i=0;i<16;i++) st[i]=-1;
        return;
    }
    if(before==m&&(m==M_ACPTR||m==M_CIOUT)) {
        if(++num>16) { printline(); num=1; }
        st[num-1]=(unsigned char)(p&0xff);
    } else {
        if(num!=0) printline();
        num=0;
        c=(unsigned char)(p&0xff);
        switch(m) {
            case M_ACPTR :
            case M_CIOUT :
                printf("IEC: %s ->\n",msg[m]);
                st[0]=c;
                num=1;
                break;
            case M_TALK :
                printf("IEC: %s $%02x (%d)\n",msg[m],c,c);
                break;
            case M_LISTEN :
                printf("IEC: %s $%02x (%d)\n",msg[m],c,c);
                break;
            case M_UNTALK :
                printf("IEC: %s\n\n",msg[m]);
                break;
            case M_UNLISTEN :
                printf("IEC: %s\n\n",msg[m]);
                break;
            case M_SEC :
            case M_TKSA :
                printf("IEC: %s $%02x ",msg[m],c);
                z=c&0xf0;
                if(z==0x60) printf(" DATA > ");
                else if(z==0xe0) printf(" CLOSE > ");
                else if(z==0xf0) printf(" OPEN > ");
                else printf(" ??? ");
                z=c&0x0f;
                if(z==0) printf("load\n");
                else if(z==1) printf("save\n");
                else if(z==15) printf("comm\n");
                else printf("%d\n",z);
                break;
        }
    }
    before=m;
}
#else
void iecmessage(void) { return; }
#endif
