#include <flagging/Flagging/FlagIndex.h>
#include <iostream>

namespace casa {

  
FlagIndex::FlagIndex(const String &time,
		     uInt ant1,
		     uInt ant2,
		     uInt spw,
		     const String &chan,
		     const String &corr)
  :
  time(time),
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
    ant1 = other.ant1;
    ant2 = other.ant2;
    spw = other.spw;
    chan = other.chan;
    corr = other.corr;
  }
  return *this;
}


ostream &operator<<(ostream &os, const casa::FlagIndex &fi) 
{
  os << "ant1 = " << fi.ant1 << endl
     << "ant2 = " << fi.ant2 << endl
     << "corr = " << fi.corr << " etc.";
  //etc.

  return os;
}

}
