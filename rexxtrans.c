/*
 *  Rexx/Trans
 *  Copyright (C) 1998-2000  Mark Hessling   <M.Hessling@qut.com.au>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * $Id: rexxtrans.c,v 1.14 2002/12/11 09:10:06 mark Exp $
 */
#if defined(HAVE_CONFIG_H)
# include "config.h"
#else
# include "defines.h"
#endif

#include <stdio.h>

#define INCL_REXXSAA

#if defined(WIN32)
# include <windows.h>
# define DYNAMIC_WIN32
#endif

#if defined(OS2)
# define INCL_DOSMODULEMGR
# define INCL_DOSMISC
# undef INCL_REXXSAA
# include <os2.h>
# define INCL_REXXSAA
# define DONT_TYPEDEF_PFN
# define DYNAMIC_OS2
#endif

#if defined(HAVE_ERRNO_H)
# include <errno.h>
#endif

#if defined(HAVE_MEMORY_H)
# include <memory.h>
#endif

#if defined(HAVE_STDARG_H)
# include <stdarg.h>
#endif

#if defined(HAVE_STDLIB_H)
# include <stdlib.h>
#endif

#if defined(HAVE_STRING_H)
# include <string.h>
#endif

#if defined(HAVE_SYS_TYPES_H)
# include <sys/types.h>
#endif

#if defined(HAVE_SYS_STAT_H)
# include <sys/stat.h>
#endif

#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#include <rexxtrans.h>

#ifdef UNIX
/*
 * Most Unix systems have dlopen(), so set this as the default and
 * unset it for platforms that don't have it - except for HPUX
 */
# if defined(__hpux)
#  define DYNAMIC_HPSHLOAD
# endif

# if defined(HAVE_DLFCN_H) && !defined(DYNAMIC_HPSHLOAD)
#  define DYNAMIC_DLOPEN
# endif

# if defined(DYNAMIC_DLOPEN)
#  include <dlfcn.h>
   typedef void *handle_type ;
#  ifndef RTLD_LAZY
#   define RTLD_LAZY 1
#  endif
# elif defined(DYNAMIC_AIXLOAD)
#  include <sys/ldr.h>
   typedef int (*handle_type) ;

# elif defined(DYNAMIC_HPSHLOAD)
#  include <dl.h>
   typedef shl_t handle_type ;

# elif defined(__BEOS__)
/*
 * BeOS add-ons are all in a directory specified in $ADDON_PATH
 */
#  include <be/kernel/image.h>
#  define DYNAMIC_BEOS  1
   typedef image_id handle_type ;
# endif

#elif defined(DYNAMIC_OS2)
   typedef HMODULE handle_type ;

#elif defined(DYNAMIC_WIN32)
   typedef HINSTANCE handle_type ;
#endif

#define FUNCTION_REXXSTART                     0
#define FUNCTION_REXXREGISTEREXITEXE           1
#define FUNCTION_REXXREGISTEREXITDLL           2
#define FUNCTION_REXXDEREGISTEREXIT            3
#define FUNCTION_REXXQUERYEXIT                 4
#define FUNCTION_REXXREGISTERSUBCOMEXE         5
#define FUNCTION_REXXDEREGISTERSUBCOM          6
#define FUNCTION_REXXREGISTERSUBCOMDLL         7
#define FUNCTION_REXXQUERYSUBCOM               8
#define FUNCTION_REXXREGISTERFUNCTIONEXE       9
#define FUNCTION_REXXREGISTERFUNCTIONDLL      10
#define FUNCTION_REXXDEREGISTERFUNCTION       11
#define FUNCTION_REXXQUERYFUNCTION            12
#define FUNCTION_REXXVARIABLEPOOL             13
#define FUNCTION_REXXALLOCATEMEMORY           14
#define FUNCTION_REXXFREEMEMORY               15
#define FUNCTION_REXXCREATEQUEUE              16
#define FUNCTION_REXXDELETEQUEUE              17
#define FUNCTION_REXXADDQUEUE                 18
#define FUNCTION_REXXPULLQUEUE                19
#define FUNCTION_REXXQUERYQUEUE               20
#define FUNCTION_REXXADDMACRO                 21
#define FUNCTION_REXXDROPMACRO                22
#define FUNCTION_REXXSAVEMACROSPACE           23
#define FUNCTION_REXXLOADMACROSPACE           24
#define FUNCTION_REXXQUERYMACRO               25
#define FUNCTION_REXXREORDERMACRO             26
#define FUNCTION_REXXCLEARMACROSPACE          27
#define FUNCTION_REXXSETHALT                  28
#define FUNCTION_REXXWAITFORTERMINATION       29
#define FUNCTION_REXXDIDREXXTERMINATE         30

#define NUM_REXX_FUNCTIONS                    31

#ifdef MODULES_NEED_USCORE
# define RX_USCORE "_"
#else
# define RX_USCORE
#endif

static char *MyFunctionName[ NUM_REXX_FUNCTIONS ] =
{
   /*  0 */  RX_USCORE "RexxStart",
   /*  1 */  RX_USCORE "RexxRegisterExitExe",
   /*  2 */  RX_USCORE "RexxRegisterExitDll",
   /*  3 */  RX_USCORE "RexxDeregisterExit",
   /*  4 */  RX_USCORE "RexxQueryExit",
   /*  5 */  RX_USCORE "RexxRegisterSubcomExe",
   /*  6 */  RX_USCORE "RexxDeregisterSubcom",
   /*  7 */  RX_USCORE "RexxRegisterSubcomDll",
   /*  8 */  RX_USCORE "RexxQuerySubcom",
   /*  9 */  RX_USCORE "RexxRegisterFunctionExe",
   /* 10 */  RX_USCORE "RexxRegisterFunctionDll",
   /* 11 */  RX_USCORE "RexxDeregisterFunction",
   /* 12 */  RX_USCORE "RexxQueryFunction",
   /* 13 */  RX_USCORE "RexxVariablePool",
   /* 14 */  RX_USCORE "RexxAllocateMemory",
   /* 15 */  RX_USCORE "RexxFreeMemory",
   /* 16 */  RX_USCORE "RexxCreateQueue",
   /* 17 */  RX_USCORE "RexxDeleteQueue",
   /* 18 */  RX_USCORE "RexxQueryQueue",
   /* 19 */  RX_USCORE "RexxAddQueue",
   /* 20 */  RX_USCORE "RexxPullQueue",
   /* 21 */  RX_USCORE "RexxAddMacro",
   /* 22 */  RX_USCORE "RexxDropMacro",
   /* 23 */  RX_USCORE "RexxSaveMacroSpace",
   /* 24 */  RX_USCORE "RexxLoadMacroSpace",
   /* 25 */  RX_USCORE "RexxQueryMacro",
   /* 26 */  RX_USCORE "RexxReorderMacro",
   /* 27 */  RX_USCORE "RexxClearMacroSpace",
   /* 28 */  RX_USCORE "RexxSetHalt",
   /* 29 */  RX_USCORE "RexxWaitForTermination",
   /* 30 */  RX_USCORE "RexxDidRexxTerminate",
};

/*
 * These must correspond to the position in the MyLibraryName array.
 */
#ifdef WIN32
# define INT_REGINA         0
# define INT_OBJECT_REXX    1
# define INT_PERSONAL_REXX  2
# define INT_WINREXX        3
# define INT_UNI_REXX       4
# define INT_REGINALD       5
#endif

#ifdef OS2
# define INT_REGINA         0
# define INT_OBJECT_REXX    1
# define INT_REGINALD       -1
#endif

#ifdef UNIX
# define INT_REGINA         0
# define INT_REXX_IMC       1
# define INT_OBJECT_REXX    2
# define INT_UNI_REXX       3
# define INT_REGINALD       -1
#endif

static char *MyLibraryName[] =
{
#ifdef WIN32
   "REGINA",        /* Regina */
   "REXXAPI",       /* Object Rexx */
   "WREXX32",       /* Quercus */
   "RXREXX",        /* Enterprise */
   "RXX",           /* uni-REXX - not included as doesn't conform*/
   "REGINALD",      /* Reginald/
#endif
#ifdef OS2
   "REGINA",        /* Regina */
   "REXXAPI",       /* Classic and Object Rexx */
#endif
#ifdef UNIX
   "regina" ,        /* Regina */
   "rexx" ,          /* REXX/imc */
   "rexxapi" ,       /* Object Rexx */
   "rx_s" ,          /* uni-REXX */
#endif
   NULL
};

static char *MyInterpreterName[] =
{
#ifdef WIN32
   "regina",      /* Regina */
   "objectrexx",  /* Object Rexx */
   "personalrexx",/* Quercus */
   "winrexx",     /* Enterprise */
   "unirexx",     /* uni-REXX */
   "reginald",    /* Reginald */
#endif
#ifdef OS2
   "regina",      /* Regina */
   "objectrexx",  /* OS/2 Rexx */
#endif
#ifdef UNIX
   "regina" ,     /* Regina */
   "rexximc" ,    /* REXX/imc */
   "objectrexx" , /* Object Rexx */
   "unirexx" ,    /* uni-REXX */
#endif
   NULL
};
#ifdef UNIX
# define OBJECT_REXX_EXTRA_DLL "rexx"
#else
# define OBJECT_REXX_EXTRA_DLL "REXX"
#endif

/*
 * Typedefs for Regina and Reginald
 */
#if defined(INT_REGINA) || defined(INT_REGINALD)
typedef APIRET APIENTRY RREXXSTART(LONG,PRXSTRING,PSZ,PRXSTRING,PSZ,LONG,PRXSYSEXIT,PSHORT,PRXSTRING );
typedef APIRET APIENTRY RREXXREGISTEREXITEXE(PSZ,PFN,PUCHAR ) ;
typedef APIRET APIENTRY RREXXREGISTEREXITDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG ) ;
typedef APIRET APIENTRY RREXXDEREGISTEREXIT(PSZ,PSZ ) ;
typedef APIRET APIENTRY RREXXQUERYEXIT(PSZ,PSZ,PUSHORT,PUCHAR ) ;
typedef APIRET APIENTRY RREXXREGISTERSUBCOMEXE(PSZ,PFN,PUCHAR ) ;
typedef APIRET APIENTRY RREXXDEREGISTERSUBCOM(PSZ,PSZ ) ;
typedef APIRET APIENTRY RREXXREGISTERSUBCOMDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG ) ;
typedef APIRET APIENTRY RREXXQUERYSUBCOM(PSZ,PSZ,PUSHORT,PUCHAR ) ;
typedef APIRET APIENTRY RREXXREGISTERFUNCTIONEXE(PSZ,PFN ) ;
typedef APIRET APIENTRY RREXXREGISTERFUNCTIONDLL(PSZ,PSZ,PSZ ) ;
typedef APIRET APIENTRY RREXXDEREGISTERFUNCTION(PSZ) ;
typedef APIRET APIENTRY RREXXQUERYFUNCTION(PSZ) ;
typedef APIRET APIENTRY RREXXVARIABLEPOOL(PSHVBLOCK );
typedef PVOID  APIENTRY RREXXALLOCATEMEMORY( ULONG );
typedef APIRET APIENTRY RREXXFREEMEMORY( PVOID );
typedef APIRET APIENTRY RREXXCREATEQUEUE(PSZ,ULONG,PSZ,ULONG* ) ;
typedef APIRET APIENTRY RREXXDELETEQUEUE(PSZ ) ;
typedef APIRET APIENTRY RREXXQUERYQUEUE(PSZ,ULONG* ) ;
typedef APIRET APIENTRY RREXXADDQUEUE( PSZ,PRXSTRING,ULONG ) ;
typedef APIRET APIENTRY RREXXPULLQUEUE(PSZ,PRXSTRING,PDATETIME,ULONG );
typedef APIRET APIENTRY RREXXADDMACRO(PSZ,PSZ,ULONG );
typedef APIRET APIENTRY RREXXDROPMACRO(PSZ );
typedef APIRET APIENTRY RREXXSAVEMACROSPACE(ULONG,PSZ *,PSZ);
typedef APIRET APIENTRY RREXXLOADMACROSPACE(ULONG ,PSZ *,PSZ);
typedef APIRET APIENTRY RREXXQUERYMACRO(PSZ,PUSHORT );
typedef APIRET APIENTRY RREXXREORDERMACRO(PSZ,ULONG );
typedef APIRET APIENTRY RREXXCLEARMACROSPACE(VOID );
typedef APIRET APIENTRY RREXXSETHALT(LONG,LONG );
typedef void   APIENTRY RREXXWAITFORTERMINATION(VOID);
typedef APIRET APIENTRY RREXXDIDREXXTERMINATE(VOID);
RREXXSTART               *RRexxStart=NULL;
RREXXREGISTEREXITEXE     *RRexxRegisterExitExe=NULL;
RREXXREGISTEREXITDLL     *RRexxRegisterExitDll=NULL;
RREXXDEREGISTEREXIT      *RRexxDeregisterExit=NULL;
RREXXQUERYEXIT           *RRexxQueryExit=NULL;
RREXXREGISTERSUBCOMEXE   *RRexxRegisterSubcomExe=NULL;
RREXXDEREGISTERSUBCOM    *RRexxDeregisterSubcom=NULL;
RREXXREGISTERSUBCOMDLL   *RRexxRegisterSubcomDll=NULL;
RREXXQUERYSUBCOM         *RRexxQuerySubcom=NULL;
RREXXREGISTERFUNCTIONEXE *RRexxRegisterFunctionExe=NULL;
RREXXREGISTERFUNCTIONDLL *RRexxRegisterFunctionDll=NULL;
RREXXDEREGISTERFUNCTION  *RRexxDeregisterFunction=NULL;
RREXXQUERYFUNCTION       *RRexxQueryFunction=NULL;
RREXXVARIABLEPOOL        *RRexxVariablePool=NULL;
RREXXALLOCATEMEMORY      *RRexxAllocateMemory=NULL;
RREXXFREEMEMORY          *RRexxFreeMemory=NULL;
RREXXCREATEQUEUE         *RRexxCreateQueue=NULL;
RREXXDELETEQUEUE         *RRexxDeleteQueue=NULL;
RREXXQUERYQUEUE          *RRexxQueryQueue=NULL;
RREXXADDQUEUE            *RRexxAddQueue=NULL;
RREXXPULLQUEUE           *RRexxPullQueue=NULL;
RREXXADDMACRO            *RRexxAddMacro=NULL;
RREXXDROPMACRO           *RRexxDropMacro=NULL;
RREXXSAVEMACROSPACE      *RRexxSaveMacroSpace=NULL;
RREXXLOADMACROSPACE      *RRexxLoadMacroSpace=NULL;
RREXXQUERYMACRO          *RRexxQueryMacro=NULL;
RREXXREORDERMACRO        *RRexxReorderMacro=NULL;
RREXXCLEARMACROSPACE     *RRexxClearMacroSpace=NULL;
RREXXSETHALT             *RRexxSetHalt=NULL;
RREXXWAITFORTERMINATION  *RRexxWaitForTermination=NULL;           /* N/A */
RREXXDIDREXXTERMINATE    *RRexxDidRexxTerminate=NULL;             /* N/A */
#endif

/*
 * Typedefs for REXX/imc
 */
