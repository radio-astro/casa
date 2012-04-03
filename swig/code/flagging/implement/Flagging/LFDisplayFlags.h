//# LFDisplayFlags: A lighter flagger - for autoflag
//# Copyright (C) 2000,2001
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
//# $Jan 28 2011 rurvashi Id$
#ifndef FLAGGING_LFDISPLAYFLAGS_H
#define FLAGGING_LFDISPLAYFLAGS_H

#include <flagging/Flagging/LFExamineFlags.h>

#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/session/DBusSession.h>

#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <ms/MeasurementSets/MSColumns.h>

#include <flagging/Flagging/LFPlotServerProxy.h>


namespace casa { //# NAMESPACE CASA - BEGIN
  
  class LFDisplayFlags : public LFExamineFlags
  {
  public:  
    // default constructor 
    LFDisplayFlags  ();

    // default destructor
    virtual ~LFDisplayFlags ();

    // Return method name
    virtual String methodName(){return String("displayflags");};

    // Set autoflag params
    virtual Bool setParameters(Record &parameters);
    
    // Get default autoflag params
    virtual Record getParameters();

    // Run the algorithm
    virtual Bool runMethod(const VisBuffer &/*inVb*/, Cube<Float> &/*inVisc*/, Cube<Bool> &/*inFlagc*/,
                           Cube<Bool> &/*inPreFlagc*/, uInt /*numT*/, uInt /*numAnt*/, uInt /*numB*/,
                           uInt /*numC*/, uInt /*numP*/)
    {throw(AipsError("DisplayFlags::runMethod requires more inputs than supplied"));};    

    // Requires list of other flagmethods
    virtual Bool runMethod(const VisBuffer &inVb, 
                   Cube<Float> &inVisc, Cube<Bool> &inFlagc, Cube<Bool> &inPreFlagc,
			   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP,
			   Vector<CountedPtr<LFBase> > &flagmethods);

    
  protected:

    virtual Bool BuildPlotWindow();
    //    virtual Bool ShowFlagPlots();

     virtual Char GetUserInput();
     char *dock_xml_p;

     void DisplayRaster(Int xdim, Int ydim, Vector<Float> &data, uInt frame);
    void DisplayLine(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold,  uInt frame);
    void DisplayScatter(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold,  uInt frame);

        // Plotter members
    FlagPlotServerProxy *plotter_p;
    Vector<dbus::variant> panels_p;

    Bool ShowPlots, StopAndExit;

    // Additional private members
    uInt a1,a2;

  };
  
  
} //# NAMESPACE CASA - END

#endif

