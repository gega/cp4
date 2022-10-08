* :ts=4                     mmuexcecpion.asm
*

    include "lvos/exec_lib.i"

                xdef    _mmuexception       *
                xdef    _sup                *

                xref    _RegFetch           * l[] 7501.asm
                xref    _taskmsgport        * *msgport mmu.c
                xref    _TouchAddress       * l
                xref    _TouchData          * l
                xref    _TrapFetchTable     * l
                xref    _maintask           * l     (mmu.c)
                xref    _SysBase

;   struct ExceptionData {
;   0   struct Task             *exd_Task;
;   4   struct MMUContext       *exd_Context;
;   8   ULONG                   *exd_Descriptor;
;   12  ULONG                   *exd_NextDescriptor;
;   16  APTR                     exd_FaultAddress;
;   20  APTR                     exd_NextFaultAddress;
;   24  ULONG                    exd_UserData;
;   28  ULONG                    exd_NextUserData;
;   32  ULONG                    exd_Data;
;   36  APTR                     exd_ReturnPC;
;   40  ULONG                    exd_Flags;
;   44  ULONG                    exd_Properties;
;   48  ULONG                    exd_NextProperties;
;   52  UBYTE                    exd_internal;
;   53  UBYTE                    exd_FunctionCode;
;   54  UBYTE                    exd_Level;
;   55  UBYTE                    exd_NextLevel;
;   56  ULONG                    exd_DataRegs[8];
;   88  ULONG                    exd_AddrRegs[7];
;   116 UWORD                   *exd_SSP;
;   120 UWORD                   *exd_USP;           
;   124 struct ExecBase         *exd_SysBase;
;   128 struct MMUBase          *exd_MMUBase;
;   };


;   struct exceptdata {                 // len
;   0   struct MsgPort *notifyport;     // 4
;   4   struct Message *notifymsg;      // 4
;   8   struct ExecBase *sysbase;       // 4
;   12  UWORD address;                  // 2 plus/4 address
;       UWORD pad0;                     // 2
;   16  UBYTE data;                     // 1 plus/4 data
;       UBYTE pad1;                     // 1
;       UWORD pad2;                     // 2
;   20  ULONG flag;                     // 4
;   };


;       Register a0         -   Pointer to the ExceptionData structure or
;                               the PageAccessData.
;       Register a1         -   struct exceptdata *
;       Register a4         -   Ditto.
;       Register a5         -   Pointer to the code itself.
;       Register a6         -   MMUBase. NOT A SCRATCH.


* Ha rom-ot kell olvasnunk, akkor adjuk azt!!!
* test ram/rom és az i/o terület lapozható része
*
* read: ha a cim >= $ff40 akkor a ram/rom szerint kell
*       adni a cuccot, különben jó a saved terület
* write: mindig a saved területre ír

        xdef _SubSuper
_SubSuper:
        movem.l a4-a6,-(a7)

        lea _CallSuper(pc),a5
        move.l _SysBase,a6
        move.l a7,a4
        jsr _LVOSupervisor(a6)

        lea $c(a7),a7                   ;remove register dump
        move.l (a7)+,(a7)               ;Remove address from stack
        rts
_CallSuper:
        move.l $10(a4),-(a7)            ;Jump in here
        movem.l (a4),a4-a6              ;restore registers
        rts                             ;jump in
;ENDFOLD


_sup
                move.w  #$0f00,$dff180      ; villanás
                rte

_mmuexception
                move.w  #$0f00,$dff180      ; villanás
;               move.l  (40,a0),d0
                clr.l   d0
;               bclr    #17,d0              ; clear _CALL flag
                bset    #16,d0              ; do not retry!
                move.l  d0,(40,a0)

;DUMMY
                move.b  #$33,d1             ; DUMMY!!
                move.l  d1,(32,a0)          ; write to read buffer
                clr.l   d0
                rts
;DUMMY


                move.l  (16,a0),d0          ; get address
                and.l   #$ffff,d0           ; clr upper
                move.l  d0,a5               ; save full addr
                sub.l   #$fd00,d0           ; get offset in storetable
                move.l  (92,a0),a4          ; get a1 (storetable)
                move.l  (40,a0),d1          ; get flag
                btst    #0,d1               ; write/read?
                beq     .read

.write          move.l  (32,a0),d1          ; get data to write
                move.l  a5,_TouchAddress    ; set address
                move.l  d1,_TouchData
                move.b  d1,(a4,d0.l)        ; store it
                move.l  (104,a0),_TrapFetchTable    ; store old fetch (a4)
                move.l  #_RegFetch,(104,a0) ; set new fetch
                clr.l   d0
                rts

.read           cmp.w   #($ff40-$fd00),d0   ; test if addr >= $ff40
                bpl     .ne                 ; >= ugrás
.ram            clr.l   d1                  ; from storage
                move.b  (a4,d0.l),d1
                move.b  #$33,d1             ; DUMMY!!
                move.l  d1,(32,a0)          ; write to read buffer
                clr.l   d0
                rts
.ne             move.l  (76,a0),d1          ; get d5 (RP)
                btst    #22,d1              ; ACTRAMROM
                beq     .ram
.rom            clr.l   d1

                move.b  $400(a4,d0.l),d1    ; get it from rom
                move.b  #$44,d1             ; DUMMY!!
                move.l  d1,(32,a0)          ; -> read buffer
                clr.l   d0
                rts

;56 d0
;60 d1
;64 d2
;68 d3
;72 d4
;76 d5
