/* :ts=4                            keymapeditgui.c
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
#include <stdio.h>
#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/keymap.h>
#include <utility/hooks.h>
#include <devices/inputevent.h>
#include "p4req.h"
#include "cp4_loc.h"

// from ted.c
extern int setkeymap(char *name);
extern int keytab[];
extern char *makefilename(struct FileRequester *f);
extern struct FileRequester *frq;

// from dbg.c
extern char mtoupper(char c);

// from cp4.c
extern char *opt_keymap;

// preferencesgui.c
extern void getpurename(char *fullname,char *name);

#define K_MAXKEY        10
#define K_NOKEY         -1

struct Library *KeymapBase=NULL;
static int modified;
static int asgrkey;
static int asgp4key;

#include "keymapedit.c"
#include "common.h"


static struct Gadget *sgad;         // gadgethelp iaddress!
static char kmapeditttl[240];
//     char kmapeditttldef[]="Keymap Editor - "; // moved to constans
static char flagclr[]="-";
static struct Requester InvisibleRequester;
static int fncrawk;             // lenyomott billentyû
static int fctab[]={-2,-9,-8,-5,-3,-7,-10,-6,-4,            // functions -> codes
                    -11,-12,-13,-14,-15,-16,-17,-18,-19,-20,-21,-22 };
static char *keynames[]={
    "@","F3","F2","F1","HELP","£","RETURN","INST/DEL","SHIFT","E","S","Z","4","A","W",
    "3","X","T","F","C","6","D","R","5","V","U","H","B","8","G","Y","7","N","O","K",
    "M","0","J","I","9",",","-",":",".","CRSR-UP","L","P","CRSR-DOWN","/","+","=",
    "ESCAPE","CRSR-RIGHT",";","*","CRSR-LEFT","RUN/STOP","Q","COMMODORE","SPACE","2",
    "CONTROL","CLEAR/HOME","1",
    NULL
};
static int kraws[128];      // rawkeyekhez -> Plus4 kódok

struct Hook KMapHook;

static ULONG SAVEDS REGARGS KMapHookFunc(REG(a0,struct Hook *mh),REG(a2,struct FileRequester *fr),REG(a1,struct AnchorPath *ap)) {
static char buf[11];
static char fullname[512];
//  int l=510;
//  char *t,*f;
    BPTR fp;
    ULONG ret=FALSE;

/*
    t=fullname;
    f=fr->fr_Drawer;
    while('\0'!=(*t++=*f++)&&--l>=0);
    t--;                                    // points to '0'
    if(*(t-1)!=':') *t++='/';               // points to after last char
    f=ap->ap_Info.fib_FileName;
    while('\0'!=(*t++=*f++)&&--l>=0);
*/
    strcpy(fullname,fr->fr_Drawer);
    AddPart(fullname,ap->ap_Info.fib_FileName,512);
    if(NULL!=(fp=Open(fullname,MODE_OLDFILE))) {
        //fgets(buf,10,fp);
        Read(fp,buf,10);
        Close(fp);
//      fread(buf,1,10,fp);
        buf[9]='\0';
        if(strcmp(buf,K_MAGIC)==0) ret=TRUE;
//      fclose(fp);
    }
    return(ret);
} // KMapHookFunc()

void InitKMapHook(void) {
    KMapHook.h_Entry=(HOOKFUNC)KMapHookFunc;
    KMapHook.h_SubEntry=NULL;
    KMapHook.h_Data=NULL;
} // InitKMapHook()

static void SleepWindow(struct Window *wnd) {
static struct TagItem BusyPointerTagList[]={
    {WA_BusyPointer,TRUE},
    {TAG_END,0}
};
    if(wnd) {
        InitRequester(&InvisibleRequester);
        Request(&InvisibleRequester,wnd);
        SetWindowPointerA(wnd,BusyPointerTagList);
    }
}

static void AwakeWindow(struct Window *wnd) {
    if(wnd) {
        EndRequest(&InvisibleRequester,wnd);
        SetWindowPointerA(wnd,NULL);
    }
}


/*
 * raw keycode to txt (buffer MIN 16 char!)
 */
