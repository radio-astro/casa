//# QPShape.h: Qwt implementation of generic PlotShape classes.
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
#ifndef QPSHAPE_H_
#define QPSHAPE_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotShape.h>
#include <casaqt/QwtPlotter/QPOptions.h>
#include <casaqt/QwtPlotter/QPPlotItem.qo.h>

#include <qwt_plot_item.h>
#include <qwt_plot_marker.h>

#include <casa/namespace.h>

namespace casa {

// QPShape is the abstract superclass for any qwt-based shapes.  It handles the
// common functionality like changing the QPen and QBrush.
class QPShape : public QPPlotItem, public virtual PlotShape {
public:
    // Static //
    
    // Makes a copy of the given generic PlotShape into its Qwt-specific
    // subclass and returns it, or NULL for an error (shouldn't happen).
    static QPShape* clone(const PlotShape& copy);
    
    
    // Non-Static //
    
    // Constructor.
    QPShape();
    
    // Copy constructor which takes generic PlotShape.  Will be called by copy
    // constructors of children.
    QPShape(const PlotShape& copy);
    
    // Destructor.
    virtual ~QPShape();
    
    
    // Include overloaded methods.
    using PlotShape::setLine;
    using PlotShape::setAreaFill;
  
    
    // QPPlotItem Methods //
    
    // Overrides QwtPlotItem::legendItem().
    virtual QWidget* legendItem() const;
    
    
    // PlotShape Methods //
    
    // Implements PlotShape::lineShown().
    bool lineShown() const;
    
    // Implements PlotShape::setLineShown().
    void setLineShown(bool line = true);
    
    // Implements PlotShape::line().
    PlotLinePtr line() const;
    
    // Implements PlotShape::setLine().
    void setLine(const PlotLine& line);
    
    // Implements PlotShape::areaFilled().
    bool areaFilled() const;
    
    // Implements PlotShape::setAreaFilled().
    void setAreaFilled(bool area = true);
    
    // Implements PlotShape::areaFill().
    PlotAreaFillPtr areaFill() const;
    
    // Implements PlotShape::setAreaFill().
    void setAreaFill(const PlotAreaFill& fill);

protected:
    QPLine m_line;     // Line
    QPAreaFill m_area; // Area fill
    
    
    // Used for creating legend items.
    virtual QwtSymbol::Style legendStyle() const = 0;
};


// Implementation of PlotShapeRectangle for Qwt plotter.
class QPRectangle : public QPShape, public PlotShapeRectangle {
public:
    // Static //
    
    // Convenient access to class name (QPRectangle).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes the upper-left and lower-right coordinates.
    QPRectangle(const PlotCoordinate& upperLeft,
                const PlotCoordinate& lowerRight);
    
    // Constructor which takes a QwtDoubleRect in world coordinates.
    QPRectangle(const QwtDoubleRect& r);
    
    // Copy constructor for generic PlotShapeRectangle.
    QPRectangle(const PlotShapeRectangle& copy);
    
    // Destructor.
    ~QPRectangle();
    
    
    // QPShape Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
    
    // Overrides QwtPlotItem::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    
    // PlotShapeRectangle Methods //
    
    // Implements PlotShape::coordinates().
    vector<PlotCoordinate> coordinates() const;
    
    // Implements PlotShape::setCoordinates().
    void setCoordinates(const vector<PlotCoordinate>& coords);
    
    // Implements PlotShapeRectangle::setRectCoordinates().
    void setRectCoordinates(const PlotCoordinate& upperLeft,
                            const PlotCoordinate& lowerRight);
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPLayerItem::draw_().  Ignores draw index and count.
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
    // Implements QPShape::legendStyle().
    QwtSymbol::Style legendStyle() const { return QwtSymbol::Rect; }
    
private:
    PlotCoordinate m_upperLeft;  // upper left coordinate
    PlotCoordinate m_lowerRight; // lower right coordinate
};


// Implementation of PlotShapeEllipse for Qwt plotter.
class QPEllipse : public QPShape, public PlotShapeEllipse {
public:
    // Static //
    
    // Convenient access to class name (QPEllipse).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes the center and radii as PlotCoordinates.
    QPEllipse(const PlotCoordinate& center, const PlotCoordinate& radii);
    
