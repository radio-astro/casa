/***
 * Tool bindings of the test single dish module that work on an MS
 *
 * @author kana
 * @version 
 ***/
#include <singledishms_cmpt.h>
#include <string>
#include <iostream>

#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Exceptions/Error.h>

#include <singledish/SingleDish/SingleDishMS.h>

#include <casa/namespace.h> // using casa namespace

using namespace std;

#define _ORIGIN LogOrigin("singledishms", __func__, WHERE)

namespace casac {

singledishms::singledishms()
{
  itsSd = 0;
  itsLog = new LogIO();
}

singledishms::~singledishms()
{
  if (itsSd != 0) delete itsSd;
  delete itsLog;
}

bool
singledishms::open(string const& ms_name)
{
  bool rstat(false);
  *itsLog << _ORIGIN;
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
singledishms::close()
{
  bool rstat(false);
  *itsLog << _ORIGIN;
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
singledishms::done()
{
   return close();
}

void
singledishms::assert_valid_ms()
{
  if (itsSd == 0)
    throw(AipsError("No MeasurementSet has been assigned, please run open."));
}

Record 
singledishms::get_time_averaging_record(const bool& timeaverage,
					const string& timebin,
					const string& timespan)
{
      Record average_param;
      average_param.define("timeaverage", timeaverage);
      if (timeaverage) {
	String average_string;
	average_string = toCasaString(timebin);
	if (average_string != "") {
	  average_param.define("timebin", average_string);
	}
	average_string = toCasaString(timespan);
	if (average_string != "") {
	  average_param.define("timespan", average_string);
	}
      }
      return average_param;
}

string
singledishms::name()
{
  *itsLog << _ORIGIN;
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
singledishms::_scale(float const factor, string const& datacolumn, string const& outfile, bool const timeaverage, string const& timebin, string const& timespan)
{
  bool rstat(false);
  *itsLog << _ORIGIN;
  try {
    assert_valid_ms();
    if (timeaverage) {
      Record average_param = get_time_averaging_record(timeaverage,timebin,
						       timespan);
      itsSd->setAverage(average_param);
    }

    itsSd->scale(factor, datacolumn, outfile);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
singledishms::subtract_baseline(string const& datacolumn,
				string const& outfile,
				string const& bltable,
				string const& blformat,
				string const& bloutput,
				bool const dosubtract,
				::casac::variant const& spw,
				::casac::variant const& pol,
				string const& blfunc,
				int const order,
				float const clip_threshold_sigma,
				int const num_fitting_max,
				bool const linefinding,
				float const threshold,
				int const avg_limit,
				int const minwidth,
				vector<int> const& edge)
{
  bool rstat(false);
  *itsLog << _ORIGIN;
  try {
    assert_valid_ms();
    itsSd->subtractBaseline(datacolumn, outfile, bltable, 
                 blformat, 
                 bloutput, 
                 dosubtract,
			    toCasaString(spw), toCasaString(pol),
			    blfunc, order, clip_threshold_sigma, 
			    num_fitting_max, linefinding, threshold,
			    avg_limit, minwidth, edge);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
singledishms::subtract_baseline_cspline(string const& datacolumn,
				string const& outfile,
				string const& bltable,
				string const& blformat,
				string const& bloutput,
				bool const dosubtract,
				::casac::variant const& spw,
				::casac::variant const& pol,
				int const npiece,
				float const clip_threshold_sigma,
				int const num_fitting_max,
				bool const linefinding,
				float const threshold,
				int const avg_limit,
				int const minwidth,
				vector<int> const& edge)
{


  bool rstat(false);
  *itsLog << _ORIGIN;
  try {
    assert_valid_ms();
    itsSd->subtractBaselineCspline(datacolumn, outfile, bltable,
                     blformat, 
                     bloutput, 
				   dosubtract, toCasaString(spw), 
				   toCasaString(pol), npiece, 
				   clip_threshold_sigma, num_fitting_max, 
				   linefinding, threshold,
				   avg_limit, minwidth, edge);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

/*
bool
singledishms::subtract_baseline_sinusoid(string const& datacolumn,
				string const& outfile,
				string const& bltable,
				string const& blformat,
				string const& bloutput,
				bool const dosubtract,
				::casac::variant const& spw,
				::casac::variant const& pol,
				int const npiece,
				float const clip_threshold_sigma,
				int const num_fitting_max)
{
  bool rstat(false);
  *itsLog << _ORIGIN;
  try {
    assert_valid_ms();
    itsSd->subtractBaselineSinusoid(datacolumn, outfile, bltable,
                     blformat, 
                     bloutput, 
				     dosubtract, toCasaString(spw), 
				     toCasaString(pol), npiece, 
				     clip_threshold_sigma, num_fitting_max);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

*/

bool
singledishms::subtract_baseline_variable(string const& datacolumn,
					 string const& outfile,
					 string const& bltable,
					 string const& blformat,
					 string const& bloutput,
					 bool const dosubtract,
					 ::casac::variant const& spw,
					 ::casac::variant const& pol,
					 string const& blparam)
{
  bool rstat(false);
  *itsLog << _ORIGIN;
  try {
    assert_valid_ms();
    itsSd->subtractBaselineVariable(datacolumn, outfile, bltable, 
                      blformat, 
                      bloutput, 
				      dosubtract, toCasaString(spw),
				      toCasaString(pol), blparam);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
singledishms::apply_baseline_table(string const& bltable,
				   string const& datacolumn,
				   ::casac::variant const& spw,
				   string const& outfile)
{
  bool rstat(false);
  *itsLog << _ORIGIN;
  try {
    assert_valid_ms();
    itsSd->applyBaselineTable(datacolumn, bltable, toCasaString(spw), outfile);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
singledishms::fit_line(string const& datacolumn,
		       ::casac::variant const& spw,
		       ::casac::variant const& pol,
		       string const& fitfunc,
		       string const& nfit,
		       string const& tempfile,
		       string const& tempoutfile)
{
  bool rstat(false);
  *itsLog << _ORIGIN;
  try {
    assert_valid_ms();
    itsSd->fitLine(datacolumn, toCasaString(spw), toCasaString(pol), 
		   fitfunc, nfit, tempfile, tempoutfile);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
singledishms::set_selection(::casac::variant const& spw,
		    ::casac::variant const& field, 
		    ::casac::variant const& antenna, 
		    ::casac::variant const& timerange, 
		    ::casac::variant const& scan,
		    ::casac::variant const& observation,
		    ::casac::variant const& polarization,
		    ::casac::variant const& beam,
		    ::casac::variant const& intent,
		    string const& taql)
{
  bool rstat(false);
  *itsLog << _ORIGIN;
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
    selection_string = toCasaString(antenna);
    if (selection_string != "")
      selection.define("baseline", selection_string);
    // time
    selection_string = toCasaString(timerange);
    if (selection_string != "")
      selection.define("timerange", selection_string);
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
      selection.define("correlation", selection_string);
    // beam
    selection_string = toCasaString(beam);
    if (selection_string != "")
      *itsLog << LogIO::WARN << "Beam selection is not yet supported. Ignoring beam selection" << LogIO::POST;
      //selection.define("beam", selection_string);
    // intent
    selection_string = toCasaString(intent);
    if (selection_string != "")
      selection.define("intent", selection_string);
    // taql
    selection_string = toCasaString(taql);
    if (selection_string != "")
      selection.define("taql", selection_string);

    itsSd->setSelection(selection);
    rstat = true;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
singledishms::smooth(string const &type, float const width,
        string const &datacolumn, string const &outfile)
{
    bool rstat(false);
    *itsLog << _ORIGIN;
    try {
      assert_valid_ms();
      itsSd->smooth(type, width, datacolumn, outfile);
      rstat = true;
    } catch  (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
          << LogIO::POST;
      RETHROW(x);
    }
    return rstat;
}

} // end of casac namespace
