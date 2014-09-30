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
#include <scimath/Functionals/ScalarSampledFunctional.h>
#include <scimath/Functionals/Interpolate1D.h>

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
  za_(),
  eff_(nSpw(),1.0),
  spwOK_()
{
  if (prtlev()>2) cout << "EGainCurve::EGainCurve(vs)" << endl;
}

EGainCurve::~EGainCurve() {
  if (prtlev()>2) cout << "EGainCurve::~EGainCurve()" << endl;
}

void EGainCurve::setApply(const Record& applypar) {

  LogMessage message;

  // Extract user's table name
  String usertab("");
  if (applypar.isDefined("table")) {
    usertab=applypar.asString("table");
  }

  if (usertab=="") {

    { ostringstream o;
      o<< "Invoking gaincurve application without a caltable (e.g., " << endl
       << " via gaincurve=T in calibration tasks) will soon be disabled." << endl
       << " Please begin using gencal to generate a gaincurve caltable, " << endl
       << " and then supply that table in the standard manner." << endl;
      message.message(o);
      message.priority(LogMessage::WARN);
      logSink().post(message);
    }

    String tempname="temporary.gaincurve";

    // Generate automatically via specify mechanism
    Record specpar;
    specpar.define("caltable",tempname);
    specpar.define("caltype","gc");
    setSpecify(specpar);
    specify(specpar);
    storeNCT();
    delete ct_; ct_=NULL;  // so we can form it in the standard way...

    // Load the caltable for standard apply
    Record newapplypar=applypar;
    newapplypar.define("table",tempname);
    SolvableVisCal::setApply(newapplypar);

    // Delete the temporary gaincurve disk table (in-mem copy still ok)
    if (calTableName()==tempname &&
	Table::canDeleteTable(calTableName()) ) {
      Table::deleteTable(calTableName());
    }

    // Revise name that will appear in log messages, etc.
    calTableName()="<"+tempname+">";

  }
  else
    // Standard apply via table
    SolvableVisCal::setApply(applypar);

  // Resize za()
  za().resize(nAnt());

}

