/*
 * JDiff.cpp
 *
 * Jojo's diff on binary files: main class.
 *
 * Copyright (C) 2002-2011 Joris Heirbaut
 *
 * This file is part of JojoDiff.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * ----------------------------------------------------------------------------
 *
 * Class JDiff takes two JFiles as input and outputs to a JOut instance.
 *
 * If you want to reuse JojoDiff, you need following files:
 * - JDiff.h/cpp        The main JojoDiff class
 * - JHashPos.h/cpp     The hash table collection of (sample-key, position)
 * - JMatchTable.h/cpp  The matching table logic
 * - JDefs.h            Global definitions
 * - JDebug.h/cpp       Debugging definitions
 * - JOut.h             Abstract output class
 * - JFile.h            Abstract input class
 * + at least one descendant of JOut and JFile (may be of your own).
 *
 * Method jdiff performs the actual diffing:
 * 1) first, we create a hashtable collection of (hkey, position) pairs, where
 *      hkey = hash key of a 32 byte sample in the original input file
 *      position = position of this sample in the file
 * 2) compares both files byte by byte
 * 3) when a difference is found, looks ahead using ufFndAhd to find the nearest
 *    equal region between both files
 * 4) output skip/delete/backtrace instructions to reach the found regions
 * 5) repeat steps 2-4 each time the end of an equal region is reached.
 *
 * Method ufFndAhd looks ahead on the input files to find the nearest equals regions:
 * - for every 32-byte sample in the new file, look in the hastable whether a
 *   similar sample exists in the original file.
 * - creates a table of matching regions using this catalog
 * - returns the nearest match from the table of matches
 *
 * There are two reasons for creating a matching table on top of the hashtable:
 * - The hashtable may only contain a (small) percentage of all samples, hence
 *   the nearest equal region is not always detected first.
 * - Samples are considered equal when their hash-keys are equal (in fact there's only
 *   1/256 chance that samples are really equal of their hash-keys match).
 *
 * Method ufFndAhdGet gets one byte of a file and counts the number of subsequent
 * equal bytes (because samples containing too many equal bytes are usually bad to
 * compare with between files).
 *
 * Method ufFndhdScn scans the left file and creates the hash table.
 *
 * TODO: allow org and new files to be the same file
 * TODO: allow sequential files as input
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
 * Joris Heirbaut        v0.8    30-06-2011 C++ version
 * Joris Heirbaut        v0.8a   08-07-2011 Optimize position 0
 * Joris Heirbaut        v0.8b   02-09-2011 Switch order of ahead/backtrack/skip logic
 * Joris Heirbaut        v0.8.1  30-11-2011 Revert to use of fread/fseek (MinGW ifstream.gseek does not correctly handle files >2GB)
 * Joris Heirbaut        v0.8.1  30-11-2011 Throw out exception handling for MinGW (trying to reduce exe size)
 *
 *******************************************************************************/

#ifndef JDIFF_H_
#define JDIFF_H_
#include "JDefs.h"
#include "JFile.h"
#include "JHashPos.h"
#include "JMatchTable.h"
#include "JOut.h"

namespace JojoDiff {

/**
 * Central JDiff and FindAhead routines for the JDiff algorithm.
 */
class JDiff {
public:
    /**
     * Create JDiff for working on specified files.
     * @param apFilOrg  Original file.
     * @param apFilNew  New file.
     * @param aiHshSze  Hastable max size in number of elements (default = 8388608)
     * @param aiVerbse  Verbose level 0=no, 1=normal, 2=high (default = 0)
     * @param abSrcBkt  Backtrace on sourcefile allowed? (default = yes)
     * @param aiSrcScn  Prescan source file: 0=no, 1=yes (default = yes)
     * @param aiMchMax  Maximum entries in matching table (default = 8)
     * @param aiMchMin  Minimum entries in matching table (default = 4)
     * @param aiAhdMax  Maximum bytes to find ahead (default = 256kB)
     */
    JDiff(JFile * const apFilOrg, JFile * const apFilNew, JOut * const apOut,
        const int aiHshSze = 8388608, const
        int aiVerbse=0,
        const int abSrcBkt=true,
        const int aiSrcScn=true,
        const int aiMchMax=8,
        const int aiMchMin=4,
        const int aiAhdMax=256*1024,
        const bool abCmpAll = true);

