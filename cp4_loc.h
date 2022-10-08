/* :ts=4							cp4_loc.h
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
#ifndef CP4_LOC_H
#define CP4_LOC_H

#include <exec/types.h>

void Opencp4Catalog(char *language);
char *GetStr(long strnum);
void Closecp4Catalog(void);

#define MSG_003A 0
#define MSG_003B 1
#define MSG_003C 2
#define MSG_005E 3
#define MSG_0060 4
#define MSG_0063 5
#define MSG_009A 6
#define MSG_00AC 7
#define MSG_00AD 8
#define MSG_00FE 9
#define MSG_0101 10
#define MSG_0102 11
#define MSG_0103 12
#define MSG_0104 13
#define MSG_0114 14
#define MSG_0115 15
#define MSG_0116 16
#define MSG_0117 17
#define MSG_0118 18
#define MSG_0119 19
#define MSG_011A 20
#define MSG_011B 21
#define MSG_011C 22
#define MSG_011D 23
#define MSG_011E 24
#define MSG_011F 25
#define MSG_0120 26
#define MSG_0121 27
#define MSG_0122 28
#define MSG_0123 29
#define MSG_0124 30
#define MSG_0125 31
#define MSG_0126 32
#define MSG_0127 33
#define MSG_0128 34
#define MSG_0129 35
#define MSG_012A 36
#define MSG_012B 37
#define MSG_012C 38
#define MSG_012D 39
#define MSG_012E 40
#define MSG_0110 41
#define MSG_012F 42
#define MSG_0130 43
#define MSG_0131 44
#define MSG_0136 45
#define MSG_0137 46
#define MSG_0138 47
#define MSG_0139 48
#define MSG_013A 49
#define MSG_013B 50
#define MSG_013C 51
#define MSG_013D 52
#define MSG_013E 53
#define MSG_013F 54
#define MSG_0140 55
#define MSG_0142 56
#define MSG_0143 57
#define MSG_0144 58
#define MSG_0145 59
#define MSG_0146 60
#define MSG_0147 61
#define MSG_0148 62
#define MSG_017A 63
#define MSG_017B 64
#define MSG_017C 65
#define MSG_017D 66
#define MSG_0180 67
#define MSG_0181 68
#define MSG_0182 69
#define MSG_0183 70
#define MSG_0184 71
#define MSG_0185 72
#define MSG_0186 73
#define MSG_0187 74
#define MSG_0188 75
#define MSG_0189 76
#define MSG_018A 77
#define MSG_018B 78
#define MSG_018C 79
#define MSG_018D 80
#define MSG_018E 81
#define MSG_018F 82
#define MSG_0190 83
#define MSG_0191 84
#define MSG_0192 85
#define MSG_0193 86
#define MSG_0194 87
#define MSG_0195 88
#define MSG_0196 89
#define MSG_0197 90
#define MSG_0198 91
#define MSG_01C7 92
#define MSG_01C8 93
#define MSG_01CA 94
#define MSG_01CB 95
#define MSG_01CE 96
#define MSG_01CF 97
#define MSG_01D0 98
#define MSG_01D1 99
#define MSG_01D2 100
#define MSG_01D4 101
#define MSG_01D5 102
#define MSG_01D6 103
#define MSG_01D8 104
#define MSG_01D9 105
#define MSG_01DA 106
#define MSG_01DB 107
#define MSG_01DC 108
#define MSG_01E1 109
#define MSG_01E2 110
#define MSG_01E3 111
#define MSG_01E4 112
#define MSG_01E5 113
#define MSG_01E6 114
#define MSG_01E7 115
#define MSG_01E8 116
#define MSG_01E9 117
#define MSG_01EA 118
#define MSG_01EB 119
#define MSG_01EE 120
#define MSG_01EF 121
#define MSG_01F0 122
#define MSG_01F1 123
#define MSG_01F2 124
#define MSG_01F3 125
#define MSG_01F5 126
#define MSG_0161 127
#define MSG_01F6 128
#define MSG_01F7 129
#define MSG_01F8 130
#define MSG_01F9 131
#define MSG_01FA 132
#define MSG_01FB 133
#define MSG_0168 134
#define MSG_01FC 135
#define MSG_0176 136
#define MSG_0162 137
#define MSG_01FD 138
#define MSG_0069 139
#define MSG_01FE 140
#define MSG_0064 141
#define MSG_0158 142
#define MSG_01FF 143
#define MSG_0200 144
#define MSG_0201 145
#define MSG_0202 146
#define MSG_0203 147
#define MSG_0204 148
#define MSG_0205 149
#define MSG_020B 150
#define MSG_020C 151
#define MSG_020D 152
#define MSG_0210 153
#define MSG_0211 154
#define MSG_0213 155
#define MSG_0214 156
#define MSG_0215 157
#define MSG_0216 158
#define MSG_0217 159
#define MSG_0218 160
#define MSG_0219 161
#define MSG_021A 162
#define MSG_021B 163
#define MSG_0220 164
#define MSG_0221 165
#define MSG_0222 166
#define MSG_0223 167
#define MSG_0224 168
#define MSG_0225 169
#define MSG_0226 170
#define MSG_0227 171
#define MSG_0228 172
#define MSG_022C 173
#define MSG_022D 174
#define MSG_022E 175
#define MSG_0240 176
#define MSG_0242 177
#define MSG_0243 178
#define MSG_0245 179
#define MSG_0246 180
#define MSG_0247 181
#define MSG_0249 182
#define MSG_024A 183
#define MSG_024B 184
#define MSG_024C 185
#define MSG_024E 186
#define MSG_0250 187
#define MSG_0252 188
#define MSG_0253 189
#define MSG_0254 190
#define MSG_0255 191
#define MSG_0256 192
#define MSG_0258 193
#define MSG_0259 194
#define MSG_025A 195
#define MSG_025B 196
#define MSG_025C 197
#define MSG_025D 198
#define MSG_025E 199
#define MSG_025F 200
#define MSG_0260 201
#define MSG_0261 202
#define MSG_0262 203
#define MSG_0263 204
#define MSG_0264 205
#define MSG_0265 206
#define MSG_0266 207
#define MSG_0267 208
#define MSG_0268 209
#define MSG_0300 210		// drive
#define MSG_0301 211		// d64
#define MSG_0302 212		// real 1541
#define MSG_0303 213		// none
#define MSG_0304 214		// select a d64 file
#define MSG_0305 215		// make snapshot
#define MSG_0306 216		// make snapshot hotkey
#define MSG_0307 217		// load snapshot
#define MSG_0308 218		// load snapshot hotkey
#define MSG_0309 219		// not a snapshot file
#define MSG_0310 220		// func: snapshot
#define MSG_0311 221		// func: restore
#define MSG_0312 222		// menu: palette
#define MSG_0313 223		// shortcut: palette
#define MSG_0314 224		// Select a palette file
#define MSG_0315 225		// Unknown snapshot subtype
#define MSG_0316 226		// Can't open xpkmaster.library
#define MSG_0317 227		// Checksum error
#define MSG_0318 228		// Xpk
#define MSG_0319 229		// On
#define MSG_0320 230		// X


#endif
