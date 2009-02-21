//# QPCanvas.qo.h: Qwt implementation of generic PlotCanvas class.
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
#ifndef QPCANVAS_QO_H_
#define QPCANVAS_QO_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotCanvas.h>
#include <graphics/GenericPlotter/PlotLogger.h>
#include <graphics/GenericPlotter/Plotter.h>
#include <casaqt/QwtPlotter/QPCanvasHelpers.qo.h>
#include <casaqt/QwtPlotter/QPOptions.h>
#include <casaqt/QwtPlotter/QPPlotItem.h>

#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>

#include <QtGui>

#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward declarations
class QPPlotter;


// Implementation of PlotCanvas for the Qwt plotter.  Mainly consists of
// wrappers and containers around a QwtPlot object.
class QPCanvas : public QFrame, public virtual PlotCanvas {
    Q_OBJECT
    
    friend class QPLayeredCanvas;
    friend class QPPlotItem;
    friend class QPPlotter;
    
public:
    // Static //
    
    // Keep a z-order variable to increment for subsequent items.
    static double zOrder;
    
    // Convenient access to class name (QPCanvas).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes (optional) parent QPPlotter.
    QPCanvas(QPPlotter* parent = NULL);

    // Destructor.
    ~QPCanvas();
    
    
    // Include overloaded methods.
    using PlotCanvas::setBackground;
    using PlotCanvas::setSelectLine;
    using PlotCanvas::setTitleFont;
    using PlotCanvas::setAxisRange;
    using PlotCanvas::setAxesRanges;
    using PlotCanvas::showCartesianAxis;
    using PlotCanvas::setAxisFont;
    using PlotCanvas::setGridMajorLine;
    using PlotCanvas::setGridMinorLine;
    using PlotCanvas::setLegendLine;
    using PlotCanvas::setLegendFill;
    using PlotCanvas::setLegendFont;


    // PlotCanvas Methods //
    
    // Implements PlotCanvas::size().
    pair<int, int> size() const;
    
    // Implements PlotCanvas::title().
    String title() const;
    
    // Implements PlotCanvas::setTitle().
    void setTitle(const String& title);

    // Implements PlotCanvas::titleFont().
    PlotFontPtr titleFont() const;
    
    // Implements PlotCanvas::setTitleFont().
    void setTitleFont(const PlotFont& font);
    
    // Implements PlotCanvas::background().
    PlotAreaFillPtr background() const;
    
    // Implements PlotCanvas::setBackground().
    void setBackground(const PlotAreaFill& areaFill);
    
    // Implements PlotCanvas::cursor().
    PlotCursor cursor() const;
    
    // Implements PlotCanvas::setCursor().
    void setCursor(PlotCursor cursor);
    
    // Implements PlotCanvas::refresh().  Calls QwtPlot::replot()
    void refresh();
    
    // Implements PlotCanvas::isQWidget().
    bool isQWidget() const { return true; }
    
    
    // Implements PlotCanvas::axisShown().
    bool axisShown(PlotAxis axis) const;
    
    // Implements PlotCanvas::showAxis().
    void showAxis(PlotAxis axis, bool show = true);
    
    // Implements PlotCanvas::axisScale().
    PlotAxisScale axisScale(PlotAxis axis) const;
    
    // Implements PlotCanvas::setAxisScale().
    void setAxisScale(PlotAxis axis, PlotAxisScale scale);
    
    // Implements PlotCanvas::axisRange().
    pair<double, double> axisRange(PlotAxis axis) const;
    
    // Implements PlotCanvas::setAxisRange().
    void setAxisRange(PlotAxis axis, double from, double to);
    
    // Overrides PlotCanvas::setAxesRanges().
    void setAxesRanges(PlotAxis xAxis, double xFrom, double xTo,
                       PlotAxis yAxis, double yFrom, double yTo);

    // Implements PlotCanvas::axesAutoRescale().
    bool axesAutoRescale() const;
    
    // Implements PlotCanvas::setAxesAutoRescale().
    void setAxesAutoRescale(bool autoRescale = true);

    // Implements PlotCanvas::rescaleAxes().
    void rescaleAxes();
    
    // Implements PlotCanvas::axesRatioLocked().
    bool axesRatioLocked() const;
    
    // Implements PlotCanvas::setAxesRatioLocked().
    void setAxesRatioLocked(bool locked = true);
    
    // Implements PlotCanvas::cartesianAxisShown().
    bool cartesianAxisShown(PlotAxis axis) const;
    
    // Implements PlotCanvas::showCartesianAxis().
    void showCartesianAxis(PlotAxis mirrorAxis, PlotAxis secondaryAxis,
                           bool show = true, bool hideNormalAxis = true);

