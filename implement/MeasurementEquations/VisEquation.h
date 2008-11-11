//# VisEquation.h: Interface definition for Vis Equation
//# Copyright (C) 1996,1997,1999,2000,2001,2002,2003
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

#ifndef SYNTHESIS_VISEQUATION_H
#define SYNTHESIS_VISEQUATION_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Cube.h>
#include <msvis/MSVis/VisBuffer.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// Visibility Measurement Equation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
// </prerequisite>
//
// <etymology>
// Visibility Equation describes a model for measurements from a
// generic synthesis telescope
// </etymology>
//
// <synopsis> 
// This is responsible for the Measurement Equation of the Generic
// Interferometer due to Hamaker, Bregman and Sault and later extended
// by Noordam, and Cornwell.
//
// See <linkto module="MeasurementEquations">MeasurementEquations</linkto>
// for more details of the form of the VisEquation.
//
// VisEquation provides the following:
// <ul>
// <li> Sorting of calibration terms (VisCals) in the Visibility Equation
// <li> Application of calibration terms to data (via delegation to VisCals)
// <li> Delivery of other evaluations of the VisEquation, e.g., (differentiated) residuals
// </ul>
// </synopsis> 
//
// <example>
// <srcblock>
//
// </srcblock>
// </example>
//
// <motivation>
// VisEquation is part of a framework of classes that are
// designed for synthesis calibration and imaging. The others are the 
// <linkto module=MeasurementComponents>MeasurementComponents</linkto>.
// </motivation>
//
// <todo asof="">
// </todo>

class VisEquation {
public:

  // Contructor
  VisEquation();
  
  // Copy ctor
  VisEquation(const VisEquation& other);

  // Destructor
  virtual ~VisEquation();
  
  // Assignment
  VisEquation& operator=(const VisEquation& other);

  // Arrange for apply of a VisCal list (i.e., sort them into the correct order)
  void setapply(PtrBlock<VisCal*>& vcin);

  // Arrange for solve of a single SolvableVisCal
  void setsolve(SolvableVisCal& svc);

  // Report if spw has solutions available from all applied tables
  inline Bool spwOK(const Int& spw) { return (napp_>0) ? spwOK_(spw) : True; };

  // Correct in place the OBSERVED visibilities in a VisBuffer
  //  with the apply-able VisCals
  void correct(VisBuffer& vb);

  // Corrupt in place the MODEL visibilities in a VisBuffer
  //  with the apply-able VisCals
  void corrupt(VisBuffer& vb);

  // Correct/Corrupt in place the OBSERVED/MODEL visibilities in a VisBuffer
  //  with the apply-able VisCals on either side of the SolvableVisCal
  void collapse(VisBuffer& vb);

  // Calculate residuals 
  //   (optionally for specific chan)
  void residuals(VisBuffer& vb, 
		 Cube<Complex>& R,
		 const Int& chan=-1);
  
  // Calculate residuals and differentiated residuals
  //   (optionally for specific chan)
  void diffResiduals(CalVisBuffer& cvb);
  void diffResiduals(VisBuffer& vb, 
		     Cube<Complex>& R, 
		     Array<Complex>& dR,
		     Matrix<Bool>& Rflg); 
  void diffResiduals(VisBuffer& R,
                     VisBuffer& dR0,
                     VisBuffer& dR1,
                     Matrix<Bool>& Rflg);

 
  // Report the VisEq's state
  void state();
  
protected:

  Bool ok();

  // Access to the PB of apply terms 
  inline PtrBlock<VisCal*>& vc()  { return (*vcpb_); };

  // Access to SVC
  inline SolvableVisCal&    svc() { return *svc_; };

  // Detect freq dependence along the Vis Equation
  void setFreqDep();

private:

  // Diagnostic print level access
  inline Int& prtlev() { return prtlev_; };

  // A local copy of the list of VisCal (pointers) for applying
  //  (No ownership responsibilities)
  PtrBlock<VisCal*>* vcpb_;

  // Number of apply VisCals in vc_
  Int napp_;

  // Frequency dependence indices
  Int lfd_;     // Right-most freq-dep term on LEFT  side
  Int rfd_;     // Left-most  freq-dep term on RIGHT side

  // VisCal with solving interface
  //  (No ownership responsibilities)
  SolvableVisCal* svc_;

  // SpwOK?
  Vector<Bool> spwOK_;

  // Diagnostic print level
  Int prtlev_;

};

} //# NAMESPACE CASA - END

#endif