#if defined(INT_REXX_IMC)
typedef APIRET APIENTRY IREXXSTART(LONG,PRXSTRING,PSZ,PRXSTRING,PSZ,LONG,PRXSYSEXIT,PSHORT,PRXSTRING );
typedef APIRET APIENTRY IREXXREGISTEREXITEXE(PSZ,RexxExitHandler*,PUCHAR ) ;
typedef APIRET APIENTRY IREXXREGISTEREXITDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG ) ; /* N/A */
typedef APIRET APIENTRY IREXXDEREGISTEREXIT(PSZ,PSZ ) ;
typedef APIRET APIENTRY IREXXQUERYEXIT(PSZ,PSZ,PUSHORT,PUCHAR ) ;
typedef APIRET APIENTRY IREXXREGISTERSUBCOMEXE(PSZ,RexxSubcomHandler*,PUCHAR ) ;
typedef APIRET APIENTRY IREXXDEREGISTERSUBCOM(PSZ,PSZ ) ;
typedef APIRET APIENTRY IREXXREGISTERSUBCOMDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG ) ; /* N/A */
typedef APIRET APIENTRY IREXXQUERYSUBCOM(PSZ,PSZ,PUSHORT,PUCHAR ) ;
typedef APIRET APIENTRY IREXXREGISTERFUNCTIONEXE(PSZ,RexxFunctionHandler* ) ;
typedef APIRET APIENTRY IREXXREGISTERFUNCTIONDLL(PSZ,PSZ,PSZ ) ;
typedef APIRET APIENTRY IREXXDEREGISTERFUNCTION(PSZ) ;
typedef APIRET APIENTRY IREXXQUERYFUNCTION(PSZ) ;
typedef APIRET APIENTRY IREXXVARIABLEPOOL(PSHVBLOCK );
typedef PVOID  APIENTRY IREXXALLOCATEMEMORY( ULONG );
typedef APIRET APIENTRY IREXXFREEMEMORY( PVOID );
typedef APIRET APIENTRY IREXXCREATEQUEUE(PSZ,ULONG,PSZ,ULONG* ) ; /* N/A */
typedef APIRET APIENTRY IREXXDELETEQUEUE(PSZ ) ;                  /* N/A */
typedef APIRET APIENTRY IREXXQUERYQUEUE(PSZ,ULONG* ) ;            /* N/A */
typedef APIRET APIENTRY IREXXADDQUEUE( PSZ,PRXSTRING,ULONG ) ;    /* N/A */
typedef APIRET APIENTRY IREXXPULLQUEUE(PSZ,PRXSTRING,PDATETIME,ULONG ); /* N/A */
typedef APIRET APIENTRY IREXXADDMACRO(PSZ,PSZ,ULONG );            /* N/A */
typedef APIRET APIENTRY IREXXDROPMACRO(PSZ );                     /* N/A */
typedef APIRET APIENTRY IREXXSAVEMACROSPACE(ULONG,PSZ *,PSZ);     /* N/A */
typedef APIRET APIENTRY IREXXLOADMACROSPACE(ULONG ,PSZ *,PSZ);    /* N/A */
typedef APIRET APIENTRY IREXXQUERYMACRO(PSZ,PUSHORT );            /* N/A */
typedef APIRET APIENTRY IREXXREORDERMACRO(PSZ,ULONG );            /* N/A */
typedef APIRET APIENTRY IREXXCLEARMACROSPACE(VOID );              /* N/A */
typedef APIRET APIENTRY IREXXSETHALT( LONG,LONG);                 /* N/A */
typedef void   APIENTRY IREXXWAITFORTERMINATION(VOID);            /* N/A */
typedef APIRET APIENTRY IREXXDIDREXXTERMINATE(VOID);              /* N/A */
IREXXSTART               *IRexxStart=NULL;
IREXXREGISTEREXITEXE     *IRexxRegisterExitExe=NULL;
IREXXREGISTEREXITDLL     *IRexxRegisterExitDll=NULL;
IREXXDEREGISTEREXIT      *IRexxDeregisterExit=NULL;
IREXXQUERYEXIT           *IRexxQueryExit=NULL;
IREXXREGISTERSUBCOMEXE   *IRexxRegisterSubcomExe=NULL;
IREXXDEREGISTERSUBCOM    *IRexxDeregisterSubcom=NULL;
IREXXREGISTERSUBCOMDLL   *IRexxRegisterSubcomDll=NULL;
IREXXQUERYSUBCOM         *IRexxQuerySubcom=NULL;
IREXXREGISTERFUNCTIONEXE *IRexxRegisterFunctionExe=NULL;
IREXXREGISTERFUNCTIONDLL *IRexxRegisterFunctionDll=NULL;
IREXXDEREGISTERFUNCTION  *IRexxDeregisterFunction=NULL;
IREXXQUERYFUNCTION       *IRexxQueryFunction=NULL;
IREXXVARIABLEPOOL        *IRexxVariablePool=NULL;
IREXXALLOCATEMEMORY      *IRexxAllocateMemory=NULL;
IREXXFREEMEMORY          *IRexxFreeMemory=NULL;
IREXXCREATEQUEUE         *IRexxCreateQueue=NULL;
IREXXDELETEQUEUE         *IRexxDeleteQueue=NULL;
IREXXQUERYQUEUE          *IRexxQueryQueue=NULL;
IREXXADDQUEUE            *IRexxAddQueue=NULL;
IREXXPULLQUEUE           *IRexxPullQueue=NULL;
IREXXADDMACRO            *IRexxAddMacro=NULL;
IREXXDROPMACRO           *IRexxDropMacro=NULL;
IREXXSAVEMACROSPACE      *IRexxSaveMacroSpace=NULL;
IREXXLOADMACROSPACE      *IRexxLoadMacroSpace=NULL;
IREXXQUERYMACRO          *IRexxQueryMacro=NULL;
IREXXREORDERMACRO        *IRexxReorderMacro=NULL;
IREXXCLEARMACROSPACE     *IRexxClearMacroSpace=NULL;
IREXXSETHALT             *IRexxSetHalt=NULL;
IREXXWAITFORTERMINATION  *IRexxWaitForTermination=NULL;
IREXXDIDREXXTERMINATE    *IRexxDidRexxTerminate=NULL;
#endif

/*
 * Typedefs for Object Rexx
 */
#if defined(INT_OBJECT_REXX)
typedef APIRET APIENTRY OREXXSTART(LONG,PRXSTRING,PSZ,PRXSTRING,PSZ,LONG,PRXSYSEXIT,PSHORT,PRXSTRING );
typedef APIRET APIENTRY OREXXREGISTEREXITEXE(PSZ,PFN,PUCHAR ) ;
typedef APIRET APIENTRY OREXXREGISTEREXITDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG ) ;
typedef APIRET APIENTRY OREXXDEREGISTEREXIT(PSZ,PSZ ) ;
typedef APIRET APIENTRY OREXXQUERYEXIT(PSZ,PSZ,PUSHORT,PUCHAR ) ;
typedef APIRET APIENTRY OREXXREGISTERSUBCOMEXE(PSZ,PFN,PUCHAR ) ;
typedef APIRET APIENTRY OREXXDEREGISTERSUBCOM(PSZ,PSZ ) ;
typedef APIRET APIENTRY OREXXREGISTERSUBCOMDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG ) ;
typedef APIRET APIENTRY OREXXQUERYSUBCOM(PSZ,PSZ,PUSHORT,PUCHAR ) ;
typedef APIRET APIENTRY OREXXREGISTERFUNCTIONEXE(PSZ,PFN ) ;
typedef APIRET APIENTRY OREXXREGISTERFUNCTIONDLL(PSZ,PSZ,PSZ ) ;
typedef APIRET APIENTRY OREXXDEREGISTERFUNCTION(PSZ) ;
typedef APIRET APIENTRY OREXXQUERYFUNCTION(PSZ) ;
typedef APIRET APIENTRY OREXXVARIABLEPOOL(PSHVBLOCK );
typedef PVOID  APIENTRY OREXXALLOCATEMEMORY( ULONG );
typedef APIRET APIENTRY OREXXFREEMEMORY( PVOID );
typedef APIRET APIENTRY OREXXCREATEQUEUE(PSZ,ULONG,PSZ,ULONG* ) ;
typedef APIRET APIENTRY OREXXDELETEQUEUE(PSZ ) ;
typedef APIRET APIENTRY OREXXQUERYQUEUE(PSZ,ULONG* ) ;
typedef APIRET APIENTRY OREXXADDQUEUE( PSZ,PRXSTRING,ULONG ) ;
typedef APIRET APIENTRY OREXXPULLQUEUE(PSZ,PRXSTRING,PDATETIME,ULONG );
typedef APIRET APIENTRY OREXXADDMACRO(PSZ,PSZ,ULONG );
typedef APIRET APIENTRY OREXXDROPMACRO(PSZ );
typedef APIRET APIENTRY OREXXSAVEMACROSPACE(ULONG,PSZ *,PSZ);
typedef APIRET APIENTRY OREXXLOADMACROSPACE(ULONG ,PSZ *,PSZ);
typedef APIRET APIENTRY OREXXQUERYMACRO(PSZ,PUSHORT );
typedef APIRET APIENTRY OREXXREORDERMACRO(PSZ,ULONG );
typedef APIRET APIENTRY OREXXCLEARMACROSPACE(VOID );
typedef APIRET APIENTRY OREXXSETHALT(LONG,LONG );
typedef void   APIENTRY OREXXWAITFORTERMINATION(VOID);
typedef APIRET APIENTRY OREXXDIDREXXTERMINATE(VOID);
OREXXSTART               *ORexxStart=NULL;
OREXXREGISTEREXITEXE     *ORexxRegisterExitExe=NULL;
OREXXREGISTEREXITDLL     *ORexxRegisterExitDll=NULL;
OREXXDEREGISTEREXIT      *ORexxDeregisterExit=NULL;
OREXXQUERYEXIT           *ORexxQueryExit=NULL;
OREXXREGISTERSUBCOMEXE   *ORexxRegisterSubcomExe=NULL;
OREXXDEREGISTERSUBCOM    *ORexxDeregisterSubcom=NULL;
OREXXREGISTERSUBCOMDLL   *ORexxRegisterSubcomDll=NULL;
OREXXQUERYSUBCOM         *ORexxQuerySubcom=NULL;
OREXXREGISTERFUNCTIONEXE *ORexxRegisterFunctionExe=NULL;
OREXXREGISTERFUNCTIONDLL *ORexxRegisterFunctionDll=NULL;
OREXXDEREGISTERFUNCTION  *ORexxDeregisterFunction=NULL;
OREXXQUERYFUNCTION       *ORexxQueryFunction=NULL;
OREXXVARIABLEPOOL        *ORexxVariablePool=NULL;
OREXXALLOCATEMEMORY      *ORexxAllocateMemory=NULL;
OREXXFREEMEMORY          *ORexxFreeMemory=NULL;
OREXXCREATEQUEUE         *ORexxCreateQueue=NULL;
OREXXDELETEQUEUE         *ORexxDeleteQueue=NULL;
OREXXQUERYQUEUE          *ORexxQueryQueue=NULL;
OREXXADDQUEUE            *ORexxAddQueue=NULL;
OREXXPULLQUEUE           *ORexxPullQueue=NULL;
OREXXADDMACRO            *ORexxAddMacro=NULL;
OREXXDROPMACRO           *ORexxDropMacro=NULL;
OREXXSAVEMACROSPACE      *ORexxSaveMacroSpace=NULL;
OREXXLOADMACROSPACE      *ORexxLoadMacroSpace=NULL;
OREXXQUERYMACRO          *ORexxQueryMacro=NULL;
OREXXREORDERMACRO        *ORexxReorderMacro=NULL;
OREXXCLEARMACROSPACE     *ORexxClearMacroSpace=NULL;
OREXXSETHALT             *ORexxSetHalt=NULL;
OREXXWAITFORTERMINATION  *ORexxWaitForTermination=NULL;
OREXXDIDREXXTERMINATE    *ORexxDidRexxTerminate=NULL;
#endif

/*
 * Typedefs for Quercus
 */
#if defined(INT_PERSONAL_REXX)
typedef SHORT FAR *LPSHORT;
typedef LONG APIENTRY   QREXXSTART(int,PRXSTRING,LPCSTR,PRXSTRING,LPCSTR,int,PRXSYSEXIT,LPSHORT,PRXSTRING );
typedef APIRET APIENTRY QREXXREGISTEREXITEXE(LPCSTR,FARPROC,LPBYTE );
typedef APIRET APIENTRY QREXXREGISTEREXITDLL(LPCSTR,LPCSTR,LPCSTR,LPBYTE,UINT );
typedef APIRET APIENTRY QREXXDEREGISTEREXIT(LPCSTR,LPCSTR );
typedef APIRET APIENTRY QREXXQUERYEXIT(LPCSTR,LPCSTR,LPWORD,LPBYTE ) ;
typedef APIRET APIENTRY QREXXREGISTERSUBCOMDLL(LPCSTR,LPCSTR,LPCSTR,LPBYTE,UINT );
typedef APIRET APIENTRY QREXXREGISTERSUBCOMEXE(LPCSTR,FARPROC,LPBYTE );
typedef APIRET APIENTRY QREXXQUERYSUBCOM(LPCSTR,LPCSTR,LPWORD,LPBYTE );
typedef APIRET APIENTRY QREXXDEREGISTERSUBCOM(LPCSTR,LPCSTR );
typedef APIRET APIENTRY QREXXREGISTERFUNCTIONDLL(LPCSTR,LPCSTR,LPCSTR);
typedef APIRET APIENTRY QREXXREGISTERFUNCTIONEXE(LPCSTR,FARPROC );
typedef APIRET APIENTRY QREXXDEREGISTERFUNCTION(LPCSTR );
typedef APIRET APIENTRY QREXXQUERYFUNCTION(LPCSTR );
typedef APIRET APIENTRY QREXXVARIABLEPOOL(PSHVBLOCK );
typedef PVOID  APIENTRY QREXXALLOCATEMEMORY( ULONG );
typedef APIRET APIENTRY QREXXFREEMEMORY( PVOID );
typedef APIRET APIENTRY QREXXCREATEQUEUE(PSZ,ULONG,PSZ,ULONG* ) ;        /* ?? */
typedef APIRET APIENTRY QREXXDELETEQUEUE(PSZ ) ;                         /* ?? */
typedef APIRET APIENTRY QREXXQUERYQUEUE(PSZ,ULONG* ) ;                   /* ?? */
typedef APIRET APIENTRY QREXXADDQUEUE( PCSZ,PRXSTRING,ULONG ) ;           /* ?? */
typedef APIRET APIENTRY QREXXPULLQUEUE(PSZ,PRXSTRING,PDATETIME,ULONG );  /* ?? */
typedef APIRET APIENTRY QREXXADDMACRO(PSZ,PSZ,ULONG );                   /* ?? */
typedef APIRET APIENTRY QREXXDROPMACRO(PSZ );                            /* ?? */
typedef APIRET APIENTRY QREXXSAVEMACROSPACE(ULONG,PSZ *,PSZ);            /* ?? */
typedef APIRET APIENTRY QREXXLOADMACROSPACE(ULONG ,PSZ *,PSZ);           /* ?? */
typedef APIRET APIENTRY QREXXQUERYMACRO(PSZ,PUSHORT );                   /* ?? */
typedef APIRET APIENTRY QREXXREORDERMACRO(PSZ,ULONG );                   /* ?? */
typedef APIRET APIENTRY QREXXCLEARMACROSPACE(VOID );                     /* ?? */
typedef APIRET APIENTRY QREXXSETHALT(LONG, LONG );                       /* ?? */
typedef void   APIENTRY QREXXWAITFORTERMINATION(VOID);
typedef APIRET APIENTRY QREXXDIDREXXTERMINATE(VOID);
QREXXSTART               *QRexxStart=NULL;
QREXXREGISTEREXITEXE     *QRexxRegisterExitExe=NULL;
QREXXREGISTEREXITDLL     *QRexxRegisterExitDll=NULL;
QREXXDEREGISTEREXIT      *QRexxDeregisterExit=NULL;
QREXXQUERYEXIT           *QRexxQueryExit=NULL;
QREXXREGISTERSUBCOMEXE   *QRexxRegisterSubcomExe=NULL;
QREXXDEREGISTERSUBCOM    *QRexxDeregisterSubcom=NULL;
QREXXREGISTERSUBCOMDLL   *QRexxRegisterSubcomDll=NULL;
QREXXQUERYSUBCOM         *QRexxQuerySubcom=NULL;
QREXXREGISTERFUNCTIONEXE *QRexxRegisterFunctionExe=NULL;
QREXXREGISTERFUNCTIONDLL *QRexxRegisterFunctionDll=NULL;
QREXXDEREGISTERFUNCTION  *QRexxDeregisterFunction=NULL;
QREXXQUERYFUNCTION       *QRexxQueryFunction=NULL;
QREXXVARIABLEPOOL        *QRexxVariablePool=NULL;
QREXXALLOCATEMEMORY      *QRexxAllocateMemory=NULL;
QREXXFREEMEMORY          *QRexxFreeMemory=NULL;
QREXXCREATEQUEUE         *QRexxCreateQueue=NULL;
QREXXDELETEQUEUE         *QRexxDeleteQueue=NULL;
QREXXQUERYQUEUE          *QRexxQueryQueue=NULL;
QREXXADDQUEUE            *QRexxAddQueue=NULL;
QREXXPULLQUEUE           *QRexxPullQueue=NULL;
QREXXADDMACRO            *QRexxAddMacro=NULL;
QREXXDROPMACRO           *QRexxDropMacro=NULL;
QREXXSAVEMACROSPACE      *QRexxSaveMacroSpace=NULL;
QREXXLOADMACROSPACE      *QRexxLoadMacroSpace=NULL;
QREXXQUERYMACRO          *QRexxQueryMacro=NULL;
QREXXREORDERMACRO        *QRexxReorderMacro=NULL;
QREXXCLEARMACROSPACE     *QRexxClearMacroSpace=NULL;
QREXXSETHALT             *QRexxSetHalt=NULL;
QREXXWAITFORTERMINATION  *QRexxWaitForTermination=NULL;
QREXXDIDREXXTERMINATE    *QRexxDidRexxTerminate=NULL;
#endif

/*
 * Typedefs for Enterprise
 */
