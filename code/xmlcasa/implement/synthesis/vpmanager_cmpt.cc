
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
#include <xmlcasa/synthesis/vpmanager_cmpt.h>
#include <casa/Logging/LogIO.h>
#include<casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include<casa/Utilities/Assert.h>
#include<measures/Measures/MDirection.h>
#include <synthesis/MeasurementEquations/VPManager.h>

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

} // casac namespace

