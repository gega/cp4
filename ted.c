/* :ts=4                            ted.c
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

/* The new OS support requires to avoid some functions, like task exceptions,
 * but on the old OS it provide a faster method to do something, so, if you
 * want to avoid use it, just uncomment the following.
 */
//#define NOTASKEXCEPTION

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/io.h>
#include <ctype.h>
#include <fcntl.h>
#include <clib/alib_protos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/timer.h>
#include <proto/asl.h>
#include <proto/xpk.h>
#include <dos/dos.h>
#include <devices/gameport.h>
#include <devices/inputevent.h>
#include <devices/audio.h>

#include <iec/iec.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "macros.h"
#include "common.h"
#include "ted.h"
#include "szamok.h"
#include "c2p_module.h"
#include "prefs.h"
#include "p4req.h"
#include "prefsc2p.h"
#include "keymap.h"
#include "cp4_loc.h"
#include "stat.h"
#include "soft_iec.h"

// statistic
unsigned long statfpsnum=0,statfpssum=0;
#ifdef STAT
unsigned long codetab[512];
#endif


// chunky2planar
struct c2pvec *(*c2pstartup)(REG(d0,unsigned long m));
int (*c2pact)(unsigned char *chunkyaddr,unsigned char *deltaaddr,int numscr);
struct c2pvec *c2pv=NULL;
ULONG c2pidcmp;                 // c2p requested IDCMPs

#include "debug_ext.h"

// from preferencesgui.c
int cp4prefsmpload(void);

// from 7501.asm
extern int initsid(void);
extern void sidpause(void);
extern void sidresume(void);
extern int endsid(void);
extern long CodeTab;

// globálisok a cp4.c-bõl
extern void cp4free(int flag);
extern char *defc2p;            // default c2p
extern int done;                // megállításra
extern int irq;                 // irq request from CPU (nonzero-request)
extern u_byte *p4ram;           // az egész ramot elérhetjük!
u_byte *myram;                  // ASM-hez
extern u_byte *p4rom;           // rom is kell
extern int lastwrite;           // az utolsó memwrite helye
extern int opt_direct;          // 1-direct mode
extern char *opt_p4dir;         // default dir
extern char *opt_keymap;        // keymapfile
extern int opt_nosound;         // hang? (0-van)
extern int opt_sid;             // sid 0-nem 1-igen
extern int opt_limit;           // ==0 nem kell speed limit
extern int opt_percent;         // ==1 kell százalék
extern int opt_iec;             // iec.lib 0-nemkell, 1-kell, -1-nincs!
extern int opt_realtime;        // 0-nincs, 1-realtime kell
extern unsigned long opt_scrmode;   // scrmode
extern unsigned long opt_overscan;  // overscan
extern unsigned long opt_twoscr;    // twoscr
extern char *opt_c2p;               // name of c2p
extern char *opt_palettefile;       // name of palette file
extern int opt_xpk;             // xpk? 0-nem
extern char *opt_xpktype;       // xpktype "----" none
extern u_byte roms[8][MAXROM];      // romok, lapozáshoz
extern int opt_onscreendisplay;     // led stb. (0-nem)

int ret;

/*
 *  grafikához
 */
int opt_render=1;                   // for norender drivers (1-render)
u_byte *c2pColor;                   // use from 7501.asm
u_byte *c2pVideo;                   // use from 7501.asm
u_byte *c2pGfx;                     // use from 7501.asm
u_byte c2pModeflag;                 // use from 7501.asm
u_byte skipflag=0;                  // realtime-hoz
long skiptime=0;                    // -o-
// c2p dir:
char c2pdir[]="PROGDIR:c2p/";
BPTR seglist;
int buffering;                      // hányszoros buffering kell? (0-3)<<2
int buffctrl[16];                   // melyik delta jön?
// framerate színek
unsigned char chalf=0x51;
unsigned char cfull=0x31;
unsigned char ledbor=0x00;
unsigned char ledcol=0x32;
// user requested blank
u_byte userblank=0;             // 0-normál, egyéb-blank
#define SCRHEIGHT   283
u_byte linedelta[SCRHEIGHT];    // sor-delta (0-rajzolni)

// asm rutinokhoz nagy táblázatok (512K)
u_byte *coltable=NULL;
u_byte *backtable=NULL;
#define COLORTABLE  0
#define BACKTABLE   1

// old cache state
ULONG oldCache;

// notify-hez
struct NotifyRequest locnotifyreq;
LONG locsignum=-1;
ULONG locsig;
#ifndef NOTASKEXCEPTION
 int locMOD;                                // módosításjelzõ
#endif
int exceptfull=0;

// többi

u_byte opt_border;          // 0-nem kell border rajz
int framerate=0;
int nospeed=0;
int fr[4]={0,0,0,0};
#define SCRSIZE             (352*283)
int sideborders=4;
int leftborder=16;
int rightborder=16;
struct Device *TimerBase=NULL;
static struct MsgPort *devmsgport=NULL;
static struct EClockVal timestart;
static struct EClockVal timeend;
static struct timerequest *devioreq=NULL;
unsigned char ochunky[SCRSIZE];             // chunky screen
unsigned char *chunky=ochunky;
unsigned char o1delta[SCRSIZE];             // delta buffers
unsigned char o2delta[SCRSIZE];             // delta buffers
unsigned char o3delta[SCRSIZE];             // delta buffers
unsigned char *delta[]={ o1delta,o2delta,o3delta,NULL };
int realmaxraster;
// framerate-hez
struct EClockVal ido1;
struct EClockVal ido2;
ULONG hz80,hz1,hz2;
// filereq
struct Library *AslBase=NULL;
struct iecbase *IECBase=NULL;
struct FileRequester *frq=NULL;
struct ScreenModeRequester *scrrq=NULL;

// error text
static char errmsg[500];

// defines
#define RAM             0
#define ROM             1
#define TEDBASE         0xFF00          // regiszter bázis
#define TED             TEDBASE
#define REGNUM          0x1F            // regiszterek száma
#define MAXRASTER       311             // emulált keret (0-311)
#define REALMAXRAS      282             // amigán csak eddig
#define CLOCKPERRASTERFAST  109         // hány órajelenként kell egy sort?
#define CLOCKPERRASTERSLOW  60

#define PIXELPERROW     455             // hány pixel van egy sorban

#define PEN3    1
#define PEN2    1
#define PEN1    1
#define PEN0    0


// bitek
#define B0      (1<<0)
#define B1      (1<<1)
#define B2      (1<<2)
#define B3      (1<<3)
#define B4      (1<<4)
#define B5      (1<<5)
#define B6      (1<<6)
#define B7      (1<<7)
#define B8      (1<<8)
#define B9      (1<<9)
#define B10     (1<<10)
#define B11     (1<<11)
#define B12     (1<<12)
#define B13     (1<<13)
#define B14     (1<<14)
#define B15     (1<<15)


// TED globálisok
int clkperrast;                 // clock/raster (114/57)
u_byte *hiram;                  // CPU használja! (RAM/ROM >$8000)
u_byte *charmap;                // innen a char-ok
int charinram;                  // chr map in ram? (1-rom)
u_word gfxaddr;                 // grafika kezdet ($2000)
int actraster;                  // aktuális rasztersor
int p4actras;                   // c+4 raster számláló
int hras;                       // horizontal raster
int colplace;                   // szín mezõ (0800)
int textplace;                  // text rész kezdete (0c00)
int singleclock;                // egyszeres órajel?
int stopcnt1,stopcnt2,stopcnt3; // a számlálók leállítása
u_byte initcnt1hi,initcnt1lo;   // 1. számláló teteje
u_byte ec1,ec2,ec3;             // a 3 számláló engedélyezése (1-igen)
int eras;                       // rasztermegszakítás engedélyezése
u_word rasreq;                  // a kívánt rasztermegszakítás helye
int curpos;                     // kurzor helye
u_byte cureor;                  // kurzor (inv/nrm) (00/80)
u_byte keymatrix[8+8];          // key-table 8x8 (plusz a 8 joystick)
u_byte *keymatrixp=keymatrix;
u_byte keyread;                 // fd30-fd3f
int frame;                      // belsõ frame-cnt
u_byte actrom;                  // aktuális rom konfig.
u_byte *amirasaddr;             // köv pixel
u_byte actramrom;               // éppen rom/ram
u_byte colback;                 // háttérszín (ff15)
u_byte colbord;                 // keretszín (ff19)
unsigned long colbordlong;      //   ugyanaz longként
u_byte col[4];                  // ff15,16,17,18
u_byte col1,col2,col3;          // ff16,17,18
u_byte *clp1=&col[1],*clp2=&col[2],*clp3=&col[3];
int hscroll,vscroll;            // screen scroll
int cols;                       // 0-38 oszl.  más-40 oszlop
int offscr;                     // képkioltás (0-kioltva)
int extcol;                     // Extended Color Mode (text) (0-nem)
int bitmap;                     // Bitmap Mode (0-nem)
int multicol;                   // Multi Color Mode (0-nem)
int tedoff;                     // TED kikapcs (0-mûködik)
int actinchar;                  // char-on belül?
int actcharaddr;                // akt char. addr (0, 40, stb)
int numraster;                  // hány soros a scr?
int tbordminus;                 // mennyit kell kivonni még (0/4)

// opsys globs
struct IntuitionBase *IntuitionBase=NULL;
struct GfxBase *GfxBase=NULL;
struct Library *GadToolsBase=NULL;
extern struct DosLibrary *DOSBase;
struct Library *XpkBase=NULL;

struct Screen *cp4Scr=NULL;
struct Window *cp4Window=NULL;

// JOYSTICK
int nojoy=0;                            // joy not found
struct GamePortTrigger joytrigger;
struct IOStdReq *joyioreq=NULL;
struct MsgPort *joyport=NULL;
int joydeviceopened=0;
int joyallocated=0;
int fulljoy=0;
struct InputEvent joyevent;
u_byte joykeyindex=9;                   // '8', ha a 2-es joy kell (^1)
u_byte joyxmask[]={ 0xfb,0xff,0xf7 };   // 0-bal, 1-semmi, 2-jobb
u_byte joyymask[]={ 0xfe,0xff,0xfd };   // 0-fel, 1-semmi, 2-le
u_byte joyfire[]={ 0,0x7f,0xbf };
u_byte joyfiredown=0xbf;                // '7f',ha a 2-es joy kell
u_byte joyfireup=0x40;                  // '80'   --o--
// FIRE   UP       DOWN
// #1  01000000  10111111   Change port: ^$c0
// #2  10000000  01111111
INLINE void dojoy(void);

// VIDEO DRIVER
#define DRIVERMSGMAX    50
int drivermsgcount=0;
struct IntuiMessage drivermsglist[DRIVERMSGMAX+1];

// AUDIO
int audiodeviceopened=0;
int fullaudio=0;
UBYTE whichannel[]={ 15 };              // all channel
struct IOAudio *audioreq=NULL;          // közös (mindig használhatónak kell lennie)
struct IOAudio *audioreq1a=NULL;        // 1. csat.
struct IOAudio *audioreq1b=NULL;        // 1. csat.
struct IOAudio *audioreq2a=NULL;        // 2. csat
struct IOAudio *audioreq2b=NULL;        // 2. csat
struct IOAudio *audioreq7_1[2]={NULL,NULL};     // 7. bit -> jobb. csat.
struct IOAudio *audioreq7_2[2]={NULL,NULL};     // 7. bit -> bal. csat.
struct MsgPort *audioport=NULL;         // reply port
extern u_byte *smpl_quad;               // in samples.c
extern u_byte *smpl_noise;
extern int quadlen;
extern int noiselen;
u_word freqvec[1024];                   // konverziós tábla
u_word noisefreqvec[1024];              // konverziós tábla a zajhoz
u_word *freq=freqvec;
u_word *freq2=freqvec;
BYTE *snd_quad=NULL;                    // négyszögjelek, minden csatornához
BYTE *snd_noise=NULL;                   // zajhullám csak a 2. csat-hoz
BYTE *snd_7bit[2]={NULL,NULL};          // módosítható hullámformák
int bit7flag=0;                         // ==1 7.bit on
#define BIT7_PERIOD     910
#define BIT7_BUFLEN     512             // IF SET IT -> SET IN ASM TOO!!!
void setbit7off(void);
int unit7bit[2]={0x03,0x0c};
int vol1=0,per1=0;                      // store a pause-hoz
int vol2=0,per2=0;

