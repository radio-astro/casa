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
#include <synthesis/TransformMachines/PBMathInterface.h>
#include <casa/Containers/Block.h>
#include <casa/Utilities/CountedPtr.h>

#include <wcslib/wcsconfig.h>  /** HAVE_SINCOS **/

#if HAVE_SINCOS
#define SINCOS(a,s,c) sincos(a,&s,&c)
#else
#define SINCOS(a,s,c)                   \
     s = sin(a);                        \
     c = cos(a)
#endif


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
  class CoordinateSystem;
  class DirectionCoordinate;

  class SimplePBConvFunc 
    {
    public:
      SimplePBConvFunc();
      SimplePBConvFunc(const PBMathInterface::PBClass typeToUse);
      SimplePBConvFunc(const RecordInterface& rec, Bool calcFlux_needed);
      virtual ~SimplePBConvFunc();
      // Inputs are the image, visbuffer, convSampling and skyjones
      // findconv return a cached convvolution function appropriate for this 
      // visbuffer and skyjones ...this one should be superseded 
      // by the one below and call setSkyJones when necessary
      virtual void findConvFunction(const ImageInterface<Complex>& , 
			    const VisBuffer& ,const Int& ,
			    SkyJones& ,
			    Matrix<Complex>& , 
			    Matrix<Complex>& , Int& ,
				    Int& ){};
      
      ////Returns the convfunctions in the Cubes...the Matrix rowChanMap maps 
      // the vb.row and channel 
      //to the plane of the convfunc appropriate 
      
      virtual void findConvFunction(const ImageInterface<Complex>& iimage, 
				    const VisBuffer& vb,
				    const Int& convSampling,
				    const Vector<Double>& visFreq,
				    Array<Complex>& convFunc,
				    Array<Complex>& weightConvFunc,
				    Vector<Int>& convsize,
				    Vector<Int>& convSupport,
				    Vector<Int>& polMap, Vector<Int>& chanMap, Vector<Int>& rowMap);
      virtual ImageInterface<Float>&  getFluxScaleImage();
      // slice fluxscale image by npol 
      virtual void sliceFluxScale(Int npol);
      //This is a function to just store the final weight image
      //as FT machines will share this object ...they can get share this too
      virtual void setWeightImage(CountedPtr<TempImage<Float> >& wgtimage);

      virtual void setSkyJones(SkyJones* sj);

      Bool findSupport(Array<Complex>& /*func*/, Float& /*threshold*/,Int& /*origin*/, Int& /*R*/) 
      {throw(AipsError("SimplePBConvFunc::findSupport() not implemented"));};
      virtual Bool makeAverageResponse(const VisBuffer& /*vb*/, 
				       const ImageInterface<Complex>& /*image*/,
				       ImageInterface<Float>& /*theavgPB*/,
				       Bool /*reset=True*/)
      {throw(AipsError("SimplePBConvFunc::makeAverageRes() called"));};

      //Serialization
     virtual  Bool toRecord(RecordInterface& outRec);
      //From record 
      //set calcfluxneeded to True if flux scale need to be computed
      virtual Bool fromRecord(String& err, const RecordInterface& rec, Bool calcFluxneeded=False);
      //give possibility to erase history
      virtual void reset();

    protected:
      SkyJones* sj_p;
      TempImage<Float> fluxScale_p;
      Int nx_p; 
      Int ny_p;
      Int nchan_p;
      Int npol_p;
      CoordinateSystem csys_p;
      DirectionCoordinate dc_p;
      MDirection::Convert pointToPix_p;
      MeasFrame pointFrame_p;
      MEpoch::Types timeMType_p;
      Unit timeUnit_p;
      Int directionIndex_p;
      MDirection direction1_p;
      MDirection direction2_p;
      Vector<Double> thePix_p;
      Bool filledFluxScale_p;
      Vector<Bool> doneMainConv_p;
      Bool calcFluxScale_p;
      std::map<String, Int> vbConvIndex_p;
      virtual Int convIndex(const VisBuffer& vb);
      std::map<String, Int> ant1PointVal_p;
      Vector<MDirection> ant1PointingCache_p;
      const MDirection& pointingDirAnt1(const VisBuffer& vb);
      virtual void storeImageParams(const ImageInterface<Complex>& iimage, const VisBuffer& vb);
      virtual void findUsefulChannels(Vector<Int>& chanMap, Vector<Double>& chanFreqs,  const VisBuffer& vb, const Vector<Double>& visFreq);
      //return the direction pixel corresponding to a direction
      virtual void toPix(const VisBuffer& vb);
      CountedPtr<TempImage<Float> > convWeightImage_p;
    private:
      Bool checkPBOfField(const VisBuffer& vb);
      void addPBToFlux(const VisBuffer& vb);
      SimpleOrderedMap <String, Int> convFunctionMap_p;
      Int actualConvIndex_p;
      PBMathInterface::PBClass pbClass_p;

      Matrix<Complex> convFunc_p;
      Matrix<Complex> weightConvFunc_p;
      Matrix<Complex> convSave_p;
      Matrix<Complex> weightSave_p;
      Int convSize_p; 
      Int convSupport_p;
      //These are Arrays of 5 dimension (x, y, npol, nchan, nrow)
      //Thus every baseline may have its own.
      Block <CountedPtr<Array<Complex> > > convFunctions_p;
      Block <CountedPtr<Array<Complex> > > convWeights_p;
      Block<CountedPtr<Vector<Int> > > convSizes_p;
      Block <CountedPtr<Vector<Int> > > convSupportBlock_p;
      Matrix<Bool> pointingPix_p;

      
    };
};// end of namespace
#endif
