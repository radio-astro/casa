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

#include <casa/namespace.h>

int main(int argc, char* argv[]) {
    // Although the qwt plotter backend can manage the QApplication, since
    // the PlotMSPlotter is a QMainWindow we need the QApplication regardless
    // of the plotting backend.
    QApplication app(argc, argv);
    
    // Parameter defaults.
    String ms    = "",
           xaxis = PMS::axis(PMS::DEFAULT_XAXIS),
           yaxis = PMS::axis(PMS::DEFAULT_YAXIS);
    PlotMSLogger::Level log = PlotMSLogger::OFF;
    PlotMSSelection select;
  
    // Parse arguments.
    String arg, arg2, arg3;
    size_t index;
    bool ok;
    String ARG_HELP1 = "-h", ARG_HELP2 = "--help", ARG_MS = "ms",
           ARG_XAXIS = "xaxis", ARG_YAXIS = "yaxis", ARG_LOG1 = "-ll",
           ARG_LOG2 = "--loglevel";
    const vector<String>& selectFields = PlotMSSelection::fieldStrings();
    
    for(int i = 1; i < argc; i++) {
        arg = arg2 = argv[i];
        arg2.downcase();
        
        if(arg.empty()) continue;
        
        if(arg2 == ARG_HELP1 || arg2 == ARG_HELP2) {
            cout << argv[0] << ": Stand-alone executable for CASA PlotMS."
                 << "\nAvailable arguments:\n"
                 
                 << "* " << ARG_HELP1 << " or " << ARG_HELP2 << "\n     "
                 << "Prints this message then exits."
                 
                 << "\n* " << ARG_MS << "=[ms]\n     "
                 << "Path to MS used for initial plot."
                 
                 << "\n* " << ARG_XAXIS << "=[axis]\n     "
                 << "X-Axis for initial plot (see documentation)."
                 
                 << "\n* " << ARG_YAXIS << "=[axis]\n     "
                 << "Y-Axis for initial plot (see documentation)."
                 
                 << "\n* "<<ARG_LOG1<<"=[lvl] or "<<ARG_LOG2<<"=[lvl]\n     "
                 << "Sets the plotter's log level to the given (see "
                 << "documentation).";
            
            for(unsigned int i = 0; i < selectFields.size(); i++) {
                cout << "\n* " << selectFields[i] << "=[val]\n     "
                     << "MS Selection parameter.";
            }
            
            cout << endl;
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
        } else {
            for(unsigned int i = 0; i < selectFields.size(); i++) {
                if(PMS::strEq(arg2, selectFields[i], true))
                    select.setValue(PlotMSSelection::field(selectFields[i]),
                                    arg3);
            }
        }
    }
    
    // Set up parameters for plotms.
    PlotMSParameters params(log);
    
    // Set up plotms object.
    PlotMS plotms(params);
    plotms.showGUI(true);
    
    // Set up parameters for single plot.
    PlotMSSinglePlotParameters plotparams(&plotms, ms);
    plotparams.setAxes(PMS::axis(xaxis), PMS::axis(yaxis));
    plotparams.setSelection(select);
    
    // If single plot is set, add the plot to plotms.
    plotms.addSinglePlot(&plotparams);
    
    return plotms.execLoop();
}
