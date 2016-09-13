//# UVContSubTVI.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef UVContSubTVI_H_
#define UVContSubTVI_H_

// Base class
#include <mstransform/TVI/FreqAxisTVI.h>

// Fitting classes
#include <scimath/Fitting/LinearFitSVD.h>
#include <scimath/Functionals/Polynomial.h>
#include <mstransform/TVI/DenoisingLib.h>

// OpenMP
#ifdef _OPENMP
#include <omp.h>
#endif


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// UVContSubTVI class
//////////////////////////////////////////////////////////////////////////

class UVContSubTVI : public FreqAxisTVI
{

public:

	UVContSubTVI(	ViImplementation2 * inputVii,
						const casacore::Record &configuration);

	~UVContSubTVI();

	// Report the the ViImplementation type
	virtual casacore::String ViiType() const { return casacore::String("UVContSub( ")+getVii()->ViiType()+" )"; };

    virtual void floatData (casacore::Cube<casacore::Float> & vis) const;
    virtual void visibilityObserved (casacore::Cube<casacore::Complex> & vis) const;
    virtual void visibilityCorrected (casacore::Cube<casacore::Complex> & vis) const;
    virtual void visibilityModel (casacore::Cube<casacore::Complex> & vis) const;

protected:

    bool parseConfiguration(const casacore::Record &configuration);
    void initialize();

    template<class T> void transformDataCube(	const casacore::Cube<T> &inputVis,
    											const casacore::Cube<casacore::Float> &inputWeight,
    											casacore::Cube<T> &outputVis) const;

    template<class T> void transformDataCore(	denoising::GslPolynomialModel<casacore::Double>* model,
												casacore::Vector<casacore::Bool> *lineFreeChannelMask,
												const casacore::Cube<T> &inputVis,
												const casacore::Cube<casacore::Bool> &inputFlags,
												const casacore::Cube<casacore::Float> &inputWeight,
												casacore::Cube<T> &outputVis,
												casacore::Int parallelCorrAxis=-1) const;
    mutable casacore::uInt fitOrder_p;
    mutable casacore::Bool want_cont_p;
    mutable casacore::String fitspw_p;
    mutable casacore::Bool gsl_p;
    mutable map<casacore::Int,casacore::Vector<casacore::Bool> > lineFreeChannelMaskMap_p;

	mutable map<casacore::Int, denoising::GslPolynomialModel<casacore::Double>* > inputFrequencyMap_p;
};

//////////////////////////////////////////////////////////////////////////
// UVContSubTVIFactory class
//////////////////////////////////////////////////////////////////////////

class UVContSubTVIFactory : public ViFactory
{

public:

	UVContSubTVIFactory(casacore::Record &configuration,ViImplementation2 *inputVII);

protected:

	vi::ViImplementation2 * createVi (VisibilityIterator2 *) const;
	vi::ViImplementation2 * createVi () const;

	casacore::Record configuration_p;
	ViImplementation2 *inputVii_p;
};

//////////////////////////////////////////////////////////////////////////
// UVContSubTVILayerFactory class  (for _recursive_ layering)
//////////////////////////////////////////////////////////////////////////

class UVContSubTVILayerFactory : public ViiLayerFactory
{

public:

	UVContSubTVILayerFactory(casacore::Record &configuration);
	virtual ~UVContSubTVILayerFactory() {};

protected:

	ViImplementation2 * createInstance(ViImplementation2* vii0) const;

	casacore::Record configuration_p;

};

//////////////////////////////////////////////////////////////////////////
// UVContSubTransformEngine class
//////////////////////////////////////////////////////////////////////////

template<class T> class UVContSubKernel; // Forward declaration

template<class T> class UVContSubTransformEngine : public FreqAxisTransformEngine2<T>
{
	using FreqAxisTransformEngine2<T>::inputData_p;
	using FreqAxisTransformEngine2<T>::outputData_p;
	using FreqAxisTransformEngine2<T>::debug_p;

public:

	UVContSubTransformEngine(	UVContSubKernel<T> *kernel,
								DataCubeMap *inputData,
								DataCubeMap *outputData	);

	void transform();

	void transformCore(DataCubeMap *inputData,DataCubeMap *outputData);

protected:

	// This member has to be a pointer, otherwise there
	// are compile time problems due to the fact that
	// it is a pure virtual class.
	UVContSubKernel<T> *uvContSubKernel_p;
};

//////////////////////////////////////////////////////////////////////////
// UVContSubKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class UVContSubKernel
{

public:

	UVContSubKernel(	denoising::GslPolynomialModel<casacore::Double>* model,
						casacore::Vector<casacore::Bool> *lineFreeChannelMask);

	virtual void kernel(DataCubeMap *inputData,
						DataCubeMap *outputData);

	virtual void changeFitOrder(size_t order) = 0;

	virtual void defaultKernel(	casacore::Vector<T> &inputVector,
								casacore::Vector<T> &outputVector) = 0;

	virtual void kernelCore(casacore::Vector<T> &inputVector,
							casacore::Vector<casacore::Bool> &inputFlags,
							casacore::Vector<casacore::Float> &inputWeights,
							casacore::Vector<T> &outputVector) = 0;

	void setDebug(casacore::Bool debug) { debug_p = debug;}

protected:

	casacore::Bool debug_p;
	size_t fitOrder_p;
	denoising::GslPolynomialModel<casacore::Double> *model_p;
	casacore::Matrix<casacore::Double> freqPows_p;
	casacore::Vector<casacore::Float> frequencies_p;
	casacore::Vector<casacore::Bool> *lineFreeChannelMask_p;
};

