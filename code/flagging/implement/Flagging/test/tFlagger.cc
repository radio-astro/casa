#include <flagging/Flagging/RFASelector.h>

#include <msvis/MSVis/VisSet.h>

#include <casa/aips.h>
#include <casa/namespace.h>
#include <cassert>

const String MSFILE = "/diska/jmlarsen/gnuactive/data/protopipe/imager/cal.3mmcont.ggtau.ms";
const String MSFILE2 = "cal.3mmcont.ggtau.ms";
int main (int,char *)
{
  //construct MS and flagger 
  assert( system(("rm -rf " + MSFILE2).c_str()) == 0);
  assert( system(("cp -r " + MSFILE  + " .").c_str()) == 0);

  MeasurementSet ms(MSFILE2, Table::Update);

  Flagger flagger(ms);

  flagger.setdata("", "", "", "", "", "", "", "", "");

  Double time(50487.6);
  Double exposure(30);
  uInt ant1(4);
  uInt ant2(3);
  uInt spw(0);
  uInt chan(61);
  String corr("XX");
  FlagIndex fi(time, exposure,
	       ant1, ant2, 
	       spw, chan, corr);

  std::vector<FlagIndex> flagIndices;
  flagIndices.push_back(fi);


  chan=62;

  flagIndices.push_back(FlagIndex(time, exposure,
				  ant1, ant2,
				  spw, chan, corr));

  if (1) {
      flagger.applyFlags(flagIndices);
  }
  else {
      flagger.setmanualflags(False, False,
			     False, "",
			     Vector<Double>(2,0.0),
			     "DATA",
			     False);
  }

  //cerr << __FILE__ << __LINE__ << "defaultAgents = " << flagger.defaultAgents() << endl;
  //Record agents(Record::Variable); 

  //cerr << __FILE__ << __LINE__ << agents << endl;


  //perform the flagging 
  Bool trial = False;
  Bool reset = True;
  flagger.run(trial, reset);

  return 0;
}
