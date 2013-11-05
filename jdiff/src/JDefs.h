/*
 * JDefs.h
 *
 * JojoDiff global definitions
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
 * Author                Version Date       Modification
 * --------------------- ------- -------    -----------------------
 * Joris Heirbaut        v0.8.2  06-12-2011 Use jfopen/jfclose/jfseek/jfread to avoid interference with LARGEFILE redefinitions
 */

#ifndef _JDEFS_H
#define _JDEFS_H

#include <stdio.h>

#ifdef _DEBUG
#define debug           1       /* Include debug code? */
#else
#define debug           0       /* Include debug code? */
#endif

/*
 * Default definitions (for GCC/Linux)
 */
#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64
#define JDIFF_LARGEFILE
#define off_t off_t
#define PRIzd "lld"

#define jfopen   fopen
#define jfclose  fclose
#define jfseek   fseeko
#define jftell   ftello

#else
#ifdef _LARGEFILE64_SOURCE
#define JDIFF_LARGEFILE
#define off_t off64_t
#ifdef __MINGW32__
#define PRIzd "I64d"
#else
#define PRIzd "lld"
#endif

#define jfopen   fopen64
#define jfclose  fclose
#define jfseek   fseeko64
#define jftell   ftello64

#else

#define off_t off_t
#define PRIzd "ld"

#define jfopen   fopen
#define jfclose  fclose
#define jfseek   fseeko
#define jftell   ftello

#endif
#endif

#ifdef JDIFF_LARGEFILE
#if debug
#define P8zd    "%10" PRIzd
#else
#define P8zd    "%12" PRIzd
#endif
#else
#define P8zd    "%8" PRIzd
#endif

/*
 * Global definitions
 */
#define JDIFF_VERSION   "0.8.1 (beta) December 2011"
#define JDIFF_COPYRIGHT "Copyright (C) 2002-2005,2009,2011 Joris Heirbaut"

#define uchar unsigned char
#define ulong unsigned long int         // unsigned long
#define null  NULL

#ifdef _LARGESAMPLE
typedef unsigned long long int  hkey ;  // 64-bit hash keys
#define PRIhkey "llx"                   // format to print a hkey
#else
typedef unsigned long int hkey ;        // 32-bit hash keys
#define PRIhkey "lx"                    // format to print a hkey
#endif /* _LARGESAMPLE */
const int SMPSZE = (int) sizeof(hkey) * 8 ;                                         // Number of bytes in a sample
const off_t MAX_OFF_T = (((off_t)-1) ^ (((off_t) 1) << (sizeof(off_t) * 8 - 1))) ;  // Largest positive offset

#define EOB (EOF - 1)                   // End-Of-Buffer constant
#define EXI_DIF  0                      // OK Exit code, differences found
#define EXI_EQL  1                      // OK Exit code, no differences found
#define EXI_ARG  2                      // Error: not enough arguments
#define EXI_FRT  3                      // Error opening first file
#define EXI_SCD  4                      // Error opening second file
#define EXI_OUT  5                      // Error opening output file
#define EXI_SEK  6                      // Error seeking file
#define EXI_LRG  7                      // Error on 64-bit number
#define EXI_RED  8                      // Error reading file
#define EXI_WRI  9                      // Error writing file
#define EXI_MEM  10                     // Error allocating memory
#define EXI_ERR  20                     // Spurious error occured

/**
 * Output routine constants
 */
#define ESC     0xA7    /* Escape       */
#define MOD     0xA6    /* Modify       */
#define INS     0xA5    /* Insert       */
#define DEL     0xA4    /* Delete       */
#define EQL     0xA3    /* Equal        */
#define BKT     0xA2    /* Backtrace    */

#endif /* _JDEFS_H */
