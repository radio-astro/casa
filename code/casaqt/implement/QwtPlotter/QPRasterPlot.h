//# QPRasterPlot.h: Qwt implementation of generic RasterPlot class.
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
#ifndef QPRASTERPLOT_H_
#define QPRASTERPLOT_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/Plot.h>
#include <casaqt/QwtPlotter/QPData.h>
#include <casaqt/QwtPlotter/QPPlotItem.qo.h>

#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// Implementation of QwtColorMap that just returns the value as a color.
class QPRasterMap : public QwtColorMap {
public:
    // Constructor.
    QPRasterMap(bool isARGB = false);
    
    // Destructor.
    ~QPRasterMap();

    // Implements QwtColorMap::copy().
    QwtColorMap* copy() const;
    
    // Implements QwtColorMap::rgb().
    QRgb rgb(const QwtDoubleInterval& interval, double value) const;
    
    // Implements QwtColorMap::colorIndex().
    unsigned char colorIndex(const QwtDoubleInterval& intv, double val) const;
    
    // Sets whether the data format is argb or not.
    void setIsARGB(bool argb);
    
private:
    // Whether the value is an rgb or an argb.
    bool m_isARGB;
};


// Implementation of RasterPlot for Qwt plotter.
class QPRasterPlot : public QPPlotItem, public RasterPlot,
                     public QwtPlotSpectrogram {
public:
    // Static //
    
    // Convenient access to class name (QPRasterPlot).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes the data, optional format and title.
    QPRasterPlot(PlotRasterDataPtr data,
                 PlotRasterData::Format format = PlotRasterData::RGB32,
                 const String& title = "Qwt Raster Plot");
    
    // Copy constructor for generic RasterPlot.
    QPRasterPlot(const RasterPlot& copy);
    
    // Destructor.
    ~QPRasterPlot();
    
    
    // Include overloaded methods.    
    using Plot::setLine;
    
    
    // PlotItem Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
    
    // Implements PlotItem::indexedDrawCount().  This returns the number of
    // points that are needed to fill in the raster plot in the current state
    // of the canvas it is attached to.
    unsigned int indexedDrawCount() const;

    
    // QwtPlotItem Methods //
    
    // Overrides QwtPlotItem::itemChanged() to call QPPlotItem's definition
    // rather than QwtPlotSpectrogram's.  (Multiple inheritance trickery.)
    void itemChanged();
    
    
    // QPPlotItem Methods //

    // Overrides QwtPlotSpectrogram::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    // Overrides QwtPlotSpectrogram::legendItem().
    QWidget* legendItem() const;
    
    
    // Plot Methods //
    
    // Implements Plot::linesShown().
    bool linesShown() const;
    
    // Implements Plot::setLinesShown().
    void setLinesShown(bool show = true);
    
    // Implements Plot::line().
    PlotLinePtr line() const;
    
    // Implements Plot::setLine().
    void setLine(const PlotLine& line);

    
    // RasterPlot Methods //
    
    // Implements RasterPlot::rasterData().
    PlotRasterDataPtr rasterData() const;
    
    // Implements RasterPlot::dataFormat().
    PlotRasterData::Format dataFormat() const;
    
    // Implements RasterPlot::setDataFormat().
    void setDataFormat(PlotRasterData::Format f);
    
    // Implements RasterPlot::dataOrigin().
    PlotRasterData::Origin dataOrigin() const;
    
    // Implements RasterPlot::setDataOrigin().
    void setDataOrigin(PlotRasterData::Origin o);
    
    // Implements RasterPlot::setXRange().
    void setXRange(double from, double to);
    
    // Implements RasterPlot::setYRange().
    void setYRange(double from, double to);
    
    // Implements RasterPlot::contourLines().
    vector<double> contourLines() const;
    
    // Implements RasterPlot::setContourLines().
    void setContourLines(const vector<double>& lines);
    
protected:
    // QPPlotItem Methods //
    
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPPlotItem::draw_().
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
private:
    QPRasterData m_data;                   // Data
    PlotRasterData::Format m_format;       // Data format
    QwtLinearColorMap m_spectMap;          // Spectrogram color map
    QPRasterMap m_rasterMap;               // Raster color map
    
    
    // Returns the rectangle in screen pixel coordinates that will contain the
    // entire raster image.
    QRect totalArea() const;
    
    // Converts between Qt's image format and CASA's.
    // <group>
    static QImage::Format format(PlotRasterData::Format f) {
        switch(f) {
        case PlotRasterData::RGB32: return QImage::Format_RGB32;
        case PlotRasterData::ARGB32: return QImage::Format_ARGB32;
        
        default: return QImage::Format_Invalid;
        }
    }    
    static PlotRasterData::Format format(QImage::Format f) {
        switch(f) {
        case QImage::Format_RGB32:  return PlotRasterData::RGB32;
        case QImage::Format_ARGB32: return PlotRasterData::ARGB32;
        
        default: return PlotRasterData::SPECTROGRAM;
        }
    }
    // </group>
};

}

#endif

#endif /*QPRASTERPLOT_H_*/
