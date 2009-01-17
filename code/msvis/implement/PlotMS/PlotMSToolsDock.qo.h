//# PlotMSToolsDock.qo.h: Widget for tools dock.
//# Copyright (C) 2008
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
//#
//# $Id:  $
#ifndef PLOTMSTOOLSDOCK_QO_H_
#define PLOTMSTOOLSDOCK_QO_H_

#include <msvis/PlotMS/PlotMSToolsDock.ui.h>

#include <msvis/PlotMS/PlotMSLogger.h>
#include <msvis/PlotMS/PlotMSOptions.h>

#include <QDockWidget>

namespace casa {

// Tools dock widget.
class PlotMSToolsDock : public QWidget, Ui::ToolsDock,
                        public PlotTrackerToolNotifier {
    Q_OBJECT
    
public:
    PlotMSToolsDock();
    
    ~PlotMSToolsDock();
    
    // Returns a dock surrounding this widget.
    QDockWidget* getDockWidget();
    
    void setOptions(const PlotMSOptions& opts);
    
    void setTrackerText(const String& text);
    
    void setLogLevel(PlotMSLogger::Level level);
    
signals:
    void showError(const String& message, const String& title, bool isWarning);

    void exportRequested(const PlotExportFormat& format, int row, int col);
    
    void mouseToolChanged(PlotStandardMouseToolGroup::Tool newTool);
    
    void trackerTurned(bool hover, bool display);
    
    void logLevelChanged(PlotMSLogger::Level level);
    
protected:
    void notifyTrackerChanged(PlotTrackerTool& tool);
    
private:
    QDockWidget* itsDock_;
    
private slots:
    void mouseTool(bool on);
    
    void tracker();

    void exportBrowse();

    void exportClicked();
    
    void logLevel(const QString& text);
};

}

#endif /* PLOTMSTOOLSDOCK_QO_H_ */
