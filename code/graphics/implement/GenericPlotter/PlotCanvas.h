//# PlotCanvas.h: Main drawing area for different plot items to be attached.
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
#ifndef PLOTCANVAS_H_
#define PLOTCANVAS_H_

#include <graphics/GenericPlotter/Plot.h>
#include <graphics/GenericPlotter/PlotAnnotation.h>
#include <graphics/GenericPlotter/PlotEventHandler.h>
#include <graphics/GenericPlotter/PlotItem.h>
#include <graphics/GenericPlotter/PlotLogger.h>
#include <graphics/GenericPlotter/PlotOperation.h>
#include <graphics/GenericPlotter/PlotOptions.h>
#include <graphics/GenericPlotter/PlotShape.h>
#include <graphics/GenericPlotter/PlotTool.h>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class PlotFactory;


// PlotCanvas is an area for different PlotItems to be attached and drawn.  It
// also has axes and other properties.  It's one of the more important plotting
// classes.
class PlotCanvas {
public:
    // Static //
    
    // Position that the legend can be in, relative to the canvas.
    enum LegendPosition {
        INT_URIGHT, // inside the canvas, on the upper right
        INT_LRIGHT, // inside the canvas, on the lower right
        INT_ULEFT,  // inside the canvas, on the upper left
        INT_LLEFT,  // inside the canvas, on the lower left
        EXT_RIGHT,  // outside the canvas, on the right
        EXT_TOP,    // outside the canvas, on the top
        EXT_LEFT,   // outside the canvas, on the left
        EXT_BOTTOM  // outside the canvas, on the bottom
    };
    
    // Returns all legend positions.
    // <group>
    static vector<LegendPosition> allLegendPositions(); 
    static vector<String> allLegendPositionStrings();
    // </group>
    
    // Converts between legend positions and their String representations.
    // <group>
    static String legendPosition(LegendPosition p); 
    static LegendPosition legendPosition(String p, bool* ok = NULL);
    // </group>
    
    // Indicates whether the given legend position is internal to the canvas or
    // not.
    static bool legendPositionIsInternal(LegendPosition p);
    
    // Convenient access to draw operation name.
    static const String OPERATION_DRAW;
    
    
    // Non-Static //
    
    // Constructor.
    PlotCanvas();
    
    // Destructor.
    virtual ~PlotCanvas();
    
    
    // ABSTRACT METHODS //
    
    // GUI Methods //
    
    // Returns the canvas size in pixels (width x height).
    virtual pair<int, int> size() const = 0;
    
    // Returns the text of the canvas title, or an empty String if none is
    // shown.  The title is shown above the plotting area.
    virtual String title() const = 0;
    
    // Sets the text of the canvas title to the given.  Setting the title to an
    // empty string should remove the title from the canvas.
    virtual void setTitle(const String& title) = 0;
    
    // Returns a copy of the font used for the canvas title.  If no title is
    // shown this behavior is undefined.
    virtual PlotFontPtr titleFont() const = 0;
    
    // Set the font of the canvas title to the given.
    virtual void setTitleFont(const PlotFont& font) = 0;
    
    // Returns a copy of the canvas background.
    virtual PlotAreaFillPtr background() const = 0;
    
    // Sets the background of the canvas to the given.
    virtual void setBackground(const PlotAreaFill& areaFill) = 0;
    
    // Gets/sets the cursor for this canvas.
    // <group>
    virtual PlotCursor cursor() const = 0;
    virtual void setCursor(PlotCursor cursor) = 0;
    // </group>    
    
    // Refreshes the GUI, which includes redrawing plot items as needed even if
    // drawing is currently being held.
    virtual void refresh() = 0;
    
    // Indicates whether a cast to QWidget would succeed for this object.
    virtual bool isQWidget() const = 0;
    
    
    // Axes Methods //
    
    // Returns true if the given axis is shown, false otherwise.
    virtual bool axisShown(PlotAxis axis) const = 0;
    
    // Shows/hides the given axis.
    virtual void showAxis(PlotAxis axis, bool show = true) = 0;
    
    // Returns the scale for the given axis.
    virtual PlotAxisScale axisScale(PlotAxis axis) const = 0;
    
    // Sets the scale for the given axis and rescales accordingly.  May reset
    // tool stacks.
    virtual void setAxisScale(PlotAxis axis, PlotAxisScale scale) = 0;
    
