// :ts=4
#include <stdio.h>
#include <stdlib.h>
#include <proto/iec.h>
#include <proto/exec.h>
#include "cp4_ver.h"

struct iecbase *IECBase=NULL;

#define VER "\0$VER: Save1541ROM 1.0 (" ## DATE ## ") Copyright © by Gáti Gergely"

unsigned char t[256];
char ver[]=VER;

int myatoi(char *str) {
    int ret;
    if(*str=='$') sscanf(str,"$%x",&ret);
    else if(*str=='0'&&*(str+1)=='x') sscanf(str,"0x%x",&ret);
    else ret=atoi(str);
    return(ret);
}

/* addr-tól kezdve beolvas n byte-ot (n<=255)
 */
void ReadMem(int addr,int n) {
    if(n>255) return;
    Listen(8);
    Second(0x6f);
    CIOut(0x4d); CIOut(0x2d); CIOut(0x52);      // m-r
    CIOut((unsigned char)(addr&0xff));          // lo
    CIOut((unsigned char)((addr>>8)&0xff));     // hi
    CIOut((unsigned char)n);                    // num
    CIOut(13);                                  // return
    UnListen();
}


int main(int argc,char **argv) {
    int i,a,k;
    int base,num,maradek;
    FILE *f;

    if(argc!=4) {
        if(argc==0) return(0);      // cli only
        fprintf(stderr,"Usage: %s addr_from addr_to romfile\nTo save ROM, type '%s $c000 $ffff romimage'\n",argv[0],argv[0]);
        return(0);
    }
    base=myatoi(argv[1]);
    num=myatoi(argv[2]);
    num-=base;
    if(num<=0) {
        fprintf(stderr,"%s: 'from' must be less than 'to'\n",argv[0]);
        return(0);
    }
    maradek=(num%128)+1;
    num=num/128;
    if(NULL==(f=fopen(argv[3],"wb"))) {
        fprintf(stderr,"%s: cannot open '%s'\n",argv[0],argv[3]);
        return(0);
    }
    if(NULL!=(IECBase=(struct iecbase *)OpenLibrary("iec.library",1L))) {
        for(a=0;a<=num;a++) {
            k=128;
            if(a==num) k=maradek;
            ReadMem(base+(a*128),k);
            Talk(8);
            TkSA(0x6f);
            for(i=0;i<k;i++) t[i]=ACPtr();
            UnTalk();
            Listen(8);
            Second(0xef);
            UnListen();
            for(i=0;i<k;i++) fputc(t[i],f);
        }
        CloseLibrary((struct Library *)IECBase);
        IECBase=NULL;
        fclose(f);
    } else {
        fprintf(stderr,"%s: cannot open iec.library\n",argv[0]);
    }
}
