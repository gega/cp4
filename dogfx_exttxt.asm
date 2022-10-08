* :ts=4							dogfx_exttxt.asm
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

GFX_TYPE	EQU		GFX_EXTTXT

		xref	_amirasaddr
*S		xref	_textplace
*S		xref	_colplace
		xref	_p4actras
*S		xref	_colback
*S		xref	_col1
*S		xref	_col2
*S		xref	_col3
		xref	_colbordlong
		xref	_myram
*S		xref	_charmap
*S		xref	_leftborder
		xref	_rightborder
		xref	_sideborders
*S		xref	_vscroll
		xref	_tbordminus
*S		xref	_numraster
*S		xref	_coltable
*S		xref	_backtable

		xdef	_dogfx_exttxti
		xdef	_dogfx_exttxtn

*----------------------------------------------------------------------------
* EXTTXT
*----------------------------------------------------------------------------
_dogfx_exttxtn	movem.l	d2-d7/a1-a6,-(sp)

* get charmap-a5
				move.l 	MY_charmap(a1),d7	* a5 - charmap						(a5)
				and.w	#~$07ff,d7			* csak a felsõ 5 bit
				bra		folyt

_dogfx_exttxti	movem.l	d2-d7/a1-a6,-(sp)
				move.l 	MY_charmap(a1),d7	* a5 - charmap						(a5)

folyt
* get p4actras-d2
*NEW
*				move.l 	_p4actras,d2		* d2 - p4actras
				clr.l	d2
				move.w	d4,d2
*NEW

* csak border? (calc ras-d2)
* d2 == p4actras
				subq.l 	#3,d2				* d2 - ras (p4actras-3)
				move.l	d2,d0
				sub.l	_tbordminus,d0
				bmi 	border
				cmp.l 	MY_numraster(a1),d0	* fent kilóg?
				bpl 	border				* igen, keret kell

* kerethatárok (amirasaddr-a0)
				clr.l	d1
				move.w	#352,d1
				move.l	_amirasaddr,a0		* a0 - amirasaddr
				add.l	d1,a0
				move.l	a0,-(sp)			* lineend
				sub.l	d1,a0
				move.l	a0,-(sp)			* linestart

* konstansok számítása (txrow-d2, p4actras-d3)
* d2 == ras
* a5 == charmap
				add.l	MY_vscroll(a1),d2	* függ. scroll
				move.w 	d2,d6				* d2/d6 - ras --
				move.l	d2,d3
				addq.l	#3,d3
				lsr.w 	#3,d2				* d2 - txrow --
				and.w 	#7,d6				* d6 - inch 						(d6)
				add.w	d6,d7
				move.l	d7,a5				* d7 - chrmap

* test badline
* d3 == p4actras
				move.l	#clinestore,a6
				moveq	#GFX_EXTTXT,d6
				move.l	a1,a4				* new BASE-REG
				tst.b	328(a6,d3.l)
				bne		.badline
				cmp.l	(a6),d6
				beq		.okline

.badline		move.l	d6,(a6)				* update type

* colinram-a1 chrinram-a2
* d2 == txrow
				move.w	(mul40,d2.l*2),d2
				move.l 	a2,a1				* a1 - p4ram --		(RAM EQUR a2)
				add.l	d2,a1				* a1 - p4ram+chad
				move.l	a1,a2				* a2 - p4ram+chad
				add.l	MY_colplace(a4),a1		* a1 - colinram (p4ram+colplace+chad a1)
				move.l	MY_textplace(a4),d0
				add.l	d0,a2				* a2 - chinram (p4ram+textplace+chad a2)

* calc linetable
* a6 == clinestore
* a1 == colinram
* a2 == chrinram
				move.l	a6,d7				; store clinestore
				addq.l	#4,a6				; skip type
