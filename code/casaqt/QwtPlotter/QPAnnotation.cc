//# QPAnnotation.cc: Qwt implementation of generic PlotAnnotation class.
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

#include <casaqt/QwtPlotter/QPAnnotation.h>

#include <casaqt/QwtPlotter/QPCanvas.qo.h>
#include <casaqt/QwtPlotter/QPFactory.h>

#include <qwt_text_label.h>

namespace casa {

//////////////////////////////
// QPANNOTATION DEFINITIONS //
//////////////////////////////

// Static //

const String QPAnnotation::CLASS_NAME = "QPAnnotation";


// Constructors/Destructors //

QPAnnotation::QPAnnotation(const String& text, const PlotCoordinate& co) :
        m_coord(co), m_orient(0) { 
    m_label.setText(text.c_str());
    setItemAttribute(QwtPlotItem::AutoScale);
    
    setFont(QPFactory::defaultAnnotationFont());
}

QPAnnotation::QPAnnotation(const String& text, const PlotCoordinate& co,
        const PlotFont& f, int o) : m_coord(co), m_orient(o) {
    QPFont font(f);
    m_label.setFont(font.asQFont());
    m_label.setColor(font.asQColor());
    m_label.setText(text.c_str());
    
    setItemAttribute(QwtPlotItem::AutoScale);
}

QPAnnotation::QPAnnotation(const PlotAnnotation& copy) :
        m_coord(copy.coordinate()), m_orient(copy.orientation()) {
    QPFont font(copy.font());
    m_label.setFont(font.asQFont());
    m_label.setColor(font.asQColor());
    m_label.setText(copy.text().c_str());

    setItemAttribute(QwtPlotItem::AutoScale);
    
    if(m_orient >= 360 || m_orient <= -360) m_orient %= 360;
}

QPAnnotation::~QPAnnotation() {
    logDestruction();
}


// Public Methods //

QwtDoubleRect QPAnnotation::boundingRect() const {
    if(m_canvas != NULL) {        
        QwtScaleMap xMap = m_canvas->asQwtPlot().canvasMap(QwtPlot::xBottom);
        QwtScaleMap yMap = m_canvas->asQwtPlot().canvasMap(QwtPlot::yLeft);
        
        int x, y;
        if(m_coord.system() == PlotCoordinate::PIXEL) {
            x = (int)(m_coord.x() + 0.5);
            y = (int)(m_coord.y() + 0.5);
        } else {
            PlotCoordinate c = m_canvas->convertCoordinate(m_coord,
                                            PlotCoordinate::WORLD);
            x = xMap.transform(c.x());
            y = yMap.transform(c.y());
        }
        
        QSize s = m_label.textSize(m_label.font());
        x += s.width();
        y += s.height();
        
        double dx, dy;
        if(m_coord.system() == PlotCoordinate::PIXEL) {
            dx = xMap.invTransform(x) - xMap.invTransform(m_coord.x());
            dy = yMap.invTransform(y) - yMap.invTransform(m_coord.y());
        } else {
            PlotCoordinate c = m_canvas->convertCoordinate(m_coord,
                                            PlotCoordinate::WORLD);
            dx = xMap.invTransform(x) - c.x();
            dy = yMap.invTransform(y) - c.y();
        }
        
        if(dx < 0) dx = -dx;
        if(dy < 0) dy = -dy;
        
        PlotCoordinate c = m_canvas->convertCoordinate(m_coord,
                                                       PlotCoordinate::WORLD);
        return QRectF(c.x(), c.y(), dx, dy);
    } else return QRectF();
}

QWidget* QPAnnotation::legendItem() const {
    QwtText text = QwtPlotItem::title();
    text.setBackgroundPen(m_label.backgroundPen());
    text.setBackgroundBrush(m_label.backgroundBrush());
    return new QwtTextLabel(text);
}


String QPAnnotation::text() const { return m_label.text().toStdString(); }

void QPAnnotation::setText(const String& newText) {
    if(newText != text()) {
        m_label.setText(newText.c_str());
        itemChanged();
    }
}

PlotFontPtr QPAnnotation::font() const {
    return new QPFont(m_label.font(), m_label.color()); }

void QPAnnotation::setFont(const PlotFont& f) {
    if(f != *font()) {
        QPFont f2(f);    
        m_label.setFont(f2.asQFont());
        m_label.setColor(f2.asQColor());
        itemChanged();
    }
}

int QPAnnotation::orientation() const { return m_orient; }

void QPAnnotation::setOrientation(int orientation) {
    if(orientation >= 360 || orientation <= -360) orientation %= 360;
    if(orientation != m_orient) {
        m_orient = orientation;
        itemChanged();
    }
}

PlotCoordinate QPAnnotation::coordinate() const { return m_coord; }

void QPAnnotation::setCoordinate(const PlotCoordinate& coord) {
    if(coord != m_coord) {     
        m_coord=(coord.system() == PlotCoordinate::WORLD || m_canvas == NULL) ?
                coord:m_canvas->convertCoordinate(coord,PlotCoordinate::WORLD);
        itemChanged();
    }
}

bool QPAnnotation::outlineShown() const {
    return m_label.backgroundPen().style() != Qt::NoPen; }

void QPAnnotation::setOutlineShown(bool show) {
    if(show != outlineShown()) {
        QPen p = m_label.backgroundPen();
        p.setStyle(show ? Qt::SolidLine : Qt::NoPen);
        m_label.setBackgroundPen(p);
        itemChanged();
    }
}

PlotLinePtr QPAnnotation::outline() const {
    return new QPLine(m_label.backgroundPen()); }

void QPAnnotation::setOutline(const PlotLine& line) {
    if(line != *outline()) {
        QPLine l(line);        
        m_label.setBackgroundPen(l.asQPen());
        itemChanged();
    }
}

PlotAreaFillPtr QPAnnotation::background() const {
    return new QPAreaFill(m_label.backgroundBrush()); }

void QPAnnotation::setBackground(const PlotAreaFill& area) {
    if(area != *background()) {
        QPAreaFill f(area);        
        m_label.setBackgroundBrush(f.asQBrush());
        itemChanged();
    }
}


QwtText& QPAnnotation::asQwtText() { return m_label; }
const QwtText& QPAnnotation::asQwtText() const { return m_label; }


// Protected Methods //

void QPAnnotation::draw_(QPainter* painter, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& /*canvasRect*/,
        unsigned int /*drawIndex*/, unsigned int /*drawCount*/) const {
    logMethod("draw_", true);
    if(!m_label.isEmpty() && m_canvas != NULL) {
        painter->save();

        QPointF p;
        if(m_coord.system() == PlotCoordinate::PIXEL)
            p = QPointF(m_coord.x(), m_coord.y());
        else {
            PlotCoordinate c = m_canvas->convertCoordinate(m_coord,
                                                       PlotCoordinate::WORLD);
            p = QPointF(xMap.transform(m_coord.x()),
                        yMap.transform(m_coord.y()));
        }
    
        if(m_orient != 0) {
            painter->rotate(m_orient);
            p = painter->worldTransform().inverted().map(p);
        }
    
        QRectF r(p, m_label.textSize(m_label.font()));
    
        if(m_label.backgroundPen().style() != Qt::NoPen ||
                m_label.backgroundBrush().style() != Qt::NoBrush) {
            // draw background        
            painter->setPen(m_label.backgroundPen());
            painter->setBrush(m_label.backgroundBrush());        
            painter->drawRect(r);
        }

        // draw text
        QPen pen = painter->pen();
        pen.setColor(m_label.color());
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        painter->setFont(m_label.font());    
        painter->drawText(r, Qt::AlignLeft | Qt::AlignTop, m_label.text());

        painter->restore();
    }
    logMethod("draw_", false);
}

}

#endif
