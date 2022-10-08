* :ts=8                         mmuhandle.asm
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

* normal settings
*       TC =02a08680
*       TT0=00000000
*       TT1=00000000
*       CRP=80000002 68408000
*       SRP=80000002 68390000
*       MMUSR=  0003

* normal settings 2
*       TC =00a08680
*       TT0=00000000
*       TT1=00000000
*       CRP=80000002 6825eba0
*       SRP=80000002 68390000
*       MMUSR=  0201

* enforcer settings
*       TC =80a08680
*       TT0=00000000
*       TT1=00000000
*       CRP=80000002 6825eba0
*       SRP=80000002 68390000
*       MMUSR=  0003

                include "exec/types.i"
                include "exec/macros.i"
                include "exec/execbase.i"
                include "lvos/exec_lib.i"

        mc68030
        pmmu

                xdef    _SupMMU_on              ; func
                xdef    _SupMMU_off             ; func
                xdef    _BusTrapHandler         ; func
                xdef    _OldTrapData            ; l
                xdef    _OldTrapCode            ; l
                xref    _TouchAddress           ; l
                xref    _TouchData              ; l
                xref    _TrapFetchTable         ; l

                xref    _SysBase                ;
                xref    _mmu_root               ; *l (mmu.c)
                xref    _RegFetch               ; l[] 7501.asm

; itt nem dolgozunk fel semmit, hanem a fetchtable-t
; eltároljuk, és átállítjuk egy másik címre, így
; a következõ utasítás helyett a register-setting
; kerül feldolgozásra, ami visszaállítja a fetchtable-t
; itt ellenõrizni kell, hogy a fetchtable_store nulla-e
; mert ha nem, akkor nem csinálunk semmit
; plusz info-ként a címet ahová/ahnnan írtak/olvastak
; és az adatot kell eltárolni.
;
; A read accesseket itt kell feldolgozni, különben hibás
; adatokat kaphat a program!


; BUS-TRAP HANDLER START --------------------------------------------------

_BusTrapHandler move.l  a6,.savea6              ; save a6
                move    usp,a6                  ; get user stack
                movem.l d0-d1,-(a6)             ; save regs to user stack
                cmpi.l  #2,(sp)                 ; test Amiga specific type
                bne     .notbus                 ; not bus-error trap
                addq    #4,sp                   ; restore CPU orig SP
                move.w  $a(sp),d0               ; get SSW
                btst    #8,d0                   ; test DF bit
                beq     .notmy                  ; non-data access
                btst    #7,d0                   ; test RM bit
                bne     .notmy                  ; read-modify-write access

; MY BUSINESS -------------------------------------------------------------

                and.w   #~$0100,$a(sp)          ; clear DF bit in stack
                move.l  $10(sp),d1              ; get fault address
                sub.l   #$fc00,d1               ; get offset in storetable
                ; storetable in a1
                btst    #6,d0                   ; test RW bit
                beq     .write

; READ ACCESS -------------------------------------------------------------

.read           cmp.w   #($ff40-$fc00),d1       ; test if addr >= $ff40
                bpl     .nreg                   ; >= ugrás
                cmp.w   #$100,d1                ; ha kisebb mint $100
                bmi     .nreg
.ram            and.b   #$30,d0                 ; get SIZE field
                cmp.b   #$10,d0
                beq     .rabyte
                cmp.b   #$30,d0
                beq     .ralong
.raword         clr.l   d0
                move.w  (a1,d1.w),d0
                bra     .prad
.ralong         move.l  (a1,d1.w),d0
                bra     .prad
.rabyte         clr.l   d0                      ; from storage
                move.b  (a1,d1.w),d0            ; (s-RAM)
.prad           move.l  d0,$2c(sp)              ; write data to DIB
                bra     .endtrap

.nreg           btst    #22,d5                  ; ACTRAMROM (RP EQUR d5)
                beq     .ram
.rom            and.b   #$30,d0                 ; get SIZE field
                cmp.b   #$10,d0
                beq     .robyte
                cmp.b   #$30,d0
                beq     .rolong
.roword         clr.l   d0
                move.w  $400(a1,d1.w),d0        ; get it from rom
                bra     .prod
.rolong         move.l  $400(a1,d1.w),d0        ; get it from rom
                bra     .prod
