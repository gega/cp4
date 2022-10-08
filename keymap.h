/* :ts=4                            keymap.h
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
#ifndef KEYMAPEDIT_H
#define KEYMAPEDIT_H

#include "macros.h"

#define K_MAGIC     "CP4KEYMAP"

extern void InitKMapHook(void);
extern struct Hook KMapHook;
extern int OpenkmapeditWindow( void );
extern void ClosekmapeditWindow( void );
extern int HandlekmapeditIDCMP( void );
extern int BeginKMap( struct Screen *scr );
extern void EndKMap( void );

#endif
