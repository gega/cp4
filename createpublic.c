/* :ts=4
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

//#define DEBUG 1

/*
Gega:WORK/C/cp4> resident c:copy
Gega:WORK/C/cp4> resident c:makedir
Gega:WORK/C/cp4> resident c:rename
Gega:WORK/C/cp4> resident c:list
Gega:WORK/C/cp4> resident c:delete
Gega:WORK/C/cp4> resident c:sort
Gega:WORK/C/cp4> resident c:type
*/

char author[]="Gáti Gergely";

/*
    keressük meg az elõzõ public release-t,
        Last_public_release.txt elsõ sora ='xxx'
        ez egy public/xxx/cp4.lzx file-lesz
    ezt tömörítsük ki a ram-ra
        unlzx x public/xxx/cp4.lzx ram:
    kérjünk róla egy listát:
        cd ram:
        list cp4 all files lformat="%p%n" >ram:filelist.old
        sort ram:filelist.old ram:filelist_old.srt
    kérjük a mostani (PublicFiles.txt)
        és a régi különbségét:
        sort PublicFiles.txt ram:filelist_new.srt
        diff -u ram:filelist_old.srt ram:filelist_new.srt >ram:diff.out
    a ram:cp4-et nevezzük át ram:cp4_xxx-re,
    hozzunk létre a ram:cp4-et (new full)
    hozzunk létre a ram:cp4_update-t (new upd)
    az update-script 2 részbõl áll: (public/update.*)
                    ide jön a verzió info
        update.1    elsõ rész
                    ide jönnek a copy-k, az update-k és a delete-k
        update.2    záró rész

    1) Full verzió:
        a PublicFiles.txt-ben lévõ file-okat
        átmásoljuk a ram:cp4-be.
        spec: a cp4_src-vel jelzett dir-ben lévõ file-ok
            a fõkönyvtárban vannak.
    2) Másolandó file-ok:
        amelyek elõtt '+' jel áll a diff.out-ban
        ezeket másolni kell->
        készítünk belõlük egy listát az installer-nek,
        és a megfelelõ hierarchiában eltároljuk
        (copy.files)
    3) Törlendõ file-ok
        ezek elõtt '-' jel áll a diff.out-ban
        ezek az update során törlõdnek (del.files)
    4) Patchelendõ file-ok
        amelyek elõtt ' ' áll a diff.out-ban
        ezeket patchelni kell.
        készítünk belõlük egy 'patch.files'-t

    5) copy.files
        tartalmát a ram:cp4_update-ba másoljuk hierarchiával együtt,
        (a 'cp4' helyett a 'copy'-t hozzuk létre!)
        plusz hozzáfûzzük az 'update.i' file-hoz soronként:
        (Copy "copy/filenamepath-cp4" "path-cp4")
    6) del.files
        minden sorhoz egy bejegyzés az 'update.i'-be:
        (Delete "filenamepath-cp4")
    7) patch.files
        minden sorhoz:
        cmp ram:cp4_xxx/filenamepath-cp4 ram:cp4/filenamepath-cp4
        ha módosult:
            (Update "filenamepath-cp4")
            legeneráljuk a patch-et:
            gcompare ram:cp4_xxx/filenamepath-cp4 ram:cp4/filenamepath-cp4 ram:cp4_update/cp4.gpch

    8) hozzuk létre az 'update.v'-t:
        ;:ts=4      $VER: cp4.update V0.68 (dd.mm.yy) Copyright © by Gáti Gergely
        (set #Vo 0)     - elõzõ ver
        (set #Ro 63)    - elõzõ rev
        (set #Vn 0)     - új ver
        (set #Rn 68)    - új rev
        (set #dif x)
            x:      a lépésköz (810/ doing)
            doing:  a másolandó+patchelendõ+törlendõ file-ok száma
    9) cat update.v update.1 update.i update.2 >ram:cp4_update/cp4.update
        copy icon, gpatch
    10) compress all:
        makedir public/yyy, ahol yyy az új verzió
        lzx a ram:cp4 public/yyy/cp4.lzx
        lzx a ram:cp4_update public/yyy/cp4_update.lzx
    11) cleanup:
        delete ram:cp4          NEW
        delete ram:cp4_xxx      OLD
        delete ram:cp4_update   UPD

*/


