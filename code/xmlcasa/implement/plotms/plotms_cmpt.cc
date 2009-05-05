//# plotms_cmpt.cc: PlotMS component tool.
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
#include <xmlcasa/plotms/plotms_cmpt.h>

#include <casaqt/PlotterImplementations/PlotterImplementations.h>
#include <xmlcasa/StdCasa/CasacSupport.h>

namespace casac {

////////////////////////
// PLOTMS DEFINITIONS //
////////////////////////

// Constructors/Destructors //

plotms::plotms() : itsCurrentPlotMS_(NULL), itsWatcher_(this) { }

plotms::~plotms() {
    if(itsCurrentPlotMS_) delete itsCurrentPlotMS_;
}


// Public Methods //

// convenience macro
#define PC_PMSP                                                               \
    PlotMSParameters& p = (itsCurrentPlotMS_ != NULL) ?                       \
            itsCurrentPlotMS_->getParameters() : itsParameters_;

void plotms::setLogLevel(const string& logLevel, const bool logDebug) {
    bool ok;
    PlotMSLogger::Level level = PlotMSLogger::level(logLevel, &ok);

    // use current log level if invalid
    if(!ok) level = PlotMSLogger::level(getLogLevel());

    PC_PMSP
    p.setLogLevel(level, logDebug);
}

string plotms::getLogLevel() {
    const PC_PMSP
    return PlotMSLogger::level(p.logLevel());
}

bool plotms::getLogDebug() {
    const PC_PMSP
    return p.logDebug();
}

void plotms::setClearSelectionOnAxesChange(const bool clearSelection) {
    PC_PMSP
    p.setClearSelectionsOnAxesChange(clearSelection);
}

bool plotms::getClearSelectionOnAxesChange() {
    const PC_PMSP
    return p.clearSelectionsOnAxesChange();
}

void plotms::setCachedImageSize(const int width, const int height) {
    PC_PMSP
    p.setCachedImageSize(width, height);
}

void plotms::setCachedImageSizeToScreenResolution() {
    PC_PMSP
    p.setCachedImageSizeToResolution();
}

int plotms::getCachedImageWidth() {
    const PC_PMSP
    return p.cachedImageSize().first;
}

int plotms::getCachedImageHeight() {
    const PC_PMSP
    return p.cachedImageSize().second;
}


void plotms::setPlotMSFilename(const string& msFilename2,
        const bool updateImmediately, const int plotIndex2) {
    int plotIndex = plotIndex2;
    bool resized = plotParameters(plotIndex);
    PlotMSSinglePlotParameters& p = itsPlotParameters_[plotIndex];
    String msFilename(msFilename2);
    if(!resized && p.filename() == msFilename) return;
    p.setFilename(msFilename);
    if(updateImmediately && itsCurrentPlotMS_ != NULL &&
       itsCurrentPlotMS_->guiShown()) {
        if(resized)
            itsCurrentPlotMS_->addSinglePlot(&p);
        else
            *itsCurrentPlotMS_->getPlotManager().singlePlotParameters(
                    plotIndex) = p;
    }
}

string plotms::getPlotMSFilename(const int plotIndex) {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size()) return "";
    else return itsPlotParameters_[plotIndex].filename();
}

void plotms::setPlotMSSelection(const string& field, const string& spw,
        const string& timerange, const string& uvrange,
        const string& antenna, const string& scan, const string& corr,
        const string& array, const string& msselect,
        const bool updateImmediately, const int plotIndex) {
    PlotMSSelection sel;
    sel.setField(field);
    sel.setSpw(spw);
    sel.setTimerange(timerange);
    sel.setUvrange(uvrange);
    sel.setAntenna(antenna);
    sel.setScan(scan);
    sel.setCorr(corr);
    sel.setArray(array);
    sel.setMsselect(msselect);
    setPlotMSSelection(sel, updateImmediately, plotIndex);
}

void plotms::setPlotMSSelectionRec(const record& selection,
		const bool updateImmediately, const int plotIndex) {
    PlotMSSelection sel;
    sel.fromRecord(*toRecord(selection));
    setPlotMSSelection(sel, updateImmediately, plotIndex);
}

record* plotms::getPlotMSSelection(const int plotIndex) {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size())
        return new record();
    else
    	return fromRecord(
    			itsPlotParameters_[plotIndex].selection().toRecord());
}

void plotms::setPlotMSAveraging(const bool channel, const double channelValue,
        const bool time, const double timeValue, const bool scan,
        const bool field, const bool baseline, const bool antenna,
        const bool spw, const bool updateImmediately, const int plotIndex) {
    PlotMSAveraging avg;
    avg.setChannel(channel);
    avg.setChannelValue(channelValue);
    avg.setTime(time);
    avg.setTimeValue(timeValue);
    avg.setScan(scan);
    avg.setField(field);
    avg.setBaseline(baseline);
    avg.setAntenna(antenna);
    avg.setSpw(spw);
    setPlotMSAveraging(avg, updateImmediately, plotIndex);
}

