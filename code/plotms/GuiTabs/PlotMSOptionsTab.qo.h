//# PlotMSOptionsTab.qo.h: Subclass of PlotMSTab for plotms options.
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
#ifndef PLOTMSOPTIONSTAB_QO_H_
#define PLOTMSOPTIONSTAB_QO_H_

#include <plotms/GuiTabs/PlotMSOptionsTab.ui.h>

#include <plotms/GuiTabs/PlotMSTab.qo.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations.
class PlotMSLoggerWidget;
class PlotMSParameters;


// Subclass of PlotMSTab that handles options for PlotMSPlotter.   Watches
// PlotMS's PlotMSParameters for changes to update the GUI as needed.
class PlotMSOptionsTab : public PlotMSTab, Ui::OptionsTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent plotter.
    PlotMSOptionsTab(PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSOptionsTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Options"; }
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Updates
    // the GUI as needed if the given parameters are the PlotMS parent's
    // parameters.
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag);
    
    // Overrides PlotMSTab::setupForMaxWidth().  MUST be called before being
    // used, as it sets up the logging widget with it.
    void setupForMaxWidth(int maxWidth);
    
private:
    // Watched parameters for PlotMS.
    PlotMSParameters& itsParameters_;
    
    // Flag for changing the parameters.
    bool itsChangeFlag_;
    
    // Logger widget.
    PlotMSLoggerWidget* itsLoggerWidget_;
    
private slots:
    // When the user changes the tool button style on the GUI.
    void toolButtonStyleChanged(int newIndex);
    
    // When the user changes the log events or priority on the GUI.
    void logChanged();
    
    // When the user changes the "clear selection" on the GUI.
    void clearSelectionChanged(bool value);
    
    // When the user changes the cached image size.
    void cachedImageSizeChanged();
    
    // When the user sets the cached image size to screen resolution.
    void cachedImageSizeScreenResolution();
    
    // When the user changes the file chooser history limit.
    void historyLimitChanged();

    //When the grid size changes.
    void gridChanged();
};

}

#endif /* PLOTMSOPTIONSTAB_QO_H_ */
