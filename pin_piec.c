/* :ts=4                        pin_piec.c
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

#include "common.h"
#include "macros.h"
#include "iec_dispatcher.h"

/*
 *  változók a cp4-bõl
 */
extern u_byte *p4ram;
extern u_byte *p4rom;
extern int opt_iec;                     // 0-nem 1-van

// direction (d_*)
#define P_IN    0                       // 1551 -> p4
#define P_OUT   1                       // p4 -> 1551

// mode
#define P_NONE      0
#define P_GETMODE   1
#define P_TALK      2
#define P_LISTEN    3
#define P_SECTALK   4
#define P_SECLISTEN 5
#define P_DATAOUT   6

// bit-mûveletek
#define B_SET       1
#define B_CLR       0
#define B_INV       -1

// rmode
#define RM_NORMAL       0
#define RM_NOFLIP       1

// registers
#define R_DATA          0
#define R_STATUS        1
#define R_HANDSHAKE     2
#define R_DATADIR       3
#define R_STATUSDIR     4
#define R_HANDSHAKEDIR  5

/* externs (7501.asm)
 */
u_byte rmode=RM_NORMAL;                     // adatolvasás után noflip EXTERN

/* globals
 */
static int d_data,d_status,d_dav,d_ack;     // directions
static int mode=P_NONE;                     // mode on data
static int drives[10];                      // inicializálva? (used: 8,9)
static int nextdata=0;                      // olvasandó adat



static u_byte memread(u_word addr) {
    return(p4ram[addr]);
} // memread

static void memwrite(u_word addr,u_byte data) {
    p4ram[addr]=data;
} // memwrite


/* beír egy regiszterbe
 */
static void writereg(int drive,int port,int data) {
    u_word addr;
    addr=(drive==8?0xfee0:0xfec0);
    memwrite(addr+port,data);
    memwrite(addr+port+0x08,data);
    memwrite(addr+port+0x10,data);
    memwrite(addr+port+0x18,data);
} // writereg()

/* handshake biteket állítja (ACK)
 * 0  - nulláz
 * >0 - beállít
 * <0 - invertál
 */
static u_byte set_ack(int state,int drive,u_byte data) {
    int and=0xff,eor=0,or=0;

    if(drives[drive]==0) return(0);
    if(state==0) and=0x7f;
    else if(state>0) or=0x80;
    else eor=0x80;
    data&=and;
    data|=or;
    data^=eor;
    writereg(drive,R_HANDSHAKE,data);
    return(data);
} // set_ack()

/* inicializálja, ha kell
 */
static void init1551(int drive) {
    if(opt_iec!=0) {
        d_Init(drive);
        rmode=RM_NORMAL;
        if(0==d_State(drive)) {
            memwrite(0xfee1,0);             // status=0
            memwrite(0xfee9,0);
            memwrite(0xfef1,0);
            memwrite(0xfef9,0);
            memwrite(0xfec1,0);
            memwrite(0xfec9,0);
            memwrite(0xfed1,0);
            memwrite(0xfed9,0);
            drives[drive]=1;                // inited
        }
    }
} // init1551()

/*
 *  Data-vezeték
 *
 *  81 40   talk
 *  81 20   listen
 *  81 3f   unlisten
 *  81 5f   untalk
 *  82 6_   secondary (data)
 *  82 e_   secondary (close)
 *  82 f_   secondary (open)
 *  83 __   output  (p4->1551)
 *  84 __   input   (1551->p4)
 *  55      init
 */