// KEYBOARD
u_byte keyand[]={   0x7f,               // 01111111
                    0xbf,               // 10111111
                    0xdf,               // 11011111
                    0xef,               // 11101111
                    0xf7,               // 11110111
                    0xfb,               // 11111011
                    0xfd,               // 11111101
                    0xfe                // 11111110
};
u_byte keyor[]= {   0x80,               // 10000000
                    0x40,               // 01000000
                    0x20,               // 00100000
                    0x10,               // 00010000
                    0x08,               // 00001000
                    0x04,               // 00000100
                    0x02,               // 00000010
                    0x01                // 00000001
};
// Amiga keymatrix, értékei: c+4 keymatrix
// key = keytab[ RawKeyCode & 0x7f ]
//   release:   keymatrix[ key>>3 ] |= keyor [ key & 7 ];
//   down:      keymatrix[ key>>3 ] &= keyand[ key & 7 ];
// -1 : UNDEF
// -2 : debug (f10) 89
// -3 : prefs (help) 95
// -4 : userblank (f5) 212 : 84
// -5 : load (f6) 213 : 85
// -6 : swapjoy (f7) 214 : 86
// -7 : reset (f8) 215 : 87
// -8 : hardreset (f9) 216 : 88
// -9 : percent (none)
//-10 : limit yes/no (none)
//-11 : joy up
//-12 : joy down
//-13 : joy left
//-14 : joy right
//-15 : joy up-left
//-16 : joy up-right
//-17 : joy down-left
//-18 : joy down-right
//-19 : joy fire
//-20 : snapshot
//-21 : restore
//-22 : quit
int keytab[]={
    51,63,60,15,12,23,20,31,    // 7
    28,39,36,-1,49,41,-1,-1,    // 15
    57,14, 9,22,17,30,25,38,    // 23
    33,46, 0,-1,-1,-1,-1,-1,    // 31
    13,10,21,18,29,26,37,34,    // 39
    45,42,53,54,-1,-1,-1,-1,    // 47
    58,11,16,19,24,27,32,35,    // 55
    40,43,48,-1,-1,-1,-1,-1,    // 63
    59, 7,61,-1, 6,51,62,-1,    // 71
    -1,-1,-1,-1,44,47,52,55,    // 79
     3, 2, 1, 4,-4,-5,-6,-7,    // 87
    -8,-2,-1,-1,-1,-1,-1,-3,    // 95 (HELP)
     8, 8, 8,56,58,50,-1, 5,    // 103
    -1,-1,-1,-1,-1,-1,-1,-1,    // 111
    -1,-1,-1,-1,-1,-1,-1,-1,    // 119
    -1,-1,-1,-1,-1,-1,-1,-1,    // 127
};
int mask[]={ 1,2,4,8,16,32,64,128,256 };


// TIMER
ULONG otvenedsec,otvened1,otvened2;

// LOGO
#define LOGOLEN 43      // 0-42
char logo[]=\
    ".********...********...********....*******." \
    "**********.**********.**********..*********" \
    "***....***.***....***.***....***........***" \
    "***....***.**********.***....***..*********" \
    "***....***.*********..***....***.**********" \
    "***....***.***........***....***.***....***" \
    "**********.**********.**********.**********" \
    ".*********..********...*********..*********" \
    ".......***...................***..........." \
    ".*********.............*********..........." \
    "..*******...............*******............";
/*
 * halvány-táblázat
 */
u_byte logotab[]={
    0x11,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    0x11,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x11,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x11,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
    0x11,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
    0x11,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x11,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
    0x11,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x11,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    0x11,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x11,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x11,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
    0x11,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
    0x11,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x11,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
    0x11,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F
};
u_byte opt_palette[384]={           // 128 rbg values
    0x00,0x00,0x00,0x2C,0x26,0x24,0x5C,0x12,0x14,0x04,0x3A,0x3C,0x4C,0x0A,0x64,
    0x04,0x46,0x04,0x1C,0x1E,0x7C,0x34,0x32,0x04,0x54,0x1E,0x04,0x44,0x26,0x04,
    0x1C,0x42,0x04,0x54,0x0E,0x2C,0x04,0x3E,0x2C,0x04,0x2A,0x74,0x24,0x16,0x84,
    0x0C,0x42,0x04,0x00,0x00,0x00,0x3C,0x36,0x3C,0x6C,0x22,0x24,0x0C,0x4A,0x54,
    0x64,0x16,0x74,0x0C,0x5A,0x0C,0x2C,0x2E,0x94,0x44,0x42,0x04,0x64,0x32,0x04,
    0x5C,0x3A,0x04,0x2C,0x52,0x04,0x6C,0x1E,0x3C,0x04,0x52,0x3C,0x14,0x3A,0x84,
    0x34,0x26,0x9C,0x1C,0x56,0x0C,0x00,0x00,0x00,0x44,0x3E,0x3C,0x74,0x2A,0x2C,
    0x0C,0x52,0x5C,0x6C,0x1E,0x7C,0x0C,0x62,0x14,0x34,0x36,0x9C,0x4C,0x4A,0x04,
    0x6C,0x36,0x04,0x64,0x3E,0x04,0x34,0x5A,0x04,0x74,0x26,0x44,0x0C,0x5A,0x44,
    0x1C,0x42,0x8C,0x3C,0x2E,0xA4,0x1C,0x5E,0x0C,0x00,0x00,0x00,0x54,0x52,0x54,
    0x8C,0x3E,0x3C,0x24,0x6A,0x74,0x7C,0x36,0x94,0x24,0x76,0x24,0x44,0x4E,0xAC,
    0x64,0x5E,0x0C,0x84,0x4E,0x0C,0x74,0x56,0x04,0x44,0x6E,0x04,0x84,0x3A,0x54,
    0x1C,0x6E,0x5C,0x2C,0x5A,0xA4,0x54,0x42,0xBC,0x34,0x72,0x0C,0x00,0x00,0x00,
    0x84,0x7E,0x84,0xB4,0x6A,0x6C,0x54,0x96,0x9C,0xAC,0x62,0xC4,0x54,0xA2,0x54,
    0x74,0x7A,0xDC,0x94,0x8A,0x24,0xAC,0x7A,0x34,0xA4,0x82,0x24,0x74,0x9A,0x1C,
    0xB4,0x66,0x84,0x4C,0x9A,0x84,0x5C,0x86,0xD4,0x84,0x6E,0xE4,0x64,0x9E,0x34,
    0x00,0x00,0x00,0xAC,0xAA,0xAC,0xE4,0x96,0x94,0x7C,0xC2,0xC4,0xD4,0x8E,0xEC,
    0x7C,0xCE,0x7C,0x9C,0xA6,0xF4,0xBC,0xB6,0x4C,0xDC,0xA6,0x64,0xCC,0xAE,0x54,
    0x9C,0xC6,0x44,0xDC,0x92,0xAC,0x7C,0xC6,0xB4,0x84,0xB2,0xFC,0xAC,0x9A,0xFC,
    0x8C,0xCA,0x64,0x00,0x00,0x00,0xC4,0xC2,0xC4,0xFC,0xB2,0xB4,0x94,0xDA,0xE4,
    0xEC,0xA6,0xFC,0x9C,0xE6,0x9C,0xBC,0xBE,0xFC,0xD4,0xD2,0x6C,0xF4,0xBE,0x7C,
    0xE4,0xC6,0x6C,0xBC,0xE2,0x5C,0xF4,0xAE,0xC4,0x94,0xDE,0xCC,0xA4,0xCA,0xFC,
    0xC4,0xB6,0xFC,0xA4,0xE2,0x7C,0x00,0x00,0x00,0xF4,0xF2,0xF4,0xF4,0xEE,0xEC,
    0xD4,0xF6,0xF4,0xF4,0xE6,0xF4,0xDC,0xF6,0xDC,0xF4,0xF6,0xFC,0xF4,0xF6,0xAC,
    0xF4,0xF6,0xBC,0xF4,0xF6,0xAC,0xF4,0xF6,0xA4,0xF4,0xEA,0xF4,0xD4,0xF6,0xF4,
    0xE4,0xF6,0xFC,0xF4,0xF2,0xF4,0xE4,0xF6,0xBC,
};

// ROM PAGING
u_byte romtabLO[16]={
    0,2,4,6,
    0,2,4,6,
    0,2,4,6,
    0,2,4,6
};
u_byte romtabHI[16]={
    1,1,1,1,
    3,3,3,3,
    5,5,5,5,
    7,7,7,7
};
u_byte *romtab=romtabLO;


// GRAPHIX
// ezt állítják a módok!
void (*dogfx)(void);                // egy sornyit rajzol (aktuális módban)
extern void dogfx_nrmtxt128(void);  // Normal Text (128 char + inv)
extern void dogfx_nrmtxt256(void);  // Normal Text (256 char)
extern void dogfx_nrmtxt128f(void); // Normal Text (128 char + inv + FLASH)
extern void dogfx_nrmtxt256f(void); // Normal Text (256 char + FLASH)
extern void dogfx_exttxti(void);    // Extended Color Text inv
extern void dogfx_exttxtn(void);    // Extended Color Text 256
extern void dogfx_mlttxti(void);    // Multi Color Text inv
extern void dogfx_mlttxtn(void);    // Multi Color Text 256
extern void dogfx_nrmgrf(void);     // Normal Bitmap
extern void dogfx_mltgrf(void);     // Multi Color Bitmap
extern void dogfx_offscr(void);     // Screen OFF (border only)
extern void dogfx_error(void);      // invalid config (black)

extern void dogfx_nrmtxt128D(void); // Normal Text (128 char + inv)
extern void dogfx_nrmtxt256D(void); // Normal Text (256 char)
extern void dogfx_nrmtxt128fD(void);    // Normal Text (128 char + inv + FLASH)
extern void dogfx_nrmtxt256fD(void);    // Normal Text (256 char + FLASH)
extern void dogfx_exttxtiD(void);   // Extended Color Text inv
extern void dogfx_exttxtnD(void);   // Extended Color Text 256
extern void dogfx_mlttxtiD(void);   // Multi Color Text inv
extern void dogfx_mlttxtnD(void);   // Multi Color Text 256
extern void dogfx_nrmgrfD(void);        // Normal Bitmap
extern void dogfx_mltgrfD(void);        // Multi Color Bitmap
extern void dogfx_offscrD(void);        // Screen OFF (border only)
extern void dogfx_errorD(void);     // invalid config (black)

extern void dogfx_none(void);       // user requested blanking

//      8           4           2           1           |  8            4           2           1
// -----------------------------------------+-----------+-----------------------------------------------
// bit: 7           6           5           4           |  3            2           1           0
//      Direct      NONE        flash       scroff(0)   |  inv(0)       extcol(1)   bitmap(1)   multi(1)
#define MODEINIT    0x10
u_byte modeflag=MODEINIT;
// ebben vannak a grafikus rész függvénymutatói
void (*modegfx[256])(void);


//---------------------------------------------------------------------------
// FUNCTIONS
//---------------------------------------------------------------------------

void freejoy(void);
int initjoy(void);
int getjoy(void);

/*
 * hiba
 */
void tederror(char *str) {
    if(str!=NULL) if(-1==p4req1(NULL,P4_ERROR,str)) fprintf(stderr,"%s\n",str);
    tedfree();
    cp4free(0);
    exit(0);
} // tederror()


/*
 * test a keymap file (magic/exists)
 */
int testkeymap(char *name) {
static char defdir[]="PROGDIR:KeyMap/";
static char buf[11];
static char magic[]=K_MAGIC;
    FILE *f;
    char *n2;

    if(name!=NULL) {
        f=fopen(name,"rb");
        if(f==NULL) {
            if(NULL==(n2=malloc(strlen(name)+20))) return(-1);
            strcpy(n2,defdir);
            strcat(n2,name);
            f=fopen(n2,"rb");
            if(f==NULL) { free(n2); return(-1); }
        }
        fgets(buf,10,f);
        if(strcmp(buf,magic)!=0) {
            p4req1(NULL,P4_WARNING,GetStr(MSG_0240));
            return(-1);
        }
    } else return(-1);
    return(0);
} // testkeymap()

