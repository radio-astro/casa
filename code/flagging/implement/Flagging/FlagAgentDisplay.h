//# FlagAgentDisplay.h: This file contains the interface definition of the FlagAgentDisplay class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef FlagAgentDisplay_H_
#define FlagAgentDisplay_H_

#include <flagging/Flagging/FlagAgentBase.h>

#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/session/DBusSession.h>

#include <ms/MeasurementSets/MSColumns.h>

#include <flagging/Flagging/LFPlotServerProxy.h>


namespace casa { //# NAMESPACE CASA - BEGIN

class FlagAgentDisplay : public FlagAgentBase {

public:

	FlagAgentDisplay(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube = false);
	~FlagAgentDisplay();

protected:

	// Compute flags for a given (time,freq) map
        void computeAntennaPairFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flag,Int antenna1,Int antenna2,vector<uInt> &rows);

        // Choose how to step through the baselines in the current chunk
  	void iterateAntennaPairsInteractive(antennaPairMap *antennaPairMap_ptr);

	// Parse configuration parameters
	void setAgentParameters(Record config);

private:

    virtual Bool BuildPlotWindow();
    //    virtual Bool ShowFlagPlots();

     virtual Char GetUserInput();
     char *dock_xml_p;

     void DisplayRaster(Int xdim, Int ydim, Vector<Float> &data, uInt frame);
    void DisplayLine(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold,  uInt frame);
    void DisplayScatter(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold,  uInt frame);

        // Plotter members
  FlagPlotServerProxy *plotter_p;  // UUU Make this a CountedPtr
    Vector<dbus::variant> panels_p;

    Bool ShowPlots, StopAndExit;

    // Additional private members
  uInt nPolarizations_p;

  Bool pause_p;

  Char userchoice_p;

};


} //# NAMESPACE CASA - END

#endif /* FLAGAGENTDISPLAY_H_ */

