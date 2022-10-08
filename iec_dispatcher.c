/* :ts=4                        iec_ dispatcher.c
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

#include <proto/iec.h>

#include "common.h"
#include "soft_iec.h"
#include "soft_printer.h"
#include "macros.h"


/*
 *  változók a cp4-bõl
 */
extern int opt_drive08;                 // 0-semmi 1-iec 2-soft
extern int opt_drive09;
extern int opt_drive10;
extern int opt_drive11;

/*
 *  változók a soft_iec-bõl
 */
extern UBYTE d64_ST;                    // soft_iec.c


/*
 * module globals
 */
static int STs[12];         // 8-11 statuses
static int dr[12];          // drive specs
static int actdevaddrT;     // actdrive 8-11                        TALK
static int olddevaddrT=0;
static int actdevaddrL;     // actdrive 8-11                        LISTEN
static int olddevaddrL=0;


static void d_pre(void) {
    dr[8]=opt_drive08;
    dr[9]=opt_drive09;
    dr[10]=opt_drive10;
    dr[11]=opt_drive11;
}

int d_GetST(char devaddr) {
    if(devaddr>=8&&devaddr<=11) return(STs[(int)devaddr]);
    else return(-1);
}
void d_Reset(char devaddr) {
    int i;
//printf("d_RESET(%d)\n",devaddr);
    if(devaddr>=8&&devaddr<=11) {
        d_pre();
        i=devaddr;
        switch(dr[i]) {
            case 1 :                        // Real1541/1551
                Listen(i);
                Second(15+CMD_OPEN);
                UnListen();
                Listen(i);
                Second(15+CMD_DATA);
                CIOut(73);                  // 'I'
                UnListen();
                Listen(i);
                Second(15+CMD_CLOSE);
                UnListen();
                break;
            case 2 :                        // .d64 file
                SOFT_Reset(i);
                break;
        }
        STs[i]=0;
    } else if(devaddr==4) {                 // PRINTER
        SPRT_Reset(4);
    }
}
int d_Init(char devaddr) {
    int ret=0;
//printf("d_INIT(%d)\n",devaddr);
    if(devaddr>=8&&devaddr<=11) {
        d_pre();
        switch(dr[(int)devaddr]) {
            case 1 :                            // IEC
                Listen(devaddr);
                Second(15+CMD_OPEN);
                UnListen();
                Listen(devaddr);
                Second(15+CMD_DATA);
                CIOut(73);              // 'I'
                UnListen();
                Listen(devaddr);
                Second(15+CMD_CLOSE);
                UnListen();
                STs[(int)devaddr]=IECBase->iec_ST;
                ret=0;
                break;
            case 2 :                            // SOFT
                ret=SOFT_Init(devaddr);
                STs[(int)devaddr]=d64_ST;
                break;
        }
    } else if(devaddr==4) {                     // PRINTER
        ret=SPRT_Init(4);
    }
    return(ret);
}
int d_State(char devaddr) {
    int ret=-1;
    if(devaddr>=8&&devaddr<=11) {
        d_pre();
        switch(dr[(int)devaddr]) {
            case 1 :                            // IEC
                if(IECBase!=NULL) ret=0;
                else ret=-1;
                break;
            case 2 :                            // SOFT
                ret=SOFT_State(devaddr);
                break;
        }
    } else if(devaddr==4) {                     // PRINTER
        ret=SPRT_State(4);
    }
    return(ret);
}
void d_Listen(char devaddr) {
//printf("d_LISTEN(%d)\n",devaddr);
    olddevaddrL=actdevaddrL;
    actdevaddrL=(int)devaddr;
    if(devaddr>=8&&devaddr<=11) {
        d_pre();
        switch(dr[(int)actdevaddrL]) {
            case 1 :                            // IEC
                Listen(actdevaddrL);
                STs[actdevaddrL]=IECBase->iec_ST;
                break;
            case 2 :                            // SOFT
                SOFT_Listen(actdevaddrL);
                STs[actdevaddrL]=d64_ST;
                break;
        }
    } else if(devaddr==4) {                     // PRINTER
        SPRT_Listen(4);
    }
}
void d_Talk(char devaddr) {
//printf("d_TALK(%d)\n",devaddr);
    olddevaddrT=actdevaddrT;
    actdevaddrT=(int)devaddr;
    if(devaddr>=8&&devaddr<=11) {
        d_pre();
        switch(dr[(int)actdevaddrT]) {
            case 1 :                            // IEC
                Talk(actdevaddrT);
                STs[actdevaddrT]=IECBase->iec_ST;
                break;
            case 2 :                            // SOFT
                SOFT_Talk(actdevaddrT);
                STs[actdevaddrT]=d64_ST;
                break;
        }
    } else if(devaddr==4) {                     // PRINTER
        // printer talk ???
    }
}
int d_ACPtr(void) {
    int ret=0;
//printf("d_ACPTR(%d)\n",actdevaddrT);
    if(actdevaddrT>=8&&actdevaddrT<=11) {
        d_pre();
        switch(dr[(int)actdevaddrT]) {
            case 1 :                            // IEC
                ret=ACPtr();
                STs[actdevaddrT]=IECBase->iec_ST;
                break;
            case 2 :                            // SOFT
                ret=SOFT_ACPtr();
                STs[actdevaddrT]=d64_ST;
                break;
        }
    } else if(actdevaddrT==4) {                     // PRINTER
        // printer input ???
        ret=0;
    }
    return(ret);
}
void d_CIOut(unsigned char chr) {
//printf("d_CIOUT(%d)\n",actdevaddrL);
    if(actdevaddrL>=8&&actdevaddrL<=11) {
//printf("d_DISKOUT: %02x '%c'\n",chr,((chr>31&&chr<128)?chr:'_'));
        d_pre();
        switch(dr[(int)actdevaddrL]) {
            case 1 :                            // IEC
//printf(" d_REAL\n");
                CIOut(chr);
                STs[actdevaddrL]=IECBase->iec_ST;
                break;
            case 2 :                            // SOFT
//printf(" d_D64\n");
                SOFT_CIOut(chr);
                STs[actdevaddrL]=d64_ST;
                break;
        }
    } else if(actdevaddrL==4) {                     // PRINTER
//printf("d_PRINTOUT: %02x '%c'\n",chr,((chr>31&&chr<128)?chr:'_'));
        SPRT_CIOut(chr);
    }
}
void d_Second(char secaddr) {
//printf("d_SECOND(%d)\n",actdevaddrL);
    if(actdevaddrL>=8&&actdevaddrL<=11) {
        d_pre();
        switch(dr[(int)actdevaddrL]) {
            case 1 :                            // IEC
                Second(secaddr);
                STs[actdevaddrL]=IECBase->iec_ST;
                break;
            case 2 :                            // SOFT
                SOFT_Second(secaddr);
                STs[actdevaddrL]=d64_ST;
                break;
        }
    } else if(actdevaddrL==4) {                     // PRINTER
        SPRT_Second(secaddr);
    }
}
void d_TkSA(char secaddr) {
//printf("d_TKSA(%d)\n",actdevaddrT);
    if(actdevaddrT>=8&&actdevaddrT<=11) {
        d_pre();
        switch(dr[(int)actdevaddrT]) {
            case 1 :                            // IEC
                TkSA(secaddr);
                STs[actdevaddrT]=IECBase->iec_ST;
                break;
            case 2 :                            // SOFT
                SOFT_TkSA(secaddr);
                STs[actdevaddrT]=d64_ST;
                break;
        }
    } else if(actdevaddrT==4) {                     // PRINTER
        // printer sectalk 'secaddr' ???
    }
}
void d_UnListen(void) {
//printf("d_UNLISTEN\n");
    if(actdevaddrL>=8&&actdevaddrL<=11) {
        d_pre();
        switch(dr[(int)actdevaddrL]) {
            case 1 :                            // IEC
                UnListen();
                STs[actdevaddrL]=IECBase->iec_ST;
                break;
            case 2 :                            // SOFT
                SOFT_UnListen();
                STs[actdevaddrL]=d64_ST;
                break;
        }
    } else if(actdevaddrL==4) {                     // PRINTER
        // printer unlisten
        SPRT_UnListen(4);
    }
    actdevaddrL=0;
    if(olddevaddrL!=0) {
        actdevaddrL=olddevaddrL;
        olddevaddrL=0;
    }
}
void d_UnTalk(void) {
//printf("d_UNTALK\n");
    if(actdevaddrT>=8&&actdevaddrT<=11) {
        d_pre();
        switch(dr[(int)actdevaddrT]) {
            case 1 :                            // IEC
                UnTalk();
                STs[actdevaddrT]=IECBase->iec_ST;
                break;
            case 2 :                            // SOFT
                SOFT_UnTalk();
                STs[actdevaddrT]=d64_ST;
                break;
        }
    } else if(actdevaddrT==4) {                     // PRINTER
        // printer untalk ???
    }
    actdevaddrT=0;
    if(olddevaddrT!=0) {
        actdevaddrT=olddevaddrT;
        olddevaddrT=0;
    }
}