/*
 * beállítja a kért keymap-et, ha nem találja, vagy hibás: -1
 * name==NULL default
 */
int setkeymap(char *name) {
static int defkeytab[]={
    51,63,60,15,12,23,20,31,28,39,36,-1,49,41,-1,-1,57,14, 9,22,17,30,25,38,
    33,46, 0,-1,-1,-1,-1,-1,13,10,21,18,29,26,37,34,45,42,53,54,-1,-1,-1,-1,
    58,11,16,19,24,27,32,35,40,43,48,-1,-1,-1,-1,-1,59, 7,61,-1, 6,51,62,-1,
    -1,-1,-1,-1,44,47,52,55, 3, 2, 1, 4,-4,-5,-6,-7,-8,-2,-1,-1,-1,-1,-1,-3,
     8, 8, 8,56,58,50,-1, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, 0
};
static char defdir[]="PROGDIR:KeyMaps/";
static char buf[11];
static char magic[]=K_MAGIC;
    int ret=-1,i,in;
    FILE *f;
    char *n2;
    signed char sch;

    if(name==NULL) {
        for(i=0;i<128;i++) keytab[i]=defkeytab[i];
        ret=0;
    } else {
        f=fopen(name,"rb");
        if(f==NULL) {
            if(NULL==(n2=malloc(strlen(name)+20))) return(-1);
            strcpy(n2,defdir);
            strcat(n2,name);
            f=fopen(n2,"rb");
            if(f==NULL) { free(n2); return(-1); }
        }
        fgets(buf,10,f);
        if(strcmp(buf,magic)!=0) {
            p4req1(NULL,P4_WARNING,GetStr(MSG_0240));
            return(-1);
        }
        for(i=0;i<128;i++) {
            in=fgetc(f);
            if(in==EOF) break;
            sch=(signed char)in;
            in=(int)sch;
            keytab[i]=in;
        }
        if(i==128) ret=0;
        fclose(f);
    }
    return(ret);
} // setkeymap()


void flushbuffers(void) {
    int i;

    /* clear delta buffers
     */
    for(i=0;i<SCRSIZE;i++) {
        o1delta[i]=0;
        o2delta[i]=0;
        o3delta[i]=0;
        ochunky[i]=0;
    }
    for(i=0;i<SCRHEIGHT;i++) linedelta[i]=0;        // valid all line
} // flushbuffers

/*
 * legenerálja a 2 táblázat egyikét
 * ------------------------------
 * table1 (colortable)
 *
 * 0_______ ________  -->  8 byte (ha a char bit 1: color, ha 0: 00)
 *   color    char
 *    7bit    8bit
 * a táblázat hossza 32768*8 byte (262144 byte, 256K)
 *
 *
 *
 * table2 (backtable)
 *
 * 0_______ ________  -->  8 byte (ha a char bit 1: 00, ha 0: color)
 *   color    char
 *    7bit    8bit
 * a táblázat hossza 32768*8 byte (262144 byte, 256K)
 *
 */
u_byte *gentab(int mode) {
    int col,chr,j=0,bit,b;
    u_byte *tab;

    tab=malloc(32768*8);
    if(tab==NULL) return(NULL);

    for(col=0;col<0x80;col++) {
        for(chr=0;chr<0x100;chr++) {
            for(bit=7;bit>=0;bit--) {
                b=chr&mask[bit];
                switch(mode) {
                    case COLORTABLE :
                        if(b!=0) tab[j]=col;
                        else tab[j]=0;
                        break;
                    case BACKTABLE :
                        if(b==0) tab[j]=col;
                        else tab[j]=0;
                        break;
                }
                j++;
            }
        }
    }
    return(tab);
} // gentab()


/*
 * kirakja a logo-t
 */
INLINE void dologo(unsigned char *chky) {
    int i,j;
    unsigned char *inp;
    char *lg=logo;

    if(opt_direct!=0) return;
    chky+=(305+(352*2));
    i=11;
    do {
        inp=chky;
        j=LOGOLEN;
        do {
            if(*lg++) *inp=logotab[*inp];
            inp++;
        } while(--j);
        chky+=352;
    } while(--i);
} // dologo


char *makefilename(struct FileRequester *f) {
static char name[260];
    strcpy(name,f->rf_Dir);
    if(f->rf_Dir!=NULL&&f->rf_Dir[0]!='\0'&&f->rf_Dir[strlen(f->rf_Dir)-1]!=':')
        strcat(name,"/");
    strcat(name,f->rf_File);
    return(name);
} // makefilename()


/*
 * reset init
 */
void reset(void) {
    int i;

    for(i=0;i<16;i++) keymatrix[i]=0xff;    // clr keymatrix
    keyread=0xff;                           // fd30-fd3f
    modeflag=MODEINIT;
    if(opt_direct!=0) modeflag|=0x80;
    cureor=0x80;
    dogfx=modegfx[modeflag];
} // reset()


static void loadfile() {
    struct Screen *pubscr;

    soundpause();
    freejoy();
    c2pv->c2p_sleep();
    pubscr=LockPubScreen(NULL);
    ScreenToFront(pubscr);
    cp4prefsmpload();
    UnlockPubScreen(NULL,pubscr);
    ScreenToFront(cp4Scr);
    ActivateWindow(cp4Window);
    c2pv->c2p_awake();
    getjoy();
    soundresume();
}

/*
 * triplabuffering, c2p, stb...
 */
INLINE void doscr(void) {
static struct IntuiMessage *imsg;
    int key;
    ULONG class;
    UWORD code;

    // JOYSTICK
    dojoy();

    // KEYBOARD & other intuievents
    while((imsg=GT_GetIMsg(cp4Window->UserPort))) {             // jött valami?
        class=imsg->Class;
        code=imsg->Code;
        if(class==IDCMP_RAWKEY) {
            GT_ReplyIMsg(imsg);
            key=keytab[code&0x7f];
            if(key>=0) {                                // Valid C+4 key
                if(code>127)    keymatrix[key>>3]|=keyor[key&7];    // release
                else            keymatrix[key>>3]&=keyand[key&7];   // down
            } else {
                switch(-key) {
                    case 2 :                            // F10 down
                        /*
                         * DEBUG
                         */
                        if(code>127) {
                            userblank=0;
                            ret|=B1;
                        }
                        break;
                    case 3 :                            // HELP down
                        /*
                         * PREFS
                         */
                        if(code>127) {
                            userblank=0;
                            ret|=(B1+B4);
                        }
                        break;
                    case 4 :                            // F5
                        /*
                         * USER BLANK
                         */
                        if(code>127&&opt_realtime==0) {
                            userblank^=1;
                            ret^=B0;
                        }
                        break;
                    case 5 :                            // F6
                        /*
                         * LOAD FILE
                         */
                        if(code>127) loadfile();
                        break;
                    case 6 :                            // F7
                        /*
                         * SWAP JOY
                         */
                        if(code>127) {
                            joykeyindex^=1;
                            joyfiredown=joyfire[joykeyindex-7];
                            joyfireup=~joyfiredown;
                            if(joyfireup==0x40) AddOption("JOY","NORMAL");
                            else AddOption("JOY","SWAPPED");
                        }
                        break;
                    case 7 :                            // F8
                        /*
                         * RESET
                         */
                        if(code>127) {
                            userblank=0;
                            reset();
                            ret|=B2;
                        }
                        break;
                    case 8 :                            // F9
                        /*
                         * HARD-RESET
                         */
                        if(code>127) {
                            userblank=0;
                            reset();
                            ret|=B2+B3;
                        }
                        break;
                    case 9 :                            // none
                        /*
                         * PERCENT ON/OFF
                         */
                        if(code>127) {
                            opt_percent^=1;
                            if(opt_percent==0) AddOption("PERCENT","NO");
                            else AddOption("PERCENT","YES");
                        }
                        break;
                    case 10 :                           // none
                        /*
                         * LIMIT ON/OFF
                         */
                        if(code>127) {
                            opt_limit^=1;
                            if(opt_limit!=0) {
                                AddOption("SPEEDLIMIT","YES");
                                if(opt_twoscr==~0) {
                                    otvenedsec=otvened2;
                                    hz80=hz2;
                                } else {
                                    otvenedsec=otvened1;
                                    hz80=hz1;
                                }
                            } else {
                                otvenedsec=0;
                                AddOption("SPEEDLIMIT","NO");
                            }
                        }
                        break;
                    case 11 :                           // JOY UP
                        if(code>127) keymatrix[joykeyindex^1]|=~joyymask[0];
                        else keymatrix[joykeyindex^1]&=joyymask[0];
                        break;
                    case 12 :                           // JOY DOWN
                        if(code>127) keymatrix[joykeyindex^1]|=~joyymask[2];
                        else keymatrix[joykeyindex^1]&=joyymask[2];
                        break;
                    case 13 :                           // JOY LEFT
                        if(code>127) keymatrix[joykeyindex^1]|=~joyxmask[0];
                        else keymatrix[joykeyindex^1]&=joyxmask[0];
                        break;
                    case 14 :                           // JOY RIGHT
                        if(code>127) keymatrix[joykeyindex^1]|=~joyxmask[2];
                        else keymatrix[joykeyindex^1]&=joyxmask[2];
                        break;
                    case 15 :                           // JOY UP-LEFT
                        if(code>127) keymatrix[joykeyindex^1]|=(~joyymask[0])|(~joyxmask[0]);
                        else keymatrix[joykeyindex^1]&=joyymask[0]&joyxmask[0];
                        break;
                    case 16 :                           // JOY UP-RIGHT
                        if(code>127) keymatrix[joykeyindex^1]|=(~joyymask[0])|(~joyxmask[2]);
                        else keymatrix[joykeyindex^1]&=joyymask[0]&joyxmask[2];
                        break;
                    case 17 :                           // JOY DOWN-LEFT
                        if(code>127) keymatrix[joykeyindex^1]|=(~joyymask[2])|(~joyxmask[0]);
                        else keymatrix[joykeyindex^1]&=joyymask[2]&joyxmask[0];
                        break;
                    case 18 :                           // JOY DOWN-RIGHT
                        if(code>127) keymatrix[joykeyindex^1]|=(~joyymask[2])|(~joyxmask[2]);
                        else keymatrix[joykeyindex^1]&=joyymask[2]&joyxmask[2];
                        break;
                    case 19 :                           // JOY FIRE
                        if(code>127) keymatrix[joykeyindex^1]|=joyfireup^0xc0;
                        else keymatrix[joykeyindex^1]&=joyfiredown^0xc0;
                        break;
                    case 20 :                           // SNAPSHOT
                        if(code>127) ret|=B1+B5;
                        break;
                    case 21 :                           // RESTORE
                        if(code>127) {
                            userblank=0;
                            ret|=B1+B5+B6;
                        }
                        break;
                    case 22 :                           // QUIT
                        ret=B7;
                        break;
                }
            }
        } else {
            // a driver-nek (is) jött
            if(class==IDCMP_INACTIVEWINDOW) freejoy();
            if(class==IDCMP_ACTIVEWINDOW) getjoy();
            if((c2pidcmp&class)!=0) {
                if(drivermsgcount<=DRIVERMSGMAX) {
                    CopyMem((char *)imsg,(char *)&drivermsglist[drivermsgcount],(long)sizeof(struct IntuiMessage));
                    AddTail(&c2pv->c2p_MsgList,(struct Node *)&drivermsglist[drivermsgcount++]);
                }
            }
            GT_ReplyIMsg(imsg);
        }
    } // while
} // doscr()


/*
 * kirak egy pontot
 */
INLINE void putpoint(unsigned char *b,char c) {
    if(c>=0) *b=c;
} // putpoint

/*
 * kirak egy számból egy sort
 */
INLINE void putnum(unsigned char *b,char *c,int n) {
    do { putpoint(b++,*c++); } while(--n);
} // putnum

