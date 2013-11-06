/*
 * JMatchTable.cpp
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

#include "JMatchTable.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <new>
using namespace std;

#include "JDebug.h"

namespace JojoDiff {

/*******************************************************************************
* Matching table functions
*
* The matching table contains a series of possibly matching regions between
* the two files.
*
* Because of the statistical nature of the hash table, we're not sure that the
* best solution will be found first. Consider, for example, that the samplerate
* is at 10% on files of about 10Mb and equal regions exist at positions 1000-2000
* and 1500-1500. Because 10% of 10Mb means one sample every 1024 bytes,
* it can happen that the 1000-2000 region is only discovered at positions 2000-3000
* (1000 bytes later). If the 1500-1500 region gets found first at say 1700-1700,
* and if we would not optimize the found solutions, then 500 equal bytes would
* get lost.
*
* Therefore we first memorize a number of matching postions found with the hashtable,
* optimize them (look 1024 bytes back) and then select the first matching solution.
*
* ufMchIni      Initializes the matching table
* ufMchAdd      Adds a potential solution to the mach table
* ufMchFre      Checks if there is room for new solutions
* ufMchBst      Optimizes the matches and returns the best one
*
*******************************************************************************/

int JMatchTable::siHshRpr = 0;     /* Number of repaired hash hits (by comparing) */

/* Construct a matching table for specified hashtable, original and new files. */
JMatchTable::JMatchTable (JHashPos const * const cpHsh,  JFile  * const apFilOrg, JFile  * const apFilNew, const bool abCmpAll)
: mpHsh(cpHsh), mpFilOrg(apFilOrg), mpFilNew(apFilNew), mbCmpAll(abCmpAll)
{
    // allocate table
	// TODO only allocate max number of elements
    msMch = (rMch *) malloc(sizeof(rMch) * MCH_MAX) ;
#ifndef __MINGW32__
    if ( msMch == null ) {
        throw bad_alloc() ;
    }
#endif

	// initialize linked list of free nodes
    for (int liIdx=0; liIdx < MCH_MAX - 1; liIdx++) {
        msMch[liIdx].ipNxt = &msMch[liIdx + 1];
    }
    msMch[MCH_MAX - 1].ipNxt = null ;
    mpMchFre = msMch ;

    memset(mpMch, 0, MCH_PME * sizeof(tMch *));

    // initialize other values
    mpMchGld = null;
    mzGldDlt = 0 ;
}

/* Destructor */
JMatchTable::~JMatchTable() {
	free(msMch);
}

/* -----------------------------------------------------------------------------
 * Auxiliary function: ufMchAdd
 * Add given match to the array of matches:
 * - add to gliding or colliding match if possible, or
 * - add at the end of the list, or
 * - override an old match otherwise
 * Returns
 *   0   if a new entry has been added and table is full
 *   1   if a new entry has been added
 *   2   if an existing entry has been enlarged
 * ---------------------------------------------------------------------------*/
