* :ts=8                         c2p_windowamigaasm.asm
*
* Copyright (C) 1997 Zavacki Ferenc
*       For any further information mail to:
*               zavacki@dragon.klte.hu
*

*
*void REGARGS c2pfull(  REG(a0,void *chunkyaddr),
*                       REG(a1,void *planaraddr));

*void REGARGS c2pdelta( REG(a0,void *chunkyaddr),
*                       REG(a1,void *deltaaddr),
*                       REG(a2,void *planaraddr) );

*       Init
*
*int REGARGS c2pinit(   REG(d0,int planewidth),
*                       REG(a0,unsigned char *linedeltavec),
*                       REG(d1,int planes) );

        xdef _c2pinit           ; d0-planewidth (in bytes)
        xdef _c2pfull           ; full version
        xdef _c2pdelta          ; delta version

        xref _c                 ; *b
        xref _d                 ; *b
        xref _PenTable          ; *b


CHUNKY_BUFFER_WIDTH     equ     352
CHUNKY_BUFFER_HEIGHT    equ     283

_c2pinit                move.l  a0,C2P_LineSkipBufferAddress
                        move.l  d0,C2P_BitplaneByteWidth
                        move.l  _PenTable,C2P_PenTable
                        clr.l   d0
                        cmp.b   #1,d1
                        beq     .1
                        cmp.b   #2,d1
                        beq     .2
                        cmp.b   #3,d1
                        beq     .3
                        cmp.b   #4,d1
                        beq     .4
                        cmp.b   #5,d1
                        beq     .5
                        cmp.b   #6,d1
                        beq     .6
                        cmp.b   #7,d1
                        beq     .7
                        cmp.b   #8,d1
                        beq     .8
                        moveq   #-1,d0
                        rts

.1                      move.l  #f_1,C2P_FullRoutine
                        move.l  #d_1,C2P_DeltaRoutine
                        rts
.2                      move.l  #f_2,C2P_FullRoutine
                        move.l  #d_2,C2P_DeltaRoutine
                        rts
.3                      move.l  #f_3,C2P_FullRoutine
                        move.l  #d_3,C2P_DeltaRoutine
                        rts
.4                      move.l  #f_4,C2P_FullRoutine
                        move.l  #d_4,C2P_DeltaRoutine
                        rts
.5                      move.l  #f_5,C2P_FullRoutine
                        move.l  #d_5,C2P_DeltaRoutine
                        rts
.6                      move.l  #f_6,C2P_FullRoutine
                        move.l  #d_6,C2P_DeltaRoutine
                        rts
.7                      move.l  #f_7,C2P_FullRoutine
                        move.l  #d_7,C2P_DeltaRoutine
                        rts
.8                      move.l  #f_8,C2P_FullRoutine
                        move.l  #d_8,C2P_DeltaRoutine
                        rts

*-------------------------------------------------------------------------------
* getpen in reg \1.l (d0-d4)
* a0 - chunky-buffer
getpen                  MACRO                           ; \1
**                      move.l  d7,-(sp)                ; ----
**                      move.l  (C2P_PenTable,pc),d7
                        move.b  \1,d7
                        move.b  (d7.l),\1               ; ---0
                        rol.l   #8,\1                   ; --0-
                        move.b  \1,d7
                        move.b  (d7.l),\1               ; --03
                        rol.l   #8,\1                   ; -03-
                        move.b  \1,d7
                        move.b  (d7.l),\1               ; -032
                        rol.l   #8,\1                   ; 032-
                        move.b  \1,d7
                        move.b  (d7.l),\1               ; 0321
                        rol.l   #8,\1                   ; 3210
**                      move.l  (sp)+,d7
        IFNE \2
                        move.l  \1,(data+((\2-1)*4))
        ENDIF
                        ENDM

getdat                  MACRO
                        move.l  (data+((\2-1)*4),pc),\1
                        ENDM

*-------------------------------------------------------------------------------
* Main enrty1
* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitnepwithbzte

_c2pfull                jmp     ([C2P_FullRoutine])
*-------------------------------------------------------------------------------
*-------------------------------------------------------------------------------
* Main enrty2
* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - LineSkip Buffer Address    (0-convert 1-no convert)
*     d0.l - BitBPlaneyBterwidth.B

_c2pdelta               jmp     ([C2P_DeltaRoutine])

