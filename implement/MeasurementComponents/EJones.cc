//# StandardVisCal.cc: Implementation of Standard VisCal types
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#include <synthesis/MeasurementComponents/EJones.h>

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBuffAccumulator.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MeasurementEquations/VisEquation.h>

#include <tables/Tables/ExprNode.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Memory.h>
#include <casa/System/Aipsrc.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  EGainCurve
//

EGainCurve::EGainCurve(VisSet& vs) :
  VisCal(vs), 
  VisMueller(vs),
  SolvableVisJones(vs),
  za_()
{
  if (prtlev()>2) cout << "EGainCurve::EGainCurve(vs)" << endl;

  // Get some information from the VisSet to help us find
  //  the right gain curves

  const ROMSColumns& mscol(vs.iter().msColumns());

  // Observation info
  const ROMSObservationColumns& obscol(mscol.observation());

  String telescope(obscol.telescopeName()(0));
  if (telescope.contains("VLA")) 
    calTableName()=Aipsrc::aipsRoot() + "/data/nrao/VLA/GainCurves";

  Vector<Double> timerange(obscol.timeRange()(0));
  obstime_ = timerange(0);

  const ROMSAntennaColumns& antcol(mscol.antenna());
  antnames_ = antcol.name().getColumn();

  const ROMSSpWindowColumns& spwcol(mscol.spectralWindow());
  spwfreqs_ = spwcol.refFrequency().getColumn();

}

EGainCurve::~EGainCurve() {
  if (prtlev()>2) cout << "EGainCurve::~EGainCurve()" << endl;
}

void EGainCurve::setApply(const Record& applypar) {

  // Extract user's table name
  if (applypar.isDefined("table")) {
    String usertab=applypar.asString("table");
    if (Table::isReadable(usertab) )
      calTableName()=usertab;
  }

  if ( !Table::isReadable(calTableName()) )
    throw(AipsError("No Gain curve data available!"));

  // Open raw gain table
  Table rawgaintab(calTableName());

  // Select on Time
  Table timegaintab = rawgaintab(rawgaintab.col("BTIME") <= obstime_
                                 && rawgaintab.col("ETIME") > obstime_);

  // ...for each spw:
  for (Int ispw=0; ispw<nSpw(); ispw++) {

    currSpw()=ispw;

    // Select on freq:
    Table freqgaintab = timegaintab(timegaintab.col("BFREQ") <= spwfreqs_(ispw)
                                    && timegaintab.col("EFREQ") > spwfreqs_(ispw));

    if (freqgaintab.nrow() > 0) {
  /*
      { ostringstream o;
      o<< "  Found the following gain curve coefficient data" << endl
       << "  for spectral window = "  << ispw << " (ref freq="
       << spwfreqs_(ispw) << "):";
      message.message(o);
      logSink().post(message);
      }
  */
      // Find nominal gain curve
      //  Nominal antenna is named "0" (this is a VLA convention)
      Matrix<Float> nomgain(4,2,0.0);
      {
        Table nomgaintab = freqgaintab(freqgaintab.col("ANTENNA")=="0");
        if (nomgaintab.nrow() > 0) {
          ROArrayColumn<Float> gain(nomgaintab,"GAIN");
          nomgain=gain(0);
        } else {
          // nominal gain is unity
          nomgain(0,0)=1.0;
          nomgain(0,1)=1.0;
        }
      }

      currRPar().resize(nPar(),1,nAnt());
      currParOK().resize(1,nAnt());
      currParOK()=True;

      ArrayIterator<Float> piter(currRPar(),1);

      for (Int iant=0; iant<nAnt(); iant++,piter.next()) {

	// Select antenna by name
        Table antgaintab = freqgaintab(freqgaintab.col("ANTENNA")==antnames_(iant));
        if (antgaintab.nrow() > 0) {
          ROArrayColumn<Float> gain(antgaintab,"GAIN");
	  piter.array().nonDegenerate().reform(gain(0).shape())=gain(0);
        } else {
	  piter.array().nonDegenerate().reform(nomgain.shape())=nomgain;
        }

    /*
        { ostringstream o;
          o<< "   Antenna=" << antnames_(iant) << ": "
           << piter.array().nonDegenerate();
          message.message(o);
          logSink().post(message);
        }
    */
      }

    } else {
  /*
      { ostringstream o;
	o<< "Could not find gain curve data for spectral window ="
	 << ispw << "(ref freq=" << spwfreqs_(ispw) << ")";
	message.message(o);
	logSink().post(message);
      }
  */
    }

  } // ispw

  // Reset currSpw()
  currSpw()=0;

  // Resize za()
  za().resize(nAnt());

}

void EGainCurve::guessPar(VisBuffer& vb) {

  throw(AipsError("Spurious attempt to guess EGainCurve for solving!"));

}

void EGainCurve::calcPar() {

  if (prtlev()>6) cout << "      EGainCurve::calcPar()" << endl;

  // NB: z.a. calc here because it is needed only 
  //   if we have a new timestamp...
  //  (opacity parameter is already ok)

  za().resize(nAnt());
  Vector<MDirection> antazel(vb().azel(currTime()));
  Double* a=za().data();
  for (Int iant=0;iant<nAnt();++iant) 
    (*a)=C::pi_2 - antazel(iant).getAngle().getValue()(1);

  // Pars now valid, matrices not yet
  validateP();
  invalidateJ();

}

void EGainCurve::calcAllJones() {

  if (prtlev()>6) cout << "       EGainCurve::calcAllJones()" << endl;

  // Nominally no gain curve effect
  currJElem()=Complex(1.0);

  Complex* J=currJElem().data();
  Float*  c=currRPar().data();
  Bool*   opok=currParOK().data();
  Double* a=za().data();

  Double loss, ang;
  for (Int iant=0; iant<nAnt(); ++iant,++opok,++a)
    if ((*opok) && (*a)<C::pi_2) 
      for (Int ipol=0;ipol<2;++ipol,++J) {
	loss=Double(*c);
	++c;
	ang=1.0;
	for (Int ipar=1;ipar<4;++ipar,++c) {
	  ang*=(*a);
	  loss+=((*c)*ang);
	}
	(*J) = Complex(loss);
      }
  
}


// **********************************************************
//  EPJones
//

EPJones::EPJones(VisSet& vs) :
  VisCal(vs), 
  VisMueller(vs),
  SolvableVisJones(vs)
{
  if (prtlev()>2) cout << "EP::EP(vs)" << endl;
}

EPJones::~EPJones() {
  if (prtlev()>2) cout << "EP::~EP()" << endl;
}

void EPJones::calcPar() {

  if (prtlev()>6) cout << "      EP::calcPar()" << endl;

  // Assume zero pointing offset for now

  // Initialize parameter arrays
  currRPar().resize(2,1,nAnt());
  currRPar()=0.0;
  currParOK().resize(1,nAnt());
  currParOK()=True;

  validateP();
  invalidateJ();

}



} //# NAMESPACE CASA - END
