//# MSMoments.h: generate moments from MS
//# Copyright (C) 1997,1998,1999,2000,2001
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
//#
//# $Id: MSMoments.h 20838 2009-12-01 10:10:55Z gervandiepen $

#ifndef _MS_MSMOMENTS_H_
#define _MS_MSMOMENTS_H_

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Slicer.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/StokesConverter.h>
#include <ms/MeasurementSets/MSDerivedValues.h>
#include <ms/MeasurementSets/MSSelectionKeywords.h>
//#include <images/Images/MomentsBase.h>
#include <imageanalysis/ImageAnalysis/MomentCalculator.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template<class T> class MomentCalcBase;
template<class T> class MomentsBase;


// <summary>
// This class generates moments from a spectral data.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MeasurementSet">MeasurementSet</linkto>
//   <li> <linkto class="ImageMoments">ImageMoments</linkto>
//   <li> <linkto class="LatticeApply">LatticeApply</linkto>
//   <li> <linkto class="MomentCalcBase">MomentCalcBase</linkto>
// </prerequisite>
//
// <etymology>
// This class computes moments from spectral data.
// </etymology>
//
// <synopsis>
// The primary goal of this class is to help spectral-line astronomers analyze their 
// spectral data by generating moments of a specified axis.
// MSMoments is defined by analogy with ImageMoments
// to satisfy a requirement that expects a functionality to generate moments 
// from spectral data in the form of MeasurementSet.
// Differences between ImageMoments and MSMoments are the following two things: 
// 1)a form of input data and 2)allowed axis type to generate moments. 
// The output file names are constructed by a prefix plus some internally generated suffixes. 
// The prefix is provided from the user (<src>ourFileName</src>) or determined from 
// the input file name. 
//
// The input for MSMoments class is 
// spectral data in the form of MeasurementSet in contrast to ImageMoments that 
// takes an image (ImageInterface) as input. As in the ImageMoments, MSMoments 
// only accepts spectral data of type <src>Float</src> and <src>Double</src>. 
// Effectively, it means that MSMoments only accepts MeasurementSet with FLOAT_DATA 
// column, i.e. autocorrelation data.
//
// <note role=caution>
// Although MeasurementSet is able to store spectral data with different frequency 
// setting, i.e. multiple SPECTRAL_WINDOW_ID, MSMoments does not take care about that.
// More specifically, MSMoments applies a certain single SPECTRAL_WINDOW_ID for all 
// spectral data that may have different ones.
// Because of this limitation, input MeasurementSet must be selected at least by 
// DATA_DESCRIPTION_ID using MSSelection or iterating MSIter.
// </note>
//
// In general, moments can be generated with respect to any moment axes. 
// In fact, ImageMoments allows to generate moments along any axes (spectral axis, 
// direction axes such as RA and DEC, and polarization axis). 
// However, MSMoments only allows to generate moments along spectral axis so far.
// Implementation of generating moments along other axis is future work.
//
// Smoothing the data and making plots are not implemented so far.
// </synopsis>
//
// <example>
// <srcBlock>
//// Set state function argument values
//
//      Vector<Int> moments(2);
//      moments(0) = ImageMoments<Float>::AVERAGE;
//      moments(1) = ImageMoments<Float>::WEIGHTED_MEAN_COORDINATE;
//      Vector<int> methods(2);
//      methods(0) = ImageMoments<Float>::WINDOW;
//      methods(1) = ImageMoments<Float>::INTERACTIVE;
//      Vector<Int> nxy(2);
//      nxy(0) = 3;
//      nxy(1) = 3;
//
//// Open MS
//     
//      MeasurementSet inMS(inName);  
//// Construct moment helper object
//
//      LogOrigin or("myClass", "myFunction(...)", WHERE);
//      LogIO os(or);
//      ImageMoments<Float> moment(inMS, os);
//
//// Specify state via control functions
//
//      if (!moment.setMoments(moments)) return 1;
//
//// Create the moments
//
//      if (!moment.createMoments()) return 1;
//
// </srcBlock>
// In this example, we generate two moments (average intensity and intensity
// weighted mean coordinate -- usually the velocity field). 
// The moment axis is automatically set to spectral axis.
// The output file names are constructed by the 
// class from the input file name plus '.moments'.
// </example>
//
// <motivation>
// MSMoments is defined to enable to generate moments from single-dish spectral data 
// in the form of MeasurementSet.
// </motivation>
//
// <todo asof="1998/12/11">
//   <li> More flexible data selection
//   <li> Channel masking
//   <li> Implementation of generating moments along other axes than spectral one.
//   <li> Implement smoothing data
//   <li> Making plots
// </todo>