;                       movem.l d0-d4/a1-a5,-(sp)
;                       move.l  a3,a1                   ; a1 - LineSkipTable
;                       move.l  _PenTable,a3            ; a3 - PenTable.l       (~)
;                       move.l  a0,a4                   ; a4 - PenArray.b       (p)
;                       move.l  _c,a5                   ; a5 - chunky.b         (c)
;                       move.l  _d,a2                   ; a2 - delta
;                       clr.l   d2
;
;**                     move.w  #((CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT)>>4)-1,d0
;
;                       move.l  #CHUNKY_BUFFER_HEIGHT-1,d3
;.outcik                        moveq   #1,d4
;                       move.l  #(CHUNKY_BUFFER_WIDTH>>4)-1,d0
;.cik                   cmp.l   (a2)+,(a5)+
;                       bne     .1
;.1f                    cmp.l   (a2)+,(a5)+
;                       bne     .2
;                       addq.l  #8,a4
;.2f                    cmp.l   (a2)+,(a5)+
;                       bne     .3
;.3f                    cmp.l   (a2)+,(a5)+
;                       bne     .4
;                       addq.l  #8,a4
;.4f                    dbra    d0,.cik
;                       ;clr.l  d4
;                       move.b  d4,(a1)+
;                       dbra    d3,.outcik
;                       bra     .end
;
;.1                     move.l  -(a5),d1
;                       addq.l  #4,a5
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)
;                       subq.l  #3,a4
;                       clr.l   d4
;                       bra     .1f
;
;.2                     move.l  -(a5),d1
;                       addq.l  #4,a5
;                       rol.l   #8,d1
;                       addq.l  #4,a4
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       clr.l   d4
;                       bra     .2f
;
;.3                     move.l  -(a5),d1
;                       addq.l  #4,a5
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)
;                       subq.l  #3,a4
;                       clr.l   d4
;                       bra     .3f
;
;.4                     move.l  -(a5),d1
;                       addq.l  #4,a5
;                       rol.l   #8,d1
;                       addq.l  #4,a4
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       rol.l   #8,d1
;                       move.b  d1,d2
;                       move.b  (a3,d2.l),(a4)+
;                       clr.l   d4
;                       bra     .4f

***                     move.w  #6204,d0
***                     clr.l   d1
***.cik                 move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     move.b  (a5)+,d1
***                     move.b  (a3,d1.l),(a4)+
***                     dbf     d0,.cik

;.end                   movem.l (sp)+,d0-d4/a1-a5
;                       jmp     ([C2P_DeltaRoutine])
*-------------------------------------------------------------------------------
*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitplane Byte Width

f_1                     movem.l d2-d7/a2-a3,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_PlanarBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_PlanarBufferAddress,pc),a1
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

                        move.l  #$55555555,d5

                        and.l   d5,d0
                        lsl.l   #3,d0
                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d2
                        lsl.l   #1,d2
                        and.l   d5,d3
                        or.l    d0,d3
                        or.l    d1,d3
                        or.l    d2,d3

                        move.l  d3,a3

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

                        move.l  a3,(a1)+

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

                        move.l  #$55555555,d5

                        and.l   d5,d0
                        lsl.l   #3,d0
                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d2
                        lsl.l   #1,d2
                        and.l   d5,d3
                        or.l    d0,d3
                        or.l    d1,d3
                        or.l    d2,d3

                        move.l  d3,a3

                        sub.l   #1,d6
                        bne     .2

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l),a1

.2                      cmp.l   a2,a0
                        bne     .1

                        move.l  a3,(a1)

                        movem.l (sp)+,d2-d7/a2-a3
                        rts

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - Line Skip Buffer Address
*     d0.l - Bitplane Byte Width

d_1                     movem.l d2-d7/a2-a4,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_DeltaBufferAddress)
                        move.l  a2,(C2P_PlanarBufferAddress)
                        move.l  a3,(C2P_LineSkipBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_DeltaBufferAddress,pc),a1
                        move.l  (C2P_PlanarBufferAddress,pc),a2
                        move.l  (C2P_LineSkipBufferAddress),a3
                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4

                        move.l  (C2P_BitplaneByteWidth,pc),d7

.12                     tst.b   (a3)+
                        beq     .14

                        add.l   #CHUNKY_BUFFER_WIDTH,a0
                        add.l   #CHUNKY_BUFFER_WIDTH,a1

                        add.l   d7,a2

                        bra     .13

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

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a2,d7.l),a2

.13                     cmp.l   a4,a0
                        bne     .12

                        movem.l (sp)+,d2-d7/a2-a4
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

