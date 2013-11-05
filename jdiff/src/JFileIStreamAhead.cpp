/*
 * JFileIStreamAhead.cpp
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

#include <stdlib.h>
#include <stdio.h>
#include <exception>

#include "JFileIStreamAhead.h"
#include "JDebug.h"

namespace JojoDiff {

/**
 * Construct a buffered JFile on an istream.
 */
JFileIStreamAhead::JFileIStreamAhead(istream * apFil, const char *asFid, const long alBufSze, const int aiBlkSze ) :
        mpStream(apFil), mlBufSze(alBufSze), miBlkSze(aiBlkSze), mlFabSek(0)
{
    mpBuf = (uchar *) malloc(mlBufSze) ;
#ifndef __MINGW32__
    if (mpBuf == null){
        throw bad_alloc() ;
    }
#endif

    mpMax = mpBuf + mlBufSze ;
    mpInp = mpBuf;
    mpRed = mpInp;

    miBufUsd = 0;
    mzPosInp = 0;
    mzPosEof = MAX_OFF_T ;
    mzPosRed = 0 ;
    miRedSze = 0 ;
    msFid = asFid ;

#if debug
    if (JDebug::gbDbg[DBGBUF])
        fprintf(JDebug::stddbg, "ufFabOpn(%s):(buf=%p,max=%p,sze=%ld)\n",
                asFid, mpBuf, mpMax, mlBufSze);
#endif
    }

JFileIStreamAhead::~JFileIStreamAhead() {
	if (mpBuf != null) free(mpBuf) ;
}

/**
 * Return number of seeks performed.
 */
long JFileIStreamAhead::seekcount(){return mlFabSek; }

/**
 * Gets one byte from the lookahead file.
 */
int JFileIStreamAhead::get (
    const off_t &azPos, /* position to read from                */
    const int aiTyp     /* 0=read, 1=hard ahead, 2=soft ahead   */
) {
    if ((miRedSze > 0) && (azPos == mzPosRed)) {
        mzPosRed++ ;
        miRedSze--;
        #if debug
        if (JDebug::gbDbg[DBGRED])
          fprintf(JDebug::stddbg, "ufFabGet(%s,"P8zd",%d)->%2x (mem %p).\n",
             msFid, azPos, aiTyp, *mpRed, mpRed);
        #endif
        return *mpRed++;
    } else {
        return get_frombuffer(azPos, aiTyp);
    }
} /* int get(...) */

/**
 * Retrieves requested position into the buffer, trying to keep the buffer as
 * large as possible (i.e. invalidating/overwriting as less as possible).
 * Calls get_frombuffer afterwards to get the data from the buffer.
 *
 * @param azPos     position to read from
 * @param aiTyp     0=read, 1=hard ahead, 2=soft ahead
 * @param aiSek     seek to perform: 0=append, 1=seek, 2=scroll back
 * @return data at requested position, EOF or EOB.
 */
/**
 * Tries to get data from the buffer. Calls get_outofbuffer if that is not possible.
 * @param azPos     position to read from
 * @param aiTyp     0=read, 1=hard ahead, 2=soft ahead
 * @return data at requested position, EOF or EOB.
 */
int JFileIStreamAhead::get_frombuffer (
    const off_t &azPos,    /* position to read from                */
    const int aiTyp        /* 0=read, 1=hard ahead, 2=soft ahead   */
){
	uchar *lpDta ;
	bool liSek=0 ;	/* reposition on file ? 0=no, 1=yes, 2=scroll back */

	/* Get data from buffer? */
	if (azPos < mzPosInp) {
	    if ((azPos >= mzPosInp - miBufUsd )) {
	        // compute position in buffer
			lpDta = mpInp - (mzPosInp - azPos) ;
			if ( lpDta < mpBuf )
				lpDta += mlBufSze ;

			#if debug
			if (JDebug::gbDbg[DBGRED])
			  fprintf(JDebug::stddbg, "ufFabGet(%s,"P8zd",%d)->%2x (mem %p).\n",
				 msFid, azPos, aiTyp, *lpDta, lpDta);
			#endif

			// prepare next reading position (but do not increase lpDta!!!)
			mzPosRed = azPos + 1;
			mpRed = lpDta + 1;
			if (mpRed == mpMax) {
			    mpRed = mpBuf ;
			}
			if (mpRed > mpInp) {
			    miRedSze = mpMax - mpRed ;
			} else {
			    miRedSze = mzPosInp - mzPosRed;
			}

			// return data
			return *lpDta ;
		} else {
		    /* Seek & reset when reading before the buffer */
		    if (azPos + miBlkSze >= mzPosInp - miBufUsd )
		        liSek = 2 ; /* reading just before buffer */
		    else
		        liSek = 1 ; /* seek & reset buffer */
		}
	} else if (azPos >= mzPosEof) {
	    /* eof */
        #if debug
        if (JDebug::gbDbg[DBGRED])
        fprintf(JDebug::stddbg, "ufFabGet(%s,"P8zd",%d)->EOF (mem).\n",
           msFid, azPos, aiTyp);
        #endif

        mzPosRed = -1;
        mpRed = null;
        miRedSze = 0 ;

        return EOF ;
	} else if (azPos >= mzPosInp + miBlkSze) {
	    /* Reset  when reading "after" the buffer */
	    liSek=1;
	}

	/* Soft ahead: continue only if no seek */
	if (aiTyp == 2 && liSek != 0)  {
		#if debug
		if (JDebug::gbDbg[DBGRED])
		  fprintf(JDebug::stddbg, "ufFabGet(%p,"P8zd",%d)->EOB.\n",
			 msFid, azPos, aiTyp);
		#endif
		return EOB ;
	}

	return get_outofbuffer(azPos, aiTyp, liSek) ;
}