#if defined(INT_WINREXX)
typedef int APIENTRY    WREXXSTART(int,PRXSTRING,LPCSTR,PRXSTRING,LPCSTR,int,PRXSYSEXIT,LPSHORT,PRXSTRING );
typedef APIRET APIENTRY WREXXREGISTEREXITDLL(LPCSTR,LPCSTR,LPCSTR,LPBYTE,UINT );
typedef APIRET APIENTRY WREXXREGISTEREXITEXE(LPCSTR,FARPROC,LPBYTE );
typedef APIRET APIENTRY WREXXDEREGISTEREXIT(LPCSTR,LPCSTR ) ;
typedef APIRET APIENTRY WREXXQUERYEXIT(LPCSTR,LPCSTR,LPWORD,LPBYTE ) ;
typedef APIRET APIENTRY WREXXREGISTERSUBCOMDLL(LPCSTR,LPCSTR,LPCSTR,LPBYTE,UINT );
typedef APIRET APIENTRY WREXXREGISTERSUBCOMEXE(LPCSTR,FARPROC,LPBYTE );
typedef APIRET APIENTRY WREXXQUERYSUBCOM(LPCSTR,LPCSTR,LPWORD,LPBYTE );
typedef APIRET APIENTRY WREXXDEREGISTERSUBCOM(LPCSTR,LPCSTR );
typedef APIRET APIENTRY WREXXREGISTERFUNCTIONDLL(LPCSTR,LPCSTR,LPCSTR);
typedef APIRET APIENTRY WREXXREGISTERFUNCTIONEXE(LPCSTR,FARPROC );
typedef APIRET APIENTRY WREXXDEREGISTERFUNCTION(LPCSTR );
typedef APIRET APIENTRY WREXXQUERYFUNCTION(LPCSTR );
typedef APIRET APIENTRY WREXXVARIABLEPOOL(PSHVBLOCK );
typedef PVOID  APIENTRY WREXXALLOCATEMEMORY( ULONG );
typedef APIRET APIENTRY WREXXFREEMEMORY( PVOID );
typedef APIRET APIENTRY WREXXCREATEQUEUE(PSZ,ULONG,PSZ,ULONG* ) ;        /* ?? */
typedef APIRET APIENTRY WREXXDELETEQUEUE(PSZ ) ;                         /* ?? */
typedef APIRET APIENTRY WREXXQUERYQUEUE(PSZ,ULONG* ) ;                   /* ?? */
typedef APIRET APIENTRY WREXXADDQUEUE( PCSZ,PRXSTRING,ULONG ) ;           /* ?? */
typedef APIRET APIENTRY WREXXPULLQUEUE(PSZ,PRXSTRING,PDATETIME,ULONG );  /* ?? */
typedef APIRET APIENTRY WREXXADDMACRO(PSZ,PSZ,ULONG );                   /* ?? */
typedef APIRET APIENTRY WREXXDROPMACRO(PSZ );                            /* ?? */
typedef APIRET APIENTRY WREXXSAVEMACROSPACE(ULONG,PSZ *,PSZ);            /* ?? */
typedef APIRET APIENTRY WREXXLOADMACROSPACE(ULONG ,PSZ *,PSZ);           /* ?? */
typedef APIRET APIENTRY WREXXQUERYMACRO(PSZ,PUSHORT );                   /* ?? */
typedef APIRET APIENTRY WREXXREORDERMACRO(PSZ,ULONG );                   /* ?? */
typedef APIRET APIENTRY WREXXCLEARMACROSPACE(VOID );                     /* ?? */
typedef APIRET APIENTRY WREXXSETHALT(LONG, LONG );
typedef void   APIENTRY WREXXWAITFORTERMINATION(VOID);
typedef APIRET APIENTRY WREXXDIDREXXTERMINATE(VOID);
WREXXSTART               *WRexxStart=NULL;
WREXXREGISTEREXITEXE     *WRexxRegisterExitExe=NULL;
WREXXREGISTEREXITDLL     *WRexxRegisterExitDll=NULL;
WREXXDEREGISTEREXIT      *WRexxDeregisterExit=NULL;
WREXXQUERYEXIT           *WRexxQueryExit=NULL;
WREXXREGISTERSUBCOMEXE   *WRexxRegisterSubcomExe=NULL;
WREXXDEREGISTERSUBCOM    *WRexxDeregisterSubcom=NULL;
WREXXREGISTERSUBCOMDLL   *WRexxRegisterSubcomDll=NULL;
WREXXQUERYSUBCOM         *WRexxQuerySubcom=NULL;
WREXXREGISTERFUNCTIONEXE *WRexxRegisterFunctionExe=NULL;
WREXXREGISTERFUNCTIONDLL *WRexxRegisterFunctionDll=NULL;
WREXXDEREGISTERFUNCTION  *WRexxDeregisterFunction=NULL;
WREXXQUERYFUNCTION       *WRexxQueryFunction=NULL;
WREXXVARIABLEPOOL        *WRexxVariablePool=NULL;
WREXXALLOCATEMEMORY      *WRexxAllocateMemory=NULL;
WREXXFREEMEMORY          *WRexxFreeMemory=NULL;
WREXXCREATEQUEUE         *WRexxCreateQueue=NULL;
WREXXDELETEQUEUE         *WRexxDeleteQueue=NULL;
WREXXQUERYQUEUE          *WRexxQueryQueue=NULL;
WREXXADDQUEUE            *WRexxAddQueue=NULL;
WREXXPULLQUEUE           *WRexxPullQueue=NULL;
WREXXADDMACRO            *WRexxAddMacro=NULL;
WREXXDROPMACRO           *WRexxDropMacro=NULL;
WREXXSAVEMACROSPACE      *WRexxSaveMacroSpace=NULL;
WREXXLOADMACROSPACE      *WRexxLoadMacroSpace=NULL;
WREXXQUERYMACRO          *WRexxQueryMacro=NULL;
WREXXREORDERMACRO        *WRexxReorderMacro=NULL;
WREXXCLEARMACROSPACE     *WRexxClearMacroSpace=NULL;
WREXXSETHALT             *WRexxSetHalt=NULL;
WREXXWAITFORTERMINATION  *WRexxWaitForTermination=NULL;
WREXXDIDREXXTERMINATE    *WRexxDidRexxTerminate=NULL;
#endif

/*
 * Typedefs for uni-REXX
 */
#if defined(INT_UNI_REXX)
# if defined(HAVE_PROTO)
#  define ORXXPrototype
# endif
# if defined(WIN32)
#  define ORXXCDecl __cdecl
# else
#  define ORXXCDecl
# endif
# define PSAASHVBLK PSHVBLOCK
typedef LONG ORXXCDecl   UREXXSTART(LONG,PRXSTRING,PSZ,PRXSTRING,PSZ,LONG,PRXSYSEXIT,PSHORT,PRXSTRING);
typedef APIRET ORXXCDecl UREXXREGISTEREXITDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG);
typedef APIRET ORXXCDecl UREXXREGISTEREXITEXE(PSZ,PFN,PUCHAR);
typedef APIRET ORXXCDecl UREXXDEREGISTEREXIT(PSZ,PSZ);
typedef APIRET ORXXCDecl UREXXQUERYEXIT(PSZ,PSZ,PUSHORT,PUCHAR ) ;
typedef APIRET ORXXCDecl UREXXREGISTERSUBCOMDLL(PSZ,PSZ,PSZ,PUCHAR,ULONG);
typedef APIRET ORXXCDecl UREXXREGISTERSUBCOMEXE(PSZ,PFN,PUCHAR);
typedef APIRET ORXXCDecl UREXXQUERYSUBCOM(PSZ,PSZ,PUSHORT,PUCHAR);
typedef APIRET ORXXCDecl UREXXDEREGISTERSUBCOM(PSZ,PSZ);
typedef APIRET ORXXCDecl UREXXREGISTERFUNCTIONDLL(PSZ,PSZ,PSZ);
typedef APIRET ORXXCDecl UREXXREGISTERFUNCTIONEXE(PSZ,PFN);
typedef APIRET ORXXCDecl UREXXDEREGISTERFUNCTION(PSZ);
typedef APIRET ORXXCDecl UREXXQUERYFUNCTION(PSZ);
typedef APIRET ORXXCDecl UREXXVARIABLEPOOL(PSAASHVBLK);
typedef PVOID  ORXXCDecl UREXXALLOCATEMEMORY( ULONG );
typedef APIRET ORXXCDecl UREXXFREEMEMORY( PVOID );
typedef APIRET ORXXCDecl UREXXCREATEQUEUE(PSZ,ULONG,PSZ,ULONG* ) ;        /* ?? */
typedef APIRET ORXXCDecl UREXXDELETEQUEUE(PSZ ) ;                         /* ?? */
typedef APIRET ORXXCDecl UREXXQUERYQUEUE(PSZ,ULONG* ) ;                   /* ?? */
typedef APIRET ORXXCDecl UREXXADDQUEUE( PSZ,PRXSTRING,ULONG ) ;           /* ?? */
typedef APIRET ORXXCDecl UREXXPULLQUEUE(PSZ,PRXSTRING,PDATETIME,ULONG );  /* ?? */
typedef APIRET ORXXCDecl UREXXADDMACRO(PSZ,PSZ,ULONG );                   /* ?? */
typedef APIRET ORXXCDecl UREXXDROPMACRO(PSZ );                            /* ?? */
typedef APIRET ORXXCDecl UREXXSAVEMACROSPACE(ULONG,PSZ *,PSZ);            /* ?? */
typedef APIRET ORXXCDecl UREXXLOADMACROSPACE(ULONG ,PSZ *,PSZ);           /* ?? */
typedef APIRET ORXXCDecl UREXXQUERYMACRO(PSZ,PUSHORT );                   /* ?? */
typedef APIRET ORXXCDecl UREXXREORDERMACRO(PSZ,ULONG );                   /* ?? */
typedef APIRET ORXXCDecl UREXXCLEARMACROSPACE(VOID );                     /* ?? */
typedef APIRET ORXXCDecl UREXXSETHALT(LONG, LONG );
typedef void   ORXXCDecl UREXXWAITFORTERMINATION(VOID);
typedef APIRET ORXXCDecl UREXXDIDREXXTERMINATE(VOID);
UREXXSTART               *URexxStart=NULL;
UREXXREGISTEREXITEXE     *URexxRegisterExitExe=NULL;
UREXXREGISTEREXITDLL     *URexxRegisterExitDll=NULL;
UREXXDEREGISTEREXIT      *URexxDeregisterExit=NULL;
UREXXQUERYEXIT           *URexxQueryExit=NULL;
UREXXREGISTERSUBCOMEXE   *URexxRegisterSubcomExe=NULL;
UREXXDEREGISTERSUBCOM    *URexxDeregisterSubcom=NULL;
UREXXREGISTERSUBCOMDLL   *URexxRegisterSubcomDll=NULL;
UREXXQUERYSUBCOM         *URexxQuerySubcom=NULL;
UREXXREGISTERFUNCTIONEXE *URexxRegisterFunctionExe=NULL;
UREXXREGISTERFUNCTIONDLL *URexxRegisterFunctionDll=NULL;
UREXXDEREGISTERFUNCTION  *URexxDeregisterFunction=NULL;
UREXXQUERYFUNCTION       *URexxQueryFunction=NULL;
UREXXVARIABLEPOOL        *URexxVariablePool=NULL;
UREXXALLOCATEMEMORY      *URexxAllocateMemory=NULL;
UREXXFREEMEMORY          *URexxFreeMemory=NULL;
UREXXCREATEQUEUE         *URexxCreateQueue=NULL;
UREXXDELETEQUEUE         *URexxDeleteQueue=NULL;
UREXXQUERYQUEUE          *URexxQueryQueue=NULL;
UREXXADDQUEUE            *URexxAddQueue=NULL;
UREXXPULLQUEUE           *URexxPullQueue=NULL;
UREXXADDMACRO            *URexxAddMacro=NULL;
UREXXDROPMACRO           *URexxDropMacro=NULL;
UREXXSAVEMACROSPACE      *URexxSaveMacroSpace=NULL;
UREXXLOADMACROSPACE      *URexxLoadMacroSpace=NULL;
UREXXQUERYMACRO          *URexxQueryMacro=NULL;
UREXXREORDERMACRO        *URexxReorderMacro=NULL;
UREXXCLEARMACROSPACE     *URexxClearMacroSpace=NULL;
UREXXSETHALT             *URexxSetHalt=NULL;
UREXXWAITFORTERMINATION  *URexxWaitForTermination=NULL;
UREXXDIDREXXTERMINATE    *URexxDidRexxTerminate=NULL;
#endif

static char TraceFileName[256];
static int Trace = 0;
static char LoadedLibrary[256];
static int InterpreterIdx = -1;

#if defined(INT_REXX_IMC)
/*
 * The following functions are to convert parameter values from the
 * "standard" to values in REXX/imc
 */
typedef struct {
   char *sysexit_name;
   short sysexit_code;
} IRXSYSEXIT;
typedef IRXSYSEXIT *PIRXSYSEXIT;
/*
 * Convert CallType for RexxStart()
 */
static LONG IRexxStartCallType( LONG CallType)
{
   LONG rc=CallType;
   switch ( CallType )
   {
      case RXCOMMAND:    rc = 1;   break;
      case RXSUBROUTINE: rc = 2;   break;
      case RXFUNCTION:   rc = 4;   break;
   }
   return rc;
}
/*
 * Convert sysexit_code for System Exits
 */
static LONG IRXSYSEXITsysexit_code( LONG sysexit_code)
{
   switch ( sysexit_code )
   {
      case RXFNC:        sysexit_code = 100;   break; /* not implemented */
      case RXCMD:        sysexit_code =   0;   break;
      case RXMSQ:        sysexit_code = 100;   break; /* not implemented */
      case RXSIO:        sysexit_code =   1;   break;
      case RXHLT:        sysexit_code = 100;   break; /* not implemented */
      case RXTRC:        sysexit_code = 100;   break; /* not implemented */
      case RXINI:        sysexit_code =   2;   break;
      case RXTER:        sysexit_code =   3;   break;
      case RXDBG:        sysexit_code = 100;   break; /* not implemented */
      case RXENDLST:     sysexit_code = 100;   break;
   }
   return sysexit_code;
}
/*
 * Convert return code from RexxVariablePool()
 */
static LONG IREXXVARIABLEPOOLrc( ULONG rc)
{
   switch ( rc )
   {
      case RXSHV_BADF:      rc = 0x20;   break;
      default:                           break;
   }
   return rc;
}
#endif

static void TraceString( char *fmt, ... )
{
   FILE *fp=NULL;
   int using_stderr = 0;
   va_list argptr;

   if ( strcmp( TraceFileName, "stderr" ) == 0 )
      using_stderr = 1;
   if ( using_stderr )
      fp = stderr;
   else
      fp = fopen( TraceFileName, "a" );
   if ( fp )
   {
      va_start( argptr, fmt );
      vfprintf( fp, fmt, argptr );
      va_end( argptr );
      if ( !using_stderr )
         fclose( fp );
   }
}

static PVOID APIENTRY RexxTransAllocateMemory( ULONG size )
{
   PVOID ret;
#if defined( WIN32 )
   ret = (PVOID)( GlobalLock( GlobalAlloc ( GMEM_FIXED, size ) ) );
   return ret;
#elif defined( OS2 )
   if ( ( BOOL )DosAllocMem( &ret, size, fPERM|PAG_COMMIT ) )
      return NULL;
   else
      return ret;
#else
   ret = (PVOID)malloc( size );
   return ret;
#endif
}

static APIRET APIENTRY RexxTransFreeMemory( PVOID ptr )
{
#if defined( WIN32 )
   GlobalFree( ptr );
#elif defined( OS2 )
   DosFreeMem( ptr );
#else
   free( ptr );
#endif
   return 0;
}

#if defined(DYNAMIC_HPSHLOAD)
void find_shared_library( const char *inname, const char *inenv, char *retname )
{
   char *paths = NULL;
   char outname[257];
   char *env_path;

   env_path = getenv( inenv );
   if ( Trace )
   {
      TraceString( "%s: In: \"%s\" Env: \"%s\" Path: \"%s\"\n",
                   "find_shared_library()",
                   inname, inenv,env_path );
   }
   strcpy( retname, inname );
   outname[0] = '\0';
   paths = env_path;
   while ( paths )
   {
      int pathlen;
      char *sep;

      while ( *paths == ':' )
         paths++;
      sep = strchr( paths, ':' );
      pathlen = sep ? (sep - paths) : strlen( paths );
      if ( pathlen == 0 )
         break; /* no more paths! */
      strncpy( outname, paths, pathlen );
      outname[pathlen] = 0;

      if ( outname[pathlen-1] != '/' )
         strcat( outname, "/" );
      strcat( outname, SHLPRE );
      strcat( outname, inname );
      strcat( outname, SHLPST );
      paths = sep ? (sep + 1) : 0; /* set up for next pass */
      if ( Trace )
      {
         TraceString( "%s: Looking for: \"%s\"\n",
                   "find_shared_library()",
                   outname );
      }
      if ( access( outname, F_OK ) == 0 )
      {
         strcpy( retname, outname );
         break;
      }
   }
   return;
}
#endif

#ifdef UNIX
static void GenerateLibraryName( char *library, char *LibraryName )
{
# if defined(DYNAMIC_HPSHLOAD)
   find_shared_library( library, "SHLIB_PATH", LibraryName );
# else
   strcpy( LibraryName, SHLPRE );
   strcat( LibraryName, library );
   strcat( LibraryName, SHLPST );
# endif
   if ( Trace )
   {
      TraceString( "%s: Generated library name of \"%s\".\n",
                   "GenerateLibraryName()",
                   LibraryName );
   }
}
#endif

