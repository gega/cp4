/* :ts=4                            cp4_defs.h
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
 */

#ifndef CP4_DEFS_H
#define CP4_DEFS_H

#define BASICBASE       0x8000
#define KERNALBASE      0xC000
#define MAXRAM          65536
#define NOOPERAND       -1      // nincs operandus
#define NOAMI           -1      // nincs amigás megfelelõ

// Addressing modes
#define  ILL        0
#define  BYTE       1
#define  ABS        2
#define  ZP         3
#define  ACC        4
#define  IMP        5
#define  IZPX       6
#define  IZPY       7
#define  ZPX        8
#define  ABSX       9
#define  ABSY       10
#define  REL        11
#define  IABS       12
#define  ZPY        13
#define  NONE       14

// addressless extra addressing modes
#define  A_ILL      -1
#define  A_BYTE     -2
#define  A_ACCU     -3
#define  A_IMP      -4

// extra operands
#define  OP_ILL     -1
#define  OP_NONE    -2

// illegal opcode
#define  O_ILL      -1

char *addressing[]={ "ILL","#BYTE","ABS","ZP","ACC","IMP","(ZP,X)",
        "(ZP),Y","ZP,X","ABS,X","ABS,Y","REL","(ABS)","ZP,Y","NONE" };

/*
 * Statements defines / names
 */
#define ADC 0
#define AND 1
#define ASL 2
#define BCC 3
#define BCS 4
#define BEQ 5
#define BIT 6
#define BMI 7
#define BNE 8
#define BPL 9
#define BRK 10
#define BVC 11
#define BVS 12
#define CLC 13
#define CLD 14
#define CLI 15
#define CLV 16
#define CMP 17
#define CPX 18
#define CPY 19
#define DEC 20
#define DEX 21
#define DEY 22
#define EOR 23
#define INC 24
#define INX 25
#define INY 26
#define JMP 27
#define JSR 28
#define LDA 29
#define LDX 30
#define LDY 31
#define LSR 32
#define NOP 33
#define ORA 34
#define PHA 35
#define PHP 36
#define PLA 37
#define PLP 38
#define ROL 39
#define ROR 40
#define RTI 41
#define RTS 42
#define SBC 43
#define SEC 44
#define SED 45
#define SEI 46
#define STA 47
#define STX 48
#define STY 49
#define TAX 50
#define TAY 51
#define TSX 52
#define TXA 53
#define TXS 54
#define TYA 55
#define XXX 56
// illegális kódok
#define I_CRA   101 // Crash
#define I_NOP   102 // NOP
#define I_NO2   103 // NOP2
#define I_NO3   104 // NOP3
#define I_LAX   105 // LDA & TAX
#define I_DCC   106 // DEC & CMP
#define I_ICS   107 // INC & SBC
#define I_RLA   108 // ROL & AND
#define I_RRA   109 // ROR & ADC
#define I_ALO   110 // ASL & ORA
#define I_LRE   111 // LSR & EOR
#define I_AND   112 // AND ZP
#define I_ALR   113 // AND BYTE & LSR
#define I_AXS   114 // AND ACC,X --> (ZP,X)
#define I_AXM   115 // AND ACC,X --> ZP
#define I_XAA   116 // TXA & AND BYTE
#define I_AXP   117 // AND ACC,X --> ABS
#define I_AXI   118 // AND ACC,X --> ZP,X
#define I_AYS   119 // AND Y,BYTE+1 --> ABS,X
#define I_AXB   120 // AND X,BYTE+1 --> ABS,Y
#define I_AXF   121 // AND ACC,X,BYTE+1 --> ABS,Y
#define I_AXN   122 // AND ACC,X --> X & X-=BYTE (noCarry)
#define I_SBC   123 // SBC BYTE

#define iCRA    101
#define iNOP    102
#define iNO2    103
#define iNO3    104
#define iSLO    105
#define iANC    106
#define iRLA    107
#define iSRE    108
#define iRRA    109
#define iARR    110
#define iSAX    111
#define iANE    112
#define iSHA    113
#define iSHS    114
#define iSHY    115
#define iSHX    116
#define iLAX    117
#define iLAS    118
#define iDCP    119
#define iSBX    120
#define iISB    121
#define iSBC    122
#define iASR    123
#define iLXA    124

char *statements[]={
    "ADC","AND","ASL","BCC","BCS","BEQ","BIT","BMI",
    "BNE","BPL","BRK","BVC","BVS","CLC","CLD","CLI",
    "CLV","CMP","CPX","CPY","DEC","DEX","DEY","EOR",
    "INC","INX","INY","JMP","JSR","LDA","LDX","LDY",
    "LSR","NOP","ORA","PHA","PHP","PLA","PLP","ROL",
    "ROR","RTI","RTS","SBC","SEC","SED","SEI","STA",
    "STX","STY","TAX","TAY","TSX","TXA","TXS","TYA","???" };

char *illstatements[]={         // [iXXX-100]
    "???","CRA","NOP","NO2","NO3","SLO","ANC","RLA",
    "SRE","RRA","ARR","SAX","ANE","SHA","SHS","SHY",
    "SHX","LAX","LAS","DCP","SBX","ISB","SBC","ASR",
    "LXA"
};

#endif
