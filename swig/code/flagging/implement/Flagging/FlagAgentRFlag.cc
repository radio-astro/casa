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
	thresholdRobust_p = vector<Double>(nIterationsRobust_p);
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
		*logger_p << logLevel_p << " doplot is " << doplot_p << LogIO::POST;
	}
	else
	{
		doplot_p = False;
	}

	// AIPS RFlag FPARM(1)
	exists = config.fieldNumber ("ntimesteps");
	if (exists >= 0)
	{
		nTimeSteps_p = config.asuInt("ntimesteps");
		*logger_p << logLevel_p << " ntimesteps is " << nTimeSteps_p << LogIO::POST;
	}
	else
	{
		nTimeSteps_p = 3;
	}

	// AIPS RFlag FPARM(3)/NOISE
	exists = config.fieldNumber ("noise");
	if (exists >= 0)
	{
		noise_p = config.asArrayDouble("noise");
		*logger_p << logLevel_p << " noise is " << noise_p << LogIO::POST;
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
		*logger_p << logLevel_p << " scutof is " << scutof_p << LogIO::POST;
	}
	else
	{
		IPosition shape(1);
		shape(0)=1;
		Array<Double> tmp(shape);
		tmp[0] = 1E6;
		scutof_p = tmp;
	}

	// AIPS RFlag FPARM(5)
	exists = config.fieldNumber ("spectralmax");
	if (exists >= 0)
	{
		spectralmax_p = config.asDouble("spectralmax");
		*logger_p << logLevel_p << " spectralmax is " << spectralmax_p << LogIO::POST;
	}
	else
	{
		spectralmax_p  = 1E6;
	}

	// AIPS RFlag FPARM(6)
	exists = config.fieldNumber ("spectralmin");
	if (exists >= 0)
	{
		spectralmin_p = config.asDouble("spectralmin");
		*logger_p << logLevel_p << " spectralmin is " << spectralmin_p << LogIO::POST;
	}
	else
	{
		spectralmin_p = 0;
	}

	// AIPS RFlag FPARM(9)
	exists = config.fieldNumber ("noisescale");
	if (exists >= 0)
	{
		noiseScale_p = config.asDouble("noisescale");
		*logger_p << logLevel_p << " noisescale is " << noiseScale_p << LogIO::POST;
	}
	else
	{
		noiseScale_p  = 5;
	}

	// AIPS RFlag FPARM(10)
	exists = config.fieldNumber ("scutofscale");
	if (exists >= 0)
	{
		scutofScale_p = config.asDouble("scutofscale");
		*logger_p << logLevel_p << " scutofscale is " << scutofScale_p << LogIO::POST;
	}
	else
	{
		scutofScale_p = 5;
	}

	return;
}

Double FlagAgentRFlag::mean(vector<Double> &data,vector<Double> &counts)
{
	Double sumAvg = 0;
	for (size_t index = 0; index < data.size();index++)
	{
		sumAvg += data[index]/counts[index];
	}
	return sumAvg/data.size();
}

Double FlagAgentRFlag::median(vector<Double> &data)
{
	Double med;
	sort(data.begin(),data.end());

	if (data.size() % 2 == 1)
	{
		med = data[(data.size()-1)/2];
	}
	else
	{
		med = 0.5*(data[data.size()/2] + data[(data.size()/2)-1]);
	}

	return med;
}

