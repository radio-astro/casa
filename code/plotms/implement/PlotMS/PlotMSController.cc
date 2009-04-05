//# PlotMSController.cc: High level class for setting up and running a PlotMS.
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
#include <plotms/PlotMS/PlotMSController.h>

#include <casaqt/PlotterImplementations/PlotterImplementations.h>

namespace casa {

//////////////////////////////////
// PLOTMSCONTROLLER DEFINITIONS //
//////////////////////////////////

PlotMSController::PlotMSController() : itsCurrentPlotMS_(NULL) { }

PlotMSController::~PlotMSController() {
    if(itsCurrentPlotMS_) delete itsCurrentPlotMS_;
}


void PlotMSController::parametersHaveChanged(
        const PlotMSWatchedParameters& params, int updateFlag,
        bool redrawRequired) {
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

void PlotMSController::plotsChanged(const PlotMSPlotManager& manager) {
    const vector<PlotMSPlotParameters*>& p = manager.plotParameters();
    itsPlotParameters_.resize(p.size(), PlotMSSinglePlotParameters(
            plotterImplementation()));
    for(unsigned int i = 0; i < p.size(); i++)
        itsPlotParameters_[i] =
            *dynamic_cast<const PlotMSSinglePlotParameters*>(p[i]);
}


// convenience macro
#define PC_PMSP                                                               \
    PlotMSParameters& p = (itsCurrentPlotMS_ != NULL) ?                       \
            itsCurrentPlotMS_->getParameters() : itsParameters_;

void PlotMSController::setLogLevel(const String& logLevel, bool logDebug) {
    bool ok;
    PlotMSLogger::Level level = PlotMSLogger::level(logLevel, &ok);
    
    // use current log level if invalid
    if(!ok) level = PlotMSLogger::level(getLogLevel());
    
    PC_PMSP
    p.setLogLevel(level, logDebug);
}

String PlotMSController::getLogLevel() const {
    const PC_PMSP
    return PlotMSLogger::level(p.logLevel());
}

bool PlotMSController::getLogDebug() const {
    const PC_PMSP
    return p.logDebug();
}

void PlotMSController::setClearSelectionOnAxesChange(bool clearSelection) {
    PC_PMSP
    p.setClearSelectionsOnAxesChange(clearSelection);
}

bool PlotMSController::clearSelectionOnAxesChange() const {
    const PC_PMSP
    return p.clearSelectionsOnAxesChange();
}

void PlotMSController::setCachedImageSize(int width, int height) {
    PC_PMSP
    p.setCachedImageSize(width, height);
}

void PlotMSController::setCachedImageSizeToScreenResolution() {
    PC_PMSP
    p.setCachedImageSizeToResolution();
}

int PlotMSController::getCachedImageWidth() const {
    const PC_PMSP
    return p.cachedImageSize().first;
}

int PlotMSController::getCachedImageHeight() const {
    const PC_PMSP
    return p.cachedImageSize().second;
}


void PlotMSController::setPlotMSFilename(const String& msFilename,
        bool updateImmediately, int plotIndex) {
    bool resized = plotParameters(plotIndex);
    PlotMSSinglePlotParameters p = itsPlotParameters_[plotIndex];
    if(!resized && p.filename() == msFilename) return;
    p.setFilename(msFilename);
    if(updateImmediately && itsCurrentPlotMS_ != NULL) {
        if(resized)
            itsCurrentPlotMS_->addSinglePlot(&p);
        else
            *itsCurrentPlotMS_->getPlotManager().plotParameters()[plotIndex]=p;
    }
}

String PlotMSController::getPlotMSFilename(int plotIndex) const {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size()) return "";
    else return itsPlotParameters_[plotIndex].filename();
}

void PlotMSController::setPlotMSSelection(const Record& selection,
        bool updateImmediately, int plotIndex) {
    PlotMSSelection sel;
    sel.fromRecord(selection);
    setPlotMSSelection(sel, updateImmediately, plotIndex);
}

void PlotMSController::setPlotMSSelection(const String& field, const String& spw,
        const String& timerange, const String& uvrange,
        const String& antenna, const String& scan, const String& corr,
        const String& array, const String& msselect,
        bool updateImmediately, int plotIndex) {
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

Record PlotMSController::getPlotMSSelection(int plotIndex) const {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size())
        return Record();
    else return itsPlotParameters_[plotIndex].selection().toRecord();
}

void PlotMSController::setPlotMSAveraging(const Record& averaging,
        bool updateImmediately, int plotIndex) {
    PlotMSAveraging avg;
    avg.fromRecord(averaging);
    setPlotMSAveraging(avg, updateImmediately, plotIndex);
}

void PlotMSController::setPlotMSAveraging(bool channel, double channelValue,
        bool time, double timeValue, bool scan, bool field, bool baseline,
        bool updateImmediately, int plotIndex) {
    PlotMSAveraging avg;
    avg.setChannel(channel);
    avg.setChannelValue(channelValue);
    avg.setTime(time);
    avg.setTimeValue(timeValue);
    avg.setScan(scan);
    avg.setField(field);
    avg.setBaseline(baseline);
    setPlotMSAveraging(avg, updateImmediately, plotIndex);
}

Record PlotMSController::getPlotMSAveraging(int plotIndex) const {
    if(plotIndex < 0 || plotIndex >= (int)itsPlotParameters_.size())
        return Record();
    else return itsPlotParameters_[plotIndex].averaging().toRecord();
}


void PlotMSController::update() {
    if(itsCurrentPlotMS_ == NULL) return;
    
    const vector<PlotMSPlotParameters*>& p =
        itsCurrentPlotMS_->getPlotManager().plotParameters();
    
    // check for added plots
    if(itsPlotParameters_.size() > p.size()) {
        vector<PlotMSSinglePlotParameters> v(itsPlotParameters_.size() -
                p.size(), PlotMSSinglePlotParameters(plotterImplementation()));
        for(unsigned int i = 0; i < v.size(); i++)
            v[i] = itsPlotParameters_[i + p.size()];
        for(unsigned int i = 0; i < v.size(); i++)
            itsCurrentPlotMS_->addSinglePlot(&v[i]);
    }
    
    for(unsigned int i = 0; i < p.size(); i++)
        if(*p[i] != itsPlotParameters_[i]) *p[i] = itsPlotParameters_[i];
}

int PlotMSController::execLoop() {
    if(itsCurrentPlotMS_ == NULL) {
        itsCurrentPlotMS_ = new PlotMS(itsParameters_);
        PlotMSPlotManager& m = itsCurrentPlotMS_->getPlotManager();
        m.addWatcher(this);
        
        itsCurrentPlotMS_->showGUI(true);
        
        if(itsPlotParameters_.size() == 0)
            itsPlotParameters_.resize(1,
                    PlotMSSinglePlotParameters(plotterImplementation()));
        
        for(unsigned int i = 0; i < itsPlotParameters_.size(); i++) {
            itsCurrentPlotMS_->addSinglePlot(&itsPlotParameters_[i]);
            m.plot(i)->parameters().addWatcher(this);
        }
        
    } else {
        itsCurrentPlotMS_->showGUI(true);
        update();
    }
    
    return itsCurrentPlotMS_->execLoop();
}


bool PlotMSController::plotParameters(int& plotIndex) const {
    if(plotIndex < 0) plotIndex = 0;
    if((unsigned int)plotIndex > itsPlotParameters_.size())
        plotIndex = itsPlotParameters_.size();
    
    bool resized = false;
    if((unsigned int)plotIndex >= itsPlotParameters_.size()) {
        resized = true;
        const_cast<PlotMSController*>(this)->itsPlotParameters_.resize(
                plotIndex + 1,
                PlotMSSinglePlotParameters(plotterImplementation()));
    }
    
    return resized;
}

void PlotMSController::setPlotMSSelection(const PlotMSSelection& selection,
        bool updateImmediately, int plotIndex) {
    bool resized = plotParameters(plotIndex);
    PlotMSSinglePlotParameters p = itsPlotParameters_[plotIndex];
    if(!resized && p.selection() == selection) return;
    p.setSelection(selection);
    if(updateImmediately && itsCurrentPlotMS_ != NULL) {
        if(resized)
            itsCurrentPlotMS_->addSinglePlot(&p);
        else
            *itsCurrentPlotMS_->getPlotManager().plotParameters()[plotIndex]=p;
    }
}

void PlotMSController::setPlotMSAveraging(const PlotMSAveraging& averaging,
        bool updateImmediately, int plotIndex) {
    bool resized = plotParameters(plotIndex);
    PlotMSSinglePlotParameters p = itsPlotParameters_[plotIndex];
    if(!resized && p.averaging() == averaging) return;
    p.setAveraging(averaging);
    if(updateImmediately && itsCurrentPlotMS_ != NULL) {
        if(resized)
            itsCurrentPlotMS_->addSinglePlot(&p);
        else
            *itsCurrentPlotMS_->getPlotManager().plotParameters()[plotIndex]=p;
    }
}

}
