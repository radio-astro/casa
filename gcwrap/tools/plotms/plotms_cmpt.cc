//# plotms_cmpt.cc: PlotMS component tool.
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $
#include <plotms_cmpt.h>

#include <stdcasa/StdCasa/CasacSupport.h>
#include <dbus_cmpt.h>

#include <unistd.h>

namespace casac {


////////////////////////
// PLOTMS DEFINITIONS //
////////////////////////

// Static //

const unsigned int plotms::LAUNCH_WAIT_INTERVAL_US = 1000;
const unsigned int plotms::LAUNCH_TOTAL_WAIT_US    = 5000000;


// Constructors/Destructors //

  plotms::plotms() : itsWatcher_(this),doIter_(True) {
	  showGui = true;
	  asyncCall = true;
  }

  plotms::~plotms() { closeApp(); }


// Public Methods //

// Convenience macros for setting/getting a single value using a record.
#define SETSINGLE(METHOD, PKEY, PVALUE, ASYNC_CALL)                                       \
    Record params;                                                            \
    params.define(PlotMSDBusApp::PARAM_##PKEY, PVALUE);                       \
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),          \
            PlotMSDBusApp::METHOD_##METHOD, params, /*true*/ASYNC_CALL);

#define SETSINGLEPLOT(PKEY, PVALUE, ASYNC_CALL)                                           \
    Record params;                                                            \
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);                 \
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately); \
    params.define(PlotMSDBusApp::PARAM_##PKEY, PVALUE);                       \
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),          \
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/ASYNC_CALL);

