
/***
 * Framework independent implementation file for vpmanager...
 *
 * Implement the vpmanager component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <vpmanager_cmpt.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/Path.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/Assert.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVFrequency.h>
#include <casa/Quanta/Quantum.h>
#include <synthesis/MeasurementEquations/VPManager.h>
#include <images/Images/AntennaResponses.h>

using namespace std;
using namespace casa;

namespace casac {

vpmanager::vpmanager()
{

  itsVPM = new VPManager();
  itsLog = new LogIO();

}

vpmanager::~vpmanager()
{
  if(itsVPM) delete itsVPM;
  itsVPM=0;
  if(itsLog) delete itsLog;
  itsLog=0;

}

bool
vpmanager::saveastable(const std::string& tablename)
{
  bool rstat=false;
  try{
    if(!itsVPM) return rstat;
    rstat=itsVPM->saveastable(String(tablename));

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
vpmanager::summarizevps(const bool verbose)
{
  bool rstat=false;

  try {
    if (!itsVPM) return rstat;
    rstat = itsVPM->summarizevps(verbose);
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
vpmanager::done()
{
  if(itsVPM) delete itsVPM;
  itsVPM=0;
  if(itsLog) delete itsLog;
  itsLog=0;
  itsVPM = new VPManager();
  itsLog = new LogIO();
  return true;
}

//TODO!
// list
// vpmanager::list_known_telescopes()
// {
//   return me.   // Use pbmath instead.
// }

::casac::record*
vpmanager::setcannedpb(const std::string& telescope, const std::string& othertelescope, const bool dopb, const std::string& commonpb, const bool dosquint, const ::casac::variant& paincrement, const bool usesymmetricbeam)
{
  ::casac::record* r=0;
  try{
    if(!itsVPM) return r;

    casa::Quantity pai;
    if(toCasaString(paincrement)==casa::String(""))
      pai=casa::Quantity(720,"deg");
    else
      pai=casaQuantity(paincrement);

    casa::Record rec;
    itsVPM->setcannedpb(String(telescope), String(othertelescope), dopb, String(commonpb),
			dosquint, pai, usesymmetricbeam, rec);
    r=fromRecord(rec);

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

::casac::record*
vpmanager::setpbairy(const std::string& telescope, const std::string& othertelescope, const bool dopb, const ::casac::variant& dishdiam, const ::casac::variant& blockagediam, const  ::casac::variant& maxrad, const ::casac::variant& reffreq, const ::casac::variant& squintdir, const ::casac::variant& squintreffreq, const bool dosquint, const ::casac::variant& paincrement, const bool usesymmetricbeam)
{

  ::casac::record* r=0;
  try{
    if(!itsVPM) return r;
    casa::Quantity rf;
    if(toCasaString(reffreq)==casa::String(""))
      rf=casa::Quantity(1.0,"GHz");
    else
      rf=casaQuantity(reffreq);
    casa::Quantity srf;
    if(toCasaString(squintreffreq)==casa::String(""))
      srf=casa::Quantity(1.0,"GHz");
    else
      srf=casaQuantity(squintreffreq);
    casa::Quantity pai;
    if(toCasaString(paincrement)==casa::String(""))
      pai=casa::Quantity(720,"deg");
    else
      pai=casaQuantity(paincrement);
    MDirection sdir;
    if(toCasaString(squintdir)==casa::String(""))
      sdir=MDirection(casa::Quantity(0,"deg"),casa::Quantity(0,"deg")) ;
    else
      casaMDirection(squintdir, sdir);
    casa::Quantity dd;
    if(toCasaString(dishdiam)==casa::String(""))
      dd=casa::Quantity(25.0,"m");
    else
      dd=casaQuantity(dishdiam);
    casa::Quantity bd;
    if(toCasaString(blockagediam)==casa::String(""))
      bd=casa::Quantity(2.5,"m");
    else
      bd=casaQuantity(blockagediam);
    casa::Quantity mr;
    if(toCasaString(maxrad)==casa::String(""))
      mr=casa::Quantity(0.8,"deg");
    else
      mr=casaQuantity(maxrad);
    casa::Record rec;
    itsVPM->setpbairy(String(telescope), String(othertelescope), dopb, dd, bd, 
		      mr, rf, sdir, srf, dosquint, pai, usesymmetricbeam, rec);
    r=fromRecord(rec);

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

::casac::record*
vpmanager::setpbcospoly(const std::string& telescope, const std::string& othertelescope, const bool dopb, const std::vector<double>& coeff, const std::vector<double>& scale, const ::casac::variant& maxrad, const ::casac::variant& reffreq, const std::string& isthispb, const ::casac::variant& squintdir, const ::casac::variant& squintreffreq, const bool dosquint, const ::casac::variant& paincrement, const bool usesymmetricbeam)
{
  ::casac::record* r=0;
  try{
    if(!itsVPM) return r;
    casa::Quantity mr;
    if(toCasaString(maxrad)==casa::String(""))
      mr=casa::Quantity(0.8,"deg");
    else
      mr=casaQuantity(maxrad);
    casa::Quantity rf;
    if(toCasaString(reffreq)==casa::String(""))
      rf=casa::Quantity(1.0,"GHz");
    else
      rf=casaQuantity(reffreq);
    MDirection sdir;
    if(toCasaString(squintdir)==casa::String(""))
      sdir=MDirection(casa::Quantity(0,"deg"),casa::Quantity(0,"deg")) ;
    else
      casaMDirection(squintdir, sdir);
    casa::Quantity srf;
    if(toCasaString(squintreffreq)==casa::String(""))
      srf=casa::Quantity(1.0,"GHz");
    else
      srf=casaQuantity(squintreffreq);
    casa::Quantity pai;
    if(toCasaString(paincrement)==casa::String(""))
      pai=casa::Quantity(720,"deg");
    else
      pai=casaQuantity(paincrement);
    casa::Record rec;
    itsVPM->setpbcospoly(String(telescope), String(othertelescope), dopb,
			 coeff, scale, mr, rf, isthispb, sdir, srf,
			 dosquint, pai, usesymmetricbeam, rec);
    r=fromRecord(rec);

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

::casac::record*
vpmanager::setpbgauss(const std::string& telescope, const std::string& othertelescope, const bool dopb, const ::casac::variant& halfwidth, const ::casac::variant& maxrad, const ::casac::variant& reffreq, const std::string& isthispb, const ::casac::variant& squintdir, const ::casac::variant& squintreffreq, const bool dosquint, const ::casac::variant& paincrement, const bool usesymmetricbeam)
{

  ::casac::record* r=0;

  try{
    if(!itsVPM) return r;
    casa::Quantity rf;
    if(toCasaString(reffreq)==casa::String(""))
      rf=casa::Quantity(1.0,"GHz");
    else
      rf=casaQuantity(reffreq);
    casa::Quantity srf;
    if(toCasaString(squintreffreq)==casa::String(""))
      srf=casa::Quantity(1.0,"GHz");
    else
      srf=casaQuantity(squintreffreq);
    casa::Quantity pai;
    if(toCasaString(paincrement)==casa::String(""))
      pai=casa::Quantity(720,"deg");
    else
      pai=casaQuantity(paincrement);
    MDirection sdir;
    if(toCasaString(squintdir)==casa::String(""))
      sdir=MDirection(casa::Quantity(0,"deg"),casa::Quantity(0,"deg")) ;
    else
      casaMDirection(squintdir, sdir);
    casa::Quantity hw;
    if(toCasaString(halfwidth)==casa::String(""))
      hw=casa::Quantity(0.5,"deg");
    else
      hw=casaQuantity(halfwidth);
    casa::Quantity mr;
    if(toCasaString(maxrad)==casa::String(""))
      mr=casa::Quantity(0.8,"deg");
    else
      mr=casaQuantity(maxrad);
    casa::Record rec;
    itsVPM->setpbgauss(String(telescope), String(othertelescope), dopb, hw, 
		      mr, rf, String(isthispb), sdir, srf, dosquint, pai, 
		      usesymmetricbeam, rec);
    r=fromRecord(rec);

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }


  return r;
}

::casac::record*
vpmanager::setpbinvpoly(const std::string& telescope, const std::string& othertelescope, const bool dopb, const std::vector<double>& coeff, const ::casac::variant& maxrad, const ::casac::variant& reffreq, const std::string& isthispb, const ::casac::variant& squintdir, const ::casac::variant& squintreffreq, const bool dosquint, const ::casac::variant& paincrement, const bool usesymmetricbeam)
{
  ::casac::record* r=0;
  try{
    if(!itsVPM) return r;
    casa::Quantity mr;
    if(toCasaString(maxrad)==casa::String(""))
      mr=casa::Quantity(0.8,"deg");
    else
      mr=casaQuantity(maxrad);
    casa::Quantity rf;
    if(toCasaString(reffreq)==casa::String(""))
      rf=casa::Quantity(1.0,"GHz");
    else
      rf=casaQuantity(reffreq);
    MDirection sdir;
    if(toCasaString(squintdir)==casa::String(""))
      sdir=MDirection(casa::Quantity(0,"deg"),casa::Quantity(0,"deg")) ;
    else
      casaMDirection(squintdir, sdir);
    casa::Quantity srf;
    if(toCasaString(squintreffreq)==casa::String(""))
      srf=casa::Quantity(1.0,"GHz");
    else
      srf=casaQuantity(squintreffreq);
    casa::Quantity pai;
    if(toCasaString(paincrement)==casa::String(""))
      pai=casa::Quantity(720,"deg");
    else
      pai=casaQuantity(paincrement);
    casa::Record rec;
    itsVPM->setpbinvpoly(String(telescope), String(othertelescope), dopb,
			 coeff, mr, rf, isthispb, sdir, srf,
			 dosquint, pai, usesymmetricbeam, rec);
    r=fromRecord(rec);

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

::casac::record*
vpmanager::setpbnumeric(const std::string& telescope, const std::string& othertelescope, const bool dopb, const std::vector<double>& vect, const ::casac::variant& maxrad, const ::casac::variant& reffreq, const std::string& isthispb, const ::casac::variant& squintdir, const ::casac::variant& squintreffreq, const bool dosquint, const ::casac::variant& paincrement, const bool usesymmetricbeam)
{
  ::casac::record* r=0;
  try{
    if(!itsVPM) return r;
    casa::Quantity mr;
    if(toCasaString(maxrad)==casa::String(""))
      mr=casa::Quantity(0.8,"deg");
    else
      mr=casaQuantity(maxrad);
    casa::Quantity rf;
    if(toCasaString(reffreq)==casa::String(""))
      rf=casa::Quantity(1.0,"GHz");
    else
      rf=casaQuantity(reffreq);
    MDirection sdir;
    if(toCasaString(squintdir)==casa::String(""))
      sdir=MDirection(casa::Quantity(0,"deg"),casa::Quantity(0,"deg")) ;
    else
      casaMDirection(squintdir, sdir);
    casa::Quantity srf;
    if(toCasaString(squintreffreq)==casa::String(""))
      srf=casa::Quantity(1.0,"GHz");
    else
      srf=casaQuantity(squintreffreq);
    casa::Quantity pai;
    if(toCasaString(paincrement)==casa::String(""))
      pai=casa::Quantity(720,"deg");
    else
      pai=casaQuantity(paincrement);
    casa::Record rec;
    itsVPM->setpbnumeric(String(telescope), String(othertelescope), dopb,
			 vect, mr, rf, isthispb, sdir, srf,
			 dosquint, pai, usesymmetricbeam, rec);
    r=fromRecord(rec);

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

::casac::record*
vpmanager::setpbimage(const std::string& telescope, const std::string& othertelescope, const bool dopb, const std::string& realimage, const std::string& imagimage)
{

  ::casac::record* r=0;
  try{
    if(!itsVPM) return r;
    casa::Record rec;
    itsVPM->setpbimage(String(telescope), String(othertelescope), dopb, 
		       String(realimage), String(imagimage), rec);
    r=fromRecord(rec);

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return r;
}

::casac::record*
vpmanager::setpbpoly(const std::string& telescope, const std::string& othertelescope, const bool dopb, const std::vector<double>& coeff, const ::casac::variant& maxrad, const ::casac::variant& reffreq, const std::string& isthispb, const ::casac::variant& squintdir, const ::casac::variant& squintreffreq, const bool dosquint, const ::casac::variant& paincrement, const bool usesymmetricbeam)
{
  ::casac::record* r=0;
  try{
    if(!itsVPM) return r;
    casa::Quantity mr;
    if(toCasaString(maxrad)==casa::String(""))
      mr=casa::Quantity(0.8,"deg");
    else
      mr=casaQuantity(maxrad);
    casa::Quantity rf;
    if(toCasaString(reffreq)==casa::String(""))
      rf=casa::Quantity(1.0,"GHz");
    else
      rf=casaQuantity(reffreq);
    MDirection sdir;
    if(toCasaString(squintdir)==casa::String(""))
      sdir=MDirection(casa::Quantity(0,"deg"),casa::Quantity(0,"deg")) ;
    else
      casaMDirection(squintdir, sdir);
    casa::Quantity srf;
    if(toCasaString(squintreffreq)==casa::String(""))
      srf=casa::Quantity(1.0,"GHz");
    else
      srf=casaQuantity(squintreffreq);
    casa::Quantity pai;
    if(toCasaString(paincrement)==casa::String(""))
      pai=casa::Quantity(720,"deg");
    else
      pai=casaQuantity(paincrement);
    casa::Record rec;
    itsVPM->setpbpoly(String(telescope), String(othertelescope), dopb,
		      coeff, mr, rf, isthispb, sdir, srf,
		      dosquint, pai, usesymmetricbeam, rec);
    r=fromRecord(rec);

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

bool 
vpmanager::createantresp(const std::string& telescope, 
			 const std::string& imdir, 
			 const std::string& starttime, 
			 const std::vector<std::string>& bandnames, 
			 const std::vector<std::string>& bandminfreq, 
			 const std::vector<std::string>& bandmaxfreq)
{
  bool rstat(False);
  try{

    *itsLog << LogOrigin("vp", "createantresp");

    cout << "telescope " << telescope << " imdir " << imdir << " starttime " << starttime << endl;
    for(int i=0; i<bandnames.size(); i++){
      cout << i << " bandnames " << bandnames[i] << " bandminfreq " << bandminfreq[i] << " bandmaxfreq " << bandmaxfreq[i] << endl;
    }

    String obsName = toCasaString(telescope);
    MPosition Xpos;
    if (obsName.length() == 0 || 
	!MeasTable::Observatory(Xpos,obsName)) {
      // unknown observatory
      *itsLog << LogIO::SEVERE << "Unknown observatory: \"" << obsName << "\"." << LogIO::POST;
      return rstat;
    }

    String imDir = toCasaString(imdir);
    Path imPath(imDir);
    String absPathName = imPath.absoluteName();
    Path absImPath(absPathName);
    if(!absImPath.isValid()){
      *itsLog << LogIO::SEVERE << "Invalid path: \"" << imDir  << "\"." << LogIO::POST;
      return rstat;
    }
      
    String startTime = toCasaString(starttime);
    MEpoch theStartTime; 
    Quantum<Double> qt;
    if (MVTime::read(qt,startTime)) {
      MVEpoch mv(qt);
      theStartTime = MEpoch(mv, MEpoch::UTC);
    } else {
      *itsLog << LogIO::SEVERE << "Invalid time format: " 
	      << startTime << LogIO::POST;
      return rstat;
    }

    uInt nBands = bandnames.size();

    if((nBands != bandminfreq.size()) || (nBands != bandmaxfreq.size())){
      *itsLog << LogIO::SEVERE << "bandnames, bandminfreq, and bandmaxfreq need to have the same number of elements." 
	      << LogIO::POST;
      return rstat;
    }
      
    for(uInt i=0; i<nBands; i++){
      Quantum<Double> freqmin, freqmax;
      if(!Quantum<Double>::read(freqmin, toCasaString(bandminfreq[i]))){
	*itsLog << LogIO::SEVERE << "Invalid quantity in bandminfreq " << i << ": " << bandminfreq[i] 
		<< LogIO::POST;
	return rstat;
      }
      if(!Quantum<Double>::read(freqmax, toCasaString(bandmaxfreq[i]))){
	*itsLog << LogIO::SEVERE << "Invalid quantity in bandmaxfreq " << i << ": " << bandmaxfreq[i] 
		<< LogIO::POST;
	return rstat;
      }

      try{
	MVFrequency fMin(freqmin);
      }
      catch(AipsError x) {
	*itsLog << LogIO::SEVERE << "Error interpreting bandminfreq " << i << " as frequency: " << bandminfreq[i] << endl << x.getMesg() << LogIO::POST;
	return rstat;
      }
      try{
	MVFrequency fMax(freqmax);
      }
      catch(AipsError x) {
	*itsLog << LogIO::SEVERE << "Error interpreting bandmaxfreq " << i << " as frequency: " << bandmaxfreq[i] << endl << x.getMesg() << LogIO::POST;
	return rstat;
      }

    }

    rstat = True;

  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}


} // casac namespace

