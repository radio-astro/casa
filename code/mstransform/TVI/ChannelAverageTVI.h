//# ChannelAverageTVI.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef ChannelAverageTVI_H_
#define ChannelAverageTVI_H_

// Base class
#include <mstransform/TVI/FreqAxisTVI.h>


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVI class
//////////////////////////////////////////////////////////////////////////

class ChannelAverageTVI : public FreqAxisTVI
{

public:

	ChannelAverageTVI(ViImplementation2 * inputVii, const casacore::Record &configuration);
	// Report the the ViImplementation type
	virtual casacore::String ViiType() const { return casacore::String("ChannelAverage( ")+getVii()->ViiType()+" )"; };


    void flag(casacore::Cube<casacore::Bool>& flagCube) const;
    void floatData (casacore::Cube<casacore::Float> & vis) const;
    void visibilityObserved (casacore::Cube<casacore::Complex> & vis) const;
    void visibilityCorrected (casacore::Cube<casacore::Complex> & vis) const;
    void visibilityModel (casacore::Cube<casacore::Complex> & vis) const;
    void weightSpectrum(casacore::Cube<casacore::Float> &weightSp) const;
    void sigmaSpectrum (casacore::Cube<casacore::Float> &sigmaSp) const;

    casacore::Bool weightSpectrumExists () const {return true;}
    casacore::Bool sigmaSpectrumExists () const {return true;}
    
    casacore::Vector<casacore::Double> getFrequencies (	casacore::Double time, casacore::Int frameOfReference,
    								casacore::Int spectralWindowId, casacore::Int msId) const;


    void writeFlag (const casacore::Cube<casacore::Bool> & flag);

protected:

    void propagateChanAvgFlags (const casacore::Cube<casacore::Bool> &avgFlagCube, casacore::Cube<casacore::Bool> &expandedFlagCube);
    casacore::Bool parseConfiguration(const casacore::Record &configuration);
    void initialize();

	casacore::Vector<casacore::Int> chanbin_p;
	mutable map<casacore::Int,casacore::uInt > spwChanbinMap_p; // Must be accessed from const methods
};

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVIFactory class
//////////////////////////////////////////////////////////////////////////

class ChannelAverageTVIFactory : public ViFactory
{

public:

	ChannelAverageTVIFactory(casacore::Record &configuration,ViImplementation2 *inputVII);

protected:

	vi::ViImplementation2 * createVi (VisibilityIterator2 *) const;
	vi::ViImplementation2 * createVi () const;

	casacore::Record configuration_p;
	ViImplementation2 *inputVii_p;;
};

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVILayerFactory class  (for _recursive_ layering)
//////////////////////////////////////////////////////////////////////////

class ChannelAverageTVILayerFactory : public ViiLayerFactory
{

public:

  ChannelAverageTVILayerFactory(casacore::Record &configuration);

  virtual ~ChannelAverageTVILayerFactory() {};

protected:

  
  virtual ViImplementation2 * createInstance(ViImplementation2* vii0) const;

  const casacore::Record configuration_p;

};

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTransformEngine class
//////////////////////////////////////////////////////////////////////////


template<class T> class ChannelAverageKernel; // Forward declaration

template<class T> class ChannelAverageTransformEngine : public FreqAxisTransformEngine2<T>
{

	using FreqAxisTransformEngine2<T>::inputData_p;
	using FreqAxisTransformEngine2<T>::outputData_p;

public:

	ChannelAverageTransformEngine	(	ChannelAverageKernel<T> *kernel,
										DataCubeMap *inputData,
										DataCubeMap *outputData,
										casacore::uInt width);

	void transform();

protected:

	casacore::uInt width_p;
	// This member has to be a pointer, otherwise there
	// are compile time problems due to the fact that
	// it is a pure virtual class.
	ChannelAverageKernel<T> *chanAvgKernel_p;
};

//////////////////////////////////////////////////////////////////////////
// ChannelAverageKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class ChannelAverageKernel
{

public:

	virtual void kernel(DataCubeMap *inputData,
						DataCubeMap *outputData,
						casacore::uInt startInputPos,
						casacore::uInt outputPos,
						casacore::uInt width) = 0;
};

//////////////////////////////////////////////////////////////////////////
// PlainChannelAverageKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class PlainChannelAverageKernel : public ChannelAverageKernel<T>
{

public:

	void kernel(	DataCubeMap *inputData,
					DataCubeMap *outputData,
					casacore::uInt startInputPos,
					casacore::uInt outputPos,
					casacore::uInt width);

};

//////////////////////////////////////////////////////////////////////////
// WeightedChannelAverageKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class WeightedChannelAverageKernel : public ChannelAverageKernel<T>
{

public:

	void kernel(DataCubeMap *inputData,
				DataCubeMap *outputData,
				casacore::uInt startInputPos,
				casacore::uInt outputPos,
				casacore::uInt width);
};

//////////////////////////////////////////////////////////////////////////
// LogicalANDKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class LogicalANDKernel : public ChannelAverageKernel<T>
{

public:

	void kernel(DataCubeMap *inputData,
				DataCubeMap *outputData,
				casacore::uInt startInputPos,
				casacore::uInt outputPos,
				casacore::uInt width);
};

//////////////////////////////////////////////////////////////////////////
// ChannelAccumulationKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class ChannelAccumulationKernel : public ChannelAverageKernel<T>
{

public:

	void kernel(DataCubeMap *inputData,
				DataCubeMap *outputData,
				casacore::uInt startInputPos,
				casacore::uInt outputPos,
				casacore::uInt width);
};

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* ChannelAverageTVI_H_ */

