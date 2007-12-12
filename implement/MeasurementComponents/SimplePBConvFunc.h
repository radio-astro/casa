//# SimplePBConvFunc.h: Definition for PixelatedConvFunc
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
#ifndef SYNTHESIS_SIMPLEPBCONVFUNC_H
#define SYNTHESIS_SIMPLEPBCONVFUNC_H

#include <casa/Arrays/Vector.h>
#include <synthesis/MeasurementComponents/PixelatedConvFunc.h>
#include <casa/Containers/Block.h>
#include <casa/Utilities/CountedPtr.h>

namespace casa{

  // <summary>  A class to support FTMachines get their convolution Function </summary>
  
  // <use visibility=export>
  // <prerequisite>
  //   <li> <linkto class=VisBuffer>VisBuffer</linkto> module
// </prerequisite>
  // <etymology>
  // "Simple" for cases that does not need parallactic angle and or frequency 
  // dependence ...hence one convolution function per pointing
  // "PB" for primary beam based convolution function
  // </etymology>
  //
  // <synopsis> 
  // FTMachines like WProjection and MosaicFT need convolution functions to 
  // deal with directional dependent issues...
  // this class and related ones provide and cache  such functions for re-use 
  //</synopsis>
  //Forward declarations
  template<class T> class ImageInterface;
  template<class T> class Matrix;
  class VisBuffer;
  class SkyJones;

  class SimplePBConvFunc : public PixelatedConvFunc<Complex>
    {
    public:
      SimplePBConvFunc();
      ~SimplePBConvFunc();
      // Inputs are the image, visbuffer, convSampling and skyjones
      // findconv return a cached convvolution function appropriate for this 
      // visbuffer and skyjones
      void findConvFunction(const ImageInterface<Complex>& iimage, 
			    const VisBuffer& vb,const Int& convSampling,
			    SkyJones& sj,
			    Matrix<Complex>& convFunc, 
			    Matrix<Complex>& weightConvFunc, Int& convsize,
			    Int& convSupport);
    private:
      Bool checkPBOfField(const VisBuffer& vb);
      SimpleOrderedMap <String, Int> convFunctionMap_p;
      Int actualConvIndex_p;

      Matrix<Complex> convFunc_p;
      Matrix<Complex> weightConvFunc_p;
      Int convSize_p; 
      Int convSupport_p;
      //These are cubes for multiple PA
      //May need a per antenna one if each antenna has its own.
      Block <CountedPtr<Cube<Complex> > > convFunctions_p;
      Block <CountedPtr<Cube<Complex> > > convWeights_p;
      Block<CountedPtr<Vector<Int> > > convSizes_p;
      Block <CountedPtr<Vector<Int> > > convSupportBlock_p;

    };
};// end of namespace
#endif