static void raw2str(int raw, char *to) {
static char *rawtab[]={
    "Spc","Bck","Tab","Enter","Ret","Esc","Del",0L,0L,0L,"Num-",0L,"Up","Down","Right","Left",
    "F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","Num[","Num]","Num/","Num*","Num+","Help",
    "LShift","RShift","Caps-Lock","Ctrl","LAlt","RAlt","LAmiga","RAmiga",0L,0L,0L,0L,0L,0L,0L,0L,
    0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,
    NULL
};
static struct InputEvent ie;
static char bf[80];
    int ln=0,i;

    to[0]='\0';
    if(raw>127) {
        strcpy(to,GetStr(MSG_01C7));
        return;
    }
    if(raw<64) {
        for(i=0;i<80;i++) bf[i]=0;
        ie.ie_Class=IECLASS_RAWKEY;
        ie.ie_SubClass=0;
        ie.ie_Code=raw;
        ie.ie_Qualifier=0;
        ie.ie_EventAddress=NULL;
        ln=MapRawKey(&ie,bf,80,NULL);
        if(ln>0) for(i=0;i<15;i++) to[i]=bf[i];
        else to[0]='\0';
        to[15]='\0';
        to[0]=mtoupper(to[0]);
    } else {
        raw-=64;
        if(rawtab[raw]!=0L) strcpy(to,rawtab[raw]);
        else strcpy(to,GetStr(MSG_01C8));
    }
    return;
} // raw2str()


/*
 * getrawkeys :
 *  egy Plus4-es kódhoz rendelt Amiga rawkey kódokat adja vissza
 *  egy stringbe (min: 150 char free - 8 rawkey)
 *  return: hány megfelelõ volt
 */
int getrawkeys(int p4code, char *bf) {
static char rc[16];
    int i,f=0;

    bf[0]='\0';
    for(i=0;i<128;i++) {
        if(kraws[i]==p4code) {
            if(f!=0) strcat(bf,", ");
            f++;
            raw2str(i,rc);
            strcat(bf,rc);
            if(f>7) break;
        }
    }
    return(f);
} // getrawkeys()

/*
 * gombnyomásra
 */
static int keybutton(int k) {
static char txt[150];
static char *tal;
static char ttl[32];
static struct IntuiMessage imsg;
static char *flagmod;
    ULONG di;
    int stop=1;

    flagmod=GetStr(MSG_012C);
    tal=GetStr(MSG_0181);
    imsg.IAddress=kmapeditGadgets[k];
    kmapeditGadgetHelp(&imsg);

    asgp4key=k;
    txt[0]='\0';
    SleepWindow(kmapeditWnd);
    kmapassignLeft=kmapeditWnd->LeftEdge+emx-(kmapassignWidth/2);
    kmapassignTop=kmapeditWnd->TopEdge+emy-(kmapassignHeight/2);
    if(0==OpenkmapassignWindow()) {
        strcpy(ttl,tal);
        strcat(ttl,keynames[k]);
        strcat(ttl,GetStr(MSG_01CA));
        SetWindowTitles(kmapassignWnd,ttl,(UBYTE *)~0);
        getrawkeys(k,txt);
        GT_SetGadgetAttrs(kmapassignGadgets[GD_ak_yet],kmapassignWnd,NULL,GTTX_Text,(ULONG)txt,TAG_DONE);
        GT_SetGadgetAttrs(kmapassignGadgets[GD_ak_new],kmapassignWnd,NULL,GTTX_Text,(ULONG)GetStr(MSG_01CB),TAG_DONE);
        modified=-1;
        while(stop!=0) {
            WaitPort(kmapassignWnd->UserPort);
            stop=HandlekmapassignIDCMP();
        }
        if(modified!=-1) {
            int i=gettxtnum(k);
            GT_GetGadgetAttrs(kmapassignGadgets[GD_ak_add],kmapassignWnd,NULL,GA_Disabled,(ULONG)&di,TAG_DONE);
            if(di!=TRUE) {
                GT_SetGadgetAttrs(kmapeditGadgets[i],kmapeditWnd,NULL,GTTX_Text,(ULONG)flagmod,TAG_DONE);
            } else {
                GT_SetGadgetAttrs(kmapeditGadgets[i],kmapeditWnd,NULL,GTTX_Text,(ULONG)flagclr,TAG_DONE);
                for(i=0;i<128;i++) if(kraws[i]==k) kraws[i]=K_NOKEY;
            }
        }
        ClosekmapassignWindow();
    }
    AwakeWindow(kmapeditWnd);
    return(0);
}


/*
 * beállítja a text gadgeteket
 */
