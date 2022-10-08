* :ts=8                         7501.asm
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

*
* 7501 CPU emu + TED
*


                include "exec/types.i"
                include "exec/macros.i"
                include "exec/execbase.i"
                include "exec/nodes.i"
                include "lvos/exec_lib.i"
                include "lvos/intuition_lib.i"
                include "hardware/intbits.i"
                include "iec/iec.i"
                include "iec/iec_lib.i"
                include "c2p_module.i"
                include "libraries/playsidbase.i"
                include "libraries/playsid_lib.i"
                include "shadow.i"
                include "stat.i"


*       d0      - WORK, byte only!!             (000000xx)
*       d1      - WORK, word only!!             (0000xxxx)

  *                                     31        23        15        7         0
RA      EQUR    d2              * ACCU  |....:....|....:....|..1.:....|....:....|
  *                                      0000 0000 modeflag  nV-B DIzc   ACCU
RX      EQUR    d3              * X-REG (1...00xx)
  *                                      ^stopcnt1
RY      EQUR    d4              * Y-REG (____00yy)
  *                                      ^p4actras.w
  *-----------------------------------------------------------------------------
  * STATUS FLAGS:
  *                                     31        23        15         7       0
RP      EQUR    d5              * FLAGS |....:....|....:....|....:....|....:....|
  *                                      3GOR PICS 7RND FB.. 0000 0000 Amiga-CCR
  *                                      ^ stopcnt3
  *                                       ^*Kellgfx flag
  *                                        ^*CntIrq
  *                                         ^ raster_enable
  *                                          -^*iec? (1-ok)
  *                                            ^ oldirq
  *                                             ^ singleclk (=1)
  *                                              ^*sound_enable
  *                                               -^ 7.bit off=0
  *                                                 ^ actramrom (1-rom)
  *                                                  ^ Normal-cnt
  *                                                   ^*SID? (1-yes)
  *                                                     ^ render? (1-yes)
  *                                                      ^ border? (0-no)
  * (*) : stay when restore a snapshot
  * ( ) : replace when restore
  *
  * test CARRY:
  *     move.w  RSP,CCR
  *     bcs     .carry_set
  * test OVERFLOW:
  *     btst    #B_OVER,RA
  *     bne     .overflow_set
  * test BREAK:
  *     btst    #B_BRK,RA
  *     bne     .break_set
  * test DECIMAL:
  *     btst    #B_DEC,RA
  *     bne     .decimal_set
  * test INTERRUPT:
  *     btst    #B_IRQ,RA
  *     bne     .interrupt_set
  * test ZERO:
  *     move.w  RP,CCR
  *     beq     .zero_set
  * test NEGATIVE:
  *     move.w  RP,CCR
  *     bmi     .negative_set
  *------------------------------------------------------------------------------
RSP     EQUR    d6              * STACK POINTER (01xx....) ~(low word: ACCR-Carry)
CYC     EQUR    d7              * gfxclock (2...0cyc)
  *                                         ^stopcnt2

RAM     EQUR    a2              * RAM POINTER (FIX RAM!!)
HIRAM   EQUR    a3              * RAM/ROM POINTER
FETCH   EQUR    a4              * fetchtable-pointer
DOGFX   EQUR    a5              * dogfx()
RPC     EQUR    a6              * PC (amiga&c+4)

;--------------------------------------------------------------------------------
;XREFs                  NAME              TYPE
;--------------------------------------------------------------------------------
                xref    _opt_twoscr     * l
                xref    _myram          * *b
                xref    _hiram          * *b
                xref    _r_a            * b
                xref    _r_x            * b
                xref    _r_y            * b
                xref    _r_sp           * b
                xref    _r_pc           * w
                xref    _r_p            * b
                xref    _countpointer   * *int
                xref    _brcntpnt       * *int

                xref    _SysBase        * *

* TED
                xref    _colback        * b
                xref    _colbord        * b
                xref    _colbordlong    * l
                xref    _col1           * b
                xref    _col2           * b
                xref    _col3           * b
                xref    _clp1           * *b
                xref    _clp2           * *b
                xref    _clp3           * *b
                xref    _col            * col[]
                xref    _rasreq         * w
                xref    _ec1            * b
                xref    _ec2            * b
                xref    _ec3            * b
                xref    _colplace       * l
                xref    _textplace      * l
                xref    _actcharaddr    * l
                xref    _p4actras       * l
                xref    _actinchar      * l
                xref    _p4rom          * l
                xref    _backtable      * *b
                xref    _coltable       * *b

                xref    _vscroll        * l
                xref    _tbordminus     * l
                xref    _numraster      * l
                xref    _userblank      * b
                xref    _modeflag       * b
                xref    _modegfx        * *func[]
                xref    _dogfx          * *func

                xref    _opt_border     * b (ted.c) 0-nem kell border
                xref    _hscroll        * l
                xref    _leftborder     * l
                xref    _rightborder    * l
                xref    _cols           * l
                xref    _sideborders    * l
                xref    _offscr         * l

                xref    _curpos         * l
                xref    _cureor         * b

                xref    _gfxaddr        * w
                xref    _charmap        * *b
                xref    _keymatrixp     * *b
                xref    _actrom         * b
                xref    _romtab         * *b
                xref    _romsp0         * *b
                xref    _romsp1         * *b
                xref    _romsp2         * *b
                xref    _romsp3         * *b
                xref    _romsp4         * *b
                xref    _romsp5         * *b
                xref    _romsp6         * *b
                xref    _romsp7         * *b

                xref    _doframe        * func
                xref    _actraster      * l
                xref    _setchannel1    * func
                xref    _setchannel2    * func
                xref    _setch2quad     * func
                xref    _setch2noise    * func
                xref    _opt_nosound    * l (0-snd on)
                xref    _setbit7on      * func
                xref    _setbit7off     * func (d0==0 ch2 quad)
                xref    _setbit7        * func
                xref    _snd_7bit       * *b (/w) (4byte sample buffer)

                xref    _IECBase        * libbase
                xref    _IntuitionBase  * libbase
                xref    _p4req1         * func (p4req.c)
        xref    _cp4prefsmpmakesnapshot * func (preferencesgui.c)
        xref    _cp4prefsmploadsnapshot * func (preferencesgui.c)
                xdef    _CodeTab        * l[]


* scans.asm
                xref    ctabdouble      * l[]
                xref    ctabsingle      * l[]
                xref    ctabblankdouble * l[]
                xref    ctabblanksingle * l[]

* debug
                xref    _dbgmodeflag    * b
                xref    _dbgamiras      * l
                xref    _dbgrasreq      * w
                xref    _dbgvideo       * w
                xref    _dbggfxbase     * w
                xref    _dbgeirq        * b
                xref    _dbgp4ras       * w
                xref    _dodebug        * func
                xref    _amirasaddr     * l (ted.c)

* c2p
                xref    _c2pact         * *func
                xref    _c2pv           * *struct
* prefs
                xref    _prefsguis      * func
                xref    _ptwoframe      * l
                xref    _pswapjoy       * l
                xref    _plimit         * l
                xref    _pc2p           * l
                xref    _pscr           * l
* realtime
                xref    _skipflag       * bw
* SID
                xref    _opt_sid        * l
                xref    _opt_sidtype    * l
                xdef    _initsid        * func
                xdef    _endsid         * func
                xdef    _sidpause       * func
                xdef    _sidresume      * func
                xdef    _PlaySidBase    *
                xdef    _6581sidBase    *
* NoRender video
                xref    _c2pColor       * *b
                xref    _c2pVideo       * *b
                xref    _c2pGfx         * *b
                xref    _c2pModeflag    * b
                xref    _opt_render     * l     (0-nem renderel)
* for direct mode
                xref    _opt_direct     * l     cp4.c (0-nincs)
* for IEC emu
                xref    _opt_iec        * l     cp4.c (0-nincs)
                xref    _pin_PIEC_W     * func  pin_piec.c
                xref    _pin_PIEC_Reset * func  pin_piec.c
                xref    _rmode          * b     (1-noflip 0-normal)
                xref    _pin_IEC_W      * func  pin_iec.c
                xref    _pin_IEC_R      * func  pin_iec.c
                xref    _pin_IEC_Reset  * func  pin_iec.c
                xref    _ieccntflag     * b     pin_iec.c
        IFD     STAT_IEC
                xref    _iecmessage     * func (iectrace)

IM_CIOUT        EQU     1
IM_TALK         EQU     2
IM_LISTEN       EQU     3
IM_UNTALK       EQU     4
IM_UNLISTEN     EQU     5
IM_ACPTR        EQU     6
IM_SEC          EQU     7
IM_TKSA         EQU     8
        ENDIF

F_NEG   EQU     $8000
F_OVER  EQU     $4000
F__     EQU     $2000           ; always 1
F_BRK   EQU     $1000
F_DEC   EQU      $800
F_IRQ   EQU      $400
F_ZERO  EQU      $200
F_CARRY EQU      $100

B_NEG   EQU     15
B_OVER  EQU     14
B__     EQU     13
B_BRK   EQU     12
B_DEC   EQU     11
B_IRQ   EQU     10
B_ZERO  EQU     9
B_CARRY EQU     8

* SIDTYPE
SIDTYPE_NONE    EQU     0
SIDTYPE_PSID    EQU     1
SIDTYPE_6581    EQU     2

* 6581sid.library
_LVOSID_AllocSID        EQU     -30
_LVOSID_FreeSID         EQU     -36
_LVOSID_Interrupt       EQU     -42
_LVOSID_Initialize      EQU     -48
_LVOSID_ResetSID        EQU     -54
_LVOSID_IRQOnOff        EQU     -60
_LVOSID_ReadReg         EQU     -72
_LVOSID_WriteReg        EQU     -78

* SID Handle structure
sid_Enabled     EQU     4
sid_Filter      EQU     5
sid_60Hz        EQU     6
sid_RingQual    EQU     7
sid_SyncQual    EQU     8
sid_ADSRQual    EQU     13
sid_IRQRate     EQU     18

*
* modeflag az RA tetjén (on the top of RA)
*
MODEINIT        EQU     $10
M_MULTI         EQU     16
M_BITMAP        EQU     17
M_EXTCOL        EQU     18
M_INV           EQU     19
M_SCROFF        EQU     20
M_FLASH         EQU     21
M_UBLANK        EQU     22
* or-values (swapped!)
ORMULTI         EQU     1
ORBITMAP        EQU     2
OREXTCOL        EQU     4
ORINV           EQU     8
ORSCROFF        EQU     16
ORFLASH         EQU     32
ORUBLANK        EQU     64

* az RX tetején (top of RX)
STOPCNT1        EQU     31

* az CYC tetején (top of CYC)
STOPCNT2        EQU     31

*
* bitek az RP tetején (bits on the top of RP)
*
STOPCNT3        EQU     31
KELLGFX         EQU     30
CNTIRQ          EQU     29
ERAS            EQU     28
IEC             EQU     27
OLDIRQ          EQU     26
SNGCLK          EQU     25
NOSND           EQU     24
BIT7OFF         EQU     23
ACTRAMROM       EQU     22
NRMCNT          EQU     21
SKIPIT          EQU     20
SID             EQU     19
RENDER          EQU     18
BORDER          EQU     17
* or/and-elhetõ értékek: (or/and-able values)
M_STOPCNT3      EQU     $80000000
M_KELLGFX       EQU     $40000000
M_CNTIRQ        EQU     $20000000
M_ERAS          EQU     $10000000
M_IEC           EQU     $08000000
M_OLDIRQ        EQU     $04000000
M_SNGCLK        EQU     $02000000
M_NOSND         EQU     $01000000
M_BIT7OFF       EQU     $00800000
M_ACTRAMROM     EQU     $00400000
M_NRMCNT        EQU     $00200000
M_SKIPIT        EQU     $00100000
M_SID           EQU     $00080000
M_RENDER        EQU     $00040000
M_BORDER        EQU     $00020000

*
* konstansok
*
CLKFAST         EQU     109             ; doksiból (from doc)
* eye corrected
CLKSLOW         EQU     53              ; doksiból-korrigálva szemmel (60-58)

* time delay for iec-patch
T_TALK          EQU     150
T_LISTEN        EQU     140
T_UNTALK        EQU     150
T_UNLISTEN      EQU     150
T_OUT           EQU     110
T_IN            EQU     110
T_SECOND        EQU     90
T_SECTALK       EQU     90

* IEC_Types
IT_NONE         EQU     0
IT_IEC          EQU     1
IT_SOFT         EQU     2

* MAIN
                xdef    _asmexecute     *Func
                xdef    _StatTab        *statistic
* SNAPSHOT-FILE
                xdef    _sfrd2          *l
                xdef    _sfrd3          *l
                xdef    _sfrd4          *l
                xdef    _sfrd5          *l
                xdef    _sfrd6          *l
                xdef    _sfrd7          *l
                xdef    _sft1           *w
                xdef    _sfpc           *w
                xdef    _sfc1           *w
                xdef    _sfc2           *w
                xdef    _sfc3           *w
                xdef    _sfac           *w
                xdef    _sfc1c          *w
                xdef    _sfc2c          *w
                xdef    _sfc3c          *w
                xdef    _sfchr          *b
                xdef    _sfchm          *l

*-------------------------------------------------------------------------
* libcall macro
*-------------------------------------------------------------------------
LIBCALL         MACRO   ;FunctionName
                jsr     _LVO\1(a6)
                ENDM

*-------------------------------------------------------------------------
* libcall macro with base init
*-------------------------------------------------------------------------
LIBCALLB        MACRO   ;FunctionName,Base
                move.l  \2,a6
                jsr     _LVO\1(a6)
                ENDM

*-------------------------------------------------------------------------
* shadowcvars
*-------------------------------------------------------------------------
shadowcvars     MACRO
                move.l  _actraster,MY_actraster(a1)
                move.l  _actinchar,MY_actinchar(a1)
                move.l  _actcharaddr,MY_actcharaddr(a1)
* ez word!!!
*       move.l  _rasreq,MY_rasreq(a1)
                move.w  _rasreq,MY_rasreq(a1)
                move.l  _numraster,MY_numraster(a1)
                move.l  _charmap,MY_charmap(a1)
                move.l  _vscroll,MY_vscroll(a1)
                move.l  _colplace,MY_colplace(a1)
                move.l  _textplace,MY_textplace(a1)
                move.l  _curpos,MY_curpos(a1)
                move.l  _leftborder,MY_leftborder(a1)
                move.l  _backtable,MY_backtable(a1)
                move.b  _colback,MY_colback(a1)
                move.l  _coltable,MY_coltable(a1)
                move.w  _gfxaddr,MY_gfxaddr(a1)
                move.b  _col1,MY_col1(a1)
                move.b  _col2,MY_col2(a1)
                move.b  _col3,MY_col3(a1)
                move.b  _actrom,MY_actrom(a1)
                move.l  _hscroll,MY_hscroll(a1)
* ONLY_STORE
                move.l  _keymatrixp,MY_keymatrixp(a1)
                move.l  _p4rom,MY_p4rom(a1)
                bset    #BORDER,RP
                tst.b   _opt_border
                bne     \@1$
                bclr    #BORDER,RP
\@1$
                ENDM

*-------------------------------------------------------------------------
* restoresomevars       (work: d0.l)
*-------------------------------------------------------------------------
restoresomevars MACRO
                move.l  MY_charmap(a1),_charmap
                move.l  MY_curpos(a1),_curpos
                move.l  RAM,d0
                move.w  MY_colplace+2(a1),d0
                move.l  d0,_c2pColor
                move.w  MY_textplace+2(a1),d0
                move.l  d0,_c2pVideo
                move.w  MY_gfxaddr(a1),d0
                move.l  d0,_c2pGfx
                swap    RA
                move.b  RA,_c2pModeflag
                swap    RA
                ENDM

*-------------------------------------------------------------------------
* restorecvars
*-------------------------------------------------------------------------
restorecvars    MACRO
                move.l  MY_actraster(a1),_actraster
                move.l  MY_actinchar(a1),_actinchar
                move.l  MY_actcharaddr(a1),_actcharaddr
* Ez word!!!
*       move.l  MY_rasreq(a1),_rasreq
                move.w  MY_rasreq(a1),_rasreq
                move.l  MY_numraster(a1),_numraster
                move.l  MY_charmap(a1),_charmap
                move.l  MY_vscroll(a1),_vscroll
                move.l  MY_colplace(a1),_colplace
                move.l  MY_textplace(a1),_textplace
                move.l  MY_curpos(a1),_curpos
                move.l  MY_leftborder(a1),_leftborder
                move.l  MY_backtable(a1),_backtable
                move.b  MY_colback(a1),_colback
                move.l  MY_coltable(a1),_coltable
                move.w  MY_gfxaddr(a1),_gfxaddr
                move.b  MY_col1(a1),_col1
                move.b  MY_col2(a1),_col2
                move.b  MY_col3(a1),_col3
                move.b  MY_actrom(a1),_actrom
                move.l  MY_hscroll(a1),_hscroll
                swap    RA
                move.b  RA,_modeflag
                swap    RA
                ENDM

*-------------------------------------------------------------------------
* set full c2p
*-------------------------------------------------------------------------
fullc2p         MACRO
                move.l  _c2pv,a0
                move.l  C2PV_DOFULL(a0),_c2pact         ; offset in struct
                ENDM

*-------------------------------------------------------------------------
* set delta c2p
*-------------------------------------------------------------------------
deltac2p        MACRO
                move.l  _c2pv,a0
                move.l  C2PV_DO(a0),_c2pact             ; offset in struct
                ENDM

*-------------------------------------------------------------------------
* WORK: a0
* test IECBase & patch ROM
*-------------------------------------------------------------------------
testiec         MACRO
                move.l  a0,-(sp)
                bclr    #IEC,RP
                move.l  #ldazp,i_lda1
                move.l  #ldazp,i_lda2
                move.l  #stazp,i_sta1
                move.l  #stazp,i_sta2
                tst.l   _opt_iec
                beq     \@1$
                move.l  #ldazpT,i_lda1
                move.l  #ldazpT,i_lda2
                move.l  #stazpT,i_sta1
                move.l  #stazpT,i_sta2
                bset    #IEC,RP
\@1$            move.l  (sp)+,a0
                ENDM
*OLD version (rom patch)
** patch rom
*               move.l  _romsp0,a0
*               move.w  #$1200,$e153-$8000(a0)          ; talk
*               move.w  #$1201,$e156-$8000(a0)          ; listen
*               move.w  #$1202,$e177-$8000(a0)          ; out
*               move.w  #$1202,$e181-$8000(a0)          ; out
*               move.w  #$1203,$e1f7-$8000(a0)          ; second
*               move.w  #$1204,$e203-$8000(a0)          ; sectalk
*               move.w  #$1205,$e22f-$8000(a0)          ; untalk
*               move.w  #$1206,$e23d-$8000(a0)          ; unlisten
*               move.w  #$1207,$e252-$8000(a0)          ; in
*               move.l  _p4rom,a0
*               add.l   #$8000,a0
*               move.w  #$1200,$e153-$8000(a0)          ; talk
*               move.w  #$1201,$e156-$8000(a0)          ; listen
*               move.w  #$1202,$e177-$8000(a0)          ; out
*               move.w  #$1202,$e181-$8000(a0)          ; out
*               move.w  #$1203,$e1f7-$8000(a0)          ; second
*               move.w  #$1204,$e203-$8000(a0)          ; sectalk
*               move.w  #$1205,$e22f-$8000(a0)          ; untalk
*               move.w  #$1206,$e23d-$8000(a0)          ; unlisten
*               move.w  #$1207,$e252-$8000(a0)          ; in
*               bra     \@2$
** repatch rom
*\@1$           move.l  _romsp0,a0
*               move.w  #$0940,$e153-$8000(a0)          ; talk
*               move.w  #$0920,$e156-$8000(a0)          ; listen
*               move.w  #$7820,$e177-$8000(a0)          ; out
*               move.w  #$7820,$e181-$8000(a0)          ; out
*               move.w  #$8595,$e1f7-$8000(a0)          ; second
*               move.w  #$8595,$e203-$8000(a0)          ; sectalk
*               move.w  #$7820,$e22f-$8000(a0)          ; untalk
*               move.w  #$a93f,$e23d-$8000(a0)          ; unlisten
*               move.w  #$78a9,$e252-$8000(a0)          ; in
*               move.l  _p4rom,a0
*               add.l   #$8000,a0
*               move.w  #$0940,$e153-$8000(a0)          ; talk
*               move.w  #$0920,$e156-$8000(a0)          ; listen
*               move.w  #$7820,$e177-$8000(a0)          ; out
*               move.w  #$7820,$e181-$8000(a0)          ; out
*               move.w  #$8595,$e1f7-$8000(a0)          ; second
*               move.w  #$8595,$e203-$8000(a0)          ; sectalk
*               move.w  #$7820,$e22f-$8000(a0)          ; untalk
*               move.w  #$a93f,$e23d-$8000(a0)          ; unlisten
*               move.w  #$78a9,$e252-$8000(a0)          ; in
*\@2$
*               ENDM

*-------------------------------------------------------------------------
* WORK: d0.b
* nextst
*-------------------------------------------------------------------------
nextst          MACRO                   * már a köv op-ra mutat az RPC
                move.b  (RPC)+,d0       * [RPC points to the next opcode]
        IFD     STAT
                addq.l  #1,(STAB,d0.l*4)
; codeview
                move.w  RPC,d1
                lsr.w   #8,d1                   ; to 0-512
                addq.l  #1,(_CodeTab,d1.l*4)
        ENDIF
                jmp     ([FETCH,d0.l*4])
                ENDM

*-------------------------------------------------------------------------
* WORK:
* next  \1: a csökkentés mértéke (cycle) (0-kimarad) [decrement value]
*-------------------------------------------------------------------------
next            MACRO                   * már a köv op-ra mutat az RPC
        IFNE    \1                      * [RPC points to the next opcode]
        IFD     STAT
                addq.l  #\1,STAC
        ENDIF
                subq.w  #\1,CYC
                bmi     \@1$
        ENDIF
                nextst
        IFNE    \1
\@1$            bra     raster
        ENDIF
                ENDM

*-------------------------------------------------------------------------
* WORK:
* next2   elõre csökkentett CYC-t vár, leteszteli. [no decrement but tst]
*-------------------------------------------------------------------------
next2           MACRO                   * már a köv op-ra mutat az RPC
                tst.w   CYC             * [RPC points to the next opcode]
                bmi     \@1$
                nextst
\@1$            bra     raster
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* makerp - a saját STATUS-ból c+4 status-t készít [conv own ST 2 c+4 st]
*-------------------------------------------------------------------------
makerp          MACRO
                and.w   #~(F_CARRY+F_NEG+F_ZERO),RA
                move.w  RSP,CCR
                bcc     \@1$
                or.w    #F_CARRY,RA
\@1$            move.w  RP,CCR
                bpl     \@2$
                or.w    #F_NEG,RA
                move.w  RP,CCR
\@2$            bne     \@3$
                or.w    #F_ZERO,RA
\@3$
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* makeccr - c+4 státuszból a saját STATUS-ba [conv c+4 st 2 own ST]
*-------------------------------------------------------------------------
makeccr         MACRO
                or.w    #F__,RA                 * F__ = 1 ( + set N flag)
                move.w  CCR,RP                  * Z=0, mert beállítottuk az F__-t
                btst    #B_ZERO,RA                ; [Z=0, becouse we set F__]
                beq     \@1$
                or.b    #4,RP
\@1$            btst    #B_CARRY,RA
                sne     RSP                     * My C,X flag
                move.l  #fetchtable,FETCH
                btst    #B_DEC,RA
                beq     \@2$
                move.l  #fetchtabledec,FETCH
\@2$
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* memreadnt d1: addr.w --> \1: data.b (high byte ???)
*                          \2: hány ciklusos az ut. [cycle of statement]
* >8000: 26    <8000: 22
*-------------------------------------------------------------------------
memreadnt       MACRO
                bpl     \@1$                    ; 4/6
                cmp.w   #$fd00,d1               ; 4
                bhs     \@4$                    ; 4/6
                move.b  (HIRAM,d1.l),\1         ; 8
                bra     \@2$                    ; 6
\@4$            moveq   #\2,d0
                bsr     reg_read
        IFNC    "d0","\1"
                move.b  d0,\1
        ENDIF
                bra     \@2$
\@1$            move.b  (RAM,d1.l),\1           ; 8
\@2$
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* memreadf RPC: addr.w --> \1: data.b (high byte 00)
*-------------------------------------------------------------------------
memreadf        MACRO
                clr.l   \1
                move.b  (RPC)+,\1
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* memreadfb RPC: addr.w --> \1: data.b
*-------------------------------------------------------------------------
memreadfb       MACRO
                move.b  (RPC)+,\1
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* makepc d1: addr.w --> RPC amiga addr
*-------------------------------------------------------------------------
makepc          MACRO
                move.l  RAM,RPC
                tst.w   d1
                bpl     \@1$
                move.l  HIRAM,RPC
\@1$            add.l   d1,RPC
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* memreadword \1: addr.w --> \2: data.w
*-------------------------------------------------------------------------
memreadword     MACRO
                bpl     \@1$
                move.w  (HIRAM,\1.l),\2
                bra     \@2$
\@1$            move.w  (RAM,\1.l),\2
\@2$            rol.w   #8,\2
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* memreadwordf RPC: addr.w --> \1: data.w
*-------------------------------------------------------------------------
memreadwordf    MACRO
                move.w  (RPC)+,\1
                rol.w   #8,\1
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* memreadwordfnc RPC: addr.w --> \1: data.w
*-------------------------------------------------------------------------
memreadwordfnc  MACRO
                move.w  (RPC),\1
                rol.w   #8,\1
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* memreadwordzp \1: addr.w --> \2: data.w
*-------------------------------------------------------------------------
memreadwordzp   MACRO
                move.w  (RAM,\1.l),\2
                rol.w   #8,\2
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* memreadzp d1.w: addr --> \1: data.b
*-------------------------------------------------------------------------
memreadzp       MACRO
                move.b  (RAM,d1.l),\1
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* memreadzpt d1.w: addr --> \1: data.b  (with iec test)
*-------------------------------------------------------------------------
memreadzpt      MACRO
                cmp.b   #$02,d1
                bhs     \@1$
                bsr     iec_read
\@1$            move.b  (RAM,d1.l),\1
                ENDM

*-------------------------------------------------------------------------
* WORK: - (ha \2==0 -> nem d0 a \1!) (\3: cycle!)
* memwrite d1: addr.w --> \1: data.b
*-------------------------------------------------------------------------
memwrite        MACRO
                cmp.w   #$d400,d1
                bhs     \@1$
                move.b  \1,(RAM,d1.l)
                next \3
\@1$
        IFEQ    \2
                move.b  \1,d0
        ENDIF
        IFD     STAT
                addq.l  #\3,STAC
        ENDIF
                subq.w  #\3,CYC
                jmp     ([RAM,d1.l*4])
                ENDM

*-------------------------------------------------------------------------
* WORK: - (ha \2==0 -> nem d0 a \1!) (\3: cycle!)
* memwritezp d1: addr.w --> \1: data.b
*-------------------------------------------------------------------------
memwritezp      MACRO
                move.b  \1,(RAM,d1.l)
                next \3
                ENDM

