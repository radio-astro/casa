//# MSTransform.h: this defines MSTransform
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
#ifndef MSTRANSFORM_H
#define MSTRANSFORM_H

#include <iostream>
#include <vector>

#include <casacore/casa/Logging/LogIO.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Containers/Record.h>
#include <casacore/casa/Quanta/Quantum.h>
#include <mstransform/MSTransform/MSTransformManager.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// MSTransform: Apply transformations to data on the fly
// </summary>

// <use visibility=global>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MSTransformManager:description">MSTransformManager</linkto>
// </prerequisite>
//
// <synopsis>
// MSTransform .......
// </synopsis>
//
// <example>
// <srcblock>
// // The following is a typical way of using this class and its methods ...
//
// // Open the MS and attach it to the tool. This will create an object of the
// // FlagDataHandler (fdh) type. The constructor of the fdh takes three arguments,
// // the MS or CAL table, the iteration approach to use and the time interval. Only the MS
// // is mandatory to use. By default it will use the FlagDataHandler::SUB_INTEGRATION iteration
// // approach and 0.0 seconds as the time interval.
//
//    MSTransform *mst = new MSTransform();
//    mst->open('my.ms')
//
// // Select the data to process. If left blank, the whole MS will be selected. This step
// // will use the MS Selection class. There are two methods to perform the selection. One takes
// // a Record of the parameters, the other takes the individual parameters as arguments.
//
// // 1) First method:
//    String spw = "0:1~10";
//    String scan = "1";
//    Record selection = Record();
//    selection.define("spw", spw);
//    selection.define("scan", scan);
//    mst->selectData(selection);
//
// // 2) Second method:
//    mst->selectData(spw=spw, scan=scan);
//
//
// // The run method .....
//
//     mst->run();
//
// // To destroy the tool, call a method to execute the destructor.
//
//     mst->done();
//
// </srcblock>
// </example>
//
// <motivation>
// To avoid multiple read/writes of the data....
// </motivation>
//


class MSTransform
{
protected:

	LogIO log_p;

	// variables used to initialize the MSTransformManager
	String msname_p;
	String outputms_p;


	// members to parse to selectData
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
	Record config_p;
	String datacolumn_p;

	// Tells if tool is already configured at least once
	Bool isconfigured_p;


	// variables for initAgents
	MSTransformManager *mdh_p;

public:  
	// default constructor
	MSTransform();

	// destructor
	~MSTransform();

	// reset everything
	void done();

	// set the defaults of the parameters
//	Record & defaultOptions();


	// configure the tool and parse the parameters
	bool configure(Record config);

	// Open the MS and select the data
	bool open();

	// Run the tool and write the flags to the MS
	Record run();


private:

	MSTransform(const MSTransform &) {};

	MSTransform& operator=(const MSTransform &)  {return *this;};

//	String validateDataColumn(String datacol);

	// Sink used to store history
	LogSink logSink_p;


};


} //# NAMESPACE CASA - END

#endif

