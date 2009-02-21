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

#include <casaqt/QwtPlotter/QPOptions.h>

#include <qwt_legend_item.h>

namespace casa {

//////////////////////////////
// QPRASTERPLOT DEFINITIONS //
//////////////////////////////

// Static //

const String QPRasterPlot::CLASS_NAME = "QPRasterPlot";


// Constructors/Destructors //

QPRasterPlot::QPRasterPlot(PlotRasterDataPtr data, PlotRasterData::Format form,
        const String& title) : m_data(data), m_format(form) {
    QPPlotItem::setTitle(title.c_str());
    QwtPlotSpectrogram::setData(m_data);
    
    setItemAttribute(QwtPlotItem::AutoScale);

    setColorMap(QPOptions::standardSpectrogramMap());
    setDisplayMode(QwtPlotSpectrogram::ImageMode);
    setDisplayMode(QwtPlotSpectrogram::ContourMode);
}

QPRasterPlot::QPRasterPlot(const RasterPlot& copy) : m_data(copy.rasterData()),
        m_format(copy.dataFormat()) {
    QwtPlotSpectrogram::setTitle(copy.title().c_str());
    QwtPlotSpectrogram::setData(m_data);
    
    setContourLines(copy.contourLines());
    setLine(copy.line());
    
    setItemAttribute(QwtPlotItem::AutoScale);
    
    setColorMap(QPOptions::standardSpectrogramMap());
    setDisplayMode(QwtPlotSpectrogram::ImageMode);
    setDisplayMode(QwtPlotSpectrogram::ContourMode);
}

QPRasterPlot::~QPRasterPlot() { }


// Public Methods //

bool QPRasterPlot::isValid() const { return m_data.isValid(); }


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


QwtDoubleRect QPRasterPlot::boundingRect() const {
    return m_data.boundingRect(); }

void QPRasterPlot::draw(QPainter* p, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect &rect) const {
    QPI_DRAWLOG1    
    QwtPlotSpectrogram::draw(p, xMap, yMap, rect);    
    QPI_DRAWLOG2
}

QWidget* QPRasterPlot::legendItem() const {
    QwtLegendItem* item = new QwtLegendItem();
    item->setText(QwtPlotSpectrogram::title());
    item->setCurvePen(defaultContourPen());
    item->setIdentifierMode(QwtLegendItem::ShowLine | QwtLegendItem::ShowText);
    return item;
}


// Protected Methods //

QImage QPRasterPlot::renderImage(const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QwtDoubleRect& area) const {
    if(m_format == PlotRasterData::SPECTROGRAM) {
        return QwtPlotSpectrogram::renderImage(xMap, yMap, area);
    } else {
        // basically copied from code in QwtPlotSpectogram
        if(area.isEmpty() || m_data.data().null()) return QImage();
    
        PlotRasterDataPtr d = m_data.data();
    
        QRect rect = transform(xMap, yMap, area);
        QImage image(rect.size(), format(m_format));
    
        double tx(0), ty(0);
        PlotCoordinate c;
        for(int y = rect.top(); y <= rect.bottom(); y++) {
            ty = yMap.invTransform(y);
            QRgb* line = (QRgb*)image.scanLine(y - rect.top());
            
            for(int x = rect.left(); x <= rect.right(); x++) {
                tx = xMap.invTransform(x);

                *line=static_cast<unsigned int>(m_data.data()->valueAt(tx,ty));
                
                line++;
            }
        }
    
        return image;
    }
}

}

#endif
