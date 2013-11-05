/*
 * JFile.h
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

#ifndef JFILE_H_
#define JFILE_H_
#include <stdio.h>

namespace JojoDiff {

/* JDiff perform "addressed" file accesses when reading,
 * hence this abstract wrapper class to translate between streamed and addressed access.
 *
 * This abstraction also allows anyone to easily apply JDiff to his own data structures, by providing
 * a JFile descendant to JDiff.
 */
class JFile {
public:
	virtual ~JFile(){};

	/**
	 * Get byte at specified address. Auto-increments the address to the next byte.
	 * Soft read ahead will return an EOB when reading out of the buffer.
	 *
	 * @param azPos		position to read from, gets incremented by one after reading
	 * @param aiTyp		0=read, 1=hard read ahead, 2=soft read ahead
	 * @return 			the read character or EOF or EOB.
	 */
	virtual int get (
	    const off_t &azPos,	/* position to read from                */
	    const int aiTyp     /* 0=read, 1=hard ahead, 2=soft ahead   */
	) = 0 ;

	/**
	 * Return number of seek operations performed.
	 */
	virtual long seekcount() = 0;
};
} /* namespace */
#endif /* JFILE_H_ */
