//# FlagAgentRFlag.cc: This file contains the implementation of the FlagAgentRFlag class.
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

#include <flagging/Flagging/FlagAgentRFlag.h>

namespace casa { //# NAMESPACE CASA - BEGIN


FlagAgentRFlag::FlagAgentRFlag(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube, Bool flag):
		FlagAgentBase(dh,config,ANTENNA_PAIRS,writePrivateFlagCube,flag)
{
	setAgentParameters(config);

	// Request pre-loading spw
	flagDataHandler_p->preLoadColumn(VisBufferComponents::SpW);

	// Initialize parameters for robust stats (spectral analysis)
	nIterationsRobust_p = 12;
	thresholdRobust_p.resize(nIterationsRobust_p);
	thresholdRobust_p[0] = 6.0;
	thresholdRobust_p[1] = 5.0;
	thresholdRobust_p[2] = 4.0;
	thresholdRobust_p[3] = 3.6;
	thresholdRobust_p[4] = 3.2;
	thresholdRobust_p[5] = 3.0;
	thresholdRobust_p[6] = 2.7;
	thresholdRobust_p[7] = 2.6;
	thresholdRobust_p[8] = 2.5;
	thresholdRobust_p[9] = 2.5;
	thresholdRobust_p[10] = 2.5;
	thresholdRobust_p[11] = 3.5;
}

FlagAgentRFlag::~FlagAgentRFlag()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void FlagAgentRFlag::setAgentParameters(Record config)
{
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

	int exists;

	// AIPS DOPLOT
	exists = config.fieldNumber ("doplot");
	if (exists >= 0)
	{
		doplot_p = config.asBool("doplot");
	}
	else
	{
		doplot_p = False;
	}

	// AIPS RFlag FPARM(3)/NOISE
	exists = config.fieldNumber ("noise");
	if (exists >= 0)
	{
		noise_p = config.asArrayDouble("noise");
	}
	else
	{
		IPosition shape(1);
		shape(0)=1;
		Array<Double> tmp(shape);
		tmp[0] = 1E6;
		noise_p = tmp;
	}

	// AIPS RFlag FPARM(4)/SCUTOF
	exists = config.fieldNumber ("scutof");
	if (exists >= 0)
	{
		scutof_p = config.asArrayDouble("scutof");
	}
	else
	{
		IPosition shape(1);
		shape(0)=1;
		Array<Double> tmp(shape);
		tmp[0] = 1E6;
		scutof_p = tmp;
	}

	// AIPS RFlag FPARM(6)
	exists = config.fieldNumber ("spectralmin");
	if (exists >= 0)
	{
		spectralmin_p = config.asDouble("spectralmin");
	}
	else
	{
		spectralmin_p = 0;
	}

	// AIPS RFlag FPARM(5)
	exists = config.fieldNumber ("spectralmax");
	if (exists >= 0)
	{
		spectralmax_p = config.asDouble("spectralmax");
	}
	else
	{
		spectralmax_p  = 1E6;
	}

	return;
}

Double FlagAgentRFlag::average(vector<Double> &data,vector<Double> &counts)
{
	Double sumAvg = 0;
	for (size_t index = 0; index < data.size();index++)
	{
		sumAvg += data[index]/counts[index];
	}
	return sumAvg/data.size();
}

FlagReport FlagAgentRFlag::getReport()
{
	FlagReport dispRep("list");

    Int current_spw;
    Double spwAverage = 0;

    // Extract time analysis report
    FlagReport noiseStd = FlagReport("plotline",agentName_p,"Noise (time direction analysis) Std", "xaxis", "yaxis");

    // Extract data from all spws and put them in one single Array
    vector<Double> total_noise;
    vector<Double> total_noise_counts;
    vector<Double> current_spw_noise;
    vector<Double> current_spw_noise_counts;
    for (	map< Int,vector<Double> >::iterator spw_iter = spw_noise_histogram_sum_p.begin();
    		spw_iter != spw_noise_histogram_sum_p.end();
    		spw_iter++)
    {
    	current_spw = spw_iter->first;
    	current_spw_noise = spw_noise_histogram_sum_p[current_spw];
    	current_spw_noise_counts = spw_noise_histogram_counts_p[current_spw];
    	total_noise.insert(total_noise.end(),current_spw_noise.begin(),current_spw_noise.end());
    	total_noise_counts.insert(total_noise_counts.end(),current_spw_noise_counts.begin(),current_spw_noise_counts.end());

    	// Display average (over baeline/channels) std per spw
    	spwAverage = average(current_spw_noise,current_spw_noise_counts);
    	*logger_p << LogIO::NORMAL << " Time analysis - Spw " << current_spw <<
    			" average (over baseline/channels) std: " << spwAverage << LogIO::POST;
    }

    // Copy values from std::vector to casa::Array
    Vector<Float> noise_index(total_noise_counts.size(),0);
    Vector<Float> noise(total_noise_counts.size(),0);
    Vector<Float> noise_counts(total_noise_counts.size(),0);
    Vector<Float> noise_avg(total_noise_counts.size(),0);
    size_t idx = 0;
    for (vector<Double>::iterator iter = total_noise.begin();iter != total_noise.end();iter++)
    {
    	noise_index(idx) = idx;
    	noise(idx) = total_noise_counts[idx];
    	noise_counts(idx) = total_noise_counts[idx];
    	noise_avg(idx) = total_noise[idx]/total_noise_counts[idx];
    	idx++;
    }

    noiseStd.addData(noise_index,noise_avg,"Noise std (average over baselines)");
    dispRep.addReport(noiseStd);

    // Extract spectral analysis report
    FlagReport scutofStd = FlagReport("plotline",agentName_p,"Spectral (frequency direction analysis) Deviation", "xaxis", "yaxis");

    // Extract data from all spws and put them in one single Array
    vector<Double> total_scutof;
    vector<Double> total_scutof_counts;
    vector<Double> current_spw_scutof;
    vector<Double> current_spw_scutof_counts;
    for (	map< Int,vector<Double> >::iterator spw_iter = spw_scutof_histogram_sum_p.begin();
    		spw_iter != spw_scutof_histogram_sum_p.end();
    		spw_iter++)
    {
    	current_spw = spw_iter->first;
    	current_spw_scutof = spw_scutof_histogram_sum_p[current_spw];
    	current_spw_scutof_counts = spw_scutof_histogram_counts_p[current_spw];
    	total_scutof.insert(total_scutof.end(),current_spw_scutof.begin(),current_spw_scutof.end());
    	total_scutof_counts.insert(total_scutof_counts.end(),current_spw_scutof_counts.begin(),current_spw_scutof_counts.end());

    	// Display average (over baeline/channels) std per spw
    	spwAverage = average(current_spw_scutof,current_spw_scutof_counts);
    	*logger_p << LogIO::NORMAL << " Spectral analysis - Spw " << current_spw <<
    			" average (over baselines/timesteps) avg: " << spwAverage << LogIO::POST;
    }

    // Copy values from std::vector to casa::Array
    Vector<Float> scutof_index(total_scutof_counts.size(),0);
    Vector<Float> scutof(total_scutof_counts.size(),0);
    Vector<Float> scutof_counts(total_scutof_counts.size(),0);
    Vector<Float> scutof_avg(total_scutof_counts.size(),0);
    idx = 0;
    for (vector<Double>::iterator iter = total_scutof.begin();iter != total_scutof.end();iter++)
    {
    	scutof_index(idx) = idx;
    	scutof(idx) = total_scutof_counts[idx];
    	scutof_counts(idx) = total_scutof_counts[idx];
    	scutof_avg(idx) = total_scutof[idx]/total_scutof_counts[idx];
    	idx++;
    }

    scutofStd.addData(scutof_index,scutof_avg,"scutof avg (average over baselines)");
    dispRep.addReport(scutofStd);

	return dispRep;
}

bool
FlagAgentRFlag::computeAntennaPairFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows)
{
	// Set logger origin
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

	// Get flag cube size
	Int nPols,nChannels,nTimesteps;
	visibilities.shape(nPols, nChannels, nTimesteps);

	// Get current chunk spw
	Int spw = visBuffer.spectralWindow();

	// Get noise level
	Double noise;
	// Only one value for all spws
	if (noise_p.size() == 1)
	{
		Bool deleteIt = False;
		noise = noise_p.getStorage(deleteIt)[0];
	}
	// One value for each spw
	else if (noise_p.size() > 1)
	{
		// Check if we already have the noise corresponding to this spw
		if (spw_noise_map_p.find(spw) != spw_noise_map_p.end())
		{
			noise = spw_noise_map_p.at(spw);
		}
		// Otherwise extract next noise value from input noise array
		else
		{
			Bool deleteIt = False;
			noise = noise_p.getStorage(deleteIt)[spw_noise_map_p.size()];
			// And add spw-noise par to map
			spw_noise_map_p[spw] = noise;
		}
	}

	// Produce time analysis histogram for each spw
	if (doplot_p)
	{
		if (spw_noise_histogram_sum_p.find(spw) == spw_noise_histogram_sum_p.end())
		{
			spw_noise_histogram_sum_p[spw] = vector<Double>(nChannels,0);
			spw_noise_histogram_counts_p[spw] = vector<Double>(nChannels,0);
		}
	}

	// Get cutof level
	Double scutof;
	// Only one value for all spws
	if (scutof_p.size() == 1)
	{
		Bool deleteIt = False;
		scutof = scutof_p.getStorage(deleteIt)[0];
	}
	// One value for each spw
	else if (scutof_p.size() > 1)
	{
		// Check if we already have the scutof corresponding to this spw
		if (spw_scutof_map_p.find(spw) != spw_scutof_map_p.end())
		{
			scutof = spw_scutof_map_p.at(spw);
		}
		// Otherwise extract next scutof value from input scutof array
		else
		{
			Bool deleteIt = False;
			scutof = scutof_p.getStorage(deleteIt)[spw_scutof_map_p.size()];
			// And add spw-scutof par to map
			spw_scutof_map_p[spw] = scutof;
		}
	}

	// Produce spectral analysis histogram for each spw
	if (doplot_p)
	{
		if (spw_scutof_histogram_sum_p.find(spw) == spw_scutof_histogram_sum_p.end())
		{
			spw_scutof_histogram_sum_p[spw] = vector<Double>(nTimesteps,0);
			spw_scutof_histogram_counts_p[spw] = vector<Double>(nTimesteps,0);
		}
	}

	// Declare variables
	Complex visibility;
	Double SumWeight = 0;
	Double SumWeightReal = 0;
	Double SumWeightImag = 0;
	Double StdTotal = 0;
    Double SumReal = 0;
	Double SumRealSquare = 0;
	Double AverageReal = 0;
	Double StdReal = 0;
	Double SumImag = 0;
	Double SumImagSquare = 0;
	Double AverageImag = 0;
	Double StdImag = 0;

	// Time-Direction analysis: Fix channel/polarization and compute stats with all time-steps
	// NOTE: It is better to operate in channel/polarization sequence for data contiguity
	for (uInt chan_j=0;chan_j<nChannels;chan_j++)
	{
		// Compute variance
		for (uInt pol_k=0;pol_k<nPols;pol_k++)
		{
			SumWeight = 0;
			StdTotal = 0;
		    SumReal = 0;
			SumRealSquare = 0;
			AverageReal = 0;
			StdReal = 0;
			SumImag = 0;
			SumImagSquare = 0;
			AverageImag = 0;
			StdImag = 0;

			for (uInt timestep_i=0;timestep_i<nTimesteps;timestep_i++)
			{
				// Ignore data point if it is already flagged
				// NOTE: In our case visibilities come w/o weights, so we check vs flags instead
				if (flags.getModifiedFlags(pol_k,chan_j,timestep_i)) continue;

				visibility = visibilities.correlationProduct(pol_k,chan_j,timestep_i);
				SumWeight += 1;
				SumReal += visibility.real();
				SumRealSquare += visibility.real()*visibility.real();
				SumImag += visibility.imag();
				SumImagSquare += visibility.imag()*visibility.imag();
			}

			// Now flag all timesteps if variance is greater than threshold
			// NOTE: In our case, SumWeight is zero when all the data is already flagged so we don't need to re-flag it
			if (SumWeight > 0)
			{
	            AverageReal = SumReal / SumWeight;
	            SumRealSquare = SumRealSquare / SumWeight;
	            StdReal = SumRealSquare - AverageReal * AverageReal;

	            AverageImag = SumImag / SumWeight;
	            SumImagSquare = SumImagSquare / SumWeight;
	            StdImag = SumImagSquare - AverageImag * AverageImag;

	            // NOTE: It it not necessary to extract the square root if then we are going to pow2
	            // StdReal = sqrt (StdReal > 0?  StdReal:0);
	            // StdImag = sqrt (StdImag > 0?  StdImag:0);
	            // StdTotal = sqrt (StdReal*StdReal + StdImag*StdImag);
	            StdReal = StdReal > 0?  StdReal:0;
	            StdImag = StdImag > 0?  StdImag:0;
	            StdTotal = sqrt(StdReal + StdImag);

	            // Apply flags or generate histogram?
	            // NOTE: AIPS RFlag has the previous code duplicated in two separated
	            // routines, but I don't see a reason to do this, performance-wise
	            if (doplot_p)
	            {
	            	spw_noise_histogram_counts_p[spw][chan_j] += 1;
	            	spw_noise_histogram_sum_p[spw][chan_j] += StdTotal;
	            }
	            else
	            {
	            	if (StdTotal > noise)
	            	{
	            		for (uInt timestep_i=0;timestep_i<nTimesteps;timestep_i++)
	            		{
	            			flags.setModifiedFlags(pol_k,chan_j,timestep_i);
	            			visBufferFlags_p += 1;
	            		}
	            	}
	            }
			}
		}
	}

	// Spectral analysis: Fix timestep/polarization and compute stats with all channels
	for (uInt timestep_i=0;timestep_i<nTimesteps;timestep_i++)
	{
		for (uInt pol_k=0;pol_k<nPols;pol_k++)
		{
			// NOTE: To apply the robust coefficients we need some initial values of avg/std
			//       In AIPS they simply use Std=1000 for the first iteration
			//       I'm not very convinced with this but if we have to cross-validate...
			AverageReal = 0;
			AverageImag = 0;
			StdReal = 1000.0;
			StdImag = 1000.0;

			for (uInt robustIter=0;robustIter<nIterationsRobust_p;robustIter++)
			{
				SumWeightReal = 0;
				SumWeightImag = 0;
			    SumReal = 0;
				SumRealSquare = 0;
				SumImag = 0;
				SumImagSquare = 0;

				for (uInt chan_j=0;chan_j<nChannels;chan_j++)
				{
					// Ignore data point if it is already flagged or weight is <= 0
					// NOTE: In our case visibilities come w/o weights, so we check only vs flags
					if (flags.getModifiedFlags(pol_k,chan_j,timestep_i)) continue;

					visibility = visibilities.correlationProduct(pol_k,chan_j,timestep_i);
					if (abs(visibility.real()-AverageReal)<thresholdRobust_p[robustIter]*StdReal)
					{
						SumWeightReal += 1;
						SumReal += visibility.real();
						SumRealSquare += visibility.real()*visibility.real();
					}

					if (abs(visibility.imag()-AverageImag)<thresholdRobust_p[robustIter]*StdImag)
					{
						SumWeightImag += 1;
						SumImag += visibility.imag();
						SumImagSquare += visibility.imag()*visibility.imag();
					}
				}

				if (SumWeightReal > 0)
				{
					AverageReal = SumReal / SumWeightReal;
					SumRealSquare = SumRealSquare / SumWeightReal;
					StdReal = SumRealSquare - AverageReal * AverageReal;
		            StdReal = sqrt(StdReal > 0?  StdReal:0);
				}

				if (SumWeightImag > 0)
				{
					AverageImag = SumImag / SumWeightImag;
	            	SumImagSquare = SumImagSquare / SumWeightImag;
	            	StdImag = SumImagSquare - AverageImag * AverageImag;
		            StdImag = sqrt(StdImag > 0?  StdImag:0);
				}
			}

            if (doplot_p)
            {
        		for (uInt chan_j=0;chan_j<nChannels;chan_j++)
        		{
					// Ignore data point if it is already flagged
					// NOTE: In our case visibilities come w/o weights, so we check vs flags instead
					if (flags.getModifiedFlags(pol_k,chan_j,timestep_i)) continue;

					visibility = visibilities.correlationProduct(pol_k,chan_j,timestep_i);

					if (AverageReal > 0)
					{
		            	spw_scutof_histogram_counts_p[spw][timestep_i] += 1;
		            	spw_scutof_histogram_sum_p[spw][timestep_i] += abs(visibility.real()-AverageReal);
					}

					if (AverageImag > 0)
					{
		            	spw_scutof_histogram_counts_p[spw][timestep_i] += 1;
		            	spw_scutof_histogram_sum_p[spw][timestep_i] += abs(visibility.imag()-AverageImag);
					}
        		}
            }
            else
            {
            	// Flag all channels?
            	if (	(StdReal > spectralmax_p) or
            			(StdImag > spectralmax_p) or
            			(StdReal < spectralmin_p) or
            			(StdImag < spectralmin_p)		)
            	{
            		for (uInt chan_j=0;chan_j<nChannels;chan_j++)
            		{
            			flags.setModifiedFlags(pol_k,chan_j,timestep_i);
            			visBufferFlags_p += 1;
            		}
            	}
            	// Check each channel separately vs the scutof level
            	else
            	{
            		for (uInt chan_j=0;chan_j<nChannels;chan_j++)
            		{
    					visibility = visibilities.correlationProduct(pol_k,chan_j,timestep_i);
    					if (	(abs(visibility.real()-AverageReal)>scutof) or
    							(abs(visibility.imag()-AverageImag)>scutof)	)
    					{
                			flags.setModifiedFlags(pol_k,chan_j,timestep_i);
                			visBufferFlags_p += 1;
    					}
            		}
            	}
            }
		}
	}

	return false;
}

} //# NAMESPACE CASA - END


