/* :ts=4                        soft_printer.c
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

#include "soft_printer.h"
#include "common.h"
#include "macros.h"
#include "prefs.h"

/* for mode
 */
#define M_NONE          0
#define M_LISTEN        1
#define M_OUT           2


#define RVSON(f) { fputc(27,f); fputc(91,f); fputc(55,f); fputc(109,f); }
#define RVSOFF(f) { fputc(27,f); fputc(91,f); fputc(48,f); fputc(109,f); }



static FILE *po=NULL;
static int mode=M_NONE;
static int inited=-1;


static void openprtfile(void) {
    char *o;
    if(po!=NULL) return;
    o=GetOption("PRINTER_FILE","PRT:");
    if(strcmp(o,"STDOUT")==0) {
        po=stdout;
        inited=0;
        return;
    }
    if(NULL==(po=fopen(o,"wb"))) po=stdout;
}

void SPRT_Reset(int devaddr) {
    if(inited!=0) SPRT_Init(devaddr);
    mode=M_NONE;
} // SPRT_Reset()

int SPRT_Init(int devaddr) {
    inited=0;
    return(0);
} // SPRT_Init()

int SPRT_State(int devaddr) {
    return(inited);
} // SPRT_State()

void SPRT_Listen(int devaddr) {
    if(inited==0&&devaddr==4) mode=M_LISTEN;
} // SPRT_Listen()

void SPRT_CIOut(unsigned char chr) {
static int inv=0;
static int quota=0;
    char *o;
    int oc;

    oc=(int)chr;
    if(mode==M_OUT) {
        openprtfile();
        o=GetOption("PRINTER_CONVERSION","YES");
        if(o[0]!='N') {
            switch(oc) {
                case 10 :
                case 13 :               // sorvég
                    oc='\n';
                    quota=0;
                    if(inv!=0) {
                        inv=0;
                        fputc('\n',po);
                        oc=-1;
                        RVSOFF(po);
                    }
                    break;
                case 18 :               // RVSON
                    oc=-1;
                    if(quota==0) {
                        inv=1;
                        RVSON(po);
                    } else {
                        if(inv==0) RVSON(po);
                        fputc('R',po);
                        if(inv==0) RVSOFF(po);
                    }
                    break;
                case 146 :              // RVSOFF
                    oc=-1;
                    if(quota==0) {
                        inv=0;
                        RVSOFF(po);
                    } else {
                        if(inv==0) RVSON(po);
                        fputc('_',po);
                        if(inv==0) RVSOFF(po);
                    }
                    break;
                case 34 :               // "
                    quota^=1;
                    break;
                default :
                    if(oc>126&&oc<160) oc='_';
                    if(oc<32) oc='_';
                    break;
            }
        }
        if(oc>=0) fputc(oc,po);
    }
} // SPRT_CIOut()

void SPRT_Second(char secaddr) {
    if(mode==M_LISTEN) mode=M_OUT;
} // SPRT_Second()

void SPRT_UnListen(int devaddr) {
    mode=M_NONE;
//  if(po!=NULL&&po!=stdout) fclose(po);
} // SPRT_UnListen()