static handle_type FindInterpreter( char *library, int idx )
{
   handle_type handle=(handle_type)NULL ;
   handle_type handle1=(handle_type)NULL ;
#ifdef WIN32
   FARPROC addr;
   char LoadError[256];
#endif
#ifdef UNIX
   char LibraryName[256];
   char LibraryName1[256];
   void *addr;
#endif
#if defined(DYNAMIC_HPSHLOAD)
   long eaddr ;
#endif
#if defined(OS2)
   CHAR LoadError[256];
   PFN addr;
#endif
   register int j=0;
   char *CurrentLibrary=NULL;

#ifdef UNIX
   /*
    * Generate the full name of the shared library...
    */
   GenerateLibraryName( library, LibraryName );
#endif

#if defined(DYNAMIC_DLOPEN)
   if ( Trace )
   {
      TraceString( "%s: Attempting to load \"%s\" using dlopen()...",
                   "FindInterpreter()",
                   LibraryName );
   }
   handle = dlopen( LibraryName, RTLD_LAZY | RTLD_GLOBAL ) ;
#elif defined(DYNAMIC_HPSHLOAD)
   if ( Trace )
   {
      TraceString( "%s: Attempting to load \"%s\" using shl_load()...",
                   "FindInterpreter()",
                   LibraryName );
   }
   handle = shl_load( LibraryName, BIND_IMMEDIATE | BIND_VERBOSE | DYNAMIC_PATH ,0L ) ;
#elif defined(DYNAMIC_OS2)
   if ( Trace )
   {
      TraceString( "%s: Attempting to load \"%s\" using DosLoadModule()...",
                   "FindInterpreter()",
                   library );
   }
   if ( DosLoadModule( LoadError, sizeof(LoadError), library, &handle ) )
   {
      handle = (handle_type)NULL;
   }
#elif defined(DYNAMIC_WIN32)
   if ( Trace )
   {
      TraceString( "%s: Attempting to load \"%s\" using LoadLibrary()...",
                   "FindInterpreter()",
                   library );
   }
   handle = LoadLibrary( library ) ;
#elif defined(DYNAMIC_BEOS)
   if ( Trace )
   {
      TraceString( "%s: Attempting to load \"%s\" using load_add_on()...",
                   "FindInterpreter()",
                   LibraryName );
   }
   handle = load_add_on( LibraryName );

   if (handle < 0) /* doco says == B_ERROR, but that doesn't work */
   {
      handle = (handle_type)NULL;
   }
#endif
   if ( handle != (handle_type)NULL )
   {
      if ( Trace )
      {
         TraceString( "found %s\n", MyInterpreterName[idx] );
      }
      /* 
       * special kludge for Object Rexx 
       */
      if ( idx == INT_OBJECT_REXX )  
      {
#ifdef UNIX
         GenerateLibraryName( OBJECT_REXX_EXTRA_DLL, LibraryName1 );
#endif
#if defined(DYNAMIC_DLOPEN)
         handle1 = dlopen( LibraryName1, RTLD_LAZY | RTLD_GLOBAL ) ;
#elif defined(DYNAMIC_HPSHLOAD)
         handle1 = shl_load( LibraryName1, BIND_IMMEDIATE | BIND_VERBOSE | DYNAMIC_PATH, 0L ) ;
#elif defined(DYNAMIC_OS2)
         if ( Trace )
         {
            TraceString( "%s: Attempting to load \"%s\" using DosLoadModule()...",
                         "FindInterpreter()",
                         OBJECT_REXX_EXTRA_DLL );
         }
         if ( DosLoadModule( LoadError, sizeof(LoadError),
                       OBJECT_REXX_EXTRA_DLL, &handle1 ) )
         {
            handle1 = (handle_type)NULL;
         }
#elif defined(DYNAMIC_WIN32)
         if ( Trace )
         {
            TraceString( "%s: Attempting to load \"%s\" using LoadLibrary()...",
                         "FindInterpreter()",
                         OBJECT_REXX_EXTRA_DLL );
         }
         handle1 = LoadLibrary( OBJECT_REXX_EXTRA_DLL ) ;
#elif defined(DYNAMIC_BEOS)
         handle1 = load_add_on( LibraryName1 );
         if (handle1 == B_ERROR)
         {
            handle1 = (handle_type)NULL;
         }
#endif
         if ( handle1 == (handle_type)NULL )
         {
#ifdef UNIX
            fprintf( stderr, "Could not find %s shared library. Cannot continue.\n", LibraryName1);
#else
            fprintf( stderr, "Could not find %s Rexx DLL. Cannot continue.\n", OBJECT_REXX_EXTRA_DLL);
#endif
            if ( Trace )
            {
#ifdef WIN32
               FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
               TraceString( "not found: %s\n", LoadError );
#endif
#ifdef OS2
               TraceString( "not found: %s\n", LoadError );
#endif
#ifdef UNIX
               TraceString( "not found: %s\n", strerror( errno ) );
#endif
            }
            /* exit(9); */
         }
         else
         {
            if ( Trace )
            {
               TraceString( "found\n" );
            }
         }
      }
      strcpy( LoadedLibrary, library );
      for ( j = 0; j < NUM_REXX_FUNCTIONS; j++ )
      {
#if defined(DYNAMIC_DLOPEN)
         addr = (void *)(dlsym( handle, MyFunctionName[j] )) ;
#elif defined(DYNAMIC_HPSHLOAD)

         if (shl_findsym( &handle, MyFunctionName[j], TYPE_PROCEDURE, &eaddr ))
         {
            addr = NULL;
         }
         else
         {
            addr = (PFN)eaddr;
         }
#elif defined(DYNAMIC_OS2)
         if ( DosQueryProcAddr( handle, 0L, MyFunctionName[j], &addr) )
         {
            addr = NULL;
         }
#elif defined(DYNAMIC_WIN32)
         addr = GetProcAddress( (HMODULE)handle, MyFunctionName[j] );
#elif defined(DYNAMIC_BEOS)
         if ( get_image_symbol( handle, MyFunctionName[j], B_SYMBOL_TYPE_TEXT, (void **)&addr ) == B_BAD_IMAGE_ID )
         {
            addr = NULL;
         }
#endif
         CurrentLibrary = library;
         /*
          * Another special kludge for Object Rexx
          * If we didn't find the entry point in the first DLL,
          * look for the entry point in the other DLL.
          */
         if ( addr == NULL 
         &&   idx == INT_OBJECT_REXX )
         {
            /*
             * For Object Rexx, only need to test for Unix, Win32 and OS/2
             */
#if defined(DYNAMIC_DLOPEN)
            addr = (void *)(dlsym( handle1, MyFunctionName[j] )) ;
#elif defined(DYNAMIC_OS2)
            if ( DosQueryProcAddr( handle1, 0L, MyFunctionName[j], &addr) )
            {
               addr = NULL;
            }
#elif defined(DYNAMIC_WIN32)
            addr = GetProcAddress( (HMODULE)handle1, MyFunctionName[j] );
#endif
            CurrentLibrary = OBJECT_REXX_EXTRA_DLL;
         }
         /*
          * Log the function and address. This is useful if the module
          * doesn't have an address for this procedure.
          */
         if (Trace)
         {
            TraceString( "%s %s: Address %x\n",
               CurrentLibrary,
               MyFunctionName[j],
               (addr == NULL) ? 0 : addr );
         }
         /*
          * Even if the function being processed is not in the module, its
          * address is still stored.  In this case it will simply be set
          * again to NULL.
          */
         switch( idx )
         {
#if defined(INT_REGINA) || defined(INT_REGINALD)
            case INT_REGINA:
            case INT_REGINALD:
               switch ( j )
               {
                  case FUNCTION_REXXSTART:                RRexxStart =                 (RREXXSTART                *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONEXE:  RRexxRegisterFunctionExe =   (RREXXREGISTERFUNCTIONEXE  *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONDLL:  RRexxRegisterFunctionDll =   (RREXXREGISTERFUNCTIONDLL  *)addr; break;
                  case FUNCTION_REXXDEREGISTERFUNCTION:   RRexxDeregisterFunction =    (RREXXDEREGISTERFUNCTION   *)addr; break;
                  case FUNCTION_REXXREGISTEREXITEXE:      RRexxRegisterExitExe =       (RREXXREGISTEREXITEXE      *)addr; break;
                  case FUNCTION_REXXREGISTEREXITDLL:      RRexxRegisterExitDll =       (RREXXREGISTEREXITDLL      *)addr; break;
                  case FUNCTION_REXXDEREGISTEREXIT:       RRexxDeregisterExit =        (RREXXDEREGISTEREXIT       *)addr; break;
                  case FUNCTION_REXXQUERYEXIT:            RRexxQueryExit =             (RREXXQUERYEXIT            *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMEXE:    RRexxRegisterSubcomExe =     (RREXXREGISTERSUBCOMEXE    *)addr; break;
                  case FUNCTION_REXXDEREGISTERSUBCOM:     RRexxDeregisterSubcom =      (RREXXDEREGISTERSUBCOM     *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMDLL:    RRexxRegisterSubcomDll =     (RREXXREGISTERSUBCOMDLL    *)addr; break;
                  case FUNCTION_REXXQUERYSUBCOM:          RRexxQuerySubcom =           (RREXXQUERYSUBCOM          *)addr; break;
                  case FUNCTION_REXXQUERYFUNCTION:        RRexxQueryFunction =         (RREXXQUERYFUNCTION        *)addr; break;
                  case FUNCTION_REXXVARIABLEPOOL:         RRexxVariablePool =          (RREXXVARIABLEPOOL         *)addr; break;
                  case FUNCTION_REXXALLOCATEMEMORY:       RRexxAllocateMemory =        (RREXXALLOCATEMEMORY       *)addr; break;
                  case FUNCTION_REXXFREEMEMORY:           RRexxFreeMemory =            (RREXXFREEMEMORY           *)addr; break;
                  case FUNCTION_REXXCREATEQUEUE:          RRexxCreateQueue =           (RREXXCREATEQUEUE          *)addr; break;
                  case FUNCTION_REXXDELETEQUEUE:          RRexxDeleteQueue =           (RREXXDELETEQUEUE          *)addr; break;
                  case FUNCTION_REXXQUERYQUEUE:           RRexxQueryQueue=             (RREXXQUERYQUEUE           *)addr; break;
                  case FUNCTION_REXXADDQUEUE:             RRexxAddQueue=               (RREXXADDQUEUE             *)addr; break;
                  case FUNCTION_REXXPULLQUEUE:            RRexxPullQueue=              (RREXXPULLQUEUE            *)addr; break;
                  case FUNCTION_REXXADDMACRO:             RRexxAddMacro=               (RREXXADDMACRO             *)addr; break;
                  case FUNCTION_REXXDROPMACRO:            RRexxDropMacro=              (RREXXDROPMACRO            *)addr; break;
                  case FUNCTION_REXXSAVEMACROSPACE:       RRexxSaveMacroSpace=         (RREXXSAVEMACROSPACE       *)addr; break;
                  case FUNCTION_REXXLOADMACROSPACE:       RRexxLoadMacroSpace=         (RREXXLOADMACROSPACE       *)addr; break;
                  case FUNCTION_REXXQUERYMACRO:           RRexxQueryMacro=             (RREXXQUERYMACRO           *)addr; break;
                  case FUNCTION_REXXREORDERMACRO:         RRexxReorderMacro=           (RREXXREORDERMACRO         *)addr; break;
                  case FUNCTION_REXXCLEARMACROSPACE:      RRexxClearMacroSpace=        (RREXXCLEARMACROSPACE      *)addr; break;
                  case FUNCTION_REXXSETHALT:              RRexxSetHalt=                (RREXXSETHALT              *)addr; break;
                  case FUNCTION_REXXWAITFORTERMINATION:   RRexxWaitForTermination=     (RREXXWAITFORTERMINATION   *)addr; break;
                  case FUNCTION_REXXDIDREXXTERMINATE:     RRexxDidRexxTerminate=       (RREXXDIDREXXTERMINATE     *)addr; break;
               }
               break;
#endif
#if defined(INT_REXX_IMC)
            case INT_REXX_IMC:
               switch ( j )
               {
                  case FUNCTION_REXXSTART:                IRexxStart =                 (IREXXSTART                *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONEXE:  IRexxRegisterFunctionExe =   (IREXXREGISTERFUNCTIONEXE  *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONDLL:  IRexxRegisterFunctionDll =   (IREXXREGISTERFUNCTIONDLL  *)addr; break;
                  case FUNCTION_REXXDEREGISTERFUNCTION:   IRexxDeregisterFunction =    (IREXXDEREGISTERFUNCTION   *)addr; break;
                  case FUNCTION_REXXREGISTEREXITEXE:      IRexxRegisterExitExe =       (IREXXREGISTEREXITEXE      *)addr; break;
                  case FUNCTION_REXXREGISTEREXITDLL:      IRexxRegisterExitDll =       (IREXXREGISTEREXITDLL      *)addr; break;
                  case FUNCTION_REXXDEREGISTEREXIT:       IRexxDeregisterExit =        (IREXXDEREGISTEREXIT       *)addr; break;
                  case FUNCTION_REXXQUERYEXIT:            IRexxQueryExit =             (IREXXQUERYEXIT            *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMEXE:    IRexxRegisterSubcomExe =     (IREXXREGISTERSUBCOMEXE    *)addr; break;
                  case FUNCTION_REXXDEREGISTERSUBCOM:     IRexxDeregisterSubcom =      (IREXXDEREGISTERSUBCOM     *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMDLL:    IRexxRegisterSubcomDll =     (IREXXREGISTERSUBCOMDLL    *)addr; break;
                  case FUNCTION_REXXQUERYSUBCOM:          IRexxQuerySubcom =           (IREXXQUERYSUBCOM          *)addr; break;
                  case FUNCTION_REXXQUERYFUNCTION:        IRexxQueryFunction =         (IREXXQUERYFUNCTION        *)addr; break;
                  case FUNCTION_REXXVARIABLEPOOL:         IRexxVariablePool =          (IREXXVARIABLEPOOL         *)addr; break;
                  case FUNCTION_REXXALLOCATEMEMORY:       IRexxAllocateMemory =        (IREXXALLOCATEMEMORY       *)RexxTransAllocateMemory; break;
                  case FUNCTION_REXXFREEMEMORY:           IRexxFreeMemory =            (IREXXFREEMEMORY           *)RexxTransFreeMemory; break;
                  case FUNCTION_REXXCREATEQUEUE:          IRexxCreateQueue=            (IREXXCREATEQUEUE          *)addr; break;
                  case FUNCTION_REXXDELETEQUEUE:          IRexxDeleteQueue=            (IREXXDELETEQUEUE          *)addr; break;
                  case FUNCTION_REXXQUERYQUEUE:           IRexxQueryQueue=             (IREXXQUERYQUEUE           *)addr; break;
                  case FUNCTION_REXXADDQUEUE:             IRexxAddQueue=               (IREXXADDQUEUE             *)addr; break;
                  case FUNCTION_REXXPULLQUEUE:            IRexxPullQueue=              (IREXXPULLQUEUE            *)addr; break;
                  case FUNCTION_REXXADDMACRO:             IRexxAddMacro=               (IREXXADDMACRO             *)addr; break;
                  case FUNCTION_REXXDROPMACRO:            IRexxDropMacro=              (IREXXDROPMACRO            *)addr; break;
                  case FUNCTION_REXXSAVEMACROSPACE:       IRexxSaveMacroSpace=         (IREXXSAVEMACROSPACE       *)addr; break;
                  case FUNCTION_REXXLOADMACROSPACE:       IRexxLoadMacroSpace=         (IREXXLOADMACROSPACE       *)addr; break;
                  case FUNCTION_REXXQUERYMACRO:           IRexxQueryMacro=             (IREXXQUERYMACRO           *)addr; break;
                  case FUNCTION_REXXREORDERMACRO:         IRexxReorderMacro=           (IREXXREORDERMACRO         *)addr; break;
                  case FUNCTION_REXXCLEARMACROSPACE:      IRexxClearMacroSpace=        (IREXXCLEARMACROSPACE      *)addr; break;
                  case FUNCTION_REXXSETHALT:              IRexxSetHalt=                (IREXXSETHALT              *)addr; break;
                  case FUNCTION_REXXWAITFORTERMINATION:   IRexxWaitForTermination=     (IREXXWAITFORTERMINATION   *)addr; break;
                  case FUNCTION_REXXDIDREXXTERMINATE:     IRexxDidRexxTerminate=       (IREXXDIDREXXTERMINATE     *)addr; break;
               }
               break;
#endif
#if defined(INT_OBJECT_REXX)
            case INT_OBJECT_REXX:
               switch ( j )
               {
                  case FUNCTION_REXXSTART:                ORexxStart =                 (OREXXSTART                *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONEXE:  ORexxRegisterFunctionExe =   (OREXXREGISTERFUNCTIONEXE  *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONDLL:  ORexxRegisterFunctionDll =   (OREXXREGISTERFUNCTIONDLL  *)addr; break;
                  case FUNCTION_REXXDEREGISTERFUNCTION:   ORexxDeregisterFunction =    (OREXXDEREGISTERFUNCTION   *)addr; break;
                  case FUNCTION_REXXREGISTEREXITEXE:      ORexxRegisterExitExe =       (OREXXREGISTEREXITEXE      *)addr; break;
                  case FUNCTION_REXXREGISTEREXITDLL:      ORexxRegisterExitDll =       (OREXXREGISTEREXITDLL      *)addr; break;
                  case FUNCTION_REXXDEREGISTEREXIT:       ORexxDeregisterExit =        (OREXXDEREGISTEREXIT       *)addr; break;
                  case FUNCTION_REXXQUERYEXIT:            ORexxQueryExit =             (OREXXQUERYEXIT            *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMEXE:    ORexxRegisterSubcomExe =     (OREXXREGISTERSUBCOMEXE    *)addr; break;
                  case FUNCTION_REXXDEREGISTERSUBCOM:     ORexxDeregisterSubcom =      (OREXXDEREGISTERSUBCOM     *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMDLL:    ORexxRegisterSubcomDll =     (OREXXREGISTERSUBCOMDLL    *)addr; break;
                  case FUNCTION_REXXQUERYSUBCOM:          ORexxQuerySubcom =           (OREXXQUERYSUBCOM          *)addr; break;
                  case FUNCTION_REXXQUERYFUNCTION:        ORexxQueryFunction =         (OREXXQUERYFUNCTION        *)addr; break;
                  case FUNCTION_REXXVARIABLEPOOL:         ORexxVariablePool =          (OREXXVARIABLEPOOL         *)addr; break;
                  case FUNCTION_REXXALLOCATEMEMORY:       ORexxAllocateMemory =        (OREXXALLOCATEMEMORY       *)RexxTransAllocateMemory; break;
                  case FUNCTION_REXXFREEMEMORY:           ORexxFreeMemory =            (OREXXFREEMEMORY           *)RexxTransFreeMemory; break;
                  case FUNCTION_REXXCREATEQUEUE:          ORexxCreateQueue=            (OREXXCREATEQUEUE          *)addr; break;
                  case FUNCTION_REXXDELETEQUEUE:          ORexxDeleteQueue=            (OREXXDELETEQUEUE          *)addr; break;
                  case FUNCTION_REXXQUERYQUEUE:           ORexxQueryQueue=             (OREXXQUERYQUEUE           *)addr; break;
                  case FUNCTION_REXXADDQUEUE:             ORexxAddQueue=               (OREXXADDQUEUE             *)addr; break;
                  case FUNCTION_REXXPULLQUEUE:            ORexxPullQueue=              (OREXXPULLQUEUE            *)addr; break;
                  case FUNCTION_REXXADDMACRO:             ORexxAddMacro=               (OREXXADDMACRO             *)addr; break;
                  case FUNCTION_REXXDROPMACRO:            ORexxDropMacro=              (OREXXDROPMACRO            *)addr; break;
                  case FUNCTION_REXXSAVEMACROSPACE:       ORexxSaveMacroSpace=         (OREXXSAVEMACROSPACE       *)addr; break;
                  case FUNCTION_REXXLOADMACROSPACE:       ORexxLoadMacroSpace=         (OREXXLOADMACROSPACE       *)addr; break;
                  case FUNCTION_REXXQUERYMACRO:           ORexxQueryMacro=             (OREXXQUERYMACRO           *)addr; break;
                  case FUNCTION_REXXREORDERMACRO:         ORexxReorderMacro=           (OREXXREORDERMACRO         *)addr; break;
                  case FUNCTION_REXXCLEARMACROSPACE:      ORexxClearMacroSpace=        (OREXXCLEARMACROSPACE      *)addr; break;
                  case FUNCTION_REXXSETHALT:              ORexxSetHalt=                (OREXXSETHALT              *)addr; break;
                  case FUNCTION_REXXWAITFORTERMINATION:   ORexxWaitForTermination=     (OREXXWAITFORTERMINATION   *)addr; break;
                  case FUNCTION_REXXDIDREXXTERMINATE:     ORexxDidRexxTerminate=       (OREXXDIDREXXTERMINATE     *)addr; break;
               }
               break;
#endif
#if defined(INT_PERSONAL_REXX)
            case INT_PERSONAL_REXX:
               switch ( j )
               {
                  case FUNCTION_REXXSTART:                QRexxStart =                 (QREXXSTART                *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONEXE:  QRexxRegisterFunctionExe =   (QREXXREGISTERFUNCTIONEXE  *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONDLL:  QRexxRegisterFunctionDll =   (QREXXREGISTERFUNCTIONDLL  *)addr; break;
                  case FUNCTION_REXXDEREGISTERFUNCTION:   QRexxDeregisterFunction =    (QREXXDEREGISTERFUNCTION   *)addr; break;
                  case FUNCTION_REXXREGISTEREXITEXE:      QRexxRegisterExitExe =       (QREXXREGISTEREXITEXE      *)addr; break;
                  case FUNCTION_REXXREGISTEREXITDLL:      QRexxRegisterExitDll =       (QREXXREGISTEREXITDLL      *)addr; break;
                  case FUNCTION_REXXDEREGISTEREXIT:       QRexxDeregisterExit =        (QREXXDEREGISTEREXIT       *)addr; break;
                  case FUNCTION_REXXQUERYEXIT:            QRexxQueryExit =             (QREXXQUERYEXIT            *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMEXE:    QRexxRegisterSubcomExe =     (QREXXREGISTERSUBCOMEXE    *)addr; break;
                  case FUNCTION_REXXDEREGISTERSUBCOM:     QRexxDeregisterSubcom =      (QREXXDEREGISTERSUBCOM     *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMDLL:    QRexxRegisterSubcomDll =     (QREXXREGISTERSUBCOMDLL    *)addr; break;
                  case FUNCTION_REXXQUERYSUBCOM:          QRexxQuerySubcom =           (QREXXQUERYSUBCOM          *)addr; break;
                  case FUNCTION_REXXQUERYFUNCTION:        QRexxQueryFunction =         (QREXXQUERYFUNCTION        *)addr; break;
                  case FUNCTION_REXXVARIABLEPOOL:         QRexxVariablePool =          (QREXXVARIABLEPOOL         *)addr; break;
                  case FUNCTION_REXXALLOCATEMEMORY:       QRexxAllocateMemory =        (QREXXALLOCATEMEMORY       *)RexxTransAllocateMemory; break;
                  case FUNCTION_REXXFREEMEMORY:           QRexxFreeMemory =            (QREXXFREEMEMORY           *)RexxTransFreeMemory; break;
                  case FUNCTION_REXXCREATEQUEUE:          QRexxCreateQueue=            (QREXXCREATEQUEUE          *)addr; break;
                  case FUNCTION_REXXDELETEQUEUE:          QRexxDeleteQueue=            (QREXXDELETEQUEUE          *)addr; break;
                  case FUNCTION_REXXQUERYQUEUE:           QRexxQueryQueue=             (QREXXQUERYQUEUE           *)addr; break;
                  case FUNCTION_REXXADDQUEUE:             QRexxAddQueue=               (QREXXADDQUEUE             *)addr; break;
                  case FUNCTION_REXXPULLQUEUE:            QRexxPullQueue=              (QREXXPULLQUEUE            *)addr; break;
                  case FUNCTION_REXXADDMACRO:             QRexxAddMacro=               (QREXXADDMACRO             *)addr; break;
                  case FUNCTION_REXXDROPMACRO:            QRexxDropMacro=              (QREXXDROPMACRO            *)addr; break;
                  case FUNCTION_REXXSAVEMACROSPACE:       QRexxSaveMacroSpace=         (QREXXSAVEMACROSPACE       *)addr; break;
                  case FUNCTION_REXXLOADMACROSPACE:       QRexxLoadMacroSpace=         (QREXXLOADMACROSPACE       *)addr; break;
                  case FUNCTION_REXXQUERYMACRO:           QRexxQueryMacro=             (QREXXQUERYMACRO           *)addr; break;
                  case FUNCTION_REXXREORDERMACRO:         QRexxReorderMacro=           (QREXXREORDERMACRO         *)addr; break;
                  case FUNCTION_REXXCLEARMACROSPACE:      QRexxClearMacroSpace=        (QREXXCLEARMACROSPACE      *)addr; break;
                  case FUNCTION_REXXSETHALT:              QRexxSetHalt=                (QREXXSETHALT              *)addr; break;
                  case FUNCTION_REXXWAITFORTERMINATION:   QRexxWaitForTermination=     (QREXXWAITFORTERMINATION   *)addr; break;
                  case FUNCTION_REXXDIDREXXTERMINATE:     QRexxDidRexxTerminate=       (QREXXDIDREXXTERMINATE     *)addr; break;
               }
               break;
#endif
#if defined(INT_WINREXX)
            case INT_WINREXX:
               switch ( j )
               {
                  case FUNCTION_REXXSTART:                WRexxStart =                 (WREXXSTART                *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONEXE:  WRexxRegisterFunctionExe =   (WREXXREGISTERFUNCTIONEXE  *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONDLL:  WRexxRegisterFunctionDll =   (WREXXREGISTERFUNCTIONDLL  *)addr; break;
                  case FUNCTION_REXXDEREGISTERFUNCTION:   WRexxDeregisterFunction =    (WREXXDEREGISTERFUNCTION   *)addr; break;
                  case FUNCTION_REXXREGISTEREXITEXE:      WRexxRegisterExitExe =       (WREXXREGISTEREXITEXE      *)addr; break;
                  case FUNCTION_REXXREGISTEREXITDLL:      WRexxRegisterExitDll =       (WREXXREGISTEREXITDLL      *)addr; break;
                  case FUNCTION_REXXDEREGISTEREXIT:       WRexxDeregisterExit =        (WREXXDEREGISTEREXIT       *)addr; break;
                  case FUNCTION_REXXQUERYEXIT:            WRexxQueryExit =             (WREXXQUERYEXIT            *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMEXE:    WRexxRegisterSubcomExe =     (WREXXREGISTERSUBCOMEXE    *)addr; break;
                  case FUNCTION_REXXDEREGISTERSUBCOM:     WRexxDeregisterSubcom =      (WREXXDEREGISTERSUBCOM     *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMDLL:    WRexxRegisterSubcomDll =     (WREXXREGISTERSUBCOMDLL    *)addr; break;
                  case FUNCTION_REXXQUERYSUBCOM:          WRexxQuerySubcom =           (WREXXQUERYSUBCOM          *)addr; break;
                  case FUNCTION_REXXQUERYFUNCTION:        WRexxQueryFunction =         (WREXXQUERYFUNCTION        *)addr; break;
                  case FUNCTION_REXXVARIABLEPOOL:         WRexxVariablePool =          (WREXXVARIABLEPOOL         *)addr; break;
                  case FUNCTION_REXXALLOCATEMEMORY:       WRexxAllocateMemory =        (WREXXALLOCATEMEMORY       *)RexxTransAllocateMemory; break;
                  case FUNCTION_REXXFREEMEMORY:           WRexxFreeMemory =            (WREXXFREEMEMORY           *)RexxTransFreeMemory; break;
                  case FUNCTION_REXXCREATEQUEUE:          WRexxCreateQueue=            (WREXXCREATEQUEUE          *)addr; break;
                  case FUNCTION_REXXDELETEQUEUE:          WRexxDeleteQueue=            (WREXXDELETEQUEUE          *)addr; break;
                  case FUNCTION_REXXQUERYQUEUE:           WRexxQueryQueue=             (WREXXQUERYQUEUE           *)addr; break;
                  case FUNCTION_REXXADDQUEUE:             WRexxAddQueue=               (WREXXADDQUEUE             *)addr; break;
                  case FUNCTION_REXXPULLQUEUE:            WRexxPullQueue=              (WREXXPULLQUEUE            *)addr; break;
                  case FUNCTION_REXXADDMACRO:             WRexxAddMacro=               (WREXXADDMACRO             *)addr; break;
                  case FUNCTION_REXXDROPMACRO:            WRexxDropMacro=              (WREXXDROPMACRO            *)addr; break;
                  case FUNCTION_REXXSAVEMACROSPACE:       WRexxSaveMacroSpace=         (WREXXSAVEMACROSPACE       *)addr; break;
                  case FUNCTION_REXXLOADMACROSPACE:       WRexxLoadMacroSpace=         (WREXXLOADMACROSPACE       *)addr; break;
                  case FUNCTION_REXXQUERYMACRO:           WRexxQueryMacro=             (WREXXQUERYMACRO           *)addr; break;
                  case FUNCTION_REXXREORDERMACRO:         WRexxReorderMacro=           (WREXXREORDERMACRO         *)addr; break;
                  case FUNCTION_REXXCLEARMACROSPACE:      WRexxClearMacroSpace=        (WREXXCLEARMACROSPACE      *)addr; break;
                  case FUNCTION_REXXSETHALT:              WRexxSetHalt=                (WREXXSETHALT              *)addr; break;
                  case FUNCTION_REXXWAITFORTERMINATION:   WRexxWaitForTermination=     (WREXXWAITFORTERMINATION   *)addr; break;
                  case FUNCTION_REXXDIDREXXTERMINATE:     WRexxDidRexxTerminate=       (WREXXDIDREXXTERMINATE     *)addr; break;
               }
               break;
#endif
#if defined(INT_UNI_REXX)
            case INT_UNI_REXX:
               switch ( j )
               {
                  case FUNCTION_REXXSTART:                URexxStart =                 (UREXXSTART                *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONEXE:  URexxRegisterFunctionExe =   (UREXXREGISTERFUNCTIONEXE  *)addr; break;
                  case FUNCTION_REXXREGISTERFUNCTIONDLL:  URexxRegisterFunctionDll =   (UREXXREGISTERFUNCTIONDLL  *)addr; break;
                  case FUNCTION_REXXDEREGISTERFUNCTION:   URexxDeregisterFunction =    (UREXXDEREGISTERFUNCTION   *)addr; break;
                  case FUNCTION_REXXREGISTEREXITEXE:      URexxRegisterExitExe =       (UREXXREGISTEREXITEXE      *)addr; break;
                  case FUNCTION_REXXREGISTEREXITDLL:      URexxRegisterExitDll =       (UREXXREGISTEREXITDLL      *)addr; break;
                  case FUNCTION_REXXDEREGISTEREXIT:       URexxDeregisterExit =        (UREXXDEREGISTEREXIT       *)addr; break;
                  case FUNCTION_REXXQUERYEXIT:            URexxQueryExit =             (UREXXQUERYEXIT            *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMEXE:    URexxRegisterSubcomExe =     (UREXXREGISTERSUBCOMEXE    *)addr; break;
                  case FUNCTION_REXXDEREGISTERSUBCOM:     URexxDeregisterSubcom =      (UREXXDEREGISTERSUBCOM     *)addr; break;
                  case FUNCTION_REXXREGISTERSUBCOMDLL:    URexxRegisterSubcomDll =     (UREXXREGISTERSUBCOMDLL    *)addr; break;
                  case FUNCTION_REXXQUERYSUBCOM:          URexxQuerySubcom =           (UREXXQUERYSUBCOM          *)addr; break;
                  case FUNCTION_REXXQUERYFUNCTION:        URexxQueryFunction =         (UREXXQUERYFUNCTION        *)addr; break;
                  case FUNCTION_REXXVARIABLEPOOL:         URexxVariablePool =          (UREXXVARIABLEPOOL         *)addr; break;
                  case FUNCTION_REXXALLOCATEMEMORY:       URexxAllocateMemory =        (UREXXALLOCATEMEMORY       *)RexxTransAllocateMemory; break;
                  case FUNCTION_REXXFREEMEMORY:           URexxFreeMemory =            (UREXXFREEMEMORY           *)RexxTransFreeMemory; break;
                  case FUNCTION_REXXCREATEQUEUE:          URexxCreateQueue=            (UREXXCREATEQUEUE          *)addr; break;
                  case FUNCTION_REXXDELETEQUEUE:          URexxDeleteQueue=            (UREXXDELETEQUEUE          *)addr; break;
                  case FUNCTION_REXXQUERYQUEUE:           URexxQueryQueue=             (UREXXQUERYQUEUE           *)addr; break;
                  case FUNCTION_REXXADDQUEUE:             URexxAddQueue=               (UREXXADDQUEUE             *)addr; break;
                  case FUNCTION_REXXPULLQUEUE:            URexxPullQueue=              (UREXXPULLQUEUE            *)addr; break;
                  case FUNCTION_REXXADDMACRO:             URexxAddMacro=               (UREXXADDMACRO             *)addr; break;
                  case FUNCTION_REXXDROPMACRO:            URexxDropMacro=              (UREXXDROPMACRO            *)addr; break;
                  case FUNCTION_REXXSAVEMACROSPACE:       URexxSaveMacroSpace=         (UREXXSAVEMACROSPACE       *)addr; break;
                  case FUNCTION_REXXLOADMACROSPACE:       URexxLoadMacroSpace=         (UREXXLOADMACROSPACE       *)addr; break;
                  case FUNCTION_REXXQUERYMACRO:           URexxQueryMacro=             (UREXXQUERYMACRO           *)addr; break;
                  case FUNCTION_REXXREORDERMACRO:         URexxReorderMacro=           (UREXXREORDERMACRO         *)addr; break;
                  case FUNCTION_REXXCLEARMACROSPACE:      URexxClearMacroSpace=        (UREXXCLEARMACROSPACE      *)addr; break;
                  case FUNCTION_REXXSETHALT:              URexxSetHalt=                (UREXXSETHALT              *)addr; break;
                  case FUNCTION_REXXWAITFORTERMINATION:   URexxWaitForTermination=     (UREXXWAITFORTERMINATION   *)addr; break;
                  case FUNCTION_REXXDIDREXXTERMINATE:     URexxDidRexxTerminate=       (UREXXDIDREXXTERMINATE     *)addr; break;
               }
               break;
#endif
         }
      }
   }
   else
   {
      if ( Trace )
      {
#ifdef WIN32
         FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
         TraceString( "not found: %s\n", LoadError );
#endif
#ifdef OS2
         TraceString( "not found: %s\n", LoadError );
#endif
#ifdef UNIX
         TraceString( "not found: %s\n", strerror( errno ) );
#endif
      }
   }
   return handle;
}

static void LoadInterpreter( void )
{
   handle_type handle=(handle_type)NULL ;
   register int i=0;
   char *ptr;

#ifdef OS2
   if ( DosScanEnv( "REXXTRANS_TRACEFILE", (PSZ *)&ptr ) )
      ptr = NULL;
#else
   ptr = getenv( "REXXTRANS_TRACEFILE" );
#endif
   if ( ptr != NULL )
   {
      Trace = 1;
      strcpy( TraceFileName, ptr );
   }

   strcpy( LoadedLibrary, "" );
   if ( Trace )
   {
      TraceString( "Starting Rexx/Trans Version %s %s\n",
                   REXXTRANS_VERSION,
                   REXXTRANS_DATE );
   }
#ifdef OS2
   if ( DosScanEnv( "REXXTRANS_INTERPRETER", (PSZ *)&ptr ) )
      ptr = NULL;
#else
   ptr = getenv( "REXXTRANS_INTERPRETER" );
#endif
   if ( ptr == NULL )
   {
      for ( i = 0; MyLibraryName[ i ] != NULL; i++ )
      {
         if ( ( handle = (handle_type)FindInterpreter( MyLibraryName[ i ], i ) ) != (handle_type)NULL )
         {
            InterpreterIdx = i;
            break;
         }
      }
      if ( handle == (handle_type)NULL )
      {
/*         fprintf( stderr, "Could not find any Rexx DLL. Cannot continue.\n" ); */
/*         exit( 11 ); MH */
      }
   }
   else
   {
      for ( i = 0; MyInterpreterName[ i ] != NULL; i++ )
      {
         if ( strcmp( ptr, MyInterpreterName[ i ] ) == 0 )
         {
            InterpreterIdx = i;
         }
      }
      if ( InterpreterIdx == (-1) )
      {
#if defined(OS2) || defined(WIN32)
         fprintf( stderr, "Don't know how to handle %s Rexx DLL specified by REXXTRANS_INTERPRETER. Cannot continue.\n",
            ptr );
#else
         fprintf( stderr, "Don't know how to handle %s Rexx shared library specified by REXXTRANS_INTERPRETER. Cannot continue.\n",
            ptr );
#endif
         exit( 12 );
      }
      else
      {
         if ( FindInterpreter( MyLibraryName[InterpreterIdx], InterpreterIdx ) == (handle_type)NULL )
         {
#if defined(OS2) || defined(WIN32)
            fprintf( stderr, "Could not find %s Rexx DLL specified by REXXTRANS_INTERPRETER. Cannot continue.\n",
               ptr );
#else
            fprintf( stderr, "Could not find %s Rexx shared library specified by REXXTRANS_INTERPRETER. Cannot continue.\n",
               ptr );
#endif
            exit( 13 );
         }
      }
   }
   if ( Trace )
   {
      TraceString( "----------- Initialisation Complete - Program Execution Begins -----------\n" );
   }
   return;
}

static void FreeInterpreter( void )
{
   if (strcmp( LoadedLibrary, "" ) == 0 )
      return;
#if defined(DYNAMIC_DLOPEN)
#elif defined(DYNAMIC_HPSHLOAD)
#elif defined(DYNAMIC_OS2)
#elif defined(DYNAMIC_WIN32)
   FreeLibrary( GetModuleHandle( LoadedLibrary ) );
   if ( InterpreterIdx == INT_OBJECT_REXX )
      FreeLibrary( GetModuleHandle( OBJECT_REXX_EXTRA_DLL) );
#elif defined(DYNAMIC_BEOS)
#endif
   return;
}


/*
 * RexxStart
 */
APIRET APIENTRY RexxStart(
   LONG ArgCount,
   PRXSTRING ArgList,
   PSZ ProgramName,
   PRXSTRING Instore,
   PSZ EnvName,
   LONG CallType,
   PRXSYSEXIT Exits,
   PSHORT ReturnCode,
   PRXSTRING Result )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace
#ifdef INT_REXX_IMC
   && InterpreterIdx != INT_REXX_IMC 
#endif
   )
   {
      /*
       * We trace REXX/imc later...
       */
      TraceString( "%s %s: ArgCount: %d ArgList: %x ProgramName: \"%s\" Instore: %x EnvName: \"%s\" Calltype: %d Exits: %x ReturnCode: %x Result: %x",
         MyLibraryName[ InterpreterIdx ],
         "RexxStart()",
         ArgCount,
         ArgList,
         ProgramName,
         Instore,
         EnvName,
         CallType,
         Exits,
         ReturnCode,
         Result );
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxStart)
            rc = (*RRexxStart)(
               (LONG)      ArgCount,
               (PRXSTRING) ArgList,
               (PSZ)       ProgramName,
               (PRXSTRING) Instore,
               (PSZ)       EnvName,
               (LONG)      CallType,
               (PRXSYSEXIT)Exits,
               (PSHORT)    ReturnCode,
               (PRXSTRING) Result ) ;
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxStart)
         {
            PIRXSYSEXIT IExits=NULL;
            if ( Trace )
            {
               TraceString( "%s %s: ArgCount: %d ArgList: %x ProgramName: \"%s\" Instore: %x EnvName: \"%s\" Calltype: %d(%d) Exits: %x ReturnCode: %x Result: %x ",
                  MyLibraryName[ InterpreterIdx ],
                  "RexxStart()",
                  ArgCount,
                  ArgList,
                  ProgramName,
                  Instore,
                  EnvName,
                  CallType,
                  IRexxStartCallType( CallType ),
                  Exits,
                  ReturnCode,
                  Result );
            }
            /*
             * For each System Specified in the "Exits" parameter
             * change the value supplied (standard) to REXX/imc value
             * We also have to generate a new array of system exit pointers
             * because REXX/imc uses "short" for sysexit_code whereas
             * the "standard" is "LONG"
             */
            if ( Exits )
            {
               int i;
               int done=0;
               /*
                * Countthe number of system exits so we know how much
                * memory to allocate to the REXX/imc system exit
                * arrays
                */
               for( i = 0; ; i++ )
               {
                  if ( Exits[i].sysexit_code == RXENDLST )
                     break;
               }
               IExits = (PIRXSYSEXIT)malloc((i+1)*sizeof(IRXSYSEXIT));
               if ( IExits == NULL )
               {
                  rc = -1;
                  break;
               }
               /*
                * Now convert the Exits[] array...
                */
               for( i = 0; ; i++ )
               {
                  if ( Trace )
                  {
                     TraceString( " ExitCode[%d]: %d(%d) ExitName[%d]: \"%s\" ",
                        i,
                        Exits[i].sysexit_code,
                        IRXSYSEXITsysexit_code( Exits[i].sysexit_code),
                        i,
                        Exits[i].sysexit_code == RXENDLST ? "**RXENDLST**" : Exits[i].sysexit_name );
                  }
                  if ( Exits[i].sysexit_code == RXENDLST )
                     done = 1;
                  IExits[i].sysexit_code = IRXSYSEXITsysexit_code( Exits[i].sysexit_code) ;
                  IExits[i].sysexit_name = Exits[i].sysexit_name;
                  if ( done )
                     break;
               }
            }
            rc = (*IRexxStart)(
               (LONG)      ArgCount,
               (PRXSTRING) ArgList,
               (PSZ)       ProgramName,
               (PRXSTRING) Instore,
               (PSZ)       EnvName,
               (LONG)      IRexxStartCallType( CallType),
               (PRXSYSEXIT)IExits,
               (PSHORT)    ReturnCode,
               (PRXSTRING) Result ) ;
            if ( IExits )
               free( IExits );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
            case INT_OBJECT_REXX:
         if (ORexxStart)
            rc = (*ORexxStart)(
               (LONG)      ArgCount,
               (PRXSTRING) ArgList,
               (PSZ)       ProgramName,
               (PRXSTRING) Instore,
               (PSZ)       EnvName,
               (LONG)      CallType,
               (PRXSYSEXIT)Exits,
               (PSHORT)    ReturnCode,
               (PRXSTRING) Result ) ;
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxStart)
            rc = (*QRexxStart)(
               (int)       ArgCount,
               (PRXSTRING) ArgList,
               (LPCSTR)    ProgramName,
               (PRXSTRING) Instore,
               (LPCSTR)    EnvName,
               (int)       CallType,
               (PRXSYSEXIT)Exits,
               (LPSHORT)   ReturnCode,
               (PRXSTRING) Result ) ;
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxStart)
            rc = (*WRexxStart)(
               (int)       ArgCount,
               (PRXSTRING) ArgList,
               (LPCSTR)    ProgramName,
               (PRXSTRING) Instore,
               (LPCSTR)    EnvName,
               (int)       CallType,
               (PRXSYSEXIT)Exits,
               (LPSHORT)   ReturnCode,
               (PRXSTRING) Result ) ;
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxStart)
            rc = (*URexxStart)(
               (LONG)      ArgCount,
               (PRXSTRING) ArgList,
               (PSZ)       ProgramName,
               (PRXSTRING) Instore,
               (PSZ)       EnvName,
               (LONG)      CallType,
               (PRXSYSEXIT)Exits,
               (PSHORT)    ReturnCode,
               (PRXSTRING) Result ) ;
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> ReturnCode %d ResultString \"%s\" Result: %d\n",
                   (Result && Result->strptr) ? Result->strptr : "",
                   ReturnCode, 
                   rc );
   }
   return rc;
}

/*
 * Variable pool
 */
APIRET APIENTRY RexxVariablePool(
   PSHVBLOCK RequestBlockList )
{
   APIRET rc=RXSHV_NOAVL;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      PSHVBLOCK tmp=RequestBlockList;
      TraceString( "%s %s: RequestBlockList %x\n",
         MyLibraryName[ InterpreterIdx ],
         "RexxVariablePool()",
         RequestBlockList );
      while(tmp)
      {
         switch( tmp->shvcode )
         {
            case RXSHV_SET: 
               TraceString("in    RXSHV_SET: shvname: \"%s\" shvnamelen: %d shvvalue: \"%s\" shvvaluelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen, tmp->shvvalue.strptr, tmp->shvvaluelen );
               break;
            case RXSHV_SYSET:
               TraceString("in  RXSHV_SYSET: shvname: \"%s\" shvnamelen: %d shvvalue: \"%s\" shvvaluelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen, tmp->shvvalue.strptr, tmp->shvvaluelen );
               break;
            case RXSHV_FETCH:
               TraceString("in  RXSHV_FETCH: shvname: \"%s\" shvnamelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen );
               break;
            case RXSHV_DROPV:
               TraceString("in  RXSHV_DROPV: shvname: \"%s\" shvnamelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen );
               break;
            case RXSHV_SYDRO:
               TraceString("in  RXSHV_SYDRO: shvname: \"%s\" shvnamelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen );
               break;
            case RXSHV_NEXTV:
               TraceString("in  RXSHV_NEXTV: shvname: \"%s\" shvnamelen: %d\n",
                  tmp->shvname.strptr, tmp->shvnamelen );
               break;
            case RXSHV_PRIV:
               TraceString("in   RXSHV_PRIV\n" );
               break;
            case RXSHV_EXIT:
               TraceString("in   RXSHV_EXIT\n" );
               break;
            default:
               break;
         }
         tmp = tmp->shvnext;
      }
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxVariablePool)
            rc = (*RRexxVariablePool)(
               (PSHVBLOCK) RequestBlockList);
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxVariablePool)
         {
            rc = (*IRexxVariablePool)(
               (PSHVBLOCK) RequestBlockList);
            rc = IREXXVARIABLEPOOLrc( rc);
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxVariablePool)
            rc = (*ORexxVariablePool)(
               (PSHVBLOCK) RequestBlockList);
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxVariablePool)
            rc = (*QRexxVariablePool)(
               (PSHVBLOCK) RequestBlockList);
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxVariablePool)
            rc = (*WRexxVariablePool)(
               (PSHVBLOCK) RequestBlockList);
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxVariablePool)
            rc = (*URexxVariablePool)(
               (PSAASHVBLK) RequestBlockList);
         break;
#endif
   }
   if (Trace)
   {
      PSHVBLOCK tmp=RequestBlockList;
      while(tmp)
      {
         switch( tmp->shvcode )
         {
            case RXSHV_SET: 
               TraceString("out   RXSHV_SET: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_SYSET:
               TraceString("out RXSHV_SYSET: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_FETCH:
               TraceString("out RXSHV_FETCH: shvret: %x shvvalue: \"%s\" shvvaluelen: %d\n",
                  tmp->shvret, tmp->shvvalue.strptr, tmp->shvvaluelen );
               break;
            case RXSHV_DROPV:
               TraceString("out RXSHV_DROPV: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_SYDRO:
               TraceString("out RXSHV_SYDRO: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_NEXTV:
               TraceString("out RXSHV_NEXTV: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_PRIV:
               TraceString("out  RXSHV_PRIV: shvret: %x\n",
                  tmp->shvret );
               break;
            case RXSHV_EXIT:
               TraceString("out  RXSHV_EXIT: shvret: %x\n",
                  tmp->shvret );
               break;
            default:
               break;
         }
         tmp = tmp->shvnext;
      }
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/*
 * System Exit functions
 */
APIRET APIENTRY RexxRegisterExitExe(
   PSZ EnvName,
   PFN EntryPoint, 
   PUCHAR UserArea )
{
   APIRET rc=RXEXIT_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: EnvName \"%s\" EntryPoint %x UserArea %x ",
         MyLibraryName[ InterpreterIdx ],
         "RexxRegisterExitExe()",
         EnvName,
         EntryPoint,
         UserArea );
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxRegisterExitExe)
         {
            rc = (*RRexxRegisterExitExe)(
               (PSZ)       EnvName,
               (PFN)       EntryPoint,
               (PUCHAR)    UserArea );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxRegisterExitExe)
         {
            rc = (*IRexxRegisterExitExe)(
               (PSZ)               EnvName,
               (RexxExitHandler *) EntryPoint,
               (PUCHAR)            UserArea );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxRegisterExitExe)
         {
            rc = (*ORexxRegisterExitExe)(
               (PSZ)       EnvName,
               (PFN)       EntryPoint,
               (PUCHAR)    UserArea );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxRegisterExitExe)
         {
            rc = (*QRexxRegisterExitExe)(
               (LPCSTR)    EnvName,
               (FARPROC)   EntryPoint,
               (LPBYTE)    UserArea );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxRegisterExitExe)
         {
            rc = (*WRexxRegisterExitExe)(
               (LPCSTR)    EnvName,
               (FARPROC)   EntryPoint,
               (LPBYTE)    UserArea );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxRegisterExitExe)
         {
            rc = (*URexxRegisterExitExe)(
               (PSZ)       EnvName,
               (PFN)       EntryPoint,
               (PUCHAR)    UserArea );
         }
         break;
#endif
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxRegisterExitDll(
   PSZ EnvName,
   PSZ ModuleName,
   PSZ ProcedureName,
   PUCHAR UserArea,
   ULONG DropAuth )
{
   APIRET rc=RXEXIT_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: EnvName \"%s\" ModuleName \"%s\" ProcedureName \"%s\" UserArea %x DropAuth %d ",
         MyLibraryName[ InterpreterIdx ],
         "RexxRegisterExitDll()",
         EnvName,
         ModuleName,
         ProcedureName,
         UserArea,
         DropAuth );
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxRegisterExitDll)
         {
            rc = (*RRexxRegisterExitDll)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PSZ)       ProcedureName,
               (PUCHAR)    UserArea,
               (ULONG)     DropAuth );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxRegisterExitDll)
         {
            rc = (*IRexxRegisterExitDll)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PSZ)       ProcedureName,
               (PUCHAR)    UserArea,
               (ULONG)     DropAuth );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxRegisterExitDll)
         {
            rc = (*ORexxRegisterExitDll)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PSZ)       ProcedureName,
               (PUCHAR)    UserArea,
               (ULONG)     DropAuth );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxRegisterExitDll)
         {
            rc = (*QRexxRegisterExitDll)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName,
               (LPCSTR)    ProcedureName,
               (LPBYTE)    UserArea,
               (UINT)      DropAuth );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxRegisterExitDll)
         {
            rc = (*WRexxRegisterExitDll)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName,
               (LPCSTR)    ProcedureName,
               (LPBYTE)    UserArea,
               (UINT)      DropAuth );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxRegisterExitDll)
         {
            rc = (*URexxRegisterExitDll)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PSZ)       ProcedureName,
               (PUCHAR)    UserArea,
               (ULONG)     DropAuth );
         }
         break;
#endif
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxDeregisterExit(
   PSZ EnvName,
   PSZ ModuleName )
{
   APIRET rc=RXEXIT_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: EnvName \"%s\" ModuleName \"%s\" ",
         MyLibraryName[ InterpreterIdx ],
         "RexxDeregisterExit()",
         EnvName,
         (ModuleName == NULL) ? "NULL" : ModuleName );
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxDeregisterExit)
         {
            rc = (*RRexxDeregisterExit)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxDeregisterExit)
         {
            rc = (*IRexxDeregisterExit)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxDeregisterExit)
         {
            rc = (*ORexxDeregisterExit)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxDeregisterExit)
         {
            rc = (*QRexxDeregisterExit)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxDeregisterExit)
         {
            rc = (*WRexxDeregisterExit)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxDeregisterExit)
         {
            rc = (*URexxDeregisterExit)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName );
         }
         break;
#endif
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQueryExit(
   PSZ EnvName,
   PSZ ModuleName,
   PUSHORT Flag,
   PUCHAR UserArea )
{
   APIRET rc=RXEXIT_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: EnvName \"%s\" ModuleName \"%s\" Flag %d UserArea %x ",
         MyLibraryName[ InterpreterIdx ],
         "RexxQueryExit()",
         EnvName,
         ModuleName,
         Flag,
         UserArea );
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxQueryExit)
         {
            rc = (*RRexxQueryExit)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PUSHORT)   Flag,
               (PUCHAR)    UserArea ) ;
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxQueryExit)
         {
            rc = (*IRexxQueryExit)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PUSHORT)   Flag,
               (PUCHAR)    UserArea ) ;
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxQueryExit)
         {
            rc = (*ORexxQueryExit)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PUSHORT)   Flag,
               (PUCHAR)    UserArea ) ;
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxQueryExit)
         {
            rc = (*QRexxQueryExit)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName,
               (LPWORD)    Flag,
               (LPBYTE)    UserArea ) ;
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxQueryExit)
         {
            rc = (*WRexxQueryExit)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName,
               (LPWORD)    Flag,
               (LPBYTE)    UserArea ) ;
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxQueryExit)
         {
            rc = (*URexxQueryExit)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PUSHORT)   Flag,
               (PUCHAR)    UserArea ) ;
         }
         break;