*-------------------------------------------------------------------------
* WORK: - (ha \2==0 -> nem d0 a \1!) (\3: cycle!) (with iec test)
* memwritezpt d1: addr.w --> \1: data.b
*-------------------------------------------------------------------------
memwritezpt     MACRO
                move.b  \1,(RAM,d1.l)
                cmp.b   #$01,d1
                bls     \@9$
                next \3
\@9$
        IFEQ    \2
                move.b  \1,d0
        ENDIF
        IFD     STAT
                addq.l  #\3,STAC
        ENDIF
                subq.w  #\3,CYC
                bra     iec_set
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* push \1: data.b
*-------------------------------------------------------------------------
push            MACRO
                swap    RSP
                move.b  \1,(RAM,RSP.w)
                subq.b  #1,RSP
                swap    RSP
                ENDM

*-------------------------------------------------------------------------
* WORK: \1
* pop \1: data.b (output)
*-------------------------------------------------------------------------
pop             MACRO
                swap    RSP
                addq.b  #1,RSP
                move.b  (RAM,RSP.w),\1
                swap    RSP
                ENDM

*-------------------------------------------------------------------------
* WORK: \1      !!! utána újabb swap RSP kell !!!
* pop \1: data.b (output)
*-------------------------------------------------------------------------
popns           MACRO
                swap    RSP
                addq.b  #1,RSP
                move.b  (RAM,RSP.w),\1
                ENDM

*-------------------------------------------------------------------------
* WORK: d0
* push2 \1: data.w
*-------------------------------------------------------------------------
push2           MACRO
                rol.w   #8,\1
                swap    RSP
                subq.b  #2,RSP
                move.w  \1,1(RAM,RSP.w)
                swap    RSP
                ENDM

*-------------------------------------------------------------------------
* WORK: \1 (16bit)
* pop2 \1: data.w (output)
*-------------------------------------------------------------------------
pop2            MACRO
                swap    RSP
                move.w  1(RAM,RSP.w),\1
                rol.w   #8,\1
                addq.b  #2,RSP
                swap    RSP
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* pushrp - c+4 statust a verembe
*-------------------------------------------------------------------------
pushrp          MACRO
                makerp
                move.w  RA,d1
                lsr.w   #8,d1
                push d1
                ENDM

*-------------------------------------------------------------------------
* WORK: - push2 d1; pushrp
* pushstatus - menti a gép státuszát: d1-RPC + rp
*-------------------------------------------------------------------------
pushstatus      MACRO
                makerp
                move.w  RPC,d1
                rol.w   #8,d1
                swap    RSP
                subq.b  #3,RSP
                move.w  d1,2(RAM,RSP.w)
                move.w  RA,d1
                lsr.w   #8,d1
                move.b  d1,1(RAM,RSP.w)
                swap    RSP
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* poprp - verembõl c+4 statuszba
*-------------------------------------------------------------------------
poprp           MACRO
                move.b  RA,d0
                pop RA
                lsl.w   #8,RA
                move.b  d0,RA
                makeccr
                ENDM

*-------------------------------------------------------------------------
* WORK: 
* resetdogfx
*-------------------------------------------------------------------------
resetdogfx      MACRO
                move.b  #$80,_cureor
                move.l  #_modegfx,a0
                swap    RA
                clr.w   RA
                move.b  RA,_userblank
                and.b   #ORFLASH,RA
                or.b    #MODEINIT,RA
                tst.l   _opt_direct
                beq     \@1$
                or.b    #$80,RA
\@1$            move.l  (a0,RA.w*4),DOGFX
                swap    RA
                ENDM

*-------------------------------------------------------------------------
* WORK: 
* updatedogfxns
*-------------------------------------------------------------------------
updatedogfxns   MACRO
                move.l  #_modegfx,a0
                move.l  (a0,RA.w*4),DOGFX
                swap    RA
                ENDM

*-------------------------------------------------------------------------
* WORK: d1
* reset7501
*-------------------------------------------------------------------------
reset7501       MACRO
                bsr     sidreset
                bsr     iec_reset
                resetdogfx
                move.b  #$ff,keyread
                move.w  $fffc(HIRAM),d1
                rol.w   #8,d1
                makepc
                clr.l   d0
                clr.l   d1
                nextst
                ENDM

*-------------------------------------------------------------------------
* WORK: d1
* irqhted
*-------------------------------------------------------------------------
irqhted         MACRO
                btst    #B_IRQ,RA               ; SEI?
                bne     \@1$
                move.b  (RPC)+,d0               ; fetch opcode
                move.l  (FETCH,d0.l*4),a0       ; next statement's address
                move.l  FETCH,actfetch          ; save FetchTable
                move.l  #fetchtableirq,FETCH    ; set special fetchtable
                jmp     (a0)
\@1$            bset    #OLDIRQ,RP
                ENDM

*-------------------------------------------------------------------------
* WORK: Itt kezeljük az irq elõtt utolsóként végrehajtott utasítás
*       utáni teendõket
* irqhandle
*-------------------------------------------------------------------------
irqhandle       move.l  actfetch(pc),FETCH      ; restore FetchTable
                subq.l  #1,RPC
                and.l   #~(M_OLDIRQ+M_CNTIRQ),RP
                and.w   #~F_BRK,RA
                pushstatus                      ; RPC, + status save
                move.w  $fffe(HIRAM),d1
                rol.w   #8,d1
                makepc
                or.w    #F_IRQ,RA               ; SEI
                next 7
        cnop    0,4
actfetch        dc.l    0
        cnop    0,4
*-------------------------------------------------------------------------
* WORK: Az irq elõtt utolsóként végrehajtott utasítás: CRASH...
* irqcrash
*-------------------------------------------------------------------------
irqcrash        move.l  actfetch(pc),FETCH      ; restore FetchTable
                jmp     halt_crash

*-------------------------------------------------------------------------
* WORK: d1
* irqhbrk
*-------------------------------------------------------------------------
irqhbrk         MACRO
                addq.l  #1,RPC
                pushstatus
                move.w  $fffe(HIRAM),d1
                rol.w   #8,d1
                makepc
                or.w    #F_IRQ,RA
                next 7
                ENDM


*-------------------------------------------------------------------------
* WORK: d0
* breakpoint
*-------------------------------------------------------------------------
breakpoint      subq.l  #1,RPC          * mutasson az opcode-ra
                moveq   #1,d0
                jsr     debug
                lsr.l   #1,d0
                bcs     hardreset
                bne     .halt
                next 0
.halt           jmp     halt_ted

*-------------------------------------------------------------------------
* WORK: d0
* halt_ted
*-------------------------------------------------------------------------
halt_ted        ;sub.l  a0,a0
                ;move.l #1,d0
                ;move.l #.txt,a1
                ;jsr    _p4req1
                moveq   #0,d0           * retval
                rts
;.txt   dc.b    "vege",0
;       cnop    0,4

*-------------------------------------------------------------------------
* WORK: d0
* halt_crash
*-------------------------------------------------------------------------
halt_crash      subq.l  #1,RPC          * mutasson az opcode-ra
                moveq   #2,d0           * crash flag
                jsr     debug
                lsr.l   #1,d0
                bcs     hardreset
                bne     .halt
                next 0
.halt           jmp     halt_ted

*-------------------------------------------------------------------------
* WORK: d0
* halt_illegal
*-------------------------------------------------------------------------
halt_illegal    subq.l  #1,RPC
                moveq   #3,d0           * illegal flag
                jsr     debug
                lsr.l   #1,d0
                bcs     hardreset
                bne     .halt
                next 0
.halt           jmp     halt_ted

*-------------------------------------------------------------------------
* WORK: debug-ról van szó
* debug
*-------------------------------------------------------------------------
debug           movem.l d2-d7/a1-a6,-(sp)
                move.l  d0,-(sp)                * call code
                restorecvars
                bsr     makesnapshot
                bsr     syncted
* ted
                swap    RA
                move.b  RA,_dbgmodeflag
                swap    RA
                move.b  $ff0a(RAM),_dbgeirq
                move.w  MY_rasreq(a1),_dbgrasreq
                move.w  MY_gfxaddr(a1),_dbggfxbase
                move.b  $ff14(RAM),d0
                and.b   #$f8,d0
                lsl.w   #8,d0
                move.w  d0,_dbgvideo
                move.l  _amirasaddr,_dbgamiras
                swap    RY
                move.w  RY,_dbgp4ras
                swap    RY
* regiszterek
                move.l  HIRAM,_hiram
                move.b  RA,_r_a
                move.b  RX,_r_x
                move.b  RY,_r_y
                makerp
                rol.w   #8,RA
                move.b  RA,_r_p
                rol.w   #8,RA
                swap    RSP
                move.b  RSP,_r_sp
                swap    RSP
                move.w  RPC,_r_pc
                move.l  (sp)+,d0                * call code
                jsr     _dodebug
                movem.l (sp)+,d2-d7/a1-a6
                shadowcvars
                move.b  _r_a,RA
                move.b  _r_x,RX
                move.b  _r_y,RY
                swap    RSP
                move.b  _r_sp,RSP
                swap    RSP
                move.l  RPC,d1
                move.w  _r_pc,d1
                move.l  d1,RPC
                clr.l   d1
                bclr    #6,d0
                beq     .noprefs
                bsr     setprefs
.noprefs        rts

*-------------------------------------------------------------------------
* WORK: prefs-rõl van szó
* prefs
*-------------------------------------------------------------------------
prefs           movem.l d2-d7/a1-a6,-(sp)
                bsr     syncted
                bsr     makesnapshot
                restorecvars
                jsr     _prefsguis
                movem.l (sp)+,d2-d7/a1-a6
                shadowcvars
                bsr     setprefs
                rts

*-------------------------------------------------------------------------
* WORK: set NOSND bit
* setsndbit
*-------------------------------------------------------------------------
setsndbit       MACRO
                bclr    #NOSND,RP               ; nincs ted==0
                bclr    #SID,RP                 ; nincs sid==0
                tst.l   _opt_nosound            ; 1==nincs hang
                bne     \@1$
                tst.b   sidinited
                bne     \@2$
                bset    #NOSND,RP               ; ted
                bra     \@1$
\@2$            bset    #SID,RP                 ; sid
\@1$
                ENDM

*-------------------------------------------------------------------------
* WORK: setprefs (save ALL regs!)
* setprefs
*-------------------------------------------------------------------------
setprefs        movem.l d0/a0,-(sp)
*               move.l  _opt_drive08,d0
*               move.b  d0,Addrs+8
*               move.l  _opt_drive09,d0         ; 0-none 1-iec 2-soft
*               move.b  d0,Addrs+9
*               move.l  _opt_drive10,d0
*               move.b  d0,Addrs+10
*               move.l  _opt_drive11,d0
*               move.b  d0,Addrs+11

                swap    RA
                and.b   #~$80,RA
                tst.l   _opt_direct
                beq     .nodirect
                or.b    #$80,RA
.nodirect       updatedogfxns
;               jsr     iec_reset
                bset    #RENDER,RP              ; def: yes
                tst.l   _opt_render             ; 1-igen
                bne     .yep
                bclr    #RENDER,RP              ; nope
.yep            testiec
                setsndbit
                tst.l   _pscr
                beq     .noscr
                fullc2p                         ; use a0!
.noscr          movem.l (sp)+,d0/a0
                rts

*-------------------------------------------------------------------------
* WORK: 
* beállítja a TED regisztereket, hogy a megfelelõ értéket mutassák
* RAM-ot piszkál csak $ff00-$ff1f
* syncted
*-------------------------------------------------------------------------
syncted         movem.l d0-d1/a0,-(sp)
                moveq   #$1f,d1
.cik            move.l  d1,-(sp)
                jsr     ([tedreadtab,d1.l*4])
                move.l  (sp)+,d1
                move.b  d0,$ff00(RAM,d1.l)
                dbf     d1,.cik
                movem.l (sp)+,d0-d1/a0
                rts

*-------------------------------------------------------------------------
* WORK: 
* elkészít egy snapshot-file-t, ami alapján tárolható a belsõ állapot
* makesnapshot
*-------------------------------------------------------------------------
makesnapshot    move.l  d0,-(sp)
                move.l  d2,d0
                and.l   #~$00c00000,d0
                move.l  d0,_sfrd2
                move.l  d3,_sfrd3
                move.l  d4,_sfrd4
                move.l  d5,_sfrd5
                move.l  d6,_sfrd6
                move.l  d7,_sfrd7
                move.w  initcnt1,_sft1
                move.l  RPC,d0
                move.w  d0,_sfpc
                move.w  counter1,_sfc1
                move.w  counter2,_sfc2
                move.w  counter3,_sfc3
                move.w  actclk,_sfac
                move.w  cnt1actclk,_sfc1c
                move.w  cnt2actclk,_sfc2c
                move.w  cnt3actclk,_sfc3c
                move.b  charinram,_sfchr
                move.l  MY_charmap(a1),_sfchm
                move.l  (sp)+,d0
                rts

savesnapshot    movem.l d0-d7/a0-a6,-(sp)
                restorecvars
                bsr     syncted
                bsr     makesnapshot
                jsr _cp4prefsmpmakesnapshot
                movem.l (sp)+,d0-d7/a0-a6
                rts

loadsnapshot    restorecvars
                movem.l d1/a0-a1,-(sp)
                jsr _cp4prefsmploadsnapshot
                movem.l (sp)+,d1/a0-a1
                tst.l   d0
                beq     snapload
                nextst

*-------------------------------------------------------------------------
* WORK: rasterenként hívódik meg
* raster
*-------------------------------------------------------------------------
*-------------------------------------------------------------------------
* COUNTER special edition
Ccounters       tst.l   RX                      ; stopcnt1
                bmi     Ccnt2
                move.w  cnt1actclk,d0
                sub.w   d0,counter1
                bcs     Cc1over
Ccnt2           tst.l   CYC                     ; stopcnt2
                bmi     Ccnt3
                move.w  cnt2actclk,d0
                sub.w   d0,counter2
                bcs     Cc2over
Ccnt3           tst.l   RP                      ; stopcnt3
                bmi     Ccntend
                move.w  cnt3actclk,d0
                sub.w   d0,counter3
                bcs     Cc3over
Ccntend         bclr    #NRMCNT,RP              ; clear nrmcnt, only if
                move.l  RX,d0                   ; all counters normal
                or.l    CYC,d0                  ; (ie: not stopped)
                or.l    RP,d0
                bpl     .sskip
                bset    #NRMCNT,RP
.sskip          moveq   #57,d0
                move.w  d0,cnt1actclk
                move.w  d0,cnt2actclk
                move.w  d0,cnt3actclk
                bra     Ccntallend
*----------------------------------------------------------------------------
* COUNTER overflow
Cc1over         move.w  counter1,d0
                move.w  initcnt1,d1
                neg.w   d0
                cmp.w   d1,d0
                bmi     .c1od0kis
                divu.w  d1,d0
                swap    d0              ; get remainder
                sub.w   d0,d1
                bra     .c1otov
.c1od0kis       neg.w   d0
                add.w   d0,d1
.c1otov         move.w  d1,counter1
                or.b    #8,$2b00+$09(a1)                * ff09-3.bit
                tst.b   _ec1
                beq     Ccnt2
                bset    #CNTIRQ,RP
                bra     Ccnt2
* CNT2 overflow
Cc2over         or.b    #$10,$2b00+$09(a1)              * ff09-4.bit
                tst.b   _ec2
                beq     Ccnt3
                bset    #CNTIRQ,RP
                bra     Ccnt3
* CNT3 overflow
Cc3over         or.b    #$40,$2b00+$09(a1)              * ff09-6.bit
                tst.b   _ec3
                beq     Ccntend
                bset    #CNTIRQ,RP
                bra     Ccntend
*----------------------------------------------------------------------------
* RASTER belépési pont
*----------------------------------------------------------------------------
raster
*----------------------------------------------------------------------------
* TED COUNTERS
                btst    #NRMCNT,RP
                bne     Ccounters
                moveq   #57,d0
                sub.w   d0,counter1
                bcs     Cc1over
                sub.w   d0,counter2
                bcs     Cc2over
                sub.w   d0,counter3
                bcs     Cc3over
Ccntallend
*----------------------------------------------------------------------------
* TED RASTER teendõk in asm
                swap    RY                      ; get _p4actras
                move.l  MY_actraster(a1),d1
                cmp.w   #283,d1                 ; REALMAXRAS
                bpl     .skip                   ; ugrás, ha d1 > 282 (noamiga)
                btst    #KELLGFX,RP
                bne     .nemkellgfx
                btst    #SKIPIT,RP
                bne     .nemkellgfx
;*              btst    #RENDER,RP
;*              bne     .nemkellgfx
                jsr     (DOGFX)
.nemkellgfx     clr.l   d0
                clr.l   d1
                move.w  RY,d1
                subq.w  #3,d1
                cmp.w   #200,d1                 ; numraster
                bpl     .skip                   ; ugrás, ha d1 >= 200 (látható rész)
                move.b  $2b00+$1f(a1),d0        ; actinchar update
                move.b  d0,d1
                addq.b  #1,d0
                and.b   #~7,d1
                and.b   #7,d0
                or.b    d0,d1
                move.b  d1,$2b00+$1f(a1)
                move.l  d0,MY_actinchar(a1)
                bne     .skip
                move.l  MY_actcharaddr(a1),d1   ; itt egy char-sort lépünk
                add.w   #40,d1
                cmp.w   #1000,d1
                bmi     .noverch                ; ugrás, ha d1 < 1000
                clr.l   d1
.noverch        move.l  d1,MY_actcharaddr(a1)
                move.w  d1,$2b00+$1a(a1)
; -- Raszter növelés
.skip
                move.w  RY,d1
                move.l  clktab,a0
                move.l  (a0,d1.l*4),d0                  ; alsó: cyc.w, felsõ: next.w
                move.w  d0,actclk
                move.w  d0,CYC
                swap    d0
                move.w  d0,RY
                clr.l   d0
; -- Frame???
                move.l  MY_actraster(a1),d1
                addq.w  #1,d1
                cmp.w   #312,d1
                bpl     .frame                  ; ugrás, ha d1 > 311
.noframe        move.l  d1,MY_actraster(a1)
                move.w  RY,$2b00+$1c(a1)
                or.b    #$fe,$2b00+$1c(a1)
; raster IRQ???
                cmp.w   MY_rasreq(a1),RY
                beq     .rasterint
                swap    RY
.noint          btst    #CNTIRQ,RP
                bne     .counterint
                btst    #OLDIRQ,RP
                bne     .oldirq
                nextst                          ; no interrupt
.oldirq         irqhted
                nextst

.rasterint      swap    RY
                or.b    #2,$2b00+$09(a1)
                btst    #ERAS,RP
                beq     .noint
                or.b    #$80,$2b00+$09(a1)
                clr.l   d0
                irqhted
                bra     .noint
.counterint     or.b    #$80,$2b00+$09(a1)
                clr.l   d0
                irqhted
                nextst
* ITT C kód
.frame
                move.w  #274,RY                 ; nem biztos, hogy kell!!
                move.b  $2b00+$1f(a1),d0
                addq.b  #8,d0
                move.b  d0,$2b00+$1f(a1)
                and.b   #$78,d0
                bne     .nem15
                moveq   #~$80,d0                ; minden 15. frame
                and.b   d0,$2b00+$1f(a1)
                not.b   d0
                eor.b   d0,_cureor
                swap    RA
                eor.b   #$20,RA
                updatedogfxns
.nem15          bchg    #KELLGFX,RP
                beq     .kellgfx
                clr.l   d0
                bra     .noret
.kellgfx        ;restoresomevars                ; RENDER
                move.l  a1,-(sp)
                jsr     _doframe
                move.l  (sp)+,a1
                and.l   _opt_twoscr,RP
                bclr    #SKIPIT,RP
                tst.b   _skipflag
                beq     .noskip
                bset    #SKIPIT,RP
.noskip         tst.l   d0
                beq     .noret
                lsr.l   #1,d0
                bcs     .ublank                 ; B0
                swap    RY
.kelledebug     lsr.l   #1,d0
                bcs     .debug                  ; B1
                lsr.l   #1,d0
                bcc     .halt
* RESET REQ
                clr.l   MY_actraster(a1)
                clr.l   d1
                lsr.l   #1,d0
                bcs     hardreset
                reset7501
* EXIT
.halt           jmp     halt_ted
.noret          clr.l   d1
                move.l  d1,MY_actcharaddr(a1)
                move.l  d1,MY_actinchar(a1)
                bra     .noframe

.debug          lsr.b   #3,d0                   ; esetleg prefs?
                bcc     .megisdebug
                clr.l   d0                      ; prefs
                bsr     prefs
                bra     .prdbret
.megisdebug     lsr.b   #1,d0                   ; talán snapshot?
                bcc     .veguldebug
                lsr.b   #1,d0
                bcc     .ssstore                ; save snapshot
                bra     loadsnapshot
.ssstore        bsr     savesnapshot
                swap    RY
                clr.l   d0
                bra     .noret
.veguldebug     clr.l   d0                      ; user requested debug
                bsr     debug
.prdbret        swap    RA
                and.b   #~$40,RA
                updatedogfxns
                lsr.l   #1,d0
                bcs     hardreset
                lsr.l   #1,d0
                bcs     .quit
                lsr.l   #1,d0
                bcs     snapload
.nnr            swap    RY
                bra     .noret
.ublank         swap    RA
                eor.b   #$40,RA
                updatedogfxns
                clr.l   d0
                bra     .noret
.quit
;               sub.l   a0,a0
;               LIBCALLB DisplayBeep,_IntuitionBase
                bra     halt_ted

*----------------------------------------------------------------------------
* SNAPSHOT LOAD
*----------------------------------------------------------------------------
snapload
; restore some flag from RP (d5)
* 1==stay                 3GOR PICS 7RND .... 
*                         0110 1001 0001 0000 -> $6910 xxxx
*
                fullc2p
                and.l   #$69100000,d5
                move.l  d5,-(sp)
                move.l  _sfrd2,d2
                move.l  _sfrd3,d3
                move.l  _sfrd4,d4
                move.l  _sfrd5,d5
                and.l   #~$69100000,d5
                or.l    (sp)+,d5
                bclr    #CNTIRQ,d5              ; lehet, hogy nem kell
                move.l  _sfrd6,d6
                move.l  _sfrd7,d7
                move.w  _sft1,initcnt1
                move.l  RPC,d0
                move.w  _sfpc,d0
                move.l  d0,RPC
                move.w  _sfc1,counter1
                move.w  _sfc2,counter2
                move.w  _sfc3,counter3
                move.w  _sfac,actclk
                move.w  _sfc1c,cnt1actclk
                move.w  _sfc2c,cnt2actclk
                move.w  _sfc3c,cnt3actclk
                move.b  _sfchr,charinram
                move.l  _sfchm,_charmap
; reload all ted reg
;               moveq   #$1f,d0
;.sncik         move.l  d0,-(sp)
;               move.l  #$0000ff00,d1
;               add.b   d0,d1
;               clr.l   d0
;               move.b  (RAM,d1.l),d0
;               jsr     registerset     ; d1-ben a cím ahová írtak (érték a címen és d0-ban)
;               move.l  (sp)+,d0
;               dbf     d0,.sncik
* resync c vars
                shadowcvars
* Update GFX
                swap    RA
                and.b   #~$40,RA
                updatedogfxns
* ram/rom settings
                clr.l   d1
                btst    #ACTRAMROM,RP
                beq     .ram
                move.l  MY_p4rom(a1),HIRAM
                move.w  RPC,d1
                bpl     .tov1
                move.l  HIRAM,RPC
                add.l   d1,RPC
                bra     .tov
.ram            move.l  RAM,HIRAM
.tov1           move.w  RPC,d1
                move.l  RAM,RPC
                add.l   d1,RPC
* zárás
.tov            clr.l   d0
                clr.l   d1
                nextst


*----------------------------------------------------------------------------
* HARD RESET
*----------------------------------------------------------------------------
hardreset       jsr     iec_reset
                jsr     sidreset
                clr.l   d0
                clr.l   d1
                resetdogfx
                move.b  #$ff,keyread
                move.l  _p4rom,a0
                move.w  $fffc(a0),d1
                rol.w   #8,d1
                move.l  a0,RPC
                add.l   d1,RPC
                jmp     TED_3e

*-------------------------------------------------------------------------
* ADDRESSING MODE MACROS (RPC --> 1st op) (after: RPC --> last op+1)
*-------------------------------------------------------------------------

*-------------------------------------------------------------------------
* WORK: RPC --> d1.w, d0
* addrizpx (ZP,X)
*-------------------------------------------------------------------------
addrizpx        MACRO                   * -tst
                memreadfb d0
                add.b   RX,d0
                memreadwordzp d0,d1
                ENDM

*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w
* addrizpy (ZP),Y
*-------------------------------------------------------------------------
addrizpy        MACRO                   * -tst
                memreadfb d0
                memreadwordzp d0,d1
                add.w   RY,d1
                ENDM
*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w                  PRECISE TIMING
* addrizpyp (ZP),Y
*-------------------------------------------------------------------------
addrizpyp       MACRO
                memreadfb d0
                memreadwordzp d0,d1
        IFD     STAT
                move.w  d1,d0
        ENDIF
                add.w   RY,d1
        IFD     STAT
                add.b   RY,d0
                cmp.w   d0,d1
                beq     \@1$
                addq.l  #1,STAR
\@1$            clr.l   d0
                tst.w   d1
        ENDIF
                ENDM

*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w
* addrabs ABS
*-------------------------------------------------------------------------
addrabs         MACRO                   * -tst
                memreadwordf d1
                ENDM

*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w
* addrabsx ABS,X
*-------------------------------------------------------------------------
addrabsx        MACRO                   * -tst
                memreadwordf d1
                add.w   RX,d1
                ENDM
*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w                  PRECISE TIMING
* addrabsxp ABS,X
*-------------------------------------------------------------------------
addrabsxp       MACRO
                memreadwordf d1
        IFD     STAT
                move.w  d1,d0
        ENDIF
                add.w   RX,d1
        IFD     STAT
                add.b   RX,d0
                cmp.w   d0,d1
                beq     \@1$
                addq.l  #1,STAR
\@1$            clr.l   d0
                tst.w   d1
        ENDIF
                ENDM

*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w
* addrabsy ABS,Y
*-------------------------------------------------------------------------
addrabsy        MACRO                   * -tst
                memreadwordf d1
                add.w   RY,d1
                ENDM
*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w                  PRECISE TIMING
* addrabsyp ABS,Y
*-------------------------------------------------------------------------
addrabsyp       MACRO
                memreadwordf d1
        IFD     STAT
                move.w  d1,d0
        ENDIF
                add.w   RY,d1
        IFD     STAT
                add.b   RY,d0
                cmp.w   d0,d1
                beq     \@1$
                addq.l  #1,STAR
\@1$            clr.l   d0
                tst.w   d1
        ENDIF
                ENDM

*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w
* addrzp ZP
*-------------------------------------------------------------------------
addrzp          MACRO                   * !! +tst
                memreadf d1
                ENDM

*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w
* addrzpx ZP,X
*-------------------------------------------------------------------------
addrzpx         MACRO                   * !! +tst
                memreadf d1
                add.b   RX,d1
                ENDM

*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w
* addrzpy ZP,Y
*-------------------------------------------------------------------------
addrzpy         MACRO                   * !! +tst
                memreadf d1
                add.b   RY,d1
                ENDM

