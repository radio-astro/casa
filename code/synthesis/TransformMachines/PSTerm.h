//# PSTerm.h: Definition for PSTerm
//# Copyright (C) 2007
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

#ifndef SYNTHESIS_PSTERM_H
#define SYNTHESIS_PSTERM_H


#include <casa/Arrays/Vector.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/CFTerms.h>

namespace casa{
  // <summary>  A class to apply the W-Term to the given image </summary>
  
  // <use visibility=export>
  // <prerequisite>
  // </prerequisite>
  // <etymology>
  // W-Term to account for non co-planar baselines
  // </etymology>
  //
  // <synopsis> 
  // 
  //</synopsis>
  class PSTerm: public CFTerms
  {
  public:
    PSTerm(): CFTerms() {};
    PSTerm (const IPosition shape, 
	    const Vector<Double>& uvScale,
	    const Vector<Double>& uvOffset,
	    const Double& psScale):psCtor_p() 
    {init(shape,uvScale,uvOffset,psScale);};

    virtual ~PSTerm () {};

    void init(const IPosition shape, 
	      const Vector<Double>& uvScale,
	      const Vector<Double>& uvOffset,
	      const Double& psScale);
    Matrix<Complex>& operator=(Matrix<Complex>&);
    Matrix<Complex>& operator*=(Matrix<Complex>&);

    void applySky(Matrix<Complex>& screen,Bool multiply=False);
    void applySky(Matrix<Complex>& screen, 
		  const Vector<Double>& sampling,
		  const Int inner);
    void normalizeImage(Lattice<Complex>& skyImage,
			const Matrix<Float>& weights);
    virtual String name() {return String("PS Term");};

    //----------------------------------------------------------
    // Implementation of pure-virtual methods of CFTerms parent class.
    // CFTerms (and its derived classes) ned clean-up so that this
    // kind of code is not required everywhere).
    int getVisParams(const VisBuffer& vb,const CoordinateSystem& skyCoord=CoordinateSystem()) 
    {(void)vb;(void)skyCoord;return 0;};

    void setPolMap(const Vector<Int>& polMap) {(void)polMap;};

    virtual Float getSupportThreshold() {return 1e-3;};

    void applySky(ImageInterface<Float>& outputImages,
		  const VisBuffer& vb, 
		  const Bool doSquint=True,
		  const Int& cfKey=0,
		  const Double /*freqVal*/ =-1) 
    {(void)outputImages;(void)vb;(void)doSquint;(void)cfKey;}

    void applySky(ImageInterface<Complex>& outputImages,
		  const VisBuffer& vb, 
		  const Bool doSquint=True,
		  const Int& cfKey=0,
		  const Double /*freqVal*/ = -1) 
    {(void)outputImages;(void)vb;(void)doSquint;(void)cfKey;};

    Vector<Int> vbRow2CFKeyMap(const VisBuffer& vb, Int& nUnique) 
    {(void)vb;(void)nUnique;return Vector<Int>();};

    Int makePBPolnCoords(const VisBuffer& vb,
			 const Int& convSize,
			 const Int& convSampling,
			 const CoordinateSystem& skyCoord,
			 const Int& skyNx, const Int& skyNy,
			 CoordinateSystem& feedCoord) 
    {
      (void)vb;(void)convSize;(void)convSampling;(void)skyCoord;(void)skyNx;(void)skyNy;(void)feedCoord;
      return 0;
    };

    Int getConvSize() {return 0;};
    Int getOversampling() {return 1;};
    Float getConvWeightSizeFactor() {return 1.0;};

  private:
    // spheroidal function
    CountedPtr<ConvolveGridder<Double, Complex> > psCtor_p;
    Vector<Double>  support_p;
    Vector<Int> sampling_p;
    Double psScale_p;
  };

};

#endif