#endif
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/*
 * Subcommands
 */
APIRET APIENTRY RexxRegisterSubcomExe(
   PSZ EnvName,
   PFN EntryPoint,
   PUCHAR UserArea )
{
   APIRET rc=RXSUBCOM_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: EnvName \"%s\" EntryPoint %x UserArea %x ",
         MyLibraryName[ InterpreterIdx ],
         "RexxRegisterSubcomExe()",
         EnvName,
         EntryPoint,
         UserArea );
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxRegisterSubcomExe)
         {
            rc = (*RRexxRegisterSubcomExe)(
               (PSZ)       EnvName,
               (PFN)       EntryPoint,
               (PUCHAR)    UserArea );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxRegisterSubcomExe)
         {
            rc = (*IRexxRegisterSubcomExe)(
               (PSZ)                 EnvName,
               (RexxSubcomHandler *) EntryPoint,
               (PUCHAR)              UserArea );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxRegisterSubcomExe)
         {
            rc = (*ORexxRegisterSubcomExe)(
               (PSZ)       EnvName,
               (PFN)       EntryPoint,
               (PUCHAR)    UserArea );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxRegisterSubcomExe)
         {
            rc = (*QRexxRegisterSubcomExe)(
               (LPCSTR)    EnvName,
               (FARPROC)   EntryPoint,
               (LPBYTE)    UserArea );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxRegisterSubcomExe)
         {
            rc = (*WRexxRegisterSubcomExe)(
               (LPCSTR)    EnvName,
               (FARPROC)   EntryPoint,
               (LPBYTE)    UserArea );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxRegisterSubcomExe)
         {
            rc = (*URexxRegisterSubcomExe)(
               (PSZ)       EnvName,
               (PFN)       EntryPoint,
               (PUCHAR)    UserArea );
         }
         break;
