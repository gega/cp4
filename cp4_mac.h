/* :ts=4                            cp4_mac.h
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

#ifndef CP4_MAC_H
#define CP4_MAC_H

//--------------------------------------------------------------------------
//----------------------OPCODE-DATA-----------------------------------------
//--------------------------------------------------------------------------

struct p4st stats[]={
//  NAME,ADRMODE    L C J               CODE
//-------------------------------------------
{   BRK,IMP,        1,7,1,  },          // 0
{   ORA,IZPX,       2,6,0,  },          // 1
{   I_CRA,NONE,     1,0,0,  },          // 2
{   iSLO,IZPX,      2,0,0,  },          // 3
{   I_NO2,IMP,      2,4,0,  },          // 4
{   ORA,ZP,         2,3,0,  },          // 5
{   ASL,ZP,         2,5,0,  },          // 6
{   iSLO,ZP,        2,0,0,  },          // 7
{   PHP,IMP,        1,3,0,  },          // 8
{   ORA,BYTE,       2,2,0,  },          // 9
{   ASL,ACC,        1,2,0,  },          // 10
{   iANC,BYTE,      2,0,0,  },          // 11
{   I_NO3,IMP,      3,6,0,  },          // 12
{   ORA,ABS,        3,4,0,  },          // 13
{   ASL,ABS,        3,6,0,  },          // 14
{   iSLO,ABS,       3,0,0,  },          // 15
{   BPL,REL,        2,2,2,  },          // 16
{   ORA,IZPY,       2,5,0,  },          // 17
{   I_CRA,NONE,     1,0,0,  },          // 18
{   iSLO,IZPY,      2,0,0,  },          // 19
{   I_NO2,IMP,      2,4,0,  },          // 20
{   ORA,ZPX,        2,4,0,  },          // 21
{   ASL,ZPX,        2,6,0,  },          // 22
{   iSLO,ZPX,       2,0,0,  },          // 23
{   CLC,IMP,        1,2,0,  },          // 24
{   ORA,ABSY,       3,4,0,  },          // 25
{   I_NOP,IMP,      1,2,0,  },          // 26
{   iSLO,ABSY,      3,0,0,  },          // 27
{   I_NO3,IMP,      3,6,0,  },          // 28
{   ORA,ABSX,       3,4,0,  },          // 29
{   ASL,ABSX,       2,7,0,  },          // 30
{   iSLO,ABSX,      3,0,0,  },          // 31
{   JSR,ABS,        3,6,1,  },          // 32
{   AND,IZPX,       2,6,0,  },          // 33
{   I_CRA,NONE,     1,0,0,  },          // 34
{   iRLA,IZPX,      2,0,0,  },          // 35
{   BIT,ZP,         2,3,0,  },          // 36
{   AND,ZP,         2,3,0,  },          // 37
{   ROL,ZP,         2,5,0,  },          // 38
{   iRLA,ZP,        2,0,0,  },          // 39
{   PLP,IMP,        1,4,0,  },          // 40
{   AND,BYTE,       2,2,0,  },          // 41
{   ROL,IMP,        1,2,0,  },          // 42
{   iANC,BYTE,      2,0,0,  },          // 43
{   BIT,ABS,        3,4,0,  },          // 44
{   AND,ABS,        3,4,0,  },          // 45
{   ROL,ABS,        3,6,0,  },          // 46
{   iRLA,ABS,       3,0,0,  },          // 47
{   BMI,REL,        2,2,2,  },          // 48
{   AND,IZPY,       2,5,0,  },          // 49
{   I_CRA,NONE,     1,0,0,  },          // 50
{   iRLA,IZPY,      2,0,0,  },          // 51
{   I_NO2,IMP,      2,4,0,  },          // 52
{   AND,ZPX,        2,4,0,  },          // 53
{   ROL,ZPX,        2,6,0,  },          // 54
{   iRLA,ZPX,       2,0,0,  },          // 55
{   SEC,IMP,        1,2,0,  },          // 56
{   AND,ABSY,       3,4,0,  },          // 57
{   I_NOP,IMP,      1,2,0,  },          // 58
{   iRLA,ABSY,      3,0,0,  },          // 59
{   I_NO3,IMP,      3,6,0,  },          // 60
{   AND,ABSX,       3,4,0,  },          // 61
{   ROL,ABSX,       3,7,0,  },          // 62
{   iRLA,ABSX,      3,0,0,  },          // 63
{   RTI,IMP,        1,6,1,  },          // 64
{   EOR,IZPX,       2,6,0,  },          // 65
{   I_CRA,NONE,     1,0,0,  },          // 66
{   iSRE,IZPX,      2,0,0,  },          // 67
{   I_NO2,IMP,      2,4,0,  },          // 68
{   EOR,ZP,         2,3,0,  },          // 69
{   LSR,ZP,         2,5,0,  },          // 70
{   iSRE,ZP,        2,0,0,  },          // 71
{   PHA,IMP,        1,3,0,  },          // 72
{   EOR,BYTE,       2,2,0,  },          // 73
{   LSR,ACC,        1,2,0,  },          // 74
{   iASR,BYTE,      2,0,0,  },          // 75
{   JMP,ABS,        3,3,1,  },          // 76
{   EOR,ABS,        3,4,0,  },          // 77
{   LSR,ABS,        3,6,0,  },          // 78
{   iSRE,ABS,       3,0,0,  },          // 79
{   BVC,REL,        2,2,2,  },          // 80
{   EOR,IZPY,       2,5,0,  },          // 81
{   I_CRA,NONE,     1,0,0,  },          // 82
{   iSRE,IZPY,      2,0,0,  },          // 83
{   I_NO2,IMP,      2,4,0,  },          // 84
{   EOR,ZPX,        2,4,0,  },          // 85
{   LSR,ZPX,        2,6,0,  },          // 86
{   iSRE,ZPX,       2,0,0,  },          // 87
{   CLI,IMP,        1,2,0,  },          // 88
{   EOR,ABSY,       3,4,0,  },          // 89
{   I_NOP,IMP,      1,2,0,  },          // 90
{   iSRE,ABSY,      3,0,0,  },          // 91
{   I_NO3,IMP,      3,6,0,  },          // 92
{   EOR,ABSX,       3,4,0,  },          // 93
{   LSR,ABSX,       3,7,0,  },          // 94
{   iSRE,ABSX,      3,0,0,  },          // 95
{   RTS,IMP,        1,6,1,  },          // 96
{   ADC,IZPX,       2,6,0,  },          // 97
{   I_CRA,NONE,     1,0,0,  },          // 98
{   iRRA,IZPX,      2,0,0,  },          // 99
{   I_NO2,IMP,      2,4,0,  },          // 100
{   ADC,ZP,         2,3,0,  },          // 101
{   ROR,ZP,         2,5,0,  },          // 102
{   iRRA,ZP,        2,0,0,  },          // 103
{   PLA,ACC,        1,4,0,  },          // 104
{   ADC,BYTE,       2,2,0,  },          // 105
{   ROR,ACC,        1,2,0,  },          // 106
{   iARR,BYTE,      2,0,0,  },          // 107
{   JMP,IABS,       3,3,1,  },          // 108
{   ADC,ABS,        3,4,0,  },          // 109
{   ROR,ABS,        3,6,0,  },          // 110
{   iRRA,ABS,       3,0,0,  },          // 111
{   BVS,REL,        2,2,2,  },          // 112
{   ADC,IZPY,       2,5,0,  },          // 113
{   I_CRA,NONE,     1,0,0,  },          // 114
{   iRRA,IZPY,      2,0,0,  },          // 115
{   I_NO2,IMP,      2,4,0,  },          // 116
{   ADC,ZPX,        2,4,0,  },          // 117
{   ROR,ZPX,        2,6,0,  },          // 118
{   iRRA,ZPX,       2,0,0,  },          // 119
{   SEI,IMP,        1,2,0,  },          // 120
{   ADC,ABSY,       3,4,0,  },          // 121
{   I_NOP,IMP,      1,2,0,  },          // 122
{   iRRA,ABSY,      3,0,0,  },          // 123
{   I_NO3,IMP,      3,6,0,  },          // 124
{   ADC,ABSX,       3,4,0,  },          // 125
{   ROR,ABSX,       3,7,0,  },          // 126
{   iRRA,ABSX,      3,0,0,  },          // 127
{   I_NO2,IMP,      2,4,0,  },          // 128
{   STA,IZPX,       2,6,0,  },          // 129
{   I_NO2,IMP,      2,4,0,  },          // 130
{   iSAX,IZPX,      2,0,0,  },          // 131
{   STY,ZP,         2,3,0,  },          // 132
{   STA,ZP,         2,3,0,  },          // 133
{   STX,ZP,         2,3,0,  },          // 134
{   iSAX,ZP,        2,0,0,  },          // 135
{   DEY,IMP,        1,2,0,  },          // 136
{   I_NO2,IMP,      2,4,0,  },          // 137
{   TXA,IMP,        1,2,0,  },          // 138
{   iANE,BYTE,      2,0,0,  },          // 139
{   STY,ABS,        3,4,0,  },          // 140
{   STA,ABS,        3,4,0,  },          // 141
{   STX,ABS,        3,4,0,  },          // 142
{   iSAX,ABS,       3,0,0,  },          // 143
{   BCC,REL,        2,2,2,  },          // 144
{   STA,IZPY,       2,6,0,  },          // 145
{   I_CRA,NONE,     1,0,0,  },          // 146
{   iSHA,IZPY,      2,0,0,  },          // 147
{   STY,ZPX,        2,4,0,  },          // 148
{   STA,ZPX,        2,4,0,  },          // 149
{   STX,ZPY,        2,4,0,  },          // 150
{   iSAX,ZPY,       2,0,0,  },          // 151
{   TYA,IMP,        1,2,0,  },          // 152
{   STA,ABSY,       3,5,0,  },          // 153
{   TXS,IMP,        1,2,0,  },          // 154
{   iSHS,ABSY,      3,0,0,  },          // 155
{   iSHY,ABSX,      3,0,0,  },          // 156
{   STA,ABSX,       3,5,0,  },          // 157
{   iSHX,ABSY,      3,0,0,  },          // 158
{   iSHA,ABSY,      3,0,0,  },          // 159
{   LDY,BYTE,       2,2,0,  },          // 160
{   LDA,IZPX,       2,6,0,  },          // 161
{   LDX,BYTE,       2,2,0,  },          // 162
{   iLAX,IZPX,      2,8,0,  },          // 163
{   LDY,ZP,         2,3,0,  },          // 164
{   LDA,ZP,         2,3,0,  },          // 165
{   LDX,ZP,         2,3,0,  },          // 166
{   iLAX,ZP,        2,6,0,  },          // 167
{   TAY,IMP,        1,2,0,  },          // 168
{   LDA,BYTE,       2,2,0,  },          // 169
{   TAX,IMP,        1,2,0,  },          // 170
{   iLXA,BYTE,      2,0,0,  },          // 171
{   LDY,ABS,        3,4,0,  },          // 172
{   LDA,ABS,        3,4,0,  },          // 173
{   LDX,ABS,        3,4,0,  },          // 174
{   iLAX,ABS,       3,8,0,  },          // 175
{   BCS,REL,        2,2,2,  },          // 176
{   LDA,IZPY,       2,5,0,  },          // 177
{   I_CRA,NONE,     1,0,0,  },          // 178
{   iLAX,IZPY,      2,8,0,  },          // 179
{   LDY,ZPX,        2,4,0,  },          // 180
{   LDA,ZPX,        2,4,0,  },          // 181
{   LDX,ZPY,        2,4,0,  },          // 182
{   iLAX,ZPY,       2,6,0,  },          // 183
{   CLV,IMP,        1,2,0,  },          // 184
{   LDA,ABSY,       3,4,0,  },          // 185
{   TSX,IMP,        1,2,0,  },          // 186
{   iLAS,ABSY,      3,0,0,  },          // 187
{   LDY,ABSX,       3,4,0,  },          // 188
{   LDA,ABSX,       3,4,0,  },          // 189
{   LDX,ABSY,       3,4,0,  },          // 190
{   iLAX,ABSY,      3,7,0,  },          // 191
{   CPY,BYTE,       2,2,0,  },          // 192
{   CMP,IZPX,       2,6,0,  },          // 193
{   I_NO2,IMP,      2,4,0,  },          // 194
{   iDCP,IZPX,      2,9,0,  },          // 195
{   CPY,ZP,         2,3,0,  },          // 196
{   CMP,ZP,         2,3,0,  },          // 197
{   DEC,ZP,         2,5,0,  },          // 198
{   iDCP,ZP,        2,8,0,  },          // 199
{   INY,IMP,        1,2,0,  },          // 200
{   CMP,BYTE,       2,2,0,  },          // 201
{   DEX,IMP,        1,2,0,  },          // 202
{   iSBX,BYTE,      2,0,0,  },          // 203
{   CPY,ABS,        3,4,0,  },          // 204
{   CMP,ABS,        3,3,0,  },          // 205
{   DEC,ABS,        3,6,0,  },          // 206
{   iDCP,ABS,       3,9,0,  },          // 207
{   BNE,REL,        2,2,2,  },          // 208
{   CMP,IZPY,       2,5,0,  },          // 209
{   I_CRA,NONE,     1,0,0,  },          // 210
{   iDCP,IZPY,      2,9,0,  },          // 211
{   I_NO2,IMP,      2,4,0,  },          // 212
{   CMP,ZPX,        2,4,0,  },          // 213
{   DEC,ZPX,        2,6,0,  },          // 214
{   iDCP,ZPX,       2,8,0,  },          // 215h
{   CLD,IMP,        1,2,0,  },          // 216
{   CMP,ABSY,       3,4,0,  },          // 217
{   I_NOP,IMP,      1,2,0,  },          // 218
{   iDCP,ABSY,      3,9,0,  },          // 219
{   I_NO3,IMP,      3,6,0,  },          // 220
{   CMP,ABSX,       3,4,0,  },          // 221
{   DEC,ABSX,       3,7,0,  },          // 222
{   iDCP,ABSX,      3,9,0,  },          // 223
{   CPX,BYTE,       2,2,0,  },          // 224
{   SBC,IZPX,       2,6,0,  },          // 225
{   I_NO2,IMP,      2,4,0,  },          // 226
{   iISB,IZPX,      2,9,0,  },          // 227
{   CPX,ZP,         2,3,0,  },          // 228
{   SBC,ZP,         2,3,0,  },          // 229
{   INC,ZP,         2,5,0,  },          // 230
{   iISB,ZP,        2,7,0,  },          // 231
{   INX,IMP,        1,2,0,  },          // 232
{   SBC,BYTE,       2,2,0,  },          // 233
{   NOP,IMP,        1,2,0,  },          // 234
{   iSBC,BYTE,      2,0,0,  },          // 235
{   CPX,ABS,        3,4,0,  },          // 236
{   SBC,ABS,        3,4,0,  },          // 237
{   INC,ABS,        3,6,0,  },          // 238
{   iISB,ABS,       3,9,0,  },          // 239
{   BEQ,REL,        2,2,2,  },          // 240
{   SBC,IZPY,       2,5,0,  },          // 241
{   I_CRA,NONE,     1,0,0,  },          // 242
{   iISB,IZPY,      2,8,0,  },          // 243
{   I_NO2,IMP,      2,4,0,  },          // 244
{   SBC,ZPX,        3,4,0,  },          // 245
{   INC,ZPX,        2,6,0,  },          // 246
{   iISB,ZPX,       2,8,0,  },          // 247h
{   SED,IMP,        1,2,0,  },          // 248
{   SBC,ABSY,       3,4,0,  },          // 249
{   I_NOP,IMP,      1,2,0,  },          // 250
{   iISB,ABSY,      3,8,0,  },          // 251
{   I_NO3,IMP,      3,6,0,  },          // 252
{   SBC,ABSX,       3,4,0,  },          // 253
{   INC,ABSX,       3,7,0,  },          // 254
{   iISB,ABSX,      3,9,0,  }           // 255
    };

#endif
