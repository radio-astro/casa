//# QPAxis.qo.h: Qwt implementation of generic PlotCanvas class.
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
#ifndef QPAXIS_QO_H_
#define QPAXIS_QO_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotOptions.h>
#include <graphics/GenericPlotter/PlotLogger.h>
#include <graphics/GenericPlotter/Plotter.h>
#include <casaqt/QwtPlotter/QPImageCache.h>
#include <casaqt/QwtPlotter/QPLayeredCanvas.qo.h>
#include <casaqt/QwtPlotter/QPExportCanvas.h>
#include <casaqt/QwtPlotter/QPOptions.h>
#include <casaqt/QwtPlotter/AxisListener.h>
#include <casaqt/QwtPlotter/QPPlotItem.qo.h>

#include <qwt_plot_picker.h>

#include <QtGui>

#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward declarations
class QPPlotter;
class ExternalAxisWidget;

// Implementation of PlotCanvas for the Qwt plotter.  Mainly consists of
// wrappers and containers around a QwtPlot object.
class QPAxis : public QFrame, public QPExportCanvas, public AxisListener {
    Q_OBJECT
    
    friend class QPAxesCache;
    friend class QPDrawThread;
    friend class QPLayeredCanvas;
    friend class QPPlotItem;
    friend class QPPlotter;
    
public:
    // Static //
    
    // Keep a z-order variable to increment for subsequent items.
    static double zOrder;
    
    // Convenient access to class name (QPCanvas).
    static const String CLASS_NAME;
    
    // Convenient access to "origin" names for logging.
    // <group>
    static const String DRAW_NAME;

    // </group>
    
    

    
    // Non-Static //
    
    // Constructor which takes (optional) parent QPPlotter.
    QPAxis(PlotAxis axis, QPPlotter* parent, QwtPlot* associatedPlot,
    		bool leftAxisInternal, bool bottomAxisInternal,
    		bool rightAxisInternal );

    // Destructor.
    virtual ~QPAxis();
    


    // Implements PlotCanvas::legendShown().
       bool legendShown() const{return false;};

       // Implements PlotCanvas::showLegend().
       void showLegend(bool /*on = true*/, LegendPosition /*position = EXT_BOTTOM*/){}

       // Implements PlotCanvas::legendPosition().
       LegendPosition legendPosition() const {
    	   return PlotCanvas::INT_URIGHT;
       }

       // Implements PlotCanvas::setLegendPosition().
       void setLegendPosition(LegendPosition /*position*/){}

       // Implements PlotCanvas::legendLine().
       PlotLinePtr legendLine() const {
    	   return PlotLinePtr();
       }

       // Implements PlotCanvas::setLegendLine().
       void setLegendLine(const PlotLine& /*line*/){}

       // Implements PlotCanvas::legendFill().
       PlotAreaFillPtr legendFill() const {
    	   return PlotAreaFillPtr();
       }

       // Implements PlotCanvas::setLegendFill().
       void setLegendFill(const PlotAreaFill& /*area*/){}

       // Implements PlotCanvas::legendFont().
       PlotFontPtr legendFont() const {
    	   return PlotFontPtr();
       }

       // Implements PlotCanvas::setLegendFont().
       void setLegendFont(const PlotFont& /*font*/){}

    // PlotCanvas Methods //
    virtual void setCommonAxes( bool /*commonX*/, bool /*commonY*/ ){}
    
    // Implements PlotCanvas::size().
    pair<int, int> size() const;
    virtual void setMinimumSize( int width, int height ){
    	QFrame::setMinimumSize( width, height );
    }

    bool isDrawing() const;

    virtual void show(){
    	QFrame::show();
    }

    virtual void hide(){
    	QFrame::show();
    }

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
    
    // Implements PlotCanvas::refresh().
    // <group>
    void refresh();
    void refresh(int drawLayersFlag);
    // </group>
    
    // Implements PlotCanvas::isQWidget().
    bool isQWidget() const { return true; }

    
    // Implements PlotCanvas::shownAxes().
    // Returns a bit set (really an int) of bits defined by PlotAxis enum or'd together
    PlotAxisBitset shownAxes() const;

    // Implements PlotCanvas::showAxes().
    // Takes a bit set, as an int, of bits defined by PlotAxis enum or'd together
    void showAxes(PlotAxisBitset axes);
    
    // Implements PlotCanvas::axisScale().
    PlotAxisScale axisScale(PlotAxis axis) const;

    // Implements PlotCanvas::setAxisScale().
    void setAxisScale(PlotAxis axis, PlotAxisScale scale);

    // Implements PlotCanvas::axisReferenceValueSet().
    bool axisReferenceValueSet(PlotAxis axis) const;
    
    // Implements PlotCanvas::axisReferenceValueValue().
    double axisReferenceValue(PlotAxis axis) const;
    
    // Implements PlotCanvas::setAxisReferenceValue().
    void setAxisReferenceValue(PlotAxis axis, bool on, double value = 0);
    
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

    
    // Implements PlotCanvas::axisRange().
    prange_t axisRange(PlotAxis axis) const;

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
    
       
    // Implements PlotCanvas::cachedAxesStackSizeLimit().
    int cachedAxesStackSizeLimit() const;

    // Implements PlotCanvas::setCachedAxesStackSizeLimit().
    void setCachedAxesStackSizeLimit(int sizeInKilobytes);
    
    // Overrides PlotCanvas::cachedAxesStackImageSize().
    pair<int, int> cachedAxesStackImageSize() const;
    
    // Overrides PlotCanvas::setCachedAxesStackImageSize().
    void setCachedAxesStackImageSize(int width, int height);


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
    
