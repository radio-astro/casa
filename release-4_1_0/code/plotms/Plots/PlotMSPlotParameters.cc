//# PlotMSPlotParameters.cc: Parameter classes for PlotMSPlot classes.
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
#include <plotms/Plots/PlotMSPlotParameters.h>

#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

/////////////////////////////////////////////
// PLOTMSPLOTPARAMETERS::GROUP DEFINITIONS //
/////////////////////////////////////////////

PlotMSPlotParameters::Group::Group(PlotFactoryPtr factory) : itsParent_(NULL),
        itsFactory_(factory) { }

PlotMSPlotParameters::Group::Group(const Group& copy) : itsParent_(NULL),
        itsFactory_(copy.itsFactory_) { }

PlotMSPlotParameters::Group::~Group() { }

PlotMSPlotParameters::Group&
PlotMSPlotParameters::Group::operator=(const Group& other) {
    if(name() == other.name() && *this != other) fromRecord(other.toRecord());
    return *this;
}

bool PlotMSPlotParameters::Group::operator==(const Group& other) const {
    return name() == other.name() && PMS::recEq(toRecord(), other.toRecord());}

void PlotMSPlotParameters::Group::updated(bool requiresRedraw) {
    if(itsParent_ != NULL) itsParent_->groupUpdated(this, requiresRedraw); }


void PlotMSPlotParameters::Group::notifyWatchers_(bool /*wasCanceled*/) {
    if(itsParent_ != NULL) itsParent_->notifyWatchers(name()); }


//////////////////////////////////////
// PLOTMSPLOTPARAMETERS DEFINITIONS //
//////////////////////////////////////

// Constructors/Destructors //

PlotMSPlotParameters::PlotMSPlotParameters(PlotFactoryPtr factory) :
        itsFactory_(factory) { }

PlotMSPlotParameters::PlotMSPlotParameters(const PlotMSPlotParameters& copy)
    : PlotMSWatchedParameters(copy), itsFactory_(copy.itsFactory_) {
    operator=(copy);
}

PlotMSPlotParameters::~PlotMSPlotParameters() {
    for(unsigned int i = 0; i < itsGroups_.size(); i++)
        delete itsGroups_[i];
}


// Public Methods //

bool PlotMSPlotParameters::equals(const PlotMSWatchedParameters& other,
        int updateFlags) const {
    const PlotMSPlotParameters* p =
        dynamic_cast<const PlotMSPlotParameters*>(&other);
    if(p == NULL) return false;
    
    const Group* g;
    for(unsigned int i = 0; i < itsGroups_.size(); i++) {
        // Only check equality if that flag is included.
        if(!(updateFlags & UPDATE_FLAG(itsGroups_[i]->name()))) continue;
        
        g = p->group(itsGroups_[i]->name());
        if(g != NULL && *itsGroups_[i] != *g) return false;
    }
    
    return true;
}

const PlotMSPlotParameters::Group*
PlotMSPlotParameters::group(const String& name) const {
    for(unsigned int i = 0; i < itsGroups_.size(); i++)
        if(itsGroups_[i]->name() == name) return itsGroups_[i];
    return NULL;
}

PlotMSPlotParameters::Group* PlotMSPlotParameters::group(const String& name) {
    for(unsigned int i = 0; i < itsGroups_.size(); i++)
        if(itsGroups_[i]->name() == name) return itsGroups_[i];
    return NULL;
}

void PlotMSPlotParameters::setGroup(const Group& group) {
    Group* g = PlotMSPlotParameters::group(group.name());
    if(g == NULL) {
        REGISTER_UPDATE_FLAG(group.name());
        g = group.clone();
        g->itsParent_ = this;
        itsGroups_.push_back(g);
        groupUpdated(g, true);//g->requiresRedrawOnChange());
    } else {
        *g = group;
    }
}

PlotMSPlotParameters&
PlotMSPlotParameters::operator=(const PlotMSPlotParameters& copy) {
    // Remove groups that aren't in the copy.
    bool found = false;
    int n = (int)itsGroups_.size();
    for(int i = 0; i < n; i++) {
        found = false;
        for(unsigned int j = 0; !found && j < copy.itsGroups_.size(); j++)
            if(itsGroups_[i]->name() == copy.itsGroups_[j]->name())
                found = true;
        if(!found) {
            delete itsGroups_[i];
            itsGroups_.erase(itsGroups_.begin() + i);
            i--;
        }
    }
    
    for(unsigned int i = 0; i < copy.itsGroups_.size(); i++)
        setGroup(*copy.itsGroups_[i]);
    
    return *this;
}


// Protected Methods //

void PlotMSPlotParameters::notifyWatchers(int updates,
        PlotMSParametersWatcher* updater) {
    holdNotification(updater);
    updateFlags(currentUpdateFlag() & updates);
    releaseNotification();
}


// Private Methods //

void PlotMSPlotParameters::groupUpdated(Group* group, bool requiresRedraw) {
    int flags = currentUpdateFlag();
    flags |= UPDATE_FLAG(group->name());
    if(requiresRedraw) flags |= PMS_PP::UPDATE_REDRAW;
    updateFlags(flags);
}

}