/**
 * Read data from the file into the buffer, then read from the buffer.
 */
int JFileIStreamAhead::get_outofbuffer (
    const off_t &azPos,    /* position to read from                */
    const int aiTyp,       /* 0=read, 1=hard ahead, 2=soft ahead   */
    const int aiSek        /* perform seek: 0=append, 1=seek, 2=scroll back  */
){
    int liTdo ;         /* number of bytes to read */
    int liDne ;         /* number of bytes read */
    uchar *lpInp ;      /* place in buffer to read to */
    off_t lzPos ;       /* position to seek */

    /* Set reading position: lzPos, lpInp and liTdo */
    switch (aiSek) {
        case (0):
            /* How many bytes can we read ? */
            liTdo = mpMax - mpInp ;
            if (liTdo > miBlkSze) liTdo = miBlkSze ;
            lpInp = mpInp ;
            lzPos = mzPosInp ;
            break ;

        case (1):
            /* reset buffer */
            mpInp = mpBuf;
            mzPosInp = azPos;
            mpRed = mpBuf;
            mzPosRed = azPos;
            miBufUsd = 0;
            miRedSze = 0 ;

            /* set position */
            lzPos = azPos ;
            lpInp = mpBuf;
            liTdo = miBlkSze ;

            break ;

        case (2):
            /* make room in buffer */
            liTdo = miBufUsd + miBlkSze - mlBufSze ; // number of bytes to remove in order to have room for giBlkSze
            if (liTdo > 0){
                miBufUsd -= liTdo ;
                mzPosInp -= liTdo ;
                mpInp -= liTdo ;
                if (mpInp < mpBuf)
                    mpInp += mlBufSze ;
            }

            /* scroll back on buffer */
            /* case 1: [^***********$-----------Tdo]   */
            /* case 2: [************$----Tdo^******]   */
            /* case 3: [Td^*********$--------------]   */
            /* case 4: [--Tdo^*****$---------------]   */
            lzPos = mzPosInp - miBufUsd ;
            liTdo = miBlkSze ;
            if ( lzPos < liTdo ) {
                liTdo = lzPos ;
            }
            lpInp = mpInp - miBufUsd ;
            if (lpInp == mpBuf){
                /* case 1 */
                lpInp = mpMax - liTdo ;
            } else if (lpInp > mpBuf) {
                /* case 3 or 4 (untested !!) */
                if (lpInp - liTdo >= mpBuf) {
                    /* case 4 */
                    lpInp -= liTdo ;
                } else {
                    /* case 3 */
                    liTdo = lpInp - mpBuf ;
                    lpInp = mpBuf ;
                }
            } else {
                /* case 2 */
                lpInp += mlBufSze - liTdo ;
            }
            miBufUsd += liTdo ;
            lzPos -= liTdo ;

            /* reset read position buffer */
            mpRed = 0 ;
            mzPosRed = -1;
            miRedSze = 0 ;
            break ;
    } /* switch aiSek */

    if (aiSek != 0) {
        #if debug
        if (JDebug::gbDbg[DBGBUF]) fprintf(JDebug::stddbg, "ufFabGet: Seek %"PRIzd".\n", azPos);
        #endif

        mlFabSek++ ;
        mpStream->seekg(lzPos) ; // throws an exception in case of error
    } /* if liSek */

    /* Read a chunk of data (in 16 kbyte blocks) */
    mpStream->read((char *)lpInp, liTdo) ;
    liDne = mpStream->gcount();
    if (liDne < liTdo) {
      #if debug
      if (JDebug::gbDbg[DBGRED])
        fprintf(JDebug::stddbg, "ufFabGet(%p,"P8zd",%d)->EOF.\n",
          msFid, azPos, aiTyp);
      #endif
      // TODO reduce number of times we pass here, init mzPosEof to MAX_OFF_T
      // Reset EOF state
      if (mpStream->eof())
          mpStream->clear();

      mzPosEof = lzPos + (off_t) liDne ;
      if (liDne == 0)
          return EOF ;
    }
    #if debug
    if (JDebug::gbDbg[DBGRED])
      fprintf(JDebug::stddbg, "ufFabGet(%s,"P8zd",%d)->%2x (sto %p).\n",
        msFid, azPos, aiTyp, *mpInp, mpInp);
    #endif

    switch (aiSek){
        case (2):
            if (liDne < liTdo){
                /* repair buffer */
                mpInp = lpInp + liDne;
                if ( mpInp >= mpMax)
                    mpInp -= mlBufSze ;
                mzPosInp = lzPos + liDne ;
                mpRed = lpInp;
                mzPosRed = lzPos;
                miBufUsd = liDne;
                miRedSze = liDne ;
            } else {
                /* Restore input position */
                mlFabSek++;
            	mpStream->seekg(mzPosInp); // throws an exception in case of error
            }
            break ;

        default:
            /* Advance input position */
            mzPosInp += liDne ;
            mpInp    += liDne ;
            if ( mpInp == mpMax ){
              mpInp = mpBuf ;
            } else if ( mpInp > mpMax ) {
              fprintf(stderr, "Buffer out of bounds on position %"PRIzd")!", azPos);
              exit(6);
            }
            if ( miBufUsd < mlBufSze ) miBufUsd += liDne ;
            if ( miBufUsd > mlBufSze ) {
                // Quite uncommon, but not an error.
                // Can happen for example after scrolling back (case 1) less than giBlkSze
                miBufUsd = mlBufSze ;
            }
            miRedSze += liDne ;
            if (mpRed == mpMax)
                mpRed = mpBuf ;
            break;
    } /* switch aiSek */

    /* read it again */
    return get(azPos, aiTyp);
} /* get_outofbuffer */
} /* namespace JojoDiff */
