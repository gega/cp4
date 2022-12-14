* :ts=8                         scans.asm
*
*    cp4 - Commodore C+4 emulator
*    Copyright (C) 1998 G?ti Gergely
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

        xdef    ctabdouble
        xdef    ctabsingle
        xdef    ctabblanksingle
        xdef    ctabblankdouble
        xdef    cbadlines
        xdef    clinestore              ; line store color/code


        cnop    0,4
clinestore
; SIZE: 328 byte exact
;       #clinestore+328 == cbadlines
;
        dc.l    0       ; ID (GFX_TYPE)
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
; PAD
        dc.l    0

; 0-normal
; 1-badline (incharline, the 2nd badline/charline)
cbadlines
        dc.b    0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0
        dc.b    0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0
        dc.b    0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1
        dc.b    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0
        dc.b    1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0
        dc.b    0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0
        dc.b    0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dc.b    0,0,0
; PAD
        dc.l    0


        cnop    0,4
ctabdouble
        dc.w    1,109
        dc.w    2,109
        dc.w    3,109
        dc.w    4,53
        dc.w    5,53
        dc.w    6,53
        dc.w    7,53
        dc.w    8,53
        dc.w    9,53
        dc.w    10,53
        dc.w    11,53
        dc.w    12,53
        dc.w    13,53
        dc.w    14,53
        dc.w    15,53
        dc.w    16,53
        dc.w    17,53
        dc.w    18,53
        dc.w    19,53
        dc.w    20,53
        dc.w    21,53
        dc.w    22,53
        dc.w    23,53
        dc.w    24,53
        dc.w    25,53
        dc.w    26,53
        dc.w    27,53
        dc.w    28,53
        dc.w    29,53
        dc.w    30,53
        dc.w    31,53
        dc.w    32,53
        dc.w    33,53
        dc.w    34,53
        dc.w    35,53
        dc.w    36,53
        dc.w    37,53
        dc.w    38,53
        dc.w    39,53
        dc.w    40,53
        dc.w    41,53
        dc.w    42,53
        dc.w    43,53
        dc.w    44,53
        dc.w    45,53
        dc.w    46,53
        dc.w    47,53
        dc.w    48,53
        dc.w    49,53
        dc.w    50,53
        dc.w    51,53
        dc.w    52,53
        dc.w    53,53
        dc.w    54,53
        dc.w    55,53
        dc.w    56,53
        dc.w    57,53
        dc.w    58,53
        dc.w    59,53
        dc.w    60,53
        dc.w    61,53
        dc.w    62,53
        dc.w    63,53
        dc.w    64,53
        dc.w    65,53
        dc.w    66,53
        dc.w    67,53
        dc.w    68,53
        dc.w    69,53
        dc.w    70,53
        dc.w    71,53
        dc.w    72,53
        dc.w    73,53
        dc.w    74,53
        dc.w    75,53
        dc.w    76,53
        dc.w    77,53
        dc.w    78,53
        dc.w    79,53
        dc.w    80,53
        dc.w    81,53
        dc.w    82,53
        dc.w    83,53
        dc.w    84,53
        dc.w    85,53
        dc.w    86,53
        dc.w    87,53
        dc.w    88,53
        dc.w    89,53
        dc.w    90,53
        dc.w    91,53
        dc.w    92,53
        dc.w    93,53
        dc.w    94,53
        dc.w    95,53
        dc.w    96,53
        dc.w    97,53
        dc.w    98,53
        dc.w    99,53
        dc.w    100,53
        dc.w    101,53
        dc.w    102,53
        dc.w    103,53
        dc.w    104,53
        dc.w    105,53
        dc.w    106,53
        dc.w    107,53
        dc.w    108,53
        dc.w    109,53
        dc.w    110,53
        dc.w    111,53
        dc.w    112,53
        dc.w    113,53
        dc.w    114,53
        dc.w    115,53
        dc.w    116,53
        dc.w    117,53
        dc.w    118,53
        dc.w    119,53
        dc.w    120,53
        dc.w    121,53
        dc.w    122,53
        dc.w    123,53
        dc.w    124,53
        dc.w    125,53
        dc.w    126,53
        dc.w    127,53
        dc.w    128,53
        dc.w    129,53
        dc.w    130,53
        dc.w    131,53
        dc.w    132,53
        dc.w    133,53
        dc.w    134,53
        dc.w    135,53
        dc.w    136,53
        dc.w    137,53
        dc.w    138,53
        dc.w    139,53
        dc.w    140,53
        dc.w    141,53
        dc.w    142,53
        dc.w    143,53
        dc.w    144,53
        dc.w    145,53
        dc.w    146,53
        dc.w    147,53
        dc.w    148,53
        dc.w    149,53
        dc.w    150,53
        dc.w    151,53
        dc.w    152,53
        dc.w    153,53
        dc.w    154,53
        dc.w    155,53
        dc.w    156,53
        dc.w    157,53
        dc.w    158,53
        dc.w    159,53
        dc.w    160,53
        dc.w    161,53
        dc.w    162,53
        dc.w    163,53
        dc.w    164,53
        dc.w    165,53
        dc.w    166,53
        dc.w    167,53
        dc.w    168,53
        dc.w    169,53
        dc.w    170,53
        dc.w    171,53
        dc.w    172,53
        dc.w    173,53
        dc.w    174,53
        dc.w    175,53
        dc.w    176,53
        dc.w    177,53
        dc.w    178,53
        dc.w    179,53
        dc.w    180,53
        dc.w    181,53
        dc.w    182,53
        dc.w    183,53
        dc.w    184,53
        dc.w    185,53
        dc.w    186,53
        dc.w    187,53
        dc.w    188,53
        dc.w    189,53
        dc.w    190,53
        dc.w    191,53
        dc.w    192,53
        dc.w    193,53
        dc.w    194,53
        dc.w    195,53
        dc.w    196,53
        dc.w    197,53
        dc.w    198,53
        dc.w    199,53
        dc.w    200,53
        dc.w    201,53
        dc.w    202,53
        dc.w    203,53
        dc.w    204,53
        dc.w    205,109
        dc.w    206,109
        dc.w    207,109
        dc.w    208,109
        dc.w    209,109
        dc.w    210,109
        dc.w    211,109
        dc.w    212,109
        dc.w    213,109
        dc.w    214,109
        dc.w    215,109
        dc.w    216,109
        dc.w    217,109
        dc.w    218,109
        dc.w    219,109
        dc.w    220,109
        dc.w    221,109
        dc.w    222,109
        dc.w    223,109
        dc.w    224,109
        dc.w    225,109
        dc.w    226,109
        dc.w    227,109
        dc.w    228,109
        dc.w    229,109
        dc.w    230,109
        dc.w    231,109
        dc.w    232,109
        dc.w    233,109
        dc.w    234,109
        dc.w    235,109
        dc.w    236,109
        dc.w    237,109
        dc.w    238,109
        dc.w    239,109
        dc.w    240,109
        dc.w    241,109
        dc.w    242,109
        dc.w    243,109
        dc.w    244,109
        dc.w    245,109
        dc.w    246,109
        dc.w    247,109
        dc.w    248,109
        dc.w    249,109
        dc.w    250,109
        dc.w    251,109
        dc.w    252,109
        dc.w    253,109
        dc.w    254,109
        dc.w    255,109
        dc.w    256,109
        dc.w    257,109
        dc.w    258,109
        dc.w    259,109
        dc.w    260,109
        dc.w    261,109
        dc.w    262,109
        dc.w    263,109
        dc.w    264,109
        dc.w    265,109
        dc.w    266,109
        dc.w    267,109
        dc.w    268,109
        dc.w    269,109
        dc.w    270,109
        dc.w    271,109
        dc.w    272,109
        dc.w    273,109
        dc.w    274,109
        dc.w    275,109
        dc.w    276,109
        dc.w    277,109
        dc.w    278,109
        dc.w    279,109
        dc.w    280,109
        dc.w    281,109
        dc.w    282,109
        dc.w    283,109
        dc.w    284,109
        dc.w    285,109
        dc.w    286,109
        dc.w    287,109
        dc.w    288,109
        dc.w    289,109
        dc.w    290,109
        dc.w    291,109
        dc.w    292,109
        dc.w    293,109
        dc.w    294,109
        dc.w    295,109
        dc.w    296,109
        dc.w    297,109
        dc.w    298,109
        dc.w    299,109
        dc.w    300,109
        dc.w    301,109
        dc.w    302,109
        dc.w    303,109
        dc.w    304,109
        dc.w    305,109
        dc.w    306,109
        dc.w    307,109
        dc.w    308,109
        dc.w    309,109
        dc.w    310,109
        dc.w    311,109
        dc.w    0,109

        cnop    0,4