void settxts(void) {
static char bf[150];
    int i;

    // set txt gadgets
    for(i=0;i<64;i++) {
        getrawkeys(i,bf);
        if(bf[0]=='\0') GT_SetGadgetAttrs(kmapeditGadgets[gettxtnum(i)],kmapeditWnd,NULL,GTTX_Text,(ULONG)flagclr,TAG_DONE);
        else GT_SetGadgetAttrs(kmapeditGadgets[gettxtnum(i)],kmapeditWnd,NULL,GTTX_Text,(ULONG)" ",TAG_DONE);
    }
} // settxts()


/*----------------*
 * REAL FUNCTIONS *
 *----------------*/

/*
 * -----------
 * Edit Window
 * -----------
 */
int BeginKMap(struct Screen *scr) {
static char kmapname[220];
    int i;

    InitKMapHook();
    if(NULL==(KeymapBase=OpenLibrary("keymap.library",36))) return(-1);
    if(NULL==(DiskfontBase=OpenLibrary("diskfont.library",37))) return(-1);
    if(!(Font=OpenDiskFont(&topaz8))) return(5L);
    Scr=scr;
    if(scr==NULL) return(1L);
    if(!(VisualInfo=GetVisualInfo(Scr,TAG_DONE,0L))) return(2L);

    if(0!=OpenkmapeditWindow()) return(-1);

    // set kraws[]
    for(i=0;i<128;i++) kraws[i]=keytab[i];

    // set txt
    settxts();

    // set title
    strcpy(kmapeditttl,GetStr(MSG_01CE));
    strcat(kmapeditttl," - ");
    if(opt_keymap!=NULL) getpurename(opt_keymap,kmapname);
    else strcpy(kmapname,GetStr(MSG_01CF));
    strcat(kmapeditttl,kmapname);
    SetWindowTitles(kmapeditWnd,kmapeditttl,(UBYTE *)~0);

    return(0L);
}

void EndKMap( void )
{
    int i,ok;

    ClosekmapeditWindow();

    if(KeymapBase) {
        CloseLibrary(KeymapBase);
        KeymapBase=NULL;
    }

    if(DiskfontBase) {
        CloseLibrary(DiskfontBase);
        DiskfontBase=NULL;
    }

    if ( VisualInfo ) {
        FreeVisualInfo( VisualInfo );
        VisualInfo = NULL;
    }
    if ( Scr        ) {
        Scr = NULL;
    }
    if ( Font       ) {
        CloseFont( Font );
        Font = NULL;
    }

    if(0!=setkeymap(opt_keymap)) {
        p4req1(NULL,P4_WARNING,GetStr(MSG_01D0));
        opt_keymap=NULL;
        AddOption("KEYMAP","DEFAULT");
        setkeymap(NULL);
    }
    for(ok=0,i=0;i<128;i++) if(keytab[i]==-3) ok=1;
    if(ok==0) {
        opt_keymap=NULL;
        p4req1(NULL,P4_WARNING,GetStr(MSG_01D1));
    }
}


static int b_openClicked( void )
{
    /* routine when gadget "Open" is clicked. */
static char kmapname[220];
    char *filename;
    int i;

    if(opt_keymap!=NULL) getpurename(opt_keymap,kmapname);
    else strcpy(kmapname,"");
    if(AslRequestTags(frq,
            ASLFR_TitleText,(ULONG)GetStr(MSG_01D2),
            ASLFR_Window, (ULONG)kmapeditWnd,
            ASLFR_SleepWindow, TRUE,
            ASLFR_RejectIcons, TRUE,
            ASLFR_DoSaveMode, FALSE,
            ASLFR_InitialDrawer, (ULONG)"PROGDIR:KeyMap",
            ASLFR_InitialFile, (ULONG)kmapname,
            ASLFR_Screen, (ULONG)Scr,
            ASLFR_FilterFunc, (ULONG)&KMapHook,
            ASLFR_FilterDrawers,FALSE,
            TAG_DONE,0L )) {
        filename=makefilename(frq);
        if(0!=setkeymap(filename)) {
            p4req1(kmapeditWnd,P4_WARNING,GetStr(MSG_01D4));
        } else {
            AddOption("KEYMAP",filename);
            opt_keymap=GetOption("KEYMAP",NULL);
            if(opt_keymap==NULL) p4req1(kmapeditWnd,P4_ERROR,GetStr(MSG_01D5));
            // set title
            strcpy(kmapeditttl,GetStr(MSG_01CE));
            strcat(kmapeditttl," - ");
            strcat(kmapeditttl,frq->rf_File);
            SetWindowTitles(kmapeditWnd,kmapeditttl,(UBYTE *)~0);
            // set kraws[]
            for(i=0;i<128;i++) kraws[i]=keytab[i];
            // set txt
            settxts();
        }
    }
    return(TRUE);
}

