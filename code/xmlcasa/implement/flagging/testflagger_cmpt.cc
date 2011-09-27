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
#include <ms/MeasurementSets/MeasurementSet.h>
#include <flagging/Flagging/TestFlagger.h>
#include <flagging/Flagging/RFCommon.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
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

void
testflagger::done()
{
	try
	{
		if (testflagger_p) {
			delete testflagger_p;
			testflagger_p = NULL;
		}

	} catch (AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}

}

bool
testflagger::configTestFlagger(const ::casac::record& tfconfig)
{
	try
	{
		Record config = *toRecord(tfconfig);
		if (testflagger_p) {
			return testflagger_p->configTestFlagger(config);
		}

		return false;
	} catch(AipsError x) {
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
			if (testflagger_p->initFlagDataHandler()) {
				return testflagger_p->initAgents();
			}
			return false;
		}

		return false;
	} catch(AipsError x) {
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

void
testflagger::run()
{
	try
	{
		if(testflagger_p){
			testflagger_p->run();
		}

		return;
	} catch(AipsError x){
		*logger_p << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}


} // casac namespace

