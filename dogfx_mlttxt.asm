* :ts=4							dogfx_mlttxt.asm
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

GFX_TYPE	EQU		GFX_MLTTXT

		xref	_mlttab

		xref	_amirasaddr
*S		xref	_textplace
*S		xref	_colplace
		xref	_p4actras
*S		xref	_colback			* ff15
*S		xref	_col1				* ff16
*S		xref	_col2				* ff17
*S		xref	_col3				* ff18
		xref	_colbordlong
		xref	_myram
*S		xref	_charmap
*S		xref	_leftborder
		xref	_rightborder
		xref	_sideborders
*S		xref	_curpos
*S		xref	_vscroll
		xref	_tbordminus
*S		xref	_numraster
*S		xref	_coltable
*S		xref	_backtable

		xdef	_dogfx_mlttxti
		xdef	_dogfx_mlttxtn

*----------------------------------------------------------------------------
* MLTTXT
*----------------------------------------------------------------------------

_dogfx_mlttxtn	movem.l	d2-d7/a1-a6,-(sp)
				move.l 	MY_charmap(a1),d7	* a5 - charmap						(a5)
				and.w	#~$07ff,d7			* csak a felsõ 5 bit
				bra		folyt

_dogfx_mlttxti	movem.l	d2-d7/a1-a6,-(sp)
				move.l 	MY_charmap(a1),d7	* a5 - charmap						(a5)

folyt
* get p4actras-d2
*NEW
*				move.l	_p4actras,d2
				clr.l	d2
				move.w	d4,d2
*NEW

* csak border? (calc ras-d2)
* d2 == p4actras
				subq.l 	#3,d2				* d2 - ras (p4actras-3)
				move.l	d2,d0
				sub.l	_tbordminus,d0
				bmi 	border
				cmp.l 	MY_numraster(a1),d0		* fent kilóg?
				bpl 	border				* igen, keret kell

* kerethatárok (amirasaddr-a0)
				move.l	#352,d1
				move.l	_amirasaddr,a0		* a0 - amirasaddr
				add.l	d1,a0
				move.l	a0,-(sp)			* lineend
				sub.l	d1,a0
				move.l	a0,-(sp)			* linestart

* konstansok számítása (inch-d6, txrow-d2, charmap-a5, p4actras-d3)
* d2 == ras
* d7 == charmap
				add.l	MY_vscroll(a1),d2	* függ. scroll
				move.l 	d2,d6				* d2/d6 - ras --
				move.l	d2,d3
				addq.l	#3,d3
				lsr.w 	#3,d2				* d2 - txrow --
				and.w 	#7,d6				* d6 - inch 						(d6)
				move.l	d7,a5				* get charmap
				add.l	d6,a5				* fix charmap (+inch)

* test badline
* d3 == p4actras
				move.l	#clinestore,a6
				moveq	#GFX_TYPE,d6		* gfx_type <=127
				move.l	a1,a4				* new BASE-REG
				tst.b	328(a6,d3.l)
				bne		.badline
				cmp.l	(a6),d6				* type same?
				beq		.okline

.badline		move.l	d6,(a6)				* update type

* colinram-a1 chrinram-a2
* d2 == txrow
				move.w	(mul40,d2.l*2),d2
				move.l 	a2,a1				* a1 - p4ram --			(RAM EQUR a2)
				add.w	d2,a1				* a1 - p4ram+chad
				move.l	a1,a2				* a2 - p4ram+chad
				add.l	MY_colplace(a4),a1		* a1 - colinram (p4ram+colplace+chad a1)
				add.l	MY_textplace(a4),a2		* a2 - chinram (p4ram+textplace+chad a2)

* calc linetable
* a6 == clinestore
* a1 == colinram
* a2 == chrinram
				move.l	a6,d7				; store clinestore
				addq.l	#4,a6				; skip type
				moveq	#$7f,d4
				clr.l	d2
; small inner
				moveq	#39,d0