    // Returns the displayed range of the given axis, in world coordinates.
    virtual pair<double, double> axisRange(PlotAxis axis) const = 0;
    
    // Sets the displayed range of the given axis, in world coordinates.
    // Implies setAxesAutoRescale(false), and does NOT reset tool stacks since
    // the tools may be using this method.
    virtual void setAxisRange(PlotAxis axis, double from, double to) = 0;
    
    // Returns true if the axes are set to autorescale when a new plot item is
    // added, false otherwise.
    virtual bool axesAutoRescale() const = 0;
    
    // Sets auto-rescaling for future plot items.  If autoRescale is true,
    // implies setAxesRatioLocked(false).
    virtual void setAxesAutoRescale(bool autoRescale = true) = 0;
    
    // Rescales the axis to show all plot items on the canvas.  Implies
    // setAxesAutoRescale(false), and may reset PlotTools.
    virtual void rescaleAxes() = 0;
    
    // Returns true if the axes ratio is locked, false otherwise.  If the
    // ratio is locked, resizing one axis will resize the others
    // proportionally.
    virtual bool axesRatioLocked() const = 0;
    
    // Sets whether the axes ratio is locked or not.
    virtual void setAxesRatioLocked(bool locked = true) = 0;
    
    // Returns whether the given Cartesian axis is shown or not.  (See
    // showCartesianAxis().)
    virtual bool cartesianAxisShown(PlotAxis axis) const = 0;
    
    // Show or hide the given Cartesian axis.  A Cartesian axis is different
    // from a normal axis in that it is inside the graph, corresponding to the
    // (0,0) location of the plot(s), rather than outside the graph.  The
    // secondary axis is the axis at which the mirrored axis will be placed at
    // the 0 value of.  For example, to show the Cartesian X_BOTTOM axis you'd
    // likely want to mirror the axis at Y_LEFT = 0, so that would be the
    // secondary axis.  If hideNormalAxis is true, it is equivalent to
    // showAxis(mirrorAxis, !hideNormalAxis).
    virtual void showCartesianAxis(PlotAxis mirrorAxis, PlotAxis secondaryAxis,
                                   bool show = true,
                                   bool hideNormalAxis = true) = 0;
    
    // Returns the label for the given axis, or an empty String if none is
    // shown.
    virtual String axisLabel(PlotAxis axis) const = 0;
    
    // Sets the label of an axis title to the given.  Setting the title to an
    // empty string should remove the title from the axis.
    virtual void setAxisLabel(PlotAxis axis, const String& title) = 0;
    
    // Returns a copy of the font used for the given axis label.  If no title
    // is shown this behavior is undefined.
    virtual PlotFontPtr axisFont(PlotAxis axis) const = 0;
    
    // Set the font of the axis title to the given.
    virtual void setAxisFont(PlotAxis axis, const PlotFont& font) = 0;
    
    // Returns true if a color bar is shown on the given axis, false otherwise.
    // A color bar should only be shown when one of more of the items on the
    // canvas is a RasterPlot or a SpectrogramPlot.  If there are multiple
    // raster or spectrograms, behavior is undefined.
    virtual bool colorBarShown(PlotAxis axis = Y_RIGHT) const = 0;
    
    // Shows/hides a color bar is shown on the given axis.  (See
    // colorBarShown().)
    virtual void showColorBar(bool show = true, PlotAxis axis = Y_RIGHT) = 0;

    
    // Miscellaneous Plotting Functionality //
    
    // Returns true if autoincrementcolors is on, false otherwise.  (See
    // setAutoIncrementColors().)
    virtual bool autoIncrementColors() const = 0;
    
    // Sets whether adding new overplotted plots should automatically
    // "increment" colors or not.  If this property is true, each new
    // overplotted plot should have a different color.  The specific handling
    // of this behavior is left up to the implementation.
    virtual void setAutoIncrementColors(bool autoInc = true) = 0;

    // Exports this canvas to the given format and returns true on success.
    virtual bool exportToFile(const PlotExportFormat& format) = 0;
    
    // Shows a file chooser dialog and return the absolute filename that the
    // user chooses.  If a directory is given, start the dialog there.  If the
    // user cancels the dialog, an empty String should be returned.
    virtual String fileChooserDialog(const String& title = "File Chooser",
                                     const String& directory = "") = 0;