static int b_saveClicked( void )
{
    /* routine when gadget "Save" is clicked. */
static char kmapname[220];
    char *filename;
    FILE *f;
    int i,ok,pr;

    if(opt_keymap!=NULL) getpurename(opt_keymap,kmapname);
    else strcpy(kmapname,"");
    if(AslRequestTags(frq,
            ASLFR_TitleText,(ULONG)GetStr(MSG_01D6),
            ASLFR_Window, (ULONG)kmapeditWnd,
            ASLFR_SleepWindow, TRUE,
            ASLFR_RejectIcons, TRUE,
            ASLFR_DoSaveMode, TRUE,
            ASLFR_InitialDrawer, (ULONG)"PROGDIR:keymap",
            ASLFR_InitialFile, (ULONG)kmapname,
            ASLFR_Screen, (ULONG)Scr,
            ASLFR_FilterFunc, (ULONG)&KMapHook,
            ASLFR_FilterDrawers,FALSE,
            TAG_DONE,0L )) {
        filename=makefilename(frq);
        ok=1;
        f=fopen(filename,"r");
        if(f!=NULL) {
            fclose(f);
            ok=p4req2(kmapeditWnd,P4_WARNING,GetStr(MSG_01D8));
        }
        if(ok!=0) {
            // DoSave
            f=fopen(filename,"wb");
            if(f!=NULL) {
                fprintf(f,K_MAGIC);
                for(pr=0,i=0;i<128;i++) if(kraws[i]==-3) pr=1;
                if(pr==0) {
                    kraws[89]=-3;
                    p4req1(kmapeditWnd,P4_WARNING,GetStr(MSG_01D9));
                }
                for(i=0;i<128;i++) fputc(kraws[i],f);
                fclose(f);
                AddOption("KEYMAP",filename);
                opt_keymap=GetOption("KEYMAP",NULL);
                if(opt_keymap==NULL) p4req1(kmapeditWnd,P4_ERROR,GetStr(MSG_01D5));
                // set title
                getpurename(opt_keymap,kmapname);
                strcpy(kmapeditttl,GetStr(MSG_01CE));
                strcat(kmapeditttl," - ");
                strcat(kmapeditttl,kmapname);
                SetWindowTitles(kmapeditWnd,kmapeditttl,(UBYTE *)~0);
            } else p4req1(kmapeditWnd,P4_WARNING,GetStr(MSG_01DA));
        }
    }
    return(TRUE);
}

static int b_cancelClicked( void )
{
    /* routine when gadget "Cancel" is clicked. */
    return(FALSE);
}

static int b_newClicked( void )
{
    /* routine when gadget "New" is clicked. */
    int i;

    for(i=0;i<128;i++) if(kraws[i]>=0) kraws[i]=K_NOKEY;
    settxts();
    // set title
    strcpy(kmapeditttl,GetStr(MSG_01CE));
    strcat(kmapeditttl," - ");
    strcat(kmapeditttl,GetStr(MSG_01DB));
    SetWindowTitles(kmapeditWnd,kmapeditttl,(UBYTE *)~0);
    return(TRUE);
}

static int b_functionsClicked( void )
{
    /* routine when gadget "Functions" is clicked. */
    int stop=1;

    SleepWindow(kmapeditWnd);
    kmapfunctionsLeft=kmapeditWnd->LeftEdge+emx-(kmapfunctionsWidth/2);
    kmapfunctionsTop=kmapeditWnd->TopEdge+emy-(kmapfunctionsHeight/2);
    if(0==OpenkmapfunctionsWindow()) {
        GT_SetGadgetAttrs(kmapfunctionsGadgets[GD_kf_new],kmapfunctionsWnd,NULL,GTTX_Text,(ULONG)GetStr(MSG_01CB),TAG_DONE);
        GT_SetGadgetAttrs(kmapfunctionsGadgets[GD_kf_assign],kmapfunctionsWnd,NULL,GA_Disabled,TRUE,TAG_DONE);
        fncrawk=K_NOKEY;
        while(stop!=0) {
            WaitPort(kmapfunctionsWnd->UserPort);
            stop=HandlekmapfunctionsIDCMP();
        }
        ClosekmapfunctionsWindow();
    }
    AwakeWindow(kmapeditWnd);
    return(TRUE);
}

