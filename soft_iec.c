/* :ts=4                        soft_iec.c
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

#include <exec/types.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <exec/memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "soft_iec.h"
#include "macros.h"
#include "cp4_loc.h"
#include "p4req.h"
#include "prefs.h"


// flags in fileinfo.flags
#define F_OPEN      0x80
#define F_PROT      0x40

// flags fileinfo.mode
#define FM_READ     0
#define FM_WRITE    1

UBYTE d64_ST;               // external
char d64_LED;               // led state (0-LED_OFF 1-LED_ON 2-LED_FLASH)

struct fileinfo {
    int type;               // PRG SEQ REL USR DEL (NONE)
    int flags;              // opened / protected
    int tr,sc;              // elsõ adatblokk
    char name[17];          // név + 0
    int ntr,nsc;            // @: track/sector
    int blocks;
    int reltr,relsc;        // REL-nél elsõ oldalszektor
    int relln;              // REL-nél rekordhossz
    int dirt,dirs;          // directory data
    int diroffs;            //   offset in block
    int wrtlasttr;          // utoljára kiírt track
    int wrtlastsc;          //                sector
    int mode;               // FM_READ/FM_WRITE
};

struct Channel {
    int busy;
    int secmode;
    int bufnum;
    struct fileinfo file;
    int filenamelen;
    char filename[32];
};

// Channel.busy defines
#define CH_FREE     0
#define CH_BUSY     1

// Channel.bufnum
#define CH_NONE     -1

// secondary addresses                 cmd  addr
#define ICMD_DATA   0x60            // 0110 0000
#define ICMD_CLOSE  0xe0            // 1110 0000
#define ICMD_OPEN   0xf0            // 1111 0000

// global mode flags
#define IM_NONE     0
#define IM_LISTEN   1
#define IM_TALK     2

// Channel.secmode flags
#define SM_NONE         0
#define SM_FILENAME     1
#define SM_CLOSE        2
#define SM_WRITETOD64   3
#define SM_READFROMD64  4
#define SM_FILEOPEN     5
#define SM_COMMAND      6
#define SM_COMMANDINPUT 7

// filetype in fileinfo.type
#define T_DIR       -3
#define T_RND       -2
#define T_NONE      -1
#define T_DEL       0
#define T_SEQ       1
#define T_PRG       2
#define T_USR       3
#define T_REL       4

// flags for diskinfo.bam[]
#define BAM_FREE            1
#define BAM_BUSY            0

// diskinfo.bfree
#define NODISK              -1

// diskinfo.writeprotect
#define WP_OFF              0
#define WP_ON               1

struct diskinfo {
    int writeprotect;       // WP_ON / WP_OFF
    unsigned char dtype;    // 'A'
    unsigned char flag;     // 0
    char name[17];          // name + 0
    char id[2];             // ID
    unsigned char ti;       // 0xa0
    char ty[2];             // '2A'
    int dt,ds;              // dir tr/sc
    int bfree;              // blocks free
    char bam[683];          // 0-szabad, 1-foglalt
    char freebpt[36];       // free blocks/track (0. unused)
    int status;             // disk status
    int statustr,statussc;  //   status track/sector
    int opencounter;        // ? file van nyitva?
    UBYTE ib[5][256];       // 5 belsõ buffer
    int ibnum[5];           //      -o-       számlálója
    FILE *d64;              // opened d64 file
    struct Channel chn[17]; // internal channels
    int numstatus;          // hol tart a kiírás
};

// free_buffer()-hez
#define FB_INTERNAL     -1
// alloc_buffer()-hez
#define AB_ANY          -1

// error strings
#define MN          "00,"
#define ML          ",00,00\015"
#define M           "?"
#define MO          " OK"
#define MFS         "FILES SCRATCHED"
#define MRE         "READ ERROR"
#define MWE         "WRITE ERROR"
#define MWPO        "WRITE PROTECT ON"
#define MDIM        "DISK ID MISMATCH"
#define MSE         "SYNTAX ERROR"
#define MFNF        "FILE NOT FOUND"
#define MRNP        "RECORD NOT PRESENT"
#define MOIR        "OWERFLOW IN RECORD"
#define MFTL        "FILE TOO LARGE"
#define MWFO        "WRITE FILE OPEN"
#define MFNO        "FILE NOT OPEN"
#define MFE         "FILE EXISTS"
#define MFTM        "FILE TYPE MISMATCH"
#define MNB         "NO BLOCK"
#define MITOS       "ILLEGAL TRACK OR SECTOR"
#define MNC         "NO CHANNEL"
#define MDE         "DIR ERROR"
#define MDF         "DISK FULL"
#define MCDV1       "CBM DOS V2.6 1541"
#define MDNR        "DRIVE NOT READY"

char *errs[]={
    MN MO ML,MN MFS ML,M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,
    MN MRE ML,MN MRE ML,MN MRE ML,MN MRE ML,MN MRE ML,MN MWE ML,
    MN MWPO ML,MN MRE ML,MN MWE ML,MN MDIM ML,MN MSE ML,MN MSE ML,MN MSE ML,
    MN MSE ML,MN MSE ML,M,M,M,M,MN MFNF ML,M,M,M,M,M,M,M,M,M,M,
    MN MRNP ML,MN MOIR ML,MN MFTL ML,M,M,M,M,M,M,M,
    MN MWFO ML,MN MFNO ML,MN MFNF ML,MN MFE ML,MN MFTM ML,MN MNB ML,
    MN MITOS ML,MN MITOS ML,M,M,MN MNC ML,MN MDE ML,MN MDF ML,MN MCDV1 ML,MN MDNR,M,
    NULL
};

static int soff[37]={ 0,
    0,21,42,63,84,105,126,147,168,189,210,231,252,273,294,315,336,357,
    376,395,414,433,452,471,490,508,526,544,562,580,598,615,632,649,666,683
};
static char *types[]={ "DEL","SEQ","PRG","USR","REL","???","???","???" };
int snum[37]={ 0,
    21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
    19,19,19,19,19,19,19,
    18,18,18,18,18,18,
    17,17,17,17,17,
    17
};
static struct diskinfo *Ds[12];     // disks (0-7 unused)
static UBYTE ibuf[257];             // block internal buffer + EOB marker
static int ibufnum=0;
static UBYTE buf[256];              // block internal buffer
static UBYTE listenaddr,talkaddr;   // our address (8-11)
static int Ad;                      // act device address (8-11)
static int mode,actsecaddr;         // (talk/listen) | (channel number)
static int inited[12];              // inicializálva?

static void close_file(int channel);


/* debug
 *          kiir egy blokkot
 */
/*
static void print_block(UBYTE *b) {
    int i,j;
    for(i=0;i<16;i++,b+=16) {
        printf("  %02x: ",i*16);
        for(j=0;j<16;j++) printf("%02x ",b[j]);
        printf(" » ");
        for(j=0;j<16;j++) printf("%c",(isprint(b[j])?b[j]:'.'));
        printf("\n");
    }
}
*/

/* beállít egy hibaszámot
 */
static int set_status(struct diskinfo *di,int errnum,int t,int s) {
    int l,ret=0;
    char *st;

    st=errs[errnum];
    if(st[0]!='?') {
        st[0]=(errnum/10)+'0';
        st[1]=(errnum%10)+'0';
        l=strlen(st);
        st[l-6]=(t/10)+'0';
        st[l-5]=(t%10)+'0';
        st[l-3]=(s/10)+'0';
        st[l-2]=(s%10)+'0';
        di->status=errnum;
        di->statustr=t;
        di->statussc=s;
        d64_LED=LED_OFF;
        d64_ST=0;
        if(errnum!=0&&errnum!=1&&errnum!=73) {
            d64_ST=64;
            d64_LED=LED_FLASH;
            ret=-1;
        }
    }
//printf("set_status %d==%s\n",errnum,errs[errnum]);
    return(ret);
} // set_status()