    // Converts the given coordinate into a coordinate with the given system.
    virtual PlotCoordinate convertCoordinate(const PlotCoordinate& coord,
           PlotCoordinate::System newSystem = PlotCoordinate::WORLD) const = 0;
    
    // Returns the width, height, and descent for the given text in the given
    // font.  Note: this was put into place for use in a matplotlib backend,
    // but probably doesn't have much use outside that.
    virtual vector<double> textWidthHeightDescent(const String& text,
            PlotFontPtr font) const = 0;
    
    // Returns the implementation of this canvas as a Plotter::Implementation
    // value.
    virtual int implementation() const = 0;
    
    // Returns a new instance of a PlotFactory that can create plot items for
    // this implementation.  It is the caller's responsibility to delete the
    // PlotFactory when finished.
    virtual PlotFactory* implementationFactory() const = 0;
    
    
    // Plot Methods //
    
    // Hold or release drawing of plot items on the canvas.  This can be used,
    // for example, when adding many items to a canvas to avoid redrawing the
    // canvas until all items are on.  Hold/release are NOT recursive.
    // <group>
    virtual void holdDrawing() = 0;
    virtual void releaseDrawing() = 0;
    virtual bool drawingIsHeld() const = 0;
    // </group>
    
    // Adds the given plot item to this canvas, if not null or otherwise
    // invalid.  Returns whether the adding succeeded or not.  May reset
    // registered PlotTools.
    virtual bool plotItem(PlotItemPtr item, PlotCanvasLayer layer = MAIN) = 0;
    
    // Returns all plot items currently on the canvas.
    virtual vector<PlotItemPtr> allPlotItems() const = 0;
    
    // Returns all plot items currently on the given layer of the canvas.
    virtual vector<PlotItemPtr> layerPlotItems(PlotCanvasLayer layer) const= 0;
    
    // Removes the given items from this canvas (if valid) and updates the GUI.
    // May reset registered PlotTools.
    virtual void removePlotItems(const vector<PlotItemPtr>& items) = 0;
    
    
    // Selection Methods //
    
    // returns true if a line is shown for the selection tool, false otherwise
    virtual bool selectLineShown() const = 0;
    
    // set whether a line is shown for the selection tool
    virtual void setSelectLineShown(bool shown = true) = 0;
    
    // returns a copy of the line for the selection tool
    virtual PlotLinePtr selectLine() const = 0;
    
    // set the line for the selection tool
    virtual void setSelectLine(const PlotLine& line) = 0;
    
    
    // Grid Methods //
    
    // Returns whether major grid lines are shown for the x-axis
    virtual bool gridXMajorShown() const = 0;
    
    // Sets whether major grid lines are shown for the x-axis
    virtual void setGridXMajorShown(bool s = true) = 0;
    
    // Returns whether minor grid lines are shown for the x-axis
    virtual bool gridXMinorShown() const = 0;
        
    // Sets whether minor grid lines are shown for the x-axis
    virtual void setGridXMinorShown(bool s = true) = 0;
    
    // Returns whether major grid lines are shown for the y-axis
    virtual bool gridYMajorShown() const = 0;
        
    // Sets whether major grid lines are shown for the y-axis
    virtual void setGridYMajorShown(bool s = true) = 0;

    // Returns whether minor grid lines are shown for the y-axis
    virtual bool gridYMinorShown() const = 0;

    // Sets whether minor grid lines are shown for the y-ax
    virtual void setGridYMinorShown(bool s = true) = 0;
    
    // Returns a copy of the line used to draw the major grid lines.
    virtual PlotLinePtr gridMajorLine() const = 0;
    
    // Sets the line used to draw the major grid lines.
    virtual void setGridMajorLine(const PlotLine& line) = 0;
    
    // Returns a copy of the line used to draw the minor grid lines.
    virtual PlotLinePtr gridMinorLine() const = 0;
    
    // Sets the line used to draw the minor grid lines.
    virtual void setGridMinorLine(const PlotLine& line) = 0;
    
    
    // Legend Methods //
    
    // Returns whether the legend is shown or not
    virtual bool legendShown() const = 0;
    
