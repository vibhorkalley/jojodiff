/*
 * JOutRgn.h
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

#ifndef JOUTRGN_H_
#define JOUTRGN_H_

/*
 *
 */
#include "JOut.h"
#include <stdio.h>

namespace JojoDiff {

class JOutRgn: public JojoDiff::JOut {
public:
    JOutRgn(FILE *apFilOut );
    virtual ~JOutRgn();

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
};

}

#endif /* JOUTRGN_H_ */
