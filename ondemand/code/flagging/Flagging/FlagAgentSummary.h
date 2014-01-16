//# FlagAgentSummary.h: This file contains the interface definition of the FlagAgentSummary class.
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

#ifndef FlagAgentSummary_H_
#define FlagAgentSummary_H_

#include <flagging/Flagging/FlagAgentBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class FlagAgentSummary : public FlagAgentBase {

public:

	FlagAgentSummary(FlagDataHandler *dh, Record config);
	~FlagAgentSummary();

	Record getResult();

protected:

	// Common functionality for each visBuffer (don't repeat at the row level)
	void preProcessBuffer(const vi::VisBuffer2 &visBuffer);

	// Compute flags for a given mapped visibility point
	bool computeRowFlags(const vi::VisBuffer2 &visBuffer, FlagMapper &flags, uInt row);

	// Parse configuration parameters
	void setAgentParameters(Record config);

        // Get the summary dictionary, and 'view' reports.
        FlagReport getReport();

private:

        // Build simple plot-reports from the summary dictionary
        FlagReport buildFlagCountPlots();
        std::map<Int , std::vector<Double> > frequencyList;

	Bool spwChannelCounts;
	Bool spwPolarizationCounts;
        Bool baselineCounts;

        std::map<std::string, std::map<std::string, uInt64> > accumflags;
        std::map<std::string, std::map<std::string, uInt64> > accumtotal;

        std::map<Int, std::map<uInt, uInt64> > accumChannelflags;
        std::map<Int, std::map<uInt, uInt64> > accumChanneltotal;

        std::map<Int, std::map<std::string, uInt64> > accumPolarizationflags;
        std::map<Int, std::map<std::string, uInt64> > accumPolarizationtotal;

        std::map<Int, std::map<Int, uInt64> > accumAntScanflags;
        std::map<Int, std::map<Int, uInt64> > accumAntScantotal;

        uInt64 accumTotalFlags, accumTotalCount;

	Int arrayId;
	Int fieldId;
	Int spw;
	Int scan;
	Int observationId;

	string arrayId_str;
	string fieldId_str;
	string spw_str;
	string scan_str;
	string observationId_str;

};


} //# NAMESPACE CASA - END

#endif /* FlagAgentSummary_H_ */

