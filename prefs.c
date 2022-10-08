/* :ts=4                            prefs.c
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
#include <proto/dos.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prefs.h"
#include "cp4_loc.h"

#define PR_BUFFER   1024

// LoadIt() flags
#define PR_LOAD         0
#define PR_FORCELOAD    1

static char filename[340];
static struct Option *Head=NULL;
static int LoadFrom;
static char *LoadName[]={
    "PROGDIR:cp4.prefs",
    "ENV:cp4.prefs",
    NULL
};
static char *SaveName[]={
    "PROGDIR:cp4.prefs",
    "ENVARC:cp4.prefs",
    NULL
};

struct Option {
    struct Option *Prev;
    struct Option *Next;
    char *Name;
    char *Value;
    int SaveIt;
};

static void PrefsError(char *s) {
    fprintf(stderr,"%s\n",s);
    exit(0);
} // PrefsError


static void FreeNode(struct Option *node) {
    if(Head==node) Head=node->Next;
    if(node->Prev!=NULL) node->Prev->Next=node->Next;
    if(node->Next!=NULL) node->Next->Prev=node->Prev;
    if(node->Value!=NULL) free(node->Value);
    if(node->Name!=NULL) free(node->Name);
    free(node);
} // FreeNode

static void AddNode(char *name,char *value) {
    struct Option *op;
    if(value==NULL||name==NULL) PrefsError(GetStr(MSG_022C));
    if(NULL==(op=malloc(sizeof(struct Option)))) PrefsError(GetStr(MSG_022D));
    op->SaveIt=0;
    if(NULL==(op->Value=malloc(strlen(value)+1))) PrefsError(GetStr(MSG_022D));
    strcpy(op->Value,value);
    if(NULL==(op->Name=malloc(strlen(name)+1))) PrefsError(GetStr(MSG_022D));
    strcpy(op->Name,name);
    op->Next=Head;
    op->Prev=NULL;
    if(Head!=NULL) Head->Prev=op;
    Head=op;
} // AddNode

static struct Option *SeekOption(char *name) {
    struct Option *op;
    for(op=Head;op!=NULL;op=op->Next) if(strcmp(op->Name,name)==0) break;
    return(op);
} // SeekOption

static char *StripSpaces(char *s) {
    char *e;
    for(e=s;*e!='\0'&&*e!='#';e++);
    *e='\0';
    e=&s[strlen(s)-1];
    while(*s==' '||*s=='\t') s++;
    while(*e==' '||*e=='\t'||*e=='\n') *e--='\0';
    return(s);
} // StripSpaces

static int LoadIt(int flag) {
static char buf[PR_BUFFER];
static NotLoadedYet=0;
    char *b,*c,*option,*value;
    FILE *f;
    if(NotLoadedYet&&flag!=PR_FORCELOAD) return(0);
    LoadFrom=0;
    if((NULL==(f=fopen(LoadName[LoadFrom],"rb")))&&(NULL==(f=fopen(LoadName[++LoadFrom],"rb")))) {
        LoadFrom=0;
        return(-1);
    }
    while(NULL!=(fgets(buf,PR_BUFFER,f))) {
        c=StripSpaces(buf);
        if(*c=='\0') continue;
        if(*c!='#') {
            option=c;
            for(;*c!=' '&&*c!='='&&*c!='\t'&&*c!='\0';c++);
            if(*c!='=') {
                for(b=c;*b!='='&&*b!='\0';b++);
                if(*b!='=') PrefsError(GetStr(MSG_022E));
                *b=' ';
            }
            *c++='\0';
            while(*c==' '||*c=='\t') c++;
            if(*c=='"') {
                value=++c;
                while(*c!='"'&&*c!='\0') c++;
                *c='\0';
            } else value=c;
            AddOption(option,value);
        }
    }
    fclose(f);
    NotLoadedYet=1;
    return(0);
} // LoadIt

static int SaveIt(char *name) {
    FILE *f=fopen(name,"wb");
    struct Option *op=Head;
    if(!f) return(-1);
    for(;op!=NULL;op=op->Next) if(op->SaveIt) fprintf(f,"%s=%s\n",op->Name,op->Value);
    fclose(f);
    return(0);
} // SaveIt


static char *GetProgdir(void) {
static char pname[512];
    BPTR lock;
    lock=GetProgramDir();
    NameFromLock(lock,pname,500);
    return(pname);
}

static char *GetENV(char *name) {
static char buf[512];
    FILE *f;
    char *val=NULL;

    strcpy(filename,"ENV:");
    strcat(filename,name);
    if(NULL!=(f=fopen(filename,"rb"))) {
        fgets(buf,510,f);
        fclose(f);
        if(strlen(buf)>0&&buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]='\0';
        val=buf;
    }
    return(val);
}

static int SetENV(char *name,char *value) {
    FILE *f;

    strcpy(filename,"ENV:");
    strcat(filename,name);
    if(NULL!=(f=fopen(filename,"wb"))) {
        fprintf(f,"%s",value);
        fclose(f);
    }
    strcpy(filename,"ENVARC:");
    strcat(filename,name);
    if(NULL!=(f=fopen(filename,"wb"))) {
        fprintf(f,"%s",value);
        fclose(f);
    }
    return(0);
}

/*
 * INTERFACE FUNCTIONS
 */


