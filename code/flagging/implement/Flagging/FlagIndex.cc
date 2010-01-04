#include <flagging/Flagging/FlagIndex.h>
#include <iostream>

namespace casa {

  
FlagIndex::FlagIndex(Double time,
		     Double exposure,
		     uInt ant1,
		     uInt ant2,
		     uInt spw,
		     uInt chan,
		     const String &corr)
  :
  time(time),
  exposure(exposure),
  ant1(ant1),
  ant2(ant2),
  spw(spw),
  chan(chan),
  corr(corr)
{}
  
FlagIndex::FlagIndex() {}

FlagIndex & FlagIndex::operator=(const FlagIndex & other)
{
  if (this != &other) {
    time = other.time;
    exposure = other.exposure;
    ant1 = other.ant1;
    ant2 = other.ant2;
    spw = other.spw;
    chan = other.chan;
    corr = other.corr;
  }
  return *this;
}

bool FlagIndex::operator==(const FlagIndex & other) const
{
    if (this == &other) return true;

    return
	(time-exposure/2.0 < other.time + other.exposure/2.0 &&
	 time+exposure/2.0 > other.time - other.exposure/2.0) &&
	ant1 == other.ant1 &&
	ant2 == other.ant2 &&
	spw  == other.spw &&
	chan == other.chan &&
	corr == other.corr;
    /* Note: criterion for overlapping time intervals:
       begin1 < end2 && end1 > begin2
    */

}

bool FlagIndex::operator!=(const FlagIndex & other) const
{
    return !(*this == other);
}

ostream &operator<<(ostream &os, const casa::FlagIndex &fi) 
{
    os << "time = " << fi.time << endl
       << "expo = " << fi.exposure << endl
       << "ant1 = " << fi.ant1 << endl
       << "ant2 = " << fi.ant2 << endl
       << "spw  = " << fi.spw  << endl
       << "chan = " << fi.chan << endl
       << "corr = " << fi.corr << endl;
  
  return os;
}

}