char oldName[]="xxxxxxxx";          // "063"
int oldVer=0;                       // 0
int oldRev=0;                       // 63
char newName[]="xxxxxxxx";          // "069"
int newVer=0;                       // 0
int newRev=0;                       // 68
char appName[30];                   // "cp4"
int day,month,year;
int ProcessFileCount=0;             // hány file-t kell feldolgozni

/* beolvas egy sort, a file-ból, és ha kell levágja a végérõl a /n-t
 */
char *getLine(char *buf,int len,FILE *f) {
    char *ret;
    ret=fgets(buf,len,f);
    len=strlen(buf)-1;
    if(buf[len]=='\n') buf[len]='\0';
    return(ret);
}

/* lefuttat egy parancsot, és kiírja
 */
int run(char *com) {
    int ret;
    printf("%s\n",com);
#ifndef DEBUG
    ret=system(com);
#else
    ret=0;
#endif
    return(ret);
}


/*  keressük meg az elõzõ public release-t,
        Last_public_release.txt elsõ sora ='xxx'
        ez egy public/xxx/cp4.lzx file-lesz
    ret: 0-ok
        -1-err!!
*/
int getLastRelease(void) {
static char bf[10];
    FILE *f;
    int ret=-1;
    if(NULL!=(f=fopen("Last_public_release.dat","rb"))) {
        getLine(oldName,7,f);
        oldVer=atoi(getLine(bf,4,f));
        oldRev=atoi(getLine(bf,4,f));
        ret=0;
        fclose(f);
    }
    return(ret);
}


/*  keressük meg az új public release-t,
        cp4_ver.data elsõ sora = ver, második = rev
    ret: 0-ok
        -1-err!!
*/
int getNewRelease(void) {
static char bf[10];
static char name[100];
    FILE *f;
    int ret=-1;
    sprintf(name,"%s_ver.data",appName);
    if(NULL!=(f=fopen(name,"rb"))) {
        newVer=atoi(getLine(bf,4,f));
        newRev=atoi(getLine(bf,4,f));
        ret=0;
        fclose(f);
        sprintf(newName,"%d%d",newVer,newRev);
    }
    return(ret);
}


int copyFiles(char *fromtxt,char *pl) {
static char comm[300];
static char bf[300];
static char b2[300];
static char bp[300];
    FILE *f;
    int ret=-1,l;
    char *p;
    bp[0]='\0';
    l=strlen(appName)+1;
    if(NULL!=(f=fopen(fromtxt,"rb"))) {
        while(NULL!=getLine(bf,240,f)) {
            strcpy(b2,&bf[l]);
            if((p=strrchr(b2,'/'))!=NULL) {
                *p='\0';
                if(0!=strcmp(bp,b2)) {
                    sprintf(comm,"mkdir -p ram:%s%s/%s",appName,pl,b2);
                    run(comm);
                    strcpy(bp,b2);
                }
            }
            sprintf(comm,"copy clone %s ram:%s%s/%s\n",&bf[l],appName,pl,&bf[l]);
            run(comm);
        }
        ret=0;
        fclose(f);
    }
    return(ret);
}


/*  1) Full verzió:
    a PublicFiles.txt-ben lévõ file-okat
    átmásoljuk a ram:cp4-be.
    spec: a cp4_src-vel jelzett dir-ben lévõ file-ok
        a fõkönyvtárban vannak.
*/
int makeFull(void) {
    int ret;
    ret=copyFiles("PublicFiles.txt","");
    return(ret);
}


