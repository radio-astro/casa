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
class PlotMSActionParameters;
class PlotMS;
class PlotMSPlot;


// ABSTRACT CLASSES //

// Abstract superclass for any action that operates on PlotMS.  Actions can
// get their parameters (if needed) from any source that implements
// PlotMSActionParameters.  PlotMSAction also holds an enum of all available
// actions and knows how to convert the enum into the proper subclass.
class PlotMSAction {
public:
    // Static //
    
    // Enum for implemented actions.
    enum Type {
        // Selection actions //
        
        // Flag the selected regions.  Operates on a single PlotMSPlot.
        // NOT IMPLEMENTED.
        FLAG,
        
        // Unflag the selected regions.  Operates on a single PlotMSPlot.
        // NOT IMPLEMENTED.
        UNFLAG,
        
        // Locate on the selected regions.  Operates on a single PlotMSPlot.
        LOCATE,
        
        // Clear the selected regions.  Operates on a single PlotMSPlot.
        CLEAR_REGIONS,
        
        
        // Iteration actions //
        
        // First iteration.  Operates on a single PlotMSPlot.
        // NOT IMPLEMENTED.
        ITER_FIRST,
        
        // Previous iteration.  Operates on a single PlotMSPlot.
        // NOT IMPLEMENTED.
        ITER_PREV,
        
        // Next iteration.  Operates on a single PlotMSPlot.
        // NOT IMPLEMENTED.
        ITER_NEXT,
        
        // Last iteration.  Operates on a single PlotMSPlot.
        // NOT IMPLEMENTED.
        ITER_LAST,
        
        
        // Tool actions //
        
        // Turns on the mark regions tool.  Operates on a single PlotMSPlot and
        // is mutually exclusive with the zoom and pan tools.
        TOOL_MARK_REGIONS,
        
        // Turns on the zoom tool.  Operates on a single PlotMSPlot and is
        // mutually exclusive with the mark regions and pan tools.
        TOOL_ZOOM,
        
        // Turns on the pan tool.  Operates on a single PlotMSPlot and is
        // mutually exclusive with the mark regions and zoom tools.
        TOOL_PAN,
        
        // Turns on the tracker tool hover function.  Operates on a single
        // PlotMSPlot.
        TRACKER_HOVER,
        
        // Turns on the tracker tool display function.  Operates on a single
        // PlotMSPlot.
        TRACKER_DISPLAY,
        
        
        // Stack actions //
        
        // Goes back in the zoom/pan stack.  Operates on a single PlotMSPlot.
        STACK_BACK,
        
        // Goes to the base of the zoom/pan stack.  Operates on a single
        // PlotMSPlot.
        STACK_BASE,
        
        // Goes forward in the zoom/pan stack.  Operates on a single
        // PlotMSPlot.
        STACK_FORWARD,
        
        
        // Cache actions //
        
        // Loads axes into the cache.  Operates on a single PlotMSPlot.
        CACHE_LOAD,
        
        // Releases axes from the cache.  Operates on a single PlotMSPlot.
        // NOT IMPLEMENTED.
        CACHE_RELEASE,
        
        
        // Plotter actions //
        
        // Holds/releases drawing for all canvases in the PlotMSPlotter.
        HOLD_RELEASE_DRAWING,   
        
        // Clears all plots and canvases from the PlotMSPlotter.
        CLEAR_PLOTTER,
        
        // Quits PlotMS.
        QUIT,
        
        
        // User actions //
        
        // Custom action that must be able to operate basically independently
        // since the rest of PlotMS will know nothing about it.  Shouldn't be
        // used by any standard PlotMS code.
        CUSTOM
    };
    
    // Returns an executable PlotMSAction subclass instantiation for the given
    // action type and parameters source.
    static PlotMSAction* action(Type type, PlotMSActionParameters* params);
    
    
    // Non-Static //
    
    // Constructor.
    PlotMSAction() { }
    
    // Destructor.
    virtual ~PlotMSAction() { }
    
    
    // Returns the action type.
    virtual Type type() const = 0;
    
    // Returns true if the action is valid or not.  Invalid actions should not
    // be executed.
    virtual bool isValid() const = 0;
    
    // Sets the parameters for this action using the given source.
    virtual void setParameters(PlotMSActionParameters* params) = 0;
    
    // Performs the action, using the given PlotMS.
    virtual void doAction(PlotMS* plotms) = 0;
};


// Abstract class for any object that can provide parameters for an action.
// The methods to get the parameters have a default definition just so that
// subclasses may only implement the relevant methods.
class PlotMSActionParameters {
public:
    // Constructor.
    PlotMSActionParameters() { }
    
    // Destructor.
    virtual ~PlotMSActionParameters() { }
    
    
    // Returns the PlotMSPlot value needed for the given action type.
    virtual PlotMSPlot* actionPlot(PlotMSAction::Type type) const {
        return NULL; }
    
    // Returns the bool value needed for the given action type.
    virtual bool actionBool(PlotMSAction::Type type) const {
        return false; }
    
    // Returns the axes value needed for the given action type.
    virtual vector<PMS::Axis> actionAxes(PlotMSAction::Type type) const {
        return vector<PMS::Axis>(); }
};


// CONCRETE CLASSES //

// Action class that acts on selected regions for the canvases of a single
// PlotMS.
class PlotMSActionOnSelection : public PlotMSAction {
public:
    // Constructor which takes the action type (must be one of the selection
    // actions) and the action parameters source.
    PlotMSActionOnSelection(Type type, PlotMSActionParameters* params);
    
