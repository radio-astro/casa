//# RegriddingTVI.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef RegriddingTVI_H_
#define RegriddingTVI_H_

// Base class
#include <mstransform/TVI/FreqAxisTVI.h>

// To handle variant parameters
#include <stdcasa/StdCasa/CasacSupport.h>

// MS Access
#include <ms/MeasurementSets.h>

// To get observatory position from observatory name
#include <measures/Measures/MeasTable.h>

// calcChanFreqs
#include <mstransform/MSTransform/MSTransformManager.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// Enumerations
//////////////////////////////////////////////////////////////////////////

enum InterpolationMethod {
    // nearest neighbour
    nearestNeighbour,
    // linear
    linear,
    // cubic
    cubic,
    // cubic spline
    spline,
    // fft shift
    fftshift
  };

const Unit Hz(String("Hz"));

//////////////////////////////////////////////////////////////////////////
// RegriddingTVI class
//////////////////////////////////////////////////////////////////////////

class RegriddingTVI : public FreqAxisTVI
{

public:

	RegriddingTVI(	ViImplementation2 * inputVii,
					const Record &configuration);

	// Navigation methods
	void origin ();

	// Frequency specification methods
    Int getReportingFrameOfReference () const; // Used by VisBufferImpl2 to determine default ref. frame
    Vector<Double> getFrequencies (	Double time, Int frameOfReference,
    								Int spectralWindowId, Int msId) const;
    // Data accessors
    void flag(Cube<Bool>& flagCube) const;
    void floatData (Cube<Float> & vis) const;
    void visibilityObserved (Cube<Complex> & vis) const;
    void visibilityCorrected (Cube<Complex> & vis) const;
    void visibilityModel (Cube<Complex> & vis) const;
    void weightSpectrum(Cube<Float> &weightSp) const;
    void sigmaSpectrum (Cube<Float> &sigmaSp) const;

protected:

    Bool parseConfiguration(const Record &configuration);
    void initialize();

    void initFrequencyGrid();
    void initFrequencyTransformationEngine() const;

	int nChan_p;
	String mode_p;
	String start_p;
	String width_p;
	String velocityType_p;
	String restFrequency_p;
	String interpolationMethodPar_p;
	String outputReferenceFramePar_p;
	casac::variant *phaseCenterPar_p;
	uInt interpolationMethod_p;

    Bool fftShiftEnabled_p;
	Bool refFrameTransformation_p;
	Bool radialVelocityCorrection_p;
	Bool radialVelocityCorrectionSignificant_p;

	MEpoch referenceTime_p;
	MDirection phaseCenter_p;
	MPosition observatoryPosition_p;
	MRadialVelocity radialVelocity_p;
	MFrequency::Types outputReferenceFrame_p;
	MFrequency::Types inputReferenceFrame_p;

	mutable MFrequency::Convert freqTransEngine_p;
	mutable uInt freqTransEngineRowId_p;

	mutable Double fftShift_p;
    FFTServer<Float, Complex> fFFTServer_p;

	ROMSColumns *selectedInputMsCols_p;
	MSFieldColumns *inputMSFieldCols_p;

    map<Int,Float> weightFactorMap_p;
	map<Int,Float> sigmaFactorMap_p;
	mutable inputOutputSpwMap inputOutputSpwMap_p;
};

//////////////////////////////////////////////////////////////////////////
// RegriddingTVIFactory class
//////////////////////////////////////////////////////////////////////////

class RegriddingTVIFactory : public ViFactory
{

public:

	RegriddingTVIFactory(Record &configuration,ViImplementation2 *inputVII);

protected:

	vi::ViImplementation2 * createVi (VisibilityIterator2 *) const;
	vi::ViImplementation2 * createVi () const;

	Record configuration_p;
	ViImplementation2 *inputVii_p;;
};



} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* RegriddingTVI_H_ */

