/*:ts=4                         mmu.c
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
#include <stdlib.h>

#include <exec/types.h>
#include <proto/exec.h>

#include "macros.h"

struct mmu_config {
    ULONG tc;
    ULONG tt0,tt1;
    ULONG crp_hi,crp_lo;
    ULONG srp_hi,srp_lo;
    UWORD mmusr;
};


// from mmuhandle.asm
extern struct mmu_config *SupMMU_on(void);
extern struct mmu_config *SupMMU_off(void);
extern ULONG BusTrapHandler;
extern APTR OldTrapData;
extern APTR OldTrapCode;


ULONG TrapFetchTable=0L;
UWORD TouchAddress;
UBYTE TouchData;

       unsigned long *mmu_root=NULL;        // osztható 16-tal!
static unsigned long *mmu_plus1=NULL;       // osztható 16-tal! (alsó 4 bit=0)
static unsigned long *mmu_plus2=NULL;       // osztható 16-tal!
static char *mmu_memroot=NULL;
static char *mmu_memplus1=NULL;
static char *mmu_memplus2=NULL;
static int mmu_inited=0;
static int mmu_on=0;
//static struct mmu_config *Old_mmu_Config;


/*********************************************************************
    MMU REGS: (set supervisor only)
    --------

    CRP:
        word0   $7fff       $7fff0002 xxxxxxx0
        word1   $0002
        word2   addr $xxxx  (a root table címe)
        word3   addr $xxx0

    TC: E     RF  PS  IS    $80a0bb00
        10000000 10100000
        10111011 00000000
        TIA TIB  TIC TID
        11  11   0   0

    TTx:                    $00000000
        0L (disabled)
*********************************************************************/


/* kikapcsolja az mmu-t
 */
int offmmu(void) {
//  struct mmu_config *o=Old_mmu_Config;
    struct Task *mytask;

//  if(mmu_inited==0) return(-1);
    if(mmu_on==0) return(0);

    mytask=FindTask(NULL);

    Disable();
    SupMMU_off();
//  mytask->tc_TrapData=OldTrapData;
    mytask->tc_TrapCode=OldTrapCode;
    Enable();

//  fprintf(stderr,"OFF\nTC =%08lx\nTT0=%08lx\nTT1=%08lx\nCRP=%08lx %08lx\nSRP=%08lx %08lx\nMMUSR=  %04x\n",o->tc,o->tt0,o->tt1,o->crp_hi,o->crp_lo,o->srp_hi,o->srp_lo,o->mmusr);

    mmu_on=0;
    return(0);
}

/* bekapcsolja az mmu-t
 */
int onmmu(void) {
//  struct mmu_config *o;
    struct Task *mytask;

//  if(mmu_inited==0) return(-1);
    if(mmu_on!=0) return(0);

    mytask=FindTask(NULL);
    OldTrapData=mytask->tc_TrapData;
    OldTrapCode=mytask->tc_TrapCode;

    Disable();
//  mytask->tc_TrapData=0;
    mytask->tc_TrapCode=&BusTrapHandler;
    SupMMU_on();
    Enable();

//  o=Old_mmu_Config;
//  fprintf(stderr,"ON\nTC =%08lx\nTT0=%08lx\nTT1=%08lx\nCRP=%08lx %08lx\nSRP=%08lx %08lx\nMMUSR=  %04x\n",o->tc,o->tt0,o->tt1,o->crp_hi,o->crp_lo,o->srp_hi,o->srp_lo,o->mmusr);

    mmu_on=1;
    return(0);
}


void freemmu(void) {
    if(mmu_inited==0) return;
    offmmu();
    if(mmu_memplus2!=NULL) free(mmu_memplus2);
    mmu_memplus2=NULL;
    mmu_plus2=NULL;
    if(mmu_memplus1!=NULL) free(mmu_memplus1);
    mmu_memplus1=NULL;
    mmu_plus1=NULL;
    if(mmu_root!=NULL) free(mmu_memroot);
    mmu_memroot=NULL;
    mmu_root=NULL;
    mmu_inited=0;
}