.9                      move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        getpen  d0,0
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        getpen  d1,0
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        getpen  d1,0
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        getpen  d2,0
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        getpen  d2,0
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        getpen  d3,0
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        getpen  d3,0
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        getpen  d4,0
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

                        move.l  #$55555555,d5

                        and.l   d5,d0
                        lsl.l   #3,d0
                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d2
                        lsl.l   #1,d2
                        and.l   d5,d3
                        or.l    d0,d3
                        or.l    d1,d3
                        or.l    d2,d3

                        move.l  d3,(a2)

                        move.l  (sp)+,d7

                        bra     .10

*-------------------------------------------------------------------------------
*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitplane Byte Width

f_2                     movem.l d2-d7/a2-a4,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_PlanarBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_PlanarBufferAddress,pc),a1
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

                        and.l   d5,d0
                        lsl.l   #2,d0
                        and.l   d5,d2
                        or.l    d0,d2

                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d3
                        or.l    d1,d3

                        move.l  #$55555555,d5

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d2,a3
                        move.l  d3,a4

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

                        move.l  a4,(a1)+

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

                        move.l  a3,(-4,a1,d7.l)

                        move.l  d3,d4
                        lsr.l   #8,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #8,d4
                        eor.l   d4,d3

                        move.l  #$33333333,d5

                        and.l   d5,d0
                        lsl.l   #2,d0
                        and.l   d5,d2
                        or.l    d0,d2

                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d3
                        or.l    d1,d3

                        move.l  #$55555555,d5

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d2,a3
                        move.l  d3,a4

                        sub.l   #1,d6
                        bne     .2

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*2),a1

.2                      cmp.l   a2,a0
                        bne     .1

                        move.l  a4,(a1)
                        move.l  a3,(a1,d7.l)

                        movem.l (sp)+,d2-d7/a2-a4
                        rts

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - Line Skip Buffer Address
*     d0.l - Bitplane Byte Width

d_2                     movem.l d2-d7/a2-a4,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_DeltaBufferAddress)
                        move.l  a2,(C2P_PlanarBufferAddress)
                        move.l  a3,(C2P_LineSkipBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_DeltaBufferAddress,pc),a1
                        move.l  (C2P_PlanarBufferAddress,pc),a2
                        move.l  (C2P_LineSkipBufferAddress),a3
                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4

                        move.l  (C2P_BitplaneByteWidth,pc),d7

.12                     tst.b   (a3)+
                        beq     .14

                        add.l   #CHUNKY_BUFFER_WIDTH,a0
                        add.l   #CHUNKY_BUFFER_WIDTH,a1

                        lea     (a2,d7.l*2),a2

                        bra     .13

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

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a2,d7.l*2),a2

.13                     cmp.l   a4,a0
                        bne     .12

                        movem.l (sp)+,d2-d7/a2-a4
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

.9                      move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        getpen  d0,0
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        getpen  d1,0
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        getpen  d1,0
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        getpen  d2,0
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        getpen  d2,0
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        getpen  d3,0
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        getpen  d3,0
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        getpen  d4,0
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

                        and.l   d5,d0
                        lsl.l   #2,d0
                        and.l   d5,d2
                        or.l    d0,d2

                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d3
                        or.l    d1,d3

                        move.l  #$55555555,d5

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2

                        move.l  (sp)+,d7

                        move.l  d2,(a2,d7.l)

                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d3,(a2)

                        bra     .10

*-------------------------------------------------------------------------------

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitplane Byte Width

f_3                     movem.l d2-d7/a2-a5,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_PlanarBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_PlanarBufferAddress,pc),a1
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

                        move.l  d1,a3
                        move.l  d2,a4
                        move.l  d3,a5

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

                        move.l  a5,(a1)+

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

                        move.l  a4,(-4,a1,d7.l)

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

                        move.l  a3,(-4,a1,d7.l*2)

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

                        move.l  d1,a3
                        move.l  d2,a4
                        move.l  d3,a5

                        sub.l   #1,d6
                        bne     .2

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*2),a1
                        add.l   d7,a1

.2                      cmp.l   a2,a0
                        bne     .1

                        move.l  a5,(a1)
                        move.l  a4,(a1,d7.l)
                        move.l  a3,(a1,d7.l*2)

                        movem.l (sp)+,d2-d7/a2-a5
                        rts

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - Line Skip Buffer Address
*     d0.l - Bitplane Byte Width