    // Show/hide the legend.  If on is true, show the legend in the given
    // position.
    virtual void showLegend(bool on = true,
                            LegendPosition position = EXT_BOTTOM) = 0;
    
    // Returns the position of the legend.  Note: if the legend is hidden,
    // this behavior is undefined.
    virtual LegendPosition legendPosition() const = 0;
    
    // Sets the position of the legend, if it is shown.
    virtual void setLegendPosition(LegendPosition position) = 0;
    
    // Returns a copy of the line used to outline the legend.
    virtual PlotLinePtr legendLine() const = 0;
    
    // Sets the line used to outline the legend.
    virtual void setLegendLine(const PlotLine& line) = 0;
    
    // Returns a copy of the area fill used for the legend background
    virtual PlotAreaFillPtr legendFill() const = 0;
    
    // Sets the area fill used for the legend background
    virtual void setLegendFill(const PlotAreaFill& area) = 0;
    
    // Returns a copy of the font used for the legend text.
    virtual PlotFontPtr legendFont() const = 0;
    
    // Sets the font used for the legend text.
    virtual void setLegendFont(const PlotFont& font) = 0;
    
    
    // Tool Methods //
    
    // Manage mouse tools.
    // <group>
    virtual void registerMouseTool(PlotMouseToolPtr tool, bool activate = true,
                                   bool blocking = false) = 0;
    virtual vector<PlotMouseToolPtr> allMouseTools() const = 0;
    virtual vector<PlotMouseToolPtr> activeMouseTools() const = 0;
    virtual void unregisterMouseTool(PlotMouseToolPtr tool) = 0;
    // </group>
    
    // Returns the standard mouse tools associated with this canvas.  If none
    // is associated, creates and associates one that is non-active.
    virtual PlotStandardMouseToolGroupPtr standardMouseTools() = 0;
    
    
    // Operation Methods //
    
    // Returns a mutex appropriate for this implementation.
    virtual PlotMutexPtr mutex() const = 0;
    
    
    // Event Handler Methods //
    
    // register the given select event handler.
    virtual void registerSelectHandler(PlotSelectEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD) = 0;
    
    // returns a list of all registered select event handlers
    virtual vector<PlotSelectEventHandlerPtr> allSelectHandlers() const = 0;
    
    // unregister the given select event handler.
    virtual void unregisterSelectHandler(PlotSelectEventHandlerPtr handler) =0;
    
    // register the given click event handler.
    virtual void registerClickHandler(PlotClickEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD) = 0;
    
    // returns a list of all registered click event handlers
    virtual vector<PlotClickEventHandlerPtr> allClickHandlers() const = 0;
    
    // unregister the given click event handler.
    virtual void unregisterClickHandler(PlotClickEventHandlerPtr handler) = 0;
    
    // register the given mouse press event handler.
    virtual void registerMousePressHandler(PlotMousePressEventHandlerPtr hndlr,
            PlotCoordinate::System system = PlotCoordinate::WORLD) = 0;
    
    // returns a list of all registered mouse press event handlers
    virtual vector<PlotMousePressEventHandlerPtr>
            allMousePressHandlers() const = 0;
    
    // unregister the given mouse press event handler.
    virtual void unregisterMousePressHandler(
            PlotMousePressEventHandlerPtr handler) = 0;
    
    // register the given mouse release event handler.
    virtual void registerMouseReleaseHandler(
            PlotMouseReleaseEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD) = 0;
    
    // returns a list of all registered mouse release event handlers
    virtual vector<PlotMouseReleaseEventHandlerPtr>
            allMouseReleaseHandlers() const = 0;
    
    // unregister the given mouse release event handler.
    virtual void unregisterMouseReleaseHandler(
            PlotMouseReleaseEventHandlerPtr handler) = 0;
    
    // register the given mouse drag handler (only between press and release)
    virtual void registerMouseDragHandler(PlotMouseDragEventHandlerPtr h,
            PlotCoordinate::System system = PlotCoordinate::WORLD) = 0;
    
    // returns a list of all registered mouse drag event handlers
    virtual vector<PlotMouseDragEventHandlerPtr> allMouseDragHandlers()const=0;
    
    // unregister the given mouse drag event handler.
    virtual void unregisterMouseDragHandler(PlotMouseDragEventHandlerPtr h)= 0;
    