static int set_status0(struct diskinfo *di,int errnum) {
    return(set_status(di,errnum,0,0));
} // set_status0()

static int set_ok(struct diskinfo *di) {
    return(set_status(di,0,0,0));
} // set_ok()

static void reset_status(struct diskinfo *di) {
    int st=d64_ST;
    set_status(di,di->status,di->statustr,di->statussc);
    d64_ST=st;
} // reset_status()

/*
 * az 5 belsõ buffer közül felszabadít egyet
 */
static void free_buffer(int i) {
//printf("free_buffer %d\n",i);
    if(i==FB_INTERNAL) ibufnum=0;           // always free
    if(i<0||i>4) return;
    Ds[Ad]->ibnum[i]=-1;
} // free_buffer()

/*
 * az 5 belsõ buffer közül lefoglal egyet, ha van még
 */
static int alloc_buffer(int i) {
    int ret=-1,j;
    if(i==AB_ANY) {
        for(j=3;j>=0;j--) {
            if(Ds[Ad]->ibnum[j]==-1) {
                ret=j;
                Ds[Ad]->ibnum[j]=2;
                break;
            }
        }
    } else if(i>=0&&i<5) if(Ds[Ad]->ibnum[i]==-1) ret=i;
//printf("alloc_buffer %d --> %d\n",i,ret);
    return(ret);
} // alloc_buffer()

/*
 * az 15 belsõ channel közül felszabadít egyet + a bufferét
 *  esetleges file-okat lezárja!
 */
static void free_channel(int channel) {
//printf("free_channel %d\n",channel);
    Ds[Ad]->chn[channel].busy=CH_FREE;
    if(Ds[Ad]->chn[channel].bufnum!=CH_NONE) {
        if(Ds[Ad]->chn[channel].file.type!=T_NONE) close_file(channel);
        free_buffer(Ds[Ad]->chn[channel].bufnum);
    }
    Ds[Ad]->chn[channel].secmode=SM_NONE;
} // free_channel()

/*
 * az 15 belsõ channel közül lefoglal egyet + egy buffert
 */
static int alloc_channel(int channel) {
//printf("alloc_channel %d\n",channel);
    if(Ds[Ad]->chn[channel].busy!=CH_FREE) free_channel(channel);
    if(channel!=15&&(-1==(Ds[Ad]->chn[channel].bufnum=alloc_buffer(AB_ANY)))) return(-1);
    if(channel==15) Ds[Ad]->chn[channel].bufnum=CH_NONE;
    Ds[Ad]->chn[channel].busy=CH_BUSY;
    Ds[Ad]->chn[channel].file.type=T_NONE;
    return(0);
} // alloc_channel()


/*
 * D64 HANDLING
 */

static int read_block(int t,int s,UBYTE *b) {
//printf("READ_BLOCK %d - %d\n",t,s);
    if(t>35) return(-1);
    if(s>=snum[t]) return(-1);
    fseek(Ds[Ad]->d64,(soff[t]+s)<<8,SEEK_SET);
    fread(b,1,256,Ds[Ad]->d64);
//print_block(b);
    return(0);
} // read_block()

static void read_bam(struct diskinfo *di) {
static UBYTE bf[256];
    int i,j,l;
    unsigned char *b;
    unsigned long ln;

    read_block(18,0,bf);
    di->dt=bf[0];
    di->ds=bf[1];
    di->dtype=bf[2];
    di->flag=bf[3];
    for(i=0;i<16;i++) di->name[i]=(bf[i+144]==0xa0?0:bf[i+144]);
    di->name[16]=0;
    di->id[0]=bf[162];
    di->id[1]=bf[163];
    di->ti=bf[164];
    di->ty[0]=bf[165];
    di->ty[1]=bf[166];
    b=bf+4;
    di->bfree=0;
    for(l=1,j=0;l<36;l++,b+=4) {
        di->freebpt[l]=b[0];
        if(l!=18) di->bfree+=b[0];
        ln=(((unsigned long)b[3])<<16)+(((unsigned long)b[2])<<8)+b[1];
        for(i=0;i<snum[l];i++) di->bam[j++]=(((ln>>i)&1)==0 ? BAM_BUSY : BAM_FREE );
    }
//printf("read_bam '%s'%c%c  %d blocks free\n",di->name,di->id[0],di->id[1],di->bfree);
} // read_bam()

static int write_block(int t,int s,UBYTE *b) {
//printf("WRITE_BLOCK %d - %d\n",t,s);
    if(t>35||t<1) return(-1);
    if(s>snum[t]||s<0) return(-1);
//print_block(b);
    if(Ds[Ad]->writeprotect!=WP_OFF) {
        set_status(Ds[Ad],26,t,s);
        return(-1);
    }
    fseek(Ds[Ad]->d64,(soff[t]+s)<<8,SEEK_SET);
    fwrite(b,1,256,Ds[Ad]->d64);
    return(0);
    return(0);
} // write_block()

static void fill_fileinfo(struct fileinfo *fi,UBYTE *b) {
    int i;
    fi->type=b[0]&7;
    fi->flags=b[0]&~7;
    fi->tr=b[1];
    fi->sc=b[2];
    for(i=0;i<16;i++) fi->name[i]=(b[i+3]==0xa0?0:b[i+3]);
    fi->name[16]=0;
    fi->reltr=b[19];
    fi->relsc=b[20];
    fi->relln=b[21];
    fi->ntr=b[26];
    fi->nsc=b[27];
    fi->blocks=(b[29]<<8)+b[28];
//printf("fill_fileinfo -> '%s' %s (%2d-%2d  %d blocks)\n",fi->name,types[fi->type],fi->tr,fi->sc,fi->blocks);
} // fill_fileinfo()

static UBYTE *delseek_in_dirblock(UBYTE *b) {
    int j;
//printf("delseek_in_dirblock\n");
    for(j=0,b+=2;j<8;j++,b+=32) if(*b==0) return(b);
    return(NULL);
} // delseek_in_dirblock()


/*
 * 0-match, más nem (exact match! csak csillaggal lehet hosszabb)
 * két \0-s végû cucc!!
 */
static int match_pattern_ex(char *pat,char *n) {
    while(*pat!='\0'&&*pat!='*') {
        if(*pat!='?'&&*pat!=*n) return(-1);
        if(*n=='\0') return(-1);
        pat++; n++;
    }
    if(*pat!='*'&&*n!='\0') return(-1);
    return(0);
} // match_pattern_ex()


static UBYTE *fileseek_in_dirblock(char *name,UBYTE *b) {
static char mb[17];
    int i,j;
    UBYTE *rt=NULL;

    b+=2;
    for(j=0;j<8;j++,b+=32) {
        if(*b!=0) {
            for(i=0;i<16;i++) mb[i]=(b[i+3]==0xa0?0:b[i+3]);
            mb[16]=0;
            if(match_pattern_ex(name,mb)==0) {
                rt=b;
                break;
            }
        }
    }
//printf("fileseek_in_dirblock (%s) -> %s\n",name,(rt==NULL?"Not Found":"Ok."));
    return(rt);
} // fileseek_in_dirblock()