d_3                     movem.l d2-d7/a2-a4,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_DeltaBufferAddress)
                        move.l  a2,(C2P_PlanarBufferAddress)
                        move.l  a3,(C2P_LineSkipBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_DeltaBufferAddress,pc),a1
                        move.l  (C2P_PlanarBufferAddress,pc),a2
                        move.l  (C2P_LineSkipBufferAddress),a3
                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4

                        move.l  (C2P_BitplaneByteWidth,pc),d7

.12                     tst.b   (a3)+
                        beq     .14

                        add.l   #CHUNKY_BUFFER_WIDTH,a0
                        add.l   #CHUNKY_BUFFER_WIDTH,a1

                        lea     (a2,d7.l*2),a2
                        add.l   d7,a2

                        bra     .13

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

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a2,d7.l*2),a2
                        add.l   d7,a2

.13                     cmp.l   a4,a0
                        bne     .12

                        movem.l (sp)+,d2-d7/a2-a4
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

.9                      move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        getpen  d0,0
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        getpen  d1,0
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        getpen  d1,0
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        getpen  d2,0
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        getpen  d2,0
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        getpen  d3,0
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        getpen  d3,0
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        getpen  d4,0
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

                        and.l   d5,d0
                        lsl.l   #1,d0
                        and.l   d5,d1
                        or.l    d0,d1

                        move.l  (sp)+,d7

                        move.l  d1,(a2,d7.l*2)

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2

                        move.l  d2,(a2,d7.l)

                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d3,(a2)

                        bra     .10

*-------------------------------------------------------------------------------

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitplane Byte Width

f_4                     movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_PlanarBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_PlanarBufferAddress,pc),a1
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

                        sub.l   #1,d6
                        bne     .2

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*4),a1

.2                      cmp.l   a2,a0
                        bne     .1

                        move.l  a6,(a1)
                        move.l  a5,(a1,d7.l)
                        move.l  a4,(a1,d7.l*2)
                        add.l   d7,a1
                        move.l  a3,(a1,d7.l*2)

                        movem.l (sp)+,d2-d7/a2-a6
                        rts

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - Line Skip Buffer Address
*     d0.l - Bitplane Byte Width

d_4                     movem.l d2-d7/a2-a4,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_DeltaBufferAddress)
                        move.l  a2,(C2P_PlanarBufferAddress)
                        move.l  a3,(C2P_LineSkipBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_DeltaBufferAddress,pc),a1
                        move.l  (C2P_PlanarBufferAddress,pc),a2
                        move.l  (C2P_LineSkipBufferAddress),a3
                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4

                        move.l  (C2P_BitplaneByteWidth,pc),d7

.12                     tst.b   (a3)+
                        beq     .14

                        add.l   #CHUNKY_BUFFER_WIDTH,a0
                        add.l   #CHUNKY_BUFFER_WIDTH,a1

                        lea     (a2,d7.l*4),a2

                        bra     .13

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

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a2,d7.l*4),a2

.13                     cmp.l   a4,a0
                        bne     .12

                        movem.l (sp)+,d2-d7/a2-a4
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

.9                      move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        getpen  d0,0
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        getpen  d1,0
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        getpen  d1,0
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        getpen  d2,0
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        getpen  d2,0
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        getpen  d3,0
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        getpen  d3,0
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        getpen  d4,0
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

                        move.l  (sp)+,d7

                        add.l   d7,a2
                        move.l  d0,(a2,d7.l*2)
                        sub.l   d7,a2

                        lsl.l   #1,d4
                        eor.l   d4,d1

                        move.l  d1,(a2,d7.l*2)

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2

                        move.l  d2,(a2,d7.l)

                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d3,(a2)

                        bra     .10

*-------------------------------------------------------------------------------

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitplane Byte Width

f_5                     movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_PlanarBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_PlanarBufferAddress,pc),a1
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

                        sub.l   #1,d6
                        bne     .2

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*4),a1
                        add.l   d7,a1

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

                        move.l  a5,(-4,a1,d7.l)

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

                        move.l  a4,(-4,a1,d7.l*2)

                        move.l  #$55555555,d5

                        and.l   d5,d0
                        lsl.l   #3,d0
                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d2
                        lsl.l   #1,d2
                        and.l   d5,d3
                        or.l    d0,d3
                        or.l    d1,d3
                        or.l    d2,d3

                        add.l   d7,a1
                        move.l  a3,(-4,a1,d7.l*2)
                        sub.l   d7,a1

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

                        move.l  d3,d4
                        lsr.l   #8,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #8,d4
                        eor.l   d4,d3

                        move.l  #$55555555,d5

                        and.l   d5,d0
                        lsl.l   #3,d0
                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d2
                        lsl.l   #1,d2
                        and.l   d5,d3
                        or.l    d0,d3
                        or.l    d1,d3
                        or.l    d2,d3

                        move.l  d3,a6

                        sub.l   #1,d6
                        bne     .4

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*4),a1
                        add.l   d7,a1