template<class T> class MSMoments : public MomentsBase<T>
{
public:
  
   friend class MomentCalcBase<T>;
  
   // construct from an MS, the MS will supply the range of the various
   // parameters that can be selected on.
   MSMoments(MeasurementSet& ms,
             LogIO &os,
             Bool overwriteOutput,
             Bool showProgress);
  
   // Copy constructor, this will initialize the MS with other's MS
   MSMoments(const MSMoments<T>& other);
   
   // Destructor
   virtual ~MSMoments();
   
   // Assignment operator.
   MSMoments<T> &operator=(const MSMoments<T> &other); 
 
   // Change or Set the MS this MSMoments refers to.
   Bool setNewMS(MeasurementSet& ms);
  
   // Set the moment axis (0 relative). This returns <src>False</src> if 
   // specified axis is not a spectral axis. If you don't
   // call this function, the default state of the class is to set the 
   // moment axis to the spectral axis if it can find one.  Otherwise 
   // an error will result.
   Bool setMomentAxis (const Int& momentAxis);

   // This function invokes smoothing of the input MS. Not implemented.
   Bool setSmoothMethod(const Vector<Int> &smoothAxes,
                        const Vector<Int>& kernelTypes,
                        const Vector<Quantum<Double> >& kernelWidths);

   // This is the function that does all the computational work.  It should be called
   // after the <src>set</src> functions.  A return value of  <src>False</src>
   // indicates that additional checking of the combined methods that you
   // have requested has shown that you have not given consistent state to the class.
   // The output PtrBlock will hold a pointer to output MeasurementSet.
   // It is your responsibility to delete the pointers.
   // The <src>doTemp</src> does no effect so far.
   //
   // If you specify outFileName, it is the root name for 
   // the output files.  Suffixes will be made up internally to append 
   // to this root.  If you only ask for one moment,
   // this will be the actual name of the output file.  If you don't set this
   // variable, the default state of the class is to set the output name root to 
   // the name of the input file.  
   Bool createMoments( PtrBlock< MeasurementSet* >& data,
                       Bool doTemp, 
                       const String& outFileName ) ;

   // Get CoordinateSystem
   CoordinateSystem coordinates() ;


   // Get shape 
   IPosition getShape() const ;

private:

   // Smooth data
   Bool smoothData (PtrHolder<MeasurementSet> &pSmoothedMS,
                    String& smoothName);

   // Determine noise level. Not implemented.
   Bool whatIsTheNoise (T& noise,
                        MeasurementSet &ms);

   // Data
   MeasurementSet *ms_p; 

   // Internal Selected Data
   MeasurementSet *msSel_p ;

   // CoordinateSystem
   CoordinateSystem cSys_ ;

   // Spectral Window ID
   Int spwId_ ;

   // Data Description ID
   Int dataDescId_ ;

protected:
  using MomentsBase<T>::os_p;
  using MomentsBase<T>::showProgress_p;
  using MomentsBase<T>::momentAxisDefault_p;
  using MomentsBase<T>::peakSNR_p;
  using MomentsBase<T>::stdDeviation_p;
  using MomentsBase<T>::yMin_p;
  using MomentsBase<T>::yMax_p;
  using MomentsBase<T>::out_p;
  using MomentsBase<T>::smoothOut_p;
  using MomentsBase<T>::goodParameterStatus_p;
  using MomentsBase<T>::doWindow_p; 
  using MomentsBase<T>::doFit_p;
  using MomentsBase<T>::doAuto_p;
  using MomentsBase<T>::doSmooth_p;
  using MomentsBase<T>::noInclude_p;
  using MomentsBase<T>::noExclude_p;
  using MomentsBase<T>::fixedYLimits_p;
  using MomentsBase<T>::momentAxis_p;
  using MomentsBase<T>::worldMomentAxis_p;
  using MomentsBase<T>::kernelTypes_p;
  using MomentsBase<T>::kernelWidths_p;   
  using MomentsBase<T>::nxy_p;
  using MomentsBase<T>::moments_p;
  using MomentsBase<T>::selectRange_p;
  using MomentsBase<T>::smoothAxes_p;
  using MomentsBase<T>::plotter_p;
  using MomentsBase<T>::overWriteOutput_p;
  using MomentsBase<T>::error_p;
  using MomentsBase<T>::convertToVelocity_p;
  using MomentsBase<T>::velocityType_p;
  using MomentsBase<T>::checkMethod;
  using MomentsBase<T>::setInExCludeRange;
 public:
  using MomentsBase<T>::setVelocityType;
  using MomentsBase<T>::setSnr;
  using MomentsBase<T>::setWinFitMethod;
  using MomentsBase<T>::setMoments;
  using MomentsBase<T>::errorMessage;
};
} //# NAMESPACE CASA - END

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <synthesis/MSVis/MSMoments.tcc>
#endif //# CASACORE_NO_AUTO_TEMPLATES
#endif