INLINE void doframerate(unsigned char *bf) {
    int i;
    int szam1,szam2;
    char *base;
    char *c1,*c2,*sz;

    if(opt_direct!=0) return;
    base=&szamok[80*15];
    i=framerate+5;
    szam1=i/100;
    if(szam1>9) szam1=0;
    c1=base+(szam1<<3);
    i%=100;
    szam2=i/10;
    c2=base+(szam2<<3);
    bf+=(352*16)+3;
    i=15;
    if(opt_realtime==0) {
        sz=&szazalek[11*15];
        do {
            putnum(bf+8,c1,8); putnum(bf+16,c2,8); putnum(bf+24,sz,11);
            sz-=11; c1-=80; c2-=80; bf-=352;
        } while(--i);
    } else {
        do {
            putnum(bf+16,c2,8);
            c2-=80; bf-=352;
        } while(--i);
    }
} // doframerate()

#ifdef STAT
void domemstat(unsigned char *bf) {
    unsigned long *pnt;
    unsigned long mx;
    int i,x;

    if(CodeTab==0) return;
    bf+=(352*18)+10;
    pnt=&CodeTab;
    pnt++;
    for(i=0,mx=0;i<256;i++) if(pnt[i]>mx) mx=*pnt;
    x=mx/7;
    if(x==0) x=8;
    memset(bf,0,258);       // felsõ csík
    bf+=352;
    *bf++=0;                // bal csík
    for(i=0;i<256;i++,pnt++) *bf++=((((*pnt)/x)<<4)+1);
    *bf=0;                  // jobb csík
    bf+=(352-257);
    memset(bf,0,258);       // alsó csík
    pnt=&CodeTab;
    pnt++;
    for(i=0;i<256;i++) codetab[i]+=pnt[i];
    memset(pnt,0,4*256);    // nullázzuk a táblát
} // domemstat()
#endif


/*
 * legenerálja az 1024 elemû c+4 --> amiga frekvencia dekódoló táblázatot
 */
void genfreq(void) {
    int samplelen=quadlen;
    double p4clk=111840.45;     // PAL
    double amiclk=3546895;      // PAL
    double freqmy;
    double period,noiseper;
    int i,act;
    u_word realperiod;

    for(i=0;i<1024;i++) {
        if(i==1023) act=-1;
        else act=i;
        freqmy=p4clk/(1024-act);
        period=(amiclk/freqmy)/samplelen;
        noiseper=(amiclk/freqmy)/noiselen;
        realperiod=(u_word)period;
        freqvec[i]=realperiod;
        realperiod=(u_word)noiseper;
        noisefreqvec[i]=realperiod;
    }
}


/*
 * init audio
 */
int initaudio(void) {
    int startper=3100;

    // generate frequency conversion table
    genfreq();

    // copy quad sample into chip ram
    if(NULL==(snd_quad=(BYTE *)AllocVec(quadlen,MEMF_CHIP|MEMF_PUBLIC))) return(-1);
    CopyMem(smpl_quad,snd_quad,quadlen);

    // copy noise into chip ram
    if(NULL==(snd_noise=(BYTE *)AllocVec(noiselen,MEMF_CHIP|MEMF_PUBLIC))) return(-1);
    CopyMem(smpl_noise,snd_noise,noiselen);

    // create 7. bit samples in chipram
    if(NULL==(snd_7bit[0]=(BYTE *)AllocVec(BIT7_BUFLEN,MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR))) return(-1);
    if(NULL==(snd_7bit[1]=(BYTE *)AllocVec(BIT7_BUFLEN,MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR))) return(-1);

    // alloc things
    if(NULL==(audioport=CreateMsgPort())) return(-1);
    if(NULL==(audioreq=(struct IOAudio *)CreateIORequest(audioport,sizeof(struct IOAudio)))) return(-1);
    if(NULL==(audioreq1a=(struct IOAudio *)CreateIORequest(audioport,sizeof(struct IOAudio)))) return(-1);
    if(NULL==(audioreq1b=(struct IOAudio *)CreateIORequest(audioport,sizeof(struct IOAudio)))) return(-1);
    if(NULL==(audioreq2a=(struct IOAudio *)CreateIORequest(audioport,sizeof(struct IOAudio)))) return(-1);
    if(NULL==(audioreq2b=(struct IOAudio *)CreateIORequest(audioport,sizeof(struct IOAudio)))) return(-1);
    if(NULL==(audioreq7_1[0]=(struct IOAudio *)CreateIORequest(audioport,sizeof(struct IOAudio)))) return(-1);
    if(NULL==(audioreq7_1[1]=(struct IOAudio *)CreateIORequest(audioport,sizeof(struct IOAudio)))) return(-1);
    if(NULL==(audioreq7_2[0]=(struct IOAudio *)CreateIORequest(audioport,sizeof(struct IOAudio)))) return(-1);
    if(NULL==(audioreq7_2[1]=(struct IOAudio *)CreateIORequest(audioport,sizeof(struct IOAudio)))) return(-1);
    audioreq->ioa_Request.io_Message.mn_Node.ln_Pri=127;
    audioreq->ioa_Request.io_Message.mn_Node.ln_Type=0;
    audioreq->ioa_Request.io_Command=ADCMD_ALLOCATE;
    audioreq->ioa_Request.io_Flags=ADIOF_NOWAIT;
    audioreq->ioa_AllocKey=0;
    audioreq->ioa_Data=whichannel;
    audioreq->ioa_Length=sizeof(whichannel);

    // channel allocating
    if(0!=OpenDevice("audio.device",0L,(struct IORequest *)audioreq,0L)) return(-1);
    audiodeviceopened=1;

    // requests inited
    audioreq1a->ioa_AllocKey=audioreq->ioa_AllocKey;
    audioreq1b->ioa_AllocKey=audioreq->ioa_AllocKey;
    audioreq2a->ioa_AllocKey=audioreq->ioa_AllocKey;
    audioreq2b->ioa_AllocKey=audioreq->ioa_AllocKey;
    audioreq1a->ioa_Request.io_Message.mn_Node.ln_Type=0;
    audioreq1b->ioa_Request.io_Message.mn_Node.ln_Type=0;
    audioreq2a->ioa_Request.io_Message.mn_Node.ln_Type=0;
    audioreq2b->ioa_Request.io_Message.mn_Node.ln_Type=0;
    audioreq1a->ioa_Request.io_Device=audioreq->ioa_Request.io_Device;
    audioreq1b->ioa_Request.io_Device=audioreq->ioa_Request.io_Device;
    audioreq2a->ioa_Request.io_Device=audioreq->ioa_Request.io_Device;
    audioreq2b->ioa_Request.io_Device=audioreq->ioa_Request.io_Device;

    // 7.bit requests inited
    audioreq7_1[0]->ioa_AllocKey=audioreq->ioa_AllocKey;
    audioreq7_1[1]->ioa_AllocKey=audioreq->ioa_AllocKey;
    audioreq7_2[0]->ioa_AllocKey=audioreq->ioa_AllocKey;
    audioreq7_2[1]->ioa_AllocKey=audioreq->ioa_AllocKey;
    audioreq7_1[0]->ioa_Request.io_Message.mn_Node.ln_Type=0;
    audioreq7_1[1]->ioa_Request.io_Message.mn_Node.ln_Type=0;
    audioreq7_2[0]->ioa_Request.io_Message.mn_Node.ln_Type=0;
    audioreq7_2[1]->ioa_Request.io_Message.mn_Node.ln_Type=0;
    audioreq7_1[0]->ioa_Request.io_Device=audioreq->ioa_Request.io_Device;
    audioreq7_1[1]->ioa_Request.io_Device=audioreq->ioa_Request.io_Device;
    audioreq7_2[0]->ioa_Request.io_Device=audioreq->ioa_Request.io_Device;
    audioreq7_2[1]->ioa_Request.io_Device=audioreq->ioa_Request.io_Device;
    audioreq7_1[0]->ioa_Request.io_Unit=(struct Unit *)0x01;    // 0,3
    audioreq7_1[1]->ioa_Request.io_Unit=(struct Unit *)0x08;
    audioreq7_2[0]->ioa_Request.io_Unit=(struct Unit *)0x02;    // 1,2
    audioreq7_2[1]->ioa_Request.io_Unit=(struct Unit *)0x04;
    audioreq7_2[0]->ioa_Request.io_Command=CMD_WRITE;
    audioreq7_2[1]->ioa_Request.io_Command=CMD_WRITE;
    audioreq7_1[0]->ioa_Request.io_Command=CMD_WRITE;
    audioreq7_1[1]->ioa_Request.io_Command=CMD_WRITE;
    audioreq7_2[0]->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq7_2[1]->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq7_1[0]->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq7_1[1]->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq7_1[0]->ioa_Data=snd_7bit[0];
    audioreq7_2[0]->ioa_Data=snd_7bit[0];
    audioreq7_1[1]->ioa_Data=snd_7bit[0];
    audioreq7_2[1]->ioa_Data=snd_7bit[0];
    audioreq7_2[0]->ioa_Length=BIT7_BUFLEN;
    audioreq7_2[1]->ioa_Length=BIT7_BUFLEN;
    audioreq7_1[0]->ioa_Length=BIT7_BUFLEN;
    audioreq7_1[1]->ioa_Length=BIT7_BUFLEN;
    audioreq7_2[0]->ioa_Period=BIT7_PERIOD;
    audioreq7_2[1]->ioa_Period=BIT7_PERIOD;
    audioreq7_1[0]->ioa_Period=BIT7_PERIOD;
    audioreq7_1[1]->ioa_Period=BIT7_PERIOD;
    audioreq7_2[0]->ioa_Volume=64;
    audioreq7_2[1]->ioa_Volume=64;
    audioreq7_1[0]->ioa_Volume=64;
    audioreq7_1[1]->ioa_Volume=64;
    audioreq7_1[0]->ioa_Cycles=1;
    audioreq7_1[1]->ioa_Cycles=1;
    audioreq7_2[0]->ioa_Cycles=1;
    audioreq7_2[1]->ioa_Cycles=1;

    // init quad tone
    audioreq1a->ioa_Request.io_Unit=(struct Unit *)0x01;
    audioreq1b->ioa_Request.io_Unit=(struct Unit *)0x02;
    audioreq2a->ioa_Request.io_Unit=(struct Unit *)0x04;
    audioreq2b->ioa_Request.io_Unit=(struct Unit *)0x08;
    audioreq1a->ioa_Request.io_Command=CMD_WRITE;
    audioreq1b->ioa_Request.io_Command=CMD_WRITE;
    audioreq2a->ioa_Request.io_Command=CMD_WRITE;
    audioreq2b->ioa_Request.io_Command=CMD_WRITE;
    audioreq1a->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq1b->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq2a->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq2b->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq1a->ioa_Data=snd_quad;
    audioreq1b->ioa_Data=snd_quad;
    audioreq2a->ioa_Data=snd_quad;
    audioreq2b->ioa_Data=snd_quad;
    audioreq1a->ioa_Length=quadlen;
    audioreq1b->ioa_Length=quadlen;
    audioreq2a->ioa_Length=quadlen;
    audioreq2b->ioa_Length=quadlen;
    audioreq1a->ioa_Cycles=0;
    audioreq1b->ioa_Cycles=0;
    audioreq2a->ioa_Cycles=0;
    audioreq2b->ioa_Cycles=0;
    audioreq1a->ioa_Volume=0;
    audioreq1b->ioa_Volume=0;
    audioreq2a->ioa_Volume=0;
    audioreq2b->ioa_Volume=0;
    audioreq1a->ioa_Period=startper;
    audioreq1b->ioa_Period=startper;
    audioreq2a->ioa_Period=startper;
    audioreq2b->ioa_Period=startper;

    // STOP all channel
    audioreq->ioa_Request.io_Unit=(struct Unit *)0x0f;
    audioreq->ioa_Request.io_Command=CMD_STOP;
    DoIO((struct IORequest *)audioreq);

    // send start command
    BeginIO((struct IORequest *)audioreq1a);
    BeginIO((struct IORequest *)audioreq1b);
    BeginIO((struct IORequest *)audioreq2a);    // négyszög default
    BeginIO((struct IORequest *)audioreq2b);

    // START synchronized sound in all channel
    audioreq->ioa_Request.io_Unit=(struct Unit *)0x0f;
    audioreq->ioa_Request.io_Command=CMD_START;
    DoIO((struct IORequest *)audioreq);

    // init for requests
    audioreq->ioa_Request.io_Command=ADCMD_PERVOL;

    vol1=vol2=per1=per2=0;
    // sound is out
    fullaudio=1;
    return(0);
} // initaudio()


