//# QPRasterPlot.cc: Qwt implementation of generic RasterPlot class.
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
#ifdef AIPS_HAS_QWT

#include <casaqt/QwtPlotter/QPRasterPlot.h>

#include <casaqt/QwtPlotter/QPCanvas.qo.h>
#include <casaqt/QwtPlotter/QPOptions.h>

#include <qwt_legend_item.h>

namespace casa {

/////////////////////////////
// QPRASTERMAP DEFINITIONS //
/////////////////////////////

QPRasterMap::QPRasterMap(bool isargb) : m_isARGB(isargb) { }
QPRasterMap::~QPRasterMap() { }

QwtColorMap* QPRasterMap::copy() const { return new QPRasterMap(); }

QRgb QPRasterMap::rgb(const QwtDoubleInterval& interval, double value) const {
    if(m_isARGB) return static_cast<QRgb>(value);
    else         return 0xFF000000 | static_cast<QRgb>(value);
}

unsigned char QPRasterMap::colorIndex(const QwtDoubleInterval& interval,
        double value) const {
    return static_cast<unsigned char>(value); }

void QPRasterMap::setIsARGB(bool argb) { m_isARGB = argb; }


//////////////////////////////
// QPRASTERPLOT DEFINITIONS //
//////////////////////////////

// Static //

const String QPRasterPlot::CLASS_NAME = "QPRasterPlot";


// Constructors/Destructors //

QPRasterPlot::QPRasterPlot(PlotRasterDataPtr data, PlotRasterData::Format form,
        const String& title) : m_data(data), m_format(form),
        m_spectMap(QPOptions::standardSpectrogramMap()) {
    QPPlotItem::setTitle(title);
    setData(m_data);
    
    QPPlotItem::setItemAttribute(QwtPlotItem::AutoScale);

    if(m_format == PlotRasterData::SPECTROGRAM) setColorMap(m_spectMap);
    else {
        m_rasterMap.setIsARGB(m_format == PlotRasterData::ARGB32);
        setColorMap(m_rasterMap);
    }
    
    setDisplayMode(QwtPlotSpectrogram::ImageMode);
    setDisplayMode(QwtPlotSpectrogram::ContourMode);
}

QPRasterPlot::QPRasterPlot(const RasterPlot& copy) : m_data(copy.rasterData()),
        m_format(copy.dataFormat()),
        m_spectMap(QPOptions::standardSpectrogramMap()) {
    QPPlotItem::setTitle(copy.title());
    setData(m_data);
    
    setContourLines(copy.contourLines());
    setLine(copy.line());
    
    QPPlotItem::setItemAttribute(QwtPlotItem::AutoScale);
    
    if(m_format == PlotRasterData::SPECTROGRAM) setColorMap(m_spectMap);
    else {
        m_rasterMap.setIsARGB(m_format == PlotRasterData::ARGB32);
        setColorMap(m_rasterMap);
    }
    
    setDisplayMode(QwtPlotSpectrogram::ImageMode);
    setDisplayMode(QwtPlotSpectrogram::ContourMode);
}

QPRasterPlot::~QPRasterPlot() { }


// Public Methods //

bool QPRasterPlot::isValid() const { return m_data.isValid(); }

unsigned int QPRasterPlot::indexedDrawCount() const {
    if(m_canvas == NULL) return 0;

    QRect prect = totalArea();
    if(!prect.isValid()) return 0;
    else return prect.width() * prect.height();
}


void QPRasterPlot::itemChanged() { QPPlotItem::itemChanged(); }


QwtDoubleRect QPRasterPlot::boundingRect() const {
    return m_data.boundingRect(); }

QWidget* QPRasterPlot::legendItem() const {
    QwtLegendItem* item = new QwtLegendItem();
    item->setText(qwtTitle());
    item->setCurvePen(defaultContourPen());
    item->setIdentifierMode(QwtLegendItem::ShowLine | QwtLegendItem::ShowText);
    return item;
}


bool QPRasterPlot::linesShown() const {
    return defaultContourPen().style() != Qt::NoPen; }

void QPRasterPlot::setLinesShown(bool show) {
    if(show != linesShown()) {
        QPen p = defaultContourPen();
        p.setStyle(show ? Qt::SolidLine : Qt::NoPen);
        setDefaultContourPen(p);
        itemChanged();
    }
}

PlotLinePtr QPRasterPlot::line() const {
    return new QPLine(defaultContourPen()); }

void QPRasterPlot::setLine(const PlotLine& line) {
    QPLine l(defaultContourPen());
    if(l != line) {
        l = QPLine(line);
        setDefaultContourPen(l.asQPen());
        itemChanged();
    }
}


PlotRasterDataPtr QPRasterPlot::rasterData() const { return m_data.data(); }

void QPRasterPlot::setXRange(double from, double to) {
    if(m_data.data().null()) return;
    
    pair<double, double> r = m_data.data()->xRange();
    m_data.data()->setXRange(from, to);
    if(from != r.first || to != r.second) itemChanged();
}

void QPRasterPlot::setYRange(double from, double to) {
    if(m_data.data().null()) return;
    
    pair<double, double> r = m_data.data()->yRange();
    m_data.data()->setYRange(from, to);
    if(from != r.first || to != r.second) itemChanged();
}

PlotRasterData::Format QPRasterPlot::dataFormat() const { return m_format; }
void QPRasterPlot::setDataFormat(PlotRasterData::Format f) {    
    if(f != m_format) {
        m_format = f;
        if(m_format == PlotRasterData::SPECTROGRAM) setColorMap(m_spectMap);
        else {
            m_rasterMap.setIsARGB(m_format == PlotRasterData::ARGB32);
            setColorMap(m_rasterMap);
        }
        itemChanged();
    }
}

PlotRasterData::Origin QPRasterPlot::dataOrigin() const {
    return m_data.data()->origin(); }
void QPRasterPlot::setDataOrigin(PlotRasterData::Origin o) {
    if(o != m_data.data()->origin()) {
        m_data.data()->setOrigin(o);
        itemChanged();
    }
}

vector<double> QPRasterPlot::contourLines() const {
    QwtValueList l = contourLevels();
    vector<double> v(l.size());
    for(unsigned int i = 0; i < v.size(); i++) v[i] = l[i];
    return v;
}

void QPRasterPlot::setContourLines(const vector<double>& lines) {
    QwtValueList l;
    for(unsigned int i = 0; i < lines.size(); i++) l << lines[i];
    if(l != contourLevels()) setContourLevels(l);
}


// Protected Methods //

void QPRasterPlot::draw_(QPainter* painter, const QwtScaleMap& xMap,
          const QwtScaleMap& yMap, const QRect& canvasRect,
          unsigned int drawIndex, unsigned int drawCount) const {
    if(!canvasRect.isValid()) return;
    
    QRect adjArea = canvasRect;    
    
    // adjust for draw index and draw count if needed
    if(drawIndex > 0 || drawCount < indexedDrawCount()) {
        QRect prect = totalArea();
        if(adjArea.isValid()) {
            int height = adjArea.height(), left = adjArea.left(),
                right = adjArea.right();
            
            unsigned int tmp = 0;
            while(tmp < drawIndex) {
                left++;
                tmp += height;
            }
            if(left > adjArea.left()) left--; // need to round down one
            
            int tmp2 = (int)drawCount;
            while(right >= left && ((right - left + 1) * height) > tmp2)
                right--;
            if(right < adjArea.right()) right++; // need to round up one
            
            prect.setLeft(left);
            prect.setRight(right);

            if(prect.isValid()) adjArea &= prect;
        }
    }
    
    QwtPlotSpectrogram::draw(painter, xMap, yMap, adjArea);
}


QRect QPRasterPlot::totalArea() const {
    if(m_canvas == NULL) return QRect();
    
    PlotRegion ranges = m_canvas->axesRanges(QPPlotItem::xAxis(),
                                             QPPlotItem::yAxis());
    // have to switch the top and bottom for some stupid reason
    QwtDoubleRect area(ranges.left(), ranges.bottom(),
                       ranges.right() - ranges.left(),
                       ranges.top() - ranges.bottom());
    QwtDoubleRect brect = boundingRect();
    
    if(area == brect) return m_canvas->asQwtPlot().canvas()->contentsRect();
    
    if(brect.isValid()) area &= brect;
    
    return QPPlotItem::transform(
            m_canvas->asQwtPlot().canvasMap(qwtXAxis()),
            m_canvas->asQwtPlot().canvasMap(qwtYAxis()), area);
}

}

#endif
