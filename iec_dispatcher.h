/* :ts=4						iec_ dispatcher.h
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
#ifndef IEC_DISPATCHER_H
#define IEC_DISPATCHER_H

int d_GetST(char devaddr);
void d_Reset(char devaddr);
int d_Init(char devaddr);
int d_State(char devaddr);
void d_Listen(char devaddr);
void d_Talk(char devaddr);
int d_ACPtr(void);
void d_CIOut(unsigned char chr);
void d_Second(char secaddr);
void d_TkSA(char secaddr);
void d_UnListen(void);
void d_UnTalk(void);

#endif
