//# QPScatterPlot.cc: Qwt implementation of generic ScatterPlot class.
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

#include <casaqt/QwtPlotter/QPScatterPlot.qo.h>

#include <casaqt/QwtPlotter/QPFactory.h>

#include <QPainter>

namespace casa {

/////////////////////////
// QPCURVE DEFINITIONS //
/////////////////////////

QPCurve::QPCurve(PlotPointDataPtr data) : m_data(data),
        m_symbol(QPFactory::defaultPlotSymbol()),
        m_line(QPFactory::defaultPlotLine()),
        m_maskedSymbol(QPFactory::defaultPlotSymbol()),
        m_maskedLine(QPFactory::defaultPlotLine()),
        m_errorLine(QPFactory::defaultPlotLine()),
        m_errorCap(QPFactory::DEFAULT_ERROR_CAP) {
    if(!data.null()) {
        PlotMaskedPointData* p = dynamic_cast<PlotMaskedPointData*>(&*data);
        if(p != NULL) m_maskedData = PlotMaskedPointDataPtr(p, false);
        PlotErrorData* e = dynamic_cast<PlotErrorData*>(&*data);
        if(e != NULL) m_errorData = PlotErrorDataPtr(e, false);
    }
    
    setItemAttribute(QwtPlotItem::AutoScale);
    
    // Hide masked stuff and make red.
    m_maskedSymbol.setSymbol(PlotSymbol::NOSYMBOL);
    m_maskedSymbol.setColor("FF0000");    
    m_maskedLine.setStyle(PlotLine::NOLINE);
    m_maskedLine.setColor("FF0000");
}

QPCurve::~QPCurve() { }


PlotPointDataPtr QPCurve::pointData() const { return m_data; }
PlotMaskedPointDataPtr QPCurve::maskedData() const { return m_maskedData; }
PlotErrorDataPtr QPCurve::errorData() const { return m_errorData; }

bool QPCurve::isValid() const {
    return !m_data.null() && m_data->size() > 0; }


void QPCurve::draw(QPainter* p, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, int from, int to) const {
    if(!isValid()) return;
    
    if(from < 0) from = 0;
    unsigned int n = m_data->size();
    if(to <= from) to = (int)n;
    n = min(n, (unsigned int)to);
    unsigned int s = (unsigned int)from;
    if(s == n) return;

    emit const_cast<QPCurve*>(this)->drawStarted();
    p->save();
    
    // Draw error lines if needed.
    bool drawErrorLine = !m_errorData.null() &&
                         m_errorLine.style() != PlotLine::NOLINE;
    if(drawErrorLine) {
        p->setPen(m_errorLine.asQPen());
        unsigned int cap = m_errorCap / 2;
        double tempx, tempy;
        int temp, min, max;
        
        bool drawNormally = m_maskedData.null();
        if(!drawNormally) {
            if(m_maskedSymbol.symbol() != PlotSymbol::NOSYMBOL ||
               m_maskedLine.style() != PlotLine::NOLINE) {
                // masked points are shown, so draw all error bars
                drawNormally = true;
                
            } else {
                // only draw unmasked error lines
                for(unsigned int i = s; i < n; i++) {
                    if(!m_maskedData->maskedAt(i)) {
                        tempx = m_errorData->xAt(i);
                        tempy = m_errorData->yAt(i);
                        
                        min=xMap.transform(tempx-m_errorData->xLeftErrorAt(i));
                       max=xMap.transform(tempx+m_errorData->xRightErrorAt(i));
                        temp = yMap.transform(tempy);
                        p->drawLine(min, temp, max, temp);
                        
                        if(cap > 0) {
                            p->drawLine(min, temp - cap, min, temp + cap);
                            p->drawLine(max, temp - cap, max, temp + cap);
                        }
                        
                      min=yMap.transform(tempy-m_errorData->yBottomErrorAt(i));
                        max=yMap.transform(tempy+m_errorData->yTopErrorAt(i));
                        temp = xMap.transform(tempx);
                        p->drawLine(temp, min, temp, max);
                        
                        if(cap > 0) {
                            p->drawLine(temp - cap, min, temp + cap, min);
                            p->drawLine(temp - cap, max, temp + cap, max);
                        }
                    }
                }
            }            
        }
            
        // draw all error lines
        if(drawNormally) {
            for(unsigned int i = s; i < n; i++) {
                tempx = m_errorData->xAt(i);
                tempy = m_errorData->yAt(i);
                
                min = xMap.transform(tempx - m_errorData->xLeftErrorAt(i));
                max = xMap.transform(tempx + m_errorData->xRightErrorAt(i));
                temp = yMap.transform(tempy);
                p->drawLine(min, temp, max, temp);
                
                if(cap > 0) {
                    p->drawLine(min, temp - cap, min, temp + cap);
                    p->drawLine(max, temp - cap, max, temp + cap);
                }
                
                min = yMap.transform(tempy - m_errorData->yBottomErrorAt(i));
                max = yMap.transform(tempy + m_errorData->yTopErrorAt(i));
                temp = xMap.transform(tempx);
                p->drawLine(temp, min, temp, max);
                
                if(cap > 0) {
                    p->drawLine(temp - cap, min, temp + cap, min);
                    p->drawLine(temp - cap, max, temp + cap, max);
                }
            }
        }
    }
    
    // TODO QPCurve: test which is faster: setting the pen multiple times and
    // going through the data once, or setting the pen twice and going through
    // the data twice.
    
    // Draw normal/masked lines
    bool drawLine = m_line.style() != PlotLine::NOLINE,
         drawMaskedLine = !m_maskedData.null() &&
                          m_maskedLine.style() != PlotLine::NOLINE;
    if(drawLine || drawMaskedLine) {
        if(!m_maskedData.null()) {
            // set the painter's pen only once if possible
            bool samePen = m_line.asQPen() == m_maskedLine.asQPen();
            if(!drawMaskedLine || samePen) p->setPen(m_line.asQPen());
            
            for(unsigned int i = s; i < n - 1; i++) {
                if(!m_maskedData->maskedAt(i)) {
                    if(drawMaskedLine && !samePen) p->setPen(m_line.asQPen());
                    p->drawLine(xMap.transform(m_data->xAt(i)),
                                yMap.transform(m_data->yAt(i)),
                                xMap.transform(m_data->xAt(i + 1)),
                                yMap.transform(m_data->yAt(i + 1)));
                } else if(drawMaskedLine) {
                    if(!samePen) p->setPen(m_maskedLine.asQPen());
                    p->drawLine(xMap.transform(m_data->xAt(i)),
                                yMap.transform(m_data->yAt(i)),
                                xMap.transform(m_data->xAt(i + 1)),
                                yMap.transform(m_data->yAt(i + 1)));
                }
            }
            
        } else {
            p->setPen(m_line.asQPen());
            for(unsigned int i = s; i < n - 1; i++)
                p->drawLine(xMap.transform(m_data->xAt(i)),
                            yMap.transform(m_data->yAt(i)),
                            xMap.transform(m_data->xAt(i + 1)),
                            yMap.transform(m_data->yAt(i + 1)));
        }
    }
    
    // Draw normal/masked symbols
    bool drawSymbol = m_symbol.symbol() != PlotSymbol::NOSYMBOL,
         drawMaskedSymbol = !m_maskedData.null() &&
                            m_maskedSymbol.symbol() != PlotSymbol::NOSYMBOL;
    if(drawSymbol || drawMaskedSymbol) {
        if(!m_maskedData.null()) {
            // set the painter's pen/brush only once if possible
            bool samePen = m_symbol.pen() == m_maskedSymbol.pen(),
                 sameBrush = m_symbol.brush() == m_maskedSymbol.brush();
            if(!drawMaskedSymbol || samePen) p->setPen(m_symbol.pen());
            if(!drawMaskedSymbol || sameBrush) p->setBrush(m_symbol.brush());
            
            QSize size = ((QwtSymbol&)m_symbol).size();
            QRect rect(0, 0, size.width(), size.height());
            size = ((QwtSymbol&)m_maskedSymbol).size();
            QRect mRect(0, 0, size.width(), size.height());
            for(unsigned int i = s; i < n; i++) {
                if(!m_maskedData->maskedAt(i)) {
                    rect.moveCenter(QPoint(xMap.transform(m_data->xAt(i)),
                                           yMap.transform(m_data->yAt(i))));
                    if(drawMaskedSymbol) {
                        if(!samePen) p->setPen(m_symbol.pen());
                        if(!sameBrush) p->setBrush(m_symbol.brush());
                    }
                    m_symbol.draw(p, rect);
                } else if(drawMaskedSymbol) {
                    mRect.moveCenter(QPoint(xMap.transform(m_data->xAt(i)),
                                            yMap.transform(m_data->yAt(i))));
                    if(!samePen) p->setPen(m_maskedSymbol.pen());
                    if(!sameBrush) p->setBrush(m_maskedSymbol.brush());
                    m_symbol.draw(p, mRect);
                }
            }
            
        } else {
            // draw all symbols normally
            p->setPen(m_symbol.pen());
            p->setBrush(m_symbol.brush());
            
            QSize size = ((QwtSymbol&)m_symbol).size();
            QRect rect(0, 0, size.width(), size.height());
            for(unsigned int i = s; i < n; i++) {
                rect.moveCenter(QPoint(xMap.transform(m_data->xAt(i)),
                                       yMap.transform(m_data->yAt(i))));
                m_symbol.draw(p, rect);
            }
        }
    }

    p->restore();
    emit const_cast<QPCurve*>(this)->drawEnded();
}

QwtDoubleRect QPCurve::boundingRect() const {
    double xMin, xMax, yMin, yMax;
    if(!const_cast<PlotPointDataPtr&>(m_data)->minsMaxes(xMin,xMax,yMin,yMax))
        return QwtDoubleRect();
    
    // have to switch y min and max for some reason..
    return QwtDoubleRect(QPointF(xMin, yMin), QPointF(xMax, yMax));
}

QWidget* QPCurve::legendItem() const {
    QwtLegendItem* i = new QwtLegendItem(m_symbol, m_line.asQPen(), title());
    i->setIdentifierMode(QwtLegendItem::ShowLine | QwtLegendItem::ShowSymbol |
                         QwtLegendItem::ShowText);
    return i;
}


///////////////////////////////
// QPSCATTERPLOT DEFINITIONS //
///////////////////////////////

// Static //

const String QPScatterPlot::CLASS_NAME = "QPScatterPlot";

const unsigned int QPScatterPlot::DEFAULT_ERROR_CAP = 10;


// Constructors/Destructors //

QPScatterPlot::QPScatterPlot(PlotPointDataPtr data, const String& title):
        m_curve(data) {
    QPPlotItem::setTitle(title);
    
    connect(&m_curve, SIGNAL(drawStarted()), SLOT(curveDrawingStarted()));
    connect(&m_curve, SIGNAL(drawEnded()), SLOT(curveDrawingEnded()));
}

QPScatterPlot::QPScatterPlot(const ScatterPlot& copy) :
        m_curve(copy.pointData()) {
    QPPlotItem::setTitle(copy.title());
    
    setLine(copy.line());
    setSymbol(copy.symbol());
    
    // check for masked plot
    const MaskedScatterPlot* m = dynamic_cast<const MaskedScatterPlot*>(&copy);
    if(m != NULL) {
        setMaskedLine(m->maskedLine());
        setMaskedSymbol(m->maskedSymbol());
    }
    
    // check for error plot
    const ErrorPlot* e = dynamic_cast<const ErrorPlot*>(&copy);
    if(e != NULL) {
        setErrorLine(e->errorLine());
        setErrorCapSize(e->errorCapSize());
    }
}

QPScatterPlot::~QPScatterPlot() { }


// Public Methods //

bool QPScatterPlot::isValid() const { return m_curve.isValid(); }


QwtPlotItem& QPScatterPlot::asQwtPlotItem() { return m_curve; }
const QwtPlotItem& QPScatterPlot::asQwtPlotItem() const { return m_curve; }


bool QPScatterPlot::linesShown() const {
    return m_curve.line_().style() != PlotLine::NOLINE; }
void QPScatterPlot::setLinesShown(bool l) {
    if(l != linesShown()) {
        m_curve.line_().setStyle(l ? PlotLine::SOLID : PlotLine::NOLINE);
        m_curve.itemChanged();
    }
}

PlotLinePtr QPScatterPlot::line() const { return new QPLine(m_curve.line_()); }
void QPScatterPlot::setLine(const PlotLine& line) {
    if(m_curve.line_() != line) {
        m_curve.line_() = line;
        m_curve.itemChanged();
    }
}


PlotPointDataPtr QPScatterPlot::pointData() const {
    return m_curve.pointData(); }

bool QPScatterPlot::symbolsShown() const {
    return m_curve.symbol_().symbol() != PlotSymbol::NOSYMBOL; }

void QPScatterPlot::setSymbolsShown(bool show) {
    if(show != symbolsShown()) {
        m_curve.symbol_().setSymbol(show ? PlotSymbol::CIRCLE :
                                           PlotSymbol::NOSYMBOL);
        m_curve.itemChanged();
    }
}

PlotSymbolPtr QPScatterPlot::symbol() const {
    return new QPSymbol(m_curve.symbol_()); }

void QPScatterPlot::setSymbol(const PlotSymbol& sym) {
    if(sym != m_curve.symbol_()) {
        m_curve.symbol_() = sym;
        m_curve.itemChanged();
    }
}


PlotMaskedPointDataPtr QPScatterPlot::maskedData() const {
    return m_curve.maskedData(); }

bool QPScatterPlot::maskedLinesShown() const {
    return m_curve.maskedLine_().style() != PlotLine::NOLINE; }
void QPScatterPlot::setMaskedLinesShown(bool s) {
    if(s != maskedLinesShown()) {
        m_curve.maskedLine_().setStyle(s ? PlotLine::SOLID : PlotLine::NOLINE);
        m_curve.itemChanged();
    }
}

PlotLinePtr QPScatterPlot::maskedLine() const {
    return new QPLine(m_curve.maskedLine_()); }
void QPScatterPlot::setMaskedLine(const PlotLine& line) {
    if(m_curve.maskedLine_() != line) {
        m_curve.maskedLine_() = line;
        m_curve.itemChanged();
    }
}

bool QPScatterPlot::maskedSymbolsShown() const {
    return m_curve.maskedSymbol_().symbol() != PlotSymbol::NOSYMBOL; }
void QPScatterPlot::setMaskedSymbolsShown(bool s) {
    if(s != maskedSymbolsShown()) {
        m_curve.maskedSymbol_().setSymbol(s ? PlotSymbol::CIRCLE :
                                              PlotSymbol::NOSYMBOL);
        m_curve.itemChanged();
    }
}

PlotSymbolPtr QPScatterPlot::maskedSymbol() const {
    return new QPSymbol(m_curve.maskedSymbol_()); }
void QPScatterPlot::setMaskedSymbol(const PlotSymbol& symbol) {
    if(symbol != m_curve.maskedSymbol_()) {
        m_curve.maskedSymbol_() = symbol;
        m_curve.itemChanged();
    }
}


PlotErrorDataPtr QPScatterPlot::errorData() const {
    return m_curve.errorData(); }

bool QPScatterPlot::errorLineShown() const {
    return m_curve.errorLine_().style() != PlotLine::NOLINE; }
void QPScatterPlot::setErrorLineShown(bool s) {
    if(s != errorLineShown()) {
        m_curve.errorLine_().setStyle(s ? PlotLine::SOLID : PlotLine::NOLINE);
        m_curve.itemChanged();
    }
}

PlotLinePtr QPScatterPlot::errorLine() const {
    return new QPLine(m_curve.errorLine_()); }
void QPScatterPlot::setErrorLine(const PlotLine& line) {
    if(m_curve.errorLine_() != line) {
        m_curve.errorLine_() = line;
        m_curve.itemChanged();
    }
}

unsigned int QPScatterPlot::errorCapSize() const { return m_curve.errorCap_();}

void QPScatterPlot::setErrorCapSize(unsigned int capSize) {
    if(capSize != errorCapSize()) {
        m_curve.errorCap_() = capSize;
        m_curve.itemChanged();
    }
}


// Private Slots //

void QPScatterPlot::curveDrawingStarted() {
    QPI_DRAWLOG1_(m_logger)
}

void QPScatterPlot::curveDrawingEnded() {
    QPI_DRAWLOG2_(m_logger)
}

}

#endif
