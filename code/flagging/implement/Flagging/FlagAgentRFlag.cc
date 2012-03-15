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

	// AIPS RFlag FPARM(1)
	exists = config.fieldNumber ("winsize");
	if (exists >= 0)
	{
	        if( config.type(exists) != TpInt )
	        {
			 throw( AipsError ( "Parameter 'winsize' must be of type 'Int'" ) );
	        }
		
		nTimeSteps_p = config.asuInt("winsize");
	}
	else
	{
		nTimeSteps_p = 3;
	}
	
	*logger_p << logLevel_p << " winsize is " << nTimeSteps_p << LogIO::POST;

	// AIPS RFlag FPARM(5)
	exists = config.fieldNumber ("spectralmax");
	if (exists >= 0)
	{
	        if( config.type(exists) != TpDouble && config.type(exists) != TpFloat  && config.type(exists) != TpInt)
	        {
			 throw( AipsError ( "Parameter 'spectralmax' must be of type 'double'" ) );
	        }
		
		spectralmax_p = config.asDouble("spectralmax");
	}
	else
	{
		spectralmax_p  = 1E6;
	}
	
	*logger_p << logLevel_p << " spectralmax is " << spectralmax_p << LogIO::POST;

	// AIPS RFlag FPARM(6)
	exists = config.fieldNumber ("spectralmin");
	if (exists >= 0)
	{
	        if( config.type(exists) != TpDouble && config.type(exists) != TpFloat && config.type(exists) != TpInt )
	        {
			 throw( AipsError ( "Parameter 'spectralmin' must be of type 'double'" ) );
	        }
		
		spectralmin_p = config.asDouble("spectralmin");
	}
	else
	{
		spectralmin_p = 0;
	}

	*logger_p << logLevel_p << " spectralmin is " << spectralmin_p << LogIO::POST;

	// AIPS RFlag FPARM(9)
	exists = config.fieldNumber ("timedevscale");
	if (exists >= 0)
	{
	        if( config.type(exists) != TpDouble && config.type(exists) != TpFloat  && config.type(exists) != TpInt)
	        {
			 throw( AipsError ( "Parameter 'timedevscale' must be of type 'double'" ) );
	        }
		
		noiseScale_p = config.asDouble("timedevscale");
	}
	else
	{
		noiseScale_p  = 5;
	}

	*logger_p << logLevel_p << " timedevscale is " << noiseScale_p << LogIO::POST;

	// AIPS RFlag FPARM(10)
	exists = config.fieldNumber ("freqdevscale");
	if (exists >= 0)
	{
	        if( config.type(exists) != TpDouble && config.type(exists) != TpFloat  && config.type(exists) != TpInt )
	        {
		         throw( AipsError ( "Parameter 'freqdevscale' must be of type 'double'" ) );
	        }
		
		scutofScale_p = config.asDouble("freqdevscale");
	}
	else
	{
		scutofScale_p = 5;
	}

	*logger_p << logLevel_p << " freqdevscale is " << scutofScale_p << LogIO::POST;

	// Determine if we have to apply the flags in the modified flag cube
	String display("none");
	exists = config.fieldNumber ("display");
	if (exists >= 0)
	{
	        if( config.type(exists) != TpString )
	        {
			 throw( AipsError ( "Parameter 'display' must be of type 'string'" ) );
	        }
		
		display = config.asString("display");
	}

	Bool writeflags(True);
	exists = config.fieldNumber ("writeflags");
	if (exists >= 0)
	{
	        if( config.type(exists) != TpBool )
	        {
			 throw( AipsError ( "Parameter 'writeflags' must be of type 'bool'" ) );
	        }
		
		writeflags = config.asBool("writeflags");
	}

	if( (writeflags == True) or (display == String("data")) or (display == String("both")) )
	{
		doflag_p = true;
		*logger_p << LogIO::DEBUG1 << " (writeflags,display)=(" <<  writeflags << "," << display << "), will apply flags on modified flag cube " << LogIO::POST;
	}
	else
	{
		doflag_p = false;
	}

	// Determine if we have to generate plot reports.
	if ( (display == String("report")) or (display == String("both")) )
	{
		doplot_p = true;
	}
	else
	{
	        doplot_p = false;
	}

	// timedev - Matrix for time analysis deviation thresholds - (old AIPS RFlag FPARM(3)/NOISE)
	noise_p = 0;
	exists = config.fieldNumber ("timedev");
	if (exists >= 0)
	{
	  if ( config.type( exists ) == casa::TpFloat ||  config.type( exists ) == casa::TpDouble || config.type(exists) == casa::TpInt )
		{
			noise_p = config.asDouble("timedev");
			*logger_p << logLevel_p << " timedev (same for all fields and spws) is " << noise_p << LogIO::POST;
		}
		else if( config.type(exists) == casa::TpArrayDouble || config.type(exists) == casa::TpArrayFloat || config.type(exists) == casa::TpArrayInt)
		{
			Matrix<Double> timedev = config.asArrayDouble( RecordFieldId("timedev") );
			if(timedev.ncolumn()==3)
			{
				*logger_p << logLevel_p << " timedev [field,spw,dev] is " << timedev << LogIO::POST;

			    IPosition shape = timedev.shape();
			    uInt nDevs = shape[0];
			    for(uInt dev_i=0;dev_i<nDevs;dev_i++)
			    {
			    	pair<Int,Int> field_spw = std::make_pair(timedev(dev_i,0),timedev(dev_i,1));
			    	field_spw_noise_map_p[field_spw] = timedev(dev_i,2);
			    	user_field_spw_noise_map_p[field_spw] = True;
			    	*logger_p << LogIO::DEBUG1 << "freqdev matrix - field=" << timedev(dev_i,0) << " spw=" << timedev(dev_i,1) << " dev=" << timedev(dev_i,2) << LogIO::POST;
			    }
			}
			else
			{
			  throw( AipsError( " Matrix for time analysis deviation thresholds (timedev) must have 3 columns [[field,spw,dev]]. Set to [] to use automatically-computed thresholds." ) );
			}
		}
		else
		{
		        *logger_p << logLevel_p << "Using automatically computed values for timedev" << LogIO::POST;
		}
	}
	else
	{
	        *logger_p << logLevel_p << "Using automatically computed values for timedev" << LogIO::POST;
		// noise_p initialized to 0 above.
	}


	// freqdev - Matrix for time analysis deviation thresholds (freqdev) - (old AIPS RFlag FPARM(4)/SCUTOF)
	scutof_p = 0;
	exists = config.fieldNumber ("freqdev");
	if (exists >= 0)
	{
		if ( config.type( exists ) == casa::TpFloat ||  config.type( exists ) == casa::TpDouble  || config.type(exists) == casa::TpInt )
		{
			scutof_p = config.asDouble("freqdev");
			*logger_p << logLevel_p << " freqdev (same for all fields and spws) is " << scutof_p << LogIO::POST;
		}
		else if( config.type(exists) == casa::TpArrayDouble || config.type(exists) == casa::TpArrayFloat || config.type(exists) == casa::TpArrayInt)
		{
			Matrix<Double> freqdev = config.asArrayDouble( RecordFieldId("freqdev") );
			if(freqdev.ncolumn()==3)
			{
				*logger_p << logLevel_p << " freqdev [field,spw,dev] is " << freqdev << LogIO::POST;

			    IPosition shape = freqdev.shape();
			    uInt nDevs = shape[0];
			    for(uInt dev_i=0;dev_i<nDevs;dev_i++)
			    {
			    	pair<Int,Int> field_spw = std::make_pair(freqdev(dev_i,0),freqdev(dev_i,1));
			    	field_spw_scutof_map_p[field_spw] = freqdev(dev_i,2);
			    	user_field_spw_scutof_map_p[field_spw] = True;
			    	*logger_p << LogIO::DEBUG1 << "freqdev matrix - field=" << freqdev(dev_i,0) << " spw=" << freqdev(dev_i,1) << " dev=" << freqdev(dev_i,2) << endl;
			    }
			}
			else
			{
			  throw( AipsError( " Matrix for spectral analysis deviation thresholds (freqdev) must have 3 columns [[field,spw,dev]]. Set to [] to use automatically-computed thresholds." ) );
			}
		}
		else
		{
		        *logger_p << logLevel_p << "Using automatically computed values for freqdev" << LogIO::POST;
		}
	}
	else
	{
	        *logger_p << logLevel_p << "Using automatically computed values for freqdev" << LogIO::POST;
		// scutof initialized to zero above.
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
	Double med,medPoint;
	vector<Double> datacopy = data;
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
	// Declare working variables
	Double avg,avgSquared,std;

	// Produce samples for median
	vector<Double> samplesForMedian(data.size(),0);
	for (size_t index = 0; index < data.size();index++)
	{
		avg = data[index]/counts[index];
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

	return (med + 1.4826*mad);
}

FlagReport FlagAgentRFlag::getReport()
{
	FlagReport totalRep(String("list"),agentName_p);

	if ((doflag_p==false))
	{
		// Plot reports (should be returned if params were calculated and display is activated)
		FlagReport noiseStd = getReportCore(	field_spw_noise_histogram_sum_p,
												field_spw_noise_histogram_sum_squares_p,
												field_spw_noise_histogram_counts_p,
												field_spw_noise_map_p,
												"Time analysis",
												noiseScale_p);
		if(doplot_p==true)
		{
		         totalRep.addReport(noiseStd);
		}

		FlagReport scutofStd = getReportCore(	field_spw_scutof_histogram_sum_p,
												field_spw_scutof_histogram_sum_squares_p,
												field_spw_scutof_histogram_counts_p,
												field_spw_scutof_map_p,
												"Spectral analysis",
												scutofScale_p);
		if(doplot_p==true)
		{
		         totalRep.addReport(scutofStd);
		}

		// Threshold reports (should be returned if params were calculated)
		Record threshList;
		Int nEntries = field_spw_noise_map_p.size();
		Matrix<Double> timedev(nEntries,3), freqdev(nEntries,3);
		Int threshCount = 0;
		pair<Int,Int> field_spw;
		for (	map< pair<Int,Int>,Double >::iterator spw_field_iter = field_spw_noise_map_p.begin();
				spw_field_iter != field_spw_noise_map_p.end();
				spw_field_iter++)
		{
			field_spw = spw_field_iter->first;

			timedev(threshCount,0) = field_spw.first;
			timedev(threshCount,1) = field_spw.second;
			timedev(threshCount,2) = field_spw_noise_map_p[field_spw];
			freqdev(threshCount,0) = field_spw.first;
			freqdev(threshCount,1) = field_spw.second;
			freqdev(threshCount,2) = field_spw_scutof_map_p[field_spw];

			threshCount++;
		}
		threshList.define( RecordFieldId("timedev") , timedev );
		threshList.define( RecordFieldId("freqdev") , freqdev );

		FlagReport returnThresh("rflag",agentName_p, threshList);
		totalRep.addReport(returnThresh);
	}


	return totalRep;
}


FlagReport FlagAgentRFlag::getReportCore(	map< pair<Int,Int>,vector<Double> > &data,
											map< pair<Int,Int>,vector<Double> > &dataSquared,
											map< pair<Int,Int>,vector<Double> > &counts,
											map< pair<Int,Int>,Double > &threshold,
											string label,
											Double scale)
{
	// Set logger origin
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

    // Declare working variables
    pair<Int,Int> current_field_spw;
    Double spwStd = 0;
    Double avg,sumSquare,variance = 0;
    FlagReport thresholdStd = FlagReport("plotpoints",agentName_p,label, "xaxis", "yaxis");

    // Extract data from all spws and put them in one single Array
    vector<Double> total_threshold;
    vector<Double> total_threshold_squared;
    vector<Double> total_threshold_counts;
    vector<Double> current_spw_threshold;
    vector<Double> current_spw_threshold_squared;
    vector<Double> current_spw_threshold_counts;
    vector<Float> total_threshold_spw_average;
    for (	map< pair<Int,Int>,vector<Double> >::iterator spw_field_iter = data.begin();
    		spw_field_iter != data.end();
    		spw_field_iter++)
    {
    	current_field_spw = spw_field_iter->first;

    	current_spw_threshold = data[current_field_spw];
    	current_spw_threshold_squared = dataSquared[current_field_spw];
    	current_spw_threshold_counts = counts[current_field_spw];

    	total_threshold.insert(total_threshold.end(),current_spw_threshold.begin(),current_spw_threshold.end());
    	total_threshold_squared.insert(total_threshold_squared.end(),current_spw_threshold_squared.begin(),current_spw_threshold_squared.end());
    	total_threshold_counts.insert(total_threshold_counts.end(),current_spw_threshold_counts.begin(),current_spw_threshold_counts.end());

    	// Display average (over baseline/channels) std per spw
    	spwStd = scale*computeThreshold(current_spw_threshold,current_spw_threshold_squared,current_spw_threshold_counts);
    	*logger_p << logLevel_p << label.c_str() << " - Field " << current_field_spw.first << " - Spw " << current_field_spw.second <<
    			" threshold (over baselines/timesteps) avg: " << spwStd << LogIO::POST;
    	threshold[current_field_spw] = spwStd;

    	vector<Float> aux(current_spw_threshold.size(),spwStd);
    	total_threshold_spw_average.insert(total_threshold_spw_average.end(),aux.begin(),aux.end());
    }

    // Copy values from std::vector to casa::vector
    Vector<Float> threshold_index(total_threshold_counts.size(),0);
    Vector<Float> threshold_avg(total_threshold_counts.size(),0);
    Vector<Float> threshold_up(total_threshold_counts.size(),0);
    Vector<Float> threshold_down(total_threshold_counts.size(),0);
    Vector<Float> threshold_variance(total_threshold_counts.size(),0); // New
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
    	threshold_variance(idx) = variance; // New
    	idx++;
    }

    // Plot the scaled threshold
    thresholdStd.addData("line", threshold_index,total_threshold_spw_average,"",Vector<Float>(),"rflag threshold");

    // OPTION 1 for mean/rms : Scatter Plot with vertical Error-Bars (pretty, but slow)
    // thresholdStd.addData("scatter", threshold_index, threshold_avg, "bar", threshold_variance, "median deviation and variance");

    // OPTION 2 for mean/rms : "avg" is a scatter plot, "up" and "down" are lines (not so pretty, but fast).
    thresholdStd.addData("line",threshold_index,threshold_up,"",Vector<Float>(),"threshold std+var (field-spw:timestep average over baselines)");
    thresholdStd.addData("scatter",threshold_index,threshold_avg,"",Vector<Float>(),"threshold std (field-spw average over baselines and timesteps)");
    thresholdStd.addData("line", threshold_index,threshold_down,"",Vector<Float>(),"threshold std-var  (field-spw:timestep average over baselines)");
    

    return thresholdStd;
}

