//# ConvolutionFunction.h: Definition for ConvolutionFunction
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

#ifndef SYNTHESIS_CONVOLUTIONFUNCTION_H
#define SYNTHESIS_CONVOLUTIONFUNCTION_H

#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/CFStore2.h>
#include <synthesis/TransformMachines/PolOuterProduct.h>
#include <synthesis/TransformMachines/Utils.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Vector.h>
#define CF_TYPE Double

namespace casa{
  // <summary>  
  //  The base class to compute convolution functions for convolutional gridding. 
  // </summary>
  
  // <use visibility=export>
  // <prerequisite>
  // </prerequisite>
  // <etymology>
  //   Class to encapsulate the convolution function for convolutional gridding.
  // </etymology>
  //
  // <synopsis> Standard method of re-sampling data to or from a
  //
  //  regular grid is done by convolutional gridding.  This requires a
  //  convolution function which a finte support size and well behaved
  //  function in the Fourier domain.  For standard gridding, the
  //  Prolate Spheroidal function are used.  Convolution functions
  //  used in Projection algorithms (like W-Projection, A-Projection,
  //  etc. and their combinations) each require potentially different
  //  mechanisms to compute.  These are implemented in separate
  //  classes in the Synthesis module.  Since these are used in common
  //  framework for gridding and de-gridding, these are all derived
  //  from a common base class.  ConvolutionFunction (this class) is
  //  that base class.
  //
  //  Most of the methods in this base class are pure virtual.  I.e.,
  //  only surviving offsprings (derived classes) of this class will
  //  be those that will have the wisdom that they methods represent.
  //
  // </synopsis>

  class ConvolutionFunction
  {
  public:
    ConvolutionFunction():logIO_p(), computeCFAngleRad_p(360.0*M_PI/180.0), rotateCFOTFAngleRad_p(0.1) {};
    ConvolutionFunction(Int dim): computeCFAngleRad_p(360.0*M_PI/180.0), rotateCFOTFAngleRad_p(0.1) {nDim=dim;};
    virtual ~ConvolutionFunction();
    
    // Set the dimention of the convolution function.
    virtual void setDimension(Int n){nDim = n;};

    // Given the pixel co-ordinates and an offset values, this returns
    // the value of the convolution function.  This is however not
    // used anywhere yet (and is therefore also not a pure virtual
    // function).
    virtual CF_TYPE getValue(Vector<CF_TYPE>& , Vector<CF_TYPE>& ) {return 0.0;};

    // A support function which, for now, returns and integer ID
    // corresponding to the on-sky frequency of the supplied VisBuffer.
    virtual int getVisParams(const VisBuffer& vb,const CoordinateSystem& skyCoord=CoordinateSystem())=0;

    // This method computes the convolution function and the
    // convolution function used for gridding the weights (typically
    // these are the same) and returns them in the cfs and cfwts
    // parameters.  The required information about the image and
    // visibility parameters is dervided from the given image and
    // VisBuffer objects.  wConvSize is the number of w-term planes
    // and pa is the Parallactic Angle in radians for which the
    // convolution function(s) are computed.
    virtual void makeConvFunction(const ImageInterface<Complex>& image,
				  const VisBuffer& vb,
				  const Int wConvSize,
				  const CountedPtr<PolOuterProduct>& pop,
				  const Float pa, 
				  const Float dpa, 
				  const Vector<Double>& uvScale, const Vector<Double>& uvOffset,
				  const Matrix<Double>& vbFreqSelection,
				  CFStore2& cfs,
				  CFStore2& cfwts,
				  Bool fillCF=True) = 0;
    // This method computes the average response function.  This is
    // typically image-plane equivalent of the convolution functions,
    // averaged over various axis.  The precise averaging will be
    // implementation dependent in the derived classes.
    virtual Bool makeAverageResponse(const VisBuffer& vb, 
				     const ImageInterface<Complex>& image,
				     ImageInterface<Float>& theavgPB,
				     Bool reset=True) = 0;
    virtual Bool makeAverageResponse(const VisBuffer& vb, 
				     const ImageInterface<Complex>& image,
				     ImageInterface<Complex>& theavgPB,
				     Bool reset=True) = 0;

    //
    virtual void setPolMap(const Vector<Int>& polMap) = 0;
    virtual void setSpwSelection(const Cube<Int>& spwChanSelFlag) {spwChanSelFlag_p.assign(spwChanSelFlag);}
    virtual void setSpwFreqSelection(const Matrix<Double>& spwFreqSel) {spwFreqSelection_p.assign(spwFreqSel);}
    virtual void setRotateCF(const Double& computeCFAngleRad, const Double& rotateOTF) 
    {computeCFAngleRad_p=computeCFAngleRad; rotateCFOTFAngleRad_p = rotateOTF;};

    //    virtual void setFeedStokes(const Vector<Int>& feedStokes) = 0;
    virtual Bool findSupport(Array<Complex>& func, Float& threshold,Int& origin, Int& R)=0;
    virtual Vector<Double> findPointingOffset(const ImageInterface<Complex>& image,
					      const VisBuffer& vb) = 0;

    // virtual void setParams(const Vector<Int>& polMap, const Vector<Int>& feedStokes)
    // {setPolMap(polMap); setFeedStokes(feedStokes);};

    //    virtual void prepareConvFunction(const VisBuffer& vb, CFStore2& cfs)=0;
    virtual void prepareConvFunction(const VisBuffer& vb, VBRow2CFBMapType& theMap)=0;
    virtual Matrix<Int> makeBaselineList(const Vector<Int>& antList);
    virtual Int mapAntIDToAntType(const Int& /*ant*/) {return 0;};
    virtual void setMiscInfo(const RecordInterface& /*params*/) {};
  private:
    Int nDim;
  protected:
    LogIO& logIO() {return logIO_p;}
    LogIO logIO_p;
    Cube<Int> spwChanSelFlag_p;
    Matrix<Double> spwFreqSelection_p;
    Double computeCFAngleRad_p, rotateCFOTFAngleRad_p;
  };

};

#endif
