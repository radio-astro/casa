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
//#include <synthesis/MeasurementComponents/CalibratingVi2Factory.h>

//#include <synthesis/MeasurementEquations/VisEquation.h>

#include <casa/Containers/Record.h>

namespace casa { //# NAMESPACE CASA - BEGIN
namespace vi { //# NAMESPACE VI - BEGIN

class CalibratingParameters {

// TBD:  add solve-context parameters, like apply pivot (solved-for term), normalization

public:

  CalibratingParameters();

  CalibratingParameters(const Record& calLibRecord);
  CalibratingParameters(Float corrFactor); // temporary, for initial testing

  CalibratingParameters(const CalibratingParameters& other);
  CalibratingParameters& operator=(const CalibratingParameters & other);

  const Record& getCalLibRecord() const;
  Float getCorrFactor() const;  // temporary, for initial testing

  void setCalLibRecord(const Record& calLibRecord);
  void setCorrFactor(Float corrFactor);  // temporary, for initial testing

private:

  void validate() const;

  Record calLibRecord_p;
  Float corrFactor_p;  // temporary, for initial testing
};




class CalibratingVi2 : public vi::TransformingVi2
{

public:

  CalibratingVi2(vi::VisibilityIterator2 * vi, 
		 vi::ViImplementation2 * inputVii,
		 const CalibratingParameters& calpar);
  ~CalibratingVi2();


  // Iterating management
  virtual void origin ();
  virtual void next ();

  // Smart corrected data generator
  virtual void visibilityCorrected(Cube<Complex>& vis) const;

  
private:
  //  VisEquationI *ve_p;   // TBD: MSTransform-side VisEquation interface

  Float corrFactor_p;  // Temporary correction factor to apply for testing


};

} //# NAMESPACE VI - END
} //# NAMESPACE CASA - END


#endif /* CalibratingVi2_H_ */

