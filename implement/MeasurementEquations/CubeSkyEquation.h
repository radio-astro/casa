//# CubeSkyEquation.h: CubeSkyEquation definition
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
//# $Id$
#ifndef SYNTHESIS_CUBESKYEQUATION_H
#define SYNTHESIS_CUBESKYEQUATION_H

#include <synthesis/MeasurementEquations/SkyEquation.h>


namespace casa { //# NAMESPACE CASA - BEGIN

//Forward

template <class T> class ImageInterface;
template <class T> class TempImage;
template <class T> class SubImage;
template <class T> class Block;

class CubeSkyEquation : public SkyEquation {

 public:
  CubeSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, ComponentFTMachine& cft, Bool noModelCol=False);
  virtual ~CubeSkyEquation();
  virtual void predict(Bool incremental=False);
  virtual void gradientsChiSquared(Bool incremental, Bool commitModel=False);
  virtual void initializePutSlice(const VisBuffer& vb, Int cubeSlice=0, Int nCubeSlice=1);
  virtual void putSlice(const VisBuffer& vb, Bool dopsf, 
			FTMachine::Type col,Int cubeSlice=0, 
			Int nCubeSlice=1);
  virtual void finalizePutSlice(const VisBuffer& vb,  
				Int cubeSlice=0, Int nCubeSlice=1);
  void initializeGetSlice(const VisBuffer& vb, Int row,
			  Bool incremental, Int cubeSlice=0, 
			  Int nCubeSlice=1);   
  virtual VisBuffer& getSlice(VisBuffer& vb, 
			      Bool incremental, Int cubeSlice=0,
			      Int nCubeSlice=1); 
  void finalizeGetSlice();
  void isLargeCube(ImageInterface<Complex>& theIm, Int& nCubeSlice);
  //void makeApproxPSF(Int model, ImageInterface<Float>& psf);
  //virtual void makeApproxPSF(Int model, ImageInterface<Float>& psf); 
  void makeApproxPSF(PtrBlock<TempImage<Float> * >& psfs);

 protected:
  Block<CountedPtr<ImageInterface<Complex> > >imGetSlice_p;
  Block<CountedPtr<ImageInterface<Complex> > >imPutSlice_p;
  Block<Matrix<Float> >weightSlice_p;
  Slicer sl_p;
  Int nchanPerSlice_p;
  // Type of copy 
  // 0 => a independent image just with coordinates gotten from cImage
  // 1 => a subImage referencing cImage ...no image copy
  void sliceCube(CountedPtr<ImageInterface<Complex> >& slice,Int model, Int cubeSlice, Int nCubeSlice, Int typeOfCopy=0); 
  void sliceCube(SubImage<Float>*& slice,ImageInterface<Float>& image, Int cubeSlice, Int nCubeSlice);
  //frequency range from image
  Bool getFreqRange(ROVisibilityIterator& vi, const CoordinateSystem& coords,
		    Int slice, Int nslice);
 private:
  // if skyjones changed in get or put we need to tell put or get respectively
  // about it
  Bool internalChangesPut_p;
  Bool internalChangesGet_p;
  Bool firstOneChangesPut_p;
  Bool firstOneChangesGet_p;

  Block< Vector<Int> >blockNumChanGroup_p, blockChanStart_p;
  Block< Vector<Int> > blockChanWidth_p, blockChanInc_p;
  Block<Vector<Int> > blockSpw_p;
  Block<CountedPtr<FTMachine> > ftm_p;
  Block<CountedPtr<FTMachine> > iftm_p;

};

} //# NAMESPACE CASA - ENDf

#endif
