/* :ts=4                            modetxt.c
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
#ifndef MODETXT_C
#define MODETXT_C
char *modetxt[]={
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};

void initmodetxt(void) {
    modetxt[0]=GetStr(MSG_01E3);
    modetxt[1]=GetStr(MSG_01E3);
    modetxt[2]=GetStr(MSG_01E3);
    modetxt[3]=GetStr(MSG_01E3);
    modetxt[4]=GetStr(MSG_01E3);
    modetxt[5]=GetStr(MSG_01E4);
    modetxt[6]=GetStr(MSG_01E3);
    modetxt[7]=GetStr(MSG_01E4);
    modetxt[8]=GetStr(MSG_01E3);
    modetxt[9]=GetStr(MSG_01E3);
    modetxt[10]=GetStr(MSG_01E3);
    modetxt[11]=GetStr(MSG_01E3);
    modetxt[12]=GetStr(MSG_01E3);
    modetxt[13]=GetStr(MSG_01E4);
    modetxt[14]=GetStr(MSG_01E3);
    modetxt[15]=GetStr(MSG_01E4);
    modetxt[16]=GetStr(MSG_01E5);
    modetxt[17]=GetStr(MSG_01E6);
    modetxt[18]=GetStr(MSG_01E7);
    modetxt[19]=GetStr(MSG_01E8);
    modetxt[20]=GetStr(MSG_01E9);
    modetxt[21]=GetStr(MSG_01E4);
    modetxt[22]=GetStr(MSG_01E7);
    modetxt[23]=GetStr(MSG_01E4);
    modetxt[24]=GetStr(MSG_01EA);
    modetxt[25]=GetStr(MSG_01E6);
    modetxt[26]=GetStr(MSG_01E7);
    modetxt[27]=GetStr(MSG_01E8);
    modetxt[28]=GetStr(MSG_01E9);
    modetxt[29]=GetStr(MSG_01E4);
    modetxt[30]=GetStr(MSG_01E7);
    modetxt[31]=GetStr(MSG_01E4);
// FLASH
    modetxt[32]=GetStr(MSG_01E3);
    modetxt[33]=GetStr(MSG_01E3);
    modetxt[34]=GetStr(MSG_01E3);
    modetxt[35]=GetStr(MSG_01E3);
    modetxt[36]=GetStr(MSG_01E3);
    modetxt[37]=GetStr(MSG_01E4);
    modetxt[38]=GetStr(MSG_01E3);
    modetxt[39]=GetStr(MSG_01E4);
    modetxt[40]=GetStr(MSG_01E3);
    modetxt[41]=GetStr(MSG_01E3);
    modetxt[42]=GetStr(MSG_01E3);
    modetxt[43]=GetStr(MSG_01E3);
    modetxt[44]=GetStr(MSG_01E3);
    modetxt[45]=GetStr(MSG_01E4);
    modetxt[46]=GetStr(MSG_01E3);
    modetxt[47]=GetStr(MSG_01E4);
    modetxt[48]=GetStr(MSG_01E5);
    modetxt[49]=GetStr(MSG_01E6);
    modetxt[50]=GetStr(MSG_01E7);
    modetxt[51]=GetStr(MSG_01E8);
    modetxt[52]=GetStr(MSG_01E9);
    modetxt[53]=GetStr(MSG_01E4);
    modetxt[54]=GetStr(MSG_01E7);
    modetxt[55]=GetStr(MSG_01E4);
    modetxt[56]=GetStr(MSG_01EA);
    modetxt[57]=GetStr(MSG_01E6);
    modetxt[58]=GetStr(MSG_01E7);
    modetxt[59]=GetStr(MSG_01E8);
    modetxt[60]=GetStr(MSG_01E9);
    modetxt[61]=GetStr(MSG_01E4);
    modetxt[62]=GetStr(MSG_01E7);
    modetxt[63]=GetStr(MSG_01E4);
// USER REQUESTED BLANK
    modetxt[64]=GetStr(MSG_01EB);
    modetxt[65]=GetStr(MSG_01EB);
    modetxt[66]=GetStr(MSG_01EB);
    modetxt[67]=GetStr(MSG_01EB);
    modetxt[68]=GetStr(MSG_01EB);
    modetxt[69]=GetStr(MSG_01EB);
    modetxt[70]=GetStr(MSG_01EB);
    modetxt[71]=GetStr(MSG_01EB);
    modetxt[72]=GetStr(MSG_01EB);
    modetxt[73]=GetStr(MSG_01EB);
    modetxt[74]=GetStr(MSG_01EB);
    modetxt[75]=GetStr(MSG_01EB);
    modetxt[76]=GetStr(MSG_01EB);
    modetxt[77]=GetStr(MSG_01EB);
    modetxt[78]=GetStr(MSG_01EB);
    modetxt[79]=GetStr(MSG_01EB);
    modetxt[80]=GetStr(MSG_01EB);
    modetxt[81]=GetStr(MSG_01EB);
    modetxt[82]=GetStr(MSG_01EB);
    modetxt[83]=GetStr(MSG_01EB);
    modetxt[84]=GetStr(MSG_01EB);
    modetxt[85]=GetStr(MSG_01EB);
    modetxt[86]=GetStr(MSG_01EB);
    modetxt[87]=GetStr(MSG_01EB);
    modetxt[88]=GetStr(MSG_01EB);
    modetxt[89]=GetStr(MSG_01EB);
    modetxt[90]=GetStr(MSG_01EB);
    modetxt[91]=GetStr(MSG_01EB);
    modetxt[92]=GetStr(MSG_01EB);
    modetxt[93]=GetStr(MSG_01EB);
    modetxt[94]=GetStr(MSG_01EB);
    modetxt[95]=GetStr(MSG_01EB);
    modetxt[96]=GetStr(MSG_01EB);
    modetxt[97]=GetStr(MSG_01EB);
    modetxt[98]=GetStr(MSG_01EB);
    modetxt[99]=GetStr(MSG_01EB);
    modetxt[100]=GetStr(MSG_01EB);
    modetxt[101]=GetStr(MSG_01EB);
    modetxt[102]=GetStr(MSG_01EB);
    modetxt[103]=GetStr(MSG_01EB);
    modetxt[104]=GetStr(MSG_01EB);
    modetxt[105]=GetStr(MSG_01EB);
    modetxt[106]=GetStr(MSG_01EB);
    modetxt[107]=GetStr(MSG_01EB);
    modetxt[108]=GetStr(MSG_01EB);
    modetxt[109]=GetStr(MSG_01EB);
    modetxt[110]=GetStr(MSG_01EB);
    modetxt[111]=GetStr(MSG_01EB);
    modetxt[112]=GetStr(MSG_01EB);
    modetxt[113]=GetStr(MSG_01EB);
    modetxt[114]=GetStr(MSG_01EB);
    modetxt[115]=GetStr(MSG_01EB);
    modetxt[116]=GetStr(MSG_01EB);
    modetxt[117]=GetStr(MSG_01EB);
    modetxt[118]=GetStr(MSG_01EB);
    modetxt[119]=GetStr(MSG_01EB);
    modetxt[120]=GetStr(MSG_01EB);
    modetxt[121]=GetStr(MSG_01EB);
    modetxt[122]=GetStr(MSG_01EB);
    modetxt[123]=GetStr(MSG_01EB);
    modetxt[124]=GetStr(MSG_01EB);
    modetxt[125]=GetStr(MSG_01EB);
    modetxt[126]=GetStr(MSG_01EB);
    modetxt[127]=GetStr(MSG_01EB);
}

#endif
