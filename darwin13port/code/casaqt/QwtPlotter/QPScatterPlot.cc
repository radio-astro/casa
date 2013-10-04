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

#include <casaqt/QwtPlotter/QPScatterPlot.h>

#include <casaqt/QwtPlotter/QPFactory.h>

#include <QPainter>

namespace casa {

///////////////////////////////
// QPSCATTERPLOT DEFINITIONS //
///////////////////////////////

// Static //

const String QPScatterPlot::CLASS_NAME = "QPScatterPlot";


// Constructors/Destructors //

QPScatterPlot::QPScatterPlot(PlotPointDataPtr data, const String& title):
        m_data(data), m_symbol(QPFactory::defaultPlotSymbol()),
        m_line(QPFactory::defaultPlotLine()),
        m_maskedSymbol(QPFactory::defaultPlotMaskedSymbol()),
        m_maskedLine(QPFactory::defaultPlotLine()),
        m_errorLine(QPFactory::defaultPlotLine()),
        m_errorCap(QPFactory::DEFAULT_ERROR_CAP) {
    QPPlotItem::setTitle(title);
    
    if(!data.null()) {
        PlotMaskedPointData* p = dynamic_cast<PlotMaskedPointData*>(&*data);
        if(p != NULL) m_maskedData = PlotMaskedPointDataPtr(p, false);
        PlotErrorData* e = dynamic_cast<PlotErrorData*>(&*data);
        if(e != NULL) m_errorData = PlotErrorDataPtr(e, false);
        PlotBinnedData* b = dynamic_cast<PlotBinnedData*>(&*data);
        if(b != NULL) m_coloredData = PlotBinnedDataPtr(b, false);
    }
    
    setItemAttribute(QwtPlotItem::AutoScale);
}

QPScatterPlot::QPScatterPlot(const ScatterPlot& copy) :
        m_data(copy.pointData()), m_symbol(QPFactory::defaultPlotSymbol()),
        m_line(QPFactory::defaultPlotLine()),
        m_maskedSymbol(QPFactory::defaultPlotMaskedSymbol()),
        m_maskedLine(QPFactory::defaultPlotLine()),
        m_errorLine(QPFactory::defaultPlotLine()),
        m_errorCap(QPFactory::DEFAULT_ERROR_CAP) {
    QPPlotItem::setTitle(copy.title());
    
    if(!m_data.null()) {
        PlotMaskedPointData* p = dynamic_cast<PlotMaskedPointData*>(&*m_data);
        if(p != NULL) m_maskedData = PlotMaskedPointDataPtr(p, false);
        PlotErrorData* e = dynamic_cast<PlotErrorData*>(&*m_data);
        if(e != NULL) m_errorData = PlotErrorDataPtr(e, false);
        PlotBinnedData* b = dynamic_cast<PlotBinnedData*>(&*m_data);
        if(b != NULL) m_coloredData = PlotBinnedDataPtr(b, false);
    }
    
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
    
    setItemAttribute(QwtPlotItem::AutoScale);
}

QPScatterPlot::~QPScatterPlot() {
    foreach(QPColor* c, m_colors) delete c;
    m_colors.clear();
    logDestruction();
}


// Public Methods //

bool QPScatterPlot::isValid() const {
    return !m_data.null() && m_data->isValid(); }


bool QPScatterPlot::shouldDraw() const {
    return isValid() && m_data->size() > 0; }

QwtDoubleRect QPScatterPlot::boundingRect() const {
    bool ret;
    double xMin, xMax, yMin, yMax;
    
    // Determine which bounding rect to return.
    if(!m_maskedData.null()) {
        bool showMasked = m_maskedLine.style() != PlotLine::NOLINE ||
                          m_maskedSymbol.symbol() != PlotSymbol::NOSYMBOL,
             showNormal = m_line.style() != PlotLine::NOLINE ||
                          m_symbol.symbol() != PlotSymbol::NOSYMBOL;
        PlotMaskedPointDataPtr data = m_maskedData;
        
        if(showMasked && !showNormal)
            ret = data->maskedMinsMaxes(xMin, xMax, yMin, yMax);
        else if(showNormal && !showMasked)
            ret = data->unmaskedMinsMaxes(xMin, xMax, yMin, yMax);
        else
            ret = data->minsMaxes(xMin, xMax, yMin, yMax);
        
    } else {
        ret = const_cast<PlotPointDataPtr&>(m_data)->minsMaxes(
                xMin, xMax, yMin, yMax);
    }
    
    // have to switch y min and max for some reason..
    if(!ret) return QwtDoubleRect();
    else return QwtDoubleRect(QPointF(xMin, yMin), QPointF(xMax, yMax));
}

QWidget* QPScatterPlot::legendItem() const {
    QwtLegendItem* i= new QwtLegendItem(m_symbol, m_line.asQPen(), qwtTitle());
    i->setIdentifierMode(QwtLegendItem::ShowLine | QwtLegendItem::ShowSymbol |
                         QwtLegendItem::ShowText);
    return i;
}


bool QPScatterPlot::linesShown() const {
    return m_line.style() != PlotLine::NOLINE; }
void QPScatterPlot::setLinesShown(bool l) {
    if(l != linesShown()) {
        m_line.setStyle(l ? PlotLine::SOLID : PlotLine::NOLINE);
        itemChanged();
    }
}

PlotLinePtr QPScatterPlot::line() const { return new QPLine(m_line); }
void QPScatterPlot::setLine(const PlotLine& line) {
    if(m_line != line) {
        m_line = line;
        itemChanged();
    }
}


PlotPointDataPtr QPScatterPlot::pointData() const { return m_data; }

bool QPScatterPlot::symbolsShown() const {
    return m_symbol.symbol() != PlotSymbol::NOSYMBOL; }
void QPScatterPlot::setSymbolsShown(bool show) {
    if(show != symbolsShown()) {
        m_symbol.setSymbol(show ? PlotSymbol::CIRCLE : PlotSymbol::NOSYMBOL);
        itemChanged();
    }
}

PlotSymbolPtr QPScatterPlot::symbol() const {
    return new QPSymbol(m_symbol); }
void QPScatterPlot::setSymbol(const PlotSymbol& sym) {
    if(sym != m_symbol) {
        m_symbol = sym;
        updateBrushes();
        itemChanged();
    }
}


PlotMaskedPointDataPtr QPScatterPlot::maskedData() const{ return m_maskedData;}

bool QPScatterPlot::maskedLinesShown() const {
    return m_maskedLine.style() != PlotLine::NOLINE; }
void QPScatterPlot::setMaskedLinesShown(bool s) {
    if(s != maskedLinesShown()) {
        m_maskedLine.setStyle(s ? PlotLine::SOLID : PlotLine::NOLINE);
        itemChanged();
    }
}

PlotLinePtr QPScatterPlot::maskedLine() const {
    return new QPLine(m_maskedLine); }
void QPScatterPlot::setMaskedLine(const PlotLine& line) {
    if(m_maskedLine != line) {
        m_maskedLine = line;
        itemChanged();
    }
}

bool QPScatterPlot::maskedSymbolsShown() const {
    return m_maskedSymbol.symbol() != PlotSymbol::NOSYMBOL; }
void QPScatterPlot::setMaskedSymbolsShown(bool s) {
    if(s != maskedSymbolsShown()) {
        m_maskedSymbol.setSymbol(s ? PlotSymbol::CIRCLE :
                                     PlotSymbol::NOSYMBOL);
        itemChanged();
    }
}

PlotSymbolPtr QPScatterPlot::maskedSymbol() const {
    return new QPSymbol(m_maskedSymbol); }
void QPScatterPlot::setMaskedSymbol(const PlotSymbol& symbol) {
    if(symbol != m_maskedSymbol) {
        m_maskedSymbol = symbol;
        updateBrushes();
        itemChanged();
    }
}


PlotErrorDataPtr QPScatterPlot::errorData() const { return m_errorData; }

bool QPScatterPlot::errorLineShown() const {
    return m_errorLine.style() != PlotLine::NOLINE; }
void QPScatterPlot::setErrorLineShown(bool s) {
    if(s != errorLineShown()) {
        m_errorLine.setStyle(s ? PlotLine::SOLID : PlotLine::NOLINE);
        itemChanged();
    }
}

PlotLinePtr QPScatterPlot::errorLine() const{ return new QPLine(m_errorLine); }
void QPScatterPlot::setErrorLine(const PlotLine& line) {
    if(m_errorLine != line) {
        m_errorLine = line;
        itemChanged();
    }
}

unsigned int QPScatterPlot::errorCapSize() const { return m_errorCap; }

void QPScatterPlot::setErrorCapSize(unsigned int capSize) {
    if(m_errorCap != capSize) {
        m_errorCap = capSize;
        itemChanged();
    }
}


PlotBinnedDataPtr QPScatterPlot::binnedColorData() const {
    return m_coloredData; }

PlotColorPtr QPScatterPlot::colorForBin(unsigned int bin) const {
    if((int)bin >= m_colors.size() || m_colors[bin] == NULL)
        return PlotColorPtr();
    else return new QPColor(*m_colors[bin]);
}

void QPScatterPlot::setColorForBin(unsigned int bin, const PlotColorPtr color){
    // Add any intermediate entries, if needed.
    for(unsigned int i = m_colors.size(); i < bin; i++)
        m_colors.insert(bin, NULL);
    
    if(color.null()) m_colors.insert(bin, NULL);
    else m_colors.insert(bin, new QPColor(*color));
    
    // Update brushes
    updateBrushes();
}


// Protected Methods //

void QPScatterPlot::draw_(QPainter* p, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& brect,
        unsigned int drawIndex, unsigned int drawCount) const {
    //logMethod("draw_", true);
    unsigned int n = m_data->size();
    if(!isValid() || n == 0 || drawIndex >= n) {
        //logMethod("draw_", false);
        return;
    }
        
    if(drawIndex + drawCount > n) drawCount = n - drawIndex;
    n = drawIndex + drawCount;

    p->save();
    
    // Draw error lines if needed.
    bool drawErrorLine = !m_errorData.null() &&
                         m_errorLine.style() != PlotLine::NOLINE;
    if(drawErrorLine) {
        p->setPen(m_errorLine.asQPen());
        unsigned int cap = m_errorCap / 2;
        double tempx, tempy, txleft, txright, tybottom, tytop;
        int temp, min, max;
        
        bool drawNormally = m_maskedData.null();
        if(!drawNormally) {
            if(m_maskedSymbol.symbol() != PlotSymbol::NOSYMBOL ||
               m_maskedLine.style() != PlotLine::NOLINE) {
                // masked points are shown, so draw all error bars
                drawNormally = true;
                
            } else {
                // only draw unmasked error lines
                for(unsigned int i = drawIndex; i < n; i++) {
                    if(!m_maskedData->maskedAt(i)) {
                        m_errorData->xyAndErrorsAt(i, tempx, tempy, txleft,
                                txright, tybottom, tytop);
                        
                        min = xMap.transform(tempx - txleft);
                        max = xMap.transform(tempx + txright);
                        temp = yMap.transform(tempy);
                        
                        if(brect.contains(min,temp)||brect.contains(max,temp)){
                            p->drawLine(min, temp, max, temp);                        
                            if(cap > 0) {
                                p->drawLine(min, temp - cap, min, temp + cap);
                                p->drawLine(max, temp - cap, max, temp + cap);
                            }
                        }
                        
                        min = yMap.transform(tempy - tybottom);
                        max = yMap.transform(tempy + tytop);
                        temp = xMap.transform(tempx);
                        
                        if(brect.contains(temp,min)||brect.contains(temp,max)){
                            p->drawLine(temp, min, temp, max);                        
                            if(cap > 0) {
                                p->drawLine(temp - cap, min, temp + cap, min);
                                p->drawLine(temp - cap, max, temp + cap, max);
                            }
                        }
                    }
                }
            }            
        }
            
        // draw all error lines
        if(drawNormally) {
            for(unsigned int i = drawIndex; i < n; i++) {
                m_errorData->xyAndErrorsAt(i, tempx, tempy, txleft,
                        txright, tybottom, tytop);
                
                min = xMap.transform(tempx - txleft);
                max = xMap.transform(tempx + txright);
                temp = yMap.transform(tempy);
                
                if(brect.contains(min, temp) || brect.contains(max, temp)) {
                    p->drawLine(min, temp, max, temp);                
                    if(cap > 0) {
                        p->drawLine(min, temp - cap, min, temp + cap);
                        p->drawLine(max, temp - cap, max, temp + cap);
                    }
                }
                
                min = yMap.transform(tempy - tybottom);
                max = yMap.transform(tempy + tytop);
                temp = xMap.transform(tempx);
                
                if(brect.contains(temp, min) || brect.contains(temp, max)) {
                    p->drawLine(temp, min, temp, max);                
                    if(cap > 0) {
                        p->drawLine(temp - cap, min, temp + cap, min);
                        p->drawLine(temp - cap, max, temp + cap, max);
                    }
                }
            }
        }
    }
    
    // Draw normal/masked lines
    bool drawLine = m_line.style() != PlotLine::NOLINE,
         drawMaskedLine = !m_maskedData.null() &&
                          m_maskedLine.style() != PlotLine::NOLINE;
    if(drawLine || drawMaskedLine) {
        double tempx, tempy, tempx2, tempy2;
        int ix, iy, ix2, iy2;
        if(!m_maskedData.null()) {
            bool mask;
            
            // set the painter's pen only once if possible
            bool samePen = m_line.asQPen() == m_maskedLine.asQPen();
            if(!drawMaskedLine || samePen) p->setPen(m_line.asQPen());
            
            m_maskedData->xyAndMaskAt(drawIndex, tempx, tempy, mask);
            ix = xMap.transform(tempx); iy = yMap.transform(tempy);
            
            for(unsigned int i = drawIndex + 1; i < n; i++) {
                m_maskedData->xAndYAt(i, tempx2, tempy2);
                ix2 = xMap.transform(tempx2); iy2 = yMap.transform(tempy2);
                if(drawLine && !mask) {
                    if(drawMaskedLine && !samePen) p->setPen(m_line.asQPen());
                    if(brect.contains(ix, iy) || brect.contains(ix2, iy2))
                        p->drawLine(ix, iy, ix2, iy2);
                } else if(drawMaskedLine && mask) {
                    if(drawLine && !samePen) p->setPen(m_maskedLine.asQPen());
                    ix2 = xMap.transform(tempx2); iy2 = yMap.transform(tempy2);
                    if(brect.contains(ix, iy) || brect.contains(ix2, iy2))
                        p->drawLine(ix, iy, ix2, iy2);
                }
                tempx = tempx2; tempy = tempy2;
                ix = ix2; iy = iy2;
            }
            
        } else {
            p->setPen(m_line.asQPen());
            m_data->xAndYAt(drawIndex, tempx, tempy);
            ix = xMap.transform(tempx); iy = yMap.transform(tempy);
            for(unsigned int i = drawIndex + 1; i < n; i++) {
                m_data->xAndYAt(i, tempx2, tempy2);
                ix2 = xMap.transform(tempx2); iy2 = yMap.transform(tempy2);
                if(brect.contains(ix, iy) || brect.contains(ix2, iy2))
                    p->drawLine(ix, iy, ix2, iy2);
                tempx = tempx2; tempy = tempy2;
                ix = ix2; iy = iy2;
            }
        }
    }
        
    // Draw normal/masked symbols
    bool drawSymbol = m_symbol.symbol() != PlotSymbol::NOSYMBOL,
         drawMaskedSymbol = !m_maskedData.null() &&
                            m_maskedSymbol.symbol() != PlotSymbol::NOSYMBOL,
         diffColor = !m_coloredData.null() && m_coloredData->isBinned();    
    if(drawSymbol || drawMaskedSymbol) {
        double tempx, tempy;
        
        if(!m_maskedData.null()) {
            bool mask;
            
            const QPen& pen = m_symbol.drawPen(),
                      & mpen = m_maskedSymbol.drawPen();
            const QBrush& brush = m_symbol.drawBrush(),
                        & mbrush = m_maskedSymbol.drawBrush();
            bool samePen = pen == mpen, sameBrush = brush == mbrush;
            
            /*
            QList<QBrush> brushes;
            if(diffColor) {
                bool allSame = true;
                for(unsigned int i = 0; i < m_coloredData->numBins(); i++) {
                    brushes << brush;
                    if((int)i < m_colors.size() && m_colors[i] != NULL) {
                        brushes[i].setColor(m_colors[i]->asQColor());
                        allSame &= brushes[i].color() == brush.color();
                    }
                }
                if(allSame) diffColor = false;
            }
            */
            
            // set the painter's pen/brush only once if possible
            if(!drawMaskedSymbol || samePen) p->setPen(pen);
            else if(!drawSymbol) p->setPen(mpen);
            if(!drawMaskedSymbol || sameBrush) p->setBrush(brush);
            else if(!drawSymbol) p->setBrush(mbrush);
            
            QSize size = ((QwtSymbol&)m_symbol).size();
            QRect rect(0, 0, size.width(), size.height());
            size = ((QwtSymbol&)m_maskedSymbol).size();
            QRect mRect(0, 0, size.width(), size.height());

            for(unsigned int i = drawIndex; i < n; i++) {
                m_maskedData->xyAndMaskAt(i, tempx, tempy, mask);
                if(drawSymbol && !mask) {
                    rect.moveCenter(QPoint(xMap.transform(tempx),
                                           yMap.transform(tempy)));
                    if(!brect.intersects(rect)) continue;
                    if(drawMaskedSymbol) {
                        if(!samePen) p->setPen(pen);
                        if(!sameBrush) p->setBrush(brush);
                    }
                    if(diffColor) {
                        p->setBrush(m_coloredBrushes[m_coloredData->binAt(i)]);
                        p->setPen(
                            m_coloredBrushes[m_coloredData->binAt(i)].color());
                    }
                    m_symbol.draw(p, rect);
                } else if(drawMaskedSymbol && mask) {
                    mRect.moveCenter(QPoint(xMap.transform(tempx),
                                            yMap.transform(tempy)));
                    if(!brect.intersects(mRect)) continue;
                    if(drawMaskedSymbol) {
                        if(!samePen) p->setPen(mpen);
                        if(!sameBrush) p->setBrush(mbrush);
                    }
                    if(diffColor) {
                        p->setBrush(m_coloredBrushes[m_coloredData->binAt(i)]);
                        p->setPen(
                            m_coloredBrushes[m_coloredData->binAt(i)].color());
                    }
                    m_maskedSymbol.draw(p, mRect);
                }
            }

        } else {
            // draw all symbols normally
            const QBrush& brush = m_symbol.drawBrush();
            p->setPen(m_symbol.drawPen());
            p->setBrush(brush);
            
            /*
            QList<QBrush> brushes;
            if(diffColor) {
                bool allSame = true;
                for(unsigned int i = 0; i < m_coloredData->numBins(); i++) {
                    brushes << brush;
                    if((int)i < m_colors.size() && m_colors[i] != NULL) {
                        brushes[i].setColor(m_colors[i]->asQColor());
                        allSame &= brushes[i].color() == brush.color();
                    }
                }
                if(allSame) diffColor = false;
            }
            */
            
            QSize size = ((QwtSymbol&)m_symbol).size();
            QRect rect(0, 0, size.width(), size.height());
                        
            for(unsigned int i = drawIndex; i < n; i++) {
                m_data->xAndYAt(i, tempx, tempy);
                rect.moveCenter(QPoint(xMap.transform(tempx),
                                       yMap.transform(tempy)));
                if(!brect.intersects(rect)) continue;
                if(diffColor) {
                    p->setBrush(m_coloredBrushes[m_coloredData->binAt(i)]);
                }
                m_symbol.draw(p, rect);
            }
        }
    }

    p->restore();
    //logMethod("draw_", false);
}


// Private Methods //

void QPScatterPlot::updateBrushes() {
    m_coloredBrushes.clear();
    if(m_coloredData.null()) return;

    const QBrush& brush = m_symbol.drawBrush();

    //bool allSame = true;
    for(unsigned int i = 0; i < m_coloredData->numBins(); i++) {
        m_coloredBrushes << brush;
        if((int)i < m_colors.size() && m_colors[i] != NULL) {
            m_coloredBrushes[i].setColor(m_colors[i]->asQColor());
            //allSame &= brushes[i].color() == brush.color();
        }
    }
    //if(allSame) diffColor = false;
}

}

#endif