	/**
	 * Destroys JDiff object.
	 */
	virtual ~JDiff();

	/*******************************************************************************
	* Difference function
	*
	* Writes out the differences between the two files passed via the constructor.
	*
	* Throws a bad_alloc exception in case of memory error.
	* Throws an io_base::failure in case of i/o error.
	*
	* @return 0             ok
	* @return EXI_SEK       Error seeking file
    * @return EXI_LRG  7    Error on 64-bit number
    * @return EXI_RED  8    Error reading file
    * @return EXI_WRI  9    Error writing file
    * @return EXI_MEM  10   Error allocating memory
    * @return EXI_ERR  20   Spurious error occured
	*******************************************************************************/
	int jdiff ();

	/* getters */
	JHashPos * getHsh(){return gpHsh;};
	int getHshErr(){return giHshErr;};

private:
	/* Context */
	JFile * const mpFilOrg ;    // Original file to read
	JFile * const mpFilNew ;    // New file to read
	JOut  * const mpOut ;       // Output handler
	JHashPos * gpHsh ;          // Hashtable containing hashes from mpFilOrg.
	JMatchTable * gpMch ;       // Table of matches

	/* Settings */
	const int miVerbse;     /* Vebosity level */
	const int mbSrcBkt;     /* Allow bactrace on original file? */
	const int miMchMax;     /* Max number of matches to find */
	const int miMchMin;     /* Min number oif matches to find */
	const int miAhdMax ;    /* Max number of bytes to look ahead */
    const bool mbCmpAll ;   /* Compare all matches, even if data not in buffer? */
    int  miSrcScn;          /* Prescan original file: 0=no, 1=yes, 2=done */

    /* State */
	off_t mzAhdOrg;        // Current ahead position on original file
	off_t mzAhdNew;        // Current ahead position on new file
	hkey mlHshOrg;         // Current hash value for original file
	hkey mlHshNew;         // Current hash value for new file
	int miValOrg;          // Current file value
	int miValNew;          // Current file value
	int miEqlOrg;          // Indicator for equal bytes in current sample
	int miEqlNew;          // Indicator for equal bytes in current sample

	/**
	 * Flush pending output
	 */
	void ufPutEql(const off_t &lzPosOrg, const off_t &lzPosNew, off_t &lzEql, bool &lbEql) ;

	/**
	 * Finds the nearest equal regions between the two files
	 *
	 * @param azRedOrg  in:  read position in original file to start looking from
	 * @param azRedNew  in:  read position in new file to start looking from
	 * @param azSkpOrg  out: number of bytes to skip (delete) in original file to reach equal region
	 * @param azSkpNew  out: number of bytes to skip (insert) in new file to reach equal region
	 * @param azAhd     out: number of bytes to go ahead on both files to reach equal regions
	 */
	int ufFndAhd (
	  off_t const &azRedOrg,        /* read position in original file                     */
	  off_t const &azRedNew,        /* read position in new file                          */
	  off_t &azSkpOrg,              /* number of bytes to skip (delete) in original file  */
	  off_t &azSkpNew,              /* number of bytes to skip (insert) in new file       */
	  off_t &azAhd                  /* number of bytes to go before similarity is reached */
	);

    /** Scans the original file and fills up the hashtable. */
    int ufFndAhdScn () ;

    /** Hashes the next byte from specified file. */
    void ufFndAhdGet(JFile *apFil, const off_t &azPos, int &aiVal, int &aiEql, int aiSft) ;

public:
    /*
     * Statistics about operations
     */
    int giHshErr ;         /* Number of false hash hits                         */
}; // class JDiff

} // namespace JojoDiff
#endif /* JDIFF_H_ */
