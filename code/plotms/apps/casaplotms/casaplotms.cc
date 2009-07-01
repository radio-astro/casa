//# casaplotms.cc: Application for running plotms standalone.
//# Copyright (C) 2008
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
//#
//# $Id$
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/PlotMS/PlotMSDBusApp.h>

#include <display/QtViewer/QtApp.h>

#include <signal.h>

#include <casa/namespace.h>

int main(int argc, char* argv[]) {    
    // Parameter defaults.
    String ms    = "",
           xaxis = PMS::axis(PMS::DEFAULT_XAXIS),
           yaxis = PMS::axis(PMS::DEFAULT_YAXIS),
           logfile = "", logfilter = "";
    PlotMSSelection select;
    PlotMSAveraging averaging;
    bool cachedImageSizeToScreenResolution = false, usePixels = false,
         casapy = false, debug = false;
  
    // Parse arguments.
    String arg, arg2, arg3;
    size_t index;
    String ARG_HELP1 = "-h", ARG_HELP2 = "--help", ARG_VIS = "vis",
           ARG_XAXIS = "xaxis", ARG_YAXIS = "yaxis", ARG_CISTSR = "-c",
           ARG_CISTSR2 = "--cachedimagesizetoscreenresolution",
           ARG_PIXELS1 = "-p", ARG_PIXELS2 = "--pixels",
           ARG_CASAPY = PlotMSDBusApp::APP_CASAPY_SWITCH,
           ARG_DEBUG1 = "-d", ARG_DEBUG2 = "--debug",
           ARG_LOGFILE = PlotMSDBusApp::APP_LOGFILENAME_SWITCH,
           ARG_LOGFILTER = PlotMSDBusApp::APP_LOGFILTER_SWITCH;
    const vector<String>& selectFields = PlotMSSelection::fieldStrings(),
                          averagingFields = PlotMSAveraging::fieldStrings();
    
    for(int i = 1; i < argc; i++) {
        arg = arg2 = argv[i];
        arg2.downcase();
        
        if(arg.empty()) continue;
        
        if(arg2 == ARG_HELP1 || arg2 == ARG_HELP2) {
            cout << argv[0] << ": Stand-alone executable for CASA PlotMS."
                 << "\nAvailable arguments:\n"
                 
                 << "* " << ARG_HELP1 << " or " << ARG_HELP2 << "\n     "
                 << "Prints this message then exits."
                 
                 << "\n* " << ARG_VIS << "=[ms]\n     "
                 << "Path to MS used for initial plot."
                 
                 << "\n* " << ARG_XAXIS << "=[axis], " << ARG_YAXIS
                 << "=[axis]\n     "
                 << "Axes for initial plot (see documentation).";
            
            if(selectFields.size() > 0) cout << "\n*";
            for(unsigned int i = 0; i < selectFields.size(); i++) {
                if(i > 0) cout << ',';
                cout << " " << selectFields[i] << "=[val]";           
            }
            if(selectFields.size() > 0)
                cout << "\n     MS Selection parameters for initial plot.";
            
            if(averagingFields.size() > 0) cout << "\n*";
            for(unsigned int i = 0; i < averagingFields.size(); i++) {
                if(i > 0) cout << ',';
                cout << " avg" << averagingFields[i];
                if(PlotMSAveraging::fieldHasValue(PlotMSAveraging::field(
                        averagingFields[i]))) {
                    cout << "[=val]";
                }      
            }
            if(averagingFields.size() > 0)
                cout << "\n     MS Averaging parameters for initial plot.";
            
            cout << "\n* " << ARG_PIXELS1 << " or " << ARG_PIXELS2 << "\n     "
                 << "Use pixels instead of symbols for initial plot."
            
                 << "\n* " << ARG_CISTSR << " or " << ARG_CISTSR2 << "\n     "
                 << "Toggles setting the cached image size to screen "
                 << "resolution."
                 
                 /*
                 // Don't advertise this switch...
                 << "\n* " << ARG_CASAPY << "\n     "
                 << "Sets up application to be controlled by casapy."
                 */
                 
                 << "\n* " << ARG_LOGFILE << "=[filename]\n     "
                 << "Sets the log file location (blank to use global)."
                 
                 << "\n* " << ARG_LOGFILTER << "=[priority]\n     "
                 << "Sets the log minimum priority filter."
                 
                 << "\n* " << ARG_DEBUG1 << " or " << ARG_DEBUG2 << "\n     "
                 << "Turn on debugging log messages."
            
                 << endl;
            return 0;
        }
        
        if((index = arg.find("=")) < arg.size() - 1) {
            arg2 = arg.before(index); arg2.downcase();
            arg3 = arg.after(index);
            
        } else if(arg2 == ARG_CISTSR || arg2 == ARG_CISTSR2) {
            cachedImageSizeToScreenResolution = true;
            
        } else if(arg2 == ARG_PIXELS1 || arg2 == ARG_PIXELS2) {
            usePixels = true;
            
        } else if(arg2 == ARG_CASAPY) {
            casapy = true;
            
        } else if(arg2 == ARG_DEBUG1 || arg2 == ARG_DEBUG2) {
            debug = true;
            
        } else if(i < argc - 1) {
            arg3 = argv[++i];
            if(arg3.size() == 0) continue;
            if(arg3[0] == '=' && arg3.size() > 1) arg3 = arg3.after(0);
            else if(arg3 == "=" && i < argc - 1) arg3 = argv[++i];
            else if(index != arg.size() - 1) continue;
            
        } else {
            arg3 = "";
            bool found = false;
            for(unsigned int i = 0; !found && i < averagingFields.size(); i++)
                if(PMS::strEq(arg2, "avg" + averagingFields[i], true))
                    found = true;
            if(!found) continue;
        }
        
        if(arg2[arg2.size() - 1] == '=') arg2.erase(arg2.size() - 1);
        
        if(arg2 == ARG_VIS)         ms = arg3;
        else if(arg2 == ARG_XAXIS) xaxis = arg3;
        else if(arg2 == ARG_YAXIS) yaxis = arg3;
        else if(arg2 == ARG_LOGFILE) logfile = arg3;
        else if(arg2 == ARG_LOGFILTER) logfilter = arg3;
        else {
            bool found = false;
            for(unsigned int i = 0; !found && i < selectFields.size(); i++) {
                if(PMS::strEq(arg2, selectFields[i], true)) {
                    select.setValue(PlotMSSelection::field(selectFields[i]),
                                    arg3);
                    found = true;
                }
            }
            PlotMSAveraging::Field f;
            double d;
            for(unsigned int i = 0; !found && i < averagingFields.size(); i++){
                if(PMS::strEq(arg2, "avg" + averagingFields[i], true)) {
                    f = PlotMSAveraging::field(averagingFields[i]);
                    averaging.setFlag(f, true);
                    if(PlotMSAveraging::fieldHasValue(f) && !arg3.empty()) {
                        if(sscanf(arg3.c_str(), "%lf", &d) >= 1)
                            averaging.setValue(f, d);
                    }
                    found = true;
                }
            }
        }
    }
    
    // If run from casapy, don't let Ctrl-C kill the application.
    if(casapy) signal(SIGINT,SIG_IGN);
    
    // Although the qwt plotter backend can manage the QApplication, since
    // the PlotMSPlotter is a QMainWindow we need the QApplication regardless
    // of the plotting backend.
    QtApp::init();
    
    // Get priority filter.
    LogMessage::Priority p = debug? LogMessage::DEBUGGING: LogMessage::NORMAL;
    if(!logfilter.empty()) {
        for(int i = LogMessage::DEBUGGING; i < LogMessage::SEVERE; i++) {
            if(LogMessage::toString(LogMessage::Priority(i)) == logfilter) {
                p = LogMessage::Priority(i);
                break;
            }
        }
    }
    
    // Set up parameters for plotms.
    PlotMSParameters params(logfile, PlotLogger::FLAG_FROM_PRIORITY(p), p);
    
    if(cachedImageSizeToScreenResolution)
        params.setCachedImageSizeToResolution();
    
    // Set up plotms object.
    PlotMS plotms(params, casapy);
    if(!casapy) plotms.showGUI(true); // don't automatically show for casapy
    
    // Set up parameters for single plot.
    PlotMSSinglePlotParameters plotparams(&plotms, ms);
    plotparams.setAxes(PMS::axis(xaxis), PMS::axis(yaxis));
    plotparams.setSelection(select);
    plotparams.setAveraging(averaging);
    
    if(usePixels) {
        PlotSymbolPtr sym = plotparams.symbol();
        sym->setSymbol(PlotSymbol::PIXEL);
        plotparams.setSymbol(sym);
    }
    
    // If single plot is set, add the plot to plotms.
    plotms.addSinglePlot(&plotparams);
    
    // If we're connected to DBus, don't quite the application when the window
    // is closed.  This is somewhat risky in that if the remote applications
    // forget to tell this application to quit, it never will.
    if(casapy) QApplication::setQuitOnLastWindowClosed(false);
    
    //return plotms.execLoop();
    return QtApp::exec();
}
