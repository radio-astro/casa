/***
 * Framework independent implementation file for flag...
 *
 * Implement the flag component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <lightflagger_cmpt.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <flagging/Flagging/LightFlagger.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/sstream.h>
#include <unistd.h>
#include <stdcasa/StdCasa/CasacSupport.h>


using namespace std;
using namespace casa;

namespace casac {

lightflagger::lightflagger()
{
    try
    {
      logger_p = new LogIO(LogOrigin("lightflagger","",WHERE));
      lightflagger_p = new LightFlagger();
	
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }

}

lightflagger::~lightflagger()
{
    try
    {
        done();
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}

bool
lightflagger::open(const std::string& msname)
{
    try
    {
	if ( !lightflagger_p ) {
	    lightflagger_p = new LightFlagger();
	}
        return lightflagger_p->attach(String(msname));
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}

bool
lightflagger::setdata(
    const std::string& field, 
    const std::string& spw, 
    const std::string& array, 
    const std::string& feed, 
    const std::string& scan, 
    const std::string& baseline, 
    const std::string& uvrange, 
    const std::string& time, 
    const std::string& correlation)
{
    try {
	if (lightflagger_p) {
	    return lightflagger_p->setdata(
		String(field),String(spw),String(array),
		String(feed),String(scan),String(baseline),
		String(uvrange),String(time),String(correlation));
        }
	return false;
    } catch (AipsError x) {
	*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}


::casac::record*
lightflagger::getparameters(const std::string& algorithm)
{
    try
    {
      casac::record* rstat(0);
	if(lightflagger_p)
        {
	  Record defparams = lightflagger_p->getparameters(String(algorithm));
          rstat = fromRecord(defparams);
        }
        return rstat;
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}

bool
lightflagger::setparameters(const std::string& algorithm, const ::casac::record& parameters)
{
    try
    {
        Record params = *toRecord(parameters);
	if(lightflagger_p)
        {
	  return lightflagger_p->setparameters(algorithm,params);
        }
        return false;
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}

  ::casac::record*
lightflagger::run(const ::casac::record& parameters)
{
    try {
      casac::record* rstat(0);
        if(lightflagger_p){
          Record params = *toRecord(parameters);
	  Record flagstats = lightflagger_p->run(params);
          rstat = fromRecord(flagstats);
        }
        return rstat;
    } catch (AipsError x) {
        *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
        RETHROW(x);
    }
}

bool
lightflagger::done()
{
    try
    {
	if(lightflagger_p) 
        {
	        delete lightflagger_p;
                lightflagger_p = NULL;
        }
        return true;
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}

bool
lightflagger::saveflagversion(const std::string& versionname, const std::string& comment, const std::string& merge)
{
    try
    {
        if( lightflagger_p )
        {
                return lightflagger_p->saveFlagVersion(String(versionname), String(comment),String(merge));
        }
        return False;
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}

bool
lightflagger::restoreflagversion(const std::vector<std::string>& versionname, const std::string& merge)
{
    try
    {
        if( lightflagger_p )
        {	
                Vector<String> verlist;
		verlist.resize(0);
		verlist = toVectorString(versionname);
                return lightflagger_p->restoreFlagVersion(verlist, String(merge));
        }
        return False;
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}

bool
lightflagger::deleteflagversion(const std::vector<std::string>& versionname)
{
    try
    {
        if( lightflagger_p )
        {
                Vector<String> verlist;
		verlist.resize(0);
		verlist = toVectorString(versionname);
                return lightflagger_p->deleteFlagVersion(verlist);
        }
        return False;
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}

std::vector<std::string>
lightflagger::getflagversionlist(const bool printflags)
{
    try
    {
        std::vector<std::string> result;

        if( lightflagger_p )
        {
                Vector<String> versionlist(0);
		lightflagger_p->getFlagVersionList(versionlist);
		for(uInt i = 0; i < versionlist.nelements(); i++) {
		    if (printflags) *logger_p << versionlist[i] << LogIO::POST;
		    result.push_back(versionlist[i]);
		}
		
        }
        return result;
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}


} // casac namespace

