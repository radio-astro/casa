/***
 * Tool bindings of the test single dish module that work on an MS
 *
 * @author kana
 * @version 
 ***/
#include <sdms_cmpt.h>
#include <string>
#include <iostream>

#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Exceptions/Error.h>

#include <singledish/SingleDish/SingleDishMS.h>

#include <casa/namespace.h> // using casa namespace

using namespace std;

namespace casac {

sdms::sdms()
{
  itsSd = 0;
  itsLog = new LogIO();
}

sdms::~sdms()
{
  if (itsSd != 0) delete itsSd;
  delete itsLog;
}

bool
sdms::open(string const& ms_name)
{
  bool rstat(false);
  try {
    // In case already open, close it!
    close();
    // create instanse
    itsSd = new SingleDishMS(ms_name);
    if (itsSd != 0) rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
sdms::close()
{
  bool rstat(false);
  try {
    if(itsSd != 0) delete itsSd;
    itsSd = 0;
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
sdms::done()
{
   return close();
}

void
sdms::assert_valid_ms()
{
  if (itsSd == 0)
    throw(AipsError("No MeasurementSet has been assigned, please run open."));
}

string
sdms::name()
{
  try {
    assert_valid_ms();
    return itsSd->name();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "MS is not yet assigned." << LogIO::POST;
    RETHROW(x);
  }
  return "";
}

bool
sdms::scale(float const factor)
{
  bool rstat(false);
  try {
    assert_valid_ms();
    itsSd->scale(factor);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
sdms::set_selection(::casac::variant const& spw,
		    ::casac::variant const& field, 
		    ::casac::variant const& baseline, 
		    ::casac::variant const& time, 
		    ::casac::variant const& scan,
		    ::casac::variant const& observation,
		    ::casac::variant const& polarization,
		    ::casac::variant const& beam,
		    string const& taql)
{
  bool rstat(false);
  try {
    assert_valid_ms();

    // make selection string to record.
    // Doing this here to make future extention easier.
    Record selection;
    String selection_string;
    // spw
    selection_string = toCasaString(spw);
    if (selection_string != "") {
      selection.define("spw", selection_string);
    }
    // field
    selection_string = toCasaString(field);
    if (selection_string != "")
      selection.define("field", selection_string);
    // baseline
    selection_string = toCasaString(baseline);
    if (selection_string != "")
      selection.define("baseline", selection_string);
    // time
    selection_string = toCasaString(time);
    if (selection_string != "")
      selection.define("time", selection_string);
    // scan
    selection_string = toCasaString(scan);
    if (selection_string != "")
      selection.define("scan", selection_string);
    // observation
    selection_string = toCasaString(observation);
    if (selection_string != "")
      selection.define("observation", selection_string);
    // polarization
    selection_string = toCasaString(polarization);
    if (selection_string != "")
      selection.define("polarization", selection_string);
    // beam
    selection_string = toCasaString(beam);
    if (selection_string != "")
      selection.define("beam", selection_string);
    // taql
    selection_string = toCasaString(taql);
    if (selection_string != "")
      selection.define("taql", selection_string);

    itsSd->set_selection(selection);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

} // end of casac namespace