ctabsingle
        dc.w    1,57
        dc.w    2,57
        dc.w    3,57
        dc.w    4,14
        dc.w    5,14
        dc.w    6,57
        dc.w    7,57
        dc.w    8,57
        dc.w    9,57
        dc.w    10,57
        dc.w    11,57
        dc.w    12,14
        dc.w    13,14
        dc.w    14,57
        dc.w    15,57
        dc.w    16,57
        dc.w    17,57
        dc.w    18,57
        dc.w    19,57
        dc.w    20,14
        dc.w    21,14
        dc.w    22,57
        dc.w    23,57
        dc.w    24,57
        dc.w    25,57
        dc.w    26,57
        dc.w    27,57
        dc.w    28,14
        dc.w    29,14
        dc.w    30,57
        dc.w    31,57
        dc.w    32,57
        dc.w    33,57
        dc.w    34,57
        dc.w    35,57
        dc.w    36,14
        dc.w    37,14
        dc.w    38,57
        dc.w    39,57
        dc.w    40,57
        dc.w    41,57
        dc.w    42,57
        dc.w    43,57
        dc.w    44,14
        dc.w    45,14
        dc.w    46,57
        dc.w    47,57
        dc.w    48,57
        dc.w    49,57
        dc.w    50,57
        dc.w    51,57
        dc.w    52,14
        dc.w    53,14
        dc.w    54,57
        dc.w    55,57
        dc.w    56,57
        dc.w    57,57
        dc.w    58,57
        dc.w    59,57
        dc.w    60,14
        dc.w    61,14
        dc.w    62,57
        dc.w    63,57
        dc.w    64,57
        dc.w    65,57
        dc.w    66,57
        dc.w    67,57
        dc.w    68,14
        dc.w    69,14
        dc.w    70,57
        dc.w    71,57
        dc.w    72,57
        dc.w    73,57
        dc.w    74,57
        dc.w    75,57
        dc.w    76,14
        dc.w    77,14
        dc.w    78,57
        dc.w    79,57
        dc.w    80,57
        dc.w    81,57
        dc.w    82,57
        dc.w    83,57
        dc.w    84,14
        dc.w    85,14
        dc.w    86,57
        dc.w    87,57
        dc.w    88,57
        dc.w    89,57
        dc.w    90,57
        dc.w    91,57
        dc.w    92,14
        dc.w    93,14
        dc.w    94,57
        dc.w    95,57
        dc.w    96,57
        dc.w    97,57
        dc.w    98,57
        dc.w    99,57
        dc.w    100,14
        dc.w    101,14
        dc.w    102,57
        dc.w    103,57
        dc.w    104,57
        dc.w    105,57
        dc.w    106,57
        dc.w    107,57
        dc.w    108,14
        dc.w    109,14
        dc.w    110,57
        dc.w    111,57
        dc.w    112,57
        dc.w    113,57
        dc.w    114,57
        dc.w    115,57
        dc.w    116,14
        dc.w    117,14
        dc.w    118,57
        dc.w    119,57
        dc.w    120,57
        dc.w    121,57
        dc.w    122,57
        dc.w    123,57
        dc.w    124,14
        dc.w    125,14
        dc.w    126,57
        dc.w    127,57
        dc.w    128,57
        dc.w    129,57
        dc.w    130,57
        dc.w    131,57
        dc.w    132,14
        dc.w    133,14
        dc.w    134,57
        dc.w    135,57
        dc.w    136,57
        dc.w    137,57
        dc.w    138,57
        dc.w    139,57
        dc.w    140,14
        dc.w    141,14
        dc.w    142,57
        dc.w    143,57
        dc.w    144,57
        dc.w    145,57
        dc.w    146,57
        dc.w    147,57
        dc.w    148,14
        dc.w    149,14
        dc.w    150,57
        dc.w    151,57
        dc.w    152,57
        dc.w    153,57
        dc.w    154,57
        dc.w    155,57
        dc.w    156,14
        dc.w    157,14
        dc.w    158,57
        dc.w    159,57
        dc.w    160,57
        dc.w    161,57
        dc.w    162,57
        dc.w    163,57
        dc.w    164,14
        dc.w    165,14
        dc.w    166,57
        dc.w    167,57
        dc.w    168,57
        dc.w    169,57
        dc.w    170,57
        dc.w    171,57
        dc.w    172,14
        dc.w    173,14
        dc.w    174,57
        dc.w    175,57
        dc.w    176,57
        dc.w    177,57
        dc.w    178,57
        dc.w    179,57
        dc.w    180,14
        dc.w    181,14
        dc.w    182,57
        dc.w    183,57
        dc.w    184,57
        dc.w    185,57
        dc.w    186,57
        dc.w    187,57
        dc.w    188,14
        dc.w    189,14
        dc.w    190,57
        dc.w    191,57
        dc.w    192,57
        dc.w    193,57
        dc.w    194,57
        dc.w    195,57
        dc.w    196,14
        dc.w    197,14
        dc.w    198,57
        dc.w    199,57
        dc.w    200,57
        dc.w    201,57
        dc.w    202,57
        dc.w    203,57
        dc.w    204,14
        dc.w    205,57
        dc.w    206,57
        dc.w    207,57
        dc.w    208,57
        dc.w    209,57
        dc.w    210,57
        dc.w    211,57
        dc.w    212,57
        dc.w    213,57
        dc.w    214,57
        dc.w    215,57
        dc.w    216,57
        dc.w    217,57
        dc.w    218,57
        dc.w    219,57
        dc.w    220,57
        dc.w    221,57
        dc.w    222,57
        dc.w    223,57
        dc.w    224,57
        dc.w    225,57
        dc.w    226,57
        dc.w    227,57
        dc.w    228,57
        dc.w    229,57
        dc.w    230,57
        dc.w    231,57
        dc.w    232,57
        dc.w    233,57
        dc.w    234,57
        dc.w    235,57
        dc.w    236,57
        dc.w    237,57
        dc.w    238,57
        dc.w    239,57
        dc.w    240,57
        dc.w    241,57
        dc.w    242,57
        dc.w    243,57
        dc.w    244,57
        dc.w    245,57
        dc.w    246,57
        dc.w    247,57
        dc.w    248,57
        dc.w    249,57
        dc.w    250,57
        dc.w    251,57
        dc.w    252,57
        dc.w    253,57
        dc.w    254,57
        dc.w    255,57
        dc.w    256,57
        dc.w    257,57
        dc.w    258,57
        dc.w    259,57
        dc.w    260,57
        dc.w    261,57
        dc.w    262,57
        dc.w    263,57
        dc.w    264,57
        dc.w    265,57
        dc.w    266,57
        dc.w    267,57
        dc.w    268,57
        dc.w    269,57
        dc.w    270,57
        dc.w    271,57
        dc.w    272,57
        dc.w    273,57
        dc.w    274,57
        dc.w    275,57
        dc.w    276,57
        dc.w    277,57
        dc.w    278,57
        dc.w    279,57
        dc.w    280,57
        dc.w    281,57
        dc.w    282,57
        dc.w    283,57
        dc.w    284,57
        dc.w    285,57
        dc.w    286,57
        dc.w    287,57
        dc.w    288,57
        dc.w    289,57
        dc.w    290,57
        dc.w    291,57
        dc.w    292,57
        dc.w    293,57
        dc.w    294,57
        dc.w    295,57
        dc.w    296,57
        dc.w    297,57
        dc.w    298,57
        dc.w    299,57
        dc.w    300,57
        dc.w    301,57
        dc.w    302,57
        dc.w    303,57
        dc.w    304,57
        dc.w    305,57
        dc.w    306,57
        dc.w    307,57
        dc.w    308,57
        dc.w    309,57
        dc.w    310,57
        dc.w    311,57
        dc.w    0,57

        cnop    0,4
