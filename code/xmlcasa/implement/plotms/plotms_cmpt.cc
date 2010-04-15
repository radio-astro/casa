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
#include <xmlcasa/plotms/plotms_cmpt.h>

#include <xmlcasa/StdCasa/CasacSupport.h>
#include <xmlcasa/utils/dbus_cmpt.h>

#include <unistd.h>

namespace casac {


////////////////////////
// PLOTMS DEFINITIONS //
////////////////////////

// Static //

const unsigned int plotms::LAUNCH_WAIT_INTERVAL_US = 1000;
const unsigned int plotms::LAUNCH_TOTAL_WAIT_US    = 5000000;


// Constructors/Destructors //

  plotms::plotms() : itsWatcher_(this) { }

  plotms::~plotms() { closeApp(); }


// Public Methods //

// Convenience macros for setting/getting a single value using a record.
#define SETSINGLE(METHOD, PKEY, PVALUE)                                       \
    Record params;                                                            \
    params.define(PlotMSDBusApp::PARAM_##PKEY, PVALUE);                       \
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),          \
            PlotMSDBusApp::METHOD_##METHOD, params, true);

#define SETSINGLEPLOT(PKEY, PVALUE)                                           \
    Record params;                                                            \
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);                 \
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately); \
    params.define(PlotMSDBusApp::PARAM_##PKEY, PVALUE);                       \
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),          \
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, true);

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

void plotms::setLogFilename(const std::string& logFilename) {
    if(app.dbusName( ).empty()) itsLogFilename_ = logFilename;
    else {
        SETSINGLE(SETLOGPARAMS, FILENAME, logFilename)
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
        SETSINGLE(SETLOGPARAMS, PRIORITY, priority)
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
    SETSINGLE(SETPLOTMSPARAMS, CLEARSELECTIONS, clearSelection) }
bool plotms::getClearSelectionOnAxesChange() {
    launchApp();
    GETSINGLEBOOL(GETPLOTMSPARAMS, CLEARSELECTIONS) }

void plotms::setCachedImageSize(const int width, const int height) {
    launchApp();
    Record params;
    params.define(PlotMSDBusApp::PARAM_WIDTH, width);
    params.define(PlotMSDBusApp::PARAM_HEIGHT, height);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTMSPARAMS, params, true);
}

void plotms::setCachedImageSizeToScreenResolution() {
    callAsync(PlotMSDBusApp::METHOD_SETCACHEDIMAGESIZETOSCREENRES); }
int plotms::getCachedImageWidth() {
    launchApp();
    GETSINGLEINT(GETPLOTMSPARAMS, WIDTH) }
int plotms::getCachedImageHeight() {
    launchApp();
    GETSINGLEINT(GETPLOTMSPARAMS, HEIGHT) }


void plotms::setPlotMSFilename(const string& msFilename,
        const bool updateImmediately, const int plotIndex) {
    launchApp();
    SETSINGLEPLOT(FILENAME, msFilename) }
string plotms::getPlotMSFilename(const int plotIndex) {
    launchApp();
    GETSINGLEPLOTSTR(FILENAME) }

void plotms::setPlotMSSelection(const string& field, const string& spw,
        const string& timerange, const string& uvrange,
        const string& antenna, const string& scan, const string& corr,
        const string& array, const string& msselect,
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
  GETSINGLEPLOTREC(TRANSFORMATIONS) }







void plotms::setPlotXAxis(const string& xAxis, const string& xDataColumn,
		const bool updateImmediately, const int plotIndex) {
    setPlotAxes(xAxis, xDataColumn, "", "", updateImmediately, plotIndex); }
void plotms::setPlotYAxis(const string& yAxis, const string& yDataColumn,
		const bool updateImmediately, const int plotIndex) {
    setPlotAxes("", "", yAxis, yDataColumn, updateImmediately, plotIndex); }

void plotms::setPlotAxes(const string& xAxis, const string& yAxis,
        const string& xDataColumn, const string& yDataColumn,
        const bool updateImmediately, const int plotIndex) {
    launchApp();
    Record params;
    if(!xAxis.empty()) params.define(PlotMSDBusApp::PARAM_AXIS_X, xAxis);
    if(!xDataColumn.empty())
        params.define(PlotMSDBusApp::PARAM_DATACOLUMN_X, xDataColumn);
    if(!yAxis.empty()) params.define(PlotMSDBusApp::PARAM_AXIS_Y, yAxis);
    if(!yDataColumn.empty())
        params.define(PlotMSDBusApp::PARAM_DATACOLUMN_Y, yDataColumn);
    if(params.nfields() == 0) return;
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, true);
}

string plotms::getPlotXAxis(const int plotIndex) {
    launchApp();
    GETSINGLEPLOTSTR(AXIS_X) }
string plotms::getPlotXDataColumn(const int plotIndex) {
    launchApp();
    GETSINGLEPLOTSTR(DATACOLUMN_X) }
string plotms::getPlotYAxis(const int plotIndex) {
    launchApp();
    GETSINGLEPLOTSTR(AXIS_Y) }
string plotms::getPlotYDataColumn(const int plotIndex) {
    launchApp();
    GETSINGLEPLOTSTR(DATACOLUMN_Y) }

record* plotms::getPlotParams(const int plotIndex) {
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


void plotms::update() { callAsync(PlotMSDBusApp::METHOD_UPDATE); }
void plotms::show()   { callAsync(PlotMSDBusApp::METHOD_SHOW);   }
void plotms::hide()   { callAsync(PlotMSDBusApp::METHOD_HIDE);   }


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
    
    // Launch PlotMS application with the DBus switch.
    pid_t pid = fork();
    if(pid == 0) {
        String file = itsLogFilename_.empty() ? "" :
                      PlotMSDBusApp::APP_LOGFILENAME_SWITCH + "=" +
                      itsLogFilename_;
        String filter = itsLogFilter_.empty() ? "" :
                        PlotMSDBusApp::APP_LOGFILTER_SWITCH + "=" +
                        itsLogFilter_;
        
        execlp(PlotMSDBusApp::APP_NAME.c_str(),
               PlotMSDBusApp::APP_NAME.c_str(),
               PlotMSDBusApp::APP_CASAPY_SWITCH.c_str(),
               file.c_str(), filter.c_str(),
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
    if(!app.dbusName( ).empty()) callAsync(PlotMSDBusApp::METHOD_QUIT);
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
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, true);
}

void plotms::setPlotMSAveraging_(const PlotMSAveraging& averaging,
        const bool updateImmediately, const int plotIndex) {
    launchApp();
    Record params;
    params.defineRecord(PlotMSDBusApp::PARAM_AVERAGING, averaging.toRecord());
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, true);
}

void plotms::setPlotMSTransformations_(const PlotMSTransformations& trans,
        const bool updateImmediately, const int plotIndex) {
    launchApp();
    Record params;
    params.defineRecord(PlotMSDBusApp::PARAM_TRANSFORMATIONS, trans.toRecord());
    params.define(PlotMSDBusApp::PARAM_UPDATEIMMEDIATELY, updateImmediately);
    params.define(PlotMSDBusApp::PARAM_PLOTINDEX, plotIndex);
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETPLOTPARAMS, params, true);
}

void plotms::setFlagging_(const PlotMSFlagging& flagging) {
    launchApp();
    Record params = flagging.toRecord();
    QtDBusXmlApp::dbusXmlCallNoRet(dbus::FROM_NAME, app.dbusName( ),
            PlotMSDBusApp::METHOD_SETFLAGGING, params, true);
}

}
