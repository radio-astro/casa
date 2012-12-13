//# Flagger.h: this defines Flagger
//# Copyright (C) 2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
#ifndef FLAGGING_FLAGGER_H
#define FLAGGING_FLAGGER_H

#include <flagging/Flagging/RFCommon.h>
#include <flagging/Flagging/RFABase.h>
#include <flagging/Flagging/FlagVersion.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MRadialVelocity.h>
#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta/Quantum.h>

#include <boost/smart_ptr.hpp>

namespace casa { //# NAMESPACE CASA - BEGIN

class VisSet;
class RFChunkStats;
        
// <summary>
// Flagger: high-performance automated flagging
// </summary>

// <use visibility=global>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> implement/Flagger
// </prerequisite>
//
// <etymology>
// MSFlagger and plain flagger were already taken.
// </etymology>
//
// <synopsis>
// Flagger performs automated flagging operations on a measurement set.
// The class is constructed from an MS. After that, the run method may be used
// to run any number of flagging agents.
// </synopsis>
//
// <example>
//        // construct MS and flagger
//        MeasurementSet ms("test.MS2",Table::Update);
//        Flagger flagger(ms);
//        // build record of global flagging options
//        Record opt(Record::Variable);
//        // build record of flagging agents to be run
//        Record selopt( flagger.defaultAgents().asRecord("select") );
//        selopt.define(RF_POLICY,"RESET");
//        selopt.define(RF_AUTOCORR,True);
//        Record agents(Record::Variable);
//        agents.defineRecord("select",selopt);
//        // perform the flagging
//        flagger.run(agents,opt);
// </example>
//
// <motivation>
// We need an automated flagging tool. Existing tools (MSFlagger and flagger.g)
// were too slow. Hence, Flagger was developed.
// </motivation>
//
// <todo asof="2001/04/16">
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>


class Flagger : public FlaggerEnums
{
protected:
// creates an agent by name
  boost::shared_ptr<RFABase> createAgent ( const String &name,RFChunkStats &chunk,const RecordInterface &parms, bool &only_selector );

// sets up record of agents and default parameters
  const RecordInterface & setupAgentDefaults ();

// print flagging reports from individual agents
  void printAgentReports  ( );

  void printSummaryReport ( RFChunkStats &chunk );
  Bool selectDataChannel();
 
  MeasurementSet   ms;
  MeasurementSet   originalms;
  std::vector<boost::shared_ptr<RFABase> > acc;

  //new added
  MeasurementSet *mssel_p;
  VisSet *vs_p;
  bool scan_looping;     /* Is scan number part of visiter looping? */
  String msname_p;
  Bool nullSelect_p;
  Bool setdata_p;
  Bool selectdata_p;
  String dataMode_p;  

//  Vector<Int> dataEnd_p;
//  Vector<Int> dataStart_p, dataStep_p;
//  Vector<Int> dataspectralwindowids_p;
//  Vector<Int> spwidnchans_p;
  Vector<String> correlations_p;
  Vector<Int> datafieldids_p;
  Vector<Int> datadescids_p;
  MRadialVelocity mDataStart_p;
  MRadialVelocity mDataStep_p;

  //
  uInt nant,nifr,nfeed,nfeedcorr;
  Vector<Int> ifr2ant1,ifr2ant2;
  Vector<String> antnames;
  Vector<Double> spwfreqs;
    
  Record agent_defaults;

  static LogIO os;

public:  
// default constructor 
  Flagger  ();
// construct and attach to a measurement set
  Flagger  ( MeasurementSet &ms );
  
  ~Flagger ();
  
// Change or set the MS this Flagger refers to.
  bool attach( MeasurementSet &ms, Bool setupAgentDefaults=True );

  // Set the data selection parameters
  Bool selectdata(Bool useoriginalms=False, 
                  String field="", String spw="", String array="", String feed="", String scan="",
	          String baseline="", String uvrange="", String time="",
	          String correlation="", String intent="", String observation="");