    // Constructor which uses the given QwtDoubleRect in world coordinates.
    QPEllipse(const QwtDoubleRect& r);
    
    // Copy constructor for generic PlotShapeEllipse.
    QPEllipse(const PlotShapeEllipse& copy);
    
    // Destructor.
    ~QPEllipse();
    
    
    // QPShape Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
    
    // Overrides QwtPlotItem::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    
    // PlotShapeEllipse Methods //
    
    // Implements PlotShape::coordinates().
    vector<PlotCoordinate> coordinates() const;
    
    // Implements PlotShape::setCoordinates().
    void setCoordinates(const vector<PlotCoordinate>& coords);
    
    // Implements PlotShapeEllipse::setEllipseCoordinates().
    void setEllipseCoordinates(const PlotCoordinate& center,
                               const PlotCoordinate& radii);
    
    // Implements PlotShapeEllipse::radii().
    PlotCoordinate radii() const;
    
    // Implements PlotShapeEllipse::setRadii().
    void setRadii(const PlotCoordinate& radii);
    
    // Implements PlotShapeEllipse::center().
    PlotCoordinate center() const;
    
    // Implements PlotShapeEllipse::setCenter().
    void setCenter(const PlotCoordinate& center);
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPLayerItem::draw_().  Ignores draw index and count.
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
    // Implements QPShape::legendStyle().
    QwtSymbol::Style legendStyle() const { return QwtSymbol::Ellipse; }
    
private:
    PlotCoordinate m_center; // Center
    PlotCoordinate m_radii;  // Radii
};


// Implementation of PlotShapePolygon for Qwt plotter.
class QPPolygon : public QPShape, public PlotShapePolygon {
public:
    // Static //
    
    // Convenient access to class name (QPPolygon).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes list of coordinates.
    QPPolygon(const vector<PlotCoordinate>& coords);
    
    // Copy constructor for generic PlotShapePolygon.
    QPPolygon(const PlotShapePolygon& copy);
    
    // Destructor.
    ~QPPolygon();
    
    
    // QPShape Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
    
    // Overrides QwtPlotItem::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    
    // PlotShapePolygon Methods //
    
    // Implements PlotShape::coordinates().
    vector<PlotCoordinate> coordinates() const;
    
    // Implements PlotShape::setCoordinates().
    void setCoordinates(const vector<PlotCoordinate>& coords);
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPLayerItem::draw_().
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
    // Implements QPShape::legendStyle().
    QwtSymbol::Style legendStyle() const { return QwtSymbol::Hexagon; }
    
private:
    vector<PlotCoordinate> m_coords; // Coordinates
};


// Implementation of PlotShapeLine for Qwt plotter.
class QPLineShape : public QPShape, public PlotShapeLine {
public:
    // Static //
    
    // Convenient access to class name (QPLineShape).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes location in world coordinates and axis.
    QPLineShape(double location, PlotAxis axis);
    
    // Copy constructor for generic PlotShapeLine.
    QPLineShape(const PlotShapeLine& copy);
    
    // Destructor.
    ~QPLineShape();
    
    
    // QPShape Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
 
    // Overrides QwtPlotItem::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    
    // PlotShapeLine Methods //
    
    // Implements PlotShape::coordinates().
    vector<PlotCoordinate> coordinates() const;
    
    // Implements PlotShape::setCoordinates().
    void setCoordinates(const vector<PlotCoordinate>& coords);
    
    // Implements PlotShapeLine::setLineCoordinates().
    void setLineCoordinates(double location, PlotAxis axis);
    
    // Implements PlotShapeLine::location().
    double location() const;
    
    // Implements PlotShapeLine::axis().
    PlotAxis axis() const;
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPLayerItem::draw_().  Ignores draw index and count.
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
    // Implements QPShape::legendStyle().
    QwtSymbol::Style legendStyle() const { return QwtSymbol::HLine; }
    
private:
    QwtPlotMarker m_marker; // Marker
    PlotAxis m_axis;        // Axis
};


// Implementation of PlotShapeArrow for Qwt plotter.
class QPArrow : public QPShape, public PlotShapeArrow {
public:
    // Static //
    
    // Convenient access to class name (QPArrow).
    static const String CLASS_NAME;
    