    // Destructor.
    ~PlotMSActionOnSelection();
    
    
    // Implements PlotMSAction::type().
    Type type() const { return itsType_; }
    
    // Implements PlotMSAction::isValid().
    bool isValid() const;
    
    // Implements PlotMSAction::setParameters().
    void setParameters(PlotMSActionParameters* params);    
    
    // Implements PlotMSAction::doAction().  See documentation for
    // PlotMSAction::Type enum.
    void doAction(PlotMS* plotms);
    
private:
    // Type.
    Type itsType_;
    
    // Plot.
    PlotMSPlot* itsPlot_;
    
    // Selected regions.
    map<PlotCanvas*, vector<PlotRegion> > itsRegions_;
};


// Action class that iterates through a single PlotMS.
class PlotMSActionOnIteration : public PlotMSAction {
public:
    // Constructor which takes the action type (must be one of the iteration
    // actions) and the action parameters source.
    PlotMSActionOnIteration(Type type, PlotMSActionParameters* params);
    
    // Destructor.
    ~PlotMSActionOnIteration();
    
    
    // Implements PlotMSAction::type().
    Type type() const { return itsType_; }
    
    // Implements PlotMSAction::isValid().
    bool isValid() const;
    
    // Implements PlotMSAction::setParameters().
    void setParameters(PlotMSActionParameters* params);    
    
    // Implements PlotMSAction::doAction().  See documentation for
    // PlotMSAction::Type enum.
    void doAction(PlotMS* plotms);
    
private:
    // Type.
    Type itsType_;
    
    // Plot.
    PlotMSPlot* itsPlot_;
};


// Action class that sets tools on all canvases for a single PlotMSPlot.
class PlotMSActionOnTools : public PlotMSAction {
public:
    // Constructor which takes the action type and the action parameters
    // source.
    PlotMSActionOnTools(Type type, PlotMSActionParameters* params);
    
    // Destructor.
    ~PlotMSActionOnTools();

    
    // Implements PlotMSAction::type().
    Type type() const { return itsType_; }
    
    // Implements PlotMSAction::isValid().
    bool isValid() const;
    
    // Implements PlotMSAction::setParameters().
    void setParameters(PlotMSActionParameters* params);    
    
    // Implements PlotMSAction::doAction().  See documentation for
    // PlotMSAction::Type enum.
    void doAction(PlotMS* plotms);
    
private:
    // Type.
    Type itsType_;
    
    // Plot.
    PlotMSPlot* itsPlot_;
    
    // Tool flags.
    bool mark_, zoom_, pan_, trackerHover_, trackerDisplay_;
};


// Action class that acts on the zoom/pan stacks on all canvases for a single
// PlotMSPlot.
class PlotMSActionOnStack : public PlotMSAction {
public:
    // Constructor which takes the action type (must be one of the stack
    // actions) and the action parameters source.
    PlotMSActionOnStack(Type type, PlotMSActionParameters* params);
    
    // Destructor.
    ~PlotMSActionOnStack();
    
    
    // Implements PlotMSAction::type().
    Type type() const { return itsType_; }
    
    // Implements PlotMSAction::isValid().
    bool isValid() const;
    
    // Implements PlotMSAction::setParameters().
    void setParameters(PlotMSActionParameters* params);    
    
    // Implements PlotMSAction::doAction().  See documentation for
    // PlotMSAction::Type enum.
    void doAction(PlotMS* plotms);
    
private:
    // Type.
    Type itsType_;
    
    // Plot.
    PlotMSPlot* itsPlot_;
};


// Action class that acts on the cache of a single PlotMSPlot.
class PlotMSActionOnCacheAxes : public PlotMSAction {
public:
    // Constructor which takes the action type (must be one of the cache
    // actions) and the action parameters source.
    PlotMSActionOnCacheAxes(Type type, PlotMSActionParameters* params);
    
    // Destructor.
    ~PlotMSActionOnCacheAxes();
    
    
    // Implements PlotMSAction::type().
    Type type() const { return itsType_; }
    
    // Implements PlotMSAction::isValid().
    bool isValid() const;
    
    // Implements PlotMSAction::setParameters().
    void setParameters(PlotMSActionParameters* params);    
    
    // Implements PlotMSAction::doAction().  See documentation for
    // PlotMSAction::Type enum.
    void doAction(PlotMS* plotms);
    
private:
    // Type.
    Type itsType_;
    
    // Plot.
    PlotMSPlot* itsPlot_;
    
    // Axes to load or release.
    vector<PMS::Axis> itsAxes_;
};


// Action class that acts on the whole plotter.
class PlotMSActionOnPlotter : public PlotMSAction {
public:
    // Constructor which takes the action type (must be one of the plotter
    // actions).  No parameters are needed.
    PlotMSActionOnPlotter(Type type);
    
    // Destructor.
    ~PlotMSActionOnPlotter();
    
    
    // Implements PlotMSAction::type().
    Type type() const { return itsType_; }
    
    // Implements PlotMSAction::isValid().
    bool isValid() const { return true; }
    
    // Implements PlotMSAction::setParameters().
    void setParameters(PlotMSActionParameters* params) { }
    
    // Implements PlotMSAction::doAction().  See documentation for
    // PlotMSAction::Type enum.
    void doAction(PlotMS* plotms);
    
private:
    // Type.
    Type itsType_;
};

}

#endif /* PLOTMSACTION_H_ */