static int file_info(char *name, struct fileinfo *fi) {
static UBYTE bf[256];
    UBYTE *r;
    int t,s;

//printf("file_info (%s)\n",name);
    if(Ds[Ad]->bfree==NODISK) read_bam(Ds[Ad]);
    t=Ds[Ad]->dt;
    s=Ds[Ad]->ds;
    read_block(t,s,bf);
    while(NULL==(r=fileseek_in_dirblock(name,bf))) {
        t=bf[0]; s=bf[1];
        if(t==0) return(-1);
        read_block(t,s,bf);
    }
    fill_fileinfo(fi,r);
    fi->dirt=t;
    fi->dirs=s;
    fi->diroffs=r-bf;
    fi->wrtlasttr=-1;
    fi->wrtlastsc=-1;
    return(0);
} // file_info()

static int file_info_modify(struct fileinfo *fi) {
    UBYTE *r;
    int t,s,i;

//printf("file_info_modify\n");
    t=fi->dirt;
    s=fi->dirs;
    read_block(t,s,buf);
    r=&buf[fi->diroffs];
    r[0]=fi->type|fi->flags;
    r[1]=fi->tr;
    r[2]=fi->sc;
    for(i=0;fi->name[i]!='\0';i++) r[3+i]=fi->name[i];
    for(;i<16;i++) r[3+i]=0xa0;
    r[19]=fi->reltr;
    r[20]=fi->relsc;
    r[21]=fi->relln;
    r[26]=fi->ntr;
    r[27]=fi->nsc;
    r[28]=(fi->blocks&0xff);
    r[29]=(fi->blocks>>8)&0xff;
    return(write_block(t,s,buf));
} // file_info()

static void write_bam(struct diskinfo *di) {
static UBYTE bf[256];
    int i,j,l;
    unsigned char *b;
    unsigned long ln;

//printf("write_bam\n");
    bf[0]=di->dt;
    bf[1]=di->ds;
    bf[2]=di->dtype;
    bf[3]=di->flag;
    for(i=0;di->name[i]!='\0';i++) bf[i+144]=di->name[i];
    for(;i<16;i++) bf[i+144]=0xa0;
    bf[161]=bf[160]=0xa0;
    bf[162]=di->id[0];
    bf[163]=di->id[1];
    bf[164]=di->ti;
    bf[165]=di->ty[0];
    bf[166]=di->ty[1];
    bf[167]=bf[168]=bf[169]=bf[170]=0xa0;
    b=bf+4;
    for(l=1,j=0;l<36;l++,b+=4) {
        b[0]=di->freebpt[l];
        ln=0;
        for(i=0;i<snum[l];i++) ln|=((di->bam[j++]==BAM_FREE?1:0)<<i);
        b[1]=ln&0xff;
        b[2]=(ln>>8)&0xff;
        b[3]=(ln>>16)&0xff;
    }
    write_block(18,0,bf);
} // write_bam()


/* blk törl
 */
static void free_block(int t,int s) {
//printf("free_block\n");
    if(Ds[Ad]->bfree==NODISK) read_bam(Ds[Ad]);
    if(Ds[Ad]->bam[soff[t]+s]==BAM_BUSY) {
        Ds[Ad]->bam[soff[t]+s]=BAM_FREE;
        Ds[Ad]->bfree++;
        Ds[Ad]->freebpt[t]++;
    }
} // free_block()

/*
 * lefoglal egy blokkot, bejegyzi a bam-ba, ha foglalt már -1
 */
static int alloc_block_fix(int t,int s) {
    int bn,r=-1;
    if(t<0||t>35||s<0||s>snum[t]) return(-1);
    if(Ds[Ad]->bfree==NODISK) read_bam(Ds[Ad]);
    if(Ds[Ad]->writeprotect==WP_ON) {
        set_status(Ds[Ad],26,t,s);
        return(-1);
    }
//printf("alloc_block_fix..");
    bn=soff[t]+s;
    if(Ds[Ad]->bam[bn]!=BAM_BUSY) {
//printf("OK\n");
        Ds[Ad]->bfree--;
        Ds[Ad]->bam[bn]=BAM_BUSY;
        Ds[Ad]->freebpt[t]--;
        r=0;
    } else {
//printf("FAILED\n");
    }
    return(r);
} // alloc_block_fix()


/* keres egy üres sectort a bam-ban 't' tracken, 'rs'-tõl felfelé
 * ha nincs -1
 */
int search_empty_sector(struct diskinfo *di,int t,int rs) {
    int s=-1,i,sf;
    for(sf=soff[t],i=rs;s==-1&&i<=snum[t];i++) if(di->bam[sf+i]!=BAM_BUSY) s=i;
    return(s);
} // search_empty_sector()

/*
 * lefoglal egy blokkot, bejegyzi a bam-ba, ha nincs -1
 * elõször rt,rs helyet nézi
 */
static int alloc_block(int *t,int *s,int rt,int rs) {
    int ret=-1,i,flag;

//printf("alloc_block\n");
    if(Ds[Ad]->bfree==NODISK) read_bam(Ds[Ad]);
    if(Ds[Ad]->bfree==0) {
        set_status0(Ds[Ad],72);
        return(-1);
    }
    if(rt==0) {                                 // Keresés a semmibõl
        for(i=1,flag=0;ret<0&&i<18;i+=flag,flag^=1) {
            rt=18+(flag?i:-i);
            if(Ds[Ad]->freebpt[rt]>0) {
                ret=*s=search_empty_sector(Ds[Ad],rt,0);
                *t=rt;
            }
        }
        if(ret<0) set_status(Ds[Ad],71,rt,0);
    } else {                                    // Keresés javaslatból
        flag=0;
        while(rt!=0) {
            if(Ds[Ad]->freebpt[rt]>0) {
                rs+=10;
                if(rs>=snum[rt]) rs-=snum[rt];
                if(rs!=0) rs--;
                *t=rt;
                ret=*s=search_empty_sector(Ds[Ad],rt,rs);
                if(ret<0) ret=*s=search_empty_sector(Ds[Ad],rt,0);
                if(ret<0) set_status(Ds[Ad],71,rt,0);
                rt=0;
            } else if(rt<18&&--rt==0&&flag==0) {
                rt=18+1; rs=0; flag=1;
            } else if(rt>18&&++rt<36&&flag==0) {
                rt=18-1; rs=0; flag=1;
            } else if(rt==18) {
                set_status0(Ds[Ad],72);
                rt=0;
            }
        }
    }
    if(ret>=0) {
        alloc_block_fix(*t,*s);
        ret=0;
    }
    return(ret);
} // alloc_block()

/*
 * lefoglal egy dir entry-t, ha tud (NULL egyébként)
 */
static UBYTE *alloc_direntry(UBYTE *b,int *tr,int *sc) {
    UBYTE *db=NULL;
    int t,s,i,lt,ls;

//printf("alloc_direntry\n");
    if(Ds[Ad]->bfree==NODISK) read_bam(Ds[Ad]);
    t=Ds[Ad]->dt;
    s=Ds[Ad]->ds;
    do {
        read_block(t,s,b);
        lt=t;
        ls=s;
        t=b[0];
        s=b[1];
    } while(NULL==(db=delseek_in_dirblock(b))&&t!=0);
    if(db==NULL) {
        if(-1!=alloc_block(&t,&s,18,ls)) {
            b[0]=18;
            b[1]=s;
            if(0==(write_block(lt,ls,b))) {
                b[0]=0;
                b[1]=0xff;
                for(i=2;i<256;i++) b[i]=0;
                if(0==(write_block(18,s,b))) {
                    db=&b[2];
                    *tr=18;
                    *sc=s;
                }
            }
        }
    } else {
        *tr=lt;
        *sc=ls;
    }
    return(db);
} // alloc_direntry()