.4                      cmp.l   a2,a0
                        bne     .3

                        move.l  a6,(a1)

                        movem.l (sp)+,d2-d7/a2-a6
                        rts

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - Line Skip Buffer Address
*     d0.l - Bitplane Byte Width

d_5                     movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_DeltaBufferAddress)
                        move.l  a2,(C2P_PlanarBufferAddress)
                        move.l  a3,(C2P_LineSkipBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)

                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4
                        move.l  a4,(C2P_ChunkyBufferEndAddress)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_DeltaBufferAddress,pc),a1
                        move.l  (C2P_PlanarBufferAddress,pc),a2
                        move.l  (C2P_LineSkipBufferAddress,pc),a3

                        move.l  (C2P_BitplaneByteWidth,pc),d7

.12                     tst.b   (a3)+
                        beq     .14

                        add.l   #CHUNKY_BUFFER_WIDTH,a0
                        add.l   #CHUNKY_BUFFER_WIDTH,a1

                        lea     (a2,d7.l*4),a2
                        add.l   d7,a2

                        bra     .13

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

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a2,d7.l*4),a2
                        add.l   d7,a2

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

.9                      move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        getpen  d0,1
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        getpen  d1,2
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        getpen  d1,3
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        getpen  d2,4
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        getpen  d2,5
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        getpen  d3,6
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        getpen  d3,7
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        getpen  d4,8
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

                        move.l  (sp)+,d7

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

**                      sub.l   #8*4,a0

                        move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$F0F0F0F0,d5

**                      move.l  (a0)+,d0
**                      getpen  d0
                        getdat  d0,1
                        and.l   d5,d0
**                      move.l  (a0)+,d1
**                      getpen  d1
                        getdat  d1,2
                        and.l   d5,d1
                        lsr.l   #4,d1
                        or.l    d1,d0

**                      move.l  (a0)+,d1
**                      getpen  d1
                        getdat  d1,3
                        and.l   d5,d1
**                      move.l  (a0)+,d2
**                      getpen  d2
                        getdat  d2,4
                        and.l   d5,d2
                        lsr.l   #4,d2
                        or.l    d2,d1

**                      move.l  (a0)+,d2
**                      getpen  d2
                        getdat  d2,5
                        and.l   d5,d2
**                      move.l  (a0)+,d3
**                      getpen  d3
                        getdat  d3,6
                        and.l   d5,d3
                        lsr.l   #4,d3
                        or.l    d3,d2

**                      move.l  (a0)+,d3
**                      getpen  d3
                        getdat  d3,7
                        and.l   d5,d3
**                      move.l  (a0)+,d4
**                      getpen  d4
                        getdat  d4,8
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

                        move.l  (sp)+,d7

                        move.l  a5,(a2,d7.l)

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

                        add.l   d7,a2
                        move.l  a4,(a2,d7.l*2)
                        sub.l   d7,a2

                        move.l  #$55555555,d5

                        and.l   d5,d0
                        lsl.l   #3,d0
                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d2
                        lsl.l   #1,d2
                        and.l   d5,d3
                        or.l    d0,d3
                        or.l    d1,d3
                        or.l    d2,d3

                        move.l  d3,(a2,d7.l*4)

                        bra     .10

*-------------------------------------------------------------------------------

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitplane Byte Width

f_6                     movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_PlanarBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_PlanarBufferAddress,pc),a1
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

                        sub.l   #1,d6
                        bne     .2

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*4),a1
                        add.l   d7,a1
                        add.l   d7,a1

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

                        and.l   d5,d0
                        lsl.l   #2,d0
                        and.l   d5,d2
                        or.l    d0,d2

                        move.l  a4,(-4,a1,d7.l*2)

                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d3
                        or.l    d1,d3

                        move.l  #$55555555,d5

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        add.l   d7,a1
                        move.l  a3,(-4,a1,d7.l*2)
                        sub.l   d7,a1

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

                        and.l   d5,d0
                        lsl.l   #2,d0
                        and.l   d5,d2
                        or.l    d0,d2

                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d3
                        or.l    d1,d3

                        move.l  #$55555555,d5

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d2,a5
                        move.l  d3,a6

                        sub.l   #1,d6
                        bne     .4

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*4),a1
                        add.l   d7,a1
                        add.l   d7,a1

