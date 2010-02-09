//# newsimulator.cc: Simulation  program
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: Simulator.cc,v 1.1.2.4 2006/10/06 21:03:19 kgolap Exp $

#include <stdexcept>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>

#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/File.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordInterface.h>

#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/ExprNode.h>

#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Fallible.h>

#include <casa/BasicSL/Constants.h>

#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <casa/Arrays/ArrayMath.h>

#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisSetUtil.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/VisCalGlobals.h>
#include <ms/MeasurementSets/NewMSSimulator.h>

#include <measures/Measures/Stokes.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/Quanta/MVAngle.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/MVEpoch.h>
#include <measures/Measures/MEpoch.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>

#include <ms/MeasurementSets/MSSummary.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <synthesis/MeasurementComponents/ImageSkyModel.h>
#include <synthesis/MeasurementComponents/SimACohCalc.h>
#include <synthesis/MeasurementComponents/SimACoh.h>
//#include <synthesis/MeasurementComponents/SimVisJones.h>
#include <synthesis/MeasurementComponents/VPSkyJones.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <lattices/Lattices/LatticeExpr.h> 

#include <synthesis/MeasurementEquations/Simulator.h>
#include <synthesis/MeasurementComponents/CleanImageSkyModel.h>
#include <synthesis/MeasurementComponents/GridBoth.h>
#include <synthesis/MeasurementComponents/WProjectFT.h>
#include <synthesis/MeasurementComponents/GridBoth.h>
#include <synthesis/MeasurementComponents/MosaicFT.h>
#include <synthesis/MeasurementComponents/SimpleComponentFTMachine.h>
#include <casa/OS/HostInfo.h>
#include <images/Images/PagedImage.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/sstream.h>

#include <casa/namespace.h>


Simulator::Simulator(): 
  msname_p(String("")), ms_p(0), mssel_p(0), vs_p(0), 
  seed_p(11111),
  ve_p(),
  vc_p(),
  ac_p(0), vp_p(0), gvp_p(0), 
  sim_p(0),
  // epJ_p(0),
  epJTableName_p(),
  nSpw(0)
{
}


Simulator::Simulator(String& msname) 
  : msname_p(msname), ms_p(0), mssel_p(0), vs_p(0), seed_p(11111),
    ve_p(),
    vc_p(),
    ac_p(0), vp_p(0), gvp_p(0), 
    sim_p(0),
    // epJ_p(0),
    epJTableName_p()
{
#ifdef RI_DEBUG
  LogIO os(LogOrigin("simulator", "simulator(String& msname)", WHERE));
#else
  LogIO os(LogOrigin("simulator", "simulator(String& msname)"));
#endif

  defaults();

  if(!sim_p) {
    sim_p= new NewMSSimulator(msname);
  }
  
  // Make a MeasurementSet object for the disk-base MeasurementSet that we just
  // created
  ms_p = new MeasurementSet(msname, TableLock(TableLock::AutoNoReadLocking), 
			    Table::Update);
  AlwaysAssert(ms_p, AipsError);

}


Simulator::Simulator(MeasurementSet &theMs)
  : msname_p(""), ms_p(0), mssel_p(0), vs_p(0), seed_p(11111),
    ve_p(),
    vc_p(),
    ac_p(0), vp_p(0), gvp_p(0), 
    sim_p(0),
    // epJ_p(0),
    epJTableName_p()
{
  LogIO os(LogOrigin("simulator", "simulator(MeasurementSet& theMs)", WHERE));

  defaults();

  msname_p=theMs.tableName();

  if(!sim_p) {
    sim_p= new NewMSSimulator(theMs);
  }

  ms_p = new MeasurementSet(theMs);
  AlwaysAssert(ms_p, AipsError);

  // get info from the MS into Simulator:
  if (!getconfig()) 
    os << "Can't find antenna information for loaded MS" << LogIO::WARN;
  if (!sim_p->getSpWindows(nSpw,spWindowName_p,nChan_p,startFreq_p,freqInc_p,stokesString_p))
    os << "Can't find spectral window information for loaded MS" << LogIO::WARN;
  if (!sim_p->getFields(nField,sourceName_p,sourceDirection_p,calCode_p))
    os << "Can't find Field/Source information for loaded MS" << LogIO::WARN;

  if (!sim_p->getFeedMode(feedMode_p))
    os << "Can't find Feed information for loaded MS" << LogIO::WARN;
  else
    feedsHaveBeenSet=True;

}



Simulator::Simulator(const Simulator &other)
  : msname_p(""), ms_p(0), vs_p(0), seed_p(11111),
    ve_p(),
    vc_p(),
    ac_p(0), vp_p(0), gvp_p(0),
    sim_p(0),
    // epJ_p(0),
    epJTableName_p()
{
  defaults();
  ms_p = new MeasurementSet(*other.ms_p);
  if(other.mssel_p) {
    mssel_p = new MeasurementSet(*other.mssel_p);
  }
}

Simulator &Simulator::operator=(const Simulator &other)
{
  if (ms_p && this != &other) {
    *ms_p = *(other.ms_p);
  }
  if (mssel_p && this != &other && other.mssel_p) {
    *mssel_p = *(other.mssel_p);
  }
  if (vs_p && this != &other) {
    *vs_p = *(other.vs_p);
  }
  if (ac_p && this != &other) {
    *ac_p = *(other.ac_p);
  }

  // TBD VisEquation/VisCal stuff

  if (vp_p && this != &other) {
    *vp_p = *(other.vp_p);
  }
  if (gvp_p && this != &other) {
    *gvp_p = *(other.gvp_p);
  }
  if (sim_p && this != &other) {
    *sim_p = *(other.sim_p);
  }
  //  if (epJ_p && this != &other) *epJ_p = *(other.epJ_p);
  return *this;
}

Simulator::~Simulator()
{
  if (ms_p) {
    ms_p->relinquishAutoLocks();
    ms_p->unlock();
    delete ms_p;
  }
  ms_p = 0;
  if (mssel_p) {
    mssel_p->relinquishAutoLocks();
    mssel_p->unlock();
    delete mssel_p;
  }
  mssel_p = 0;
  if (vs_p) {
    delete vs_p;
  }
  vs_p = 0;

  // Delete all vis-plane calibration corruption terms
  resetviscal();

  // Delete all im-plane calibration corruption terms
  resetimcal();

  if(sim_p) delete sim_p; sim_p = 0;

  if(sm_p) delete sm_p; sm_p = 0;
  if(ft_p) delete ft_p; ft_p = 0;
  if(cft_p) delete cft_p; cft_p = 0;

}


void Simulator::defaults()
{
  UnitMap::putUser("Pixel", UnitVal(1.0), "Pixel solid angle");
  UnitMap::putUser("Beam", UnitVal(1.0), "Beam solid angle");
  gridfunction_p="SF";
  // Use half the machine memory as cache. The user can override
  // this via the setoptions function().
  cache_p=(HostInfo::memoryTotal()/8)*1024;

  tile_p=16;
  ftmachine_p="gridft";
  padding_p=1.3;
  facets_p=1;
  sm_p = 0;
  ft_p = 0;
  cft_p = 0;
  vp_p = 0;
  gvp_p = 0;
  sim_p = 0;
  images_p = 0;
  nmodels_p = 1;
  // info for configurations
  areStationCoordsSet_p = False;
  telescope_p = "UNSET";
  nmodels_p = 0;

  // info for fields and schedule:
  nField=0;
  sourceName_p.resize(1);
  sourceName_p[0]="UNSET";
  calCode_p.resize(1);
  calCode_p[0]="";
  sourceDirection_p.resize(1);  

  // info for spectral windows
  nSpw=0;
  spWindowName_p.resize(1);
  nChan_p.resize(1);
  startFreq_p.resize(1);
  freqInc_p.resize(1);
  freqRes_p.resize(1);
  stokesString_p.resize(1);
  spWindowName_p[0]="UNSET";
  nChan_p[0]=1;
  startFreq_p[0]=Quantity(50., "GHz");
  freqInc_p[0]=Quantity(0.1, "MHz");
  freqRes_p[0]=Quantity(0.1, "MHz");
  stokesString_p[0]="RR RL LR LL";

  // feeds
  feedMode_p = "perfect R L";
  nFeeds_p = 1;
  feedsHaveBeenSet = False;
  feedsInitialized = False;

  // times
  integrationTime_p = Quantity(10.0, "s");
  useHourAngle_p=True;
  refTime_p = MEpoch(Quantity(0.0, "s"), MEpoch::UTC);
  timesHaveBeenSet_p=False;

  // VP stuff
  doVP_p=False;
  doDefaultVP_p = True;

};


Bool Simulator::close()
{
  LogIO os(LogOrigin("Simulator", "close()", WHERE));
  os << "Closing MeasurementSet and detaching from Simulator"
     << LogIO::POST;

  // Delete all im-plane calibration corruption terms
  resetimcal();
  // Delete all vis-plane calibration corruption terms
  resetviscal();

  ms_p->unlock();
  if(mssel_p) mssel_p->unlock();
  if(vs_p) delete vs_p; vs_p = 0;
  if(mssel_p) delete mssel_p; mssel_p = 0;
  if(ms_p) delete ms_p; ms_p = 0;
  if(sm_p) delete sm_p; sm_p = 0;
  if(ft_p) delete ft_p; ft_p = 0;
  if(cft_p) delete cft_p; cft_p = 0;

  return True;
}

