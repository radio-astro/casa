//# SimVisJones.cc: Implementation of Jones classes
//# Copyright (C) 1996,2000,2001,2003
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
//# $Id$

#include <synthesis/MeasurementComponents/SimVisJones.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// ------------------------------------------------------------------
// Start of methods for a non-solveable Jones matrix for any solely
// visibility indexed gain. This caches the interferometer gains and
// the inverses according to time. It could be generalized to cache on
// something else. The antenna gain is calculated as needed per slot.


// From VisSet. 
SimVisJones::SimVisJones(VisSet& vs, Int seed, Double interval, Double deltat) 
  : SolvableVisJones(vs), rndGen_p(seed)
{

  interval_=interval;

  // Make local VisSet (specialized in SimVisJones w/out FIELD_ID iter
  makeLocalVisSet();

  // initialize caches
  initMetaCache();
  fillMetaCache();
  initAntGain();
  initThisGain();

  // This is essentially apply context
  setSolved(False);
  setApplied(True);

}

SimVisJones::SimVisJones(const SimVisJones& other)
{
  //  operator=(other);
}

// Assignment
SimVisJones& SimVisJones::operator=(const SimVisJones& other) {
    if(this!=&other) {
      //  This won't work yet!
      //	TimeVarVisJones::operator=(other);  
	rndGen_p = other.rndGen_p;
    }
    return *this;
}


// 
// Simulated components are not (yet) Field-dependent
void SimVisJones::makeLocalVisSet() {
  Block<Int> columns;
  columns.resize(3);
  columns[0]=MS::ARRAY_ID;
  columns[1]=MS::DATA_DESC_ID;
  columns[2]=MS::TIME;
  
  vs_= new VisSet(*vs_,columns,interval_);
  localVS_=True;
  
  // find out how many intervals we have
  VisIter& ioc(vs_->iter());
  ioc.originChunks();
  VisBuffer vb(ioc);
  for (ioc.originChunks(); ioc.moreChunks();ioc.nextChunk()) {
    // count number of slots per spw
    numberSlot_[ioc.spectralWindow()]++;
  }
  
  LogMessage message(LogOrigin("SimVisJones", "makeLocalVisSet"));
  {
    ostringstream o; o<<"For interval of "<<interval_<<" seconds, found "<<
                       sum(numberSlot_)<<" slots";message.message(o);
    logSink().post(message);
  }

}

SimGJones::SimGJones(VisSet& vs, Int seed, 
		     Distribution ampDist, Double ampMean, Double ampSpread,
		     Distribution phaseDist, Double phaseMean, Double phaseSpread,
		     Double interval, Double deltat) :
  SimVisJones(vs, seed, interval, deltat) 
{

  nPar_=2;
  switch (ampDist) {
  case uniform: ampDist_p=new Uniform(&rndGen_p, ampMean-ampSpread/2,
				      ampMean+ampSpread/2);
    break;		  
  case normal:  
  default:      ampDist_p=new Normal(&rndGen_p, ampMean, ampSpread);
    break;
  }
  switch (phaseDist) {
  case uniform: phaseDist_p=new Uniform(&rndGen_p, phaseMean-phaseSpread/2,
					phaseMean+phaseSpread/2);
    break;		  
  case normal:  
  default:	  phaseDist_p=new Normal(&rndGen_p, phaseMean, phaseSpread);
    break;
  }
  init();
}

SimGJones::~SimGJones()
{
    delete ampDist_p;
    delete phaseDist_p;
}

void SimGJones::init()
{
  Complex gain;
  IPosition ip(4,0,0,0,0);
  for (Int iSpw=0; iSpw<numberSpw_; iSpw++) {
    for (Int iSlot=0; iSlot<numberSlot_(iSpw); iSlot++) {
      ip(3)=iSlot;
      for (Int iAnt=0; iAnt<numberAnt_; iAnt++) {
	ip(2)=iAnt;
	for (Int iPar=0; iPar<nPar_; iPar++) {
	  ip(0)=iPar;
	  gain=Float((*ampDist_p)())*exp(Complex(0.f,Float((*phaseDist_p)())));
	  (*jonesPar_[iSpw])(ip)=gain;
	}
	(*jonesParOK_[iSpw])(0,iAnt,iSlot)=True;
      }
    }
  }
}

SimDJones::SimDJones(VisSet& vs, Int seed, Distribution dist,
	       Double mean, Double spread,
	       Double interval, Double deltat) :
	       SimVisJones(vs, seed, interval, deltat) 
{
  nPar_=4;
  switch (dist) {
  case uniform: leakageDist_p=new Uniform(&rndGen_p, mean-spread/2,
					  mean+spread/2);
    break;		  
  case normal:  
  default:      leakageDist_p=new Normal(&rndGen_p, mean, spread);
    break;
  }
  init();
}

SimDJones::~SimDJones()
{
  delete leakageDist_p;
}


void SimDJones::init()
{
  Complex leakage;
  IPosition ip(4,0,0,0,0);
  for (Int iSpw=0; iSpw<numberSpw_; iSpw++) {
    for (Int iSlot=0; iSlot<numberSlot_(iSpw); iSlot++) {
      ip(3)=iSlot;
      for (Int iAnt=0; iAnt<numberAnt_; iAnt++) {
	ip(2)=iAnt;
	ip(0)=0; (*jonesPar_[iSpw])(ip)=Complex(1.,0.);
	ip(0)=3; (*jonesPar_[iSpw])(ip)=Complex(1.,0.);
	for (Int iPar=1; iPar<3; iPar++) {
	  leakage=Complex((*leakageDist_p)(),
			   (*leakageDist_p)());
	  ip(0)=iPar;
	  (*jonesPar_[iSpw])(ip)=leakage;
	}
      }
    }
  }
}

} //# NAMESPACE CASA - END

