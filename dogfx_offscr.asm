* :ts=4							dogfx_offscr.asm
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
*
		include "dogfx.i"
		include "shadow.i"

GFX_TYPE	EQU		GFX_NODATA

		xref	_amirasaddr
		xref	_colbordlong

		xdef	_dogfx_offscr

_dogfx_offscr	move.l	#clinestore,a0
				move.l	#GFX_TYPE,(a0)
				move.l	_amirasaddr,a0
				move.l	_colbordlong,d1
				moveq	#10,d0
.cik			move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				dbf		d0,.cik
theend			move.l	a0,_amirasaddr
				rts

	cnop	0,4
			dc.l	0
