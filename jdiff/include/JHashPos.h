/*
 * JHashPos.cpp
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
 *******************************************************************************
 * Hash table functions:
 *  add      Insert value into hashtable
 *  get      Lookup value into hashtable
 *  hash     Incremental hash function on array of bytes
 *
 * The hash table stores positions within files. The key reflects the contents
 * of the file at that position. This way, we can efficiently find regions
 * that are equal between both files.
 *
 * Hash function on array of bytes:
 *
 * Principle:
 * ----------
 * Input:  a[32]  32 8-bit values (characters)
 *         p      prime number
 * Output: h = (a[31] x 2^31 + a[30] x 2^30 + .. + a[0]) % 2^32 % p
 *
 *
 * Largest n-bit primes: 251, 509, 1021, 2039, 4093, 8191, 16381, 32749, 65521,
 *                       131071 (17 bit), ..., 4294967291 (32 bit)
 *
 * Table entries contain a 32-bit hash value and a file position.
 *
 * The collision strategy tries to create a uniform distributed set of samples
 * over the investigated region, which is either the whole file or the
 * look-ahead region.
 * This is achieved by overwriting the original entry if either
 * - the original entry lies before the investigatd region (the base position), or
 * - a number of subsequent non-overwriting collisions occur where
 *   x = (region size / hashtable size) + 2
 *   --> after x collisions, the collision wins where x is decreasing
 *       as the region mapped by the hashtable grows.
 * as of 23-06-2009 (v0.7)
 * - samples having a high number of equal characters are less meaningless
 *   than samples without equal characters, therefore higher quality samples
 *   win in the collision strategy.
 * - quality is a number between 0 and 3:
 * 		3 = 31 to 24 doubles (equal characters) (lowest quality)
 * 		2 = 23 to 16 doubles (equal characters)
 * 		1 = 15 to 08 doubles (equal characters)
 * 		0 =  7 to  0 doubles (equal characters) (highest quality)
 *
 * Only samples from the original file are stored.
 * Samples from the new file are looked up.
 *
 * The investigated region is either
 * - the whole file when the prescan/backtrace option is used (default)
 * - the look-ahead region otherwise (options -f or -ff)
 *
 * With prescan/backtrace enabled, the algorithm behaves like a kind of
 * copy/insert algorithm (simulated with insert/delete/modify and backtrace
 * instructions). Without prescan/backtrace, the algorithm behaves like an
 * insert/delete algorithm.
 *******************************************************************************/

#ifndef JHASHPOS_H_
#define JHASHPOS_H_

#include "JDefs.h"
#include "JDebug.h"

namespace JojoDiff {

/*
 * Hashtable of file positions for JDiff.
 */
class JHashPos {
public:
    /**
     * Create a new hash-table with size not larger that the given size.
     *
     * Actual size will be based on the highest prime below the highest power of 2
     * lower or equal to the specified size, e.g. aiSze=8192 will create a hashtable
     * of 8191 elements.
     *
     * @param aiSze   size, in number of elements.
     */
	JHashPos(int aiSze);

	virtual ~JHashPos();

	/* The hash function:
	 * Generate a new hash value by adding a new byte.
	 * Old bytes are shifted out from the hash value in such a way that
	 * the new value corresponds to a sample of 32 bytes (the lowest bit of the 32'th
	 * byte still influences the highest bit of the hash value).
	 */
	void hash ( int const acNew, hkey &akCurHsh ) const {
	    //alCurHsh =  (alCurHsh << 1) ^ aiCurVal  ;
	    akCurHsh =  (akCurHsh * 2) + acNew  ;   // faster
	    #if debug
	    if (JDebug::gbDbg[DBGHSK])
	        fprintf(JDebug::stddbg, "Hash Key %"PRIhkey" %x %c\n", akCurHsh, acNew,
	                (acNew>=32 && acNew <= 127)?acNew:' ');
	    #endif
	}

	/* Return the reliability range: reliability decreases as the hashtable load
	 * increases. This function returns an estimation of the number of bytes to verify
	 * before deciding that regions do not match.
	 */
	inline int get_reliability() const {
		return miHshRlb ;
	}

	/* Hastable insert */
	void add (hkey akCurHsh, off_t azPos, int aiEqlCnt ) ;

	/* Hashtable lookup */
	bool get (const hkey akCurHsh, off_t &azPos) ;

	/* Hashtable printout */
	void print() ;

	/* Printout hashtable distribution */
	void dist(off_t azMax, int aiBck);

	/* Return the index to use to create a hashtable of at most the given size. */
	static int get_size_index(int sze);

	/* return hashtable primme number */
	int get_hashprime(){return miHshPme;}

	/* return hashtable size in bytes */
	int get_hashsize(){return miHshSze;}

	/* return hastable collision override threshold */
	int get_hashcolmax(){return miHshColMax;}

	/* return number of hits found by this hashtable */
	int get_hashhits(){return miHshHit;}

private:
	/* The hash table. Using a struct causes certain compilers (gcc) to align        */
	/* fields on 64-bit boundaries, causing 25% memory loss. Therefore, I use        */
	/* two arrays instead of an array of structs.                                    */
	off_t *mzHshTblPos ;    /* Hash values: positions within the original file       */
	hkey  *mkHshTblHsh ;    /* Hash keys                                             */

	/* Size */
	int miHshPme  ;         /* prime number for size and hashing              				*/
	int miHshSze ;          /* Actual size in bytes of the hashtable          				*/

    /* State */
	int miHshColMax;        /* max number of collisions before override       				*/
	int miHshColCnt;        /* current number of subsequent collisions.               		*/
	int miHshRlb ;          /* hashtable reliability: decreases as the overloading grows 	*/
    int miLodCnt ;          /* hashtable load-counter                                       */

    /* Statistics */
    int miHshHit;           /* number of hits found by this hashtable                       */
};
}
#endif /* JHASHPOS_H_ */
