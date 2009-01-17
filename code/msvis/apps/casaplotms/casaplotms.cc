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
#include <msvis/PlotMS/PlotMS.h>

#include <casa/namespace.h>

int main(int argc, char* argv[]) {
    // Although the qwt plotter backend can manage the QApplication, since
    // the PlotMSPlotter is a QMainWindow we need the QApplication regardless
    // of the plotting backend.
    QApplication app(argc, argv);
    
    // Parameter defaults.
    String ms    = "",
           xaxis = PMS::axis(PlotMSParameters::DEFAULT_XAXIS),
           yaxis = PMS::axis(PlotMSParameters::DEFAULT_YAXIS);
    PlotMSLogger::Level log = PlotMSLogger::OFF;
  
    // Parse arguments.
    String arg, arg2, arg3;
    size_t index;
    bool ok;
    String ARG_HELP1 = "-h", ARG_HELP2 = "--help", ARG_MS = "ms",
           ARG_XAXIS = "xaxis", ARG_YAXIS = "yaxis", ARG_LOG1 = "-l",
           ARG_LOG2 = "--log";
    for(int i = 1; i < argc; i++) {
        arg = arg2 = argv[i];
        arg2.downcase();
        
        if(arg.empty()) continue;
        
        if(arg2 == ARG_HELP1 || arg2 == ARG_HELP2) {
            cout << argv[0] << ": Stand-alone executable for CASA PlotMS."
                 << "\nAvailable arguments:\n"
                 << "\t-h or --help\t\tPrints this message then exits.\n"
                 << "\tms=[ms]\t\t\tMS used for initial plot.\n"
                 << "\txaxis=[axis]"
                 << "\t\tX-Axis for initial plot (see documentation).\n"
                 << "\tyaxis=[axis]"
                 << "\t\tY-Axis for initial plot (see documentation).\n"
                 << "\t-l=[lvl] or --log=[lvl]\tSets the plotter's log "
                 << "level to the given (see documentation)." << endl;
            return 0;
        }
        
        if((index = arg.find("=")) < arg.size() - 1) {
            arg2 = arg.before(index); arg2.downcase();
            arg3 = arg.after(index);
            
        } else if(i < argc - 1) {
            arg3 = argv[++i];
            if(arg3.size() == 0) continue;
            if(arg3[0] == '=' && arg3.size() > 1) arg3 = arg3.after(0);
            else if(arg3 == "=" && i < argc - 1) arg3 = argv[++i];
            else if(index != arg.size() - 1) continue;
            
        } else if(arg2 == ARG_LOG1 || arg2 == ARG_LOG2) {
            // log is last argument, use PlotMSLogger::MED
            arg3 = PlotMSLogger::level(PlotMSLogger::MED);
            
        } else continue;
        
        if(arg2[arg2.size() - 1] == '=') arg2.erase(arg2.size() - 1);
        
        if(arg2 == ARG_MS)         ms = arg3;
        else if(arg2 == ARG_XAXIS) xaxis = arg3;
        else if(arg2 == ARG_YAXIS) yaxis = arg3;
        else if(arg2 == ARG_LOG1 || arg2 == ARG_LOG2) {
            log = PlotMSLogger::level(arg3, &ok);
            if(!ok) log = PlotMSLogger::OFF;
        }
    }
    
    // Set up plotting parameters.
    PlotMSParameters params(ms, PMS::axis(xaxis), PMS::axis(yaxis));
    if(ms.empty()) params.setIsSet(false);
    PlotMSOptions opts(1, 1); // one row, one column by default
    opts.setParameters(0, 0, params);
    
    // Set up plotter.
    PlotMS plot(log);    
    if(!plot.setOptions(opts))
        plot.showWarning("Could not set options: " + plot.getLastError());
    
    return plot.execLoop();
}
