//# ATerm.h: Definition for ATerm
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

#ifndef SYNTHESIS_NOOPATERM_H
#define SYNTHESIS_NOOPATERM_H


#include <casa/Arrays/Vector.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/Containers/Block.h>
#include <synthesis/TransformMachines/CFTerms.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/CFStore2.h>

namespace casa{
  // <summary>  
  //  The base class to represent the Aperture-Term of the Measurement Equation. 
  // </summary>
  
  // <use visibility=export>
  // <prerequisite>
  // </prerequisite>
  // <etymology>
  //   A NoOp A-Term which does nothing but keeps the framework unchanged.  Used when no A-term corrections are required.
  // </etymology>
  //
  // <synopsis> 
  // 
  //</synopsis>
  class NoOpATerm: public ATerm
  {
  public:
    NoOpATerm (): ATerm() {};
    virtual ~NoOpATerm () {};

    virtual String name() {return String("NoOpATerm");}

    virtual void applySky(ImageInterface<Float>& ,//outputImages,
			  const VisBuffer& ,//vb, 
			  const Bool ,//doSquint=True,
			  const Int& ,//cfKey=0,
			  const Double // freqVal=-1
			  ) 
    {};
    virtual void applySky(ImageInterface<Complex>& ,//outputImages,
			  const VisBuffer& ,//vb, 
			  const Bool ,//doSquint=True,
			  const Int& ,//cfKey=0,
			  const Double // freqVal=-1
			  ) 
    {};

    virtual void normalizeImage(Lattice<Complex>& ,//skyImage,
				const Matrix<Float>& // weights
				) 
    {};

    virtual int getVisParams(const VisBuffer& ,// vb
			     const CoordinateSystem& // skyCoord=CoordinateSystem()
			     ) 
    {return 0;};
    
    virtual void rotate(const VisBuffer& ,//vb, 
			CFCell&, // cfs
			const Double& // rotAngle
			) 
    {};
    //
    // As the name indicates, this class should always return True
    //
    virtual Bool isNoOp() {return True;};

    //
    // Method used in the framework for other CFTerms as well.  These are now all in the base class.
    //
    // virtual Int getConvSize() {};
    // virtual Vector<Int> vbRow2CFKeyMap(const VisBuffer& vb, Int& nUnique) 
    //     {Vector<Int> tmp; tmp.resize(vb.nRow()); tmp=0; nUnique=1; return tmp;}

    // virtual Int makePBPolnCoords(const VisBuffer& vb,
    // 				 const Int& convSize,
    // 				 const Int& convSampling,
    // 				 const CoordinateSystem& skyCoord,
    // 				 const Int& skyNx, const Int& skyNy,
    // 				 CoordinateSystem& feedCoord) {throw(AipsError("NoOpATerm::makePBPolnCoords() called"));};

    // virtual Float getConvWeightSizeFactor()           {return 1.0;};
    // virtual Int getOversampling()                     {return 20;};
    virtual Float getSupportThreshold()               {return 1e-3;};
    // virtual Int mapAntIDToAntType(const Int& /*ant*/) {return 0;};
    // virtual void setPolMap(const Vector<Int>& polMap) {polMap_p_base.resize(0);polMap_p_base=polMap;}
    // virtual void getPolMap(Vector<Int>& polMap)       {polMap.resize(0); polMap = polMap_p_base;};
    // virtual Vector<Int> getAntTypeList()              {Vector<Int> tt(1); tt=0;return tt;};
  };

};

#endif
