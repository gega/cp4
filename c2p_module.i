* :ts=4							c2p_module.i
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

	IFND	C2P_MODULE_I
C2P_MODULE_I	SET		1

	include "exec/types.i"

	STRUCTURE	C2PV,0
		FPTR	C2PV_INIT
		FPTR	C2PV_FREE
		FPTR	C2PV_DO
		FPTR	C2PV_DOFULL
		FPTR	C2PV_SLEEP
		FPTR	C2PV_AWAKE
		APTR	C2PV_INFO
		APTR	C2PV_AUTHOR
		APTR	C2PV_VERSION
		APTR	C2PV_REVISION
		APTR	C2PV_NAME
		APTR	C2PV_SCR
		APTR	C2PV_WIN
		FPTR	C2PV_GETOPTIONINT
		FPTR	C2PV_ADDOPTIONINT
		FPTR	C2PV_GETOPTIONSTR
		FPTR	C2PV_ADDOPTIONSTR
		LONG	C2PV_BUFFERING
		APTR	C2PV_MSGLIST			; beágyazott!

	ENDIF
