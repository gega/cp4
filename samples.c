/* :ts=4                            samples.c
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
#include "common.h"

/*
 * 2 byte
 */
u_byte smpl_quadvec[]={
    0x7f,0x80
};

u_byte *smpl_quad=(u_byte *)smpl_quadvec;

char smpl_noisevec[]={
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
u_byte *smpl_noise=(u_byte *)smpl_noisevec;
int quadlen=sizeof(smpl_quadvec);
int noiselen=sizeof(smpl_noisevec);