static int port_data(u_byte data,char drive) {
    int ok=0;
    switch(mode) {
        case P_NONE :
            switch(data) {
                case 0x81 :
                    if(d_data==P_OUT) {
                        mode=P_GETMODE;
                        ok=1;
                        rmode=RM_NOFLIP;
                    }
                    break;
                case 0x55 :
                    if(drives[(int)drive]==0) data=0;
                    break;
                case 0x83 :
                    mode=P_DATAOUT;
                    ok=1;
                    break;
                case 0x84 :
                    if(drives[(int)drive]!=0) {
                        nextdata=d_ACPtr();
                        ok=1;
                    }
                    break;
            }
            break;
        case P_GETMODE :
            if(d_data==P_OUT) {
                switch(data) {
                    case 0x40 :
                        mode=P_TALK;
                        if(drives[(int)drive]!=0) {
                            d_Talk(drive);
                            ok=1;
                        }
                        break;
                    case 0x20 :
                        mode=P_LISTEN;
                        if(drives[(int)drive]!=0) {
                            d_Listen(drive);
                            ok=1;
                        }
                        break;
                    case 0x3f :
                        mode=P_NONE;
                        if(drives[(int)drive]!=0) {
                            d_UnListen();
                            ok=1;
                        }
                        break;
                    case 0x5f :
                        mode=P_NONE;
                        if(drives[(int)drive]!=0) {
                            d_UnTalk();
                            ok=1;
                        }
                        break;
                }
            }
            break;
        case P_TALK :
            if(data==0x82&&d_data==P_OUT) {
                mode=P_SECTALK;
                ok=1;
            }
            break;
        case P_LISTEN :
            if(data==0x82&&d_data==P_OUT) {
                mode=P_SECLISTEN;
                ok=1;
            }
            break;
        case P_SECTALK :
            if(d_data==P_OUT) {                                 // 1551->p4
                if(drives[(int)drive]!=0) {
                    d_TkSA(data);
                    ok=1;
                }
                mode=P_NONE;
            }
            break;
        case P_SECLISTEN :
            if(d_data==P_OUT) {                                 // p4->1551
                if(drives[(int)drive]!=0) {
                    d_Second(data);
                    ok=1;
                }
                mode=P_NONE;
            }
            break;
        case P_DATAOUT :
            if(d_data==P_OUT&&drives[(int)drive]!=0) {
                d_CIOut(data);
                ok=1;
            }
            mode=P_NONE;
            break;
    }
    if(ok!=0) set_ack(B_INV,drive,memread((drive==8?0xfef2:0xfec2)));
    return((int)data);
} // port_data()




/* extern called by 7501.asm
 * ret: 0.l
 */
int REGARGS pin_PIEC_Reset(void) {
    int i;
    rmode=RM_NORMAL;
    mode=P_NONE;
    for(i=8;i<12;i++) d_Reset(i);
    return(0);
} // pin_PIEC_Reset()



/* extern called by 7501.asm
 * ret: az új érték a regiszterben
 */
int REGARGS pin_PIEC_W(REG(d0,u_byte data),REG(d1,u_word addr)) {
    int drive,port,ret=0;

    drive=((addr&0x20)==0?9:8);
    port=addr&7;
    if(drives[drive]==0) init1551(drive);
    if(drives[drive]==0) return(0);
    if(port==0) rmode=RM_NORMAL;
    switch(port) {
        case 0 :                                            // A - Data
            ret=port_data(data,(char)drive);
            break;
        case 1 :                                            // B - Status
            if(d_status==P_IN) ret=(d_GetST(drive)==64?3:0);
            break;
        case 2 :                                            // C - HandShake
            if(d_dav==P_OUT&&rmode!=RM_NOFLIP) ret=set_ack(B_INV,drive,data);
            rmode=RM_NORMAL;
            break;
        case 3 :                                            //dA - Data Direction
            d_data=(0==data?P_IN:P_OUT);
            writereg(drive,R_DATA,nextdata);
            break;
        case 4 :                                            //dB - Status Direction
            d_status=(0==(data&3)?P_IN:P_OUT);
            break;
        case 5 :                                            //dC - HandShake Direction
            d_dav=(0==(data&0x40)?P_IN:P_OUT);
            d_ack=(0==(data&0x80)?P_IN:P_OUT);
            break;
    }
    if(d_status==P_IN) {
        int padr=0xfec0;
        int st;
        st=(d_GetST(drive)==64?3:0);
        padr+=(drive==8?0x20:0);
        memwrite(padr+0x01,st);
        memwrite(padr+0x09,st);
        memwrite(padr+0x11,st);
        memwrite(padr+0x19,st);
    }
    return(ret);
} // pin_PIEC_W()