    // Implements PlotCanvas::axisLabel().
    String axisLabel(PlotAxis axis) const;
    
    // Implements PlotCanvas::setAxisLabel().
    void setAxisLabel(PlotAxis axis, const String& title);

    // Implements PlotCanvas::axisFont().
    PlotFontPtr axisFont(PlotAxis axis) const;
    
    // Implements PlotCanvas::setAxisFont().
    void setAxisFont(PlotAxis axis, const PlotFont& font);
    
    // Implements PlotCanvas::colorBarShown().
    bool colorBarShown(PlotAxis axis = Y_RIGHT) const;

    // Implements PlotCanvas::showColorBar().
    void showColorBar(bool show = true, PlotAxis axis = Y_RIGHT);

    
    // Implements PlotCanvas::autoIncrementColors().
    bool autoIncrementColors() const;
    
    // Implements PlotCanvas::setAutoIncrementColors().
    void setAutoIncrementColors(bool autoInc = true);   

    // Implements PlotCanvas::exportToFile().
    bool exportToFile(const PlotExportFormat& format);
    
    // Implements PlotCanvas::fileChooserDialog().
    String fileChooserDialog(const String& title = "File Chooser",
                             const String& directory = "");
    
    // Implements PlotCanvas::convertCoordinate().
    PlotCoordinate convertCoordinate(const PlotCoordinate& coord,
            PlotCoordinate::System newSystem = PlotCoordinate::WORLD) const;
    
    // Implements PlotCanvas::textWidthHeightDescent().
    vector<double> textWidthHeightDescent(const String& text,
                                          PlotFontPtr font) const;
    
    // Implements PlotCanvas::implementation().
    int implementation() const { return Plotter::QWT; }
    
    // Implements PlotCanvas::implementationFactory().
    PlotFactory* implementationFactory() const;
    
    
    // Implements PlotCanvas::holdDrawing().
    void holdDrawing();
    
    // Implements PlotCanvas::releaseDrawing().
    void releaseDrawing();
    
    // Implements PlotCanvas::drawingIsHeld().
    bool drawingIsHeld() const;
    
    // Implements PlotCanvas::plotItem().  If the given items is NOT an
    // instance of a QPPlotItem, a copy of the given items is made.  The
    // original item is NOT kept by the canvas, so any subsequent changes to
    // the original items will not be reflected on the canvas.
    bool plotItem(PlotItemPtr item, PlotCanvasLayer layer = MAIN);
    
    // Implements PlotCanvas::allPlotItems().
    vector<PlotItemPtr> allPlotItems() const;
    
    // Implements PlotCanvas::layerPlotItems().
    vector<PlotItemPtr> layerPlotItems(PlotCanvasLayer layer) const;
    
    // Overrides PlotCanvas::numPlotItems().
    unsigned int numPlotItems() const;

    // Overrides PlotCanvas::numLayerPlotItems().
    unsigned int numLayerPlotItems(PlotCanvasLayer layer) const;
    
    // Implements PlotCanvas::removePlotItems().
    void removePlotItems(const vector<PlotItemPtr>& items);

    // Overrides PlotCanvas::clearPlotItems().
    void clearPlotItems();
    
    // Overrides PlotCanvas::clearPlots().
    void clearPlots();
    
    // Overrides PlotCanvas::clearLayer().
    void clearLayer(PlotCanvasLayer layer);

       
    // Implements PlotCanvas::selectLineShown().
    bool selectLineShown() const;

    // Implements PlotCanvas::setSelectLineShown().
    void setSelectLineShown(bool shown = true);

    // Implements PlotCanvas::selectLine().
    PlotLinePtr selectLine() const;

    // Implements PlotCanvas::setSelectLine().
    void setSelectLine(const PlotLine& line);
    
    
    // Implements PlotCanvas::gridXMajorShown().
    bool gridXMajorShown() const;
    
    // Implements PlotCanvas::setGridXMajorShown().
    void setGridXMajorShown(bool s = true);
    
    // Implements PlotCanvas::gridXMinorShown().
    bool gridXMinorShown() const;
       
    // Implements PlotCanvas::setGridXMinorShown().
    void setGridXMinorShown(bool s = true);
    
    // Implements PlotCanvas::gridYMajorShown().
    bool gridYMajorShown() const;
        
    // Implements PlotCanvas::setGridYMajorShown().
    void setGridYMajorShown(bool s = true);

    // Implements PlotCanvas::gridYMinorShown().
    bool gridYMinorShown() const;

    // Implements PlotCanvas::setGridYMinorShown().
    void setGridYMinorShown(bool s = true);
    
    // Implements PlotCanvas::gridMajorLine().
    PlotLinePtr gridMajorLine() const;
    
    // Implements PlotCanvas::setGridMajorLine().
    void setGridMajorLine(const PlotLine& line);
    
