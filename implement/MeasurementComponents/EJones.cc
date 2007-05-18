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
      currParOK().resize(nPar(),1,nAnt());
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

      { ostringstream o;
	o<< "Could not find gain curve data for Spw="
	 << ispw << " (reffreq=" << spwfreqs_(ispw)/1.0e9 << " GHz)";

	throw(AipsError(o.str()));
      }

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

  za().resize(nAnt());
  Vector<MDirection> antazel(vb().azel(currTime()));
  Double* a=za().data();
  for (Int iant=0;iant<nAnt();++iant,++a) 
    (*a)=C::pi_2 - antazel(iant).getAngle().getValue()(1);

  // Pars now valid, matrices not yet
  validateP();
  invalidateJ();

}

void EGainCurve::calcAllJones() {

  if (prtlev()>6) cout << "       EGainCurve::calcAllJones()" << endl;

  // Nominally no gain curve effect
  currJElem()=Complex(1.0);
  currJElemOK()=False;

  Complex* J=currJElem().data();
  Bool*    JOk=currJElemOK().data();
  Float*  c=currRPar().data();
  Double* a=za().data();

  Double loss, ang;
  for (Int iant=0; iant<nAnt(); ++iant,++a)
    if ((*a)<C::pi_2) 
      for (Int ipol=0;ipol<2;++ipol,++J,++JOk) {
	loss=Double(*c);
	++c;
	ang=1.0;
	for (Int ipar=1;ipar<4;++ipar,++c) {
	  ang*=(*a);
	  loss+=((*c)*ang);
	}
	(*J) = Complex(loss);
	(*JOk) = True;
      }
  
}


// **********************************************************
//  EPJones
//

EPJones::EPJones(VisSet& vs) :
  VisCal(vs), 
  VisMueller(vs),
  SolvableVisJones(vs),
  pointPar_()
{
  if (prtlev()>2) cout << "EP::EP(vs)" << endl;
}

EPJones::~EPJones() {
  if (prtlev()>2) cout << "EP::~EP()" << endl;
}

Matrix<Float>& EPJones::pointPar() {

  // Local reference to reshaped currpar()
  pointPar_.reference(currRPar().nonDegenerate(1));
  return pointPar_;

}
		      


// Specialized setapply extracts image info
void EPJones::setApply(const Record& applypar) {

  // Call generic
  SolvableVisCal::setApply(applypar);

  // Extract image or complist to predict from

  // TBD

}

void EPJones::applyCal(VisBuffer& vb,
		       Cube<Complex>& Mout) {

  // Inflate model data in VB, Mout references it
  //  (In this type, model data is always re-calc'd from scratch)
  vb.modelVisCube(True);
  Mout.reference(vb.modelVisCube());

  // Call predict:
  //  TBD

  // At this point Mout (and thus vb.modelVisibility) contain
  //  pointing-corrupted visibilities

}


void EPJones::differentiate(VisBuffer& vb,
		   Cube<Complex>& Mout,
		   Array<Complex>& dMout,
		   Matrix<Bool>& Mflg) {

  Int nCorr(2); // TBD

  // Size differentiated model
  dMout.resize(IPosition(5,nCorr,nPar(),1,vb.nRow(),2));
  IPosition blc(dMout.shape()); blc=0;
  IPosition trc(dMout.shape()); trc-=1;
  
  // Model vis shape must match visibility
  vb.modelVisCube(False);
  Mout.reference(vb.modelVisCube());

  // Since predict likes to fill the VisBuffer,
  //  we will fill modelVisibility() for each type of
  //  predict, and copy out the result (morphing shape)
  //  as needed

  // Predict w/ derivatives w.r.t. FIRST par (az?)
  // TBD!

  // Copy to dMout
  blc(1)=trc(1)=0;   // deriv w.r.t. FIRST par
  //  blc(4)=trc(4)=; // ???   (which ant in each baseline?)
  //  dMout(blc,trc)=;
  blc(1)=trc(1)=1;   // deriv w.r.t SECOND par
  //  blc(4)=trc(4)=; // ???   (which ant in each baseline?)
  //  dMout(blc,trc)=;


  // Mflg just references vb.flag()?
  Mflg.reference(vb.flag());

}


} //# NAMESPACE CASA - END
