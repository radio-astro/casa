//# QPBarPlot.cc: Qwt implementation of generic BarPlot class.
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

#include <casaqt/QwtPlotter/QPBarPlot.h>

#include <casaqt/QwtPlotter/QPFactory.h>

#include <QPainter>

#include <qwt_legend_item.h>

namespace casa {

///////////////////////////
// QPBARPLOT DEFINITIONS //
///////////////////////////

// Static //

const String QPBarPlot::CLASS_NAME = "QPBarPlot";


// Constructors/Destructors //

QPBarPlot::QPBarPlot(PlotPointDataPtr data, const String& title): m_data(data){
    QPPlotItem::setTitle(title);
    
    setLine(QPFactory::defaultShapeLine());
    setAreaFill(QPFactory::defaultBarPlotAreaFill());
    
    calculateBarWidth();
    
    setItemAttribute(QwtPlotItem::AutoScale);
}

QPBarPlot::QPBarPlot(const BarPlot& copy) : m_data(copy.data()) {
    QPPlotItem::setTitle(copy.title());
    
    setLine(copy.line());
    setAreaFill(copy.areaFill());
    
    calculateBarWidth();
    
    setItemAttribute(QwtPlotItem::AutoScale);
}

QPBarPlot::~QPBarPlot() { }


// Public Methods //

bool QPBarPlot::isValid() const { return m_data.size() > 0; }

bool QPBarPlot::linesShown() const {
    return m_line.style() != PlotLine::NOLINE; }

void QPBarPlot::setLinesShown(bool l) {
    if(l != linesShown()) {
        m_line.setStyle(l ? PlotLine::SOLID : PlotLine::NOLINE);
        itemChanged();
    }
}

PlotLinePtr QPBarPlot::line() const { return new QPLine(m_line); }

void QPBarPlot::setLine(const PlotLine& line) {
    if(line != m_line) {
        m_line = QPLine(line);
        itemChanged();
    }
}

PlotPointDataPtr QPBarPlot::pointData() const { return m_data.data(); }

bool QPBarPlot::areaIsFilled() const {
    return m_areaFill.pattern() != PlotAreaFill::NOFILL; }

void QPBarPlot::setAreaFilled(bool fill) {
    if(fill != areaIsFilled()) {
        m_areaFill.setPattern(fill? PlotAreaFill::FILL : PlotAreaFill::NOFILL);
        itemChanged();
    }
}

PlotAreaFillPtr QPBarPlot::areaFill() const {
    return new QPAreaFill(m_areaFill); }

void QPBarPlot::setAreaFill(const PlotAreaFill& areaFill) {
    if(areaFill != m_areaFill) {
        m_areaFill = QPAreaFill(areaFill);
        itemChanged();
    }
}

void QPBarPlot::draw(QPainter* p, const QwtScaleMap& xMap,
                     const QwtScaleMap& yMap, int from, int to) const {
    QPI_DRAWLOG1
    
    if(to < 0) to = m_data.size() - 1;
    if(to < from) qSwap(to, from);
    
    if(to < 0)                     to = 0;
    if(from >= (int)m_data.size()) from = m_data.size() - 1;
    
    p->save();
    p->setPen(m_line.asQPen());
    p->setBrush(m_areaFill.asQBrush());
    
    double x, y, width = m_barWidth / 2;
    int x1, x2, y1, y2;
    for(int i = from; i <= to; i++) {
        x = m_data.x(i); y = m_data.y(i);
        if(m_barWidth <= 0) {
            x1 = xMap.transform(x), y1 = yMap.transform(y);
            p->drawLine(x1, yMap.transform(0), x1, y1);
        } else {
            x1 = xMap.transform(x - width);
            x2 = xMap.transform(x + width);
            y1 = yMap.transform(0);
            y2 = yMap.transform(y);
            if(y < 0) qSwap(y1, y2);

            p->drawRect(QRect(x1, y2, abs(x2 - x1), abs(y2 - y1)));
        }
    }
    
    p->restore();
    QPI_DRAWLOG2
}

QwtDoubleRect QPBarPlot::boundingRect() const { return m_data.boundingRect(); }

QWidget* QPBarPlot::legendItem() const {
    QwtSymbol s(QwtSymbol::Rect, m_areaFill.asQBrush(), m_line.asQPen(),
                QSize(10, 10));
    QwtLegendItem* item = new QwtLegendItem(s, m_line.asQPen(),
                                            QwtPlotItem::title());
    item->setIdentifierMode(QwtLegendItem::ShowSymbol |
                            QwtLegendItem::ShowText);
    return item;
}


// Private Methods //

void QPBarPlot::calculateBarWidth() {
    m_barWidth = 0;
    unsigned int n = m_data.size();
    if(n > 1) {
        double x1 = m_data.x(0), x2 = m_data.x(1);
        double width = abs(x1 - x2);
        double minWidth = width;
        
        for(unsigned int i = 2; i < n; i++) {
            x1 = m_data.x(i);
            width = abs(x1 - x2);
            if(width < minWidth) minWidth = width;
            x2 = x1;
        }

        m_barWidth = minWidth;
    }
}

}

#endif