    // Implements PlotCanvas::gridMinorLine().
    PlotLinePtr gridMinorLine() const;
    
    // Implements PlotCanvas::setGridMinorLine().
    void setGridMinorLine(const PlotLine& line);
    
    
    // Implements PlotCanvas::legendShown().
    bool legendShown() const;
    
    // Implements PlotCanvas::showLegend().
    void showLegend(bool on = true, LegendPosition pos = EXT_BOTTOM);
    
    // Implements PlotCanvas::legendPosition().
    LegendPosition legendPosition() const;
    
    // Implements PlotCanvas::setLegendPosition().
    void setLegendPosition(LegendPosition pos);
    
    // Implements PlotCanvas::legendLine().
    PlotLinePtr legendLine() const;
    
    // Implements PlotCanvas::setLegendLine().
    void setLegendLine(const PlotLine& line);
    
    // Implements PlotCanvas::legendFill().
    PlotAreaFillPtr legendFill() const;
    
    // Implements PlotCanvas::setLegendFill().
    void setLegendFill(const PlotAreaFill& area);
    
    // Implements PlotCanvas::legendFont().
    PlotFontPtr legendFont() const;
        
    // Implements PlotCanvas::setLegendFont().
    void setLegendFont(const PlotFont& font);
    
    
    // Implements PlotCanvas::registerMouseTool().
    void registerMouseTool(PlotMouseToolPtr tool, bool activate = true,
                           bool blocking = false);
    
    // Implements PlotCanvas::allMouseTools().
    vector<PlotMouseToolPtr> allMouseTools() const;
    
    // Implements PlotCanvas::activeMouseTools().
    vector<PlotMouseToolPtr> activeMouseTools() const;
    
    // Implements PlotCanvas::unregisterMouseTool().
    void unregisterMouseTool(PlotMouseToolPtr tool);
    
    // Implements PlotCanvas::standardMouseTools().
    PlotStandardMouseToolGroupPtr standardMouseTools();
    
    
    // Implements PlotCanvas::mutex().
    PlotMutexPtr mutex() const;
    
    
    // Implements PlotCanvas handler methods.
    // <group>
    void registerSelectHandler(PlotSelectEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD);
    vector<PlotSelectEventHandlerPtr> allSelectHandlers() const;
    void unregisterSelectHandler(PlotSelectEventHandlerPtr handler);
    void registerClickHandler(PlotClickEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD);
    vector<PlotClickEventHandlerPtr> allClickHandlers() const;
    void unregisterClickHandler(PlotClickEventHandlerPtr handler);
    void registerMousePressHandler(PlotMousePressEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD);
    vector<PlotMousePressEventHandlerPtr> allMousePressHandlers() const;
    void unregisterMousePressHandler(PlotMousePressEventHandlerPtr handler);
    void registerMouseReleaseHandler(PlotMouseReleaseEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD);
    vector<PlotMouseReleaseEventHandlerPtr> allMouseReleaseHandlers() const;
    void unregisterMouseReleaseHandler(PlotMouseReleaseEventHandlerPtr handlr);
    void registerMouseDragHandler(PlotMouseDragEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD);
    vector<PlotMouseDragEventHandlerPtr> allMouseDragHandlers() const;
    void unregisterMouseDragHandler(PlotMouseDragEventHandlerPtr handler);
    void registerMouseMoveHandler(PlotMouseMoveEventHandlerPtr h,
            PlotCoordinate::System system = PlotCoordinate::WORLD);
    vector<PlotMouseMoveEventHandlerPtr> allMouseMoveHandlers() const;
    void unregisterMouseMoveHandler(PlotMouseMoveEventHandlerPtr h);
    void registerWheelHandler(PlotWheelEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD);
    vector<PlotWheelEventHandlerPtr> allWheelHandlers() const;
    void unregisterWheelHandler(PlotWheelEventHandlerPtr handler);
    void registerKeyHandler(PlotKeyEventHandlerPtr handler);
    vector<PlotKeyEventHandlerPtr> allKeyHandlers() const;
    void unregisterKeyHandler(PlotKeyEventHandlerPtr handler);
    void registerResizeHandler(PlotResizeEventHandlerPtr handler);
    vector<PlotResizeEventHandlerPtr> allResizeHandlers() const;
    void unregisterResizeHandler(PlotResizeEventHandlerPtr handler);
    // </group>
    

    // QPCanvas Methods //
    
    // Provides access to the underlying QPLayeredCanvas.
    QPLayeredCanvas& asQwtPlot();
    
    // Provides access to the QwtPlotPicker used for selection events.
    QwtPlotPicker& getSelecter();
    
