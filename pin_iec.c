/* :ts=4                        pin_iec.c
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
#include <stdio.h>
#include <ctype.h>

#include "common.h"
#include "macros.h"
#include "iec_dispatcher.h"


/* read from 7501.asm
 */
u_byte ieccntflag=0;

/* változók a cp4-bõl
 */
extern u_byte *p4ram;
extern u_byte *p4rom;
extern int opt_iec;                     // 0-nem 1-van


/* port 00/01 (00-direction  01-data)
 */
#define SR_DATWRT       1               //          *
#define SR_CLKWRT       2               // CASWRT   *
#define SR_ATNWRT       4               //          *
#define SR_CASMOTOR     8
#define SR_CASREAD      16
#define SR_NONE         32
#define SR_CLKREAD      64              //          *
#define SR_DATREAD      128             //          *


/* mode-hoz
 */
#define M_IDLE          0       //                      ATN:1
#define M_COM           1       // listen/talk jön          0
#define M_LISTEN        2       // seclisten jön            1
#define M_TALK          3       // sectalk jön              1
#define M_DATIN         4       //                          1
#define M_DATOUT        5       //                          1

/* bitmode-hoz
 */
#define BM_NONE         0
#define BM_BEGIN        1       // atn 0 után
#define BM_BIT          2       // bit jön
#define BM_BITB         3       // bit vége

/* helyi globálisok
 */
static int prevdatabyte;                // elõzõ írás (alsó 3 bit)
static int mode=M_IDLE;                 // aktuális módok
static int bitmode=BM_NONE;             // almód (bitátvitel)
static int o_dat=0,o_clk=0;             // output vezetékek állapota
static int i_dat,i_clk,i_atn;           // aktuális input vonalak
static int readbyte=0;
static int readbitcnt=0;

/* egy byte kivitele
 */
static void outbyte(int databyte) {
    char device,comm;
    switch(mode) {
        case M_COM :
            device=databyte&0x0f;
            comm=databyte&0xf0;
            switch(comm) {
                case 0x40 :                         // TALK
//printf(":>TALK(%d)\n",device);
                    d_Talk(device);
                    mode=M_TALK;
                    break;
                case 0x20 :                         // LISTEN
//printf(":>LISTEN(%d)\n",device);
                    d_Listen(device);
                    mode=M_LISTEN;
                    break;
                case 0x30 :                         // UNLISTEN
//printf(":>UNLISTEN\n");
                    if(device==15) d_UnListen();
                    mode=M_IDLE;
                    break;
                case 0x50 :                         // UNTALK
//printf(":>UNTALK\n");
                    if(device==15) d_UnTalk();
                    mode=M_IDLE;
                    break;
            }
            break;
        case M_TALK :
//printf(":>TKSA($%02x)\n",databyte);
            mode=M_DATIN;
            d_TkSA(databyte);
        case M_LISTEN :
//printf(":>SECOND($%02x)\n",databyte);
            mode=M_DATOUT;
            d_Second(databyte);
            break;
        case M_DATIN :
//printf("INPUT\n");
            break;
        case M_DATOUT :
//printf(":>DATOUT:%3d ($%02X) ['%c']\n",databyte,databyte,(isprint(databyte)?databyte:'_'));
            d_CIOut((unsigned char)databyte);
            break;
    }
} // outbyte()


/* $0001 címre írás -> return: új érték
 */
static int handledata(u_byte data) {
    int ret;

    i_dat=((data&1)==0?1:0);        // inverz kimenet
    i_clk=((data&2)==0?1:0);
    i_atn=((data&4)==0?1:0);

    ieccntflag=0;
    if(mode!=M_COM&&i_atn==0) {
//printf("ATN\n");
        mode=M_COM;
        bitmode=BM_NONE;
        o_dat=0;
    } else if(mode!=M_IDLE&&bitmode==BM_NONE&&i_clk==1) {
//printf("1st bit jön\n");
        ieccntflag=1;
        bitmode=BM_BIT;
        o_dat=1;
    } else if(mode!=M_IDLE&&bitmode!=BM_NONE) {
        switch(bitmode) {
            case BM_BIT :
                if(i_clk==1) {
//printf("BitIn: %d. %d\n",readbitcnt,i_dat);
                    bitmode=BM_BITB;
                    readbyte>>=1;
                    if(i_dat!=0) readbyte|=0x80;
                    readbitcnt++;
                }
                break;
            case BM_BITB :
                if(i_clk==0&&readbitcnt==8) o_dat=0;
                if(i_dat==1) {
                    if(readbitcnt==8) {
//printf("8. bit OK\n");
                        outbyte(readbyte);
                        readbitcnt=0;
                        o_dat=0;
                        bitmode=BM_NONE;
                        readbyte=0;
                    } else {
//printf("next bit\n");
                        bitmode=BM_BIT;
                        o_dat=1;
                    }
                }
                break;
        }
    }

    ret=data&~0xc0;
    if(o_dat!=0) ret|=0x80;
    if(o_clk!=0) ret|=0x40;

    // datasette OFF
    ret&=~SR_NONE;
    ret|=SR_CASMOTOR;
    ret&=~SR_CASREAD;

    return(ret);
} // handledata()




/* Called by 7501.asm
 *
 * data: amit beírtak,
 * addr: 0/1 a cím
 * ret: ami oda kerül végülis (új adat)
 */
int REGARGS pin_IEC_W(REG(d0,u_byte data),REG(d1,u_byte addr)) {
    int ret=0;
    if(addr==0) {                                           // DIRECTION
        if(data!=0x0f) printf("illegal direction request\n");
        ret=data;
    } else {                                                // DATA
        ret=handledata(data);
        prevdatabyte=data&7;
    }
    return(ret);
}

int REGARGS pin_IEC_R(REG(d0,u_byte data)) {
static int cnt=0;
    if(cnt==-1) {
        data|=0x80;
        cnt=0;
    } else if(ieccntflag!=0) {
        cnt++;
//printf("cnt:%d\n",cnt);
        if(cnt>=5) {
            data&=~0x80;
            ieccntflag=0;
            cnt=-1;
        }
    } else cnt=0;
    return(data);
}


void REGARGS pin_IEC_Reset(void) {
    int i;
    for(i=0;i<15;i++) d_Reset(i);
}






