/*
 * abort playing, etc
 */
void freeaudio(void) {
    if(bit7flag!=0) setbit7off();
    if(NULL==CheckIO((struct IORequest *)audioreq2a)) AbortIO((struct IORequest *)audioreq2a);
    WaitIO((struct IORequest *)audioreq2a);
    if(NULL==CheckIO((struct IORequest *)audioreq2b)) AbortIO((struct IORequest *)audioreq2b);
    WaitIO((struct IORequest *)audioreq2b);
    if(NULL==CheckIO((struct IORequest *)audioreq1a)) AbortIO((struct IORequest *)audioreq1a);
    WaitIO((struct IORequest *)audioreq1a);
    if(NULL==CheckIO((struct IORequest *)audioreq1b)) AbortIO((struct IORequest *)audioreq1b);
    WaitIO((struct IORequest *)audioreq1b);
    if(audiodeviceopened) { CloseDevice((struct IORequest *)audioreq); audiodeviceopened=0; }
    if(audioreq) { DeleteIORequest(audioreq); audioreq=NULL; }
    if(audioreq1a) { DeleteIORequest(audioreq1a); audioreq1a=NULL; }
    if(audioreq1b) { DeleteIORequest(audioreq1b); audioreq1b=NULL; }
    if(audioreq2a) { DeleteIORequest(audioreq2a); audioreq2a=NULL; }
    if(audioreq2b) { DeleteIORequest(audioreq2b); audioreq2b=NULL; }
    if(audioport) { DeleteMsgPort(audioport); audioport=NULL; }
    if(snd_quad) { FreeVec(snd_quad); snd_quad=NULL; }
    if(snd_noise) { FreeVec(snd_noise); snd_noise=NULL; }
    if(snd_7bit[0]) { FreeVec(snd_7bit[0]); snd_7bit[0]=NULL; }
    if(snd_7bit[1]) { FreeVec(snd_7bit[1]); snd_7bit[1]=NULL; }
    fullaudio=0;
} // freeaudio()


void REGARGS setchannel1(REG(d0,u_byte vol),REG(d1,u_word per)) {
    audioreq->ioa_Request.io_Unit=(struct Unit *)0x03;      // channel 0,1
    per1=audioreq->ioa_Period=freqvec[per];
    vol1=audioreq->ioa_Volume=vol;
    DoIO((struct IORequest *)audioreq);
} // setchannel1()

void REGARGS setchannel2(REG(d0,u_byte vol),REG(d1,u_word per)) {
    audioreq->ioa_Request.io_Unit=(struct Unit *)0x0c;      // channel 2,3
    per2=audioreq->ioa_Period=freq2[per];
    vol2=audioreq->ioa_Volume=vol;
    DoIO((struct IORequest *)audioreq);
} // setchannel2()

void soundpause(void) {
    int v1=vol1,v2=vol2,p1=per1,p2=per2;
    if(opt_nosound!=0) return;
    if(opt_sid==0) {
        setchannel1(0,0);
        setchannel2(0,0);
        vol1=v1;
        vol2=v2;
        per1=p1;
        per2=p2;
    } else sidpause();
} // soundpause()

void soundresume(void) {
    if(opt_nosound!=0) return;
    if(opt_sid==0) {
        audioreq->ioa_Request.io_Command=ADCMD_PERVOL;
        audioreq->ioa_Request.io_Unit=(struct Unit *)0x03;      // channel 0,1
        audioreq->ioa_Period=per1;
        audioreq->ioa_Volume=vol1;
        DoIO((struct IORequest *)audioreq);
        audioreq->ioa_Request.io_Unit=(struct Unit *)0x0c;      // channel 2,3
        audioreq->ioa_Period=per2;
        audioreq->ioa_Volume=vol2;
        DoIO((struct IORequest *)audioreq);
    } else sidresume();
} // soundresume()

void REGARGS setbit7(REG(a0,char *buf)) {
static int acnt=0;
    BYTE *newsnd;

    newsnd=snd_7bit[acnt];
    memcpy(newsnd,buf,BIT7_BUFLEN);

    WaitIO((struct IORequest *)audioreq7_1[acnt]);
    WaitIO((struct IORequest *)audioreq7_2[acnt]);

    audioreq7_1[acnt]->ioa_Data=newsnd;
    audioreq7_2[acnt]->ioa_Data=newsnd;

    BeginIO((struct IORequest *)audioreq7_1[acnt]);
    BeginIO((struct IORequest *)audioreq7_2[acnt]);

    acnt^=1;
} // setbit7()

void setch2noise(void) {
    AbortIO((struct IORequest *)audioreq2a);
    WaitIO((struct IORequest *)audioreq2a);
    AbortIO((struct IORequest *)audioreq2b);
    WaitIO((struct IORequest *)audioreq2b);

    audioreq2a->ioa_Request.io_Command=CMD_WRITE;
    audioreq2b->ioa_Request.io_Command=CMD_WRITE;
    audioreq2a->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq2b->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq2a->ioa_Data=snd_noise;
    audioreq2b->ioa_Data=snd_noise;
    audioreq2a->ioa_Length=noiselen;
    audioreq2b->ioa_Length=noiselen;
    audioreq2a->ioa_Period=0;
    audioreq2b->ioa_Period=0;
    audioreq2a->ioa_Volume=0;
    audioreq2b->ioa_Volume=0;

    audioreq->ioa_Request.io_Unit=(struct Unit *)0x0c;
    audioreq->ioa_Request.io_Command=CMD_STOP;
    DoIO((struct IORequest *)audioreq);

    BeginIO((struct IORequest *)audioreq2a);
    BeginIO((struct IORequest *)audioreq2b);

    audioreq->ioa_Request.io_Command=CMD_START;
    DoIO((struct IORequest *)audioreq);

    freq2=noisefreqvec;
    vol2=per2=0;

    audioreq->ioa_Request.io_Command=ADCMD_PERVOL;
} // setch2noise()

void setch2quad(void) {
    AbortIO((struct IORequest *)audioreq2a);
    WaitIO((struct IORequest *)audioreq2a);
    AbortIO((struct IORequest *)audioreq2b);
    WaitIO((struct IORequest *)audioreq2b);

    audioreq2a->ioa_Request.io_Command=CMD_WRITE;
    audioreq2b->ioa_Request.io_Command=CMD_WRITE;
    audioreq2a->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq2b->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq2a->ioa_Data=snd_quad;
    audioreq2b->ioa_Data=snd_quad;
    audioreq2a->ioa_Length=quadlen;
    audioreq2b->ioa_Length=quadlen;
    audioreq2a->ioa_Period=0;
    audioreq2b->ioa_Period=0;
    audioreq2a->ioa_Volume=0;
    audioreq2b->ioa_Volume=0;

    audioreq->ioa_Request.io_Unit=(struct Unit *)0x0c;
    audioreq->ioa_Request.io_Command=CMD_STOP;
    DoIO((struct IORequest *)audioreq);

    audioreq->ioa_Request.io_Command=CMD_START;

    BeginIO((struct IORequest *)audioreq2a);
    BeginIO((struct IORequest *)audioreq2b);

    // START CH_2+CH_1
    DoIO((struct IORequest *)audioreq);

    freq2=freqvec;
    vol2=per2=0;
    audioreq->ioa_Request.io_Command=ADCMD_PERVOL;
} // setch2quad()

void setbit7on(void) {
    AbortIO((struct IORequest *)audioreq2a);
    WaitIO((struct IORequest *)audioreq2a);
    AbortIO((struct IORequest *)audioreq2b);
    WaitIO((struct IORequest *)audioreq2b);
    AbortIO((struct IORequest *)audioreq1a);
    WaitIO((struct IORequest *)audioreq1a);
    AbortIO((struct IORequest *)audioreq1b);
    WaitIO((struct IORequest *)audioreq1b);

    BeginIO((struct IORequest *)audioreq7_1[0]);
    BeginIO((struct IORequest *)audioreq7_2[0]);
    BeginIO((struct IORequest *)audioreq7_1[1]);
    BeginIO((struct IORequest *)audioreq7_2[1]);
    bit7flag=1;
    vol2=per2=vol1=per1=0;
} // setbit7on()

/*
 * input: 0-ch2-n négyszög van, más-zaj van
 */
void setbit7off(void) {
    if(NULL==CheckIO((struct IORequest *)audioreq7_2[0])) AbortIO((struct IORequest *)audioreq7_2[0]);
    WaitIO((struct IORequest *)audioreq7_2[0]);
    if(NULL==CheckIO((struct IORequest *)audioreq7_2[1])) AbortIO((struct IORequest *)audioreq7_2[1]);
    WaitIO((struct IORequest *)audioreq7_2[1]);
    if(NULL==CheckIO((struct IORequest *)audioreq7_1[0])) AbortIO((struct IORequest *)audioreq7_1[0]);
    WaitIO((struct IORequest *)audioreq7_1[0]);
    if(NULL==CheckIO((struct IORequest *)audioreq7_1[1])) AbortIO((struct IORequest *)audioreq7_1[1]);
    WaitIO((struct IORequest *)audioreq7_1[1]);

    memset(snd_7bit[0],0,BIT7_BUFLEN);
    memset(snd_7bit[1],0,BIT7_BUFLEN);
    audioreq2a->ioa_Request.io_Command=CMD_WRITE;
    audioreq2b->ioa_Request.io_Command=CMD_WRITE;
    audioreq1a->ioa_Request.io_Command=CMD_WRITE;
    audioreq1b->ioa_Request.io_Command=CMD_WRITE;
    audioreq2a->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq2b->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq1a->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq1b->ioa_Request.io_Flags=ADIOF_PERVOL;
    audioreq2a->ioa_Volume=0;
    audioreq2b->ioa_Volume=0;
    audioreq1a->ioa_Volume=0;
    audioreq1b->ioa_Volume=0;

    audioreq->ioa_Request.io_Unit=(struct Unit *)0x0f;
    audioreq->ioa_Request.io_Command=CMD_STOP;
    DoIO((struct IORequest *)audioreq);

    BeginIO((struct IORequest *)audioreq2a);
    BeginIO((struct IORequest *)audioreq2b);
    BeginIO((struct IORequest *)audioreq1a);
    BeginIO((struct IORequest *)audioreq1b);

    audioreq->ioa_Request.io_Command=CMD_START;
    DoIO((struct IORequest *)audioreq);

    audioreq->ioa_Request.io_Command=ADCMD_PERVOL;
    bit7flag=0;
} // setbit7off()


/*
 * megpróbálja lefoglalni a joy-t
 */
int getjoy(void) {
    int r=0;
    if(nojoy==0) {
        if(0!=(r=initjoy())) p4req1(NULL,P4_WARNING,GetStr(MSG_0242));
        if(0!=(r=initjoy())) p4req1(NULL,P4_WARNING,GetStr(MSG_0243));
        if(r!=0) nojoy=1;
    }
    return(r);
}

/*
 * beállítja a cuccokat, a joy2 olvasásához
 * ret: 0-ok, más-hiba
 */
