* :ts=8                         shadow.i
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

        IFND    SHADOW_I
SHADOW_I        EQU     1

* Shadow variables (from C)
MY_actraster    EQU     $2c00           *
MY_actinchar    EQU     $2c00+4         *
MY_actcharaddr  EQU     $2c00+8         *
MY_rasreq       EQU     $2c00+12        *
MY_tbordminus   EQU     $2c00+16        ;
MY_numraster    EQU     $2c00+20        *
MY_amirasaddr   EQU     $2c00+24        ;
MY_vscroll      EQU     $2c00+28        *
MY_charmap      EQU     $2c00+32        *
MY_myram        EQU     $2c00+36        ; NOUPDATE
MY_colplace     EQU     $2c00+40        *
MY_textplace    EQU     $2c00+44        *
MY_curpos       EQU     $2c00+48        *
MY_cureor       EQU     $2c00+52        ; Byte
MY_leftborder   EQU     $2c00+56        *
MY_backtable    EQU     $2c00+60        * NOUPDATE
MY_coltable     EQU     $2c00+64        * NOUPDATE
MY_colback      EQU     $2c00+68        * Byte
MY_sideborders  EQU     $2c00+72        ;
MY_colbordlong  EQU     $2c00+76        ;
MY_rightborder  EQU     $2c00+80        ;

MY_lineend      EQU     $2c00+84        * use by dogfx_*
MY_linestart    EQU     $2c00+88        * use by dogfx_*

MY_gfxaddr      EQU     $2c00+92        * Word
MY_col1         EQU     $2c00+96        * Byte
MY_col2         EQU     $2c00+100       * Byte
MY_col3         EQU     $2c00+104       * Byte

MY_cbadlines    EQU     $2c00+108       ; *b
MY_clinestore   EQU     $2c00+112       ; *l
MY_p4rom        EQU     $2c00+116       *
MY_actrom       EQU     $2c00+120       * Byte
MY_hscroll      EQU     $2c00+124       *
MY_keymatrixp   EQU     $2c00+128       *
MY_ec1          EQU     $2c00+132       ; Byte
MY_ec2          EQU     $2c00+136       ; Byte
MY_ec3          EQU     $2c00+140       ; Byte

        ENDIF