#endif
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxDeregisterSubcom(
   PSZ EnvName,
   PSZ ModuleName )
{
   APIRET rc=RXSUBCOM_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: EnvName \"%s\" ModuleName \"%s\" ",
         MyLibraryName[ InterpreterIdx ],
         "RexxDeregisterSubcom()",
         EnvName,
         (ModuleName == NULL) ? "NULL" : ModuleName );
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxDeregisterSubcom)
         {
            rc = (*RRexxDeregisterSubcom)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxDeregisterSubcom)
         {
            rc = (*IRexxDeregisterSubcom)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxDeregisterSubcom)
         {
            rc = (*ORexxDeregisterSubcom)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxDeregisterSubcom)
         {
            rc = (*QRexxDeregisterSubcom)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxDeregisterSubcom)
         {
            rc = (*WRexxDeregisterSubcom)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxDeregisterSubcom)
         {
            rc = (*URexxDeregisterSubcom)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName );
         }
         break;
#endif
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxRegisterSubcomDll(
   PSZ EnvName,
   PSZ ModuleName,
   PSZ ProcedureName,
   PUCHAR UserArea,
   ULONG DropAuth )
{
   APIRET rc=RXSUBCOM_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: EnvName \"%s\" ModuleName \"%s\" ProcedureName \"%s\" UserArea %x DropAuth %d ",
         MyLibraryName[ InterpreterIdx ],
         "RexxQuerySubcom()",
         EnvName,
         ModuleName,
         ProcedureName,
         UserArea,
         DropAuth );
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxRegisterSubcomDll)
         {
            rc = (*RRexxRegisterSubcomDll)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PSZ)       ProcedureName,
               (PUCHAR)    UserArea,
               (ULONG)     DropAuth );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxRegisterSubcomDll)
         {
            rc = (*IRexxRegisterSubcomDll)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PSZ)       ProcedureName,
               (PUCHAR)    UserArea,
               (ULONG)     DropAuth );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxRegisterSubcomDll)
         {
            rc = (*ORexxRegisterSubcomDll)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PSZ)       ProcedureName,
               (PUCHAR)    UserArea,
               (ULONG)     DropAuth );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxRegisterSubcomDll)
         {
            rc = (*QRexxRegisterSubcomDll)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName,
               (LPCSTR)    ProcedureName,
               (LPBYTE)    UserArea,
               (UINT)      DropAuth );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxRegisterSubcomDll)
         {
            rc = (*WRexxRegisterSubcomDll)(
               (LPCSTR)    EnvName,
               (LPCSTR)    ModuleName,
               (LPCSTR)    ProcedureName,
               (LPBYTE)    UserArea,
               (UINT)      DropAuth );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxRegisterSubcomDll)
         {
            rc = (*URexxRegisterSubcomDll)(
               (PSZ)       EnvName,
               (PSZ)       ModuleName,
               (PSZ)       ProcedureName,
               (PUCHAR)    UserArea,
               (ULONG)     DropAuth );
         }
         break;
