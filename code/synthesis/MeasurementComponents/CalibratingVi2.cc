//# CalibratingVi2.cc: Implementation of the CalibratingVi2 class.
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

#include <synthesis/MeasurementComponents/CalibratingVi2.h>
#include <synthesis/MeasurementEquations/VisEquation.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN




// -----------------------------------------------------------------------
//

CalibratingParameters::CalibratingParameters() :
  calLibRecord_p(Record()),
  corrFactor_p(100.0)  // temporary, for initial testing  (default is a non-trivial factor)
{}

CalibratingParameters::CalibratingParameters(const Record& calLibRecord) :
  calLibRecord_p(calLibRecord),
  corrFactor_p(1.0)  // temporary, for initial testing
{
  validate();
}

CalibratingParameters::CalibratingParameters(Float corrFactor) :
  calLibRecord_p(Record()),
  corrFactor_p(corrFactor)  // temporary, for initial testing
{
  validate();
}

CalibratingParameters::CalibratingParameters(const CalibratingParameters& other)
{
  *this = other;
}

CalibratingParameters& CalibratingParameters::operator=(const CalibratingParameters& other)
{
  if (this != &other) {
    calLibRecord_p = other.calLibRecord_p;
    corrFactor_p = other.corrFactor_p;
    validate();
  }
  return *this;
}

const Record& CalibratingParameters::getCalLibRecord() const
{
  return calLibRecord_p;
}

// temporary, for initial testing
Float CalibratingParameters::getCorrFactor() const
{
  return corrFactor_p;
}


void CalibratingParameters::setCalLibRecord(const Record& calLibRecord)
{
  calLibRecord_p = calLibRecord;
}

// temporary, for initial testing
void CalibratingParameters::setCorrFactor(Float corrFactor)
{
  corrFactor_p = corrFactor;
}

void CalibratingParameters::validate() const
{
  // nothing meaningful to do yet
}



// -----------------------------------------------------------------------
CalibratingVi2::CalibratingVi2(	vi::VisibilityIterator2 * vi,
				vi::ViImplementation2 * inputVii,
				const CalibratingParameters& calpar) :
  TransformingVi2 (vi, inputVii),
  //,  ve_p(ve)   // MSTransform-side VisEquation interface: TBD
  corrFactor_p(calpar.getCorrFactor()) // temporary
{

  // Generate non-trival working VisEquation interface that can do calibration from calparam
  //  TBD, to look something like:
  //  ve_p = new VisEquationI(calpar);


  // Initialize underlying ViImpl2
  getVii()->originChunks();
  getVii()->origin();
 
  // Make a VisBuffer for CalibratingVi2 clients (it is connected to the vi interface)
  setVisBuffer(VisBuffer2::factory(vi,VbPlain,VbRekeyable));

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
CalibratingVi2::~CalibratingVi2()
{
  // Delete VisEquationI and related, if nec. 
  // TBD
  //  delete ve_p;
}



// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void
CalibratingVi2::origin() 
{

  // Drive underlying VII2
  getVii()->origin();

  // Keep my VB2 happily synchronized
  //  (this comes from TransformingVi2)
  configureNewSubchunk();

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void CalibratingVi2::next() 
{

  // Drive underlying VII2
  getVii()->next();

  // Keep my VB2 happily synchronized
  //  (this comes from TransformingVi2)
  configureNewSubchunk();

}


// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void CalibratingVi2::visibilityCorrected(Cube<Complex>& vis) const
{

  // TBD:
  //  o consider if underlying VisBuffer should be maintained const? 
  //       (and this layer's VB set and adjusted)
  //       (will this break on-demand VB stuff?)
  //  o make corresponding visibilityModel method...  (solve context)

  // Get the underlying ViImpl2's VisBuffer, to munge it
  VisBuffer2 *vb = getVii()->getVisBuffer();

  // Set the the initialize corrected data w/ data
  vb->setVisCubeCorrected(vb->visCube());

  // Apply calibration  (TBD, includes weight calibration?)
  //  ve_p->correct2(*vb,....);

  // Use supplied corrFactor_p to make this corrected data look changed
  //  In leiu of working VisEquation that is TBD
  Cube<Complex> vCC(vb->visCubeCorrected());
  vCC*=corrFactor_p;
  vb->setVisCubeCorrected(vCC);

  // copy result to caller's Cube
  vis.assign(vb->visCubeCorrected());

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------

} //# NAMESPACE VI - END
} //# NAMESPACE CASA - END


