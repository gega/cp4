* :ts=4							dogfx_nrmgrf.asm
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

GFX_TYPE	EQU		GFX_NRMGRF

		xref	_grflumtab
		xref	_grfcoltab
		xref	_grfmasktab

		xref	_amirasaddr
*S		xref	_textplace
*S		xref	_colplace
		xref	_p4actras
*S		xref	_colback
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
*S		xref	_gfxaddr

		xdef	_dogfx_nrmgrf


*----------------------------------------------------------------------------
* NRMGRF
*----------------------------------------------------------------------------

_dogfx_nrmgrf	movem.l	d2-d7/a1-a6,-(sp)

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
				move.l	#352,d1
				move.l	_amirasaddr,a0		* a0 - amirasaddr
				add.l	d1,a0
				move.l	a0,-(sp)			* lineend
				sub.l	d1,a0
				move.l	a0,-(sp)			* linestart

* konstansok számítása (inch-d6, txrow-d2, p4actras-d3)
* d2 == ras
				add.l	MY_vscroll(a1),d2	* függ. scroll
 				move.w 	d2,d6				* d2/d6 - ras --
				move.l	d2,d3
				addq.l	#3,d3
				lsr.w 	#3,d2				* d2 - txrow --
				and.w 	#7,d6				* d6 - inch 						(d6)

* calc gfxaddr-a5
* d2 == txrow
				move.l	d2,d4				* d4 - txrow store
				move.l 	a2,a5				* a5 - p4ram --		(RAM EQUR a2)
				clr.l	d0
				move.w	MY_gfxaddr(a1),d0
				add.l 	d0,a5				* a5 - gfxaddr						(a5)
				add.w	d6,a5				*      +inch
				move.w	(mul320,d4.l*2),d4
				add.l	d4,a5

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
				move.l 	a2,a1				* a1 - p4ram --		(RAM EQUR a2)
				add.w	d2,a1				* a1 - p4ram+chad
				move.l	a1,a2				* a2 - p4ram+chad
				add.l	MY_colplace(a4),a1	* a1 - colinram (p4ram+colplace+chad a1)
				add.l	MY_textplace(a4),a2	* a2 - chinram (p4ram+textplace+chad a2)

* calc linetable
* a6 == clinestore
* a1 == colinram
* a2 == chrinram
				move.l	#_grflumtab,a3		; 256*8=2048 ($800)
***				move.l	#_grfcoltab,a4		;						$0800 + a3
				move.l	a6,d7				; store clinestore
				addq.l	#4,a6				; skip type
; small inner
				clr.l	d1
				clr.l	d2
				moveq	#39,d0
.scik			move.b	(a1)+,d1			; colinram
				move.b	(a2)+,d2			; chrinram
				move.l	(a3,d1.l*8),d3
				move.l	4(a3,d1.l*8),d4
				or.l	($0800,a3,d2.l*8),d3		; d3-backcol.l
				or.l	($0804,a3,d2.l*8),d4		; d4-frontcol.l
				move.l	d3,(a6)+
				move.l	d4,(a6)+
				dbf		d0,.scik
				move.l	d7,a6				; restore clinestore

* minden OK, nem kell badtable-t számolni
* a6 == clinetable
* a5 == gfxaddr
* a0 == amirasaddr
.okline
				addq.l	#4,a6				; get start of clinetable
				move.l	#_grfmasktab,a3
				add.l	MY_leftborder(a4),a0		* a0 - amirasaddr+16 (kép)
				clr.l	d1
				clr.l	d2
*----------------------------------------------------------------------------
* INNER
				moveq	#39,d0					; H 	CYC		T
;--------------------------------------------------------------------
.cik			move.l	(a6)+,d3				; get backcol
				move.b	(a5),d1					; get (gfxaddr)
				beq		.back
				addq.l	#8,a5
				move.l	(a6)+,d4				; get frontcol
				move.l	(a3,d1.l*8),d5			; get mask lo
				move.l	d3,d7					; store back col  (d7)
				move.l	d4,d6					;  -o-  front col (d6)
				and.l	d5,d6
				not.l	d5
				and.l	d5,d7
				or.l	d6,d7
				move.l	d7,(a0)+				; to screen
				move.l	4(a3,d1.l*8),d5			; get mask hi
				and.l	d5,d4
				not.l	d5
				and.l	d5,d3
				or.l	d3,d4
				move.l	d4,(a0)+				; to screen
				dbf		d0,.cik
				bra		.end
.back			move.l	d3,(a0)+
				addq.l	#8,a5
				move.l	d3,(a0)+
				addq.l	#4,a6
				dbf		d0,.cik
.end
;-------------------------------------------------------------------------
* keret rajzolás
;				move.l	_sideborders,d0
;				subq	#1,d0
				move.l	(sp)+,a2				* linestart
				move.l	(sp)+,a1				* lineend
				move.l	_colbordlong,d1
;.brdcik			move.l	d1,(a2)+
; 				move.l	d1,-(a1)
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

	cnop 0,4
mul40	dc.w	0,40,80,120,160,200,240,280,320,360,400,440,480,520,560,600,640,680,720,760,800,840,880,920,960,1000
	cnop 0,4
mul320	dc.w	0,320,640,960,1280,1600,1920,2240,2560,2880,3200,3520,3840,4160,4480,4800,5120,5440,5760,6080,6400,6720,7040,7360,7680,8000
	cnop 0,4
