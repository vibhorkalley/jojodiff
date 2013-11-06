/*
 * JOutRgn.cpp
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

#include "JOutRgn.h"

namespace JojoDiff {

JOutRgn::JOutRgn( FILE *apFilOut ) : mpFilOut(apFilOut){
}

JOutRgn::~JOutRgn() {
}

bool JOutRgn::put (
  int   aiOpr,
  off_t azLen,
  int   aiOrg,
  int   aiNew,
  off_t azPosOrg,
  off_t azPosNew
)
{ static int   siOprCur=ESC ;
  static off_t szOprCnt ;

  /* write output when operation code changes */
  if (aiOpr != siOprCur) {
    switch (siOprCur) {
      case (MOD) :
        gzOutBytCtl+=2;
        gzOutBytDta+=szOprCnt ;
        fprintf(mpFilOut, P8zd " " P8zd " MOD %"PRIzd"\n", azPosOrg - szOprCnt, azPosNew - szOprCnt, szOprCnt) ;
        break;

      case (INS) :
        gzOutBytCtl+=2;
        gzOutBytDta+=szOprCnt ;
        fprintf(mpFilOut, P8zd " " P8zd " INS %"PRIzd"\n", azPosOrg, azPosNew - szOprCnt, szOprCnt) ;
        break;

      case (DEL) :
        gzOutBytCtl+=2;
        gzOutBytDel+=szOprCnt;
        fprintf(mpFilOut, P8zd " " P8zd " DEL %"PRIzd"\n", azPosOrg - szOprCnt, azPosNew, szOprCnt);
        break;

      case (BKT) :
        gzOutBytCtl+=2;
        gzOutBytBkt+=szOprCnt;
        fprintf(mpFilOut, P8zd " " P8zd " BKT %"PRIzd"\n", azPosOrg + szOprCnt, azPosNew, szOprCnt);
        break;

      case (EQL) :
        gzOutBytCtl+=2;
        gzOutBytEql+=szOprCnt ;
        fprintf(mpFilOut, P8zd " " P8zd " EQL %"PRIzd"\n", azPosOrg - szOprCnt, azPosNew - szOprCnt, szOprCnt);
        break;
    }

    siOprCur = aiOpr;
    szOprCnt = 0;
  }

  /* accumulate operation codes */
  switch (aiOpr) {
	case (INS):
	case (MOD):
		if (aiNew == ESC)
			gzOutBytEsc++;
		// continue !
	case (DEL):
	case (BKT):
	case (EQL):
		szOprCnt += azLen;
		break;
	}
  return true ; // we never need details
} /* ufOutBytRgn */

}