    // register the given mouse move handler.  NOTE: since this can be costly,
    // it should be avoided if possible.  Implementation canvases are expected
    // to conserve resources if no move handlers are registered.
    virtual void registerMouseMoveHandler(PlotMouseMoveEventHandlerPtr h,
            PlotCoordinate::System system = PlotCoordinate::WORLD) = 0;
    
    // returns a list of all registered mouse move event handlers
    virtual vector<PlotMouseMoveEventHandlerPtr> allMouseMoveHandlers()const=0;
    
    // unregister the given mouse move event handler.
    virtual void unregisterMouseMoveHandler(PlotMouseMoveEventHandlerPtr h)= 0;
    
    // register the given wheel event handler.
    virtual void registerWheelHandler(PlotWheelEventHandlerPtr handler,
            PlotCoordinate::System system = PlotCoordinate::WORLD) = 0;
    
    // returns a list of all registered wheel event handlers
    virtual vector<PlotWheelEventHandlerPtr> allWheelHandlers() const = 0;
    
    // unregister the given wheel event handler.
    virtual void unregisterWheelHandler(PlotWheelEventHandlerPtr handler) = 0;
    
    // the canvas should emit key events for a - z or 1 - 9, with optional
    // modifiers in {control, alt, shift} and F1 - F12.
    
    // register the given select event handler.
    virtual void registerKeyHandler(PlotKeyEventHandlerPtr handler) = 0;
    
    // returns a list of all registered select event handlers
    virtual vector<PlotKeyEventHandlerPtr> allKeyHandlers() const = 0;
    
    // unregister the given select event handler.
    virtual void unregisterKeyHandler(PlotKeyEventHandlerPtr handler) = 0;
    
    // register the given resize event handler.
    virtual void registerResizeHandler(PlotResizeEventHandlerPtr handler) = 0;
    
    // returns a list of all registered resize event handlers
    virtual vector<PlotResizeEventHandlerPtr> allResizeHandlers() const = 0;
    
    // unregister the given resize event handler.
    virtual void unregisterResizeHandler(PlotResizeEventHandlerPtr handler) =0;
    
    
    // IMPLEMENTED METHODS //
    
    // Convenience method for setting title font.
    virtual void setTitleFont(const PlotFontPtr font);
    
    // Convenience methods for setting background.
    // <group>
    virtual void setBackground(const PlotAreaFillPtr areaFill);
    virtual void setBackground(const String& color,
            PlotAreaFill::Pattern pattern = PlotAreaFill::FILL);
    // </group>
    
    // Convenience method for showing/hiding two axes at once.
    virtual void showAxes(PlotAxis xAxis, PlotAxis yAxis, bool show = true);
    
    // Convenience method for showing/hiding all four axes at once.
    virtual void showAxes(bool show = true);
    
    // Returns the displayed ranges of the given axes, in world coordinates.
    virtual PlotRegion axesRanges(PlotAxis xAxis, PlotAxis yAxis) const;
    
    // See setAxisRange(PlotAxis, double, double).
    virtual void setAxisRange(PlotAxis axis, const pair<double,double>& range);
    
    // Sets the displayed ranges of the given axes, in world coordinates.
    // Implies setAxesAutoRescale(false), and does NOT reset tool stacks since
    // the tools may be using this method.  It is recommended that subclasses
    // reimplement this to be more efficient.
    // <group>
    virtual void setAxesRanges(PlotAxis xAxis, double xFrom, double xTo,
                               PlotAxis yAxis, double yFrom, double yTo);    
    virtual void setAxesRanges(PlotAxis xAxis,
            const pair<double, double>& xRange, PlotAxis yAxis,
            const pair<double, double>& yRange);
    // </group>
    
    // Sets the displayed range of the given axes. Implies
    // setAxesAutoRescale(false),  and does NOT reset tool stacks since the
    // tools may be using this method.
    virtual void setAxesRegion(const PlotRegion& region, PlotAxis xAxis,
                               PlotAxis yAxis);
    
    // Moves (pans) the axes range by the given delta, in world coordinates.
    // Implies setAxesAutoRescale(false), and does NOT reset tool stacks since
    // the tools may be using these methods.
    // <group>
    virtual void moveAxisRange(PlotAxis axis, double delta);
    virtual void moveAxesRanges(PlotAxis xAxis, double xDelta,
                                PlotAxis yAxis, double yDelta);
    // </group>
    
