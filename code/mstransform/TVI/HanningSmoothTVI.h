//# HanningSmoothTVI.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef HanningSmoothTVI_H_
#define HanningSmoothTVI_H_

// Base class
#include <mstransform/TVI/ConvolutionTVI.h>


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// HanningSmoothTVI class
//////////////////////////////////////////////////////////////////////////

class HanningSmoothTVI : public ConvolutionTVI
{

public:

	HanningSmoothTVI(	ViImplementation2 * inputVii,
						const casacore::Record &configuration);

	// Report the the ViImplementation type
	virtual casacore::String ViiType() const { return casacore::String("HanningSmooth( ")+getVii()->ViiType()+" )"; };

protected:

	void initialize();
};

//////////////////////////////////////////////////////////////////////////
// HanningSmoothTVIFactory class
//////////////////////////////////////////////////////////////////////////

class HanningSmoothTVIFactory : public ViFactory
{

public:

	HanningSmoothTVIFactory(casacore::Record &configuration,ViImplementation2 *inputVII);

protected:

	vi::ViImplementation2 * createVi (VisibilityIterator2 *) const;
	vi::ViImplementation2 * createVi () const;

	casacore::Record configuration_p;
	ViImplementation2 *inputVii_p;;
};

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* HanningSmoothTVI_H_ */

