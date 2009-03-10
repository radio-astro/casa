//# PlotMSAction.cc: Actions on plotms that can be triggered.
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
#include <plotms/PlotMS/PlotMSAction.h>

#include <plotms/PlotMS/PlotMS.h>
#include <plotms/PlotMS/PlotMSPlot.h>

namespace casa {

//////////////////////////////
// PLOTMSACTION DEFINITIONS //
//////////////////////////////

PlotMSAction* PlotMSAction::action(Type type, PlotMSActionParameters* p) {
    switch(type) {
    case FLAG: case UNFLAG: case LOCATE: case CLEAR_REGIONS:
        return new PlotMSActionOnSelection(type , p);
    
    case ITER_FIRST: case ITER_PREV: case ITER_NEXT: case ITER_LAST:
        return new PlotMSActionOnIteration(type, p);
    
    case TOOL_MARK_REGIONS: case TOOL_ZOOM: case TOOL_PAN: case TRACKER_HOVER:
    case TRACKER_DISPLAY:
        return new PlotMSActionOnTools(type, p);

    case STACK_BACK: case STACK_BASE: case STACK_FORWARD:
        return new PlotMSActionOnStack(type, p);
    
    case CACHE_LOAD: case CACHE_RELEASE:
        return new PlotMSActionOnCacheAxes(type, p);
    
    case HOLD_RELEASE_DRAWING: case CLEAR_PLOTTER:  case QUIT:
        return new PlotMSActionOnPlotter(type);
        
    case CUSTOM: return NULL;
    }
    
    return NULL;
}


/////////////////////////////////////////
// PLOTMSACTIONONSELECTION DEFINITIONS //
/////////////////////////////////////////

PlotMSActionOnSelection::PlotMSActionOnSelection(Type type,
        PlotMSActionParameters* params) : itsType_(type), itsPlot_(NULL) {
    setParameters(params); }

PlotMSActionOnSelection::~PlotMSActionOnSelection() { }


bool PlotMSActionOnSelection::isValid() const {
    return itsPlot_ != NULL && (itsType_ == FLAG || itsType_ == UNFLAG ||
           itsType_ == LOCATE || itsType_ == CLEAR_REGIONS);
}

void PlotMSActionOnSelection::setParameters(PlotMSActionParameters* p) {
    if(p == NULL) return;
    itsPlot_ = p->actionPlot(itsType_);
    itsRegions_.clear();
    if(itsPlot_ != NULL) {
        vector<PlotCanvasPtr> canv = itsPlot_->canvases();
        for(unsigned int i = 0; i < canv.size(); i++) {
            if(canv[i].null()) continue;
            itsRegions_[&*canv[i]]= canv[i]->standardMouseTools()->selectTool()
                                    ->getSelectedRects();
        }
    }
}

void PlotMSActionOnSelection::doAction(PlotMS* plotms) {
    if(!isValid() || plotms == NULL) return;
    
    if(itsType_ == LOCATE) {
        // NOTE: for now, only works with single plots
        PlotMSSinglePlot* plot = dynamic_cast<PlotMSSinglePlot*>(itsPlot_);
        if(plot == NULL) return;
        
        stringstream ss;
        stringstream tempx, tempy, temp;
        bool ssFlag = false;
        PlotLogMessage* m;
        LogMessage::Priority p = LogMessage::NORMAL;        

        PlotMSSinglePlotParameters& params = plot->singleParameters();
        PlotMSData& data = plot->data();
        
        for(map<PlotCanvas*,vector<PlotRegion> >::iterator iter =
            itsRegions_.begin(); iter != itsRegions_.end(); iter++) {
            if(iter->second.size() == 0) continue;
            
            if(iter != itsRegions_.begin()) {
                tempx.str(""); tempy.str(""); temp.str("");
            }
            
            for(unsigned int i = 0; i < iter->second.size(); i++) {
                if(i > 0) {
                    tempx << " or ";
                    tempy << " or ";
                    temp << '\n';
                }
                tempx << '[' << iter->second[i].left() << ' '
                      << iter->second[i].right() << ']';
                tempy << '[' << iter->second[i].bottom() << ' '
                      << iter->second[i].top() << ']';
                m = data.locate(iter->second[i].left(),iter->second[i].right(),
                        iter->second[i].bottom(), iter->second[i].top());
                m->message(temp);
                p = m->priority();
                delete m;
            }
            
            if(ssFlag) ss << "\n\n";
            else       ssFlag = true;
            
            ss << "Locate " << PMS::axis(params.xAxis()) << " in "
               << tempx.str() << ", " << PMS::axis(params.yAxis()) << " in "
               << tempy.str() << ":\n" << temp.str();
        }
        
        plotms->getLogger().postMessage(PlotMS::CLASS_NAME, PlotMS::LOG_LOCATE,
                ss.str(), p);
        
    } else if(itsType_ == FLAG || itsType_ == UNFLAG) {
        /*
        vector<PlotMSRect> v;
        
        unsigned int crow(0), ccol(0);
        vector<double> ulX, ulY, lrX, lrY;
        for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
            crow = i / nCols_;
            ccol = i % nCols_;
            itsCanvases_[i]->standardMouseTools()->selectTool()->
                             getSelectedRects(ulX, ulY, lrX, lrY);
            for(unsigned int j = 0; j < ulX.size(); j++)
                v.push_back(PlotMSRect(ulX[j], ulY[j], lrX[j], lrY[j],
                                       crow, ccol));
        }
        if(v.size() == 0) return;
        
        if(act == actionFlag)        itsParent_->handleFlag(v);
        else if(act == actionUnflag) itsParent_->handleUnflag(v);
        else if(act == actionLocate) itsParent_->handleLocate(v);
         */
        plotms->getLogger().postMessage("PlotMS",
                itsType_ == FLAG ? "flag" : "unflag",
                "Method not yet implemented!");

    } else if(itsType_ == CLEAR_REGIONS) {   
        for(map<PlotCanvas*,vector<PlotRegion> >::iterator iter =
            itsRegions_.begin(); iter != itsRegions_.end(); iter++) {
            iter->first->standardMouseTools()->selectTool()
                ->clearSelectedRects();
        }
    }
}


/////////////////////////////////////////
// PLOTMSACTIONONITERATION DEFINITIONS //
/////////////////////////////////////////

PlotMSActionOnIteration::PlotMSActionOnIteration(Type type,
        PlotMSActionParameters* params) : itsType_(type), itsPlot_(NULL) {
    setParameters(params);
}

PlotMSActionOnIteration::~PlotMSActionOnIteration() { }


bool PlotMSActionOnIteration::isValid() const {
    return itsPlot_ != NULL && (itsType_ == ITER_FIRST ||
           itsType_ == ITER_PREV || itsType_ == ITER_NEXT ||
           itsType_ == ITER_LAST);
}

void PlotMSActionOnIteration::setParameters(PlotMSActionParameters* params) {
    if(params != NULL) itsPlot_ = params->actionPlot(itsType_);
}

void PlotMSActionOnIteration::doAction(PlotMS* plotms) {
    if(!isValid() || plotms == NULL) return;

    plotms->getLogger().postMessage("PlotMS", "iteration",
            "Methods not yet implemented!");
}


/////////////////////////////////////
// PLOTMSACTIONONTOOLS DEFINITIONS //
/////////////////////////////////////

PlotMSActionOnTools::PlotMSActionOnTools(Type type,
        PlotMSActionParameters* params) : itsType_(type), itsPlot_(NULL),
        mark_(false), zoom_(false), pan_(false), trackerHover_(false),
        trackerDisplay_(false) {
    setParameters(params);
}

PlotMSActionOnTools::~PlotMSActionOnTools() { }


bool PlotMSActionOnTools::isValid() const {
    if(itsPlot_ == NULL || (itsType_ != TOOL_MARK_REGIONS &&
       itsType_ != TOOL_ZOOM && itsType_ != TOOL_PAN &&
       itsType_ != TRACKER_HOVER && itsType_ != TRACKER_DISPLAY)) return false;
    if(mark_ && (zoom_ || pan_)) return false;
    if(zoom_ && (mark_ || pan_)) return false;
    if(pan_ && (mark_ || zoom_)) return false;
    return true;
}

void PlotMSActionOnTools::setParameters(PlotMSActionParameters* params) {
    if(params == NULL) return;
    
    itsPlot_ = params->actionPlot(itsType_);
    mark_ = params->actionBool(TOOL_MARK_REGIONS);
    if(mark_) zoom_ = false;
    else      zoom_ = params->actionBool(TOOL_ZOOM);
    if(mark_ || zoom_) pan_ = false;
    else               pan_ = params->actionBool(TOOL_PAN);
    trackerHover_ = params->actionBool(TRACKER_HOVER);
    trackerDisplay_ = params->actionBool(TRACKER_DISPLAY);
}

void PlotMSActionOnTools::doAction(PlotMS* plotms) {
    if(!isValid() || plotms == NULL) return;
    
    PlotStandardMouseToolGroup::Tool tool = PlotStandardMouseToolGroup::NONE;
    if(mark_)      tool = PlotStandardMouseToolGroup::SELECT;
    else if(zoom_) tool = PlotStandardMouseToolGroup::ZOOM;
    else if(pan_)  tool = PlotStandardMouseToolGroup::PAN;
    
    vector<PlotCanvasPtr> canv = itsPlot_->canvases();
    PlotStandardMouseToolGroupPtr tools;
    for(unsigned int i = 0; i < canv.size(); i++) {
        if(canv[i].null()) continue;
        tools = canv[i]->standardMouseTools();
        tools->setActiveTool(tool);
        tools->turnTracker(trackerHover_ || trackerDisplay_);
        tools->turnTrackerDrawText(trackerHover_);
    }
    
    // Update checkable actions in the plotter.
    PlotMSPlotter* plotter = plotms->getPlotter();
    plotter->setActionIsChecked(TOOL_MARK_REGIONS, mark_);
    plotter->setActionIsChecked(TOOL_ZOOM, zoom_);
    plotter->setActionIsChecked(TOOL_PAN, pan_);
    plotter->setActionIsChecked(TRACKER_HOVER, trackerHover_);
    plotter->setActionIsChecked(TRACKER_DISPLAY, trackerDisplay_);
}


/////////////////////////////////////
// PLOTMSACTIONONSTACK DEFINITIONS //
/////////////////////////////////////

PlotMSActionOnStack::PlotMSActionOnStack(Type type,
        PlotMSActionParameters* params) : itsType_(type), itsPlot_(NULL) {
    setParameters(params);
}

PlotMSActionOnStack::~PlotMSActionOnStack() { }


bool PlotMSActionOnStack::isValid() const {
    return itsPlot_ != NULL && (itsType_ == STACK_BACK ||
           itsType_ == STACK_BASE || itsType_ == STACK_FORWARD);
}

void PlotMSActionOnStack::setParameters(PlotMSActionParameters* params) {
    if(params == NULL) return;
    itsPlot_ = params->actionPlot(itsType_);
}

void PlotMSActionOnStack::doAction(PlotMS* plotms) {
    if(!isValid() || plotms == NULL) return;
    
    int delta = 0;
    if(itsType_ == STACK_BACK)         delta = -1;
    else if(itsType_ == STACK_FORWARD) delta = 1;
    vector<PlotCanvasPtr> canv = itsPlot_->canvases();
    for(unsigned int i = 0; i < canv.size(); i++) {
        if(canv[i].null()) continue;
        canv[i]->canvasAxesStackMove(delta);
    }
}


/////////////////////////////////////////
// PLOTMSACTIONONCACHEAXES DEFINITIONS //
/////////////////////////////////////////

PlotMSActionOnCacheAxes::PlotMSActionOnCacheAxes(Type type,
        PlotMSActionParameters* params) : itsType_(type), itsPlot_(NULL) {
    setParameters(params);
}

PlotMSActionOnCacheAxes::~PlotMSActionOnCacheAxes() { }


bool PlotMSActionOnCacheAxes::isValid() const {
    return itsPlot_ != NULL && (itsType_ == CACHE_RELEASE ||
           itsType_ == CACHE_LOAD);
}

void PlotMSActionOnCacheAxes::setParameters(PlotMSActionParameters* params) {
    if(params == NULL) return;
    itsPlot_ = params->actionPlot(itsType_);
    itsAxes_ = params->actionAxes(itsType_);
}

void PlotMSActionOnCacheAxes::doAction(PlotMS* plotms) {
    if(!isValid() || plotms == NULL) return;
    
    PlotMSPlotParameters& params = itsPlot_->parameters();
    if(!params.isSet()) return;
    
    PlotMSData& data = itsPlot_->data();
    vector<PMS::Axis> a;
        
    // Remove any duplicates or axes.  If loading, also make sure that the
    // given axes are not already loaded.  If releasing, make sure that the
    // axes are loaded.
    vector<pair<PMS::Axis, unsigned int> > loaded = data.loadedAxes();
    bool valid;
    for(unsigned int i = 0; i < itsAxes_.size(); i++) {
        valid = true;
        for(unsigned int j = 0; valid && j < a.size(); j++)
            if(a[j] == itsAxes_[i]) valid = false;
            
        if(valid) {
            if(itsType_ == CACHE_LOAD) {
                for(unsigned int j = 0; valid && j < loaded.size(); j++)
                    if(loaded[j].first == itsAxes_[i]) valid = false;
            } else {
                valid = false;
                for(unsigned int j = 0; !valid && j < loaded.size(); j++)
                    if(loaded[j].first == itsAxes_[i]) valid = true;
            }
        }
            
        if(valid) a.push_back(itsAxes_[i]);
    }
        
    // Make sure that meta-data isn't being released (shouldn't happen).
    if(itsType_ == CACHE_RELEASE) {
        bool hasMeta = false;
        for(unsigned int i = 0; !hasMeta && i < a.size(); i++)
            if(PlotMSCache::axisIsMetaData(a[i])) hasMeta = true;
            
        if(hasMeta) {
            bool keepMeta = plotms->getPlotter()->showQuestion("One or more of"
                    " the selected axes are meta-information!  Releasing these"
                    " axes may significantly affect functionality!  Do you "
                    "really want to release these axes?",
                    "Release Meta-Information");
            if(!keepMeta) {
                for(unsigned int i = 0; i < a.size(); i++) {
                    if(PlotMSCache::axisIsMetaData(a[i])) {
                        a.erase(a.begin() + i);
                        i--;
                    }
                }
            }
        }
    }

    if(a.size() > 0) {
        if(itsType_ == CACHE_LOAD) {
            data.loadCache(*itsPlot_->visSet(), a, vector<PMS::DataColumn>(
                           a.size(), PMS::DEFAULT_DATACOLUMN),
                           itsPlot_->parameters().averaging());
            
            // Notify watchers that cache has changed.  No redraw required.
            params.notifyWatchers(PlotMSWatchedParameters::CACHE, false);
            
        } else {
            plotms->getLogger().postMessage("PlotMS", "releaseCache",
                    "Method not yet implemented!");
        }
    }
}


///////////////////////////////////////
// PLOTMSACTIONONPLOTTER DEFINITIONS //
///////////////////////////////////////

PlotMSActionOnPlotter::PlotMSActionOnPlotter(Type type) : itsType_(type) { }

PlotMSActionOnPlotter::~PlotMSActionOnPlotter() { }


void PlotMSActionOnPlotter::doAction(PlotMS* plotms) {
    if(!isValid() || plotms == NULL) return;
    
    if(itsType_ == HOLD_RELEASE_DRAWING) {
        PlotMSPlotter* plotter = plotms->getPlotter();
        bool hold = plotter->actionIsChecked(HOLD_RELEASE_DRAWING);
        if(hold) plotter->holdDrawing();
        else     plotter->releaseDrawing();
        
        // Update checkable actions in the plotter.
        plotter->setActionText(HOLD_RELEASE_DRAWING,
                               hold ? "Release Drawing" : "Hold Drawing");

    } else if(itsType_ == CLEAR_PLOTTER) {
        plotms->getPlotManager().clearPlotsAndCanvases();
        
    } else if(itsType_ == QUIT) {
        plotms->close();
    }
}

}
