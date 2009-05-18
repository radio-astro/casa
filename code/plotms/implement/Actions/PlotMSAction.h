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

#include <map>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward declarations
class PlotMS;
class PlotMSPlot;


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
        TRACKER_HOVER,

        // Turns on/off the tracker tool display function.  Operates on all
        // canvases of all plots AT THE TIME of the call to doAction().
        // Required parameters: P_ON_OFF.
        TRACKER_DISPLAY,


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

        // Exports a single PlotMSPlot to a file.  If format isn't given, it is
        // set using the file name.  If DPI, width, or height aren't set or are
        // < 0, the default is used.
        // Required parameters: P_PLOT, P_FILE.
        // Optional parameters: P_FORMAT, P_HIGHRES, P_DPI, P_WIDTH, P_HEIGHT.
        PLOT_EXPORT,


        // Plotter actions //

        // Holds/releases drawing for all canvases in the PlotMSPlotter.
        // Required parameters: P_ON_OFF.
        HOLD_RELEASE_DRAWING,

        // Clears all plots and canvases from the PlotMSPlotter.
        // No required parameters.
        CLEAR_PLOTTER,

        // Quits PlotMS.
        // No required parameters.
        QUIT
    };

    // Parameter names.
    // <group>
    static const String P_PLOT;    // Type: PlotMSPlot*
    static const String P_ON_OFF;  // Type: bool
    static const String P_AXES;    // Type: vector<PMS::Axis>
    static const String P_FILE;    // Type: String
    static const String P_FORMAT;  // Type: String
    static const String P_HIGHRES; // Type: bool
    static const String P_DPI;     // Type: int
    static const String P_WIDTH;   // Type: int
    static const String P_HEIGHT;  // Type: int
    // </group>
    
    
    // Returns true if the given type requires the given parameter, false
    // otherwise.
    static bool requires(Type type, const String& parameter);


    // Non-Static //

    // Constructor.
    PlotMSAction(Type type);

    // Destructor.
    ~PlotMSAction();


    // Returns the action type.
    Type type() const;

    // Returns true if the action is valid or not.  Invalid actions should not
    // be executed.
    bool isValid() const;

    // Sets the given parameter to the given value.
    // <group>
    void setParameter(const String& parameter, PlotMSPlot* value);
    void setParameter(const String& parameter, bool value);
    void setParameter(const String& parameter, const String& value);
    void setParameter(const String& parameter, int value);
    void setParameter(const String& parameter, const vector<PMS::Axis>& value);
    // </group>

    // Performs the action, using the given PlotMS, and returns true for
    // success or false or failure.
    bool doAction(PlotMS* plotms);

    // Returns the result of doAction(), if applicable.  Usually this is used
    // to return the error/reason why the action failed.
	const String& doActionResult() const;

private:
	// Action type.
	Type itsType_;

	// Action parameters.
	// <group>
	map<String, PlotMSPlot*> itsPlotValues_;
	map<String, bool> itsBoolValues_;
	map<String, String> itsStringValues_;
	map<String, int> itsIntValues_;
	map<String, vector<PMS::Axis> > itsAxesValues_;
	// </group>

	// Result of doAction, if applicable.
	String itsDoActionResult_;


	// Returns true if the given parameter is defined for the given type, false
	// otherwise.
	// <group>
	bool isDefinedPlot(const String& parameter) const;
	bool isDefinedBool(const String& parameter) const;
	bool isDefinedString(const String& parameter) const;
	bool isDefinedInt(const String& parameter) const;
	bool isDefinedAxes(const String& parameter) const;
	// </group>

	// Returns the value for the given parameter.  Not valid if the proper
	// isDefined method returns false.
	// <group>
	PlotMSPlot* valuePlot(const String& parameter);
	const PlotMSPlot* valuePlot(const String& parameter) const;
	bool valueBool(const String& parameter) const;
	const String& valueString(const String& parameter) const;
	int valueInt(const String& parameter) const;
	const vector<PMS::Axis>& valueAxes(const String& parameter) const;
	// </group>
};

}

#endif /* PLOTMSACTION_H_ */