Bool Simulator::resetviscal() {
  LogIO os(LogOrigin("Simulator", "reset()", WHERE));
  try {

    os << "Resetting all visibility corruption components" << LogIO::POST;
    
    // The noise term (for now)
    if(ac_p) delete ac_p; ac_p=0;

    // Delete all VisCals
    for (uInt i=0;i<vc_p.nelements();++i)
      if (vc_p[i]) delete vc_p[i];
    vc_p.resize(0,True);

    // reset the VisEquation (by sending an empty vc_p)
    ve_p.setapply(vc_p);

  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}


Bool Simulator::resetimcal() {
  LogIO os(LogOrigin("Simulator", "reset()", WHERE));
  try {

    os << "Reset all image-plane corruption components" << LogIO::POST;
    
    if(vp_p) delete vp_p; vp_p=0;
    if(gvp_p) delete gvp_p; gvp_p=0;
    /*
    //    if(epJ_p) delete epJ_p; epJ_p=0;
    */

  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}


Bool Simulator::reset() {
  LogIO os(LogOrigin("Simulator", "reset()", WHERE));
  try {
    
    // reset vis-plane cal terms
    resetviscal();

    // reset im-plane cal terms
    resetimcal();

  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}


String Simulator::name() const
{
  if (detached()) {
    return "none";
  }
  return msname_p;
}

String Simulator::state()
{
  ostringstream os;
  os << "Need to write the state() method!" << LogIO::POST;
  if(doVP_p) {
    os << "  Primary beam correction is enabled" << endl;
  }
  return String(os);
}

Bool Simulator::summary()
{
  LogIO os(LogOrigin("Simulator", "summary()", WHERE));
  createSummary(os);
  predictSummary(os);
  corruptSummary(os);

  return True;
}


Bool Simulator::createSummary(LogIO& os) 
{
  Bool configResult = configSummary(os);
  Bool fieldResult = fieldSummary(os);
  Bool windowResult = spWindowSummary(os);
  Bool feedResult = feedSummary(os);

  if (!configResult && !fieldResult && !windowResult && !feedResult) {
    os << "=======================================" << LogIO::POST;
    os << "No create-type information has been set" << LogIO::POST;
    os << "=======================================" << LogIO::POST;
    return False;
  } else {
    // user has set at least ONE, so we report on each
    if (!configResult) {
      os << "No configuration information set yet, but other create-type info HAS been set" << LogIO::POST;
    }
    if (!fieldResult) {
      os << "No field information set yet, but other create-type info HAS been set" << LogIO::POST;
    }
    if (!windowResult) {
      os << "No window information set yet, but other create-type info HAS been set" << LogIO::POST;
    }
    if (!feedResult) {
      os << "No feed information set yet, but other create-type info HAS been set" << LogIO::POST;
      os << "(feeds will default to perfect R-L feeds if not set)" << LogIO::POST;
    }
    os << "======================================================================" << LogIO::POST;
  }
  return True;
}



Bool Simulator::configSummary(LogIO& os)
{
  if ( ! areStationCoordsSet_p ) {
    return False;
  } else {
    os << "----------------------------------------------------------------------" << LogIO::POST;
    os << "Generating (u,v,w) using this configuration: " << LogIO::POST;
    os << "   x     y     z     diam     mount     station " << LogIO::POST;
    for (uInt i=0; i< x_p.nelements(); i++) {
      os << x_p(i)
	 << "  " << y_p(i)
	 << "  " << z_p(i)
	 << "  " << diam_p(i)
	 << "  " << mount_p(i)
	 << "  " << padName_p(i)
	 << LogIO::POST;
    }
    os << " Coordsystem = " << coordsystem_p << LogIO::POST;
    os << " RefLocation = " << 
      mRefLocation_p.getAngle("deg").getValue("deg") << LogIO::POST;
  }
  return True;

}



Bool Simulator::fieldSummary(LogIO& os)
{
  os << "----------------------------------------------------------------------" << LogIO::POST;
  os << " Field information: " << LogIO::POST;
  if (nField==0)
    os << "NO Field window information set" << LogIO::POST;
  else 
    os << " Name  direction  calcode" << LogIO::POST; 
  for (Int i=0;i<nField;i++) 
    os << sourceName_p[i] 
       << "  " << formatDirection(sourceDirection_p[i])
       << "  " << calCode_p[i]
       << LogIO::POST;
  return True;
}



Bool Simulator::timeSummary(LogIO& os)
{
  if(integrationTime_p.getValue("s") <= 0.0) {
    return False;
  } else {
    os << "----------------------------------------------------------------------" << LogIO::POST;
    os << " Time information: " << LogIO::POST;
    os << " integration time = " << integrationTime_p.getValue("s") 
       << " s" << LogIO::POST;
    os << " reference time = " << MVTime(refTime_p.get("s").getValue("d")).string()
       << LogIO::POST;
  }
  return True;
}



Bool Simulator::spWindowSummary(LogIO& os)
{
  os << "----------------------------------------------------------------------" << LogIO::POST;
  os << " Spectral Windows information: " << LogIO::POST;
  if (nSpw==0)
    os << "NO Spectral window information set" << LogIO::POST;
  else 
    os << " Name  nchan  freq[GHz]  freqInc[MHz]  freqRes[MHz]  stokes" << LogIO::POST;
  for (Int i=0;i<nSpw;i++) 
    os << spWindowName_p[i] 	 
       << "  " << nChan_p[i]
       << "  " << startFreq_p[i].getValue("GHz")
       << "  " << freqInc_p[i].getValue("MHz")
       << "  " << freqRes_p[i].getValue("MHz")
       << "  " << stokesString_p[i]
       << LogIO::POST;
  return True;
}


Bool Simulator::feedSummary(LogIO& os)
{
  if (!feedsHaveBeenSet) {
    return False;
  } else {
    os << "----------------------------------------------------------------------" << LogIO::POST;
    os << " Feed information: " << LogIO::POST;
    os << feedMode_p << LogIO::POST;
  }
  return True;
}


Bool Simulator::predictSummary(LogIO& os)
{
  Bool vpResult = vpSummary(os);
  Bool optionsResult = optionsSummary(os);

  // keep compiler happy
  if (!vpResult && !optionsResult) {}
  return True;
}


Bool Simulator::vpSummary(LogIO& os)
{
  if (vp_p) {
    vp_p->summary();
    return True;
  } else {
    return False;
  }
}


Bool Simulator::optionsSummary(LogIO& os)
{
  return True;
}
 

Bool Simulator::corruptSummary(LogIO& os)
{
  if (vc_p.nelements()<1 && !ac_p) {
    os << "===========================================" << LogIO::POST;
    os << "No corrupting-type information has been set" << LogIO::POST;
    os << "===========================================" << LogIO::POST;
    return False;
  }
  else {
    os << "Visibilities will be CORRUPTED with the following terms:" << LogIO::POST;

    Int napp(vc_p.nelements());
    for (Int iapp=0;iapp<napp;++iapp)
      os << LogIO::NORMAL << ".   "
	 << vc_p[iapp]->siminfo()
	 << LogIO::POST;
    
    // Report also on the noise settings
    noiseSummary(os);  

  }
  return True;
}


Bool Simulator::noiseSummary(LogIO& os)
{
  if (!ac_p) {
   return False;
  } else {
    os << "Thermal noise corruption activated" << LogIO::POST;
    os << "Thermal noise mode: " << noisemode_p << LogIO::POST;
  }
  return True;
}













//========================================================================
//       SETUP OBSERVATION


Bool Simulator::settimes(const Quantity& integrationTime, 
			 const Bool      useHourAngle,
			 const MEpoch&   refTime)
{
  
  LogIO os(LogOrigin("simulator", "settimes()", WHERE));
  try {
    
    integrationTime_p=integrationTime;
    useHourAngle_p=useHourAngle;
    refTime_p=refTime;

    os << "Times " << endl
       <<  "     Integration time " << integrationTime.getValue("s") << "s" << LogIO::POST;
    if(useHourAngle) {
      os << "     Times will be interpreted as hour angles for first source" << LogIO::POST;
    }

    sim_p->settimes(integrationTime, useHourAngle, refTime);
    
    timesHaveBeenSet_p=True;
    
    return True;
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
  
}



Bool Simulator::setseed(const Int seed) {
  seed_p = seed;
  return True;
}



Bool Simulator::setconfig(const String& telname,
			     const Vector<Double>& x, 
			     const Vector<Double>& y, 
			     const Vector<Double>& z,
			     const Vector<Double>& dishDiameter,
			     const Vector<Double>& offset,
			     const Vector<String>& mount,
			     const Vector<String>& antName,
			     const Vector<String>& padName,
			     const String& coordsystem,
			     const MPosition& mRefLocation) 
{

  telescope_p = telname;
  x_p.resize(x.nelements());
  x_p = x;
  y_p.resize(y.nelements());
  y_p = y;
  z_p.resize(z.nelements());
  z_p = z;
  diam_p.resize(dishDiameter.nelements());
  diam_p = dishDiameter;
  offset_p.resize(offset.nelements());
  offset_p = offset;
  mount_p.resize(mount.nelements());
  mount_p = mount;
  antName_p.resize(antName.nelements());
  antName_p = antName;
  padName_p.resize(padName.nelements());
  padName_p = padName;
  coordsystem_p = coordsystem;
  mRefLocation_p = mRefLocation;

  uInt nn = x_p.nelements();

  if (diam_p.nelements() == 1) {
    diam_p.resize(nn);
    diam_p.set(dishDiameter(0));
  }
  if (mount_p.nelements() == 1) {
    mount_p.resize(nn);
    mount_p.set(mount(0));
  }
  if (mount_p.nelements() == 0) {
    mount_p.resize(nn);
    mount_p.set("alt-az");
  }
  if (offset_p.nelements() == 1) {
    offset_p.resize(nn);
    offset_p.set(offset(0));
  }
  if (offset_p.nelements() == 0) {
    offset_p.resize(nn);
    offset_p.set(0.0);
  }
  if (antName_p.nelements() == 1) {
    antName_p.resize(nn);
    antName_p.set(antName(0));
  }
  if (antName_p.nelements() == 0) {
    antName_p.resize(nn);
    antName_p.set("UNKNOWN");
  }
  if (padName_p.nelements() == 1) {
    padName_p.resize(nn);
    padName_p.set(padName(0));
  }
  if (padName_p.nelements() == 0) {
    padName_p.resize(nn);
    padName_p.set("UNKNOWN");
  }

  AlwaysAssert( (nn == y_p.nelements())  , AipsError);
  AlwaysAssert( (nn == z_p.nelements())  , AipsError);
  AlwaysAssert( (nn == diam_p.nelements())  , AipsError);
  AlwaysAssert( (nn == offset_p.nelements())  , AipsError);
  AlwaysAssert( (nn == mount_p.nelements())  , AipsError);

  areStationCoordsSet_p = True;
  
  sim_p->initAnt(telescope_p, x_p, y_p, z_p, diam_p, offset_p, mount_p, antName_p, padName_p,
		 coordsystem_p, mRefLocation_p);
  
  return True;  
}



Bool Simulator::getconfig() {
  // get it from NewMSSimulator
  Matrix<Double> xyz_p;
  Int nAnt;
  if (sim_p->getAnt(telescope_p, nAnt, &xyz_p, diam_p, offset_p, mount_p, antName_p, padName_p, 
		    coordsystem_p, mRefLocation_p)) {
    x_p.resize(nAnt);
    y_p.resize(nAnt);
    z_p.resize(nAnt);
    for (Int i=0;i<nAnt;i++) {
      x_p(i)=xyz_p(0,i);
      y_p(i)=xyz_p(1,i);
      z_p(i)=xyz_p(2,i);
    }
    areStationCoordsSet_p = True;
    return True;
  } else {
    return False;
  }
}



Bool Simulator::setfield(const String& sourceName,           
			 const MDirection& sourceDirection,  
			 const String& calCode,
			 const Quantity& distance)
{
  LogIO os(LogOrigin("Simulator", "setfield()", WHERE));
  //#ifndef RI_DEBUG
  try {
//#else 
//    os << LogIO::WARN << "debug mode - not catching errors." << LogIO::POST;  
//#endif
    
    if (sourceName == "") {
      os << LogIO::SEVERE << "must provide a source name" << LogIO::POST;  
      return False;
    }

    nField++;    
#ifdef RI_DEBUG
    os << "nField = " << nField << LogIO::POST;  
#endif
    distance_p.resize(nField,True);
    distance_p[nField-1]=distance;
    sourceName_p.resize(nField,True);
    sourceName_p[nField-1]=sourceName;
    sourceDirection_p.resize(nField,True);
    sourceDirection_p[nField-1]=sourceDirection;
    calCode_p.resize(nField,True);
    calCode_p[nField-1]=calCode;

    sim_p->initFields(sourceName, sourceDirection, calCode);

    //#ifndef RI_DEBUG
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  //#endif
  return True;
};




Bool Simulator::setmosaicfield(const String& sourcename, const String& calcode, const MDirection& fieldcenter, const Int xmosp, const Int ymosp, const Quantity& mosspacing, const Quantity& distance)
{

        Int nx = xmosp;
        Int ny = ymosp;          
        Double roffset = mosspacing.getValue("rad");
        Double newraval, newdecval;
        uInt k=0;
        for (Int i=0; i< nx; ++i) {
          for(Int j=0; j < ny; ++j) {
            if((nx/2)!=floor(nx/2)) { // odd number of fields in x direction(ra)
              newraval = fieldcenter.getAngle().getValue("rad")(0) + 
		(i-ceil(nx/2.0))*roffset/cos(fieldcenter.getAngle().getValue("rad")(1));
            }
            else { //even case
              newraval = fieldcenter.getAngle().getValue("rad")(0) + 
		((i-ceil(nx/2)) - 0.5)*roffset/cos(fieldcenter.getAngle().getValue("rad")(1));
            }
            if((ny/2)!=floor(ny/2)) {
              newdecval = fieldcenter.getAngle().getValue("rad")(1) + 
		(j-ceil(ny/2.0))*roffset;
            }
            else {
              newdecval = fieldcenter.getAngle().getValue("rad")(1) + 
		((j-ceil(ny/2.0)) - 0.5)*roffset;
            }
            if(newraval >2.0*C::pi) {
              newraval = newraval - 2.0*C::pi;
            }
	    Int sign;
            if(abs(newdecval) >C::pi/2) {
              if(newdecval<0) {
                 sign = -1;
              }
              else {
		sign = 1;
              }
              newdecval =  sign*(C::pi - abs(newdecval));
              newraval = abs(C::pi - newraval); 
            } 
            Quantity newdirra(newraval, "rad");
            Quantity newdirdec(newdecval, "rad");
            MDirection newdir(newdirra, newdirdec);
	    newdir.setRefString(fieldcenter.getRefString());
	    ostringstream oos;
	    oos << sourcename << "_" << k ;
  

           setfield(String(oos), newdir, calcode, distance); 
    
            ++k;
          }
	}


	return True;
}



Bool Simulator::setspwindow(const String& spwName,           
			    const Quantity& freq,
			    const Quantity& deltafreq,
			    const Quantity& freqresolution,
			    const Int nChan,
			    const String& stokes) 

{
  LogIO os(LogOrigin("Simulator", "setspwindow()", WHERE));
  try {
    if (nChan == 0) {
      os << LogIO::SEVERE << "must provide nchannels" << LogIO::POST;  
      return False;
    }

    nSpw++;    
#ifdef RI_DEBUG
    os << "nspw = " << nSpw << LogIO::POST;  
#endif
    spWindowName_p.resize(nSpw,True);
    spWindowName_p[nSpw-1] = spwName;   
    nChan_p.resize(nSpw,True);
    nChan_p[nSpw-1] = nChan;
    startFreq_p.resize(nSpw,True);
    startFreq_p[nSpw-1] = freq;
    freqInc_p.resize(nSpw,True);
    freqInc_p[nSpw-1] = deltafreq;
    freqRes_p.resize(nSpw,True);
    freqRes_p[nSpw-1] = freqresolution;        
    stokesString_p.resize(nSpw,True);
    stokesString_p[nSpw-1] = stokes;   

#ifdef RI_DEBUG
    os << "sending init to MSSim for spw = " << spWindowName_p[nSpw-1] << LogIO::POST;  
#endif

    sim_p->initSpWindows(spWindowName_p[nSpw-1], nChan_p[nSpw-1], 
			 startFreq_p[nSpw-1], freqInc_p[nSpw-1], 
			 freqRes_p[nSpw-1], stokesString_p[nSpw-1]);

  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
};


Bool Simulator::setfeed(const String& mode,
			   const Vector<Double>& x,
			   const Vector<Double>& y,
			   const Vector<String>& pol)
{
  LogIO os(LogOrigin("Simulator", "setfeed()", WHERE));
  
  if (mode != "perfect R L" && mode != "perfect X Y" && mode != "list") {
    os << LogIO::SEVERE << 
      "Currently, only perfect R L or perfect X Y feeds or list are recognized" 
       << LogIO::POST;
    return False;
  }
  sim_p->initFeeds(feedMode_p, x, y, pol);

  feedMode_p = mode;
  nFeeds_p = x.nelements();
  feedsHaveBeenSet = True;

  return True;
};




Bool Simulator::setvp(const Bool dovp,
			 const Bool doDefaultVPs,
			 const String& vpTable,
			 const Bool doSquint,
			 const Quantity &parAngleInc,
			 const Quantity &skyPosThreshold,
			 const Float &pbLimit)
{
  LogIO os(LogOrigin("Simulatore", "setvp()", WHERE));
  
  os << "Setting voltage pattern parameters" << LogIO::POST;
  
  doVP_p=dovp;
  doDefaultVP_p = doDefaultVPs;
  vpTableStr_p = vpTable;
  if (doSquint) {
    squintType_p = BeamSquint::GOFIGURE;
  } else {
    squintType_p = BeamSquint::NONE;
  }
  parAngleInc_p = parAngleInc;
  skyPosThreshold_p = skyPosThreshold;

  if (doDefaultVP_p) {
    os << "Using system default voltage patterns for each telescope"  << LogIO::POST;
  } else {
    os << "Using user defined voltage patterns in Table "<<  vpTableStr_p 
       << LogIO::POST;
  }
  if (doSquint) {
    os << "Beam Squint will be included in the VP model" <<  LogIO::POST;
    os << "and the parallactic angle increment is " 
       << parAngleInc_p.getValue("deg") << " degrees"  << LogIO::POST;
  }
  pbLimit_p = pbLimit;
  return True;
};













//========================================================================
//       Create corruption terms - top level specialized routines


// NEW NOISE WITH ANoise

Bool Simulator::setnoise(const String& mode, 
			 const String& caltable, // if blank, not stored
			 const Quantity& simplenoise,
			 // ATM calculation
			 const Quantity& pground,
			 const Float relhum,
			 const Quantity& altitude,
			 const Quantity& waterheight,
			 const Quantity& pwv,
			 // OR user-specified tau and tatmos 
			 const Float tatmos=250.0, 
			 const Float tau=0.1,
			 // antenna parameters used for either option
			 const Float antefficiency=0.80,
			 const Float spillefficiency=0.85,
			 const Float correfficiency=0.85,
			 const Float trx=150.0, 
			 const Float tground=270.0, 
			 const Float tcmb=2.7
			 ) {
  
  LogIO os(LogOrigin("Simulator", "setnoise2()", WHERE));
#ifndef RI_DEBUG
  try {
#else
    cout<<pground<<" "<<relhum<<" "<<altitude<<" "<<waterheight<<" "<<pwv<<endl;
#endif

    noisemode_p = mode;

    RecordDesc simparDesc;
    simparDesc.addField ("type", TpString);
    simparDesc.addField ("mode", TpString);
    simparDesc.addField ("caltable", TpString);

    simparDesc.addField ("amplitude", TpFloat);  // for constant scale
    // simparDesc.addField ("scale", TpFloat);  // for fractional fluctuations
    simparDesc.addField ("combine"        ,TpString);

    // have to be defined here or else have to be added later
    simparDesc.addField ("startTime", TpDouble);
    simparDesc.addField ("stopTime", TpDouble);

    simparDesc.addField ("antefficiency"  ,TpFloat);
    simparDesc.addField ("spillefficiency",TpFloat);
    simparDesc.addField ("correfficiency" ,TpFloat);
    simparDesc.addField ("trx"		  ,TpFloat);
    simparDesc.addField ("tground"	  ,TpFloat);
    simparDesc.addField ("tcmb"           ,TpFloat);

    // user-override of ATM calculated tau
    simparDesc.addField ("tatmos"	  ,TpFloat);
    simparDesc.addField ("tau0"		  ,TpFloat);

    simparDesc.addField ("mean_pwv"	  ,TpDouble);
    simparDesc.addField ("pground"	  ,TpDouble);
    simparDesc.addField ("relhum"	  ,TpFloat);
    simparDesc.addField ("altitude"	  ,TpDouble);
    simparDesc.addField ("waterheight"	  ,TpDouble);

    // RI todo setnoise2 if tau0 is not defined, use freqdep

    String caltbl=caltable;
    caltbl.trim();
    string::size_type strlen;
    strlen=caltbl.length();
    if (strlen>3) 
      if (caltbl.substr(strlen-3,3)=="cal") {
	caltbl.resize(strlen-3);
	strlen-=3;
      }
    if (strlen>1)
      if (caltbl.substr(strlen-1,1)==".") {
	caltbl.resize(strlen-1);
	strlen-=1;
      }
    if (strlen>1)
      if (caltbl.substr(strlen-1,1)=="_") {
	caltbl.resize(strlen-1);
	strlen-=1;
      }
    
    Record simpar(simparDesc,RecordInterface::Variable);
    simpar.define ("type", "A Noise");
    if (strlen>1) 
      simpar.define ("caltable", caltbl+".A.cal");      
    simpar.define ("mode", mode);
    simpar.define ("combine", ""); // SPW,FIELD, etc

    if (mode=="simple") {
      os << "Using simple noise model with noise level of " << simplenoise.getValue("Jy")
	 << " Jy" << LogIO::POST;
      simpar.define ("amplitude", Float(simplenoise.getValue("Jy")) );
      simpar.define ("mode", "simple");

    } else if (mode=="tsys-atm" or mode=="tsys-manual") {
      os << "adding noise with unity amplitude" << LogIO::POST;
      // do be scaled in a minute by a Tsys-derived M below
      simpar.define ("amplitude", Float(1.0) );
      simpar.define ("mode", mode);

    } else {
      throw(AipsError("unsupported mode "+mode+" in setnoise()"));
    }

    // create the ANoise
    if (!create_corrupt(simpar)) 
      throw(AipsError("could not create ANoise in Simulator::setnoise"));
        
    if (mode=="tsys-atm" or mode=="tsys-manual") {

      simpar.define ("antefficiency"  ,antefficiency  ); 
      simpar.define ("correfficiency" ,correfficiency );
      simpar.define ("spillefficiency",spillefficiency);
      simpar.define ("trx"	      ,trx	      );
      simpar.define ("tground"	      ,tground	      );
      simpar.define ("tcmb"           ,tcmb           );

      if (pwv.getValue("mm")>0.)
	simpar.define ("mean_pwv", pwv.getValue("mm"));
      else {
	simpar.define ("mean_pwv", Double(1.));
	// we want to set it, but it doesn't get used unless ATM is being used
	if (mode=="tsys-atm") os<<"User has not set PWV, using 1mm"<<LogIO::POST;
      }
      
      if (mode=="tsys-manual") {
	// user can override the ATM calculated optical depth
	// with tau0 to be used over the entire SPW,
	simpar.define ("tau0"	      ,tau	      );      
	if (tatmos>10)
	  simpar.define ("tatmos"	      ,tatmos	      );
	else
	  simpar.define ("tatmos"	      ,250.	      );
	// AtmosCorruptor cal deal with 
	// an MF in tsys-manual mode - it will use ATM to calculate 
	// the relative opacity across the band, but it won't properly
	// integrate the atmosphere to get T_ebb.  
	// so for now we'll just make tsys-manual mean freqDepPar=False
	simpar.define ("type", "M");
      } else {
	// otherwise ATM will be used to calculate tau from pwv
	// catch uninitialized variant @#$^@! XML interface
	if (pground.getValue("mbar")>100.)
	  simpar.define ("pground", pground.getValue("mbar"));
	else {
	  simpar.define ("pground", Double(560.));
	  os<<"User has not set ground pressure, using 560mb"<<LogIO::POST;
	}
	if (relhum>0)
	  simpar.define ("relhum", relhum);
	else {
	  simpar.define ("relhum", 20.);
	  os<<"User has not set ground relative humidity, using 20%"<<LogIO::POST;
	}
	if (altitude.getValue("m")>0.)
	  simpar.define ("altitude", altitude.getValue("m"));
	else {
	  simpar.define ("altitude", Double(5000.));
	  os<<"User has not set site altitude, using 5000m"<<LogIO::POST;
	}
	if (waterheight.getValue("m")>100.)
	  simpar.define ("waterheight", waterheight.getValue("km"));
	else {
	  simpar.define ("waterheight", Double(2.));
	  os<<"User has not set water scale height, using 2km"<<LogIO::POST;
	}
	// as a function of frequency  (freqDepPar=True)
	simpar.define ("type", "MF");
      }

      if (strlen>1) 
	simpar.define ("caltable", caltbl+".M.cal");
      
      // create the M
      if (!create_corrupt(simpar)) 
	throw(AipsError("could not create M in Simulator::setnoise"));        
    } 

#ifndef RI_DEBUG
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
#endif
  return True;
}




Bool Simulator::setgain(const String& mode, 
			const String& caltable,
			const Quantity& interval, 
			const Vector<Double>& amplitude)
{ 
  LogIO os(LogOrigin("Simulator", "setgain()", WHERE));

#ifndef RI_DEBUG
  try {
#endif
        
    if(mode=="table") {      
      os << LogIO::SEVERE << "Cannot yet read from table" << LogIO::POST;
      return False;
    }
    else {
      // RI TODO Sim::setgain add mode=simple and =normal
      if (mode=="fbm" or mode=="random") {
	
	// set record format for calibration table simulation information
	RecordDesc simparDesc;
	simparDesc.addField ("type", TpString);
	simparDesc.addField ("caltable", TpString);
	simparDesc.addField ("mode", TpString);
	simparDesc.addField ("interval", TpDouble);
	simparDesc.addField ("camp", TpComplex);
	simparDesc.addField ("amplitude", TpDouble);
	simparDesc.addField ("combine", TpString);
	simparDesc.addField ("startTime", TpDouble);
	simparDesc.addField ("stopTime", TpDouble);
	
	// Create record with the requisite field values
	Record simpar(simparDesc,RecordInterface::Variable);
	simpar.define ("type", "G JONES");
	simpar.define ("interval", interval.getValue("s"));
	simpar.define ("mode", mode);
	Complex camp(0.1,0.1);
	if (amplitude.size()==1)
	  camp=Complex(amplitude[0],amplitude[0]);
	else 
	  camp=Complex(amplitude[0],amplitude[1]);
	simpar.define ("camp", camp);
	os << LogIO::NORMAL << "Gain corruption with complex RMS amplitude = " << camp << LogIO::POST;
	simpar.define ("amplitude", Double(abs(camp)));
	//simpar.define ("amplitude", amplitude);
	simpar.define ("caltable", caltable);
	simpar.define ("combine", "");
	
	// create the G
	if (!create_corrupt(simpar)) 
	  throw(AipsError("could not create G in Simulator::setgain"));        
	
      } else {
	throw(AipsError("unsupported mode "+mode+" in setgain()"));
      }
    }
#ifndef RI_DEBUG
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
#endif
  return True;
}







Bool Simulator::settrop(const String& mode, 
			const String& caltable,   // output
			const Float pwv,
			const Float deltapwv,
			const Float beta,
			const Float windspeed) {
  
  LogIO os(LogOrigin("Simulator", "settrop()", WHERE));

#ifndef RI_DEBUG
  try {
#endif
        
    if (mode=="test"||mode=="individual"||mode=="screen") {
      
      // set record format for calibration table simulation information
      RecordDesc simparDesc;
      simparDesc.addField ("type", TpString);
      simparDesc.addField ("caltable", TpString);
      simparDesc.addField ("mean_pwv", TpFloat);
      simparDesc.addField ("mode", TpString);
      simparDesc.addField ("delta_pwv", TpFloat);
      simparDesc.addField ("beta", TpFloat);
      simparDesc.addField ("windspeed", TpFloat);
      simparDesc.addField ("combine", TpString);
      simparDesc.addField ("startTime", TpDouble);
      simparDesc.addField ("stopTime", TpDouble);

      simparDesc.addField ("antefficiency"  ,TpFloat);
      simparDesc.addField ("spillefficiency",TpFloat);
      simparDesc.addField ("correfficiency" ,TpFloat);
      simparDesc.addField ("trx"		  ,TpFloat);
      simparDesc.addField ("tcmb"           ,TpFloat);
      simparDesc.addField ("tatmos"           ,TpFloat);

      simparDesc.addField ("tground"	  ,TpFloat);
      simparDesc.addField ("pground"	  ,TpDouble);
      simparDesc.addField ("relhum"	  ,TpFloat);
      simparDesc.addField ("altitude"	  ,TpDouble);
      simparDesc.addField ("waterheight"	  ,TpDouble);
    
      // create record with the requisite field values
      Record simpar(simparDesc,RecordInterface::Variable);
      simpar.define ("type", "TF");
      simpar.define ("caltable", caltable);
      simpar.define ("mean_pwv", pwv);
      simpar.define ("mode", mode);
      simpar.define ("delta_pwv", deltapwv);
      simpar.define ("beta", beta);
      simpar.define ("windspeed", deltapwv);
      simpar.define ("combine", "");

//      if (tground>100.)
//	simpar.define ("tground", tground);
//      else {
	simpar.define ("tground", Float(270.));
//	os<<"User has not set ground temperature, using 270K"<<LogIO::POST;
//      }
//      if (pground.getValue("mbar")>100.)
//	simpar.define ("pground", pground.getValue("mbar"));
//      else {
	simpar.define ("pground", Double(560.));
//	os<<"User has not set ground pressure, using 560mb"<<LogIO::POST;
//      }
//      if (relhum>0)
//	simpar.define ("relhum", relhum);
//      else {
	simpar.define ("relhum", Float(20.));
//	os<<"User has not set ground relative humidity, using 20%"<<LogIO::POST;
//      }
//      if (altitude.getValue("m")>0.)
//	simpar.define ("altitude", altitude.getValue("m"));
//      else {
	simpar.define ("altitude", Double(5000.));
//	os<<"User has not set site altitude, using 5000m"<<LogIO::POST;
//      }
//      if (waterheight.getValue("m")>100.)
//	simpar.define ("waterheight", waterheight.getValue("km"));
//      else {
	simpar.define ("waterheight", Double(2.));
//	os<<"User has not set water scale height, using 2km"<<LogIO::POST;
//      }

      // create the T
      if (!create_corrupt(simpar)) 
	throw(AipsError("could not create T in Simulator::settrop"));        

    } else {
      throw(AipsError("unsupported mode "+mode+" in settrop()"));
    }

#ifndef RI_DEBUG
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
#endif
  return True;
}






Bool Simulator::setleakage(const String& mode, const String& table,
			   //const Quantity& interval, 
			   const Vector<Double>& amplitude,
			   const Vector<Double>& offset)
{
  
  LogIO os(LogOrigin("Simulator", "setleakage()", WHERE));
  
#ifndef RI_DEBUG
  try {
#endif

    // set record format for calibration table simulation information
    RecordDesc simparDesc;
    simparDesc.addField ("type", TpString);
    simparDesc.addField ("caltable", TpString);
    // leave this one for generic SVC::createCorruptor
    simparDesc.addField ("amplitude", TpDouble);
    simparDesc.addField ("camp", TpComplex);
    simparDesc.addField ("offset", TpComplex);
    simparDesc.addField ("combine", TpString);
    //simparDesc.addField ("interval", TpDouble);
    simparDesc.addField ("simint", TpString);
    simparDesc.addField ("startTime", TpDouble);
    simparDesc.addField ("stopTime", TpDouble);
            
    // create record with the requisite field values
    Record simpar(simparDesc,RecordInterface::Variable);
    simpar.define ("type", "D");
    simpar.define ("caltable", table);
    Complex camp(0.1,0.1);
    if (amplitude.size()==1)
      camp=Complex(amplitude[0],amplitude[0]);
    else 
      camp=Complex(amplitude[0],amplitude[1]);
    simpar.define ("camp", camp);
    simpar.define ("amplitude", Double(abs(camp)));
    Complex off(0.,0.);
    if (offset.size()==1)
      off=Complex(offset[0],offset[0]);
    else 
      off=Complex(offset[0],offset[1]);
    simpar.define ("offset", off);

    //simpar.define ("interval", interval.getValue("s"));
    // provide infinite interval
    simpar.define ("simint", "infinite");

    simpar.define ("combine", "");

    
    // create the D
    if (!create_corrupt(simpar)) 
      throw(AipsError("could not create D in Simulator::setleakage"));        

#ifndef RI_DEBUG
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
#endif
  return True;
}



















//========================================================================
//        OLD as yet unconverted corruption generation routines

// OLD NOISE WITH ACoh

Bool Simulator::oldsetnoise(const String& mode, 
			    const String& table,
			    const Quantity& simplenoise,
			    const Float antefficiency=0.80,
			    const Float correfficiency=0.85,
			    const Float spillefficiency=0.85,
			    const Float tau=0.0,
			    const Float trx=50.0, 
			    const Float tatmos=250.0, 
			    const Float tcmb=2.7) {
  
  LogIO os(LogOrigin("Simulator", "oldsetnoise()", WHERE));
  try {
    
    noisemode_p = mode;

    os << LogIO::WARN << "Using deprecated ACoh Noise - this will dissapear in the future - please switch to sm.setnoise unless you are simulating single dish data" << LogIO::POST;

    if(mode=="table") {
      os << LogIO::SEVERE << "Cannot yet read from table" << LogIO::POST;
      return False;
    }
    else if (mode=="simplenoise") {
      os << "Using simple noise model with noise level of " << simplenoise.getValue("Jy")
	 << " Jy" << LogIO::POST;
	if(ac_p) delete ac_p; ac_p = 0;
	ac_p = new SimACoh(seed_p, simplenoise.getValue("Jy") );
    }
    else {
      os << "Using the Brown calculated noise model" << LogIO::POST;
      os << "  eta_ant=" << antefficiency << " eta_corr=" << correfficiency << " eta_spill=" << spillefficiency << LogIO::POST;
      os << "  tau=" << tau << " trx=" << trx << " tatmos=" << tatmos << " tcmb=" << tcmb << LogIO::POST;
	if(ac_p) delete ac_p; ac_p = 0;
	ac_p = new SimACohCalc(seed_p, antefficiency, correfficiency,
			       spillefficiency, tau, Quantity(trx, "K"), 
			       Quantity(tatmos, "K"), Quantity(tcmb, "K"));
    }

    return True;
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
  
}





Bool Simulator::setpa(const String& mode, const String& table,
		      const Quantity& interval) {
  
  LogIO os(LogOrigin("Simulator", "setpa()", WHERE));
  
  try {
    
    throw(AipsError("Corruption by simulated errors temporarily disabled (06Nov20 gmoellen)"));
 /*
    if(mode=="table") {
      os << LogIO::SEVERE << "Cannot yet read from table" << LogIO::POST;
      return False;
    }
    else {
      makeVisSet();
      if(pj_p) delete pj_p; pj_p = 0;
      pj_p = new PJones (*vs_p, interval.get("s").getValue());
      os <<"Using parallactic angle correction"<< LogIO::POST;
    }
 */
    return True;
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
};




Bool Simulator::setbandpass(const String& mode, const String& table,
			    const Quantity& interval,
			    const Vector<Double>& amplitude) {
  
  LogIO os(LogOrigin("Simulator", "setbandpass()", WHERE));
  
  try {

    throw(AipsError("Corruption by simulated errors temporarily disabled (06Nov20 gmoellen)"));

    /*    
    if(mode=="table") {
      os << LogIO::SEVERE << "Cannot yet read from table" << LogIO::POST;
      return False;
    }
    else {
      os << LogIO::SEVERE << "Cannot yet calculate bandpass" << LogIO::POST;
      return False;
    }
    return True;
    */
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;

    return False;
  } 
  return True;
}




Bool Simulator::setpointingerror(const String& epJTableName,
				    const Bool applyPointingOffsets,
				    const Bool doPBCorrection)
{
  LogIO os(LogOrigin("Simulator", "close()", WHERE));
  epJTableName_p = epJTableName;
  //  makeVisSet();
  try {
    throw(AipsError("Corruption by simulated errors temporarily disabled (06Nov20 gmoellen)"));
    /*    
    if (epJ_p) delete epJ_p;epJ_p=0;
    {
      epJ_p = new EPJones(*vs_p);
      epJ_p->load(epJTableName_p,"","diagonal");
    }
    */
  }
  catch (AipsError x)
    {
      os << LogIO::SEVERE << "Caught exception: "
	 << x.getMesg() << LogIO::POST;
      return False;
    }

  applyPointingOffsets_p = applyPointingOffsets;
  doPBCorrection_p = doPBCorrection;
  return True;
}











//========================================================================
//       CORRUPTION - GENERIC VISCAL FUNCTIONS



Bool Simulator::create_corrupt(Record& simpar)
{
  LogIO os(LogOrigin("Simulator", "create_corrupt()", WHERE));
  SolvableVisCal *svc(NULL);
  
  // RI todo sim::create_corrupt assert that ms has certain structure
  
#ifndef RI_DEBUG
    try {
#endif
    makeVisSet();
    
    String upType=simpar.asString("type");
    upType.upcase();
    os << LogIO::NORMAL << "Creating "<< upType <<" Calibration structure for data corruption." << LogIO::POST;
    
    svc = createSolvableVisCal(upType,*vs_p);

#ifdef RI_DEBUG
    svc->setPrtlev(4);
#else 
    svc->setPrtlev(0);
#endif

    Vector<Double> solTimes;
    svc->setSimulate(*vs_p,simpar,solTimes);
    
    // add to the pointer block of VCs:
    uInt napp=vc_p.nelements();
    vc_p.resize(napp+1,False,True);
    vc_p[napp] = (VisCal*) svc;
    // svc=NULL;
    ve_p.setapply(vc_p);
            
#ifndef RI_DEBUG
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    if (svc) delete svc;
    throw(AipsError("Error in Simulator::create_corrupt"));
    return False;
  }
#endif
  return True;
}







//========================================================================
//       corrupt and setapply, for actually changing visibilities 


/// this can be used to load any table, just that it has to have the right form

Bool Simulator::setapply(const String& type,
			 const Double& t,
			 const String& table,
			 const String& spw,
			 const String& field,
			 const String& interp,
			 const Bool& calwt,
			 const Vector<Int>& spwmap,
			 const Float& opacity)
{
  LogIO os(LogOrigin("Simulator", "setapply()", WHERE));

  // First try to create the requested VisCal object
  VisCal *vc(NULL);
    
  try {

    makeVisSet();

    // Set record format for calibration table application information
    RecordDesc applyparDesc;
    applyparDesc.addField ("t", TpDouble);
    applyparDesc.addField ("table", TpString);
    applyparDesc.addField ("interp", TpString);
    applyparDesc.addField ("spw", TpArrayInt);
    applyparDesc.addField ("field", TpArrayInt);
    applyparDesc.addField ("calwt",TpBool);
    applyparDesc.addField ("spwmap",TpArrayInt);
    applyparDesc.addField ("opacity",TpFloat);
    
    // Create record with the requisite field values
    Record applypar(applyparDesc);
    applypar.define ("t", t);
    applypar.define ("table", table);
    applypar.define ("interp", interp);
    applypar.define ("spw",Vector<Int>());
    applypar.define ("field",Vector<Int>());
    //    applypar.define ("spw",getSpwIdx(spw));
    //    applypar.define ("field",getFieldIdx(field));
    applypar.define ("calwt",calwt);
    applypar.define ("spwmap",spwmap);
    applypar.define ("opacity", opacity);
    
    String upType=type;
    if (upType=="")
      // Get type from table
      upType = calTableType(table);

    // Must be upper case
    upType.upcase();
    
    os << LogIO::NORMAL
       << "Arranging to CORRUPT with:"
       << LogIO::POST;
    
    // Add a new VisCal to the apply list
    vc = createVisCal(upType,*vs_p);
    
    vc->setApply(applypar);

    os << LogIO::NORMAL << ".   "
       << vc->applyinfo()
       << LogIO::POST;
    
  } catch (AipsError x) {
    os << LogIO::SEVERE << x.getMesg()
       << " Check inputs and try again."
       << LogIO::POST;
    if (vc) delete vc;
    throw(AipsError("Error in Simulator::setapply."));
    return False;
  }

  // Creation apparently successful, so add to the apply list
  // TBD: consolidate with above?
  try {

    uInt napp=vc_p.nelements();
    vc_p.resize(napp+1,False,True);
    vc_p[napp] = vc;
    vc=NULL;

    // Maintain sort of apply list
    ve_p.setapply(vc_p);

    return True;

  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg()
       << LogIO::POST;
    if (vc) delete vc;
    throw(AipsError("Error in Simulator::setapply."));
    return False;
  }
  return False;
}






Bool Simulator::corrupt() {

  // VIS-plane (only) corruption
  
  LogIO os(LogOrigin("Simulator", "corrupt()", WHERE));

  try {
    
    ms_p->lock();
    if(mssel_p) mssel_p->lock();

//    makeVisSet();
//    AlwaysAssert(vs_p, AipsError);

    // Arrange the sort for efficient cal apply
    Block<Int> columns;
    // include scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;

    Matrix<Int> noselection;
    if(mssel_p) {
      vs_p = new VisSet(*mssel_p,columns,noselection);
    }
    else {
      vs_p = new VisSet(*ms_p,columns,noselection);
    }
    AlwaysAssert(vs_p, AipsError);

    // initCalSet() happens in VS creation unless there is a stored channel selection 
    // in the ms, and it equals the channel selection passed from here in mssel_p
    // vs_p->initCalSet();
    
    VisIter& vi=vs_p->iter();
    VisBuffer vb(vi);

    // Ensure VisEquation is ready - this sorts the VCs
    // if we want a different order for corruption we will either need to 
    // implement the sort here or create a VE::setcorrupt(vc_p)
    ve_p.setapply(vc_p);

    // set to corrupt Model down to B (T,D,G,etc) and correct Observed with AN,M,K
    ve_p.setPivot(VisCal::B); 
    
    // Apply 
    if (vc_p.nelements()>0) {
      os << LogIO::NORMAL << "Doing visibility corruption." 
	 << LogIO::POST;
      for (Int i=0;i<vc_p.nelements();i++) {
	//	os << vc_p[i]->longTypeName() << endl << vc_p[i]->siminfo() << endl <<
	//	  "spwok = " << vc_p[i]->spwOK() << LogIO::POST;
	os << vc_p[i]->siminfo() << "spwok = " << vc_p[i]->spwOK();
	if (vc_p[i]->type() >= ve_p.pivot()) 
	  os << " in corrupt mode." << endl << LogIO::POST;
	else 
	  os << " in correct mode." << endl << LogIO::POST;
      }
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	// Only if 
	if (ve_p.spwOK(vi.spectralWindow())) {
	  for (vi.origin(); vi.more(); vi++) {

	    // corrupt model to pivot, correct data up to pivot
	    ve_p.collapseForSim(vb);	    

	    // Deposit corrupted visibilities into DATA
	    // vi.setVis(vb.modelVisCube(), VisibilityIterator::Observed);
	    vi.setVis(vb.visCube(), VisibilityIterator::Observed);
	    // for now, Also deposit in corrected 
	    // (until newmmssimulator doesn't make corrected anymore)
	    // actually we should have this check if corrected is there, 
	    // and if it is for some reason, copy data into it.
	    // RI TODO Sim::corrupt check for existence of Corrected
	    vi.setVis(vb.visCube(), VisibilityIterator::Corrected);

	    // RI TODO is this 100% right?
	    vi.setWeightMat(vb.weightMat());

	  }
	}
	else 
	  cout << "Encountered data spw " << vi.spectralWindow() << " for which there is no (simulated) calibration." << endl;
      }
    }


    // Old-fashioned noise, for now
    if(ac_p != NULL){
      //      os << LogIO::WARN << "Using deprecated ACoh Noise - this will dissapear in the future - please switch to sm.setnoise" << LogIO::POST;
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	for (vi.origin(); vi.more(); vi++) {
	  
	  // affects vb.visibility() i.e. Observed
	  ac_p->apply(vb);
	  vi.setVis(vb.visibility(), VisibilityIterator::Observed);
	  vi.setVis(vb.visibility(), VisibilityIterator::Corrected);
	}
      }
    }

    // Flush to disk
    vs_p->flush();

    ms_p->relinquishAutoLocks();
    ms_p->unlock();
    if(mssel_p) mssel_p->unlock();

  } catch (std::exception& x) {
    ms_p->relinquishAutoLocks();
    ms_p->unlock();
    if(mssel_p) mssel_p->unlock();
    throw(AipsError(x.what()));
    return False;
  } 
  return True;
}




















Bool Simulator::observe(const String&   sourcename,
			   const String&   spwname,
			   const Quantity& startTime, 
			   const Quantity& stopTime)
{
  LogIO os(LogOrigin("Simulator", "observe()", WHERE));
  

  try {
    
    if(!feedsHaveBeenSet && !feedsInitialized) {
      os << "Feeds have not been set - using default " << feedMode_p << LogIO::WARN;
      sim_p->initFeeds(feedMode_p);
      feedsInitialized = True;
    }
    if(!timesHaveBeenSet_p) {
      os << "Times have not been set - using defaults " << endl
	 << "     Times will be interpreted as hour angles for first source"
	 << LogIO::WARN;
    }

    sim_p->observe(sourcename, spwname, startTime, stopTime);

    if(ms_p) delete ms_p; ms_p=0;
    if(mssel_p) delete mssel_p; mssel_p=0;
    ms_p = new MeasurementSet(msname_p, 
			      TableLock(TableLock::AutoNoReadLocking), 
			      Table::Update);

    ms_p->flush();
    ms_p->unlock();

  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}




Bool Simulator::observemany(const Vector<String>&   sourcenames,
			   const String&   spwname,
			   const Vector<Quantity>& startTimes, 
			    const Vector<Quantity>& stopTimes,
			    const Vector<MDirection>& directions)
{
  LogIO os(LogOrigin("Simulator", "observemany()", WHERE));
  

  try {
    
    if(!feedsHaveBeenSet && !feedsInitialized) {
      os << "Feeds have not been set - using default " << feedMode_p << LogIO::WARN;
      sim_p->initFeeds(feedMode_p);
      feedsInitialized = True;
    }
    if(!timesHaveBeenSet_p) {
      os << "Times have not been set - using defaults " << endl
	 << "     Times will be interpreted as hour angles for first source"
	 << LogIO::WARN;
    }

    sim_p->observe(sourcenames, spwname, startTimes, stopTimes, directions);

    if(ms_p) delete ms_p; ms_p=0;
    if(mssel_p) delete mssel_p; mssel_p=0;
    ms_p = new MeasurementSet(msname_p, 
			      TableLock(TableLock::AutoNoReadLocking), 
			      Table::Update);

    ms_p->flush();
    ms_p->unlock();

  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}






Bool Simulator::predict(const Vector<String>& modelImage, 
			   const String& compList,
			   const Bool incremental) {
  
  LogIO os(LogOrigin("Simulator", "predict()", WHERE));
  
  // Note that incremental here does not apply to se_p->predict(False),
  // Rather it means: add the calculated model visibility to the data visibility.
  // We return a MS with Data, Model, and Corrected columns identical

  try {

    os << "Predicting visibilities using model: " << modelImage << 
      " and componentList: " << compList << LogIO::POST;
    if (incremental) {
      os << "The data column will be incremented" <<  LogIO::POST;
    } else {
      os << "The data column will be replaced" <<  LogIO::POST;
    }
    if(!ms_p) {
      os << "MeasurementSet pointer is null : logic problem!"
	 << LogIO::EXCEPTION;
    }
    ms_p->lock();   
    if(mssel_p) mssel_p->lock();   
    if (!createSkyEquation( modelImage, compList)) {
      os << LogIO::SEVERE << "Failed to create SkyEquation" << LogIO::POST;
      return False;
    }
    if (incremental) {
      se_p->predict(False,MS::MODEL_DATA);  
    } else {
      se_p->predict(False,MS::DATA);   //20091030 RI changed SE::predict to use DATA
    }
    destroySkyEquation();

    // Copy the predicted visibilities over to the observed and 
    // the corrected data columns
    makeVisSet();

    VisIter& vi = vs_p->iter();
    VisBuffer vb(vi);
    vi.origin();
    vi.originChunks();

    //os << "Copying predicted visibilities from MODEL_DATA to DATA and CORRECTED_DATA" << LogIO::POST;
      
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()){
      for (vi.origin(); vi.more(); vi++) {
	//	vb.setVisCube(vb.modelVisCube());

	if (incremental) {
	  vi.setVis( (vb.modelVisCube() + vb.visCube()),
		     VisibilityIterator::Corrected);
	  vi.setVis(vb.correctedVisCube(),VisibilityIterator::Observed);

	  // model=1 is more consistent with VS::initCalSet 
	  // vi.setVis(vb.correctedVisCube(),VisibilityIterator::Model);
	} else {
	  // from above, the prediction is now already in Observed.
	  // RI TODO remove scratch columns from NewMSSimulator; 
	  // until then we;ll just leave them 1 and Corr=Obs (for imaging)
	  vi.setVis(vb.visCube(),VisibilityIterator::Corrected);
	}
	vb.setModelVisCube(Complex(1.0,0.0));
	vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
      }
    }
    ms_p->unlock();     
    if(mssel_p) mssel_p->lock();   

  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    ms_p->unlock();     
    if(mssel_p) mssel_p->lock();   
    return False;
  } 
  return True;
}

Bool Simulator::createSkyEquation(const Vector<String>& image,
				     const String complist)
{

  LogIO os(LogOrigin("Simulator", "createSkyEquation()", WHERE));

  try {
    if(sm_p==0) {
      sm_p = new CleanImageSkyModel();
    }
    AlwaysAssert(sm_p, AipsError);
    
    // Add the componentlist
    if(complist!="") {
      if(!Table::isReadable(complist)) {
	os << LogIO::SEVERE << "ComponentList " << complist
	   << " not readable" << LogIO::POST;
	return False;
      }
      componentList_p=new ComponentList(complist, True);
      if(componentList_p==0) {
	os << LogIO::SEVERE << "Cannot create ComponentList from " << complist
	   << LogIO::POST;
	return False;
      }
      if(!sm_p->add(*componentList_p)) {
	os << LogIO::SEVERE << "Cannot add ComponentList " << complist
	   << " to SkyModel" << LogIO::POST;
	return False;
      }
    } else {
      componentList_p=0;
    }
    
    nmodels_p = image.nelements();
    if (nmodels_p == 1 && image(0) == "") nmodels_p = 0;
    
    if (nmodels_p > 0) {
      images_p.resize(nmodels_p); 
      
      for (Int model=0;model<Int(nmodels_p);model++) {
	if(image(model)=="") {
	  os << LogIO::SEVERE << "Need a name for model "  << model+1
	     << LogIO::POST;
	  return False;
	} else {
	  if(!Table::isReadable(image(model))) {
	    os << LogIO::SEVERE << image(model) << " is unreadable" << LogIO::POST;
	  } else {
	    images_p[model]=0;
	    os << "About to open model " << model+1 << " named "
	       << image(model) << LogIO::POST;
	    images_p[model]=new PagedImage<Float>(image(model));

	    AlwaysAssert(images_p[model], AipsError);
	    // RI TODO is this a logic problem with more than one source??
	    // Add distance
	    if(abs(distance_p[nField-1].get().getValue())>0.0) {
	      os << "  Refocusing to distance " << distance_p[nField-1].get("km").getValue()
		 << " km" << LogIO::POST;
	    }
	    Record info(images_p[model]->miscInfo());
	    info.define("distance", distance_p[nField-1].get("m").getValue());
	    images_p[model]->setMiscInfo(info);
	    if(sm_p->add(*images_p[model])!=model) {
	      os << LogIO::SEVERE << "Error adding model " << model+1 << LogIO::POST;
	      return False;
	    }
	  }
	}
      }
    }
    
    
    if(vs_p) {
      delete vs_p; vs_p=0;
    }
    makeVisSet();
    
    cft_p = new SimpleComponentFTMachine();
    
    MeasurementSet *ams=0;
    
    if(mssel_p) {
      ams=mssel_p;
    }
    else {
      ams=ms_p;
    }
    if((ftmachine_p=="sd")||(ftmachine_p=="both")||(ftmachine_p=="mosaic")) {
      if(!gvp_p) {
	os << "Using default primary beams for gridding" << LogIO::POST;
	gvp_p=new VPSkyJones(*ams, True, parAngleInc_p, squintType_p);
      }
      if(ftmachine_p=="sd") {
	os << "Single dish gridding " << LogIO::POST;
	if(gridfunction_p=="pb") {
	  ft_p = new SDGrid(*gvp_p, cache_p/2, tile_p, gridfunction_p);
	}
	else {
	  ft_p = new SDGrid(cache_p/2, tile_p, gridfunction_p);
	}
      }
      else if(ftmachine_p=="mosaic") {
	os << "Performing Mosaic gridding" << LogIO::POST;
	// RI TODO need stokesString for current spw - e.g. currSpw()?
	ft_p = new MosaicFT(gvp_p, mLocation_p, stokesString_p[0], cache_p/2, tile_p, True);
      }
      else if(ftmachine_p=="both") {
	os << "Performing single dish gridding with convolution function "
	   << gridfunction_p << LogIO::POST;
	os << "and interferometric gridding with convolution function SF"
	   << LogIO::POST;
	
	ft_p = new GridBoth(*gvp_p, cache_p/2, tile_p,
			    mLocation_p, 
			    gridfunction_p, "SF", padding_p);
      }
      
      VisIter& vi(vs_p->iter());
      // Get bigger chunks o'data: this should be tuned some time
      // since it may be wrong for e.g. spectral line
      vi.setRowBlocking(100);
    }
    else {
      os << "Synthesis gridding " << LogIO::POST;
      // Now make the FTMachine
      //    if(wprojPlanes_p>1) {
      if (ftmachine_p=="wproject") {
	os << "Fourier transforms will use specified common tangent point:" << LogIO::POST;
	// RI TODO how does this work with more than one field?
	os << formatDirection(sourceDirection_p[nField-1]) << LogIO::POST;
	//      ft_p = new WProjectFT(*ams, facets_p, cache_p/2, tile_p, False);
	ft_p = new WProjectFT(wprojPlanes_p, mLocation_p, cache_p/2, tile_p, False);
      }
      else if (ftmachine_p=="pbwproject") {
	os << "Fourier transfroms will use specified common tangent point and PBs" 
	   << LogIO::POST;
	os << formatDirection(sourceDirection_p[nField-1]) << LogIO::POST;
	
	//	if (!epJ_p)
	  os << "Antenna pointing related term (EPJones) not set.  "
	     << "This is required when using pbwproject FTMachine." 
	     << "(gmoellen 06Nov20: pointing errors temporarily disabled)"
	     << LogIO::EXCEPTION;

   /*
	doVP_p = False; // Since this FTMachine includes PB
	if (wprojPlanes_p<=1)
	  {
	    os << LogIO::NORMAL
	       << "You are using wprojplanes=1. Doing co-planar imaging "
	       << "(no w-projection needed)" 
	       << LogIO::POST;
	    os << "Performing pb-projection"
	       << LogIO::POST;
	  }
	if((wprojPlanes_p>1)&&(wprojPlanes_p<64)) 
	  {
	    os << LogIO::WARN
	       << "No. of w-planes set too low for W projection - recommend at least 128"
	       << LogIO::POST;
	    os << "Performing pb + w-plane projection"
	       << LogIO::POST;
	  }
//	  epJ_p = new EPJones(*vs_p);
//	  epJ_p->load(epJTableName_p,"","diagonal");
	if(!gvp_p) 
	  {
	    os << "Using defaults for primary beams used in gridding" << LogIO::POST;
	    gvp_p=new VPSkyJones(*ms_p, True, parAngleInc_p, squintType_p);
	  }
//	  ft_p = new PBWProjectFT(*ms_p, epJ, gvp_p, facets_p, cache_p/2, 
//	  doPointing, tile_p, paStep_p, 
//	  pbLimit_p, True);

	String cfCacheDirName = "cache";
	if (mssel_p)
	  ft_p = new PBWProjectFT(*mssel_p, epJ_p, 
	  // gvp_p,
				  wprojPlanes_p, cache_p/2, 
				  cfCacheDirName,
				  applyPointingOffsets_p, doPBCorrection_p, 
				  tile_p, 
				  0.0, // Not required here. parAngleInc_p is used in gvp_p 
				  pbLimit_p, True);
	else
	  ft_p = new PBWProjectFT(*ms_p, epJ_p, 
				  // gvp_p, 
				  wprojPlanes_p, cache_p/2, 
				  cfCacheDirName,
				  applyPointingOffsets_p, doPBCorrection_p, 
				  tile_p, 
				  0.0, // Not required here. parAngleInc_p is used in gvp_p 
				  pbLimit_p, True);
	AlwaysAssert(ft_p, AipsError);
	cft_p = new SimpleComponentFTMachine();
	AlwaysAssert(cft_p, AipsError);

   */
      }
      else {
	os << "Fourier transforms will use image centers as tangent points" << LogIO::POST;
	ft_p = new GridFT(cache_p/2, tile_p, gridfunction_p, mLocation_p, padding_p);
      }
    }
    AlwaysAssert(ft_p, AipsError);
    
    se_p = new SkyEquation ( *sm_p, *vs_p, *ft_p, *cft_p );
    
    // Now add any SkyJones that are needed
    if(doVP_p) {
      if (doDefaultVP_p) {
	os << "Using default primary beams for mosaicing (use setvp to change)" << LogIO::POST;
	vp_p=new VPSkyJones(*ams, True, parAngleInc_p, squintType_p, skyPosThreshold_p);
      } else {
	Table vpTable( vpTableStr_p );
	vp_p=new VPSkyJones(*ams, vpTable, parAngleInc_p, squintType_p);
      }
      vp_p->summary();
      se_p->setSkyJones(*vp_p);
    }
    else {
      vp_p=0;
    }
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    ms_p->unlock();     
    if(mssel_p) mssel_p->lock();   
    return False;
  } 
  return True;
};

void Simulator::destroySkyEquation() 
{
  if(se_p) delete se_p; se_p=0;
  if(sm_p) delete sm_p; sm_p=0;
  if(vp_p) delete vp_p; vp_p=0;
  if(componentList_p) delete componentList_p; componentList_p=0;

  for (Int model=0;model<Int(nmodels_p);model++) {
    if(images_p[model]) delete images_p[model]; images_p[model]=0;
  }
};














Bool Simulator::setlimits(const Double shadowLimit,
			  const Quantity& elevationLimit)
{
  
  LogIO os(LogOrigin("Simulator", "setlimits()", WHERE));
  
  try {
    
    sim_p->setFractionBlockageLimit( shadowLimit );
    sim_p->setElevationLimit( elevationLimit );
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}
      
Bool Simulator::setauto(const Double autocorrwt) 
{
  
  LogIO os(LogOrigin("Simulator", "setauto()", WHERE));
  
  try {
    
    sim_p->setAutoCorrelationWt(autocorrwt);

  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}
      
void Simulator::makeVisSet() {

  if(vs_p) return;
  
  Block<int> sort(0);
  sort.resize(5);
  sort[0] = MS::FIELD_ID;
  sort[1] = MS::FEED1;
  sort[2] = MS::ARRAY_ID;
  sort[3] = MS::DATA_DESC_ID;
  sort[4] = MS::TIME;
  Matrix<Int> noselection;
  if(mssel_p) {
    vs_p = new VisSet(*mssel_p,sort,noselection);
  }
  else {
    if (ms_p) 
      vs_p = new VisSet(*ms_p,sort,noselection);
    else
      throw(AipsError("No measurement set open in Simulator."));
  }
  AlwaysAssert(vs_p, AipsError);
  
}



Bool Simulator::setoptions(const String& ftmachine, const Int cache, const Int tile,
			      const String& gridfunction, const MPosition& mLocation,
			      const Float padding, const Int facets, const Double maxData,
			      const Int wprojPlanes)
{
  LogIO os(LogOrigin("Simulator", "setoptions()", WHERE));
  
  os << "Setting processing options" << LogIO::POST;
  
  sim_p->setMaxData(maxData*1024.0*1024.0);

  ftmachine_p=downcase(ftmachine);
  if(cache>0) cache_p=cache;
  if(tile>0) tile_p=tile;
  gridfunction_p=downcase(gridfunction);
  mLocation_p=mLocation;
  if(padding>=1.0) {
    padding_p=padding;
  }
  facets_p=facets;
  wprojPlanes_p = wprojPlanes;
  // Destroy the FTMachine
  if(ft_p) {delete ft_p; ft_p=0;}
  if(cft_p) {delete cft_p; cft_p=0;}

  return True;
}


Bool Simulator::detached() const
{
  return False;
}

String Simulator::formatDirection(const MDirection& direction) {
  MVAngle mvRa=direction.getAngle().getValue()(0);
  MVAngle mvDec=direction.getAngle().getValue()(1);
  ostringstream oss;
  oss.setf(ios::left, ios::adjustfield);
  oss.width(14);
  oss << mvRa(0.0).string(MVAngle::TIME,8);
  oss.width(14);
  oss << mvDec.string(MVAngle::DIG2,8);
  oss << "     " << MDirection::showType(direction.getRefPtr()->getType());
  return String(oss);
}

String Simulator::formatTime(const Double time) {
  MVTime mvtime(Quantity(time, "s"));
  return mvtime.string(MVTime::DMY,7);
}



Bool Simulator::setdata(const Vector<Int>& spectralwindowids,
			   const Vector<Int>& fieldids,
			   const String& msSelect)
  
{

  
  LogIO os(LogOrigin("Simulator", "setdata()", WHERE));

  if(!ms_p) {
    os << LogIO::SEVERE << "Program logic error: MeasurementSet pointer ms_p not yet set"
       << LogIO::POST;
    return False;
  }

  try {
    
    os << "Selecting data" << LogIO::POST;
    
   // Map the selected spectral window ids to data description ids
    MSDataDescColumns dataDescCol(ms_p->dataDescription());
    Vector<Int> ddSpwIds=dataDescCol.spectralWindowId().getColumn();

    Vector<Int> datadescids(0);
    for (uInt row=0; row<ddSpwIds.nelements(); row++) {
      Bool found=False;
      for (uInt j=0; j<spectralwindowids.nelements(); j++) {
	if (ddSpwIds(row)==spectralwindowids(j)) found=True;
      };
      if (found) {
	datadescids.resize(datadescids.nelements()+1,True);
	datadescids(datadescids.nelements()-1)=row;
      };
    };

    if(vs_p) delete vs_p; vs_p=0;
    if(mssel_p) delete mssel_p; mssel_p=0;
      
    // If a selection has been made then close the current MS
    // and attach to a new selected MS. We do this on the original
    // MS. 
    if(fieldids.nelements()>0||datadescids.nelements()>0) {
      os << "Performing selection on MeasurementSet" << LogIO::POST;
      Table& original=*ms_p;
      
      // Now we make a condition to do the old FIELD_ID, SPECTRAL_WINDOW_ID
      // selection
      TableExprNode condition;
      String colf=MS::columnName(MS::FIELD_ID);
      String cols=MS::columnName(MS::DATA_DESC_ID);
      if(fieldids.nelements()>0&&datadescids.nelements()>0){
	condition=original.col(colf).in(fieldids)&&original.col(cols).in(datadescids);
        os << "Selecting on field and spectral window ids" << LogIO::POST;
      }
      else if(datadescids.nelements()>0) {
	condition=original.col(cols).in(datadescids);
        os << "Selecting on spectral window id" << LogIO::POST;
      }
      else if(fieldids.nelements()>0) {
	condition=original.col(colf).in(fieldids);
        os << "Selecting on field id" << LogIO::POST;
      }
      
      // Now remake the original ms
      mssel_p = new MeasurementSet(original(condition));

      //AlwaysAssert(mssel_p, AipsError);
      //mssel_p->rename(msname_p+"/SELECTED_TABLE", Table::Scratch);
      if(mssel_p->nrow()==0) {
	delete mssel_p; mssel_p=0;
	os << LogIO::WARN
	   << "Selection is empty: reverting to original MeasurementSet"
	   << LogIO::POST;
	mssel_p=new MeasurementSet(original);
      }
      else {
	mssel_p->flush();
      }

    }
    else {
      mssel_p=new MeasurementSet(*ms_p);
    }
    {
      Int len = msSelect.length();
      Int nspace = msSelect.freq (' ');
      Bool nullSelect=(msSelect.empty() || nspace==len);
      if (!nullSelect) {
	os << "Now applying selection string " << msSelect << LogIO::POST;
	MeasurementSet* mssel_p2;
	// Apply the TAQL selection string, to remake the original MS
	String parseString="select from $1 where " + msSelect;
	mssel_p2=new MeasurementSet(tableCommand(parseString,*mssel_p));
	AlwaysAssert(mssel_p2, AipsError);
	// Rename the selected MS as */SELECTED_TABLE2
	//mssel_p2->rename(msname_p+"/SELECTED_TABLE2", Table::Scratch); 
	if (mssel_p2->nrow()==0) {
	  os << LogIO::WARN
	     << "Selection string results in empty MS: "
	     << "reverting to original MeasurementSet"
	     << LogIO::POST;
	  delete mssel_p2;
	} else {
	  if (mssel_p) {
	    delete mssel_p; 
	    mssel_p=mssel_p2;
	    mssel_p->flush();
	  }
	}
      } else {
	os << "No selection string given" << LogIO::POST;
      }

      if(mssel_p->nrow()!=ms_p->nrow()) {
	os << "By selection " << ms_p->nrow() << " rows are reduced to "
	   << mssel_p->nrow() << LogIO::POST;
      }
      else {
	os << "Selection did not drop any rows" << LogIO::POST;
      }
    }
    
    // Now create the VisSet
    if(vs_p) delete vs_p; vs_p=0;
    makeVisSet();
    //Now assign the source directions to something selected or sensible
    {
      Int fieldsel=0;
      if(fieldids.nelements() >0) {
	fieldsel=fieldids(0);
	// RI TODO does sim:setdata need this?
	nField=fieldids.nelements();
	for (Int i=0;i<nField;i++) {
	  // RI TODO check whether index in field column is just i or need
	  // to search for fieldid  
	  (vs_p->iter()).msColumns().field().name().get(i,sourceName_p[i]);
	  sourceDirection_p[i]=(vs_p->iter()).msColumns().field().phaseDirMeas(i); 
	  (vs_p->iter()).msColumns().field().code().get(i,calCode_p[i]);
	}       
      }
    }
    return True;
  } catch (AipsError x) {
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg()
       << LogIO::POST;
    return False;
  } 
  return True;
}

