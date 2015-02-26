//# componentlist_cmpt.cc...interface class for componentlist tools 
//# Copyright (C) 2007
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
//# $Id$

/***
 * Framework independent implementation file for componentlist...
 *
 * Implement the componentlist component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 * Need to implement the error in position and flux for each component
 * 
 * 
 ***/

#include <iostream>
#include <componentlist_cmpt.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/SpectralModel.h>
#include <components/ComponentModels/TabularSpectrum.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/STLIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogIO.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/MUString.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/namespace.h>
#include <casa/OS/Path.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/SkyComponent.h>
#include <measures/Measures/MeasureHolder.h>

#include <stdcasa/cboost_foreach.h>

using namespace std;

namespace casac {

componentlist::componentlist() 
{
      itsLog = new casa::LogIO();
      itsList = new ComponentList();
      itsBin = new ComponentList();
}

componentlist::~componentlist()
{
   if(itsLog){
      delete itsLog;
      itsLog = 0;
   }
   if(itsList){
      delete itsList;
      itsList = 0;
   }
   if(itsBin){
      delete itsBin;
      itsBin = 0;
   }
}

bool componentlist::open(const std::string& filename, const bool nomodify,
                         const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "open"));

  bool rstat(false);
  try {
    if(!(Table::isReadable(filename)) || !(Table(filename).tableDesc().isColumn("Shape_Parameters"))){
      throw(AipsError(String(filename)+" is non existant or is not a componentlist table"));
    }
    if(itsList !=0) 
      delete itsList;
    if(itsBin !=0) 
      delete itsBin;
    if(filename.size() > 0){
      itsList = new ComponentList(Path(filename), nomodify);
      itsBin = new ComponentList();
    } else {
      itsList = new ComponentList();
      itsBin = new ComponentList();
    }
    rstat=true;
  } catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

int componentlist::asciitocomponentlist(const std::string& /*filename*/,
                                        const std::string& /*asciifile*/,
                                        const std::string& /*refer*/,
                                        const std::string& /*format*/,
                                        const ::casac::record& /*direction_*/,
                                        const ::casac::record& /*spectrum*/,
                                        const ::casac::record& /*flux*/, const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "asciitocomponentlist"));

  // TODO : IMPLEMENT ME HERE !
  int rstat(0);
  try{
    if(itsList && itsBin)
      *itsLog << LogIO::WARN
              << "asciitocomponentlist not implemented yet" << LogIO::POST;
    else
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::concatenate(const ::casac::variant& list,
                                const std::vector<int>& which, const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "concatenate"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      Vector<Int> what(which);
      ComponentList toBeAdded;
      if(list.type() == ::casac::variant::STRING ||
         list.type() == ::casac::variant::STRINGVEC){
        String filename(list.toString());
        if(!casa::Table::isReadable(filename)){
          throw(AipsError("Cannot read componentlist "+filename));
        }
        toBeAdded=ComponentList(Path(filename), true);
      }
      else if(list.type()==::casac::variant::RECORD){
        ::casac::variant localvar(list);
        Record * ptrRec = toRecord(localvar.asRecord());
        String error;
        if(!(toBeAdded.fromRecord(error, *ptrRec))){
          delete ptrRec;
          throw(AipsError(String("Error ") + error +
                          String(" in converting from record")));
        }
        delete ptrRec;
      }
      else{
        *itsLog << LogIO::SEVERE
                << "Can concatenate only componentlists which are on disk or are a record for now "
                << LogIO::POST;
        return false;
      }
      if(toBeAdded.nelements() < 1){
        *itsLog << LogIO::WARN << "Empty componentlist" << LogIO::POST;
        return false;
      }
      if(what.nelements()==0 || what[0] < 0){
        what.resize(toBeAdded.nelements());
        casa::indgen(what);
      }
      for (uInt k=0; k < what.nelements(); ++k){
        if(uInt(what[k]) < toBeAdded.nelements()){
          itsList->add(toBeAdded.component(k));
        }
        else{
          *itsLog << LogIO::SEVERE 
                  << "component " << k << "does not exist in this list "
                  << LogIO::POST;
        }
      } 
      return true;
    }
    else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool
componentlist::remove(const std::vector<int>& which, const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "remove"));

  bool rstat(false);
  try{
    std::vector<int> witch=which;
    
    if(itsList && itsBin){
      const Vector<Int> intVec = checkIndices(which, "remove",
                                              "No components removed");
      for(uInt c = 0; c < intVec.nelements(); c++)
        itsBin->add(itsList->component(intVec(c)));
      itsList->remove(intVec);
      rstat = true;
    }
    else{
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::purge()
{
  itsLog->origin(LogOrigin("componentlist", "purge"));

  bool rstat(false);
  try {
        if(itsList && itsBin){
           Vector<Int> indices(itsBin->nelements());
           indgen(indices);
           itsBin->remove(indices);
	} else {
	  *itsLog << LogIO::WARN << "componentlist is not opened, please open first" << LogIO::POST;
	}
  } catch (AipsError x){
	  *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x)
  }
  return rstat;
}

bool componentlist::recover(const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "recover"));

  bool rstat(false);
  try {
        if(itsList && itsBin){
       uInt i = itsBin->nelements();
       while (i > 0) {
              i--;
              itsList->add(itsBin->component(i));
       }
       purge();
	} else {
	  *itsLog << LogIO::WARN << "componentlist is not opened, please open first" << LogIO::POST;
	}

  } catch (AipsError x){
	  *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x)
  }
  return rstat;
}

