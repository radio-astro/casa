//# FluxCalc_SS_JPL_Butler.cc: Implementation of FluxCalc_SS_JPL_Butler.h
//# Copyright (C) 2010
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
//----------------------------------------------------------------------------

//#include <components/ComponentModels/FluxStandard.h>
#include <components/ComponentModels/FluxCalc_SS_JPL_Butler.h>
#include <components/ComponentModels/ComponentType.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/String.h>
#include <casa/Quanta.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/Directory.h>
#include <casa/OS/DirectoryIterator.h>
#include <casa/System/Aipsrc.h>
#include <casa/sstream.h>
#include <casa/iomanip.h>
#include <measures/Measures.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MFrequency.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Recommended constructor.
FluxCalc_SS_JPL_Butler::FluxCalc_SS_JPL_Butler(const String& objname,
                                               const MEpoch& time) :
  name_p(objname),
  hasName_p(true),
  time_p(time),
  hasTime_p(true),
  hasEphemInfo_p(false)
{
  hasObjNum_p = setObjNum();
}

FluxCalc_SS_JPL_Butler::FluxCalc_SS_JPL_Butler() :
  name_p(""),
  hasName_p(false),
  time_p(MEpoch()),
  hasTime_p(false),
  hasEphemInfo_p(false)
{
// Default constructor for making arrays, etc.
  hasObjNum_p = false;
  objnum_p    = FluxCalc_SS_JPL_Butler::N_KNOWN;
}

FluxCalc_SS_JPL_Butler::~FluxCalc_SS_JPL_Butler()
{
// Default destructor
}

Bool FluxCalc_SS_JPL_Butler::setObjNum()
{
  LogIO os(LogOrigin("FluxCalc_SS_JPL_Butler", "setObjNum"));

  if(!hasName_p){
    os << LogIO::SEVERE
       << "Please provide the source name."
       << LogIO::POST;
    return false;
  }
  
  String lname = name_p;
  lname.downcase();
  Bool matched = true;
  
  if(lname == "mercury")
    objnum_p = FluxCalc_SS_JPL_Butler::Mercury;
  else if(lname == "venus")
    objnum_p = FluxCalc_SS_JPL_Butler::Venus;
  else if(lname == "mars")
    objnum_p = FluxCalc_SS_JPL_Butler::Mars;
  else if(lname == "jupiter")
    objnum_p = FluxCalc_SS_JPL_Butler::Jupiter;
  else if(lname == "uranus")
    objnum_p = FluxCalc_SS_JPL_Butler::Uranus;
  else if(lname == "neptune")
    objnum_p = FluxCalc_SS_JPL_Butler::Neptune;
  else if(lname == "pluto")
    objnum_p = FluxCalc_SS_JPL_Butler::Pluto;
  else if(lname == "ganymede")
    objnum_p = FluxCalc_SS_JPL_Butler::Ganymede;
  else if(lname == "callisto")
    objnum_p = FluxCalc_SS_JPL_Butler::Callisto;
  else if(lname == "titan")
    objnum_p = FluxCalc_SS_JPL_Butler::Titan;
  else if(lname == "ceres")
    objnum_p = FluxCalc_SS_JPL_Butler::Ceres;
  else if(lname == "pallas")
    objnum_p = FluxCalc_SS_JPL_Butler::Pallas;
  else if(lname == "vesta")
    objnum_p = FluxCalc_SS_JPL_Butler::Vesta;
  else if(lname == "juno")
    objnum_p = FluxCalc_SS_JPL_Butler::Juno;
  else if(lname == "victoria")
    objnum_p = FluxCalc_SS_JPL_Butler::Victoria;
  else if(lname == "davida")
    objnum_p = FluxCalc_SS_JPL_Butler::Davida;
  else{
    os << LogIO::SEVERE
       << name_p << " is not a recognized Solar System flux calibration object."
       << LogIO::POST;
    matched = false;
  }
  
  // Changing the object invalidates the cached ephemeris info (if any).
  hasEphemInfo_p = false;
  
  return matched;
}

Bool FluxCalc_SS_JPL_Butler::getName(String& output) const
{
  if(!hasName_p)
    return false;
  output = name_p;
  return true;
}

Bool FluxCalc_SS_JPL_Butler::getTime(MEpoch& output) const
{
  if(!hasTime_p)
    return false;
  output = time_p;
  return true;
}

void FluxCalc_SS_JPL_Butler::setTime(const MEpoch& time)
{
  time_p = time;

  // Changing the time *possibly* invalidates the cached ephemeris info (if
  // any).  Leave it up to readEphem() to decide whether or not the ephemeris
  // info is valid (i.e. the new time is close enough to the old time).
  hasEphemInfo_p = false;  
}

ComponentType::Shape FluxCalc_SS_JPL_Butler::getShape(Double& angdiam)
{
  if(!hasEphemInfo_p && !readEphem())
    return ComponentType::UNKNOWN_SHAPE;
  
  angdiam = 2.0 * mean_rad_p / delta_p;
  return ComponentType::DISK;
}

uInt FluxCalc_SS_JPL_Butler::n_known() const
{
  return N_KNOWN;
}