; small inner
; 0b0d00c0
; b: Backcode (2 bit, charcode lsr.b #6)
; d: charcoDe (code & $3f)
; c: Color & $7f
				clr.l	d3
				moveq	#$3f,d5
				moveq	#$7f,d6
				moveq	#39,d0
.scik			move.b	(a1)+,d1			; ......cc	color	(d1)
				and.l	d6,d1
				move.b	(a2)+,d3			; ....00dd	code	(d3)
				move.l	d3,d4
				and.b	d5,d3
				move.w	d3,(a6)+			; 0d charcoDe
				lsr.b	#6,d4
				move.w	d4,(a6)+			; 0b Backcode
				lsl.l	#8,d1
				move.l	d1,(a6)+			; 00c0 Color
				dbf		d0,.scik
				move.l	d7,a6				; restore clinestore

* minden OK, nem kell badtable-t számolni
* --IN-------------
* a6 == clinetable
* a5 == charmap
* a0 == amirasaddr
* --OUT------------
* a0 - amirasaddr
* a1 - clinetable
* a3 - backtable
* a4 - coltable
* a6 - coltab (ff15-16-17-18)
.okline			addq.l	#4,a6				; get start of clinetable
				move.l	a6,a1				; to a1
				add.l	MY_leftborder(a4),a0		; a0 - amirasaddr+16 (kép)
				move.l	MY_backtable(a4),a3
				move.b 	MY_colback(a4),d1
				move.l	#coltab,a6
				moveq	#$7f,d2
				and.b	d2,d1
				move.b	d1,(a6)				* ff15
				move.b	MY_col1(a4),d1
				and.b	d2,d1
				move.b	d1,2(a6)			* ff16
				move.b	MY_col2(a4),d1
				and.b	d2,d1
				move.b	d1,4(a6)			* ff17
				move.b	MY_col3(a4),d1
				and.b	d2,d1
				move.b	d1,6(a6)			* ff18
				move.l	MY_coltable(a4),a4
*----------------------------------------------------------------------------
* INNER
				moveq	#39,d0
.cik			move.l	(a1)+,d3				; get 0d0b - charcoDe, Backcode
				move.w	(a6,d3.w*2),d1			; cc00 - backCol
				swap	d3						; get 0b0d - Backcode, charcoDe
				move.b	(a5,d3.w*8),d1			; ccgg - backCol, Gfx (d1)
				beq		.back
				move.l	(a1)+,d2				; get col 0000cc00
				move.l	(a3,d1.w*8),d3
				move.b	d1,d2
				or.l	(a4,d2.w*8),d3
				move.l	d3,(a0)+
				move.l	4(a3,d1.w*8),d3
				or.l	4(a4,d2.w*8),d3
				move.l	d3,(a0)+
				dbf		d0,.cik
				bra		.ok
.back			move.w	d1,d2					; ....cc00 d1/d2
				lsr.w	#8,d1					; ....00cc d1
				move.b	d1,d2					; ....cccc d2
				move.w	d2,d1					; ....cccc d1
				swap	d2						; cccc.... d2
				move.w	d1,d2					; cccccccc d2
				move.l	d2,(a0)+
				addq.l	#4,a1
				move.l	d2,(a0)+
				dbf		d0,.cik
.ok
*----------------------------------------------------------------------------
* keret rajzolás
;				move.l	_sideborders,d0
;				subq.l	#1,d0
				move.l	(sp)+,a2				* linestart
				move.l	(sp)+,a1				* lineend
				move.l	_colbordlong,d1
;.brdcik		move.l	d1,(a2)+
;				move.l	d1,-(a1)
;				dbf		d0,.brdcik
				move.l	d1,(a2)+
				move.l	d1,-(a1)
				move.l	d1,(a2)+
				move.l	d1,-(a1)
				move.l	d1,(a2)+
				move.l	d1,-(a1)
				move.l	d1,(a2)+
				move.l	d1,-(a1)				* ez mindig kell (4)
				tst.l	_sideborders
				bne		.brdplus
* vége, amirasaddr update
				add.l	_rightborder,a0		* amirasaddr+=rightborder
				move.l 	a0,_amirasaddr		* update
				movem.l	(sp)+,d2-d7/a1-a6
				rts
.brdplus		move.l	d1,(a2)+
				move.l	d1,-(a1)
				move.l	d1,(a2)+
				move.l	d1,-(a1)				* néha (6)
; vége, amirasaddr update
				add.l	_rightborder,a0		* amirasaddr+=rightborder
				move.l 	a0,_amirasaddr		* update
				movem.l	(sp)+,d2-d7/a1-a6
				rts

* CSAK KERET
border			moveq	#10,d0
				move.l	_colbordlong,d1
				move.l	_amirasaddr,a0
.bordcik		move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				move.l	d1,(a0)+
				dbf		d0,.bordcik
				move.l	a0,_amirasaddr
				movem.l	(sp)+,d2-d7/a1-a6
				rts
*----------------------------------------------------------------------------
	cnop	0,4
mul40		dc.w	0,40,80,120,160,200,240,280,320,360,400,440,480,520,560,600,640,680,720,760,800,840,880,920,960,1000
	cnop	0,4
*	shift			1		3		5		7
*	FF				15		16		17		18
coltab		dc.b	0,0,	0,0,	0,0,	0,0
	cnop	0,4
			dc.l	0
