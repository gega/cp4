* :ts=8                                 c2pi.asm
*
* Copyright (C) 1997 Zavacki Ferenc
*       For any further information mail to:
*               zavacki@dragon.klte.hu
*

*       Delta version
*
*void REGARGS c2p(      REG(a0,void *chunkyaddr),
*                       REG(a1,void *deltaaddr),
*                       REG(d0,void *planaraddr));

*       Full version
*
*void REGARGS c2pf(     REG(a0,void *chunkyaddr),
*                       REG(a1,void *planaraddr));

*       Init
*
*void REGARGS c2pinit(REG(d0,int planewidth));

        xdef _c2p               ; delta version
        xdef _c2pinit           ; d0-planewidth (in bytes)
        xdef _c2pf              ; full version

        xref    _GfxBase

        include "lvos/graphics_lib.i"


LIBCALL         MACRO   ;FunctionName
                jsr     _LVO\1(a6)
                ENDM

CHUNKY_BUFFER_WIDTH     equ     352
CHUNKY_BUFFER_HEIGHT    equ     283


* d0 - bmx
* a0 - linedelta

_c2pinit                move.l  d0,C2P_BitplaneByteWidth
                        move.l  a0,C2P_LineSkipTable
                        rts


*-------------------------------------------------------------------------------
_c2pf                   move.l  a1,d0
                        movem.l d2-d7/a2-a6,-(sp)
                        move.l  d0,a2
                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4
                        move.l  a4,(C2P_ChunkyBufferEndAddress)
                        move.l  (C2P_BitplaneByteWidth,pc),d7

.14                     move.l  #CHUNKY_BUFFER_WIDTH/32,d6
.11                     addq.l  #4,a0
                        bra     .1
.10                     addq.l  #4,a2
                        subq.l  #1,d6
                        bne     .11

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a2,d7.l*8),a2
                        sub.l   d7,a2
                        lea     (a2,d7.l*8),a2
                        sub.l   d7,a2
                        add.l   #CHUNKY_BUFFER_WIDTH,a0

.13                     cmp.l   (C2P_ChunkyBufferEndAddress,pc),a0
                        bmi     .14

                        movem.l (sp)+,d2-d7/a2-a6
                        rts

.1                      sub.l   #1*4,a0

.9                      move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        and.l   d5,d4
                        or.l    d4,d3

                        move.w  d2,d4
                        move.w  d0,d2
                        swap    d2
                        move.w  d2,d0
                        move.w  d4,d2

                        move.w  d3,d4
                        move.w  d1,d3
                        swap    d3
                        move.w  d3,d1
                        move.w  d4,d3

                        move.l  #$00FF00FF,d5

                        move.l  d1,d4
                        lsr.l   #8,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #8,d4
                        eor.l   d4,d1

                        move.l  d3,d4
                        lsr.l   #8,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #8,d4
                        eor.l   d4,d3

                        move.l  #$33333333,d5

                        move.l  d2,d4
                        lsr.l   #2,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #2,d4
                        eor.l   d4,d2

                        move.l  d3,d4
                        lsr.l   #2,d4
                        eor.l   d1,d4
                        and.l   d5,d4
                        eor.l   d4,d1
                        lsl.l   #2,d4
                        eor.l   d4,d3

                        move.l  #$55555555,d5

                        move.l  d1,d4
                        lsr.l   #1,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #1,d4
                        eor.l   d4,d1

                        move.l  d1,(a2,d7.l*2)

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d0,a4
                        move.l  d2,a5
                        move.l  d3,a6

                        sub.l   #8*4,a0

                        move.l  #$F0F0F0F0,d5

                        move.l  (a0)+,d0
                        and.l   d5,d0
                        move.l  (a0)+,d1
                        and.l   d5,d1
                        lsr.l   #4,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        and.l   d5,d1
                        move.l  (a0)+,d2
                        and.l   d5,d2
                        lsr.l   #4,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        and.l   d5,d2
                        move.l  (a0)+,d3
                        and.l   d5,d3
                        lsr.l   #4,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        and.l   d5,d3
                        move.l  (a0)+,d4
                        and.l   d5,d4
                        lsr.l   #4,d4
                        or.l    d4,d3

                        move.l  a6,(a2)

                        move.w  d2,d4
                        move.w  d0,d2
                        swap    d2
                        move.w  d2,d0
                        move.w  d4,d2

                        move.w  d3,d4
                        move.w  d1,d3
                        swap    d3
                        move.w  d3,d1
                        move.w  d4,d3

                        move.l  #$00FF00FF,d5

                        move.l  d1,d4
                        lsr.l   #8,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #8,d4
                        eor.l   d4,d1

                        move.l  a5,(a2,d7.l)

                        move.l  d3,d4
                        lsr.l   #8,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #8,d4
                        eor.l   d4,d3

                        move.l  #$33333333,d5

                        move.l  d2,d4
                        lsr.l   #2,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #2,d4
                        eor.l   d4,d2

                        add.l   d7,a2
                        move.l  a4,(a2,d7.l*2)
                        sub.l   d7,a2

                        move.l  d3,d4
                        lsr.l   #2,d4
                        eor.l   d1,d4
                        and.l   d5,d4
                        eor.l   d4,d1
                        lsl.l   #2,d4
                        eor.l   d4,d3

                        move.l  #$55555555,d5

                        and.l   d5,d0
                        lsl.l   #1,d0
                        and.l   d5,d1
                        or.l    d0,d1

                        lea     (a2,d7.l*4),a2
                        move.l  d1,(a2,d7.l*2)

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d3,(a2)
                        move.l  d2,(a2,d7.l)

                        lsl.l   #2,d7
                        sub.l   d7,a2
                        lsr.l   #2,d7

                        bra     .10

