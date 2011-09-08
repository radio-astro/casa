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

#include <flagging/Flagging/RFCommon.h>
#include <flagging/Flagging/RFABase.h>
#include <tableplot/TablePlot/FlagVersion.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MRadialVelocity.h>
#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta/Quantum.h>

#include <flagging/Flagging/FlagDataHandler.h>

#include <boost/smart_ptr.hpp>

namespace casa { //# NAMESPACE CASA - BEGIN

class VisSet;
class RFChunkStats;

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
// <todo asof="2001/04/16">
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>


class TestFlagger : public FlaggerEnums
{
protected:

	// variables to parse to configTestFlagger
	String msname_p;
	Bool asyncio_p;
	Bool parallel_p;

	// variables to parse to configDataSelection
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
	Record *dataselection_p;

	static LogIO os;

	// variables for initFlagDataHandler
	FlagDataHandler *fdh_p;
	Vector<Record> *agents_config_list_p;
	Vector<FlagAgentBase> *agents_list_p;

public:  
	// default constructor
	TestFlagger  ();
	// construct and attach to a measurement set
	TestFlagger  ( MeasurementSet &ms );

	// destructor
	~TestFlagger ();

	// reset everything
	void done();

	// configure the tool
	void configTestFlagger(Record &config);

	// parse the data selection
	void configDataSelection(Record &selrec);

	// configure the parameters of the agent
	void configAgentParameters(Record &aparams);


private:

	TestFlagger(const TestFlagger &) {};

	TestFlagger& operator=(const TestFlagger &)  {return *this;};

	// Sink used to store history
	LogSink logSink_p;

};


} //# NAMESPACE CASA - END

#endif