#define GETSINGLE(METHOD, PKEY, PTYPE, PASTYPE, PDEFVAL)                      \
    Record result;                                                            \
    QtDBusXmlApp::dbusXmlCall(dbus::FROM_NAME, app.dbusName( ),               \
            PlotMSDBusApp::METHOD_##METHOD, Record(), result);                \
    if(result.isDefined(PlotMSDBusApp::PARAM_##PKEY) &&                       \
       result.dataType(PlotMSDBusApp::PARAM_##PKEY) == PTYPE )                \
        return result. PASTYPE (PlotMSDBusApp::PARAM_##PKEY);                 \
    else return PDEFVAL;

#define GETSINGLEPLOT(PKEY, PTYPE, PASTYPE, PDEFVAL)                          \
    Record params;                                                            \
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);                 \
    Record result;                                                            \
    QtDBusXmlApp::dbusXmlCall(dbus::FROM_NAME, app.dbusName( ),               \
            PlotMSDBusApp::METHOD_GETPLOTPARAMS, params, result);             \
    if(result.isDefined(PlotMSDBusApp::PARAM_##PKEY) &&                       \
       result.dataType(PlotMSDBusApp::PARAM_##PKEY) == PTYPE )                \
        return result. PASTYPE (PlotMSDBusApp::PARAM_##PKEY);                 \
    else return PDEFVAL;

#define GETSINGLEPLOTREC(PKEY)                                                \
    Record params;                                                            \
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);                 \
    Record result;                                                            \
    QtDBusXmlApp::dbusXmlCall(dbus::FROM_NAME, app.dbusName( ),               \
            PlotMSDBusApp::METHOD_GETPLOTPARAMS, params, result);             \
    if(result.isDefined(PlotMSDBusApp::PARAM_##PKEY) &&                       \
       result.dataType(PlotMSDBusApp::PARAM_##PKEY) == TpRecord)              \
        return fromRecord(result.asRecord(PlotMSDBusApp::PARAM_##PKEY));      \
    else return new record();

#define GETSINGLEBOOL(METHOD, PKEY) GETSINGLE(METHOD,PKEY,TpBool,asBool,false)
#define GETSINGLESTR(METHOD, PKEY) GETSINGLE(METHOD,PKEY,TpString,asString,"")
#define GETSINGLEINT(METHOD, PKEY) GETSINGLE(METHOD, PKEY, TpInt, asInt, 0)

#define GETSINGLEPLOTSTR(PKEY) GETSINGLEPLOT(PKEY, TpString, asString, "")
#define GETSINGLEPLOTBOOL(PKEY) GETSINGLEPLOT(PKEY, TpBool,  asBool, false)
#define GETSINGLEPLOTINT(PKEY) GETSINGLEPLOT(PKEY, TpInt,  asInt, 0)


void plotms::setLogFilename(const std::string& logFilename) {
    if(app.dbusName( ).empty()) itsLogFilename_ = logFilename;
    else {
        SETSINGLE(SETLOGPARAMS, FILENAME, logFilename, asyncCall)
    }
}
string plotms::getLogFilename() {
    if(app.dbusName( ).empty()) return itsLogFilename_;
    else {
        GETSINGLESTR(GETLOGPARAMS, FILENAME)
    }
}

void plotms::setLogFilter(const std::string& priority) {
    if(app.dbusName( ).empty()) itsLogFilter_ = priority;
    else {
        SETSINGLE(SETLOGPARAMS, PRIORITY, priority, asyncCall )
    }
}
string plotms::getLogFilter() {
    if(app.dbusName( ).empty()) return itsLogFilter_;
    else {
        GETSINGLESTR(GETLOGPARAMS, PRIORITY)
    }
}

void plotms::setClearSelectionOnAxesChange(const bool clearSelection) {
    launchApp();
    SETSINGLE(SETPLOTMSPARAMS, CLEARSELECTIONS, clearSelection, asyncCall) }
bool plotms::getClearSelectionOnAxesChange() {
    launchApp();
    GETSINGLEBOOL(GETPLOTMSPARAMS, CLEARSELECTIONS) }

void plotms::setCachedImageSize(const int width, const int height) {
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_WIDTH, width);
    params.define(PlotMSDBusApp::PARAM_HEIGHT, height);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTMSPARAMS, params, /*true*/asyncCall);
}

void plotms::setCachedImageSizeToScreenResolution() {
    callAsync(PlotMSDBusApp::METHOD_SETCACHEDIMAGESIZETOSCREENRES);
}
int plotms::getCachedImageWidth() {
    launchApp();
    GETSINGLEINT(GETPLOTMSPARAMS, WIDTH) }
int plotms::getCachedImageHeight() {
    launchApp();
    GETSINGLEINT(GETPLOTMSPARAMS, HEIGHT) }


void plotms::setPlotMSFilename(
		const string& msFilename,
        const bool updateImmediately, const int plotIndex ) {
    launchApp();
    SETSINGLEPLOT(FILENAME, msFilename, asyncCall )
}

string plotms::getPlotMSFilename(const int plotIndex) {
    launchApp();
    GETSINGLEPLOTSTR(FILENAME) }

void plotms::setPlotMSSelection(const string& field, const string& spw,
        const string& timerange, const string& uvrange,
        const string& antenna, const string& scan, const string& corr,
	const string& array, const string& observation, const string& msselect,
        const bool updateImmediately, const int plotIndex) {
    PlotMSSelection sel;
    
    sel.setField(field);
    sel.setSpw(spw);
    sel.setTimerange(timerange);
    sel.setUvrange(uvrange);
    sel.setAntenna(antenna);
    sel.setScan(scan);
    sel.setCorr(corr);
    sel.setArray(array);
    sel.setObservation(observation);
    sel.setMsselect(msselect);
    
    setPlotMSSelection_(sel, updateImmediately, plotIndex);
}

void plotms::setPlotMSSelectionRec(const record& selection,
		const bool updateImmediately, const int plotIndex) {
    Record* sel1 = toRecord(selection);
    PlotMSSelection sel;
    sel.fromRecord(*sel1);
    delete sel1;
    
    setPlotMSSelection_(sel, updateImmediately, plotIndex);
}

record* plotms::getPlotMSSelection(const int plotIndex) {
    launchApp();
    GETSINGLEPLOTREC(SELECTION) }

void plotms::setPlotMSAveraging(const string& channel, const string& time,
				const bool scan, const bool field, 
				const bool baseline,
				const bool antenna, const bool spw, 
				const bool scalar,
				const bool updateImmediately,
				const int plotIndex) {
    PlotMSAveraging avg;
    
    avg.setChannel(channel);
    avg.setTime(time);
    avg.setScan(scan);
    avg.setField(field);
    avg.setBaseline(baseline);
    avg.setAntenna(antenna);
    avg.setSpw(spw);
    avg.setScalarAve(scalar);
    
    setPlotMSAveraging_(avg, updateImmediately, plotIndex);
}

void plotms::setPlotMSAveragingRec(const record& averaging,
		const bool updateImmediately, const int plotIndex) {
    Record* avg1 = toRecord(averaging);
    PlotMSAveraging avg;
    avg.fromRecord(*avg1);
    delete avg1;
    
    setPlotMSAveraging_(avg, updateImmediately, plotIndex);
}

record* plotms::getPlotMSAveraging(const int plotIndex) {
    launchApp();
    GETSINGLEPLOTREC(AVERAGING) }

void plotms::setPlotMSTransformations(const std::string& freqframe, 
				      const std::string& veldef, 
				      const ::casac::variant& restfreq, 
				      const double xshift, 
				      const double yshift, 
				      const bool updateImmediately, 
				      const int plotIndex) {
    PlotMSTransformations trans;
    
    trans.setFrame(freqframe);
    trans.setVelDef(veldef);
    casa::Quantity restFreq= casaQuantity(restfreq);
    trans.setRestFreq(restFreq);
    trans.setXpcOffset(xshift);
    trans.setYpcOffset(yshift);
    
    setPlotMSTransformations_(trans, updateImmediately, plotIndex);

}

void plotms::setPlotMSIterate(const std::string& iteraxis,
			      const bool xselfscale,
			      const bool yselfscale,
			      const bool updateImmediately, 
			      const int plotIndex) {

    PlotMSIterParam iter;
    
    iter.setIterAxis(iteraxis);
    if (iteraxis=="") {
      //iter.setXSelfScale(False);
      //iter.setYSelfScale(False);
      iter.setGlobalScaleX( False);
      iter.setGlobalScaleY( False);
    }
    else {
      //iter.setXSelfScale(xselfscale);
      //iter.setYSelfScale(yselfscale);
    	iter.setGlobalScaleX( xselfscale);
    	iter.setGlobalScaleY( yselfscale);
    }

    // Only if iteration enabled...
    if (doIter_)
      setPlotMSIterate_(iter, updateImmediately, plotIndex);
   
}

void plotms::setPlotMSTransformationsRec(const record& transformations, 
					 const bool updateImmediately, 
					 const int plotIndex) {
    Record* trans1 = toRecord(transformations);
    PlotMSTransformations trans;
    trans.fromRecord(*trans1);
    delete trans1;
    
    setPlotMSTransformations_(trans, updateImmediately, plotIndex);
}

record* plotms::getPlotMSTransformations(const int plotIndex) {
  launchApp();
  GETSINGLEPLOTREC(TRANSFORMATIONS) 
}



void plotms::setColorizeFlag(const bool colorize, const bool updateImmediately, const int plotIndex) 
{
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_COLORIZE, colorize);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
         PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}


bool plotms::getColorizeFlag(const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTBOOL(COLORIZE) 
}


void plotms::setColorAxis(const string&  coloraxis, const bool updateImmediately, const int plotIndex) 
{
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_COLORAXIS, coloraxis);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}

void plotms::setSymbol(const string& symbolshape, const int symbolsize,
                       const string& symbolcolor, const string& symbolfill,
                       const bool symboloutline, const bool updateImmediately,
                       const int plotIndex)
{
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_SYMBOL, true);
    params.define(PlotMSDBusApp::PARAM_SYMBOLSHAPE, symbolshape);
    params.define(PlotMSDBusApp::PARAM_SYMBOLSIZE, symbolsize);
    params.define(PlotMSDBusApp::PARAM_SYMBOLCOLOR, symbolcolor);
    params.define(PlotMSDBusApp::PARAM_SYMBOLFILL, symbolfill);
    params.define(PlotMSDBusApp::PARAM_SYMBOLOUTLINE, symboloutline);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName(),
        PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}

void plotms::setFlaggedSymbol(
    const string& flaggedsymbolshape, const int flaggedsymbolsize,
    const string& flaggedsymbolcolor, const string& flaggedsymbolfill,
    const bool flaggedsymboloutline, const bool updateImmediately,
    const int plotIndex)
{
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_FLAGGEDSYMBOL, true);
    params.define(PlotMSDBusApp::PARAM_FLAGGEDSYMBOLSHAPE, flaggedsymbolshape);
    params.define(PlotMSDBusApp::PARAM_FLAGGEDSYMBOLSIZE, flaggedsymbolsize);
    params.define(PlotMSDBusApp::PARAM_FLAGGEDSYMBOLCOLOR, flaggedsymbolcolor);
    params.define(PlotMSDBusApp::PARAM_FLAGGEDSYMBOLFILL, flaggedsymbolfill);
    params.define(PlotMSDBusApp::PARAM_FLAGGEDSYMBOLOUTLINE, flaggedsymboloutline);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName(),
        PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}

string plotms::getColorAxis(const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTSTR(COLORAXIS) 
}



void plotms::setTitle(const string& text,  const bool updateImmediately, const int plotIndex) 
{
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_CANVASTITLE,  text);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}


void plotms::setXAxisLabel(const string& text,  const bool updateImmediately, const int plotIndex) 
{
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_XAXISLABEL,  text);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}


void plotms::setYAxisLabel(const string& text,  const bool updateImmediately, const int plotIndex) 
{
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_YAXISLABEL,  text);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}


string plotms::getTitle(const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTSTR(CANVASTITLE) 
}


string plotms::getXAxisLabel(const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTSTR(XAXISLABEL) 
}


string plotms::getYAxisLabel(const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTSTR(YAXISLABEL) 
}




void plotms::setPlotXAxis(const string& xAxis, const string& xDataColumn,
                   const bool updateImmediately, const int plotIndex) 
{
    setPlotAxes(xAxis, xDataColumn, "", "", updateImmediately, plotIndex); 
}


void plotms::setPlotYAxis(const string& yAxis, const string& yDataColumn,
                   const bool updateImmediately, const int plotIndex) 
{
    setPlotAxes("", "", yAxis, yDataColumn, updateImmediately, plotIndex); 
}





void plotms::setPlotAxes(const string& xAxis, const string& yAxis,
        const string& xDataColumn, const string& yDataColumn,
        const bool updateImmediately, const int plotIndex) {
    launchApp();
    string xdc = xDataColumn, ydc = yDataColumn;
    if(xdc == "residual") xdc = "corrected-model";
    if(ydc == "residual") ydc = "corrected-model";
    Record params;
    if(!xAxis.empty()) params.define(PlotMSDBusApp::PARAM_AXIS_X, xAxis);
    if(!xdc.empty())
        params.define(PlotMSDBusApp::PARAM_DATACOLUMN_X, xdc);
    if(!yAxis.empty()) params.define(PlotMSDBusApp::PARAM_AXIS_Y, yAxis);
    if(!ydc.empty())
        params.define(PlotMSDBusApp::PARAM_DATACOLUMN_Y, ydc);
    if(params.nfields() == 0) return;
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}


string plotms::getPlotXAxis(const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTSTR(AXIS_X) 
}

string plotms::getPlotXDataColumn(const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTSTR(DATACOLUMN_X) 
}

string plotms::getPlotYAxis(const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTSTR(AXIS_Y) 
}

string plotms::getPlotYDataColumn(const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTSTR(DATACOLUMN_Y) 
}


record* plotms::getPlotParams(const int plotIndex) 
{
    launchApp();
    Record params, retValue;
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCall(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_GETPLOTPARAMS, params, retValue);
    return fromRecord(retValue);
}


void plotms::setFlagExtension(const bool extend, const string& correlation,
        const bool channel, const bool spw, const string& antenna,
        const bool time, const bool scans, const bool field,
        const record& alternateSelection) {
    PlotMSFlagging flag;
    
    flag.setExtend(extend);
    flag.setCorr(correlation);
    flag.setChannel(channel);
    flag.setSpw(spw);
    flag.setAntenna(antenna);
    flag.setTime(time);
    flag.setScans(scans);
    flag.setField(field);
    
    if(alternateSelection.size() > 0) {
        Record* sel1 = toRecord(alternateSelection);
        PlotMSSelection sel;
        sel.fromRecord(*sel1);
        delete sel1;
        flag.setSelectionAlternate(true);
        flag.setSelectionAlternateSelection(sel);
    } else flag.setSelectionSelected(true);
    
    setFlagging_(flag);
}

void plotms::setFlagExtensionRec(const record& flagExtension) {
    Record* flag1 = toRecord(flagExtension);
    PlotMSFlagging flag;
    flag.fromRecord(*flag1);
    delete flag1;
    
    setFlagging_(flag);
}

record* plotms::getFlagExtension() {
    launchApp();
    Record result;
    QtDBusXmlApp::dbusXmlCall(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_GETFLAGGING, Record(), result);
    return fromRecord(result);
}

record* plotms::locateInfo() {
    Record result;
    QtDBusXmlApp::dbusXmlCall(dbus::FROM_NAME, app.dbusName(),
                              PlotMSDBusApp::METHOD_LOCATEINFO, Record(), result);
    return fromRecord(result);
}

void plotms::update() {
		QtDBusXmlApp::dbusXmlCallNoRet( dbus::FROM_NAME, app.dbusName(),
				PlotMSDBusApp::METHOD_UPDATE, Record(), asyncCall );
}

void plotms::waitUntilIdle() {

  // Wait for it to have launched...
  bool idle = false;
  while(!idle) {
    usleep(500000);
    idle=!this->isDrawing();
  }
  return;
}

/*void plotms::show()   {
	if ( !showGui ){
		callAsync(PlotMSDBusApp::METHOD_SHOW);
	}
	else {
		QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
	            PlotMSDBusApp::METHOD_SHOW, Record(), asyncCall);
	}
}

void plotms::hide()   {
	if ( !showGui ){
		callAsync(PlotMSDBusApp::METHOD_HIDE);
	}
	else {
		QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
			            PlotMSDBusApp::METHOD_HIDE, Record(), asyncCall);
	}
}*/

void plotms::setShowGui( bool showGui ){
	this->showGui = showGui;
	if ( showGui ){
		//This is needed in the case where the is running plotms
		//from casapy.  If the user closes plotms, and then calls
		//the constructor again, plotms will not be shown without
		//this line.
		callAsync(PlotMSDBusApp::METHOD_SHOW);
	}
	asyncCall = showGui;
}



bool plotms::save(const string& filename, const string& format, const bool highres, const bool interactive) {
    launchApp();
    Record params;
    bool retValue;
    params.define(PlotMSDBusApp::PARAM_EXPORT_FILENAME, filename);
    params.define(PlotMSDBusApp::PARAM_EXPORT_FORMAT, format);
    params.define(PlotMSDBusApp::PARAM_EXPORT_HIGHRES, highres);
    params.define(PlotMSDBusApp::PARAM_EXPORT_INTERACTIVE, interactive);
    params.define(PlotMSDBusApp::PARAM_EXPORT_ASYNC, false);
    QtDBusXmlApp::dbusXmlCall(
    	dbus::FROM_NAME, app.dbusName( ),
        PlotMSDBusApp::METHOD_SAVE, params, retValue
    );
    return retValue;
}

bool plotms::isDrawing() {
	launchApp();
	Record params;
	bool retValue;
	QtDBusXmlApp::dbusXmlCall(
		dbus::FROM_NAME, app.dbusName(),
	    PlotMSDBusApp::METHOD_ISDRAWING, params, retValue
	);
	return retValue;
}

bool plotms::isClosed() {
	launchApp();
	Record params;
	bool retValue;
	QtDBusXmlApp::dbusXmlCall(
		dbus::FROM_NAME, app.dbusName(),
	    PlotMSDBusApp::METHOD_ISCLOSED, params, retValue
	);
	return retValue;
}

// Private Methods //

bool plotms::displaySet() {
    bool set = getenv("DISPLAY") != NULL;
    if(!set) {
        app.dbusName( ) = "";
        cerr << "ERROR: DISPLAY environment variable is not set!  Cannot open"
                " plotms." << endl;
    }
    return set;
}

void plotms::launchApp() {

    if(!app.dbusName( ).empty() || !displaySet()) return;
    
    String scriptClient;
    if ( !showGui ){
    	scriptClient = "--nogui";
    }

    // Launch PlotMS application with the DBus switch.
    pid_t pid = fork();
    if(pid == 0) {

        String file = itsLogFilename_.empty() ? "" :
                      PlotMSDBusApp::APP_LOGFILENAME_SWITCH + "=" +
                      itsLogFilename_;
        String filter = itsLogFilter_.empty() ? "" :
                        PlotMSDBusApp::APP_LOGFILTER_SWITCH + "=" +
                        itsLogFilter_;

	String nopop="--nopopups";

	// If user has turned off iter, be sure not to launch with it
	String iter="";
	if (!doIter_)
	  iter="--noiter";

        execlp(PlotMSDBusApp::APP_NAME.c_str(),
               PlotMSDBusApp::APP_NAME.c_str(),
               PlotMSDBusApp::APP_CASAPY_SWITCH.c_str(),
	       nopop.c_str(),
               file.c_str(), filter.c_str(),
	       iter.c_str(), scriptClient.c_str(),
               NULL);
        
    } else {

        app.dbusName( ) = to_string(QtDBusApp::generateServiceName(app.getName( ),pid));
        
        // Wait for it to have launched...
        unsigned int slept = 0;
        bool launched = false;
        while(!launched && slept < LAUNCH_TOTAL_WAIT_US) {
            usleep(LAUNCH_WAIT_INTERVAL_US);
            launched = QtDBusApp::serviceIsAvailable(app.dbusName( ));
            slept += LAUNCH_WAIT_INTERVAL_US;
        }
        
        if(launched) {        
            itsLogFilename_ = "";
            itsLogFilter_ = "";
            
        } else {
            app.dbusName( ) = "";
            cerr << "ERROR: plotms application did not launch within specified"
                    " time window.  Check running processes and try again if "
                    "desired." << endl;
        }
    }
}

void plotms::closeApp() {
    // Tell PlotMS application to quit.
    if(!app.dbusName( ).empty()){
    	QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
    			PlotMSDBusApp::METHOD_QUIT, Record(), asyncCall);
    }
}

void plotms::callAsync(const String& methodName) {
    launchApp();
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ), methodName,
                                   Record(), true);
}