int initjoy(void) {
    int ok=0;
    BYTE controller_type=0;
    BYTE type=GPCT_ABSJOYSTICK;
    char *o;

    if(fulljoy!=0) return(0);
    if(NULL==(joyport=CreateMsgPort())) return(-1);
    if(NULL==(joyioreq=CreateIORequest(joyport,sizeof(struct IOStdReq)))) {
        freejoy();
        return(-1);
    }
    joyioreq->io_Message.mn_Node.ln_Type=NT_UNKNOWN;
    if(0!=OpenDevice("gameport.device",1,(struct IORequest *)joyioreq,0)) {
        freejoy();
        return(-1);
    }
    joydeviceopened=1;
    ok=-1;

    Forbid();
    joyioreq->io_Command=GPD_ASKCTYPE;          /* inquire current status */
    joyioreq->io_Flags=IOF_QUICK;
    joyioreq->io_Data=(APTR)&controller_type;   /* put answer in here */
    joyioreq->io_Length=1;
    if(DoIO((struct IORequest *)joyioreq)) {
        Permit();
        freejoy();
        return(-1);
    }
    if(controller_type==GPCT_NOCONTROLLER) {
        joyioreq->io_Command=GPD_SETCTYPE;
        joyioreq->io_Flags=IOF_QUICK;
        joyioreq->io_Data=(APTR)&type;
        joyioreq->io_Length=1;
        if(DoIO((struct IORequest *)joyioreq)) {
            Permit();
            freejoy();
            return(-1);
        }
        ok=0;
    }
    Permit();

    if(ok!=0) {
        freejoy();
        return(-1);
    }

    joyallocated=1;

    // trigger on all joystick key transitions
    joytrigger.gpt_Keys=GPTF_UPKEYS|GPTF_DOWNKEYS;
    joytrigger.gpt_XDelta=1;
    joytrigger.gpt_YDelta=1;
    joytrigger.gpt_Timeout=2000;
    joyioreq->io_Command=GPD_SETTRIGGER;
    joyioreq->io_Flags=IOF_QUICK;
    joyioreq->io_Data=(APTR)&joytrigger;
    joyioreq->io_Length=(LONG)sizeof(struct GamePortTrigger);
    if(DoIO((struct IORequest *)joyioreq)) {
        freejoy();
        return(-1);
    }

    // flush buffer
    joyioreq->io_Command=CMD_CLEAR;
    joyioreq->io_Flags=IOF_QUICK;
    joyioreq->io_Data=NULL;
    joyioreq->io_Length=0;
    if(DoIO((struct IORequest *)joyioreq)) {
/* Workaround for "NSD compliant"
 * ?? Bug in NSDPatch? I don't know...
 */
//      freejoy();
//      return(-1);
    }

    // first request
    joyioreq->io_Command=GPD_READEVENT;
    joyioreq->io_Flags=0;
    joyioreq->io_Data=(APTR)&joyevent;
    joyioreq->io_Length=sizeof(struct InputEvent);
    SendIO((struct IORequest *)joyioreq);

    // read prefs
    o=GetOption("JOY","NORMAL");
    if(strcmp(o,"NORMAL")==0) {         // normal
        joykeyindex=9;
        joyfiredown=0xbf;
        joyfireup=0x40;
    } else {                            // swapped
        joykeyindex=8;
        joyfiredown=0x7f;
        joyfireup=0x80;
    }

    fulljoy=1;
    return(0);
} // initjoy()


void freejoy() {
    BYTE notype=GPCT_NOCONTROLLER;
    int i;

    if(joyallocated!=0) {
        // wait for request in progress
        if(fulljoy!=0) {
            if(NULL==CheckIO((struct IORequest *)joyioreq)) AbortIO((struct IORequest *)joyioreq);
            WaitIO((struct IORequest *)joyioreq);
        }
        // free gameport
        joyioreq->io_Command=GPD_SETCTYPE;
        joyioreq->io_Flags=IOF_QUICK;
        joyioreq->io_Data=(APTR)&notype;
        joyioreq->io_Length=1;
        if(DoIO((struct IORequest *)joyioreq)) p4req1(NULL,P4_ERROR,GetStr(MSG_0245));
        joyallocated=0;
    }

    if(joydeviceopened) { CloseDevice((struct IORequest *)joyioreq); joydeviceopened=0; }
    if(joyioreq) { DeleteIORequest(joyioreq); joyioreq=NULL; }
    if(joyport) { DeleteMsgPort(joyport); joyport=NULL; }
    for(i=8;i<16;i++) keymatrix[i]=0xff;    // clr joy-part of keymatrix
    fulljoy=0;
} // freejoy()


/*
 * frame-nként
 * megnézi, hogy áll a joy, és a megfelelõ
 * bejegyzéseket állítja a keymátrixban
 */
INLINE void dojoy() {
    u_byte val;

    if(!fulljoy) return;

    if(NULL!=CheckIO((struct IORequest *)joyioreq)) {
        WaitIO((struct IORequest *)joyioreq);
        switch(joyevent.ie_Code) {
            case IECODE_LBUTTON :                           // fire down
                keymatrix[joykeyindex]&=joyfiredown;
                break;
            case (IECODE_LBUTTON|IECODE_UP_PREFIX) :        // fire up
                keymatrix[joykeyindex]|=joyfireup;
                break;
            case IECODE_NOBUTTON :                          // irány?
                // 0-bal, 1-semmi, 2-jobb
                val=joyxmask[joyevent.ie_X+1]&joyymask[joyevent.ie_Y+1];
                keymatrix[joykeyindex]|=0x0f;
                keymatrix[joykeyindex]&=val;
                break;
        }

        // send new request
        SendIO((struct IORequest *)joyioreq);
    }
} // dojoy()


INLINE void cleanupdrivermsglist(void) {
    c2pv->c2p_MsgList.lh_Head=(struct Node *)&c2pv->c2p_MsgList.lh_Tail;
    c2pv->c2p_MsgList.lh_Tail=NULL;
    c2pv->c2p_MsgList.lh_TailPred=(struct Node *)&c2pv->c2p_MsgList.lh_Head;
    drivermsgcount=0;
} // cleanupdrivermsglist


/*
 * beállítja a c2p új globálasit + idcmp-t
 */
int setupc2p(void) {
    cleanupdrivermsglist();
    flushbuffers();
    if(c2pv->c2p_Interface<4||c2pv->c2p_Direct==0||c2pv->c2p_lock==NULL||c2pv->c2p_unlock==NULL) {
//      modeflag&=~0x80;
        opt_direct=0;
        amirasaddr=chunky;
    } else {
        opt_direct=1;
//      modeflag|=0x80;
        amirasaddr=0;
    }
    c2pact=c2pv->c2p_dofull;
    cp4Scr=c2pv->c2p_Scr;
    opt_render=c2pv->c2p_Render;
    buffering=c2pv->c2p_Buffering<<2;
    opt_border=1;
    if(c2pv->c2p_Interface>2) opt_border=c2pv->c2p_Border;
    if(opt_border!=0) leftborder=16+hscroll;
    else leftborder=hscroll;
    cp4Window=c2pv->c2p_Win;
    if(cp4Window==NULL||cp4Scr==NULL) return(-1);
    c2pidcmp=cp4Window->IDCMPFlags;
    ModifyIDCMP(cp4Window,((c2pidcmp|IDCMP_RAWKEY|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW)&~IDCMP_VANILLAKEY));
    SetWindowTitles(cp4Window,(UBYTE *)~0,GetStr(MSG_0246));
    return(0);
} // setupc2p()


/*
 * inicializálja a screennel kapcs. dolgokat
 */
int initdisplay(void) {
static char c2pname[256];
    int def;
    char *erv;

    flushbuffers();

    /* open c2p
     */
    strcpy(c2pname,c2pdir);
    strcat(c2pname,opt_c2p);
    def=0;
    if(NULL==(seglist=LoadSeg(c2pname))) {
        strcpy(c2pname,c2pdir);
        strcat(c2pname,defc2p);
        def=1;
        if(NULL==(seglist=LoadSeg(c2pname))) {
            p4req1(NULL,P4_ERROR,GetStr(MSG_020C));
            return(-1);
        }
    }
    c2pstartup=(struct c2pvec *(*)(REG(d0,unsigned long m)))(((ULONG)seglist)*4+4);
    if(NULL==(c2pv=c2pstartup(C2P_MAGIC))) return(-1);
    nospeed=c2pv->c2p_NoSpeed;

    /* init c2p
     */
    if(c2pv->c2p_Interface>1) c2pv->c2p_Palette=opt_palette;
    c2pv->c2p_GetOptionInt=c2pGetOptionInt;
    c2pv->c2p_AddOptionInt=c2pAddOptionInt;
    c2pv->c2p_GetOptionStr=c2pGetOptionStr;
    c2pv->c2p_AddOptionStr=c2pAddOptionStr;
    if(NULL!=(erv=c2pv->c2p_init(opt_scrmode,opt_overscan,linedelta))) {
        if(*erv!='\0') {
            if(strlen(erv)>150) p4req1(NULL,P4_WARNING,GetStr(MSG_0247));
            else {
                strcpy(c2pname,c2pv->c2p_Name);
                strcat(c2pname,": ");
                strcat(c2pname,erv);
                p4req1(NULL,P4_C2P,c2pname);
            }
        }
        c2pv->c2p_free();
        if(def==0) {
            p4req1(NULL,P4_WARNING,GetStr(MSG_0249));
            strcpy(c2pname,c2pdir);
            strcat(c2pname,defc2p);
            if(NULL==(seglist=LoadSeg(c2pname))) return(-1);
            c2pstartup=(struct c2pvec *(*)(REG(d0,unsigned long m)))(((ULONG)seglist)*4+4);
            if(NULL==(c2pv=c2pstartup(C2P_MAGIC))) return(-1);
            if(c2pv->c2p_Interface>1) c2pv->c2p_Palette=opt_palette;
            c2pv->c2p_GetOptionInt=c2pGetOptionInt;
            c2pv->c2p_AddOptionInt=c2pAddOptionInt;
            c2pv->c2p_GetOptionStr=c2pGetOptionStr;
            c2pv->c2p_AddOptionStr=c2pAddOptionStr;
            if(NULL!=(erv=c2pv->c2p_init(opt_scrmode,opt_overscan,linedelta))) {
                if(*erv!='\0') {
                    if(strlen(erv)>150) p4req1(NULL,P4_WARNING,GetStr(MSG_0247));
                    else {
                        strcpy(c2pname,c2pv->c2p_Name);
                        strcat(c2pname,": ");
                        strcat(c2pname,erv);
                        p4req1(NULL,P4_C2P,c2pname);
                    }
                }
                c2pv->c2p_free();
                return(-1);
            }
        } else return(-1);
    }

    if(0!=(setupc2p())) return(-1);
    return(0);
} // initdisplay

void freedisplay(void) {
    cleanupdrivermsglist();
    if(c2pv) c2pv->c2p_free();
    cp4Window=NULL;
    cp4Scr=NULL;
    c2pact=NULL;
    if(seglist) { UnLoadSeg(seglist); seglist=NULL; }
} // freedisplay


void initszamok(char *p) {
    int i;
    if(*p=='\0') return;
    for(i=0;p[i]!='\0';i++) {
        switch(p[i]) {
            case '.' :
                p[i]=chalf;
                break;
            case '*' :
                p[i]=cfull;
                break;
            default :
                p[i]=0xff;
                break;
        }
    }
} // initszamok()


void initled(char *p) {
    int i;
    for(i=0;p[i]!='\0';i++) {
        switch(p[i]) {
            case '*' :
                p[i]=ledbor;
                break;
            case '#' :
                p[i]=ledcol;
                break;
            default :
                p[i]=0xff;
                break;
        }
    }
} // initled()


/* Exec exception routine
 */
#ifndef NOTASKEXCEPTION
 ULONG REGARGS LocException(REG(d0,ULONG sigs),REG(a1,APTR exdat)) {
    *((int *)exdat)=1;
    return(sigs);
 } // LocException()
#endif


void loadpalette(char *file, struct Window *win) {
    FILE *f;
    if(NULL!=(f=fopen(opt_palettefile,"rb"))) fread(opt_palette,1,384,f);
    else {
        sprintf(errmsg,"%s (%s)",GetStr(MSG_01DA),opt_palettefile);
        p4req1(win,P4_WARNING,errmsg);
    }
}


/*
 * az emuláció elõtt meghívódik
 */
