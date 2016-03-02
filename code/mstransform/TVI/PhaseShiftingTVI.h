//# PhaseShiftingTVI.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef PhaseShiftingTVI_H_
#define PhaseShiftingTVI_H_

// Base class
#include <mstransform/TVI/FreqAxisTVI.h>


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// PhaseShiftingTVI class
//////////////////////////////////////////////////////////////////////////

class PhaseShiftingTVI : public FreqAxisTVI
{

public:

	PhaseShiftingTVI(	ViImplementation2 * inputVii,
						const Record &configuration);

    virtual void visibilityObserved (Cube<Complex> & vis) const;
    virtual void visibilityCorrected (Cube<Complex> & vis) const;
    virtual void visibilityModel (Cube<Complex> & vis) const;

protected:

    Bool parseConfiguration(const Record &configuration);
    void initialize();

	Double dx_p, dy_p;
};

//////////////////////////////////////////////////////////////////////////
// PhaseShiftingTVIFactory class
//////////////////////////////////////////////////////////////////////////

class PhaseShiftingTVIFactory : public ViFactory
{

public:

	PhaseShiftingTVIFactory(Record &configuration,ViImplementation2 *inputVII);

protected:

	vi::ViImplementation2 * createVi (VisibilityIterator2 *) const;
	vi::ViImplementation2 * createVi () const;

	Record configuration_p;
	ViImplementation2 *inputVii_p;;
};

//////////////////////////////////////////////////////////////////////////
// PhaseShiftingTransformEngine class
//////////////////////////////////////////////////////////////////////////

template<class T> class PhaseShiftingTransformEngine : public FreqAxisTransformEngine<T>
{

	using FreqAxisTransformEngine<T>::row_p;
	using FreqAxisTransformEngine<T>::corr_p;

public:

	PhaseShiftingTransformEngine(Double dx, Double dy,
								Matrix<Double> *uvw,
								Vector<Double> *frequencies);

	void transform(Vector<T> &inputVector,Vector<T> &outputVector);

protected:

	Double dx_p, dy_p;
	Matrix<Double> *uvw_p;
	Vector<Double> *frequencies_p;
};


} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* PhaseShiftingTVI_H_ */