static int kmapeditGadgetHelp(struct IntuiMessage *imsg) {
    /* routine for "IDCMP_GADGETHELP". */
static char rws[150];
static char txt[150+16];
static char empty[]="";
    int i;

    sgad=imsg->IAddress;
    for(i=0;i<64;i++) if(sgad==kmapeditGadgets[i]||sgad==kmapeditGadgets[gettxtnum(i)]) break;
    if(i!=64) {
        // valid gadget
        txt[0]='\'';
        txt[1]='\0';
        strcat(txt,keynames[i]);
        strcat(txt,"' ==> ");
        getrawkeys(i,rws);
        if(rws[0]!='\0') strcat(txt,rws);
        else strcat(txt,GetStr(MSG_01CB));
        GT_SetGadgetAttrs(kmapeditGadgets[GD_b_status],kmapeditWnd,NULL,GTTX_Text,(ULONG)txt,TAG_DONE);
    } else {
        GT_SetGadgetAttrs(kmapeditGadgets[GD_b_status],kmapeditWnd,NULL,GTTX_Text,(ULONG)empty,TAG_DONE);
    }
    return(TRUE);
}

static int kmapeditCloseWindow( void )      // --> Cancel
{
    /* routine for "IDCMP_CLOSEWINDOW". */
    return(b_cancelClicked());
}

static void catkname(char *s,int i) {
static firstrun=0;
static char *funcnames[]={      // MAX 16 char yet!!
    "-","-",    NULL,   NULL,   NULL,
    NULL,       NULL,   NULL,   NULL,
    NULL,       NULL,   NULL,   NULL,
    NULL,       NULL,   NULL,   NULL,
    NULL,       NULL,   NULL,   NULL,
    NULL,       NULL,   NULL,
};

    if(firstrun==0) {   // initialize
        firstrun=1;
        funcnames[2]=GetStr(MSG_0264);
        funcnames[3]=GetStr(MSG_018B);
        funcnames[4]=GetStr(MSG_018E);
        funcnames[5]=GetStr(MSG_018A);
        funcnames[6]=GetStr(MSG_018D);
        funcnames[7]=GetStr(MSG_0119);
        funcnames[8]=GetStr(MSG_0189);
        funcnames[9]=GetStr(MSG_0188);
        funcnames[10]=GetStr(MSG_018C);
        funcnames[11]=GetStr(MSG_018F);
        funcnames[12]=GetStr(MSG_0190);
        funcnames[13]=GetStr(MSG_0191);
        funcnames[14]=GetStr(MSG_0192);
        funcnames[15]=GetStr(MSG_0193);
        funcnames[16]=GetStr(MSG_0194);
        funcnames[17]=GetStr(MSG_0195);
        funcnames[18]=GetStr(MSG_0196);
        funcnames[19]=GetStr(MSG_0197);
        funcnames[19]=GetStr(MSG_0197);
        funcnames[20]=GetStr(MSG_0310);
        funcnames[21]=GetStr(MSG_0311);
        funcnames[22]=GetStr(MSG_011C);
    }
    if(i!=K_NOKEY) {
        if(i>=0) {
            strcat(s," (");
            strcat(s,keynames[i]);
            strcat(s,")");
        } else {
            // function key
            strcat(s," <");
            strcat(s,funcnames[-i]);
            strcat(s,">");
        }
    }
} // catkname()

static int kmapeditRawKey(struct IntuiMessage *imsg) {
    /* routine for "IDCMP_RAWKEY". */
static char *tx;
static char rw[15+16+16+4];
static struct IntuiMessage myimsg;
    int i;

    tx=GetStr(MSG_01E1);
    if(imsg->Code<128) {                // DOWN
        strcpy(rw,tx);
        strcat(rw," ");
        raw2str(imsg->Code,&rw[strlen(rw)]);
        i=kraws[imsg->Code];
        catkname(rw,i);
        GT_SetGadgetAttrs(kmapeditGadgets[GD_b_status],kmapeditWnd,NULL,GTTX_Text,(ULONG)rw,TAG_DONE);
    } else {                            // RELEASE
        myimsg.IAddress=sgad;
        kmapeditGadgetHelp(&myimsg);
    }
    return(TRUE);
}


