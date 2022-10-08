/* :ts=4                            snapshot.h
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
#ifndef SNAPSHOT_H
#define SNAPSHOT_H

extern unsigned long sfrd2;
extern unsigned long sfrd3;
extern unsigned long sfrd4;
extern unsigned long sfrd5;
extern unsigned long sfrd6;
extern unsigned long sfrd7;
extern u_word sft1;
extern u_word sfpc;
extern u_word sfc1;
extern u_word sfc2;
extern u_word sfc3;
extern u_word sfac;
extern u_word sfc1c;
extern u_word sfc2c;
extern u_word sfc3c;
extern u_byte sfchr;
extern unsigned long sfchm;

#define TYPE_PURE   '\n'
#define TYPE_XPK    'X'
#define TYPE_PUREC  'p'
#define TYPE_XPKC   'x'


// current snapshot file (uncompressed) length (without header) (+chksum [+4])
#define SNAP_LEN    65666

#endif