.scik			move.b	(a1)+,d1			; colinram
				and.b	d4,d1
				bclr	#3,d1
				beq		.snrm
.smlt			clr.w	(a6)+				; set multi
				move.b	(a2)+,d2			; set chrinram
				move.w	d2,(a6)+
				move.b	d1,d3
				lsl.w	#8,d3
				move.b	d1,d3
				move.w	d3,(a6)+			; make multi data
				dbf		d0,.scik
				bra		.send
.snrm			move.w	a2,(a6)+			; set normal (--> a2 != $0000)
				lsl.w	#8,d1
				move.w	d1,(a6)+			; 'mmc0..'
				move.b	(a2)+,d2			; 'mmc00d'
				move.w	d2,(a6)+
				dbf		d0,.scik
.send			move.l	d7,a6				; restore clinestore

* minden OK, nem kell badtable-t számolni
* a6 == clinetable
* a5 == gfxaddr
* a0 == amirasaddr
.okline
				addq.l	#4,a6				; get start of clinetable
				move.l	a6,a1				; ... and put it into a1
				add.l	MY_leftborder(a4),a0		; a0 - amirasaddr+16 (kép)
* táblázatok:
				move.l	#coltab,a6
				move.l	MY_backtable(a4),a3
				move.l	#_mlttab,a2			; multi segéd table
* háttérszín.w:
* 0ccccccc xxxxxxxx
*  backcol
				clr.l	d1					* FF15
				move.b 	MY_colback(a4),d1	* d1 - back							(d1)
				and.b	#$7f,d1
				move.b	d1,d4
				lsl.w	#8,d1
				move.b	d4,d1
				move.w	d1,(a6)
				move.w	d1,d4
				swap	d4
				move.w	d1,d4				* backlong
* for color:
* 0ccccccc xxxxxxxx
*  frntcol
				clr.l	d2
				move.b	MY_col1(a4),d0		* FF16
				move.b	d0,2(a6)
				move.b	d0,3(a6)
				move.b	MY_col2(a4),d0			* FF17
				move.b	d0,4(a6)
				move.b	d0,5(a6)
				move.l	MY_coltable(a4),a4
				moveq	#3,d5				* for and
				moveq	#39,d0

.cik			tst.w	(a1)+				; test linetable
				bne		.nrm

.mlt			move.w	(a1)+,d3			; get code
				move.w	(a1)+,6(a6)			; get color11
				move.b	(a5,d3.w*8),d2
				beq		.back
				move.l	(a2,d2.w*8),d3
				move.w	(a6,d3.w*2),(a0)+
				swap	d3
				move.w	(a6,d3.w*2),(a0)+
				move.w	6(a2,d2.w*8),d3
				move.w	(a6,d3.w*2),(a0)+
				and.b	d5,d2
				move.w	(a6,d2.w*2),(a0)+
				dbf		d0,.cik
				bra		last

.nrm			move.l	(a1)+,d7
				move.b	(a5,d7.w*8),d1
				beq		.back
				move.l	(a3,d1.w*8),d6
				swap	d7
				move.b	d1,d7
				or.l	(a4,d7.w*8),d6
				move.l	d6,(a0)+
				move.l	4(a3,d1.w*8),d6
				or.l	4(a4,d7.w*8),d6
				move.l	d6,(a0)+
				dbf		d0,.cik
				bra		last

.back			move.l	d4,(a0)+
				move.l	d4,(a0)+
				dbf		d0,.cik

*----------------------------------------------------------------------------
* keret rajzolás
last			;move.l	_sideborders,d0
				;subq.l	#1,d0
				move.l	(sp)+,a2				* linestart
				move.l	(sp)+,a1				* lineend
				move.l	_colbordlong,d1
;.brdcik			move.l	d1,(a2)+
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
; vége, amirasaddr update
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
coltab		dc.w	0,0,0,0
	cnop	0,4
			dc.l	0
