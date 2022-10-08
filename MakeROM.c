/* :ts=4
 */
#include <proto/exec.h>
#include <exec/types.h>
#include <proto/intuition.h>
#include <libraries/locale.h>
#include <proto/locale.h>
#include <stdio.h>
#include "cp4_ver.h"

#define VER "\0$VER: MakeROM 1.3 (" ## DATE ## ") Copyright © by Gáti Gergely"
#define ROMSIZE 16384

struct IntuitionBase *IntuitionBase=NULL;
char Ver[]=VER;

#define MSG_BASEX       0
#define MSG_OK          1
#define MSG_OKCANCEL    2
#define MSG_WARN        3
#define MSG_ERRBAS      4
#define MSG_KEREX       5
#define MSG_ERRKER      6
#define MSG_ERROS96     7
#define MSG_INFO        8
#define MSG_PREFSOVER   9

static char *defstr[]={
    "OS96 low ROM exists\nOverwrite?",
    "Ok",
    "Ok|Cancel",
    "MakeROM Warning",
    "Can't open ROM.OS96_LO",
    "OS96 high ROM exists\nOverwrite?",
    "Can't open ROM.OS96_HI",
    "Can't find Support/os96/OS96LOW.PRG",
    "OS96 ROMs created\nPlease read the OS96 manual",
    "There's an existing Prefs file\nCan I overwrite it according to the new ROMs?",
    NULL
};
static struct LocaleBase *LocaleBase;
static struct Catalog *MR_Catalog=0L;
static int MR_NumberOfMessages=0;
static char **MR_DefaultStrings;

static char *MR_ShowReq(long StringNum) {
 static char nostr[]="***NOMSG";
 static struct EasyStruct es_req={
    sizeof(struct EasyStruct),
    0,
    "Locale error",
    "Can't find string number %ld",
    "Ok"
 };
 struct IntuitionBase *storeIntuiBase;
    storeIntuiBase=IntuitionBase;
    if(0L!=(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",36))) {
        EasyRequest(0L,&es_req,0L,(ULONG)StringNum);
        CloseLibrary((struct Library *)IntuitionBase);
        IntuitionBase=storeIntuiBase;
    }
    return(nostr);
} // MR_ShowReq()

void MR_OpenCatalog(char *DefaultStrings[]) {
    long i;
    if(DefaultStrings==NULL) return;
    for(i=0;DefaultStrings[i]!=NULL;i++);
    MR_DefaultStrings=DefaultStrings;
    MR_NumberOfMessages=i-1;
    if(i==0) return;
    if(!LocaleBase) LocaleBase=(struct LocaleBase *)OpenLibrary("locale.library",37);
    if(LocaleBase!=0L&&MR_Catalog==0L) {
        MR_Catalog=OpenCatalog(0L,(char *)"MakeROM.catalog",
            OC_BuiltInLanguage, (unsigned long)"english",
            OC_Version, 1L,
            TAG_DONE,0L);
    }
} // MR_OpenCatalog()

char *MR_GetStr(long StringNum) {
    char *rs=0L;
    if(StringNum>=0&&StringNum<=MR_NumberOfMessages&&LocaleBase!=0L) rs=GetCatalogStr(MR_Catalog,StringNum+1,MR_DefaultStrings[StringNum]);
    else if(StringNum<=MR_NumberOfMessages) rs=MR_DefaultStrings[StringNum];
    else rs=MR_ShowReq(StringNum);
    return(rs);
} // MR_GetStr()
#define STR(n)  MR_GetStr(n)

void MR_CloseCatalog(void) {
    if(LocaleBase!=0L) CloseCatalog(MR_Catalog);
    MR_Catalog=0L;
    if(LocaleBase) CloseLibrary((struct Library *)LocaleBase);
    MR_NumberOfMessages=0;
} // MR_CloseCatalog()

static int mrreq(char *txt,int num) {
static char *answ[]={ NULL,NULL };
static struct EasyStruct es_req={
    sizeof(struct EasyStruct),
    0,  NULL,   NULL,   NULL,
};
    int ret;
    answ[0]=STR(MSG_OK);
    answ[1]=STR(MSG_OKCANCEL);
    if(num<0||num>1) return(-1);
    if(!(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",37))) return(20);
    es_req.es_TextFormat=txt;
    es_req.es_GadgetFormat=answ[num];
    es_req.es_Title=STR(MSG_WARN);
    ret=EasyRequestArgs(NULL,&es_req,NULL,NULL);
    CloseLibrary((struct Library *)IntuitionBase);
    return(ret);
}

int main(int argc,char **argv) {
    FILE *f,*fo;
    int i,quiet=0,wrt=0;

    if(argc>1) {
        if(argv[1][0]=='Q') quiet=1;
    }
    MR_OpenCatalog(defstr);
    f=fopen("PROGDIR:Support/os96/OS96LOW.PRG","rb");
    if(f!=NULL) {
        fgetc(f);
        fgetc(f);
        fo=fopen("PROGDIR:ROM.OS96_LO","r");
        if(fo!=NULL) {
            fclose(fo);
            if(0==(mrreq(STR(MSG_BASEX),1))) {
                fclose(f);
                MR_CloseCatalog();
                return(0);
            }
        }
        fo=fopen("PROGDIR:ROM.OS96_LO","wb");
        if(fo==NULL) {
            mrreq(STR(MSG_ERRBAS),0);
            fclose(f);
            MR_CloseCatalog();
            return(0);
        }
        for(i=0;i<ROMSIZE;i++) fputc(fgetc(f),fo);
        fclose(fo);
        fo=fopen("PROGDIR:ROM.OS96_HI","r");
        if(fo!=NULL) {
            fclose(fo);
            if(0==(mrreq(STR(MSG_KEREX),1))) {
                fclose(f);
                MR_CloseCatalog();
                return(0);
            }
        }
        fo=fopen("PROGDIR:ROM.OS96_HI","wb");
        if(fo==NULL) {
            mrreq(STR(MSG_ERRKER),0);
            fclose(f);
            MR_CloseCatalog();
            return(0);
        }
        for(i=0;i<ROMSIZE;i++) fputc(fgetc(f),fo);
        fclose(fo);
        fclose(f);
    } else {
        mrreq(STR(MSG_ERROS96),0);
        MR_CloseCatalog();
        return(0);
    }
    if(quiet==0) {
        if(NULL!=(f=fopen("PROGDIR:cp4.prefs","rb"))) {
            if(0!=(mrreq(STR(MSG_PREFSOVER),1))) wrt=1;
            fclose(f);
        } else wrt=1;
    } else wrt=1;
    if(wrt!=0&&NULL!=(f=fopen("PROGDIR:cp4.prefs","wb"))) {
        fprintf(f,"BASIC=ROM.OS96_LO\n");
        fprintf(f,"KERNAL=ROM.OS96_HI\n");
        fclose(f);
        f=NULL;
    }
    if(quiet==0) mrreq(STR(MSG_INFO),0);
    MR_CloseCatalog();
    return(0);
}