/*
 * 0-match, más nem
 */
static int match_pattern(UBYTE *pat,UBYTE *n) {
    while(*pat!='\0'&&*pat!='*') {
        if(*pat!='?') if(*pat!=*n) return(-1);
        if(*n==0xa0) return(-1);
        pat++; n++;
    }
    return(0);
} // match_pattern()

/*
 * (mode==0 elsõ lesz, 1: sokadik block)
 * kitölti a bf buffert a directory-val (2. tól!)
 * pattern=pattern, first, NULL, sokadik
 */
static void fill_next_dir(UBYTE *bf,char *pattern) {
static char efix[]="BLOCKS FREE.";
static UBYTE fix[]={1,4,1,1,0,0,0x12,34};
static UBYTE store[4680];                           // 4672==144*32 + 2*32
static UBYTE *p,*now;
static int len;
    UBYTE *d;
    int i,t,s,nb,ty,j;

    if(pattern!=NULL) {
        if(Ds[Ad]->bfree==NODISK) read_bam(Ds[Ad]);
        p=store;
        for(i=0;i<8;i++) *p++=fix[i];
        for(i=0;Ds[Ad]->name[i]!='\0';i++) *p++=Ds[Ad]->name[i];
        for(;i<16;i++) *p++=' ';
        *p++=34;
        *p++=Ds[Ad]->id[0]; *p++=Ds[Ad]->id[1]; *p++=(Ds[Ad]->ti==0xa0?' ':Ds[Ad]->ti); *p++=Ds[Ad]->ty[0]; *p++=Ds[Ad]->ty[1];
        *p++=0;
        t=Ds[Ad]->dt;
        s=Ds[Ad]->ds;
        len=1;
        while(t!=0&&++len<20) {
            read_block(t,s,ibuf);
            t=ibuf[0];
            s=ibuf[1];
            for(i=0,d=ibuf+2;i<8;i++,d+=32) {
                if(*d&&0==match_pattern((UBYTE *)pattern,&d[3])) {
                    *p++=1; *p++=1;
                    *p++=d[28];
                    *p++=d[29];
                    *p++=' ';
                    nb=(d[29]<<8)+d[28];
                    if(nb<10) *p++=' ';
                    if(nb<100) *p++=' ';
                    *p++=34;
                    for(j=0;d[3+j]!=0xa0&&j<16;j++) *p++=d[j+3];
                    *p++=34;
                    for(;j<16;j++) *p++=' ';
                    if(*d&F_OPEN) *p++=' ';
                    else *p++='*';
                    ty=*d&15;
                    *p++=types[ty][0];
                    *p++=types[ty][1];
                    *p++=types[ty][2];
                    if(*d&0x40) *p++='<';
                    else *p++=' ';
                    *p++=' ';
                    if(nb<10) *p++=' ';
                    if(nb<100) *p++=' ';
                    *p++=0;
                }
            }
        }
        d=p;
        for(i=0;i<29;i++) *d++=' ';
        *p++=1; *p++=1;
        *p++=Ds[Ad]->bfree&0xff;
        *p++=(Ds[Ad]->bfree>>8)&0xff;
        for(i=0;efix[i]!='\0';i++) *p++=efix[i];
        p=d;
        *p++=0; *p++=0; *p++=0;
        len=p-store;
        now=store;
    }
    if(now<p) {
        d=bf; bf+=2;
        for(i=0;now<p&&i<254;i++) *bf++=*now++;
        if(now>=p) {
            *d++=0;
            *d=(UBYTE)i;
        } else *d=18;
    }
} // fill_next_dir()

/*
 * megnyit egy file-t, ha nem létezik hiba      OPEN READ
 */
static int open_file(int c) {
static char name[17];
    int i,bn,len;
    UBYTE *b;
    struct fileinfo *fi;

//printf("open_file\n");
    d64_LED=LED_ON;
    len=Ds[Ad]->chn[c].filenamelen;
    b=Ds[Ad]->chn[c].filename;
    fi=&Ds[Ad]->chn[c].file;
    bn=Ds[Ad]->chn[c].bufnum;
    fi->wrtlasttr=-1;
    fi->wrtlastsc=-1;
    fi->ntr=-1;
    fi->nsc=-1;
    fi->mode=FM_READ;
    Ds[Ad]->ibnum[bn]=2;
    if(b[0]=='$') {
//printf("  DIR\n");
        fi->type=T_DIR;
        if(Ds[Ad]->bfree==NODISK) read_bam(Ds[Ad]);
        fi->tr=Ds[Ad]->dt;
        fi->sc=Ds[Ad]->ds;
        Ds[Ad]->ib[bn][0]=fi->tr;
        Ds[Ad]->ib[bn][1]=fi->sc;
        fill_next_dir(Ds[Ad]->ib[bn],&b[1]);
        Ds[Ad]->opencounter++;
        return(0);
    }
    if(b[0]=='#') {
//printf("  RND\n");
        fi->type=T_RND;
        fi->tr=-1;
        fi->sc=-1;
        Ds[Ad]->ibnum[bn]=0;
        Ds[Ad]->opencounter++;
        return(0);
    }
    if(b[0]=='0'&&b[1]==':') { b+=2; len-=2; }
    for(i=0;i<len;i++) name[i]=b[i];
    name[i]='\0';
    for(i=0;name[i]!='\0';i++) Ds[Ad]->chn[c].filename[i]=name[i];
    Ds[Ad]->chn[c].filename[i]='\0';
    Ds[Ad]->chn[c].filenamelen=i-1;
//printf("  name: '%s'\n",name);
    if(0==(file_info(name,fi))) {
        read_block(fi->tr,fi->sc,Ds[Ad]->ib[bn]);
    } else {
        set_status0(Ds[Ad],62);
        return(-1);
    }
    Ds[Ad]->opencounter++;
    return(0);
} // open_file()

/*
 * megnyit egy file-t írásra, ha létezik hiba       OPEN WRITE
 */
static int open_file_wrt(int c) {
static char name[17];
    struct fileinfo *fi;
    int over=0,bn;
    UBYTE *b;
    int i,t,s,len,ftype;

//printf("open_file_wrt\n");
    d64_LED=LED_ON;
    len=Ds[Ad]->chn[c].filenamelen;
    b=Ds[Ad]->chn[c].filename;
    fi=&Ds[Ad]->chn[c].file;
    bn=Ds[Ad]->chn[c].bufnum;
    Ds[Ad]->ibnum[bn]=2;
    fi->type=T_NONE;
    fi->mode=FM_WRITE;
    if(*b=='@') {
        if(b[1]!=':') {
            set_status0(Ds[Ad],34);
            return(-1);
        }
        over=1;
        b+=2;
    }
    if(*b=='#') {
        fi->type=T_RND;
        fi->tr=-1;
        fi->sc=-1;
        Ds[Ad]->ibnum[bn]=0;
        Ds[Ad]->opencounter++;
        return(0);
    }
    for(i=0;i<len;i++) name[i]=b[i];
    name[i]='\0';
    if(c<2) ftype=T_PRG;
    else {
        set_status0(Ds[Ad],30);
        return(-1);
    }
/*
    else {                  // nem 0-1 channel, need explicit filetype
        if(name[len-2]==','&&name[len-4]==',') {
            if(name[len-1]=='W') {
                // write opened
            } else if(name[len-1]=='A') {
                // append opened
            } else {
                set_status0(Ds[Ad],30);
                return(-1);
            }
        } else {
            set_status0(Ds[Ad],30);
            return(-1);
        }
    }
*/
    if(0==(file_info(name,fi))&&over==0) {
        set_status0(Ds[Ad],63);
        return(-1);
    } else {
        if(fi->type!=T_NONE) {      // overwrite
            fi->flags&=~F_OPEN;
            fi->wrtlasttr=-1;
            fi->wrtlastsc=-1;
            fi->ntr=-1;
            fi->nsc=-1;
            fi->blocks=0;
            if(0!=(file_info_modify(fi))) free_channel(c);
        } else {                    // new file
            b=alloc_direntry(buf,&t,&s);
            if(b!=NULL) {
                fi->type=ftype;
                fi->flags=0;
                fi->tr=0;
                fi->sc=0;
                for(i=0;name[i]!='\0';i++) fi->name[i]=name[i];
                fi->name[i]=0;
                fi->ntr=-1;
                fi->nsc=-1;
                fi->blocks=0;
                fi->reltr=fi->relsc=fi->relln=0;
                fi->dirt=t;
                fi->dirs=s;
                fi->diroffs=b-buf;
                fi->wrtlasttr=-1;
                fi->wrtlastsc=-1;
                if(0!=(file_info_modify(fi))) free_channel(c);
            } else {
                set_status0(Ds[Ad],72);
                return(-1);
            }
        }
    }
    Ds[Ad]->opencounter++;
    return(0);
} // open_file_wrt()