int JMatchTable::add (
  off_t const &azFndOrgAdd,      /* match to add               */
  off_t const &azFndNewAdd,
  off_t const &azBseNew,
  int   const aiEqlNew
){
    off_t lzDlt ;            /* delta key of match */
    rMch *lpCur ;            /* current item */
    int liIdx ;              /* lzDlt % MCH_MAX */

    lzDlt = azFndOrgAdd - azFndNewAdd ;

    // add to gliding match
    if (mpMchGld != null){
        if (lzDlt == mzGldDlt) {
            mpMchGld->iiTyp = -1 ;
            mpMchGld->iiCnt ++ ;
            mpMchGld->izNew = azFndNewAdd ;
            mzGldDlt--;
            return 2 ;
        } else {
            mpMchGld = null ;
        }
    }

    // add or override colliding match
    liIdx = lzDlt % MCH_PME ;
    if (liIdx < 0)
        liIdx = - liIdx ;
    for (lpCur = mpMch[liIdx] ; lpCur != null; lpCur = lpCur->ipNxt){
        if (lpCur->izDlt == lzDlt){
            // add to colliding match
            lpCur->iiCnt ++ ;
            lpCur->iiTyp = 1 ;
            lpCur->izNew = azFndNewAdd ;
            lpCur->izOrg = azFndOrgAdd ;

            return 2 ;
        }
    } /* for */

    // create new match
    if (mpMchFre != null){
        // remove from free-list
        lpCur = mpMchFre ;
        mpMchFre = mpMchFre->ipNxt ;

        // fill out the form
        lpCur->izOrg = azFndOrgAdd ;
        lpCur->izNew = azFndNewAdd ;
        lpCur->izBeg = azFndNewAdd ;
        lpCur->izDlt = lzDlt ;
        lpCur->iiCnt = 1 ;
        lpCur->iiTyp = 0 ;

        // add to hashtable
        lpCur->ipNxt = mpMch[liIdx];
        mpMch[liIdx] = lpCur ;

        // potential gliding match
        mpMchGld = lpCur ;
        mzGldDlt = lzDlt - 1 ;

        #if debug
        if (JDebug::gbDbg[DBGMCH])
          fprintf(JDebug::stddbg, "Mch Add ("P8zd","P8zd") New ("P8zd","P8zd") Bse (%"PRIzd")\n",
                  azFndOrgAdd, azFndNewAdd, lpCur->izOrg, lpCur->izNew, azBseNew) ;
        #endif

        return (mpMchFre != null) ; // still place ?
    } else {
        #if debug
        if (JDebug::gbDbg[DBGMCH]) fprintf(JDebug::stddbg, "Mch ("P8zd", "P8zd") Ful\n", azFndOrgAdd, azFndNewAdd) ;
        #endif

        return 0 ; // not added
    }
} /* add() */

/* -----------------------------------------------------------------------------
 * Get the best match from the array of matches
 * ---------------------------------------------------------------------------*/
