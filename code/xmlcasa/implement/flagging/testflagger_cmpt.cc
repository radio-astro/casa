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
#include <testflagger_cmpt.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Exceptions/Error.h>
#include <flagging/Flagging/TestFlagger.h>
#include <flagging/Flagging/RFCommon.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Containers/Record.h>
#include <casa/sstream.h>
#include <unistd.h>
#include <xmlcasa/StdCasa/CasacSupport.h>


using namespace std;
using namespace casa;

namespace casac {

testflagger::testflagger()
{
	try
	{
		logger_p = new LogIO(LogOrigin("testflagger","",WHERE));
		testflagger_p = new TestFlagger();

	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}

}

testflagger::~testflagger()
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
testflagger::done()
{
	try
	{
		if (testflagger_p) {
			delete testflagger_p;
			testflagger_p = NULL;
		}
		if (logger_p){
			delete logger_p;
			logger_p = NULL;
		}

		return true;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}

}

bool
testflagger::open(const std::string& msname, const double ntime, bool combinescans)
{
	try
	{
		if ( !testflagger_p ) {
		    testflagger_p = new TestFlagger();
		}
		if (testflagger_p) {
			return testflagger_p->open(msname, ntime, combinescans);
		}

		return false;
	} catch(AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}

}


bool
testflagger::selectdata(
	const ::casac::record& selconfig,
    const std::string& field,
    const std::string& spw,
    const std::string& array,
    const std::string& feed,
    const std::string& scan,
    const std::string& antenna,
    const std::string& uvrange,
    const std::string& timerange,
    const std::string& correlation,
    const std::string& intent,
    const std::string& observation)
{

	try {

	if (testflagger_p) {

		if (! selconfig.empty()) {
			Record config = *toRecord(selconfig);
			// Select based on the record
			return testflagger_p->selectData(config);
		}
		else {

			// Select based on the parameters
		    return testflagger_p->selectData(
			String(field),String(spw),String(array),
			String(feed),String(scan),String(antenna),
			String(uvrange),String(timerange),String(correlation),
	        String(intent), String(observation));
		}
    }

	return false;
    } catch (AipsError x) {
    	*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    	RETHROW(x);
    }
}

bool
testflagger::parseDataSelection(const ::casac::record& selconfig)
{
	try
	{
		Record config = *toRecord(selconfig);
		if (testflagger_p) {
			return testflagger_p->parseDataSelection(config);
		}

		return false;
	} catch(AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}


bool
testflagger::parseAgentParameters(const ::casac::record& aparams)
{
	try
	{

		Record agent_params = *toRecord(aparams);

		if(testflagger_p){
			return testflagger_p->parseAgentParameters(agent_params);
		}

		return false;
	} catch(AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

bool
testflagger::init()
{
	try
	{
		if(testflagger_p){
				return testflagger_p->initAgents();
//			}
		}

		return false;
	} catch(AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
testflagger::run(bool writeflags)
{
    casac::record *rstat(0);
	try
	{
		if(testflagger_p){
			rstat =  fromRecord(testflagger_p->run(writeflags));
		}
		else{
			rstat = fromRecord(Record());
		}

		return rstat;
	} catch(AipsError x){
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}


/*
 * Methods to deal with flag backup
 */
std::vector<std::string>
testflagger::getflagversionlist(const bool printflags)
{
    try
    {
        std::vector<std::string> result;

        if(testflagger_p)
        {
        	Vector<String> versionlist(0);
        	testflagger_p->getFlagVersionList(versionlist);
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

bool
testflagger::printflagselection()
{
	try
	{
		if(testflagger_p)
		{
			return testflagger_p->printFlagSelections();
		}
		return false;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

bool
testflagger::saveflagversion(const std::string& versionname, const std::string& comment,
							 const std::string& merge)
{
	try
	{
		if(testflagger_p)
		{
			return testflagger_p->saveFlagVersion(String(versionname), String(comment),String(merge));
		}
		return False;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}


} // casac namespace

