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

#include <stdlib.h>
#include <stdio.h>

#include "JFileIStream.h"
#include "JDebug.h"

namespace JojoDiff {
JFileIStream::JFileIStream(istream * apFil, const char *asFid) :
    mpStream(apFil), msFid(asFid), mzPosInp(0), mlFabSek(0)
{
}

JFileIStream::~JFileIStream() {
}

/**
 * Return number of seeks performed.
 */
long JFileIStream::seekcount(){return mlFabSek; }

/**
 * Gets one byte from the lookahead file.
 */
int JFileIStream:: get (
    const off_t &azPos,    	/* position to read from                */
    const int aiTyp     /* 0=read, 1=hard ahead, 2=soft ahead   */
) {
    if (azPos != mzPosInp){
        mlFabSek++;
        if (mpStream->eof())
            mpStream->clear();
        mpStream->seekg(azPos, std::ios::beg); // may throw an exception
    }
    mzPosInp = azPos + 1;
    return mpStream->get();
} /* function get */
} /* namespace JojoDiff */