*-------------------------------------------------------------------------------
*-------------------------------------------------------------------------------


* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     d0.l - Planar Buffer Address

_c2p                    movem.l d2-d7/a2-a6,-(sp)
                        move.l  d0,a2
                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4
                        move.l  a4,(C2P_ChunkyBufferEndAddress)
                        move.l  (C2P_BitplaneByteWidth,pc),d7

.14                     move.l  #CHUNKY_BUFFER_WIDTH/32,d6
.11                     cmp.l   (a0)+,(a1)+
                        bne     .1
                        cmp.l   (a0)+,(a1)+
                        bne     .2
                        cmp.l   (a0)+,(a1)+
                        bne     .3
                        cmp.l   (a0)+,(a1)+
                        bne     .4
                        cmp.l   (a0)+,(a1)+
                        bne     .5
                        cmp.l   (a0)+,(a1)+
                        bne     .6
                        cmp.l   (a0)+,(a1)+
                        bne     .7
                        cmp.l   (a0)+,(a1)+
                        bne     .8

.10                     addq.l  #4,a2
                        subq.l  #1,d6
                        bne     .11

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a2,d7.l*8),a2
                        sub.l   d7,a2
                        lea     (a2,d7.l*8),a2
                        sub.l   d7,a2
                        add.l   #CHUNKY_BUFFER_WIDTH,a0
                        add.l   #CHUNKY_BUFFER_WIDTH,a1

.13                     cmp.l   (C2P_ChunkyBufferEndAddress,pc),a0
                        bmi     .14

                        movem.l (sp)+,d2-d7/a2-a6
                        rts

.1                      sub.l   #1*4,a0
                        add.l   #7*4,a1
                        bra     .9

.2                      sub.l   #2*4,a0
                        add.l   #6*4,a1
                        bra     .9

.3                      sub.l   #3*4,a0
                        add.l   #5*4,a1
                        bra     .9

.4                      sub.l   #4*4,a0
                        add.l   #4*4,a1
                        bra     .9

.5                      sub.l   #5*4,a0
                        add.l   #3*4,a1
                        bra     .9

.6                      sub.l   #6*4,a0
                        add.l   #2*4,a1
                        bra     .9

.7                      sub.l   #7*4,a0
                        add.l   #1*4,a1
                        bra     .9

.8                      sub.l   #8*4,a0
                        add.l   #0*4,a1
                        bra     .9

