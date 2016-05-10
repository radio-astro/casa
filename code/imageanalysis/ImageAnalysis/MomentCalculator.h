//# MomentCalculator.h: 
//# Copyright (C) 1997,1999,2000,2001,2002
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: MomentCalculator.h 20299 2008-04-03 05:56:44Z gervandiepen $

#ifndef IMAGES_MOMENTCALCULATOR_H
#define IMAGES_MOMENTCALCULATOR_H

//# Includes
#include <casa/aips.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <lattices/LatticeMath/LineCollapser.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Mathematics/NumericTraits.h>
#include <casa/Arrays/Vector.h>
#include <casa/Logging/LogIO.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
template <class T> class MomentsBase;

// <summary> Computes moments from a windowed profile </summary>
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <prerequisite>
//   <li> <linkto class="MomentsBase">MomentsBase</linkto>
//   <li> <linkto class="ImageMoments">ImageMoments</linkto>
//   <li> <linkto class="MSMoments">MSMoments</linkto>
//   <li> <linkto class="LatticeApply">LatticeApply</linkto>
//   <li> <linkto class="MomentCalcBase">MomentCalcBase</linkto>
//   <li> <linkto class="LineCollapser">LineCollapser</linkto>
// </prerequisite>
//
// <synopsis>
//  This concrete class is derived from the abstract base class MomentCalcBase
//  which provides an interface layer to the ImageMoments or MSMoments driver class.
//  ImageMoments or MSMoments creates a MomentWindow object and passes it to the LatticeApply
//  function lineMultiApply.  This function iterates through a given lattice, 
//  and invokes the <src>multiProcess</src> member function of MomentWindow on each profile
//  of pixels that it extracts from the input lattice.  The <src>multiProcess</src> function 
//  returns a vector of moments which are inserted into the output lattices also
//  supplied to the LatticeApply function.
//
//  MomentWindow computes moments from a subset of the pixels selected from  the
//  input profile.  This subset is a simple index range, or window.  The window is
//  selected, for each profile, that is thought to surround the spectral feature 
//  of interest.  This window can be found from the primary lattice, or from an 
//  ancilliary lattice (ImageMoments or MSMoments offers a smoothed version of the primary 
//  lattice as the ancilliary lattice).  The moments are always computed from 
//  primary lattice data.   
//
//  For each profile, the window can be found either interactively or automatically.
//  There are two interactive methods.  Either you just mark the window with the
//  cursor, or you interactively fit a Gaussian to the profile and the +/- 3-sigma
//  window is returned.  There are two automatic methods.  Either Bosma's converging
//  mean algorithm is used, or an automatically  fit Gaussian +/- 3-sigma window
//  is returned.
// 
//  The constructor takes an MomentsBase object that is actually an ImageMoments or 
//  an MSMoments object; the one that is constructing
//  the MomentWindow object of course.   There is much control information embodied  
//  in the state  of the ImageMoments or MSMoments object.  This information is extracted by the
//  MomentCalcBase class and passed on to MomentWindow for consumption.
//
//  Note that the ancilliary lattice is only accessed if the pointer to it
//  is non zero.
//  
//  See the <linkto class="MomentsBase">MomentsBase</linkto>, 
//  <linkto class="ImageMoments">ImageMoments</linkto>, and 
//  <linkto class="MSMoments">MSMoments</linkto>
//  for discussion about the moments that are available for computation.
//  
// </synopsis>
//
// <example>
// This example comes from ImageMoments.   outPt is a pointer block holding
// pointers to the output lattices.  The ancilliary masking lattice is
// just a smoothed version of the input lattice.  os_P is a LogIO object.
//   
// <srcBlock>
// 
//// Construct desired moment calculator object.  Use it polymorphically via 
//// a pointer to the base class.
//  
//   MomentCalcBase<T>* pMomentCalculator = 0;
//   if (clipMethod || smoothClipMethod) {
//      pMomentCalculator = new MomentClip<T>(pSmoothedImage, *this, os_p, outPt.nelements());
//   } else if (windowMethod) {
//      pMomentCalculator = new MomentWindow<T>(pSmoothedImage, *this, os_p, outPt.nelements());
//   } else if (fitMethod) {
//      pMomentCalculator = new MomentFit<T>(*this, os_p, outPt.nelements());
//   }
//  
//// Iterate optimally through the image, compute the moments, fill the output lattices
//  
//   LatticeApply<T>::lineMultiApply(outPt, *pInImage_p, *pMomentCalculator,   
//                                   momentAxis_p, pProgressMeter);
//   delete pMomentCalculator;
//  
// </srcBlock>
// </example>
//
// <motivation>
// </motivation>
//
// <note role=tip>
// Note that there are is assignment operator or copy constructor.
// Do not use the ones the system would generate either.
// </note>
//
// <todo asof="yyyy/mm/dd">
// </todo>