const int FZY=0 ;	// Fuzzy factor: for differences smaller than this number of bytes, take the longest looking sequence
bool JMatchTable::get (
  off_t const &azRedOrg,       // current read position on original file
  off_t const &azRedNew,       // current read position on new file
  off_t &azBstOrg,             // best position found on original file
  off_t &azBstNew              // best position found on new file
) const{
    int liIdx ;             // index in mpMch
    int liDst ;             // distance: number of bytes to compare before failing

    rMch *lpCur ;           /* current match under investigation            */
    int liCurCnt ;          /* current match count                          */
    int liCurCmp ;          /* current match compare state: 0=equal, 1=unsure(EOB), 2=unequal, 7=most probably unequal */

    rMch *lpBst=null ;      /* best match.                                  */
    int liBstCnt=0 ;        /* best match count                             */
    int liBstCmp ;          /* best match compare state                     */

    off_t lzTstNew ;    // test/found position in new file
    off_t lzTstOrg ;    // test/found position in old file

    int liRlb = mpHsh->get_reliability() ;  // current reliability range
    if (liRlb < 1024) liRlb = 1024 ;

    /* loop on the table */
    for (liIdx = 0; liIdx < MCH_PME; liIdx ++) {	// TODO loop on linked list instead of full table!
        for (lpCur = mpMch[liIdx]; lpCur != null; lpCur=lpCur->ipNxt) {
            liCurCnt = (lpCur->iiTyp < 0) ? 0 : lpCur->iiCnt ; // TODO do this later, only when necessary

            // if empty or old
            if ((lpCur->iiCnt == 0)	/* empty */
                    || (lpCur->izNew + mpHsh->get_reliability() < azRedNew)){ /* old */
                // do nothing: skip empty or old entries
            }
            // else if potentially better
            else if ((lpBst == null)                            // no solution found yet ?
                    ||  ((lpCur->izBeg - liRlb < azBstNew + FZY)  // or probably nearer
                            && ((azRedNew < azBstNew + FZY)       // and still possible to improve ?
                                    || (liCurCnt > liBstCnt))))   // or probably longer ?
            {
                /* calculate the test position */
            	lzTstNew = lpCur->izBeg - liRlb ;
                if (lzTstNew >= azRedNew){
                    liDst = liRlb ;
                } else {
                    lzTstNew = azRedNew ;
                    liDst = lpCur->izBeg - lzTstNew ; // TODO liDst may overflow ??
                    if (liDst < liRlb)
                        liDst=liRlb;
                }

                /* calculate the test position on the original file by applying izDlt */
                if ((lpCur->iiTyp < 0)){
                    // we're on a gliding match
                    if (lzTstNew >= lpCur->izBeg) {
                        // within gliding match
                        lzTstOrg = lpCur->izOrg ;
                    } else {
                        // before gliding match
                        lzTstOrg = lzTstNew + lpCur->izDlt;
                        if (lzTstOrg < 0) {
                            lzTstNew -= lzTstOrg ;
                            lzTstOrg = 0 ;
                        }
                    }
                } else {
                    // colliding match
                    lzTstOrg =  lzTstNew + lpCur->izDlt ;
                    if (lzTstOrg < 0) {
                        lzTstNew -= lzTstOrg ;
                        lzTstOrg = 0 ;
                    }
                } /* if else gliding/colliding match */

                /* compare */
                liCurCmp = check(lzTstOrg, lzTstNew, liDst, mbCmpAll?1:2) ;

                /* soft eof reached, then rely on hash function */
                if (liCurCmp == 1){
                    if (lpCur->iiCnt < 2){
                        liCurCmp = 7 ; // most probably unequal
                    } else {
                        // estimate a realistic "find" position
                        if (lpCur->izBeg >= azRedNew)
                            lzTstNew = lpCur->izBeg ;
                        else if (lpCur->izNew >= azRedNew)
                            lzTstNew = azRedNew ;
                        else
                            liCurCmp = 7 ;
                        lzTstOrg = lzTstNew + lpCur->izDlt ;
                    }
                } /* if liCmp == 1 */

                /* remove false matches */
                if (liCurCmp >= 2){
                    lpCur->iiCnt-- ;
                    siHshRpr++ ;
                }

                /* evaluate: keep the best solution */
                if (liCurCmp <= 1){
                    if ((lpBst == NULL)								// first found
                       || (lzTstNew + FZY < azBstNew)				// substantially nearer
                       || ((lzTstNew <= azBstNew + FZY)				// potentially longer
                            && (liCurCnt > liBstCnt)
                             && (liCurCmp <= liBstCmp))
                    ) {
                        // new solution seems to be better
                        azBstNew = lzTstNew ;
                        azBstOrg = lzTstOrg ;
                        lpBst = lpCur ;
                        liBstCnt = liCurCnt ;
                        liBstCmp = liCurCmp ;
                    }
                }

                /* show table */
                #if debug
                if (JDebug::gbDbg[DBGMCH])
                    fprintf(JDebug::stddbg, "Mch %1d%c[%c"P8zd","P8zd","P8zd",%4d]"P8zd":%"PRIzd":%d\n",
                            liCurCmp,
                            (lpBst == lpCur)?'*': (liCurCmp == 0)?'=': (liCurCmp==1)?'?':':',
                                            (lpCur->iiTyp<0)?'G': (lpCur->iiTyp>0)?'C': ' ',
                                                    lpCur->izOrg, lpCur->izNew, lpCur->izBeg, lpCur->iiCnt,
                                                    lzTstNew, lpCur->izDlt, liDst) ;
                #endif
            } else {
                /* show table */
                #if debug
                if (JDebug::gbDbg[DBGMCH])
                    if ((lpCur->iiCnt > 0) && (lpCur->izBeg > 0))
                        fprintf(JDebug::stddbg, "Mch  :[%c"P8zd","P8zd","P8zd",%4d] D=%"PRIzd"\n",
                                (lpCur->iiTyp<0)?'G': (lpCur->iiTyp>0)?'C': ' ',
                                        lpCur->izOrg, lpCur->izNew, lpCur->izBeg, lpCur->iiCnt,
                                        lpCur->izDlt) ;
                #endif
            } /* if else lpCur old, empty, better */
        } /* for lpCur */
    } /* for liIdx */

    #if debug
    if (JDebug::gbDbg[DBGMCH])
        if (lpBst == null)
            fprintf(JDebug::stddbg, "Mch Err\n") ;
    #endif

    return (lpBst != null);
} /* get() */

/* -----------------------------------------------------------------------------
 * ufMchFre: cleanup & check if there is free space in the table of matches
 * ---------------------------------------------------------------------------*/
