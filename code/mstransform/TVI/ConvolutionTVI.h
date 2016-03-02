//# ConvolutionTVI.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef ConvolutionTVI_H_
#define ConvolutionTVI_H_

// Base class
#include <mstransform/TVI/FreqAxisTVI.h>


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// ConvolutionTVI class
//////////////////////////////////////////////////////////////////////////

class ConvolutionTVI : public FreqAxisTVI
{

public:

	ConvolutionTVI(	ViImplementation2 * inputVii,
					const Record &configuration);

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

    mutable Vector<Float> convCoeff_p;
};

//////////////////////////////////////////////////////////////////////////
// ConvolutionTVIFactory class
//////////////////////////////////////////////////////////////////////////

class ConvolutionTVIFactory : public ViFactory
{

public:

	ConvolutionTVIFactory(Record &configuration,ViImplementation2 *inputVII);

protected:

	vi::ViImplementation2 * createVi (VisibilityIterator2 *) const;
	vi::ViImplementation2 * createVi () const;

	Record configuration_p;
	ViImplementation2 *inputVii_p;;
};

//////////////////////////////////////////////////////////////////////////
// ConvolutionTransformEngine class
//////////////////////////////////////////////////////////////////////////


template<class T> class ConvolutionKernel; // Forward declaration

template<class T> class ConvolutionTransformEngine : public FreqAxisTransformEngine<T>
{

public:

	ConvolutionTransformEngine(ConvolutionKernel<T> *kernel, uInt width);

	void transform(Vector<T> &inputVector,Vector<T> &outputVector);

protected:

	uInt width_p;
	// This member has to be a pointer, otherwise there
	// are compile time problems due to the fact that
	// it is a pure virtual class.
	ConvolutionKernel<T> *convolutionKernel_p;
};

//////////////////////////////////////////////////////////////////////////
// ConvolutionKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class ConvolutionKernel
{

public:

	ConvolutionKernel(Vector<Float> *convCoeff);

	virtual void kernel(	Vector<T> &inputVector,
							Vector<T> &outputVector,
							uInt startInputPos,
							uInt outputPos) = 0;

protected:

	uInt width_p;
	Vector<Float> *convCoeff_p;

};

//////////////////////////////////////////////////////////////////////////
// ConvolutionDataKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class ConvolutionDataKernel : public ConvolutionKernel<T>
{
	using ConvolutionKernel<T>::width_p;
	using ConvolutionKernel<T>::convCoeff_p;

public:

	ConvolutionDataKernel(Vector<Float> *convCoeff);

	void kernel(	Vector<T> &inputVector,
					Vector<T> &outputVector,
					uInt startInputPos,
					uInt outputPos);
};

//////////////////////////////////////////////////////////////////////////
// ConvolutionLogicalORKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class ConvolutionLogicalORKernel : public ConvolutionKernel<T>
{

	using ConvolutionKernel<T>::width_p;
	using ConvolutionKernel<T>::convCoeff_p;

public:

	ConvolutionLogicalORKernel(Vector<Float> *convCoeff);

	void kernel(	Vector<T> &inputVector,
					Vector<T> &outputVector,
					uInt startInputPos,
					uInt outputPos);
};

//////////////////////////////////////////////////////////////////////////
// ConvolutionWeightPropagationKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class ConvolutionWeightPropagationKernel : public ConvolutionKernel<T>
{

	using ConvolutionKernel<T>::width_p;
	using ConvolutionKernel<T>::convCoeff_p;

public:

	ConvolutionWeightPropagationKernel(Vector<Float> *convCoeff);

	void kernel(	Vector<T> &inputVector,
					Vector<T> &outputVector,
					uInt startInputPos,
					uInt outputPos);
};


} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* ConvolutionTVI_H_ */

