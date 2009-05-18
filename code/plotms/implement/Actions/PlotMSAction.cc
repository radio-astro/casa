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
#include <plotms/Actions/PlotMSAction.h>

#include <plotms/Actions/PlotMSExportThread.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>

namespace casa {

//////////////////////////////
// PLOTMSACTION DEFINITIONS //
//////////////////////////////

// Static //

const String PlotMSAction::P_PLOT = "plot";
const String PlotMSAction::P_ON_OFF = "on_off";
const String PlotMSAction::P_AXES = "axes";
const String PlotMSAction::P_FILE = "file";
const String PlotMSAction::P_FORMAT = "format";
const String PlotMSAction::P_HIGHRES = "highres";
const String PlotMSAction::P_DPI = "dpi";
const String PlotMSAction::P_WIDTH = "width";
const String PlotMSAction::P_HEIGHT = "height";

bool PlotMSAction::requires(Type type, const String& parameter) {
    switch(type) {
    case TOOL_MARK_REGIONS: case TOOL_ZOOM: case TOOL_PAN:
    case TOOL_ANNOTATE_TEXT: case TOOL_ANNOTATE_RECTANGLE: case TRACKER_HOVER:
    case TRACKER_DISPLAY: case HOLD_RELEASE_DRAWING:
        return parameter == P_ON_OFF;

    case CACHE_LOAD: case CACHE_RELEASE:
        return parameter == P_PLOT || parameter == P_AXES;

    case PLOT_EXPORT:
        return parameter == P_PLOT || parameter == P_FILE;
    
    default: return false;
    }
}


// Constructors/Destructors //

PlotMSAction::PlotMSAction(Type type) : itsType_(type) { }

PlotMSAction::~PlotMSAction() { }


// Public Methods //

PlotMSAction::Type PlotMSAction::type() const { return itsType_; }

bool PlotMSAction::isValid() const {
	switch(itsType_) {
	case TOOL_MARK_REGIONS: case TOOL_ZOOM: case TOOL_PAN:
	case TOOL_ANNOTATE_TEXT: case TOOL_ANNOTATE_RECTANGLE: case TRACKER_HOVER:
	case TRACKER_DISPLAY: case HOLD_RELEASE_DRAWING:
		return isDefinedBool(P_ON_OFF);

	case CACHE_LOAD: case CACHE_RELEASE:
		return isDefinedPlot(P_PLOT) && valuePlot(P_PLOT) != NULL &&
		       isDefinedAxes(P_AXES) && valueAxes(P_AXES).size() > 0;

	case PLOT_EXPORT:
		return isDefinedPlot(P_PLOT) && valuePlot(P_PLOT) != NULL &&
		       isDefinedString(P_FILE) && !valueString(P_FILE).empty();

	case SEL_FLAG: case SEL_UNFLAG: case SEL_LOCATE: case SEL_CLEAR_REGIONS:
	case ITER_FIRST: case ITER_PREV: case ITER_NEXT: case ITER_LAST:
	case STACK_BACK: case STACK_BASE: case STACK_FORWARD: case CLEAR_PLOTTER:
	case QUIT:
		return true;

	default: return false;
	}
}

void PlotMSAction::setParameter(const String& parameter, PlotMSPlot* value) {
	itsPlotValues_[parameter] = value; }
void PlotMSAction::setParameter(const String& parameter, bool value) {
	itsBoolValues_[parameter] = value; }
void PlotMSAction::setParameter(const String& p, const vector<PMS::Axis>& v) {
	itsAxesValues_[p] = v; }
void PlotMSAction::setParameter(const String& parameter, const String& value) {
	itsStringValues_[parameter] = value; }
void PlotMSAction::setParameter(const String& parameter, int value) {
	itsIntValues_[parameter] = value; }

#define PMSA_PRINTPARAMS1(TYPE,MEMBER)                                        \
    for(map<String, TYPE >::iterator iter = MEMBER .begin();                  \
        iter != MEMBER .end(); iter++) {

#define PMSA_PRINTPARAMS2 cout << ", " << iter->first << "=";
#define PMSA_PRINTPARAMS3 cout << iter->second;
#define PMSA_PRINTPARAMS4 }

#define PMSA_PRINTPARAMS(TYPE,MEMBER)                                         \
    PMSA_PRINTPARAMS1(TYPE,MEMBER)                                            \
    PMSA_PRINTPARAMS2                                                         \
    PMSA_PRINTPARAMS3                                                         \
    PMSA_PRINTPARAMS4

bool PlotMSAction::doAction(PlotMS* plotms) {
	itsDoActionResult_ = "";

	if(!isValid() || plotms == NULL) {
		itsDoActionResult_ = "Set parameters were not valid!";
		return false;
	}

	/*
	cout << "Executing action: {type=" << itsType_;
	
	PMSA_PRINTPARAMS(PlotMSPlot*, itsPlotValues_)
	PMSA_PRINTPARAMS(bool, itsBoolValues_)
	PMSA_PRINTPARAMS(String, itsStringValues_)
	PMSA_PRINTPARAMS(int, itsIntValues_)
	
	PMSA_PRINTPARAMS1(vector<PMS::Axis>, itsAxesValues_)
        PMSA_PRINTPARAMS2
        cout << "[";
        for(unsigned int i = 0; i < iter->second.size(); i++) {
            if(i > 0) cout << ", ";
            cout << PMS::axis(iter->second[i]);
        }
        cout << "]";
	PMSA_PRINTPARAMS4
	
	cout << "}" << endl;
	*/
	
	switch(itsType_) {
	
	case SEL_FLAG: case SEL_UNFLAG: case SEL_LOCATE: {
	    const vector<PlotMSPlot*>& plots = plotms->getPlotManager().plots();
	    vector<PlotCanvasPtr> visibleCanv = plotms->getPlotter()
                                            ->currentCanvases();
	    PlotMSSinglePlot* plot;

	    stringstream ss;
	    stringstream tempx, tempy, temp;
	    bool ssFlag = false;
	    PlotLogMessage* m;
	    LogMessage::Priority p = LogMessage::NORMAL;
	    
	    // TODO !distance from GUI
	    PlotMSFlagging flagging = plotms->getPlotter()->getFlaggingTab()
	                              ->getValue();

	    vector<PlotCanvasPtr> canv;
	    vector<PlotRegion> regions;
	    vector<PlotMSSinglePlot*> redrawPlots;
	    bool found;

	    for(unsigned int i = 0; i < plots.size(); i++) {
	        // NOTE: for now, only works with single plots
	        plot = dynamic_cast<PlotMSSinglePlot*>(plots[i]);
	        if(plot == NULL) continue;

	        if(i > 0) ss.str("");

	        PlotMSSinglePlotParameters& params = plot->singleParameters();
	        PlotMSData& data = plot->data();

            if(itsType_ == SEL_FLAG)
                flagging.setMS(&plot->ms(),&plot->selectedMS(),plot->visSet());

	        canv = plot->canvases();
	        for(unsigned int j = 0; j < canv.size(); j++) {
	            if(canv[j].null()) continue;

	            // Only apply to visible canvases.
	            found = false;
	            for(unsigned int k = 0; !found && k < visibleCanv.size(); k++)
	                if(canv[j] == visibleCanv[k]) found = true;
	            if(!found) continue;

	            regions = canv[j]->standardMouseTools()->selectTool()
	                      ->getSelectedRects();
	            if(regions.size() == 0) continue;

	            if(j > 0) {
	                tempx.str(""); tempy.str(""); temp.str("");
	            }

	            for(unsigned int k = 0; k < regions.size(); k++) {
	                if(k > 0) {
	                    tempx << " or ";
	                    tempy << " or ";
	                    temp << '\n';
	                }
	                tempx << '[' << regions[k].left() << ' '
	                      << regions[k].right()	<< ']';
	                tempy << '[' << regions[k].bottom() << ' '
	                      << regions[k].top() << ']';
                    m = NULL;
                    if(itsType_ == SEL_LOCATE) {
                        m = data.locateRange(regions[k].left(),
                                regions[k].right(), regions[k].bottom(),
                                regions[k].top());
                    } else {                        
                        m = data.flagRange(flagging, regions[k].left(),
                                regions[k].right(), regions[k].bottom(),
                                regions[k].top(), itsType_ == SEL_FLAG);
                    }
                    if(m != NULL) {
                        m->message(temp);
                        p = m->priority();
                        delete m;
                    }
	            }

	            if(!temp.str().empty()) {
	                if(ssFlag) ss << "\n\n";
	                else       ssFlag = true;

	                if(itsType_ == SEL_LOCATE) ss << "Locate ";
                    else if(itsType_ == SEL_FLAG) ss << "Flag ";
                    else ss << "Unflag ";
                    ss << PMS::axis(params.xAxis()) << " in " << tempx.str()
                       << ", " << PMS::axis(params.yAxis()) << " in "
                       << tempy.str() << ":\n" << temp.str();
	            }
	            
	            // If this plot was flagged/unflagged, add it to the redraw
	            // list.
	            if(itsType_ == SEL_FLAG || itsType_ == SEL_UNFLAG)
	                redrawPlots.push_back(plot);
	        }

	        if(!ss.str().empty()) {
                const String* method = NULL;
                if(itsType_ == SEL_LOCATE) method = &PlotMS::LOG_LOCATE;
                else if(itsType_ == SEL_FLAG) method = &PlotMS::LOG_FLAG;
                else if(itsType_ == SEL_UNFLAG) method = &PlotMS::LOG_UNFLAG;
                plotms->getLogger().postMessage(PlotMS::CLASS_NAME, *method,
                        ss.str(), p);
	        }
	        
	        // For a flag/unflag, need to tell the plots to redraw themselves,
	        // and clear selected regions.
	        if(itsType_ == SEL_FLAG || itsType_ == SEL_UNFLAG) {
	            bool hold = plotms->getPlotter()->allDrawingHeld();
	            if(!hold) plotms->getPlotter()->holdDrawing();
	            
	            for(unsigned int i = 0; i < redrawPlots.size(); i++) {
	                redrawPlots[i]->plotDataChanged();
	                
	                canv = plot->canvases();
	                for(unsigned int j = 0; j < canv.size(); j++) {
	                    if(canv[j].null()) continue;
	                    
	                    // Only apply to visible canvases.
	                     found = false;
	                     for(unsigned int k = 0;
	                         !found && k < visibleCanv.size(); k++)
	                         if(canv[j] == visibleCanv[k]) found = true;
	                     if(!found) continue;
	                     
	                     canv[j]->standardMouseTools()->selectTool()
	                            ->clearSelectedRects();
	                }
	            }	            
	            
	            if(!hold) plotms->getPlotter()->releaseDrawing();
	        }
	    }
	    return true;
	}

	case SEL_CLEAR_REGIONS: {
		const vector<PlotMSPlot*>& plots = plotms->getPlotManager().plots();
		vector<PlotCanvasPtr> visibleCanv = plotms->getPlotter()
		                                    ->currentCanvases();
		vector<PlotCanvasPtr> canv;
		bool found;

		for(unsigned int i = 0; i < plots.size(); i++) {
			if(plots[i] == NULL) continue;
			canv = plots[i]->canvases();
			for(unsigned int j = 0; j < canv.size(); j++) {
				if(canv[j].null()) continue;

				// Only apply to visible canvases.
				found = false;
				for(unsigned int k = 0; !found && k < visibleCanv.size(); k++)
					if(canv[j] == visibleCanv[k]) found = true;
				if(!found) continue;

				canv[j]->standardMouseTools()->selectTool()
				       ->clearSelectedRects();
			}
		}

		return true;
	}

	case TOOL_MARK_REGIONS:	case TOOL_ZOOM:	case TOOL_PAN:
	case TOOL_ANNOTATE_TEXT: case TOOL_ANNOTATE_RECTANGLE: {
		bool on = valueBool(P_ON_OFF);
	    PlotStandardMouseToolGroup::Tool tool=PlotStandardMouseToolGroup::NONE;
	    if(on && itsType_ == TOOL_MARK_REGIONS)
	    	tool = PlotStandardMouseToolGroup::SELECT;
	    else if(on && itsType_ == TOOL_ZOOM)
	    	tool = PlotStandardMouseToolGroup::ZOOM;
	    else if(on && itsType_ == TOOL_PAN)
	    	tool = PlotStandardMouseToolGroup::PAN;
	    
	    bool useAnnotator = on && (itsType_ == TOOL_ANNOTATE_TEXT ||
	                        itsType_ == TOOL_ANNOTATE_RECTANGLE);
	    PlotMSAnnotator::Mode annotate = PlotMSAnnotator::TEXT;
	    if(on && itsType_ == TOOL_ANNOTATE_RECTANGLE)
	        annotate = PlotMSAnnotator::RECTANGLE;
	    
	    const vector<PlotMSPlot*>& plots = plotms->getPlotManager().plots();
	    vector<PlotCanvasPtr> canv;
	    for(unsigned int i = 0; i < plots.size(); i++) {
	    	if(plots[i] == NULL) continue;
	    	canv = plots[i]->canvases();
	    	for(unsigned int j = 0; j < canv.size(); j++) {
	    		if(canv[j].null()) continue;
	    		
	    		// Update standard mouse tools.
	    		canv[j]->standardMouseTools()->setActive(!useAnnotator);
	    		if(!useAnnotator)
	    		    canv[j]->standardMouseTools()->setActiveTool(tool);
	    	}
	    }
	    
	    // Update annotator.
	    PlotMSPlotter* plotter = plotms->getPlotter();
	    plotter->getAnnotator().setActive(useAnnotator);
	    if(useAnnotator) plotter->getAnnotator().setDrawingMode(annotate);

	    return true;
	}

	case TRACKER_HOVER:	case TRACKER_DISPLAY: {
	    PlotMSPlotter* plotter = plotms->getPlotter();
	    bool hover = itsType_ == TRACKER_HOVER ? valueBool(P_ON_OFF) :
                     plotter->actionIsChecked(TRACKER_HOVER),
           display = itsType_ == TRACKER_DISPLAY ? valueBool(P_ON_OFF) :
	                 plotter->actionIsChecked(TRACKER_DISPLAY);

   	    const vector<PlotMSPlot*>& plots = plotms->getPlotManager().plots();
   	    vector<PlotCanvasPtr> canv;
   	    for(unsigned int i = 0; i < plots.size(); i++) {
   	    	if(plots[i] == NULL) continue;
   	    	canv = plots[i]->canvases();
   	    	for(unsigned int j = 0; j < canv.size(); j++) {
   	    		if(canv[j].null()) continue;
   		    	canv[i]->standardMouseTools()->turnTracker(hover | display);
   		    	canv[i]->standardMouseTools()->turnTrackerDrawText(hover);
   	    	}
   	    }

	    return true;
	}

	case STACK_BACK: case STACK_BASE: case STACK_FORWARD: {
	    int delta = 0;
	    if(itsType_ == STACK_BACK)         delta = -1;
	    else if(itsType_ == STACK_FORWARD) delta = 1;
	    vector<PlotCanvasPtr> canv = plotms->getPlotter()->currentCanvases();
	    for(unsigned int i = 0; i < canv.size(); i++) {
	        if(canv[i].null()) continue;
	        canv[i]->axesStackMove(delta);
	    }
	    return true;
	}

	case CACHE_LOAD: case CACHE_RELEASE: {
		PlotMSPlot* plot = valuePlot(P_PLOT);
		vector<PMS::Axis> axes = valueAxes(P_AXES);

	    PlotMSPlotParameters& params = plot->parameters();
	    if(!params.isSet()) {
	    	itsDoActionResult_ = "MS has not been loaded into the cache!";
	    	return false;
	    }

	    PlotMSData& data = plot->data();
	    vector<PMS::Axis> a;

	    // Remove any duplicates or axes.  If loading, also make sure that the
	    // given axes are not already loaded.  If releasing, make sure that the
	    // axes are loaded.
	    vector<pair<PMS::Axis, unsigned int> > loaded = data.loadedAxes();
	    bool valid;
	    for(unsigned int i = 0; i < axes.size(); i++) {
	        valid = true;
	        for(unsigned int j = 0; valid && j < a.size(); j++)
	            if(a[j] == axes[i]) valid = false;

	        if(valid) {
	            if(itsType_ == CACHE_LOAD) {
	                for(unsigned int j = 0; valid && j < loaded.size(); j++)
	                    if(loaded[j].first == axes[i]) valid = false;
	            } else {
	                valid = false;
	                for(unsigned int j = 0; !valid && j < loaded.size(); j++)
	                    if(loaded[j].first == axes[i]) valid = true;
	            }
	        }

	        if(valid) a.push_back(axes[i]);
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
	            data.loadCache(*plot->visSet(), a, vector<PMS::DataColumn>(
	                           a.size(), PMS::DEFAULT_DATACOLUMN),
	                           plot->parameters().averaging());

	            // Notify watchers that cache has changed.  No redraw required.
	            params.notifyWatchers(PlotMSWatchedParameters::CACHE, false);

	        } else {
	            itsDoActionResult_ = "Action type is currently unimplemented!";
	            return false;
	        }
	    }

	    return true;
	}

	case PLOT_EXPORT: {
	    const String& file = valueString(P_FILE);
	    String form = isDefinedString(P_FORMAT) ? valueString(P_FORMAT) : "";
	    bool ok;
	    PlotExportFormat::Type t = PlotExportFormat::exportFormat(form, &ok);
	    if(!ok) {
	        t = PlotExportFormat::typeForExtension(file, &ok);
	        if(!ok) {
	        	itsDoActionResult_ = "Invalid format extension for filename '"+
	        	                     file + "'!";
	            return false;
	        }
	    }

	    PlotExportFormat format(t, file);
	    format.resolution = isDefinedBool(P_HIGHRES) && valueBool(P_HIGHRES) ?
	    	                PlotExportFormat::HIGH : PlotExportFormat::SCREEN;
	    format.dpi = isDefinedInt(P_DPI) ? valueInt(P_DPI) : -1;
	    if(format.dpi <= 0) format.dpi = -1;
	    format.width = isDefinedInt(P_WIDTH) ? valueInt(P_WIDTH) : -1;
	    if(format.width <= 0) format.width = -1;
	    format.height = isDefinedInt(P_HEIGHT) ? valueInt(P_HEIGHT) : -1;
	    if(format.height <= 0) format.height = -1;


	    plotms->getPlotter()->doThreadedOperation(
	    		new PlotMSExportThread(valuePlot(P_PLOT), format));

		return true;
	}

	case HOLD_RELEASE_DRAWING: {
        PlotMSPlotter* plotter = plotms->getPlotter();
        bool hold = valueBool(P_ON_OFF);
        if(hold) plotter->holdDrawing();
        else     plotter->releaseDrawing();

        return true;
	}

	case CLEAR_PLOTTER:
		plotms->getPlotManager().clearPlotsAndCanvases();
		return true;

	case QUIT:
		plotms->close();
		return true;

	// Unimplemented actions.
	case ITER_FIRST: case ITER_PREV: case ITER_NEXT: case ITER_LAST:
		itsDoActionResult_ = "Action type is currently unimplemented!";
		return false;
	}

	itsDoActionResult_ = "Unknown action type!";
	return false;
}

const String& PlotMSAction::doActionResult() const{ return itsDoActionResult_;}


// Private Methods //

bool PlotMSAction::isDefinedPlot(const String& parameter) const {
	return itsPlotValues_.find(parameter) != itsPlotValues_.end(); }
bool PlotMSAction::isDefinedBool(const String& parameter) const {
	return itsBoolValues_.find(parameter) != itsBoolValues_.end(); }
bool PlotMSAction::isDefinedString(const String& parameter) const {
	return itsStringValues_.find(parameter) != itsStringValues_.end(); }
bool PlotMSAction::isDefinedInt(const String& parameter) const {
	return itsIntValues_.find(parameter) != itsIntValues_.end(); }
bool PlotMSAction::isDefinedAxes(const String& parameter) const {
	return itsAxesValues_.find(parameter) != itsAxesValues_.end(); }

PlotMSPlot* PlotMSAction::valuePlot(const String& parameter) {
	return itsPlotValues_[parameter]; }
const PlotMSPlot* PlotMSAction::valuePlot(const String& parameter) const {
	return const_cast<map<String, PlotMSPlot*>&>(itsPlotValues_)[parameter]; }
bool PlotMSAction::valueBool(const String& parameter) const {
	return const_cast<map<String, bool>&>(itsBoolValues_)[parameter]; }
const String& PlotMSAction::valueString(const String& parameter) const {
	return const_cast<map<String, String>&>(itsStringValues_)[parameter]; }
int PlotMSAction::valueInt(const String& parameter) const {
	return const_cast<map<String, int>&>(itsIntValues_)[parameter]; }
const vector<PMS::Axis>& PlotMSAction::valueAxes(const String& p) const {
	return const_cast<map<String, vector<PMS::Axis> >&>(itsAxesValues_)[p]; }

}
