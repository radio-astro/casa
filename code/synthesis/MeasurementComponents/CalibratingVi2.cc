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
#include <casa/Arrays/ArrayPartMath.h>

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
  corrFactor_p(calpar.getCorrFactor()), // temporary
  visCorrOK_p(False)
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

  // Data/wts not yet corrected
  visCorrOK_p=False;
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

  // Data/wts not yet corrected
  visCorrOK_p=False;

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void CalibratingVi2::flag(Cube<Bool>& flagC) const
{
  //  cout << "CVI2::flag(Cube)...";

  // Call for correction, which might set some flags
  correctCurrentVB();

  // copy result to caller's Cube<Bool>
  flagC.assign(getVii()->getVisBuffer()->flagCube());

}



// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
/* DEPRECATED?
void CalibratingVi2::flag(Matrix<Bool>& flagM) const
{
  // Corr-indep flags
  //  cout << "CVI2::flag(Matrix)...";

  // Get corr-dep flags
  Cube<Bool> flagC;
  this->flag(flagC);

  // sum on corr axis
  uInt nr=flagC.shape()(2);
  uInt nch=flagC.shape()(1);
  flagM.resize(nch,nr);
  flagM=partialMaxs(flagC,IPosition(1,0)); 


}
*/



// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void CalibratingVi2::weight(Matrix<Float>& wt) const
{
 
  //  cout << "CVI2::weight...";

  // Call for correction
  //   TBD: optimize w.r.t. calibrating only the weights?
  correctCurrentVB();

  // copy result to caller's Matrix<Float>
  wt.assign(getVii()->getVisBuffer()->weight());

}


// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void CalibratingVi2::weightSpectrum(Cube<Float>& wtsp) const
{

  //  cout << "CVI2::weightSpectrum...";

  if (this->weightSpectrumExists()) {

    // Call for correction
    //   TBD: optimize w.r.t. calibrating only the weights?
    correctCurrentVB();
    
    // copy result to caller's Matrix<Float>
    wtsp.assign(getVii()->getVisBuffer()->weightSpectrum());

  }
  else {
    // same as ordinary VisibilityIteratorImpl2
    wtsp.resize(0,0,0);
  }
}




// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void CalibratingVi2::visibilityCorrected(Cube<Complex>& vis) const
{

  //  cout << "CVI2::visibilityCorrected...";

  // TBD:
  //  o consider if underlying VisBuffer should be maintained const? 
  //       (and this layer's VB set and adjusted)
  //       (will this break on-demand VB stuff?)
  //  o make corresponding visibilityModel method...  (solve context)

  // Call the actual correction method
  //  (only does the actual work, if needed)
  correctCurrentVB();

  // copy result to caller's Cube<Complex>
  vis.assign(getVii()->getVisBuffer()->visCubeCorrected());

}



// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool CalibratingVi2::existsColumn(VisBufferComponent2 id) const
{
    Bool result;
    switch (id){

    case VisibilityCorrected:
    case VisibilityCubeCorrected:

        result = True;
        break;

    default:
        result = getVii()->existsColumn(id);
        break;
    }

    return result;

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void CalibratingVi2::correctCurrentVB() const
{
  // This method must call NO ordinary VB2 accessors that require ViImpl 
  //   methods that are defined in this class, _even_implicitly_, because 
  //   the VB2 uses the VI2 that has its ViImpl overridden by these local 
  //   methods.  This causes infinite loops!!!
  // One way to avoid this is to ensure that every method in this class 
  //   initializes the VB2 fields via getVii() methods.....

  //  cout << " correctCurrentVB(): " << boolalpha << visCorrOK_p;

  // Do the correction, if not done yet
  if (!visCorrOK_p) {

    // Get the underlying ViImpl2's VisBuffer, to munge it
    VisBuffer2 *vb = getVii()->getVisBuffer();

    // sense if WEIGHT_SPECTRUM exists
    Bool doWtSp = getVii()->weightSpectrumExists();

    // Init the flagCube from below
    //   This does not use any CVi2 overloads
    Cube<Bool> flC;
    getVii()->flag(flC);
    vb->setFlagCube(flC);

    // Initialize the to-be-calibrated weights (this is smart re spec weights or not)
    //   This does not use any CVi2 overloads  (luckily)
    vb->resetWeightsUsingSigma();
    
    // Set the the initialize corrected data w/ data
    //   This does not use any CVi2 overloads
    vb->setVisCubeCorrected(vb->visCube());
    
    // Apply calibration  (TBD, includes weight calibration?)
    //  ve_p->correct2(*vb,....);
    
    // Use supplied corrFactor_p to make this corrected data look changed
    //  In leiu of working VisEquation that is TBD
    Cube<Complex> vCC(vb->visCubeCorrected());
    vCC*=corrFactor_p;
    vb->setVisCubeCorrected(vCC);

    if (doWtSp) {
      // Calibrate the WS
      Cube<Float> wS(vb->weightSpectrum());   // Was set above
      wS/=(corrFactor_p*corrFactor_p);
      vb->setWeightSpectrum(wS);
      // Set W via median on chan axis
      vb->setWeight(partialMedians(wS,IPosition(1,1)));
    }
    else {
      // Just calibrate the W
      Matrix<Float> w(vb->weight());          // Was set above
      w/=(corrFactor_p*corrFactor_p);
      vb->setWeight(w);
    }

    // Signal that we have applied the correction, to avoid unnecessary redundancy
    visCorrOK_p=True;

    //    cout << "-->" << visCorrOK_p;

  }    
  //  cout << endl;
}




} //# NAMESPACE VI - END
} //# NAMESPACE CASA - END


