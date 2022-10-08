/* :ts=4                            debug_ext.h
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
#ifndef DEBUG_EXT_H
#define DEBUG_EXT_H

#include <proto/diskfont.h>
struct Library *DiskfontBase=NULL;

// dbg.c
extern int dodebug_out(int flag);
// ted.c
extern void soundpause(void);
extern void soundresume(void);
extern int getjoy(void);
extern void freejoy(void);

/*
 * flag: user hívta (0), vagy breakpoint (1)
 */
int REGARGS dodebug(REG(d0,int flag)) {
    int diskfontopened=0;
    int r;

    if(!DiskfontBase) {
        if(NULL==(DiskfontBase=OpenLibrary("diskfont.library",37)))
            return(128);
        diskfontopened=1;
    }
    soundpause();
    freejoy();
    c2pv->c2p_sleep();
    r=dodebug_out(flag);
    if((r&128)==0) soundresume();
    getjoy();
    if(diskfontopened!=0) {
        CloseLibrary(DiskfontBase);
        DiskfontBase=NULL;
    }
    c2pv->c2p_awake();
    return(r);
}

#endif
