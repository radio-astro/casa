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
    MultiTermAWProjectWBFT(Int nFacets, Long cachesize,
			   CountedPtr<CFCache>& cfcache,
			   CountedPtr<ConvolutionFunction>& cf,
			   Bool applyPointingOffset=True,
			   Bool doPBCorr=True,
			   Int tilesize=16, 
			   Float paSteps=5.0, 
			   Float pbLimit=5e-2,
			   Bool usezero=False) 
    {};
    // </group>
    
    // Construct from a Record containing the AWProjectWBFT state
    MultiTermAWProjectWBFT(const RecordInterface& stateRec) 
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
    virtual void normalizeImage(Lattice<Complex>& skyImage,
				const Matrix<Double>& sumOfWts,
				Lattice<Float>& sensitivityImage,
				Bool fftNorm=True) 
    {};
    //
    // In AWProjectWBFT and its derivatives, sensitivity image is
    // computed by accumulating weight functions (images) during the
    // first gridding cycle.  AWProjectFT::makeSensitivityImage() is
    // overloaded in AWProjectWBFT and only issues a log message.
    //
    // The following method is used to Fourier transform normalize the
    // accumulated weight images.  doFFTNorm when True, the FFT
    // normalization (by pixel volume) is also done.
    //
    virtual void makeSensitivityImage(Lattice<Complex>& wtImage,
				      ImageInterface<Float>& sensitivityImage,
				      const Matrix<Float>& sumWt=Matrix<Float>(),
				      const Bool& doFFTNorm=True)
    {};
    //
    // Method used to make normalized image from gridded visibilites.
    // This calls makeSensitivityImage() to make the sensitivity image
    // and AWProjectFT::getImage() to make the image from gridded
    // visibilites.  AWProjectFT::getImage() internally calls
    // normalizeImage() which uses the sensitivty image computed by
    // makeSensitivtyImage().
    //
    virtual ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True)
    {};

    //
    // Returns True if accumulation during gridding to compute the
    // average PB must be done.
    //
    virtual Bool computeAvgPB(const Double& actualPA, const Double& lastPAUsedForWtImg) 
    {return (avgPBReady_p==False);};

    virtual String name() const {return "MultiTermAWProjectWBFT";};

    void normalizeAvgPB(ImageInterface<Complex>& inImage, ImageInterface<Float>& outImage) 
    {throw(AipsError("MTAWPWBFT::normalizeAvgPB(Complex,Float)"));}
  };
} //# NAMESPACE CASA - END

#endif