    virtual void setNotUsed(){}

    // Overrides PlotCanvas::clearLayer().
    void clearLayer(PlotCanvasLayer layer);

    
    // Implements PlotCanvas::holdDrawing().
    void holdDrawing();
    
    // Implements PlotCanvas::releaseDrawing().
    void releaseDrawing();
    
    // Implements PlotCanvas::drawingIsHeld().
    bool drawingIsHeld() const;

        
    // Implements PlotCanvas::setSelectLineShown().
    void setSelectLineShown(bool shown = true);
    
    // Implements PlotCanvas::selectLine().
    PlotLinePtr selectLine() const;
    
    // Implements PlotCanvas::setSelectLine().
    void setSelectLine(const PlotLine& line);

    
    // Implements PlotCanvas::gridShown().
    bool gridShown(bool* xMajor = NULL, bool* xMinor = NULL,
            bool* yMajor = NULL, bool* yMinor = NULL) const;
    
    // Implements PlotCanvas::showGrid().
    void showGrid(bool xMajor, bool xMinor, bool yMajor,bool yMinor);
    
    // Implements PlotCanvas::gridMajorLine().
    PlotLinePtr gridMajorLine() const;

    // Implements PlotCanvas::setGridMajorLine().
    void setGridMajorLine(const PlotLine& line);

    // Implements PlotCanvas::gridMinorLine().
    PlotLinePtr gridMinorLine() const;

    // Implements PlotCanvas::setGridMinorLine().
    void setGridMinorLine(const PlotLine& line);



    // Implements PlotCanvas::autoIncrementColors().
    bool autoIncrementColors() const;

    // Implements PlotCanvas::setAutoIncrementColors().
    void setAutoIncrementColors(bool autoInc = true);

    // Implements PlotCanvas::exportToFile().
    bool exportToFile(const PlotExportFormat& format);

    // Implements PlotCanvas::fileChooserDialog().
    String fileChooserDialog(const String& title = "File Chooser",
            const String& directory = "");
    
    // Implements PlotCanvas::dateFormat().
    const String& dateFormat() const;
    
    // Implements PlotCanvas::setDateFormat().
    void setDateFormat(const String& dateFormat);
    
    // Implements PlotCanvas::relativeDateFormat().
    const String& relativeDateFormat() const;
    
    // Implements PlotCanvas::setRelativeDateFormat().
    void setRelativeDateFormat(const String& dateFormat);

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
    virtual bool print( QPrinter& printer );
    virtual QImage grabImageFromCanvas(const PlotExportFormat& format );
    virtual bool print(  QPainter* painter, PlotAreaFillPtr paf, double widgetWidth,
    		double widgetHeight, int axisWidth, int axisHeight, int rowIndex, int colIndex, QRect imageRect );
    virtual const QPalette& palette() const;
    virtual QPalette::ColorRole backgroundRole() const;
    virtual bool isAxis() const {
    		return true;
    	}
    virtual bool isVertical() const {
    	bool vertical = false;
    	if ( axisType == QwtPlot::yLeft || axisType == QwtPlot::yRight ){
    		vertical = true;
    	}
    	return vertical;
    }

protected:
    // Sets the parent QPPlotter to the given.  This MUST be done when a canvas
    // is added to the plotter so that it can use the plotter's logger if
    // needed.
    void setQPPlotter(QPPlotter* parent);
    
    // Returns the parent's logger.
    virtual PlotLoggerPtr logger() const;
    
    // See QPPlotter::logObject().  If called before setQPPlotter() is called,
    // creates a queue that is then posted when setQPPlotter() is called.
    void logObject(const String& className, void* address, bool creation,
            const String& message = String());
    
    // See QPPlotter::logMethod().  Does NOT queue messages if called before
    // setQPPlotter() is called.
    void logMethod(const String& className, const String& methodName,
            bool entering, const String& message = String());
    
    // Provides access to the cached axes stack.
    // <group>
    //QPAxesCache& axesCache();
    //const QPAxesCache& axesCache() const;
    // </group>
    


private:
    // Parent QPPlotter.
    QPPlotter* m_parent;
    
    QwtPlot::Axis axisType;

    // Queued log messages before parent is set.
    vector<PlotLogObject> m_queuedLogs;
    

    // Whether the axes ratio is locked or not.
    bool m_axesRatioLocked;
    
    // Used for recalculating axes ranges if the ratio is locked.
    vector<double> m_axesRatios;

    // Date formats.
    // <group>
    String m_dateFormat;
    String m_relativeDateFormat;
    // </group>
    
    //External Axis
    ExternalAxisWidget* axisWidget;
       
    // Converts the given Qt global position to a pixel PlotCoordinate.
    // <group>
    PlotCoordinate globalPosToPixelCoord(int x, int y);
    PlotCoordinate globalPosToPixelCoord(QMouseEvent* event) {
        return globalPosToPixelCoord(event->globalX(), event->globalY()); }
    PlotCoordinate globalPosToPixelCoord(QWheelEvent* event) {
        return globalPosToPixelCoord(event->globalX(), event->globalY()); }
    // </group>
    

    
    // Converts between axes bitset flags (1,2,4,8 in PlotAxis and vector indices (0-3).
    // (Does not deal with bitsets for combinations of axes, only single axis)
    // <group>
    static unsigned int axisIndex(PlotAxis a);   
    static PlotAxis axisIndex(unsigned int i);
    // </group>
    
    
    virtual int canvasWidth() const{
      	return width();
      }
      virtual int canvasHeight() const {
      	return height();
      }


};

}

#endif

#endif /*QPCANVAS_QO_H_*/