#endif
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQuerySubcom(
   PSZ Envname,
   PSZ ModuleName,
   PUSHORT Flag,
   PUCHAR UserArea )
{
   APIRET rc=RXSUBCOM_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: EnvName \"%s\" ModuleName \"%s\" Flag %d UserArea %x ",
         MyLibraryName[ InterpreterIdx ],
         "RexxQuerySubcom()",
         Envname,
         ModuleName,
         Flag,
         UserArea );
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxQuerySubcom)
         {
            rc = (*RRexxQuerySubcom)(
               (PSZ)       Envname,
               (PSZ)       ModuleName,
               (PUSHORT)   Flag,
               (PUCHAR)    UserArea );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxQuerySubcom)
         {
            rc = (*IRexxQuerySubcom)(
               (PSZ)       Envname,
               (PSZ)       ModuleName,
               (PUSHORT)   Flag,
               (PUCHAR)    UserArea );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxQuerySubcom)
         {
            rc = (*ORexxQuerySubcom)(
               (PSZ)       Envname,
               (PSZ)       ModuleName,
               (PUSHORT)   Flag,
               (PUCHAR)    UserArea );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxQuerySubcom)
         {
            rc = (*QRexxQuerySubcom)(
               (LPCSTR)    Envname,
               (LPCSTR)    ModuleName,
               (LPWORD)    Flag,
               (LPBYTE)    UserArea );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxQuerySubcom)
         {
            rc = (*WRexxQuerySubcom)(
               (LPCSTR)    Envname,
               (LPCSTR)    ModuleName,
               (LPWORD)    Flag,
               (LPBYTE)    UserArea );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxQuerySubcom)
         {
            rc = (*URexxQuerySubcom)(
               (PSZ)       Envname,
               (PSZ)       ModuleName,
               (PUSHORT)   Flag,
               (PUCHAR)    UserArea );
         }
         break;
#endif
   }
   if ( Trace )
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/*
 * External functions
 */
APIRET APIENTRY RexxRegisterFunctionExe(
   PSZ name,
   PFN EntryPoint )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: Name \"%s\" Entrypoint %x ",
         MyLibraryName[ InterpreterIdx ],
         "RexxRegisterFunctionExe()",
         name,
         EntryPoint );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxRegisterFunctionExe)
            rc = (*RRexxRegisterFunctionExe)(
               (PSZ)       name,
               (PFN)       EntryPoint );
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxRegisterFunctionExe)
            rc = (*IRexxRegisterFunctionExe)(
               (PSZ)                   name,
               (RexxFunctionHandler *) EntryPoint );
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxRegisterFunctionExe)
            rc = (*ORexxRegisterFunctionExe)(
               (PSZ)       name,
               (PFN)       EntryPoint );
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxRegisterFunctionExe)
            rc = (*QRexxRegisterFunctionExe)(
               (LPCSTR)    name,
               (FARPROC)   EntryPoint );
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxRegisterFunctionExe)
            rc = (*WRexxRegisterFunctionExe)(
               (LPCSTR)    name,
               (FARPROC)   EntryPoint );
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxRegisterFunctionExe)
            rc = (*URexxRegisterFunctionExe)(
               (PSZ)       name,
               (PFN)       EntryPoint );
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxRegisterFunctionDll(
   PSZ ExternalName,
   PSZ LibraryName,
   PSZ InternalName )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: External %s Library %s Internal %s ",
         MyLibraryName[ InterpreterIdx ],
         "RexxRegisterFunctionDll()",
         ExternalName,
         LibraryName,
         InternalName );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxRegisterFunctionDll)
            rc = (*RRexxRegisterFunctionDll)(
               (PSZ)       ExternalName,
               (PSZ)       LibraryName,
               (PSZ)       InternalName );
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxRegisterFunctionDll)
            rc = (*IRexxRegisterFunctionDll)(
               (PSZ)       ExternalName,
               (PSZ)       LibraryName,
               (PSZ)       InternalName );
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxRegisterFunctionDll)
            rc = (*ORexxRegisterFunctionDll)(
               (PSZ)       ExternalName,
               (PSZ)       LibraryName,
               (PSZ)       InternalName );
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxRegisterFunctionDll)
            rc = (*QRexxRegisterFunctionDll)(
               (LPCSTR)    ExternalName,
               (LPCSTR)    LibraryName,
               (LPCSTR)    InternalName );
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxRegisterFunctionDll)
            rc = (*WRexxRegisterFunctionDll)(
               (LPCSTR)    ExternalName,
               (LPCSTR)    LibraryName,
               (LPCSTR)    InternalName );
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxRegisterFunctionDll)
            rc = (*URexxRegisterFunctionDll)(
               (PSZ)       ExternalName,
               (PSZ)       LibraryName,
               (PSZ)       InternalName );
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxDeregisterFunction(
   PSZ name )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: %s ",
         MyLibraryName[ InterpreterIdx ],
         "RexxDeregisterFunction()",
         name );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxDeregisterFunction)
            rc = (*RRexxDeregisterFunction)(
               (PSZ)       name );
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxDeregisterFunction)
            rc = (*IRexxDeregisterFunction)(
               (PSZ)       name );
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxDeregisterFunction)
            rc = (*ORexxDeregisterFunction)(
               (PSZ)       name );
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxDeregisterFunction)
            rc = (*QRexxDeregisterFunction)(
               (LPCSTR)    name );
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxDeregisterFunction)
            rc = (*WRexxDeregisterFunction)(
               (LPCSTR)    name );
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxDeregisterFunction)
            rc = (*URexxDeregisterFunction)(
               (PSZ)       name );
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQueryFunction(
   PSZ name )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: %s ",
         MyLibraryName[ InterpreterIdx ],
         "RexxQueryFunction()",
         name );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxQueryFunction)
            rc = (*RRexxQueryFunction)(
               (PSZ)       name );
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxQueryFunction)
            rc = (*IRexxQueryFunction)(
               (PSZ)       name );
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxQueryFunction)
            rc = (*ORexxQueryFunction)(
               (PSZ)       name );
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxQueryFunction)
            rc = (*QRexxQueryFunction)(
               (LPCSTR)    name );
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxQueryFunction)
            rc = (*WRexxQueryFunction)(
               (LPCSTR)    name );
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxQueryFunction)
            rc = (*URexxQueryFunction)(
               (PSZ)       name );
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/* ============================================================= */
/* Named queue interface */

APIRET APIENTRY RexxCreateQueue( PSZ Buffer,
                                 ULONG BuffLen,
                                 PSZ RequestedName,
                                 ULONG* DupFlag)
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: Buffer: %x BuffLen: %d RequestedName: \"%s\" DupFlag: %ld ",
         MyLibraryName[ InterpreterIdx ],
         "RexxCreateQueue()",
         Buffer,
         BuffLen,
         RequestedName,
         DupFlag );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxCreateQueue)
         {
            rc = (*RRexxCreateQueue)( 
               (PSZ)       Buffer,
               (ULONG)     BuffLen,
               (PSZ)       RequestedName,
               (ULONG*)    DupFlag );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxCreateQueue)
         {
            rc = (*IRexxCreateQueue)(
               (PSZ)       Buffer,
               (ULONG)     BuffLen,
               (PSZ)       RequestedName,
               (ULONG*)    DupFlag );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxCreateQueue)
         {
            rc = (*ORexxCreateQueue)(
               (PSZ)       Buffer,
               (ULONG)     BuffLen,
               (PSZ)       RequestedName,
               (ULONG*)    DupFlag );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxCreateQueue)
         {
            rc = (*QRexxCreateQueue)(
               (PSZ)       Buffer,
               (ULONG)     BuffLen,
               (PSZ)       RequestedName,
               (ULONG*)    DupFlag );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxCreateQueue)
         {
            rc = (*WRexxCreateQueue)(
               (PSZ)       Buffer,
               (ULONG)     BuffLen,
               (PSZ)       RequestedName,
               (ULONG*)    DupFlag );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxCreateQueue)
         {
            rc = (*URexxCreateQueue)(
               (PSZ)       Buffer,
               (ULONG)     BuffLen,
               (PSZ)       RequestedName,
               (ULONG*)    DupFlag );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d ", rc );
      if ( rc == RXQUEUE_OK )
         TraceString( "Buffer: \"%s\" DupFlag: %d\n", rc );
      else
         TraceString( "\n" );
   }
   return rc;
}

APIRET APIENTRY RexxDeleteQueue( PSZ QueueName )
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: QueueName: \"%s\" ",
         MyLibraryName[ InterpreterIdx ],
         "RexxDeleteQueue()",
         QueueName );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxDeleteQueue)
         {
            rc = (*RRexxDeleteQueue)( 
               (PSZ)       QueueName );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxDeleteQueue)
         {
            rc = (*IRexxDeleteQueue)(
               (PSZ)       QueueName );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxDeleteQueue)
         {
            rc = (*ORexxDeleteQueue)(
               (PSZ)       QueueName );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxDeleteQueue)
         {
            rc = (*QRexxDeleteQueue)(
               (PSZ)       QueueName );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxDeleteQueue)
         {
            rc = (*WRexxDeleteQueue)(
               (PSZ)       QueueName );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxDeleteQueue)
         {
            rc = (*URexxDeleteQueue)(
               (PSZ)       QueueName );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQueryQueue( PSZ QueueName, 
                                ULONG* Count)
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: QueueName: \"%s\" Count: %x ",
         MyLibraryName[ InterpreterIdx ],
         "RexxQueryQueue()",
         QueueName,
         Count );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxQueryQueue)
         {
            rc = (*RRexxQueryQueue)( 
               (PSZ)       QueueName,
               (ULONG*)    Count );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxQueryQueue)
         {
            rc = (*IRexxQueryQueue)(
               (PSZ)       QueueName,
               (ULONG*)    Count );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxQueryQueue)
         {
            rc = (*ORexxQueryQueue)(
               (PSZ)       QueueName,
               (ULONG*)    Count );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxQueryQueue)
         {
            rc = (*QRexxQueryQueue)(
               (PSZ)       QueueName,
               (ULONG*)    Count );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxQueryQueue)
         {
            rc = (*WRexxQueryQueue)(
               (PSZ)       QueueName,
               (ULONG*)    Count );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxQueryQueue)
         {
            rc = (*URexxQueryQueue)(
               (PSZ)       QueueName,
               (ULONG*)    Count );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d ", rc );
      if ( rc == RXQUEUE_OK )
         TraceString( "Count: %ld\n", *Count );
      else
         TraceString( "\n" );
   }
   return rc;
}