*-------------------------------------------------------------------------
* WORK: RPC.w --> \1.w          !! nem címet ad, hanem op-ot !!
* addrbyte #BYTE
*-------------------------------------------------------------------------
addrbyte        MACRO
                memreadfb \1
                ENDM

*-------------------------------------------------------------------------
* WORK: RPC.w --> d1.w
* addiabs (ABS)
*-------------------------------------------------------------------------
addriabs        MACRO
                memreadwordf d1
                memreadword d1,d1
                ENDM

*-------------------------------------------------------------------------
* WORK: d0,d1
* branch
*-------------------------------------------------------------------------
* nincs laphatárátlépés-vizsgálat, csak STAT esetén
branch          MACRO
                memreadfb d1                    * get op (rel)
        IFD     STAT
                move.w  RPC,d0
                add.b   d1,d0
        ENDIF
                ext.w   d1                      *  rel (elõjelesen)
                add.w   d1,RPC
        IFD     STAT
                cmp.w   RPC,d0
                beq     \@1$
                addq.l  #1,STAR
\@1$            clr.l   d0
        ENDIF
                ENDM

*-------------------------------------------------------------------------
*--------------------------- Register & IEC set --------------------------
*-------------------------------------------------------------------------

*-------------------------------------------------------------------------
* setclktabon (a0)
*-------------------------------------------------------------------------
setclktabon     MACRO
                btst    #SNGCLK,RP
                beq     \@1$
                move.l  #ctabsingle,a0
                bra     \@2$
\@1$            move.l  #ctabdouble,a0
\@2$            move.l  a0,clktab
                ENDM

*-------------------------------------------------------------------------
* setclktaboff (a0)
*-------------------------------------------------------------------------
setclktaboff    MACRO
                btst    #SNGCLK,RP
                beq     \@1$
                move.l  #ctabblanksingle,a0
                bra     \@2$
\@1$            move.l  #ctabblankdouble,a0
\@2$            move.l  a0,clktab
                ENDM


*-------------------------------------------------------------------------
* WORK:
* d1-ben a cím ahová írtak (érték a címen és d0-ban)
*-------------------------------------------------------------------------
registerset     cmp.w   #$feff,d1
                bls     otherio
* !! TEDWRITE !!
                and.w   #$ff,d1
                jmp     ([tedhandletab,d1.l*4])
*-------------------------------------------------------------------------
* WORK: other I/O
* d1-ben a cím ahová írtak, d0.b mit
*-------------------------------------------------------------------------
otherio         move.b  d0,(RAM,d1.l)           ; az írás elvégzése
                cmp.w   #$fddf,d1
                bhi     .pariec
                cmp.b   #$0f,d1
                bls     .acia
                cmp.b   #$1f,d1
                bls     .par
                cmp.b   #$2f,d1
                bls     .nochip
                cmp.b   #$3f,d1
                bls     .keyread
                cmp.b   #$cf,d1
                bls     .nochip
                cmp.b   #$df,d1
                bls     MemWrite_MAP
.nochip
.endreg         next2

*-------------------------------------------------------------------------
.pariec         cmp.w   #$febf,d1
                bls     .nochip
* PARIEC ignored yet
                move.b  #0,(RAM,d1.l)
                next2

*-------------------------------------------------------------------------
* ACIA - ignored yet
.acia           bra     .endreg

*-------------------------------------------------------------------------
* PAR - ignored yet
.par            bra     .endreg

*-------------------------------------------------------------------------
* KEYREAD
.keyread        move.b  d0,keyread
                next2

*-------------------------------------------------------------------------
* ROMMAP        (.rommap)
MemWrite_MAP    and.w   #$000f,d1
                cmp.b   MY_actrom(a1),d1
                beq     .end
                move.l  a1,-(sp)
                move.b  d1,MY_actrom(a1)
                move.l  _romtab,a0
                move.b  16(a0,d1.l),d0
                swap    d0
                move.b  (a0,d1.l),d0
                move.l  #romsp,a0
                move.l  (a0,d0.w*4),a1
                move.l  _p4rom,a0
                add.l   #$00008000,a0
                move.w  #4095,d1
.rommap_c1      move.l  (a1)+,(a0)+
                dbf     d1,.rommap_c1
                clr.w   d0
                swap    d0
                move.l  #romsp,a1
                move.l  (a1,d0.l*4),a1
                move.w  #3839,d1
.rommap_c2      move.l  (a1)+,(a0)+
                dbf     d1,.rommap_c2
                move.l  (sp)+,a1
.end            next2

*-------------------------------------------------------------------------
* írás a ram-ra (már megtörtént)
*-------------------------------------------------------------------------
TED_RAM         move.b  d0,(RAM,d1.l)
                next2
*-------------------------------------------------------------------------
* írás nemlétezõ regiszterbe
*-------------------------------------------------------------------------
TED_NOREG       move.b  d0,(RAM,d1.l)
                next2
*-------------------------------------------------------------------------
* CNT1LO
*-------------------------------------------------------------------------
TED_00
        IFD     STAT
                addq.l  #1,STATW00
        ENDIF
                move.b  d0,$2b00+$00(a1)
                bset    #STOPCNT1,RX
                move.w  $2b00+$00(a1),d1
                move.b  counter1,d1
                ror.w   #8,d1
                bne     .1wok
                addq.w  #1,d1
.1wok           move.w  d1,counter1
                bset    #NRMCNT,RP
                next2
*-------------------------------------------------------------------------
* CNT1HI
*-------------------------------------------------------------------------
TED_01
        IFD     STAT
                addq.l  #1,STATW01
        ENDIF
                move.b  d0,$2b00+$01(a1)
                bclr    #STOPCNT1,RX
                move.w  $2b00+$00(a1),d1
                ror.w   #8,d1
                bne     .1wok
                addq.w  #1,d1
.1wok           move.w  d1,counter1
                move.w  d1,initcnt1
                move.w  CYC,cnt1actclk
                bset    #NRMCNT,RP
                next2
*-------------------------------------------------------------------------
* CNT2LO
*-------------------------------------------------------------------------
TED_02
        IFD     STAT
                addq.l  #1,STATW02
        ENDIF
                move.b  d0,$2b00+$02(a1)
                bset    #STOPCNT2,CYC
                move.w  $2b00+$02(a1),d1
                move.b  counter2,d1
                ror.w   #8,d1
                move.w  d1,counter2
                bset    #NRMCNT,RP
                next2
*-------------------------------------------------------------------------
* CNT2HI
*-------------------------------------------------------------------------
TED_03
        IFD     STAT
                addq.l  #1,STATW03
        ENDIF
                move.b  d0,$2b00+$03(a1)
                bclr    #STOPCNT2,CYC
                move.w  $2b00+$02(a1),d1
                ror.w   #8,d1
                move.w  d1,counter2
                move.w  CYC,cnt2actclk
                bset    #NRMCNT,RP
                next2
*-------------------------------------------------------------------------
* CNT3LO
*-------------------------------------------------------------------------
TED_04
        IFD     STAT
                addq.l  #1,STATW04
        ENDIF
                move.b  d0,$2b00+$04(a1)
                bset    #STOPCNT3,RP
                move.w  $2b00+$04(a1),d1
                move.b  counter3,d1
                ror.w   #8,d1
                move.w  d1,counter3
                bset    #NRMCNT,RP
                next2
*-------------------------------------------------------------------------
* CNT3HI
*-------------------------------------------------------------------------
TED_05
        IFD     STAT
                addq.l  #1,STATW05
        ENDIF
                move.b  d0,$2b00+$05(a1)
                bclr    #STOPCNT3,RP
                move.w  $2b00+$04(a1),d1
                ror.w   #8,d1
                move.w  d1,counter3
                move.w  CYC,cnt3actclk
                bset    #NRMCNT,RP
                next2
*-------------------------------------------------------------------------
* GFX1
*-------------------------------------------------------------------------
TED_06
        IFD     STAT
                addq.l  #1,STATW06
        ENDIF
                move.b  d0,$2b00+$06(a1)
* vscroll=3-(wrt&7);
                move.l  d0,d1
                and.b   #7,d0
* GEGA-NEW, DE NEM JÓ
                move.l  vscrolltab(pc,d0.l*4),d0
*RÉGI, NEM JÓ
*               sub.l   #3,d0
*               neg.l   d0
*RÉGI
                move.l  d0,MY_vscroll(a1)

* rows=wrt&B3;
                move.l  d1,d0
                and.b   #8,d0
* if(rows==0)
                bne     .else
                moveq   #4,d0
                move.l  d0,_tbordminus
                move.l  #192,MY_numraster(a1)
                bra     .endif
.else           clr.l   _tbordminus
                move.l  #200,MY_numraster(a1)
.endif
* modeflag&=~0x16;
                swap    RA                      ; get modeflag
                and.b   #~$16,RA
*       bitmap=wrt&B5;
*       modeflag|=bitmap>>4;
                move.l  d1,d0
                and.b   #$20,d0
                lsr.b   #4,d0
                or.b    d0,RA
*       extcol=wrt&B6;
*       modeflag|=extcol>>4;
                move.l  d1,d0
                and.b   #$40,d0
                lsr.b   #4,d0
                or.b    d0,RA
*       offscr=wrt&B4;
*       modeflag|=offscr;
                move.l  d1,d0
                and.b   #$10,d0
                or.b    d0,RA
                tst.b   d0
                bne     .on
                setclktaboff
                bra     .eon
.on             setclktabon
.eon
*       dogfx=modegfx[modeflag];
                updatedogfxns
*       hiram[0xff06]&=~B7;
                and.b   #~$80,$2b00+$06(a1)
                next2

*GEGA-NEW, DE NEM JÓ
*vscrolltab     dc.l    4,3,2, 1,0,-1,6,5
*vscrolltab     dc.l    3,2,1, 0,-1,-2,5,4      ; jó a terror news-hoz
vscrolltab      dc.l    3,2,1,0,-1,-2,-3,-4     ; calculated

*-------------------------------------------------------------------------
* GFX2
*-------------------------------------------------------------------------
TED_07
        IFD     STAT
                addq.l  #1,STATW07
        ENDIF
                move.b  d0,$2b00+$07(a1)
*!!!!
*       tedoff=wrt&B5; // ignored...
*       6. bit PAL/NTSC - ignored...
*!!!!
*       hscroll=wrt&7;
                move.l  d0,d1
                and.b   #7,d1
                move.l  d1,MY_hscroll(a1)
*       leftborder=16+hscroll;
*       rightborder=16-hscroll;                         !! csak akkor, ha opt_border!=0
                btst    #BORDER,RP
                beq     .skipbor
                add.b   #16,d1
.skipbor        move.l  d1,MY_leftborder(a1)
                moveq   #16,d1
                sub.l   MY_hscroll(a1),d1
                move.l  d1,_rightborder
*       cols=wrt&B3;
                move.l  d0,d1
                and.b   #8,d1
*NOTINUSE       move.l  d1,_cols
*       if(cols==0) sideborders=24/4;
*       else sideborders=16/4;
                bne     .else
                moveq   #1,d1                   ; #6 volt
                bra     .tov
.else           moveq   #0,d1                   ; #4 volt
.tov            move.l  d1,_sideborders
*       modeflag&=~0x09;
                swap    RA                      ; get modeflag
                and.b   #~$09,RA
*       multicol=wrt&B4;
                move.l  d0,d1
                and.b   #$10,d1
*       modeflag|=multicol>>4;          // set multi
                lsr.b   #4,d1
                or.b    d1,RA
*       einv=wrt&0x80;
*       modeflag|=einv>>4;              // set inv
                move.l  d0,d1
                and.b   #$80,d1
                lsr.b   #4,d1
                or.b    d1,RA
                updatedogfxns
                next2


*-------------------------------------------------------------------------
* KEYREAD       (d7 felsõ fele dirty!)
*-------------------------------------------------------------------------
TED_08
        IFD     STAT
                addq.l  #1,STATW08
        ENDIF
                move.b  d0,.joyreq
                move.l  MY_keymatrixp(a1),a0
                move.l  d7,-(sp)
                st      d0
                moveq   #7,d7
                move.b  keyread(pc),d1
.cik            lsr.b   #1,d1
                bcs     .nemkell
                and.b   (a0),d0
.nemkell        addq.l  #1,a0
                dbf     d7,.cik
                move.l  (sp)+,d7
*JOY
                move.b  .joyreq(pc),d1
                lsr.b   #2,d1
                bcs     .s1
                and.b   (a0),d0
.s1             addq.l  #1,a0
                lsr.b   #1,d1
                bcs     .s2
                and.b   (a0),d0
.s2
*JOY
                move.b  d0,$2b00+$08(a1)
                next2

.joyreq         dc.b    0
keyread         dc.b    $ff
        cnop    0,4

*-------------------------------------------------------------------------
* IRQFLAG
*-------------------------------------------------------------------------
TED_09
        IFD     STAT
                addq.l  #1,STATW09
        ENDIF
*               move.b  d0,$2b00+$09(a1)
*               bclr    #OLDIRQ,RP      ; minus4 szerint...
*               not.b   d0
*               move.b  sFF09,d1
*               and.b   d0,d1
*               or.b    #$25,d1
*               move.b  d1,$2b00+$09(a1)

                bclr    #OLDIRQ,RP
*               clr.b   $2b00+$09(a1)           ; ??? mit kell itt csinálni
                not.b   d0
                and.b   d0,$2b00+$09(a1)
                or.b    #$25,$2b00+$09(a1)      ; set 0,2,5 bit
                next2

*-------------------------------------------------------------------------
* IRQENABLE
*-------------------------------------------------------------------------
TED_0a
        IFD     STAT
                addq.l  #1,STATW0A
        ENDIF
                move.b  d0,$2b00+$0a(a1)
                bset    #ERAS,RP
                btst    #1,d0
                bne     .erasskip
                bclr    #ERAS,RP
.erasskip       btst    #3,d0
                sne     d1
                move.b  d1,_ec1
                btst    #4,d0
                sne     d1
                move.b  d1,_ec2
                btst    #6,d0
                sne     d1
                move.b  d1,_ec3
                move.w  $2b00+$0a(a1),d1
                and.w   #$1ff,d1
                move.w  d1,MY_rasreq(a1)
                next2
*-------------------------------------------------------------------------
* RASTER
*-------------------------------------------------------------------------
TED_0b
        IFD     STAT
                addq.l  #1,STATW0B
        ENDIF
                move.b  d0,$2b00+$0b(a1)
                move.w  $2b00+$0a(a1),d1
                and.w   #$1ff,d1
                move.w  d1,MY_rasreq(a1)
                next2
*-------------------------------------------------------------------------
* CURPOS1
*-------------------------------------------------------------------------
TED_0c
        IFD     STAT
                addq.l  #1,STATW0C
        ENDIF
                or.b    #~3,d0
                move.b  d0,$2b00+$0c(a1)
                move.w  $2b00+$0c(a1),d1
                and.w   #$3ff,d1
                move.l  d1,MY_curpos(a1)
                next2
*-------------------------------------------------------------------------
* CURPOS2
*-------------------------------------------------------------------------
TED_0d
        IFD     STAT
                addq.l  #1,STATW0D
        ENDIF
                move.b  d0,$2b00+$0d(a1)
                move.w  $2b00+$0c(a1),d1
                and.w   #$3ff,d1
                move.l  d1,MY_curpos(a1)
                next2
*-------------------------------------------------------------------------
* SND1FREQ1
*-------------------------------------------------------------------------
TED_0e
        IFD     STAT
                addq.l  #1,STATW0E
        ENDIF
                move.b  d0,$2b00+$0e(a1)
                btst    #NOSND,RP
                beq     .nos
                btst    #BIT7OFF,RP
                bne     .end
                move.b  $2b00+$12(a1),d1
                and.b   #3,d1
                lsl.w   #8,d1
                move.b  d0,d1
                cmp.w   actfreq1(pc),d1
                beq     .end
                move.w  d1,actfreq1
                move.b  enable1,d0
                and.b   actvol,d0
                beq     .end
                move.b  actvol,d0
                lsl.b   #3,d0
                move.l  a1,-(sp)
                jsr     _setchannel1
                move.l  (sp)+,a1
                clr.l   d1
.end            clr.l   d0
.nos            next2

*-------------------------------------------------------------------------
* SND2FREQ1
*-------------------------------------------------------------------------
TED_0f
        IFD     STAT
                addq.l  #1,STATW0F
        ENDIF
                move.b  d0,$2b00+$0f(a1)
                btst    #NOSND,RP
                beq     .nos
                btst    #BIT7OFF,RP
                bne     .end
                move.b  $2b00+$10(a1),d1
                and.b   #3,d1
                lsl.w   #8,d1
                move.b  d0,d1
                cmp.w   actfreq2(pc),d1
                beq     .end
                move.w  d1,actfreq2
                move.b  enable2,d0
                and.b   actvol,d0
                beq     .end
                move.b  actvol,d0
                lsl.b   #3,d0
                move.l  a1,-(sp)
                jsr     _setchannel2
                move.l  (sp)+,a1
                clr.l   d1
.end            clr.l   d0
.nos            next2

*-------------------------------------------------------------------------
* SND2FREQ2
*-------------------------------------------------------------------------
TED_10
        IFD     STAT
                addq.l  #1,STATW10
        ENDIF
                move.b  d0,$2b00+$10(a1)
                btst    #NOSND,RP
                beq     .nos
                btst    #BIT7OFF,RP
                bne     .end
                and.b   #3,d0
                lsl.w   #8,d0
                move.b  $2b00+$0f(a1),d0
                cmp.w   actfreq2(pc),d0
                beq     .end
                move.w  d0,actfreq2
                move.w  d0,d1
                move.b  enable2,d0
                and.b   actvol,d0
                beq     .end
                move.b  actvol,d0
                lsl.b   #3,d0
                move.l  a1,-(sp)
                jsr     _setchannel2
                move.l  (sp)+,a1
                clr.l   d1
.end            clr.l   d0
.nos            next2


*-------------------------------------------------------------------------
        cnop    0,4
actfreq1        dc.w    0
actfreq2        dc.w    0
        cnop    0,4
actvol          dc.b    0       ; ami a regiszterben
enable1         dc.b    0       ; 0-letiltva ff-eng
enable2         dc.b    0       ; 0-letiltva ff-eng (ha zaj-??)
noise           dc.b    0       ; 0-letiltva ff-eng
nownoise        dc.b    0       ; 0-most négyszögjel, ff-zaj szól a ch2-n
        cnop    0,4
eorvolume       dc.b    0
eorenable1      dc.b    0
eorenable2      dc.b    0
eornoise        dc.b    0
        cnop    0,4
*-------------------------------------------------------------------------


*-------------------------------------------------------------------------
* SNDCTRL
*-------------------------------------------------------------------------
TED_11
        IFD     STAT
                addq.l  #1,STATW11
        ENDIF
                move.b  d0,$2b00+$11(a1)
                btst    #NOSND,RP
                beq     .nos
                move.l  a1,-(sp)
                move.b  d0,$2b00+$11(a1)
                bmi     .bit7
                move.l  actvol,eorvolume        ; copy all eor values
                move.b  d0,d1
                and.b   #$0f,d1
                swap    d1
                add.b   d0,d0
                smi     d1
                move.b  d1,noise
                eor.b   d1,eornoise
                add.b   d0,d0
                smi     d1
                move.b  d1,enable2
                eor.b   d1,eorenable2
                add.b   d0,d0
                smi     d1
                move.b  d1,enable1
                eor.b   d1,eorenable1
* volume ctrl
                swap    d1
                cmp.b   #$09,d1
                bmi     .volok
                moveq   #8,d1
.volok          move.b  d1,actvol
                eor.b   d1,eorvolume
* TEST OLD BIT7
                bclr    #BIT7OFF,RP
                beq     .bit7skip
                jsr     _setbit7off
                clr.l   d0
                clr.l   d1
                not.l   d1
                move.l  d1,eorvolume            ; minden módosul!
                clr.l   d1
.bit7skip
* TEST CHANNEL #1
                move.b  eorenable1,d0
                or.b    eorvolume,d0
                beq     .channel2
        ; változás kell a CH#1-en
                tst.b   actvol
                beq     .quiet1
                tst.b   enable1
                beq     .quiet1q
        ; hang a CH#1-en
                move.w  actfreq1(pc),d1
                move.b  actvol,d0
                lsl.b   #3,d0
                jsr     _setchannel1
                clr.l   d0
                bra     .channel2
        ; CH#1 kikapcs
.quiet1q        tst.b   eorenable1
                beq     .channel2
.quiet1         clr.l   d1
                clr.l   d0
                jsr     _setchannel1
                clr.l   d0
* TEST CHANNEL #2
.channel2       move.b  noise,d0
                and.b   enable2,d0              ; ha egyszerre akar zajt és
                bne     .quiet2                 ; négyszöget -> csendet kap
                move.b  eorenable2,d0
                or.b    eorvolume,d0
                beq     .noise2
        ; változás kell a CH#2-en
                tst.b   actvol
                beq     .quiet2
                tst.b   enable2
                beq     .quietquad
        ; hang a CH#2-en
                tst.b   nownoise
                beq     .oke
        ; eddig zaj volt, át kell kapcsolni
                jsr     _setch2quad
                clr.b   nownoise
.oke            move.w  actfreq2(pc),d1
                move.b  actvol,d0
                lsl.b   #3,d0
                jsr     _setchannel2
                clr.l   d0
                bra     .end
        ; CH#2 kikapcs
.quietnoise     tst.b   eornoise
                beq     .end
                bra     .quiet2
.quietquad      tst.b   eorenable2
                beq     .noise2
.quiet2         clr.l   d1
                clr.l   d0
                jsr     _setchannel2
                clr.l   d0
                bra     .end
* CHANNEL #2 ZAJ
.noise2         move.b  eornoise,d0
                or.b    eorvolume,d0
                beq     .end
        ; változás kell a CH#2-en
                tst.b   actvol
                beq     .quiet2
                tst.b   noise
                beq     .quietnoise
        ; hang a CH#2-en
                tst.b   nownoise
                bne     .oke2
        ; eddig négyszög volt, át kell kapcsolni
                jsr     _setch2noise
                st      nownoise
.oke2           move.w  actfreq2(pc),d1
                move.b  actvol,d0
                lsl.b   #3,d0
                jsr     _setchannel2
                clr.l   d0
* vege
.end            clr.l   d1
                move.l  (sp)+,a1
.nos            next2

* - - - - - - - - - - - - - -  -  - - -  -
* BIT-7 HANDLE
.bit7           bset    #BIT7OFF,RP
                bne     .bit7mareng
                jsr     _setbit7on
                clr.l   d0
                move.b  $ff11(RAM),d0
.bit7mareng     and.b   #$0f,d0
                cmp.b   #$09,d0
                bmi     .bit7volok
                moveq   #8,d0
.bit7volok      clr.l   d1
                move.w  .bufcnt(pc),d1
                move.l  #.sndbuf,a0
                move.b  bit7table(pc,d0.l),(a0,d1.l)
                addq.w  #1,d1
                cmp.w   #512,d1                 ; BIT7_BUFLEN
                bne     .tov
                jsr     _setbit7
                clr.l   d0
                clr.l   d1
.tov            move.w  d1,.bufcnt
                move.l  (sp)+,a1
                next2

        cnop    0,4
.bufcnt         dc.w    0
        cnop    0,4
.sndbuf         dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
                dc.b    0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
        cnop    0,4
;bit7table      dc.b    $80,$a0,$c0,$e0,$00,$20,$40,$60,$7f
bit7table       dc.b    $80,$a4,$c8,$ed,$11,$36,$5a,$7f,$00
        cnop    0,4

*-------------------------------------------------------------------------
* GFX3
*-------------------------------------------------------------------------
TED_12
        IFD     STAT
                addq.l  #1,STATW12
        ENDIF
                move.b  d0,$2b00+$12(a1)
                move.l  d0,d1
* gfxaddr
                and.b   #$38,d1
                lsl.w   #8,d1
                lsl.w   #2,d1
                move.w  d1,MY_gfxaddr(a1)
* charinram
                lsr.b   #2,d0
                and.b   #1,d0
                move.b  $2b00+$13(a1),d1
                and.b   #$fc,d1
                move.b  d0,charinram
                beq     .rammap
.rommap         move.l  RAM,a0
                lsl.w   #8,d1
                bpl     .inram
                move.l  MY_p4rom(a1),a0
.inram          add.l   d1,a0
                move.l  a0,MY_charmap(a1)
                bra     .end
.rammap         lsl.w   #8,d1
                move.l  RAM,a0
                add.l   d1,a0
                move.l  a0,MY_charmap(a1)
.end            btst    #NOSND,RP
                beq     .sndend
* SND1FREQ2
                btst    #BIT7OFF,RP
                bne     .sndend
                move.b  $2b00+$12(a1),d1
                and.b   #3,d1
                lsl.w   #8,d1
                move.b  $2b00+$0e(a1),d1
                cmp.w   actfreq1(pc),d1
                beq     .sndend
                move.w  d1,actfreq1
                move.b  enable1,d0
                and.b   actvol,d0
                beq     .sndend
                move.b  actvol,d0
                lsl.b   #3,d0
                move.l  a1,-(sp)
                jsr     _setchannel1
                move.l  (sp)+,a1
                clr.l   d1
.sndend         clr.l   d0
                next2

charinram       dc.b    1
        cnop    0,4

*-------------------------------------------------------------------------
* CHARGEN
*-------------------------------------------------------------------------
TED_13
        IFD     STAT
                addq.l  #1,STATW13
        ENDIF
                move.b  d0,$2b00+$13(a1)
* singleclock ((d0>>1)&1)
                bclr    #SNGCLK,RP
                move.b  d0,d1
                and.b   #2,d1
                beq     .dblclk
                bset    #SNGCLK,RP
.dblclk         btst    #M_SCROFF,RA
                bne     .on
                setclktaboff
                bra     .eon
.on             setclktabon
.eon
* check ram/rom
                or.b    #1,d0
                cmp.l   RAM,HIRAM
                bne     .chrmap
                and.b   #~1,d0
.chrmap         move.b  d0,$2b00+$13(a1)
                and.b   #$fc,d0
                tst.b   charinram
                beq     .rammap
.rommap         move.l  RAM,a0
                lsl.w   #8,d0
                bpl     .inram
                move.l  MY_p4rom(a1),a0
.inram          add.l   d0,a0
                move.l  a0,MY_charmap(a1)
                bra     .end
.rammap         lsl.w   #8,d0
                move.l  RAM,a0
                add.l   d0,a0
                move.l  a0,MY_charmap(a1)
.end            clr.l   d0
                next2
*-------------------------------------------------------------------------
* VIDEO-MATRIX
*-------------------------------------------------------------------------
TED_14
        IFD     STAT
                addq.l  #1,STATW14
        ENDIF
                move.b  d0,$2b00+$14(a1)
                and.b   #$f8,d0
                lsl.w   #8,d0
                move.l  d0,MY_colplace(a1)
                add.w   #$400,d0
                move.l  d0,MY_textplace(a1)
                clr.l   d0
                next2
*-------------------------------------------------------------------------
* COL0
*-------------------------------------------------------------------------
TED_15
        IFD     STAT
                addq.l  #1,STATW15
        ENDIF
                move.b  d0,$2b00+$15(a1)
                move.b  d0,MY_colback(a1)
