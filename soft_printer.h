/* :ts=4						soft_printer.h
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
#ifndef SOFT_PRINTER_H
#define SOFT_PRINTER_H

void SPRT_Reset(int devaddr);
int SPRT_Init(int devaddr);
int SPRT_State(int devaddr);
void SPRT_Listen(int devaddr);
void SPRT_CIOut(unsigned char chr);
void SPRT_Second(char secaddr);
void SPRT_UnListen(int devaddr);

#endif