ctabblankdouble
        dc.w    1,109
        dc.w    2,109
        dc.w    3,109
        dc.w    4,109
        dc.w    5,109
        dc.w    6,109
        dc.w    7,109
        dc.w    8,109
        dc.w    9,109
        dc.w    10,109
        dc.w    11,109
        dc.w    12,109
        dc.w    13,109
        dc.w    14,109
        dc.w    15,109
        dc.w    16,109
        dc.w    17,109
        dc.w    18,109
        dc.w    19,109
        dc.w    20,109
        dc.w    21,109
        dc.w    22,109
        dc.w    23,109
        dc.w    24,109
        dc.w    25,109
        dc.w    26,109
        dc.w    27,109
        dc.w    28,109
        dc.w    29,109
        dc.w    30,109
        dc.w    31,109
        dc.w    32,109
        dc.w    33,109
        dc.w    34,109
        dc.w    35,109
        dc.w    36,109
        dc.w    37,109
        dc.w    38,109
        dc.w    39,109
        dc.w    40,109
        dc.w    41,109
        dc.w    42,109
        dc.w    43,109
        dc.w    44,109
        dc.w    45,109
        dc.w    46,109
        dc.w    47,109
        dc.w    48,109
        dc.w    49,109
        dc.w    50,109
        dc.w    51,109
        dc.w    52,109
        dc.w    53,109
        dc.w    54,109
        dc.w    55,109
        dc.w    56,109
        dc.w    57,109
        dc.w    58,109
        dc.w    59,109
        dc.w    60,109
        dc.w    61,109
        dc.w    62,109
        dc.w    63,109
        dc.w    64,109
        dc.w    65,109
        dc.w    66,109
        dc.w    67,109
        dc.w    68,109
        dc.w    69,109
        dc.w    70,109
        dc.w    71,109
        dc.w    72,109
        dc.w    73,109
        dc.w    74,109
        dc.w    75,109
        dc.w    76,109
        dc.w    77,109
        dc.w    78,109
        dc.w    79,109
        dc.w    80,109
        dc.w    81,109
        dc.w    82,109
        dc.w    83,109
        dc.w    84,109
        dc.w    85,109
        dc.w    86,109
        dc.w    87,109
        dc.w    88,109
        dc.w    89,109
        dc.w    90,109
        dc.w    91,109
        dc.w    92,109
        dc.w    93,109
        dc.w    94,109
        dc.w    95,109
        dc.w    96,109
        dc.w    97,109
        dc.w    98,109
        dc.w    99,109
        dc.w    100,109
        dc.w    101,109
        dc.w    102,109
        dc.w    103,109
        dc.w    104,109
        dc.w    105,109
        dc.w    106,109
        dc.w    107,109
        dc.w    108,109
        dc.w    109,109
        dc.w    110,109
        dc.w    111,109
        dc.w    112,109
        dc.w    113,109
        dc.w    114,109
        dc.w    115,109
        dc.w    116,109
        dc.w    117,109
        dc.w    118,109
        dc.w    119,109
        dc.w    120,109
        dc.w    121,109
        dc.w    122,109
        dc.w    123,109
        dc.w    124,109
        dc.w    125,109
        dc.w    126,109
        dc.w    127,109
        dc.w    128,109
        dc.w    129,109
        dc.w    130,109
        dc.w    131,109
        dc.w    132,109
        dc.w    133,109
        dc.w    134,109
        dc.w    135,109
        dc.w    136,109
        dc.w    137,109
        dc.w    138,109
        dc.w    139,109
        dc.w    140,109
        dc.w    141,109
        dc.w    142,109
        dc.w    143,109
        dc.w    144,109
        dc.w    145,109
        dc.w    146,109
        dc.w    147,109
        dc.w    148,109
        dc.w    149,109
        dc.w    150,109
        dc.w    151,109
        dc.w    152,109
        dc.w    153,109
        dc.w    154,109
        dc.w    155,109
        dc.w    156,109
        dc.w    157,109
        dc.w    158,109
        dc.w    159,109
        dc.w    160,109
        dc.w    161,109
        dc.w    162,109
        dc.w    163,109
        dc.w    164,109
        dc.w    165,109
        dc.w    166,109
        dc.w    167,109
        dc.w    168,109
        dc.w    169,109
        dc.w    170,109
        dc.w    171,109
        dc.w    172,109
        dc.w    173,109
        dc.w    174,109
        dc.w    175,109
        dc.w    176,109
        dc.w    177,109
        dc.w    178,109
        dc.w    179,109
        dc.w    180,109
        dc.w    181,109
        dc.w    182,109
        dc.w    183,109
        dc.w    184,109
        dc.w    185,109
        dc.w    186,109
        dc.w    187,109
        dc.w    188,109
        dc.w    189,109
        dc.w    190,109
        dc.w    191,109
        dc.w    192,109
        dc.w    193,109
        dc.w    194,109
        dc.w    195,109
        dc.w    196,109
        dc.w    197,109
        dc.w    198,109
        dc.w    199,109
        dc.w    200,109
        dc.w    201,109
        dc.w    202,109
        dc.w    203,109
        dc.w    204,109
        dc.w    205,109
        dc.w    206,109
        dc.w    207,109
        dc.w    208,109
        dc.w    209,109
        dc.w    210,109
        dc.w    211,109
        dc.w    212,109
        dc.w    213,109
        dc.w    214,109
        dc.w    215,109
        dc.w    216,109
        dc.w    217,109
        dc.w    218,109
        dc.w    219,109
        dc.w    220,109
        dc.w    221,109
        dc.w    222,109
        dc.w    223,109
        dc.w    224,109
        dc.w    225,109
        dc.w    226,109
        dc.w    227,109
        dc.w    228,109
        dc.w    229,109
        dc.w    230,109
        dc.w    231,109
        dc.w    232,109
        dc.w    233,109
        dc.w    234,109
        dc.w    235,109
        dc.w    236,109
        dc.w    237,109
        dc.w    238,109
        dc.w    239,109
        dc.w    240,109
        dc.w    241,109
        dc.w    242,109
        dc.w    243,109
        dc.w    244,109
        dc.w    245,109
        dc.w    246,109
        dc.w    247,109
        dc.w    248,109
        dc.w    249,109
        dc.w    250,109
        dc.w    251,109
        dc.w    252,109
        dc.w    253,109
        dc.w    254,109
        dc.w    255,109
        dc.w    256,109
        dc.w    257,109
        dc.w    258,109
        dc.w    259,109
        dc.w    260,109
        dc.w    261,109
        dc.w    262,109
        dc.w    263,109
        dc.w    264,109
        dc.w    265,109
        dc.w    266,109
        dc.w    267,109
        dc.w    268,109
        dc.w    269,109
        dc.w    270,109
        dc.w    271,109
        dc.w    272,109
        dc.w    273,109
        dc.w    274,109
        dc.w    275,109
        dc.w    276,109
        dc.w    277,109
        dc.w    278,109
        dc.w    279,109
        dc.w    280,109
        dc.w    281,109
        dc.w    282,109
        dc.w    283,109
        dc.w    284,109
        dc.w    285,109
        dc.w    286,109
        dc.w    287,109
        dc.w    288,109
        dc.w    289,109
        dc.w    290,109
        dc.w    291,109
        dc.w    292,109
        dc.w    293,109
        dc.w    294,109
        dc.w    295,109
        dc.w    296,109
        dc.w    297,109
        dc.w    298,109
        dc.w    299,109
        dc.w    300,109
        dc.w    301,109
        dc.w    302,109
        dc.w    303,109
        dc.w    304,109
        dc.w    305,109
        dc.w    306,109
        dc.w    307,109
        dc.w    308,109
        dc.w    309,109
        dc.w    310,109
        dc.w    311,109
        dc.w    0,109

        cnop    0,4
