# :	ts=8			 Makefile
#
#    cp4 - Commodore C+4 emulator
#    Copyright (C) 1998 Gáti Gergely
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software Foundation,
#    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#    e-mail:	gatig@dragon.klte.hu
#

CFLAGS = -m68020 -Wall -O3 -s -noixemul
DFLAGS = -m68020 -Wall -O0 -g -noixemul
NOPT   = -O2

all:			cp4 assp4 mod MakeROM Save1541ROM
		cp -u -f cp4_ver.h c2p_src/cp4_ver.h
		cp -u -f c2p_module.h c2p_src/c2p_module.h
		cp -u -f ver.c c2p_src/ver.c
		cp -u -f c2p_module.c c2p_src/c2p_module.c
		cp -u -f c2p_color_priority.c c2p_src/c2p_color_priority.c
		cp -u -f c2p_palette.c c2p_src/c2p_palette.c
		rm -f cp4_ver.o

public:			cp4_ver.o
		rm cp4_ver.o
		make all
		createpublic cp4 >NIL:	
		ver cp4 -r

rev:	
		ver cp4 -r
		rm cp4_ver.o

clean:	
		rm -f *.o

mod:		    c2p_zavacki c2p_card c2p_carddelta c2p_mono c2p_windowmono c2p_windowcard c2p_windowscale c2p_windowamiga c2p_perdevpernull c2p_ocs c2p_grey16 c2p_ehb c2p_interlace c2p_windowcarddelta c2p_noborder c2p_nobordercard c2p_direct c2p_directtry

cp4:		    cp4.o ted.o 7501.o dogfx_nrmtxt.o dogfx_mlttxt.o dogfx_nrmgrf.o dogfx_offscr.o dogfx_mltgrf.o dogfx_exttxt.o dogfx_error.o dogfx_none.o grftabs.o samples.o dbg.o scans.o prefs.o preferencesgui.o p4req.o prefsc2p.o cp4_ver.o cp4_rom.o keymapeditgui.o cp4_loc.o soft_iec.o iectrace.o pin_piec.o pin_iec.o iec_dispatcher.o soft_printer.o
		gcc $(CFLAGS) -o cp4 swapstack.o cp4.o ted.o 7501.o dogfx_nrmtxt.o dogfx_mlttxt.o dogfx_nrmgrf.o dogfx_offscr.o dogfx_mltgrf.o dogfx_exttxt.o dogfx_error.o dogfx_none.o grftabs.o samples.o dbg.o scans.o prefs.o preferencesgui.o p4req.o prefsc2p.o cp4_ver.o cp4_rom.o keymapeditgui.o cp4_loc.o soft_iec.o iectrace.o pin_piec.o pin_iec.o iec_dispatcher.o soft_printer.o

Save1541ROM:	save1541rom.c
		gcc $(CFLAGS) -O1 -o Save1541ROM save1541rom.c

iectrace.o:	iectrace.c stat.h
		gcc $(CFLAGS) -O1 -c iectrace.c

MakeROM:		MakeROM.c
		gcc $(CFLAGS) -O1 -o MakeROM MakeROM.c

assp4:		  assp4.o
		gcc $(CFLAGS) -o assp4 assp4.o

pin_piec.o:	pin_piec.c common.h iec_dispatcher.h
		gcc $(CFLAGS) -c pin_piec.c

pin_iec.o:	pin_iec.c common.h iec_dispatcher.h
		gcc $(CFLAGS) -c pin_iec.c

soft_printer.o:	soft_printer.c soft_printer.h prefs.h
		gcc $(CFLAGS) -c soft_printer.c

iec_dispatcher.o:	iec_dispatcher.c iec_dispatcher.h common.h soft_iec.h soft_printer.h
			gcc $(CFLAGS) -c iec_dispatcher.c

c2p_interlace:	c2pi.o c2p_interlace.c c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_interlace.c c2pi.o -o c2p_interlace
		cp -f c2p_interlace c2p/interlace.c2p

c2p_zavacki:	c2p.o c2p_zavacki.c c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_zavacki.c c2p.o -o c2p_zavacki
		cp -f c2p_zavacki c2p/zavacki.c2p

