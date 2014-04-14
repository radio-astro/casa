//# FJones.cc: Implementation of Ionosphere
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003,2011,2014
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

#include <synthesis/MeasurementComponents/FJones.h>
//#include <synthesis/MeasurementComponents/CalCorruptor.h>

#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/CalTables/CTIter.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/BasicMath/Math.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Memory.h>
#include <casa/System/Aipsrc.h>

#include <casa/sstream.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  FJones
//

FJones::FJones(VisSet& vs) :
  VisCal(vs), 
  VisMueller(vs),
  SolvableVisJones(vs),
  mframe_(),
  emm_(NULL),
  ionhgt_(450.,"km"),
  za_(),
  radper_(2.3649)   // rad*Hz2/G
{
  if (prtlev()>2) cout << "FJones::FJones(vs)" << endl;

  // Prepare zenith angle storage
  za().resize(nAnt());
  za().set(0.0);

  // Prepare line-of-sight B field (G) storage
  BlosG_.resize(nAnt());
  BlosG_.set(0.0);

}

FJones::~FJones() {
  if (prtlev()>2) cout << "FJones::~FJones()" << endl;

  if (emm_)
    delete emm_;

}



String FJones::applyinfo() {

  ostringstream o;
  o << typeName();
  o << ": table=" << calTableName();

  return String(o);

}


void FJones::calcPar() {

  if (prtlev()>6) cout << "      FJones::calcPar()" << endl;

  // Calculate zenith angle for current time/direction
  za().resize(nAnt());
  Vector<MDirection> antazel(vb().azel(currTime()));
  Double* a=za().data();
  for (Int iant=0;iant<nAnt();++iant,++a) 
    (*a)=C::pi_2 - antazel(iant).getAngle().getValue()(1);


  // set time in mframe_
  MEpoch epoch(Quantity(currTime(),"s"));
  epoch.setRefString("UTC");
  mframe_.set(epoch);

  // Set this antenna's position in mframe_
  const MPosition& antpos0 = vb().msColumns().antenna().positionMeas()(0);
  mframe_.set(antpos0);

  // set direction ref in emm
  const MDirection& phasedir = vb().msColumns().field().phaseDirMeas(currField());
  const MDirection::Ref phasedirR=phasedir.getRef();
  const MVDirection phasedirV=phasedir.getValue();

  // Construct workable EMM (if not yet done)
  if (!emm_) 
    emm_ = new EarthMagneticMachine(phasedirR,ionhgt_,mframe_);

  // Calculate ant-dep mag field
  BlosG_.resize(nAnt());

  // loop over antennas
  for (Int iant=0;iant<nAnt();++iant) {

    // Set this antenna's position in mframe_
    const MPosition& antpos = vb().msColumns().antenna().positionMeas()(iant);
    mframe_.resetPosition(antpos);

    // calculate this ant's field
    emm_->calculate(phasedirV);
    BlosG_(iant)=emm_->getLOSField("G").getValue();    // UNITS!!!

  }

  //  cout.precision(16);
  //  cout << "BlosG_ = " << BlosG_ << endl;
 
  // Get current zenith tec(t)
  SolvableVisCal::calcPar();

  //  cout << "currRPar() = " << currRPar() << endl;


  // Pars now valid, matrices not yet
  validateP();
  invalidateJ();  // Force new calculation of za-dep matrix elements

}


void FJones::calcAllJones() {

  if (prtlev()>6) cout << "       FJones::calcAllJones()" << endl;

  // Nominally no ionosphere
  currJElem()=Complex(1.0);
  currJElemOK().set(True);

  //  cout << currSpw() << " " 
  //       << currTime() << " " 
  //       << "currJElem().shape() = " << currJElem().shape() << endl;

  Complex* J=currJElem().data();
  Float*  ztec=currRPar().data();
  Bool*   ztecok=currParOK().data();
  Double* a=za().data();
  Double f,rotpers2,tec,rot;
  
  for (Int iant=0; iant<nAnt(); ++iant,++ztec,++ztecok,++a) {
    if ((*ztecok) && (*a)<C::pi_2) {
      tec = Double(*ztec)/cos(*a);
      rotpers2 = radper_*tec*BlosG_(iant);

      for (Int ich=0;ich<vb().nChannel();++ich,J+=2) {
	f=vb().frequency()(ich);
	rot = rotpers2/f/f;
	J[1]=Complex(cos(rot),sin(rot));
	J[0]=conj(J[1]);
      }
    }

  }
  //  cout << "tec = " << tec << endl;
  // cout << "rot = " << rot << " " << rotpers2 << endl;
  // cout << "currJElem() = " << currJElem() << endl;

  validateJ();
}



} //# NAMESPACE CASA - END
