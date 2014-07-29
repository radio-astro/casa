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

#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisBuffer.h>   // still used in apply context
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
#include <images/Images/PagedImage.h>

#include <casa/sstream.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>

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
  tectype_(""),
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

// Enforce calwt=F and spwmap=[0]
void FJones::setApply(const Record& apply) {

  // Force spwmap to all 0 (FJones pars are not spw-dep
  // NB: this is required before calling parent, which sets
  //  up spwmap-dep interpolation
  logSink() << " (" << this->typeName()
	     << ": Overriding with spwmap=[0] since " << this->typeName()
	     << " is not spw-dependent)"
	     << LogIO::POST;
  spwMap().assign(Vector<Int>(1,0));

  // Remove spwmap from input Record, and pass to generic coe
  Record newapply;
  newapply=apply;
  if (newapply.isDefined("spwmap"))
    newapply.removeField("spwmap");

  // Do conventional things
  SolvableVisJones::setApply(newapply);

  // Extract ionhgt_ from table header, if available
  if (ct_) {
    const TableRecord& tr(ct_->keywordSet());
    if (tr.isDefined("IonosphereHeight(km)"))
      ionhgt_=Quantity(tr.asDouble("IonosphereHeight(km)"),"km");
  }
  cout << "Using ionosphere height = " << ionhgt_ << endl;


  // Enforce no weight calibration (this is a phase-like correction)
  calWt()=False;

}


String FJones::applyinfo() {

  ostringstream o;
  o << typeName();
  o << ": table=" << calTableName();

  return String(o);

}

void FJones::setSpecify(const Record& specify) {

  // extract TEC retrieval mode... (madrigal, ionics, etc.)
  if (specify.isDefined("caltype")) {
    tectype_=upcase(specify.asString("caltype"));
    //tectype_.upcase();
    cout << "tectype_=" << tectype_ << endl;
  }
 
  // Trap simple ZTEC mode and call generic setSpecify
  if (tectype_=="ZTEC")
    return SolvableVisCal::setSpecify(specify);
 
  // extract intended cal table name...
  if (specify.isDefined("caltable")) {
    calTableName()=specify.asString("caltable");
    
    if (Table::isReadable(calTableName()))
      logSink() << "FYI: We are going to overwrite an existing CalTable: "
                << calTableName()
                << LogIO::POST;
  }
  // we are creating a table from scratch
  logSink() << "Creating " << typeName()
            << " table."
            << LogIO::POST;

  // tectype_-specific default ionosphere heights
  if (tectype_=="ITEC")
    ionhgt_ = Quantity(450.,"km");
  else if (tectype_=="MTEC")
    ionhgt_ = Quantity(350.,"km");

  // Extract user's ionhgt_, if specified
  Vector<Double> parameters;
  if (specify.isDefined("parameter")) {
    parameters=specify.asArrayDouble("parameter");
    if (parameters.nelements()==1) {
      ionhgt_=Quantity(parameters[0],"km");
      cout << "Using user-specified ionosphere height = " << ionhgt_ << endl;
    }
    else
      cout << "Using default ionosphere height ("+tectype_+") = " << ionhgt_ << endl;
  }
  else
    cout << "Using default ionosphere height ("+tectype_+") = " << ionhgt_ << endl;

  // Create a new caltable to fill up
  createMemCalTable();

  // Add ionhgt_ to table keyword
  if (ct_) {

    TableRecord& tr(ct_->rwKeywordSet());
    tr.define("IonosphereHeight(km)",ionhgt_.getValue("km"));

  }


  // Set up solveAllRPar
  initSolvePar();

  // Initialize parameter space
  solveAllRPar()=0.0;
  solveAllParOK()=True;
  solveAllParErr()=0.0;   // what to use here?
  solveAllParSNR()=100.0; // what to use here?

}

