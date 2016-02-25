//# DecimationTVI.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef DecimationTVI_H_
#define DecimationTVI_H_

// Base class
#include <mstransform/TVI/FreqAxisTVI.h>


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// DecimationTVI class
//////////////////////////////////////////////////////////////////////////

class DecimationTVI : public FreqAxisTVI
{

public:

	DecimationTVI(	ViImplementation2 * inputVii,
						const Record &configuration);

    virtual void flag(Cube<Bool>& flagCube) const = 0;
    virtual void floatData (Cube<Float> & vis) const = 0;
    virtual void visibilityObserved (Cube<Complex> & vis) const = 0;
    virtual void visibilityCorrected (Cube<Complex> & vis) const = 0;
    virtual void visibilityModel (Cube<Complex> & vis) const = 0;
    virtual void weightSpectrum(Cube<Float> &weightSp) const = 0;
    virtual void sigmaSpectrum (Cube<Float> &sigmaSp) const = 0;

    Vector<Double> getFrequencies (	Double time, Int frameOfReference,
    								Int spectralWindowId, Int msId) const;

    void writeFlag (const Cube<Bool> & flag);

protected:

    void propagateChanAvgFlags (const Cube<Bool> &avgFlagCube, Cube<Bool> &expandedFlagCube);
    Bool parseConfiguration(const Record &configuration);
    void initialize();

	Vector<Int> chanbin_p;
	mutable map<Int,uInt > spwChanbinMap_p; // Must be accessed from const methods
};

//////////////////////////////////////////////////////////////////////////
// DecimationTVIFactory class
//////////////////////////////////////////////////////////////////////////

class DecimationTVIFactory : public ViFactory
{

public:

	DecimationTVIFactory(Record &configuration,ViImplementation2 *inputVII);

protected:

	vi::ViImplementation2 * createVi (VisibilityIterator2 *) const;
	vi::ViImplementation2 * createVi () const;

	Record configuration_p;
	ViImplementation2 *inputVii_p;;
};

//////////////////////////////////////////////////////////////////////////
// DecimationTransformEngine class
//////////////////////////////////////////////////////////////////////////


template<class T> class DecimationKernel; // Forward declaration

template<class T> class DecimationTransformEngine : public FreqAxisTransformEngine<T>
{

public:

	DecimationTransformEngine(DecimationKernel<T> *kernel, uInt width)
	{
		width_p = width;
		chanAvgKernel_p = kernel;
	}

	void transform(Vector<T> &inputVector,
					Vector<T> &outputVector,
					DataCubeMap &auxiliaryData)
	{

		uInt startChan = 0;
		uInt outChanIndex = 0;
		uInt tail = inputVector.size() % width_p;
		uInt limit = inputVector.size() - tail;
		while (startChan < limit)
		{
			chanAvgKernel_p->kernel(inputVector,outputVector,auxiliaryData,
									startChan,outChanIndex,width_p);
			startChan += width_p;
			outChanIndex += 1;
		}

		if (tail and (outChanIndex <= outputVector.size()-1) )
		{
			chanAvgKernel_p->kernel(inputVector,outputVector,auxiliaryData,
									startChan,outChanIndex,tail);
		}

		return;
	}


protected:

	uInt width_p;
	// This member has to be a pointer, otherwise there
	// are compile time problems due to the fact that
	// it is a pure virtual class.
	DecimationKernel<T> *chanAvgKernel_p;
};

//////////////////////////////////////////////////////////////////////////
// DecimationKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class DecimationKernel
{

public:

	virtual void kernel(	Vector<T> &inputVector,
							Vector<T> &outputVector,
							DataCubeMap &,
							uInt startInputPos,
							uInt outputPos,
							uInt width) = 0;

};

//////////////////////////////////////////////////////////////////////////
// PlainDecimationKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class PlainDecimationKernel : public DecimationKernel<T>
{

public:

	void kernel(	Vector<T> &inputVector,
					Vector<T> &outputVector,
					DataCubeMap &auxiliaryData,
					uInt startInputPos,
					uInt outputPos,
					uInt width);
};


} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* DecimationTVI_H_ */

