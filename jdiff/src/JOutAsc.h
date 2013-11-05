/*
 * JOutAsc.h
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

#ifndef JOUTASC_H_
#define JOUTASC_H_
#include "JDefs.h"
/*
 *
 */
#include "JOut.h"

namespace JojoDiff {

class JOutAsc: public JOut {
public:
    JOutAsc(FILE *apFilOut );
    virtual ~JOutAsc();

    virtual bool put (
      int   aiOpr,
      off_t azLen,
      int   aiOrg,
      int   aiNew,
      off_t azPosOrg,
      off_t azPosNew
    );

private:
    FILE *mpFilOut ;    // output file

    int ufPutSze ( off_t azLen );
}; /* class */
} /* namespace */
#endif /* JOUTASC_H_ */
