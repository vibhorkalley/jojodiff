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

#ifndef JFILEISTREAMAHEAD_H_
#define JFILEISTREAMAHEAD_H_

#include <istream>
using namespace std;

#include "JDefs.h"
#include "JFile.h"

namespace JojoDiff {
/**
 * Buffered JFile access: optimized buffering logic for the specific way JDiff
 * accesses files, that is reading ahead to find equal regions and then coming
 * back to the base position for actual comparisons.
 */
class JFileIStreamAhead: public JFile {
public:
    JFileIStreamAhead(istream * apFil, const char *asFid, const long alBufSze = 256*1024, const int aiBlkSze = 4096 );
    virtual ~JFileIStreamAhead();

    /**
     * Get one byte from the file at given position. Position is incremented by one.
     * Soft reading returns EOB when requested data is not in the buffer.
     */
    int get(
        const off_t &azPos,   /* position to read from                */
        const int aiTyp 	  /* 0=read, 1=hard ahead, 2=soft ahead   */
    );

    /**
     * Return number of seek operations performed.
     */
    long seekcount();

private:
    /**
     * Tries to get data from the buffer. Calls get_outofbuffer if that is not possible.
     * @param azPos		position to read from
     * @param aiTyp		0=read, 1=hard ahead, 2=soft ahead
     * @return data at requested position, EOF or EOB.
     */
    int get_frombuffer(
        const off_t &azPos,   /* position to read from                */
        const int aiTyp       /* 0=read, 1=hard ahead, 2=soft ahead   */
    );

    /**
     * Retrieves requested position into the buffer, trying to keep the buffer as
     * large as possible (i.e. invalidating/overwriting as less as possible).
     * Calls get_frombuffer afterwards to get the data from the buffer.
     *
     * @param azPos		position to read from
     * @param aiTyp		0=read, 1=hard ahead, 2=soft ahead
     * @param aiSek		seek to perform: 0=append, 1=seek, 2=scroll back
     * @return data at requested position, EOF or EOB.
     */
    int get_outofbuffer(
        const off_t &azPos, /* position to read from                */
        const int aiTyp, /* 0=read, 1=hard ahead, 2=soft ahead   */
        const int aiSek /* perform seek: 0=append, 1=seek, 2=scroll back  */
    );

private:
    /* Context */
    const char *msFid;  /* file id (for debugging)                      */
    istream *mpStream;  /* file handle                                  */

    /* Settings */
    long mlBufSze;      /* File lookahead buffer size                   */
    int miBlkSze;       /* Read file in blocks of 4096 bytes            */

    /* Buffer state */
    long miRedSze;      /* distance between izPosRed to izPosInp        */
    long miBufUsd;      /* number of bytes used in buffer               */
    uchar *mpBuf;       /* read-ahead buffer                            */
    uchar *mpMax;       /* read-ahead buffer end                        */
    uchar *mpInp;       /* current position in buffer                   */
    uchar *mpRed;       /* last position read from buffer				*/
    off_t mzPosInp;     /* current position in file                     */
    off_t mzPosRed;     /* last position read from buffer				*/
    off_t mzPosEof;     /* eof position 			                    */

    /* Statistics */
    long mlFabSek ;      /* Number of times an fseek operation was performed  */
};
}
#endif /* JFileIStreamAhead_H_ */