void tedinit(void) {
static char locname[]="ENV:sys/locale.prefs";
    int i;

    if(NULL==(coltable=gentab(COLORTABLE)))
        tederror(GetStr(MSG_024A));
    if(NULL==(backtable=gentab(BACKTABLE)))
        tederror(GetStr(MSG_024B));

    /* Init logo
     */
    if(logo[0]!='\0') for(i=0;logo[i]!='\0';i++) if(logo[i]!='*') logo[i]='\0';

    /* load palette file, if required
     */
    if(strcmp(opt_palettefile,"NONE")!=0) loadpalette(opt_palettefile,NULL);

    /* Init framerate
     */
    initszamok(szamok);
    initszamok(szazalek);
    initszamok(pont);
    initled(ledgfx);

    /* Init exception handling
     */
    locsignum=-1;
    locsig=~0;
    exceptfull=0;
    if((locsignum=AllocSignal(-1L))!=-1) {
        struct Task *tt;
        tt=FindTask(NULL);
        locnotifyreq.nr_Name=locname;
        locnotifyreq.nr_Flags=NRF_SEND_SIGNAL;
        locnotifyreq.nr_stuff.nr_Signal.nr_Task=tt;
        locnotifyreq.nr_stuff.nr_Signal.nr_SignalNum=locsignum;
        if(DOSTRUE==(StartNotify(&locnotifyreq))) {
            locsig=1L<<locsignum;
#ifndef NOTASKEXCEPTION
            tt->tc_ExceptCode=LocException;
            tt->tc_ExceptData=&locMOD;
            SetExcept(-1,locsig);
#endif
            exceptfull=1;
        } else {
            FreeSignal(locsignum);
            locsignum=-1;
            locsig=~0;
        }
    }

    // ROM default
    hiram=p4rom;

    // ASM-hez
    myram=p4ram;

    // set keymap
    if(0!=setkeymap(opt_keymap)) {
        p4req1(NULL,P4_WARNING,GetStr(MSG_024C));
        opt_keymap=NULL;
        AddOption("KEYMAP","DEFAULT");
        setkeymap(NULL);
    }

    // grafikus rendszer init
    modegfx[0]=dogfx_offscr;
    modegfx[1]=dogfx_offscr;
    modegfx[2]=dogfx_offscr;
    modegfx[3]=dogfx_offscr;
    modegfx[4]=dogfx_offscr;
    modegfx[5]=dogfx_error;
    modegfx[6]=dogfx_offscr;
    modegfx[7]=dogfx_error;
    modegfx[8]=dogfx_offscr;
    modegfx[9]=dogfx_offscr;
    modegfx[10]=dogfx_offscr;
    modegfx[11]=dogfx_offscr;
    modegfx[12]=dogfx_offscr;
    modegfx[13]=dogfx_error;
    modegfx[14]=dogfx_offscr;
    modegfx[15]=dogfx_error;
    modegfx[16]=dogfx_nrmtxt128;            // inverz (128char)
    modegfx[17]=dogfx_mlttxti;
    modegfx[18]=dogfx_nrmgrf;
    modegfx[19]=dogfx_mltgrf;
    modegfx[20]=dogfx_exttxti;
    modegfx[21]=dogfx_error;
    modegfx[22]=dogfx_nrmgrf;
    modegfx[23]=dogfx_error;    
    modegfx[24]=dogfx_nrmtxt256;            // noinverz (256char)
    modegfx[25]=dogfx_mlttxtn;
    modegfx[26]=dogfx_nrmgrf;
    modegfx[27]=dogfx_mltgrf;
    modegfx[28]=dogfx_exttxtn;
    modegfx[29]=dogfx_error;
    modegfx[30]=dogfx_nrmgrf;
    modegfx[31]=dogfx_error;
// FLASH
    modegfx[32]=dogfx_offscr;
    modegfx[33]=dogfx_offscr;
    modegfx[34]=dogfx_offscr;
    modegfx[35]=dogfx_offscr;
    modegfx[36]=dogfx_offscr;
    modegfx[37]=dogfx_error;
    modegfx[38]=dogfx_offscr;
    modegfx[39]=dogfx_error;
    modegfx[40]=dogfx_offscr;
    modegfx[41]=dogfx_offscr;
    modegfx[42]=dogfx_offscr;
    modegfx[43]=dogfx_offscr;
    modegfx[44]=dogfx_offscr;
    modegfx[45]=dogfx_error;
    modegfx[46]=dogfx_offscr;
    modegfx[47]=dogfx_error;
    modegfx[48]=dogfx_nrmtxt128f;           // inverz (128char) + flash
    modegfx[49]=dogfx_mlttxti;
    modegfx[50]=dogfx_nrmgrf;
    modegfx[51]=dogfx_mltgrf;
    modegfx[52]=dogfx_exttxti;
    modegfx[53]=dogfx_error;
    modegfx[54]=dogfx_nrmgrf;
    modegfx[55]=dogfx_error;    
    modegfx[56]=dogfx_nrmtxt256f;           // noinverz (256char) +flash
    modegfx[57]=dogfx_mlttxtn;
    modegfx[58]=dogfx_nrmgrf;
    modegfx[59]=dogfx_mltgrf;
    modegfx[60]=dogfx_exttxtn;
    modegfx[61]=dogfx_error;
    modegfx[62]=dogfx_nrmgrf;
    modegfx[63]=dogfx_error;
// USER REQUESTED BLANK
    for(i=64;i<128;i++) modegfx[i]=dogfx_none;

// DIRECT MODE
    modegfx[128]=dogfx_offscrD;
    modegfx[129]=dogfx_offscrD;
    modegfx[130]=dogfx_offscrD;
    modegfx[131]=dogfx_offscrD;
    modegfx[132]=dogfx_offscrD;
    modegfx[133]=dogfx_errorD;
    modegfx[134]=dogfx_offscrD;
    modegfx[135]=dogfx_errorD;
    modegfx[136]=dogfx_offscrD;
    modegfx[137]=dogfx_offscrD;
    modegfx[138]=dogfx_offscrD;
    modegfx[139]=dogfx_offscrD;
    modegfx[140]=dogfx_offscrD;
    modegfx[141]=dogfx_errorD;
    modegfx[142]=dogfx_offscrD;
    modegfx[143]=dogfx_errorD;
    modegfx[144]=dogfx_nrmtxt128D;          // inverz (128char)
    modegfx[145]=dogfx_mlttxtiD;
    modegfx[146]=dogfx_nrmgrfD;
    modegfx[147]=dogfx_mltgrfD;
    modegfx[148]=dogfx_exttxtiD;
    modegfx[149]=dogfx_errorD;
    modegfx[150]=dogfx_nrmgrfD;
    modegfx[151]=dogfx_errorD;  
    modegfx[152]=dogfx_nrmtxt256D;          // noinverz (256char)
    modegfx[153]=dogfx_mlttxtnD;
    modegfx[154]=dogfx_nrmgrfD;
    modegfx[155]=dogfx_mltgrfD;
    modegfx[156]=dogfx_exttxtnD;
    modegfx[157]=dogfx_errorD;
    modegfx[158]=dogfx_nrmgrfD;
    modegfx[159]=dogfx_errorD;
// FLASH
    modegfx[160]=dogfx_offscrD;
    modegfx[161]=dogfx_offscrD;
    modegfx[162]=dogfx_offscrD;
    modegfx[163]=dogfx_offscrD;
    modegfx[164]=dogfx_offscrD;
    modegfx[165]=dogfx_errorD;
    modegfx[166]=dogfx_offscrD;
    modegfx[167]=dogfx_errorD;
    modegfx[168]=dogfx_offscrD;
    modegfx[169]=dogfx_offscrD;
    modegfx[170]=dogfx_offscrD;
    modegfx[171]=dogfx_offscrD;
    modegfx[172]=dogfx_offscrD;
    modegfx[173]=dogfx_errorD;
    modegfx[174]=dogfx_offscrD;
    modegfx[175]=dogfx_errorD;
    modegfx[176]=dogfx_nrmtxt128fD;         // inverz (128char) + flash
    modegfx[177]=dogfx_mlttxtiD;
    modegfx[178]=dogfx_nrmgrfD;
    modegfx[179]=dogfx_mltgrfD;
    modegfx[180]=dogfx_exttxtiD;
    modegfx[181]=dogfx_errorD;
    modegfx[182]=dogfx_nrmgrfD;
    modegfx[183]=dogfx_errorD;  
    modegfx[184]=dogfx_nrmtxt256fD;         // noinverz (256char) +flash
    modegfx[185]=dogfx_mlttxtnD;
    modegfx[186]=dogfx_nrmgrfD;
    modegfx[187]=dogfx_mltgrfD;
    modegfx[188]=dogfx_exttxtnD;
    modegfx[189]=dogfx_errorD;
    modegfx[190]=dogfx_nrmgrfD;
    modegfx[191]=dogfx_errorD;
// USER REQUESTED BLANK
    for(i=192;i<256;i++) modegfx[i]=dogfx_none;

    // fill buffctrl:
    // melyik cnt-hez és bufferinghez melyik delta tömb kell:
    //                                  buffering     count
    buffctrl[0]=3;                      //  0           0       NULL
    buffctrl[1]=3;                      //  0           1       NULL
    buffctrl[2]=3;                      //  0           2       NULL
    buffctrl[3]=3;                      // ---------------      NULL
    buffctrl[4]=1;                      //  1           0
    buffctrl[5]=2;                      //  1           1
    buffctrl[6]=0;                      //  1           2
    buffctrl[7]=3;                      // ---------------      NULL
    buffctrl[8]=2;                      //  2           0
    buffctrl[9]=0;                      //  2           1
    buffctrl[10]=1;                     //  2           2
    buffctrl[11]=3;                     // ---------------      NULL
    buffctrl[12]=0;                     //  3           0
    buffctrl[13]=1;                     //  3           1
    buffctrl[14]=2;                     //  3           2
    buffctrl[15]=3;                     // ---------------      NULL

    // Gyári konfiguráció
    for(i=0;i<MAXROM;i++) p4rom[0x8000+i]=roms[ROM_BASIC][i];
    for(i=0;i<MAXROM;i++) p4rom[0xc000+i]=roms[ROM_KERNAL][i];

    // nullázás
    for(i=0;i<REGNUM;i++) hiram[TED+i]=0;

    // globálisok inicializálása
    charmap=&p4rom[0xd000];                 // gyári settings
    charinram=1;                            // charok a ROMban
    actraster=0;                            // akt raszter
    p4actras=274;                           // c+4 raster száml (3-203 valid)
    hras=0;                                 // horiz. raster
    textplace=0x0c00;
    colplace=0x0800;
    singleclock=0;                          // doubleclock on
    hiram[0xff12]|=B2;
    gfxaddr=0x2000;                         // grafika
    stopcnt1=0; stopcnt2=0; stopcnt3=0;     // minden számláló megy
    initcnt1hi=0xff; initcnt1lo=0xff;       //      mint a többi
    ec1=0; ec2=0; ec3=0;                    //      mind letiltva
    eras=0;                                 // raszter irq letiltva
    rasreq=0;                               // 0-s raszternél
    curpos=0;                               // kurzor helye
    cureor=0x80;                            // kurzor (inv/nrm)
    keyread=0xff;                           // fd30-fd3f
    for(i=0;i<16;i++) keymatrix[i]=0xff;    // keymatrix nullázás
    frame=0;
    actrom=0;                               // KERNAL - BASIC
    actramrom=ROM;                          // default - ROM
    colback=col[0]=0;                       // háttérszín
    colbord=0;                              // keretszín (ff19)
    col[1]=col1=0;                          // ff16
    col[2]=col2=0;                          // ff17
    col[3]=col3=0;                          // ff18
    hscroll=0;                              // no
    vscroll=0;                              //    screen scrolls
    offscr=B4;                              // képkioltás (0-kioltva)
    extcol=0;                               // Extended Color Mode off
    multicol=0;                             // Multi Color Mode off
    bitmap=0;                               // Bitmap Mode off
    cols=B3;                                // 40 oszlopos kép
    tedoff=0;                               // TED enable
    dogfx=dogfx_nrmtxt128;                  // default: normal text mode
    leftborder=rightborder=16;              // szélsõ keret
    sideborders=4;                          // igazi keret
    realmaxraster=REALMAXRAS;
    actinchar=0;                            // char-on belül
    actcharaddr=0;                          // akt. char. addr (def)
    tbordminus=0;                           // nincs 24-soros scr
    numraster=200;                          // normal 25 soros scr
    clkperrast=CLOCKPERRASTERFAST;          // kereten vagyunk

    // CPU I/O port init
    p4ram[0]=0x0f;
    p4ram[1]=0xc8;

    /*
     * open libraries
     */
    if(!IntuitionBase) if(!(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",37))) tederror(GetStr(MSG_0060));
    if(!GfxBase) if((GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",37))==NULL) tederror(GetStr(MSG_024E));
    if(!GadToolsBase) if(NULL==(GadToolsBase=OpenLibrary("gadtools.library",37))) tederror(GetStr(MSG_0250));
    if(!AslBase) if(NULL==(AslBase=OpenLibrary("asl.library",37))) tederror(GetStr(MSG_0252));
    if(!IECBase) IECBase=(struct iecbase *)OpenLibrary("iec.library",0);
    if(!XpkBase) XpkBase=OpenLibrary(XPKNAME,0);
    if(XpkBase==NULL||strcmp(opt_xpktype,"----")==0) opt_xpk=0;
    if(opt_iec==0&&IECBase!=NULL) { CloseLibrary((struct Library *)IECBase); IECBase=NULL; }

    /* Allocate screenmode requester
     */
    if((scrrq=AllocAslRequestTags(ASL_ScreenModeRequest,TAG_DONE,0L))==NULL) tederror(GetStr(MSG_0253));

    /* Ask screenmode, if firstrun on this system
     */
//  if(opt_scrmode==~0||opt_overscan==~0) {
//      struct Screen *scr;
//      scr=LockPubScreen(NULL);
//      if(AslRequestTags(scrrq,
//          ASLSM_Screen, (ULONG)scr,
//          ASLSM_TitleText,(ULONG)GetStr(MSG_0211),
//          ASLSM_InitialOverscanType, OSCAN_STANDARD,
//          ASLSM_DoOverscanType,TRUE,
//          ASLSM_PropertyMask, (ULONG)0,
//          TAG_DONE,0L)) {
//          opt_scrmode=scrrq->sm_DisplayID;
//          opt_overscan=scrrq->sm_OverscanType;
//      }
//      UnlockPubScreen(NULL,scr);
//      if(opt_scrmode==~0||opt_overscan==~0) tederror(GetStr(MSG_0254));
//  }

    /* MsgPort foglalás
     */
    if(NULL==(devmsgport=CreateMsgPort()))
        tederror(GetStr(MSG_0255));

    /* CreateIORequest
     */
    if(NULL==(devioreq=CreateIORequest(devmsgport,sizeof(struct timerequest))))
        tederror(GetStr(MSG_0256));

    /* Opening Timer.device
     */
    if(0!=(OpenDevice("timer.device",UNIT_ECLOCK,(struct IORequest *)devioreq,0)))
        tederror(GetStr(MSG_0258));
    TimerBase=devioreq->tr_node.io_Device;
    devioreq->tr_node.io_Command=TR_ADDREQUEST;
    devioreq->tr_time.tv_secs=0;

    /* init display
     */
    if(0!=initdisplay()) tederror(GetStr(MSG_0259));

    /* Init time things (framerate)
     */
    hz80=ReadEClock(&ido1);
    otvened1=hz80/50;
    hz1=hz80*10;
    otvened2=hz80/25;
    hz2=hz80*20;
    if(opt_twoscr==~0) {
        otvenedsec=hz80/25;
        hz80*=20;
    } else {
        otvenedsec=hz80/50;
        hz80*=10;
    }
    if(opt_limit==0) otvenedsec=0;
    timestart.ev_lo=ido1.ev_lo;
    timestart.ev_hi=ido1.ev_hi;

    /* Allocate filerequester (if reqtools needed --> use a patch)
     */
    if((frq=AllocAslRequestTags(    ASL_FileRequest,
                                    ASLFR_SleepWindow,TRUE,
                                    ASLFR_RejectIcons,TRUE,
                                    TAG_DONE,0L ))==NULL)
        tederror(GetStr(MSG_025A));

    if(opt_direct==0) amirasaddr=chunky;    // triplebuffer init
    else amirasaddr=0;
    hiram[0xff15]=0xee;             // háttér
    hiram[0xff1f]&=~B7;             // crsr flash - hogy mindig túlcsorduljon
    hiram[0xff1c]=0xfe;
    colback=col[0]=0xee;

    /* Allocate & initialize joyport(s)
     */
    if(0!=initjoy()) {
        p4req1(NULL,P4_WARNING,GetStr(MSG_0243));
        nojoy=1;
    }

    /* Allocate & initialize audio
     */
    if(opt_nosound==0&&opt_sid==0) if(0!=initaudio()) opt_nosound=1;
    if(opt_nosound==0&&opt_sid!=0) if(0!=initsid()) opt_nosound=1;

    // burst mode enable
    oldCache=CacheControl(CACRF_DBE,CACRF_DBE);

    // show screen
    ScreenToFront(cp4Scr);
} // tedinit()


/*
 * kilépés elõtt hívódik, ezután nem hívható a fõrutin
 */
void tedfree(void) {
#ifdef STAT
    unsigned long *pnt;
    int i;
#endif
    if(TimerBase) ReadEClock(&timeend);
    CacheControl(oldCache,(ULONG)~0);
    if(coltable) { free(coltable); coltable=NULL; }
    if(backtable) { free(backtable); backtable=NULL; }
    freedisplay();
    if(XpkBase) { CloseLibrary(XpkBase); XpkBase=NULL; }
    if(GfxBase) { CloseLibrary((struct Library *)GfxBase); GfxBase=NULL; }
    if(IntuitionBase) { CloseLibrary((struct Library *)IntuitionBase); IntuitionBase=NULL; }
    if(GadToolsBase) { CloseLibrary(GadToolsBase); GadToolsBase=NULL; }
    if(frq) { FreeAslRequest(frq); frq=NULL; }
    if(scrrq) { FreeAslRequest(scrrq); scrrq=NULL; }
    if(AslBase) { CloseLibrary(AslBase); AslBase=NULL; }
    if(IECBase) { CloseLibrary((struct Library *)IECBase); IECBase=NULL; }
    if(DiskfontBase) { CloseLibrary(DiskfontBase); DiskfontBase=NULL; }
    if(TimerBase) { CloseDevice((struct IORequest *)devioreq); TimerBase=NULL; }
    if(devioreq) { DeleteIORequest(devioreq); devioreq=NULL; }
    if(devmsgport) { DeleteMsgPort(devmsgport); devmsgport=NULL; }
    if(fulljoy) { freejoy(); fulljoy=0; }
    if(fullaudio) { freeaudio(); fullaudio=0; }
    endsid();
    if(locsignum!=-1) {
        FreeSignal(locsignum); locsignum=-1; locsig=~0;
        if(exceptfull!=0) { EndNotify(&locnotifyreq); exceptfull=0; }
    }
#ifdef STAT
    if(CodeTab!=0) {            // visszamásoljuk a helyére
        pnt=&CodeTab;
        pnt++;
        for(i=0;i<256;i++) pnt[i]=codetab[i];
    }
#endif
    return;
} // tedfree()


void doled(unsigned char *b) {
static int n=0;
    char *ld;
    int i,j;

    ld=ledgfx;
    c2pv->c2p_Led=0;
    if((d64_LED==LED_ON)||(d64_LED==LED_FLASH&&(n&8)==0)) {
        ld+=32;
        c2pv->c2p_Led=1;
    }
    if(opt_direct!=0) return;
    b+=((352*248)+4);
    i=4;
    do {
        j=8;
        do {
            if(*ld>=0) *b=*ld;
            ld++; b++;
        } while(--j);
        b+=(352-8);
    } while(--i);
    n++;
    n&=15;
} // doled()

/*
 * frame-enként
 */
#define MAXSKIP     14
int doframe(void) {
static int cnt=0;
static int frc=0;
    int nextdelta;
    int r=0;
    u_byte *tmpscr;
    ULONG frametime;

    nextdelta=buffctrl[cnt+buffering];

    // c2p, triplabuffer, stb...
    ret=0;

//  c2pv->c2p_CharMap=charmap;
//  c2pv->c2p_VideoMatrix=c2pVideo;
//  c2pv->c2p_ColorMatrix=c2pColor;
//  c2pv->c2p_Graphics=c2pGfx;
//  c2pv->c2p_ModeFlag=c2pModeflag;
//  c2pv->c2p_CursorPos=curpos;

    c2pv->c2p_Speed=-1;
    if(opt_percent!=0) {
        if(nospeed==0) doframerate(chunky);
        c2pv->c2p_Speed=framerate;
    }
    if(opt_onscreendisplay!=0) doled(chunky);
    else c2pv->c2p_Led=-1;
    dologo(chunky);
#ifdef STAT
    domemstat(chunky);
#endif
    doscr();

    /* Handle Module locale notify
     */
#ifdef NOTASKEXCEPTION
    if(SetSignal(0,0)&locsig) {
        SetSignal(0,locsig);
#else
    if(locMOD!=0) {
        locMOD=0;
#endif
        r=c2pv->c2p_internal(C2PIF_MODLOCALE);
        if(r!=0) goto errorhandle;
        setupc2p();
        Closecp4Catalog();
        Opencp4Catalog(NULL);
        flushbuffers();
        c2pact=c2pv->c2p_dofull;            // set full c2p
    }

    if(userblank==0&&skipflag==0) {
        if(opt_border!=0) r=c2pact(chunky,delta[nextdelta],nextdelta);
        else r=c2pact(chunky+(41*320),delta[nextdelta]+(41*320),nextdelta);
        cleanupdrivermsglist();             // clean msg list
        tmpscr=chunky;
        chunky=delta[cnt];
        delta[cnt]=tmpscr;
        if(opt_direct==0) amirasaddr=chunky;
        else amirasaddr=0;
        c2pact=c2pv->c2p_do;                // back to delta c2p
    } else {
        if(userblank!=0) r=c2pv->c2p_dont();
    }

errorhandle:
    if(r!=0) {
        switch(r) {
            case -2 :                       // Error
                p4req1(NULL,P4_ERROR,GetStr(MSG_025B));
                ret|=(B1+B4);
                break;
            case -1 :                       // New Win.
                if(0!=setupc2p()) {
                    p4req1(NULL,P4_ERROR,GetStr(MSG_025C));
                    ret|=(B1+B4);
                }
                break;
            case 0 :
            case 1 :
                ret|=B1;                    // debug req
                break;
            case 2 :
                ret|=(B1+B4);               // prefs req
                break;
            case 3 :
                reset();                    // reset req
                ret|=B2;
                break;
            case 4 :
                reset();                    // hard-reset req
                ret|=B2+B3;
                break;
            case 5 :                        // quit req
                ret=B7;
                break;
        }
        if(userblank!=0||skipflag!=0) {
            skiptime=0;
            skipflag=0;
            userblank=0;
            c2pv->c2p_dofull(chunky,NULL,nextdelta);
            tmpscr=chunky;
            chunky=delta[cnt];
            delta[cnt]=tmpscr;
            if(opt_direct==0) amirasaddr=chunky;
            else amirasaddr=0;
        }
    }
    ReadEClock(&ido2);
    frametime=ido2.ev_lo-ido1.ev_lo;
    if(frametime<otvenedsec) {
        if(opt_realtime!=0&&skipflag!=0) {
            // kisebb, de nem várni kell, mert REALTIME-ot kértek
            skiptime-=otvenedsec-frametime;
            if(skiptime<=0) {
                skipflag=0;
            } else if(++skipflag>=MAXSKIP) {
                // maximum 6 frame-t skippelünk
                skipflag=0;
                skiptime=0;
            }
        } else {
            // idõzítés, ha kell...
            devioreq->tr_time.tv_micro=otvenedsec-frametime;
            DoIO((struct IORequest *)devioreq);
            ReadEClock(&ido2);
            frametime=ido2.ev_lo-ido1.ev_lo;
        }
    } else if(opt_realtime!=0) {
        // REALTIME request, count time
        skiptime+=frametime-otvenedsec;
        if(++skipflag>=MAXSKIP) {
            // maximum 6 frame-t skippelünk
            skipflag=0;
            skiptime=0;
        }
    }
#ifndef STAT
    if(opt_percent!=0) {
#endif
        frc++;
        frc&=3;
        if(opt_realtime!=0) fr[frc]=skipflag*5;
        else fr[frc]=hz80/frametime;
        framerate=(fr[0]+fr[1]+fr[2]+fr[3])>>1;
#ifdef STAT
        if(opt_realtime==0) statfpssum+=(framerate/10);
        else statfpssum+=100;
        statfpsnum++;
#else
    }
#endif
    ido1.ev_lo=ido2.ev_lo;
    cnt=(cnt!=2 ? cnt+1 : 0);
    return(ret);
} // doframe()