void plotms::setPlotMSSelection_(const PlotMSSelection& selection,
        const bool updateImmediately, const int plotIndex) {
    launchApp();
    Record params;
    params.defineRecord(PlotMSDBusApp::PARAM_SELECTION, selection.toRecord());
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}

void plotms::setPlotMSAveraging_(const PlotMSAveraging& averaging,
        const bool updateImmediately, const int plotIndex) {
    launchApp();
    Record params;
    params.defineRecord(PlotMSDBusApp::PARAM_AVERAGING, averaging.toRecord());
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}

void plotms::setPlotMSTransformations_(const PlotMSTransformations& trans,
        const bool updateImmediately, const int plotIndex) {
    launchApp();
    Record params;
    params.defineRecord(PlotMSDBusApp::PARAM_TRANSFORMATIONS, trans.toRecord());
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}

void plotms::setPlotMSIterate_(const PlotMSIterParam& iter,
			       const bool updateImmediately, const int plotIndex) {
    launchApp();
    Record params;
    params.defineRecord(PlotMSDBusApp::PARAM_ITERATE, iter.toRecord());
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}

void plotms::setFlagging_(const PlotMSFlagging& flagging) {
    launchApp();
    Record params = flagging.toRecord();
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETFLAGGING, params, /*true*/asyncCall);
}