/*  2) Másolandó file-ok:
        amelyek elõtt '+' jel áll a diff.out-ban
        ezeket másolni kell->
        készítünk belõlük egy listát az installer-nek,
        és a megfelelõ hierarchiában eltároljuk
        (copy.files)
    3) Törlendõ file-ok
        ezek elõtt '-' jel áll a diff.out-ban
        ezek az update során törlõdnek (del.files)
    4) Patchelendõ file-ok
        amelyek elõtt ' ' áll a diff.out-ban
        ezeket patchelni kell.
        készítünk belõlük egy 'patch.files'-t
*/
int makeLists(void) {
static char comm[300];
static char bf[300];
    FILE *f;
    int ret=-1;
    if(NULL!=(f=fopen("ram:diff.out","rb"))) {
        run("echo >ram:copy.files");
        run("echo >ram:patch.files");
        run("echo >ram:del.files");
        while(NULL!=getLine(bf,240,f)) {
            if(*bf==*(bf+1)) continue;
            switch(*bf) {
                case '+' :
                    sprintf(comm,"echo \"%s\" >>ram:copy.files\n",&bf[1]);
                    break;
                case '-' :
                    sprintf(comm,"echo \"%s\" >>ram:del.files\n",&bf[1]);
                    break;
                case ' ' :
                    sprintf(comm,"echo \"%s\" >>ram:patch.files\n",&bf[1]);
                    break;
            }
            run(comm);
        }
        ret=0;
        fclose(f);
    }
    return(ret);
}


/*  5) copy.files
        tartalmát a ram:cp4_update-ba másoljuk hierarchiával együtt,
        (a 'cp4' helyett a 'copy'-t hozzuk létre!)
        plusz hozzáfûzzük az 'update.i' file-hoz soronként:
        (Copy "copy/filenamepath-cp4" "path-cp4")
*/
int updateCopy(void) {
static char comm[300];
static char bf[300];
    int ret,l;
    FILE *f;
    char *b;

    sprintf(comm,"makedir ram:%s_update/copy",appName);
    run(comm);
    ret=copyFiles("ram:copy.files","_update/copy");
    if(NULL!=(f=fopen("ram:copy.files","rb"))) {
        l=strlen(appName)+1;
        while(NULL!=getLine(bf,240,f)) {
            b=&bf[l];
            sprintf(comm,"echo >>ram:update.i (P_Copy 'copy/%s' '%s')",b,b);
            run(comm);
            ProcessFileCount++;
        }
        fclose(f);
        run("echo >>ram:update.i");
    } else ret=-1;
    return(ret);
}


/*  6) del.files
        minden sorhoz egy bejegyzés az 'update.i'-be:
        (Delete "filenamepath-cp4")
*/
int updateDel(void) {
static char comm[300];
static char bf[300];
    int ret=-1,l;
    FILE *f;
    char *b;

    if(NULL!=(f=fopen("ram:del.files","rb"))) {
        l=strlen(appName)+1;
        while(NULL!=getLine(bf,240,f)) {
            b=&bf[l];
            sprintf(comm,"echo >>ram:update.i (P_Delete '%s')",b);
            run(comm);
            ProcessFileCount++;
        }
        fclose(f);
        ret=0;
        run("echo >>ram:update.i");
    }
    return(ret);
}


/*  7) patch.files
        minden sorhoz:
        cmp ram:cp4_xxx/filenamepath-cp4 ram:cp4/filenamepath-cp4
        ha módosult: ('update.i')
            (Update "filenamepath-cp4")
            legeneráljuk a patch-et:
            gcompare ram:cp4_xxx/filenamepath-cp4 ram:cp4/filenamepath-cp4 ram:cp4_update/cp4.gpch
*/
int updatePatch(void) {
static char comm[300];
static char bf[300];
    int ret=-1,l,r;
    FILE *f;
    char *b;

    if(NULL!=(f=fopen("ram:patch.files","rb"))) {
        l=strlen(appName)+1;
        while(NULL!=getLine(bf,240,f)) {
            b=&bf[l];
            sprintf(comm,"cmp -s ram:%s_%s/%s ram:%s/%s",appName,oldName,b,appName,b);
            r=run(comm);
            if(r!=0) {
                sprintf(comm,"echo >>ram:update.i (P_Update '%s')",b);
                run(comm);
                ProcessFileCount++;
                sprintf(comm,"gcompare >NIL: ram:%s_%s/%s ram:%s/%s ram:%s_update/%s.gpch MODE=1 VARY=0 DEEP",appName,oldName,b,appName,b,appName,appName);
                run(comm);
            }
        }
        fclose(f);
        ret=0;
        run("echo >>ram:update.i");
    }
    return(ret);
}

