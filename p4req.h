/* :ts=4                            p4req.h
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
#ifndef P4REQ_H
#define P4REQ_H

#include "macros.h"

#define P4_REQUEST  0
#define P4_WARNING  1
#define P4_ERROR    2
#define P4_C2P      3
#define P4_INFO     4

#define P4_OK       0
#define P4_OKCANCEL 1

int p4req1(REG(a0,void *w),REG(d0,int title),REG(a1,char *text));
int p4req2(REG(a0,void *w),REG(d0,int title),REG(a1,char *text));

#endif
