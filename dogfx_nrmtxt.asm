* :ts=4							dogfx_nrmtxt.asm
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
		xref	_opt_noborder
*S		xref	_curpos
		xref	_cureor
*S		xref	_vscroll
		xref	_tbordminus
*S		xref	_numraster
*S		xref	_coltable
*S		xref	_backtable

		xdef	_dogfx_nrmtxt128
		xdef	_dogfx_nrmtxt256
		xdef	_dogfx_nrmtxt128f
		xdef	_dogfx_nrmtxt256f

*----------------------------------------------------------------------------
* konstansszámítás
* countconsts \1
* \1: 0: 128 char		1: 256 char mode
* \2: 0: kell curpos	1: nem kell
*----------------------------------------------------------------------------
countconsts		MACRO

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
				cmp.l 	MY_numraster(a1),d0		* fent kilóg?
				bpl 	border				* igen, keret kell

* kerethatárok (amirasaddr-a0)
				move.l	#352,d1
				move.l	_amirasaddr,a0		* a0 - amirasaddr
				add.l	d1,a0
				move.l	a0,-(sp)			* lineend
				sub.l	d1,a0
				move.l	a0,-(sp)			* linestart

* konstansok számítása (txrow-d2, charmap-a5, p4actras-d3)
* d2 == ras
				add.l	MY_vscroll(a1),d2	* függ. scroll
				move.l 	d2,d6				* d2/d6 - ras --
				move.l	d2,d3
				addq.l	#3,d3
				lsr.w 	#3,d2				* d2 - txrow --
				and.w 	#7,d6				* d6 - inch 						(d6)
				move.l 	MY_charmap(a1),d0	* a5 - charmap						(a5)
	IFNE	\1
			; 256 char mode (\1!=0)
				and.w	#~$07ff,d0			* csak a felsõ 5 bit
	ENDIF
				add.w	d6,d0
				move.l	d0,a5

* test badline
* d3 == p4actras
				move.l	#clinestore,a6
	IFEQ	\1
		; 128 char mode
				moveq	#GFX_NRMTXT_I,d6
	ELSE
		; 256 char mode
				moveq	#GFX_NRMTXT_2,d6
	ENDIF
				move.l	a1,a4				* new BASE-REG
				tst.b	328(a6,d3.l)
				bne		\@1$				* badline
				cmp.l	(a6),d6
				beq		\@2$				* okline

* BADLINE
* -------
\@1$			move.l	d6,(a6)				* update type

* colinram-a1 chrinram-a2
* d2 == txrow
				move.w	(mul40,d2.l*2),d2
				move.l 	a2,a1				* a1 - p4ram --		(RAM EQUR a2)
	IFEQ	\2
		; curpos kell
				move.l	a1,a3
	ENDIF
				add.l	d2,a1				* a1 - p4ram+chad
				move.l	a1,a2				* a2 - p4ram+chad
				add.l	MY_colplace(a4),a1		* a1 - colinram (p4ram+colplace+chad a1)
				move.l	MY_textplace(a4),d0
				add.l	d0,a2				* a2 - chinram (p4ram+textplace+chad a2)
	IFEQ	\2
		; curpos kell
				add.l	d0,a3
				add.l	MY_curpos(a4),a3			* set curpos
				move.b	_cureor,d6			* store cureor in d6
				eor.b	d6,(a3)				* set cursor
	ENDIF

* calc linetable
* a6 == clinestore
* a1 == colinram
* a2 == chrinram
				move.l	a6,d7				; store clinestore
				addq.l	#4,a6				; skip type
				clr.l	d3
				move.w	#$7fff,d4
	IFEQ	\1
		; 128 char mode
				moveq	#$7f,d5
	ENDIF
; small inner
				moveq	#39,d0