Bool FluxCalc_SS_JPL_Butler::readEphem()
{
  LogIO os(LogOrigin("FluxCalc_SS_JPL_Butler", "readEphem"));

  if(!hasName_p || !hasTime_p){
    os << LogIO::SEVERE
       << "The source and time have not been set."
       << LogIO::POST;
    return false;
  }
  
  // Try to find a matching JPL-Horizons ephemeris table.
  // Note: these are not the same as the DE200 and DE405 JPL tables used
  // by measures to get the direction to a planet.
  // There may be more than one because of overlapping date ranges.
  const String tabpat(Regex::makeCaseInsensitive(name_p +
                                                 "_[-0-9.]+-[-0-9.]+[ydhms].+\\.tab"));
  String horpath;
  Bool foundStd = Aipsrc::findDir(horpath, "data/ephemerides/JPL-Horizons");
  if(!foundStd)
    horpath = ".";
  
  os << LogIO::NORMAL
     << "Looking for an ephemeris table matching " << tabpat
     << "\n\tin " << horpath
     << LogIO::POST;
  
  Directory hordir(horpath);
  DirectoryIterator dirIter(hordir, tabpat);
  Bool found = false;
  uInt firstTimeStart = name_p.length() + 1;  // The + 1 is for the _.
  Regex timeUnitPat("[ydhms]");
  Path path;

  while(!dirIter.pastEnd()){
    path = dirIter.name();
    String basename(path.baseName());

    // Look for, respectively, the positions of '--', 'd', and '.' in
    // '-12345--67890dUTC.tab'.  Note that, just to be tricky, the times in
    // this example are negative. 
    uInt firstTimeLen = basename.find('-', firstTimeStart + 1) - firstTimeStart;
    uInt lastTimeLen = basename.find(timeUnitPat,
                                     firstTimeStart + firstTimeLen + 1)
                       - firstTimeStart - firstTimeLen - 1;
    uInt unitPos  = firstTimeStart + firstTimeLen + 1 + lastTimeLen;
    
    Double firstTime = String::toDouble(basename.at(firstTimeStart, firstTimeLen));
    Double lastTime = String::toDouble(basename.at(firstTimeLen + firstTimeLen + 1,
                                                   lastTimeLen));
    Unit unit(basename[unitPos]);
    String ref(basename.at(unitPos + 1,
                           basename.find('.', unitPos + 1) - unitPos - 1));
    
    os << LogIO::DEBUG1
       << basename << ": (first, last)time = ("
       << firstTime << ", " << lastTime << ")" << unit.getName()
       << " " << ref
       << LogIO::POST;
    
    MEpoch::Types refEnum;
    Bool refIsValid = MEpoch::getType(refEnum, ref);
    
    if(refIsValid){
      MEpoch::Convert mtimeToDirFrame(time_p, MEpoch::Ref(refEnum));
      MEpoch mtimeInDirFrame(mtimeToDirFrame());
      Double dtime = mtimeInDirFrame.get(unit).getValue();

      if(dtime <= lastTime && dtime >= firstTime){
        found = true;
        break;
      }
    }
    // else maybe tabpat isn't specific enough.  Don't panic yet.

    ++dirIter;
  }

  if(!found){
    // Needing an MEpoch::Convert just to print an MEpoch is a pain.
    os << LogIO::SEVERE
       << "Could not find an ephemeris table for " << name_p
       << LogIO::POST;
    cout << " at " << MEpoch::Convert(time_p, MEpoch::Ref(MEpoch::UTC))() << endl;
  }
  else{
    os << LogIO::NORMAL
       << "Using ephemeris table " << path.baseName()
       << LogIO::POST;
  }
  
  hasEphemInfo_p = found;
  return found;
}


ComponentType::Shape FluxCalc_SS_JPL_Butler::compute(Vector<Flux<Double> >& values,
                                                     Vector<Flux<Double> >& errors,
                                                     Double& angdiam,
                                                     const Vector<MFrequency>& mfreqs)
{
  LogIO os(LogOrigin("FluxCalc_SS_JPL_Butler", "compute"));

  // Calls readEphem() if necessary.
  ComponentType::Shape rettype(getShape(angdiam));
  if(rettype == ComponentType::UNKNOWN_SHAPE)
    return rettype;
  
  if(!hasObjNum_p){
    hasObjNum_p = setObjNum();  // Also has its own errmsgs.
    if(!hasObjNum_p)
      return ComponentType::UNKNOWN_SHAPE;
  }

  switch(objnum_p){
  default: 
    {
      os << LogIO::NORMAL << "Using blackbody model." << LogIO::POST;
      compute_BB(values, errors, angdiam, mfreqs);
    };
  };

  return rettype;
}

void FluxCalc_SS_JPL_Butler::compute_BB(Vector<Flux<Double> >& values,
                                        Vector<Flux<Double> >& errors,
                                        const Double angdiam,
                                        const Vector<MFrequency>& mfreqs)
{
  uInt nfreqs = mfreqs.nelements();
  Quantum<Double> temperature(temperature_p, "K");
  Quantum<Double> freq_peak(QC::h / (QC::k * temperature));
  Quantum<Double> freq_ind_fac;  // Frequency independent factor.
  Quantum<Double> rocd2(0.5 * angdiam / QC::c);
  Unit freq_unit("Hz");

  rocd2 *= rocd2;
  freq_ind_fac = 2.0e26 * QC::h * C::pi * rocd2;

  for(uInt f = 0; f < nfreqs; ++f){
    Quantum<Double> freq(mfreqs[f].get(freq_unit));
    
    values[f].setValue((freq_ind_fac * freq * freq * freq).getValue() /
                       (exp((freq / freq_peak).getValue()) - 1.0));
    errors[f].setValue(0.0);
  }
}

Bool FluxCalc_SS_JPL_Butler::setObj(const String& objname)
{
  name_p = objname;
  return setObjNum();
}

} //# NAMESPACE CASA - END

