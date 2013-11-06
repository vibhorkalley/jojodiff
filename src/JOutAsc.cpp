/*
 * JOutAsc.cpp
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

#include "JOutAsc.h"

namespace JojoDiff {

JOutAsc::JOutAsc(FILE *apFilOut ) : mpFilOut(apFilOut){
}

JOutAsc::~JOutAsc() {
}

/* ---------------------------------------------------------------
 * ufOutBytAsc: simple ascii output function for visualisation
 * ---------------------------------------------------------------*/
bool JOutAsc::put (
  int   aiOpr,
  off_t azLen,
  int   aiOrg,
  int   aiNew,
  off_t azPosOrg,
  off_t azPosNew
){
  static int liOprCur = ESC ;

  if (aiOpr == ESC) return false ;

  fprintf(mpFilOut, P8zd" ", azPosOrg) ;
  fprintf(mpFilOut, P8zd" ", azPosNew) ;

  switch (aiOpr) {
    case (MOD) :
      fprintf(mpFilOut, "MOD %3o %3o %c-%c\n", aiOrg, aiNew,
        ((aiOrg >= 32 && aiOrg <= 127)?(char) aiOrg:' '),
        ((aiNew >= 32 && aiNew <= 127)?(char) aiNew:' '));

      if (liOprCur != aiOpr) {
        liOprCur = aiOpr ;
        gzOutBytCtl+=2;
      }
      if (aiNew == ESC) gzOutBytEsc++;
      gzOutBytDta++;
      break;

    case (INS) :
      fprintf(mpFilOut, "INS     %3o  -%c\n", aiNew,
        ((aiNew >= 32 && aiNew <= 127)?(char) aiNew:' '));

      if (liOprCur != aiOpr) {
        liOprCur = aiOpr ;
        gzOutBytCtl+=2;
      }
      if (aiNew == ESC) gzOutBytEsc++;
      gzOutBytDta++;
      break;

    case (DEL) :
      fprintf(mpFilOut, "DEL %"PRIzd"\n", azLen);

      liOprCur=DEL;
      gzOutBytCtl+=2+ufPutSze(azLen);
      gzOutBytDel+=azLen;
      break;

    case (BKT) :
      fprintf(mpFilOut, "BKT %"PRIzd"\n", azLen);

      liOprCur=BKT;
      gzOutBytCtl+=2+ufPutSze(azLen);
      gzOutBytBkt+=azLen;
      break;

    case (EQL) :
      fprintf(mpFilOut, "EQL %3o %3o %c-%c\n", aiOrg, aiNew,
        ((aiOrg >= 32 && aiOrg <= 127)?(char) aiOrg:' '),
        ((aiNew >= 32 && aiNew <= 127)?(char) aiNew:' '));

      if (liOprCur != aiOpr) {
        liOprCur = aiOpr ;
        gzOutBytCtl+=2+4; // 4=approx length uf ufPutLen
      }
      gzOutBytEql++;
      break;
  }
  return false ;	// we always want details
} /* put */

int JOutAsc::ufPutSze ( off_t azLen )
{ if (azLen <= 252) {
    return 1 ;
  } else if (azLen <= 508) {
    return 2;
  } else if (azLen <= 0xffff) {
    return 3;
#ifdef JDIFF_LARGEFILE
  } else if (azLen <= 0xffffffff) {
#else
  } else {
#endif
    return 5;
  }
#ifdef JDIFF_LARGEFILE
  else {
    return 9;
  }
#endif
} /* ufPutSze */
} /* namespace */