void FJones::specify(const Record& specify) {

  // Trap simple ZTEC mode and call generic setSpecify
  if (tectype_=="ZTEC")
    return SolvableVisCal::specify(specify);

  cout << "FJones-specific specify: " << tectype_ << endl;

  String tecimname;
  tecimname=msName();
  if (tectype_=="ITEC") 
    tecimname+=".IGS_TEC.im";
  else if (tectype_=="MTEC")
    tecimname+=".MAPGPS_TEC.im";
  else
    throw(AipsError("Unrecognized TEC mode: "+tectype_));

  if ( !Table::isReadable(tecimname) )
    throw(AipsError("Cannot find expected TEC image: "+tecimname));
  else
    cout << "Found required TEC image: "+tecimname << endl;

  PagedImage<Float> tecim(tecimname);
  CoordinateSystem teccs(tecim.coordinates());

  // Specific sort columns for FJones
  Block<Int> cols(5);
  cols[0]=MS::ARRAY_ID;
  cols[1]=MS::SCAN_NUMBER;
  cols[2]=MS::TIME;
  cols[3]=MS::FIELD_ID;      // NB: assumed unique per timestamp!
  cols[4]=MS::DATA_DESC_ID;  // required but irrelevant in this context
  
  MeasurementSet ms(msName());
  vi::SortColumns sc(cols);
  vi::VisibilityIterator2 vi2(ms,sc,True);
  vi::VisBuffer2 *vb = vi2.getVisBuffer();
  vi2.originChunks();
  vi2.origin();

  ROMSAntennaColumns antcol(ms.antenna());

  // Measures stuff for ionosphere coords
  MeasFrame mframe;
  MEpoch when(MVEpoch(Quantity(vb->time()(0),"s")),MEpoch::Ref(MEpoch::UTC));
  mframe.set(when);
  MPosition where;
  MeasTable::Observatory(where,"VLA");
  MVPosition vla=where.getValue();
  mframe.set(where);
  MDirection::Ref mdref=vb->phaseCenter().getRef();
  EarthMagneticMachine* emm = new EarthMagneticMachine(mdref,ionhgt_,mframe);

  // Workspace for geometry/TEC retrieval/interp
  MDirection md;
  MVPosition ionpos;

  Vector<Double> cf(3);
  Vector<Int> c(3);
  IPosition corner(3,0,0,0),lastcorner(3,0,0,0),twos(3,2,2,2);

  Double day0(-999.0);
  Vector<Double> refv(teccs.referenceValue());
  if (refv[2]>0.0)
    day0=0.0;

  Vector<Double> wc(3,0.0), wc0(3,0.0);
  Double &lng(wc(0)), &lat(wc(1));
  wc0=teccs.toWorld(lastcorner);
  Double &lng0(wc0(0)), &lat0(wc0(1)), &time0(wc0(2));
  Double dlng(0.0),dlat(0.0),dtime(0.0);

  Vector<Double> inc(teccs.increment());
  Double& dlng0(inc(0)), dlat0(inc(1)), dtime0(inc(2));

  Cube<Float> tecube(2,2,2,0.0);  // lng, lat, time
  tecube=tecim.getSlice(lastcorner,lastcorner+2,False);
  Float &a0(tecube(0,0,0)), &a1(tecube(1,0,0)), &a2(tecube(1,1,0)), &a3(tecube(0,1,0));
  Float &b0(tecube(0,0,1)), &b1(tecube(1,0,1)), &b2(tecube(1,1,1)), &b3(tecube(0,1,1));

  Float t0(0.0), u0(0.0), v0(0.0);
  Float teca(0.0), tecb(0.0), tec(4e18);

  Double lasttime(-1);
  Int lastscan(-999);
  Int isol(0);
  for (vi2.originChunks(); vi2.moreChunks(); vi2.nextChunk()) {
    
    for (vi2.origin(); vi2.more(); vi2.next()) {

      refTime()=vb->time()(0);
      if (day0<0.0)
	day0=86400.0*floor(refTime()/86400.0);

      wc(2)=refTime()-day0;

      currScan()=vb->scan()(0);
      
      // only when the timestamp changes
      if (refTime()!=lasttime) {

	// Set meta-info (TBD: use syncMeta?)
	currSpw()=0;  // only spw=0 for FJones (TEC is not freq-dep)
	currObs()=vb->observationId()(0);
	currScan()=vb->scan()(0);
	currField()=vb->fieldId()(0);

	// This vb's direction measure...
	md=vb->phaseCenter();

	when.set(Quantity(refTime(),"s"));
	mframe.set(when);

	for (Int iant=0;iant<nAnt();++iant) {
	  mframe.set(antcol.positionMeas()(iant));

	  // Force measures calculation...
	  emm->set(mframe);  // ensure frame is updating...
	  emm->calculate(md.getValue());  
	  ionpos=emm->getPosition();
	  wc(0)=ionpos.getLong("deg").getValue();
	  wc(1)=ionpos.getLat("deg").getValue();

	  // Find corner of 2x2x2 cube that we need
	  teccs.toPixel(cf,wc);  // fractional pixel coord
	  convertArray(c,cf);     // integer pixel coord
	  corner=IPosition(c);   // as IPosition
	  if (corner!=lastcorner) { // If new
	    wc0=teccs.toWorld(corner);
	    tecube=tecim.getSlice(corner,twos,False);
	    lastcorner=corner;

	    /*
	    cout << "***tecube = " << tecube << endl;
	    cout << a0 << " " << a1 << " " << a2 << " " << a3
		 << b0 << " " << b1 << " " << b2 << " " << b3 << endl;


	    if (iant<4)
	      cout << "**";
	  }
	  else
	    if (iant<4)
	      cout << "  ";
	    */
	  }

	  dlng=lng-lng0;
	  dlat=lat-lat0;
	  dtime=wc(2)-time0;

	  t0=dlng/dlng0;
	  u0=dlat/dlat0;
	  v0=dtime/dtime0;

	  if (False && iant==0) {
	    cout << dlng << " " << dlng0 << " " << t0 << endl;
	    cout << dlat << " " << dlat0 << " " << u0 << endl;
	    cout << dtime << " " << dtime0 << " " << v0 << endl;
	  }

	  teca=(1.-t0)*(1.-u0)*a0 + t0*(1.-u0)*a1 + t0*u0*a2 + (1.-t0)*u0*a3;
	  tecb=(1.-t0)*(1.-u0)*b0 + t0*(1.-u0)*b1 + t0*u0*b2 + (1.-t0)*u0*b3;
	  tec=(1.-v0)*teca + v0*tecb;
	  
	  // retrieve time-, direction-, and antenna- specific vertical TEC...
	  solveAllRPar()(0,0,iant)=tec*1.e16;   // in e-/m2

	  if (iant==0 && currScan()!=lastscan) {
	    lastscan=currScan();

	    MVDirection azel=vb->azel0(refTime()).getValue();
	    cout.precision(12);
	    cout << isol << " "
		 << " obs="  << currObs()
		 << " scan=" << currScan() 
		 << " fld="  << currField()
		 << " time=" << wc(2)
		 << " ant=" << antcol.name()(iant)+":"+antcol.station()(iant)
	      //<< " az/el="   << azel.getLong("deg").getValue() << "/" << azel.getLat("deg").getValue()
		 << " long/lat=" << lng << "/" << lat
	      //<< " pixel=" << cf << " " << c
		 << " tec=" // << teca << " " << tecb << " " 
		 << tec
		 << endl;
	  }
	}

	// store in the caltable
	keepNCT();
	
	++isol;

      }

      lasttime=refTime();

    }
  }
}