void FlagAgentRFlag::computeAntennaPairFlagsCore(	pair<Int,Int> spw_field,
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
	if ( (noise == 0) or ((noise > 0) and (doflag_p == true) and (prepass_p == false)) )
	{
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
					if (flags.getOriginalFlags(pol_k,chan_j,timestep_i)) continue;

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
	            	if (noise==0)
	            	{
	            		field_spw_noise_histogram_counts_p[spw_field][chan_j] += 1;
	            		field_spw_noise_histogram_sum_p[spw_field][chan_j]  += StdTotal;
	            		field_spw_noise_histogram_sum_squares_p[spw_field][chan_j]  += StdTotal*StdTotal;
	            	}
	            	else if (StdTotal > noise)
	            	{
	            		for (uInt timestep_i=timeStart;timestep_i<=timeStop;timestep_i++)
	            		{
							if (!flags.getModifiedFlags(pol_k,chan_j,timestep_i))
							{
								flags.setModifiedFlags(pol_k,chan_j,timestep_i);
								visBufferFlags_p += 1;
							}
	            		}
	            	}
				}
			}
		}
	}

	// Spectral analysis: Fix timestep/polarization and compute stats with all channels
	if ( (scutof == 0) or ((scutof > 0) and (doflag_p == true) and (prepass_p == false)) )
	{
		for (uInt timestep_i=centralTime;timestep_i<=centralTime;timestep_i++)
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
						if (flags.getOriginalFlags(pol_k,chan_j,timestep_i)) continue;

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

				if (scutof==0)
				{
					for (uInt chan_j=0;chan_j<nChannels;chan_j++)
					{
						// Ignore data point if it is already flagged
						// NOTE: In our case visibilities come w/o weights, so we check vs flags instead
						if (flags.getOriginalFlags(pol_k,chan_j,timestep_i)) continue;

						visibility = visibilities.correlationProduct(pol_k,chan_j,timestep_i);

						if (SumWeightReal > 0)
						{
							deviationReal = abs(visibility.real()-AverageReal);
							field_spw_scutof_histogram_counts_p[spw_field][chan_j]  += 1;
							field_spw_scutof_histogram_sum_p[spw_field][chan_j]  += deviationReal;
							field_spw_scutof_histogram_sum_squares_p[spw_field][chan_j]  += deviationReal*deviationReal;
						}

						if (SumWeightImag > 0)
						{
							deviationImag = abs(visibility.imag()-AverageImag);
							field_spw_scutof_histogram_counts_p[spw_field][chan_j]  += 1;
							field_spw_scutof_histogram_sum_p[spw_field][chan_j]  += deviationImag;
							field_spw_scutof_histogram_sum_squares_p[spw_field][chan_j]  += deviationImag*deviationImag;
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
							if (!flags.getModifiedFlags(pol_k,chan_j,timestep_i))
							{
								flags.setModifiedFlags(pol_k,chan_j,timestep_i);
								visBufferFlags_p += 1;
							}
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
								if (!flags.getModifiedFlags(pol_k,chan_j,timestep_i))
								{
									flags.setModifiedFlags(pol_k,chan_j,timestep_i);
									visBufferFlags_p += 1;
								}
							}
						}
					}
				}
			}
		}
	}

	/*
	// Extend flags across polarizations (AIPS 'compress stokes')
	if ( 	(doflag_p == true) and (prepass_p == false) and
			(noise > 0) or (scutof > 0)						 )
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
	*/

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

	// Make field-spw pair
	Int field = visBuffer.fieldId();
	Int spw = visBuffer.spectralWindow();
	pair<Int,Int> field_spw = std::make_pair(field,spw);

	// Get noise and scutoff levels
	Double noise = 0;
	if (field_spw_noise_map_p.find(field_spw) != field_spw_noise_map_p.end())
	{
		noise = field_spw_noise_map_p[field_spw];
	}
	else if (noise_p)
	{
		noise = noise_p;
	}
	else if (field_spw_noise_histogram_sum_p.find(field_spw) == field_spw_noise_histogram_sum_p.end())
	{
		field_spw_noise_histogram_sum_p[field_spw] = vector<Double>(nChannels,0);
		field_spw_noise_histogram_counts_p[field_spw] = vector<Double>(nChannels,0);
		field_spw_noise_histogram_sum_squares_p[field_spw] = vector<Double>(nChannels,0);
		if (doflag_p) prepass_p = true;
	}

	// Get cutoff level
	Double scutof = 0;
	if (field_spw_scutof_map_p.find(field_spw) != field_spw_scutof_map_p.end())
	{
		scutof = field_spw_scutof_map_p[field_spw];
	}
	else if (scutof_p)
	{
		scutof = scutof_p;
	}
	else if (field_spw_scutof_histogram_sum_p.find(field_spw) == field_spw_scutof_histogram_sum_p.end())
	{
		field_spw_scutof_histogram_sum_p[field_spw] = vector<Double>(nChannels,0);
		field_spw_scutof_histogram_counts_p[field_spw] = vector<Double>(nChannels,0);
		field_spw_scutof_histogram_sum_squares_p[field_spw] = vector<Double>(nChannels,0);
		if (doflag_p) prepass_p = true;
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

	// Beginning time range: Move only central point (only for spectral analysis)
	// We set start/stop time with decreasing values to deactivate time analysis
	for (uInt timestep_i=0;timestep_i<effectiveNTimeStepsDelta;timestep_i++)
	{
		// computeAntennaPairFlagsCore(field_spw,scutof,0,effectiveNTimeSteps,timestep_i,visibilities,flags);
		computeAntennaPairFlagsCore(field_spw,noise,scutof,-1,-2,timestep_i,visibilities,flags);
	}

	for (uInt timestep_i=effectiveNTimeStepsDelta;timestep_i<nTimesteps-effectiveNTimeStepsDelta;timestep_i++)
	{
		computeAntennaPairFlagsCore(field_spw,noise,scutof,timestep_i-effectiveNTimeStepsDelta,timestep_i+effectiveNTimeStepsDelta,timestep_i,visibilities,flags);
	}

	// End time range: Move only central point (only for spectral analysis)
	// We set start/stop time with decreasing values to deactivate time analysis
	for (uInt timestep_i=nTimesteps-effectiveNTimeStepsDelta;timestep_i<nTimesteps;timestep_i++)
	{
		// computeAntennaPairFlagsCore(field_spw,scutof,nTimesteps-effectiveNTimeSteps,nTimesteps-1,timestep_i,visibilities,flags);
		computeAntennaPairFlagsCore(field_spw,noise,scutof,-1,-2,timestep_i,visibilities,flags);
	}

	return false;
}

