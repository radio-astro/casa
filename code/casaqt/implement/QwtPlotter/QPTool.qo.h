//# QPTool.qo.h: Qwt implementations of PlotTools.
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
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $
#ifndef QPTOOL_QO_H_
#define QPTOOL_QO_H_

#ifdef AIPS_HAS_QWT
#include <graphics/GenericPlotter/PlotTool.h>

#include <casaqt/QwtPlotter/QPAnnotation.h>

#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class QPTracker;


// Subclass of PlotSelectTool specific to Qwt plotter.  Currently isn't
// specialized.
class QPSelectTool : public PlotSelectTool {
public:
    // Constructor which takes the tool's coordinate system.
    QPSelectTool(PlotCoordinate::System sys = PlotCoordinate::WORLD);
    
    // Constructor which takes the tool's axes and coordinate system.
    QPSelectTool(PlotAxis xAxis, PlotAxis yAxis,
                 PlotCoordinate::System sys = PlotCoordinate::WORLD);
    
    // Destructor.
    ~QPSelectTool();
};


// Subclass of PlotZoomTool specific to Qwt plotter.  Currently isn't
// specialized.
class QPZoomTool : public PlotZoomTool {
public:
    // Constructor which takes the tool's coordinate system.
    QPZoomTool(PlotCoordinate::System sys = PlotCoordinate::WORLD);
    
    // Constructor which takes the tool's axes and coordinate system.
    QPZoomTool(PlotAxis xAxis, PlotAxis yAxis,
               PlotCoordinate::System sys = PlotCoordinate::WORLD);
    
    // Destructor.
    ~QPZoomTool();
};


// Subclass of PlotPanTool specific to Qwt plotter.  Uses a QwtPlotPanner to
// manage some behavior.
class QPPanTool : public QObject, public PlotPanTool {
    Q_OBJECT
    
public:
    // Constructor which takes the tool's coordinate system.
    QPPanTool(PlotCoordinate::System sys = PlotCoordinate::WORLD);
    
    // Constructor which takes the tool's axes and coordinate system.
    QPPanTool(PlotAxis xAxis, PlotAxis yAxis,
              PlotCoordinate::System sys = PlotCoordinate::WORLD);
    
    // Destructor.
    ~QPPanTool();
    
    
    // PlotTool Methods //

    // Overrides PlotTool::setActive().
    void setActive(bool isActive = true);
    
    
    // PlotMouseTool Methods //

    // Implements PlotMouseTool::handleMouseEvent() to handle wheel and right
    // click.
    void handleMouseEvent(const PlotEvent& event);
    
protected:
    // PlotTool Methods //

    // Overrides PlotTool::attach().
    void attach(PlotCanvas* canvas);
    
private:
    QwtPlotPanner* m_panner; // Panner
    
private slots:
    // For when the pan window changes (to update the stack).
    void panned(int dx, int dy);
};


// Subclass of PlotTrackerTool specific to Qwt plotter, using a QPTracker.
class QPTrackerTool : public PlotTrackerTool {
public:
    // Constructor which takes the tool's coordinate system.
    QPTrackerTool(PlotCoordinate::System sys = PlotCoordinate::WORLD);
    
    // Constructor which takes the tool's axes and coordinate system.
    QPTrackerTool(PlotAxis xAxis, PlotAxis yAxis,
                  PlotCoordinate::System sys = PlotCoordinate::WORLD);
    
    // Destructor.
    ~QPTrackerTool();
    
    // PlotTool Methods //

    // Overrides PlotTool::setActive().
    void setActive(bool isActive = true);
    
    
    // PlotMouseTool Methods //
    
    // Implements PlotMouseTool::handleMouseEvent() to update PlotTrackerTool's
    // annotation with that of the QPTracker.
    void handleMouseEvent(const PlotEvent& event);
    
    // PlotTrackerTool Methods //

    // Overrides PlotTrackerTool::setDrawText().
    void setDrawText(bool draw = true);
    
protected:
    // PlotTool Methods //

    // Overrides PlotTool::attach().
    void attach(PlotCanvas* canvas);
    
    // Overrides PlotTool::detach().
    void detach();
    
private:
    QPTracker* m_tracker; // Tracker.
};


// Subclass of QwtPlotPicker to be more compatible with PlotTrackerTool.
class QPTracker : public QwtPlotPicker {
    Q_OBJECT
    
public:
    // Constructor which takes the tracker and the canvas to be installed on.
    QPTracker(QPTrackerTool& tracker, QwtPlotCanvas* canvas);
    
    // Destructor.
    ~QPTracker();
    
    
    // Returns the annotation which holds the current position and tracker
    // text.
    PlotAnnotationPtr getAnnotation() {
        return PlotAnnotationPtr(&m_annotation, false); }
    
    
    // QwtPlotPicker Methods //
    
    // Overrides QwtPlotPicker::trackerText() to take font and format into
    // account.
    // <group>
    QwtText trackerText(const QPoint& pos) const {
        return trackerText(invTransform(pos)); }
    QwtText trackerText(const QwtDoublePoint& pos) const;
    // </group>
    
private:
    QPTrackerTool& m_tracker;  // Tracker tool
    QPAnnotation m_annotation; // Current annotation
};

}

#endif

#endif /* QPTOOL_QO_H_ */
