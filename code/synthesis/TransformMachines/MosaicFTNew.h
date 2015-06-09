//# MosaicFTNew.h: Definition for MosaicFTNew
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

#ifndef SYNTHESIS_MOSAICFTNEW_H
#define SYNTHESIS_MOSAICFTNEW_H

#include <synthesis/TransformMachines/MosaicFT.h>

namespace casa { //# NAMESPACE CASA - BEGIN

/*
  class MosaicFTNew;
  class SimplePBConvFunc;

  class MPosition;
  class UVWMachine;
*/

class MosaicFTNew : public MosaicFT {
public:

  // Constructor: cachesize is the size of the cache in words
  // (e.g. a few million is a good number), tilesize is the
  // size of the tile used in gridding (cannot be less than
  // 12, 16 works in most cases). 
  // <group>
  MosaicFTNew(SkyJones* sj, MPosition mloc, String stokes,
	    Long cachesize, Int tilesize=16, 
	      Bool usezero=True, Bool useDoublePrec=False):
    MosaicFT(sj,mloc,stokes,cachesize,tilesize,usezero,useDoublePrec){}

  FTMachine* cloneFTM();


  // </group>

  // Get the final image: do the Fourier transform and
  // grid-correct, then optionally normalize by the summed weights
  ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);
 
  // Get the final weights image
  void getWeightImage(ImageInterface<Float>&, Matrix<Float>&);

  virtual String name() const {return "MosaicFTNew";};

protected:        

};

} //# NAMESPACE CASA - END

#endif