*NOTINUSE       move.b  d0,_col         * col[0]
                next2
*-------------------------------------------------------------------------
* COL1
*-------------------------------------------------------------------------
TED_16
        IFD     STAT
                addq.l  #1,STATW16
        ENDIF
                move.b  d0,$2b00+$16(a1)
                move.b  d0,MY_col1(a1)
*NOTINUSE       move.l  _clp1,a0
*               move.b  d0,(a0)         * col[1]
                next2
*-------------------------------------------------------------------------
* COL2
*-------------------------------------------------------------------------
TED_17
        IFD     STAT
                addq.l  #1,STATW17
        ENDIF
                move.b  d0,$2b00+$17(a1)
                move.b  d0,MY_col2(a1)
*NOTINUSE       move.l  _clp2,a0
*               move.b  d0,(a0)         * col[2]
                next2
*-------------------------------------------------------------------------
* COL3
*-------------------------------------------------------------------------
TED_18
        IFD     STAT
                addq.l  #1,STATW18
        ENDIF
                move.b  d0,$2b00+$18(a1)
                move.b  d0,MY_col3(a1)
*NOTINUSE       move.l  #_clp3,a0
*               move.b  d0,(a0)         * col[3]
                next2
*-------------------------------------------------------------------------
* COL4 (BORDER)
*-------------------------------------------------------------------------
TED_19
        IFD     STAT
                addq.l  #1,STATW19
        ENDIF
                move.b  d0,$2b00+$19(a1)
*NOTINUSE       move.b  d0,_colbord
                move.b  d0,d1
                lsl.w   #8,d0
                move.b  d1,d0
                move.w  d0,d1
                swap    d0
                move.w  d1,d0
                move.l  d0,_colbordlong
                clr.l   d0
                next2
*-------------------------------------------------------------------------
* FIRSTCHARADDR1 (readonly yet)
*-------------------------------------------------------------------------
TED_1a
        IFD     STAT
                addq.l  #1,STATW1A
        ENDIF
                or.b    #~3,d0
                move.b  d0,$2b00+$1a(a1)
                move.w  $2b00+$1a(a1),d1
                and.w   #$3ff,d1
                move.l  d1,MY_actcharaddr(a1)
                next2
*-------------------------------------------------------------------------
* FIRSTCHARADDR2 (readonly yet)
*-------------------------------------------------------------------------
TED_1b
        IFD     STAT
                addq.l  #1,STATW1B
        ENDIF
                move.b  d0,$2b00+$1b(a1)
                move.w  $2b00+$1a(a1),d1
                and.w   #$2ff,d1
                move.l  d1,MY_actcharaddr(a1)
                next2
*-------------------------------------------------------------------------
* ACTRASTER1
*-------------------------------------------------------------------------
TED_1c
        IFD     STAT
                addq.l  #1,STATW1C
        ENDIF
                move.b  d0,$2b00+$1c(a1)
                or.b    #$fe,d0
                move.b  d0,$2b00+$1c(a1)
                move.w  $2b00+$1c(a1),d1
                and.w   #$1ff,d1
                swap    RY
                move.w  d1,RY
                swap    RY
                next2
*-------------------------------------------------------------------------
* ACTRASTER2
*-------------------------------------------------------------------------
TED_1d
        IFD     STAT
                addq.l  #1,STATW1D
        ENDIF
                move.b  d0,$2b00+$1d(a1)
                move.w  $2b00+$1c(a1),d1
                and.w   #$1ff,d1
                swap    RY
                move.w  d1,RY
                swap    RY
                next2
*-------------------------------------------------------------------------
* HRAS (readonly in emu)
*-------------------------------------------------------------------------
TED_1e
        IFD     STAT
                addq.l  #1,STATW1E
        ENDIF
                next2
*-------------------------------------------------------------------------
* FLASH (readonly yet - actinchar)
*-------------------------------------------------------------------------
TED_1f
        IFD     STAT
                addq.l  #1,STATW1F
        ENDIF
                move.l  d0,d1
                and.b   #7,d1
                move.l  d1,MY_actinchar(a1)
                and.b   #~$80,d0                ; feltételezve, hogy a
                move.b  d0,$2b00+$1f(a1)        ; nem használt bit 0.
                next2
*-------------------------------------------------------------------------
* SWITCH TO ROM
*-------------------------------------------------------------------------
TED_3e          move.b  d0,$2b00+$3e(a1)
                bset    #ACTRAMROM,RP
                bne     .tov
                move.l  MY_p4rom(a1),HIRAM
                or.b    #1,$2b00+$13(a1)
                move.w  RPC,d1
                bpl     .tov
                move.l  HIRAM,RPC
                add.l   d1,RPC
.tov            next2
*-------------------------------------------------------------------------
* SWITCH TO RAM
*-------------------------------------------------------------------------
TED_3f          move.b  d0,$2b00+$3f(a1)
                bclr    #ACTRAMROM,RP
                beq     .tov
                move.l  RAM,HIRAM
                and.b   #~1,$2b00+$13(a1)
.tov            next2

*-------------------------------------------------------------------------
* WORK:  -= IEC =-
* d1-ben a cím ahová írtak ($00/$01) (érték a címen +d0.b)
*-------------------------------------------------------------------------
iec_set         * egyenlõre ignoráljuk az IEC-buszt
                tst.w   d1
                beq     .adr00
* $01-re írtak
                and.b   #~$e0,d0                ; 5.bit mindig 0 (+6,7=0)
*               btst    #0,d0
*               seq     d1
*               and.b   #$80,d1
*               or.b    d1,d0
*               btst    #1,d0
*               seq     d1
*               and.b   #$40,d1
*               or.b    d1,d0
                moveq   #1,d1
                movem.l a0-a1,-(sp)
                jsr     _pin_IEC_W
                movem.l (sp)+,a0-a1
                clr.l   d1
                move.b  d0,1(RAM)
                next2
* $00-ra írtak
.adr00          and.b   #~$20,d0
                movem.l a0-a1,-(sp)
                jsr     _pin_IEC_W
                movem.l (sp)+,a0-a1
                clr.l   d1
                move.b  d0,(RAM)
                next2

*-------------------------------------------------------------------------
        cnop    0,4
*-------------------------------------------------------------------------

*-------------------------------------------------------------------------
*--------------------------- Register & IEC READ -------------------------
*-------------------------------------------------------------------------

*-------------------------------------------------------------------------
* IEC READ
* input:  d1 - a cím ahonnan olvastunk (00/01)
* output: d1 - a cím, és ott az érték
*-------------------------------------------------------------------------
iec_read        move.b  1(RAM),d0
                movem.l d1/a0-a1,-(sp)
                jsr     _pin_IEC_R
                movem.l (sp)+,d1/a0-a1
                move.b  d0,1(RAM)
                rts

*               tst.b   _ieccntflag             ; read flag
*               beq     .zero
*               move.b  .counter(pc),d0
*               addq.b  #1,d0
*               cmp.b   #10,d0
*               bne     .notexpired
*               move.b  1(RAM),d0               ; DAT=0
*               and.b   #~$80,d0
*               move.b  d0,1(RAM)
*               clr.l   d0
*               move.b  d0,_ieccntflag          ; clear flag
*               move.b  d0,.counter
*               rts
*.notexpired    clr.l   d0
*               rts
*.zero          clr.b   .counter
*               rts
*.counter       dc.b    0
        cnop    0,4

* Az iec_set már a jó eredményt rakja ide... (néha!!)
*               tst.w   d1
*               bne     .r01
*.r00           move.b  #$0f,(RAM)
*               rts
*.r01           move.b  #$c8,1(RAM)
*               rts

*-------------------------------------------------------------------------
* TED READ
* input:  d1 - a cím ahonnan olvastunk ($fd00-$ffff)
* output: d0 - a cucc
*
*  kilépéskor a flageknek az olvasott érték szerint kell beállnia,
*  ha nem így jön ki: tst.b d0
*-------------------------------------------------------------------------
reg_read        cmp.w   #$feff,d1
                bls     otherioR
* .. TEDREAD ..
                and.w   #$ff,d1
                jmp     ([tedreadtab,d1.l*4])
*-------------------------------------------------------------------------
* WORK: other I/O
* d1-ben a cím ahonnan olvastak
*-------------------------------------------------------------------------
otherioR        cmp.w   #$fddf,d1
                bhi     .pariecR
                cmp.b   #$0f,d1
                bls     .aciaR          ; USERPORT rs232c
                cmp.b   #$1f,d1
                bls     .parR           ; USERPORT parallel (8bit port)
                cmp.b   #$2f,d1
                bls     .nochip         ; vajon mi lehet itt??
                cmp.b   #$3f,d1
                bls     .keyreadR
                cmp.b   #$cf,d1
                bls     .nochip
                cmp.b   #$df,d1
                bls     .rommapR
.nochip         move.b  (RAM,d1.l),d0
                rts


* PARIECR fde0-feff
.pariecR        move.b  (RAM,d1.l),d0
                and.b   #7,d1
                bne     .parend
                move.b  #1,_rmode               ; pin_piec.c
.parend         rts

* ACIA  fd00-fd0f
.aciaR          move.b  (RAM,d1.l),d0
                rts

* PAR fd10-fd1f
.parR           move.b  (RAM,d1.l),d0
                rts
                
* KEYREAD fd30-fd3f
.keyreadR       move.b  (RAM,d1.l),d0
                rts

* ROMMAP fdd0-fddf
.rommapR        move.b  (RAM,d1.l),d0
                rts

*-------------------------------------------------------------
* SIMA TED regiszter olvasása
TEDREAD
        IFD     STAT
                addq.l  #1,(STATR00,d1.w*4)
        ENDIF
                or.w    #$ff00,d1
                move.b  (RAM,d1.l),d0
                rts

* nemlétezõ regiszter olvasása ($ff20-ff3d)
TED_NOREGR      or.w    #$ff00,d1
                move.b  (HIRAM,d1.l),d0
                rts

* ted mögötti ram/rom olvasása
TED_RAMR        or.w    #$ff00,d1
                move.b  (HIRAM,d1.l),d0
                rts



*-------------------------------------------------------------------------
* WORK: d1.l (-> d0.b az épp végrehajtott utasítás ciklusa)
* megadja a d1-ben, hogy mennyi órajel telt el ebbõl a rasztersorból
*-------------------------------------------------------------------------
getclk          MACRO
                add.w   d0,d1
                sub.w   CYC,d1
                ENDM

*-------------------------------------------------------------
* counter 1 LO
TED_00R
        IFD     STAT
                addq.l  #1,STATR00
        ENDIF
                move.w  cnt1actclk,d1   ; 32
                getclk                  ; 32 cyc telt el
                move.w  counter1,d0     ; counter==11
                sub.w   d1,d0           ; 11-32=-11
                bcc     .1lo
                move.w  initcnt1,d1     ; !OVERFLOW occurred (start==8)
                neg.w   d0              ; +11
                cmp.w   d1,d0           ; 11-8 az pozitív
                bmi     .1d0kis
                divu.w  d1,d0           ; 11%8==3
                swap    d0              ; get remainder
                sub.w   d0,d1           ; 8-3=5
                bra     .1tov
.1d0kis         neg.w   d0
                add.w   d0,d1
.1tov           move.l  d1,d0           ; clear quotient from d0-hi
.1lo            move.w  #$ff00,d1
                and.w   #$ff,d0
                tst.b   d0
                rts

* counter 1 HI
TED_01R
        IFD     STAT
                addq.l  #1,STATR01
        ENDIF
                move.w  cnt1actclk,d1   ; 32
                getclk                  ; 32 cyc telt el
                move.w  counter1,d0     ; counter==11
                sub.w   d1,d0           ; 11-32=-11
                bcc     .1lo
                move.w  initcnt1,d1     ; !OVERFLOW occurred (start==8)
                neg.w   d0              ; +11
                cmp.w   d1,d0           ; 11-8 az pozitív
                bmi     .1d0kis
                divu.w  d1,d0           ; 11%8==3
                swap    d0              ; get remainder
                sub.w   d0,d1           ; 8-3=5
                bra     .1tov
.1d0kis         neg.w   d0
                add.w   d0,d1
.1tov           move.l  d1,d0           ; clear quotient from d0-hi
.1lo            move.w  #$ff01,d1
                lsr.w   #8,d0
                tst.b   d0
                rts

* counter 2 LO
TED_02R
        IFD     STAT
                addq.l  #1,STATR02
        ENDIF
                move.w  cnt2actclk,d1
                getclk
                move.b  counter2+1,d0
                sub.b   d1,d0
                move.w  #$ff02,d1
                tst.b   d0
                rts

* counter 2 HI
TED_03R
        IFD     STAT
                addq.l  #1,STATR03
        ENDIF
                move.w  cnt2actclk,d1
                getclk
                move.w  counter2,d0
                sub.w   d1,d0
                move.w  #$ff03,d1
                lsr.w   #8,d0
                rts

* counter 3 LO
TED_04R
        IFD     STAT
                addq.l  #1,STATR04
        ENDIF
                move.w  cnt3actclk,d1
                getclk
                move.b  counter3+1,d0
                sub.b   d1,d0
                move.w  #$ff04,d1
                tst.b   d0
                rts

* counter 3 HI
TED_05R
        IFD     STAT
                addq.l  #1,STATR05
        ENDIF
                move.w  cnt3actclk,d1
                getclk
                move.w  counter3,d0
                sub.w   d1,d0
                move.w  #$ff05,d1
                lsr.w   #8,d0
                rts

* ACTRASTER1 read
TED_1cR
        IFD     STAT
                addq.l  #1,STATR1C
        ENDIF
                move.l  RY,d1
                swap    d1              ; get _p4actras
                lsr.w   #8,d1
                move.b  d1,d0
                clr.l   d1
                move.w  #$ff1c,d1
                or.b    #$fe,d0
                rts

* ACTRASTER2 read
TED_1dR
        IFD     STAT
                addq.l  #1,STATR1D
        ENDIF
                move.l  RY,d1
                swap    d1              ; get _p4actras
                move.b  d1,d0
                clr.l   d1
                move.w  #$ff1d,d1
                tst.b   d0
                rts

* HRAS kiszámítása
TED_1eR
        IFD     STAT
                addq.l  #1,STATR1E
        ENDIF
                getclk
                move.b  d1,d0
                move.w  #$ff1e,d1
                lsl.b   #2,d0
                rts

* ROMSWITCH READ (3e-3f)
TED_ROMSWR      or.w    #$ff00,d1
                move.b  (HIRAM,d1.l),d0
                rts



*-------------------------------------------------------------------------
* WORK:  üzenet kiírás
*-------------------------------------------------------------------------
        IFD     STAT_IEC
iecmessage      MACRO
                movem.l d0-d1/a0-a1,-(sp)
                move.l  \2,d1
                moveq   #\1,d0
                jsr     _iecmessage
                movem.l (sp)+,d0-d1/a0-a1
                ENDM
        ENDIF

*-------------------------------------------------------------------------
* WORK:
*-------------------------------------------------------------------------
iec_reset       movem.l d0-d7/a0-a6,-(sp)
                jsr     _pin_IEC_Reset
                btst    #IEC,RP
                beq     .rts
                jsr     _pin_PIEC_Reset
.rts            movem.l (sp)+,d0-d7/a0-a6
                rts

*OLD routine
*               moveq   #8,d0
*               move.b  Addrs+8,d1              ; ,8
*               cmp.b   #1,d1
*               bne     .s01
*               bsr     .reset_iec
*.s01           cmp.b   #2,d1
*               bne     .s02
*               bsr     .reset_soft
*.s02           moveq   #9,d0
*               move.b  Addrs+9,d1              ; ,9
*               cmp.b   #1,d1
*               bne     .s03
*               bsr     .reset_iec
*.s03           cmp.b   #2,d1
*               bne     .s04
*               bsr     .reset_soft
*.s04           moveq   #10,d0
*               move.b  Addrs+10,d1             ; ,10
*               cmp.b   #1,d1
*               bne     .s05
*               bsr     .reset_iec
*.s05           cmp.b   #2,d1
*               bne     .s06
*               bsr     .reset_soft
*.s06           moveq   #11,d0
*               move.b  Addrs+11,d1             ; ,11
*               cmp.b   #1,d1
*               bne     .s07
*               bsr     .reset_iec
*.s07           cmp.b   #2,d1
*               bne     .s08
*               bsr     .reset_soft
*.s08           movem.l (sp)+,d0-d7/a0-a6
*.rts           rts
*.reset_iec     movem.l d0-d1,-(sp)
*               move.l  _IECBase,a6             ; device addr in d0
*               move.l  d0,-(sp)
*               LIBCALL Listen
*               moveq   #15,d0
*               add.b   #CMD_OPEN,d0
*               LIBCALL Second
*               LIBCALL UnListen
*               move.l  (sp),d0                 ; device addr (read only!)
*               LIBCALL Listen
*               moveq   #15,d0
*               add.b   #CMD_DATA,d0
*               LIBCALL Second
*               moveq   #73,d0                  ; 'I'
*               LIBCALL CIOut
*               LIBCALL UnListen
*               move.l  (sp)+,d0                ; device addr
*               LIBCALL Listen
*               moveq   #15,d0
*               add.b   #CMD_CLOSE,d0
*               LIBCALL Second
*               LIBCALL UnListen
*               movem.l (sp)+,d0-d1
*               rts
*.reset_soft    movem.l d0-d1,-(sp)
*               jsr     _SOFT_Reset             ; devaddr in d0
*               movem.l (sp)+,d0-d1
*               rts

**************************************************************************
* DELETED, IEC only by registers (1551 only)
**-------------------------------------------------------------------------
** WORK:  IEC PATCH $12 illegal opcode patched
**-------------------------------------------------------------------------
*iecpatch       movem.l a1/a6,-(sp)
*               move.b  (RPC),d0        ; get code
*               cmp.b   #0,d0
*               beq     .talk
*               cmp.b   #1,d0
*               beq     .listen
*               cmp.b   #2,d0
*               beq     .out
*               cmp.b   #3,d0
*               beq     .second
*               cmp.b   #4,d0
*               beq     .sectalk
*               cmp.b   #5,d0
*               beq     .untalk
*               cmp.b   #6,d0
*               beq     .unlisten
*               cmp.b   #7,d0
*               beq     .in
*.ill           clr.l   d0
*               clr.l   d1
*               movem.l (sp)+,a1/a6
*               jmp     halt_crash
**---------------------------------------.
*.status                move.b  iec_ST(a6),d0   ;
*.st2           or.b    d0,$90(RAM)     ; set STATUS
*               clr.b   $95(RAM)        ; clear buffer
*               clr.b   $aa(RAM)        ; bitszámláló
*               clr.b   RSP             ; clear carry & X       CLC
*               and.w   #~F_IRQ,RA      ;                       CLI
*               move.b  RX,RA           ;
*               move.w  CCR,RP          ;
**---------------------------------------+
*.end           clr.l   d0              ;
*               clr.l   d1              ;
*               movem.l (sp)+,a1/a6     ;
*               jmp     rts             ;
**---------------------------------------'
*.seterr                move.b  #$80,$90(RAM)
*               bra     .st2
*.bufout                tst.b   $94(RAM)
*               bpl     .ures
*               move.b  $94(RAM),d0
*               lsr.b   #1,d0
*               move.b  d0,$94(RAM)
*               move.b  $95(RAM),d0
*       IFD     STAT_IEC
*               iecmessage IM_CIOUT,d0
*       ENDIF
*               LIBCALLB CIOut,_IECBase
*               move.b  iec_ST(a6),d0
*               or.b    d0,$90(RAM)     ; set STATUS
*.ures          rts
*.soft_bufout   tst.b   $94(RAM)
*               bpl     .ures
*               move.b  $94(RAM),d0
*               lsr.b   #1,d0
*               move.b  d0,$94(RAM)
*               move.b  $95(RAM),d0
*               jsr     _SOFT_CIOut
*               move.b  _d64_ST,d0
*               or.b    d0,$90(RAM)     ; set STATUS
*               rts

** TALK
** ---- egységszám in RA
*.talk          sub.w   #T_TALK,CYC
*               move.l  #Addrs,a0
*               clr.l   d0
*               move.b  RA,d0
*               add.l   d0,a0
*               cmp.b   #1,(a0)                 ; 1-iec
*               beq     .talk_iec
*               cmp.b   #2,(a0)                 ; 2-soft
*               beq     .talk_soft
*               move.b  #IT_NONE,Act_Use
*               bra     .seterr
*.talk_soft     or.b    #IT_SOFT,Act_Use
*               bsr     .soft_bufout
*               move.b  RA,d0
*               jsr     _SOFT_Talk
*               move.b  _d64_ST,d0
*               bra     .st2
*.talk_iec      btst    #IEC,RP
*               beq     .ill
*               or.b    #IT_IEC,Act_Use
*               bsr     .bufout
*               move.b  RA,d0
*       IFD     STAT_IEC
*               iecmessage IM_TALK,d0
*       ENDIF
*               LIBCALLB Talk,_IECBase
*               bra     .status

** LISTEN
** ------ egységszám in RA
*.listen                sub.w   #T_LISTEN,CYC
*               move.l  #Addrs,a0
*               clr.l   d0
*               move.b  RA,d0
*               add.l   d0,a0
*               cmp.b   #1,(a0)                 ; 1-iec
*               beq     .listen_iec
*               cmp.b   #2,(a0)                 ; 2-soft
*               beq     .listen_soft
*               move.b  #IT_NONE,Act_Use
*               bra     .seterr
*.listen_soft   or.b    #IT_SOFT,Act_Use
*               bsr     .soft_bufout
*               move.b  RA,d0
*               jsr     _SOFT_Listen
*               move.b  _d64_ST,d0
*               bra     .st2
*.listen_iec    btst    #IEC,RP
*               beq     .ill
*               or.b    #IT_IEC,Act_Use
*               bsr     .bufout
*               move.b  RA,d0
*       IFD     STAT_IEC
*               iecmessage IM_LISTEN,d0
*       ENDIF
*               LIBCALLB Listen,_IECBase
*               bra     .status

** UNTALK
** ------
*.untalk                sub.w   #T_UNTALK,CYC
*               move.b  Act_Use,d0
*               move.b  #IT_NONE,Act_Use
*               cmp.b   #IT_IEC,d0
*               beq     .untalk_iec
*               cmp.b   #IT_SOFT,d0
*               beq     .untalk_soft
*               bra     .seterr
*.untalk_soft   bsr     .soft_bufout
*               jsr     _SOFT_UnTalk
*               move.b  _d64_ST,d0
*               bra     .st2
*.untalk_iec    btst    #IEC,RP
*               beq     .ill
*               bsr     .bufout
*       IFD     STAT_IEC
*               iecmessage IM_UNTALK,#-1
*       ENDIF
*               LIBCALLB UnTalk,_IECBase
*               bra     .status

** UNLISTEN
** --------
*.unlisten      sub.w   #T_UNLISTEN,CYC
*               move.b  Act_Use,d0
*               move.b  #IT_NONE,Act_Use
*               cmp.b   #IT_IEC,d0
*               beq     .unlisten_iec
*               cmp.b   #IT_SOFT,d0
*               beq     .unlisten_soft
*               bra     .seterr
*.unlisten_soft bsr     .soft_bufout
*               jsr     _SOFT_UnListen
*               move.b  _d64_ST,d0
*               bra     .st2
*.unlisten_iec  btst    #IEC,RP
*               beq     .ill
*               bsr     .bufout
*       IFD     STAT_IEC
*               iecmessage  IM_UNLISTEN,#-1
*       ENDIF
*               LIBCALLB UnListen,_IECBase
*               bra     .status

** OUT
** --- ZP $95 byte-out
*.out           sub.w   #T_OUT,CYC
*               move.b  Act_Use,d0
*               cmp.b   #IT_IEC,d0
*               beq     .out_iec
*               cmp.b   #IT_SOFT,d0
*               beq     .out_soft
*               bra     .seterr
*.out_soft      move.b  $95(RAM),d0
*               jsr     _SOFT_CIOut
*               move.b  _d64_ST,d0
*               bra     .st2
*.out_iec       btst    #IEC,RP
*               beq     .ill
*               move.b  $95(RAM),d0
*       IFD     STAT_IEC
*               iecmessage IM_CIOUT,d0
*       ENDIF
*               LIBCALLB CIOut,_IECBase
*               bra     .status

** IN
** -- $A8 byte-in & RA
*.in            sub.w   #T_IN,CYC
*               move.b  Act_Use,d0
*               cmp.b   #IT_IEC,d0
*               beq     .in_iec
*               cmp.b   #IT_SOFT,d0
*               beq     .in_soft
*               bra     .seterr
*.in_soft       jsr     _SOFT_ACPtr
*               move.b  _d64_ST,ST_asm
*               bra     .in_finish
*.in_iec                btst    #IEC,RP
*               beq     .ill
*               LIBCALLB ACPtr,_IECBase
*       IFD     STAT_IEC
*               iecmessage IM_ACPTR,d0
*       ENDIF
*               move.b  iec_ST(a6),ST_asm
*.in_finish     move.b  d0,$A8(RAM)
*               move.b  d0,RA
*               move.w  CCR,RP
*               move.b  ST_asm(pc),d0
*               or.b    d0,$90(RAM)     ; set STATUS
*               clr.b   $aa(RAM)        ; próbálkozásszámláló
*               tst.b   d0
*               beq     .inok
*               move.b  #1,$aa(RAM)
*.inok          clr.b   RSP             ; clear carry & X       CLC
*               and.w   #~F_IRQ,RA      ;                       CLI
*               bra     .end

** SECOND
** ------ addr RA-ban
*.second                sub.w   #T_SECOND,CYC
*               move.b  Act_Use,d0
*               cmp.b   #IT_IEC,d0
*               beq     .second_iec
*               cmp.b   #IT_SOFT,d0
*               beq     .second_soft
*               bra     .seterr
*.second_soft   move.b  RA,d0
*               jsr     _SOFT_Second
*               move.b  _d64_ST,d0
*               bra     .st2
*.second_iec    btst    #IEC,RP
*               beq     .ill
*               move.b  RA,d0
*       IFD     STAT_IEC
*               iecmessage IM_SEC,d0
*       ENDIF
*               LIBCALLB Second,_IECBase
*               and.w   #~F_OVER,RA
*               bra     .status

** SECTALK
** ------- addr RA-ban
*.sectalk       sub.w   #T_SECTALK,CYC
*               move.b  Act_Use,d0
*               cmp.b   #IT_IEC,d0
*               beq     .sectalk_iec
*               cmp.b   #IT_SOFT,d0
*               beq     .sectalk_soft
*               bra     .seterr
*.sectalk_soft  move.b  RA,d0
*               jsr     _SOFT_TkSA
*               move.b  _d64_ST,d0
*               bra     .st2
*.sectalk_iec   btst    #IEC,RP
*               beq     .ill
*               move.b  RA,d0
*       IFD     STAT_IEC
*               iecmessage IM_TKSA,d0
*       ENDIF
*               LIBCALLB TkSA,_IECBase
*               and.w   #~F_OVER,RA
*               bra     .status

*       cnop    0,4
*ST_asm         dc.b    0               ; shadow status
*Act_Use                dc.b    0               ; use? 1-iec 2-soft 0-semmi
*Addrs          dc.b    0,0,0,0,0,0,0,0 ; dummy
*VAddrs         dc.b    0,0,0,0         ; 8-11
*       cnop    0,4

