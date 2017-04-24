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

#ifndef STATWTTVI_H_
#define STATWTTVI_H_

#include <msvis/MSVis/TransformingVi2.h>

#include <casacore/ms/MSSel/MSSelection.h>

#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <mstransform/TVI/UtilsTVI.h>

namespace casa {

namespace vi {

class StatWtTVI : public TransformingVi2 {

public:

	// Lifecycle
	StatWtTVI(ViImplementation2 * inputVii, const casacore::Record &configuration);
	~StatWtTVI();

	// Navigation methods
	void origin();
	void next();

    virtual void weightSpectrum(casacore::Cube<casacore::Float>& wtsp) const;

    virtual void weight(casacore::Matrix<casacore::Float> & wtmat) const;
    
    virtual void flag(casacore::Cube<casacore::Bool>& flagCube) const;

    virtual void flagRow (casacore::Vector<casacore::Bool> & flagRow) const;

    // virtual void scan (casacore::Vector<casacore::Int>& scans) const;
    // General TVI info (common for all sub-classes)
    //casacore::Bool existsColumn (VisBufferComponent2 id) const;
    //casacore::Bool flagCategoryExists() const {return false;}

	// casacore::List of methods that should be implemented by derived classes
    // virtual void floatData (casacore::Cube<casacore::Float> & vis) const = 0;
    // virtual void visibilityObserved (casacore::Cube<casacore::Complex> & vis) const = 0;
    // virtual void visibilityCorrected (casacore::Cube<casacore::Complex> & vis) const = 0;
    // virtual void visibilityModel (casacore::Cube<casacore::Complex> & vis) const = 0;
    // virtual void sigmaSpectrum (casacore::Cube<casacore::Float> &sigmaSp) const = 0;
    // virtual casacore::Vector<casacore::Double> getFrequencies (	casacore::Double time, casacore::Int frameOfReference,casacore::Int spectralWindowId, casacore::Int msId) const = 0;
    // virtual void writeFlag (const casacore::Cube<casacore::Bool> & flagCube) = 0;

    // Common transformation for all sub-classes
    /*
    void writeFlagRow (const casacore::Vector<casacore::Bool>& flag);

    casacore::Vector<casacore::Int> getChannels(
        casacore::Double time, casacore::Int frameOfReference,
    	casacore::Int spectralWindowId, casacore::Int msId
    ) const;
    
    
    void weight (casacore::Matrix<casacore::Float> & weight) const;
    
    void sigma (casacore::Matrix<casacore::Float> & sigma) const;
*/
private:

    mutable casacore::Bool _weightsComputed = false;
    mutable casacore::Bool _wtSpExists = true;
    mutable casacore::Cube<casacore::Float> _newWtSp;
    mutable casacore::Matrix<casacore::Float> _newWt;
    mutable casacore::Cube<casacore::Bool> _newFlag;
    mutable casacore::Vector<casacore::Bool> _newFlagRow;

    casacore::String spwSelection_p;
    mutable casacore::LogIO logger_p;
    mutable map<casacore::Int,casacore::uInt > spwOutChanNumMap_p; // Must be accessed from const methods
    mutable map<casacore::Int,vector<casacore::Int> > spwInpChanIdxMap_p; // Must be accessed from const methods

    void _computeNewWeights() const;

    casacore::Bool _parseConfiguration(const casacore::Record &configuration);
	
    void _initialize();

	// Form spwInpChanIdxMap_p via calls to underlying Vii
	void _formSelectedChanMap();

};

}

}

#endif 

