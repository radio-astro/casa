//# FreqAxisTVI.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef FreqAxisTVI_H_
#define FreqAxisTVI_H_

// Base class
#include <msvis/MSVis/TransformingVi2.h>

// VI/VB framework
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisibilityIterator2.h>

// TVI framework
#include <mstransform/TVI/UtilsTVI.h>

// Measurement Set
#include <casacore/ms/MSSel/MSSelection.h>

// NOTE: See implementation include below


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// FreqAxisTVI class
//////////////////////////////////////////////////////////////////////////

template<class T> class FreqAxisTransformEngine; // Forward declaration

class FreqAxisTVI : public TransformingVi2
{

public:

	// Lifecycle
	FreqAxisTVI(ViImplementation2 * inputVii,const Record &configuration);

	// Navigation methods
	void origin ();
	void next ();

	// General TVI info (common for all sub-classes)
    Bool existsColumn (VisBufferComponent2 id) const;
    Bool flagCategoryExists () const {return False;}
    Bool weightSpectrumExists () const {return True;}
    Bool sigmaSpectrumExists () const {return True;}

	// List of methods that should be implemented by derived classes
    virtual void flag(Cube<Bool>& flagCube) const = 0;
    virtual void floatData (Cube<Float> & vis) const = 0;
    virtual void visibilityObserved (Cube<Complex> & vis) const = 0;
    virtual void visibilityCorrected (Cube<Complex> & vis) const = 0;
    virtual void visibilityModel (Cube<Complex> & vis) const = 0;
    virtual void weightSpectrum(Cube<Float> &weightSp) const = 0;
    virtual void sigmaSpectrum (Cube<Float> &sigmaSp) const = 0;
    virtual void writeFlag (const Cube<Bool> & flag) = 0;
    virtual void writeFlagRow (const Vector<Bool> & rowflags) = 0;

    // Common transformation for all sub-classes
    Vector<Int> getChannels (	Double time, Int frameOfReference,
    							Int spectralWindowId, Int msId) const;
    void flagRow (Vector<Bool> & flagRow) const;
    void weight (Matrix<Float> & weight) const;
    void sigma (Matrix<Float> & sigma) const;

protected:

    // Method implementing main loop
	template <class T> void transformFreqAxis(	Cube<T> const &inputDataCube,
												Cube<T> &outputDataCube,
												DataCubeMap &auxiliaryData,
												FreqAxisTransformEngine<T> &transformer) const;

	LogIO logger_p;
	String spwSelection_p;
	mutable map<Int,uInt > spwOutChanNumMap_p; // Must be accessed from const methods
	mutable map<Int,vector<Int> > spwInpChanIdxMap_p; // Must be accessed from const methods

private:

	Bool parseConfiguration(const Record &configuration);
	void initialize();
};

//////////////////////////////////////////////////////////////////////////
// FreqAxisTransformEngine class
//////////////////////////////////////////////////////////////////////////

template<class T> class FreqAxisTransformEngine
{

public:

	virtual void transform(Vector<T> &inputVector,
							Vector<T> &outputVector,
							DataCubeMap &auxiliaryData) = 0;
};

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

// Include implementation file to avoid compile problems
// due to the fact that the compiler does not now which
// version of the template it has to instantiate
#include <mstransform/TVI/FreqAxisTVI.cc>

#endif /* FreqAxisTVI_H_ */