/*
 * flush pending buffer
 */
static int flush_buffer(int bn,struct fileinfo *fi) {
    int t,s,rt=0,rs=0,ret=0;

//printf("flush_buffer\n");
    if(Ds[Ad]->ibnum[bn]>2&&fi->type!=T_RND&&fi->type!=T_NONE&&fi->type!=T_DIR) {
        if(fi->wrtlasttr>0) { rt=fi->wrtlasttr; rs=fi->wrtlastsc; }
        else if(fi->ntr>0) { rt=fi->ntr; rs=fi->nsc; }
        else if(fi->tr>0) { rt=fi->tr; rs=fi->sc; }
        if(-1!=(alloc_block(&t,&s,rt,rs))) {
            Ds[Ad]->ib[bn][0]=0;
            Ds[Ad]->ib[bn][1]=Ds[Ad]->ibnum[bn]-1;
            if(0==(write_block(t,s,Ds[Ad]->ib[bn]))) {
                if(fi->wrtlasttr!=-1) {
                    read_block(fi->wrtlasttr,fi->wrtlastsc,buf);
                    buf[0]=t;
                    buf[1]=s;
                    if(0!=(write_block(fi->wrtlasttr,fi->wrtlastsc,buf))) ret=-1;
                } else {
                    fi->ntr=t;
                    fi->nsc=s;
                }
                fi->wrtlasttr=t;
                fi->wrtlastsc=s;
                fi->blocks++;
                Ds[Ad]->ibnum[bn]=2;
            } else ret=-1;
        }
    } else if(fi->type==T_RND) Ds[Ad]->ibnum[bn]=0;
    return(ret);
} // flush_buffer()

/*
 * letöröl egy láncot
 */
static void delete_chain(int t,int s,UBYTE *bf) {
//printf("delete_chain\n");
    while(t!=0) {
//printf("   %2d-%d\n",t,s);
        read_block(t,s,bf);
        free_block(t,s);
        t=bf[0];
        s=bf[1];
    }
} // delete_chain()

/*
 * lezár egy file-t, ha írták az utolsó buffert üríti
 */
static void close_file(int channel) {
    struct fileinfo *fi;
    int bn;

//printf("close_file\n");
    if(--Ds[Ad]->opencounter==0) d64_LED=LED_OFF;
    fi=&Ds[Ad]->chn[channel].file;
    if(fi->mode==FM_WRITE&&fi->type!=T_RND&&fi->type!=T_NONE&&fi->type!=T_DIR) {
        bn=Ds[Ad]->chn[channel].bufnum;
        if(bn!=-1&&Ds[Ad]->ibnum[bn]>2) flush_buffer(bn,fi);
        if(fi->tr>0) delete_chain(fi->tr,fi->sc,Ds[Ad]->ib[bn]);
        fi->tr=fi->ntr;
        fi->sc=fi->nsc;
        fi->ntr=0;
        fi->nsc=0;
        fi->flags|=F_OPEN;
        file_info_modify(fi);
        write_bam(Ds[Ad]);
    }
} // close_file()

/*
 * Beolvas egy blokkot egy file-ból vagy dir-bõl
 */
static void read_from_file(int channel) {
    int bn;
    bn=Ds[Ad]->chn[channel].bufnum;
    Ds[Ad]->ibnum[bn]=2;
    if(Ds[Ad]->chn[channel].file.type==T_DIR) fill_next_dir(Ds[Ad]->ib[bn],NULL);
    else if(Ds[Ad]->chn[channel].file.type!=T_RND) read_block(Ds[Ad]->ib[bn][0],Ds[Ad]->ib[bn][1],Ds[Ad]->ib[bn]);
    else Ds[Ad]->ibnum[bn]=0;
} // read_from_file()


#define SKIPSPC(s) while(*s==' ') s++

/*
 * összehasonlít két \0s stringet
 * ha OK, a második végénél tér vissza (NULL különben)
 * utolsó utáni char
 */
static char *cmp_comm_name(char *u,char *com) {
    char *ret=NULL;
//printf("cmp: '%s' ? '%s'\n",u,com);
    while(*com==*u&&*com!='\0') { com++; u++; }
    if(*u!='\0') set_status0(Ds[Ad],31);
    else ret=com;
//printf("cmp: '%s' (%s)\n",com,(ret==NULL?"fail":"ok"));
    return(ret);
} // cmp_comm_name()

/*
 * beolvas egy int paramétert + update char *
 */
static char *getparam(char *cm,int *i) {
    char *o,oc;
    while(*cm==' '||*cm==',') cm++;
    o=cm;
    while(*o!=' '&&*o!='\0'&&*o!=',') o++;
    oc=*o;
    *o='\0';
    *i=atoi(cm);
    *o=oc;
    return(o);
} // getparam()


void do_new(UBYTE *com) {
static char ut[]="EW";
static char nm[17];         // lemeznév
static char id[2];          // id
    int t,s,i;

//printf("new: '%s'\n",com);
    if(*com=='0') com++;
    if(*com!=':'&&(NULL==(com=cmp_comm_name(ut,com)))) return;
    if(*com!=':') {
        set_status0(Ds[Ad],31);
        return;
    }
    com++;
    SKIPSPC(com);
    for(i=0;*com!=','&&*com!='\0'&&i<17;i++) nm[i]=*com++;
    nm[i]='\0';
    if(*com!=',') {
        set_status0(Ds[Ad],31);
        return;
    }
    id[0]=*++com;
    id[1]=*++com;
    if(id[0]==0||id[1]==0) {
        set_status0(Ds[Ad],31);
        return;
    }
    for(t=1;t<36;t++) for(s=0;s<snum[t];s++) free_block(t,s);
    for(i=1;i<256;i++) ibuf[i]=1;
    ibuf[0]='K';
    for(t=1;t<36;t++) for(s=0;s<snum[t];s++) write_block(t,s,ibuf);
    alloc_block_fix(18,0);
    alloc_block_fix(18,1);
    for(i=0;nm[i]!='\0';i++) Ds[Ad]->name[i]=nm[i];
    Ds[Ad]->name[i]='\0';
    Ds[Ad]->dt=18;
    Ds[Ad]->ds=1;
    Ds[Ad]->id[0]=id[0];
    Ds[Ad]->id[1]=id[1];
    Ds[Ad]->ti=0xa0;
    Ds[Ad]->ty[0]='2';
    Ds[Ad]->ty[1]='A';
    write_bam(Ds[Ad]);
    for(i=0;i<256;i++) ibuf[i]=0;
    ibuf[0]=0;
    ibuf[1]=0xff;
    write_block(18,1,ibuf);
    read_bam(Ds[Ad]);
    set_ok(Ds[Ad]);
} // do_new()

