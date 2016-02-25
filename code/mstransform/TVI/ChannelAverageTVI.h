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
#include <mstransform/TVI/DecimationTVI.h>


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVI class
//////////////////////////////////////////////////////////////////////////

class ChannelAverageTVI : public DecimationTVI
{

public:

	ChannelAverageTVI(	ViImplementation2 * inputVii,
						const Record &configuration);

    void flag(Cube<Bool>& flagCube) const;
    void floatData (Cube<Float> & vis) const;
    void visibilityObserved (Cube<Complex> & vis) const;
    void visibilityCorrected (Cube<Complex> & vis) const;
    void visibilityModel (Cube<Complex> & vis) const;
    void weightSpectrum(Cube<Float> &weightSp) const;
    void sigmaSpectrum (Cube<Float> &sigmaSp) const;
};

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVIFactory class
//////////////////////////////////////////////////////////////////////////

class ChannelAverageTVIFactory : public ViFactory
{

public:

	ChannelAverageTVIFactory(Record &configuration,ViImplementation2 *inputVII);

protected:

	vi::ViImplementation2 * createVi (VisibilityIterator2 *) const;
	vi::ViImplementation2 * createVi () const;

	Record configuration_p;
	ViImplementation2 *inputVii_p;;
};

//////////////////////////////////////////////////////////////////////////
// WeightedChannelAverageKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class WeightedChannelAverageKernel : public DecimationKernel<T>
{

public:

	void kernel(	Vector<T> &inputVector,
					Vector<T> &outputVector,
					DataCubeMap &auxiliaryData,
					uInt startInputPos,
					uInt outputPos,
					uInt width);
};

//////////////////////////////////////////////////////////////////////////
// LogicalANDKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class LogicalANDKernel : public DecimationKernel<T>
{

public:

	void kernel(	Vector<T> &inputVector,
					Vector<T> &outputVector,
					DataCubeMap &auxiliaryData,
					uInt startInputPos,
					uInt outputPos,
					uInt width);
};

//////////////////////////////////////////////////////////////////////////
// ChannelAccumulationKernel class
//////////////////////////////////////////////////////////////////////////

template<class T> class ChannelAccumulationKernel : public DecimationKernel<T>
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

#endif /* ChannelAverageTVI_H_ */

