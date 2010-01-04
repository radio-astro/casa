
/***
 * Framework independent implementation file for sdfit...
 *
 * Implementation of the sdfit component.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author Tak Tsutsumi
 * @version 
 ***/

#include <iostream>
#include <xmlcasa/singledish/sdfit_cmpt.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>


using namespace std;
using namespace casa;


namespace casac {

/* default constructor */
sdfit::sdfit()
{
  itsSDFitTable = new asap::SDFitTable();
  itsLog = new LogIO();

}

sdfit::sdfit(const asap::SDFitTable& fittab)
{
  itsSDFitTable = new asap::SDFitTable(fittab);
  itsLog = new LogIO();

}


sdfit::~sdfit()
{
  delete itsSDFitTable;
  delete itsLog;

}

::casac::sdfit *
sdfit::open(const std::string& tablename)
{

    // TODO : IMPLEMENT ME HERE !
    // Right now this is doing nothing
    // Is this class for accessing stored fitted parameters??
  casac::sdfit *rstat(0);
  try {
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat; 
}


int
sdfit::__len__(const int nfits_)
{
  int rstat = 0;
  try {
    rstat = itsSDFitTable->STLlength();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat; 
}

std::vector<bool>
sdfit::getfixedparameters(const int which)
{

    // TODO :  
  std::vector<bool> rstat(False);
  try {
    rstat = itsSDFitTable->getSTLParameterMask(which);
  } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<double>
sdfit::getparameters(const int which)
{

  std::vector<double> rstat;
  try {
    rstat = itsSDFitTable->getSTLParameters(which);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<std::string>
sdfit::getfunctions(const int which)
{

  std::vector<std::string> rstat;
  try {
    rstat = itsSDFitTable->getSTLFunctions(which);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<int>
sdfit::getcomponents(const int which)
{
  std::vector<int> rstat;
  try {
    rstat = itsSDFitTable->getSTLComponents(which);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<std::string>
sdfit::getframeinfo(const int which)
{

  std::vector<std::string> rstat;
  try {
    rstat = itsSDFitTable->getSTLFrameInfo(which);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

} // casac namespace
