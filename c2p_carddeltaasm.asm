* :ts=4             c2p_carddeltaasm.asm
*
*    cp4 - Commodore C+4 emulator
*    Copyright (C) 1999 Gáti Gergely
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

                xdef    _carddelta

                xref    _GfxBase

        include "lvos/graphics_lib.i"

LIBCALL         MACRO               ;FunctionName
                jsr _LVO\1(a6)
                ENDM

SCRHEIGHT   EQU 283
SCRWIDTH    EQU 352

* IN:   a0: rp
*       a1: temprp
*       d0: chunky
*       d1: delta
_carddelta      movem.l d2-d4/a2-a6,-(sp)
                move.l  #(SCRHEIGHT-1)*SCRWIDTH,d3
                add.l   d3,d0
                add.l   d3,d1
                move.l  d0,a2           ; WritePixelLine8()-nak (chunky)
                move.l  d1,a3           ; delta buffer
                move.l  _GfxBase,a6
                move.l  #SCRHEIGHT-1,d3 ; ciklus érték (ystart)
                move.l  #SCRWIDTH,d2    ; width
                move.l  d2,d4
                lsr.l   #4,d4           ; width/16
                subq.l  #1,d4
                move.l  a0,a4           ; rp
                move.l  a1,a5           ; temprp
.cik            move.l  d4,d0           ; (width/16)-1
                move.l  a2,a0           ; chunky save
                move.l  a3,a1           ; delta save
.delta          cmp.l   (a0)+,(a1)+
                bne     .cnv
                cmp.l   (a0)+,(a1)+
                bne     .cnv
                cmp.l   (a0)+,(a1)+
                bne     .cnv
                cmp.l   (a0)+,(a1)+
                bne     .cnv
                dbra    d0,.delta
                sub.l   d2,a2           ; egy sorral feljebb... (chunky)
                sub.l   d2,a3           ; deltában is
                dbra    d3,.cik
                bra     .end
.cnv            move.l  d3,d1           ; ystart
                clr.l   d0              ; xstart
                move.l  a4,a0           ; rp
                move.l  a5,a1           ; temprp
                LIBCALL WritePixelLine8
                sub.l   d2,a2           ; egy sorral feljebb... (chunky)
                sub.l   d2,a3           ; deltában is
                dbra    d3,.cik
.end            movem.l (sp)+,d2-d4/a2-a6
                rts