void plotms::setPlotMSAveragingRec(const record& averaging,
		const bool updateImmediately, const int plotIndex) {
    PlotMSAveraging avg;
    avg.fromRecord(*toRecord(averaging));
    setPlotMSAveraging(avg, updateImmediately, plotIndex);
}

record* plotms::getPlotMSAveraging(const int plotIndex) {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size())
        return new record();
    else
    	return fromRecord(
    			itsPlotParameters_[plotIndex].averaging().toRecord());
}


void plotms::setPlotXAxis(const string& xAxis, const string& xDataColumn,
		const bool updateImmediately, const int plotIndex) {
    bool ok;
    PMS::Axis axis = PMS::axis(xAxis, &ok);
    if(!ok) return;
    PMS::DataColumn data = PMS::dataColumn(xDataColumn, &ok);
    if(!ok) data = PMS::DEFAULT_DATACOLUMN;
    setPlotAxes(axis, axis, data, data, true, false, updateImmediately,
                plotIndex);
}

void plotms::setPlotYAxis(const string& yAxis, const string& yDataColumn,
		const bool updateImmediately, const int plotIndex) {
    bool ok;
    PMS::Axis axis = PMS::axis(yAxis, &ok);
    if(!ok) return;
    PMS::DataColumn data = PMS::dataColumn(yDataColumn, &ok);
    if(!ok) data = PMS::DEFAULT_DATACOLUMN;
    setPlotAxes(axis, axis, data, data, false, true, updateImmediately,
                plotIndex);
}

void plotms::setPlotAxes(const string& xAxis, const string& yAxis,
        const string& xDataColumn, const string& yDataColumn,
        const bool updateImmediately, const int plotIndex) {
    bool useX, useY, ok;
    PMS::Axis x = PMS::axis(xAxis, &useX), y = PMS::axis(yAxis, &useY);
    if(!useX && !useY) return;
    PMS::DataColumn xd = PMS::dataColumn(xDataColumn, &ok);
    if(!ok) xd = PMS::DEFAULT_DATACOLUMN;
    PMS::DataColumn yd = PMS::dataColumn(yDataColumn, &ok);
    if(!ok) yd = PMS::DEFAULT_DATACOLUMN;
    setPlotAxes(x, y, xd, yd, useX, useY, updateImmediately, plotIndex);
}

string plotms::getPlotXAxis(const int plotIndex) {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size()) return "";
    else return PMS::axis(itsPlotParameters_[plotIndex].xAxis());
}

string plotms::getPlotXDataColumn(const int plotIndex) {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size()) return "";
    else return PMS::dataColumn(itsPlotParameters_[plotIndex].xDataColumn());
}

string plotms::getPlotYAxis(const int plotIndex) {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size()) return "";
    else return PMS::axis(itsPlotParameters_[plotIndex].yAxis());
}

string plotms::getPlotYDataColumn(const int plotIndex) {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size()) return "";
    else return PMS::dataColumn(itsPlotParameters_[plotIndex].yDataColumn());
}


void plotms::update() {
    if(itsCurrentPlotMS_ == NULL) return;

    unsigned int n = itsCurrentPlotMS_->getPlotManager().plotParameters()
                     .size();

    // check for added plots
    if(itsPlotParameters_.size() > n) {
        vector<PlotMSSinglePlotParameters> v(itsPlotParameters_.size() -
                n, PlotMSSinglePlotParameters(plotterImplementation()));
        for(unsigned int i = 0; i < v.size(); i++)
            v[i] = itsPlotParameters_[i + n];
        for(unsigned int i = 0; i < v.size(); i++)
            itsCurrentPlotMS_->addSinglePlot(&v[i]);
    }

    PlotMSSinglePlotParameters* p;
    for(unsigned int i = 0; i < n; i++) {
        p = itsCurrentPlotMS_->getPlotManager().singlePlotParameters(i);
        if(p == NULL) continue;
        if(*p != itsPlotParameters_[i]) *p = itsPlotParameters_[i];
    }
}

int plotms::execLoop() {
    if(itsCurrentPlotMS_ == NULL) {
        itsCurrentPlotMS_ = new PlotMS(itsParameters_);
        PlotMSPlotManager& m = itsCurrentPlotMS_->getPlotManager();
        m.addWatcher(&itsWatcher_);

        itsCurrentPlotMS_->showGUI(true);

        if(itsPlotParameters_.size() == 0)
            itsPlotParameters_.resize(1,
                    PlotMSSinglePlotParameters(plotterImplementation()));

        for(unsigned int i = 0; i < itsPlotParameters_.size(); i++) {
            itsCurrentPlotMS_->addSinglePlot(&itsPlotParameters_[i]);
            m.plot(i)->parameters().addWatcher(&itsWatcher_);
        }

    } else {
        itsCurrentPlotMS_->showGUI(true);
        update();
    }

    return itsCurrentPlotMS_->execLoop();
}