**************************************************************************
*------------------------------------------------------------------------*
*------------------------------------------------------------------------*
*--------------------------- 7501 statements ----------------------------*
*------------------------------------------------------------------------*
*------------------------------------------------------------------------*
**************************************************************************


*-------------------------------------------------------------------------
* WORK:
* ADC d0
*-------------------------------------------------------------------------
sadcbin         MACRO
                and.w   #~F_OVER,RA             * clear overflow
                or.b    #4,RSP                  * set Z for addx
                move.w  RSP,CCR                 * c+4-Carry -> X
                addx.b  d0,RA
                move.w  CCR,RP
                bvc     \@1$
                or.w    #F_OVER,RA
\@1$            move.w  RP,RSP                  * save CCR with carry
                ENDM

*-------------------------------------------------------------------------
* WORK:         DECIMAL
* ADC d0
*-------------------------------------------------------------------------
sadcdec         MACRO
                and.w   #~F_OVER,RA             * clear overflow
                move.w  RSP,CCR
                abcd    d0,RA
                move.w  CCR,RP
                bvc     \@1$
                or.w    #F_OVER,RA              * set overflow
\@1$            move.w  RP,RSP                  * save carry
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* AND d0
*-------------------------------------------------------------------------
sand            MACRO
                and.b   \1,RA
                move.w  CCR,RP
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* ASL \1
*-------------------------------------------------------------------------
sasl            MACRO
                asl.b   #1,\1
                move.w  CCR,RP
                move.w  RP,RSP                  * save carry
                ENDM

*-------------------------------------------------------------------------
* WORK: d1
* BIT d0
*-------------------------------------------------------------------------
sbit            MACRO
                and.w   #~F_OVER,RA
                btst    #6,d0
                beq     \@1$
                or.w    #F_OVER,RA
\@1$            tst.b   d0
                move.w  CCR,d1          * save N
                and.b   RA,d0           * A & op
                move.w  CCR,RP
                or.b    d1,RP
                ENDM

*-------------------------------------------------------------------------
* WORK:
* COMPARE d0
*-------------------------------------------------------------------------
*
scompare        MACRO
                cmp.b   \2,\1
                move.w  CCR,RP
                scc     RSP
                ENDM

*-------------------------------------------------------------------------
* WORK:
* CMP d0
*-------------------------------------------------------------------------
scmp            MACRO
;               scompare RA,\1
                cmp.b   \1,RA
                move.w  CCR,RP
                scc     RSP
                ENDM

*-------------------------------------------------------------------------
* WORK:
* CPX d0
*-------------------------------------------------------------------------
scpx            MACRO
;               scompare RX,\1
                cmp.b   \1,RX
                move.w  CCR,RP
                scc     RSP
                ENDM

*-------------------------------------------------------------------------
* WORK:
* CPY d0
*-------------------------------------------------------------------------
scpy            MACRO
;               scompare RY,\1
                cmp.b   \1,RY
                move.w  CCR,RP
                scc     RSP
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* DEC d0
*-------------------------------------------------------------------------
sdec            MACRO
                subq.b  #1,\1
                move.w  CCR,RP
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* EOR d0
*-------------------------------------------------------------------------
seor            MACRO
                eor.b   \1,RA
                move.w  CCR,RP
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* INC d0
*-------------------------------------------------------------------------
sinc            MACRO
                addq.b  #1,\1
                move.w  CCR,RP
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* LSR \1
*-------------------------------------------------------------------------
slsr            MACRO
                lsr.b   #1,\1                   * CARRY a C-ben
                move.w  CCR,RP
                move.w  RP,RSP
                ENDM

*-------------------------------------------------------------------------
* WORK:
* ORA d0
*-------------------------------------------------------------------------
sora            MACRO
                or.b    \1,RA
                move.w  CCR,RP
                ENDM

*-------------------------------------------------------------------------
* WORK: -
* ROL \1 (dX)
*-------------------------------------------------------------------------
srol            MACRO
                move.w  RSP,CCR         * carry -> X
                roxl.b  #1,\1           * leendõ CARRY az C-ben
                move.w  CCR,RP
                move.w  RP,RSP
                ENDM

*-------------------------------------------------------------------------
* WORK:
* ROR \1
*-------------------------------------------------------------------------
sror            MACRO
                move.w  RSP,CCR
                roxr.b  #1,\1           * leendõ CARRY az C-ben
                move.w  CCR,RP
                move.w  RP,RSP
                ENDM

*-------------------------------------------------------------------------
* WORK:
* SBC d0
*-------------------------------------------------------------------------
ssbcbin         MACRO
                not.b   RSP                     * invert carry
                and.w   #~F_OVER,RA             * clear V
                or.b    #4,RSP                  * set Z for subx
                move.w  RSP,CCR
                subx.b  d0,RA
                move.w  CCR,RP
                bvc     \@1$
                or.w    #F_OVER,RA              * set V
\@1$            move.w  RP,RSP
                not.b   RSP                     * invert carry
                ENDM

*-------------------------------------------------------------------------
* WORK:         DECIMAL
* SBC d0
*-------------------------------------------------------------------------
ssbcdec         MACRO
                not.b   RSP
                and.w   #~F_OVER,RA
                move.w  RSP,CCR
                sbcd    d0,RA
                move.w  CCR,RP
                bvc     \@1$
                or.w    #F_OVER,RA
\@1$            move.w  RP,RSP
                not.b   RSP
                ENDM


*=========================================================================
*                       7501 OPCODES
*=========================================================================

*-------------------------------------------------------------------------
* WORK: -
* ..-CRA
*-------------------------------------------------------------------------
crash           jmp     halt_crash

*-------------------------------------------------------------------------
* WORK: -
* ..-ILL
*-------------------------------------------------------------------------
illegal         jmp     halt_illegal

*-------------------------------------------------------------------------
* WORK: -
* 0-BRK
*-------------------------------------------------------------------------
brk             or.w    #F_BRK,RA
                irqhbrk

*-------------------------------------------------------------------------
* WORK: d1.w  d0.b
* 1-ORA (ZP,X)
*-------------------------------------------------------------------------
oraizpx         addrizpx
                memreadnt d0,6
                sora d0
                next 6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 16-BPL REL
*-------------------------------------------------------------------------
bpl             
                move.w  RP,CCR
                bmi     .nbr
                branch
                next 3
.nbr            addq.l  #1,RPC
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 48-BMI REL
*-------------------------------------------------------------------------
bmi             
                move.w  RP,CCR
                bpl     .nbr
                branch
                next 3
.nbr            addq.l  #1,RPC
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 80-BVC REL
*-------------------------------------------------------------------------
bvc             
                btst    #B_OVER,RA
                bne     .nbr
                branch
                next 3
.nbr            addq.l  #1,RPC
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 112-BVS REL
*-------------------------------------------------------------------------
bvs             
                btst    #B_OVER,RA
                beq     .nbr
                branch
                next 3
.nbr            addq.l  #1,RPC
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 144-BCC REL
*-------------------------------------------------------------------------
bcc             
                move.w  RSP,CCR
                bcs     .nbr
                branch
                next 3
.nbr            addq.l  #1,RPC
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 176-BCS REL
*-------------------------------------------------------------------------
bcs             
                move.w  RSP,CCR
                bcc     .nbr
                branch
                next 3
.nbr            addq.l  #1,RPC
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 208-BNE REL
*-------------------------------------------------------------------------
bne             
                move.w  RP,CCR
                beq     .nbr
                branch
                next 3
***!ERR         subq.w  #3,CYC
***             nextst
.nbr            addq.l  #1,RPC
                next 2
***!ERR         subq.w  #2,CYC
***             nextst

*-------------------------------------------------------------------------
* WORK: d0,d1
* 240-BEQ REL
*-------------------------------------------------------------------------
beq             
                move.w  RP,CCR
                bne     .nbr
                branch
                next 3
***!ERR         subq.w  #3,CYC
***!ERR         nextst
.nbr            addq.l  #1,RPC
                next 2
***!ERR         subq.w  #2,CYC
***!ERR         nextst


*-------------------------------------------------------------------------
* WORK: d0
* 5-ORA ZP
*-------------------------------------------------------------------------
orazp           
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
;               sora d0
                sora <(RAM,d0.l)>
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 6-ASL ZP
*-------------------------------------------------------------------------
aslzp           
                addrzp
                memreadzp d0
                sasl d0
                memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: -
* 8-PHP
*-------------------------------------------------------------------------
php             
                move.l  RA,a0
                or.w    #F_BRK,RA
                pushrp
                move.l  a0,RA
                next 3

*-------------------------------------------------------------------------
* WORK: d0
* 9-ORA #BYTE
*-------------------------------------------------------------------------
orabyte         
;               addrbyte d0
;               sora d0
                sora <(RPC)+>
                next 2

*-------------------------------------------------------------------------
* WORK: -
* 10-ASL ACCU
*-------------------------------------------------------------------------
aslaccu         
                sasl RA
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 13-ORA ABS
*-------------------------------------------------------------------------
oraabs          
                addrabs
                memreadnt d0,4
                sora d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 14-ASL ABSá
*-------------------------------------------------------------------------
aslabs          
                addrabs
                memreadnt d0,6
                sasl d0
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 17-ORA (ZP),Y
*-------------------------------------------------------------------------
oraizpy         
                addrizpy
                memreadnt d0,5
                sora d0
                next 5

*-------------------------------------------------------------------------
* WORK: d0,d1
* 21-ORA ZP,X
*-------------------------------------------------------------------------
orazpx          
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp d0
;               sora d0
                sora <(RAM,d0.l)>
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 22-ASL ZP,X
*-------------------------------------------------------------------------
aslzpx          
                addrzpx
                memreadzp d0
                sasl d0
                memwritezp d0,1,6

*-------------------------------------------------------------------------
* WORK: -
* 24-CLC
*-------------------------------------------------------------------------
clc             
                clr.b   RSP             ; clear carry & X
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 25-ORA ABS,Y
*-------------------------------------------------------------------------
oraabsy         
                addrabsyp
                memreadnt d0,4
                sora d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 29-ORA ABS,X
*-------------------------------------------------------------------------
oraabsx         
                addrabsxp
                memreadnt d0,4
                sora d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 30-ASL ABS,X
*-------------------------------------------------------------------------
aslabsx         
                addrabsx
                memreadnt d0,7
                sasl d0
                memwrite d0,1,7

*-------------------------------------------------------------------------
* WORK: d0,d1
* 32-JSR ABS
*-------------------------------------------------------------------------
jsrabs          
                memreadwordfnc d1
                addq.l  #1,RPC
                move.w  RPC,d0
                push2   d0
                clr.l   d0
                makepc
                next 6

*-------------------------------------------------------------------------
* WORK: d1  d0
* 33-AND (ZP,X)
*-------------------------------------------------------------------------
andizpx         
                addrizpx
                memreadnt d0,6
                sand d0
                next 6

*-------------------------------------------------------------------------
* WORK: d1
* 36-BIT ZP
*-------------------------------------------------------------------------
bitzp           
                addrzp
                memreadzp d0
                sbit
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 37-AND ZP
*-------------------------------------------------------------------------
andzp           
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
;               sand d0
                sand <(RAM,d0.l)>
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 38-ROL ZP
*-------------------------------------------------------------------------
rolzp           
                addrzp
                memreadzp d0
                srol d0
                memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: 
* 40-PLP
*-------------------------------------------------------------------------
plp             
                poprp
                next 4

*-------------------------------------------------------------------------
* WORK: 
* 41-AND #BYTE
*-------------------------------------------------------------------------
andbyte         
;               addrbyte d0
;               sand d0
                sand <(RPC)+>
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 42-ROL ACC
*-------------------------------------------------------------------------
rolaccu         
                srol RA
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 44-BIT ABS
*-------------------------------------------------------------------------
bitabs          
                addrabs
                memreadnt d0,4
                sbit
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 45-AND ABS
*-------------------------------------------------------------------------
andabs          
                addrabs
                memreadnt d0,4
                sand d0
                next 4


*-------------------------------------------------------------------------
* WORK: d0,d1
* 46-ROL ABS
*-------------------------------------------------------------------------
rolabs          
                addrabs
                memreadnt d0,6
                srol d0
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 49-AND (ZP),Y
*-------------------------------------------------------------------------
andizpy         
                addrizpy
                memreadnt d0,5
                sand d0
                next 5

*-------------------------------------------------------------------------
* WORK: d0,d1
* 53-AND ZP,X
*-------------------------------------------------------------------------
andzpx          
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp d0
;               sand d0
                sand <(RAM,d0.l)>
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 54-ROL ZP,X
*-------------------------------------------------------------------------
rolzpx          
                addrzpx
                memreadzp d0
                srol d0
                memwritezp d0,1,6

*-------------------------------------------------------------------------
* WORK: -
* 56-SEC
*-------------------------------------------------------------------------
sec             
                st      RSP             ; set carry & X
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 57-AND ABS,Y
*-------------------------------------------------------------------------
andabsy         
                addrabsyp
                memreadnt d0,4
                sand d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 61-AND ABS,X
*-------------------------------------------------------------------------
andabsx         
                addrabsxp
                memreadnt d0,4
                sand d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 62-ROL ABS,X
*-------------------------------------------------------------------------
rolabsx         
                addrabsx
                memreadnt d0,7
                srol d0
                memwrite d0,1,7

*-------------------------------------------------------------------------
* WORK: d0
* 64-RTI
*-------------------------------------------------------------------------
rti             
                poprp
                pop2 d1
                makepc
                next 6

*-------------------------------------------------------------------------
* WORK: d1  d0
* 65-EOR (ZP,X)
*-------------------------------------------------------------------------
eorizpx         
                addrizpx
                memreadnt d0,6
                seor d0
                next 6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 69-EOR ZP
*-------------------------------------------------------------------------
eorzp           
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
                move.b  (RAM,d0.l),d0
                seor d0
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 70-LSR ZP
*-------------------------------------------------------------------------
lsrzp           
                addrzp
                memreadzp d0
                slsr d0
                memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: 
* 72-PHA
*-------------------------------------------------------------------------
pha             
                push RA
                next 3

*-------------------------------------------------------------------------
* WORK: 
* 73-EOR #BYTE
*-------------------------------------------------------------------------
eorbyte         
                addrbyte d0
                seor d0
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 74-LSR ACC
*-------------------------------------------------------------------------
lsraccu         
                slsr RA
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 76-JMP ABS
*-------------------------------------------------------------------------
jmpabs          
                memreadwordfnc d1
                makepc
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 77-EOR ABS
*-------------------------------------------------------------------------
eorabs          
                addrabs
                memreadnt d0,4
                seor d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 78-LSR ABS
*-------------------------------------------------------------------------
lsrabs          
                addrabs
                memreadnt d0,6
                slsr d0
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 81-EOR (ZP),Y
*-------------------------------------------------------------------------
eorizpy         
                addrizpyp
                memreadnt d0,5
                seor d0
                next 5

*-------------------------------------------------------------------------
* WORK: d0,d1
* 85-EOR ZP,X
*-------------------------------------------------------------------------
eorzpx          
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp d0
                move.b  (RAM,d0.l),d0
                seor d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 86-LSR ZP,X
*-------------------------------------------------------------------------
lsrzpx          
                addrzpx
                memreadzp d0
                slsr d0
                memwritezp d0,1,6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 88-CLI
*-------------------------------------------------------------------------
cli             
                and.w   #~F_IRQ,RA
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 89-EOR ABS,Y
*-------------------------------------------------------------------------
eorabsy         
                addrabsyp
                memreadnt d0,4
                seor d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 93-EOR ABS,X
*-------------------------------------------------------------------------
eorabsx         
                addrabsxp
                memreadnt d0,4
                seor d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 94-LSR ABS,X
*-------------------------------------------------------------------------
lsrabsx         
                addrabsx
                memreadnt d0,6
                slsr d0
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: d0
* 96-RTS
*-------------------------------------------------------------------------
rts             
                pop2 d1
                addq.w  #1,d1
                makepc
                next 6

*-------------------------------------------------------------------------
* WORK: d1  d0
* 97-ADC (ZP,X)
*-------------------------------------------------------------------------
adcizpxbin      
                addrizpx
                memreadnt d0,6
                sadcbin
                next 6

*-------------------------------------------------------------------------
* WORK: d1  d0 DECIMAL
* 97-ADC (ZP,X)
*-------------------------------------------------------------------------
adcizpxdec      
                addrizpx
                memreadnt d0,6
                sadcdec
                next 6

*-------------------------------------------------------------------------
* WORK: d1  d0
* 101-ADC ZP
*-------------------------------------------------------------------------
adczpbin        
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
                move.b  (RAM,d0.l),d0
                sadcbin
                next 3

*-------------------------------------------------------------------------
* WORK: d1  d0 DECIMAL
* 101-ADC ZP
*-------------------------------------------------------------------------
adczpdec        
                addrzp
                memreadzp d0
                sadcdec
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 102-ROR ZP
*-------------------------------------------------------------------------
rorzp           
                addrzp
                memreadzp d0
                sror d0
                memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: -
* 104-PLA
*-------------------------------------------------------------------------
pla             
                popns RA
                move.w  CCR,RP
                swap    RSP
                next 4

*-------------------------------------------------------------------------
* WORK: d1  d0
* 105-ADC #BYTE
*-------------------------------------------------------------------------
adcbytebin      
                addrbyte d0
                sadcbin
                next 2
*-------------------------------------------------------------------------
* WORK: d1  d0 DECIMAL
* 105-ADC #BYTE
*-------------------------------------------------------------------------
adcbytedec      
                addrbyte d0
                sadcdec
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 106-ROR ACCU
*-------------------------------------------------------------------------
roraccu         
                sror RA
                next 2

*-------------------------------------------------------------------------
* WORK: d1
* 108-JMP (ABS)
*-------------------------------------------------------------------------
jmpiabs         
                addriabs
                makepc
                next 3

*-------------------------------------------------------------------------
* WORK: d1  d0
* 109-ADC ABS
*-------------------------------------------------------------------------
adcabsbin       
                addrabs
                memreadnt d0,4
                sadcbin
                next 4
*-------------------------------------------------------------------------
* WORK: d1  d0 DECIMAL
* 109-ADC ABS
*-------------------------------------------------------------------------
adcabsdec       
                addrabs
                memreadnt d0,4
                sadcdec
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 110-ROR ABS
*-------------------------------------------------------------------------
rorabs          
                addrabs
                memreadnt d0,6
                sror d0
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: d1  d0
* 113-ADC (ZP),Y
*-------------------------------------------------------------------------
adcizpybin      
                addrizpyp
                memreadnt d0,5
                sadcbin
                next 5

*-------------------------------------------------------------------------
* WORK: d1  d0 DECIMAL
* 113-ADC (ZP),Y
*-------------------------------------------------------------------------
adcizpydec      
                addrizpyp
                memreadnt d0,5
                sadcdec
                next 5

*-------------------------------------------------------------------------
* WORK: d1  d0
* 117-ADC ZP,X
*-------------------------------------------------------------------------
adczpxbin       
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp d0
                move.b  (RAM,d0.l),d0
                sadcbin
                next 4

*-------------------------------------------------------------------------
* WORK: d1  d0 DECIMAL
* 117-ADC ZP,X
*-------------------------------------------------------------------------
adczpxdec       
                addrzpx
                memreadzp d0
                sadcdec
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 118-ROR ZP,X
*-------------------------------------------------------------------------
rorzpx          
                addrzpx
                memreadzp d0
                sror d0
                memwritezp d0,1,6

*-------------------------------------------------------------------------
* WORK: -
* 120-SEI
*-------------------------------------------------------------------------
sei             
                or.w    #F_IRQ,RA
                next 2

*-------------------------------------------------------------------------
* WORK: d1  d0
* 121-ADC ABS,Y
*-------------------------------------------------------------------------
adcabsybin      
                addrabsyp
                memreadnt d0,4
                sadcbin
                next 4
*-------------------------------------------------------------------------
* WORK: d1  d0 DECIMAL
* 121-ADC ABS,Y
*-------------------------------------------------------------------------
adcabsydec      
                addrabsyp
                memreadnt d0,4
                sadcdec
                next 4

*-------------------------------------------------------------------------
* WORK: d1  d0
* 125-ADC ABS,X
*-------------------------------------------------------------------------
adcabsxbin      
                addrabsxp
                memreadnt d0,4
                sadcbin
                next 4
*-------------------------------------------------------------------------
* WORK: d1  d0 DECIMAL
* 125-ADC ABS,X
*-------------------------------------------------------------------------
adcabsxdec      
                addrabsxp
                memreadnt d0,4
                sadcdec
                next 4


*-------------------------------------------------------------------------
* WORK: d0,d1
* 126-ROR ABS,X
*-------------------------------------------------------------------------
rorabsx         
                addrabsx
                memreadnt d0,7
                sror d0
                memwrite d0,1,7

*-------------------------------------------------------------------------
* WORK: 
* 129-STA (ZP,X)
*-------------------------------------------------------------------------
staizpx         
                addrizpx
                memwrite RA,0,6

*-------------------------------------------------------------------------
* WORK: d0
* 132-STY ZP
*-------------------------------------------------------------------------
styzp           
;               addrzp
;               memwritezp RY,0,3
                move.b  (RPC)+,d0
                move.b  RY,(RAM,d0.l)
                next 3

                

*-------------------------------------------------------------------------
* WORK: d0
* 133-STA ZP
*-------------------------------------------------------------------------
stazpT
;               addrzp
;               memwritezpt RA,0,3
;;              memwritezp RA,0,3
                move.b  (RPC)+,d0
                move.b  RA,(RAM,d0.l)
                cmp.b   #$01,d0
                bls     .1
                next 3
.1              subq.w  #3,CYC
                move.b  d0,d1
                move.b  RA,d0
                bra     iec_set

*-------------------------------------------------------------------------
* WORK: d0
* 133-STA ZP
*-------------------------------------------------------------------------
stazp
;               addrzp
;               memwritezpt RA,0,3
;;              memwritezp RA,0,3
                move.b  (RPC)+,d0
                move.b  RA,(RAM,d0.l)
                next 3

*-------------------------------------------------------------------------
* WORK: d0
* 134-STX ZP
*-------------------------------------------------------------------------
stxzp           
;               addrzp
;               memwritezp RX,0,3
                move.b  (RPC)+,d0
                move.b  RX,(RAM,d0.l)
                next 3

*-------------------------------------------------------------------------
* WORK: -
* 136-DEY
*-------------------------------------------------------------------------
dey             
                subq.b  #1,RY
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 138-TXA
*-------------------------------------------------------------------------
txa             
                move.b  RX,RA
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 140-STY ABS
*-------------------------------------------------------------------------
styabs          
                addrabs
                memwrite RY,0,4

*-------------------------------------------------------------------------
* WORK: d0
* 141-STA ABS
*-------------------------------------------------------------------------
staabs          
                addrabs
                memwrite RA,0,4

*-------------------------------------------------------------------------
* WORK: d0
* 142-STX ABS
*-------------------------------------------------------------------------
stxabs          
                addrabs
                memwrite RX,0,4

*-------------------------------------------------------------------------
* WORK: d0
* 145-STA (ZP),Y
*-------------------------------------------------------------------------
staizpy         
                addrizpy
                memwrite RA,0,6

*-------------------------------------------------------------------------
* WORK: d0
* 148-STY ZP,X
*-------------------------------------------------------------------------
styzpx          
;               addrzpx
;               memwritezp RY,0,4
                move.b  (RPC)+,d0
                add.b   RX,d0
                move.b  RY,(RAM,d0.l)
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 149-STA ZP,X
*-------------------------------------------------------------------------
stazpx          
;               addrzpx
;               memwritezp RA,0,4
                move.b  (RPC)+,d0
                add.b   RX,d0
                move.b  RA,(RAM,d0.l)
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 150-STX ZP,Y
*-------------------------------------------------------------------------
stxzpy          
;               addrzpy
;               memwritezp RX,0,4
                move.b  (RPC)+,d0
                add.b   RY,d0
                move.b  RX,(RAM,d0.l)
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 152-TYA
*-------------------------------------------------------------------------
tya             
                move.b  RY,RA
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 153-STA ABS,Y
*-------------------------------------------------------------------------
staabsy         
                addrabsy
                memwrite RA,0,5

*-------------------------------------------------------------------------
* WORK: d0
* 154-TXS
*-------------------------------------------------------------------------
txs             
                swap    RSP
                move.b  RX,RSP
                swap    RSP
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 157-STA ABS,X
*-------------------------------------------------------------------------
staabsx         
                addrabsx
                memwrite RA,0,5

*-------------------------------------------------------------------------
* WORK: -
* 160-LDY #BYTE
*-------------------------------------------------------------------------
ldybyte         
                addrbyte RY
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: -
* 161-LDA (ZP,X)
*-------------------------------------------------------------------------
ldaizpx         
                addrizpx
                memreadnt RA,6
                move.w  CCR,RP
                next 6

*-------------------------------------------------------------------------
* WORK: -
* 162-LDX #BYTE
*-------------------------------------------------------------------------
ldxbyte         
                addrbyte RX
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 164-LDY ZP
*-------------------------------------------------------------------------
ldyzp           
;               addrzp
;               memreadzp RY
                move.b  (RPC)+,d0
                move.b  (RAM,d0.l),RY
                move.w  CCR,RP                  ; orig
                next 3

*-------------------------------------------------------------------------
* WORK: d0
* 165-LDA ZP
*-------------------------------------------------------------------------
ldazpT          
                addrzp
                memreadzpt RA
;               memreadzp RA
                move.w  CCR,RP
                next 3

*-------------------------------------------------------------------------
* WORK: d0
* 165-LDA ZP
*-------------------------------------------------------------------------
ldazp
                addrzp
;               memreadzpt RA
                memreadzp RA
                move.w  CCR,RP
                next 3

*-------------------------------------------------------------------------
* WORK: d0
* 166-LDX ZP
*-------------------------------------------------------------------------
ldxzp           
;               addrzp
;               memreadzp RX
                move.b  (RPC)+,d0
                move.b  (RAM,d0.l),RX
                move.w  CCR,RP                  ; orig
                next 3

*-------------------------------------------------------------------------
* WORK: d0
* 168-TAY
*-------------------------------------------------------------------------
tay             
                move.b  RA,RY
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: -
* 169-LDA #BYTE
*-------------------------------------------------------------------------
ldabyte         
                addrbyte RA
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 170-TAX
*-------------------------------------------------------------------------
tax             
                move.b  RA,RX
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 172-LDY ABS
*-------------------------------------------------------------------------
ldyabs          
                addrabs
                memreadnt RY,4
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 173-LDA ABS
*-------------------------------------------------------------------------
ldaabs          
                addrabs
                memreadnt RA,4
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 174-LDX ABS
*-------------------------------------------------------------------------
ldxabs          
                addrabs
                memreadnt RX,4
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 177-LDA (ZP),Y
*-------------------------------------------------------------------------
ldaizpy         
                addrizpyp
                memreadnt RA,5
                move.w  CCR,RP
                next 5