/*
 * -------------
 * Assign Window
 * -------------
 */
static int ak_addClicked( void )
{
    /* routine when gadget "Add" is clicked. */
    modified=1;
    kraws[asgrkey]=asgp4key;
    return(FALSE);
}

static int ak_replaceClicked( void )
{
    /* routine when gadget "Replace" is clicked. */
    int i;

    modified=1;
    for(i=0;i<128;i++) if(kraws[i]==asgp4key) kraws[i]=K_NOKEY;
    kraws[asgrkey]=asgp4key;
    return(FALSE);
}

static int ak_cancelClicked( void )
{
    /* routine when gadget "Cancel" is clicked. */
    return(FALSE);
}
static int kmapassignCloseWindow( void )
{
    /* routine for "IDCMP_CLOSEWINDOW". */
    return(FALSE);
}

static int kmapassignRawKey( struct IntuiMessage *imsg )
{
static char rw[16+16+4];
    /* routine for "IDCMP_RAWKEY". */
    int i;
    if(imsg->Code>=128) return(TRUE);
    raw2str(imsg->Code,rw);
    i=kraws[imsg->Code];
    asgrkey=imsg->Code;
    catkname(rw,i);
    GT_SetGadgetAttrs(kmapassignGadgets[GD_ak_new],kmapassignWnd,NULL,GTTX_Text,(ULONG)rw,TAG_DONE);
    return(TRUE);
}



/*
 * ----------------
 * Functions Window
 * ----------------
 */

static int kf_flistClicked( void )
{
    /* routine when gadget "Functions" is clicked. */
static char rw[16+4];
    int i,ok,fnc;
    ULONG fncload;

    GT_SetGadgetAttrs(kmapfunctionsGadgets[GD_kf_assign],kmapfunctionsWnd,NULL,GA_Disabled,FALSE,TAG_DONE);
    GT_GetGadgetAttrs(kmapfunctionsGadgets[GD_kf_flist],kmapfunctionsWnd,NULL,GTLV_Selected,(ULONG)&fncload,TAG_DONE);
    if(fncload!=~0) {
        fnc=fctab[fncload];
        if(fnc<0) {
            for(ok=-1,i=0;i<128;i++) if(kraws[i]==fnc) ok=i;
            if(ok<0) strcpy(rw,GetStr(MSG_01CB));
            else raw2str(ok,rw);
        } else strcpy(rw,GetStr(MSG_01E2));
        GT_SetGadgetAttrs(kmapfunctionsGadgets[GD_kf_yet],kmapfunctionsWnd,NULL,GTTX_Text,(ULONG)rw,TAG_DONE);
    }
    return(TRUE);
}

static int kf_assignClicked( void )
{
    /* routine when gadget "Assign" is clicked. */
static struct IntuiMessage imsg;
    int i,fnc;
    ULONG fncload;

    GT_GetGadgetAttrs(kmapfunctionsGadgets[GD_kf_flist],kmapfunctionsWnd,NULL,GTLV_Selected,(ULONG)&fncload,TAG_DONE);
    if(fncload==~0) return(TRUE);
    fnc=fctab[fncload];
    if(fnc>=0) return(TRUE);
    for(i=0;i<128;i++) if(kraws[i]==fnc) kraws[i]=K_NOKEY;
    if(fncrawk!=K_NOKEY) kraws[fncrawk]=fnc;
    kf_flistClicked();
    imsg.Code=fncrawk;
    kmapfunctionsRawKey(&imsg);
    return(TRUE);
}

static int kf_cancelClicked( void )
{
    /* routine when gadget "Ok" is clicked. */
    return(FALSE);
}

static int kmapfunctionsCloseWindow( void )
{
    /* routine for "IDCMP_CLOSEWINDOW". */
    return(FALSE);
}

static int kmapfunctionsRawKey(struct IntuiMessage *imsg)
{
    /* routine for "IDCMP_RAWKEY". */
static char rw[16+16+4];
    int i;

    if(imsg->Code>=128) return(TRUE);
    fncrawk=K_NOKEY;
    raw2str(imsg->Code,rw);
    i=kraws[imsg->Code];
    fncrawk=imsg->Code;
    catkname(rw,i);
    GT_SetGadgetAttrs(kmapfunctionsGadgets[GD_kf_new],kmapfunctionsWnd,NULL,GTTX_Text,(ULONG)rw,TAG_DONE);
    return(TRUE);
}