/*
bool plotms::exportPlot(const string& filename, const bool highResolution,
		const int dpi, const int width, const int height, const int plotIndex){
	if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size() ||
	   itsCurrentPlotMS_ == NULL) return false; // TODO

	PlotMSAction action(PlotMSAction::PLOT_EXPORT);
	action.setParameter(PlotMSAction::P_FILE, filename);
	action.setParameter(PlotMSAction::P_HIGHRES, highResolution);
	action.setParameter(PlotMSAction::P_DPI, dpi);
	action.setParameter(PlotMSAction::P_WIDTH, width);
	action.setParameter(PlotMSAction::P_HEIGHT, height);
	action.setParameter(PlotMSAction::P_PLOT,
			itsCurrentPlotMS_->getPlotManager().plot(plotIndex));

	// TODO

	return false;
}
*/


// Private Methods //

void plotms::parametersHaveChanged(const PlotMSWatchedParameters& params,
        int updateFlag, bool redrawRequired) {
    if(&params == &itsCurrentPlotMS_->getParameters()) {
        itsParameters_ = dynamic_cast<const PlotMSParameters&>(params);

    } else {
        const vector<PlotMSPlotParameters*>& p =
            itsCurrentPlotMS_->getPlotManager().plotParameters();
        unsigned int index = 0;
        for(; index < p.size(); index++) if(&params == p[index]) break;
        if(index >= itsPlotParameters_.size()) return; // shouldn't happen
        itsPlotParameters_[index] =
            *dynamic_cast<const PlotMSSinglePlotParameters*>(p[index]);
    }
}

void plotms::plotsChanged(const PlotMSPlotManager& manager) {
    const vector<PlotMSPlotParameters*>& p = manager.plotParameters();
    itsPlotParameters_.resize(p.size(), PlotMSSinglePlotParameters(
            plotterImplementation()));
    for(unsigned int i = 0; i < p.size(); i++)
        itsPlotParameters_[i] =
            *dynamic_cast<const PlotMSSinglePlotParameters*>(p[i]);
}

bool plotms::plotParameters(int& plotIndex) const {
    if(plotIndex < 0) plotIndex = 0;
    if((unsigned int)plotIndex > itsPlotParameters_.size())
        plotIndex = itsPlotParameters_.size();

    bool resized = false;
    if((unsigned int)plotIndex >= itsPlotParameters_.size()) {
        resized = true;
        const_cast<plotms*>(this)->itsPlotParameters_.resize(
                plotIndex + 1,
                PlotMSSinglePlotParameters(plotterImplementation()));
    }

    return resized;
}

void plotms::setPlotMSSelection(const PlotMSSelection& selection,
        bool updateImmediately, int plotIndex) {
    bool resized = plotParameters(plotIndex);
    PlotMSSinglePlotParameters& p = itsPlotParameters_[plotIndex];
    if(!resized && p.selection() == selection) return;
    p.setSelection(selection);
    if(updateImmediately && itsCurrentPlotMS_ != NULL &&
       itsCurrentPlotMS_->guiShown()) {
        if(resized)
            itsCurrentPlotMS_->addSinglePlot(&p);
        else
            *itsCurrentPlotMS_->getPlotManager().singlePlotParameters(
                    plotIndex) = p;
    }
}

void plotms::setPlotMSAveraging(const PlotMSAveraging& averaging,
        bool updateImmediately, int plotIndex) {
    bool resized = plotParameters(plotIndex);
    PlotMSSinglePlotParameters& p = itsPlotParameters_[plotIndex];
    if(!resized && p.averaging() == averaging) return;
    p.setAveraging(averaging);
    if(updateImmediately && itsCurrentPlotMS_ != NULL &&
       itsCurrentPlotMS_->guiShown()) {
        if(resized)
            itsCurrentPlotMS_->addSinglePlot(&p);
        else
            *itsCurrentPlotMS_->getPlotManager().singlePlotParameters(
                    plotIndex) = p;
    }
}

void plotms::setPlotAxes(PMS::Axis xAxis, PMS::Axis yAxis,
        PMS::DataColumn xDataColumn, PMS::DataColumn yDataColumn, bool useX,
        bool useY, bool updateImmediately, int plotIndex) {
    if(!useX && !useY) return;
    bool resized = plotParameters(plotIndex);
    PlotMSSinglePlotParameters& p = itsPlotParameters_[plotIndex];
    if(!resized && ((useX && xAxis == p.xAxis() &&
       xDataColumn == p.xDataColumn()) || (useY && yAxis == p.yAxis() &&
       yDataColumn == p.yDataColumn()))) return;

    if(useX) {
        p.setXAxis(xAxis);
        p.setXDataColumn(xDataColumn);
    }
    if(useY) {
        p.setYAxis(yAxis);
        p.setYDataColumn(yDataColumn);
    }

    if(updateImmediately && itsCurrentPlotMS_ != NULL &&
       itsCurrentPlotMS_->guiShown()) {
        if(resized)
            itsCurrentPlotMS_->addSinglePlot(&p);
        else
            *itsCurrentPlotMS_->getPlotManager().singlePlotParameters(
                    plotIndex) = p;
    }
}

}