    // Returns the two points necessary to make an arrow shape from the given
    // point to the given point, with the given length.  The two arrow points
    // are at a 45 degree angle from the "from" point and the line between the
    // point and the "from" point is equal to "length".
    // <group>
    static pair<QPointF, QPointF> arrowPoints(QPointF from, QPointF to,
                                              double length);    
    static void arrowPoints(double x1, double y1, double x2, double y2,
                            double length, double& resX1, double& resY1,
                            double& resX2, double& resY2);
    // </group>
    
    
    // Non-Static //
    
    // Constructor which takes the two coordinates.
    QPArrow(const PlotCoordinate& from, const PlotCoordinate& to);
    
    // Copy constructor for generic PlotshapeArrow.
    QPArrow(const PlotShapeArrow& copy);
    
    // Destructor.
    ~QPArrow();
    
    
    // QPShape Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
    
    // Overrides QwtPlotItem::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    
    // PlotShapeArrow Methods //
    
    // Implements PlotShape::coordinates().
    vector<PlotCoordinate> coordinates() const;
    
    // Implements PlotShape::setCoordinates().
    void setCoordinates(const vector<PlotCoordinate>& coords);    
    
    // Implements PlotShapeArrow::setArrowCoordinates().
    void setArrowCoordinates(const PlotCoordinate& from,
                             const PlotCoordinate& to);
    
    // Implements PlotShapeArrow::arrowStyleFrom().
    Style arrowStyleFrom() const;
    
    // Implements PlotShapeArrow::arrowStyleTo().
    Style arrowStyleTo() const;
    
    // Implements PlotShapeArrow::setArrowStyleFrom().
    void setArrowStyleFrom(Style style) { setArrowStyles(style, m_toStyle); }
    
    // Implements PlotShapeArrow::setArrowStyleTo().
    void setArrowStyleTo(Style style) { setArrowStyles(m_fromStyle, style); }
    
    // Overrides PlotShapeArrow::setArrowStyles().
    void setArrowStyles(Style from, Style to);
    
    // Implements PlotShapeArrow::arrowSize().
    double arrowSize() const;
    
    // Implements PlotShapeArrow::setArrowSize().
    void setArrowSize(double size);
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPLayerItem::draw_().  Ignores draw index and count.
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
    // Implements QPShape::legendStyle().
    QwtSymbol::Style legendStyle() const { return QwtSymbol::HLine; }
    
private:
    PlotCoordinate m_from;        // From point
    PlotCoordinate m_to;          // To point
    Style m_fromStyle, m_toStyle; // Arrow styles
    double m_size;                // Arrow size
    
    
    // Static //
    
    // Helper for arrowPoints.  Calculates the points under the assumption that
    // the arrow is pointing in an assumed direction, and then lets arrowPoints
    // rotate/transform the results as needed.
    static pair<QPointF, QPointF> arrowPointsHelper(QPointF p1, QPointF p2,
                                                    double length);
};


// Implementation of PlotShapePath for Qwt plotter.
class QPPath : public QPShape, public PlotShapePath {
public:
    // Static //
    
    // Convenient access to class name (QPPath).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes the points.
    QPPath(const vector<PlotCoordinate>& points);
    
    // Copy constructor for generic PlotShapePath.
    QPPath(const PlotShapePath& copy);
    
    // Destructor.
    ~QPPath();
    
    
    // QPShape Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
    
    // Overrides QwtPlotItem::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    
    // PlotShapePath Methods //
    
    // Implements PlotShape::coordinates().
    vector<PlotCoordinate> coordinates() const;
    
    // Implements PlotShape::setCoordinates().
    void setCoordinates(const vector<PlotCoordinate>& coords);
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPLayerItem::draw_().
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
    // Implements QPShape::legendStyle().
    QwtSymbol::Style legendStyle() const { return QwtSymbol::HLine; }
    
private:
    vector<PlotCoordinate> m_coords; // Coordinates
};


// Implementation of PlotShapeArc for Qwt plotter.
class QPArc : public QPShape, public PlotShapeArc {
public:
    // Static //
    
    // Convenient access to class name (QPArc).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes the start coordinate, width, height, start
    // angle, and span angle.
    QPArc(const PlotCoordinate& start, const PlotCoordinate& widthHeight,
          int startAngle, int spanAngle);
    
    // Copy constructor for generic PlotShapeArc.
    QPArc(const PlotShapeArc& copy);
    