/*
 * KEYMATRIX
 */
static int kkukacClicked( void )
{
    /* routine when gadget "@" is clicked. */
    keybutton(0);
    return(TRUE);
}
static int kf3Clicked( void )
{
    /* routine when gadget "F3" is clicked. */
    keybutton(1);
    return(TRUE);
}
static int kf2Clicked( void )
{
    /* routine when gadget "F2" is clicked. */
    keybutton(2);
    return(TRUE);
}
static int kf1Clicked( void )
{
    /* routine when gadget "F1" is clicked. */
    keybutton(3);
    return(TRUE);
}
static int khelpClicked( void )
{
    /* routine when gadget "HELP" is clicked. */
    keybutton(4);
    return(TRUE);
}
static int kfontClicked( void )
{
    /* routine when gadget "£" is clicked. */
    keybutton(5);
    return(TRUE);
}
static int kreturnClicked( void )
{
    /* routine when gadget "RET" is clicked. */
    keybutton(6);
    return(TRUE);
}
static int kinstdelClicked( void )
{
    /* routine when gadget "INST" is clicked. */
    keybutton(7);
    return(TRUE);
}
static int kshiftClicked( void )
{
    /* routine when gadget "SH" is clicked. */
    keybutton(8);
    return(TRUE);
}
static int keClicked( void )
{
    /* routine when gadget "E" is clicked. */
    keybutton(9);
    return(TRUE);
}
static int ksClicked( void )
{
    /* routine when gadget "S" is clicked. */
    keybutton(10);
    return(TRUE);
}
static int kzClicked( void )
{
    /* routine when gadget "Z" is clicked. */
    keybutton(11);
    return(TRUE);
}
static int k4Clicked( void )
{
    /* routine when gadget "4" is clicked. */
    keybutton(12);
    return(TRUE);
}
static int kaClicked( void )
{
    /* routine when gadget "A" is clicked. */
    keybutton(13);
    return(TRUE);
}
static int kwClicked( void )
{
    /* routine when gadget "W" is clicked. */
    keybutton(14);
    return(TRUE);
}
static int k3Clicked( void )
{
    /* routine when gadget "3" is clicked. */
    keybutton(15);
    return(TRUE);
}
static int kxClicked( void )
{
    /* routine when gadget "X" is clicked. */
    keybutton(16);
    return(TRUE);
}
static int ktClicked( void )
{
    /* routine when gadget "T" is clicked. */
    keybutton(17);
    return(TRUE);
}
static int kfClicked( void )
{
    /* routine when gadget "F" is clicked. */
    keybutton(18);
    return(TRUE);
}
static int kcClicked( void )
{
    /* routine when gadget "C" is clicked. */
    keybutton(19);
    return(TRUE);
}
static int k6Clicked( void )
{
    /* routine when gadget "6" is clicked. */
    keybutton(20);
    return(TRUE);
}
static int kdClicked( void )
{
    /* routine when gadget "D" is clicked. */
    keybutton(21);
    return(TRUE);
}
static int krClicked( void )
{
    /* routine when gadget "R" is clicked. */
    keybutton(22);
    return(TRUE);
}
static int k5Clicked( void )
{
    /* routine when gadget "5" is clicked. */
    keybutton(23);
    return(TRUE);
}
static int kvClicked( void )
{
    /* routine when gadget "V" is clicked. */
    keybutton(24);
    return(TRUE);
}
static int kuClicked( void )
{
    /* routine when gadget "U" is clicked. */
    keybutton(25);
    return(TRUE);
}
static int khClicked( void )
{
    /* routine when gadget "H" is clicked. */
    keybutton(26);
    return(TRUE);
}
static int kbClicked( void )
{
    /* routine when gadget "B" is clicked. */
    keybutton(27);
    return(TRUE);
}
static int k8Clicked( void )
{
    /* routine when gadget "8" is clicked. */
    keybutton(28);
    return(TRUE);
}
static int kgClicked( void )
{
    /* routine when gadget "G" is clicked. */
    keybutton(29);
    return(TRUE);
}
static int kyClicked( void )
{
    /* routine when gadget "Y" is clicked. */
    keybutton(30);
    return(TRUE);
}
static int k7Clicked( void )
{
    /* routine when gadget "7" is clicked. */
    keybutton(31);
    return(TRUE);
}
static int knClicked( void )
{
    /* routine when gadget "N" is clicked. */
    keybutton(32);
    return(TRUE);
}
static int koClicked( void )
{
    /* routine when gadget "O" is clicked. */
    keybutton(33);
    return(TRUE);
}
static int kkClicked( void )
{
    /* routine when gadget "K" is clicked. */
    keybutton(34);
    return(TRUE);
}
static int kmClicked( void )
{
    /* routine when gadget "M" is clicked. */
    keybutton(35);
    return(TRUE);
}
static int k0Clicked( void )
{
    /* routine when gadget "0" is clicked. */
    keybutton(36);
    return(TRUE);
}
static int kjClicked( void )
{
    /* routine when gadget "J" is clicked. */
    keybutton(37);
    return(TRUE);
}
static int kiClicked( void )
{
    /* routine when gadget "I" is clicked. */
    keybutton(38);
    return(TRUE);
}
static int k9Clicked( void )
{
    /* routine when gadget "9" is clicked. */
    keybutton(39);
    return(TRUE);
}
static int kvesszoClicked( void )
{
    /* routine when gadget "," is clicked. */
    keybutton(40);
    return(TRUE);
}
static int kminuszClicked( void )
{
    /* routine when gadget "-" is clicked. */
    keybutton(41);
    return(TRUE);
}
static int kkettospontClicked( void )
{
    /* routine when gadget ":" is clicked. */
    keybutton(42);
    return(TRUE);
}
static int kpontClicked( void )
{
    /* routine when gadget "." is clicked. */
    keybutton(43);
    return(TRUE);
}
static int kupClicked( void )
{
    /* routine when gadget "UP" is clicked. */
    keybutton(44);
    return(TRUE);
}
static int klClicked( void )
{
    /* routine when gadget "L" is clicked. */
    keybutton(45);
    return(TRUE);
}
static int kpClicked( void )
{
    /* routine when gadget "P" is clicked. */
    keybutton(46);
    return(TRUE);
}
static int kdownClicked( void )
{
    /* routine when gadget "DOWN" is clicked. */
    keybutton(47);
    return(TRUE);
}
static int kperClicked( void )
{
    /* routine when gadget "/" is clicked. */
    keybutton(48);
    return(TRUE);
}
static int kpluszClicked( void )
{
    /* routine when gadget "+" is clicked. */
    keybutton(49);
    return(TRUE);
}
static int kegyenloClicked( void )
{
    /* routine when gadget "=" is clicked. */
    keybutton(50);
    return(TRUE);
}
static int kescapeClicked( void )
{
    /* routine when gadget "ESC" is clicked. */
    keybutton(51);
    return(TRUE);
}
static int krightClicked( void )
{
    /* routine when gadget "->" is clicked. */
    keybutton(52);
    return(TRUE);
}
static int kpontosvesszoClicked( void )
{
    /* routine when gadget ";" is clicked. */
    keybutton(53);
    return(TRUE);
}
static int kcsillagClicked( void )
{
    /* routine when gadget "*" is clicked. */
    keybutton(54);
    return(TRUE);
}
static int kleftClicked( void )
{
    /* routine when gadget "<-" is clicked. */
    keybutton(55);
    return(TRUE);
}
static int krunstopClicked( void )
{
    /* routine when gadget "STOP" is clicked. */
    keybutton(56);
    return(TRUE);
}
static int kqClicked( void )
{
    /* routine when gadget "Q" is clicked. */
    keybutton(57);
    return(TRUE);
}
static int kcommodoreClicked( void )
{
    /* routine when gadget "C=" is clicked. */
    keybutton(58);
    return(TRUE);
}
static int kspaceClicked( void )
{
    /* routine when gadget "SPC" is clicked. */
    keybutton(59);
    return(TRUE);
}
static int k2Clicked( void )
{
    /* routine when gadget "2" is clicked. */
    keybutton(60);
    return(TRUE);
}
static int kcontrolClicked( void )
{
    /* routine when gadget "CTRL" is clicked. */
    keybutton(61);
    return(TRUE);
}
static int kclearhomeClicked( void )
{
    /* routine when gadget "CLR" is clicked. */
    keybutton(62);
    return(TRUE);
}
static int k1Clicked( void )
{
    /* routine when gadget "1" is clicked. */
    keybutton(63);
    return(TRUE);
}
