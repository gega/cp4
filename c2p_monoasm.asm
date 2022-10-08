* :ts=8				c2p_monoasm.asm
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

SCRHEIGHT	EQU	283
SCRWIDTH	EQU	352

	xref	_bmx			* l

	xdef	_convdeltaasm
	xdef	_convfullasm

*
* IN:	a0 - chunky
*	a1 - planar
_convfullasm	movem.l	d2-d7/a2-a3,-(sp)
		move.l	_bmx,d5
		move.l	#$40404040,d7
		move.w	#SCRHEIGHT-2,d0

.out		moveq	#(SCRWIDTH/32)-1,d1
		move.l	a1,a2
.in
	; 1. byte
		move.l	(a0)+,d3	; D3
		and.l	d7,d3		;.1......|.2......|.3......|.4......
		lsr.w	#1,d3		;.1......|.2......|..3.....|..4.....
		move.w	d3,d4		;                 \------ D4 ------/
		swap	d3		;             D3:  .1......|.2......
		add.w	d3,d3		;                  1.......|2.......
		or.w	d3,d4		;             D4:  1.3.....|2.4.....
		lsr.b	#1,d4		;             D4:  1.3.....|.2.4....
		move.w	d4,d3		;                 \------ D3 ------/
		lsr.w	#8,d3		;             D3:  ........|1.3.....
		or.b	d4,d3		;             D3:           1234....
		move.b	d3,d6		;             D6:           1234....

		move.l	(a0)+,d3	; D3
		and.l	d7,d3		;.1......|.2......|.3......|.4......
		lsr.w	#5,d3		;.1......|.2......|......3.|......4.
		move.w	d3,d4		;                 \------ D4 ------/
		swap	d3		;             D3:  .1......|.2......
		lsr.w	#3,d3		;             D3:  ....1...|....2...
		or.w	d3,d4		;                  ....1.3.|....2.4.
		lsr.b	#1,d4		;             D4:  ....1.3.|.....2.4
		move.w	d4,d3		;                 \------ D3 ------/
		lsr.w	#8,d3		;             D3:  ........|....1.3.
		or.b	d4,d3		;             D3:           ....1234
		or.b	d3,d6		;             D6:           12341234
		lsl.l	#8,d6

	; 2. byte
		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#1,d3
		move.w	d3,d4
		swap	d3
		add.w	d3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			; xxxx....
		move.b	d3,d6			; xxxx....

		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#5,d3
		move.w	d3,d4
		swap	d3
		lsr.w	#3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			;....xxxx
		or.b	d3,d6			;xxxxxxxx
		lsl.l	#8,d6

	; 3. byte
		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#1,d3
		move.w	d3,d4
		swap	d3
		add.w	d3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			; xxxx....
		move.b	d3,d6			; xxxx....

		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#5,d3
		move.w	d3,d4
		swap	d3
		lsr.w	#3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			;....xxxx
		or.b	d3,d6			;xxxxxxxx
		lsl.l	#8,d6

	; 4. byte
		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#1,d3
		move.w	d3,d4
		swap	d3
		add.w	d3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			; xxxx....
		move.b	d3,d6			; xxxx....

		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#5,d3
		move.w	d3,d4
		swap	d3
		lsr.w	#3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			;....xxxx
		or.b	d3,d6			;xxxxxxxx

	; TO-CHIP
		move.l	d6,(a2)+
		dbf	d1,.in

		add.l	d5,a1
		dbf	d0,.out

		movem.l	(sp)+,d2-d7/a2-a3
		rts


*
* IN:	a0 - chunky
*	a1 - planar
*	d0 - delta
*
_convdeltaasm	movem.l	d2-d7/a2-a3,-(sp)
		move.l	_bmx,d5
		move.l	#$40404040,d7
		move.l	d0,a3
		move.w	#SCRHEIGHT-2,d0

.out		moveq	#(SCRWIDTH/32)-1,d1
		move.l	a1,a2

