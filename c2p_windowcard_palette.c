/* :ts=4							c2p_windowcard_palette.c
 *
 *    cp4 - Commodore C+4 emulator
 *    Copyright (C) 1998 G�ti Gergely
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
#ifndef C2P_WINDOWCARD_PALETTE_C
#define C2P_WINDOWCARD_PALETTE_C

static ULONG Palette[]={
	0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,
	0x30303030,0x05050505,0x05050505,
	0x00000000,0x18181818,0x14141414,
	0x40404040,0x00000000,0x40404040,
	0x00000000,0x18181818,0x00000000,
	0x00000000,0x00000000,0x40404040,
	0x18181818,0x18181818,0x00000000,
	0x5A5A5A5A,0x2A2A2A2A,0x00000000,
	0x28282828,0x14141414,0x00000000,
	0x18181818,0x20202020,0x0C0C0C0C,
	0x48484848,0x00000000,0x00000000,
	0x00000000,0x30303030,0x2C2C2C2C,
	0x00000000,0x00000000,0x40404040,
	0x00000000,0x00000000,0x40404040,
	0x00000000,0x18181818,0x00000000,
	0x00000000,0x00000000,0x00000000,
	0x24242424,0x24242424,0x24242424,
	0x60606060,0x08080808,0x08080808,
	0x00000000,0x38383838,0x34343434,
	0x58585858,0x08080808,0x5C5C5C5C,
	0x08080808,0x38383838,0x04040404,
	0x08080808,0x08080808,0x5C5C5C5C,
	0x38383838,0x38383838,0x00000000,
	0x70707070,0x34343434,0x00000000,
	0x48484848,0x24242424,0x04040404,
	0x34343434,0x40404040,0x18181818,
	0x64646464,0x04040404,0x08080808,
	0x00000000,0x4C4C4C4C,0x44444444,
	0x0C0C0C0C,0x10101010,0x5C5C5C5C,
	0x08080808,0x0C0C0C0C,0x5C5C5C5C,
	0x08080808,0x38383838,0x04040404,
	0x00000000,0x00000000,0x00000000,
	0x48484848,0x48484848,0x48484848,
	0x7C7C7C7C,0x14141414,0x14141414,
	0x00000000,0x58585858,0x54545454,
	0x74747474,0x14141414,0x74747474,
	0x14141414,0x58585858,0x0C0C0C0C,
	0x14141414,0x18181818,0x74747474,
	0x58585858,0x58585858,0x04040404,
	0x84848484,0x48484848,0x04040404,
	0x64646464,0x38383838,0x10101010,
	0x4C4C4C4C,0x60606060,0x24242424,
	0x7C7C7C7C,0x08080808,0x18181818,
	0x00000000,0x6C6C6C6C,0x5C5C5C5C,
	0x1C1C1C1C,0x2C2C2C2C,0x74747474,
	0x18181818,0x1C1C1C1C,0x74747474,
	0x18181818,0x58585858,0x0C0C0C0C,
	0x00000000,0x00000000,0x00000000,
	0x6C6C6C6C,0x6C6C6C6C,0x6C6C6C6C,
	0x94949494,0x24242424,0x24242424,
	0x00000000,0x78787878,0x78787878,
	0x90909090,0x24242424,0x90909090,
	0x20202020,0x78787878,0x18181818,
	0x24242424,0x28282828,0x90909090,
	0x78787878,0x78787878,0x08080808,
	0x9C9C9C9C,0x58585858,0x04040404,
	0x84848484,0x50505050,0x24242424,
	0x68686868,0x80808080,0x34343434,
	0x94949494,0x10101010,0x28282828,
	0x00000000,0x88888888,0x78787878,
	0x34343434,0x4C4C4C4C,0x90909090,
	0x2C2C2C2C,0x30303030,0x90909090,
	0x2C2C2C2C,0x78787878,0x1C1C1C1C,
	0x00000000,0x00000000,0x00000000,
	0x90909090,0x90909090,0x90909090,
	0xB0B0B0B0,0x38383838,0x38383838,
	0x00000000,0x98989898,0x94949494,
	0xACACACAC,0x3C3C3C3C,0xACACACAC,
	0x30303030,0x98989898,0x2C2C2C2C,
	0x3C3C3C3C,0x40404040,0xACACACAC,
	0x98989898,0x94949494,0x10101010,
	0xB4B4B4B4,0x70707070,0x08080808,
	0xA0A0A0A0,0x68686868,0x38383838,
	0x80808080,0x9C9C9C9C,0x40404040,
	0xB0B0B0B0,0x18181818,0x3C3C3C3C,
	0x00000000,0xA4A4A4A4,0x90909090,
	0x54545454,0x70707070,0xACACACAC,
	0x48484848,0x4C4C4C4C,0xACACACAC,
	0x48484848,0x98989898,0x34343434,
	0x00000000,0x00000000,0x00000000,
	0xB4B4B4B4,0xB4B4B4B4,0xB4B4B4B4,
	0xC8C8C8C8,0x50505050,0x50505050,
	0x00000000,0xBCBCBCBC,0xB4B4B4B4,
	0xC4C4C4C4,0x58585858,0xC4C4C4C4,
	0x48484848,0xBCBCBCBC,0x40404040,
	0x58585858,0x58585858,0xC4C4C4C4,
	0xBCBCBCBC,0xB4B4B4B4,0x14141414,
	0xCCCCCCCC,0x84848484,0x0C0C0C0C,
	0xC0C0C0C0,0x88888888,0x54545454,
	0x9C9C9C9C,0xBCBCBCBC,0x4C4C4C4C,
	0xC8C8C8C8,0x24242424,0x58585858,
	0x00000000,0xC0C0C0C0,0xACACACAC,
	0x78787878,0x98989898,0xC4C4C4C4,
	0x68686868,0x6C6C6C6C,0xC4C4C4C4,
	0x68686868,0xBCBCBCBC,0x4C4C4C4C,
	0x00000000,0x00000000,0x00000000,
	0xD8D8D8D8,0xD8D8D8D8,0xD8D8D8D8,
	0xE0E0E0E0,0x6C6C6C6C,0x6C6C6C6C,
	0x00000000,0xDCDCDCDC,0xDCDCDCDC,
	0xE0E0E0E0,0x78787878,0xE0E0E0E0,
	0x64646464,0xDCDCDCDC,0x5C5C5C5C,
	0x78787878,0x78787878,0xE0E0E0E0,
	0xDCDCDCDC,0xD4D4D4D4,0x20202020,
	0xE4E4E4E4,0x9C9C9C9C,0x14141414,
	0xDCDCDCDC,0xA4A4A4A4,0x74747474,
	0xB4B4B4B4,0xDCDCDCDC,0x58585858,
	0xE0E0E0E0,0x30303030,0x70707070,
	0x00000000,0xE0E0E0E0,0xC4C4C4C4,
	0xA4A4A4A4,0xC0C0C0C0,0xE0E0E0E0,
	0x90909090,0x94949494,0xE0E0E0E0,
	0x90909090,0xDCDCDCDC,0x6C6C6C6C,
	0x00000000,0x00000000,0x00000000,
	0xFCFCFCFC,0xFCFCFCFC,0xFCFCFCFC,
	0xFCFCFCFC,0x8C8C8C8C,0x8C8C8C8C,
	0x00000000,0xFCFCFCFC,0xFCFCFCFC,
	0xFFFFFFFF,0x9F9F9F9F,0xFFFFFFFF,
	0x80808080,0xFCFCFCFC,0x7C7C7C7C,
	0x9C9C9C9C,0x9C9C9C9C,0xFCFCFCFC,
	0xFCFCFCFC,0xF4F4F4F4,0x2C2C2C2C,
	0xFCFCFCFC,0xB4B4B4B4,0x18181818,
	0xFCFCFCFC,0xC8C8C8C8,0x9C9C9C9C,
	0xD0D0D0D0,0xFFFFFFFF,0x65656565,
	0xFFFFFFFF,0x40404040,0x8F8F8F8F,
	0x00000000,0xFFFFFFFF,0xE0E0E0E0,
	0xD8D8D8D8,0xECECECEC,0xFCFCFCFC,
	0xBCBCBCBC,0xC0C0C0C0,0xFCFCFCFC,
	0xBABABABA,0xFFFFFFFF,0x90909090,

	0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,
	0x30303030,0x05050505,0x05050505,
	0x00000000,0x18181818,0x14141414,
	0x40404040,0x00000000,0x40404040,
	0x00000000,0x18181818,0x00000000,
	0x00000000,0x00000000,0x40404040,
	0x18181818,0x18181818,0x00000000,
	0x5A5A5A5A,0x2A2A2A2A,0x00000000,
	0x28282828,0x14141414,0x00000000,
	0x18181818,0x20202020,0x0C0C0C0C,
	0x48484848,0x00000000,0x00000000,
	0x00000000,0x30303030,0x2C2C2C2C,
	0x00000000,0x00000000,0x40404040,
	0x00000000,0x00000000,0x40404040,
	0x00000000,0x18181818,0x00000000,
	0x00000000,0x00000000,0x00000000,
	0x24242424,0x24242424,0x24242424,
	0x60606060,0x08080808,0x08080808,
	0x00000000,0x38383838,0x34343434,
	0x58585858,0x08080808,0x5C5C5C5C,
	0x08080808,0x38383838,0x04040404,
	0x08080808,0x08080808,0x5C5C5C5C,
	0x38383838,0x38383838,0x00000000,
	0x70707070,0x34343434,0x00000000,
	0x48484848,0x24242424,0x04040404,
	0x34343434,0x40404040,0x18181818,
	0x64646464,0x04040404,0x08080808,
	0x00000000,0x4C4C4C4C,0x44444444,
	0x0C0C0C0C,0x10101010,0x5C5C5C5C,
	0x08080808,0x0C0C0C0C,0x5C5C5C5C,
	0x08080808,0x38383838,0x04040404,
	0x00000000,0x00000000,0x00000000,
	0x48484848,0x48484848,0x48484848,
	0x7C7C7C7C,0x14141414,0x14141414,
	0x00000000,0x58585858,0x54545454,
	0x74747474,0x14141414,0x74747474,
	0x14141414,0x58585858,0x0C0C0C0C,
	0x14141414,0x18181818,0x74747474,
	0x58585858,0x58585858,0x04040404,
	0x84848484,0x48484848,0x04040404,
	0x64646464,0x38383838,0x10101010,
	0x4C4C4C4C,0x60606060,0x24242424,
	0x7C7C7C7C,0x08080808,0x18181818,
	0x00000000,0x6C6C6C6C,0x5C5C5C5C,
	0x1C1C1C1C,0x2C2C2C2C,0x74747474,
	0x18181818,0x1C1C1C1C,0x74747474,
	0x18181818,0x58585858,0x0C0C0C0C,
	0x00000000,0x00000000,0x00000000,
	0x6C6C6C6C,0x6C6C6C6C,0x6C6C6C6C,
	0x94949494,0x24242424,0x24242424,
	0x00000000,0x78787878,0x78787878,
	0x90909090,0x24242424,0x90909090,
	0x20202020,0x78787878,0x18181818,
	0x24242424,0x28282828,0x90909090,
	0x78787878,0x78787878,0x08080808,
	0x9C9C9C9C,0x58585858,0x04040404,
	0x84848484,0x50505050,0x24242424,
	0x68686868,0x80808080,0x34343434,
	0x94949494,0x10101010,0x28282828,
	0x00000000,0x88888888,0x78787878,
	0x34343434,0x4C4C4C4C,0x90909090,
	0x2C2C2C2C,0x30303030,0x90909090,
	0x2C2C2C2C,0x78787878,0x1C1C1C1C,
	0x00000000,0x00000000,0x00000000,
	0x90909090,0x90909090,0x90909090,
	0xB0B0B0B0,0x38383838,0x38383838,
	0x00000000,0x98989898,0x94949494,
	0xACACACAC,0x3C3C3C3C,0xACACACAC,
	0x30303030,0x98989898,0x2C2C2C2C,
	0x3C3C3C3C,0x40404040,0xACACACAC,
	0x98989898,0x94949494,0x10101010,
	0xB4B4B4B4,0x70707070,0x08080808,
	0xA0A0A0A0,0x68686868,0x38383838,
	0x80808080,0x9C9C9C9C,0x40404040,
	0xB0B0B0B0,0x18181818,0x3C3C3C3C,
	0x00000000,0xA4A4A4A4,0x90909090,
	0x54545454,0x70707070,0xACACACAC,
	0x48484848,0x4C4C4C4C,0xACACACAC,
	0x48484848,0x98989898,0x34343434,
	0x00000000,0x00000000,0x00000000,
	0xB4B4B4B4,0xB4B4B4B4,0xB4B4B4B4,
	0xC8C8C8C8,0x50505050,0x50505050,
	0x00000000,0xBCBCBCBC,0xB4B4B4B4,
	0xC4C4C4C4,0x58585858,0xC4C4C4C4,
	0x48484848,0xBCBCBCBC,0x40404040,
	0x58585858,0x58585858,0xC4C4C4C4,
	0xBCBCBCBC,0xB4B4B4B4,0x14141414,
	0xCCCCCCCC,0x84848484,0x0C0C0C0C,
	0xC0C0C0C0,0x88888888,0x54545454,
	0x9C9C9C9C,0xBCBCBCBC,0x4C4C4C4C,
	0xC8C8C8C8,0x24242424,0x58585858,
	0x00000000,0xC0C0C0C0,0xACACACAC,
	0x78787878,0x98989898,0xC4C4C4C4,
	0x68686868,0x6C6C6C6C,0xC4C4C4C4,
	0x68686868,0xBCBCBCBC,0x4C4C4C4C,
	0x00000000,0x00000000,0x00000000,
	0xD8D8D8D8,0xD8D8D8D8,0xD8D8D8D8,
	0xE0E0E0E0,0x6C6C6C6C,0x6C6C6C6C,
	0x00000000,0xDCDCDCDC,0xDCDCDCDC,
	0xE0E0E0E0,0x78787878,0xE0E0E0E0,
	0x64646464,0xDCDCDCDC,0x5C5C5C5C,
	0x78787878,0x78787878,0xE0E0E0E0,
	0xDCDCDCDC,0xD4D4D4D4,0x20202020,
	0xE4E4E4E4,0x9C9C9C9C,0x14141414,
	0xDCDCDCDC,0xA4A4A4A4,0x74747474,
	0xB4B4B4B4,0xDCDCDCDC,0x58585858,
	0xE0E0E0E0,0x30303030,0x70707070,
	0x00000000,0xE0E0E0E0,0xC4C4C4C4,
	0xA4A4A4A4,0xC0C0C0C0,0xE0E0E0E0,
	0x90909090,0x94949494,0xE0E0E0E0,
	0x90909090,0xDCDCDCDC,0x6C6C6C6C,
	0x00000000,0x00000000,0x00000000,
	0xFCFCFCFC,0xFCFCFCFC,0xFCFCFCFC,
	0xFCFCFCFC,0x8C8C8C8C,0x8C8C8C8C,
	0x00000000,0xFCFCFCFC,0xFCFCFCFC,
	0xFFFFFFFF,0x9F9F9F9F,0xFFFFFFFF,
	0x80808080,0xFCFCFCFC,0x7C7C7C7C,
	0x9C9C9C9C,0x9C9C9C9C,0xFCFCFCFC,
	0xFCFCFCFC,0xF4F4F4F4,0x2C2C2C2C,
	0xFCFCFCFC,0xB4B4B4B4,0x18181818,
	0xFCFCFCFC,0xC8C8C8C8,0x9C9C9C9C,
	0xD0D0D0D0,0xFFFFFFFF,0x65656565,
	0xFFFFFFFF,0x40404040,0x8F8F8F8F,
	0x00000000,0xFFFFFFFF,0xE0E0E0E0,
	0xD8D8D8D8,0xECECECEC,0xFCFCFCFC,
	0xBCBCBCBC,0xC0C0C0C0,0xFCFCFCFC,
	0xBABABABA,0xFFFFFFFF,0x90909090,
};
#endif