Double FlagAgentRFlag::computeThreshold(vector<Double> &data,vector<Double> &dataSquared,vector<Double> &counts)
{
/*
 *      DO 100 JI = 1,NI
         K = 0
         MEDR = 0.0
         MEDRR = 0.0
         DO 10 JC = 1,NC
            IF (RMSRMS(3,JC,JI).GE.1.0D0) THEN
               RMSRMS(1,JC,JI) = RMSRMS(1,JC,JI) / RMSRMS(3,JC,JI)
               RMSRMS(2,JC,JI) = RMSRMS(2,JC,JI) / RMSRMS(3,JC,JI) -
     *            RMSRMS(1,JC,JI) * RMSRMS(1,JC,JI)
               RMSRMS(2,JC,JI) = SQRT (MAX (0.0D0, RMSRMS(2,JC,JI)))
               K = K + 1
               VALUES(K) = RMSRMS(1,JC,JI)
               END IF
 10         CONTINUE
         IF (K.GT.0) THEN
            MEDR = MEDIAN (K, VALUES)
            DO 20 JC = 1,K
               VALUES(JC) = ABS (VALUES(JC)-MEDR)
 20            CONTINUE
            MEDRR = 1.4826 * MEDIAN (K, VALUES)
            END IF
         IF (SCALE.GT.0.0) XNOISE(JI) = SCALE * (MEDR + MEDRR)
 100     CONTINUE
C
 *
 */
	// Declare working variables
	Double avg,avgSquared,std;

	// Produce samples for median
	vector<Double> samplesForMedian(data.size(),0);
	for (size_t index = 0; index < data.size();index++)
	{
		avg = data[index]/counts[index];
		avgSquared = dataSquared[index]/counts[index];
		std = avgSquared - avg*avg;
		std = sqrt(std > 0?  std:0);
		samplesForMedian[index] = avg;
	}

	// Compute median
	Double med = median(samplesForMedian);

	// Produce samples for median absolute deviation
	vector<Double> samplesForMad(data.size(),0);
	for (size_t index = 0; index < data.size();index++)
	{
		samplesForMad[index] = abs(samplesForMedian[index] - med);
	}

	// Compute median absolute deviation
	Double mad = median(samplesForMad);

	// Return median of std plus mad of std
	/*
	cout << " med is:" 	<< 1000*med
						<< " mad is:" << 1000*mad
						<< " med + mad is:" << 1000*(med + mad)
						<< " med + 1.4826*mad is:" << 1000*(med + 1.4826*mad) << endl;
	*/

	return (med + 1.4826*mad);
}

FlagReport FlagAgentRFlag::getReport()
{
	FlagReport dispRep("list");

	FlagReport noiseStd = getReportCore(	spw_noise_histogram_sum_p,
											spw_noise_histogram_sum_squares_p,
											spw_noise_histogram_counts_p,
											"Time analysis",
											noiseScale_p);
    dispRep.addReport(noiseStd);
/*
	FlagReport scutofStd = getReportCore(	spw_scutof_histogram_sum_p,
											spw_scutof_histogram_sum_squares_p,
											spw_scutof_histogram_counts_p,
											"Spectral analysis",
											scutofScale_p);
    dispRep.addReport(scutofStd);
*/
	return dispRep;
}

