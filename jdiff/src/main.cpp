/*******************************************************************************
 * Jojo's Diff : diff on binary files
 *
 * Copyright (C) 2002-2011 Joris Heirbaut
 *
 * This file is part of JojoDiff.
 *
 * JojoDiff is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Usage:
 * ------
 * jdiff [options] <original file> <new file> [<output file>]
 *   -v          Verbose (greeting, results and tips).
 *   -vv         Verbose (debug info).
 *   -h          Help (this text).
 *   -l          Listing (ascii output).
 *   -lr         Regions (ascii output).
 *   -b          Try to be better (using more memory).
 *   -f          Try to be faster: no out of buffer compares.
 *   -ff         Try to be faster: no out of buffer compares, nor pre-scanning.
 *   -m size     Size (in kB) for look-ahead buffers (default 128).
 *   -bs size    Block size (in bytes) for reading from files (default 4096).
 *   -s size     Number of samples per file (e.g. 8192).
 *   -min count  Minimum number of solutions to find before choosing one.
 *   -max count  Maximum number of solutions to find before choosing one.
 *
 * Exit codes
 * ----------
 *  0  ok, differences found
 *  1  ok, no differences found
 *  2  error: not enough arguments
 *  3  error: could not open first input file
 *  4  error: could not open second input file
 *  5  error: could not open output file
 *  6  error: seek or other i/o error when reading or writing
 *  7  error: 64 bit numbers not supported
 *  8  error on reading
 *  9  error on writing
 *  10  error: malloc failed
 *  20  error: spurious errors
 *
 * Author                Version Date       Modification
 * --------------------- ------- -------    -----------------------
 * Joris Heirbaut        v0.0    10-06-2002 hashed compare
 * Joris Heirbaut                14-06-2002 full compare
 * Joris Heirbaut                17-06-2002 global positions
 * Joris Heirbaut        v0.1    18-06-2002 first well-working runs!!!
 * Joris Heirbaut                19-06-2002 compare in buffer before read position
 * Joris Heirbaut        v0.1    20-06-2002 optimized esc-sequences & lengths
 * Joris Heirbaut        v0.2    24-06-2002 running okay again
 * Joris Heirbaut        v0.2b   01-07-2002 bugfix on length=252
 * Joris Heirbaut        v0.2c   09-07-2002 bugfix on divide by zero in statistics
 * Joris Heirbaut        v0.3a   09-07-2002 hashtable hint only on samplerate
 * Joris Heirbaut          |     09-07-2002 exit code 1 if files are equal
 * Joris Heirbaut          |     12-07-2002 bugfix using ufFabPos in function call
 * Joris Heirbaut        v0.3a   16-07-2002 backtrack on original file
 * Joris Heirbaut        v0.4a   19-07-2002 prescan sourcefile
 * Joris Heirbaut          |     30-08-2002 bugfix in ufFabRst and ufFabPos
 * Joris Heirbaut          |     03-08-2002 bugfix for backtrack before start-of-file
 * Joris Heirbaut          |     09-09-2002 reimplemented filebuffer
 * Joris Heirbaut        v0.4a   10-09-2002 take best of multiple possibilities
 * Joris Heirbaut        v0.4b   11-09-2002 soft-reading from files
 * Joris Heirbaut          |     18-09-2002 moved ufFabCmp from ufFndAhdChk to ufFndAhdAdd/Bst
 * Joris Heirbaut          |     18-09-2002 ufFabOpt - optimize a found solution
 * Joris Heirbaut          |     10-10-2002 added Fab->izPosEof to correctly handle EOF condition
 * Joris Heirbaut        v0.4b   16-10-2002 replaces ufFabCmpBck and ufFabCmpFwd with ufFabFnd
 * Joris Heirbaut        v0.4c   04-11-2002 use ufHshFnd after prescanning
 * Joris Heirbaut          |     04-11-2002 no reset of matching table
 * Joris Heirbaut          |     21-12-2002 rewrite of matching table logic
 * Joris Heirbaut          |     24-12-2002 no compare in ufFndAhdAdd
 * Joris Heirbaut          |     02-01-2003 restart finding matches at regular intervals when no matches are found
 * Joris Heirbaut          |     09-01-2003 renamed ufFabBkt to ufFabSek, use it for DEL and BKT instructions
 * Joris Heirbaut        v0.4c   23-01-2003 distinguish between EOF en EOB
 * Joris Heirbaut        v0.5    27-02-2003 dropped "fast" hash method (it was slow!)
 * Joris Heirbaut          |     22-05-2003 started    rewrite of FAB-abstraction
 * Joris Heirbaut          |     30-06-2003 terminated rewrite ...
 * Joris Heirbaut          |     08-07-2003 correction in ufMchBst (llTstNxt = *alBstNew + 1 iso -1)
 * Joris Heirbaut        v0.5    02-09-2003 production
 * Joris Heirbaut        v0.6    29-04-2005 large-file support
 * Joris Heirbaut        v0.7    23-06-2009 differentiate between position 0 and notfound in ufMchBst
 * Joris Heirbaut          |     23-06-2009 optimize collission strategy using sample quality
 * Joris Heirbaut          |     24-06-2009 introduce quality of samples
 * Joris Heirbaut          |     26-06-2009 protect first samples
 * Joris Heirbaut        v0.7g   24-09-2009 use fseeko for cygwin largefiles
 * Joris Heirbaut          |     24-09-2009 removed casts to int from ufFndAhd
 * Joris Heirbaut        v0.7h   24-09-2009 faster ufFabGetNxt
 * Joris Heirbaut          |     24-09-2009 faster ufHshAdd: remove quality of hashes
 * Joris Heirbaut        v0.7i   25-09-2009 drop use of ufHshBitCnt
 * Joris Heirbaut        v0.7l   04-10-2009 increment glMchMaxDst as hashtable overloading grows
 * Joris Heirbaut          |     16-10-2009 finalization
 * Joris Heirbaut        v0.7m   17-10-2009 gprof optimization ufHshAdd
 * Joris Heirbaut        v0.7n   17-10-2009 gprof optimization ufFabGet
 * Joris Heirbaut        v0.7o   18-10-2009 gprof optimization asFab->iiRedSze
 * Joris Heirbaut        v0.7p   19-10-2009 ufHshAdd: check uniform distribution
 * Joris Heirbaut        v0.7q   23-10-2009 ufFabGet: scroll back on buffer
 * Joris Heirbaut          |     19-10-2009 ufFndAhd: liMax = giBufSze
 * Joris Heirbaut          |     25-10-2009 ufMchAdd: gliding matches
 * Joris Heirbaut          |     25-10-2009 ufOut: return true for faster EQL sequences in jdiff function
 * Joris Heirbaut        v0.7r   27-10-2009 ufMchBst: test position for gliding matches
 * Joris Heirbaut          |     27-10-2009 ufMchBst: remove double loop
 * Joris Heirbaut          |     27-10-2009 ufMchBst: double linked list ordered on azPosOrg
 * Joris Heirbaut          |     27-10-2009 ufFndAhd: look back on reset (liBck)
 * Joris Heirbaut          |     27-10-2009 ufFndAhd: reduce lookahead after giMchMin (speed optimization)
 * Joris Heirbaut        v0.7x   05-11-2009 ufMchAdd: hashed method
 * Joris Heirbaut        v0.7y   13-11-2009 ufMchBst: store unmatched samples too (reduce use of ufFabFnd)
 *
 *******************************************************************************/