void AddOption(char *name,char *value) {
    struct Option *op;
    op=SeekOption(name);
    if(op!=NULL) FreeNode(op);
    AddNode(name,value);
} // AddOption

void AddOptionInt(char *name,int value) {
static char s[32];
    sprintf(s,"%d",value);
    AddOption(name,s);
} // AddOptionInt

int UsePrefs(void) {
    struct Option *op;
    LoadIt(PR_DEF);
    if(LoadFrom==0) return(0);
    if(Head==NULL) return(0);
    for(op=Head;op!=NULL;op=op->Next) op->SaveIt=1;
    SaveIt(LoadName[1]);
    for(op=Head;op!=NULL;op=op->Next) op->SaveIt=0;
    return(0);
} // UsePrefs

int SavePrefs(int flag) {
    struct Option *op;
    LoadIt(PR_DEF);
    if(Head==NULL) return(0);
    if(flag==PR_ALL) for(op=Head;op!=NULL;op=op->Next) op->SaveIt=1;
    SaveIt(SaveName[LoadFrom]);
    if(flag==PR_ALL) for(op=Head;op!=NULL;op=op->Next) op->SaveIt=0;
    return(0);
} // SavePrefs

int SavePrefsAll(void) {
    UsePrefs();
    return(SavePrefs(PR_ALL));
} // SavePrefsAll

int Save(char *name,char *value) {
    struct Option *op;
    LoadIt(PR_DEF);
    if(NULL==(op=SeekOption(name))) {
        value=value;
        AddOption(name,value);
        op=SeekOption(name);
        op->SaveIt=1;
    } else {
        op->SaveIt=1;
    }
    return(0);
} // Save

char *GetOption(char *name,char *def) {
    struct Option *op;
    char *value;
    LoadIt(PR_DEF);
    if(NULL==(op=SeekOption(name))) {
        value=def;
        AddOption(name,value);
    } else value=op->Value;
    return(value);
} // GetOption

int GetOptionInt(char *name,int def) {
    struct Option *op;
    char *value;
    int val=0;
    LoadIt(PR_DEF);
    if(NULL==(op=SeekOption(name))) {
        val=def;
        value=NULL;
        AddOptionInt(name,def);
    } else value=op->Value;
    if(value!=NULL) val=atoi(value);
    return(val);
} // GetOptionInt


void FreePrefs(void) {
    struct Option *op;
    for(op=Head;op!=NULL;op=Head) FreeNode(op);
} // FreePrefs

void ReLoadPrefs(void) {
    LoadIt(PR_FORCELOAD);
} // ReLoadPrefs

void LoadPrefs(void) {
    char *pdir,*env;

    LoadIt(PR_DEF);
    pdir=GetProgdir();
    env=GetENV("cp4_PATH");
    if(env==NULL||strcmp(env,pdir)!=0) SetENV("cp4_PATH",pdir);
} // LoadPrefs

