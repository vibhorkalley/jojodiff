/*
 * JDebug.h
 *
 * Debugging routines and definitions, typically empty when compiling without the -Ddebug option.
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
 */

#ifndef JDEBUG_H_
#define JDEBUG_H_

#include <stdio.h>

#include "JDefs.h"

/**
 * Debug constants
 */
#if debug
#define AreWeHere fprintf(stderr, "test\n") ; fflush(stderr) ;

#define DBGHSH 0  // Debug Hash                     -dhsh
#define DBGAHD 1  // Debug Ahead                    -dahd
#define DBGCMP 2  // Debug Compare                  -dcmp
#define DBGPRG 3  // Debug Progress                 -dprg
#define DBGBUF 4  // Debug Ahead Buffer             -dbuf
#define DBGAHH 5  // Debug Ahead Hash               -dahh
#define DBGHSK 6  // Debug ufHshNxt                 -dahh
#define DBGBKT 7  // Debug ufFabSek                 -dbkt
#define DBGRED 8  // Debug ufFabGet                 -dred
#define DBGMCH 9  // Debug ufMch...                 -dmch
#define DBGDST 10 // Debug Hashtable distribution   -ddst
#else
#define AreWeHere ;
#endif

/*
 * Inlined debugging routines
 */
class JDebug {
public:
	static FILE *stddbg;   /* Debug output to stderr or stdout                */

#if debug
	static int gbDbg[16];
#endif
};

#endif /* JDEBUG_H_ */