void do_initialize(UBYTE *com) {
static char ut[]="NITIALISE";
    if(*com!='\0'&&(NULL==(com=cmp_comm_name(ut,com)))) return;
    if(*com!='\0') {
        set_status0(Ds[Ad],31);
        return;
    }
    read_bam(Ds[Ad]);
    set_ok(Ds[Ad]);
} // do_initialize()

void do_scratch(UBYTE *com) {
static char ut[]="CRATCH";
static char bf[40];
    struct fileinfo *fi;
    char *u;
    int numdel=0;

//printf("scratch: '%s'\n",com);
    fi=&Ds[Ad]->chn[15].file;
    if(*com=='0') com++;
    if(*com!=':'&&(NULL==(com=cmp_comm_name(ut,com)))) return;
    if(*com!=':') {
        set_status0(Ds[Ad],31);
        return;
    }
    while(*com!='\0') {
        com++;
        SKIPSPC(com);
        for(u=bf;*com!=','&&*com!='\0';) *u++=*com++;
        *u='\0';
//printf("  >'%s'\n",bf);
        while(0==(file_info(bf,fi))) {
            numdel++;
            delete_chain(fi->tr,fi->sc,ibuf);
            fi->type=fi->flags=0;
            file_info_modify(fi);
            write_bam(Ds[Ad]);
        }
    }
    if(numdel>0) set_status(Ds[Ad],1,numdel,0);
    else set_status0(Ds[Ad],62);
} // do_scratch()

void do_rename(UBYTE *com) {
static char ut[]="ENAME";
static char new[40];
static char old[40];
    struct fileinfo *fi;
    char *u;

    fi=&Ds[Ad]->chn[15].file;
    if(*com=='0') com++;
    if(*com!=':'&&(NULL==(com=cmp_comm_name(ut,com)))) return;
    if(*com!=':') {
        set_status0(Ds[Ad],31);
        return;
    }
    com++;
    SKIPSPC(com);
    for(u=new;*com!='='&&*com!='\0';) *u++=*com++;
    *u='\0';
    if(*com!='=') {
        set_status0(Ds[Ad],31);
        return;
    }
    com++;
    SKIPSPC(com);
    for(u=old;*com!='\0';) *u++=*com++;
    *u='\0';
    if(0!=(file_info(old,fi))) {
        set_status0(Ds[Ad],62);
        return;
    }
    strcpy(fi->name,new);
    file_info_modify(fi);
} // do_rename()

static void do_copy(UBYTE *com) {
} // do_copy()

static int make_chain_valid(int t,int s) {
static UBYTE bf[256];
    int store_t,store_s,ret=0;
    store_t=t;
    store_s=s;
    while(t>0&&t<36&&s>=0&&s<=snum[t]) {
        alloc_block_fix(t,s);
        read_block(t,s,bf);
        t=bf[0];
        s=bf[1];
    }
    if(t!=0) {
        for(t=store_t,s=store_s;t>0&&t<36&&s>=0&&s<=snum[t];t=bf[0],s=bf[1]) {
            free_block(t,s);
            read_block(t,s,bf);
        }
        ret=-1;
    }
    return(ret);
} // make_chain_valid()

static void do_validate(UBYTE *com) {
static char ut[]="ALIDATE";
    int t,s,i,tu,su,flag,d;
    UBYTE *db;

    if(*com=='0') com++;
    if(*com!='\0'&&(NULL==(com=cmp_comm_name(ut,com)))) return;
    if(*com!='\0') {
        set_status0(Ds[Ad],31);
        return;
    }
//printf("validate\n");
    for(t=1;t<36;t++) if(t!=18) for(s=0;s<snum[t];s++) free_block(t,s);
    tu=18;
    su=1;
    for(t=Ds[Ad]->dt,s=Ds[Ad]->ds;t>0&&t<36&&s>=0&&s<=snum[t];t=ibuf[0],s=ibuf[1]) {
        tu=t; su=s;
        read_block(t,s,ibuf);
        db=ibuf+2;
        for(flag=0,i=0;i<8;i++,db+=32) {
            d=*db;
            d&=0x87;
            if(d>=0x80&&d<=0x84) {
                if(0!=(make_chain_valid(db[1],db[2]))) { *db=0; flag=1; }
            } else if(*db!=0) { *db=0; flag=1; }
        }
        if(flag!=0) write_block(t,s,ibuf);
    }
    if(t!=0) {
        ibuf[0]=0;
        ibuf[1]=0xff;
        write_block(tu,su,ibuf);
    }
    write_bam(Ds[Ad]);
    set_ok(Ds[Ad]);
} // do_validate()

/*
 * p=-1 write
 */
static int com_block_rw(char *com,int p) {
    int ch,dr,t,s,ret=-1;
    com=getparam(com,&ch);
    com=getparam(com,&dr);
    if(dr==0) {
        if(Ds[Ad]->chn[ch].secmode==SM_FILEOPEN) {
            open_file(ch);
            Ds[Ad]->chn[actsecaddr].secmode=SM_READFROMD64;
        }
        if(Ds[Ad]->chn[ch].file.type==T_RND) {
            com=getparam(com,&t);
            com=getparam(com,&s);
            if(t>0&&t<36&&s>=0&&s<snum[t]) {
                if(p>=0) {
                    ret=read_block(t,s,Ds[Ad]->ib[Ds[Ad]->chn[ch].bufnum]);
                    Ds[Ad]->ibnum[Ds[Ad]->chn[ch].bufnum]=p;
                } else if(p==-1) {
                    ret=write_block(t,s,Ds[Ad]->ib[Ds[Ad]->chn[ch].bufnum]);
                } else {
                    Ds[Ad]->ib[Ds[Ad]->chn[ch].bufnum][0]=Ds[Ad]->chn[ch].bufnum;
                    ret=write_block(t,s,Ds[Ad]->ib[Ds[Ad]->chn[ch].bufnum]);
                }
            } else set_status(Ds[Ad],66,t,s);
        } else set_status0(Ds[Ad],64);
    } else set_status0(Ds[Ad],74);
    return(ret);
} // com_block_rw()


static void do_block(UBYTE *com) {
    int t,s,dr,ch,pos;

    if(*com++!='-') {
        set_status0(Ds[Ad],31);
        return;
    }
    switch(*com++) {
        case 'A' :
            com=getparam(com,&dr);
            if(dr==0) {
                com=getparam(com,&t);
                com=getparam(com,&s);
                if(t>0&&t<36&&s>=0&&s<=snum[t]) {
                    if(0!=(alloc_block_fix(t,s))) set_status(Ds[Ad],65,t,s);
                } else set_status(Ds[Ad],66,t,s);
            } else set_status0(Ds[Ad],74);
            break;
        case 'P' :
            com=getparam(com,&ch);
            if(Ds[Ad]->chn[ch].file.type==T_RND) {
                getparam(com,&pos);
                if(pos>=0&&pos<256) Ds[Ad]->ibnum[Ds[Ad]->chn[ch].bufnum]=pos;
            } else set_status0(Ds[Ad],64);
            break;
        case 'W' :
            com_block_rw(com,-2);
            break;
        case 'R' :
            com_block_rw(com,1);
            break;
        case 'F' :
            com=getparam(com,&dr);
            if(dr==0) {
                com=getparam(com,&t);
                com=getparam(com,&s);
                if(t>0&&t<36&&s>=0&&s<=snum[t]) free_block(t,s);
                else set_status(Ds[Ad],66,t,s);
            } else set_status0(Ds[Ad],74);
            break;
        default :
            set_status0(Ds[Ad],31);
            break;
    }
} // do_block()