.9                      move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        and.l   d5,d4
                        or.l    d4,d3

                        move.w  d2,d4
                        move.w  d0,d2
                        swap    d2
                        move.w  d2,d0
                        move.w  d4,d2

                        move.w  d3,d4
                        move.w  d1,d3
                        swap    d3
                        move.w  d3,d1
                        move.w  d4,d3

                        move.l  #$00FF00FF,d5

                        move.l  d1,d4
                        lsr.l   #8,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #8,d4
                        eor.l   d4,d1

                        move.l  d3,d4
                        lsr.l   #8,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #8,d4
                        eor.l   d4,d3

                        move.l  #$33333333,d5

                        move.l  d2,d4
                        lsr.l   #2,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #2,d4
                        eor.l   d4,d2

                        move.l  d3,d4
                        lsr.l   #2,d4
                        eor.l   d1,d4
                        and.l   d5,d4
                        eor.l   d4,d1
                        lsl.l   #2,d4
                        eor.l   d4,d3

                        move.l  #$55555555,d5

                        move.l  d1,d4
                        lsr.l   #1,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #1,d4
                        eor.l   d4,d1

                        move.l  d1,(a2,d7.l*2)

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d0,a4
                        move.l  d2,a5
                        move.l  d3,a6

                        sub.l   #8*4,a0

                        move.l  #$F0F0F0F0,d5

                        move.l  (a0)+,d0
                        and.l   d5,d0
                        move.l  (a0)+,d1
                        and.l   d5,d1
                        lsr.l   #4,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        and.l   d5,d1
                        move.l  (a0)+,d2
                        and.l   d5,d2
                        lsr.l   #4,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        and.l   d5,d2
                        move.l  (a0)+,d3
                        and.l   d5,d3
                        lsr.l   #4,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        and.l   d5,d3
                        move.l  (a0)+,d4
                        and.l   d5,d4
                        lsr.l   #4,d4
                        or.l    d4,d3

                        move.l  a6,(a2)

                        move.w  d2,d4
                        move.w  d0,d2
                        swap    d2
                        move.w  d2,d0
                        move.w  d4,d2

                        move.w  d3,d4
                        move.w  d1,d3
                        swap    d3
                        move.w  d3,d1
                        move.w  d4,d3

                        move.l  #$00FF00FF,d5

                        move.l  d1,d4
                        lsr.l   #8,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #8,d4
                        eor.l   d4,d1

                        move.l  a5,(a2,d7.l)

                        move.l  d3,d4
                        lsr.l   #8,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #8,d4
                        eor.l   d4,d3

                        move.l  #$33333333,d5

                        move.l  d2,d4
                        lsr.l   #2,d4
                        eor.l   d0,d4
                        and.l   d5,d4
                        eor.l   d4,d0
                        lsl.l   #2,d4
                        eor.l   d4,d2

                        add.l   d7,a2
                        move.l  a4,(a2,d7.l*2)
                        sub.l   d7,a2

                        move.l  d3,d4
                        lsr.l   #2,d4
                        eor.l   d1,d4
                        and.l   d5,d4
                        eor.l   d4,d1
                        lsl.l   #2,d4
                        eor.l   d4,d3

                        move.l  #$55555555,d5

                        and.l   d5,d0
                        lsl.l   #1,d0
                        and.l   d5,d1
                        or.l    d0,d1

                        lea     (a2,d7.l*4),a2
                        move.l  d1,(a2,d7.l*2)

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d3,(a2)
                        move.l  d2,(a2,d7.l)

                        lsl.l   #2,d7
                        sub.l   d7,a2
                        lsr.l   #2,d7

                        bra     .10

*-------------------------------------------------------------------------------

                                cnop    0,4

C2P_ChunkyBufferAddress         dc.l    0
C2P_DeltaBufferAddress          dc.l    0
C2P_PlanarBufferAddress         dc.l    0
C2P_BitplaneByteWidth           dc.l    0
C2P_ChunkyBufferEndAddress      dc.l    0
C2P_LineSkipTable               dc.l    0
C2P_Parity                      dc.b    0