c2p_card:	c2p_card.c c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_card.c -o c2p_card
		cp -f c2p_card c2p/card.c2p
		cp -u -f c2p_card.c c2p_src/c2p_card.c

c2p_direct:	c2p_direct.c c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_direct.c -o c2p_direct
		cp -f c2p_direct c2p/direct.c2p

c2p_directtry:	c2p_directtry.c c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_directtry.c -o c2p_directtry
		cp -f c2p_directtry c2p/directtry.c2p

c2p_noborder:	c2p_noborder.c c2p_module.c c2p_module.h c2p_noborderasm.o
		gcc $(CFLAGS) -nostartfiles c2p_noborder.c c2p_noborderasm.o -o c2p_noborder
		cp -f c2p_noborder c2p/noborder.c2p

c2p_noborderasm.o:	c2p_noborderasm.asm
			phxass c2p_noborderasm.asm
			hunk2aout >NIL:	c2p_noborderasm.o
			rm -f c2p_noborderasm.o
			mv `ls obj.*` c2p_noborderasm.o

c2p_nobordercard:	c2p_nobordercard.c c2p_module.c c2p_module.h c2p_nobordercardasm.o
		gcc $(CFLAGS) -nostartfiles c2p_nobordercard.c c2p_nobordercardasm.o -o c2p_nobordercard
		cp -f c2p_nobordercard c2p/nobordercard.c2p

c2p_nobordercardasm.o:	c2p_nobordercardasm.asm
			phxass c2p_nobordercardasm.asm
			hunk2aout >NIL:	c2p_nobordercardasm.o
			rm -f c2p_nobordercardasm.o
			mv `ls obj.*` c2p_nobordercardasm.o

c2p_carddelta:	c2p_carddelta.c c2p_module.c c2p_module.h c2p_carddeltaasm.o
		gcc $(CFLAGS) -nostartfiles c2p_carddelta.c c2p_carddeltaasm.o -o c2p_carddelta
		cp -f c2p_carddelta c2p/carddelta.c2p

c2p_carddeltaasm.o:	c2p_carddeltaasm.asm
			phxass c2p_carddeltaasm.asm
			hunk2aout >NIL:	c2p_carddeltaasm.o
			rm -f c2p_carddeltaasm.o
			mv `ls obj.*` c2p_carddeltaasm.o

c2p_mono:	c2p_mono.c c2p_monoasm.o c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_mono.c c2p_monoasm.o -o c2p_mono
		cp -f c2p_mono c2p/mono.c2p
		cp -u -f c2p_mono.c c2p_src/c2p_mono.c

c2p_monoasm.o:	c2p_monoasm.asm
		phxass c2p_monoasm.asm
		hunk2aout >NIL:	c2p_monoasm.o
		rm -f c2p_monoasm.o
		mv `ls obj.*` c2p_monoasm.o
		cp -f c2p_monoasm.asm c2p_src/c2p_monoasm.asm

c2p_ehb:		c2p_ehb.c c2p_ehbasm.o c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_ehb.c c2p_ehbasm.o -o c2p_ehb
		cp -f c2p_ehb c2p/ehb.c2p

c2p_ehbasm.o:	c2p_ehbasm.asm
		phxass c2p_ehbasm.asm
		hunk2aout >NIL:	c2p_ehbasm.o
		rm -f c2p_ehbasm.o
		mv `ls obj.*` c2p_ehbasm.o

c2p_windowcard:	c2p_windowcard.c c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_windowcard.c -o c2p_windowcard
		cp -f c2p_windowcard c2p/windowcard.c2p
		cp -u -f c2p_windowcard.c c2p_src/c2p_windowcard.c

c2p_windowcarddelta:	c2p_windowcarddelta.c c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_windowcarddelta.c -o c2p_windowcarddelta
		cp -f c2p_windowcarddelta c2p/windowcarddelta.c2p

c2p_windowscale:		c2p_windowscale.c c2p_module.c c2p_module.h
			gcc $(CFLAGS) -O1 -nostartfiles c2p_windowscale.c -o c2p_windowscale
			cp -f c2p_windowscale c2p/windowscale.c2p
			cp -u -f c2p_windowscale.c c2p_src/c2p_windowscale.c