static void do_memory(UBYTE *com) {
} // do_memory()

static void do_user(UBYTE *com) {
    int dad;
    switch(*com++) {
        case 'A' :
        case '1' :                  // b-r
            com_block_rw(com,0);
            break;
        case 'B' :
        case '2' :                  // b-w
            com_block_rw(com,-1);
            break;
        case 'J' :
        case ':' :                  // reset
            dad=8;
            if(Ad>=8&&Ad<=11) dad=Ad;
            SOFT_Reset(dad);
            break;
        default :
            set_status0(Ds[Ad],30);
            break;
    }
} // do_user()

/*
 * végrehajt egy parancsot
 */
static void execute_comm(UBYTE *com) {
//printf("execute_comm\n");
    if(strlen(com)>40) {
        set_status0(Ds[Ad],32);
        return;
    }
    switch(com[0]) {
        case 'N' :                  // New
            do_new(&com[1]);
            break;
        case 'I' :                  // Initialize
            do_initialize(&com[1]);
            break;
        case 'S' :                  // Scratch
            do_scratch(&com[1]);
            break;
        case 'R' :                  // Rename
            do_rename(&com[1]);
            break;
        case 'C' :                  // Copy
            do_copy(&com[1]);
            break;
        case 'V' :                  // Validate
            do_validate(&com[1]);
            break;
        case 'B' :                  // Block - ?
            do_block(&com[1]);
            break;
        case 'M' :                  // Memory - ?
            do_memory(&com[1]);
            break;
        case 'U' :                  // User ?
            do_user(&com[1]);
            break;
        default :
            set_status0(Ds[Ad],30);
            break;
    }
} // execute_comm()

/*
 * INTERFACE functions
 */

/* ret==0 oké
 *     -1 nincs megnyitva
 */
int SOFT_State(REG(d0,char drive)) {
    if(inited[(int)drive]==0) return(-1);
    return(0);
}

/* Init d64
 */
int SOFT_Init(REG(d0,char devaddr)) {
static char optname[]="D64NAME___";
static char opt2name[]="DRIVE__";
    char *d64name;
    struct FileInfoBlock *fib;
    BPTR bp;
    int ret=0;
    char acc[5];

    if(devaddr<8||devaddr>11) return(SERR_ILLDEV);          // 8-11
    if(inited[(int)devaddr]!=0) return(0);
//printf("SOFT_Init\n");
    optname[8]=(devaddr/10)+'0';
    optname[9]=(devaddr%10)+'0';
    d64name=GetOption(optname,"NONE");
    if(strcmp(d64name,"NONE")!=0) {
        Ad=devaddr;
        if(NULL==(Ds[Ad]=calloc(sizeof(struct diskinfo),1))) return(SERR_NOMEM);
        if(NULL==(fib=AllocVec(sizeof(struct FileInfoBlock),MEMF_CLEAR))) return(SERR_NOMEM);
        inited[Ad]=1;
        Ds[Ad]->bfree=NODISK;
        bp=Lock(d64name,ACCESS_READ);
        if(bp!=0) {
            Examine(bp,fib);
            if((fib->fib_Protection&4)==0) {
                strcpy(acc,"rb+");
                Ds[Ad]->writeprotect=WP_OFF;
            } else {
                strcpy(acc,"rb");
                Ds[Ad]->writeprotect=WP_ON;
            }
            UnLock(bp);
            if(NULL!=(Ds[Ad]->d64=fopen(d64name,"rb"))) {
                fseek(Ds[Ad]->d64,0,SEEK_END);
                if(ftell(Ds[Ad]->d64)==174848) {
                    fseek(Ds[Ad]->d64,0,SEEK_SET);
                    fclose(Ds[Ad]->d64);
                    if(NULL!=(Ds[Ad]->d64=fopen(d64name,acc))) {
//printf("  inited:'%s' (%d)\n",d64name,Ad);
                        SOFT_Reset(Ad);
                    } else {
                        ret=SERR_REOPEN;
                        p4req1(NULL,P4_ERROR,GetStr(MSG_01DC));
                        SOFT_Close(Ad);
                    }
                } else {
                    ret=SERR_WRONGD64;
                    p4req1(NULL,P4_ERROR,GetStr(MSG_0267));
                    SOFT_Close(devaddr);
                }
            } else {
                ret=SERR_NOTFOUND;
                p4req1(NULL,P4_ERROR,GetStr(MSG_0268));
                SOFT_Close(devaddr);
            }
        } else {
            ret=SERR_NOTFOUND;
            p4req1(NULL,P4_ERROR,GetStr(MSG_0268));
            SOFT_Close(devaddr);
        }
        FreeVec(fib);
    } else {
        opt2name[5]=(devaddr/10)+'0';
        opt2name[6]=(devaddr%10)+'0';
        AddOption(opt2name,"NONE");
        SOFT_Close(devaddr);
    }
    return(ret);
} // SOFT_Init()

/* Internal close
 */
static void SOFT_I_Close(int devaddr) {
    if(inited[devaddr]==0) return;
    inited[devaddr]=0;
    if(Ds[devaddr]!=NULL) {
        if(Ds[devaddr]->d64!=NULL) fclose(Ds[devaddr]->d64);
        free(Ds[devaddr]);
    }
    Ds[devaddr]=NULL;
//printf("  i_closed %d\n",devaddr);
} // SOFT_I_Close()

/* Close d64
 */
void SOFT_Close(REG(d0,char devaddr)) {
    int i;
//printf("SOFT_Close (%d)\n",devaddr);
    if(devaddr==CLOSE_ALL) {
        for(i=8;i<12;i++) SOFT_I_Close(i);
        return;
    }
    if(devaddr<8||devaddr>11) return;       // 8-11
    SOFT_I_Close(devaddr);
} // SOFT_Close()

void REGARGS SOFT_Reset(REG(d0,char devaddr)) {
    int i;
    if(devaddr<8||devaddr>11) return;               // 8-11
//printf("SOFT_Reset\n");
    actsecaddr=-1;
    Ad=devaddr;
    Ds[Ad]->opencounter=0;
    mode=IM_NONE;
    free_buffer(FB_INTERNAL);
    for(i=0;i<5;i++) free_buffer(i);
    for(i=0;i<16;i++) Ds[Ad]->chn[i].busy=CH_FREE;
    read_bam(Ds[Ad]);
    set_status0(Ds[Ad],73);
    d64_ST=0;
    d64_LED=LED_OFF;
} // SOFT_Reset()

/*
 * IEC functions
 */