    // Returns a single PlotAxesStack associated with this canvas.  Note that
    // the canvas itself doesn't modify/use this stack AT ALL; it is expressly
    // for outside tools/classes/etc to have a single stack per canvas.
    virtual PlotAxesStack& canvasAxesStack() const;
    
    // Convenience method to move along the PlotAxesStack associated with this
    // canvas (see canvasAxesStack()).  Calls the stack's moveAndReturn method,
    // and then sets the axes ranges to the returned PlotRegion.  If delta is
    // 0, the stack is moved to its base; otherwise it moves forward or
    // backward from the current stack index (see
    // PlotAxesStack::moveAndReturn()).  Returns true if the operation
    // succeeded, false otherwise (for invalid delta).
    virtual bool canvasAxesStackMove(int delta);
    
    // Convenience methods for most common Cartesian axes cases.
    // <group>
    virtual void showCartesianAxis(PlotAxis mirrorAxis, bool show = true,
                                   bool hideNormalAxis = true);
    virtual void showCartesianAxes(bool show = true, bool hideNormal = true);
    // </group>
    
    // Convenience method for clearing labels from all axes.
    virtual void clearAxesLabels();
    
    // Convenience method for setting axis font.
    virtual void setAxisFont(PlotAxis axis, const PlotFontPtr font);
        
    // Converts the given region into a region with the given system.
    virtual PlotRegion convertRegion(const PlotRegion& region,
                                     PlotCoordinate::System newSystem) const;
    
    // Returns the width, height, and descent for the given annotation.  Note:
    // this was put into place for use in a matplotlib backend, but probably
    // doesn't have much use outside that.
    virtual vector<double> annotationWidthHeightDescent(
            const PlotAnnotationPtr annot) const;
    
    // Item-specific add methods.
    // <group>
    virtual bool plot(PlotPtr plot, bool overplot = true);
    virtual bool plotPoint(PlotPointPtr point);
    virtual bool drawShape(PlotShapePtr shape);
    virtual bool drawAnnotation(PlotAnnotationPtr annotation);
    // </group>
    
    // Item-specific all methods.
    // <group>
    virtual vector<PlotPtr> allPlots() const;
    virtual vector<PlotPointPtr> allPoints() const;
    virtual vector<PlotShapePtr> allShapes() const;
    virtual vector<PlotAnnotationPtr> allAnnotations() const;
    // </group>
    
    // Item-specific layer methods.
    // <group>
    virtual vector<PlotPtr> layerPlots(PlotCanvasLayer layer) const;
    virtual vector<PlotPointPtr> layerPoints(PlotCanvasLayer layer) const;
    virtual vector<PlotShapePtr> layerShapes(PlotCanvasLayer layer) const;
    virtual vector<PlotAnnotationPtr> layerAnnotations(PlotCanvasLayer l)const;
    // </group>
    
    // Returns the number of plot items currently on the canvas.
    virtual unsigned int numPlotItems() const;
    
    // Returns the number of plot items currently on the given layer of the
    // canvas.
    virtual unsigned int numLayerPlotItems(PlotCanvasLayer layer) const;
    
    // Item-specific num methods.
    // <group>
    virtual unsigned int numPlots() const;
    virtual unsigned int numPoints() const;
    virtual unsigned int numShapes() const;
    virtual unsigned int numAnnotations() const;
    // </group>
    
    // Item-specific layer num methods.
    // <group>
    virtual unsigned int numLayerPlots(PlotCanvasLayer layer) const;
    virtual unsigned int numLayerPoints(PlotCanvasLayer layer) const;
    virtual unsigned int numLayerShapes(PlotCanvasLayer layer) const;
    virtual unsigned int numLayerAnnotations(PlotCanvasLayer layer) const;
    // </group>
    
    // Removes the given item from this canvas (if valid) and updates the GUI.
    // May reset registered PlotTools.
    virtual void removePlotItem(PlotItemPtr item);
    
    // Item-specific remove methods.
    // <group>
    virtual void removePlot(PlotPtr plot);
    virtual void removePoint(PlotPointPtr point);
    virtual void removeShape(PlotShapePtr shape);
    virtual void removeAnnotation(PlotAnnotationPtr annotation);
    // </group>
    
