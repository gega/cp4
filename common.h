/* :ts=4                            common.h
 *
 *    cp4 - Commodore C+4 emulator
 *    Copyright (C) 1998 G�ti Gergely
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

#ifndef CP4_COMMON_H
#define CP4_COMMON_H

#ifdef __SASC
#define inline __inline
#endif

// machine dependent types
typedef unsigned short  u_word;     // 16bit
typedef unsigned char   u_byte;     // 8bit
typedef signed short    s_word;     // 16bit
typedef signed char     s_byte;     // 8bit

// lastwrite glob�lishoz
#define NOWRITE     -1

// rom size
#define MAXROM          16384

// ROM names
#define ROM_BASIC       0
#define ROM_KERNAL      1
#define ROM_FUNCLOW     2
#define ROM_FUNCHIG     3
#define ROM_CAR1LOW     4
#define ROM_CAR1HIG     5
#define ROM_CAR2LOW     6
#define ROM_CAR2HIG     7

#endif
