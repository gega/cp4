/* :ts=4                            ted.h
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
#ifndef TED_H
#define TED_H

extern int load(char *filename,int *to2d);          // in cp4.c
void tedinit(void);
void tedfree(void);
int doframe(void);

#endif
