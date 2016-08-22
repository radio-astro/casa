//
// C++ Implementation: STCalSkyOtfAlma
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp> (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <vector>
#include "STSelector.h"
#include "STCalSkyOtfAlma.h"
#include "RowAccumulator.h"
#include "STIdxIter.h"
#include "STDefs.h"
#include "EdgeMarker.h"

#include <atnf/PKSIO/SrcType.h>

using namespace std;
using namespace casa;

namespace asap {
  STCalSkyOtfAlma::STCalSkyOtfAlma(CountedPtr<Scantable> &s, bool israster)
    : STCalSkyPSAlma(s),
      israster_(israster)
{
  rowNumbers_ = scantable_->table().rowNumbers();
}

void STCalSkyOtfAlma::setupSelector(const STSelector &sel)
{
  sel_ = sel;

  // Detect edges using EdgeMarker
  EdgeMarker marker(israster_); 

  // we can set insitu=True since we only need 
  // a list of rows to be marked. No marking is 
  // done here.
  marker.setdata(scantable_, True);  
  marker.examine();
  marker.setoption(options_);
  marker.detect();
  Block<uInt> rows = marker.getDetectedRows();
  vector<int> vectorRows(rows.nelements());
  for (size_t i = 0; i < vectorRows.size(); ++i)
    vectorRows[i] = rowNumbers_[rows[i]];
  
  // Set list of row indices to selector
  sel_.setRows(vectorRows);
}

}
