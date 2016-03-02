//# PhaseShiftingTVI.h: This file contains the implementation of the PhaseShiftingTVI class.
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

#include <mstransform/TVI/PhaseShiftingTVI.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// PhaseShiftingTVI class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
PhaseShiftingTVI::PhaseShiftingTVI(	ViImplementation2 * inputVii,
								const Record &configuration):
								FreqAxisTVI (inputVii,configuration)
{
	// Parse and check configuration parameters
	// Note: if a constructor finishes by throwing an exception, the memory
	// associated with the object itself is cleaned up — there is no memory leak.
	if (not parseConfiguration(configuration))
	{
		throw AipsError("Error parsing PhaseShiftingTVI configuration");
	}

	initialize();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool PhaseShiftingTVI::parseConfiguration(const Record &configuration)
{
	int exists = 0;
	Bool ret = True;

	exists = configuration.fieldNumber ("XpcOffset");
	if (exists >= 0)
	{
		configuration.get (exists, dx_p);
	}

	exists = configuration.fieldNumber ("YpcOffset");
	if (exists >= 0)
	{
		configuration.get (exists, dy_p);
	}

	if (abs(dx_p) > 0 or abs(dy_p) > 0)
	{
		logger_p 	<< LogIO::NORMAL << LogOrigin("PhaseShiftingTVI", __FUNCTION__)
					<< "Phase shift is dx="<< dx_p << " dy=" << dy_p << LogIO::POST;
	}

	return ret;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void PhaseShiftingTVI::initialize()
{
	// Populate nchan input-output maps
	Int spw;
	uInt spw_idx = 0;
	map<Int,vector<Int> >::iterator iter;
	for(iter=spwInpChanIdxMap_p.begin();iter!=spwInpChanIdxMap_p.end();iter++)
	{
		spw = iter->first;
		spwOutChanNumMap_p[spw] = spwInpChanIdxMap_p[spw].size();

		spw_idx++;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void PhaseShiftingTVI::visibilityObserved (Cube<Complex> & vis) const
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Matrix<Double> uvw = vb->uvw();
	Vector<Double> frequencies = vb->getFrequencies(0);

	// Configure Transformation Engine
	PhaseShiftingTransformEngine<Complex> transformer(dx_p,dy_p,&uvw,&frequencies);

	// Transform data
	transformFreqAxis(vb->visCube(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void PhaseShiftingTVI::visibilityCorrected (Cube<Complex> & vis) const
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Matrix<Double> uvw = vb->uvw();
	Vector<Double> frequencies = vb->getFrequencies(0);

	// Configure Transformation Engine
	PhaseShiftingTransformEngine<Complex> transformer(dx_p,dy_p,&uvw,&frequencies);

	// Transform data
	transformFreqAxis(vb->visCubeCorrected(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void PhaseShiftingTVI::visibilityModel (Cube<Complex> & vis) const
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Matrix<Double> uvw = vb->uvw();
	Vector<Double> frequencies = vb->getFrequencies(0);

	// Configure Transformation Engine
	PhaseShiftingTransformEngine<Complex> transformer(dx_p,dy_p,&uvw,&frequencies);

	// Transform data
	transformFreqAxis(vb->visCubeModel(),vis,transformer);

	return;
}

//////////////////////////////////////////////////////////////////////////
// PhaseShiftingTVIFactory class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
PhaseShiftingTVIFactory::PhaseShiftingTVIFactory (	Record &configuration,
													ViImplementation2 *inputVii)
{
	inputVii_p = inputVii;
	configuration_p = configuration;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * PhaseShiftingTVIFactory::createVi(VisibilityIterator2 *) const
{
	return new PhaseShiftingTVI(inputVii_p,configuration_p);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * PhaseShiftingTVIFactory::createVi() const
{
	return new PhaseShiftingTVI(inputVii_p,configuration_p);
}

//////////////////////////////////////////////////////////////////////////
// PhaseShiftingTransformEngine class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> PhaseShiftingTransformEngine<T>::PhaseShiftingTransformEngine(
															Double dx, Double dy,
															Matrix<Double> *uvw,
															Vector<Double> *frequencies)
{
	uvw_p = uvw;
	frequencies_p = frequencies;

	// Offsets in radians (input is arcsec)
	dx_p = dx*(C::pi / 180.0 / 3600.0);
	dy_p = dy*(C::pi / 180.0 / 3600.0);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void PhaseShiftingTransformEngine<T>::transform(	Vector<T> &inputVector,
																	Vector<T> &outputVector)
{
	// Extra path as fraction of U and V in m
	Double phase = dx_p*(*uvw_p)(0,row_p) + dy_p*(*uvw_p)(1,row_p);

	// In radian/Hz
	phase *= -2.0 * C::pi / C::c;

	// Main loop
	Double phase_i;
	Complex factor;
	for (uInt chan_i=0;chan_i<inputVector.size();chan_i++)
	{
		phase_i = phase * (*frequencies_p)(chan_i);
		factor = Complex(cos(phase_i), sin(phase_i));
		outputVector(chan_i) = factor*inputVector(chan_i);
	}
}


} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END