void FJones::syncJones(const Bool& doInv) {

  // Circulars
  if (vb().corrType()(0)==5)
    pjonestype_=Jones::Diagonal;

  // Linears
  else if (vb().corrType()(0)==9)
    pjonestype_=Jones::General;

  VisJones::syncJones(doInv);

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
    emm_->set(mframe_);  // seems to be needed to force new position in emm calculations

    // calculate this ant's field
    emm_->calculate(phasedirV);

    BlosG_(iant)=(emm_->getLOSField("G").getValue());    // Sign?

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
  Double f,rotpers2,tec,del,rot;
  Complex cdel;
  
  for (Int iant=0; iant<nAnt(); ++iant,++ztec,++ztecok,++a) {
    if ((*ztecok) && (*a)<C::pi_2) {
      tec = Double(*ztec)/cos(*a);
      rotpers2 = radper_*tec*BlosG_(iant);

      for (Int ich=0;ich<vb().nChannel();++ich) {
	f=vb().frequency()(ich);   // Hz
	del = 8.4483e-7*tec/f;
	rot = rotpers2/f/f;

	switch (jonesType()) {
	  // Circular version:
	case Jones::Diagonal: {
	  J[0]=Complex(cos(del-rot),sin(del-rot));
	  J[1]=Complex(cos(del+rot),sin(del+rot));
	  J+=2;
	  break;
	}
	  // Linear version:
	case Jones::General: {
	  cdel=Complex(cos(del),sin(del));
	  J[0]=J[3]=cdel*cos(rot);
	  J[1]=cdel*sin(rot);
	  J[2]=cdel*sin(-rot);
	  J+=4;
	  break;
	}
	default:
	  throw(AipsError("PJones doesn't know if it is Diag (Circ) or General (Lin)"));
	  break;
	  
	}

      }
    }

  }
  /*
  cout << "tec = " << tec << endl;
  cout << "rot = " << rot << " " << rotpers2 << endl;
  cout << "currJElem() = " << currJElem() << endl;
  */
  validateJ();
}



} //# NAMESPACE CASA - END