*-------------------------------------------------------------------------
* WORK: d0
* 180-LDY ZP,X
*-------------------------------------------------------------------------
ldyzpx          
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp RY
                move.b  (RAM,d0.l),RY
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 181-LDA ZP,X
*-------------------------------------------------------------------------
ldazpx          
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp RA
                move.b  (RAM,d0.l),RA
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 182-LDX ZP,Y
*-------------------------------------------------------------------------
ldxzpy          
;               addrzpy
                move.b  (RPC)+,d0
                add.b   RY,d0
;               memreadzp RX
                move.b  (RAM,d0.l),RX
                move.w  CCR,RP
                next 4
*-------------------------------------------------------------------------
* WORK: -
* 184-CLV
*-------------------------------------------------------------------------
clv             
                and.w   #~F_OVER,RA
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 185-LDA ABS,Y
*-------------------------------------------------------------------------
ldaabsy         
                addrabsyp
                memreadnt RA,4
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: -
* 186-TSX
*-------------------------------------------------------------------------
tsx             
                swap    RSP
                move.b  RSP,RX
                move.w  CCR,RP
                swap    RSP
                next 2

*-------------------------------------------------------------------------
* WORK: d0
* 188-LDY ABS,X
*-------------------------------------------------------------------------
ldyabsx         
                addrabsxp
                memreadnt RY,4
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 189-LDA ABS,X
*-------------------------------------------------------------------------
ldaabsx         
                addrabsxp
                memreadnt RA,4
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0
* 190-LDX ABS,Y
*-------------------------------------------------------------------------
ldxabsy         
                addrabsyp
                memreadnt RX,4
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 192-CPY #BYTE
*-------------------------------------------------------------------------
cpybyte         
;               addrbyte d0
;               scpy d0
                scpy <(RPC)+>
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 193-CMP (ZP,X)
*-------------------------------------------------------------------------
cmpizpx         
                addrizpx
                memreadnt d0,6
                scmp d0
                next 6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 196-CPY ZP
*-------------------------------------------------------------------------
cpyzp           
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
;               scpy d0
                scpy <(RAM,d0.l)>
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 197-CMP ZP
*-------------------------------------------------------------------------
cmpzp           
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
;               scmp d0
                scmp <(RAM,d0.l)>
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 198-DEC ZP
*-------------------------------------------------------------------------
deczp           
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
                sdec <(RAM,d0.l)>
                next 5
;               memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: -
* 200-INY
*-------------------------------------------------------------------------
iny             
                addq.b  #1,RY
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 201-CMP #BYTE
*-------------------------------------------------------------------------
cmpbyte         
;               addrbyte d0
;               scmp d0
                scmp <(RPC)+>
                next 2

*-------------------------------------------------------------------------
* WORK: -
* 202-DEX
*-------------------------------------------------------------------------
dex             
                subq.b  #1,RX
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 204-CPY ABS
*-------------------------------------------------------------------------
cpyabs          
                addrabs
                memreadnt d0,4
                scpy d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 205-CMP ABS
*-------------------------------------------------------------------------
cmpabs          
                addrabs
                memreadnt d0,3
                scmp d0
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 206-DEC ABS
*-------------------------------------------------------------------------
decabs          
                addrabs
                memreadnt d0,6
                sdec d0
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 209-CMP (ZP),Y
*-------------------------------------------------------------------------
cmpizpy         
                addrizpyp
                memreadnt d0,5
                scmp d0
                next 5

*-------------------------------------------------------------------------
* WORK: d0,d1
* 213-CMP ZP,X
*-------------------------------------------------------------------------
cmpzpx          
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp d0
;               scmp d0
                scmp <(RAM,d0.l)>
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 214-DEC ZP,X
*-------------------------------------------------------------------------
deczpx          
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp d0
;               sdec d0
                sdec <(RAM,d0.l)>
                next 6
;               memwritezp d0,1,6

*-------------------------------------------------------------------------
* WORK: -
* 216-CLD
*-------------------------------------------------------------------------
cld             
                and.w   #~F_DEC,RA
                move.l  #fetchtable,FETCH
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 217-CMP ABS,Y
*-------------------------------------------------------------------------
cmpabsy         
                addrabsyp
                memreadnt d0,4
                scmp d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 221-CMP ABS,X
*-------------------------------------------------------------------------
cmpabsx         
                addrabsxp
                memreadnt d0,4
                scmp d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 222-DEC ABS,X
*-------------------------------------------------------------------------
decabsx         
                addrabsx
                memreadnt d0,7
                sdec d0
                memwrite d0,1,7

*-------------------------------------------------------------------------
* WORK: d0,d1
* 224-CPX #BYTE
*-------------------------------------------------------------------------
cpxbyte         
;               addrbyte d0
;               scpx d0
                scpx <(RPC)+>
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 225-SBC (ZP,X)
*-------------------------------------------------------------------------
sbcizpxbin      
                addrizpx
                memreadnt d0,6
                ssbcbin
                next 6
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 225-SBC (ZP,X)
*-------------------------------------------------------------------------
sbcizpxdec      
                addrizpx
                memreadnt d0,6
                ssbcdec
                next 6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 228-CPX ZP
*-------------------------------------------------------------------------
cpxzp           
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
;               scpx d0
                scpx <(RAM,d0.l)>
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 229-SBC ZP
*-------------------------------------------------------------------------
sbczpbin        
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
                move.b  (RAM,d0.l),d0
                ssbcbin
                next 3
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 229-SBC ZP
*-------------------------------------------------------------------------
sbczpdec        
                addrzp
                memreadzp d0
                ssbcdec
                next 3

*-------------------------------------------------------------------------
* WORK: d0,d1
* 230-INC ZP
*-------------------------------------------------------------------------
inczp           
;               addrzp
                move.b  (RPC)+,d0
;               memreadzp d0
                sinc <(RAM,d0.l)>
                next 5
;               memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: d0,d1
* 232-INX
*-------------------------------------------------------------------------
inx             
                addq.b  #1,RX
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 233-SBC #BYTE
*-------------------------------------------------------------------------
sbcbytebin      
                addrbyte d0
                ssbcbin
                next 2
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 233-SBC #BYTE
*-------------------------------------------------------------------------
sbcbytedec      
                addrbyte d0
                ssbcdec
                next 2

*-------------------------------------------------------------------------
* WORK: -
* 234-NOP
*-------------------------------------------------------------------------
nop             
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 236-CPX ABS
*-------------------------------------------------------------------------
cpxabs          
                addrabs
                memreadnt d0,4
                scpx d0
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 237-SBC ABS
*-------------------------------------------------------------------------
sbcabsbin       
                addrabs
                memreadnt d0,4
                ssbcbin
                next 4
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 237-SBC ABS
*-------------------------------------------------------------------------
sbcabsdec       
                addrabs
                memreadnt d0,4
                ssbcdec
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 238-INC ABS
*-------------------------------------------------------------------------
incabs          
                addrabs
                memreadnt d0,6
                sinc d0
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: d0,d1
* 241-SBC (ZP),Y
*-------------------------------------------------------------------------
sbcizpybin      
                addrizpy
                memreadnt d0,5
                ssbcbin
                next 5
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 241-SBC (ZP),Y
*-------------------------------------------------------------------------
sbcizpydec      
                addrizpy
                memreadnt d0,5
                ssbcdec
                next 5

*-------------------------------------------------------------------------
* WORK: d0,d1
* 245-SBC ZP,X
*-------------------------------------------------------------------------
sbczpxbin       
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp d0
                move.b  (RAM,d0.l),d0
                ssbcbin
                next 4
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 245-SBC ZP,X
*-------------------------------------------------------------------------
sbczpxdec       
                addrzpx
                memreadzp d0
                ssbcdec
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 246-INC ZP,X
*-------------------------------------------------------------------------
inczpx          
;               addrzpx
                move.b  (RPC)+,d0
                add.b   RX,d0
;               memreadzp d0
;               sinc d0
                sinc <(RAM,d0.l)>
                next 6
;               memwritezp d0,1,6

*-------------------------------------------------------------------------
* WORK: -
* 248-SED
*-------------------------------------------------------------------------
sed             
                or.w    #F_DEC,RA
                move.l  #fetchtabledec,FETCH
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* 249-SBC ABS,Y
*-------------------------------------------------------------------------
sbcabsybin      
                addrabsyp
                memreadnt d0,4
                ssbcbin
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 249-SBC ABS,Y
*-------------------------------------------------------------------------
sbcabsydec      
                addrabsyp
                memreadnt d0,4
                ssbcdec
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 253-SBC ABS,X
*-------------------------------------------------------------------------
sbcabsxbin      
                addrabsxp
                memreadnt d0,4
                ssbcbin
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 253-SBC ABS,X
*-------------------------------------------------------------------------
sbcabsxdec      
                addrabsxp
                memreadnt d0,4
                ssbcdec
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* 254-INC ABS,X
*-------------------------------------------------------------------------
incabsx         
                addrabsx
                memreadnt d0,7
                sinc d0
                memwrite d0,1,7

*-------------------------------------------------------------------------
*       ILLEGAL OPCODES
*-------------------------------------------------------------------------

*-------------------------------------------------------------------------
* WORK: 
* 0b,2b-AND #BYTE (carry=neg)
*-------------------------------------------------------------------------
andzpcn         
                clr.b   RSP             ; clear carry
                addrzp
                memreadzp d0
                sand d0
                bpl     .azpcn
                not.b   RSP             ; set carry & X
.azpcn          next 2

*-------------------------------------------------------------------------
* WORK: -
* ..-NOP2
*-------------------------------------------------------------------------
nop2            
                addq.l  #1,RPC
                next 2

*-------------------------------------------------------------------------
* WORK: -
* ..-NOP3
*-------------------------------------------------------------------------
nop3            
                addq.l  #2,RPC
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* a3-LDA (ZP,X) & TAX
*-------------------------------------------------------------------------
ldaizpxtax      
                addrizpx
                memreadnt RA,6
                move.b  RA,RX
                move.w  CCR,RP
                next 6

*-------------------------------------------------------------------------
* WORK: d0,d1
* b3-LDA (ZP),Y & TAX
*-------------------------------------------------------------------------
ldaizpytax      
                addrizpy
                memreadnt RA,5
                move.b  RA,RX
                move.w  CCR,RP
                next 5

*-------------------------------------------------------------------------
* WORK: d0,d1
* af-LDA ABS & TAX
*-------------------------------------------------------------------------
_laxabs         addrabs
                memreadnt RA,4
                move.b  RA,RX
                move.w  CCR,RP
                next 4
                
*-------------------------------------------------------------------------
* WORK: d0,d1
* bb-???
*-------------------------------------------------------------------------
lasabsy         addrabsy
                memreadnt d0,4
                swap    RSP
                and.b   RSP,d0
                swap    RSP
                move.b  d0,RX
                move.b  d0,RA
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* bf-LDA ABS,Y & TAX
*-------------------------------------------------------------------------
ldaabsytax
                addrabsy
                memreadnt RA,4
                move.b  RA,RX
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* a7-LDA ZP & TAX
*-------------------------------------------------------------------------
ldazptax        
                addrzp
                memreadzp RA
                move.b  RA,RX
                move.w  CCR,RP
                next 3

*-------------------------------------------------------------------------
* WORK: -
* ab-LXA #BYTE
*-------------------------------------------------------------------------
_lxabyte        addrbyte d0
                or.b    #$ee,RA
                and.b   d0,RA
                move.w  CCR,RP
                move.b  RA,RX
                next 2

*-------------------------------------------------------------------------
* WORK: d0,d1
* b7-LDA ZP,Y & TAX
*-------------------------------------------------------------------------
ldazpytax       
                addrzpy
                memreadzp RA
                move.b  RA,RX
                move.w  CCR,RP
                next 4

*-------------------------------------------------------------------------
* WORK: d0,d1
* c3-DEC (ZP,X) & CMP (ZP,X)
*-------------------------------------------------------------------------
decizpxcmp      
                addrizpx
                memreadzp d0
                sdec d0
                scmp d0
                memwritezp d0,1,8

*-------------------------------------------------------------------------
* WORK: d0,d1
* d3-DEC (ZP),Y & CMP (ZP),Y
*-------------------------------------------------------------------------
decizpycmp      
                addrizpy
                memreadzp d0
                sdec d0
                scmp d0
                memwritezp d0,1,8

*-------------------------------------------------------------------------
* WORK: d0,d1
* cf-DEC ABS & CMP ABS
*-------------------------------------------------------------------------
decabscmp       
                addrabs
                memreadnt d0,6
                sdec d0
                scmp d0
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: d0,d1
* df-DEC ABS,X & CMP ABS,X
*-------------------------------------------------------------------------
decabsxcmp      
                addrabsx
                memreadnt d0,6
                sdec d0
                scmp d0
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: d0,d1
* db-DEC ABS,Y & CMP ABS,Y
*-------------------------------------------------------------------------
_dcpabsy
                addrabsy
                memreadnt d0,7
                subq.b  #1,d0
                cmp.b   d0,RA
                bmi     .skip
                st      RSP
.skip           move.w  d0,-(sp)        ; save data (for writing)
                neg.b   d0
                add.b   RA,d0           ; d0 = RA - d0
                move.w  CCR,RP          ; set nz
                move.w  (sp)+,d0        ; restore data to write
                memwrite d0,1,7

*-------------------------------------------------------------------------
* WORK: d0,d1
* d7-DEC ZP,X & CMP ZP,X
*-------------------------------------------------------------------------
deczpxcmp       
                addrzpx
                memreadzp d0
                sdec d0
                scmp d0
                memwritezp d0,1,6

*-------------------------------------------------------------------------
* WORK: d0,d1
* c7-DEC ZP & CMP ZP
*-------------------------------------------------------------------------
deczpcmp        
                addrzp
                memreadzp d0
                sdec d0
                scmp d0
                memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: d0,d1
* e3-INC (ZP,X) & SBC (ZP,X)
*-------------------------------------------------------------------------
incizpxsbcbin   
                addrizpx
                memreadzp d0
                sinc d0
                ssbcbin
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* e3-INC (ZP,X) & SBC (ZP,X)
*-------------------------------------------------------------------------
incizpxsbcdec   
                addrizpx
                memreadzp d0
                sinc d0
                ssbcdec
                memwritezp d0,1,8

*-------------------------------------------------------------------------
* WORK: d0,d1
* f3-INC (ZP),Y & SBC (ZP),Y
*-------------------------------------------------------------------------
incizpysbcbin   
                addrizpy
                memreadzp d0
                sinc d0
                ssbcbin
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* f3-INC (ZP),Y & SBC (ZP),Y
*-------------------------------------------------------------------------
incizpysbcdec   
                addrizpy
                memreadzp d0
                sinc d0
                ssbcdec
                memwritezp d0,1,8

*-------------------------------------------------------------------------
* WORK: d0,d1
* ef-INC ABS & SBC ABS
*-------------------------------------------------------------------------
_isbabsbin      addrabs
                memreadnt d0,6
                addq.b  #1,d0
                move.w  d0,-(sp)        ; store value for writing
                ssbcbin
                move.w  (sp)+,d0        ; restore
                memwrite d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* ef-INC ABS & SBC ABS
*-------------------------------------------------------------------------
_isbabsdec      addrabs
                memreadnt d0,6
                addq.b  #1,d0
                move.w  d0,-(sp)        ; store value for writing
                ssbcdec
                move.w  (sp)+,d0        ; restore
                memwrite d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* ff-INC ABS,X & SBC ABS,X
*-------------------------------------------------------------------------
incabsxsbcbin   
                addrabsx
                memreadnt d0,7
                addq.b  #1,d0
                move.w  d0,-(sp)        ; store value for writing
                ssbcbin
                move.w  (sp)+,d0        ; restore
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* ff-INC ABS,X & SBC ABS,X
*-------------------------------------------------------------------------
incabsxsbcdec   
                addrabsx
                memreadnt d0,7
                addq.b  #1,d0
                move.w  d0,-(sp)        ; store value for writing
                ssbcdec
                move.w  (sp)+,d0        ; restore
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* fb-INC ABS,Y & SBC ABS,Y
*-------------------------------------------------------------------------
incabsysbcbin   
                addrabsy
                memreadnt d0,7
                sinc d0
                ssbcbin
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* fb-INC ABS,Y & SBC ABS,Y
*-------------------------------------------------------------------------
incabsysbcdec   
                addrabsy
                memreadnt d0,7
                sinc d0
                ssbcdec
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* f7-INC ZP,X & SBC ZP,X
*-------------------------------------------------------------------------
inczpxsbcbin    
                addrzpx
                memreadzp d0
                sinc d0
                ssbcbin
                memwritezp d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* f7-INC ZP,X & SBC ZP,X
*-------------------------------------------------------------------------
inczpxsbcdec    
                addrzpx
                memreadzp d0
                sinc d0
                ssbcdec
                memwritezp d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* e7-INC ZP,X & SBC ZP,X
*-------------------------------------------------------------------------
inczpsbcbin     
                addrzp
                memreadzp d0
                sinc d0
                ssbcbin
                memwritezp d0,1,5
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* e7-INC ZP,X & SBC ZP,X
*-------------------------------------------------------------------------
inczpsbcdec     
                addrzp
                memreadzp d0
                sinc d0
                ssbcdec
                memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: d0,d1
* 23-ROL (ZP,X) & AND
*-------------------------------------------------------------------------
rolizpxand      
                addrizpx
                memreadzp d0
                srol d0
                sand d0
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1
* 33-ROL (ZP),Y & AND
*-------------------------------------------------------------------------
rolizpyand      
                addrizpy
                memreadzp d0
                srol d0
                sand d0
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1
* 2f-ROL ABS & AND
*-------------------------------------------------------------------------
rolabsand       
                addrabs
                memreadnt d0,6
                srol d0
                sand d0
                memwrite d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* 3f-ROL ABS,X & AND
*-------------------------------------------------------------------------
rolabsxand      
                addrabsx
                memreadnt d0,7
                srol d0
                sand d0
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* 3b-ROL ABS,Y & AND
*-------------------------------------------------------------------------
rolabsyand      
                addrabsy
                memreadnt d0,7
                srol d0
                sand d0
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* 37-ROL ZP,X & AND
*-------------------------------------------------------------------------
rolzpxand       
                addrzpx
                memreadzp d0
                srol d0
                sand d0
                memwritezp d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* 27-ROL ZP & AND
*-------------------------------------------------------------------------
rolzpand        
                addrzp
                memreadzp d0
                srol d0
                sand d0
                memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: d0,d1
* 63-ROR (ZP,X) & ADC
*-------------------------------------------------------------------------
rorizpxadcbin   
                addrizpx
                memreadzp d0
                sror d0
                sadcbin
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 63-ROR (ZP,X) & ADC
*-------------------------------------------------------------------------
rorizpxadcdec   
                addrizpx
                memreadzp d0
                sror d0
                sadcdec
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1
* 73-ROR (ZP),Y & ADC
*-------------------------------------------------------------------------
rorizpyadcbin   
                addrizpy
                memreadzp d0
                sror d0
                sadcbin
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 73-ROR (ZP),Y & ADC
*-------------------------------------------------------------------------
rorizpyadcdec   
                addrizpy
                memreadzp d0
                sror d0
                sadcdec
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1
* 6f-ROR ABS & ADC
*-------------------------------------------------------------------------
rorabsadcbin    
                addrabs
                memreadnt d0,6
                sror d0
                sadcbin
                memwrite d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 6f-ROR ABS & ADC
*-------------------------------------------------------------------------
rorabsadcdec    
                addrabs
                memreadnt d0,6
                sror d0
                sadcdec
                memwrite d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* 7f-ROR ABS,X & ADC
*-------------------------------------------------------------------------
rorabsxadcbin   
                addrabsx
                memreadnt d0,7
                sror d0
                sadcbin
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 7f-ROR ABS,X & ADC
*-------------------------------------------------------------------------
rorabsxadcdec   
                addrabsx
                memreadnt d0,7
                sror d0
                sadcdec
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* 7b-ROR ABS,Y & ADC
*-------------------------------------------------------------------------
rorabsyadcbin   
                addrabsy
                memreadnt d0,7
                sror d0
                sadcbin
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 7b-ROR ABS,Y & ADC
*-------------------------------------------------------------------------
rorabsyadcdec   
                addrabsy
                memreadnt d0,7
                sror d0
                sadcdec
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* 77-ROR ZP,X & ADC
*-------------------------------------------------------------------------
rorzpxadcbin    
                addrzpx
                memreadzp d0
                sror d0
                sadcbin
                memwritezp d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 77-ROR ZP,X & ADC
*-------------------------------------------------------------------------
rorzpxadcdec    
                addrzpx
                memreadzp d0
                sror d0
                sadcdec
                memwritezp d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* 67-ROR ZP & ADC
*-------------------------------------------------------------------------
rorzpadcbin     
                addrzp
                memreadzp d0
                sror d0
                sadcbin
                memwritezp d0,1,5
*-------------------------------------------------------------------------
* WORK: d0,d1 DECIMAL
* 67-ROR ZP & ADC
*-------------------------------------------------------------------------
rorzpadcdec     
                addrzp
                memreadzp d0
                sror d0
                sadcdec
                memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: d0,d1
* 03-ASL (ZP,X) & ORA
*-------------------------------------------------------------------------
aslizpxora      
                addrizpx
                memreadzp d0
                sasl d0
                sora d0
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1
* 13-ASL (ZP),Y & ORA
*-------------------------------------------------------------------------
aslizpyora      
                addrizpy
                memreadzp d0
                sasl d0
                sora d0
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK:
* 6b-???
*-------------------------------------------------------------------------
arrbyte         addrbyte RA
                sror RA
                next 2
*-------------------------------------------------------------------------
* WORK: d0,d1
* 0f-ASL ABS & ORA
*-------------------------------------------------------------------------
aslabsora       
                addrabs
                memreadnt d0,6
                sasl d0
                sora d0
                memwrite d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* 1f-ASL ABS,X & ORA
*-------------------------------------------------------------------------
aslabsxora      
                addrabsx
                memreadnt d0,7
                sasl d0
                sora d0
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* 1b-ASL ABS,Y & ORA
*-------------------------------------------------------------------------
aslabsyora      
                addrabsy
                memreadnt d0,7
                sasl d0
                sora d0
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* 17-ASL ZP,X & ORA
*-------------------------------------------------------------------------
aslzpxora       
                addrzpx
                memreadzp d0
                sasl d0
                sora d0
                memwritezp d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* 07-ASL ZP & ORA
*-------------------------------------------------------------------------
aslzpora        
                addrzp
                memreadzp d0
                sasl d0
                sora d0
                memwritezp d0,1,5
*-------------------------------------------------------------------------
* WORK: d0,d1
* 43-LSR (ZP,X) & EOR
*-------------------------------------------------------------------------
lsrizpxeor      
                addrizpx
                memreadzp d0
                slsr d0
                seor d0
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1
* 53-LSR (ZP),Y & EOR
*-------------------------------------------------------------------------
lsrizpyeor      
                addrizpy
                memreadzp d0
                slsr d0
                seor d0
                memwritezp d0,1,8
*-------------------------------------------------------------------------
* WORK: d0,d1
* 4f-LSR ABS & EOR
*-------------------------------------------------------------------------
lsrabseor       
                addrabs
                memreadnt d0,6
                slsr d0
                seor d0
                memwrite d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* 5f-LSR ABS,X & EOR
*-------------------------------------------------------------------------
lsrabsxeor      
                addrabsx
                memreadnt d0,7
                slsr d0
                seor d0
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* 5b-LSR ABS,Y & EOR
*-------------------------------------------------------------------------
lsrabsyeor      
                addrabsy
                memreadnt d0,7
                slsr d0
                seor d0
                memwrite d0,1,7
*-------------------------------------------------------------------------
* WORK: d0,d1
* 57-LSR ZP,X & EOR
*-------------------------------------------------------------------------
lsrzpxeor       
                addrzpx
                memreadzp d0
                slsr d0
                seor d0
                memwritezp d0,1,6
*-------------------------------------------------------------------------
* WORK: d0,d1
* 47-LSR ZP & EOR
*-------------------------------------------------------------------------
lsrzpeor        
                addrzp
                memreadzp d0
                slsr d0
                seor d0
                memwritezp d0,1,5

*-------------------------------------------------------------------------
* WORK: 
* 4b-AND #BYTE & LSR
*-------------------------------------------------------------------------
andbytelsr      
                addrbyte d0
                sand d0
                slsr RA
                next 2
*-------------------------------------------------------------------------
* WORK: 
* 83-AND A,X --> (ZP,X)
*-------------------------------------------------------------------------
andaxizpx       
                addrizpx
                move.b  RA,d0
                and.b   RX,d0
                move.w  CCR,RP
                memwrite d0,1,6
*-------------------------------------------------------------------------
* WORK: 
* 87-AND A,X --> ZP
*-------------------------------------------------------------------------
andaxzp         
                addrzp
                move.b  RA,d0
                and.b   RX,d0
                move.w  CCR,RP
                memwritezp d0,1,3
*-------------------------------------------------------------------------
* WORK: 
* 8b-ANE #BYTE
*-------------------------------------------------------------------------
_anebyte
                addrbyte d0
                or.b    #$ee,RA
                and.b   RX,RA
                and.b   d0,RA
                move.w  CCR,RP
                next 2
*-------------------------------------------------------------------------
* WORK: 
* 8f-AND A,X --> ABS
*-------------------------------------------------------------------------
_saxabs 
                addrabs
                move.b  RA,d0
                and.b   RX,d0
                move.w  CCR,RP
                memwrite d0,1,4
*-------------------------------------------------------------------------
* WORK: 
* 97-AND A,X --> ZP,X
*-------------------------------------------------------------------------
andaxzpx        
                addrzpx
                move.b  RA,d0
                and.b   RX,d0
                move.w  CCR,RP
                memwritezp d0,1,4
*-------------------------------------------------------------------------
* WORK: 
* 9c-AND Y,#BYTE+1 --> ABS,X
*-------------------------------------------------------------------------
_shyabsx        
                addq.l  #1,RPC
                move.b  (RPC),d0
                subq.l  #1,RPC
                addrabsx
                addq.b  #1,d0
                and.b   RY,d0
                move.w  CCR,RP
                memwrite d0,1,5
*-------------------------------------------------------------------------
* WORK: 
* 9e-AND X,#BYTE+1 --> ABS,Y
*-------------------------------------------------------------------------
_shxabsy
                addq.l  #1,RPC
                move.b  (RPC),d0
                subq.l  #1,RPC
                addrabsy
                addq.b  #1,d0
                and.b   RX,d0
                move.w  CCR,RP
                memwrite d0,1,5
*-------------------------------------------------------------------------
* WORK: 
* 93-9f-AND A,X,#BYTE+1 --> ABS,Y
*-------------------------------------------------------------------------
_shaabsy
                addq.l  #1,RPC
                move.b  (RPC),d0
                subq.l  #1,RPC
                addrabsy
                addq.b  #1,d0
                and.b   RX,d0
                and.b   RA,d0
                move.w  CCR,RP
                memwrite d0,1,6
