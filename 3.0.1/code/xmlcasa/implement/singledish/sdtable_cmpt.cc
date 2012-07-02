
/***
 * Framework independent implementation file for sdtable...
 *
 * Implement the sdtable component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <xmlcasa/singledish/sdtable_cmpt.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>



using namespace std;
using namespace casa;

namespace casac {

sdtable::sdtable()
{
  itsSDMemTableWrapper = 0;
  itsLog = new LogIO();
}
  
sdtable::sdtable(const asap::SDMemTableWrapper& mt)
{
  int rstat=0;
  itsSDMemTableWrapper = new asap::SDMemTableWrapper(mt);
  itsLog = new LogIO();
}


sdtable::~sdtable()
{
  delete itsSDMemTableWrapper;
  delete itsLog;
}

casac::sdtable*
sdtable::copy(const std::string& tosdtable)
{

    // TODO : IMPLEMENT ME HERE !
  casac::sdtable *rstat(0);
  try {
    asap::SDMemTableWrapper mycopy;
    mycopy = itsSDMemTableWrapper->copy();
    rstat = new casac::sdtable(mycopy);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::open(const std::string& insdtable)
{

    // TODO : IMPLEMENT ME HERE !
  Bool rstat(False);
  try {
    asap::SDMemTableWrapper mycopy(insdtable);
    itsSDMemTableWrapper = new asap::SDMemTableWrapper(mycopy);
    rstat = True;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::close()
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      delete itsSDMemTableWrapper;
      itsSDMemTableWrapper = 0;
      rstat = True;
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

int
sdtable::getif()
{

  int rstat=0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getIF();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;

}

int
sdtable::getbeam()
{

  int rstat=0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getBeam();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

int
sdtable::getpol()
{

  int rstat=0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getPol();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::string
sdtable::lines()
{

  string rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->spectralLines();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

int
sdtable::nif()
{

  int rstat=0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->nIF();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

int
sdtable::nbeam()
{

  int rstat=0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->nBeam();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

int
sdtable::npol()
{
  int rstat=0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->nPol();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

int
sdtable::nchan()
{

  int rstat=0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->nChan();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

int
sdtable::nrow()
{

  int rstat=0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->nRow();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdtable::setif(const int whichif)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->setIF(whichif);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdtable::setbeam(const int whichbeam)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->setBeam(whichbeam);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::setpol(const int whichpol)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->setPol(whichpol);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::string
sdtable::getfluxunit()
{
  string rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getFluxUnit();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::setfluxunit(const std::string& unit)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      itsSDMemTableWrapper->setFluxUnit(unit);
      rstat = True;
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::setinstrument(const std::string& instrument)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      itsSDMemTableWrapper->setInstrument(instrument);
      rstat = True;
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

casac::sdtable*
sdtable::getscan(const std::vector<int>& scan)
{
  casac::sdtable *rstat(0);
  try {
    if(itsSDMemTableWrapper) {
      asap::SDMemTableWrapper mycopy;
      mycopy = itsSDMemTableWrapper->getScan(scan);
      rstat = new casac::sdtable(mycopy);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

casac::sdtable*
sdtable::getsource(const std::string& source)
{
  casac::sdtable *rstat(0);
  try {
    asap::SDMemTableWrapper mycopy;
    mycopy = itsSDMemTableWrapper->getSource(source);
    rstat = new casac::sdtable(mycopy);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<double>
sdtable::getspectrum(const int whichRow)
{
  std::vector<double> rstat;
  std::vector<float> tempSpec; 
  try {
    if(itsSDMemTableWrapper) {
      tempSpec = itsSDMemTableWrapper->getSpectrum(whichRow);
      rstat.resize(tempSpec.size());
      for (uInt i = 0; i<tempSpec.size();i++) {
        rstat[i] = (double)tempSpec[i];
      } 
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

int
sdtable::nstokes()
{
  int rstat=0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->nStokes();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<double>
sdtable::getstokesspectrum(const int whichRow, const bool doPol)
{
  std::vector<double> rstat;
  try {
    if(itsSDMemTableWrapper) {
      std::vector<float> tempfv;
      tempfv = itsSDMemTableWrapper->getStokesSpectrum(whichRow, doPol);
      rstat.resize(tempfv.size());
      for (uInt i = 0; i<tempfv.size();i++) {
        rstat[i] = (double)tempfv[i];
      } 
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<double>
sdtable::stokestopolspectrum(const int whichRow, const bool tolinear, const int thepol)
{
  std::vector<double> rstat;
  try {
    if(itsSDMemTableWrapper) {
      std::vector<float> tempfv;
      tempfv = itsSDMemTableWrapper->stokesToPolSpectrum(whichRow, tolinear, thepol);
      for (uInt i = 0; i<tempfv.size();i++) {
        rstat[i] = (double)tempfv[i];
      } 
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::string
sdtable::getpolarizationlabel(const bool linear, const bool stokes, const bool linpol)
{
  std::string rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getPolarizationLabel(linear, stokes, linpol);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::setspectrum(const std::vector<double>& spectrum, const int whichRow)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      std::vector<float> fspectrum;
      fspectrum.resize(spectrum.size());
      for (uInt i = 0; i<spectrum.size();i++) {
        fspectrum[i] = (float)spectrum[i];
      } 
      itsSDMemTableWrapper->setSpectrum(fspectrum, whichRow);
      rstat = True; 
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<double>
sdtable::getabcissa(const int whichRow)
{
  std::vector<double> rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getAbcissa(whichRow);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::string
sdtable::getabcissalabel(const int whichRow)
{
  std::string rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getAbcissaString(whichRow);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<bool>
sdtable::getmask(const int whichRow)
{
  std::vector<bool> rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getMask(whichRow);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<double>
sdtable::gettsys()
{
  std::vector<double> rstat;
  try {
    if(itsSDMemTableWrapper) {
      std::vector<float> tempfv;
      tempfv = itsSDMemTableWrapper->getTsys();
      rstat.resize(tempfv.size());
      for (uInt i = 0; i<tempfv.size();i++) {
        rstat[i] = (double)tempfv[i];
      } 
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::string
sdtable::getsourcename(const int whichRow)
{
  std::string rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getSourceName(whichRow);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

double
sdtable::getelevation(const int whichRow)
{
  double rstat = 0.0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getElevation(whichRow);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

double
sdtable::getazimuth(const int whichRow)
{
  double rstat = 0.0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getAzimuth(whichRow);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

double
sdtable::getparangle(const int whichRow)
{
  double rstat = 0.0;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getParAngle(whichRow);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::string
sdtable::gettime(const int whichRow)
{
  std::string rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getTime(whichRow);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::flag(const int whichRow)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      itsSDMemTableWrapper->flag(whichRow);
      rstat = True; 
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::save(const std::string& filename)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      itsSDMemTableWrapper->makePersistent(filename);
      rstat = True; 
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

//std::string
bool
sdtable::summary(const bool verbose)
{
  Bool rstat(False);
  std::string summaryresults; 
  try {
    if(itsSDMemTableWrapper) {
      summaryresults = itsSDMemTableWrapper->summary(verbose);
      rstat = True;
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
 *itsLog <<  summaryresults <<LogIO::POST;
  return rstat;
}

std::vector<double>
sdtable::getrestfreqs()
{
  std::vector<double> rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getRestFreqs();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::setrestfreqs(const std::vector<double>& restfreqs, const std::string& unit, const std::vector<std::string>& lines, const std::string& source, const int whichif)
{
  bool rstat(false);
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->setRestFreqs(restfreqs, unit, lines, source, whichif);
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::setcoordinfo(const std::vector<std::string>& theinfo)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      itsSDMemTableWrapper->setCoordInfo(theinfo);
      rstat = True;
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<std::string>
sdtable::getcoordinfo()
{
  std::vector<std::string> rstat;
  try {
    if(itsSDMemTableWrapper) {
      itsSDMemTableWrapper->getCoordInfo();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<std::string>
sdtable::gethistory()
{
  std::vector<std::string> rstat;
  try {
    if(itsSDMemTableWrapper) {
      rstat = itsSDMemTableWrapper->getHistory();
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::addhistory(const std::string& theinfo)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      itsSDMemTableWrapper->addHistory(theinfo);
      rstat = True;
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::addfit(const int whichRow, const std::vector<double>& p, const std::vector<bool>& m, const std::vector<std::string>& f, const std::vector<int>& c)
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      itsSDMemTableWrapper->addFit(whichRow, p, m, f, c);
      rstat = True;
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

bool
sdtable::recalc_azel()
{
  Bool rstat(False);
  try {
    if(itsSDMemTableWrapper) {
      itsSDMemTableWrapper->calculateAZEL();
      rstat = True;
    } else {
       *itsLog << LogIO::WARN << "No sd table specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

} // casac namespace

