//# QPShape.cc: Qwt implementation of generic PlotShape classes.
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
#include <casaqt/QwtPlotter/QPShape.h>

#include <casa/BasicSL/Constants.h>

#ifdef AIPS_HAS_QWT
    #include <casaqt/QwtPlotter/QPCanvas.qo.h>
    #include <casaqt/QwtPlotter/QPFactory.h>

    #include <qwt_painter.h>
    #include <qwt_legend_item.h>

    #include <QPainter>
#endif

#include <math.h>

namespace casa {

////////////////////////////
// STATIC QPARROW METHODS //
////////////////////////////

pair<QPointF, QPointF> QPArrow::arrowPoints(QPointF from, QPointF to,
        double length) {
    if((from.x() == to.x() && from.y() == to.y()) || length <= 0)
        return pair<QPointF, QPointF>(QPointF(), QPointF());
    
    double x1 = from.x(), y1 = from.y(), x2 = to.x(), y2 = to.y();
    if(x1 > x2) {
        double temp = x1;
        x1 = x2;
        x2 = temp;
        temp = y1;
        y1 = y2;
        y2 = temp;
    }
    
    pair<QPointF, QPointF> result = arrowPointsHelper(QPointF(x1, y1),
                                    QPointF(x2, y2), length);
    
    x1 = from.x(), y1 = from.y(), x2 = to.x(), y2 = to.y();
    if(x1 > x2) {
        double dx = x1 - result.first.x();
        double dy = y1 - result.first.y();
        result.first.setX(x2 + dx);
        result.first.setY(y2 + dy);
        
        dx = x1 - result.second.x();
        dy = y1 - result.second.y();
        result.second.setX(x2 + dx);
        result.second.setY(y2 + dy);
    }
                                    
    return result;
}

void QPArrow::arrowPoints(double x1, double y1, double x2, double y2,
        double length, double& resX1, double& resY1, double& resX2,
        double& resY2) {
    QPointF from(x1, y1), to(x2, y2);
    pair<QPointF, QPointF> res = arrowPoints(from, to, length);
    resX1 = res.first.x();  resY1 = res.first.y();
    resX2 = res.second.x(); resY2 = res.second.y();
}

pair<QPointF, QPointF> QPArrow::arrowPointsHelper(QPointF p1, QPointF p2,
        double length) {
    // assumes: p2.x > p1.x, p2.y > p1.y
    double alpha =
#ifdef AIPS_HAS_QWT
        std::atan
#else
        atan          // wtf? "error: 'atan' is not a member of 'std'"
#endif
        ((p2.y() - p1.y()) / (p2.x() - p1.x()));
    double x = p2.x(), y = p2.y();
    
    double i1, i2, j1, j2;
    
    // i1, j1
    double beta = alpha - (C::pi / 4);
    if(beta == 0) {
        i1 = x - length;
        j1 = y;
    } else if(beta > 0) {
        i1 = x - (length * cos(beta));
        j1 = y - (length * sin(beta));
    } else {
        beta = -beta;
        i1 = x - (length * cos(beta));
        j1 = y + (length * sin(beta));
    }
    QPointF ij1(i1, j1);
    
    // i2, j2
    alpha = (C::pi / 2) - alpha;
    beta = alpha - (C::pi / 4);
    if(beta == 0) {
        i2 = x;
        j2 = y - length;
    } else if(beta > 0) {
        i2 = x - (length * sin(beta));
        j2 = y - (length * cos(beta));
    } else {
        beta = -beta;
        i2 = x + (length * sin(beta));
        j2 = y - (length * cos(beta));
    }
    QPointF ij2(i2, j2);
    
    return pair<QPointF, QPointF>(ij1, ij2);
}

#ifdef AIPS_HAS_QWT

/////////////////////////
// QPSHAPE DEFINITIONS //
/////////////////////////

// Static //

QPShape* QPShape::clone(const PlotShape& copy) {    
    const PlotShapeRectangle* r=dynamic_cast<const PlotShapeRectangle*>(&copy);
    if(r != NULL) return new QPRectangle(*r);
    
    const PlotShapeEllipse* e = dynamic_cast<const PlotShapeEllipse*>(&copy);
    if(e != NULL) return new QPEllipse(*e);
    
    const PlotShapePolygon* p = dynamic_cast<const PlotShapePolygon*>(&copy);
    if(p != NULL) return new QPPolygon(*p);
    
    const PlotShapeLine* l = dynamic_cast<const PlotShapeLine*>(&copy);
    if(l != NULL) return new QPLineShape(*l);
    
    const PlotShapeArrow* a = dynamic_cast<const PlotShapeArrow*>(&copy);
    if(a != NULL) return new QPArrow(*a);
    
    const PlotShapeArc* ar = dynamic_cast<const PlotShapeArc*>(&copy);
    if(ar != NULL) return new QPArc(*ar);
    
    const PlotShapePath* pa = dynamic_cast<const PlotShapePath*>(&copy);
    if(pa != NULL) return new QPPath(*pa);
    
    return NULL; // shouldn't happen
}


// Non-Static //

QPShape::QPShape() {
    setLine(QPFactory::defaultShapeLine());
    setAreaFill(QPFactory::defaultShapeAreaFill());
    
    setItemAttribute(QwtPlotItem::AutoScale);
    setItemAttribute(QwtPlotItem::Legend, false);
}

QPShape::QPShape(const PlotShape& copy) {
    setLine(copy.line());
    setAreaFill(copy.areaFill());    
    // leave the setCoordinates(copy.coordinates()) call to the subclasses
    // since we can't call it here
    
    setItemAttribute(QwtPlotItem::AutoScale);
    setItemAttribute(QwtPlotItem::Legend, false);
}

QPShape::~QPShape() { }


QWidget* QPShape::legendItem() const {
    QwtSymbol s(legendStyle(), m_area.asQBrush(), m_line.asQPen(),
                QSize(10, 10));
    QwtLegendItem* item = new QwtLegendItem(s, m_line.asQPen(),
                                            QwtPlotItem::title());
    item->setIdentifierMode(QwtLegendItem::ShowSymbol |
                            QwtLegendItem::ShowText);
    return item;
}


bool QPShape::lineShown() const { return m_line.style() != PlotLine::NOLINE; }
void QPShape::setLineShown(bool l) {
    if(l != lineShown()) {
        m_line.setStyle(l ? PlotLine::SOLID : PlotLine::NOLINE);
        itemChanged();
    }
}

PlotLinePtr QPShape::line() const { return new QPLine(m_line); }
void QPShape::setLine(const PlotLine& line) {
    if(m_line != line) {
        m_line = line;
        itemChanged();
    }
}

bool QPShape::areaFilled() const {
    return m_area.pattern() != PlotAreaFill::NOFILL; }
void QPShape::setAreaFilled(bool a) {
    if(a != areaFilled()) {
        m_area.setPattern(a ? PlotAreaFill::FILL : PlotAreaFill::NOFILL);
        itemChanged();
    }
}

PlotAreaFillPtr QPShape::areaFill() const { return new QPAreaFill(m_area); }
void QPShape::setAreaFill(const PlotAreaFill& fill) {
    if(m_area != fill) {
        m_area = fill;
        itemChanged();
    }
}


/////////////////////////////
// QPRECTANGLE DEFINITIONS //
/////////////////////////////

// Static //

const String QPRectangle::CLASS_NAME = "QPRectangle";


// Constructors/Destructors //

QPRectangle::QPRectangle(const PlotCoordinate& ul,
        const PlotCoordinate& lr) : m_upperLeft(ul), m_lowerRight(lr) { }

QPRectangle::QPRectangle(const QwtDoubleRect& r): 
    m_upperLeft(r.left(), r.top()), m_lowerRight(r.right(), r.bottom()) { }

QPRectangle::QPRectangle(const PlotShapeRectangle& copy) : QPShape(copy) {
    setCoordinates(copy.coordinates());
}

QPRectangle::~QPRectangle() {
    logDestruction();
}


bool QPRectangle::isValid() const { return boundingRect().isValid(); }

QwtDoubleRect QPRectangle::boundingRect() const {    
    if((m_upperLeft.system() == PlotCoordinate::WORLD &&
       m_lowerRight.system() == PlotCoordinate::WORLD) || m_canvas != NULL) {
        PlotCoordinate ul = m_upperLeft, lr = m_lowerRight;
        if(ul.system() != PlotCoordinate::WORLD)
            ul = m_canvas->convertCoordinate(ul, PlotCoordinate::WORLD);
        if(lr.system() != PlotCoordinate::WORLD)
            lr = m_canvas->convertCoordinate(lr, PlotCoordinate::WORLD);
        
        // have to switch the min and max y for some reason
        double x = ul.x(), y = lr.y(), width = lr.x() - x, height = y - ul.y();
        if(width < 0) width = -width;
        if(height < 0) height = -height;        
        return QRectF(x, y, width, height);        
    } else return QRectF();
}


vector<PlotCoordinate> QPRectangle::coordinates() const {
    vector<PlotCoordinate> v(2);
    v[0] = m_upperLeft;
    v[1] = m_lowerRight;
    return v;
}

void QPRectangle::setCoordinates(const vector<PlotCoordinate>& c) {
    if(c.size() >= 2) setRectCoordinates(c[0], c[1]);
}

void QPRectangle::setRectCoordinates(const PlotCoordinate& ul,
                                     const PlotCoordinate& lr) {
    if(m_upperLeft != ul || m_lowerRight != lr) {        
        m_upperLeft = ul;
        m_lowerRight = lr;        
        itemChanged();
    }
}


void QPRectangle::draw_(QPainter* painter, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& canvasRect,
        unsigned int drawIndex, unsigned int drawCount) const {
    logMethod("draw_", true);
    QRectF rect = boundingRect();
    if(rect.isValid()) {
        const QRect r = transform(xMap, yMap, rect);
        painter->setPen(m_line.asQPen());
        painter->setBrush(m_area.asQBrush());        
        painter->drawRect(r);
    }
    logMethod("draw_", false);
}


///////////////////////////
// QPELLIPSE DEFINITIONS //
///////////////////////////

// Static //

const String QPEllipse::CLASS_NAME = "QPEllipse";


// Constructors/Destructors //

QPEllipse::QPEllipse(const PlotCoordinate& center,
        const PlotCoordinate& radii) : m_center(center), m_radii(radii) { }

QPEllipse::QPEllipse(const QwtDoubleRect& r) {
    QPointF c = r.center();
    m_center = PlotCoordinate(c.x(), c.y());
    m_radii = PlotCoordinate(r.width() / 2, r.height() / 2);
}

QPEllipse::QPEllipse(const PlotShapeEllipse& copy) : QPShape(copy) {
    setCoordinates(copy.coordinates());
}

QPEllipse::~QPEllipse() {
    logDestruction();
}


bool QPEllipse::isValid() const { return boundingRect().isValid(); }

QwtDoubleRect QPEllipse::boundingRect() const {
    if((m_center.system() == PlotCoordinate::WORLD &&
       m_radii.system() == PlotCoordinate::WORLD) || m_canvas != NULL) {
        PlotCoordinate c = m_center, r = m_radii;
        if(c.system() != PlotCoordinate::WORLD)
            c = m_canvas->convertCoordinate(c, PlotCoordinate::WORLD);
        if(r.system() != PlotCoordinate::WORLD)
            r = m_canvas->convertCoordinate(r, PlotCoordinate::WORLD);
        
        // Have to have min y for some reason.
        double x = c.x() - r.x(), y = c.y() - r.y();
        double width = 2 * r.x(), height = 2 * r.y();
        return QRectF(x, y, width, height);
        
    } else return QRectF();
}


vector<PlotCoordinate> QPEllipse::coordinates() const {
    vector<PlotCoordinate> v(2);
    v[0] = m_center;
    v[1] = m_radii;
    return v;
}

void QPEllipse::setCoordinates(const vector<PlotCoordinate>& c) {
    if(c.size() >= 2 && (m_center != c[0] || m_radii != c[1])) {
        m_center = c[0];
        m_radii = c[1];        
        itemChanged();
    }
}

void QPEllipse::setEllipseCoordinates(const PlotCoordinate& c,
                                      const PlotCoordinate& r) {
    if(c != m_center || r != m_radii) {
        m_center = c;
        m_radii = r;
        itemChanged();
    }
}

PlotCoordinate QPEllipse::radii() const { return m_radii; }

void QPEllipse::setRadii(const PlotCoordinate& radii) {
    if(m_radii != radii) {
        m_radii = radii;
        itemChanged();
    }
}

PlotCoordinate QPEllipse::center() const { return m_center; }

void QPEllipse::setCenter(const PlotCoordinate& center) {
    if(m_center != center) {
        m_center = center;
        itemChanged();
    }
}


void QPEllipse::draw_(QPainter* painter, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& canvasRect,
        unsigned int drawIndex, unsigned int drawCount) const {
    logMethod("draw_", true);
    QRectF rect = boundingRect();
    if(rect.isValid()) {
        int x, y, width, height;
        if(m_center.system() == PlotCoordinate::PIXEL) {
            x = (int)(m_center.x() + 0.5);
            y = (int)(m_center.y() + 0.5);
        } else {
            PlotCoordinate c = m_canvas->convertCoordinate(m_center,
                                             PlotCoordinate::WORLD);
            x = xMap.transform(c.x());
            y = yMap.transform(c.y());
        }
        if(m_radii.system() == PlotCoordinate::PIXEL) {
            width = (int)((2 * m_radii.x()) + 0.5);
            height = (int)((2 * m_radii.y()) + 0.5);
        } else {
            PlotCoordinate c = m_canvas->convertCoordinate(m_center,
                                             PlotCoordinate::WORLD);
            PlotCoordinate r = m_canvas->convertCoordinate(m_radii,
                                             PlotCoordinate::WORLD);
            width = xMap.transform(c.x() + r.x()) -
                    xMap.transform(c.x() - r.x());
            height = yMap.transform(c.y() + r.y()) -
                     yMap.transform(c.y() - r.y());
        }
        x = (int)(x - (width/2.0) + 0.5);
        y = (int)(y - (height/2.0) + 0.5);
        
        QRect r(x, y, width, height);
        painter->setPen(m_line.asQPen());
        painter->setBrush(m_area.asQBrush());
        painter->drawEllipse(r);
    }
    logMethod("draw_", false);
}


///////////////////////////
// QPPOLYGON DEFINITIONS //
///////////////////////////

// Static //

const String QPPolygon::CLASS_NAME = "QPPolygon";


// Constructors/Destructors //

QPPolygon::QPPolygon(const vector<PlotCoordinate>& c): m_coords(c) { }

QPPolygon::QPPolygon(const PlotShapePolygon& copy) : QPShape(copy) {
    setCoordinates(copy.coordinates());
}

QPPolygon::~QPPolygon() {
    logDestruction();
}


bool QPPolygon::isValid() const {
    return m_coords.size() >= 3 && boundingRect().isValid(); }

QwtDoubleRect QPPolygon::boundingRect() const {
    bool allWorld = true;
    for(unsigned int i = 0; i < m_coords.size(); i++) {
        if(m_coords[i].system() != PlotCoordinate::WORLD) {
            allWorld = false;
            break;
        }
    }
    
    if(allWorld || m_canvas != NULL) {
        QPolygonF p(m_coords.size());
        for(unsigned int i = 0; i < m_coords.size(); i++) {
            PlotCoordinate c = m_coords[i];
            if(c.system() != PlotCoordinate::WORLD)
                c = m_canvas->convertCoordinate(c, PlotCoordinate::WORLD);
            
            p[i].setX(c.x());
            p[i].setY(c.y());
        }
        return p.boundingRect();
        
    } else return QRectF();
}


vector<PlotCoordinate> QPPolygon::coordinates() const { return m_coords; }

void QPPolygon::setCoordinates(const vector<PlotCoordinate>& c) {
    if(m_coords != c) {   
        m_coords = c;
        itemChanged();
    }
}


void QPPolygon::draw_(QPainter* painter, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& canvasRect,
        unsigned int drawIndex, unsigned int drawCount) const {
    logMethod("draw_", true);
    if(isValid() && m_canvas != NULL) {
        unsigned int n = m_coords.size();
        if(drawIndex >= n) return;
        if(drawIndex + drawCount > n) drawCount = n - drawIndex;
        n = drawIndex + drawCount;
        
        QwtPolygon p(n);
        for(unsigned int i = drawIndex; i < n; i++) {
            PlotCoordinate c = m_coords[i];
            if(c.system() != PlotCoordinate::WORLD)
                c = m_canvas->convertCoordinate(c, PlotCoordinate::WORLD);
            
            p[i].setX(xMap.transform(c.x()));
            p[i].setY(yMap.transform(c.y()));
        }
        
        painter->setPen(m_line.asQPen());
        painter->setBrush(m_area.asQBrush());
        painter->drawPolygon(p);
    }
    logMethod("draw_", false);
}


/////////////////////////////
// QPLINESHAPE DEFINITIONS //
/////////////////////////////

// Static //

const String QPLineShape::CLASS_NAME = "QPLineShape";


// Constructors/Destructors //

QPLineShape::QPLineShape(double location, PlotAxis axis) {
    setLineCoordinates(location, axis);
    m_marker.setLinePen(m_line.asQPen());
    m_marker.setItemAttribute(QwtPlotItem::AutoScale, false);
}

QPLineShape::QPLineShape(const PlotShapeLine& copy) {
    setLineCoordinates(copy.location(), copy.axis());
    m_marker.setLinePen(m_line.asQPen());
    m_marker.setItemAttribute(QwtPlotItem::AutoScale, false);
}

QPLineShape::~QPLineShape() {
    logDestruction();
}


bool QPLineShape::isValid() const { return true; }

QwtDoubleRect QPLineShape::boundingRect() const {
    return m_marker.boundingRect(); }


vector<PlotCoordinate> QPLineShape::coordinates() const {
    vector<PlotCoordinate> v(1);
    v[0] = PlotCoordinate((m_axis == X_BOTTOM || m_axis == X_TOP) ?
                                 m_marker.xValue() : 0,
                          (m_axis == Y_LEFT || m_axis == Y_RIGHT) ?
                                 m_marker.yValue() : 0);
    return v;
}

void QPLineShape::setCoordinates(const vector<PlotCoordinate>& coords) {
    if(coords.size() >= 1)
        m_marker.setValue(coords[0].x(), coords[0].y());
}

void QPLineShape::setLineCoordinates(double location, PlotAxis axis) {
    m_axis = axis;
    switch(axis) {
    case X_BOTTOM: case X_TOP:
        m_marker.setXValue(location);
        m_marker.setLineStyle(QwtPlotMarker::VLine);
        break;
    
    case Y_LEFT: case Y_RIGHT:
        m_marker.setYValue(location);
        m_marker.setLineStyle(QwtPlotMarker::HLine);
    }
}

double QPLineShape::location() const {
    switch(m_axis) {
    case X_BOTTOM: case X_TOP: return m_marker.xValue();
    case Y_LEFT: case Y_RIGHT: return m_marker.yValue();
    
    default: return 0;
    }
}

PlotAxis QPLineShape::axis() const { return m_axis; }


void QPLineShape::draw_(QPainter* painter, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& canvasRect,
        unsigned int drawIndex, unsigned int drawCount) const {
    logMethod("draw_", true);
    const_cast<QwtPlotMarker&>(m_marker).setLinePen(m_line.asQPen());
    m_marker.draw(painter, xMap, yMap, canvasRect);
    logMethod("draw_", false);
}


/////////////////////////
// QPARROW DEFINITIONS //
/////////////////////////

// Static //

const String QPArrow::CLASS_NAME = "QPArrow";


// Constructors/Destructors //

QPArrow::QPArrow(const PlotCoordinate& from, const PlotCoordinate& to) :
        m_from(from), m_to(to), m_fromStyle(PlotShapeArrow::NOARROW),
        m_toStyle(PlotShapeArrow::V_ARROW), m_size(20) { }

QPArrow::QPArrow(const PlotShapeArrow& copy) {
    setCoordinates(copy.coordinates());
    setArrowStyles(copy.arrowStyleFrom(), copy.arrowStyleTo());
    setArrowSize(copy.arrowSize());
}

QPArrow::~QPArrow() {
    logDestruction();
}


bool QPArrow::isValid() const { return boundingRect().isValid(); }

QwtDoubleRect QPArrow::boundingRect() const {
    if((m_from.system() == PlotCoordinate::WORLD &&
       m_to.system() == PlotCoordinate::WORLD) || m_canvas != NULL) {
        PlotCoordinate f = m_from, t = m_to;
        if(f.system() != PlotCoordinate::WORLD)
            f = m_canvas->convertCoordinate(f, PlotCoordinate::WORLD);
        if(t.system() != PlotCoordinate::WORLD)
            t = m_canvas->convertCoordinate(t, PlotCoordinate::WORLD);
        
        // have to switch the min and max y values for some reason
        double x = (f.x() < t.x()) ? f.x() : t.x();
        double y = (f.y() > t.y()) ? f.y() : t.y();
        double w = ((f.x() < t.x()) ? t.x() : f.x()) - x;
        double h = ((f.y() > t.y()) ? t.y() : f.y()) - y;
        
        if(w < 0) w = -w;
        if(h < 0) h = -h;
        
        return QRectF(x, y, w, h);

    } else return QRectF();
}


vector<PlotCoordinate> QPArrow::coordinates() const {
    vector<PlotCoordinate> v(2);
    v[0] = m_from;
    v[1] = m_to;
    return v;
}

void QPArrow::setCoordinates(const vector<PlotCoordinate>& coords) {
    if(coords.size() >= 2 && (m_from != coords[0] || m_to != coords[1])) {
        m_from = coords[0];
        m_to = coords[1];        
        itemChanged();
    }
}

void QPArrow::setArrowCoordinates(const PlotCoordinate& from,
                                  const PlotCoordinate& to) {
    if(from != m_from || to != m_to) {
        m_from = from;
        m_to = to;        
        itemChanged();
    }
}

PlotShapeArrow::Style QPArrow::arrowStyleFrom() const { return m_fromStyle; }
PlotShapeArrow::Style QPArrow::arrowStyleTo() const { return m_toStyle; }

void QPArrow::setArrowStyles(Style from, Style to) {
    if(from != m_fromStyle || to != m_toStyle) {
        m_fromStyle = from;
        m_toStyle = to;
        itemChanged();
    }
}

double QPArrow::arrowSize() const { return m_size; }

void QPArrow::setArrowSize(double size) {
    if(size != m_size && size >= 1) {
        m_size = size;
        itemChanged();
    }
}


void QPArrow::draw_(QPainter* painter, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& canvasRect,
        unsigned int drawIndex, unsigned int drawCount) const {
    logMethod("draw_", true);
    QRectF rect = boundingRect();
    if(rect.isValid() || m_from.x() == m_to.x() || m_from.y() == m_to.y()) {        
        painter->setPen(m_line.asQPen());
        painter->setBrush(m_area.asQBrush());
        
        PlotCoordinate f = m_from, t = m_to;
        if(f.system() != PlotCoordinate::WORLD)
            f = m_canvas->convertCoordinate(f, PlotCoordinate::WORLD);
        if(t.system() != PlotCoordinate::WORLD)
            t = m_canvas->convertCoordinate(t, PlotCoordinate::WORLD);
        
        // first draw the line
        int x1, x2, y1, y2;
        if(m_from.system() == PlotCoordinate::PIXEL) {
            x1 = (int)(m_from.x() + 0.5);
            y1 = (int)(m_from.y() + 0.5);
        } else {
            x1 = xMap.transform(f.x());
            y1 = yMap.transform(f.y());
        }
        if(m_to.system() == PlotCoordinate::PIXEL) {
            x2 = (int)(m_to.x() + 0.5);
            y2 = (int)(m_to.y() + 0.5);
        } else {
            x2 = xMap.transform(t.x());
            y2 = yMap.transform(t.y());
        }
        painter->drawLine(x1, y1, x2, y2);
        
        // from arrow
        if(m_fromStyle == TRIANGLE || m_fromStyle == V_ARROW) {
            pair<QPointF, QPointF> points = arrowPoints(QPointF(x2, y2),
                                            QPointF(x1, y1), m_size);
            
            if(m_fromStyle == V_ARROW) {
                painter->drawLine(QPOptions::round(points.first.x()),
                                  QPOptions::round(points.first.y()), x1, y1);
                painter->drawLine(QPOptions::round(points.second.x()),
                                  QPOptions::round(points.second.y()), x1, y1);
            } else {
                QPolygonF polygon;
                polygon << points.first << points.second << QPointF(x1, y1);
                painter->drawPolygon(polygon);
            }
        }
        
        // to arrow
        if(m_toStyle == TRIANGLE || m_toStyle == V_ARROW) {
            pair<QPointF, QPointF> points = arrowPoints(QPointF(x1, y1),
                                            QPointF(x2, y2), m_size);
            
            if(m_toStyle == V_ARROW) {
                painter->drawLine(QPOptions::round(points.first.x()),
                                  QPOptions::round(points.first.y()), x2, y2);
                painter->drawLine(QPOptions::round(points.second.x()),
                                  QPOptions::round(points.second.y()), x2, y2);
            } else {
                QPolygonF polygon;
                polygon << points.first << points.second << QPointF(x2, y2);
                painter->drawPolygon(polygon);
            }
        }
    }
    logMethod("draw_", false);
}


////////////////////////
// QPPATH DEFINITIONS //
////////////////////////

// Static //

const String QPPath::CLASS_NAME = "QPPath";


// Constructors/Destructors //

QPPath::QPPath(const vector<PlotCoordinate>& points) : QPShape(),
        m_coords(points) { }

QPPath::QPPath(const PlotShapePath& copy) : QPShape(copy),
        m_coords(copy.coordinates()) { }

QPPath::~QPPath() {
    logDestruction();
}


bool QPPath::isValid() const { return m_coords.size() > 1; }

QwtDoubleRect QPPath::boundingRect() const {
    if(m_coords.size() <= 1) return QwtDoubleRect();
    if(m_canvas == NULL) {
        for(unsigned int i = 0; i < m_coords.size(); i++)
            if(m_coords[i].system() != PlotCoordinate::WORLD)
                return QwtDoubleRect();
    }
    
    QVector<QPointF> points;
    PlotCoordinate c;
    for(unsigned int i = 0; i < m_coords.size(); i++) {
        c = m_coords[i];
        if(c.system() != PlotCoordinate::WORLD)
            c = m_canvas->convertCoordinate(c, PlotCoordinate::WORLD);
        points << QPointF(c.x(), c.y());
    }
    
    return QPolygonF(points).boundingRect();
}


vector<PlotCoordinate> QPPath::coordinates() const { return m_coords; }

void QPPath::setCoordinates(const vector<PlotCoordinate>& coords) {
    if(coords != m_coords) {
        m_coords = coords;
        itemChanged();
    }
}


void QPPath::draw_(QPainter* painter, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& canvasRect,
        unsigned int drawIndex, unsigned int drawCount) const {
    logMethod("draw_", true);
    unsigned int n = m_coords.size();
    if(n <= 1 || m_canvas == NULL || drawIndex >= n) {
        logMethod("draw_", false);
        return;
    }
    
    if(drawIndex + drawCount > n) drawCount = n - drawIndex;
    n = drawIndex + drawCount;
    
    /*
    QVector<QPoint> points;
    PlotCoordinate c;
    for(unsigned int i = 0; i < m_coords.size(); i++) {
        if(m_coords[i].system() == PlotCoordinate::PIXEL)
            points << QPoint((int)(m_coords[i].x() + 0.5),
                             (int)(m_coords[i].y() + 0.5));
        else {
            c= m_canvas->convertCoordinate(m_coords[i], PlotCoordinate::WORLD);
            points << QPoint(xMap.transform(c.x()), yMap.transform(c.y()));
        }
        
        // because qpainter::drawlines expects pairs, we have to duplicate
        if(i > 0 && i < m_coords.size() - 1)
            points << points[points.size() - 1];
    }
    */
    
    //QPainterPath path;
    QVector<QPoint> v;
    PlotCoordinate c;
    int x, y;
    for(unsigned int i = drawIndex; i < n; i++) {
        if(m_coords[i].system() == PlotCoordinate::PIXEL) {
            /*
            if(i > 0) path.lineTo(m_coords[i].x(), m_coords[i].y());
            else path.moveTo(m_coords[i].x(), m_coords[i].y());
            */
            v << QPoint((int)(m_coords[i].x() + 0.5),
                        (int)(m_coords[i].y() + 0.5));
        } else {
            c= m_canvas->convertCoordinate(m_coords[i], PlotCoordinate::WORLD);
            x = xMap.transform(c.x()); y = yMap.transform(c.y());
            //if(i > 0) path.lineTo(x, y);
            //else path.moveTo(x, y);
            v << QPoint(x, y);
        }
    }
    
    painter->save();
    painter->setPen(m_line.asQPen());
    painter->setBrush(m_area.asQBrush());
    
    //painter->drawPath(path);
    painter->drawPolyline(QPolygon(v));
    
    painter->restore();
    logMethod("draw_", false);
}


///////////////////////
// QPARC DEFINITIONS //
///////////////////////

// Static //

const String QPArc::CLASS_NAME = "QPArc";


// Constructors/Destructors //

QPArc::QPArc(const PlotCoordinate& start, const PlotCoordinate& widthHeight,
        int startAngle, int spanAngle) : m_start(start), m_size(widthHeight),
        m_startAngle(startAngle), m_spanAngle(spanAngle), m_orient(0) { }

QPArc::QPArc(const PlotShapeArc& copy) : QPShape(copy),
        m_start(copy.startCoordinate()), m_size(copy.widthHeight()),
        m_startAngle(copy.startAngle()), m_spanAngle(copy.spanAngle()),
        m_orient(copy.orientation()) {
    m_orient %= 360;
}

QPArc::~QPArc() {
    logDestruction();
}


bool QPArc::isValid() const { return boundingRect().isValid(); }

QwtDoubleRect QPArc::boundingRect() const {
    if((m_start.system() == PlotCoordinate::WORLD &&
       m_size.system() == PlotCoordinate::WORLD) || m_canvas != NULL) {
        PlotCoordinate s = m_start, wh = m_size;
        if(s.system() != PlotCoordinate::WORLD)
            s = m_canvas->convertCoordinate(s, PlotCoordinate::WORLD);
        if(wh.system() != PlotCoordinate::WORLD)
            wh = m_canvas->convertCoordinate(wh, PlotCoordinate::WORLD);
        return QRectF(s.x(), s.y(), wh.x(), wh.y());
    } else return QwtDoubleRect();
}


vector<PlotCoordinate> QPArc::coordinates() const {
    vector<PlotCoordinate> v(2);
    v[0] = m_start;
    v[1] = m_size;
    return v;
}

void QPArc::setCoordinates(const vector<PlotCoordinate>& coords) {
    if(coords.size() >= 2) {
        m_start = coords[0];
        m_size = coords[1];        
        itemChanged();
    }
}

PlotCoordinate QPArc::startCoordinate() const { return m_start; }

void QPArc::setStartCoordinate(const PlotCoordinate& coord) {
    if(m_start != coord) {
        m_start = coord;
        itemChanged();
    }
}

PlotCoordinate QPArc::widthHeight() const { return m_size; }

void QPArc::setWidthHeight(const PlotCoordinate& widthHeight) {
    if(m_size != widthHeight) {
        m_size = widthHeight;
        itemChanged();
    }
}

int QPArc::startAngle() const { return m_startAngle; }
void QPArc::setStartAngle(int startAngle) {
    if(m_startAngle != startAngle) {
        m_startAngle = startAngle;
        itemChanged();
    }
}

int QPArc::spanAngle() const { return m_spanAngle; }
void QPArc::setSpanAngle(int spanAngle) {
    if(m_spanAngle != spanAngle) {
        m_spanAngle = spanAngle;
        itemChanged();
    }
}

int QPArc::orientation() const { return m_orient; }
void QPArc::setOrientation(int o) {
    if(o % 360 != m_orient) {
        m_orient = o % 360;
        itemChanged();
    }
}


void QPArc::draw_(QPainter* painter, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& canvasRect,
        unsigned int drawIndex, unsigned int drawCount) const {
    logMethod("draw_", true);
    if(m_canvas != NULL) {
        painter->save();
        painter->setBrush(m_area.asQBrush());
        
        int x, y, width, height;
        if(m_start.system() == PlotCoordinate::PIXEL) {
            x = (int)(m_start.x() + 0.5);
            y = (int)(m_start.y() + 0.5);
            width = (int)(m_size.x() + 0.5);
            height = (int)(m_size.y() + 0.5);
        } else {
            PlotCoordinate s = m_canvas->convertCoordinate(m_start,
                                            PlotCoordinate::WORLD);
            PlotCoordinate wh = m_canvas->convertCoordinate(m_size,
                                            PlotCoordinate::WORLD);
            x = xMap.transform(s.x());
            y = yMap.transform(s.y());
            width = xMap.transform(s.x() + wh.x()) - x;
            height = yMap.transform(s.y() + wh.y()) - y;
        }
        
        // ?
        x = (int)(x - (width/2.0) + 0.5);
        y = (int)(y - (height/2.0) + 0.5);
        
        if(m_orient != 0) {
            painter->rotate(m_orient);
            painter->worldTransform().inverted().map(x, y, &x, &y);
        }
        
        // note: drawChord and drawArc are expecting 1/16ths of degrees
        // first draw chord to get the background
        painter->setPen(QPen(Qt::NoPen));
        painter->drawChord(x, y, width, height,
                           16 * m_startAngle, 16 * m_spanAngle);
        
        // then draw the arc with the line
        painter->setPen(m_line.asQPen());
        painter->drawArc(x, y, width, height,
                         16 * m_startAngle, 16 * m_spanAngle);
        
        painter->restore();
    }
    logMethod("draw_", false);
}


/////////////////////////
// QPPOINT DEFINITIONS //
/////////////////////////

// Static //

const String QPPoint::CLASS_NAME = "QPPoint";


// Constructors/Destructors //

QPPoint::QPPoint(const PlotCoordinate& coordinate, const PlotSymbol& symbol) :
        m_symbol(symbol), m_coord(coordinate) {
    setItemAttribute(QwtPlotItem::AutoScale);
}

QPPoint::QPPoint(const PlotCoordinate& coordinate, const PlotSymbolPtr symbol):
        m_coord(coordinate) {
    if(!symbol.null()) m_symbol = *symbol;
    else               setSymbol(QPFactory::defaultPlotSymbol());
    setItemAttribute(QwtPlotItem::AutoScale);
}

QPPoint::QPPoint(const PlotCoordinate& coordinate, PlotSymbol::Symbol symbol) :
        m_coord(coordinate) {
    setSymbol(QPFactory::defaultPlotSymbol());
    m_symbol.setSymbol(symbol);
    
    setItemAttribute(QwtPlotItem::AutoScale);
}

QPPoint::QPPoint(const PlotPoint& copy) : m_symbol(copy.symbol()),
        m_coord(copy.coordinate()) {
    setItemAttribute(QwtPlotItem::AutoScale);
}

QPPoint::~QPPoint() {
    logDestruction();
}


QwtDoubleRect QPPoint::boundingRect() const {
    if(m_canvas != NULL) {
        PlotCoordinate c = m_coord;
        if(c.system() != PlotCoordinate::WORLD)
            c = m_canvas->convertCoordinate(c, PlotCoordinate::WORLD);
        
        QwtScaleMap xMap = m_canvas->asQwtPlot().canvasMap(QwtPlot::xBottom);
        QwtScaleMap yMap = m_canvas->asQwtPlot().canvasMap(QwtPlot::yLeft);
        
        int x = xMap.transform(c.x());
        int y = yMap.transform(c.y());
        
        psize_t size = m_symbol.size();
        double width = size.first, height = size.second;
        int x1 = x - (int)((width/2) + 0.5), x2 = x + (int)((width/2) + 0.5);
        int y1 = y - (int)((height/2) + 0.5), y2 = y + (int)((height/2) + 0.5);
        
        double dx = xMap.invTransform(x2) - xMap.invTransform(x1);
        double dy = yMap.invTransform(y2) - yMap.invTransform(y1);
        if(dx < 0) dx = -dx;
        if(dy < 0) dy = -dy;
        
        return QRectF(c.x(), c.y(), dx, dy);
    } else return QwtDoubleRect();
}

QWidget* QPPoint::legendItem() const {
    QwtLegendItem* item = new QwtLegendItem(m_symbol, QPen(),
                                            QwtPlotItem::title());
    item->setIdentifierMode(QwtLegendItem::ShowSymbol |
                            QwtLegendItem::ShowText);
    return item;
}


PlotCoordinate QPPoint::coordinate() const { return m_coord; }

void QPPoint::setCoordinate(const PlotCoordinate& coordinate) {
    if(m_coord != coordinate) {
        m_coord = coordinate;
        itemChanged();
    }
}

PlotSymbolPtr QPPoint::symbol() const { return new QPSymbol(m_symbol); }

void QPPoint::setSymbol(const PlotSymbol& s) {
    if(s != *symbol()) {
        m_symbol = s;        
        itemChanged();
    }
}

void QPPoint::draw_(QPainter* painter, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& canvasRect,
        unsigned int drawIndex, unsigned int drawCount) const {
    logMethod("draw_", true);
    painter->save();
    
    if(m_symbol.symbol() != PlotSymbol::PIXEL) {
        painter->setPen(m_symbol.pen());
        painter->setBrush(m_symbol.brush());
    } else painter->setPen(m_symbol.brush().color());

    QRect rect;
    rect.setSize(QwtPainter::metricsMap().screenToLayout(
                 dynamic_cast<const QwtSymbol*>(&m_symbol)->size()));

    PlotCoordinate c = m_coord;
    if(c.system() != PlotCoordinate::WORLD)
        c = m_canvas->convertCoordinate(c, PlotCoordinate::WORLD);

    int x = xMap.transform(c.x());
    int y = yMap.transform(c.y());
    rect.moveCenter(QPoint(x, y));
    m_symbol.draw(painter, rect);
    
    painter->restore();
    logMethod("draw_", false);
}

#endif

}