ctabblanksingle
        dc.w    1,57
        dc.w    2,57
        dc.w    3,57
        dc.w    4,57
        dc.w    5,57
        dc.w    6,57
        dc.w    7,57
        dc.w    8,57
        dc.w    9,57
        dc.w    10,57
        dc.w    11,57
        dc.w    12,57
        dc.w    13,57
        dc.w    14,57
        dc.w    15,57
        dc.w    16,57
        dc.w    17,57
        dc.w    18,57
        dc.w    19,57
        dc.w    20,57
        dc.w    21,57
        dc.w    22,57
        dc.w    23,57
        dc.w    24,57
        dc.w    25,57
        dc.w    26,57
        dc.w    27,57
        dc.w    28,57
        dc.w    29,57
        dc.w    30,57
        dc.w    31,57
        dc.w    32,57
        dc.w    33,57
        dc.w    34,57
        dc.w    35,57
        dc.w    36,57
        dc.w    37,57
        dc.w    38,57
        dc.w    39,57
        dc.w    40,57
        dc.w    41,57
        dc.w    42,57
        dc.w    43,57
        dc.w    44,57
        dc.w    45,57
        dc.w    46,57
        dc.w    47,57
        dc.w    48,57
        dc.w    49,57
        dc.w    50,57
        dc.w    51,57
        dc.w    52,57
        dc.w    53,57
        dc.w    54,57
        dc.w    55,57
        dc.w    56,57
        dc.w    57,57
        dc.w    58,57
        dc.w    59,57
        dc.w    60,57
        dc.w    61,57
        dc.w    62,57
        dc.w    63,57
        dc.w    64,57
        dc.w    65,57
        dc.w    66,57
        dc.w    67,57
        dc.w    68,57
        dc.w    69,57
        dc.w    70,57
        dc.w    71,57
        dc.w    72,57
        dc.w    73,57
        dc.w    74,57
        dc.w    75,57
        dc.w    76,57
        dc.w    77,57
        dc.w    78,57
        dc.w    79,57
        dc.w    80,57
        dc.w    81,57
        dc.w    82,57
        dc.w    83,57
        dc.w    84,57
        dc.w    85,57
        dc.w    86,57
        dc.w    87,57
        dc.w    88,57
        dc.w    89,57
        dc.w    90,57
        dc.w    91,57
        dc.w    92,57
        dc.w    93,57
        dc.w    94,57
        dc.w    95,57
        dc.w    96,57
        dc.w    97,57
        dc.w    98,57
        dc.w    99,57
        dc.w    100,57
        dc.w    101,57
        dc.w    102,57
        dc.w    103,57
        dc.w    104,57
        dc.w    105,57
        dc.w    106,57
        dc.w    107,57
        dc.w    108,57
        dc.w    109,57
        dc.w    110,57
        dc.w    111,57
        dc.w    112,57
        dc.w    113,57
        dc.w    114,57
        dc.w    115,57
        dc.w    116,57
        dc.w    117,57
        dc.w    118,57
        dc.w    119,57
        dc.w    120,57
        dc.w    121,57
        dc.w    122,57
        dc.w    123,57
        dc.w    124,57
        dc.w    125,57
        dc.w    126,57
        dc.w    127,57
        dc.w    128,57
        dc.w    129,57
        dc.w    130,57
        dc.w    131,57
        dc.w    132,57
        dc.w    133,57
        dc.w    134,57
        dc.w    135,57
        dc.w    136,57
        dc.w    137,57
        dc.w    138,57
        dc.w    139,57
        dc.w    140,57
        dc.w    141,57
        dc.w    142,57
        dc.w    143,57
        dc.w    144,57
        dc.w    145,57
        dc.w    146,57
        dc.w    147,57
        dc.w    148,57
        dc.w    149,57
        dc.w    150,57
        dc.w    151,57
        dc.w    152,57
        dc.w    153,57
        dc.w    154,57
        dc.w    155,57
        dc.w    156,57
        dc.w    157,57
        dc.w    158,57
        dc.w    159,57
        dc.w    160,57
        dc.w    161,57
        dc.w    162,57
        dc.w    163,57
        dc.w    164,57
        dc.w    165,57
        dc.w    166,57
        dc.w    167,57
        dc.w    168,57
        dc.w    169,57
        dc.w    170,57
        dc.w    171,57
        dc.w    172,57
        dc.w    173,57
        dc.w    174,57
        dc.w    175,57
        dc.w    176,57
        dc.w    177,57
        dc.w    178,57
        dc.w    179,57
        dc.w    180,57
        dc.w    181,57
        dc.w    182,57
        dc.w    183,57
        dc.w    184,57
        dc.w    185,57
        dc.w    186,57
        dc.w    187,57
        dc.w    188,57
        dc.w    189,57
        dc.w    190,57
        dc.w    191,57
        dc.w    192,57
        dc.w    193,57
        dc.w    194,57
        dc.w    195,57
        dc.w    196,57
        dc.w    197,57
        dc.w    198,57
        dc.w    199,57
        dc.w    200,57
        dc.w    201,57
        dc.w    202,57
        dc.w    203,57
        dc.w    204,57
        dc.w    205,57
        dc.w    206,57
        dc.w    207,57
        dc.w    208,57
        dc.w    209,57
        dc.w    210,57
        dc.w    211,57
        dc.w    212,57
        dc.w    213,57
        dc.w    214,57
        dc.w    215,57
        dc.w    216,57
        dc.w    217,57
        dc.w    218,57
        dc.w    219,57
        dc.w    220,57
        dc.w    221,57
        dc.w    222,57
        dc.w    223,57
        dc.w    224,57
        dc.w    225,57
        dc.w    226,57
        dc.w    227,57
        dc.w    228,57
        dc.w    229,57
        dc.w    230,57
        dc.w    231,57
        dc.w    232,57
        dc.w    233,57
        dc.w    234,57
        dc.w    235,57
        dc.w    236,57
        dc.w    237,57
        dc.w    238,57
        dc.w    239,57
        dc.w    240,57
        dc.w    241,57
        dc.w    242,57
        dc.w    243,57
        dc.w    244,57
        dc.w    245,57
        dc.w    246,57
        dc.w    247,57
        dc.w    248,57
        dc.w    249,57
        dc.w    250,57
        dc.w    251,57
        dc.w    252,57
        dc.w    253,57
        dc.w    254,57
        dc.w    255,57
        dc.w    256,57
        dc.w    257,57
        dc.w    258,57
        dc.w    259,57
        dc.w    260,57
        dc.w    261,57
        dc.w    262,57
        dc.w    263,57
        dc.w    264,57
        dc.w    265,57
        dc.w    266,57
        dc.w    267,57
        dc.w    268,57
        dc.w    269,57
        dc.w    270,57
        dc.w    271,57
        dc.w    272,57
        dc.w    273,57
        dc.w    274,57
        dc.w    275,57
        dc.w    276,57
        dc.w    277,57
        dc.w    278,57
        dc.w    279,57
        dc.w    280,57
        dc.w    281,57
        dc.w    282,57
        dc.w    283,57
        dc.w    284,57
        dc.w    285,57
        dc.w    286,57
        dc.w    287,57
        dc.w    288,57
        dc.w    289,57
        dc.w    290,57
        dc.w    291,57
        dc.w    292,57
        dc.w    293,57
        dc.w    294,57
        dc.w    295,57
        dc.w    296,57
        dc.w    297,57
        dc.w    298,57
        dc.w    299,57
        dc.w    300,57
        dc.w    301,57
        dc.w    302,57
        dc.w    303,57
        dc.w    304,57
        dc.w    305,57
        dc.w    306,57
        dc.w    307,57
        dc.w    308,57
        dc.w    309,57
        dc.w    310,57
        dc.w    311,57
        dc.w    0,57