.4                      cmp.l   a2,a0
                        bne     .3

                        move.l  a6,(a1)
                        move.l  a5,(a1,d7.l)

                        movem.l (sp)+,d2-d7/a2-a6
                        rts

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - Line Skip Buffer Address
*     d0.l - Bitplane Byte Width

d_6                     movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_DeltaBufferAddress)
                        move.l  a2,(C2P_PlanarBufferAddress)
                        move.l  a3,(C2P_LineSkipBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)

                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4
                        move.l  a4,(C2P_ChunkyBufferEndAddress)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_DeltaBufferAddress,pc),a1
                        move.l  (C2P_PlanarBufferAddress,pc),a2
                        move.l  (C2P_LineSkipBufferAddress,pc),a3

                        move.l  (C2P_BitplaneByteWidth,pc),d7

.12                     tst.b   (a3)+
                        beq     .14

                        add.l   #CHUNKY_BUFFER_WIDTH,a0
                        add.l   #CHUNKY_BUFFER_WIDTH,a1

                        lea     (a2,d7.l*4),a2
                        add.l   d7,a2
                        add.l   d7,a2

                        bra     .13

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

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a2,d7.l*4),a2
                        add.l   d7,a2
                        add.l   d7,a2

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

.9                      move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        getpen  d0,1
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        getpen  d1,2
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        getpen  d1,3
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        getpen  d2,4
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        getpen  d2,5
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        getpen  d3,6
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        getpen  d3,7
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        getpen  d4,8
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

                        move.l  (sp)+,d7

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

**                      sub.l   #8*4,a0

                        move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$F0F0F0F0,d5

**                      move.l  (a0)+,d0
**                      getpen  d0
                        getdat  d0,1
                        and.l   d5,d0
**                      move.l  (a0)+,d1
**                      getpen  d1
                        getdat  d1,2
                        and.l   d5,d1
                        lsr.l   #4,d1
                        or.l    d1,d0

**                      move.l  (a0)+,d1
**                      getpen  d1
                        getdat  d1,3
                        and.l   d5,d1
**                      move.l  (a0)+,d2
**                      getpen  d2
                        getdat  d2,4
                        and.l   d5,d2
                        lsr.l   #4,d2
                        or.l    d2,d1

**                      move.l  (a0)+,d2
**                      getpen  d2
                        getdat  d2,5
                        and.l   d5,d2
**                      move.l  (a0)+,d3
**                      getpen  d3
                        getdat  d3,6
                        and.l   d5,d3
                        lsr.l   #4,d3
                        or.l    d3,d2

**                      move.l  (a0)+,d3
**                      getpen  d3
                        getdat  d3,7
                        and.l   d5,d3
**                      move.l  (a0)+,d4
**                      getpen  d4
                        getdat  d4,8
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

                        move.l  (sp)+,d7

                        move.l  a5,(a2,d7.l)

                        move.l  d3,d4
                        lsr.l   #8,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2
                        lsl.l   #8,d4
                        eor.l   d4,d3

                        move.l  #$33333333,d5

                        and.l   d5,d0
                        lsl.l   #2,d0
                        and.l   d5,d2
                        or.l    d0,d2

                        add.l   d7,a2
                        move.l  a4,(a2,d7.l*2)
                        sub.l   d7,a2

                        and.l   d5,d1
                        lsl.l   #2,d1
                        and.l   d5,d3
                        or.l    d1,d3

                        move.l  #$55555555,d5

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2

                        lea     (a2,d7.l*4),a2
                        move.l  d2,(a2,d7.l)

                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d3,(a2)

                        sub.l   d7,a2
                        sub.l   d7,a2
                        sub.l   d7,a2
                        sub.l   d7,a2

                        bra     .10

*-------------------------------------------------------------------------------

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitplane Byte Width

f_7                     movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_PlanarBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_PlanarBufferAddress,pc),a1
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

                        sub.l   #1,d6
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

                        sub.l   #1,d6
                        bne     .4

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*8),a1
                        sub.l   d7,a1

.4                      cmp.l   a2,a0
                        bne     .3

                        move.l  a6,(a1)
                        move.l  a5,(a1,d7.l)
                        move.l  a4,(a1,d7.l*2)

                        movem.l (sp)+,d2-d7/a2-a6
                        rts

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - Line Skip Buffer Address
*     d0.l - Bitplane Byte Width