/*
 * Includes
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>

using namespace std ;

#ifdef __MINGW32__
#include "JFileAhead.h"
#else
#include <iostream>
#include <istream>
#include <fstream>
#include "JFileIStream.h"
#include "JFileIStreamAhead.h"
#endif

#include "JDefs.h"
#include "JDiff.h"
#include "JOutBin.h"
#include "JOutAsc.h"
#include "JOutRgn.h"
#include "JFile.h"

using namespace JojoDiff ;

/*******************************************************************************
* Main function
*******************************************************************************/
int main(int aiArgCnt, char *acArg[])
{
  const char *lcFilNamOrg;
  const char *lcFilNamNew;
  const char *lcFilNamOut;

  FILE  *lpFilOut;

  int liOptArgCnt=0 ;
  int lbOptArgDne=false ;
  char lcHlp='\0';

  /* Default settings */
  int liOutTyp = 0 ;            /* 0 = JOutBin, 1 = JOutAsc, 2 = JOutRgn */
  int liVerbse = 0;             /* Verbose level 0=no, 1=normal, 2=high            */
  int lbSrcBkt = true;          /* Backtrace on sourcefile allowed?                */
  bool lbCmpAll = true ;        /* Compare even if data not in buffer?             */
  int liSrcScn = 1 ;            /* Prescan source file: 0=no, 1=do, 2=done         */
  int liMchMax = 32 ;           /* Maximum entries in matching table.              */
  int liMchMin = 8 ;            /* Minimum entries in matching table.              */
  int liHshMbt = 8 ; 	        /* Hashtable size in mega-samples (default 8 * 1024 * 1024) */
  long llBufSze = 256*1024 ;    /* Default file-buffers size */
  int liBlkSze = 4096 ;         /* Default block size */
  int liAhdMax = 0;             /* Lookahead range (0=same as llBufSze) */

  JDebug::stddbg        = stderr ;

  /* Read options */
  while (! lbOptArgDne && (aiArgCnt-1 > liOptArgCnt)) {
    liOptArgCnt++ ;
    if (strcmp(acArg[liOptArgCnt], "-v") == 0) {
      liVerbse = 1;
    } else if (strcmp(acArg[liOptArgCnt], "-vv") == 0) {
      liVerbse = 2;
    } else if (strcmp(acArg[liOptArgCnt], "-vvv") == 0) {
      liVerbse = 3;
    } else if (strcmp(acArg[liOptArgCnt], "-h") == 0) {
      lcHlp = 'h' ;

    } else if (strcmp(acArg[liOptArgCnt], "-a") == 0) {
        liOptArgCnt ++;
        if (aiArgCnt > liOptArgCnt) {
          liAhdMax = atoi(acArg[liOptArgCnt]) / 2 * 1024;
        }
    } else if (strcmp(acArg[liOptArgCnt], "-m") == 0) {
        liOptArgCnt ++;
        if (aiArgCnt > liOptArgCnt) {
          llBufSze = atoi(acArg[liOptArgCnt]) / 2 * 1024;
        }
    } else if (strcmp(acArg[liOptArgCnt], "-bs") == 0) {
        liOptArgCnt ++;
        if (aiArgCnt > liOptArgCnt) {
        	liBlkSze = atoi(acArg[liOptArgCnt]) ;
        }
    } else if (strcmp(acArg[liOptArgCnt], "-s") == 0) {
        liOptArgCnt++;
        if (aiArgCnt > liOptArgCnt) {
        	liHshMbt = atoi(acArg[liOptArgCnt]) ;
        	while (liHshMbt > 1024) liHshMbt /= 1024 ;
        }
    } else if (strcmp(acArg[liOptArgCnt], "-min") == 0) {
        liOptArgCnt++;
        if (aiArgCnt > liOptArgCnt) {
          liMchMin = atoi(acArg[liOptArgCnt]) ;
          if (liMchMin > MCH_MAX)
              liMchMin = MCH_MAX ;
        }
    } else if (strcmp(acArg[liOptArgCnt], "-max") == 0) {
        liOptArgCnt++;
        if (aiArgCnt > liOptArgCnt) {
          liMchMax = atoi(acArg[liOptArgCnt]) ;
          if (liMchMax > MCH_MAX)
              liMchMax = MCH_MAX ;
        }

    } else if (strcmp(acArg[liOptArgCnt], "-l") == 0) {
        liOutTyp = 1 ;
    } else if (strcmp(acArg[liOptArgCnt], "-lr") == 0) {
        liOutTyp = 2 ;
    } else if (strcmp(acArg[liOptArgCnt], "-b") == 0) {
        // Larger hashtables
    	lbCmpAll = true ;
        llBufSze = 4096 * 1024 ;
        lbSrcBkt = true;
        liSrcScn = 1 ;
        liMchMin = 16 ;
        liMchMax = 128 ;
        liHshMbt = 32 ; // 32meg elements
    } else if (strcmp(acArg[liOptArgCnt], "-f") == 0) {
        // No compare out-of-buffer
    	lbCmpAll = false ;
        llBufSze = 64 * 1024 ;
        lbSrcBkt = true ;
        liSrcScn = 1  ;
        liMchMin = 8 ;
        liMchMax = 16 ;
        liHshMbt = 4 ; // 4Meg samples
    } else if (strcmp(acArg[liOptArgCnt], "-ff") == 0) {
        // No compare out-of-buffer and no backtracing
    	lbCmpAll = false ;
        llBufSze = 4096 * 1024 ;
        lbSrcBkt = true ;
        liSrcScn = 0 ;
        liMchMin = 4 ;
        liMchMax = 16 ;
        liHshMbt = 1 ; // 1Meg samples
    } else if (strcmp(acArg[liOptArgCnt], "-do") == 0) {
      JDebug::stddbg = stdout;

    #if debug
    } else if (strcmp(acArg[liOptArgCnt], "-dhsh") == 0) {
      JDebug::gbDbg[DBGHSH] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-dahd") == 0) {
      JDebug::gbDbg[DBGAHD] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-dcmp") == 0) {
      JDebug::gbDbg[DBGCMP] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-dprg") == 0) {
      JDebug::gbDbg[DBGPRG] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-dbuf") == 0) {
      JDebug::gbDbg[DBGBUF] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-dhsk") == 0) {
      JDebug::gbDbg[DBGHSK] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-dahh") == 0) {
      JDebug::gbDbg[DBGAHH] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-dbkt") == 0) {
      JDebug::gbDbg[DBGBKT] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-dred") == 0) {
      JDebug::gbDbg[DBGRED] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-dmch") == 0) {
      JDebug::gbDbg[DBGMCH] = true ;
    } else if (strcmp(acArg[liOptArgCnt], "-ddst") == 0) {
      JDebug::gbDbg[DBGDST] = true ;
    #endif

    } else {
      lbOptArgDne = true ;
      liOptArgCnt-- ;
    }
  }

  /* Output greetings */
  if ((liVerbse>0) || (lcHlp == 'h') || (aiArgCnt - liOptArgCnt < 3)) {
    fprintf(JDebug::stddbg, "JDIFF - Jojo's binary diff version " JDIFF_VERSION "\n") ;
    fprintf(JDebug::stddbg, JDIFF_COPYRIGHT "\n");
    fprintf(JDebug::stddbg, "\n") ;
    fprintf(JDebug::stddbg, "JojoDiff is free software: you can redistribute it and/or modify\n");
    fprintf(JDebug::stddbg, "it under the terms of the GNU General Public License as published by\n");
    fprintf(JDebug::stddbg, "the Free Software Foundation, either version 3 of the License, or\n");
    fprintf(JDebug::stddbg, "(at your option) any later version.\n");
    fprintf(JDebug::stddbg, "\n");
    fprintf(JDebug::stddbg, "This program is distributed in the hope that it will be useful,\n");
    fprintf(JDebug::stddbg, "but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    fprintf(JDebug::stddbg, "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    fprintf(JDebug::stddbg, "GNU General Public License for more details.\n");
    fprintf(JDebug::stddbg, "\n");
    fprintf(JDebug::stddbg, "You should have received a copy of the GNU General Public License\n");
    fprintf(JDebug::stddbg, "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
    fprintf(JDebug::stddbg, "\n");

    off_t maxoff_t_gb = (MAX_OFF_T >> 30) + 1 ;
    const char *maxoff_t_mul = "GB";
    if (maxoff_t_gb > 1024){
    	maxoff_t_gb = maxoff_t_gb >> 10 ;
    	maxoff_t_mul = "TB";
    }
    fprintf(JDebug::stddbg, "File adressing is %d bit (files up to %"PRIzd" %s), samples are %d bytes.\n\n",
        sizeof(off_t) * 8, maxoff_t_gb, maxoff_t_mul, SMPSZE) ;
  }

  if ((aiArgCnt - liOptArgCnt < 3) || (lcHlp == 'h') || (liVerbse>2)) {
    fprintf(JDebug::stddbg, "Usage: jdiff [options] <original file> <new file> [<output file>]\n") ;
    fprintf(JDebug::stddbg, "  -v          Verbose (greeting, results and tips).\n");
    fprintf(JDebug::stddbg, "  -vv         Verbose (debug info).\n");
    fprintf(JDebug::stddbg, "  -h          Help (this text).\n");
    fprintf(JDebug::stddbg, "  -l          Listing (ascii output).\n");
    fprintf(JDebug::stddbg, "  -lr         Regions (ascii output).\n");
    fprintf(JDebug::stddbg, "  -do         Write verbose and debug info to stdout instead of stddbg.\n");
    fprintf(JDebug::stddbg, "  -b          Try to be better (using more memory).\n");
    fprintf(JDebug::stddbg, "  -f          Try to be faster: no out of buffer compares.\n");
    fprintf(JDebug::stddbg, "  -ff         Try to be faster: no out of buffer compares, nor pre-scanning.\n");
    fprintf(JDebug::stddbg, "  -m size     Size (in kB) for look-ahead buffer (default 512kB");
#ifndef __MINGW32__
    fprintf(JDebug::stddbg, ", 0=no buffers");
#endif
    fprintf(JDebug::stddbg, ").\n");
    fprintf(JDebug::stddbg, "  -bs size    Block size (in bytes) for reading from files (default 4096).\n");
    fprintf(JDebug::stddbg, "  -s size     Number of samples per file in MB (default 8).\n");
    fprintf(JDebug::stddbg, "  -a size     Number of kB to look ahead (default=same as buffer-size).\n");
    fprintf(JDebug::stddbg, "  -min count  Minimum number of solutions to find (default %d, max %d).\n", liMchMin, MCH_MAX);
    fprintf(JDebug::stddbg, "  -max count  Maximum number of solutions to find (default %d, max %d).\n", liMchMax, MCH_MAX);
    fprintf(JDebug::stddbg, "Principles:\n");
    fprintf(JDebug::stddbg, "  JDIFF tries to find equal regions between two binary files using a heuristic\n");
    fprintf(JDebug::stddbg, "  hash algorithm and outputs the differences between both files.\n");
    fprintf(JDebug::stddbg, "  Heuristics are generally used for improving performance and memory usage,\n");
    fprintf(JDebug::stddbg, "  at the cost of accuracy. Therefore, this program may not find a minimal set\n");
    fprintf(JDebug::stddbg, "  of differences between files.\n");
    fprintf(JDebug::stddbg, "Notes:\n");
    fprintf(JDebug::stddbg, "  Options -b, -f or -ff should be used before other options.\n");
    fprintf(JDebug::stddbg, "  Accuracy may be improved by increasing the number of samples.\n");
    fprintf(JDebug::stddbg, "  Sample size is always lowered to the largest n-bit prime (n < 32)\n");
    fprintf(JDebug::stddbg, "  Original and new file must be random access files.\n");
    fprintf(JDebug::stddbg, "  Output is sent to standard output if output file is missing.\n");
    fprintf(JDebug::stddbg, "Hint:\n");
    fprintf(JDebug::stddbg, "  Do not use jdiff directly on compressed files, such as zip, gzip, rar, ...\n");
    fprintf(JDebug::stddbg, "  Instead use uncompressed files, such as tar, cpio or zip-0, and then compress\n");
    fprintf(JDebug::stddbg, "  the jdiff's output file afterwards.\n");
    fprintf(JDebug::stddbg, "\n");
                    /******************************************************************************/
    if ((aiArgCnt - liOptArgCnt < 3) || (lcHlp == 'h'))
        exit(EXI_ARG);
  }

  /* Read filenames */
  lcFilNamOrg = acArg[1 + liOptArgCnt];
  lcFilNamNew = acArg[2 + liOptArgCnt];
  if (aiArgCnt - liOptArgCnt >= 4)
    lcFilNamOut = acArg[3 + liOptArgCnt];
  else
    lcFilNamOut = "-" ;

  /* MinGW does not correctly handle files > 2GB using fstream.gseek */
#ifdef __MINGW32__
  if (llBufSze == 0){
      llBufSze = liBlkSze ;
  }
#endif

  JFile *lpFilOrg = NULL ;
  JFile *lpFilNew = NULL ;

  FILE *lfFilOrg = NULL ;
  FILE *lfFilNew = NULL ;

#ifndef __MINGW32__
  ifstream *liFilOrg = NULL ;
  ifstream *liFilNew = NULL ;
#endif

  /* Open first file */
#ifdef __MINGW32__
  lfFilOrg = jfopen(lcFilNamOrg, "rb") ;
  if (lfFilOrg != NULL){
      lpFilOrg = new JFileAhead(lfFilOrg, "Org", llBufSze, liBlkSze);
  }
#else
  liFilOrg = new ifstream();
  liFilOrg->open(lcFilNamOrg, ios_base::in | ios_base::binary) ;
  if (liFilOrg->is_open()){
	  if (llBufSze > 0){
		  lpFilOrg = new JFileIStreamAhead(liFilOrg, "Org",  llBufSze, liBlkSze);
	  } else {
		  lpFilOrg = new JFileIStream(liFilOrg, "Org");
	  }
  }
#endif
  if (lpFilOrg == NULL){
      fprintf(JDebug::stddbg, "Could not open first file %s for reading.\n", lcFilNamOrg);
      exit(EXI_FRT);
  }

  /* Open second file */
#ifdef __MINGW32__
  lfFilNew = jfopen(lcFilNamNew, "rb") ;
  if (lfFilNew != NULL){
      lpFilNew = new JFileAhead(lfFilNew, "New", llBufSze, liBlkSze);
  }
#else
  liFilNew = new ifstream();
  liFilNew->open(lcFilNamNew, ios_base::in | ios_base::binary) ;
  if (liFilNew->is_open()){
	  if (llBufSze > 0){
		  lpFilNew = new JFileIStreamAhead(liFilNew, "New",  llBufSze, liBlkSze);
	  } else {
		  lpFilNew = new JFileIStream(liFilNew, "New");
	  }
  }
#endif
  if (lpFilNew == NULL){
      fprintf(JDebug::stddbg, "Could not open second file %s for reading.\n", lcFilNamNew);
      exit(EXI_SCD);
  }

  /* Open output */
  if (strcmp(lcFilNamOut,"-") == 0 )
      lpFilOut = stdout ;
  else
      lpFilOut = fopen(lcFilNamOut, "wb") ;
  if ( lpFilOut == null ) {
    fprintf(JDebug::stddbg, "Could not open output file %s for writing.\n", lcFilNamOut) ;
    exit(EXI_OUT);
  }

  /* Init output */
  JOut *lpOut ;
  switch (liOutTyp){
  case 0:
      lpOut = new JOutBin(lpFilOut);
      break;
  case 1:
      lpOut = new JOutAsc(lpFilOut);
      break;
  case 2:
  default:  // XXX get rid of uninitialized warning
      lpOut = new JOutRgn(lpFilOut);
      break;
  }

  /* Go ... */
  JDiff loJDiff(lpFilOrg, lpFilNew, lpOut,
      liHshMbt * 1024 * 1024, liVerbse,
      lbSrcBkt, liSrcScn, liMchMax, liMchMin, liAhdMax==0?llBufSze:liAhdMax, lbCmpAll);
  if (liVerbse>1) {
      fprintf(JDebug::stddbg, "Lookahead buffers: %lu kb. (%lu kb. per file).\n",llBufSze * 2 / 1024, llBufSze / 1024) ;
      fprintf(JDebug::stddbg, "Hastable size    : %d kb. (%d samples).\n", (loJDiff.getHsh()->get_hashsize() + 512) / 1024, loJDiff.getHsh()->get_hashprime()) ;
  }

  int liRet = loJDiff.jdiff();

  /* Write statistics */
  if (liVerbse > 1) {
      fprintf(JDebug::stddbg, "Hashtable size          = %d samples, %d KB, %d MB\n",
              loJDiff.getHsh()->get_hashsize(),
              (loJDiff.getHsh()->get_hashsize() + 512) / 1024,
              ((loJDiff.getHsh()->get_hashsize() + 512) / 1024 + 512) / 1024) ;
      fprintf(JDebug::stddbg, "Hashtable prime         = %d\n",   loJDiff.getHsh()->get_hashprime()) ;
      fprintf(JDebug::stddbg, "Hashtable hits          = %d\n",   loJDiff.getHsh()->get_hashhits()) ;
      fprintf(JDebug::stddbg, "Hashtable errors        = %d\n",   loJDiff.getHshErr()) ;
      fprintf(JDebug::stddbg, "Hashtable repairs       = %d\n",   JMatchTable::siHshRpr) ;
      fprintf(JDebug::stddbg, "Hashtable overloading   = %d\n",   loJDiff.getHsh()->get_hashcolmax() / 3 - 1);
      fprintf(JDebug::stddbg, "Reliability distance    = %d\n",   loJDiff.getHsh()->get_reliability());
      fprintf(JDebug::stddbg, "Random    accesses      = %ld\n",  lpFilOrg->seekcount() + lpFilNew->seekcount());
      fprintf(JDebug::stddbg, "Delete    bytes         = %"PRIzd"\n", lpOut->gzOutBytDel);
      fprintf(JDebug::stddbg, "Backtrack bytes         = %"PRIzd"\n", lpOut->gzOutBytBkt);
      fprintf(JDebug::stddbg, "Escape    bytes written = %"PRIzd"\n", lpOut->gzOutBytEsc);
      fprintf(JDebug::stddbg, "Control   bytes written = %"PRIzd"\n", lpOut->gzOutBytCtl);
  }
  if (liVerbse > 0) {
      fprintf(JDebug::stddbg, "Equal     bytes         = %"PRIzd"\n", lpOut->gzOutBytEql);
      fprintf(JDebug::stddbg, "Data      bytes written = %"PRIzd"\n", lpOut->gzOutBytDta);
      fprintf(JDebug::stddbg, "Overhead  bytes written = %"PRIzd"\n", lpOut->gzOutBytCtl + lpOut->gzOutBytEsc);
  }

  /* Cleanup */
  delete lpFilOrg;
  delete lpFilNew;
#ifndef __MINGW32__
  if (liFilOrg != NULL) {
	  liFilOrg->close();
	  delete liFilOrg ;
  }
  if (liFilNew != NULL) {
	  liFilNew->close();
	  delete liFilNew ;
  }
#endif
  if (lfFilOrg != NULL) jfclose(lfFilOrg);
  if (lfFilNew != NULL) jfclose(lfFilNew);


  /* Exit */
  switch (liRet){
  case - EXI_SEK:
      fprintf(JDebug::stddbg, "Seek error !");
      exit (EXI_SEK);
  case - EXI_LRG:
      fprintf(JDebug::stddbg, "64-bit offsets not supported !");
      exit (EXI_LRG);
  case - EXI_RED:
      fprintf(JDebug::stddbg, "Error reading file !");
      exit (EXI_RED);
  case - EXI_WRI:
      fprintf(JDebug::stddbg, "Error writing file !");
      exit (EXI_WRI);
  case - EXI_MEM:
      fprintf(JDebug::stddbg, "Error allocating memory !");
      exit (EXI_MEM);
  case - EXI_ERR:
      fprintf(JDebug::stddbg, "Spurious error occured !");
      exit (EXI_ERR);
  }

  if (lpOut->gzOutBytDta == 0 && lpOut->gzOutBytDel == 0)
      return(1);    /* no differences found */
  else
      return(0);    /* differences found    */
}
