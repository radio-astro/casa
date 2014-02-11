//# PlotMSAction.h: Actions on plotms that can be triggered.
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
#ifndef PLOTMSACTION_H_
#define PLOTMSACTION_H_

#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/Threads/ThreadController.h>
#include <map>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward declarations
class PlotMSApp;
class PlotMSPlot;
class Client;
class BackgroundThread;

// ABSTRACT CLASSES //

// Class that implements actions that operate on PlotMS.  Actions get their
// type from the Type enum and their parameters (if needed) from mappings from
// Strings to values.  Current parameter value types are: PlotMSPlot*, bool,
// String, int, and vector<PMS::Axis>.
class PlotMSAction {
public:
    // Static //
	// Enum for implemented actions.
		enum Type {
			// Selection actions //

			// Flag the selected regions.   Operates on all visible canvases AT THE
			// TIME of the call to doAction().
			// No required parameters.
			SEL_FLAG,

			// Unflag the selected regions.   Operates on all visible canvases AT
			// THE TIME of the call to doAction().
			// No required parameters.
			SEL_UNFLAG,

			// Locate on the selected regions.   Operates on all visible canvases
			// AT THE TIME of the call to doAction().
			// No required parameters.
			SEL_LOCATE,

			// Get metadata of points in the selected regions.  Operates on all
			// visible canvases AT THE TIME of the call to doAction().
			// No required parameters.
			SEL_INFO,

			// Clear the selected regions.   Operates on all visible canvases AT
			// THE TIME of the call to doAction().
			// No required parameters.
			SEL_CLEAR_REGIONS,


			// Iteration actions //

			// Go to the first page of iteration.
			// No required parameters.
			// NOT IMPLEMENTED.
			ITER_FIRST,

			// Go to the previous page of iteration.
			// No required parameters.
			// NOT IMPLEMENTED.
			ITER_PREV,

			// Go to the next page of iteration.
			// No required parameters.
			// NOT IMPLEMENTED.
			ITER_NEXT,

			// Go to the last page of iteration.
			// No required parameters.
			// NOT IMPLEMENTED.
			ITER_LAST,


			// Tool actions //

			// Turns on/off the mark regions tool.  Operates on all canvases of all
			// plots AT THE TIME of the call to doAction().  Only one of the mark
			// regions, zoom, pan, and annotate tools can be turned on at one time.
			// Required parameters: P_ON_OFF.
			TOOL_MARK_REGIONS,

			// Turns on/off the subtract regions tool.
			// works like MARK REGIONS, but where user drags, all regions within are deleted.
			// (note: CAS-1971 DSW)
			TOOL_SUBTRACT_REGIONS,

			// Turns on/off the zoom tool.  Operates on all canvases of all plots
			// AT THE TIME of the call to doAction().  Only one of the mark
			// regions, zoom, pan, and annotate tools can be turned on at one time.
			// Required parameters: P_ON_OFF.
			TOOL_ZOOM,

			// Turns on/off the pan tool.  Operates on all canvases of all plots AT
			// THE TIME of the call to doAction().  Only one of the mark regions,
			// zoom, pan, and annotate tools can be turned on at one time.
			// Required parameters: P_ON_OFF.
			TOOL_PAN,

			// Turns on/off the annotator tool's different modes.  Operates on all
			// canvases of all plots AT THE TIME of the call to doAction().  Only
			// one of the mark regions, zoom, pan, and annotate tools can be turned
			// on at one time.
			// Required parameters: P_ON_OFF.
			// <group>
			TOOL_ANNOTATE_TEXT,
			TOOL_ANNOTATE_RECTANGLE,
			// </group>

			// Turns on/off the tracker tool hover function.  Operates on all
			// canvases of all plots AT THE TIME of the call to doAction().
			// Required parameters: P_ON_OFF.
			TRACKER_ENABLE_HOVER,

			// Turns on/off the tracker tool display function.  Operates on all
			// canvases of all plots AT THE TIME of the call to doAction().
			// Required parameters: P_ON_OFF.
			TRACKER_ENABLE_DISPLAY,


			// Stack actions //

			// Goes back in the zoom/pan stack.  Operates on all visible canvases
			// AT THE TIME of the call to doAction().
			// No required parameters.
			STACK_BACK,

			// Goes to the base of the zoom/pan stack.  Operates on all visible
			// canvases AT THE TIME of the call to doAction().
			// No required parameters.
			STACK_BASE,

			// Goes forward in the zoom/pan stack.  Operates on all visible
			// canvases AT THE TIME of the call to doAction().
			// No required parameters.
			STACK_FORWARD,


			// Cache actions //

			// Loads axes into the cache.  Operates on a single PlotMSPlot.
			// Required parameters: P_PLOT, P_AXES.
			CACHE_LOAD,

			// Releases axes from the cache.  Operates on a single PlotMSPlot.
			// Required parameters: P_PLOT, P_AXES.
			// NOT IMPLEMENTED.
			CACHE_RELEASE,


			// Plot actions //

			// Display MS summary info for a specific file.
			// Required parameters: P_PLOT.
			MS_SUMMARY,

			//Show the summary dialog for the plot that allows the user
			//to set up parameters for MS_SUMMARY
			SUMMARY_DIALOG,

			// Updates any set parameters, which updates the plots.
			// No required parameters.
			PLOT,

			// Exports a single PlotMSPlot to a file.  If format isn't given, it is
			// set using the file name.  If DPI, width, or height aren't set or are
			// < 0, the default is used.
			// Required parameters: P_PLOT, P_FILE.
			// Optional parameters: P_FORMAT, P_HIGHRES, P_DPI, P_WIDTH, P_HEIGHT.
			PLOT_EXPORT,

			//Show a dialog that allows the client to set up export parameters.
			EXPORT_DIALOG,


			// Plotter actions //

			// Holds/releases drawing for all canvases in the PlotMSPlotter.
			// Required parameters: P_ON_OFF.
			HOLD_RELEASE_DRAWING,

			// Clears all plots and canvases from the PlotMSPlotter.
			// No required parameters.
			CLEAR_PLOTTER,

			// Quits PlotMS.
			// No required parameters.
			QUIT,

			//Open an ms or cal table for plotting.
			OPEN
		};

    // Non-Static //

    // Constructor.
    PlotMSAction( Client * client, PMSPTMethod postThreadMethod = NULL,
    		vector<PlotMSPlot*> postThreadObject = vector<PlotMSPlot*>());

    // Destructor.
    virtual ~PlotMSAction();

    // Performs the action, using the given PlotMSApp, and returns true for
    // success or false or failure.
    bool doAction(PlotMSApp* plotms);

    void setUseThreading( bool useThread );

    virtual bool doActionWithResponse(PlotMSApp* plotms, Record &retval);

    // Returns the result of doAction(), if applicable.  Usually this is used
    // to return the error/reason why the action failed.
	const String& doActionResult() const;

protected:
	 bool useThreading;
	 virtual bool loadParameters();
	 virtual bool doActionSpecific( PlotMSApp* plotms ) = 0;
	 void setUpClientCommunication( BackgroundThread* thread, int index );
	 bool initiateWork( BackgroundThread* thread );
	 Client* client;
	 // Action type.
	 Type itsType_;

	 // Result of doAction, if applicable.
	 String itsDoActionResult_;
	 ThreadController* threadController;

	 vector<PlotMSPlot*> postThreadObject;
	 PMSPTMethod* postThreadMethod;
private:
	// Returns true if the action is valid or not.  Invalid actions should not
	// be executed.
	bool isValid();


	PlotMSAction( const PlotMSAction& other );

};

}

#endif /* PLOTMSACTION_H_ */