//////////////////////////////////////////////////////////////////////////
// UVContSubtractionKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class UVContSubtractionKernel : public UVContSubKernel<T>
{
	using UVContSubKernel<T>::fitOrder_p;
	using UVContSubKernel<T>::model_p;
	using UVContSubKernel<T>::freqPows_p;
	using UVContSubKernel<T>::frequencies_p;
	using UVContSubKernel<T>::lineFreeChannelMask_p;
	using UVContSubKernel<T>::debug_p;


public:

	UVContSubtractionKernel(	denoising::GslPolynomialModel<casacore::Double>* model,
								casacore::Vector<casacore::Bool> *lineFreeChannelMask=NULL);

	void changeFitOrder(size_t order);

	void defaultKernel(	casacore::Vector<casacore::Complex> &inputVector,
						casacore::Vector<casacore::Complex> &outputVector);

	void defaultKernel(	casacore::Vector<casacore::Float> &inputVector,
						casacore::Vector<casacore::Float> &outputVector);

	void kernelCore(	casacore::Vector<casacore::Complex> &inputVector,
						casacore::Vector<casacore::Bool> &inputFlags,
						casacore::Vector<casacore::Float> &inputWeights,
						casacore::Vector<casacore::Complex> &outputVector);

	void kernelCore(	casacore::Vector<casacore::Float> &inputVector,
						casacore::Vector<casacore::Bool> &inputFlags,
						casacore::Vector<casacore::Float> &inputWeights,
						casacore::Vector<casacore::Float> &outputVector);

private:

	casacore::LinearFitSVD<casacore::Float> fitter_p;
};

//////////////////////////////////////////////////////////////////////////
// UVContEstimationKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class UVContEstimationKernel : public UVContSubKernel<T>
{

	using UVContSubKernel<T>::fitOrder_p;
	using UVContSubKernel<T>::model_p;
	using UVContSubKernel<T>::freqPows_p;
	using UVContSubKernel<T>::frequencies_p;
	using UVContSubKernel<T>::lineFreeChannelMask_p;
	using UVContSubKernel<T>::debug_p;

public:

	UVContEstimationKernel(	denoising::GslPolynomialModel<casacore::Double>* model,
							casacore::Vector<casacore::Bool> *lineFreeChannelMask=NULL);

	void changeFitOrder(size_t order);

	void defaultKernel(	casacore::Vector<casacore::Complex> &inputVector,
						casacore::Vector<casacore::Complex> &outputVector);

	void defaultKernel(	casacore::Vector<casacore::Float> &inputVector,
						casacore::Vector<casacore::Float> &outputVector);

	void kernelCore(	casacore::Vector<casacore::Complex> &inputVector,
						casacore::Vector<casacore::Bool> &inputFlags,
						casacore::Vector<casacore::Float> &inputWeights,
						casacore::Vector<casacore::Complex> &outputVector);

	void kernelCore(	casacore::Vector<casacore::Float> &inputVector,
						casacore::Vector<casacore::Bool> &inputFlags,
						casacore::Vector<casacore::Float> &inputWeights,
						casacore::Vector<casacore::Float> &outputVector);

private:

	casacore::LinearFitSVD<casacore::Float> fitter_p;
};

//////////////////////////////////////////////////////////////////////////
// UVContSubtractionGSLKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class UVContSubtractionGSLKernel : public UVContSubKernel<T>
{
	using UVContSubKernel<T>::fitOrder_p;
	using UVContSubKernel<T>::model_p;
	using UVContSubKernel<T>::freqPows_p;
	using UVContSubKernel<T>::frequencies_p;
	using UVContSubKernel<T>::lineFreeChannelMask_p;
	using UVContSubKernel<T>::debug_p;


public:

	UVContSubtractionGSLKernel(	denoising::GslPolynomialModel<casacore::Double>* model,
								casacore::Vector<casacore::Bool> *lineFreeChannelMask=NULL);

	void changeFitOrder(size_t order);

	void defaultKernel(	casacore::Vector<T> &inputVector,
						casacore::Vector<T> &outputVector);

	void kernelCore(	casacore::Vector<T> &inputVector,
						casacore::Vector<casacore::Bool> &inputFlags,
						casacore::Vector<casacore::Float> &inputWeights,
						casacore::Vector<T> &outputVector);

private:

	denoising::GslMultifitWeightedLinear fitter_p;
};

//////////////////////////////////////////////////////////////////////////
// UVContEstimationGSLKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class UVContEstimationGSLKernel : public UVContSubKernel<T>
{

	using UVContSubKernel<T>::fitOrder_p;
	using UVContSubKernel<T>::model_p;
	using UVContSubKernel<T>::freqPows_p;
	using UVContSubKernel<T>::frequencies_p;
	using UVContSubKernel<T>::lineFreeChannelMask_p;
	using UVContSubKernel<T>::debug_p;

public:

	UVContEstimationGSLKernel(	denoising::GslPolynomialModel<casacore::Double>* model,
							casacore::Vector<casacore::Bool> *lineFreeChannelMask=NULL);

	void changeFitOrder(size_t order);

	void defaultKernel(	casacore::Vector<T> &inputVector,
						casacore::Vector<T> &outputVector);

	void kernelCore(	casacore::Vector<T> &inputVector,
						casacore::Vector<casacore::Bool> &inputFlags,
						casacore::Vector<casacore::Float> &inputWeights,
						casacore::Vector<T> &outputVector);

private:

	denoising::GslMultifitWeightedLinear fitter_p;
};



} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* UVContSubTVI_H_ */