\@3$			move.b	(a1)+,d1			; ......cc	color	(d1)
				lsl.w	#8,d1				; ....cc..			(d1)
	IFEQ	\1
		; 128 char mode						; ci0d1f
				spl		d2					; ....ffFF	flash	(d2)
				move.b	(a2)+,d3			; ....00dd	code	(d3)
				smi		d1					; ....ccii	inverz	(d1)
				and.w	d4,d1				; ....7fff	d4		(d1)
				move.w	d1,(a6)+			; >> ci
				and.b	d5,d3				; ......7f	d5		(d3)
				move.w	d3,(a6)+			; >> ci0d
				move.w	d2,(a6)+			; >> ci0d1f
	ELSE
		; 256 char mode						; cf0d
				spl		d1
				and.w	d4,d1				; ....7fff	d4		(d1)
				move.w	d1,(a6)+			; >> cf
				move.b	(a2)+,d3			; ....00dd	code	(d3)
				move.w	d3,(a6)+			; >> cf0d
	ENDIF
				dbf		d0,\@3$

				move.l	d7,a6				; restore clinestore
	IFEQ	\2
		; curpos kell
				eor.b	d6,(a3)				; restore cursor
	ENDIF

* OKLINE
* ------
* minden OK, nem kell badtable-t számolni
* --IN-------------
* a6 == clinetable
* a5 == charmap
* a0 == amirasaddr
* --OUT------------
* d1 - backcol : $....bb..
* d4 - $7f.l
* d6 - backlong.l
* a0 - amirasaddr
* a3 - backtable
* a4 - coltable
* a6 - clinetable
\@2$
				addq.l	#4,a6				; get start of clinetable
				add.l	MY_leftborder(a4),a0		; a0 - amirasaddr+16 (kép)
				move.l	MY_backtable(a4),a3
				move.b 	MY_colback(a4),d1			* d1 - back							(d1)
				move.l	MY_coltable(a4),a4
				and.b	#$7f,d1
				move.b	d1,d6
				lsl.w	#8,d1
				move.b	d6,d1
				move.w	d1,d6
				swap	d6
				move.w	d1,d6
				ENDM


*----------------------------------------------------------------------------
* keretrajz (szélsõk)
*----------------------------------------------------------------------------
sideborder		MACRO
;				move.l	_sideborders,d0
;				subq.l	#1,d0

				move.l	(sp)+,a2			* linestart
				move.l	(sp)+,a1			* lineend
				move.l	_colbordlong,d1

;\@1$			move.l	d1,(a2)+
;				move.l	d1,-(a1)
;				dbf		d0,\@1$

				move.l	d1,(a2)+
				move.l	d1,-(a1)
				move.l	d1,(a2)+
				move.l	d1,-(a1)
				move.l	d1,(a2)+
				move.l	d1,-(a1)
				move.l	d1,(a2)+
				move.l	d1,-(a1)				* ez mindig kell (4)
				tst.l	_sideborders
				beq		\@2$
				move.l	d1,(a2)+
				move.l	d1,-(a1)
				move.l	d1,(a2)+
				move.l	d1,-(a1)				* néha (6)
* vége, amirasaddr update
\@2$			add.l	_rightborder,a0		* amirasaddr+=rightborder
				move.l 	a0,_amirasaddr		* update
				ENDM

*----------------------------------------------------------------------------
*----------------------------------------------------------------------------
*----------------------------------------------------------------------------


*--------------------------------------------------------------------
* CSAK KERET
*--------------------------------------------------------------------
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
*----------------------------------------------------------------------------
*----------------------------------------------------------------------------


*----------------------------------------------------------------------------
* NRMTXT NOFLASH 128
*----------------------------------------------------------------------------
_dogfx_nrmtxt128
				movem.l	d2-d7/a1-a6,-(sp)
				countconsts 0,0
* INNER
				moveq	#39,d0
;--------------------------------------------------------------------
.cik			move.l	(a6)+,d7
				move.b	(a5,d7.w*8),d2
				swap	d7
				eor.b	d2,d7
				beq		.back
				move.l	(a4,d7.w*8),d3
				move.b	d7,d1
				or.l	(a3,d1.w*8),d3
				move.l	d3,(a0)+
				move.l	4(a4,d7.w*8),d3
				addq.l	#2,a6
				or.l	4(a3,d1.w*8),d3
				move.l	d3,(a0)+
				dbf		d0,.cik
				bra		.ok
.back			move.l	d6,(a0)+
				addq.l	#2,a6
				move.l	d6,(a0)+
				dbf		d0,.cik
