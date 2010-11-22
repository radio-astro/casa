

/***
 * Framework independent implementation file for sdlinefinder...
 *
 * Implement the sdlinefinder component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author Tak Tsutsumi
 * @version 
 ***/

#include <iostream>
#include <sdlinefinder_cmpt.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>


using namespace std;
using namespace casa;

namespace casac {

sdlinefinder::sdlinefinder()
{
  itsSDLineFinder = new asap::SDLineFinder();
  itsLog = new LogIO();
}

sdlinefinder::~sdlinefinder()
{
  delete itsSDLineFinder;
  delete itsLog; 
}

bool
sdlinefinder::setoptions(const double threshold, const int minnchan, const int avglimit, const double boxsize)
{

    // TODO : IMPLEMENT ME HERE !
  Bool  rstat(False);
  try {
     itsSDLineFinder->setOptions(threshold, minnchan, avglimit, boxsize);
     rstat = True; 
  } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;

}


//Need to handle sdtable as an argument
void
sdlinefinder::setscan(const ::casac::record& scan, const std::vector<bool>& mask, const std::vector<int>& edge)
{

    // TODO : IMPLEMENT ME HERE !
}


int
sdlinefinder::findlines(const int whichRow)
{

    // TODO : IMPLEMENT ME HERE !
  int rstat=0;
  try {
    rstat = itsSDLineFinder->findLines(whichRow);
  } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<bool>
sdlinefinder::getmask(const bool invert)
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<bool> rstat(false);
  try {
    rstat = itsSDLineFinder->getMask(invert);
  } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<double>
sdlinefinder::getlineranges()
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<double> rstat;
  try {
    rstat = itsSDLineFinder->getLineRanges();
  } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<int>
sdlinefinder::getlinerangesinchannels()
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<int> rstat;
  try {
    rstat = itsSDLineFinder->getLineRangesInChannels();
  } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;

}

} // casac namespace

