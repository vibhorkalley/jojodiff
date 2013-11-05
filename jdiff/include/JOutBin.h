/*
 * JOutBin.h
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

#ifndef JOUTBIN_H_
#define JOUTBIN_H_

/*
 *
 */
#include <stdio.h>

#include "JOut.h"

namespace JojoDiff {

class JOutBin: public JOut {
public:
    JOutBin(FILE *apFilOut );
    virtual ~JOutBin();

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

    int   miOprCur ;        /* current operand: INS, MOD, EQL or DEL. ESC means none */
    off_t mzEqlCnt ;        /* number of pending equal bytes */
    int   miEqlBuf[4];      /* first four equal bytes */
    int mbOutEsc;           /* Pending escape character in data stream  ?*/

    void ufPutByt ( int aiByt ) ;
    void ufPutOpr ( int aiOpr ) ;
    int  ufPutSze ( off_t azLen ) ;
    void ufPutLen ( off_t azLen ) ;
};

}

#endif /* JOUTBIN_H_ */