.ok
;--------------------------------------------------------------------
				sideborder
				movem.l	(sp)+,d2-d7/a1-a6
				rts

*----------------------------------------------------------------------------
* NRMTXT FLASH 128
*----------------------------------------------------------------------------

_dogfx_nrmtxt128f
				movem.l	d2-d7/a1-a6,-(sp)
				countconsts 0,1
*----------------------------------------------------------------------------
* INNER
				moveq	#39,d0
.cik			move.l	(a6)+,d7
				move.b	(a5,d7.w*8),d2
				swap	d7
				and.w	(a6)+,d2
				eor.b	d2,d7					; ....ccdd	d7
				beq		.back
				move.l	(a4,d7.w*8),d3
				move.b	d7,d1
				or.l	(a3,d1.w*8),d3
				move.l	d3,(a0)+
				move.l	4(a4,d7.w*8),d3
				or.l	4(a3,d1.w*8),d3
				move.l	d3,(a0)+
				dbf		d0,.cik
				bra		.ok
.back			move.l	d6,(a0)+
				move.l	d6,(a0)+
				dbf		d0,.cik
.ok
*----------------------------------------------------------------------------
				sideborder
				movem.l	(sp)+,d2-d7/a1-a6
				rts


*----------------------------------------------------------------------------
*----------------------------------------------------------------------------
*----------------------------------------------------------------------------

*----------------------------------------------------------------------------
* NRMTXT 256 noflash
*----------------------------------------------------------------------------

_dogfx_nrmtxt256
				movem.l	d2-d7/a1-a6,-(sp)
				countconsts 1,1
*----------------------------------------------------------------------------
* INNER
				moveq	#39,d0
.cik			move.l	(a6)+,d7				; c00d	Color / coDe	d7
				move.b	(a5,d7.w*8),d1			; ..bd	Backcol+coDe	d1
				beq		.back
				swap	d7						; 0dc0	coDe / Color	d7
				move.l	(a3,d1.w*8),d3
				move.b	d1,d7					; 0dcd	Color / coDe	d7
				or.l	(a4,d7.w*8),d3
				move.l	d3,(a0)+
				move.l	4(a3,d1.w*8),d3
				or.l	4(a4,d7.w*8),d3
				move.l	d3,(a0)+
				dbf		d0,.cik
				bra		.ok
.back			move.l	d6,(a0)+
				move.l	d6,(a0)+
				dbf		d0,.cik
.ok
*----------------------------------------------------------------------------
				sideborder
				movem.l	(sp)+,d2-d7/a1-a6
				rts
*----------------------------------------------------------------------------


*----------------------------------------------------------------------------
* NRMTXT 256 flash
*----------------------------------------------------------------------------

_dogfx_nrmtxt256f
				movem.l	d2-d7/a1-a6,-(sp)
				countconsts 1,1
*----------------------------------------------------------------------------
* INNER
				moveq	#39,d0
.cik			move.l	(a6)+,d7				; c00d	Color / coDe	d7
				move.b	(a5,d7.w*8),d1			; ..bd	Backcol+coDe	d1
				beq		.back
				swap	d7						; 0dc0	coDe / Color	d7
				and.b	d7,d1					; flash
				move.l	(a3,d1.w*8),d3
				move.b	d1,d7					; 0dcd	Color / coDe	d7
				or.l	(a4,d7.w*8),d3
				move.l	d3,(a0)+
				move.l	4(a3,d1.w*8),d3
				or.l	4(a4,d7.w*8),d3
				move.l	d3,(a0)+
				dbf		d0,.cik
				bra		.ok
.back			move.l	d6,(a0)+
				move.l	d6,(a0)+
				dbf		d0,.cik
.ok
*----------------------------------------------------------------------------
				sideborder
				movem.l	(sp)+,d2-d7/a1-a6
				rts
*----------------------------------------------------------------------------

	cnop	0,4
mul40		dc.w	0,40,80,120,160,200,240,280,320,360,400,440,480,520,560,600,640,680,720,760,800,840,880,920,960,1000
	cnop	0,4
			dc.l	0