    // Reinstalls the tracker filter (in case another QwtPlotPicker is added to
    // the QwtPlotCanvas).
    void reinstallTrackerFilter();
    
protected:
    // Sets the parent QPPlotter to the given.  This MUST be done when a canvas
    // is added to the plotter so that it can use the plotter's logger if
    // needed.
    void setQPPlotter(QPPlotter* parent);
    
    // Returns a PlotLogger to be used for the given measurement event.  If the
    // return value is NULL, then the event should NOT be logged, otherwise it
    // should.  Should be used by QPPlotItems attached to the canvas.
    PlotLoggerPtr loggerForMeasurement(PlotLogger::MeasurementEvent event);
    
    // For catching Qt press events.
    void mousePressEvent(QMouseEvent* event);
    
    // For catching Qt click and release events.
    void mouseReleaseEvent(QMouseEvent* event);
    
    // For catching Qt double-click events.
    void mouseDoubleClickEvent(QMouseEvent* event);
    
    // For catching Qt key events.
    void keyReleaseEvent(QKeyEvent* event);
    
    // For catching Qt scroll wheel events.
    void wheelEvent(QWheelEvent* event);
    
    // For catching Qt resize events.
    void resizeEvent(QResizeEvent* event);

private:
    // Parent QPPlotter.
    QPPlotter* m_parent;
    
    // Main QwtPlot object.
    QPLayeredCanvas m_canvas;

    // Main-layer plot items.
    vector<pair<PlotItemPtr, QPPlotItem*> > m_plotItems;
    vector<pair<PlotItemPtr, QPPlotItem*> > m_layeredItems;
    
    // Cartesian axes (NULL if not shown).
    vector<QPCartesianAxis*> m_cartAxes;
    
    // Whether the axes ratio is locked or not.
    bool m_axesRatioLocked;
    
    // Used for recalculating axes ranges if the ratio is locked.
    vector<double> m_axesRatios;

    // Whether auto-increment colors is turned on or not.
    bool m_autoIncColors;
    
    // Used auto-incremented colors.
    vector<int> m_usedColors;
    
    // Picker used for select events.
    QwtPlotPicker m_picker;
    
    // Filter used for mouse move events.  Has to initialize after the picker
    // to be first in the filter.
    QPMouseFilter m_mouseFilter;
    
    // Used for displaying grid.
    QwtPlotGrid m_grid;
    
    // Legend properties.
    // <group>
    QFrame m_legendFrame;
    QPLine m_legendLine;
    QPAreaFill m_legendFill;
    LegendPosition m_legendPosition;
    QPFont m_legendFont;
    bool m_legendFontSet;
    // </group>
    
    // Registered mouse tools.
    vector<PlotMouseToolPtr> m_mouseTools;
    
    // Standard mouse tools (or NULL if none have been set yet).
    PlotStandardMouseToolGroupPtr m_standardTools;
    
    // Handlers.
    // <group>
    vector<pair<PlotSelectEventHandlerPtr, PlotCoordinate::System> >
        m_selectHandlers;
    vector<pair<PlotClickEventHandlerPtr, PlotCoordinate::System> >
        m_clickHandlers;
    vector<pair<PlotMousePressEventHandlerPtr, PlotCoordinate::System> >
        m_pressHandlers;
    vector<pair<PlotMouseReleaseEventHandlerPtr, PlotCoordinate::System> >
        m_releaseHandlers;
    vector<pair<PlotMouseDragEventHandlerPtr, PlotCoordinate::System> >
        m_dragHandlers;
    vector<pair<PlotMouseMoveEventHandlerPtr, PlotCoordinate::System> >
        m_moveHandlers;
    vector<pair<PlotWheelEventHandlerPtr, PlotCoordinate::System> >
        m_wheelHandlers;
    vector<PlotKeyEventHandlerPtr> m_keyHandlers;
    vector<PlotResizeEventHandlerPtr> m_resizeHandlers;
    bool m_inDraggingMode;
    // </group>
    
    // For catching single vs. double clicks.
    // <group>
    bool m_ignoreNextRelease;
    QTimer m_timer;
    QMouseEvent* m_clickEvent;
    // </group>
    
    
    // Resets all mouse tools.
    void resetStacks();
    
    
    // Static //
    
    // Converts between axes and vector indices.
    // <group>
    static unsigned int axisIndex(PlotAxis a);   
    static PlotAxis axisIndex(unsigned int i);
    // </group>
    
private slots:    
    // For when the selecter has selected a region; emit a PlotSelectEvent.
    void regionSelected(const QwtDoubleRect&);
    
    // For catching single vs. double clicks.
    void timeout();
    
    // For catching mouse move events from the filter.
    void trackerMouseEvent(QMouseEvent* event);
};

}

#endif

#endif /*QPCANVAS_QO_H_*/