APIRET APIENTRY RexxAddQueue( PSZ QueueName, 
                              PRXSTRING EntryData, 
                              ULONG AddFlag)
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: QueueName: \"%s\" AddFlag: %ld ",
         MyLibraryName[ InterpreterIdx ],
         "RexxAddQueue()",
         QueueName,
         AddFlag );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxAddQueue)
         {
            rc = (*RRexxAddQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) EntryData,
               (ULONG)     AddFlag );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxAddQueue)
         {
            rc = (*IRexxAddQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) EntryData,
               (ULONG)     AddFlag );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxAddQueue)
         {
            rc = (*ORexxAddQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) EntryData,
               (ULONG)     AddFlag );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxAddQueue)
         {
            rc = (*QRexxAddQueue)(
               (LPCSTR)    QueueName,
               (PRXSTRING) EntryData,
               (ULONG)     AddFlag );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxAddQueue)
         {
            rc = (*WRexxAddQueue)(
               (LPCSTR)    QueueName,
               (PRXSTRING) EntryData,
               (ULONG)     AddFlag );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxAddQueue)
         {
            rc = (*URexxAddQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) EntryData,
               (ULONG)     AddFlag );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxPullQueue( PSZ QueueName, 
                               PRXSTRING DataBuf,   
                               PDATETIME TimeStamp,
                               ULONG WaitFlag)
{
   APIRET rc=RXQUEUE_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: QueueName: \"%s\" DataBuf: %x TimeStamp: %x WaitFlag: %ld ",
         MyLibraryName[ InterpreterIdx ],
         "RexxPullQueue()",
         QueueName,
         DataBuf,
         TimeStamp,
         WaitFlag );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxPullQueue)
         {
            rc = (*RRexxPullQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) DataBuf,
               (PDATETIME) TimeStamp,
               (ULONG)     WaitFlag );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxPullQueue)
         {
            rc = (*IRexxPullQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) DataBuf,
               (PDATETIME) TimeStamp,
               (ULONG)     WaitFlag );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxPullQueue)
         {
            rc = (*ORexxPullQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) DataBuf,
               (PDATETIME) TimeStamp,
               (ULONG)     WaitFlag );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxPullQueue)
         {
            rc = (*QRexxPullQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) DataBuf,
               (PDATETIME) TimeStamp,
               (ULONG)     WaitFlag );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxPullQueue)
         {
            rc = (*WRexxPullQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) DataBuf,
               (PDATETIME) TimeStamp,
               (ULONG)     WaitFlag );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxPullQueue)
         {
            rc = (*URexxPullQueue)(
               (PSZ)       QueueName,
               (PRXSTRING) DataBuf,
               (PDATETIME) TimeStamp,
               (ULONG)     WaitFlag );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
      if ( rc == RXQUEUE_OK )
         TraceString( "DataBuf->strlength: %ld DataBuf->strptr: \"%s\"\n", 
            DataBuf->strlength, 
            DataBuf->strptr );
      else
         TraceString( "\n" );
   }
   return rc;
}

/* ============================================================= */
/* Asynchronous Request Rexx API interface */

APIRET APIENTRY RexxSetHalt( LONG pid,
                             LONG tid)
{
   APIRET rc=RXARI_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: pid: %ld tid: %ld ",
         MyLibraryName[ InterpreterIdx ],
         "RexxSetHalt()",
         pid,
         tid );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxSetHalt)
         {
            rc = (*RRexxSetHalt)(
               (LONG)      pid,
               (LONG)      tid);
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxSetHalt)
         {
            rc = (*IRexxSetHalt)(
               (LONG)      pid,
               (LONG)      tid);
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxSetHalt)
         {
            rc = (*ORexxSetHalt)(
               (LONG)      pid,
               (LONG)      tid);
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxSetHalt)
         {
            rc = (*QRexxSetHalt)(
               (LONG)      pid,
               (LONG)      tid);
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxSetHalt)
         {
            rc = (*WRexxSetHalt)(
               (LONG)      pid,
               (LONG)      tid);
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxSetHalt)
         {
            rc = (*URexxSetHalt)(
               (LONG)      pid,
               (LONG)      tid);
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

/* ============================================================= */
/* MacroSpace Rexx API interface */

APIRET APIENTRY RexxAddMacro( PSZ FuncName,
                              PSZ SourceFile,
                              ULONG Position )
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: FuncName: \"%s\" SourceFile: \"%s\" Position: %ld ",
         MyLibraryName[ InterpreterIdx ],
         "RexxAddMacro()",
         FuncName,
         SourceFile,
         Position );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINALD:
         if (RRexxAddMacro)
         {
            rc = (*RRexxAddMacro)( 
               (PSZ)       FuncName,
               (PSZ)       SourceFile,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxAddMacro)
         {
            rc = (*IRexxAddMacro)(
               (PSZ)       FuncName,
               (PSZ)       SourceFile,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxAddMacro)
         {
            rc = (*ORexxAddMacro)(
               (PSZ)       FuncName,
               (PSZ)       SourceFile,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxAddMacro)
         {
            rc = (*QRexxAddMacro)(
               (PSZ)       FuncName,
               (PSZ)       SourceFile,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxAddMacro)
         {
            rc = (*WRexxAddMacro)(
               (PSZ)       FuncName,
               (PSZ)       SourceFile,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxAddMacro)
         {
            rc = (*URexxAddMacro)(
               (PSZ)       FuncName,
               (PSZ)       SourceFile,
               (ULONG)     Position );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxDropMacro( PSZ FuncName)
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: FuncName: \"%s\" ",
         MyLibraryName[ InterpreterIdx ],
         "RexxDropMacro()",
         FuncName );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
         if (RRexxDropMacro)
         {
            rc = (*RRexxDropMacro)( 
               (PSZ)       FuncName );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxDropMacro)
         {
            rc = (*IRexxDropMacro)(
               (PSZ)       FuncName );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxDropMacro)
         {
            rc = (*ORexxDropMacro)(
               (PSZ)       FuncName );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxDropMacro)
         {
            rc = (*QRexxDropMacro)(
               (PSZ)       FuncName );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxDropMacro)
         {
            rc = (*WRexxDropMacro)(
               (PSZ)       FuncName );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxDropMacro)
         {
            rc = (*URexxDropMacro)(
               (PSZ)       FuncName );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxSaveMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile)
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      unsigned int i;

      TraceString( "%s %s: FuncCount %d MacroLibFile \"%s\" ",
         MyLibraryName[ InterpreterIdx ],
         "RexxSaveMacroSpace()",
         FuncCount, MacroLibFile );
      if ( FuncCount && FuncNames )
      {
         for ( i = 0; i < FuncCount; i++ )
         {
            TraceString( "%s %s: FuncName \"%s\" ",
               MyLibraryName[ InterpreterIdx ],
               "RexxSaveMacroSpace()",
               FuncNames[i] );
         }
      }
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxSaveMacroSpace)
         {
            rc = (*RRexxSaveMacroSpace)( 
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxSaveMacroSpace)
         {
            rc = (*IRexxSaveMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxSaveMacroSpace)
         {
            rc = (*ORexxSaveMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxSaveMacroSpace)
         {
            rc = (*QRexxSaveMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxSaveMacroSpace)
         {
            rc = (*WRexxSaveMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxSaveMacroSpace)
         {
            rc = (*URexxSaveMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxLoadMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile)
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      unsigned int i;

      TraceString( "%s %s: FuncCount %d MacroLibFile \"%s\" ",
         MyLibraryName[ InterpreterIdx ],
         "RexxLoadMacroSpace()",
         FuncCount, MacroLibFile );
      if ( FuncCount && FuncNames )
      {
         for ( i = 0; i < FuncCount; i++ )
         {
            TraceString( "%s %s: FuncName \"%s\" ",
               MyLibraryName[ InterpreterIdx ],
               "RexxLoadMacroSpace()",
               FuncNames[i] );
         }
      }
   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxLoadMacroSpace)
         {
            rc = (*RRexxLoadMacroSpace)( 
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxLoadMacroSpace)
         {
            rc = (*IRexxLoadMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxLoadMacroSpace)
         {
            rc = (*ORexxLoadMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxLoadMacroSpace)
         {
            rc = (*QRexxLoadMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxLoadMacroSpace)
         {
            rc = (*WRexxLoadMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxLoadMacroSpace)
         {
            rc = (*URexxLoadMacroSpace)(
               (ULONG)     FuncCount,
               (PSZ*)      FuncNames,
               (PSZ)       MacroLibFile
                );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxQueryMacro( PSZ FuncName,
                                PUSHORT Position )
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: FuncName: \"%s\" Position: %x ",
         MyLibraryName[ InterpreterIdx ],
         "RexxQueryMacro()",
         FuncName,
         Position );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxQueryMacro)
         {
            rc = (*RRexxQueryMacro)( 
               (PSZ)       FuncName,
               (PUSHORT)   Position );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxQueryMacro)
         {
            rc = (*IRexxQueryMacro)(
               (PSZ)       FuncName,
               (PUSHORT)   Position );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxQueryMacro)
         {
            rc = (*ORexxQueryMacro)(
               (PSZ)       FuncName,
               (PUSHORT)   Position );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxQueryMacro)
         {
            rc = (*QRexxQueryMacro)(
               (PSZ)       FuncName,
               (PUSHORT)   Position );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxQueryMacro)
         {
            rc = (*WRexxQueryMacro)(
               (PSZ)       FuncName,
               (PUSHORT)   Position );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxQueryMacro)
         {
            rc = (*URexxQueryMacro)(
               (PSZ)       FuncName,
               (PUSHORT)   Position );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
      if ( rc == RXMACRO_OK )
         TraceString( "Position: %d\n",
            *Position );
      else
         TraceString( "\n" );
   }
   return rc;
}

APIRET APIENTRY RexxReorderMacro( PSZ FuncName,
                                  ULONG Position )
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: FuncName: \"%s\" Position: %ld",
         MyLibraryName[ InterpreterIdx ],
         "RexxReorderMacro()",
         FuncName,
         Position );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxReorderMacro)
         {
            rc = (*RRexxReorderMacro)( 
               (PSZ)       FuncName,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxReorderMacro)
         {
            rc = (*IRexxReorderMacro)(
               (PSZ)       FuncName,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxReorderMacro)
         {
            rc = (*ORexxReorderMacro)(
               (PSZ)       FuncName,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxReorderMacro)
         {
            rc = (*QRexxReorderMacro)(
               (PSZ)       FuncName,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxReorderMacro)
         {
            rc = (*WRexxReorderMacro)(
               (PSZ)       FuncName,
               (ULONG)     Position );
         }
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxReorderMacro)
         {
            rc = (*URexxReorderMacro)(
               (PSZ)       FuncName,
               (ULONG)     Position );
         }
         break;
#endif
   }
   if (Trace)
   {
      TraceString( "<=> Result: %d\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxClearMacroSpace( VOID )
{
   APIRET rc=RXMACRO_NOT_FOUND;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s:",
         MyLibraryName[ InterpreterIdx ],
         "RexxClearMacroSpace()" );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxClearMacroSpace)
            rc = (*RRexxClearMacroSpace)(
                 );
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxClearMacroSpace)
            rc = (*IRexxClearMacroSpace)(
                 );
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxClearMacroSpace)
            rc = (*ORexxClearMacroSpace)(
                 );
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxClearMacroSpace)
            rc = (*QRexxClearMacroSpace)(
                 );
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxClearMacroSpace)
            rc = (*WRexxClearMacroSpace)(
                 );
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxClearMacroSpace)
            rc = (*URexxClearMacroSpace)(
                 );
         break;
#endif
   }
   if (Trace)
   {
      TraceString( ": Result: %x\n", rc );
   }
   return rc;
}

PVOID APIENTRY RexxAllocateMemory(
   ULONG size )
{
   PVOID rc=NULL;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: %d",
         MyLibraryName[ InterpreterIdx ],
         "RexxAllocateMemory()",
         size );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxAllocateMemory)
            rc = (*RRexxAllocateMemory)(
               (ULONG)     size );
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxAllocateMemory)
            rc = (*IRexxAllocateMemory)(
               (ULONG)     size );
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxAllocateMemory)
            rc = (*ORexxAllocateMemory)(
               (ULONG)     size );
         else
            rc = RexxTransAllocateMemory(
               (ULONG)     size );
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxAllocateMemory)
            rc = (*QRexxAllocateMemory)(
               (ULONG)     size );
         else
            rc = RexxTransAllocateMemory(
               (ULONG)     size );
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxAllocateMemory)
            rc = (*WRexxAllocateMemory)(
               (ULONG)     size );
         else
            rc = RexxTransAllocateMemory(
               (ULONG)     size );
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxAllocateMemory)
            rc = (*URexxAllocateMemory)(
               (ULONG)     size );
         else
            rc = RexxTransAllocateMemory(
               (ULONG)     size );
         break;
#endif
   }
   if (Trace)
   {
      TraceString( ": Result: %x\n", rc );
   }
   return rc;
}

APIRET APIENTRY RexxFreeMemory(
   PVOID ptr )
{
   APIRET rc=RXFUNC_NOTREG;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s: %x",
         MyLibraryName[ InterpreterIdx ],
         "RexxFreeMemory()",
         ptr );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxFreeMemory)
            rc = (*RRexxFreeMemory)(
               (PVOID)     ptr );
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxFreeMemory)
            rc = (*IRexxFreeMemory)(
               (PVOID)     ptr );
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxFreeMemory)
            rc = (*ORexxFreeMemory)(
               (PVOID)     ptr );
         else
            rc = RexxTransFreeMemory(
               (PVOID)     ptr );
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxFreeMemory)
            rc = (*QRexxFreeMemory)(
               (PVOID)     ptr );
         else
            rc = RexxTransFreeMemory(
               (PVOID)     ptr );
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxFreeMemory)
            rc = (*WRexxFreeMemory)(
               (PVOID)     ptr );
         else
            rc = RexxTransFreeMemory(
               (PVOID)     ptr );
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxFreeMemory)
            rc = (*URexxFreeMemory)(
               (PVOID)     ptr );
         else
            rc = RexxTransFreeMemory(
               (PVOID)     ptr );
         break;
#endif
   }
   if (Trace)
   {
      TraceString( ": Result: %x\n", rc );
   }
   return rc;
}

void   APIENTRY RexxWaitForTermination(
   VOID )
{
   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s:",
         MyLibraryName[ InterpreterIdx ],
         "RexxWaitForTermination()" );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxWaitForTermination)
            (*RRexxWaitForTermination)(
                 );
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxWaitForTermination)
            (*IRexxWaitForTermination)(
                 );
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxWaitForTermination)
            (*ORexxWaitForTermination)(
                 );
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxWaitForTermination)
            (*QRexxWaitForTermination)(
                 );
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxWaitForTermination)
            (*WRexxWaitForTermination)(
                 );
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxWaitForTermination)
            (*URexxWaitForTermination)(
                 );
         break;
#endif
   }
   if (Trace)
   {
      TraceString( ": Result: void\n" );
   }
   return;
}

APIRET APIENTRY RexxDidRexxTerminate(
   VOID )
{
   APIRET rc=1;

   if ( InterpreterIdx == -1 )
      LoadInterpreter();

   if (Trace)
   {
      TraceString( "%s %s:",
         MyLibraryName[ InterpreterIdx ],
         "RexxDidRexxTerminate()" );

   }
   switch( InterpreterIdx )
   {
#if defined(INT_REGINA) || defined(INT_REGINALD)
      case INT_REGINA:
      case INT_REGINALD:
         if (RRexxDidRexxTerminate)
            rc = (*RRexxDidRexxTerminate)(
                 );
         break;
#endif
#if defined(INT_REXX_IMC)
      case INT_REXX_IMC:
         if (IRexxDidRexxTerminate)
            rc = (*IRexxDidRexxTerminate)(
                 );
         break;
#endif
#if defined(INT_OBJECT_REXX)
      case INT_OBJECT_REXX:
         if (ORexxDidRexxTerminate)
            rc = (*ORexxDidRexxTerminate)(
                 );
         break;
#endif
#if defined(INT_PERSONAL_REXX)
      case INT_PERSONAL_REXX:
         if (QRexxDidRexxTerminate)
            rc = (*QRexxDidRexxTerminate)(
                 );
         break;
#endif
#if defined(INT_WINREXX)
      case INT_WINREXX:
         if (WRexxDidRexxTerminate)
            rc = (*WRexxDidRexxTerminate)(
                 );
         break;
#endif
#if defined(INT_UNI_REXX)
      case INT_UNI_REXX:
         if (URexxDidRexxTerminate)
            rc = (*URexxDidRexxTerminate)(
                 );
         break;
#endif
   }
   if (Trace)
   {
      TraceString( ": Result: %x\n", rc );
   }
   return rc;
}
