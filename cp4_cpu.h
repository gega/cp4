/* :ts=4                            cp4_cpu.h
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

#ifndef CP4_CPU_H
#define CP4_CPU_H

void setflag_n(u_byte r);
void setflag_z(u_byte r);
void setflag_c(int e);
void push(u_byte a);
u_byte pop(void);
void push2(u_word a);
u_word pop2(void);
void gethilo(u_byte a, int *ah, int *al);
int makedec(int hi,int lo);
int decadd(u_byte a,u_byte b,u_byte c);
int decsub(u_byte a, u_byte b);

#endif