template <class T> class MomentWindow : public MomentCalcBase<T>
{
public:

// Constructor.  The pointer is to a lattice containing the masking
// lattice (created by ImageMoments or MSMoments).   We also need the 
// ImageMoments or MSMoments object which is calling us, its logger,
// and the number of output lattices it has created.
   MomentWindow(shared_ptr<Lattice<T>> pAncilliaryLattice,
                MomentsBase<T>& iMom,
                LogIO& os,
                const uInt nLatticeOut);

// Destructor (does nothing).
  ~MomentWindow();

// This function is not implemented and throws an exception.
   virtual void process(T& out,
                        Bool& outMask,
                        const Vector<T>& in,
                        const Vector<Bool>& inMask,
                        const IPosition& pos);

// This function returns a vector of numbers from each input vector.
// the output vector contains the moments known to the ImageMoments
// or MSMoments object passed into the constructor.
   virtual void multiProcess(Vector<T>& out,
                             Vector<Bool>& outMask,
                             const Vector<T>& in,
                             const Vector<Bool>& inMask,
                             const IPosition& pos);

                             
private:

   shared_ptr<Lattice<T>> _ancilliaryLattice;
   MomentsBase<T>& iMom_p;
   LogIO os_p;

   const Vector<T>* pProfileSelect_p;
   Vector<T> ancilliarySliceRef_p;
   Vector<T> selectedData_p;
   T stdDeviation_p, peakSNR_p;
   Bool doAuto_p, doFit_p;
   IPosition sliceShape_p;

// Automatically determine the spectral window
   Bool getAutoWindow(uInt& nFailed,
                      Vector<Int>& window,
                      const Vector<T>& x,
                      const Vector<T>& y,
                      const Vector<Bool>& mask,
                      const T peakSNR,
                      const T stdDeviation,
                      const Bool doFit) const;

// Automatically determine the spectral window via Bosma's algorithm
   Bool getBosmaWindow (Vector<Int>& window,
                        const Vector<T>& y,
                        const Vector<Bool>& mask,
                        const T peakSNR,
                        const T stdDeviation) const;

// Take the fitted Gaussian parameters and set an N-sigma window.
// If the window is too small return a Fail condition.
   Bool setNSigmaWindow(Vector<Int>& window,  
                        const T pos,
                        const T width,
                        const Int nPts,
                        const Int N) const;


  //# Make members of parent class known.
protected:
  using MomentCalcBase<T>::constructorCheck;
  using MomentCalcBase<T>::setPosLabel;
  //using MomentCalcBase<T>::convertF;
  using MomentCalcBase<T>::selectMoments_p;
  using MomentCalcBase<T>::calcMoments_p;
  using MomentCalcBase<T>::calcMomentsMask_p;
  using MomentCalcBase<T>::doMedianI_p;
  using MomentCalcBase<T>::doMedianV_p;
  using MomentCalcBase<T>::doAbsDev_p;
  using MomentCalcBase<T>::cSys_p;
  using MomentCalcBase<T>::doCoordProfile_p;
  using MomentCalcBase<T>::doCoordRandom_p;
  using MomentCalcBase<T>::pixelIn_p;
  using MomentCalcBase<T>::worldOut_p;
  using MomentCalcBase<T>::sepWorldCoord_p;
  using MomentCalcBase<T>::integratedScaleFactor_p;
  using MomentCalcBase<T>::momAxisType_p;
  using MomentCalcBase<T>::nFailed_p;
  using MomentCalcBase<T>::abcissa_p;
};



