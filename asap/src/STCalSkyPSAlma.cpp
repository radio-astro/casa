//
// C++ Implementation: STCalSkyPSAlma
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

#include <casa/Logging/LogIO.h>

#include "STSelector.h"
#include "STCalSkyPSAlma.h"
#include "STDefs.h"
#include <atnf/PKSIO/SrcType.h>

using namespace std;
using namespace casa;

namespace asap {
STCalSkyPSAlma::STCalSkyPSAlma(CountedPtr<Scantable> &s)
  : STCalibration(s, "SPECTRA")
{
  applytable_ = new STCalSkyTable(*s, "PSALMA");
}

void STCalSkyPSAlma::setupSelector(const STSelector &sel)
{
  sel_ = sel;
  vector<int> types = sel_.getTypes();
  if (types.size() == 0) {
    types.resize(1);
    types[0] = SrcType::PSOFF;
    sel_.setTypes(types);
  }
  else if (find(types.begin(), types.end(), SrcType::PSOFF) == types.end()) {
    LogIO os(LogOrigin("STCalSkyPSAlma", "setupSelector", WHERE));
    os << LogIO::SEVERE << "Selection contains no data." << LogIO::EXCEPTION;
  }
  else {
    types.resize(1);
    types[0] = SrcType::PSOFF;
    sel_.setTypes(types);
  }
}
  
void STCalSkyPSAlma::appenddata(uInt scanno, uInt cycleno, 
				uInt beamno, uInt ifno, uInt polno, 
				uInt freqid, Double time, Float elevation, 
				const Vector<Float> &any_data,
				const Vector<uChar> &channel_flag)
{
  STCalSkyTable *p = dynamic_cast<STCalSkyTable *>(&(*applytable_));
  p->appenddata(scanno, cycleno, beamno, ifno, polno,
		freqid, time, elevation, any_data, channel_flag);
}

}
