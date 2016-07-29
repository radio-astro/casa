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


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// UVContSubTVI class
//////////////////////////////////////////////////////////////////////////

class UVContSubTVI : public FreqAxisTVI
{

public:

	UVContSubTVI(	ViImplementation2 * inputVii,
						const Record &configuration);

    virtual void floatData (Cube<Float> & vis) const;
    virtual void visibilityObserved (Cube<Complex> & vis) const;
    virtual void visibilityCorrected (Cube<Complex> & vis) const;
    virtual void visibilityModel (Cube<Complex> & vis) const;

protected:

    Bool parseConfiguration(const Record &configuration);
    void initialize();

    template<class T> void transformDataCube(	const Cube<T> &inputVis,
    											const Cube<Float> &inputSigma,
    											Bool sigmaAsWeight,
    											Cube<T> &outputVis) const;

    mutable uInt fitOrder_p;
    mutable Bool want_cont_p;
    mutable String fitspw_p;
    mutable LinearFitSVD<Float> fitter_p;
    mutable map<Int,Vector<Bool> > lineFreeChannelMaskMap_p;
	mutable map<Int, Matrix<Float> > inputFrequencyMap_p;
};

//////////////////////////////////////////////////////////////////////////
// UVContSubTVIFactory class
//////////////////////////////////////////////////////////////////////////

class UVContSubTVIFactory : public ViFactory
{

public:

	UVContSubTVIFactory(Record &configuration,ViImplementation2 *inputVII);

protected:

	vi::ViImplementation2 * createVi (VisibilityIterator2 *) const;
	vi::ViImplementation2 * createVi () const;

	Record configuration_p;
	ViImplementation2 *inputVii_p;;
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

	UVContSubKernel(	uInt fitOrder,
						LinearFitSVD<Float> *fitter,
						Matrix<Float> *freqPows,
						Vector<Bool> *lineFreeChannelMask);

	virtual void kernel(DataCubeMap *inputData,
						DataCubeMap *outputData) = 0;

	void setDebug(Bool debug) { debug_p = debug;}

protected:

	Bool debug_p;
	uInt fitOrder_p;
	LinearFitSVD<Float> *fitter_p;
	Matrix<Float> *freqPows_p;
	Vector<Float> frequencies_p;
	Vector<Bool> *lineFreeChannelMask_p;
};

//////////////////////////////////////////////////////////////////////////
// UVContSubtractionKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class UVContSubtractionKernel : public UVContSubKernel<T>
{
	using UVContSubKernel<T>::fitOrder_p;
	using UVContSubKernel<T>::fitter_p;
	using UVContSubKernel<T>::freqPows_p;
	using UVContSubKernel<T>::frequencies_p;
	using UVContSubKernel<T>::lineFreeChannelMask_p;
	using UVContSubKernel<T>::debug_p;


public:

	UVContSubtractionKernel(	uInt fitOrder,
								LinearFitSVD<Float> *fitter,
								Matrix<Float> *freqPows,
								Vector<Bool> *lineFreeChannelMask=NULL);

	void kernel(DataCubeMap *inputData,
				DataCubeMap *outputData);

	void kernelCore(	Vector<Complex> &inputVector,
						Vector<Bool> &inputFlags,
						Vector<Float> &inputSigma,
						Vector<Complex> &outputVector);

	void kernelCore(	Vector<Float> &inputVector,
						Vector<Bool> &inputFlags,
						Vector<Float> &inputSigma,
						Vector<Float> &outputVector);
};

//////////////////////////////////////////////////////////////////////////
// UVContEstimationKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class UVContEstimationKernel : public UVContSubKernel<T>
{

	using UVContSubKernel<T>::fitOrder_p;
	using UVContSubKernel<T>::fitter_p;
	using UVContSubKernel<T>::freqPows_p;
	using UVContSubKernel<T>::frequencies_p;
	using UVContSubKernel<T>::lineFreeChannelMask_p;
	using UVContSubKernel<T>::debug_p;

public:

	UVContEstimationKernel(	uInt fitOrder,
							LinearFitSVD<Float> *fitter,
							Matrix<Float> *freqPows,
							Vector<Bool> *lineFreeChannelMask=NULL);

	void kernel(DataCubeMap *inputData,
				DataCubeMap *outputData);

	void kernelCore(	Vector<Complex> &inputVector,
						Vector<Bool> &inputFlags,
						Vector<Float> &inputSigma,
						Vector<Complex> &outputVector);

	void kernelCore(	Vector<Float> &inputVector,
						Vector<Bool> &inputFlags,
						Vector<Float> &inputSigma,
						Vector<Float> &outputVector);
};



} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* UVContSubTVI_H_ */

