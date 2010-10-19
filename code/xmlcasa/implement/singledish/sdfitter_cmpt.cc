
/***
 * Framework independent implementation file for sdfitter...
 *
 * Implement the sdfitter component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <sdfitter_cmpt.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>


using namespace std;
using namespace casa;

namespace casac {

sdfitter::sdfitter()
{
  itsSDFitter = new asap::SDFitter();
  itsLog = new LogIO();
}

sdfitter::~sdfitter()
{
  delete itsSDFitter;
  delete itsLog;
}

bool
sdfitter::setdata(const std::vector<double>& xdat, const std::vector<double>& ydat, const std::vector<bool>& mask)
{

    // TODO : IMPLEMENT ME HERE !
  Bool rstat(False);
  try {
    std::vector<float> fxdat;
    std::vector<float> fydat;
    fxdat.resize(xdat.size());
    fydat.resize(ydat.size());
    for (uint i = 0; i <xdat.size();i++) { fxdat[i] = (float)xdat[i]; }
    for (uint i = 0; i <ydat.size();i++) { fydat[i] = (float)ydat[i]; }
    rstat = itsSDFitter->setData(fxdat, fydat, mask);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdfitter::setexpression(const std::string& expr, const int ncomp)
{

    // TODO : IMPLEMENT ME HERE !
  Bool rstat(False);
  try {
    rstat = itsSDFitter->setExpression(expr, ncomp);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}


std::vector<double>
sdfitter::getresidual()
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<double> rstat;
  try {
    std::vector<float> tempRes;
    tempRes = itsSDFitter->getResidual();
    rstat.resize(tempRes.size());
    for (uint i = 0; i<tempRes.size();i++) { rstat[i]=tempRes[i];}
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

std::vector<double>
sdfitter::getfit()
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<double> rstat;
  try {
    std::vector<float> tempFit;
    tempFit = itsSDFitter->getResidual();
    rstat.resize(tempFit.size());
    for (uint i = 0; i<tempFit.size();i++) { rstat[i]=tempFit[i];}
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

std::vector<bool>
sdfitter::getfixedparameters()
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<bool> rstat;
  try {
    rstat = itsSDFitter->getFixedParameters();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdfitter::setfixedparameters(const std::vector<bool>& fxd)
{

    // TODO : IMPLEMENT ME HERE !
  Bool rstat(False);
  try {
    rstat = itsSDFitter->setFixedParameters(fxd);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

std::vector<double>
sdfitter::getparameters()
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<double> rstat;
  try {
    std::vector<float> tempParam;
    tempParam = itsSDFitter->getParameters();
    rstat.resize(tempParam.size());
    for (uint i = 0; i<tempParam.size();i++) { rstat[i]=tempParam[i];}
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdfitter::setparameters(const std::vector<double>& params)
{

    // TODO : IMPLEMENT ME HERE !
  Bool rstat(False);
  try {
    std::vector<float> fparams;
    fparams.resize(params.size());
    for (uint i=0; i < params.size(); i++) {fparams[i] = (float)params[i];}
    rstat = itsSDFitter->setParameters(fparams);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

std::vector<double>
sdfitter::getestimate()
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<double> rstat;
  try {
    std::vector<float> frstat;
    frstat = itsSDFitter->getEstimate();
    rstat.resize(frstat.size());
    for (uint i=0; i < frstat.size(); i++) {rstat[i] = (double)frstat[i];}
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdfitter::estimate()
{

    // TODO : IMPLEMENT ME HERE !
  Bool rstat(False);
  try {
    rstat = itsSDFitter->computeEstimate();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

std::vector<double>
sdfitter::geterrors()
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<double> rstat;
  try {
    std::vector<float> frstat;
    frstat = itsSDFitter->getErrors();
    rstat.resize(frstat.size());
    for (uint i=0; i < frstat.size(); i++) {rstat[i] = (double)frstat[i];}
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

double
sdfitter::getchi2()
{

    // TODO : IMPLEMENT ME HERE !
  double rstat=0.0;
  try {
    rstat = itsSDFitter->getChisquared();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdfitter::reset()
{

    // TODO : IMPLEMENT ME HERE !
  Bool rstat(False);
  try {
    itsSDFitter->reset();
    rstat = True;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdfitter::fit()
{

    // TODO : IMPLEMENT ME HERE !
  Bool rstat(False);
  try {
    itsSDFitter->fit();
    rstat = True;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

std::vector<double>
sdfitter::evaluate(const int whichComp)
{

    // TODO : IMPLEMENT ME HERE !
  std::vector<double> rstat;
  try {
    std::vector<float> frstat;
    frstat = itsSDFitter->evaluate(whichComp);
    rstat.resize(frstat.size());
    for (uint i=0; i < frstat.size(); i++) {rstat[i] = (double)frstat[i];}
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

} // casac namespace