bool plotms::getGridMajorShown( const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTBOOL(/*PARAM_*/SHOWMAJORGRID);
}


bool plotms::getGridMinorShown( const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTBOOL(SHOWMINORGRID);
}


int plotms::getGridMajorWidth( const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTINT(MAJORWIDTH);
}


int plotms::getGridMinorWidth( const int plotIndex) 
{
    launchApp();
    GETSINGLEPLOTINT(MINORWIDTH);
}



void plotms::setGridParams(
                const bool showmajorgrid,  const int majorwidth, const string& majorstyle,  const string &majorcolor,
                const bool showminorgrid,  const int minorwidth, const string& minorstyle,  const string &minorcolor,
                const bool updateImmediately, const int plotIndex) 
{
    launchApp();
    
    string smaj = majorstyle;
    string smin = minorstyle;
    if (smaj=="")   smaj="solid";
    if (smin=="")   smin="solid";
    
    Record params;
    params.define(PlotMSDBusApp::PARAM_SHOWMAJORGRID,  showmajorgrid);
    params.define(PlotMSDBusApp::PARAM_MAJORWIDTH,  majorwidth);
    params.define(PlotMSDBusApp::PARAM_MAJORSTYLE,  smaj);
    params.define(PlotMSDBusApp::PARAM_MAJORCOLOR,  majorcolor);
    params.define(PlotMSDBusApp::PARAM_SHOWMINORGRID,  showminorgrid);
    params.define(PlotMSDBusApp::PARAM_MINORWIDTH,  minorwidth);
    params.define(PlotMSDBusApp::PARAM_MINORSTYLE,  smin);
    params.define(PlotMSDBusApp::PARAM_MINORCOLOR,  minorcolor);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
}