.in		cmp.l	(a3)+,(a0)+
		bne	.1
		cmp.l	(a3)+,(a0)+
		bne	.2
		cmp.l	(a3)+,(a0)+
		bne	.3
		cmp.l	(a3)+,(a0)+
		bne	.4
		cmp.l	(a3)+,(a0)+
		bne	.5
		cmp.l	(a3)+,(a0)+
		bne	.6
		cmp.l	(a3)+,(a0)+
		bne	.7
		cmp.l	(a3)+,(a0)+
		bne	.8
		addq.l	#4,a2
.tov		dbf	d1,.in

		add.l	d5,a1
		dbf	d0,.out

		movem.l	(sp)+,d2-d7/a2-a3
		rts

.1		subq.l	#4,a0
		add.l	#28,a3
		bra	.conv
.2		subq.l	#8,a0
		add.l	#24,a3
		bra	.conv
.3		sub.l	#12,a0
		add.l	#20,a3
		bra	.conv
.4		sub.l	#16,a0
		add.l	#16,a3
		bra	.conv
.5		sub.l	#20,a0
		add.l	#12,a3
		bra	.conv
.6		sub.l	#24,a0
		addq.l	#8,a3
		bra	.conv
.7		sub.l	#28,a0
		addq.l	#4,a3
		bra	.conv
.8		sub.l	#32,a0

	; 1. byte
.conv		move.l	(a0)+,d3	; D3
		and.l	d7,d3		;.1......|.2......|.3......|.4......
		lsr.w	#1,d3		;.1......|.2......|..3.....|..4.....
		move.w	d3,d4		;                 \------ D4 ------/
		swap	d3		;             D3:  .1......|.2......
		add.w	d3,d3		;                  1.......|2.......
		or.w	d3,d4		;             D4:  1.3.....|2.4.....
		lsr.b	#1,d4		;             D4:  1.3.....|.2.4....
		move.w	d4,d3		;                 \------ D3 ------/
		lsr.w	#8,d3		;             D3:  ........|1.3.....
		or.b	d4,d3		;             D3:           1234....
		move.b	d3,d6		;             D6:           1234....

		move.l	(a0)+,d3	; D3
		and.l	d7,d3		;.1......|.2......|.3......|.4......
		lsr.w	#5,d3		;.1......|.2......|......3.|......4.
		move.w	d3,d4		;                 \------ D4 ------/
		swap	d3		;             D3:  .1......|.2......
		lsr.w	#3,d3		;             D3:  ....1...|....2...
		or.w	d3,d4		;                  ....1.3.|....2.4.
		lsr.b	#1,d4		;             D4:  ....1.3.|.....2.4
		move.w	d4,d3		;                 \------ D3 ------/
		lsr.w	#8,d3		;             D3:  ........|....1.3.
		or.b	d4,d3		;             D3:           ....1234
		or.b	d3,d6		;             D6:           12341234
		lsl.l	#8,d6

	; 2. byte
		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#1,d3
		move.w	d3,d4
		swap	d3
		add.w	d3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			; xxxx....
		move.b	d3,d6			; xxxx....

		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#5,d3
		move.w	d3,d4
		swap	d3
		lsr.w	#3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			;....xxxx
		or.b	d3,d6			;xxxxxxxx
		lsl.l	#8,d6

	; 3. byte
		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#1,d3
		move.w	d3,d4
		swap	d3
		add.w	d3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			; xxxx....
		move.b	d3,d6			; xxxx....

		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#5,d3
		move.w	d3,d4
		swap	d3
		lsr.w	#3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			;....xxxx
		or.b	d3,d6			;xxxxxxxx
		lsl.l	#8,d6

	; 4. byte
		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#1,d3
		move.w	d3,d4
		swap	d3
		add.w	d3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			; xxxx....
		move.b	d3,d6			; xxxx....

		move.l	(a0)+,d3
		and.l	d7,d3
		lsr.w	#5,d3
		move.w	d3,d4
		swap	d3
		lsr.w	#3,d3
		or.w	d3,d4
		lsr.b	#1,d4
		move.w	d4,d3
		lsr.w	#8,d3
		or.b	d4,d3			;....xxxx
		or.b	d3,d6			;xxxxxxxx

	; TO-CHIP
		move.l	d6,(a2)+
		bra	.tov
