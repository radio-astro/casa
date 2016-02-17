//# ConvolutionTVI.h: This file contains the implementation of the ConvolutionTVI class.
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

#include <mstransform/TVI/ConvolutionTVI.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// ConvolutionTVI class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
ConvolutionTVI::ConvolutionTVI(	ViImplementation2 * inputVii,
								const Record &configuration):
								FreqAxisTVI (inputVii,configuration)
{
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ConvolutionTVI::flag(Cube<Bool>& flagCube) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Configure Transformation Engine
	ConvolutionLogicalORKernel<Bool> kernel(&convCoeff_p);
	ConvolutionTransformEngine<Bool> transformer(&kernel,convCoeff_p.size());

	// Transform data
	transformFreqAxis(vb->flagCube(),flagCube,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ConvolutionTVI::floatData (Cube<Float> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Configure Transformation Engine
	ConvolutionDataKernel<Float> kernel(&convCoeff_p);
	ConvolutionTransformEngine<Float> transformer(&kernel,convCoeff_p.size());

	// Transform data
	transformFreqAxis(vb->visCubeFloat(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ConvolutionTVI::visibilityObserved (Cube<Complex> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Configure Transformation Engine
	ConvolutionDataKernel<Complex> kernel(&convCoeff_p);
	ConvolutionTransformEngine<Complex> transformer(&kernel,convCoeff_p.size());

	// Transform data
	transformFreqAxis(vb->visCube(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ConvolutionTVI::visibilityCorrected (Cube<Complex> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Configure Transformation Engine
	ConvolutionDataKernel<Complex> kernel(&convCoeff_p);
	ConvolutionTransformEngine<Complex> transformer(&kernel,convCoeff_p.size());

	// Transform data
	transformFreqAxis(vb->visCubeCorrected(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ConvolutionTVI::visibilityModel (Cube<Complex> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Configure Transformation Engine
	ConvolutionDataKernel<Complex> kernel(&convCoeff_p);
	ConvolutionTransformEngine<Complex> transformer(&kernel,convCoeff_p.size());

	// Transform data
	transformFreqAxis(vb->visCubeModel(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ConvolutionTVI::weightSpectrum(Cube<Float> &weightSp) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Configure Transformation Engine
	ConvolutionWeightPropagationKernel<Float> kernel(&convCoeff_p);
	ConvolutionTransformEngine<Float> transformer(&kernel,convCoeff_p.size());

	// Transform data
	transformFreqAxis(vb->weightSpectrum(),weightSp,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ConvolutionTVI::sigmaSpectrum(Cube<Float> &sigmaSp) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Configure Transformation Engine
	ConvolutionWeightPropagationKernel<Float> kernel(&convCoeff_p);
	ConvolutionTransformEngine<Float> transformer(&kernel,convCoeff_p.size());

	// Get weightSpectrum from sigmaSpectrum
	Cube<Float> weightSpFromSigmaSp;
	weightSpFromSigmaSp.resize(vb->sigmaSpectrum().shape(),False);
	weightSpFromSigmaSp = vb->sigmaSpectrum(); // = Operator makes a copy
	arrayTransformInPlace (weightSpFromSigmaSp,sigmaToWeight);

	// Transform data
	transformFreqAxis(weightSpFromSigmaSp,sigmaSp,transformer);

	// Transform back from weight format to sigma format
	arrayTransformInPlace (sigmaSp,weightToSigma);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Vector<Double> ConvolutionTVI::getFrequencies (	Double time,
												Int frameOfReference,
												Int spectralWindowId,
												Int msId) const
{
	return getVii()->getFrequencies (time, frameOfReference,spectralWindowId, msId);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ConvolutionTVI::writeFlag (const Cube<Bool> & flag)
{
	getVii()->writeFlag(flag);

	return;
}

//////////////////////////////////////////////////////////////////////////
// ConvolutionTVIFactory class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
ConvolutionTVIFactory::ConvolutionTVIFactory (Record &configuration,
													ViImplementation2 *inputVii)
{
	inputVii_p = inputVii;
	configuration_p = configuration;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * ConvolutionTVIFactory::createVi(VisibilityIterator2 *) const
{
	return new ConvolutionTVI(inputVii_p,configuration_p);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * ConvolutionTVIFactory::createVi() const
{
	return new ConvolutionTVI(inputVii_p,configuration_p);
}

//////////////////////////////////////////////////////////////////////////
// ConvolutionTransformEngine class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> ConvolutionTransformEngine<T>::ConvolutionTransformEngine
												(ConvolutionKernel<T> *kernel,
												uInt width)
{
	width_p = width;
	convolutionKernel_p = kernel;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ConvolutionTransformEngine<T>::transform(Vector<T> &inputVector,
																Vector<T> &outputVector,
																DataCubeMap &)
{
	uInt startChanIndex = 0;
	uInt outChanStart = width_p / 2;
	uInt outChanIndex = outChanStart;
	uInt outChanStop = inputVector.size() - width_p / 2;
	while (startChanIndex < outChanStop)
	{
		convolutionKernel_p->kernel(inputVector,outputVector,startChanIndex,outChanIndex);
		startChanIndex += 1;
		outChanIndex += 1;
	}

	// Process low end
	for (uInt chanIndex = 0; chanIndex<outChanStart; chanIndex++)
	{
		outputVector(chanIndex) = inputVector(chanIndex);
		chanIndex += 1;
	}

	// Process high end
	for (uInt chanIndex = outChanStop; chanIndex<inputVector.size(); chanIndex++)
	{
		outputVector(chanIndex) = inputVector(chanIndex);
		chanIndex += 1;
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
// ConvolutionKernel class
//////////////////////////////////////////////////////////////////////////
template<class T> ConvolutionKernel<T>::ConvolutionKernel(Vector<Float> *convCoeff)
{
	convCoeff_p = convCoeff;
	width_p = convCoeff->size();
}

//////////////////////////////////////////////////////////////////////////
// ConvolutionDataKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> ConvolutionDataKernel<T>::ConvolutionDataKernel(Vector<Float> *convCoeff):
		ConvolutionKernel<T>(convCoeff)
{

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ConvolutionDataKernel<T>::kernel(	Vector<T> &inputVector,
															Vector<T> &outputVector,
															uInt startInputPos,
															uInt outputPos)
{
	// Initialization
	outputVector(outputPos) = (*convCoeff_p)(0)*inputVector(startInputPos);

	// Main loop
	for (uInt chanIndex = 1; chanIndex<width_p; chanIndex++)
	{
		outputVector(outputPos) += (*convCoeff_p)(chanIndex)*inputVector(startInputPos+chanIndex);
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
// ConvolutionLogicalORKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> ConvolutionLogicalORKernel<T>::ConvolutionLogicalORKernel(Vector<Float> *convCoeff):
		ConvolutionKernel<T>(convCoeff)
{


}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ConvolutionLogicalORKernel<T>::kernel(	Vector<T> &inputVector,
																Vector<T> &outputVector,
																uInt startInputPos,
																uInt outputPos)
{
	// Output sample is flagged if any of the contributors are flagged
	for (uInt chanIndex = 0; chanIndex<width_p; chanIndex++)
	{
		if (inputVector(startInputPos+chanIndex)) outputVector(outputPos) = True;
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
// ConvolutionWeightPropagationKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> ConvolutionWeightPropagationKernel<T>::ConvolutionWeightPropagationKernel(Vector<Float> *convCoeff):
		ConvolutionKernel<T>(convCoeff)
{

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ConvolutionWeightPropagationKernel<T>::kernel(	Vector<T> &inputVector,
																		Vector<T> &outputVector,
																		uInt startInputPos,
																		uInt outputPos)
{
	// Initialization (mind for zeros as there is a division operation)
	outputVector(outputPos) = 0;
	if (inputVector(startInputPos) > FLT_MIN)
	{
		outputVector(outputPos) = (*convCoeff_p)(0)*(*convCoeff_p)(0)/inputVector(startInputPos);
	}

	// Main accumulation loop
	for (uInt chanIndex = 1; chanIndex<width_p; chanIndex++)
	{
		// Mind for zeros as there is a division operation
		if (inputVector(startInputPos+chanIndex) > FLT_MIN)
		{
			outputVector(outputPos) += (*convCoeff_p)(chanIndex)*(*convCoeff_p)(chanIndex)/inputVector(startInputPos+chanIndex);
		}
	}

	// Final propagated weight is the inverse of the accumulation
	if (outputVector(outputPos) > FLT_MIN)
	{
		outputVector(outputPos) = 1/outputVector(outputPos);
	}

	return;
}

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END


