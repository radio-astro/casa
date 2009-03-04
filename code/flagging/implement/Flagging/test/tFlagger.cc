#include <flagging/Flagging/RFASelector.h>

#include <msvis/MSVis/VisSet.h>

#include <casa/aips.h>
#include <casa/namespace.h>
#include <cassert>

const String MSFILE = "/diska/home/jmlarsen/ALMA/casa/data/protopipe/imager/cal.3mmcont.ggtau.ms";
const String MSFILE2 = "cal.3mmcont.ggtau.ms";
int main (int,char *)
{
  //construct MS and flagger 
  assert( system(("rm -rf " + MSFILE2).c_str()) == 0);
  assert( system(("cp -r " + MSFILE  + " .").c_str()) == 0);

  MeasurementSet ms(MSFILE2, Table::Update);

  Flagger flagger(ms);

  flagger.setdata("", "", "", "", "", "", "", "", "");

  String time("now");
  uInt ant1(5);
  uInt ant2(1);
  String corr("LL");
  uInt spw(3);
  String chan("62");
  FlagIndex fi(time, ant1, ant2, spw, chan, corr);

  std::vector<FlagIndex> flagIndices;
  flagIndices.push_back(fi);

  flagger.applyFlags(flagIndices);

  //cerr << __FILE__ << __LINE__ << "defaultAgents = " << flagger.defaultAgents() << endl;
  //Record agents(Record::Variable); 

  //cerr << __FILE__ << __LINE__ << agents << endl;


  //perform the flagging 
  Bool trial = False;
  Bool reset = True;
  flagger.run(trial, reset);

  return 0;
}
