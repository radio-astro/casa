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
#include <stdcasa/StdCasa/CasacSupport.h>
#include <tables/Tables/Table.h>


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
	    Table::relinquishAutoLocks(True);
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
    Table::relinquishAutoLocks(True);

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

		return true;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}

}

bool
testflagger::open(const std::string& msname, const double ntime)
{
	try
	{
		if ( !testflagger_p ) {
			testflagger_p = new TestFlagger();
		}
		if (testflagger_p) {
			return testflagger_p->open(msname, ntime);
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
testflagger::parseagentparameters(const ::casac::record& aparams)
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
		}

		return false;
	} catch(AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
 testflagger::run(bool writeflags, bool sequential)
{
	casac::record *rstat(0);
	try
	{
		if(testflagger_p){
			rstat =  fromRecord(testflagger_p->run(writeflags, sequential));
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

bool
testflagger::restoreflagversion(const std::vector<std::string>& versionname,
		const std::string& merge)
{
    try
    {
        if(testflagger_p)
        {
        	Vector<String> verlist;
        	verlist.resize(0);
        	verlist = toVectorString(versionname);
            return testflagger_p->restoreFlagVersion(verlist, String(merge));
        }
        return False;
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}

bool
testflagger::deleteflagversion(const std::vector<std::string>& versionname)
{
    try
    {
        if(testflagger_p)
        {
        	Vector<String> verlist;
        	verlist.resize(0);
        	verlist = toVectorString(versionname);
            return testflagger_p->deleteFlagVersion(verlist);
        }
        return False;
    } catch (AipsError x) {
            *logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
            RETHROW(x);
    }
}


bool
testflagger::parsemanualparameters(
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
		const std::string& observation,
		const bool autocorr,
		const bool apply)
{

	try {

		if (testflagger_p) {

			// Parse the manual parameters
			return testflagger_p->parseManualParameters(
					String(field),String(spw),String(array),
					String(feed),String(scan),String(antenna),
					String(uvrange),String(timerange),String(correlation),
					String(intent), String(observation), Bool(autocorr), Bool(apply));

		}

		return false;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

bool
testflagger::parseclipparameters(
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
		const std::string& observation,
		const std::string& datacolumn,
		const std::vector<double>& clipminmax,
		const bool clipoutside,
		const bool channelavg,
		const bool clipzeros,
		const bool apply)
{

	try {

		if (testflagger_p) {

			// Parse the clip parameters
			return testflagger_p->parseClipParameters(
					String(field),String(spw),String(array),
					String(feed),String(scan),String(antenna),
					String(uvrange),String(timerange),String(correlation),
					String(intent), String(observation),
					String(datacolumn), clipminmax, Bool(clipoutside),
					Bool(channelavg), Bool(clipzeros), Bool(apply));

		}

		return false;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

bool
testflagger::parsequackparameters(
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
		const std::string& observation,
		const std::string& quackmode,
		const double quackinterval,
		const bool quackincrement,
		const bool apply)
{

	try {

		if (testflagger_p) {

			// Parse the quack parameters
			return testflagger_p->parseQuackParameters(
					String(field),String(spw),String(array),
					String(feed),String(scan),String(antenna),
					String(uvrange),String(timerange),String(correlation),
					String(intent), String(observation), String(quackmode),
					quackinterval, Bool(quackincrement), Bool(apply));

		}

		return false;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

bool
testflagger::parseelevationparameters(
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
		const std::string& observation,
		const double lowerlimit,
		const double upperlimit,
		const bool apply)
{

	try {

		if (testflagger_p) {

			// Parse the elevation parameters
			return testflagger_p->parseElevationParameters(
					String(field),String(spw),String(array),
					String(feed),String(scan),String(antenna),
					String(uvrange),String(timerange),String(correlation),
					String(intent), String(observation), lowerlimit,
					upperlimit, Bool(apply));

		}

		return false;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

bool
testflagger::parsetfcropparameters(
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
		const std::string& observation,
		const double ntime,
		const bool combinescans,
		const std::string& datacolumn,
		const double timecutoff,
		const double freqcutoff,
		const std::string& timefit,
		const std::string& freqfit,
		const int maxnpieces,
		const std::string& flagdimension,
		const std::string& usewindowstats,
		const int halfwin,
		const bool apply)
{

	try {

		if (testflagger_p) {

			// Parse the tfcrop parameters
			return testflagger_p->parseTfcropParameters(
					String(field),String(spw),String(array),
					String(feed),String(scan),String(antenna),
					String(uvrange),String(timerange),String(correlation),
					String(intent), String(observation), ntime, Bool(combinescans),
		       	    String(datacolumn), timecutoff,
		       	    freqcutoff, String(timefit), String(freqfit), maxnpieces,
		       	    String(flagdimension), String(usewindowstats), halfwin, Bool(apply));

		}

		return false;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

bool
testflagger::parseextendparameters(
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
		const std::string& observation,
		const double ntime,
		const bool combinescans,
		const bool extendpols,
		const double growtime,
		const double growfreq,
		const bool growaround,
		const bool flagneartime,
		const bool flagnearfreq,
		const bool apply)
{

	try {

		if (testflagger_p) {

			// Parse the extend parameters
			return testflagger_p->parseExtendParameters(
					String(field),String(spw),String(array),
					String(feed),String(scan),String(antenna),
					String(uvrange),String(timerange),String(correlation),
					String(intent), String(observation), ntime, Bool(combinescans),
					Bool(extendpols), growtime, growfreq, Bool(growaround),
					Bool(flagneartime), Bool(flagnearfreq), Bool(apply));

		}

		return false;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}


bool
testflagger::parsesummaryparameters(
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
		const std::string& observation,
		const bool spwchan,
		const bool spwcorr,
		const bool basecnt)
{

	try {

		if (testflagger_p) {

			// Parse the summary parameters
			return testflagger_p->parseSummaryParameters(
					String(field),String(spw),String(array),
					String(feed),String(scan),String(antenna),
					String(uvrange),String(timerange),String(correlation),
					String(intent), String(observation), Bool(spwchan),
					Bool(spwcorr), Bool(basecnt));

		}

		return false;
	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}






} // casac namespace