int main(int argc, char **argv) {
static char comm[300];
    time_t tval;
    struct tm *lt;

    /* get time
     */
    time(&tval);
    lt=localtime(&tval);
    day=lt->tm_mday;
    month=lt->tm_mon+1;
    year=lt->tm_year;

    /* get appname
     */
    if(argc<2) return(0);
    strcpy(appName,argv[1]);

    /*  aktuális dir
     */
    chdir("PROGDIR:");
    /*  keressük meg az elõzõ public release-t,
            Last_public_release.dat elsõ sora ='xxx'
            ez egy public/xxx/cp4.lzx file-lesz
    */
    if(argc>3) {
        oldVer=atoi(argv[2]);
        oldRev=atoi(argv[3]);
        sprintf(oldName,"%d%d",oldVer,oldRev);
    } else getLastRelease();

    /*  ezt tömörítsük ki a ram-ra
            unlzx x public/xxx/cp4.lzx ram:
     */
    fprintf(stderr,"%s: Uncompressing the last public release (V%d.%d)\n",argv[0],oldVer,oldRev);
    sprintf(comm,"unlzx >NIL: x public/%s/%s_%s.lzx ram:",oldName,appName,oldName);
    run(comm);
    sprintf(comm,"rename ram:%s_%s ram:%s",appName,oldName,appName);
    run(comm);
    /*  kérjünk róla egy listát:
            cd ram:
            list cp4 all files lformat="%p%n" >ram:filelist.old
            sort ram:filelist.old ram:filelist_old.srt
    */
    chdir("ram:");
    sprintf(comm,"list %s all files lformat=\"%%p%%n\" >ram:filelist.old",appName);
    run(comm);
    chdir("PROGDIR:");
    run("sort ram:filelist.old ram:filelist_old.srt");
    /*  kérjük a mostani (PublicFiles.txt)
            és a régi különbségét:
            sort PublicFiles.txt ram:filelist_new.srt
            diff --old-line-format='-%L' ram:filelist_old.srt ram:filelist_new.srt >ram:diff.out
    */
    run("sort PublicFiles.txt ram:filelist_new.srt");
    run("diff --old-line-format \"-%L\" --new-line-format \"+%L\" --unchanged-line-format \" %L\" ram:filelist_old.srt ram:filelist_new.srt >ram:diff.out");
    /*  a ram:cp4-et nevezzük át ram:cp4_xxx-re,
        hozzunk létre a ram:cp4-et (new full)
        hozzunk létre a ram:cp4_update-t (new upd)
    */
    sprintf(comm,"rename ram:%s ram:%s_%s",appName,appName,oldName);
    run(comm);
    sprintf(comm,"makedir ram:%s",appName);
    run(comm);
    sprintf(comm,"makedir ram:%s_update",appName);
    run(comm);

    /*  1) Full verzió:
            a PublicFiles.txt-ben lévõ file-okat
            átmásoljuk a ram:cp4-be.
            spec: a cp4_src-vel jelzett dir-ben lévõ file-ok
                a fõkönyvtárban vannak.
    */
    getNewRelease();
    fprintf(stderr,"%s: Create new public release (V%d.%d)\n",argv[0],newVer,newRev);
    makeFull();
    /*  2) Másolandó file-ok:
            amelyek elõtt '+' jel áll a diff.out-ban
            ezeket másolni kell->
            készítünk belõlük egy listát az installer-nek,
            és a megfelelõ hierarchiában eltároljuk
            (copy.files)
        3) Törlendõ file-ok
            ezek elõtt '-' jel áll a diff.out-ban
            ezek az update során törlõdnek (del.files)
        4) Patchelendõ file-ok
            amelyek elõtt ' ' áll a diff.out-ban
            ezeket patchelni kell.
            készítünk belõlük egy 'patch.files'-t
    */
    fprintf(stderr,"%s: Create file lists\n",argv[0]);
    makeLists();

    /*  5) copy.files
            tartalmát a ram:cp4_update-ba másoljuk hierarchiával együtt,
            (a 'cp4' helyett a 'copy'-t hozzuk létre!)
            plusz hozzáfûzzük az 'update.i' file-hoz soronként:
            (Copy "copy/filenamepath-cp4" "path-cp4")
        6) del.files
            minden sorhoz egy bejegyzés az 'update.i'-be:
            (Delete "filenamepath-cp4")
        7) patch.files
            minden sorhoz:
            cmp ram:cp4_xxx/filenamepath-cp4 ram:cp4/filenamepath-cp4
            ha módosult: ('update.i')
                (Update "filenamepath-cp4")
                legeneráljuk a patch-et:
                gcompare ram:cp4_xxx/filenamepath-cp4 ram:cp4/filenamepath-cp4 ram:cp4_update/cp4.gpch
    */
    fprintf(stderr,"%s: Create update distribution\n",argv[0]);
    run("echo >ram:update.i");
    ProcessFileCount=0;
    updateDel();
    updatePatch();
    updateCopy();

    /*  8) hozzuk létre az 'update.v'-t:
            ;:ts=4      $VER: cp4.update V0.68 (dd.mm.yy) Copyright © by Gáti Gergely
            (set #Vo 0)     - elõzõ ver
            (set #Ro 63)    - elõzõ rev
            (set #Vn 0)     - új ver
            (set #Rn 68)    - új rev
            (set #dif x)
                x:      a lépésköz (810/ doing)
                doing:  a másolandó+patchelendõ+törlendõ file-ok száma
        9) cat update.v update.1 update.i update.2 >ram:cp4_update/cp4.update
            copy icon, gpatch
    */
    sprintf(comm,"echo >ram:update.v \";:ts=4\tMachine generated script\"");
    run(comm);
    sprintf(comm,"echo >>ram:update.v \";\"");
    run(comm);
    sprintf(comm,"echo >>ram:update.v \";\t$VER: %s.update V%d.%d (%d.%d.%d) Copyright © by %s\"",appName,newVer,newRev,day,month,year%100,author);
    run(comm);
    sprintf(comm,"echo >>ram:update.v \"(set #Vo %d)\"",oldVer);
    run(comm);
    sprintf(comm,"echo >>ram:update.v \"(set #Ro %d)\"",oldRev);
    run(comm);
    sprintf(comm,"echo >>ram:update.v \"(set #Vn %d)\"",newVer);
    run(comm);
    sprintf(comm,"echo >>ram:update.v \"(set #Rn %d)\"",newRev);
    run(comm);
    sprintf(comm,"echo >>ram:update.v \"(set #dif %d)\"",(930/ProcessFileCount));
    run(comm);
    run("echo >>ram:update.v");
    sprintf(comm,"type ram:update.v public/update.1 ram:update.i public/update.2 >ram:%s_update/%s.update",appName,appName);
    run(comm);
    sprintf(comm,"copy clone public/gpatch ram:%s_update",appName);
    run(comm);
    sprintf(comm,"copy clone public/Uninstall ram:%s_update",appName);
    run(comm);
    sprintf(comm,"copy clone public/Uninstall_icon ram:%s_update",appName);
    run(comm);
    sprintf(comm,"copy clone public/update.info ram:%s_update/%s.update.info",appName,appName);
    run(comm);
    sprintf(comm,"copy clone %s.readme ram:%s_update",appName,appName);
    run(comm);
    sprintf(comm,"copy clone %s_#?.readme ram:%s_update",appName,appName);
    run(comm);
    sprintf(comm,"echo >ram:%s_update/V%d.%d-V%d.%d \"Only for these versions!\"",appName,oldVer,oldRev,newVer,newRev);
    run(comm);

    fprintf(stderr,"%s: Compress new release\n",argv[0]);
    /*  10) compress all:
            makedir public/yyy, ahol yyy az új verzió
            lzx a ram:cp4 public/yyy/cp4.lzx
            lzx a ram:cp4_update public/yyy/cp4_update.lzx
    */
    sprintf(comm,"makedir public/%s",newName);
    run(comm);
    sprintf(comm,"copy clone %s.readme public/%s",appName,newName);
    run(comm);
    sprintf(comm,"copy clone %s.readme public/%s/%s_update.readme",appName,newName,appName);
    run(comm);
    sprintf(comm,"copy clone public/drawer.info ram:%s.info",appName);
    run(comm);
    sprintf(comm,"copy clone public/drawer.info ram:%s_update.info",appName);
    run(comm);
    sprintf(comm,"lha >NIL: -x -r a public/%s/%s.lha ram:%s ram:%s.info",newName,appName,appName,appName);
    run(comm);
    sprintf(comm,"lha >NIL: -x -r a public/%s/%s_update.lha ram:%s_update ram:%s_update.info",newName,appName,appName,appName);
    run(comm);
    sprintf(comm,"rename ram:%s ram:%s_%s",appName,appName,newName);
    run(comm);
    sprintf(comm,"rename ram:%s_update ram:%s_%s_update",appName,appName,newName);
    run(comm);
    sprintf(comm,"lzx >NIL: -3 -r -R -e a public/%s/%s_%s.lzx ram:%s_%s/#?",newName,appName,newName,appName,newName);
    run(comm);
    sprintf(comm,"lzx >NIL: -3 -r -R -e a public/%s/%s_%s_update.lzx ram:%s_%s_update/#?",newName,appName,newName,appName,newName);
    run(comm);

    /*  10.b) create source release
    */
    fprintf(stderr,"%s: Create source archive\n",argv[0]);
    sprintf(comm,"makedir ram:%s_src",appName);
    run(comm);
    copyFiles("SourceFiles.txt","_src");
    sprintf(comm,"rename ram:%s_src ram:%s_%s_src",appName,appName,newName);
    run(comm);
    sprintf(comm,"lzx >NIL: -3 -r -R -e a public/%s/%s_%s_src.lzx ram:%s_%s_src/#?",newName,appName,newName,appName,newName);
    run(comm);

    /*  11) cleanup:
            delete ram:cp4          NEW
            delete ram:cp4_xxx      OLD
            delete ram:cp4_update   UPD
    */
    fprintf(stderr,"%s: Cleanup\n",argv[0]);
    sprintf(comm,"delete all force ram:%s_%s",appName,newName);
    run(comm);
    sprintf(comm,"delete all force ram:%s_%s_src",appName,newName);
    run(comm);
    sprintf(comm,"delete all force ram:%s_%s",appName,oldName);
    run(comm);
    sprintf(comm,"delete all force ram:%s_%s_update",appName,newName);
    run(comm);
    run("delete ram:update.i");
    run("delete ram:update.v");
    run("delete ram:copy.files");
    run("delete ram:del.files");
    run("delete ram:patch.files");
    run("delete ram:filelist.old");
    run("delete ram:filelist_old.srt");
    run("delete ram:filelist_new.srt");
    run("delete ram:diff.out");
    sprintf(comm,"delete ram:%s.info",appName);
    run(comm);
    sprintf(comm,"delete ram:%s_update.info",appName);
    run(comm);

    /* update new public release
     */
    sprintf(comm,"echo >Last_public_release.dat \"%s\"",newName);
    run(comm);
    sprintf(comm,"echo >>Last_public_release.dat \"%d\"",newVer);
    run(comm);
    sprintf(comm,"echo >>Last_public_release.dat \"%d\"",newRev);
    run(comm);
    return(0);
}