*-------------------------------------------------------------------------
* WORK: 
* 9b-SHA & TXS (X-> (A&X)
*-------------------------------------------------------------------------
_shsabsy
                addq.l  #1,RPC
                move.b  (RPC),d0
                subq.l  #1,RPC
                addrabsy
                addq.b  #1,d0
                and.b   RX,d0
                and.b   RA,d0
                swap    RSP
                move.b  RX,RSP
                swap    RSP
                memwrite d0,1,6

*-------------------------------------------------------------------------
* WORK: 
* cb-AND A,X --> X - #BYTE
*-------------------------------------------------------------------------
andaxbyte       
                addrbyte d0
                move.b  RA,RX
                sub.b   d0,RX
                move.w  CCR,RP
                next 2

*-------------------------------------------------------------------------
* MAIN PRG (asmexecute(u_word from)) ha from==0 -> reset
* ret: 0-exit by TED
*      1-crash
*    más-illegal opcode fetched
*-------------------------------------------------------------------------
_asmexecute
                movem.l d2-d7/a2-a6,-(sp)
        IFD     STAT
                clr.l   STAR
                clr.l   STAC
        ENDIF
        IFD     STAT_IEC
                moveq   #-2,d0
                jsr     _iecmessage
        ENDIF
; set addresses of two different type of 1541 emulation
*               move.l  _opt_drive08,d0
*               move.b  d0,Addrs+8
*               move.l  _opt_drive09,d0         ; 0-none 1-iec 2-soft
*               move.b  d0,Addrs+9
*               move.l  _opt_drive10,d0
*               move.b  d0,Addrs+10
*               move.l  _opt_drive11,d0
*               move.b  d0,Addrs+11
*               clr.b   Act_Use

                moveq   #57,d0
                move.w  d0,cnt1actclk
                move.w  d0,cnt2actclk
                move.w  d0,cnt3actclk
                clr.l   RP
                move.l  #ctabdouble,clktab
                testiec                         ; test iec
* initialize 7501
                move.l  #tedhandletab,a0
                moveq   #$0e,d0
                move.b  #$ff,keyread
                setsndbit
;---------------------------------------------------
.tov            move.b  #1,charinram
                bset    #ACTRAMROM,RP
                move.w  #$ffff,initcnt1
                clr.w   counter1
                clr.w   counter2
                clr.w   counter3
                move.w  #109,actclk
                clr.l   CYC
                move.w  #109,CYC
                move.l  #romsp,a0
                move.l  _romsp0,(a0)+
                move.l  _romsp1,(a0)+
                move.l  _romsp2,(a0)+
                move.l  _romsp3,(a0)+
                move.l  _romsp4,(a0)+
                move.l  _romsp5,(a0)+
                move.l  _romsp6,(a0)+
                move.l  _romsp7,(a0)+
                clr.l   d0
                clr.l   d1
                moveq   #0,RA
                or.w    #F__,RA
                moveq   #0,RX
                move.l  _p4actras,RY
                swap    RY
                clr.w   RY
                moveq   #0,RSP
                move.w  #$01ff,RSP
                swap    RSP
                move.l  _myram,RAM
                move.l  _hiram,HIRAM
                move.l  HIRAM,RPC
                move.l  #fetchtable,FETCH
                and.l   #$0000ffff,d0
                move.l  _dogfx,DOGFX
                resetdogfx
                jsr     iec_reset
                jsr     jtabinit
                move.l  RAM,a1
                add.l   #$0000d400,a1
                shadowcvars
                clr.l   d0
                move.w  $fffc(HIRAM),d0         ; RESET
                rol.w   #8,d0
                add.l   d0,RPC
                clr.l   d1
                clr.l   d0
                move.b  (RPC)+,d0
                jsr     ([FETCH,d0.l*4])
                restorecvars
                move.b  RA,_r_a
                move.b  RX,_r_x
                move.b  RY,_r_y
                makerp
                rol.w   #8,RA
                move.b  RA,_r_p
                rol.w   #8,RA
                move.b  RSP,_r_sp
                move.w  RPC,_r_pc
        IFD     STAT_IEC
                move.l  d0,-(sp)
                moveq   #-1,d0
                jsr     _iecmessage
                move.l  (sp)+,d0
        ENDIF
                movem.l (sp)+,d2-d7/a2-a6
                rts

*-------------------------------------------------------------------------
* jtabinit      (a0,a1,d0)
*-------------------------------------------------------------------------
jtabinit        move.l  RAM,a0
                add.l   #$00035000,a0
                moveq   #$1f,d0
                move.l  #MemWrite_SID,a1        ; SID   d400-d41f
.1              move.l  a1,(a0)+
                dbf     d0,.1
                move.w  #$fcff-$d420,d0         ; $28df
                move.l  #MemWrite_MEM,a1        ; -     d420-fcff
.2              move.l  a1,(a0)+
                dbf     d0,.2
                moveq   #$f,d0
                move.l  #MemWrite_ACIA,a1       ; ACIA  fd00-fd0f
.3              move.l  a1,(a0)+
                dbf     d0,.3
                moveq   #$f,d0
                move.l  #MemWrite_PAR,a1        ; PAR   fd10-fd1f
.4              move.l  a1,(a0)+
                dbf     d0,.4
                moveq   #$f,d0
                move.l  #MemWrite_NONE,a1       ; -     fd20-fd2f
.5              move.l  a1,(a0)+
                dbf     d0,.5
                moveq   #$f,d0
                move.l  #MemWrite_KEY,a1        ; KEY   fd20-fd2f
.6              move.l  a1,(a0)+
                dbf     d0,.6
                move.w  #$008f,d0
                move.l  #MemWrite_NONE,a1       ; -     fd40-fdcf
.7              move.l  a1,(a0)+
                dbf     d0,.7
                moveq   #$f,d0
                move.l  #MemWrite_MAP,a1        ; MAP   fdd0-fddf
.8              move.l  a1,(a0)+
                dbf     d0,.8
                move.w  #$00df,d0
                move.l  #MemWrite_NONE,a1       ; -     fde0-febf
.9              move.l  a1,(a0)+
                dbf     d0,.9
                moveq   #$3f,d0
                move.l  #MemWrite_PIEC,a1       ; PIEC  fec0-feff
.10             move.l  a1,(a0)+
                dbf     d0,.10
                moveq   #$3f,d0
                move.l  #TedWriteTab,a1
.11             move.l  (a1)+,(a0)+             ; TED   ff00-ff3f
                dbf     d0,.11
                move.w  #$00bf,d0
                move.l  #MemWrite_NTED,a1       ; -     ff40-ffff
.12             move.l  a1,(a0)+
                dbf     d0,.12
                clr.l   d0
                rts

*=========================================================================
*-------------------------------------------------------------------------
* Memory Write routines         d0.b-adat       d1.w-addr
*-------------------------------------------------------------------------
*=========================================================================

MemWrite_MEM    move.b  d0,(RAM,d1.l)
                next2
*-------------------------------------------------------------------------

MemWrite_ACIA   next2
*-------------------------------------------------------------------------

MemWrite_PAR    next2
*-------------------------------------------------------------------------

MemWrite_KEY    move.b  d0,keyread
                next2
*-------------------------------------------------------------------------

MemWrite_NONE   next2
*-------------------------------------------------------------------------

;MemWrite_MAP   ; lásd feljebb
*-------------------------------------------------------------------------
;MemWrite_SID   ; lásd lejjebb

*-------------------------------------------------------------------------
*
* parallel IEC, or nothing (PARIEC: $FEC0-$FEFF) (1551 drive)
*  if 1551 isn't installed, this mem read as 0s! (.pariecR)
*
*   device 8: fee0-fee7 (fef0-fef7 « used by +4 rom)
*          9: fec0-fedf
*     # 9 8    REG  FUNCTION
*    ------------------------
*       c/f0:   A  - DATA
*       c/f1:   B  - STATUS     xxxxxxST -> ST 00:EOI
*       c/f2:   C  - HANDSHAKE  ADxxxxxx -> A-ACK D-DAV signal
*       c/f3:   dA - \
*       c/f4:   dB -  > Data direction register for A,B,C
*       c/f5:   dC - /   write $FF: +4 » 1551, $00: 1551 » +4
*
* From Minus4:
* INIT          equ     $55
* COMMAND1      equ     $81
* COMMAND2      equ     $82
* SENDBYTE      equ     $83
* READBYTE      equ     $84
*
MemWrite_PIEC   btst    #IEC,RP
                beq     .nope
                movem.l d1/a0-a1,-(sp)
                move.b  d0,(RAM,d1.l)
                jsr     _pin_PIEC_W             ; d0-adat d1-cím
                movem.l (sp)+,d1/a0-a1          ; ret:
                move.b  d0,(RAM,d1.l)
                clr.l   d0
                next2
.nope           move.b  #0,(RAM,d1.l)
                next2
*-------------------------------------------------------------------------

MemWrite_NTED   move.b  d0,(RAM,d1.l)
                next2
*-------------------------------------------------------------------------


*=========================================================================
*-------------------------------------------------------------------------
* SID handlers
*-------------------------------------------------------------------------
*=========================================================================

*-------------------------------------------------------------------------
* initsid
*-------------------------------------------------------------------------
_initsid        movem.l a1/a6,-(sp)
                tst.b   sidinited
                bne     .end
                st.b    sidinited
                move.l  _SysBase,a6
                move.l  _opt_sidtype,d0
                cmp.b   #2,d0
                beq     .a64_1

; try playsid.library FIRST
.psid_1         moveq   #0,d0
                lea     psname,a1
                LIBCALL OpenLibrary
                move.l  d0,_PlaySidBase
                bne     .initpsid
; try 6581sid.library
                moveq   #0,d0
                lea     sidname,a1
                LIBCALL OpenLibrary
                move.l  d0,_6581sidBase
                beq     .err
                bra     .init6581

; try 6581sid.library FIRST
.a64_1          moveq   #0,d0
                lea     sidname,a1
                LIBCALL OpenLibrary
                move.l  d0,_6581sidBase
                bne     .init6581
; try playsid.library
.trypsid        moveq   #0,d0
                lea     psname,a1
                LIBCALL OpenLibrary
                move.l  d0,_PlaySidBase
                beq     .err
* PLAYSID
.initpsid       move.l  d0,a6
                LIBCALL AllocEmulResource
                tst.l   d0
                bne     .cerr
                lea     pshd,a0
                move.l  a0,a1
                move.l  #pse-pshd,d0
                LIBCALL SetModule
                moveq   #1,d0
                LIBCALL StartSong
                tst.l   d0
                beq     .lastps
                LIBCALL FreeEmulResource        ; err
.cerr           move.l  a6,a1
                move.l  _SysBase,a6
                LIBCALL CloseLibrary
                clr.l   _PlaySidBase
                clr.l   _6581sidBase
.err            movem.l (sp)+,a1/a6
                move.l  #swdummy,a0
                move.l  a0,sidwritef
                moveq   #SIDTYPE_NONE,d0
                move.b  d0,sidtype
                moveq   #-1,d0
                clr.l   d1
                clr.b   sidinited
                rts
* 6581SID
.init6581       move.l  d0,a6                   ; get base
                LIBCALL SID_AllocSID
                move.l  d0,sidhandle
                beq     .cerr
                move.l  d0,a1
                st.b    sid_Enabled(a1)
                clr.b   sid_Filter(a1)
                st.b    sid_60Hz(a1)
                move.b  #48,sid_RingQual(a1)
                move.b  #48,sid_SyncQual(a1)
                move.b  #48,sid_ADSRQual(a1)
                move.w  #60,sid_IRQRate(a1)
                LIBCALL SID_Initialize
                move.l  sidhandle,a1
                moveq   #0,d0
                LIBCALL SID_IRQOnOff
                move.l  _SysBase,a6
                lea     vblankirq,a1
                moveq   #INTB_VERTB,d0
                LIBCALL AddIntServer
.last65         move.l  #sw65,a0
                move.l  a0,sidwritef
                moveq   #SIDTYPE_6581,d0
                move.b  d0,sidtype
                bra     .end

.lastps         ;bset   #1,$bfe001              ;Filter out
                move.l  #swps,a0
                move.l  a0,sidwritef
                moveq   #SIDTYPE_PSID,d0
                move.b  d0,sidtype
.end            movem.l (sp)+,a1/a6
                bsr     sidreset
                clr.l   d0
                clr.l   d1
                rts

*-------------------------------------------------------------------------
* irqroutine
*-------------------------------------------------------------------------
irqroutine      move.l  _6581sidBase,a6
                LIBCALL SID_Interrupt
                clr.l   d0
                rts


*-------------------------------------------------------------------------
* endsid
*-------------------------------------------------------------------------
_endsid         movem.l a1/a6,-(sp)
                tst.b   sidinited
                beq     .end
                bsr     _sidresume
                move.b  sidtype,d0
                cmp.b   #SIDTYPE_PSID,d0
                bne     .end65
* PLAYSID
.endps          move.l  _PlaySidBase,a6
                LIBCALL StopSong
                LIBCALL FreeEmulResource
                move.l  a6,a1
                move.l  _SysBase,a6
                LIBCALL CloseLibrary
                clr.l   _PlaySidBase
                bra     .last
* 6581SID
.end65          move.l  sidhandle,d0
                beq     .last
                move.l  _6581sidBase,a6
                move.l  d0,a1
                moveq   #-1,d0
                LIBCALL SID_IRQOnOff
                move.l  _SysBase,a6
                lea     vblankirq,a1
                moveq   #INTB_VERTB,d0
                LIBCALL RemIntServer
                move.l  _6581sidBase,a6
                move.l  sidhandle,a1            ;Und SIDHandle freigeben
                LIBCALL SID_FreeSID
                clr.l   sidhandle
                move.l  a6,a1
                move.l  _SysBase,a6
                LIBCALL CloseLibrary
                clr.l   _6581sidBase

.last           ;bclr   #1,$bfe001              ;Filter on
                clr.b   sidinited
                move.l  #swdummy,a0
                move.l  a0,sidwritef
                moveq   #SIDTYPE_NONE,d0
                move.b  d0,sidtype
.end            movem.l (sp)+,a1/a6
                clr.l   d0
                rts

*-------------------------------------------------------------------------
* sidpause
*-------------------------------------------------------------------------
_sidpause       movem.l a1/a6,-(sp)
                tst.b   sidinited
                beq     .end
                tst.b   sidpaused
                bne     .end
                move.b  sidtype,d0
                cmp.b   #SIDTYPE_PSID,d0
                bne     .sp65
* PLAYSID
.spps           move.l  _PlaySidBase,a6
                LIBCALL PauseSong
                bra     .last
* 6581SID
.sp65           move.l  sidhandle,d0
                move.l  _6581sidBase,a6
                move.l  d0,a1
                moveq   #-1,d0
                LIBCALL SID_IRQOnOff

.last           st.b    sidpaused
.end            movem.l (sp)+,a1/a6
                rts

*-------------------------------------------------------------------------
* sidresume
*-------------------------------------------------------------------------
_sidresume      movem.l a1/a6,-(sp)
                tst.b   sidinited
                beq     .end
                tst.b   sidpaused
                beq     .end
                move.b  sidtype,d0
                cmp.b   #SIDTYPE_PSID,d0
                bne     .sr65
* PLAYSID
.srps           move.l  _PlaySidBase,a6
                LIBCALL ContinueSong
                bra     .last
* 6581SID
.sr65           move.l  sidhandle,d0
                move.l  _6581sidBase,a6
                move.l  d0,a1
                moveq   #0,d0
                LIBCALL SID_IRQOnOff

.last           clr.b   sidpaused
.end            movem.l (sp)+,a1/a6
                rts

*-------------------------------------------------------------------------
* sidwrite      d1.b-sidreg, d0.b-érték (d1.w-addr)
*-------------------------------------------------------------------------
MemWrite_SID
sidwrite        move.b  d0,(RAM,d1.l)
                jmp     ([sidwritef,pc])
* NOSID
swdummy         next2
* PLAYSID
swps            movem.l a1/a6,-(sp)
                move.l  _PlaySidBase,a6
                move.l  $15a(a6),a0     ;Code in C64-RAM of playsid.library
                move.b  d0,$1002(a0)
                move.b  d1,$1004(a0)
                moveq   #1,d0                   ; call player 1x
                LIBCALL ForwardSong
                movem.l (sp)+,a1/a6
                clr.l   d0
                clr.l   d1
                next2
* 6581SID
sw65            movem.l a1/a6,-(sp)
                and.w   #$00ff,d1
                exg     d0,d1
                move.l  _6581sidBase,a6
                move.l  sidhandle,a1            ;d0: Regnum, d1: Byte
                LIBCALL SID_WriteReg
                movem.l (sp)+,a1/a6
                clr.l   d0
                clr.l   d1
                next2

*-------------------------------------------------------------------------
* sidreset
*-------------------------------------------------------------------------
sidreset        movem.l d0-d1/a0-a1/a6,-(sp)
                tst.b   sidinited
                beq     .end
                bsr     _sidresume
                move.b  sidtype,d0
                cmp.b   #SIDTYPE_PSID,d0
                bne     .sres65
* PLAYSID
.sresps         move.l  _PlaySidBase,a6
                LIBCALL StopSong
                moveq   #1,d0
                LIBCALL StartSong
                bra     .end
* 6581SID
.sres65         move.l  sidhandle,d0
                move.l  _6581sidBase,a6
                move.l  d0,a1
                LIBCALL SID_ResetSID

.end            movem.l (sp)+,d0-d1/a0-a1/a6
                rts

*========================================================================{
        cnop    0,4
initcnt1        dc.w    $ffff
counter1        dc.w    0
counter2        dc.w    0
counter3        dc.w    0

        cnop    0,4
actclk          dc.w    0
cnt1actclk      dc.w    0
cnt2actclk      dc.w    0
cnt3actclk      dc.w    0

        cnop    0,4
romsp           dc.l    0,0,0,0,0,0,0,0

        cnop    0,4
clktab          dc.l    0

* SID
        cnop    0,4
sidpaused       dc.b    0
sidinited       dc.b    0
sidtype         dc.b    0               ; 0-semmi, 1-playsid, 2-6581sid
psname          PLAYSIDNAME
sidname         dc.b    "6581sid.library",0
        cnop    0,4
sidwritef       dc.l    swdummy         ; függvénymutató a sidtype szerint
_PlaySidBase    dc.l    0
_6581sidBase    dc.l    0
        cnop    0,4
vblankirq       dc.l    0,0
                dc.b    NT_INTERRUPT,0
                dc.l    irqname
sidhandle       dc.l    0
                dc.l    irqroutine
irqname         dc.b    "cp4 for 6581sid",0
        cnop    0,4
pshd            dc.b    "PSID"                          ; playsid.library
                dc.w    2
                dc.w    pshde-pshd
                dc.w    $1000                           ;Load
                dc.w    $1000                           ;Init
                dc.w    $1001                           ;Play
                dc.w    1
                dc.w    1
                dc.l    0
                ds.b    HEADERINFO_SIZE
                ds.b    HEADERINFO_SIZE
                ds.b    HEADERINFO_SIZE
                dc.w    0
                dc.l    0
pshde                                   ;Init:
                dc.b    $60             ; RTS
                                        ;Play:
                dc.b    $a9,0           ; LDA #xx
                dc.b    $8d,0,$d4       ; STA $D4xx
                dc.b    $60             ; RTS
pse

        cnop    0,4
TedWriteTab
tedhandletab
        dc.l    TED_00          * CNT1LO
        dc.l    TED_01          * CNT1HI
        dc.l    TED_02          * CNT2LO
        dc.l    TED_03          * CNT2HI
        dc.l    TED_04          * CNT3LO
        dc.l    TED_05          * CNT3HI
        dc.l    TED_06          * GFX1
        dc.l    TED_07          * GFX2
        dc.l    TED_08          * KEYREAD
        dc.l    TED_09          * IRQFLAG
        dc.l    TED_0a          * IRQENABLE
        dc.l    TED_0b          * RASTER
        dc.l    TED_0c          * CURPOS1
        dc.l    TED_0d          * CURPOS2
        dc.l    TED_0e          * SND1FREQ1     SND
        dc.l    TED_0f          * SND2FREQ1     SND
        dc.l    TED_10          * SND2FREQ2     SND
        dc.l    TED_11          * SNDCTRL       SND
        dc.l    TED_12          * GFX3
        dc.l    TED_13          * CHARGEN
        dc.l    TED_14          * VIDEO-MATRIX
        dc.l    TED_15          * COL0
        dc.l    TED_16          * COL1
        dc.l    TED_17          * COL2
        dc.l    TED_18          * COL3
        dc.l    TED_19          * COL4 (BORDER)
        dc.l    TED_1a          * FIRSTCHARADDR1
        dc.l    TED_1b          * FIRSTCHARADDR2
        dc.l    TED_1c          * ACTRASTER1
        dc.l    TED_1d          * ACTRASTER2
        dc.l    TED_1e          * HRAS
        dc.l    TED_1f          * FLASH
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_NOREG
        dc.l    TED_3e          * SWITCH TO ROM
        dc.l    TED_3f          * SWITCH TO RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM
        dc.l    TED_RAM

tedreadtab
        dc.l    TED_00R         * CNT1LO
        dc.l    TED_01R         * CNT1HI
        dc.l    TED_02R         * CNT2LO
        dc.l    TED_03R         * CNT2HI
        dc.l    TED_04R         * CNT3LO
        dc.l    TED_05R         * CNT3HI
        dc.l    TEDREAD         * GFX1
        dc.l    TEDREAD         * GFX2
        dc.l    TEDREAD         * KEYREAD
        dc.l    TEDREAD         * IRQFLAG
        dc.l    TEDREAD         * IRQENABLE
        dc.l    TEDREAD         * RASTER
        dc.l    TEDREAD         * CURPOS1
        dc.l    TEDREAD         * CURPOS2
        dc.l    TEDREAD         * SND1FREQ1
        dc.l    TEDREAD         * SND2FREQ1
        dc.l    TEDREAD         * SND2FREQ2
        dc.l    TEDREAD         * SNDCTRL
        dc.l    TEDREAD         * GFX3
        dc.l    TEDREAD         * CHARGEN
        dc.l    TEDREAD         * VIDEO-MATRIX
        dc.l    TEDREAD         * COL0
        dc.l    TEDREAD         * COL1
        dc.l    TEDREAD         * COL2
        dc.l    TEDREAD         * COL3
        dc.l    TEDREAD         * COL4 (BORDER)
        dc.l    TEDREAD         * FIRSTCHARADDR1
        dc.l    TEDREAD         * FIRSTCHARADDR2
        dc.l    TED_1cR         * ACTRASTER1
        dc.l    TED_1dR         * ACTRASTER2
        dc.l    TED_1eR         * HRAS
        dc.l    TEDREAD         * FLASH
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_NOREGR
        dc.l    TED_ROMSWR      * SWITCH TO ROM
        dc.l    TED_ROMSWR      * SWITCH TO RAM
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR
        dc.l    TED_RAMR

fetchtable
        dc.l    brk             * 0
        dc.l    oraizpx         * 1
        dc.l    breakpoint      * 2
        dc.l    aslizpxora      * 3
        dc.l    nop2            * 4
        dc.l    orazp           * 5
        dc.l    aslzp           * 6
        dc.l    aslzpora        * 7
        dc.l    php             * 8
        dc.l    orabyte         * 9
        dc.l    aslaccu         * a
        dc.l    andzpcn         * b
        dc.l    nop3            * c
        dc.l    oraabs          * d
        dc.l    aslabs          * e
        dc.l    aslabsora       * f
        dc.l    bpl             *10
        dc.l    oraizpy         *11
        dc.l    crash           *12
        dc.l    aslizpyora      *13
        dc.l    nop2            *14
        dc.l    orazpx          *15
        dc.l    aslzpx          *16
        dc.l    aslzpxora       *17
        dc.l    clc             *18
        dc.l    oraabsy         *19
        dc.l    nop             *1a
        dc.l    aslabsyora      *1b
        dc.l    nop3            *1c
        dc.l    oraabsx         *1d
        dc.l    aslabsx         *1e
        dc.l    aslabsxora      *1f
        dc.l    jsrabs          *20
        dc.l    andizpx         *21
        dc.l    crash           *22
        dc.l    rolizpxand      *23
        dc.l    bitzp           *24
        dc.l    andzp           *25
        dc.l    rolzp           *26
        dc.l    rolzpand        *27
        dc.l    plp             *28
        dc.l    andbyte         *29
        dc.l    rolaccu         *2a
        dc.l    andzpcn         *2b
        dc.l    bitabs          *2c
        dc.l    andabs          *2d
        dc.l    rolabs          *2e
        dc.l    rolabsand       *2f
        dc.l    bmi             *30
        dc.l    andizpy         *31
        dc.l    crash           *32
        dc.l    rolizpyand      *33
        dc.l    nop2            *34
        dc.l    andzpx          *35
        dc.l    rolzpx          *36
        dc.l    rolzpxand       *37
        dc.l    sec             *38
        dc.l    andabsy         *39
        dc.l    nop             *3a
        dc.l    rolabsyand      *3b
        dc.l    nop3            *3c
        dc.l    andabsx         *3d
        dc.l    rolabsx         *3e
        dc.l    rolabsxand      *3f
        dc.l    rti             *40
        dc.l    eorizpx         *41
        dc.l    crash           *42
        dc.l    lsrizpxeor      *43
        dc.l    nop2            *44
        dc.l    eorzp           *45
        dc.l    lsrzp           *46
        dc.l    lsrzpeor        *47
        dc.l    pha             *48
        dc.l    eorbyte         *49
        dc.l    lsraccu         *4a
        dc.l    andbytelsr      *4b
        dc.l    jmpabs          *4c
        dc.l    eorabs          *4d
        dc.l    lsrabs          *4e
        dc.l    lsrabseor       *4f
        dc.l    bvc             *50
        dc.l    eorizpy         *51
        dc.l    crash           *52
        dc.l    lsrizpyeor      *53
        dc.l    nop2            *54
        dc.l    eorzpx          *55
        dc.l    lsrzpx          *56
        dc.l    lsrzpxeor       *57
        dc.l    cli             *58
        dc.l    eorabsy         *59
        dc.l    nop             *5a
        dc.l    lsrabsyeor      *5b
        dc.l    nop3            *5c
        dc.l    eorabsx         *5d
        dc.l    lsrabsx         *5e
        dc.l    lsrabsxeor      *5f
        dc.l    rts             *60
        dc.l    adcizpxbin      *61
        dc.l    crash           *62
        dc.l    rorizpxadcbin   *63
        dc.l    nop2            *64
        dc.l    adczpbin        *65
        dc.l    rorzp           *66
        dc.l    rorzpadcbin     *67
        dc.l    pla             *68
        dc.l    adcbytebin      *69
        dc.l    roraccu         *6a
        dc.l    arrbyte         *6b
        dc.l    jmpiabs         *6c
        dc.l    adcabsbin       *6d
        dc.l    rorabs          *6e
        dc.l    rorabsadcbin    *6f
        dc.l    bvs             *70
        dc.l    adcizpybin      *71
        dc.l    crash           *72
        dc.l    rorizpyadcbin   *73
        dc.l    nop2            *74
        dc.l    adczpxbin       *75
        dc.l    rorzpx          *76
        dc.l    rorzpxadcbin    *77
        dc.l    sei             *78
        dc.l    adcabsybin      *79
        dc.l    nop             *7a
        dc.l    rorabsyadcbin   *7b
        dc.l    nop3            *7c
        dc.l    adcabsxbin      *7d
        dc.l    rorabsx         *7e
        dc.l    rorabsxadcbin   *7f
        dc.l    nop2            *80
        dc.l    staizpx         *81
        dc.l    nop2            *82
        dc.l    andaxizpx       *83
        dc.l    styzp           *84
i_sta1  dc.l    stazpT          *85
        dc.l    stxzp           *86
        dc.l    andaxzp         *87
        dc.l    dey             *88
        dc.l    nop2            *89
        dc.l    txa             *8a
        dc.l    _anebyte        *8b
        dc.l    styabs          *8c
        dc.l    staabs          *8d
        dc.l    stxabs          *8e
        dc.l    _saxabs         *8f
        dc.l    bcc             *90
        dc.l    staizpy         *91
        dc.l    crash           *92
        dc.l    _shaabsy        *93
        dc.l    styzpx          *94
        dc.l    stazpx          *95
        dc.l    stxzpy          *96
        dc.l    andaxzpx        *97
        dc.l    tya             *98
        dc.l    staabsy         *99
        dc.l    txs             *9a
        dc.l    _shsabsy        *9b
        dc.l    _shyabsx        *9c
        dc.l    staabsx         *9d
        dc.l    _shxabsy        *9e
        dc.l    _shaabsy        *9f
        dc.l    ldybyte         *a0
        dc.l    ldaizpx         *a1
        dc.l    ldxbyte         *a2
        dc.l    ldaizpxtax      *a3
        dc.l    ldyzp           *a4
i_lda1  dc.l    ldazpT          *a5
        dc.l    ldxzp           *a6
        dc.l    ldazptax        *a7
        dc.l    tay             *a8
        dc.l    ldabyte         *a9
        dc.l    tax             *aa
        dc.l    _lxabyte        *ab
        dc.l    ldyabs          *ac
        dc.l    ldaabs          *ad
        dc.l    ldxabs          *ae
        dc.l    _laxabs         *af
        dc.l    bcs             *b0
        dc.l    ldaizpy         *b1
        dc.l    crash           *b2
        dc.l    ldaizpytax      *b3
        dc.l    ldyzpx          *b4
        dc.l    ldazpx          *b5
        dc.l    ldxzpy          *b6
        dc.l    ldazpytax       *b7
        dc.l    clv             *b8
        dc.l    ldaabsy         *b9
        dc.l    tsx             *ba
        dc.l    lasabsy         *bb
        dc.l    ldyabsx         *bc
        dc.l    ldaabsx         *bd
        dc.l    ldxabsy         *be
        dc.l    ldaabsytax      *bf
        dc.l    cpybyte         *c0
        dc.l    cmpizpx         *c1
        dc.l    nop2            *c2
        dc.l    decizpxcmp      *c3
        dc.l    cpyzp           *c4
        dc.l    cmpzp           *c5
        dc.l    deczp           *c6
        dc.l    deczpcmp        *c7
        dc.l    iny             *c8
        dc.l    cmpbyte         *c9
        dc.l    dex             *ca
        dc.l    andaxbyte       *cb
        dc.l    cpyabs          *cc
        dc.l    cmpabs          *cd
        dc.l    decabs          *ce
        dc.l    decabscmp       *cf
        dc.l    bne             *d0
        dc.l    cmpizpy         *d1
        dc.l    crash           *d2
        dc.l    decizpycmp      *d3
        dc.l    nop2            *d4
        dc.l    cmpzpx          *d5
        dc.l    deczpx          *d6
        dc.l    deczpxcmp       *d7
        dc.l    cld             *d8
        dc.l    cmpabsy         *d9
        dc.l    nop             *da
        dc.l    _dcpabsy        *db
        dc.l    nop3            *dc
        dc.l    cmpabsx         *dd
        dc.l    decabsx         *de
        dc.l    decabsxcmp      *df
        dc.l    cpxbyte         *e0
        dc.l    sbcizpxbin      *e1
        dc.l    nop2            *e2
        dc.l    incizpxsbcbin   *e3
        dc.l    cpxzp           *e4
        dc.l    sbczpbin        *e5
        dc.l    inczp           *e6
        dc.l    inczpsbcbin     *e7
        dc.l    inx             *e8
        dc.l    sbcbytebin      *e9
        dc.l    nop             *ea
        dc.l    sbcbytebin      *eb
        dc.l    cpxabs          *ec
        dc.l    sbcabsbin       *ed
        dc.l    incabs          *ee
        dc.l    _isbabsbin      *ef
        dc.l    beq             *f0
        dc.l    sbcizpybin      *f1
        dc.l    crash           *f2
        dc.l    incizpysbcbin   *f3
        dc.l    nop2            *f4
        dc.l    sbczpxbin       *f5
        dc.l    inczpx          *f6
        dc.l    inczpxsbcbin    *f7
        dc.l    sed             *f8
        dc.l    sbcabsybin      *f9
        dc.l    nop             *fa
        dc.l    incabsysbcbin   *fb
        dc.l    nop3            *fc
        dc.l    sbcabsxbin      *fd
        dc.l    incabsx         *fe
        dc.l    incabsxsbcbin   *ff

        CNOP 0,4

fetchtabledec
        dc.l    brk             * 0
        dc.l    oraizpx         * 1
        dc.l    breakpoint      * 2
        dc.l    aslizpxora      * 3
        dc.l    nop2            * 4
        dc.l    orazp           * 5
        dc.l    aslzp           * 6
        dc.l    aslzpora        * 7
        dc.l    php             * 8
        dc.l    orabyte         * 9
        dc.l    aslaccu         * a
        dc.l    andzpcn         * b
        dc.l    nop3            * c
        dc.l    oraabs          * d
        dc.l    aslabs          * e
        dc.l    aslabsora       * f
        dc.l    bpl             *10
        dc.l    oraizpy         *11
        dc.l    crash           *12
        dc.l    aslizpyora      *13
        dc.l    nop2            *14
        dc.l    orazpx          *15
        dc.l    aslzpx          *16
        dc.l    aslzpxora       *17
        dc.l    clc             *18
        dc.l    oraabsy         *19
        dc.l    nop             *1a
        dc.l    aslabsyora      *1b
        dc.l    nop3            *1c
        dc.l    oraabsx         *1d
        dc.l    aslabsx         *1e
        dc.l    aslabsxora      *1f
        dc.l    jsrabs          *20
        dc.l    andizpx         *21
        dc.l    crash           *22
        dc.l    rolizpxand      *23
        dc.l    bitzp           *24
        dc.l    andzp           *25
        dc.l    rolzp           *26
        dc.l    rolzpand        *27
        dc.l    plp             *28
        dc.l    andbyte         *29
        dc.l    rolaccu         *2a
        dc.l    andzpcn         *2b
        dc.l    bitabs          *2c
        dc.l    andabs          *2d
        dc.l    rolabs          *2e
        dc.l    rolabsand       *2f
        dc.l    bmi             *30
        dc.l    andizpy         *31
        dc.l    crash           *32
        dc.l    rolizpyand      *33
        dc.l    nop2            *34
        dc.l    andzpx          *35
        dc.l    rolzpx          *36
        dc.l    rolzpxand       *37
        dc.l    sec             *38
        dc.l    andabsy         *39
        dc.l    nop             *3a
        dc.l    rolabsyand      *3b
        dc.l    nop3            *3c
        dc.l    andabsx         *3d
        dc.l    rolabsx         *3e
        dc.l    rolabsxand      *3f
        dc.l    rti             *40
        dc.l    eorizpx         *41
        dc.l    crash           *42
        dc.l    lsrizpxeor      *43
        dc.l    nop2            *44
        dc.l    eorzp           *45
        dc.l    lsrzp           *46
        dc.l    lsrzpeor        *47
        dc.l    pha             *48
        dc.l    eorbyte         *49
        dc.l    lsraccu         *4a
        dc.l    andbytelsr      *4b
        dc.l    jmpabs          *4c
        dc.l    eorabs          *4d
        dc.l    lsrabs          *4e
        dc.l    lsrabseor       *4f
        dc.l    bvc             *50
        dc.l    eorizpy         *51
        dc.l    crash           *52
        dc.l    lsrizpyeor      *53
        dc.l    nop2            *54
        dc.l    eorzpx          *55
        dc.l    lsrzpx          *56
        dc.l    lsrzpxeor       *57
        dc.l    cli             *58
        dc.l    eorabsy         *59
        dc.l    nop             *5a
        dc.l    lsrabsyeor      *5b
        dc.l    nop3            *5c
        dc.l    eorabsx         *5d
        dc.l    lsrabsx         *5e
        dc.l    lsrabsxeor      *5f
        dc.l    rts             *60
        dc.l    adcizpxdec      *61
        dc.l    crash           *62
        dc.l    rorizpxadcdec   *63
        dc.l    nop2            *64
        dc.l    adczpdec        *65
        dc.l    rorzp           *66
        dc.l    rorzpadcdec     *67
        dc.l    pla             *68
        dc.l    adcbytedec      *69
        dc.l    roraccu         *6a
        dc.l    illegal         *6b
        dc.l    jmpiabs         *6c
        dc.l    adcabsdec       *6d
        dc.l    rorabs          *6e
        dc.l    rorabsadcdec    *6f
        dc.l    bvs             *70
        dc.l    adcizpydec      *71
        dc.l    crash           *72
        dc.l    rorizpyadcdec   *73
        dc.l    nop2            *74
        dc.l    adczpxdec       *75
        dc.l    rorzpx          *76
        dc.l    rorzpxadcdec    *77
        dc.l    sei             *78
        dc.l    adcabsydec      *79
        dc.l    nop             *7a
        dc.l    rorabsyadcdec   *7b
        dc.l    nop3            *7c
        dc.l    adcabsxdec      *7d
        dc.l    rorabsx         *7e
        dc.l    rorabsxadcdec   *7f
        dc.l    nop2            *80
        dc.l    staizpx         *81
        dc.l    nop2            *82
        dc.l    andaxizpx       *83
        dc.l    styzp           *84
i_sta2  dc.l    stazpT          *85
        dc.l    stxzp           *86
        dc.l    andaxzp         *87
        dc.l    dey             *88
        dc.l    nop2            *89
        dc.l    txa             *8a
        dc.l    _anebyte        *8b
        dc.l    styabs          *8c
        dc.l    staabs          *8d
        dc.l    stxabs          *8e
        dc.l    _saxabs         *8f
        dc.l    bcc             *90
        dc.l    staizpy         *91
        dc.l    crash           *92
        dc.l    _shaabsy        *93
        dc.l    styzpx          *94
        dc.l    stazpx          *95
        dc.l    stxzpy          *96
        dc.l    andaxzpx        *97
        dc.l    tya             *98
        dc.l    staabsy         *99
        dc.l    txs             *9a
        dc.l    _shsabsy        *9b
        dc.l    _shyabsx        *9c
        dc.l    staabsx         *9d
        dc.l    _shxabsy        *9e
        dc.l    _shaabsy        *9f
        dc.l    ldybyte         *a0
        dc.l    ldaizpx         *a1
        dc.l    ldxbyte         *a2
        dc.l    ldaizpxtax      *a3
        dc.l    ldyzp           *a4
i_lda2  dc.l    ldazpT          *a5
        dc.l    ldxzp           *a6
        dc.l    ldazptax        *a7
        dc.l    tay             *a8
        dc.l    ldabyte         *a9
        dc.l    tax             *aa
        dc.l    _lxabyte        *ab
        dc.l    ldyabs          *ac
        dc.l    ldaabs          *ad
        dc.l    ldxabs          *ae
        dc.l    _laxabs         *af
        dc.l    bcs             *b0
        dc.l    ldaizpy         *b1
        dc.l    crash           *b2
        dc.l    ldaizpytax      *b3
        dc.l    ldyzpx          *b4
        dc.l    ldazpx          *b5
        dc.l    ldxzpy          *b6
        dc.l    ldazpytax       *b7
        dc.l    clv             *b8
        dc.l    ldaabsy         *b9
        dc.l    tsx             *ba
        dc.l    illegal         *bb
        dc.l    ldyabsx         *bc
        dc.l    ldaabsx         *bd
        dc.l    ldxabsy         *be
        dc.l    ldaabsytax      *bf
        dc.l    cpybyte         *c0
        dc.l    cmpizpx         *c1
        dc.l    nop2            *c2
        dc.l    decizpxcmp      *c3
        dc.l    cpyzp           *c4
        dc.l    cmpzp           *c5
        dc.l    deczp           *c6
        dc.l    deczpcmp        *c7
        dc.l    iny             *c8
        dc.l    cmpbyte         *c9
        dc.l    dex             *ca
        dc.l    andaxbyte       *cb
        dc.l    cpyabs          *cc
        dc.l    cmpabs          *cd
        dc.l    decabs          *ce
        dc.l    decabscmp       *cf
        dc.l    bne             *d0
        dc.l    cmpizpy         *d1
        dc.l    crash           *d2
        dc.l    decizpycmp      *d3
        dc.l    nop2            *d4
        dc.l    cmpzpx          *d5
        dc.l    deczpx          *d6
        dc.l    deczpxcmp       *d7
        dc.l    cld             *d8
        dc.l    cmpabsy         *d9
        dc.l    nop             *da
        dc.l    _dcpabsy        *db
        dc.l    nop3            *dc
        dc.l    cmpabsx         *dd
        dc.l    decabsx         *de
        dc.l    decabsxcmp      *df
        dc.l    cpxbyte         *e0
        dc.l    sbcizpxdec      *e1
        dc.l    nop2            *e2
        dc.l    incizpxsbcdec   *e3
        dc.l    cpxzp           *e4
        dc.l    sbczpdec        *e5
        dc.l    inczp           *e6
        dc.l    inczpsbcdec     *e7
        dc.l    inx             *e8
        dc.l    sbcbytedec      *e9
        dc.l    nop             *ea
        dc.l    sbcbytedec      *eb
        dc.l    cpxabs          *ec
        dc.l    sbcabsdec       *ed
        dc.l    incabs          *ee
        dc.l    _isbabsdec      *ef
        dc.l    beq             *f0
        dc.l    sbcizpydec      *f1
        dc.l    crash           *f2
        dc.l    incizpysbcdec   *f3
        dc.l    nop2            *f4
        dc.l    sbczpxdec       *f5
        dc.l    inczpx          *f6
        dc.l    inczpxsbcdec    *f7
        dc.l    sed             *f8
        dc.l    sbcabsydec      *f9
        dc.l    nop             *fa
        dc.l    incabsysbcdec   *fb
        dc.l    nop3            *fc
        dc.l    sbcabsxdec      *fd
        dc.l    incabsx         *fe
        dc.l    incabsxsbcdec   *ff

fetchtableirq
        dc.l    irqhandle       * 0
        dc.l    irqhandle       * 1
        dc.l    irqhandle       * 2
        dc.l    irqhandle       * 3
        dc.l    irqhandle       * 4
        dc.l    irqhandle       * 5
        dc.l    irqhandle       * 6
        dc.l    irqhandle       * 7
        dc.l    irqhandle       * 8
        dc.l    irqhandle       * 9
        dc.l    irqhandle       * a
        dc.l    irqhandle       * b
        dc.l    irqhandle       * c
        dc.l    irqhandle       * d
        dc.l    irqhandle       * e
        dc.l    irqhandle       * f
        dc.l    irqhandle       *10
        dc.l    irqhandle       *11
        dc.l    irqhandle       *12
        dc.l    irqhandle       *13
        dc.l    irqhandle       *14
        dc.l    irqhandle       *15
        dc.l    irqhandle       *16
        dc.l    irqhandle       *17
        dc.l    irqhandle       *18
        dc.l    irqhandle       *19
        dc.l    irqhandle       *1a
        dc.l    irqhandle       *1b
        dc.l    irqhandle       *1c
        dc.l    irqhandle       *1d
        dc.l    irqhandle       *1e
        dc.l    irqhandle       *1f
        dc.l    irqhandle       *20
        dc.l    irqhandle       *21
        dc.l    irqcrash        *22
        dc.l    irqhandle       *23
        dc.l    irqhandle       *24
        dc.l    irqhandle       *25
        dc.l    irqhandle       *26
        dc.l    irqhandle       *27
        dc.l    irqhandle       *28
        dc.l    irqhandle       *29
        dc.l    irqhandle       *2a
        dc.l    irqhandle       *2b
        dc.l    irqhandle       *2c
        dc.l    irqhandle       *2d
        dc.l    irqhandle       *2e
        dc.l    irqhandle       *2f
        dc.l    irqhandle       *30
        dc.l    irqhandle       *31
        dc.l    irqcrash        *32
        dc.l    irqhandle       *33
        dc.l    irqhandle       *34
        dc.l    irqhandle       *35
        dc.l    irqhandle       *36
        dc.l    irqhandle       *37
        dc.l    irqhandle       *38
        dc.l    irqhandle       *39
        dc.l    irqhandle       *3a
        dc.l    irqhandle       *3b
        dc.l    irqhandle       *3c
        dc.l    irqhandle       *3d
        dc.l    irqhandle       *3e
        dc.l    irqhandle       *3f
        dc.l    irqhandle       *40
        dc.l    irqhandle       *41
        dc.l    irqcrash        *42
        dc.l    irqhandle       *43
        dc.l    irqhandle       *44
        dc.l    irqhandle       *45
        dc.l    irqhandle       *46
        dc.l    irqhandle       *47
        dc.l    irqhandle       *48
        dc.l    irqhandle       *49
        dc.l    irqhandle       *4a
        dc.l    irqhandle       *4b
        dc.l    irqhandle       *4c
        dc.l    irqhandle       *4d
        dc.l    irqhandle       *4e
        dc.l    irqhandle       *4f
        dc.l    irqhandle       *50
        dc.l    irqhandle       *51
        dc.l    irqcrash        *52
        dc.l    irqhandle       *53
        dc.l    irqhandle       *54
        dc.l    irqhandle       *55
        dc.l    irqhandle       *56
        dc.l    irqhandle       *57
        dc.l    irqhandle       *58
        dc.l    irqhandle       *59
        dc.l    irqhandle       *5a
        dc.l    irqhandle       *5b
        dc.l    irqhandle       *5c
        dc.l    irqhandle       *5d
        dc.l    irqhandle       *5e
        dc.l    irqhandle       *5f
        dc.l    irqhandle       *60
        dc.l    irqhandle       *61
        dc.l    irqcrash        *62
        dc.l    irqhandle       *63
        dc.l    irqhandle       *64
        dc.l    irqhandle       *65
        dc.l    irqhandle       *66
        dc.l    irqhandle       *67
        dc.l    irqhandle       *68
        dc.l    irqhandle       *69
        dc.l    irqhandle       *6a
        dc.l    irqhandle       *6b
        dc.l    irqhandle       *6c
        dc.l    irqhandle       *6d
        dc.l    irqhandle       *6e
        dc.l    irqhandle       *6f
        dc.l    irqhandle       *70
        dc.l    irqhandle       *71
        dc.l    irqcrash        *72
        dc.l    irqhandle       *73
        dc.l    irqhandle       *74
        dc.l    irqhandle       *75
        dc.l    irqhandle       *76
        dc.l    irqhandle       *77
        dc.l    irqhandle       *78
        dc.l    irqhandle       *79
        dc.l    irqhandle       *7a
        dc.l    irqhandle       *7b
        dc.l    irqhandle       *7c
        dc.l    irqhandle       *7d
        dc.l    irqhandle       *7e
        dc.l    irqhandle       *7f
        dc.l    irqhandle       *80
        dc.l    irqhandle       *81
        dc.l    irqhandle       *82
        dc.l    irqhandle       *83
        dc.l    irqhandle       *84
        dc.l    irqhandle       *85
        dc.l    irqhandle       *86
        dc.l    irqhandle       *87
        dc.l    irqhandle       *88
        dc.l    irqhandle       *89
        dc.l    irqhandle       *8a
        dc.l    irqhandle       *8b
        dc.l    irqhandle       *8c
        dc.l    irqhandle       *8d
        dc.l    irqhandle       *8e
        dc.l    irqhandle       *8f
        dc.l    irqhandle       *90
        dc.l    irqhandle       *91
        dc.l    irqcrash        *92
        dc.l    irqhandle       *93
        dc.l    irqhandle       *94
        dc.l    irqhandle       *95
        dc.l    irqhandle       *96
        dc.l    irqhandle       *97
        dc.l    irqhandle       *98
        dc.l    irqhandle       *99
        dc.l    irqhandle       *9a
        dc.l    irqhandle       *9b
        dc.l    irqhandle       *9c
        dc.l    irqhandle       *9d
        dc.l    irqhandle       *9e
        dc.l    irqhandle       *9f
        dc.l    irqhandle       *a0
        dc.l    irqhandle       *a1
        dc.l    irqhandle       *a2
        dc.l    irqhandle       *a3
        dc.l    irqhandle       *a4
        dc.l    irqhandle       *a5
        dc.l    irqhandle       *a6
        dc.l    irqhandle       *a7
        dc.l    irqhandle       *a8
        dc.l    irqhandle       *a9
        dc.l    irqhandle       *aa
        dc.l    irqhandle       *ab
        dc.l    irqhandle       *ac
        dc.l    irqhandle       *ad
        dc.l    irqhandle       *ae
        dc.l    irqhandle       *af
        dc.l    irqhandle       *b0
        dc.l    irqhandle       *b1
        dc.l    irqcrash        *b2
        dc.l    irqhandle       *b3
        dc.l    irqhandle       *b4
        dc.l    irqhandle       *b5
        dc.l    irqhandle       *b6
        dc.l    irqhandle       *b7
        dc.l    irqhandle       *b8
        dc.l    irqhandle       *b9
        dc.l    irqhandle       *ba
        dc.l    irqhandle       *bb
        dc.l    irqhandle       *bc
        dc.l    irqhandle       *bd
        dc.l    irqhandle       *be
        dc.l    irqhandle       *bf
        dc.l    irqhandle       *c0
        dc.l    irqhandle       *c1
        dc.l    irqhandle       *c2
        dc.l    irqhandle       *c3
        dc.l    irqhandle       *c4
        dc.l    irqhandle       *c5
        dc.l    irqhandle       *c6
        dc.l    irqhandle       *c7
        dc.l    irqhandle       *c8
        dc.l    irqhandle       *c9
        dc.l    irqhandle       *ca
        dc.l    irqhandle       *cb
        dc.l    irqhandle       *cc
        dc.l    irqhandle       *cd
        dc.l    irqhandle       *ce
        dc.l    irqhandle       *cf
        dc.l    irqhandle       *d0
        dc.l    irqhandle       *d1
        dc.l    irqcrash        *d2
        dc.l    irqhandle       *d3
        dc.l    irqhandle       *d4
        dc.l    irqhandle       *d5
        dc.l    irqhandle       *d6
        dc.l    irqhandle       *d7
        dc.l    irqhandle       *d8
        dc.l    irqhandle       *d9
        dc.l    irqhandle       *da
        dc.l    irqhandle       *db
        dc.l    irqhandle       *dc
        dc.l    irqhandle       *dd
        dc.l    irqhandle       *de
        dc.l    irqhandle       *df
        dc.l    irqhandle       *e0
        dc.l    irqhandle       *e1
        dc.l    irqhandle       *e2
        dc.l    irqhandle       *e3
        dc.l    irqhandle       *e4
        dc.l    irqhandle       *e5
        dc.l    irqhandle       *e6
        dc.l    irqhandle       *e7
        dc.l    irqhandle       *e8
        dc.l    irqhandle       *e9
        dc.l    irqhandle       *ea
        dc.l    irqhandle       *eb
        dc.l    irqhandle       *ec
        dc.l    irqhandle       *ed
        dc.l    irqhandle       *ee
        dc.l    irqhandle       *ef
        dc.l    irqhandle       *f0
        dc.l    irqhandle       *f1
        dc.l    irqcrash        *f2
        dc.l    irqhandle       *f3
        dc.l    irqhandle       *f4
        dc.l    irqhandle       *f5
        dc.l    irqhandle       *f6
        dc.l    irqhandle       *f7
        dc.l    irqhandle       *f8
        dc.l    irqhandle       *f9
        dc.l    irqhandle       *fa
        dc.l    irqhandle       *fb
        dc.l    irqhandle       *fc
        dc.l    irqhandle       *fd
        dc.l    irqhandle       *fe
        dc.l    irqhandle       *ff

_CodeTab
        IFND    STAT
        dc.l    0
        ELSE
        dc.l    1
        dc.l    0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0
        dc.l    0,0     ; zárás
        ENDIF

_StatTab
        IFD     STAT
        dc.l    1
STAR    dc.l    0
STAC    dc.l    0

STATR00 dc.l    0
STATR01 dc.l    0
STATR02 dc.l    0
STATR03 dc.l    0
STATR04 dc.l    0
STATR05 dc.l    0
STATR06 dc.l    0
STATR07 dc.l    0
STATR08 dc.l    0
STATR09 dc.l    0
STATR0A dc.l    0
STATR0B dc.l    0
STATR0C dc.l    0
STATR0D dc.l    0
STATR0E dc.l    0
STATR0F dc.l    0
STATR10 dc.l    0
STATR11 dc.l    0
STATR12 dc.l    0
STATR13 dc.l    0
STATR14 dc.l    0
STATR15 dc.l    0
STATR16 dc.l    0
STATR17 dc.l    0
STATR18 dc.l    0
STATR19 dc.l    0
STATR1A dc.l    0
STATR1B dc.l    0
STATR1C dc.l    0
STATR1D dc.l    0
STATR1E dc.l    0
STATR1F dc.l    0

STATW00 dc.l    0
STATW01 dc.l    0
STATW02 dc.l    0
STATW03 dc.l    0
STATW04 dc.l    0
STATW05 dc.l    0
STATW06 dc.l    0
STATW07 dc.l    0
STATW08 dc.l    0
STATW09 dc.l    0
STATW0A dc.l    0
STATW0B dc.l    0
STATW0C dc.l    0
STATW0D dc.l    0
STATW0E dc.l    0
STATW0F dc.l    0
STATW10 dc.l    0
STATW11 dc.l    0
STATW12 dc.l    0
STATW13 dc.l    0
STATW14 dc.l    0
STATW15 dc.l    0
STATW16 dc.l    0
STATW17 dc.l    0
STATW18 dc.l    0
STATW19 dc.l    0
STATW1A dc.l    0
STATW1B dc.l    0
STATW1C dc.l    0
STATW1D dc.l    0
STATW1E dc.l    0
STATW1F dc.l    0

STAB    dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        ELSE
        dc.l    0
        ENDIF

*========================================================================}

*------------------------
* SNAPSHOT-FILE
*
* !!WARNING!!
* if you modify this 'struct', update the snapshot.h please!
*------------------------
        cnop    0,4
_sfrd2  dc.l    0
_sfrd3  dc.l    0
_sfrd4  dc.l    0
_sfrd5  dc.l    0
_sfrd6  dc.l    0
_sfrd7  dc.l    0
_sfchm  dc.l    0               ; charmap
_sft1   dc.w    0               ; initcnt1
_sfpc   dc.w    0               ; RPC [LO]
_sfc1   dc.w    0               ; counter1
_sfc2   dc.w    0               ; counter2
_sfc3   dc.w    0               ; counter3
_sfac   dc.w    0               ; actclk
_sfc1c  dc.w    0               ; cnt1actclk
_sfc2c  dc.w    0               ; cnt2actclk
_sfc3c  dc.w    0               ; cnt3actclk
_sfchr  dc.b    0               ; charinram