void
FlagAgentRFlag::passIntermediate(const VisBuffer &visBuffer)
{
	// Set logger origin
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

	// Make field-spw pair
	Int field = visBuffer.fieldId();
	Int spw = visBuffer.spectralWindow();
	pair<Int,Int> field_spw = std::make_pair(field,spw);

	if (field_spw_noise_map_p.find(field_spw) == field_spw_noise_map_p.end())
	{
		Double noise = noiseScale_p*computeThreshold(	field_spw_noise_histogram_sum_p[field_spw],
														field_spw_noise_histogram_sum_squares_p[field_spw],
														field_spw_noise_histogram_counts_p[field_spw]		);

		field_spw_noise_map_p[field_spw] = noise;
		*logger_p << LogIO::DEBUG1 << " field=" << field << " spw=" <<  spw << " noise=" << noise << LogIO::POST;
	}

	if (field_spw_scutof_map_p.find(field_spw) == field_spw_scutof_map_p.end())
	{
		Double scutof = scutofScale_p*computeThreshold(	field_spw_scutof_histogram_sum_p[field_spw],
														field_spw_scutof_histogram_sum_squares_p[field_spw],
														field_spw_scutof_histogram_counts_p[field_spw]		);

		field_spw_scutof_map_p[field_spw] = scutof;
		*logger_p << LogIO::DEBUG1 << " field=" << field << " spw=" <<  spw << " scutof=" << scutof << LogIO::POST;
	}


	return;
}

void
FlagAgentRFlag::passFinal(const VisBuffer &visBuffer)
{

	// Make field-spw pair
	Int field = visBuffer.fieldId();
	Int spw = visBuffer.spectralWindow();
	pair<Int,Int> field_spw = std::make_pair(field,spw);
	if (user_field_spw_noise_map_p.find(field_spw) == user_field_spw_noise_map_p.end())
	{
		field_spw_noise_map_p.erase(field_spw);
		field_spw_noise_histogram_sum_p.erase(field_spw);
		field_spw_noise_histogram_sum_squares_p.erase(field_spw);
		field_spw_noise_histogram_counts_p.erase(field_spw);
	}

	if (user_field_spw_scutof_map_p.find(field_spw) == user_field_spw_scutof_map_p.end())
	{
		field_spw_scutof_map_p.erase(field_spw);
		field_spw_scutof_histogram_sum_p.erase(field_spw);
		field_spw_scutof_histogram_sum_squares_p.erase(field_spw);
		field_spw_scutof_histogram_counts_p.erase(field_spw);
	}

	return;
}

} //# NAMESPACE CASA - END