.robyte         clr.l   d0
                move.b  $400(a1,d1.w),d0        ; get it from rom
.prod           move.l  d0,$2c(sp)              ; write data to DIB
                bra     .endtrap

; WRITE ACCESS ------------------------------------------------------------

.write
                and.b   #$30,d0                 ; get SIZE field
                cmp.b   #$10,d0
                beq     .wbyte
                cmp.b   #$30,d0
                beq     .wlong
.wword          move.l  $18(sp),d0              ; get data to write
                move.w  d0,(a1,d1.w)            ; store it (s-RAM)
                bra     .endtrap
.wlong          move.l  $18(sp),d0              ; get data to write
                move.l  d0,(a1,d1.w)            ; store it (s-RAM)
                bra     .endtrap
.wbyte          move.l  $18(sp),d0              ; get data to write
                move.b  d0,(a1,d1.w)            ; store it (s-RAM)
                cmp.w   #$340,d1                ; test if addr >= $ff40
                bpl     .endtrap                ; >= ugrás
                cmp.w   #$100,d1                ; ha kisebb mint $100
                bmi     .endtrap
                tst.l   _TrapFetchTable         ; már patchelve van
                bne     .endtrap
                move.l  a4,_TrapFetchTable      ; FETCH EQUR a4
                move.l  #_RegFetch,a4           ; set new fetch
                move.w  d1,_TouchAddress        ; set address
                move.b  d0,_TouchData

; FINALIZE ----------------------------------------------------------------

.endtrap        movem.l (a6)+,d0-d1             ; restore regs from user stack
                move.l  .savea6(pc),a6
                rte

; NOT MY BUSINESS ---------------------------------------------------------

.notmy          subq    #4,sp
.notbus         movem.l (a6)+,d0-d1             ; restore regs from user stack
                move.l  .savea6(pc),a6
                tst.l   _OldTrapCode
                beq.s   .restoresp
                move.l  _OldTrapCode,-(sp)
                rts
.restoresp      addq    #4,sp                   ; restore CPU orig SP
                rte

; DATA AREA ---------------------------------------------------------------

.savea6         dc.l    0

; BUS-TRAP HANDLER END ----------------------------------------------------


_SupMMU_on      movem.l a5-a6,-(sp)
                lea     mmu_on,a5
                move.l  _SysBase,a6
                jsr _LVOSupervisor(a6)
                movem.l (sp)+,a5-a6
;               move.l  #oldies,d0
                rts
        cnop    0,4
mmu_on          move.l  _mmu_root,new_crp_lo
                pmove   tc,old_tc               ; save old values
                pmove   tt0,old_tt0
                pmove   tt1,old_tt1
                pmove   crp,old_crp
                pmove   srp,old_srp
                pmove   mmusr,old_mmusr
                pmove   disable_tc,tc           ; disable mmu
                pmove   new_tt0,tt0
                pmove   new_tt1,tt1
                pmove   new_crp,crp
                pmove   new_tc,tc               ; enable
                rte

_SupMMU_off     movem.l a5-a6,-(sp)
                lea     mmu_off,a5
                move.l  _SysBase,a6
                jsr _LVOSupervisor(a6)
                movem.l (sp)+,a5-a6
                rts
        cnop    0,4
mmu_off         pmove   old_tt0,tt0
                pmove   old_tt1,tt1
                pmove   disable_tc,tc           ; disable mmu
                pmove   old_crp,crp
                pmove   old_srp,srp
                pmove   old_tc,tc               ; restore original
                rte

oldies
old_tc          dc.l    0
old_tt0         dc.l    0
old_tt1         dc.l    0
old_crp         dc.l    0,0
old_srp         dc.l    0,0
old_mmusr       dc.w    0

news
new_tc          dc.l    $80a0bb00               ; fix   !
new_tt0         dc.l    0                       ; fix   !
new_tt1         dc.l    0                       ; fix   !
new_crp         dc.l    $80000002               ; fix   !
new_crp_lo      dc.l    0                       ;       !
new_srp         dc.l    $80000002
new_srp_lo      dc.l    0
new_mmusr       dc.w    0

disable_tc      dc.l    $00a08680               ; normal system

_OldTrapData    dc.l    0
_OldTrapCode    dc.l    0
