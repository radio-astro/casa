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

	enum optype {

		MEAN,
		ROBUST_MEAN,
		MEDIAN,
		ROBUST_MEDIAN
	};

public:

	FlagAgentRFlag(FlagDataHandler *dh, casacore::Record config, casacore::Bool writePrivateFlagCube = false, casacore::Bool flag = true);
	~FlagAgentRFlag();

protected:

	// Parse configuration parameters
	void setAgentParameters(casacore::Record config);

	// Compute flags for a given (time,freq) map
	bool computeAntennaPairFlags(const vi::VisBuffer2 &visBuffer, VisMapper &visibilities,FlagMapper &flags,casacore::Int antenna1,casacore::Int antenna2,vector<casacore::uInt> &rows);

	// Extract automatically computed thresholds to use them in the next pass
	void passIntermediate(const vi::VisBuffer2 &visBuffer);

	// Remove automatically computed thresholds for the following scans
	void passFinal(const vi::VisBuffer2 &visBuffer);

	// Convenience function to get simple averages
	casacore::Double mean(vector<casacore::Double> &data,vector<casacore::Double> &counts);

	// Convenience function to compute median
	casacore::Double median(vector<casacore::Double> &data);

	//
	void noiseVsRef(vector<casacore::Double> &data, casacore::Double ref);

	// Convenience function to get simple averages
	casacore::Double computeThreshold(vector<casacore::Double> &data, vector<casacore::Double> &dataSquared, vector<casacore::Double> &counts);

	// casacore::Function to be called for each timestep/channel
	void computeAntennaPairFlagsCore(	pair<casacore::Int,casacore::Int> spw_field,
										casacore::Double noise,
										casacore::Double scutof,
										casacore::uInt timeStart,
										casacore::uInt timeStop,
										casacore::uInt centralTime,
										VisMapper &visibilities,
										FlagMapper &flags);

	void robustMean(	casacore::uInt timestep_i,
						casacore::uInt pol_k,
						casacore::uInt nChannels,
						casacore::Double &AverageReal,
						casacore::Double &AverageImag,
						casacore::Double &StdReal,
						casacore::Double &StdImag,
						casacore::Double &SumWeightReal,
						casacore::Double &SumWeightImag,
						VisMapper &visibilities,
						FlagMapper &flags);

	void simpleMedian(	casacore::uInt timestep_i,
						casacore::uInt pol_k,
						casacore::uInt nChannels,
						casacore::Double &AverageReal,
						casacore::Double &AverageImag,
						casacore::Double &StdReal,
						casacore::Double &StdImag,
						casacore::Double &SumWeightReal,
						casacore::Double &SumWeightImag,
						VisMapper &visibilities,
						FlagMapper &flags);

	// casacore::Function to return histograms
	FlagReport getReport();

	// casacore::Function to return histograms
	FlagReport getReportCore(	map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > &data,
								map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > &dataSquared,
								map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > &counts,
								map< pair<casacore::Int,casacore::Int>,casacore::Double > &threshold,
								FlagReport &totalReport,
								string label,
								casacore::Double scale);

	// Dedicated method to generate threshold values
	void generateThresholds(	map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > &data,
								map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > &dataSquared,
								map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > &counts,
								map< pair<casacore::Int,casacore::Int>,casacore::Double > &threshold,
								string label,
								casacore::Double scale);

private:

	// General parameters
	casacore::Bool doflag_p;
	casacore::Bool doplot_p;
	casacore::uInt nTimeSteps_p;
	casacore::Double noiseScale_p;
	casacore::Double scutofScale_p;

	// Spectral Robust fit
	casacore::uInt nIterationsRobust_p;
	vector<casacore::Double> thresholdRobust_p;
	casacore::Double spectralmin_p;
	casacore::Double spectralmax_p;
	casacore::uInt optype_p;
	void (casa::FlagAgentRFlag::*spectralAnalysis_p)(casacore::uInt,casacore::uInt,casacore::uInt,casacore::Double&,casacore::Double&,casacore::Double&,casacore::Double&,casacore::Double&,casacore::Double&,VisMapper&,FlagMapper&);

	// Store frequency to be used in Reports
	map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > field_spw_frequency_p;
	map< pair<casacore::Int,casacore::Int>,casacore::Double > field_spw_frequencies_p;

	// casacore::Time-direction analysis
	casacore::Double noise_p;
	map< pair<casacore::Int,casacore::Int>,casacore::Double > field_spw_noise_map_p;
	map< pair<casacore::Int,casacore::Int>,casacore::Bool > user_field_spw_noise_map_p;
	map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > field_spw_noise_histogram_sum_p;
	map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > field_spw_noise_histogram_sum_squares_p;
	map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > field_spw_noise_histogram_counts_p;

	// Spectral analysis
	casacore::Double scutof_p;
	map< pair<casacore::Int,casacore::Int>,casacore::Double > field_spw_scutof_map_p;
	map< pair<casacore::Int,casacore::Int>,casacore::Bool > user_field_spw_scutof_map_p;
	map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > field_spw_scutof_histogram_sum_p;
	map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > field_spw_scutof_histogram_sum_squares_p;
	map< pair<casacore::Int,casacore::Int>,vector<casacore::Double> > field_spw_scutof_histogram_counts_p;
};


} //# NAMESPACE CASA - END

#endif /* FlagAgentRFlag_H_ */

