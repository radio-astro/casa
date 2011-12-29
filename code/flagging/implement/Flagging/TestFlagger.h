//# TestFlagger.h: this defines TestFlagger
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

#include <iostream>
#include <vector>

#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta/Quantum.h>

#include <flagging/Flagging/FlagDataHandler.h>
#include <flagging/Flagging/FlagAgentBase.h>
#include <flagging/Flagging/FlagAgentSummary.h>

#include <boost/smart_ptr.hpp>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// TestFlagger: high-performance automated flagging
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
// TestFlagger performs automated flagging operations on a measurement set.
// The class is constructed from an MS. After that, the run method may be used
// to run any number of flagging agents.
// </synopsis>
//
// <example>
//        // construct MS and flagger
//        MeasurementSet ms("test.MS2",Table::Update);
//        TestFlagger testflagger(ms);
//        // build record of global flagging options
//        Record opt(Record::Variable);
//        // build record of flagging agents to be run
//        Record selopt( testflagger.defaultAgents().asRecord("select") );
//        selopt.define(RF_POLICY,"RESET");
//        selopt.define(RF_AUTOCORR,True);
//        Record agents(Record::Variable);
//        agents.defineRecord("select",selopt);
//        // perform the flagging
//        testflagger.run(agents,opt);
// </example>
//
// <motivation>
// We need an automated flagging tool. Existing tools (MSFlagger and flagger.g)
// were too slow. Hence, Flagger was developed.
// </motivation>
//

// TODO: write the above comments

class TestFlagger
{
protected:

	static LogIO os;

	// variables to parse to FlagDataHandler
	String msname_p;
	uShort iterationApproach_p;
	Double timeInterval_p;


	// members to parse to parseDataSelection
	String spw_p;
	String scan_p;
	String field_p;
	String antenna_p;
	String timerange_p;
	String correlation_p;
	String intent_p;
	String feed_p;
	String array_p;
	String uvrange_p;
	String observation_p;
	Record dataselection_p;

	// agent's members
	String mode_p;
	Record agentParams_p;
	FlagAgentSummary *summaryAgent_p;


	// variables for initFlagDataHandler and initAgents
	FlagDataHandler *fdh_p;
	std::vector<Record> agents_config_list_p;
	FlagAgentList agents_list_p;

public:  
	// default constructor
	TestFlagger();

	// destructor
	~TestFlagger();

	// reset everything
	void done();

	// configure the tool
	bool open(String msname, Double ntime);

	// DEPRECATED
	bool parseDataSelection(Record selrec);

	// parse the data selection
	bool selectData(Record selrec);
	bool selectData(String field, String spw, String array, String feed, String scan,
		       	    String antenna, String uvrange, String timerange,
		       	    String correlation, String intent, String observation="");

	// parse the parameters of the agent
	bool parseAgentParameters(Record agent_params);
	String getExpressionFunction(String expression);
	bool isExpressionPolarizationAll(String expression);


	// DEPRECATED
	bool initFlagDataHandler();

	bool initAgents();

	Record run();

	// Flag backup methods
	bool printFlagSelections();
	bool saveFlagVersion(String versionname, String comment, String merge);
	bool restoreFlagVersion(Vector<String> versionname, String merge);
	bool deleteFlagVersion(Vector<String> versionname);
	bool getFlagVersionList( Vector<String> &verlist);


private:

	TestFlagger(const TestFlagger &) {};

	TestFlagger& operator=(const TestFlagger &)  {return *this;};

	// Sink used to store history
	LogSink logSink_p;

	// Debug message flag
	static const bool dbg;

};


} //# NAMESPACE CASA - END

#endif

