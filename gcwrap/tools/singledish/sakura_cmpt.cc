/***
 * Tool bindings of sakura module
 *
 * @author kana,kawakami,
 * @version 
 ***/
#include <sakura_cmpt.h>
#include <casa_sakura/SakuraUtils.h>
#include <string>
#include <iostream>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Exceptions/Error.h>
#include <stdcasa/StdCasa/CasacSupport.h>
#include <casa/namespace.h>

using namespace std;

#define _ORIGIN LogOrigin("sakura", __func__, WHERE)

namespace casac {

sakura::sakura() : logger_(),sakuraUtils_()
{
  try {
	  logger_ << _ORIGIN;
  } catch (AipsError x) {
	  logger_ << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
  }
}

sakura::~sakura()
{
  try {
	  logger_ << _ORIGIN;
  } catch (AipsError x) {
	  logger_ << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
  }
}

void
sakura::welcome() {
  try {
	  logger_ << _ORIGIN;
	  logger_ << "Welcome to sakura " << LogIO::POST;
  } catch (AipsError x) {
	  logger_ << LogIO::SEVERE << "Exception Reported: " << x.getMesg()<< LogIO::POST;
	  RETHROW(x);
  }
}

bool
sakura::initialize_sakura(const string &loglevel) {
  try {
	  logger_ << _ORIGIN;
	  if(sakuraUtils_.IsSakuraInitialized()){
		  logger_ << "sakura is already initialized. No need to initialize." << LogIO::POST;
		  return true;
	  }
	  string loglevel_for_init("");
	  if(!loglevel.empty()) {
		  if (loglevel == "ERROR"){
		  	  loglevel_for_init = "SEVERE";
		  }else if (loglevel == "" || loglevel == "WARN"){
			  loglevel_for_init = "WARN";
		  }else if (loglevel == "INFO"){
			  loglevel_for_init = loglevel;
		  }else if (loglevel == "DEBUG"){
			  loglevel_for_init = loglevel;
		  }else{
			  logger_ << loglevel << "  is undefined. Available log level is ERROR|WARN|INFO|DEBUG" << LogIO::POST;
			  return false;
		  }
		  logger_ << "expected log level == " << loglevel << LogIO::POST;
	  }
	  if(sakuraUtils_.InitializeSakura(loglevel_for_init)){
		  logger_ << "initialize_sakura is called. sakura was initialized successfully. " << LogIO::POST;
		  return true;
	  }
  } catch (AipsError x) {
	  logger_ << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	  RETHROW(x);
      return false;
  }
  return false;
}

void
sakura::cleanup_sakura() {
  try {
	  logger_ << _ORIGIN;
	  if(sakuraUtils_.IsSakuraInitialized()){
		  sakuraUtils_.CleanUpSakura();
    	  logger_ << "cleanup_sakura is called "<< LogIO::POST;
	  }else{
		  logger_ << "No need to cleanup. sakura is not initialized yet."<< LogIO::POST;
	  }
  } catch (AipsError x) {
    logger_ << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
}

}  // casac namespace