d_7                     movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_DeltaBufferAddress)
                        move.l  a2,(C2P_PlanarBufferAddress)
                        move.l  a3,(C2P_LineSkipBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)

                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4
                        move.l  a4,(C2P_ChunkyBufferEndAddress)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_DeltaBufferAddress,pc),a1
                        move.l  (C2P_PlanarBufferAddress,pc),a2
                        move.l  (C2P_LineSkipBufferAddress,pc),a3

                        move.l  (C2P_BitplaneByteWidth,pc),d7

.12                     tst.b   (a3)+
                        beq     .14

                        add.l   #CHUNKY_BUFFER_WIDTH,a0
                        add.l   #CHUNKY_BUFFER_WIDTH,a1

                        lea     (a2,d7.l*8),a2
                        sub.l   d7,a2

                        bra     .13

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

.9                      move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        getpen  d0,1
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        getpen  d1,2
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        getpen  d1,3
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        getpen  d2,4
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        getpen  d2,5
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        getpen  d3,6
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        getpen  d3,7
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        getpen  d4,8
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

                        move.l  (sp)+,d7

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

**                      sub.l   #8*4,a0

                        move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$F0F0F0F0,d5

**                      move.l  (a0)+,d0
**                      getpen  d0
                        getdat  d0,1
                        and.l   d5,d0
**                      move.l  (a0)+,d1
**                      getpen  d1
                        getdat  d1,2
                        and.l   d5,d1
                        lsr.l   #4,d1
                        or.l    d1,d0

**                      move.l  (a0)+,d1
**                      getpen  d1
                        getdat  d1,3
                        and.l   d5,d1
**                      move.l  (a0)+,d2
**                      getpen  d2
                        getdat  d2,4
                        and.l   d5,d2
                        lsr.l   #4,d2
                        or.l    d2,d1

**                      move.l  (a0)+,d2
**                      getpen  d2
                        getdat  d2,5
                        and.l   d5,d2
**                      move.l  (a0)+,d3
**                      getpen  d3
                        getdat  d3,6
                        and.l   d5,d3
                        lsr.l   #4,d3
                        or.l    d3,d2

**                      move.l  (a0)+,d3
**                      getpen  d3
                        getdat  d3,7
                        and.l   d5,d3
**                      move.l  (a0)+,d4
**                      getpen  d4
                        getdat  d4,8
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

                        move.l  (sp)+,d7

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

                        move.l  d2,(a2,d7.l)

                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d3,(a2)

                        sub.l   d7,a2
                        sub.l   d7,a2
                        sub.l   d7,a2
                        sub.l   d7,a2

                        bra     .10

*-------------------------------------------------------------------------------

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Planar Buffer Address
*     d0.l - Bitplane Byte Width

f_8                     movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_PlanarBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_PlanarBufferAddress,pc),a1
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

                        sub.l   #1,d6
                        bne     .2

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*8),a1

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

                        sub.l   #1,d6
                        bne     .4

                        move.l  #CHUNKY_BUFFER_WIDTH/32,d6

                        lea     (-CHUNKY_BUFFER_WIDTH/8,a1,d7.l*8),a1

.4                      cmp.l   a2,a0
                        bne     .3

                        move.l  a6,(a1)
                        move.l  a5,(a1,d7.l)
                        move.l  a4,(a1,d7.l*2)
                        add.l   d7,a1
                        move.l  a3,(a1,d7.l*2)

                        movem.l (sp)+,d2-d7/a2-a6
                        rts

*-------------------------------------------------------------------------------

* IN: a0.l - Chunky Buffer Address
*     a1.l - Delta Buffer Address
*     a2.l - Planar Buffer Address
*     a3.l - Line Skip Buffer Address
*     d0.l - Bitplane Byte Width

d_8                     movem.l d2-d7/a2-a6,-(sp)

                        move.l  a0,(C2P_ChunkyBufferAddress)
                        move.l  a1,(C2P_DeltaBufferAddress)
                        move.l  a2,(C2P_PlanarBufferAddress)
                        move.l  a3,(C2P_LineSkipBufferAddress)
                        move.l  d0,(C2P_BitplaneByteWidth)

                        lea     (CHUNKY_BUFFER_WIDTH*CHUNKY_BUFFER_HEIGHT,a0),a4
                        move.l  a4,(C2P_ChunkyBufferEndAddress)



                        move.l  (C2P_ChunkyBufferAddress,pc),a0
                        move.l  (C2P_DeltaBufferAddress,pc),a1
                        move.l  (C2P_PlanarBufferAddress,pc),a2
                        move.l  (C2P_LineSkipBufferAddress,pc),a3

                        move.l  (C2P_BitplaneByteWidth,pc),d7

