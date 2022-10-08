/* :ts=4							cp4_loc.c
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

extern struct IntuitionBase *IntuitionBase;

struct LocaleBase *LocaleBase=0L;
struct Catalog *cp4_Catalog=0L;

#include <proto/exec.h>
#include <libraries/locale.h>
#include <proto/locale.h>
#include <exec/types.h>
#include <proto/intuition.h>

#include <stdio.h>

#include "cp4_loc.h"
#include "cp4_locstrs.c"

static int cp4_NumberOfMessages=0;
static struct EasyStruct es_req={
	sizeof(struct EasyStruct),
	0,
	"Locale error",
	"Can't find string number %ld\nContinue with dummy string?",
	"Continue|Abort"
};

void Opencp4Catalog(char *language) {
	long tag,tagarg=0L;
	int i;
	for(i=0;cp4_DefaultStrings[i]!=NULL;i++);
	cp4_NumberOfMessages=i-1;
	if(!LocaleBase) LocaleBase=(struct LocaleBase *)OpenLibrary("locale.library",37);
	if(language == 0L) tag=TAG_IGNORE;
	else {
		tag=OC_Language;
		tagarg=(long)language;
	}
	if(LocaleBase!=0L&&cp4_Catalog==0L) cp4_Catalog=OpenCatalog(0L,(char *)"cp4.catalog",
			OC_BuiltInLanguage, (unsigned long)"english",
			tag, tagarg,
			OC_Version, 3L,
			TAG_DONE,0L);
}

char *GetStr(long strnum) {
static char nostr[]="***NOMSG";
	char *rs=0L;

	if(strnum<0) {
		rs=nostr;
		if(IntuitionBase!=0L) {
			if(0==(EasyRequest(0L,&es_req,0L,(ULONG)strnum))) exit(20);
		} else {
			fprintf(stderr,"Can't find string number %ld\n",strnum);
		}
	} else {
		if(cp4_Catalog!=0L) rs=GetCatalogStr(cp4_Catalog,strnum+1,0L);
		if(rs==0L&&strnum<=cp4_NumberOfMessages) {
			rs=cp4_DefaultStrings[strnum];
		}
		if(rs==0L) {
			rs=nostr;
			if(IntuitionBase!=0L) {
				if(0==(EasyRequest(0L,&es_req,0L,(ULONG)strnum))) exit(20);
			} else {
				fprintf(stderr,"Can't find string number %ld\n",strnum);
			}
		}
	}
	return(rs);
}

void Closecp4Catalog(void) {
	if(LocaleBase!=0L) CloseCatalog(cp4_Catalog);
	cp4_Catalog=0L;
	if(LocaleBase) CloseLibrary((struct Library *)LocaleBase);
	LocaleBase=0L;
}
