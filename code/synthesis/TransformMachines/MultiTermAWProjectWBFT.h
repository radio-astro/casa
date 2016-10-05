//# MultiTermAWProjectWBFT.h: Definition for MultiTermAWProjectWBFT
//# Copyright (C) 1996,1997,1998,1999,2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_MULTITERMAWPROJECTWBFT_H
#define SYNTHESIS_MULTITERMAWPROJECTWBFT_H
#define DELTAPA 1.0
#define MAGICPAVALUE -999.0

#include <synthesis/MeasurementComponents/AWProjectWBFT.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class MultiTermAWProjectWBFT : public AWProjectWBFT {

  public:
    //
    // Constructor: cachesize is the size of the cache in words
    // (e.g. a few million is a good number), tilesize is the size of
    // the tile used in gridding (cannot be less than 12, 16 works in
    // most cases).  
    // <group>
    //
    MultiTermAWProjectWBFT(casacore::Int nFacets, casacore::Long cachesize,
			   casacore::CountedPtr<CFCache>& cfcache,
			   casacore::CountedPtr<ConvolutionFunction>& cf,
			   casacore::Bool applyPointingOffset=true,
			   casacore::Bool doPBCorr=true,
			   casacore::Int tilesize=16, 
			   casacore::Float paSteps=5.0, 
			   casacore::Float pbLimit=5e-2,
			   casacore::Bool usezero=false) 
    {};
    // </group>
    
    // Construct from a casacore::Record containing the AWProjectWBFT state
    MultiTermAWProjectWBFT(const casacore::RecordInterface& stateRec) 
    {};
    
    // Assignment operator
    MultiTermAWProjectWBFT &operator=(const MultiTermAWProjectWBFT &other) 
    {};
    
    ~MultiTermAWProjectWBFT() 
    {};

    //
    // Given the sky image (Fourier transform of the visibilities),
    // sum of weights and the sensitivity image, this method replaces
    // the skyImage with the normalized image of the sky.
    //
    virtual void normalizeImage(casacore::Lattice<casacore::Complex>& skyImage,
				const casacore::Matrix<casacore::Double>& sumOfWts,
				casacore::Lattice<casacore::Float>& sensitivityImage,
				casacore::Bool fftNorm=true) 
    {};
    //
    // In AWProjectWBFT and its derivatives, sensitivity image is
    // computed by accumulating weight functions (images) during the
    // first gridding cycle.  AWProjectFT::makeSensitivityImage() is
    // overloaded in AWProjectWBFT and only issues a log message.
    //
    // The following method is used to Fourier transform normalize the
    // accumulated weight images.  doFFTNorm when true, the FFT
    // normalization (by pixel volume) is also done.
    //
    virtual void makeSensitivityImage(casacore::Lattice<casacore::Complex>& wtImage,
				      casacore::ImageInterface<casacore::Float>& sensitivityImage,
				      const casacore::Matrix<casacore::Float>& sumWt=casacore::Matrix<casacore::Float>(),
				      const casacore::Bool& doFFTNorm=true)
    {};
    //
    // Method used to make normalized image from gridded visibilites.
    // This calls makeSensitivityImage() to make the sensitivity image
    // and AWProjectFT::getImage() to make the image from gridded
    // visibilites.  AWProjectFT::getImage() internally calls
    // normalizeImage() which uses the sensitivty image computed by
    // makeSensitivtyImage().
    //
    virtual casacore::ImageInterface<casacore::Complex>& getImage(casacore::Matrix<casacore::Float>&, casacore::Bool normalize=true)
    {};

    //
    // Returns true if accumulation during gridding to compute the
    // average PB must be done.
    //
    virtual casacore::Bool computeAvgPB(const casacore::Double& actualPA, const casacore::Double& lastPAUsedForWtImg) 
    {return (avgPBReady_p==false);};

    virtual casacore::String name() const {return "MultiTermAWProjectWBFT";};

    void normalizeAvgPB(casacore::ImageInterface<casacore::Complex>& inImage, casacore::ImageInterface<casacore::Float>& outImage) 
    {throw(casacore::AipsError("MTAWPWBFT::normalizeAvgPB(Complex,Float)"));}
  };
} //# NAMESPACE CASA - END

#endif
