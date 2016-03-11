//# RegriddingTVI.h: This file contains the implementation of the RegriddingTVI class.
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

#include <mstransform/TVI/RegriddingTVI.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// RegriddingTVI class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
RegriddingTVI::RegriddingTVI(	ViImplementation2 * inputVii,
								const Record &configuration):
								FreqAxisTVI (inputVii,configuration)
{
	// Frequency specification parameters
	nChan_p = -1;
	mode_p = String("channel"); 					// Options are: channel, frequency, velocity
	start_p = String("0");
	width_p = String("1");								// -1 means use all the input channels
	velocityType_p = String("radio");				// When mode is velocity options are: optical, radio
	restFrequency_p = String("");
	interpolationMethodPar_p = String("linear");	// Options are: nearest, linear, cubic, spline, fftshift
	outputReferenceFramePar_p = String("");			// Options are: LSRK, LSRD, BARY, GALACTO, LGROUP, CMB, GEO, or TOPO
	phaseCenterPar_p = new casac::variant("");
	interpolationMethod_p = linear;

	// Sub-cases
    fftShiftEnabled_p = False;
	refFrameTransformation_p = False;
	radialVelocityCorrection_p = False;
	fftShift_p = 0;

	// SPW-indexed maps
    weightFactorMap_p.clear();
	sigmaFactorMap_p.clear();
	inputOutputSpwMap_p.clear();
	freqTransEngineRowId_p = UINT_MAX;

	// Parse and check configuration parameters
	// Note: if a constructor finishes by throwing an exception, the memory
	// associated with the object itself is cleaned up — there is no memory leak.
	if (not parseConfiguration(configuration))
	{
		throw AipsError("Error parsing RegriddingTVI configuration");
	}

	initialize();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool RegriddingTVI::parseConfiguration(const Record &configuration)
{
	int exists = 0;
	Bool ret = True;

	exists = configuration.fieldNumber ("phasecenter");
	if (exists >= 0)
	{
		//If phase center is a simple numeric value then it is taken
		// as a FIELD_ID otherwise it is converted to a MDirection
        if( configuration.type(exists) == TpInt )
        {
        	int fieldIdForPhaseCenter = -1;
    		configuration.get (exists, fieldIdForPhaseCenter);
    		logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
    				<< "Field Id for phase center is " << fieldIdForPhaseCenter << LogIO::POST;
    		if (phaseCenterPar_p) delete phaseCenterPar_p;
    		phaseCenterPar_p = new casac::variant(fieldIdForPhaseCenter);
        }
        else
        {
        	String phaseCenter("");
    		configuration.get (exists, phaseCenter);
    		logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
    				<< "Phase center is " << phaseCenter << LogIO::POST;
    		if (phaseCenterPar_p) delete phaseCenterPar_p;
    		phaseCenterPar_p = new casac::variant(phaseCenter);
        }
	}

	exists = configuration.fieldNumber ("restfreq");
	if (exists >= 0)
	{
		configuration.get (exists, restFrequency_p);
		if (!restFrequency_p.empty())
		{
			logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
					<< "Rest frequency is " << restFrequency_p << LogIO::POST;
		}
	}

	exists = configuration.fieldNumber ("outframe");
	if (exists >= 0)
	{
		configuration.get (exists, outputReferenceFramePar_p);
		logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
				<< "Output reference frame is " << outputReferenceFramePar_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("interpolation");
	if (exists >= 0)
	{
		configuration.get (exists, interpolationMethodPar_p);

		if (interpolationMethodPar_p.contains("nearest"))
		{
			interpolationMethod_p = nearestNeighbour;
		}
		else if (interpolationMethodPar_p.contains("linear"))
		{
			interpolationMethod_p = linear;
		}
		else if (interpolationMethodPar_p.contains("cubic"))
		{
			interpolationMethod_p = cubic;
		}
		else if (interpolationMethodPar_p.contains("spline"))
		{
			interpolationMethod_p = spline;
		}
		else if (interpolationMethodPar_p.contains("fftshift"))
		{
			fftShiftEnabled_p = True;
			interpolationMethod_p = linear;
		}
		else
		{
			logger_p << LogIO::SEVERE << LogOrigin("RegriddingTVI", __FUNCTION__)
					<< "Interpolation method " << interpolationMethodPar_p  << " not available " << LogIO::POST;
		}

		logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
				<< "Interpolation method is " << interpolationMethodPar_p  << LogIO::POST;
	}
	else
	{
		interpolationMethod_p = linear;
	}

	exists = configuration.fieldNumber ("mode");
	if (exists >= 0)
	{
		configuration.get (exists, mode_p);
		logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
				<< "Mode is " << mode_p<< LogIO::POST;

		if ((mode_p == "frequency") or (mode_p == "velocity"))
		{
			start_p = String("");
			width_p = String("");
		}
	}

	exists = configuration.fieldNumber ("nchan");
	if (exists >= 0)
	{
		configuration.get (exists, nChan_p);
		logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
				<< "Number of channels is " << nChan_p<< LogIO::POST;
	}

	exists = configuration.fieldNumber ("start");
	if (exists >= 0)
	{
		configuration.get (exists, start_p);
		logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
				<< "Start is " << start_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("width");
	if (exists >= 0)
	{
		configuration.get (exists, width_p);
		logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
				<< "Width is " << width_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("veltype");
	if ((exists >= 0) and (mode_p == "velocity"))
	{
		configuration.get (exists, velocityType_p);
		logger_p << LogIO::NORMAL << LogOrigin("RegriddingTVI", __FUNCTION__)
				<< "Velocity type is " << velocityType_p << LogIO::POST;
	}

	return ret;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::initialize()
{
    // Determine input reference frame from the first row in the SPW sub-table of the output (selected) MS
	MSSpectralWindow spwTable = getVii()->ms().spectralWindow();
	MSSpWindowColumns spwCols(spwTable);
    inputReferenceFrame_p = MFrequency::castType(spwCols.measFreqRef()(0));

    // Parse output reference frame
    refFrameTransformation_p = True;
    radialVelocityCorrection_p = False;
    if(outputReferenceFramePar_p.empty())
    {
    	outputReferenceFrame_p = inputReferenceFrame_p;
    }
    // CAS-6778: Support for new ref. frame SOURCE that requires radial velocity correction
    else if (outputReferenceFramePar_p == "SOURCE")
    {
    	outputReferenceFrame_p = MFrequency::GEO;
    	radialVelocityCorrection_p = True;
    }
    else if(!MFrequency::getType(outputReferenceFrame_p, outputReferenceFramePar_p))
    {
    	logger_p << LogIO::SEVERE << LogOrigin("RegriddingTVI", __FUNCTION__)
    			<< "Problem parsing output reference frame:" << outputReferenceFramePar_p  << LogIO::EXCEPTION;
    }

    if (outputReferenceFrame_p == inputReferenceFrame_p) {
    	refFrameTransformation_p = False;
    }


    // Determine observatory position from the first row in the observation sub-table of the output (selected) MS
    MSObservation observationTable = getVii()->ms().observation();
    MSObservationColumns observationCols(observationTable);
    String observatoryName = observationCols.telescopeName()(0);
    MeasTable::Observatory(observatoryPosition_p,observatoryName);

    // jagonzal: This conversion is needed only for cosmetic reasons
    // observatoryPosition_p=MPosition::Convert(observatoryPosition_p, MPosition::ITRF)();

    // Determine observation time from the first row in the selected MS
    selectedInputMsCols_p = new ROMSColumns(getVii()->ms());
    referenceTime_p = selectedInputMsCols_p->timeMeas()(0);

    // Access FIELD cols to get phase center and radial velocity
    MSField field = getVii()->ms().field();
    inputMSFieldCols_p = new MSFieldColumns(field);

	// Determine phase center
    if (phaseCenterPar_p->type() == casac::variant::INT)
    {
    	Int fieldIdForPhaseCenter = phaseCenterPar_p->toInt();

    	if (fieldIdForPhaseCenter >= (Int)inputMSFieldCols_p->nrow() || fieldIdForPhaseCenter < 0)
    	{
    		logger_p << LogIO::SEVERE << LogOrigin("RegriddingTVI", __FUNCTION__)
    				<< "Selected FIELD_ID to determine phase center does not exist " << LogIO::POST;
    	}
    	else
    	{
    		// CAS-6778: Support for new ref. frame SOURCE that requires radial velocity correction
    		if (radialVelocityCorrection_p)
    		{
    			radialVelocity_p = inputMSFieldCols_p->radVelMeas(fieldIdForPhaseCenter, referenceTime_p.get("s").getValue());
    			phaseCenter_p = inputMSFieldCols_p->phaseDirMeas(fieldIdForPhaseCenter,referenceTime_p.get("s").getValue());
    		}
    		else
    		{
    			phaseCenter_p = inputMSFieldCols_p->phaseDirMeasCol()(fieldIdForPhaseCenter)(IPosition(1,0));
    		}
    	}
    }
    else
    {
    	String phaseCenter = phaseCenterPar_p->toString(True);

    	// Determine phase center from the first row in the FIELD sub-table of the output (selected) MS
    	if (phaseCenter.empty())
    	{
    		// CAS-6778: Support for new ref. frame SOURCE that requires radial velocity correction
    		if (radialVelocityCorrection_p)
    		{
    			radialVelocity_p = inputMSFieldCols_p->radVelMeas(0, referenceTime_p.get("s").getValue());
    			phaseCenter_p = inputMSFieldCols_p->phaseDirMeas(0,referenceTime_p.get("s").getValue());
    		}
    		else
    		{
    			phaseCenter_p = inputMSFieldCols_p->phaseDirMeasCol()(0)(IPosition(1,0));
    		}
    	}
    	// Parse phase center
    	else
    	{
        	if(!casaMDirection(phaseCenter, phaseCenter_p))
        	{
        		logger_p << LogIO::SEVERE << LogOrigin("MSTransformManager", __FUNCTION__)
        				<< "Cannot interpret phase center " << phaseCenter << LogIO::POST;
        		return;
        	}
    	}
    }

   if (radialVelocityCorrection_p && (radialVelocity_p.getRef().getType() != MRadialVelocity::GEO))
   {
	   logger_p << LogIO::SEVERE << LogOrigin("RegriddingTVI", __FUNCTION__)
			   << "Cannot perform radial velocity correction with ephemerides of type "
			   << MRadialVelocity::showType(radialVelocity_p.getRef().getType()) << ".\nType needs to be GEO."
			   << LogIO::EXCEPTION;
   }

   return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::origin()
{
	// Drive underlying ViImplementation2
	getVii()->origin();

	// Define output grid for this chunk (also defines shape)
	initFrequencyGrid();

	// Synchronize own VisBuffer
	configureNewSubchunk();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Int RegriddingTVI::getReportingFrameOfReference () const
{
	return outputReferenceFrame_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::initFrequencyGrid()
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Check if frequency grid has to be initialized for this SPW
	Int spwId = vb->spectralWindows()(0);
	if (inputOutputSpwMap_p.find(spwId) == inputOutputSpwMap_p.end())
	{
		// Get input frequencies in reporting frame of reference
		// i.e. as they are stored in the SPW sub-table
		Vector<Double> inputFreq = vb->getFrequencies(0);
		Vector<Double> inputWidth(inputFreq.size(),inputFreq(1)-inputFreq(0));

		// Declare output variables
		Double weightScale;
		Vector<Double> outputFreq;
		Vector<Double> outputWidth;

		// Use calcChanFreqs to change reference frame and regrid
		Bool ret = MSTransformRegridder::calcChanFreqs(	logger_p,
														outputFreq,
														outputWidth,
														weightScale,
														inputFreq,
														inputWidth,
														phaseCenter_p,
														inputReferenceFrame_p,
														referenceTime_p,
														observatoryPosition_p,
														mode_p,
														nChan_p,
														start_p,
														width_p,
														restFrequency_p,
														outputReferenceFramePar_p,
														velocityType_p,
														True, // verbose
														radialVelocity_p
														);

		if (not ret)
		{
			throw AipsError("Error calculating output grid");
		}

        // Add input-output SPW pair to map
        spwInfo inputSpw(inputFreq,inputWidth);
        spwInfo outputSpw(outputFreq,outputWidth);
    	inputOutputSpwMap_p[spwId] = std::make_pair(inputSpw,outputSpw);

    	// Store weight/sigma factors
    	weightFactorMap_p[spwId] = weightScale;
    	sigmaFactorMap_p[spwId] = 1 / sqrt(weightScale);

    	// Populate nchan input-output maps
    	spwOutChanNumMap_p[spwId] = outputSpw.NUM_CHAN;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::initFrequencyTransformationEngine() const
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Check if frequency transformation engine has to be re-constructed
	uInt rowId = vb->rowIds()[0];
	if (freqTransEngineRowId_p != rowId)
	{
		// Mark this rowId as the current one
		freqTransEngineRowId_p = rowId;

		// Get spwId to access input-output SPW map
		Int spwId = vb->spectralWindows()[0];

		// Get current time
		ScalarMeasColumn<MEpoch> mainTimeMeasCol = selectedInputMsCols_p->timeMeas();
		MEpoch currentRowTime = mainTimeMeasCol(rowId);

		// Check if radial velocity correction if necessary
		MDoppler radVelCorr;
		MDirection inputFieldDirection;
		Bool radVelSignificant = False;
		if (radialVelocityCorrection_p && inputMSFieldCols_p->needInterTime(vb->fieldId()(0)))
		{
			MRadialVelocity mRV = inputMSFieldCols_p->radVelMeas(vb->fieldId()(0),vb->time()(0));
			Quantity mrv = mRV.get("m/s");
			Quantity offsetMrv = radialVelocity_p.get("m/s"); // the radvel by which the out SPW def was shifted
			radVelCorr =  MDoppler(mrv-(Quantity(2.)*offsetMrv));
			if (fabs(mrv.getValue()) > 1E-6) radVelSignificant = True;

			inputFieldDirection = inputMSFieldCols_p->phaseDirMeas(vb->fieldId()(0), vb->time()(0));
		}
		else
		{
			inputFieldDirection = vb->phaseCenter();
		}

		// Construct reference frame transformation engine
		MFrequency::Ref inputFrameRef = MFrequency::Ref(inputReferenceFrame_p,
														MeasFrame(inputFieldDirection,
																observatoryPosition_p,
																currentRowTime));

		MFrequency::Ref outputFrameRef = MFrequency::Ref(outputReferenceFrame_p,
														MeasFrame(phaseCenter_p,
																observatoryPosition_p,
																referenceTime_p));

		// Calculate new frequencies (also for FFT mode)
		freqTransEngine_p = MFrequency::Convert(Hz, inputFrameRef, outputFrameRef);

	    for(uInt chan_idx=0; chan_idx<inputOutputSpwMap_p[spwId].first.CHAN_FREQ.size(); chan_idx++)
	    {
	    	inputOutputSpwMap_p[spwId].first.CHAN_FREQ_aux[chan_idx] =
	    			freqTransEngine_p(inputOutputSpwMap_p[spwId].first.CHAN_FREQ[chan_idx]).get(Hz).getValue();
	    }

	    // Apply radial velocity correction if necessary
		if (radVelSignificant)
		{
			inputOutputSpwMap_p[spwId].first.CHAN_FREQ_aux =
					radVelCorr.shiftFrequency(inputOutputSpwMap_p[spwId].first.CHAN_FREQ_aux);
		}

		// Calculate FFT shift if necessary
		if (fftShiftEnabled_p)
		{
			uInt centralChan = inputOutputSpwMap_p[spwId].first.CHAN_FREQ.size()/2;
			Double absoluteShift = inputOutputSpwMap_p[spwId].first.CHAN_FREQ_aux[centralChan]
			                      -inputOutputSpwMap_p[spwId].first.CHAN_FREQ[centralChan];

			Double chanWidth = inputOutputSpwMap_p[spwId].second.CHAN_FREQ[1]
			                 - inputOutputSpwMap_p[spwId].second.CHAN_FREQ[0];

			Double bandwidth = inputOutputSpwMap_p[spwId].second.CHAN_FREQ[inputOutputSpwMap_p[spwId].second.NUM_CHAN-1]
			                 - inputOutputSpwMap_p[spwId].second.CHAN_FREQ[0];

			bandwidth += chanWidth;

			fftShift_p = - absoluteShift / bandwidth;
		}
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Vector<Double> RegriddingTVI::getFrequencies (	Double time,
												Int frameOfReference,
												Int spectralWindowId,
												Int msId) const
{
	if (frameOfReference == outputReferenceFrame_p)
	{
		return inputOutputSpwMap_p[spectralWindowId].second.CHAN_FREQ;
	}
	else
	{
		// Get frequencies from input TVI
		Vector<Double> inputFreq = getVii()->getFrequencies(time,frameOfReference,spectralWindowId,msId);

		Vector<Double> inputWidth(inputFreq.size(),0);
		for (uInt chan_i=0;chan_i<inputFreq.size()-1;chan_i++)
		{
			inputWidth(chan_i) = inputFreq(chan_i+1)-inputFreq(chan_i);
		}
		inputWidth(inputFreq.size()-1) =  inputWidth(inputFreq.size()-2);

		// Declare output variables
		Double weightScale;
		Vector<Double> outputFreq;
		Vector<Double> outputWidth;

		// Use calcChanFreqs for re-gridding only (output frame = inputFrame)
	    MFrequency::Types frameOfReferenceEnum = static_cast<MFrequency::Types> (frameOfReference);
		String frameOfReferenceStr = MFrequency::showType(frameOfReferenceEnum);
		MSTransformRegridder::calcChanFreqs(	logger_p,
												outputFreq,
												outputWidth,
												weightScale,
												inputFreq,
												inputWidth,
												phaseCenter_p,
												frameOfReferenceEnum,
												referenceTime_p,
												observatoryPosition_p,
												mode_p,
												nChan_p,
												start_p,
												width_p,
												restFrequency_p,
												frameOfReferenceStr,
												velocityType_p,
												False, // verbose
												radialVelocity_p
												);

		return outputFreq;
	}

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::flag(Cube<Bool>& flagCube) const
{
	initFrequencyTransformationEngine();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::floatData (Cube<Float> & vis) const
{
	initFrequencyTransformationEngine();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::visibilityObserved (Cube<Complex> & vis) const
{
	initFrequencyTransformationEngine();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::visibilityCorrected (Cube<Complex> & vis) const
{
	initFrequencyTransformationEngine();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::visibilityModel (Cube<Complex> & vis) const
{
	initFrequencyTransformationEngine();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::weightSpectrum(Cube<Float> &weightSp) const
{
	initFrequencyTransformationEngine();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void RegriddingTVI::sigmaSpectrum (Cube<Float> &sigmaSp) const
{
	initFrequencyTransformationEngine();

	return;
}

//////////////////////////////////////////////////////////////////////////
// RegriddingTVIFactory class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
RegriddingTVIFactory::RegriddingTVIFactory (Record &configuration,
											ViImplementation2 *inputVii)
{
	inputVii_p = inputVii;
	configuration_p = configuration;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * RegriddingTVIFactory::createVi(VisibilityIterator2 *) const
{
	return new RegriddingTVI(inputVii_p,configuration_p);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * RegriddingTVIFactory::createVi() const
{
	return new RegriddingTVI(inputVii_p,configuration_p);
}

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END


