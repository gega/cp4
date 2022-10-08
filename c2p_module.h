/* :ts=4							c2p_module.h
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
#ifndef C2P_MODULE_H
#define C2P_MODULE_H

#include <exec/types.h>
#include <graphics/gfx.h>
#include <intuition/intuition.h>
#include <exec/lists.h>

#define C2P_MAGIC	0xDEAFB055			// deafboss

#define C2PIF_MODLOCALE		0

struct c2pvec {
	char *(*c2p_init)(ULONG scrmode, ULONG overscan, unsigned char *linedeltatab);
	void (*c2p_free)(void);
	int (*c2p_do)(unsigned char *chunky,unsigned char *delta,int numscreen);
	int (*c2p_dofull)(unsigned char *chunky,unsigned char *delta,int numscreen);
	void (*c2p_sleep)(void);
	void (*c2p_awake)(void);
	char *c2p_Info;										// Set via #define-s (NoTouch!)
	char *c2p_Author;									// Set via #define-s (NoTouch!)
	char *c2p_Version;									// Set via #define-s (NoTouch!)
	char *c2p_Revision;									// Set via #define-s (NoTouch!)
	char *c2p_Name;										// Set via #define-s (NoTouch!)
	struct Screen *c2p_Scr;								// Read/Write
	struct Window *c2p_Win;								// Read/Write
	int (*c2p_GetOptionInt)(char *name,int defval);		// Callable Functions
	void (*c2p_AddOptionInt)(char *name,int value);		// -
	char *(*c2p_GetOptionStr)(char *name,char *defval);	// -
	void (*c2p_AddOptionStr)(char *name,char *value);	// -
	int c2p_Buffering;									// Set via #define-s (NoTouch!)
	int c2p_Speed;										// Read only
	int c2p_NoSpeed;									// PRIVATE!
	struct List c2p_MsgList;							// Manipulate Allowed
	int (*c2p_dont)(void);
	int (*c2p_internal)(int);							// Internal function
	int c2p_Render;										// Set via #define-s (NoTouch!)
	UBYTE *c2p_CharMap;									// Read only
	UBYTE *c2p_VideoMatrix;								// Read only
	UBYTE *c2p_ColorMatrix;								// Read only
	UBYTE *c2p_Graphics;								// Read only
	UWORD c2p_CursorPos;								// Read only
	UWORD c2p_ModeFlag;									// Read only
	int c2p_Interface;									// Can't touch this
	UBYTE *c2p_Palette;									// Read only (128 rbg-s)
};

#endif
