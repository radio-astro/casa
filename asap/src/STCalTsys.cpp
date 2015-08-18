//
// C++ Implementation: STCalTsys
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

#include <casa/Arrays/ArrayMath.h>
#include <casa/Logging/LogIO.h>

#include "STSelector.h"
#include "STCalTsys.h"
#include "STDefs.h"
#include <atnf/PKSIO/SrcType.h>

using namespace std;
using namespace casa;

namespace asap {
STCalTsys::STCalTsys(CountedPtr<Scantable> &s, vector<int> &iflist)
  : STCalibration(s, "TSYS"),
    iflist_(iflist),
    tsysspw_(),
    do_average_(false)
{
  applytable_ = new STCalTsysTable(*s);
}

STCalTsys::STCalTsys(CountedPtr<Scantable> &s, Record &iflist, bool average)
  : STCalibration(s, "TSYS"),
    iflist_(),
    tsysspw_(iflist),
    do_average_(average)
{
  iflist_.resize(tsysspw_.nfields());
  for (uInt i = 0; i < tsysspw_.nfields(); ++i) {
    iflist_[i] = std::atoi(tsysspw_.name(i).c_str());
  }
  applytable_ = new STCalTsysTable(*s);
}

void STCalTsys::setupSelector(const STSelector &sel)
{
  sel_ = sel;
  vector<int> ifnos = sel_.getIFs();
  if (ifnos.size() > 0) {
    int nif = 0;
    int nifOrg = iflist_.size();
    vector<int> iflistNew(iflist_);
    for (int i = 0; i < nifOrg; i++) {
      if (find(ifnos.begin(), ifnos.end(), iflist_[i]) != ifnos.end()) {
        iflistNew[nif] = iflist_[i];
        ++nif;
      }
    }
    if (nif == 0) {
      LogIO os(LogOrigin("STCalTsys", "setupSelector", WHERE));
      os << LogIO::SEVERE << "Selection contains no data." << LogIO::EXCEPTION;
    }
    sel_.setIFs(iflistNew);
  }
  else {
    sel_.setIFs(iflist_);
  }
}

void STCalTsys::appenddata(uInt scanno, uInt cycleno, 
			   uInt beamno, uInt ifno, uInt polno, 
			   uInt freqid, Double time, Float elevation, 
			   const Vector<Float> &any_data,
			   const Vector<uChar> &channel_flag)
{
  STCalTsysTable *p = dynamic_cast<STCalTsysTable *>(&(*applytable_));
  if (do_average_ && tsysspw_.isDefined(String::toString(ifno))) {
    LogIO os(LogOrigin("STCalTsys", "appenddata", WHERE));
    Vector<Float> averaged_data(any_data.size());
    Vector<uChar> averaged_flag(any_data.size(), 0);
    Float averaged_value = 0.0;
    size_t num_value = 0;
    Vector<Double> channel_range = tsysspw_.asArrayDouble(String::toString(ifno));
    os << LogIO::DEBUGGING << "do averaging: channel range for IFNO " << ifno << " is " << channel_range << LogIO::POST;
    for (uInt i = 1; i < channel_range.size(); i += 2) {
      size_t start = (size_t)channel_range[i-1];
      size_t end = std::min((size_t)channel_range[i] + 1, averaged_data.size());
      os << LogIO::DEBUGGING << "start=" << start << ", end=" << end << LogIO::POST;
      float sum_per_segment = 0.0;
      size_t count = 0;
      for (size_t j = start; j < end; ++j) {
	if (channel_flag[j] == 0) {
	  sum_per_segment += any_data[j];
	  count += 1;
	}
      }
      averaged_value += sum_per_segment;
      num_value += count;
    }
    averaged_value /= (Float)num_value;
    averaged_data = averaged_value;
    os << LogIO::DEBUGGING << "averaged_data = " << averaged_data << LogIO::POST;
    os << LogIO::DEBUGGING << "any_data = " << any_data << LogIO::POST;
    p->appenddata(scanno, cycleno, beamno, ifno, polno,
		  freqid, time, elevation, averaged_data,
		  averaged_flag);
  }
  else {
    p->appenddata(scanno, cycleno, beamno, ifno, polno,
		  freqid, time, elevation, any_data,
		  channel_flag);
  }
}

}
