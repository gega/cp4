/* :ts=4						soft_iec.h
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
#ifndef SOFT_IEC_H
#define SOFT_IEC_H

#include "macros.h"

extern char d64_LED;
#define LED_OFF		0
#define LED_ON		1
#define LED_FLASH	2

// for SOFT_Close()
#define CLOSE_ALL	44

// SOFT_Init() return values
#define SERR_NOMEM		-1
#define SERR_ILLDEV		-2			// device 8-11
#define SERR_NOTFOUND	-3			// d64 file not found
#define SERR_WRONGD64	-4			// wrong filetype of d64 file
#define SERR_REOPEN		-5			// can't reopen d64 file

int SOFT_Init(REG(d0,char devaddr));
void SOFT_Close(REG(d0,char devaddr));
void REGARGS SOFT_Reset(REG(d0,char devaddr));
void REGARGS SOFT_Listen(REG(d0,char devaddr));
void REGARGS SOFT_Talk(REG(d0,char devaddr));
int SOFT_ACPtr(void);
void REGARGS SOFT_CIOut(REG(d0,unsigned char chr));
void REGARGS SOFT_Second(REG(d0,char secaddr));
void REGARGS SOFT_TkSA(REG(d0,char secaddr));
void SOFT_UnListen(void);
void SOFT_UnTalk(void);
int SOFT_State(REG(d0,char drive));

#endif