void plotms::setXRange(const bool xautorange,  const double xmin, const double xmax,
                const bool updateImmediately, const int plotIndex) 
{
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_XAUTORANGE,  xautorange);
    params.define(PlotMSDBusApp::PARAM_XMIN,        xmin);
    params.define(PlotMSDBusApp::PARAM_XMAX,        xmax);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
    
}



void plotms::setYRange(const bool yautorange,  const double ymin, const double ymax,
                const bool updateImmediately, const int plotIndex) 
{
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_YAUTORANGE,  yautorange);
    params.define(PlotMSDBusApp::PARAM_YMIN,        ymin);
    params.define(PlotMSDBusApp::PARAM_YMAX,        ymax);
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, /*true*/asyncCall);
    
}



// A _temporary_ method to enable turning off the iteration path
bool plotms::enableIter(const bool enable) {

  // Do something only if changing state
  if (enable!=doIter_) {

    if (enable)
      cout << "IF you have _already_ launched plotms, please exit and" << endl
	   << " restart casapy to enable iteration-capable plotting." << endl
	   << " Otherwise, you should be good to go!" << endl;
    else {
      cout << "IF you have _already_ launched plotms, please exit and" << endl
	   << " restart casapy _and_ rerun this method to disable iteration-capable plotting." << endl
	   << " Otherwise, you should be good to go!" << endl;
    }
    doIter_=enable;
  }
  else
    cout << "Interation is already " << (enable ? "enabled." : "disabled.") << endl;

  return true;

}




}  // end namespace
