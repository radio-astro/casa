//# HetArrayConvFunc.h: Definition for HetArrayConvFunc
//# Copyright (C) 2008
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
#ifndef SYNTHESIS_HETARRAYCONVFUNC_H
#define SYNTHESIS_HETARRAYCONVFUNC_H

#include <synthesis/TransformMachines/SimplePBConvFunc.h>

namespace casa{

  // <summary>  A class to support FTMachines get their convolution Function </summary>
  
  // <use visibility=export>
  // <prerequisite>
  //   <li> <linkto class=VisBuffer>VisBuffer</linkto> module
  // </prerequisite>
  // <etymology>
  // "HetArray" for Heterogeneous Array => different dish sizes
  // "ConvFunc" for Convolution Functions 
  //  appropriate convfunctions for each pair of antenna generated and cached
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
  class MosaicFT;
  class HetArrayConvFunc : public SimplePBConvFunc

  {
  public:
    HetArrayConvFunc();
    HetArrayConvFunc(const PBMathInterface::PBClass 
		     typeToUse, const String vpTable="");
    //Constructor from record
    //if for prediction only no need to recover fluxscale
    HetArrayConvFunc(const RecordInterface& rec, Bool calcFluxscale);
    virtual ~HetArrayConvFunc();

    //Returns the convfunctions in the Arrays...the rowMap maps the vb.row 
    //to the  plane of the convfunc appropriate...chanMap and polMap similarly 

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
    // slice flux scale images 
    virtual void sliceFluxScale(const Int npol);
    //Serialization
   virtual Bool toRecord(RecordInterface& rec);
   virtual Bool fromRecord(String& err, const RecordInterface& rec, Bool calcFluxscale=False);
   virtual void reset();

    //----------------------------------------------

    private:
   void applyGradientToYLine(const Int iy, Complex*& convFunctions, 
			     Complex*& convWeights, const Double pixXdir, const Double pixYdir, 
			     Int convSize, const Int ndishpair, const Int nchan, const Int nPol);
      Int factorial(Int n);
      // the return value are -1 or False for not in cache yet but pointing direction 
      //seems to be inside image
      // 1 if value is cached..we have stopped caching..so it should not return this value
      // 2 pointing is off image ...thus valid but not useful
      Int checkPBOfField(const VisBuffer& vb, Vector<Int>& rowMap);
      void findAntennaSizes(const VisBuffer& vb);
      void supportAndNormalize(Int plane, Int convSampling);
      void supportAndNormalizeLatt(Int plane, Int convSampling, TempLattice<Complex>& convFuncLat,
				   TempLattice<Complex>& weightConvFuncLat);
      void init(const PBMathInterface::PBClass typeToUse);
      void makerowmap(const VisBuffer& vb, Vector<Int>& rowMap);
      PBMathInterface::PBClass pbClass_p;
      //SimpleOrderedMap <String, Int> convFunctionMap_p;
      Vector<Int64> convFunctionMap_p;
      Int64 nDefined_p;
      SimpleOrderedMap <String, Int> antDiam2IndexMap_p;
      Vector<Int> antIndexToDiamIndex_p;
      Block<CountedPtr<PBMathInterface> > antMath_p;
      Int msId_p;
      Int actualConvIndex_p;
      Array<Complex> convFunc_p;
      Array<Complex> weightConvFunc_p;
      Array<Complex> convSave_p;
      Array<Complex> weightSave_p;
      Int convSize_p; 
      String vpTable_p;
      Vector<Int> convSupport_p;
      Block <CountedPtr<Array<Complex> > > convFunctions_p;
      Block <CountedPtr<Array<Complex> > > convWeights_p;
      Block<CountedPtr<Vector<Int> > > convSizes_p;
      Block <CountedPtr<Vector<Int> > > convSupportBlock_p;

    };
}; // end of namespace casa

#endif