    // Removes the last plot item added to the canvas and updates the GUI.  May
    // reset registered PlotTools.
    virtual void removeLastPlotItem();
    
    // Item-specific remove last methods.
    // <group>
    virtual void removeLastPlot();
    virtual void removeLastPoint();
    virtual void removeLastShape();
    virtual void removeLastAnnotation();
    // </group>
    
    // Clears all plot items from this canvas and updates the GUI.  May reset
    // registered PlotTools.
    virtual void clearItems();
    
    // Item-specific clear methods.
    // <group>
    virtual void clearPlots();
    virtual void clearPoints();
    virtual void clearShapes();
    virtual void clearAnnotations();
    // </group>
    
    // Clears all plot items from the given layer on this canvas and updates
    // the GUI.  May reset registered PlotTools.
    virtual void clearLayer(PlotCanvasLayer layer);
        
    // Locate data indices that have values within the given region such that:
    // x >= region.upperLeft().x() && x <= region.lowerRight().x() &&
    // y <= region.upperLeft().y() && y >= region.lowerRight().y().
    // The returned vector has length equal to allPlots().size() (in other
    // words, each plot on the canvas has an index in the returned vector).
    // Each element of the returned vector is a list of all the data indices
    // that are in the region.  Each element in this list is a range of indices.
    // For plots that have no data in the region, the list is empty.
    // 
    // For example, say there are two plots on the canvas.  In the first plot,
    // indices 1, 2, 3, 7, and 10 are in the region; in the second plot,
    // indices 6 and 7 are in the region.  The returned value would be a vector
    // of size 2.  The first element would be the list [(1, 3), (7, 7),
    // (10, 10)] while the second element would be the list [(6, 7)].
    virtual vector<vector<pair<unsigned int, unsigned int> > >* locate(
            const PlotRegion& region) const;
    
    // Calls locate() and posts the result to the given log.
    virtual void locateAndLog(const PlotRegion& region,
            PlotLoggerPtr logger) const;
    
    // Convenience methods for setting selection line.
    // <group>
    virtual void setSelectLine(const PlotLinePtr line);
    virtual void setSelectLine(const String& color,
                               PlotLine::Style style = PlotLine::SOLID,
                               double width = 1.0);
    // </group>
    
    // Convenience methods for showing/hiding grid.
    // <group>
    virtual void setGridShown(bool showAll = true);
    virtual void setGridShown(bool xMajor, bool xMinor, bool yMajor,
                              bool yMinor);
    virtual void setGridMajorShown(bool show = true);
    virtual void setGridMinorShown(bool show = true);
    // </group>
    
    // Convenience methods for setting major/minor grid lines.
    virtual void setGridMajorLine(const PlotLinePtr line);
    virtual void setGridMajorLine(const String& color,
            PlotLine::Style style = PlotLine::SOLID, double width = 1.0);
    virtual void setGridMinorLine(const PlotLinePtr line);
    virtual void setGridMinorLine(const String& color,
            PlotLine::Style style = PlotLine::SOLID, double width = 1.0);
    // </group>
    
    // Convenience methods for setting legend line.
    // <group>
    virtual void setLegendLine(const PlotLinePtr line);
    virtual void setLegendLine(const String& color,
            PlotLine::Style style = PlotLine::SOLID, double width = 1.0);
    // </group>
    
    // Convenience methods for setting legend background.
    // <group>
    virtual void setLegendFill(const PlotAreaFillPtr area);
    virtual void setLegendFill(const String& color,
            PlotAreaFill::Pattern pattern = PlotAreaFill::FILL);
    // </group>
    
    // Convenience method for setting legend font.
    virtual void setLegendFont(const PlotFontPtr font);
    
    // Returns a PlotOperation object that can be used to track drawing
    // progress across threads.
    // <group>
    virtual PlotOperationPtr operationDraw() const;
    virtual PlotOperationPtr operationDraw(PlotMutexPtr mutex) const;
    // </group>
    
protected:
    // Provides children access to PlotTool friend methods.
    // <group>
    void attachTool(PlotToolPtr tool);
    void detachTool(PlotToolPtr tool);
    // </group>
};
typedef CountedPtr<PlotCanvas> PlotCanvasPtr;

}

#endif /*PLOTCANVAS_H_*/
