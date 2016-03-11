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
	~FreqAxisTVI();

	// Navigation methods
	virtual void origin ();
	virtual void next ();

	// General TVI info (common for all sub-classes)
    Bool existsColumn (VisBufferComponent2 id) const;
    Bool flagCategoryExists () const {return False;}
    Bool weightSpectrumExists () const {return True;}
    Bool sigmaSpectrumExists () const {return True;}

	// List of methods that should be implemented by derived classes
    // virtual void flag(Cube<Bool>& flagCube) const = 0;
    // virtual void floatData (Cube<Float> & vis) const = 0;
    // virtual void visibilityObserved (Cube<Complex> & vis) const = 0;
    // virtual void visibilityCorrected (Cube<Complex> & vis) const = 0;
    // virtual void visibilityModel (Cube<Complex> & vis) const = 0;
    // virtual void weightSpectrum(Cube<Float> &weightSp) const = 0;
    // virtual void sigmaSpectrum (Cube<Float> &sigmaSp) const = 0;
    // virtual Vector<Double> getFrequencies (	Double time, Int frameOfReference,Int spectralWindowId, Int msId) const = 0;
    // virtual void writeFlag (const Cube<Bool> & flagCube) = 0;

    // Common transformation for all sub-classes
    void writeFlagRow (const Vector<Bool> & flag);
    Vector<Int> getChannels (	Double time, Int frameOfReference,
    							Int spectralWindowId, Int msId) const;
    void flagRow (Vector<Bool> & flagRow) const;
    void weight (Matrix<Float> & weight) const;
    void sigma (Matrix<Float> & sigma) const;

protected:

    // Method implementing main loop  (with auxiliary data)
	template <class T> void transformFreqAxis(	Cube<T> const &inputDataCube,
												Cube<T> &outputDataCube,
												FreqAxisTransformEngine<T> &transformer) const
	{
		// Re-shape output data cube
		outputDataCube.resize(getVisBufferConst()->getShape(),False);

		// Get data shape for iteration
		const IPosition &inputShape = inputDataCube.shape();
		uInt nRows = inputShape(2);
		uInt nCorrs = inputShape(0);

		// Initialize input-output planes
		Matrix<T> inputDataPlane;
		Matrix<T> outputDataPlane;

		// Initialize input-output vectors
		Vector<T> inputDataVector;
		Vector<T> outputDataVector;

		for (uInt row=0; row < nRows; row++)
		{
			// Assign input-output planes by reference
			transformer.setRowIndex(row);
			inputDataPlane.reference(inputDataCube.xyPlane(row));
			outputDataPlane.reference(outputDataCube.xyPlane(row));

			for (uInt corr=0; corr < nCorrs; corr++)
			{
				// Assign input-output vectors by reference
				transformer.setCorrIndex(corr);
				inputDataVector.reference(inputDataPlane.row(corr));
				outputDataVector.reference(outputDataPlane.row(corr));

				// Transform data
				transformer.transform(inputDataVector,outputDataVector);
			}
		}

		return;
	}

	Bool parseConfiguration(const Record &configuration);
	void initialize();

	String spwSelection_p;
	mutable LogIO logger_p;
	mutable map<Int,uInt > spwOutChanNumMap_p; // Must be accessed from const methods
	mutable map<Int,vector<Int> > spwInpChanIdxMap_p; // Must be accessed from const methods
};

//////////////////////////////////////////////////////////////////////////
// FreqAxisTransformEngine class
//////////////////////////////////////////////////////////////////////////

template<class T> class FreqAxisTransformEngine
{

public:

	virtual void transform(Vector<T> &inputVector,Vector<T> &outputVector) = 0;
	virtual void setRowIndex(uInt row) {row_p = row;}
	virtual void setCorrIndex(uInt corr) {corr_p = corr;}

protected:

	uInt row_p;
	uInt corr_p;

};

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* FreqAxisTVI_H_ */