c2p_windowmono:	c2p_windowmono.c c2p_windowmonoasm.o c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_windowmono.c c2p_windowmonoasm.o -o c2p_windowmono
		cp -f c2p_windowmono c2p/windowmono.c2p
		cp -u -f c2p_windowmonoasm.asm c2p_src/c2p_windowmonoasm.asm
		cp -u -f c2p_windowmono.c c2p_src/c2p_windowmono.c

c2p_windowmonoasm.o:	c2p_windowmonoasm.asm
			phxass c2p_windowmonoasm.asm
			hunk2aout >NIL:	c2p_windowmonoasm.o
			rm -f c2p_windowmonoasm.o
			mv `ls obj.*` c2p_windowmonoasm.o

c2p_windowamiga:		c2p_windowamiga.c c2p_windowamigaasm.o c2p_module.c c2p_module.h
			gcc $(CFLAGS) -nostartfiles c2p_windowamiga.c c2p_windowamigaasm.o -o c2p_windowamiga
			cp -f c2p_windowamiga c2p/windowamiga.c2p
			cp -u -f c2p_windowamiga.c c2p_src/c2p_windowamiga.c


c2p_windowamigaasm.o:	c2p_windowamigaasm.asm
			phxass c2p_windowamigaasm.asm
			hunk2aout >NIL:	c2p_windowamigaasm.o
			rm -f c2p_windowamigaasm.o
			mv `ls obj.*` c2p_windowamigaasm.o
			cp -f c2p_windowamigaasm.asm c2p_src/c2p_windowamigaasm.asm

c2p_ocs:		c2p_ocs.c c2p_ocsasm.o c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_ocs.c c2p_ocsasm.o -o c2p_ocs
		cp -f c2p_ocs c2p/ocs.c2p

c2p_ocsasm.o:	c2p_ocsasm.asm
		phxass c2p_ocsasm.asm
		hunk2aout >NIL:	c2p_ocsasm.o
		rm -f c2p_ocsasm.o
		mv `ls obj.*` c2p_ocsasm.o

c2p_grey16:	c2p_grey16.c c2p_grey16asm.o c2p_module.c c2p_module.h
		gcc $(CFLAGS) -nostartfiles c2p_grey16.c c2p_grey16asm.o -o c2p_grey16
		cp -f c2p_grey16 c2p/grey16.c2p

c2p_grey16asm.o:		c2p_grey16asm.asm
			phxass c2p_grey16asm.asm
			hunk2aout >NIL:	c2p_grey16asm.o
			rm -f c2p_grey16asm.o
			mv `ls obj.*` c2p_grey16asm.o

c2p_perdevpernull:	c2p_perdevpernull.c c2p_module.c c2p_module.h
			gcc $(CFLAGS) -nostartfiles c2p_perdevpernull.c -o c2p_perdevpernull
			cp -f c2p_perdevpernull c2p/perdevpernull.c2p

cp4_ver.o:	
		ver cp4
		gcc $(CFLAGS) -O0 -c cp4_ver.c

cp4_loc.o:	cp4_loc.c cp4_loc.h cp4_locstrs.c
		gcc $(CFLAGS) -c cp4_loc.c

cp4.o:		  common.h cp4.c cp4.h cp4_cpu.h cp4_defs.h cp4_mac.h prefs.h p4req.h cp4_rom.h cp4_loc.h
		gcc $(CFLAGS) -c cp4.c

assp4.o:		common.h assp4.c
		gcc $(CFLAGS) -c assp4.c

soft_iec.o:	soft_iec.c p4req.h cp4_loc.h soft_iec.h prefs.h
		gcc $(CFLAGS) -c soft_iec.c

ted.o:		  common.h ted.c ted.h debug_ext.h prefs.h p4req.h c2p_module.h prefsc2p.h keymap.h cp4_loc.h stat.h soft_iec.h
		gcc $(CFLAGS) -c ted.c