bool JMatchTable::cleanup ( off_t const &azBseNew ){
    rMch *lpCur ;
    rMch *lpPrv ;

    for (int liIdx = 0; liIdx < MCH_PME; liIdx ++) {
        lpPrv = null ;
        lpCur=mpMch[liIdx];
        while(lpCur != null)  {
            // if bad or old
            if (lpCur->iiCnt == 0 || lpCur->izNew < azBseNew){
                // remove from list
                if (lpPrv == null)
                    mpMch[liIdx] = lpCur->ipNxt ;
                else
                    lpPrv->ipNxt = lpCur->ipNxt ;

                // add to free-list
                lpCur->ipNxt = mpMchFre ;
                mpMchFre = lpCur ;

                // next
                if (lpPrv == null){
                    lpCur = mpMch[liIdx] ;
                } else {
                    lpCur = lpPrv->ipNxt ;
                }
            } else {
                lpPrv = lpCur ;
                lpCur = lpCur->ipNxt ;
            }
        }
    }

    return (mpMchFre != null) ;
} /* cleanup() */

/* -----------------------------------------------------------------------------
 * check(): verify and optimize matches
 *
 * Searches at given positions for a run of 24 equal bytes.
 * Searching continues for the given length unless soft-reading is specified
 * and the end-of-buffer is reached.
 *
 * Arguments:     &rzPosOrg    in/out  position on first file
 *                &rzPosNew    in/out  position on second file
 *                 aiLen       in      number of bytes to compare
 *                 aiSft       in      1=hard read, 2=soft read
 *
 * Return value:   0 = run of 24 equal bytes found
 *                 1 = end-of-buffer reached
 *                 2 = no run of equal byes found
 * ---------------------------------------------------------------------------*/
int JMatchTable::check (
    off_t &azPosOrg, off_t &azPosNew,
    int aiLen, int aiSft
    ) const
{ int lcOrg=EOF ;
  int lcNew=EOF ;
  int liEql=0 ;
  int liRet=0 ;

  #if debug
  if (JDebug::gbDbg[DBGCMP])
    fprintf( JDebug::stddbg, "Fnd ("P8zd","P8zd",%4d,%d): ",
      azPosOrg, azPosNew, aiLen, aiSft) ;
  #endif

  /* Compare bytes */
  for (; aiLen > SMPSZE - 8 && liRet == 0 && liEql < SMPSZE - 8; aiLen--) {
    lcOrg = mpFilOrg->get(azPosOrg ++, aiSft) ;
    lcNew = mpFilNew->get(azPosNew ++, aiSft) ;

    if (lcOrg == lcNew)
        liEql ++ ;
    else if (lcOrg < 0 || lcNew < 0)
        liRet = 1 ;
    else
        liEql = 0 ;
  }

  /* Compare last 24 bytes */
  for (; aiLen > 0 && liRet == 0 && liEql < SMPSZE - 8; aiLen--) {
      lcOrg = mpFilOrg->get(azPosOrg ++, aiSft) ;
      lcNew = mpFilNew->get(azPosNew ++, aiSft) ;

      if (lcOrg == lcNew)
          liEql ++ ;
      else if (lcOrg < 0 || lcNew < 0)
          liRet = 1 ;
      else
          liRet = 2 ;
  }

  #if debug
  if (JDebug::gbDbg[DBGCMP])
    fprintf( JDebug::stddbg, ""P8zd" "P8zd" %2d %s (%c)%3o == (%c)%3o\n",
             azPosOrg - liEql, azPosNew - liEql, liEql,
             (liRet==0)?"OK!":(liRet==1)?"EOF":"NOK",
             (lcOrg>=32 && lcOrg <= 127)?lcOrg:' ',lcOrg,
             (lcNew>=32 && lcNew <= 127)?lcNew:' ',lcNew);
  #endif

  switch (liRet){
  case 0: /* equality found */
      azPosOrg = azPosOrg - liEql ;
      azPosNew = azPosNew - liEql ;
      break;
  case 1:
      if (lcOrg == EOF || lcNew == EOF) {
          /* surely different (hard eof reached) */
          liRet = 2 ;
      } else {
          /* may be different (soft eof reached) */
          // TODO: caller should determine correct place here !
          azPosOrg = azPosOrg + aiLen ;
          azPosNew = azPosNew + aiLen ;
      }
      break ;
  case 2:
      /* surely different */
      break ;
  }
  return liRet ;
} /* check() */

} /* namespace JojoDiff */