void EGainCurve::setSpecify(const Record& specify) {

  // Get some information from the MS to help us find
  //  the right gain curves

  MeasurementSet ms(msName());
  ROMSColumns mscol(ms);

  // The antenna names
  const ROMSAntennaColumns& antcol(mscol.antenna());
  antnames_ = antcol.name().getColumn();

  // Observation info
  const ROMSObservationColumns& obscol(mscol.observation());

  String telescope(obscol.telescopeName()(0));
  if (telescope.contains("VLA")) {
    gainCurveSrc_=Aipsrc::aipsRoot() + "/data/nrao/VLA/GainCurves";
    if ( !Table::isReadable(gainCurveSrc_) )
      throw(AipsError("Gain curve table "+calTableName()+" is unreadable or absent."));

    // Strip any ea/va baloney from the MS antenna names so 
    //  they match the integers (as strings) in the GainCurves table
    for (uInt iant=0;iant<antnames_.nelements();++iant) {
      antnames_(iant)=antnames_(iant).from(RXint);
      if (antnames_(iant).find('0')==0)
	antnames_(iant)=antnames_(iant).after('0');
    }
  }

  Vector<Double> timerange(obscol.timeRange()(0));
  obstime_ = timerange(0);

  const ROMSSpWindowColumns& spwcol(mscol.spectralWindow());
  spwfreqs_.resize(nSpw());
  spwfreqs_.set(0.0);
  spwbands_.resize(nSpw());
  spwbands_.set(String("?"));
  for (Int ispw=0;ispw<nSpw();++ispw) {
    Vector<Double> chanfreqs=spwcol.chanFreq()(ispw);
    spwfreqs_(ispw)=chanfreqs(chanfreqs.nelements()/2);
    String bname=spwcol.name()(ispw);
    if (bname.contains("EVLA"))
      spwbands_(ispw)=String(bname.before("#")).after("EVLA_");
  }
  //  cout << "spwfreqs_ = " << spwfreqs_ << endl;
  //  cout << "spwbands_ = " << spwbands_ << endl;

  // Neither applying nor solving in specify context
  setSolved(False);
  setApplied(False);

  // Collect Cal table parameters
  if (specify.isDefined("caltable")) {
    calTableName()=specify.asString("caltable");

    if (Table::isReadable(calTableName()))
      logSink() << "FYI: We are going to overwrite an existing CalTable: "
                << calTableName()
                << LogIO::POST;
  }

  // Create a new caltable to fill
  createMemCalTable();

  // Setup shape of solveAllRPar
  initSolvePar();

  /* From AIPS TYAPL (2012Sep14):

      DATA TF / 1.0,  1.1,  1.2,  1.3,  1.4,  1.5,  1.6,  1.7,  1.8,
     *    1.9,  2.0,  2.0,  2.3,  2.7,  3.0,  3.5,  3.7,  4.0,  4.0,
     *    5.0,  6.0,  7.0,  8.0,  8.0, 12.0, 12.0, 13.0, 14.0, 15.0,
     *   16.0, 17.0, 18.0, 19.0, 24.0, 26.0, 26.5, 28.0, 33.0, 38.0,
     *   40.0, 43.0, 48.0/
C                                       Rick Perley efficiencies
      DATA TE /0.45, 0.48, 0.48, 0.45, 0.46, 0.45, 0.43, 0.44, 0.44,
     *   0.49, 0.48, 0.52, 0.52, 0.51, 0.53, 0.55, 0.53, 0.54, 0.55,
     *   0.54, 0.56, 0.62, 0.64, 0.60, 0.60, 0.65, 0.65, 0.62, 0.58,
     *   0.59, 0.60, 0.60, 0.57, 0.52, 0.48, 0.50, 0.49, 0.42, 0.35,
     *   0.29, 0.28, 0.26/
  */


  Double Efffrq[] = {1.0,  1.1,  1.2,  1.3,  1.4,  1.5,  1.6,  1.7,  1.8,
		     1.9,  2.0,  2.0+1e-9,  2.3,  2.7,  3.0,  3.5,  3.7,  4.0,  4.0+1e-9,
		     5.0,  6.0,  7.0,  8.0,  8.0+1e-9, 12.0, 12.0+1e-9, 13.0, 14.0, 15.0,
		     16.0, 17.0, 18.0, 19.0, 24.0, 26.0, 26.5, 28.0, 33.0, 38.0,
		     40.0, 43.0, 48.0};
  Double Eff[] = {0.45, 0.48, 0.48, 0.45, 0.46, 0.45, 0.43, 0.44, 0.44,
		  0.49, 0.48, 0.52, 0.52, 0.51, 0.53, 0.55, 0.53, 0.54, 0.55,
		  0.54, 0.56, 0.62, 0.64, 0.60, 0.60, 0.65, 0.65, 0.62, 0.58,
		  0.59, 0.60, 0.60, 0.57, 0.52, 0.48, 0.50, 0.49, 0.42, 0.35,
		  0.29, 0.28, 0.26};

  Vector<Double> f,ef;

  f.takeStorage(IPosition(1,42),Efffrq);
  ef.takeStorage(IPosition(1,42),Eff);

  // Fractional -> K/Jy (for 25m)
  ef/=Double(5.622);

  // convert to voltagey units
  ef=sqrt(ef);

  ScalarSampledFunctional<Double> fx(f);
  ScalarSampledFunctional<Double> fy(ef);
  Interpolate1D<Double,Double> eff(fx,fy);
  eff.setMethod(Interpolate1D<Double,Double>::linear);
  for (Int ispw=0;ispw<nSpw();++ispw) {
    Double fghz=spwfreqs_(ispw)/1.e9;
    eff_(ispw)=eff(fghz);
  }

  //  cout << "spwfreqs_ = " << spwfreqs_ << endl;
  //  cout << "eff_      = " << eff_ << endl;


}