    // Destructor.
    ~QPArc();
    
    
    // Include overloaded methods.
    using PlotShapeArc::setWidthHeight;
    
    
    // QPShape Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
    
    // Overrides QwtPlotItem::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    
    // PlotShapeArc Methods //
    
    // Implements PlotShape::coordinates().
    vector<PlotCoordinate> coordinates() const;
    
    // Implements PlotShape::coordinates().
    void setCoordinates(const vector<PlotCoordinate>& coords);
    
    // Implements PlotShapeArc::startCoordinate().
    PlotCoordinate startCoordinate() const;
    
    // Implements PlotShapeArc::setStartCoordinate().
    void setStartCoordinate(const PlotCoordinate& coord);
    
    // Implements PlotShapeArc::widthHeight().
    PlotCoordinate widthHeight() const;
    
    // Implements PlotShapeArc::setWidthHeight().
    void setWidthHeight(const PlotCoordinate& widthHeight);
    
    // Implements PlotShapeArc::startAngle().
    int startAngle() const;
    
    // Implements PlotShapeArc::setStartAngle().
    void setStartAngle(int startAngle);
    
    // Implements PlotShapeArc::spanAngle().
    int spanAngle() const;
    
    // Implements PlotShapeArc::setSpanAngle().
    void setSpanAngle(int spanAngle);
    
    // Implements PlotShapeArc::orientation().
    int orientation() const;
    
    // Implements PlotShapeArc::setOrientation().
    void setOrientation(int o);
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPLayerItem::draw_().  Ignores draw index and count.
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
    // Implements QPShape::legendStyle().
    QwtSymbol::Style legendStyle() const { return QwtSymbol::HLine; }
    
private:
    PlotCoordinate m_start; // Start coordinate
    PlotCoordinate m_size;  // Width and height
    int m_startAngle;       // Start angle
    int m_spanAngle;        // Span angle
    int m_orient;           // Orientation
};


// Implementation of PlotPoint for Qwt plotter.
class QPPoint : public QPPlotItem, public PlotPoint {
public:
    // Static //
    
    // Convenient access to class name (QPPoint).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructors which take the location and symbol.
    // <group>
    QPPoint(const PlotCoordinate& coordinate, const PlotSymbol& symbol);
    QPPoint(const PlotCoordinate& coordinate, const PlotSymbolPtr symbol);    
    QPPoint(const PlotCoordinate& coordinate,
            PlotSymbol::Symbol symbol = PlotSymbol::DIAMOND);
    // </group>
    
    // Copy constructor for generic PlotPoint.
    QPPoint(const PlotPoint& copy);
    
    // Destructor.
    ~QPPoint();
    
    
    // Include overloaded methods.
    using PlotPoint::setSymbol;
    
    
    // QPPlotItem Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const { return true; }

    // Overrides QwtPlotItem::boundingRect();
    QwtDoubleRect boundingRect() const;
    
    // Overrides QwtPlotItem::legendItem().
    QWidget* legendItem() const;
    
    
    // PlotPoint Methods //
    
    // Implements PlotPoint::coordinate().
    PlotCoordinate coordinate() const;
    
    // Implements PlotPoint::setCoordinate().
    void setCoordinate(const PlotCoordinate& coordinate);
    
    // Implements PlotPoint::symbol().
    PlotSymbolPtr symbol() const;
    
    // Implements PlotPoint::setSymbol().
    void setSymbol(const PlotSymbol& symbol);
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPLayerItem::draw_().  Ignores draw index and count.
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
private:
    QPSymbol m_symbol;      // symbol
    PlotCoordinate m_coord; // location
};

}

#else

#include <QPointF>
#include <casa/cppconfig.h>

using namespace std;

namespace casa {

// Used by the viewer's region shapes, so have it outside the ifdefs in case
// the flag isn't on.
class QPArrow {
public:
    static pair<QPointF, QPointF> arrowPoints(QPointF from, QPointF to,
                                              double length);
    
    static void arrowPoints(double x1, double y1, double x2, double y2,
                            double length, double& resX1, double& resY1,
                            double& resX2, double& resY2);
    
private:
    static pair<QPointF, QPointF> arrowPointsHelper(QPointF p1, QPointF p2,
                                                    double length);
};

}

#endif

#endif /*QPSHAPE_H_*/
