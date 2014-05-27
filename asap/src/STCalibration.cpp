//
// C++ Implementation: STCalibration
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp> (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "STCalibration.h"
#include "STCalSkyTable.h"
#include "RowAccumulator.h"
#include "STIdxIter.h"

using namespace casa;

namespace asap {
STCalibration::STCalibration(CountedPtr<Scantable> &s, const String target_column)
  : scantable_(s),
    target_column_(target_column)
{
}

void STCalibration::calibrate()
{
  STSelector selOrg = scantable_->getSelection();
  setupSelector(selOrg);
  scantable_->setSelection(sel_);
  
  fillCalTable();

  scantable_->setSelection(selOrg);
}

void STCalibration::fillCalTable()
{
  RowAccumulator acc(W_TINT);
  
  vector<string> cols(3);
  cols[0] = "IFNO";
  cols[1] = "POLNO";
  cols[2] = "BEAMNO";
  STIdxIter2 iter(scantable_, cols);

  ROScalarColumn<Double> *tcol = new ROScalarColumn<Double>(scantable_->table(), "TIME");
  Vector<Double> timeSec = tcol->getColumn() * 86400.0;
  tcol->attach(scantable_->table(), "INTERVAL");
  Vector<Double> intervalSec = tcol->getColumn();
  delete tcol;
  ROScalarColumn<Float> *ecol = new ROScalarColumn<Float>(scantable_->table(), "ELEVATION");
  Vector<Float> elevation = ecol->getColumn();
  delete ecol;

  ROArrayColumn<Float> specCol(scantable_->table(), target_column_);
  ROArrayColumn<uChar> flagCol(scantable_->table(), "FLAGTRA");
  ROScalarColumn<uInt> freqidCol(scantable_->table(), "FREQ_ID");

  // dummy Tsys: the following process doesn't need Tsys but RowAccumulator 
  //             requires to set it with spectral data
  Vector<Float> tsys(1, 1.0);

  Double timeCen = 0.0;
  Float elCen = 0.0;
  uInt count = 0;

  while(!iter.pastEnd()) {
    Vector<uInt> rows = iter.getRows(SHARE);
    Record current = iter.currentValue();
    //os_ << "current=" << current << LogIO::POST;
    uInt len = rows.nelements();
    if (len == 0) {
      iter.next();
      continue;
    }
    else if (len == 1) {
      uInt irow = rows[0];
      appenddata(0, 0, current.asuInt("BEAMNO"), current.asuInt("IFNO"), current.asuInt("POLNO"),
		 freqidCol(irow), timeSec[irow], elevation[irow], specCol(irow));
      iter.next();
      continue;
    }
    
    uInt nchan = scantable_->nchan(scantable_->getIF(rows[0]));
    Vector<uChar> flag(nchan);
    Vector<Bool> bflag(nchan);
    Vector<Float> spec(nchan);

    Vector<Double> timeSep(len); 
    for (uInt i = 0; i < len-1; i++) {
      timeSep[i] = timeSec[rows[i+1]] - timeSec[rows[i]] ;
    }
    Double tMedian = median(timeSep(IPosition(1,0), IPosition(1,len-2)));
    timeSep[len-1] = tMedian * 10000.0 ; // any large value

    uInt irow ;
    uInt jrow ;
    for (uInt i = 0; i < len; i++) {
      //os_ << "start row " << rows[i] << LogIO::POST;
      irow = rows[i];
      jrow = (i < len-1) ? rows[i+1] : rows[i];
      // accumulate data
      flagCol.get(irow, flag);
      convertArray(bflag, flag);
      specCol.get(irow, spec);
      if ( !allEQ(bflag,True) ) 
        acc.add( spec, !bflag, tsys, intervalSec[irow], timeSec[irow] ) ;
      timeCen += timeSec[irow];
      elCen += elevation[irow];
      count++;

      // check time gap
      double gap = 2.0 * timeSep[i] / (intervalSec[jrow] + intervalSec[irow]);
      if ( gap > 1.1 ) {
        if ( acc.state() ) {
          acc.replaceNaN() ;
//           const Vector<Bool> &msk = acc.getMask();
//           convertArray(flag, !msk);
//           for (uInt k = 0; k < nchan; ++k) {
//             uChar userFlag = 1 << 7;
//             if (msk[k]==True) userFlag = 0 << 7;
//             flag(k) = userFlag;
//           }
          timeCen /= (Double)count * 86400.0; // sec->day
          elCen /= (Float)count;
	  appenddata(0, 0, current.asuInt("BEAMNO"), current.asuInt("IFNO"), current.asuInt("POLNO"),
		     freqidCol(irow), timeCen, elCen, acc.getSpectrum());
        }
        acc.reset() ;
        timeCen = 0.0;
        elCen = 0.0;
        count = 0;
      }
    }

    iter.next() ;
    //os_ << "end " << current << LogIO::POST;
  }  
}
  
}
