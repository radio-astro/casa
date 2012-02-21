//# FlagAgentRFlag.h: This file contains the interface definition of the FlagAgentRFlag class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2012, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2012, All rights reserved.
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

#ifndef FlagAgentRFlag_H_
#define FlagAgentRFlag_H_

#include <flagging/Flagging/FlagAgentBase.h>
#include <casa/Utilities/DataType.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class FlagAgentRFlag : public FlagAgentBase {

public:

	FlagAgentRFlag(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube = false, Bool flag = true);
	~FlagAgentRFlag();

protected:

	// Parse configuration parameters
	void setAgentParameters(Record config);

	// Compute flags for a given (time,freq) map
	bool computeAntennaPairFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows);

	// Convenience function to get simple averages
	Double mean(vector<Double> &data,vector<Double> &counts);

	// Convenience function to compute median
	Double median(vector<Double> &data);

	// Convenience function to get simple averages
	Double computeThreshold(vector<Double> &data, vector<Double> &dataSquared, vector<Double> &counts);

	// Function to be called for each timestep/channel
	void computeAntennaPairFlagsCore(	Int spw,
										Double noise,
										Double scutof,
										uInt timeStart,
										uInt timeStop,
										uInt centralTime,
										VisMapper &visibilities,
										FlagMapper &flags);
	// Function to return histograms
	FlagReport getReport();

	// Function to return histograms
	FlagReport getReportCore(	map< Int,vector<Double> > &data,
								map< Int,vector<Double> > &dataSquared,
								map< Int,vector<Double> > &counts,
								string label,
								Double scale);

private:

	// General parameters
	Bool doplot_p;
	uInt nTimeSteps_p;
	Double noiseScale_p;
	Double scutofScale_p;

	// Spectral Robust fit
	uInt nIterationsRobust_p;
	vector<Double> thresholdRobust_p;
	Double spectralmin_p;
	Double spectralmax_p;

	// Time-direction analysis
	Array<Double> noise_p;
	map<Int,Double> spw_noise_map_p;
	map< Int,vector<Double> > spw_noise_histogram_sum_p;
	map< Int,vector<Double> > spw_noise_histogram_sum_squares_p;
	map< Int,vector<Double> > spw_noise_histogram_counts_p;

	// Spectral analysis
	Array<Double> scutof_p;
	map<Int,Double> spw_scutof_map_p;
	map< Int,vector<Double> > spw_scutof_histogram_sum_p;
	map< Int,vector<Double> > spw_scutof_histogram_sum_squares_p;
	map< Int,vector<Double> > spw_scutof_histogram_counts_p;
};


} //# NAMESPACE CASA - END

#endif /* FlagAgentRFlag_H_ */