.12                     tst.b   (a3)+
                        beq     .14

                        add.l   #CHUNKY_BUFFER_WIDTH,a0
                        add.l   #CHUNKY_BUFFER_WIDTH,a1

                        lea     (a2,d7.l*8),a2

                        bra     .13

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

.9                      move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$0F0F0F0F,d5

                        move.l  (a0)+,d0
                        getpen  d0,1
                        and.l   d5,d0
                        lsl.l   #4,d0
                        move.l  (a0)+,d1
                        getpen  d1,2
                        and.l   d5,d1
                        or.l    d1,d0

                        move.l  (a0)+,d1
                        getpen  d1,3
                        and.l   d5,d1
                        lsl.l   #4,d1
                        move.l  (a0)+,d2
                        getpen  d2,4
                        and.l   d5,d2
                        or.l    d2,d1

                        move.l  (a0)+,d2
                        getpen  d2,5
                        and.l   d5,d2
                        lsl.l   #4,d2
                        move.l  (a0)+,d3
                        getpen  d3,6
                        and.l   d5,d3
                        or.l    d3,d2

                        move.l  (a0)+,d3
                        getpen  d3,7
                        and.l   d5,d3
                        lsl.l   #4,d3
                        move.l  (a0)+,d4
                        getpen  d4,8
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

                        move.l  (sp)+,d7

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

**                      sub.l   #8*4,a0

                        move.l  d7,-(sp)
                        move.l  (C2P_PenTable,pc),d7

                        move.l  #$F0F0F0F0,d5

**                      move.l  (a0)+,d0
**                      getpen  d0
                        getdat  d0,1
                        and.l   d5,d0
**                      move.l  (a0)+,d1
**                      getpen  d1
                        getdat  d1,2
                        and.l   d5,d1
                        lsr.l   #4,d1
                        or.l    d1,d0

**                      move.l  (a0)+,d1
**                      getpen  d1
                        getdat  d1,3
                        and.l   d5,d1
**                      move.l  (a0)+,d2
**                      getpen  d2
                        getdat  d2,4
                        and.l   d5,d2
                        lsr.l   #4,d2
                        or.l    d2,d1

**                      move.l  (a0)+,d2
**                      getpen  d2
                        getdat  d2,5
                        and.l   d5,d2
**                      move.l  (a0)+,d3
**                      getpen  d3
                        getdat  d3,6
                        and.l   d5,d3
                        lsr.l   #4,d3
                        or.l    d3,d2

**                      move.l  (a0)+,d3
**                      getpen  d3
                        getdat  d3,7
                        and.l   d5,d3
**                      move.l  (a0)+,d4
**                      getpen  d4
                        getdat  d4,8
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

                        move.l  (sp)+,d7

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

                        lea     (a2,d7.l*4),a2
                        move.l  d0,(a2,d7.l*2)
                        sub.l   d7,a2

                        lsl.l   #1,d4
                        eor.l   d4,d1

                        move.l  d1,(a2,d7.l*2)

                        move.l  d3,d4
                        lsr.l   #1,d4
                        eor.l   d2,d4
                        and.l   d5,d4
                        eor.l   d4,d2

                        move.l  d2,(a2,d7.l)

                        lsl.l   #1,d4
                        eor.l   d4,d3

                        move.l  d3,(a2)

                        sub.l   d7,a2
                        sub.l   d7,a2
                        sub.l   d7,a2
                        sub.l   d7,a2

                        bra     .10

*-------------------------------------------------------------------------------

*-------------------------------------------------------------------------------

                        cnop    0,4

C2P_ChunkyBufferAddress         dc.l    0
C2P_DeltaBufferAddress          dc.l    0
C2P_PlanarBufferAddress         dc.l    0
C2P_LineSkipBufferAddress       dc.l    0
C2P_BitplaneByteWidth           dc.l    0
C2P_ChunkyBufferEndAddress      dc.l    0

C2P_FullRoutine         dc.l    0
C2P_DeltaRoutine        dc.l    0
C2P_PenTable            dc.l    0

data
dat0                    dc.l    0
dat1                    dc.l    0
dat2                    dc.l    0
dat3                    dc.l    0
dat4                    dc.l    0
dat5                    dc.l    0
dat6                    dc.l    0
dat7                    dc.l    0
