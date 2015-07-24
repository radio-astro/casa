//# CalibratingVi2.h: Interface definition of the CalibratingVi2 class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef CalibratingVi2_H_
#define CalibratingVi2_H_

// Where TransformingVi2 interface is defined
#include <msvis/MSVis/TransformingVi2.h>
#include <synthesis/MeasurementComponents/Calibrater.h>
#include <synthesis/MeasurementEquations/VisEquation.h>

#include <casa/Containers/Record.h>

namespace casa { //# NAMESPACE CASA - BEGIN
namespace vi { //# NAMESPACE VI - BEGIN

class CalibratingParameters {

// TBD:  add solve-context parameters, like apply pivot (solved-for term), normalization

public:

  CalibratingParameters();

  CalibratingParameters(const Record& calLibRecord);
  CalibratingParameters(const String& callib);
  CalibratingParameters(Float corrFactor); // temporary, for initial testing

  CalibratingParameters(const CalibratingParameters& other);
  CalibratingParameters& operator=(const CalibratingParameters & other);

  Bool byCalLib() const;

  const Record& getCalLibRecord() const;
  Float getCorrFactor() const;  

  void setCalLibRecord(const Record& calLibRecord);
  void setCorrFactor(Float corrFactor);  

private:

  void validate() const;
  Bool byCalLib_p;
  Record calLibRecord_p;
  Float corrFactor_p;  // temporary, for initial testing
};




class CalibratingVi2 : public vi::TransformingVi2
{

public:

  CalibratingVi2(vi::VisibilityIterator2 * vi, 
		 vi::ViImplementation2 * inputVii,
		 const CalibratingParameters& calpar);

  CalibratingVi2(vi::VisibilityIterator2 * vi, 
		 vi::ViImplementation2 * inputVii,
		 const CalibratingParameters& calpar,
		 String msname);

  ~CalibratingVi2();


  // Iterating management
  virtual void origin ();
  virtual void next ();

  // Smart corrected data/weights/flags accessors
  virtual void weight(Matrix<Float>& wt) const;
  virtual void weightSpectrum(Cube<Float>& wtsp) const;
  virtual void visibilityCorrected(Cube<Complex>& vis) const;
  virtual void flag(Cube<Bool>& flagC) const;
  //  virtual void flag(Matrix<Bool>& flagM) const;  // CORR-INDEP FLAGS DEPRECATED IN VI2/VB2?

  // Reports True in case of *Corrected columns
  //  (because this class provides it, even if it doesn't exist physically!)
  virtual Bool existsColumn (VisBufferComponent2 id) const;
  
private:

  // Correct the current VB
  void correctCurrentVB() const;

  // Calibrater and VisEquation
  Calibrater cb_p;
  VisEquation *ve_p;   

  // A simple factor for testing
  Float corrFactor_p;  

  // signals whether or not correctCurrentVB has been called
  mutable Bool visCorrOK_p;

};

} //# NAMESPACE VI - END
} //# NAMESPACE CASA - END


#endif /* CalibratingVi2_H_ */

