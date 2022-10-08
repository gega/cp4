* :ts=8                                 c2p.asm
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

CHUNKY_BUFFER_WIDTH     equ     352
CHUNKY_BUFFER_HEIGHT    equ     283


* d0 - bmx
* a0 - linedelta

_c2pinit                move.l  d0,C2P_BitplaneByteWidth
                        move.l  a0,C2P_LineSkipTable
                        rts


*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitplane Byte Width


_c2pf                   movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_PlanarBufferAddress)
                        *move.l d0,(C2P_BitplaneByteWidth)

                        *move.l (C2P_ChunkyBufferAddress,pc),a0
                        *move.l (C2P_PlanarBufferAddress,pc),a1

                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a2

                        move.l  (C2P_BitplaneByteWidth,pc),d7

                        move.l  #$0F0F0F0F,d5

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

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d0,a3
                        move.l  d1,a4
                        move.l  d2,a5
                        move.l  d3,a6

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

.1                      move.l  #$0F0F0F0F,d5

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

                        move.l  a6,(a1)+

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

                        move.l  a5,(-4,a1,d7.l)

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

                        move.l  a4,(-4,a1,d7.l*2)

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

                        add.l   d7,a1
                        move.l  a3,(-4,a1,d7.l*2)
                        sub.l   d7,a1

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d0,a3
                        move.l  d1,a4
                        move.l  d2,a5
                        move.l  d3,a6

                        subq.l  #1,d6
                        bne     .2

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6
                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*8),a1
                        sub.l   d7,a1

.2                      cmp.l   a2,a0
                        bne     .1

                        move.l  (C2P_ChunkyBufferAddress,pc),a0

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

                        move.l  a6,(a1)+

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

                        move.l  a5,(-4,a1,d7.l)

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

                        move.l  a4,(-4,a1,d7.l*2)

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

                        add.l   d7,a1
                        move.l  a3,(-4,a1,d7.l*2)
                        sub.l   d7,a1

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d1,a4
                        move.l  d2,a5
                        move.l  d3,a6

                        lea     ([C2P_PlanarBufferAddress,pc],d7.l*4),a1

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

.3                      move.l  #$F0F0F0F0,d5

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

                        move.l  a6,(a1)+

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

                        move.l  a5,(-4,a1,d7.l)

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

                        move.l  a4,(-4,a1,d7.l*2)

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

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d1,a4
                        move.l  d2,a5
                        move.l  d3,a6

                        subq.l  #1,d6
                        bne     .4

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6
                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*8),a1
                        sub.l   d7,a1

.4                      cmp.l   a2,a0
                        bne     .3

                        move.l  a6,(a1)+
                        move.l  a5,(-4,a1,d7.l)
                        move.l  a4,(-4,a1,d7.l*2)

                        movem.l (sp)+,d2-d7/a2-a6
                        rts

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - 
*     d0.l - Bitplane Byte Width

_c2p                    movem.l d2-d7/a2-a6,-(sp)

                        *move.l a0,(C2P_ChunkyBufferAddress)
                        *move.l a1,(C2P_DeltaBufferAddress)
                        *move.l a2,(C2P_PlanarBufferAddress)
                        *move.l d0,(C2P_BitplaneByteWidth)
                        move.l  d0,a2

                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4
                        move.l  a4,(C2P_ChunkyBufferEndAddress)

*nolineskip             move.l  (C2P_LineSkipTable),a3

                        *move.l (C2P_ChunkyBufferAddress,pc),a0
                        *move.l (C2P_DeltaBufferAddress,pc),a1
                        *move.l (C2P_PlanarBufferAddress,pc),a2

                        move.l  (C2P_BitplaneByteWidth,pc),d7

.12                     ;tst.b  (a3)+
                        ;beq    .14

                ;       add.l   #CHUNKY_BUFFER_WIDTH,a0
                ;       add.l   #CHUNKY_BUFFER_WIDTH,a1

                ;       lea     (a2,d7.l*8),a2
                ;       sub.l   d7,a2

                ;       bra     .13

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

.10                     add.l   #4,a2

                        sub.l   #1,d6
                        bne     .11

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a2,d7.l*8),a2
                        sub.l   d7,a2

.13                     cmp.l   (C2P_ChunkyBufferEndAddress,pc),a0
                        bne     .12

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

