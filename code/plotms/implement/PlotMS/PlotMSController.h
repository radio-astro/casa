//# PlotMSController.h: High level class for setting up and running a PlotMS.
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
#ifndef PLOTMSCONTROLLER_H_
#define PLOTMSCONTROLLER_H_

#include <plotms/PlotMS/PlotMS.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// High level class for setting up and running a PlotMS.  Eventually will be
// used in a Python interface.
class PlotMSController : public PlotMSParametersWatcher,
                         public PlotMSPlotManagerWatcher {
public:
    // Constructor.
    PlotMSController();
    
    // Destructor.
    ~PlotMSController();
    
    
    // Watcher Methods //
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired);
    
    // Implements PlotMSPlotManagerWatcher::plotsChanged().
    void plotsChanged(const PlotMSPlotManager& manager);
    
    
    // PlotMS Parameter Methods //
    
    // Gets/Sets the log level and debug.
    // <group>
    void setLogLevel(const String& logLevel, bool logDebug);
    String getLogLevel() const;
    bool getLogDebug() const;
    // </group>
    
    // Gets/Sets whether any selections are cleared on an axes change or not.
    // <group>
    void setClearSelectionOnAxesChange(bool clearSelection);
    bool clearSelectionOnAxesChange() const;
    // </group>
    
    // Gets/Sets the size for the cached images.
    // <group>
    void setCachedImageSize(int width, int height);
    void setCachedImageSizeToScreenResolution();
    int getCachedImageWidth() const;
    int getCachedImageHeight() const;
    // </group>
    
    
    // PlotMSSinglePlot Parameter Methods //
    
    // Gets/Sets the MS filename for the plot at the given index.  If
    // updateImmediately is true, this change takes effect immediately;
    // otherwise it will only be applied next time update() is called.
    // <group>
    void setPlotMSFilename(const String& msFilename,
            bool updateImmediately = true, int plotIndex = 0);
    String getPlotMSFilename(int plotIndex = 0) const;
    // </group>
    
    // Gets/Sets the MS selection parameters for the plot at the given index.
    // The selection can be set either using a Record
    // (see PlotMSSelection::toRecord()) or with individual values.  If
    // updateImmediately is true, this change takes effect immediately;
    // otherwise it will only be applied next time update() is called.
    // <group>
    void setPlotMSSelection(const Record& selection,
            bool updateImmediately = true, int plotIndex = 0);
    void setPlotMSSelection(const String& field, const String& spw,
            const String& timerange, const String& uvrange,
            const String& antenna, const String& scan, const String& corr,
            const String& array, const String& msselect,
            bool updateImmediately = true, int plotIndex = 0);
    Record getPlotMSSelection(int plotIndex = 0) const;
    // </group>
    
    // Gets/Sets the MS averaging parameters for the plot at the given index.
    // The selection can be set either using a Record
    // (see PlotMSAveraging::toRecord()) or with individual values.  If
    // updateImmediately is true, this change takes effect immediately;
    // otherwise it will only be applied next time update() is called.
    // <group>
    void setPlotMSAveraging(const Record& averaging,
            bool updateImmediately = true, int plotIndex = 0);
    void setPlotMSAveraging(bool channel, double channelValue, bool time,
            double timeValue, bool scan, bool field, bool baseline,
            bool updateImmediately = true, int plotIndex = 0);
    Record getPlotMSAveraging(int plotIndex = 0) const;
    // </group>
    
    
    // Execution Methods //
    
    // Applies any unapplied changes to the currently running PlotMS.
    void update();
    
    // Enters PlotMS's execution loop, after running update().
    int execLoop();
    
private:
    // Currently running PlotMS, or NULL for none.
    PlotMS* itsCurrentPlotMS_;
    
    // Set PlotMS parameters that haven't yet been transferred to the current
    // PlotMS.
    PlotMSParameters itsParameters_;
    
    // Set PlotMSSinglePlot parameters that haven't yet been transfered to the
    // current PlotMS.
    vector<PlotMSSinglePlotParameters> itsPlotParameters_;
    
    
    // Adjusts the given plot index to be an acceptable, and returns whether
    // the parameters were resized or not.
    bool plotParameters(int& plotIndex) const;
    
    // Helper for setting MS selection.
    void setPlotMSSelection(const PlotMSSelection& selection,
            bool updateImmediately, int plotIndex);
    
    // Helper for setting MS averaging.
    void setPlotMSAveraging(const PlotMSAveraging& averaging,
            bool updateImmediately, int plotIndex);
};

}

#endif /* PLOTMSCONTROLLER_H_ */
