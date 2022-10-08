#ifndef CP4_MACROS_H
#define CP4_MACROS_H

#ifdef AMIGA
  #include "exec/types.h"
#else
  #define VOID void
  typedef signed char    BYTE;
  typedef unsigned char  UBYTE;
  typedef signed short   WORD;
  typedef unsigned short UWORD;
  typedef signed long    LONG;
  typedef unsigned long  ULONG;
  typedef short          BOOL;
  typedef char *         STRPTR;
  typedef void *         APTR;
#endif

#ifdef __SASC
  #define REG(rn, parm) register __ ## rn parm
  #define REGARGS __asm
  #define STDARGS __stdargs
  #define SAVEDS __saveds
  #define ALIGNED __aligned
  #define INLINE __inline
#endif

#ifdef __GNUC__
  #define REG(rn, parm) parm __asm(#rn)
  #define REGARGS __attribute__((regparm(4)))
  #define STDARGS __attribute__((stkparm))
  #define SAVEDS __attribute__((saveds))
  #define ALIGNED __attribute__((aligned(4)))
  #define INLINE __inline__
#endif

#endif