void REGARGS SOFT_Listen(REG(d0,char devaddr)) {        // ***LISTEN***
    if(devaddr<8||devaddr>11) return;               // 8-11
    if(inited[(int)devaddr]==0) SOFT_Init(devaddr);
//printf("SOFT_Listen (%d)\n",devaddr);
    d64_ST=0;
    Ad=listenaddr=devaddr;                  // 8-11
    mode=IM_LISTEN;
    actsecaddr=-1;
}
void REGARGS SOFT_Talk(REG(d0,char devaddr)) {          // ***TALK***
    if(devaddr<8||devaddr>11) return;               // 8-11
    if(inited[(int)devaddr]==0) SOFT_Init(devaddr);
//printf("SOFT_Talk (%d)\n",devaddr);
    d64_ST=0;
    Ad=talkaddr=devaddr;                    // 8-11
    mode=IM_TALK;
    actsecaddr=-1;
}
int SOFT_ACPtr(void) {                                  // ***IN***
    int bn,ret=0;
    if(inited[Ad]==0) return(-1);
//printf("SOFT_ACPtr\n");
    d64_ST=0;
    if(actsecaddr!=-1&&actsecaddr!=15&&Ds[Ad]->chn[actsecaddr].secmode==SM_READFROMD64) {
        bn=Ds[Ad]->chn[actsecaddr].bufnum;
        if(Ds[Ad]->ib[bn][0]!=0&&Ds[Ad]->ibnum[bn]>255) {
//printf("\n -> ");
            read_from_file(actsecaddr);
        }
        if(Ds[Ad]->ib[bn][0]==0&&Ds[Ad]->ibnum[bn]>Ds[Ad]->ib[bn][1]) {
            set_ok(Ds[Ad]);
            d64_ST=64;
        }
        ret=Ds[Ad]->ib[bn][Ds[Ad]->ibnum[bn]++];
//printf("  (%x) '%c'",ret,(isprint(ret)?ret:'.'));
    } else if(actsecaddr==15&&Ds[Ad]->numstatus>=0) {
        ret=errs[Ds[Ad]->status][Ds[Ad]->numstatus++];
        if(ret==015) {
            d64_ST=64;
            Ds[Ad]->numstatus=-1;
            set_ok(Ds[Ad]);
        }
//printf("  %c (status)",(isprint(ret)?ret:'^'));
    } else d64_ST=64;
//printf("\n");
    return(ret);
}
void REGARGS SOFT_CIOut(REG(d0,unsigned char chr)) {    // ***OUT***
    int sm,bn;
    if(inited[Ad]==0) return;
//printf("SOFT_CIOut (%x) (%c)\n",chr,(isprint(chr)?chr:'.'));
    d64_ST=0;
    if(mode==IM_LISTEN&&actsecaddr!=-1) {
        sm=Ds[Ad]->chn[actsecaddr].secmode;
        switch(sm) {
            case SM_WRITETOD64 :
//printf("    write2d64\n");
                bn=Ds[Ad]->chn[actsecaddr].bufnum;
                if(Ds[Ad]->ibnum[bn]>255) flush_buffer(bn,&Ds[Ad]->chn[actsecaddr].file);
                Ds[Ad]->ib[bn][Ds[Ad]->ibnum[bn]++]=chr;
                break;
            case SM_FILENAME :
//printf("    filename\n");
                if(Ds[Ad]->chn[actsecaddr].filenamelen<30) Ds[Ad]->chn[actsecaddr].filename[Ds[Ad]->chn[actsecaddr].filenamelen++]=chr;
                break;
            case SM_COMMANDINPUT :
            case SM_COMMAND :
//printf("    command\n");
                if(actsecaddr==15) {
                    Ds[Ad]->chn[15].secmode=SM_COMMAND;
                    if(ibufnum<50) ibuf[ibufnum++]=chr;
                }
                break;
        }
    }
}
void REGARGS SOFT_Second(REG(d0,char secaddr)) {        // ***SECLST***
    int smode,i;
    if(inited[Ad]==0) return;
//printf("SOFT_Second (%x)\n",(unsigned char)secaddr);
    d64_ST=0;
    if(mode==IM_LISTEN) {
        smode=secaddr&0xf0;
        actsecaddr=secaddr&0x0f;
        if(actsecaddr!=15) {
            switch(smode) {
                case ICMD_DATA :            // $6_
//printf("  data\n");
                    if(Ds[Ad]->chn[actsecaddr].secmode==SM_FILEOPEN) {
                        if(Ds[Ad]->chn[actsecaddr].file.type==T_NONE) {
                            if(0==(open_file_wrt(actsecaddr))) Ds[Ad]->chn[actsecaddr].secmode=SM_WRITETOD64;
                        } else if(Ds[Ad]->chn[actsecaddr].file.type==T_RND) {
                            Ds[Ad]->chn[actsecaddr].secmode=SM_WRITETOD64;
                            Ds[Ad]->chn[actsecaddr].file.mode=FM_WRITE;
                        }
                    }
                    break;
                case ICMD_CLOSE :           // $e_
//printf("  close\n");
                    Ds[Ad]->chn[actsecaddr].secmode=SM_CLOSE;
                    break;
                case ICMD_OPEN :            // $f_
//printf("  open\n");
                    if(0==(alloc_channel(actsecaddr))) {
                        Ds[Ad]->chn[actsecaddr].secmode=SM_FILENAME;
                        Ds[Ad]->chn[actsecaddr].filenamelen=0;
                        for(i=0;i<32;i++) Ds[Ad]->chn[actsecaddr].filename[i]='\0';
                    } else set_status0(Ds[Ad],70);
                    break;
            }
        } else if(ICMD_CLOSE!=smode) {
//printf("  command\n");
            alloc_channel(15);
            Ds[Ad]->chn[15].secmode=SM_COMMANDINPUT;
            ibufnum=0;
        }
    }
}
void REGARGS SOFT_TkSA(REG(d0,char secaddr)) {          // ***SECTLK***
    int smode;
    if(inited[Ad]==0) return;
//printf("SOFT_TkSA (%x)\n",(unsigned char)secaddr);
    d64_ST=0;
    if(mode==IM_TALK) {
        smode=secaddr&0xf0;
        actsecaddr=secaddr&0x0f;
        if(actsecaddr!=15&&smode==ICMD_DATA&&Ds[Ad]->chn[actsecaddr].busy==CH_BUSY) {
            if(Ds[Ad]->chn[actsecaddr].secmode==SM_FILEOPEN) {
                if(0==(open_file(actsecaddr))) {
                    Ds[Ad]->chn[actsecaddr].secmode=SM_READFROMD64;
                }
            } else if(Ds[Ad]->chn[actsecaddr].file.type!=T_NONE) {
                Ds[Ad]->chn[actsecaddr].secmode=SM_READFROMD64;
                Ds[Ad]->chn[actsecaddr].file.mode=FM_READ;
            }
        } else if(actsecaddr==15) {
            Ds[Ad]->numstatus=0;
            reset_status(Ds[Ad]);
        }
    }
}
void SOFT_UnListen(void) {                              // ***UNLST***
    if(inited[Ad]==0) return;
//printf("SOFT_UnListen\n");
    d64_ST=0;
    if(mode==IM_LISTEN&&actsecaddr!=-1) {
        switch(Ds[Ad]->chn[actsecaddr].secmode) {
            case SM_WRITETOD64 :
//printf("  < write to d64\n");
                break;
            case SM_CLOSE :
//printf("  < close\n");
                free_channel(actsecaddr);
                break;
            case SM_FILENAME :
//printf("  < filename\n");
                Ds[Ad]->chn[actsecaddr].secmode=SM_FILEOPEN;
                break;
            case SM_COMMAND :
//printf("  < command\n");
                ibuf[ibufnum]='\0';
                execute_comm(ibuf);
                Ds[Ad]->chn[15].secmode=SM_NONE;
                break;
        }
        mode=IM_NONE;
    }
}
void SOFT_UnTalk(void) {                                // ***UNTLK***
    if(inited[Ad]==0) return;
//printf("SOFT_UnTalk\n");
    d64_ST=0;
    if(mode==IM_TALK) {
        mode=IM_NONE;
        actsecaddr=-1;
    }
}