void EGainCurve::specify(const Record& specify) {

  LogMessage message;

  Bool doeff(False);
  Bool dogc(False);
  if (specify.isDefined("caltype")) {
    String caltype=specify.asString("caltype");
    //cout << "caltype=" << caltype  << endl;
    doeff = (caltype.contains("eff"));
    dogc = (caltype.contains("gc"));
  }

  // Open raw gain curve source table
  Table rawgaintab(gainCurveSrc_);

  logSink() << "Using " << Path(gainCurveSrc_).absoluteName()
	    << " nrow=" << rawgaintab.nrow() << LogIO::POST;

  // Select on Time
  Table timegaintab = rawgaintab(rawgaintab.col("BTIME") <= obstime_
                                 && rawgaintab.col("ETIME") > obstime_);

  // ...for each spw:
  spwOK_.resize(nSpw());  
  spwOK_.set(False);  // will set True when we find them

  for (Int ispw=0; ispw<nSpw(); ispw++) {

    currSpw()=ispw;


    if (dogc) {

      // Select on freq:
      Table freqgaintab=timegaintab(timegaintab.col("BANDNAME")==spwbands_(ispw));

      // If we fail to select anything by bandname, try to select by freq
      //   (this can get wrong answer near band edges if center freq "out-of-band")
      if (freqgaintab.nrow()==0)
	freqgaintab = timegaintab(timegaintab.col("BFREQ") <= spwfreqs_(ispw)
				  && timegaintab.col("EFREQ") > spwfreqs_(ispw));

      if (freqgaintab.nrow() > 0) {

	/*	
	{ logSink() // << "EGainCurve::specify:"
		    << "  Found the following gain curve coefficient data" << endl
		    << "  for spectral window = "  << ispw << " (band=" << spwbands_(ispw) << ", center freq="
		    << spwfreqs_(ispw) << "):" << LogIO::POST;
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

	solveAllParOK()=True;
	
	ArrayIterator<Float> piter(solveAllRPar(),1);
	
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
	  { 
	    logSink() << "   Antenna=" << antnames_(iant) << ": "
		      << piter.array().nonDegenerate() << LogIO::POST;
	  }
	  */
	}
	
	spwOK_(currSpw())=True;
	
      }
      else {
	logSink() << "Could not find gain curve data for Spw="
		  << ispw << " (reffreq=" << spwfreqs_(ispw)/1.0e9 << " GHz) "
		  << "Using flat unit gaincurve." << LogIO::POST;
	// We used to punt here
	//throw(AipsError(o.str()));
	
	// Use unity
	solveAllParOK()=True;
	solveAllRPar().set(0.0);
	solveAllRPar()(Slice(0,1,1),Slice(),Slice()).set(1.0);
	solveAllRPar()(Slice(4,1,1),Slice(),Slice()).set(1.0);
	
      }
    }
    else {
      // Use unity, flat
      solveAllParOK()=True;
      solveAllRPar().set(0.0);
      solveAllRPar()(Slice(0,1,1),Slice(),Slice()).set(1.0);
      solveAllRPar()(Slice(4,1,1),Slice(),Slice()).set(1.0);
      spwOK_(currSpw())=True;
    }

    // Scale by efficiency factor, if requested
    if (doeff) {
      solveAllRPar()*=Float(eff_(ispw));
    }

    // Record in the memory caltable
    keepNCT();

  } // ispw

  if (allEQ(spwOK_,False))
    throw(AipsError("Found no gaincurve data for any spw."));


  // Reset currSpw()
  currSpw()=0;

  // Resize za()
  za().resize(nAnt());

}


void EGainCurve::guessPar(VisBuffer&) {

  throw(AipsError("Spurious attempt to guess EGainCurve for solving!"));

}

// EGainCurve needs to zenith angle (old VB version)
void EGainCurve::syncMeta(const VisBuffer& vb) {

  // Call parent (sets currTime())
  SolvableVisJones::syncMeta(vb);

  // Current time's zenith angle...  (in _degrees_)
  za().resize(nAnt());
  Vector<MDirection> antazel(vb.azel(currTime()));
  Double* a=za().data();
  for (Int iant=0;iant<nAnt();++iant,++a) 
    (*a)=90.0 - antazel(iant).getAngle().getValue()(1)*180.0/C::pi;

}

// EGainCurve needs to zenith angle  (VB2 version) 
void EGainCurve::syncMeta2(const vi::VisBuffer2& vb) {

  // Call parent (sets currTime())
  SolvableVisJones::syncMeta2(vb);

  // Current time's zenith angle...(in _degrees_)
  za().resize(nAnt());
  Vector<MDirection> antazel(vb.azel(currTime()));
  Double* a=za().data();
  for (Int iant=0;iant<nAnt();++iant,++a) 
    (*a)=90.0 - antazel(iant).getAngle().getValue()(1)*180.0/C::pi;

}



void EGainCurve::calcPar() {

  if (prtlev()>6) cout << "      EGainCurve::calcPar()" << endl;

  // Could do the following in setApply, so it only happens once?
  if (ci_)
    SolvableVisCal::calcPar();
  else
    throw(AipsError("Problem in EGainCurve::calcPar()"));

  // Pars now valid, matrices not yet
  validateP();
  invalidateJ();

}

void EGainCurve::calcAllJones() {

  if (prtlev()>6) cout << "       EGainCurve::calcAllJones()" << endl;

  // Nominally no gain curve effect
  currJElem()=Complex(1.0);
  currJElemOK()=False;

  /*
  cout << "currSpw() = " << currSpw() << endl;
  cout << "   spwMap() = " << spwMap() << endl;
  cout << "   currRPar().shape() = " << currRPar().shape() << endl;
  cout << "   currRPar() = " << currRPar() << endl;
  */

  Complex* J=currJElem().data();
  Bool*    JOk=currJElemOK().data();
  Float*  c=currRPar().data();
  Double* a=za().data();

  Double loss, ang;
  for (Int iant=0; iant<nAnt(); ++iant,++a)
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


} //# NAMESPACE CASA - END
