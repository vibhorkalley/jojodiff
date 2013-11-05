/*
 * JFileIStream.cpp
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

#ifndef JFILEISTREAM_H_
#define JFILEISTREAM_H_

#include <istream>
using namespace std ;

#include "JDefs.h"
#include "JFile.h"

namespace JojoDiff {

/*
 * Unbuffered IStream access: all calls to JFile go straight through to istream.
 */
class JFileIStream: public JFile {
public:
    /**
     * Construct an unbuffered JFile on an istream.
     */
	JFileIStream(istream * apFil, const char *asFid);

	/**
	 * Destroy the JFile.
	 */
	virtual ~JFileIStream();

	/**
	 * Get one byte from the file from the given position.
	 */
	int get (
		    const off_t &azPos,	/* position to read from                */
		    const int aiTyp     /* 0=read, 1=hard ahead, 2=soft ahead   */
		);

    /**
     * Return number of seek operations performed.
     */
    long seekcount();

private:
	/* Context */
    istream *mpStream;      /* file handle                                  */
    const char *msFid;      /* file id (for debugging)                      */

    /* State */
    off_t mzPosInp;         /* current position in file                     */

    /* Statistics */
    long mlFabSek ;      /* Number of times an fseek operation was performed  */
};
}
#endif /* JFILEISTREAM_H_ */