int componentlist::length()
{
  itsLog->origin(LogOrigin("componentlist", "length"));

  int rstat(0);
  try{
    if(itsList && itsBin){
      rstat = itsList->nelements();	  
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::vector<int> componentlist::indices()
{
  itsLog->origin(LogOrigin("componentlist", "indices"));

  // TODO : IMPLEMENT ME HERE !
  std::vector<int> rstat(0);
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "indices not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch(AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::sort(const std::string& criteria, const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "sort"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      ComponentList::SortCriteria sortEnum = ComponentList::type(criteria);
      if (sortEnum == ComponentList::UNSORTED) {
        *itsLog << "Bad sort criteria." << endl
               << "Allowed values are: 'flux', 'position' & 'polarization'"
               << endl << "No sorting done."
               << LogIO::EXCEPTION;
      }
      itsList->sort(sortEnum);
      return true;
    } else {
      *itsLog << LogIO::WARN << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
	  *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x)
  }
  return rstat;
}

bool componentlist::isphysical(const std::vector<int>& which)
{
  itsLog->origin(LogOrigin("componentlist", "isphysical"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      const Vector<Int> intVec = checkIndices(which, "is_physical",
                                              "Not checking any components");
      rstat = itsList->isPhysical(intVec);
    }
    else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::vector<double> componentlist::sample(const ::casac::variant& /*direction_*/, 
                                          const ::casac::variant& /*pixellatsize*/, 
                                          const ::casac::variant& /*pixellongsize*/, 
                                          const ::casac::variant& /*frequency*/)
{
  itsLog->origin(LogOrigin("componentlist", "sample"));

  // TODO : IMPLEMENT ME HERE !
  std::vector<double> rstat(0);
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "sample not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::rename(const std::string& filename, const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "rename"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      itsList->rename(Path(filename), Table::NewNoReplace);
      rstat=True;
    }
    else{
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::simulate(const int howmany, const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "simulate"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      for (int k=0; k < howmany; ++k){
        itsList->add(SkyComponent());
      }	  
      rstat=True;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::addcomponent(const ::casac::variant& flux,
                                 const std::string& fluxunit,
                                 const std::string& polarization,
                                 const ::casac::variant& dir, 
                                 const std::string& shape,
                                 const ::casac::variant& majoraxis,
                                 const ::casac::variant& minoraxis,
                                 const ::casac::variant& positionangle,
                                 const ::casac::variant& freq, 
                                 const std::string& spectrumtype,
                                 const double index,
                                 const std::vector<double>& optionalparms,
                                 const std::string& label)
{
  itsLog->origin(LogOrigin("componentlist", "addcomponent"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      simulate(1);
      int which = itsList->nelements()-1;
      /*	  std::vector<complex> newflux(4, complex(0.0, 0.0));
                  if(flux.size() ==0){
            
                  if(upcase(polarization).compare(std::string("STOKES"))){
                  newflux.resize(1);
                  }
                  newflux[0]=complex(1.0, 0.0);
                  }
                  else if(flux.size() == 4){
                  newflux=flux; 
                  } 
                  else{
                  throw(AipsError("flux has to have 1 or 4 elements"));
                  }
      */
      setlabel(which, label, True);
      /*
        std::vector<complex> error(4);
        std::vector<std::complex<double> > stanerr(4, std::complex<double>(0.0, 0.0));
        for(unsigned int i=0; i<3; i++)
        error[i] = complex(real(stanerr[i]), imag(stanerr[i]));
      */

      ::casac::variant error;
      setflux(which, flux, fluxunit, polarization, error, True);
      casa::MDirection theDir;
      ::casac::variant *tmpdir=0;
      //Default case
      if(String(dir.toString())==String("[]")){
        tmpdir=new ::casac::variant(std::string("J2000 00h00m00.0 90d00m00"));
      }
      else{
        tmpdir=new ::casac::variant(dir);
      }

      if(!casaMDirection(*tmpdir, theDir)){
        *itsLog << LogIO::SEVERE 
                << "Could not interpret direction parameter" 
                << LogIO::POST;      
      }
      if(tmpdir != 0)
        delete tmpdir;
      MVDirection newDir=theDir.getValue();
      const Vector<Int> intVec =
        checkIndices(which, "addcomponent",
                     "Direction not changed on any components");
      itsList->setRefDirection(intVec, newDir);
      setrefdirframe(which, theDir.getRefString(), True);
      ::casac::variant majoraxiserror; 
      ::casac::variant minoraxiserror; 
      ::casac::variant positionangleerror;
      setshape(which, shape, majoraxis, minoraxis, positionangle,
               majoraxiserror,minoraxiserror, positionangleerror, optionalparms);
      setspectrum(which, spectrumtype, index);
      MFrequency theFreq;
      ::casac::variant *tmpfreq=0;
      if(String(freq.toString())== String("[]")){
        tmpfreq=new ::casac::variant(std::string("LSRK 1.420GHz"));
      }
      else{
        tmpfreq=new ::casac::variant(freq);
      }

      if(!casaMFrequency(*tmpfreq, theFreq)){
	    
        *itsLog << LogIO::SEVERE 
                << "Could not interpret frequency parameter" 
                << LogIO::POST;      
      }
      setfreq(which, theFreq.get("GHz").getValue(), "GHz", True);
      setfreqframe(which, theFreq.getRefString(), True);
      rstat=True;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
    //exception is thrown so the last component is bad ...lets remove it
    Vector<Int> remov(1,0);
    if (itsList->nelements() >0){
      remov[0]=(itsList->nelements())-1;
      itsList->remove(remov);
    }
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::close(const bool log)
{
  itsLog->origin(LogOrigin("componentlist", "close"));

  bool rstat(false);
  try {
	  if(itsList)
	     delete itsList;
	  if(itsBin)
	     delete itsBin;
	  itsList = 0;
          itsBin = 0;
	  itsList = new ComponentList();
	  itsBin = new ComponentList();
	  if(log)
	     *itsLog << LogIO::WARN << "componentlist closed" << LogIO::POST;
	  rstat=True;
  } catch (AipsError x){
	  *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x)
  }
  return rstat;
}

  bool componentlist::edit(const int /*which*/, const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "edit"));

  // TODO : IMPLEMENT ME HERE !
  bool rstat(false);
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "edit  not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::done()
{
  itsLog->origin(LogOrigin("componentlist", "done"));

  bool rstat(false);
  try{
    if(itsList)
      delete itsList;
    if(itsBin)
      delete itsBin;
    itsList = 0;
    itsBin = 0;
    //bring it back to the state of construction
    itsList = new ComponentList();
    itsBin = new ComponentList();
    rstat=True;
  }
  catch(AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::select(const std::vector<int>& which)
{
  itsLog->origin(LogOrigin("componentlist", "select"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      const Vector<Int> intVec = checkIndices(which, "select",
                                              "No components selected");
      itsList->select(intVec);
      rstat=true;
    }
    else{
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::deselect(const std::vector<int>& which)
{
  itsLog->origin(LogOrigin("componentlist", "deselect"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      const Vector<Int> intVec = checkIndices(which, "deselect",
                                              "No components deselected");
      itsList->deselect(intVec);
       rstat=true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::vector<int> componentlist::selected()
{
  itsLog->origin(LogOrigin("componentlist", "selected"));

  std::vector<int> rstat(0);
  try{
    if(itsList && itsBin){
      Vector<Int> theChosen = itsList->selected();
      theChosen.tovector(rstat);
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::string componentlist::getlabel(const int which)
{
  itsLog->origin(LogOrigin("componentlist", "getlabel"));

  std::string rstat("");
  try{
    if(itsList && itsBin){
      _checkIndex(which);
      rstat = itsList->component(which).label().c_str();
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::setlabel(const int which, const std::string& value,
                             const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "setlabel"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      itsList->setLabel(Vector<Int>(1, which) , value);
      rstat=true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::vector<double> componentlist::getfluxvalue(const int which)
{
  itsLog->origin(LogOrigin("componentlist", __FUNCTION__));
  std::vector<double> rstat(0);
  try{
    if(itsList && itsBin){
      _checkIndex(which);
      Vector<DComplex> fluxes = itsList->component(which).flux().value();

      Vector<Double> realfluxes(fluxes.nelements());
      for (uInt k=0; k < fluxes.nelements(); ++k){
        realfluxes[k]=real(fluxes[k]);
      }
      realfluxes.tovector(rstat);
    }
    else{
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::string componentlist::getfluxunit(const int which)
{
  itsLog->origin(LogOrigin("componentlist", "getfluxunit"));

  std::string rstat("");
  try{
    if(itsList && itsBin){
      _checkIndex(which);
      rstat=itsList->component(which).flux().unit().getName();
    }
    else{
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::string componentlist::getfluxpol(const int /*which*/)
{
  itsLog->origin(LogOrigin("componentlist", "getfluxpol"));

  // TODO : IMPLEMENT ME HERE !
  std::string rstat("");
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "getfluxpol not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::vector<double> componentlist::getfluxerror(const int which) {
	itsLog->origin(LogOrigin("componentlist", __FUNCTION__));
	try{
		ThrowIf(
			! itsList || ! itsBin,
			"componentlist is not opened, please open first"
		);
		_checkIndex(which);
		Vector<DComplex> fluxes = itsList->component(which).flux().errors();
		vector<Double> realfluxes;
		foreach_(DComplex flux, fluxes) {
	        realfluxes.push_back(real(flux));
		}
		return realfluxes;
	}
	catch (const AipsError& x){
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	    RETHROW(x)
	}
	return vector<double>();
}

bool componentlist::setflux(
	const int which, const ::casac::variant& varvalue,
	const std::string& unit,
	const std::string& polarization,
	const ::casac::variant& error, const bool /*log*/
) {
	itsLog->origin(LogOrigin("componentlist", "setflux"));

	bool rstat(false);
	try{
		ThrowIf(
			! itsList || ! itsBin,
			"componentlist is not opened, please open first"
		);
		Flux<Double> newFlux;
		const ComponentType::Polarisation pol = (ComponentType::Polarisation)(checkFluxPol(polarization));
		newFlux.setPol(pol);

		const Unit fluxUnit(unit);
		if (fluxUnit != Unit("Jy")) {
			*itsLog << "The flux units must have the same dimensions as the Jansky"
				<< endl << "Flux not changed on any components"
				<< LogIO::EXCEPTION;
		}
		newFlux.setUnit(fluxUnit);
		//Deal with the value...complex for nothing for now
		Vector<Complex> value;
		if( String(varvalue.toString()) == String("[]") ||
		    (varvalue.type()== ::casac::variant::INTVEC) ||
		    (varvalue.type()==::casac::variant::INT) ||
		    (varvalue.type()== ::casac::variant::DOUBLEVEC) ||
		    (varvalue.type()==::casac::variant::DOUBLE) ||
		    (varvalue.type()== ::casac::variant::COMPLEXVEC) ||
		    (varvalue.type()==::casac::variant::COMPLEX)
		) {  
		  Vector<DComplex> tmpintvec= varvalue.toComplexVec().size() > 0 ? Vector<DComplex>(varvalue.toComplexVec()): Vector<DComplex>(4, 0.0);
			value.resize(tmpintvec.size());
			convertArray(value, tmpintvec);
		}
		else{
			throw(AipsError("Could not understand the type of flux value variable"));
		}
		variant::TYPE errorType = error.type();
		Vector<DComplex> errs;
		if(
			(errorType == ::casac::variant::INTVEC) ||
			(errorType == ::casac::variant::INT) ||
			(errorType == ::casac::variant::DOUBLEVEC) ||
			(errorType ==::casac::variant::DOUBLE) ||
			(errorType == ::casac::variant::COMPLEXVEC) ||
			(errorType ==::casac::variant::COMPLEX)
		) {
			Vector<DComplex> tmpError(error.toComplexVec());
			ThrowIf(
				tmpError.size() != value.size(),
				"Error array must have same length as value array"
			);
			errs.resize(tmpError.size());
			convertArray(errs, tmpError);
		}

		if ((value.size() == 1) &&  (pol == ComponentType::STOKES)) {
			newFlux.setValue(real(value[0]));
			if (errs.size() == 1) {
				newFlux.setErrors(errs);
			}
		}
		else if (value.size() == 4) {
			Vector<Double> tmpC(4);
			Vector<Double> tmpE(errs.size());
			for(int i=0;i<4;i++) {
				tmpC[i] = real(value[i]);
				if (tmpE.size() == 4) {
					tmpE[i] = real(errs[i]);
				}
			}
			newFlux.setValue(tmpC);
			if (tmpE.size() == 4) {
				newFlux.setErrors(tmpE[0], tmpE[1], tmpE[2], tmpE[3]);
			}
		}
		else {
			*itsLog << "The flux must have one or four elements," << endl
				<< "one element can only be used if the polarization is 'Stokes'."
				<< endl << "Flux not changed on any components"
				<< LogIO::EXCEPTION;
		}
		const Vector<Int> intVec = checkIndices(which, "setflux",
				"Flux not changed on any components");
		itsList->setFlux(intVec, newFlux);
		rstat = true;

	}
	catch (AipsError x){
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x)
	}
	return rstat;
}

bool componentlist::convertfluxunit(const int which, const std::string& unit)
{
  itsLog->origin(LogOrigin("componentlist", "convertfluxunit"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      const Unit fluxUnit(unit);
      if (fluxUnit != Unit("Jy")) {
        *itsLog << LogIO::SEVERE
                << "The flux units must have the same dimensions as the Jansky"
                << endl << "Flux not changed on any components"
                << LogIO::POST;
        return false;
      }
      const Vector<Int> intVec = checkIndices(which, "convertfluxunit", 
                                              "Flux not changed on any components");
      itsList->convertFluxUnit(intVec, fluxUnit);
      rstat = true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::convertfluxpol(const int which, const std::string& polarization)
{
  itsLog->origin(LogOrigin("componentlist", "convertfluxpol"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      const Vector<Int> intVec = checkIndices(which, "convertfluxunit",
                                              "Flux not changed on any components");
      itsList->convertFluxPol(intVec,
                       (ComponentType::Polarisation)checkFluxPol(polarization));
      rstat=true;
    }
    else{
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch(AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

::casac::record* componentlist::getrefdir(const int which)
{
  itsLog->origin(LogOrigin("componentlist", "getrefdir"));

  ::casac::record *retval = 0;
  try{
    if(itsList && itsBin){
      _checkIndex(which);
      casa::MDirection refdir = itsList->component(which).shape().refDirection();
      ostringstream oss;
      refdir.print(oss);
      *itsLog << LogIO::NORMAL3 << String(oss) << LogIO::POST;
      MeasureHolder dirhold(refdir);
      Record outRec;
      String error;
      if (dirhold.toRecord(error, outRec)) {
        retval = fromRecord(outRec);
      } else {
        error += String("....Failed coversion of direction to record.\n");
        throw(AipsError(error));
      }
    }
    else{
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return retval;
}

std::string componentlist::getrefdirra(const int /*which*/, const std::string& /*unit*/,
                                       const int /*precision*/)
{
  itsLog->origin(LogOrigin("componentlist", "getrefdirra"));

  // TODO : IMPLEMENT ME HERE !
  std::string rstat("");
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "getrefdirra not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::string componentlist::getrefdirdec(const int /*which*/, const std::string& /*unit*/,
                                        const int /*precision*/)
{
  itsLog->origin(LogOrigin("componentlist", "setrefdir"));

  // TODO : IMPLEMENT ME HERE !
  std::string rstat("");
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "getrefdirdec not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::string componentlist::getrefdirframe(const int which)
{
  itsLog->origin(LogOrigin("componentlist", "setrefdir"));

  std::string rstat("");
  try{
    if(itsList && itsBin){
      _checkIndex(which);
      casa::MDirection md = itsList->component(which).shape().refDirection();
      rstat = md.getRefString().c_str();
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::setrefdir(const int which, const ::casac::variant& ra, 
                              const ::casac::variant& dec, const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "setrefdir"));

  bool rstat(false);
  try {
        if(itsList && itsBin){
	 String raval = ra.toString();
	 String decval = dec.toString();
         Quantum<Double> myra=casaQuantity(ra);
         Quantum<Double> mydec=casaQuantity(dec);
 
         const MVDirection newDir(myra, mydec);

         const Vector<Int> intVec(checkIndices(which, "setrefdir",
                                        "Direction not changed on any components"));
         itsList->setRefDirection(intVec, newDir);
         rstat = true;
	} else {
	  *itsLog << LogIO::WARN << "componentlist is not opened, please open first" << LogIO::POST;
	}
  } catch (AipsError x){
	  *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x)
  }
  return rstat;
}

bool componentlist::setrefdirframe(const int which, const std::string& frame,
                                   const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "setrefdirframe"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
	    casa::MDirection::Types newFrame;
      if (!casa::MDirection::getType(newFrame, frame)) {
        *itsLog << LogIO::SEVERE
                << "Could not parse the 'frame' string: Direction frame not changed"
                << LogIO::POST;
      }
      Vector<Int> intVec(1,which);
      itsList->setRefDirectionFrame(intVec, newFrame);
      rstat = true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE
            << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::convertrefdir(const int which, const std::string& frame)
{
  itsLog->origin(LogOrigin("componentlist", "convertrefdir"));

  bool rstat(false);
  try {
    if(itsList && itsBin){
	    casa::MDirection::Types newFrame;
      if (!casa::MDirection::getType(newFrame, frame)) {
        *itsLog << LogIO::SEVERE
                << "Could not parse the 'frame' string: Direction frame not changed"
                << LogIO::POST;
        return false;
      }                               
      Vector<Int> intVec(1,which);
      itsList->convertRefDirection(intVec, newFrame);
      rstat = true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
	  *itsLog << LogIO::SEVERE
                  << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x)
  }
  return rstat;
}

std::string
componentlist::shapetype(const int which)
{
  itsLog->origin(LogOrigin("componentlist", "shapetype"));

  std::string rstat("");
  try {
    if(itsList && itsBin){
      rstat = ComponentType::name(itsList->component(which).shape().type()).c_str();
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x) {
    *itsLog << LogIO::SEVERE
            << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::fromrecord(const ::casac::record& rec)
{
  itsLog->origin(LogOrigin("componentlist", "fromrecord"));

  bool rstat=false;
  try {
    if(itsList && (itsList->nelements() > 0)){
      *itsLog << LogIO::WARN
              << "componentlist is not empty; overwriting it" << LogIO::POST;
      delete itsList;
      itsList= new ComponentList();
      if(!itsBin){
	delete itsBin;
      }
      itsBin= new ComponentList();
    }
    Record *elRec=toRecord(rec);
    String error;
    if(!(itsList->fromRecord(error, *elRec))){
      delete elRec;
      throw(AipsError(String("Error ")+error+ String(" in converting from record")));
    }
    delete elRec;
    rstat=True;

  } catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

::casac::record* componentlist::torecord()
{
  itsLog->origin(LogOrigin("componentlist", "torecord"));

  casac::record* rstat(0);
  try {
    if(itsList && (itsList->nelements() > 0)){
      Record theRec;
      String error;
      if(!(itsList->toRecord(error, theRec))){
        throw(AipsError("converting componentlist to record failed with "+error));
      }
      rstat = fromRecord(theRec); 
    }
    else{
      *itsLog << LogIO::WARN << "componentlist is empty" << LogIO::POST;

    }
       
  } catch (AipsError x){
	  *itsLog << LogIO::SEVERE
                  << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x)
	    }
  return rstat;
}

::casac::record* componentlist::getshape(const int which)
{
  itsLog->origin(LogOrigin("componentlist", "getshape"));

  casac::record* rstat(0);
  try {
    if(itsList && itsBin){
      const ComponentShape& shape = itsList->component(which).shape();
      Record rec;
      String errorMessage;
      if (!shape.toRecord(errorMessage, rec)) {
        *itsLog << "Could not get the component shape because:" << endl
                << errorMessage
                << "Empty record returned" << LogIO::EXCEPTION;
      }
      rec.removeField(RecordFieldId("type"));
      rec.removeField(RecordFieldId("direction"));
      rstat = fromRecord(rec);
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
	  *itsLog << LogIO::SEVERE
                  << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x)
  }
  return rstat;
}

bool componentlist::setshape(const int which, const std::string& type,
                             const ::casac::variant& majoraxis,
                             const ::casac::variant& minoraxis,
                             const ::casac::variant& positionangle,
                             const ::casac::variant& majoraxiserror,
                             const ::casac::variant& minoraxiserror,
                             const ::casac::variant& positionangleerror,
                             const std::vector<double>& optionalparms,
                             const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "setshape"));

  bool rstat(false);
  try {
    if(itsList && itsBin){
      ComponentType::Shape reqShape = ComponentType::shape(type);
      ComponentShape* shapePtr = ComponentType::construct(reqShape);
      if (shapePtr == 0) {
        *itsLog << LogIO::SEVERE
                << "Could not translate the shape type to a known value." << endl
                << "Known types are:" << endl;
        for (uInt i = 0; i < ComponentType::NUMBER_SHAPES - 1; i++) {
          reqShape = (ComponentType::Shape) i;
          *itsLog <<  ComponentType::name(reqShape) + String("\n");
        }
        *itsLog << "Shape not changed." << LogIO::POST;
        return false;
      }

       
      String errorMessage;
      Record rec;
      Record ma1;
      QuantumHolder(casaQuantity(majoraxis)).toRecord(errorMessage, ma1);
      Record ma2;
      QuantumHolder(casaQuantity(minoraxis)).toRecord(errorMessage, ma2);
      Record ma3;
      QuantumHolder(casaQuantity(positionangle)).toRecord(errorMessage, ma3);
      rec.defineRecord("majoraxis",ma1);
      rec.defineRecord("minoraxis",ma2);
      rec.defineRecord("positionangle",ma3);
       
      if( !(String(majoraxiserror.toString())==String("[]")) &&
          !(String(minoraxiserror.toString())==String("[]")) &&
          !(String(positionangleerror.toString())==String("[]"))){
        Record ma4;
        QuantumHolder(casaQuantity(majoraxiserror)).toRecord(errorMessage, ma4);
        Record ma5;
        QuantumHolder(casaQuantity(minoraxiserror)).toRecord(errorMessage, ma5);
        Record ma6;
        QuantumHolder(casaQuantity(positionangleerror)).toRecord(errorMessage, ma6);
       
        rec.defineRecord("majoraxiserror",ma4);
        rec.defineRecord("minoraxiserror",ma5);
        rec.defineRecord("positionangleerror",ma6);
      }
      else{
        Record ma4;
        QuantumHolder(casa::Quantity(0.0,"rad")).toRecord(errorMessage, ma4);
        rec.defineRecord("majoraxiserror",ma4);
        rec.defineRecord("minoraxiserror",ma4);
        rec.defineRecord("positionangleerror",ma4);
      }
      if (!shapePtr->fromRecord(errorMessage, rec)) {
        *itsLog << LogIO::SEVERE
                << "Could not parse the shape parameters. The error was:" << endl
                << "\t" << errorMessage
                << "Shape not changed." << LogIO::POST;
        return false;
      }
      Vector<Int> intVec(1,which);
      if (type=="limbdarkeneddisk") {
        shapePtr->setOptParameters(optionalparms);
        itsList->setOptParms(intVec, *shapePtr);
      }
      itsList->setShapeParms(intVec, *shapePtr);

      delete shapePtr;
      rstat = true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
	  *itsLog << LogIO::SEVERE
                  << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x)
  }
  return rstat;
}

bool componentlist::convertshape(const int /*which*/, const std::string& /*majoraxis*/,
                                 const std::string& /*minoraxis*/,
                                 const std::string& /*positionangle*/)
{
  itsLog->origin(LogOrigin("componentlist", "convertshape"));
  
  // TODO : IMPLEMENT ME HERE !       
  bool rstat(false);
  try {
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "convertshape not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::string componentlist::spectrumtype(const int which)
{
  itsLog->origin(LogOrigin("componentlist", "spectrumtype"));

  std::string rstat("");
  try {
    if(itsList && itsBin){
      rstat = ComponentType::name(itsList->component(which).spectrum().type()).c_str();
    }
    else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

::casac::record* componentlist::getspectrum(const int which)
{
  itsLog->origin(LogOrigin("componentlist", "getspectrum"));

  casac::record *rstat(0);
  try{
    if(itsList && itsBin){
      const SpectralModel& spectrum = itsList->component(which).spectrum();
      String errorMessage;
      Record rec;
      if (!spectrum.toRecord(errorMessage, rec)) {
        *itsLog << "Could not get the component spectrum because:" << endl
                << errorMessage
                << "Empty record returned" << LogIO::EXCEPTION;
      }
      rstat = fromRecord (rec);
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::setspectrum(const int which, const std::string& eltype,
                                const double index, const std::vector<double>& tabfreqs, const std::vector<double>& tabflux, const std::string& freqframe)
{
  itsLog->origin(LogOrigin("componentlist", "setspectrum"));


  bool rstat(false);
  try {
    if(itsList && itsBin){
      String type(eltype);
      type.upcase();
      if(type.contains("TABU"))
	 type="Tabular Spectrum";	 
      if(type.contains("SPECTRAL"))
	 type="Spectral Index";
       ComponentType::SpectralShape reqSpectrum = ComponentType::spectralShape(type);
       SpectralModel* spectrumPtr = ComponentType::construct(reqSpectrum);
       if (spectrumPtr == 0) {
         *itsLog << LogIO::SEVERE
                << "Could not translate the spectral type to a known value." << endl
                << "Known types are:" << endl;
         for(uInt i = 0; i < ComponentType::NUMBER_SPECTRAL_SHAPES - 1; ++i){
           reqSpectrum = static_cast<ComponentType::SpectralShape>(i);
           *itsLog << "\t" << ComponentType::name(reqSpectrum) + String("\n");
         } 
         *itsLog << "Spectrum not changed." << LogIO::POST;
         return false;
       }
       if(reqSpectrum==ComponentType::TABULAR_SPECTRUM){
	 if(tabfreqs.size() <2)
	   throw(AipsError("There need to be at least 2 points in a tabular spectrum to interpolate in between"));
	 if(tabfreqs.size() != tabflux.size())
	   throw(AipsError("lengths of tabular frequencies and values have to be the same"));
	 Vector<MVFrequency> freqs(tabfreqs.size());
	 Vector<Flux<Double> > fluxval(tabfreqs.size());
	 for (Int i=0; i < Int(tabfreqs.size()) ; ++i){
	   freqs[i]=casa::Quantity(tabfreqs[i], "Hz");
	   fluxval[i]=Flux<Double>(tabflux[i]);
	 }
	 MFrequency::Types freqFrameType;
	 if(!MFrequency::getType(freqFrameType, freqframe))
	   throw(AipsError(String(freqframe) + String(" is not a frequency frame that is understood")));
	 MFrequency refreq(freqs[0], freqFrameType);
	 delete spectrumPtr;
	 spectrumPtr=new TabularSpectrum(refreq, freqs, fluxval, MFrequency::Ref(freqFrameType));
	 

       }
       else{
	 String errorMessage;
	 Record rec;
	 rec.define("frequency", "current");
	 *itsLog << LogIO::DEBUG1 << "index: " << index << LogIO::POST;
	 // Vector<Double> indexVec(index);
	 // if(indexVec.nelements() > 0)
	 //   rec.define("index", indexVec[0]);
	 // else
	 rec.define("index", index);
	 if (!spectrumPtr->fromRecord(errorMessage, rec)) {
	   *itsLog << LogIO::SEVERE
		   << "Could not parse the spectrum parameters. The error was:" << endl
		   << "\t" << errorMessage << endl
		   << "Spectrum not changed."
		   << LogIO::POST;
	   return false;
	 }
       }
       Vector<Int> intVec(1, which);
       itsList->setSpectrumParms(intVec, *spectrumPtr);
       delete spectrumPtr;

       rstat = true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::setstokesspectrum(const int which, const std::string& eltype,
				      const std::vector<double>& index, const std::vector<double>& tabfreqs, const std::vector<double>& tabi, const std::vector<double>& tabq, 
				      const std::vector<double>& tabu,  const std::vector<double>& tabv, const ::casac::variant& reffreq,  
				      const std::string& freqframe)
{
  itsLog->origin(LogOrigin("componentlist", "setspectrum"));


  bool rstat(false);
  try {
    if(itsList && itsBin){
      String type(eltype);
      type.upcase();
      if(type.contains("TABU"))
	 type="Tabular Spectrum";	 
      if(type.contains("SPECTRAL"))
	 type="Spectral Index";
       ComponentType::SpectralShape reqSpectrum = ComponentType::spectralShape(type);
       SpectralModel* spectrumPtr = ComponentType::construct(reqSpectrum);
       if (spectrumPtr == 0) {
         *itsLog << LogIO::SEVERE
                << "Could not translate the spectral type to a known value." << endl
                << "Known types are:" << endl;
         for(uInt i = 0; i < ComponentType::NUMBER_SPECTRAL_SHAPES - 1; ++i){
           reqSpectrum = static_cast<ComponentType::SpectralShape>(i);
           *itsLog << "\t" << ComponentType::name(reqSpectrum) + String("\n");
         } 
         *itsLog << "Spectrum not changed." << LogIO::POST;
         return false;
       }
       if(reqSpectrum==ComponentType::TABULAR_SPECTRUM){
	 if(tabfreqs.size() <2)
	   throw(AipsError("There need to be at least 2 points in a tabular spectrum to interpolate in between"));
	 if(tabfreqs.size() != tabi.size())
	   throw(AipsError("lengths of tabular frequencies and I values have to be the same"));
	 if(tabfreqs.size() != tabq.size())
	   throw(AipsError("lengths of tabular frequencies and Q values have to be the same"));
	 if(tabfreqs.size() != tabu.size())
	   throw(AipsError("lengths of tabular frequencies and U values have to be the same"));
	  if(tabfreqs.size() != tabv.size())
	   throw(AipsError("lengths of tabular frequencies and V values have to be the same"));
	 Vector<MVFrequency> freqs(tabfreqs.size());
	 Vector<Flux<Double> > fluxval(tabfreqs.size());
	 for (Int i=0; i < Int(tabfreqs.size()) ; ++i){
	   freqs[i]=casa::Quantity(tabfreqs[i], "Hz");
	   fluxval[i]=Flux<Double>(tabi[i], tabq[i], tabu[i], tabv[i]);
	 }
	 MFrequency::Types freqFrameType;
	 if(!MFrequency::getType(freqFrameType, freqframe))
	   throw(AipsError(String(freqframe) + String(" is not a frequency frame that is understood")));
	 MFrequency refreq(freqs[0], freqFrameType);
	 delete spectrumPtr;
	 spectrumPtr=new TabularSpectrum(refreq, freqs, fluxval, MFrequency::Ref(freqFrameType));
	 

       }
       else{
	 String errorMessage;
	 Record rec;
	 Record freqRec;
	 
	 casa::Quantity freqQ=String(reffreq.toString()) != String("[]") ? casaQuantity(reffreq) : casa::Quantity(1.4, "GHz");
	 MFrequency::Types eltype;
	 if(!MFrequency::getType(eltype, freqframe))
	   throw(AipsError(String(freqframe) + String(" is not a frequency frame that is understood")));
	 MeasureHolder(MFrequency(freqQ, eltype)).toRecord(errorMessage,freqRec);
	  setfreq(which, freqQ.get("GHz").getValue(), "GHz", True);
	  setfreqframe(which, freqframe, True);
	 rec.defineRecord("frequency", freqRec);
	 *itsLog << LogIO::DEBUG1 << "index: " << index << LogIO::POST;
	 // Vector<Double> indexVec(index);
	 // if(indexVec.nelements() > 0)
	 //   rec.define("index", indexVec[0]);
	 // else
	 rec.define("index", index[0]);
	 if(index.size() == 4)
	   rec.define("stokesindex", Vector<Double>(index));
	 if (!spectrumPtr->fromRecord(errorMessage, rec)) {
	   *itsLog << LogIO::SEVERE
		   << "Could not parse the spectrum parameters. The error was:" << endl
		   << "\t" << errorMessage << endl
		   << "Spectrum not changed."
		   << LogIO::POST;
	   return false;
	 }
       }
       Vector<Int> intVec(1, which);
       itsList->setSpectrumParms(intVec, *spectrumPtr);
       delete spectrumPtr;

       rstat = true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

::casac::record* componentlist::getfreq(const int /*which*/)
{
  itsLog->origin(LogOrigin("componentlist", "getfreq"));

  // TODO : IMPLEMENT ME HERE !
  ::casac::record *rstat(0);
  try {
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "getfreq not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

double componentlist::getfreqvalue(const int /*which*/)
{
  itsLog->origin(LogOrigin("componentlist", "getfreqvalue"));

  // TODO : IMPLEMENT ME HERE !
  double rstat(false);
  try {
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "getfreqvalue not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::string componentlist::getfrequnit(const int /*which*/)
{
  itsLog->origin(LogOrigin("componentlist", "getfrequnit"));

  // TODO : IMPLEMENT ME HERE !
  std::string rstat("");
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "getfrequnit not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

std::string componentlist::getfreqframe(const int /*which*/)
{
  itsLog->origin(LogOrigin("componentlist", "getfreqframe"));

  // TODO : IMPLEMENT ME HERE !
  std::string rstat("");
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "getfreqframe not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch(AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::setfreq(const int which, const double value,
                            const std::string& unit, const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "setfreq"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      const MVFrequency newFreq(Quantum<Double>(value, Unit(unit)));
      Vector<Int> intVec(1,which);
      itsList->setRefFrequency(intVec, newFreq);
      convertfrequnit(which, unit);
		    
      rstat = true;
    }
    else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::setfreqframe(const int which, const std::string& frame,
                                 const bool /*log*/)
{
  itsLog->origin(LogOrigin("componentlist", "setfreqframe"));
  
  bool rstat(false);
  try{
    if(itsList && itsBin){
      MFrequency::Types newFrame;
      if (!MFrequency::getType(newFrame, frame)) {
        *itsLog << LogIO::SEVERE
                << "Could not parse the 'frame' string: Frequency frame not changed"
                << LogIO::POST;
        return false;
      }
      Vector<Int> intVec(1, which);
      itsList->setRefFrequencyFrame(intVec, newFrame);

      rstat = true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  } catch (AipsError x){
    *itsLog << LogIO::SEVERE
            << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::convertfrequnit(const int which, const std::string& unit)
{
  itsLog->origin(LogOrigin("componentlist", "convertfrequnit"));
  
  bool rstat(false);
  try{
    if(itsList && itsBin){
      Vector<Int> intVec(1, which);
      itsList->setRefFrequencyUnit(intVec, Unit(unit));
      rstat = true;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}


::casac::record* componentlist::getcomponent(const int which, const bool /*iknow*/)
{
  itsLog->origin(LogOrigin("componentlist", "getcomponent"));

  casac::record *rstat(0);
  try{
    if(itsList && itsBin){
      _checkIndex(which);
      const SkyComponent& listRef(itsList->component(which));
      String error;
      Record leRec;
      if(!listRef.toRecord(error, leRec)){
        throw(AipsError(String("Converting compenent torecord failed with error")
                        + error));
      }
      rstat=fromRecord(leRec);
    }
    else{
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE
            << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::add(const ::casac::record& thecomponent, const bool /*iknow*/)
{
  itsLog->origin(LogOrigin("componentlist", "add"));

  bool rstat(false);
  try{
    if(itsList && itsBin){
      Record *elRec=toRecord(thecomponent);
      SkyComponent elComp;
      String err;
      if(!elComp.fromRecord(err, *elRec)){
        throw(AipsError(String("Converting Component from record failed with error ")
                        + err));
      }
      itsList->add(elComp);
      if(elRec !=0){
        delete elRec;
      }
      rstat=True;
    }
    else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::replace(const int /*which*/, const ::casac::record& /*list*/,
                            const std::vector<int>& /*whichones*/)
{
  itsLog->origin(LogOrigin("componentlist", "replace"));

  // TODO : IMPLEMENT ME HERE !
  bool rstat(false);
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN << "replace not implemented yet" << LogIO::POST;
    } else {
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

bool componentlist::summarize(const int which) {
	itsLog->origin(LogOrigin("componentlist", __FUNCTION__));
	Bool rstat = False;
	try{
		if(itsList && itsBin) {
			if(uInt(which) >= itsList->nelements() ) {
				ostringstream oss;
				oss << "List has only " << itsList->nelements()
    					<< " components, but zero-based component " << which << " specified. "
    					<< "Please specify a component less than " << itsList->nelements();
					throw AipsError(oss.str());
			}
			*itsLog << LogIO::NORMAL << itsList->summarize(which) << LogIO::POST;
			rstat = True;
		} else {
			*itsLog << LogIO::WARN
					<< "componentlist is not opened, please open first" << LogIO::POST;
			rstat = False;
		}
	}
	catch (AipsError x){
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x)
	}
	return rstat;
}

bool componentlist::iscomponentlist(const ::casac::variant& /*tool*/)
{
  itsLog->origin(LogOrigin("componentlist", "iscomponentlist"));

  // TODO : IMPLEMENT ME HERE !
  bool rstat(false);
  try{
    if(itsList && itsBin){
      *itsLog << LogIO::WARN
              << "is_componentlist not implemented yet" << LogIO::POST;
    }
    else{
      *itsLog << LogIO::WARN
              << "componentlist is not opened, please open first" << LogIO::POST;
    }
  }
  catch(AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x)
  }
  return rstat;
}

Vector<Int> componentlist::checkIndices(int which, const String& function,
                                         const String& message) const
{
  std::vector<int> whichVec(1, which);
  return checkIndices(whichVec, function, message);
}
//
//
Vector<Int> componentlist::checkIndices(const vector<int>& which,
                                        const String& function,
                                        const String& message) const
{
  itsLog->origin(LogOrigin("componentlist", function));
  
  Vector<Int> intVec(which);
  if((intVec.nelements()==1) && (intVec[0]==-1)){
    intVec.resize( itsList->nelements());
    indgen(intVec);
  }
  const Int listLength = itsList->nelements();
  const uInt whichLength = which.size();
  for (uInt c = 0; c < whichLength; c++) {
    if (intVec(c) < 0 || intVec(c) >= listLength) {
      *itsLog << LogIO::SEVERE
              << "Index out of range." << endl
              << "A component number is less than one or greater than"
              << " the list length" << endl
              << message
              << LogIO::EXCEPTION;
    }
  }
  return intVec;
}

int componentlist::checkFluxPol(const String& polString)
{
  itsLog->origin(LogOrigin("componentlist", "checkFluxPol"));

  const ComponentType::Polarisation pol(ComponentType::polarisation(polString));
  if(pol == ComponentType::UNKNOWN_POLARISATION){
    *itsLog << "Unknown polarization. Possible values are:" << endl;
    for (uInt i = 0; i < ComponentType::NUMBER_POLARISATIONS - 1; i++) {
      *itsLog << " '"
             <<ComponentType::name(static_cast<ComponentType::Polarisation>(i))
             << "' ";
    }
    *itsLog << LogIO::EXCEPTION;
  }
  return pol;
}

void componentlist::_checkIndex(int which) const {
	Int m = itsList->nelements() - 1;
	ThrowIf(
		which < 0 || which > m,
		"Index out of range. The component number must be between 0 and " + String::toString(m)
    );
}

} // casac namespace

