//# FlagReport.h: This file contains the interface definition of the FlagReport class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef FLAGREPORT_H_
#define FLAGREPORT_H_

// .casarc interface
#include <casa/System/AipsrcValue.h>

// Records interface
#include <casa/Containers/Record.h>

// System utilities (for profiling macros)
#include <casa/OS/HostInfo.h>
#include <sys/time.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Everything going into the FlagReport is by value
// Everything going out of it is by reference
class FlagReport : public Record
{
	public:
                FlagReport(String type=String("none"),String name=String(""), 
                                 String title=String(""), 
			         String xlabel=String(""), String ylabel=String("") );
                 // TODO : By value. Change to by-reference
                FlagReport(String type, String name, const Record &other); 
                FlagReport(const Record &other);
		~FlagReport();

                // Add, query and access reports from a "list" type FlagReport
                Bool addReport(FlagReport inpReport); 
                Int nReport();
                Bool accessReport(Int index, FlagReport &outReport);

                // Add and query data to plot, for FlagReports of type "plotraster","plotline","plotscatter"
                Bool addData(Array<Float> data);
                //Bool addData(Vector<Float> xdata,Vector<Float> ydata, String label);
                Bool addData(String plottype, Vector<Float> xdata,Vector<Float> ydata, String errortype, Vector<Float> error, String label);
                Int nData();

                String reportType();
                // Check validity of FlagReport.
                Bool verifyFields();

	protected:

	private:
                casa::LogIO logger_p;
};




} //# NAMESPACE CASA - END

#endif /* FLAGDATAHANDLER_H_ */