  // Make a data selection
  Bool setdata(String field, String spw, String array, String feed, String scan,
	       String baseline, String uvrange, String time,
	       String correlation, String intent, String observation="");
  
  // Make a selection for manual flagging
  Bool setmanualflags(Bool autocorr,
                      Bool unflag, 
                      String clipexpr, 
                      Vector<Double> cliprange, 
                      String clipcolumn, 
                      Bool outside, 
                      Bool channel_average,
                      Double quackinterval=0.0, 
                      String quackmode=String("beg"),
                      Bool quackincrement=Bool(false),
                      String opmode=String("flag"),
                      Double diameter = -1.0,
                      Double lowerlimit = -1.0,
                      Double upperlimit = 91.0);

  // Clean up all agents of type "select".
  //Bool clearflagselections(Vector<Int> &recordlist,Vector<String> &agentlist);
  Bool clearflagselections(Int recordindex);
  Bool printflagselections();

  // Fill the selection record to attach to the list of agents
  Bool fillSelections(Record &rec);

  // Set autoflag params
  Bool setautoflagparams(String algorithm, Record &parameters);

  // Get default autoflag params
  Record getautoflagparams(String algorithm);

  Bool addAgent(RecordInterface &newAgent);
  
  // Detaches from the MS  
  void detach();
  
  Record run(Bool trial, Bool reset);    

  void summary ( const RecordInterface &agents ); 

    // flag version support.
  Bool  saveFlagVersion(String versionname, String comment, String merge);
  Bool  restoreFlagVersion(Vector<String> versionname, String merge);
  Bool  deleteFlagVersion(Vector<String> versionname);
  Bool  getFlagVersionList( Vector<String> &verlist);
 
  
  // returns current MS
  const MeasurementSet & measSet() const { return ms; }
    
// number of antennas in MS
  uInt numAnt    () const 
      { return nant; };

// number of IFRs in MS
  uInt numIfr    () const 
      { return nifr; };

// number of feeds in MS
  uInt numFeed    () const 
      { return nfeed; };

// number of feed correlations in MS
  uInt numFeedCorr    () const 
      { return nfeedcorr; };

// names of antennas
  const Vector<String> & antNames() const 
      { return antnames; };
  
// derives a flat IFR index from two antenna indices
  uInt ifrNumber ( Int ant1,Int ant2 ) const;

// vector version of above
  Vector<Int> ifrNumbers ( Vector<Int> ant1,Vector<Int> ant2 ) const;

// derives antenna indices from a flat IFR index
  void ifrToAnt ( uInt &ant1,uInt &ant2,uInt ifr ) const;

// returns a record with all available agents and their default options
  const RecordInterface & defaultAgents () const 
      { return agent_defaults; }

// returns a record of available options
  static const RecordInterface & defaultOptions ();

// returns the log sink 
  static LogIO & logSink ()       { return os; }

  /* Get rid of negative indices (meaning negation of antenna) in baselinelist */
  static void reform_baselinelist(Matrix<Int> &baselinelist, unsigned nant);
  
  static int my_aipspp_sum(const Array<Bool> &a);

private:
    
  Flagger( const Flagger & )          {};

  Flagger& operator=(const Flagger &)  { return *this; };

  void printAgentRecord(String &, uInt, const RecordInterface &);

  // Sink used to store history
  LogSink logSink_p;

  // Hold the original ms 
  MeasurementSet *originalms_p;
  
  // MS Selection
  MSSelection *msselection_p;
  bool spw_selection;  //non-trivial spw-selection

  // List of Agents
  Record *agents_p;
  Int agentCount_p;

  // List of extra options
  Record *opts_p;

  // Debug Message flag
  static const bool dbg;

  Bool quack_agent_exists;
  /* More initialization is required, if there exists a quacking agent */
};


} //# NAMESPACE CASA - END

#endif