// <summary> Compute moments from a Gaussian fitted to a profile</summary>
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <prerequisite>
//   <li> <linkto class="MomentsBase">MomentsBase</linkto>
//   <li> <linkto class="ImageMoments">ImageMoments</linkto>
//   <li> <linkto class="MSMoments">MSMoments</linkto>
//   <li> <linkto class="LatticeApply">LatticeApply</linkto>
//   <li> <linkto class="MomentCalcBase">MomentCalcBase</linkto>
//   <li> <linkto class="LineCollapser">LineCollapser</linkto>
// </prerequisite>
//
// <synopsis>
//  This concrete class is derived from the abstract base class MomentCalcBase
//  which provides an interface layer to the ImageMoments or MSMoments driver class.  
//  ImageMoments or MSMoments creates a MomentFit object and passes it to the LatticeApply
//  function, lineMultiApply. This function iterates through a given lattice,
//  and invokes the <src>multiProcess</src> member function of MomentFit on each vector
//  of pixels that it extracts from the input lattice.  The <src>multiProcess</src>
//  function returns a vector of moments which are inserted into the output
//  lattices also supplied to the LatticeApply function.
// 
//  MomentFit computes moments by fitting a Gaussian to each profile.  The
//  moments are then computed from that fit.   The fitting can be done either
//  interactively or automatically.
// 
//  The constructor takes MomentsBase object that is actually an ImageMoments or 
//  an MSMoments object; the one that is constructing
//  the MomentFit object of course.   There is much control information embodied
//  in the state of the ImageMoments object.  This information is extracted by the
//  MomentCalcBase class and passed on to MomentFit for consumption.
//   
//  See the <linkto class="MomentsBase">MomentsBase</linkto>, 
//  <linkto class="ImageMoments">ImageMoments</linkto>, and 
//  <linkto class="MSMoments">MSMoments</linkto>
//  for discussion about the moments that are available for computation.
//  
// </synopsis>
//
// <example>
// This example comes from ImageMoments.   outPt is a pointer block holding
// pointers to the output lattices.   os_P is a LogIO object.
//                                     
// <srcBlock>
// 
//// Construct desired moment calculator object.  Use it polymorphically via
//// a pointer to the base class.
//
//   MomentCalcBase<T>* pMomentCalculator = 0;
//   if (clipMethod || smoothClipMethod) {
//      pMomentCalculator = new MomentClip<T>(pSmoothedImage, *this, os_p, outPt.nelements());
//   } else if (windowMethod) {
//      pMomentCalculator = new MomentWindow<T>(pSmoothedImage, *this, os_p, outPt.nelements());
//   } else if (fitMethod) {
//      pMomentCalculator = new MomentFit<T>(*this, os_p, outPt.nelements());
//   }
//
//// Iterate optimally through the image, compute the moments, fill the output lattices
//
//   LatticeApply<T>::lineMultiApply(outPt, *pInImage_p, *pMomentCalculator,   
//                                   momentAxis_p, pProgressMeter);
//   delete pMomentCalculator;
// </srcBlock>
// </example>
//
// <motivation>
// </motivation>
//
// <note role=tip>
// Note that there are is assignment operator or copy constructor.
// Do not use the ones the system would generate either.
// </note>
//
// <todo asof="yyyy/mm/dd">
// </todo>


template <class T> class MomentFit : public MomentCalcBase<T>
{
public:

// Constructor.  We need the ImageMoments or MSMoments object which is calling us, 
// its logger, and the number of output lattices it has created.
   MomentFit(MomentsBase<T>& iMom,
             LogIO& os,
             const uInt nLatticeOut);

// Destructor (does nothing).
  ~MomentFit();

// This function is not implemented and throws an exception.
   virtual void process(T& out,
                        Bool& outMask,
                        const Vector<T>& in,
                        const Vector<Bool>& inMask,
                        const IPosition& pos);

// This function returns a vector of numbers from each input vector.
// the output vector contains the moments known to the ImageMoments
// or MSMoments object passed into the constructor.
   virtual void multiProcess(Vector<T>& out,
                             Vector<Bool>& outMask,
                             const Vector<T>& in,
                             const Vector<Bool>& inMask,
                             const IPosition& pos);

private:
   MomentsBase<T>& iMom_p;
   LogIO os_p;
   T stdDeviation_p, peakSNR_p;
   Bool doFit_p;
   Gaussian1D<T> gauss_p;

  //# Make members of parent class known.
protected:
  using MomentCalcBase<T>::constructorCheck;
  using MomentCalcBase<T>::setPosLabel;
  using MomentCalcBase<T>::selectMoments_p;
  using MomentCalcBase<T>::calcMoments_p;
  using MomentCalcBase<T>::calcMomentsMask_p;
  using MomentCalcBase<T>::doMedianI_p;
  using MomentCalcBase<T>::doMedianV_p;
  using MomentCalcBase<T>::doAbsDev_p;
  using MomentCalcBase<T>::cSys_p;
  using MomentCalcBase<T>::doCoordProfile_p;
  using MomentCalcBase<T>::doCoordRandom_p;
  using MomentCalcBase<T>::pixelIn_p;
  using MomentCalcBase<T>::worldOut_p;
  using MomentCalcBase<T>::sepWorldCoord_p;
  using MomentCalcBase<T>::integratedScaleFactor_p;
  using MomentCalcBase<T>::momAxisType_p;
  using MomentCalcBase<T>::nFailed_p;
  using MomentCalcBase<T>::abcissa_p;
};


}

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <imageanalysis/ImageAnalysis/MomentCalculator.tcc>
#endif //# CASACORE_NO_AUTO_TEMPLATES
#endif