preferencesgui.o:	prefs.h preferences.c preferences.h preferencesgui.c p4req.h c2p_module.h common.h keymap.h cp4_loc.h soft_iec.h snapshot.h
			gcc $(CFLAGS) $(NOPT) -c preferencesgui.c

keymapeditgui.o:		common.h keymapedit.c keymapedit.h keymapeditgui.c prefs.h p4req.h cp4_loc.h
			gcc $(CFLAGS) $(NOPT) -O1 -c keymapeditgui.c

prefs.o:		prefs.h prefs.c cp4_loc.h
		gcc $(CFLAGS) -c prefs.c

prefsc2p.o:	prefsc2p.h prefsc2p.c
		gcc $(CFLAGS) $(NOPT) -c prefsc2p.c

c2p.o:		  c2p.asm
		phxass c2p.asm
		hunk2aout >NIL:	c2p.o
		mv -f `ls obj.*` c2p.o

c2pi.o:		 c2pi.asm
		phxass c2pi.asm
		hunk2aout >NIL:	c2pi.o
		mv -f `ls obj.*` c2pi.o

samples.o:	common.h samples.c
		gcc $(CFLAGS) $(NOPT) -c samples.c

cp4_rom.o:	cp4_rom.c cp4_rom.h common.h
		gcc $(CFLAGS) -O0 -c cp4_rom.c

7501.o:		 7501.asm c2p_module.i shadow.i stat.i
		phxass 7501.asm
		hunk2aout >NIL:	7501.o
		mv -f `ls obj.*` 7501.o

grftabs.o:	grftabs.c common.h
		gcc $(CFLAGS) $(NOPT) -c grftabs.c

p4req.o:		p4req.h p4req.c cp4_loc.h
		gcc $(CFLAGS) $(NOPT) -c p4req.c

dbg.o:		  dbg.c dbg.h common.h debug.h debug.c modetxt.c prefs.h p4req.h cp4_loc.h
		gcc $(CFLAGS) $(NOPT) -O1 -c dbg.c

scans.o:		scans.asm
		phxass scans.asm
		hunk2aout >NIL:	scans.o
		mv -f `ls obj.*` scans.o

dogfx_nrmtxt.o:		 dogfx_nrmtxt.asm dogfx.i shadow.i
			phxass dogfx_nrmtxt.asm
			hunk2aout >NIL:	dogfx_nrmtxt.o
			mv -f `ls obj.*` dogfx_nrmtxt.o

dogfx_mlttxt.o:		 dogfx_mlttxt.asm dogfx.i shadow.i
			phxass dogfx_mlttxt.asm
			hunk2aout >NIL:	dogfx_mlttxt.o
			mv -f `ls obj.*` dogfx_mlttxt.o

dogfx_nrmgrf.o:		 dogfx_nrmgrf.asm dogfx.i shadow.i
			phxass dogfx_nrmgrf.asm
			hunk2aout >NIL:	dogfx_nrmgrf.o
			mv -f `ls obj.*` dogfx_nrmgrf.o

dogfx_offscr.o:		 dogfx_offscr.asm dogfx.i shadow.i
			phxass dogfx_offscr.asm
			hunk2aout >NIL:	dogfx_offscr.o
			mv -f `ls obj.*` dogfx_offscr.o

dogfx_mltgrf.o:		 dogfx_mltgrf.asm dogfx.i shadow.i
			phxass dogfx_mltgrf.asm
			hunk2aout >NIL:	dogfx_mltgrf.o
			mv -f `ls obj.*` dogfx_mltgrf.o

dogfx_exttxt.o:		 dogfx_exttxt.asm dogfx.i shadow.i
			phxass dogfx_exttxt.asm
			hunk2aout >NIL:	dogfx_exttxt.o
			mv -f `ls obj.*` dogfx_exttxt.o

dogfx_error.o:		  dogfx_error.asm dogfx.i shadow.i
			phxass dogfx_error.asm
			hunk2aout >NIL:	dogfx_error.o
			mv -f `ls obj.*` dogfx_error.o

dogfx_none.o:		   dogfx_none.asm dogfx.i shadow.i
			phxass dogfx_none.asm
			hunk2aout >NIL:	dogfx_none.o
			mv -f `ls obj.*` dogfx_none.o