FlagReport FlagAgentRFlag::getReportCore(	map< Int,vector<Double> > &data,
											map< Int,vector<Double> > &dataSquared,
											map< Int,vector<Double> > &counts,
											string label,
											uInt scale)
{
	// Set logger origin
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

    // Declare working variables
    Int current_spw;
    Double spwStd = 0;
    Double avg,sumSquare,variance = 0;
    FlagReport thresholdStd = FlagReport("plotline",agentName_p,label, "xaxis", "yaxis");

    // Extract data from all spws and put them in one single Array
    vector<Double> total_threshold;
    vector<Double> total_threshold_squared;
    vector<Double> total_threshold_counts;
    vector<Double> current_spw_threshold;
    vector<Double> current_spw_threshold_squared;
    vector<Double> current_spw_threshold_counts;
    vector<Float> total_threshold_spw_average;
    for (	map< Int,vector<Double> >::iterator spw_iter = data.begin();
    		spw_iter != data.end();
    		spw_iter++)
    {
    	current_spw = spw_iter->first;

    	current_spw_threshold = data[current_spw];
    	current_spw_threshold_squared = dataSquared[current_spw];
    	current_spw_threshold_counts = counts[current_spw];

    	total_threshold.insert(total_threshold.end(),current_spw_threshold.begin(),current_spw_threshold.end());
    	total_threshold_squared.insert(total_threshold_squared.end(),current_spw_threshold_squared.begin(),current_spw_threshold_squared.end());
    	total_threshold_counts.insert(total_threshold_counts.end(),current_spw_threshold_counts.begin(),current_spw_threshold_counts.end());

    	// Display average (over baeline/channels) std per spw
    	spwStd = scale*computeThreshold(current_spw_threshold,current_spw_threshold_squared,current_spw_threshold_counts);
    	*logger_p << LogIO::NORMAL << label.c_str() << " - Spw " << current_spw <<
    			" threshold (over baselines/timesteps) avg: " << spwStd << LogIO::POST;

    	vector<Float> aux(current_spw_threshold.size(),spwStd);
    	total_threshold_spw_average.insert(total_threshold_spw_average.end(),aux.begin(),aux.end());
    }

    // Copy values from std::vector to casa::vector
    Vector<Float> threshold_index(total_threshold_counts.size(),0);
    Vector<Float> threshold_avg(total_threshold_counts.size(),0);
    Vector<Float> threshold_up(total_threshold_counts.size(),0);
    Vector<Float> threshold_down(total_threshold_counts.size(),0);
    size_t idx = 0;
    for (vector<Double>::iterator iter = total_threshold.begin();iter != total_threshold.end();iter++)
    {
    	threshold_index(idx) = idx;
    	avg = total_threshold[idx]/total_threshold_counts[idx];
    	threshold_avg(idx) = avg;

    	sumSquare = total_threshold_squared[idx]/total_threshold_counts[idx];
    	variance = sqrt(sumSquare - avg*avg);
    	threshold_up(idx) = avg+variance;
    	threshold_down(idx) = avg-variance;
    	idx++;
    }

    thresholdStd.addData(threshold_index,threshold_avg,"threshold std (spw:timestep average over baselines)");
    thresholdStd.addData(threshold_index,threshold_up,"threshold std+var (spw:timestep average over baselines)");
    thresholdStd.addData(threshold_index,total_threshold_spw_average,"threshold std (spw average over baselines and timesteps)");
    thresholdStd.addData(threshold_index,threshold_down,"threshold std-var  (spw:timestep average over baselines)");

    return thresholdStd;
}

