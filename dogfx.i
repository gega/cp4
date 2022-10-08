* :ts=8                         dogfx.i
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

        IFND    DOGFX_I
DOGFX_I         EQU     1


        xref    cbadlines       ; b     (BadLineTable.b)
        xref    clinestore      ; l     (LineDataStorage)

        xref    _c2pv           ; struct (c2p_module.h)

direct                  MACRO
                                move.l  a1,-(sp)
                                move.l  _c2pv,a0
                                add.l   #140,a0
                                move.l  (a0),a0
                                jsr             (a0)                            ; Lock
                                tst.l   d0
                                beq             \@1$
                                move.l  d0,a0
                                move.l  a0,-(sp)
                                move.l  (a0),d0                         ; DisplayMem
                                move.l  _amirasaddr,-(sp)
                                add.l   d0,_amirasaddr
                                jsr             \1                                      ; Draw
                                move.l  (sp)+,_amirasaddr
                                move.l  (sp)+,a0
                                addq.l  #4,a0
                                move.l  (a0),d0
                                add.l   d0,_amirasaddr          ; +BytesPerRow
                                move.l  _c2pv,a0
                                add.l   #144,a0
                                move.l  (a0),a0
                                jsr             (a0)                            ; UnLock
\@1$                    move.l  (sp)+,a1
                                ENDM


GFX_NODATA      EQU     0
        ; NODATA (0)
        ; No local data


GFX_NRMTXT_I    EQU     1
        ; NRMTXT_I (240) 128 char (inv)
        ; 120 word
        ; ci0d1f / char (8 pixel)
        ; c: Color & $7f
        ; i: Inverz $00 - ha code pozitív
        ;           $ff - ha code negatív
        ; 0: $00
        ; d: coDe & $7f
        ; 1: $ff
        ; f: Flash  $00 - ha color negatív
        ;           $ff - ha color pozitív

GFX_NRMTXT_2    EQU     6
        ; NRMTXT_2 (160) 256 char (256)
        ; 40 long
        ; cf0d / char (8 pixel)
        ; c: Color & $7f
        ; f: Flash  $00 - ha color negatív
        ;           $ff - ha color pozitív
        ; d: coDe

GFX_NRMGRF      EQU     2
        ; NRMGRF (320)
        ; 80 long
        ; bbbbffff / char (8 pixel)
        ; b: BackColor
        ; f: FrontColor

GFX_MLTTXT      EQU     3
        ; MLTTXT (240)
        ; 120 word
        ; mmxxxx / char (8 pixel)
        ; 000dff        ha multi
        ; 11c00d        ha normál
        ; c: Color
        ; d: coDe
        ; f: front color (11)
        ; m: 0-ha multi, más ha normál char (mindig csak az a rész érvényes!)

GFX_MLTGRF      EQU     4
        ; MLTGRF (160)
        ; 40 long
        ; aabb / char (8 pixel)
        ; a: color0 (01)
        ; b: color1 (10)

GFX_EXTTXT      EQU     5
        ; EXTTXT
        ; 80 long (320)
        ; 0b0d00c0
        ; 0: zero
        ; b: Backcode (2 bit, char code lsr.b #6)
        ; d: charcoDe (charcode & $3f)
        ; c: color & $7f


        ENDIF
*----------------------------------------------------------------------------