int initmmu(int ram, int rom) {
    int i,n,ramt,romt,ramb,romb,k,j;

    if(mmu_inited!=0) return(0);

    if((ram%0x10000)!=0) return(-1);
    if((rom%0x10000)!=0) return(-1);

    // test if ram/rom in the same page (need only 16k table)

    ramt=ram&~0x1fffff;
    romt=rom&~0x1fffff;

    ramb=ram&0x1f0000;
    romb=rom&0x1f0000;

    if(NULL==(mmu_memroot=(char *)malloc((2048*4)+16))) return(-1);
    if(NULL==(mmu_memplus1=(char *)malloc((2048*4)+16))) return(-1);

    // align $xxxxx0
    for(i=0;i<16;i++) {
        n=(int)&mmu_memroot[i];
        if((n%16)==0) mmu_root=(unsigned long *)&mmu_memroot[i];
        n=(int)&mmu_memplus1[i];
        if((n%16)==0) mmu_plus1=(unsigned long *)&mmu_memplus1[i];
    }

    if(romt==ramt) {    // on the same page  (need 16k table)

        for(i=0;i<2048;i++) {
            n=i<<21;
            if(n==ramt) {
                mmu_root[i]=((((unsigned long)(mmu_plus1))&~0xf)|0xa);
            } else {
                mmu_root[i]=n|25;
            }
        }

        // ram-táblázat
        for(i=0;i<2048;i++) {
            n=(i<<10)&0x1f0000;
            if(n==ramb) {                   // Plus/4 RAM **************
                n=(i<<10)|ramt;
                k=n&0xfc00;
                k^=0xfc00;
                if(k==0) {                  // i/o area (invalid)
                    mmu_plus1[i]=0;
//N                 mmu_plus1[i]=n|25;  // no-invalid yet
                } else {                    // normal ram (transparent)
                    mmu_plus1[i]=n|25;
                }
            } else if(n==romb) {            // Plus/4 ROM **************
                n=(i<<10)|romt;
                k=n&0xfc00;
                k^=0xfc00;
                j=n&0x8000;
                if(k==0) {                  // i/o area (invalid)
                    mmu_plus1[i]=0;
//N                 mmu_plus1[i]=n|25;  // no-invalid yet
                } else if(j==0) {           // map to ram (map)
                    mmu_plus1[i]=((n-0x10000)|25);
                } else {                    // normal rom (transparent)
                    mmu_plus1[i]=n|25;
                }
            } else {                        // Amiga MEM ***************
                n=((i<<10)|ramt)|25;        // (transparent)
                mmu_plus1[i]=n;
            }
        }

    } else {            // on different page (need 24k table)

        if(NULL==(mmu_memplus2=(char *)malloc((2048*4)+16))) return(-1);

        // align $xxxxx0
        for(i=0;i<16;i++) {
            n=(int)&mmu_memplus2[i];
            if((n%16)==0) {
                mmu_plus2=(unsigned long *)&mmu_memplus2[i];
                break;
            }
        }

        /* ramp -> mmu_plus1
         * romp -> mmu_plus2
         */
        for(i=0;i<2048;i++) {
            n=i<<21;
            if(n==ramt) {
                mmu_root[i]=((((unsigned long)(mmu_plus1))&~0xf)|0xa);
            } else if(n==romt) {
                mmu_root[i]=((((unsigned long)(mmu_plus2))&~0xf)|0xa);
            } else {
                mmu_root[i]=n|25;
            }
        }

        // ram-táblázat
        for(i=0;i<2048;i++) {
            n=(i<<10)&0x1f0000;
            if(n==ramb) {                   // Plus/4 RAM **************
                n=(i<<10)|ramt;
                k=n&0xfc00;
                k^=0xfc00;
                if(k==0) {                  // i/o area (invalid)
                    mmu_plus1[i]=0;
//N                 mmu_plus1[i]=n|25;  // no-invalid yet
                } else {                    // normal ram (transparent)
                    mmu_plus1[i]=n|25;
                }
            } else {                        // Amiga MEM ***************
                n=((i<<10)|ramt)|25;        // (transparent)
                mmu_plus1[i]=n;
            }
        }

        // rom-táblázat
        for(i=0;i<2048;i++) {
            n=(i<<10)&0x1f0000;
            if(n==romb) {                   // Plus/4 ROM **************
                n=(i<<10)|romt;
                k=n&0xfc00;
                k^=0xfc00;
                j=n&0x8000;
                if(k==0) {                  // i/o area (invalid)
                    mmu_plus2[i]=0;
//N                 mmu_plus2[i]=n|25;  // no-invalid yet
                } else if(j==0) {           // map to ram (map)
                    mmu_plus2[i]=((n-0x10000)|25);
                } else {                    // normal rom (transparent)
                    mmu_plus2[i]=n|25;
                }
            } else {                        // Amiga MEM ***************
                n=((i<<10)|ramt)|25;        // (transparent)
                mmu_plus2[i]=n;
            }
        }

    }

    mmu_inited=1;
    return(0);
}