void FlagAgentRFlag::computeAntennaPairFlagsCore(	Int spw,
													Double noise,
													Double scutof,
													uInt timeStart,
													uInt timeStop,
													uInt centralTime,
													VisMapper &visibilities,
													FlagMapper &flags)
{
	// Get flag cube size
	Int nPols,nChannels,nTimesteps;
	visibilities.shape(nPols, nChannels, nTimesteps);

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
	Double deviationReal = 0;
	Double deviationImag = 0;

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

			for (uInt timestep_i=timeStart;timestep_i<=timeStop;timestep_i++)
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
	            	spw_noise_histogram_sum_squares_p[spw][chan_j] += StdTotal*StdTotal;
	            }
	            else
	            {
	            	if (StdTotal > noise)
	            	{
	            		for (uInt timestep_i=timeStart;timestep_i<=timeStop;timestep_i++)
	            		{
	            			/*
	            			cout 	<< "pol: " << pol_k
	            					<< " channel:" << chan_j
	            					<< " timeStart:" << timeStart
	            					<< " timeStop:" << timeStop
	            					<< " StdTotal:" << StdTotal
	            					<< " noise:" << noise << endl;
	            				*/
	            			flags.setModifiedFlags(pol_k,chan_j,timestep_i);
	            			visBufferFlags_p += 1;
	            		}
	            	}
	            }
			}
		}
	}

	// Spectral analysis: Fix timestep/polarization and compute stats with all channels
	for (uInt timestep_i=timeStart;timestep_i<=timeStop;timestep_i++)
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
						deviationReal = abs(visibility.real()-AverageReal);
						spw_scutof_histogram_counts_p[spw][timestep_i] += 1;
						spw_scutof_histogram_sum_p[spw][timestep_i] += deviationReal;
						spw_scutof_histogram_sum_squares_p[spw][timestep_i] += deviationReal*deviationReal;
					}

					if (AverageImag > 0)
					{
						deviationImag = abs(visibility.imag()-AverageImag);
		            	spw_scutof_histogram_counts_p[spw][timestep_i] += 1;
		            	spw_scutof_histogram_sum_p[spw][timestep_i] += deviationImag;
		            	spw_scutof_histogram_sum_squares_p[spw][timestep_i] += deviationImag*deviationImag;
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

	// Extend flags across polarizations (AIPS 'compress stokes')
	if (!doplot_p)
	{
		for (uInt timestep_i=timeStart;timestep_i<=timeStop;timestep_i++)
		{
			for (uInt chan_j=0;chan_j<nChannels;chan_j++)
			{
				for (uInt pol_k=0;pol_k<nPols;pol_k++)
				{
					if (flags.getModifiedFlags(pol_k,chan_j,timestep_i))
					{
						for (Int ineer_pol_k=0;ineer_pol_k<nPols;ineer_pol_k++)
						{
							if (!flags.getModifiedFlags(ineer_pol_k,chan_j,timestep_i))
							{
								flags.setModifiedFlags(ineer_pol_k,chan_j,timestep_i);
								visBufferFlags_p += 1;
							}
						}
						break;
					}
				}
			}
		}
	}

	return;
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
			spw_noise_histogram_sum_squares_p[spw] = vector<Double>(nChannels,0);
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
			spw_scutof_histogram_sum_squares_p[spw] = vector<Double>(nTimesteps,0);
		}
		else if (spw_scutof_histogram_sum_p[spw].size() < nTimesteps)
		{
			vector<Double> aux(nTimesteps-spw_scutof_histogram_sum_p[spw].size(),0);
			spw_scutof_histogram_sum_p[spw].insert(spw_scutof_histogram_sum_p[spw].end(),aux.begin(),aux.end());
			spw_scutof_histogram_counts_p[spw].insert(spw_scutof_histogram_counts_p[spw].end(),aux.begin(),aux.end());
			spw_scutof_histogram_sum_squares_p[spw].insert(spw_scutof_histogram_sum_squares_p[spw].end(),aux.begin(),aux.end());
		}
	}

	uInt effectiveNTimeSteps;
	if (nTimesteps > nTimeSteps_p)
	{
		effectiveNTimeSteps = nTimeSteps_p;
	}
	else
	{
		effectiveNTimeSteps = nTimesteps;
	}

	uInt effectiveNTimeStepsDelta = (effectiveNTimeSteps - 1)/2;

	// Beginning time range: Move only central point
	for (uInt timestep_i=0;timestep_i<effectiveNTimeStepsDelta;timestep_i++)
	{
		computeAntennaPairFlagsCore(spw,noise,scutof,0,effectiveNTimeSteps,timestep_i,visibilities,flags);
	}

	for (uInt timestep_i=effectiveNTimeStepsDelta;timestep_i<nTimesteps-effectiveNTimeStepsDelta;timestep_i++)
	{
		computeAntennaPairFlagsCore(spw,noise,scutof,timestep_i-effectiveNTimeStepsDelta,timestep_i+effectiveNTimeStepsDelta,timestep_i,visibilities,flags);
	}

	// End time range: Move only central point
	for (uInt timestep_i=nTimesteps-effectiveNTimeStepsDelta;timestep_i<nTimesteps;timestep_i++)
	{
		computeAntennaPairFlagsCore(spw,noise,scutof,nTimesteps-effectiveNTimeSteps,nTimesteps-1,timestep_i,visibilities,flags);
	}

	return false;
}

} //# NAMESPACE CASA - END


