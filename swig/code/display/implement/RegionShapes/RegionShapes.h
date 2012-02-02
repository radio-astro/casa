//# RegionShapes.h: Classes for displaying region shapes on the viewer.
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
//# $Id$
#ifndef REGIONSHAPES_H_
#define REGIONSHAPES_H_

#include <display/RegionShapes/RegionShape.h>

#include <cctype>

#include <casa/namespace.h>

namespace casa {

// Subclass of RegionShape used for drawing ellipses.  An ellipse has the
// following properties in addition to those listed for RegionShape:
// <ul><li>x and y coordinate</li>
//     <li>x-radius and y-radius</li>
//     <li>angle of rotation</li></ul>
class RSEllipse : public RegionShape {
public:
    // World Constructor.  x, y, xRadius, yRadius, and angle must all be
    // 1) using the same world system as defined by worldSystem, and 2) in
    // unit RegionShape::UNIT.  Angle is counterclockwise, and angle 0 is the
    // east vector.
    RSEllipse(double x, double y, double xRadius, double yRadius,
              MDirection::Types worldSystem, double angle = 0);

    // Pixel Constructor.  Angle in counterclockwise degrees.
    RSEllipse(double x, double y, double xRadius, double yRadius,
              double angle = 0);

    // Record Constructor.
    RSEllipse(const RecordInterface& properties);
    
    // Destructor.
    virtual ~RSEllipse();

    // Implements RegionShape::drawAndUpdateBoundingBox.
    virtual bool drawAndUpdateBoundingBox(WorldCanvasHolder& wch,
                                          String* error = NULL);

    // Implements RegionShape::type.
    virtual String type() const { return "ellipse"; }

    // Gets ellipse-specific coordinates, to avoid dependence on
    // coordParameterValues() when the shape type is known.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getEllipseCoordinates(double& x, double& y, double& xRadius,
                               double& yRadius, double& angle,
                               String toSystem = "") const;
    
    // Implements RegionShape::coordParameterValues.
    virtual vector<double> coordParameterValues() const;
    
    // Implements RegionShape::coordParameterValues.
    virtual vector<double> coordParameterValues(String toSystem) const;
    
    // Implements RegionShape::coordParameterNames.
    virtual vector<String> coordParameterNames() const;
    
    // Implements RegionShape::coordParameterPositions.
    virtual vector<CoordinateParameterType> coordParameterTypes() const;
    
    // Implements RegionShape::setCoordParameters.
    virtual void setCoordParameters(const vector<double>& vals);
    
    // Implements RegionShape::setCoordParameters.
    virtual void setCoordParameters(const vector<double>& vals, String system);
    
    // Implements RegionShape::move.
    virtual void move(double dx, double dy, String system = "");
    
    // Implements RegionShape::getHandle.
    virtual RSHandle getHandle() const;
    
    // Implements RegionShape option methods.  Note: ellipses do not have
    // additonal options.
    // <group>
    virtual vector<String> optionNames() const { return vector<String>(); }
    virtual vector<OptionType> optionTypes() const {
        return vector<OptionType>(); }
    virtual vector<RSOption> optionValues() const { return vector<RSOption>();}
    virtual void setOptionValues(const vector<RSOption>& options) { }
    // </group>

protected:
    vector<double> m_spec;        // order: [x, y, x-radius, y-radius, angle]
                                  // either world or pixel coords
    vector<double> m_screenSpec;  // same order, screen coords

    
    // Implements RegionShape::getShapeSpecificProperties.
    virtual void getShapeSpecificProperties(Record& properties) const {
        properties.define(PROPTYPE, PROPTYPE_ELLIPSE);
    }
    
    // Implements RegionShape::setShapeSpecificProperties.
    virtual void setShapeSpecificProperties(const RecordInterface& props) { }
    
    // Updates screen coords using world/pixel coords and the given WorldCanvas
    virtual bool updateScreenCoordinates(WorldCanvasHolder& wch, String* err);
    
    // Update m_xMin, m_xMax, m_yMin, and m_yMax.
    virtual void updateMinMax();
};


// Subclass of RSEllipse used for drawing circles.  Only one radius (x) is
// taken and no angle.
class RSCircle : public RSEllipse {
public:
    // World constructor.  x, y, and xRadius must be of the type defined by
    // worldSys and in units of RegionShape::UNIT.
    RSCircle(double x, double y, double xRadius, MDirection::Types worldSys);
    
    // Pixel constructor.
    RSCircle(double x, double y, double xRadius);

    // Record Constructor.
    RSCircle(const RecordInterface& properties);
    
    // Destructor.
    ~RSCircle();
    
    // Overrides RSEllipse::type.
    String type() const { return "circle"; }
    
    // Gets circle-specific coordinates, to avoid dependence on
    // coordParameterValues() when the shape type is known.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getCircleCoordinates(double& x, double& y, double& xRadius,
                              String toSystem = "") const;
    
    // Overrides RSEllipse::coordParameterValues.
    vector<double> coordParameterValues() const;
    
    // Overrides RSEllipse::coordParameterValues.
    vector<double> coordParameterValues(String toSystem) const;
    
    // Overrides RSEllipse::coordParameterNames.
    vector<String> coordParameterNames() const;
    
    // Overrides RSEllipse::coordParameterPositions.
    vector<CoordinateParameterType> coordParameterTypes() const;
    
    // Overrides RSEllipse::setCoordParameters.
    void setCoordParameters(const vector<double>& vals);
    
    // Overrides RSEllipse::setCoordParameters.
    void setCoordParameters(const vector<double>& vals, String valSystem);
    
protected:
    // Overrides RSEllipse::getShapeSpecificProperties.
    void getShapeSpecificProperties(Record& properties) const {
        properties.define(PROPTYPE, PROPTYPE_CIRCLE);
    }
    
    // Overrides RSEllipse::updateScreenCoordinates.
    bool updateScreenCoordinates(WorldCanvasHolder& wch, String* err);
};


// Subclass of RegionShape used for drawing rectangles.  A rectangle has the
// following properties in addition to those listed for RegionShape:
// <ul><li>x and y coordinate of center</li>
//     <li>width and height</li></ul>
class RSRectangle : public RegionShape {
public:
    // Static Members //
    
    // Gets the four rectangle points for the given center, radii, and angle
    // and puts them in the given vectors.  Returns whether or not the
    // operation succeeded.  Angle is in counterclockwise degrees.  The point
    // order is [top right, top left, bottom left, bottom right] in the
    // coordinate system of the given points.
    static bool getRectanglePoints(double centerX, double centerY,
                                   double radiusX, double radiusY,
                                   double angle, Vector<double>& xPoints,
                                   Vector<double>& yPoints);
    
    
    // Non-Static Members //
    
    // World Constructor.  x, y, width, and height must all be 1) using the
    // same world system as defined by worldSystem, and 2) in unit
    // RegionShape::UNIT.  (x, y) specifies the center of the rectangle.
    // Angle is in counterclockwise degrees starting at the east vector.
    RSRectangle(double x, double y, double width, double height,
                MDirection::Types worldSystem, double angle = 0);
    
    // Pixel Constructor.  (x, y) specifies the center of the rectangle.
    RSRectangle(double x, double y, double width, double height,
                double angle = 0);

    // Record Constructor.
    RSRectangle(const RecordInterface& properties);

    // Destructor.
    ~RSRectangle();

    // Implements RegionShape::drawAndUpdateBoundingBox.
    bool drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err = NULL);

    // Implements RegionShape::type.
    String type() const { return "rectangle"; }
    
    // Gets rectangle-specific coordinates, to avoid dependence on
    // coordParameterValues() when the shape type is known.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getRectangleCoordinates(double& x, double& y, double& width,
                                 double& height, double& angle,
                                 String toSystem = "") const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues() const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues(String toSystem) const;
    
    // Implements RegionShape::coordParameterNames.
    vector<String> coordParameterNames() const;
    
    // Implements RegionShape::coordParameterPositions.
    vector<CoordinateParameterType> coordParameterTypes() const;
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals);
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals, String valSystem);
    
    // Implements RegionShape::move.
    void move(double dx, double dy, String system = "");
    
    // Implements RegionShape::getHandle.
    RSHandle getHandle() const;
    
    // Implements RegionShape option methods.  Note: rectangles do not have
    // additonal options.
    // <group>
    vector<String> optionNames() const { return vector<String>(); }
    vector<OptionType> optionTypes() const { return vector<OptionType>(); }
    vector<RSOption> optionValues() const { return vector<RSOption>();}
    void setOptionValues(const vector<RSOption>& options) { }
    // </group>

protected:
    vector<double> m_spec;               // order: [x, y, width, height, angle]
                                         // either world or pixel coords
    Vector<double> m_x, m_y;             // pixel/world coords for four points
                                         // in order: [upL, boL, boR, upR]
    Vector<double> m_screenX, m_screenY; // screen coords for four points
                                         // same order

    // Implements RegionShape::getShapeSpecificProperties.
    void getShapeSpecificProperties(Record& properties) const {
        properties.define(PROPTYPE, PROPTYPE_RECTANGLE);
    }
    
    // Implements RegionShape::setShapeSpecificProperties.
    void setShapeSpecificProperties(const RecordInterface& properties) { }
    
    // Updates screen coords using world/pixel coords and the given WorldCanvas
    bool updateScreenCoordinates(WorldCanvasHolder& wch, String* err);
    
    // Translates m_spec into m_x and m_y.
    void updatePoints();
    
    // Update m_xMin, m_xMax, m_yMin, and m_yMax.
    void updateMinMax();
};


// Subclass of RegionShape used for drawing polygons.  A polygon has the
// following properties in addition to those listed for RegionShape:
// <ul><li>list of x and y coordinates</li>
//     <li>optional custom name</li></ul>
// The following properties are also available:
// <ul><li><b>RSPolygon::PROPXCOORDINATES</b> (<i>Array&lt;double&gt;</i>):
//         vertices x values.  <b>NOT STRICTLY REQUIRED, BUT A VALID POLYGON
//         HAS AT LEAST TWO VERTICES.</b></li>
//     <li><b>RSPolygon::PROPYCOORDINATES</b> (<i>Array&lt;double&gt;</li>):
//         vertices y values.  <b>NOT STRICTLY REQUIRED, BUT A VALID POLYGON
//         HAS AT LEAST TWO VERTICES.</b></li></ul>
class RSPolygon : public RegionShape {
public:
    // Static Members //
    
    // Properties.  (See class descriptions of RegionShape and RSPolygon.)
    // <group>
    static const String PROPXCOORDINATES;
    static const String PROPYCOORDINATES;
    // </group>
    
    
    // Non-Static Members //
    
    // World Constructor.  x and y must both be 1) using the same world system
    // as defined by worldSystem, and 2) in unit RegionShape::UNIT.  A custom
    // name can be provided to be returned with RSPolygon::type().
    RSPolygon(const Vector<double>& x, const Vector<double>& y,
              MDirection::Types worldSystem, String custName = "");

    // Pixel Constructor.
    RSPolygon(const Vector<double>& x, const Vector<double>& y,
              String custName = "");

    // Record Constructor.
    RSPolygon(const RecordInterface& properties);

    // Destructor.
    ~RSPolygon();

    // Sets the polygon coordinates to the given.
    void setPolygonCoordinates(const Vector<double>& x,
                               const Vector<double>& y);
    
    // Implements RegionShape::drawAndUpdateBoundingBox.
    bool drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err = NULL);

    // Implements RegionShape::type.
    String type() const {
        if(!m_custName.empty()) return m_custName;
        else                    return "polygon";
    }
    
    // Overrides RegionShape::oneWordType.
    String oneWordType() const { return "polygon"; }
    
    // Gets polygon-specific coordinates, to avoid dependence on
    // coordParameterValues() when the shape type is known.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getPolygonCoordinates(double& centerX, double& centerY,
                               String toSystem = "") const;
    
    // Gets actual polygon coordinates.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getPolygonCoordinates(vector<double>& x, vector<double>& y,
                               String toSystem = "") const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues() const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues(String toSystem) const;
    
    // Implements RegionShape::coordParameterNames.
    vector<String> coordParameterNames() const;
    
    // Implements RegionShape::coordParameterPositions.
    vector<CoordinateParameterType> coordParameterTypes() const;
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals);
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals, String valSystem);
    
    // Implements RegionShape::move.
    void move(double dx, double dy, String system = "");
    
    // Implements RegionShape::getHandle.
    RSHandle getHandle() const;
    
    // Implements RegionShape::optionNames.
    vector<String> optionNames() const;
    
    // Implements RegionShape::optionTypes.
    vector<OptionType> optionTypes() const;
    
    // Implements RegionShape::optionValues.
    vector<RSOption> optionValues() const;
    
    // Implements RegionShape::setOptions.
    void setOptionValues(const vector<RSOption>& options);

protected:
    Vector<Double> m_x, m_y;             // coordinates, world or pixel
    double m_centerX, m_centerY;         // center of polygon, world or pixel
    Vector<Double> m_screenX, m_screenY; // coordinates, screen
    unsigned int m_n;                    // number of points
    String m_custName;                   // optional custom name

    // Implements RegionShape::getShapeSpecificProperties.
    void getShapeSpecificProperties(Record& properties) const;
    
    // Implements RegionShape::setShapeSpecificProperties.
    void setShapeSpecificProperties(const RecordInterface& properties);
    
    // Updates screen coords using world/pixel coords and the given WorldCanvas
    bool updateScreenCoordinates(WorldCanvasHolder& wch, String* err);
    
    // Update m_xMin, m_xMax, m_yMin, and m_yMax.
    void updateMinMax();
};


// Subclass of RegionShape used for drawing lines.  A line has the
// following properties in addition to those listed for RegionShape:
// <ul><li>x1, y1 and x2, y2 coordinates</li>
//     <li>whether to draw an arrow on either of the endpoints</li>
//     <li>arrow length</li></ul>
class RSLine : public RegionShape {
public:
    // Static Members //
    
    // Different ways to draw the arrow(s) at the end of the line.
    enum ArrowType {
        FilledTriangle, Triangle, V, FilledDoubleV
    };
    
    // Returns all possible arrow types.
    // <group>
    static vector<ArrowType> allArrowTypes() {
        static vector<ArrowType> v(4);
        v[0] = FilledTriangle; v[1] = Triangle;
        v[2] = V;              v[3] = FilledDoubleV;
        return v;
    }

    static vector<String> allArrowTypesStrings() {
        vector<ArrowType> types = allArrowTypes();
        vector<String> v(types.size());
        for(unsigned int i = 0; i < v.size(); i++) v[i] = arrowType(types[i]);
        return v;
    }
    // </group>
    
    // Converts to/from the String representation of arrow types.
    // <group>
    static String arrowType(ArrowType type) {
        switch(type) {
        case FilledTriangle: return "filled triangle";
        case Triangle:       return "triangle";
        case V:              return "v";
        case FilledDoubleV:  return "filled double v";
        
        default: return "";
        }
    }
    
    static ArrowType arrowType(const String& type) {
        String t = type;
        for(unsigned int i = 0; i < t.size(); i++) t[i] = tolower(t[i]);
        if(t == "filled triangle")      return FilledTriangle;
        else if(t == "triangle")        return Triangle;
        else if(t == "v")               return V;
        else if(t == "filled double v") return FilledDoubleV;
        
        else                            return FilledTriangle;
    }
    // </group>
    
    
    // Non-Static Members //
    
    // World Constructor.  x1, y1, x2, and y2 must all be 1) using the
    // same world system as defined by worldSystem, and 2) in unit
    // RegionShape::UNIT.  arrowLength is in screen pixels.  arrow1 defines
    // whether (x1, y1) has an arrow; arrow2 for (x2, y2).
    RSLine(double x1, double y1, double x2, double y2,
           MDirection::Types worldSystem, int arrowLength, bool arrow1 = false,
           bool arrow2 = false, ArrowType type1 = FilledDoubleV,
           ArrowType type2 = FilledDoubleV);
    
    // Pixel Constructor.  arrowLength is in screen pixels.  arrow1 defines
    // whether (x1, y1) has an arrow; arrow2 for (x2, y2).
    RSLine(double x1, double y1, double x2, double y2, int arrowLength,
           bool arrow1 = false, bool arrow2 = false,
           ArrowType type1 = FilledDoubleV, ArrowType type2 = FilledDoubleV);

    // Record Constructor.
    RSLine(const RecordInterface& properties);
    
    // Destructor.
    virtual ~RSLine();
    
    // Returns whether the two endpoints have arrows or not.
    // </group>
    bool p1Arrow() const { return m_arrow1; }
    bool p2Arrow() const { return m_arrow2; }
    // </group>
    
    // Implements RegionShape::drawAndUpdateBoundingBox.
    virtual bool drawAndUpdateBoundingBox(WorldCanvasHolder& wch,
                                          String* error = NULL);
    
    // Implements RegionShape::type.
    virtual String type() const { return "line"; }
    
    // Gets line-specific coordinates, to avoid dependence on
    // coordParameterValues() when the shape type is known.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getLineCoordinates(double& x1, double& y1, double& x2, double& y2,
                            int& arrowLength, String toSystem = "") const;
    
    // Implements RegionShape::coordParameterVales.
    virtual vector<double> coordParameterValues() const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues(String toSystem) const;
    
    // Implements RegionShape::coordParameterNames.
    virtual vector<String> coordParameterNames() const;
    
    // Implements RegionShape::coordParameterPositions.
    virtual vector<CoordinateParameterType> coordParameterTypes() const;
    
    // Implements RegionShape::setCoordParameters.
    virtual void setCoordParameters(const vector<double>& vals);
    
    // Implements RegionShape::setCoordParameters.
    virtual void setCoordParameters(const vector<double>& vals, String System);
    
    // Implements RegionShape::move.
    virtual void move(double dx, double dy, String system = "");
    
    // Implements RegionShape::getHandle.
    virtual RSHandle getHandle() const;
    
    // Implements RegionShape::optionNames.
    virtual vector<String> optionNames() const;
    
    // Implements RegionShape::optionTypes.
    virtual vector<OptionType> optionTypes() const;
    
    // Implements RegionShape::optionChoices.
    virtual vector<vector<String> > optionChoices() const;
    
    // Implements RegionShape::optionValues.
    virtual vector<RSOption> optionValues() const;
    
    // Implements RegionShape::setOptions.
    virtual void setOptionValues(const vector<RSOption>& options);
    
protected:
    vector<double> m_spec;        // order: [x1 y1 x2 y2], either world or pixel
    vector<double> m_screenSpec;  // same order, screen coords
    int m_arrowLength;            // arrow length, screen coords
    bool m_arrow1, m_arrow2;      // whether to draw the arrows
    ArrowType m_aType1, m_aType2; // arrow types
    
    // Implements RegionShape::getShapeSpecificProperties.
    virtual void getShapeSpecificProperties(Record& properties) const {
        properties.define(PROPTYPE, PROPTYPE_LINE);
    }
    
    // Implements RegionShape::setShapeSpecificProperties.
    virtual void setShapeSpecificProperties(const RecordInterface& props) { }
    
    // Updates screen coords using world/pixel coords and the given WorldCanvas
    virtual bool updateScreenCoordinates(WorldCanvasHolder& wch, String* err);
    
    // Update m_xMin, m_xMax, m_yMin, and m_yMax.
    void updateMinMax();
    
    // Generates arrow points based on the given (x1, y1) (x2, y2) points.
    // NOTE: Duplicated code from casaqt/QwtPlotter/QPShape (QPArrow class).
    static bool arrowPoints(double x1, double y1, double x2, double y2,
                            double length, double& resX1, double& resY1,
                            double& resX2, double& resY2);
    
    // Helper for arrowPoints.  NOTE: Duplicated code.
    static bool arrowPointsHelper(double x1, double y1, double x2, double y2,
                                  double length, double& resX1, double& resY1,
                                  double& resX2, double& resY2);
};


// Subclass of RSLine that takes a length and angle instead of a second point.
class RSVector : public RSLine {
public:
    // World Constructor.  x, y, length, and angle must all be 1) using the
    // same world system as defined by worldSystem, and 2) in unit
    // RegionShape::UNIT.  Angle is counterclockwise, and the base is the east
    // vector.   arrowLength is in screen pixels. arrow defines whether the
    // second point has an arrow.
    RSVector(double x, double y, double length, double angle,
             MDirection::Types worldSystem, int arrowLength,
             bool arrow = true, ArrowType arrowType = FilledDoubleV);
    
    // Pixel Constructor.  Angle is counterclockwise.  arrowLength is in screen
    // pixels.  arrow defines whether the second point has an arrow.
    RSVector(double x, double y, double length, double angle, int arrowLength,
             bool arrow = true, ArrowType arrowType = FilledDoubleV);

    // Record Constructor.
    RSVector(const RecordInterface& properties);
    
    // Destructor.
    ~RSVector();
    
    // Returns whether the vector has an arrow at the end or not.
    bool arrow() const { return p2Arrow(); }
    
    // Overrides RSLine::type.
    String type() const { return "vector"; }
    
    // Gets vector-specific coordinates, to avoid dependence on
    // coordParameterValues() when the shape type is known.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getVectorCoordinates(double& x, double& y, double& angle,
                              double& length, int& arrowLength,
                              String toSystem = "") const;
    
    // Overrides RSLine::coordParameterValues.
    vector<double> coordParameterValues() const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues(String toSystem) const;
    
    // Overrides RSLine::coordParameterNames.
    vector<String> coordParameterNames() const;
    
    // Overrides RSLine::coordParameterPositions.
    vector<CoordinateParameterType> coordParameterTypes() const;
    
    // Overrides RSLine::setCoordParameters.
    void setCoordParameters(const vector<double>& vals);
    
    // Overrides RSLine::setCoordParameters.
    void setCoordParameters(const vector<double>& vals, String valSystem);
    
    // Overrides RSLine::optionNames.
    vector<String> optionNames() const;
    
    // Overrides RSLine::optionTypes.
    vector<OptionType> optionTypes() const;
    
    // Overrides RSLine::optionChoices.
    vector<vector<String> > optionChoices() const;
    
    // Overrides RSLine::optionValues.
    vector<RSOption> optionValues() const;
    
    // Overrides RSLine::setOptions.
    void setOptionValues(const vector<RSOption>& options);
    
protected:
    double m_length; // length, world or pixel
    double m_angle;  // angle, world or pixel
    
    // Overrides RSLine::getShapeSpecificProperties.
    void getShapeSpecificProperties(Record& properties) const {
        properties.define(PROPTYPE, PROPTYPE_VECTOR);
    }
};


// Subclass of RegionShape used for drawing markers.  A marker has the
// following properties in addition to those listed for RegionShape:
// <ul><li>x and y coordinates</li>
//     <li>marker type, or list of marker types</li>
//     <li>pixel height</li></ul>
class RSMarker : public RegionShape {
public:
    // World Constructor for a single marker type.  x and y must all be
    // 1) using the same world system as defined by worldSystem, and
    // 2) in the unit RegionShape::UNIT.  pixelHeight is in screen pixels.
    RSMarker(double x, double y, Display::Marker marker, int pixelHeight,
             MDirection::Types worldSystem);

    // World Constructor for a compound marker type.  x and y must all be
    // 1) using the same world system as defined by worldSystem, and
    // 2) in the unit RegionShape::UNIT.  pixelHeight is in screen pixels.
    RSMarker(double x, double y, const vector<Display::Marker>& markers,
             int pixelHeight, MDirection::Types worldSystem);
    
    // Pixel Constructor for a single marker type.  pixelHeight is in screen
    // pixels.
    RSMarker(double x, double y, Display::Marker marker, int pixelHeight);
    
    // Pixel Constructor for a compound marker type.  pixelHeight is in screen
    // pixels.
    RSMarker(double x, double y, const vector<Display::Marker>& markers,
             int pixelHeight);

    // Record Constructor.
    RSMarker(const RecordInterface& properties);

    // Destructor.
    ~RSMarker();
    
    // Gets/sets the markers used for this shape.
    // <group>
    vector<Display::Marker> markers() const;
    void setMarker(Display::Marker marker);
    void setMarkers(const vector<Display::Marker>& markers);
    // </group>
    

    // Implements RegionShape::drawAndUpdateBoundingBox.
    bool drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err = NULL);

    // Implements RegionShape::type.
    String type() const;
    
    // Overrides RegionShape::oneWordType.
    String oneWordType() const { return "marker"; }
    
    // Gets marker-specific coordinates, to avoid dependence on
    // coordParameterValues() when the shape type is known.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getMarkerCoordinates(double& x, double& y, int& pixelHeight,
                              String toSystem = "") const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues() const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues(String toSystem) const;
    
    // Implements RegionShape::coordParameterNames.
    vector<String> coordParameterNames() const;
    
    // Implements RegionShape::coordParameterPositions.
    vector<CoordinateParameterType> coordParameterTypes() const;
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals);
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals, String valSystem);
    
    // Implements RegionShape::move.
    void move(double dx, double dy, String system = "");
    
    // Implements RegionShape::getHandle.
    RSHandle getHandle() const;
    
    // Implements RegionShape::optionNames.
    vector<String> optionNames() const;
    
    // Implements RegionShape::optionTypes.
    vector<OptionType> optionTypes() const;
    
    // Overrides RegionShape::optionChoices.
    vector<vector<String> > optionChoices() const;
    
    // Implements RegionShape::optionValues.
    vector<RSOption> optionValues() const;
    
    // Implements RegionShape::setOptions.
    void setOptionValues(const vector<RSOption>& options);

protected:
    double m_x, m_y;                   // location, world or pixel
    double m_screenX, m_screenY;       // location, screen
    vector<Display::Marker> m_markers; // marker(s) to be drawn
    int m_pixelHeight;                 // pixel height for marker(s)

    // Implements RegionShape::getShapeSpecificProperties.
    void getShapeSpecificProperties(Record& properties) const {
        properties.define(PROPTYPE, PROPTYPE_MARKER);
    }
    
    // Implements RegionShape::setShapeSpecificProperties.
    void setShapeSpecificProperties(const RecordInterface& properties) { }
    
    // Updates screen coords using world/pixel coords and the given WorldCanvas
    bool updateScreenCoordinates(WorldCanvasHolder& wch, String* err);
    
    // Update m_xMin, m_xMax, m_yMin, and m_yMax.
    void updateMinMax();
    
    
    static String markerToString(Display::Marker marker) {
        switch(marker) {
        case Display::Cross:                 return "cross";
        case Display::X:                     return "x";
        case Display::Diamond:               return "diamond";
        case Display::Circle:                return "circle";
        case Display::Square:                return "square";
        case Display::Triangle:              return "triangle";
        case Display::InvertedTriangle:      return "inverted triangle";
        case Display::FilledCircle:          return "filled circle";
        case Display::FilledSquare:          return "filled square";
        case Display::FilledDiamond:         return "filled diamond";
        case Display::FilledTriangle:        return "filled triangle";
        case Display::FilledInvertedTriangle:return "filled inverted triangle";
        case Display::CircleAndCross:        return "circle and cross";
        case Display::CircleAndX:            return "circle and x";
        case Display::CircleAndDot:          return "circle and dot";
        
        default: return "";
        }
    }
    
    static bool stringToMarker(const String& str, Display::Marker& marker) {
        String m = str;
        for(unsigned int i = 0; i < m.size(); i++) m[i] = tolower(str[i]);
        
        if(m == "cross")                  marker = Display::Cross;
        else if(m == "x")                 marker = Display::X;
        else if(m == "diamond")           marker = Display::Diamond;
        else if(m == "circle")            marker = Display::Circle;
        else if(m == "square")            marker = Display::Square;
        else if(m == "triangle")          marker = Display::Triangle;
        else if(m == "inverted triangle") marker = Display::InvertedTriangle;
        else if(m == "filled circle")     marker = Display::FilledCircle;
        else if(m == "filled square")     marker = Display::FilledSquare ;
        else if(m == "filled diamond")    marker = Display::FilledDiamond;
        else if(m == "filled triangle")   marker = Display::FilledTriangle;
        else if(m == "filled inverted triangle")
            marker = Display::FilledInvertedTriangle;
        else if(m == "circle and cross")  marker = Display::CircleAndCross;
        else if(m == "circle and x")      marker = Display::CircleAndX;
        else if(m == "circle and dot")    marker = Display::CircleAndDot;
        else return false;
        return true;
    }
    
    static vector<Display::Marker> allMarkers() {
        static vector<Display::Marker> v(15);
        v[0] = Display::Cross;            v[1] = Display::X;
        v[2] = Display::Diamond;          v[3] = Display::Circle;
        v[4] = Display::Square;           v[5] = Display::Triangle;
        v[6] = Display::InvertedTriangle; v[7] = Display::FilledCircle;
        v[8] = Display::FilledSquare;     v[9] = Display::FilledDiamond;
        v[10] = Display::FilledTriangle; v[11]=Display::FilledInvertedTriangle; 
        v[12] = Display::CircleAndCross;  v[13] = Display::CircleAndX;
        v[14] = Display::CircleAndDot;
        return v;
    }
};


// Subclass of RegionShape used for drawing text.  Text uses RegionShape's
// label property.
class RSText : public RegionShape {
public:
    // World Constructor.  x, y, and angle must both be 1) using the same world
    // system as defined by worldSystem, and 2) in unit RegionShape::UNIT.
    // Angle is counterclockwise, and the base is the east vector.
    RSText(double x, double y, const String& text, MDirection::Types worldSys,
           double angle = 0);

    // Pixel Constructor.  Angle is counterclockwise.
    RSText(double x, double y, const String& text, double angle = 0);

    // Record Constructor.
    RSText(const RecordInterface& properties);

    // Destructor.
    ~RSText();

    // Implements RegionShape::drawAndUpdateBoundingBox.
    bool drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err = NULL);

    // Implements RegionShape::type.
    String type() const { return "text"; }

    // Gets text-specific coordinates, to avoid dependence on
    // coordParameterValues() when the shape type is known.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getTextCoordinates(double& x, double& y, double& angle,
                            String toSystem = "") const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues() const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues(String toSystem) const;
    
    // Implements RegionShape::coordParameterNames.
    vector<String> coordParameterNames() const;
    
    // Implements RegionShape::coordParameterPositions.
    vector<CoordinateParameterType> coordParameterTypes() const;
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals);
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals, String valSystem);
    
    // Implements RegionShape::move.
    void move(double dx, double dy, String system = "");
    
    // Implements RegionShape::getHandle.
    RSHandle getHandle() const;
    
    // Implements RegionShape option methods.  Note: text do not have
    // additonal options.
    // <group>
    vector<String> optionNames() const { return vector<String>(); }
    vector<OptionType> optionTypes() const { return vector<OptionType>(); }
    vector<RSOption> optionValues() const { return vector<RSOption>();}
    void setOptionValues(const vector<RSOption>& options) { }
    // </group>

protected:
    double m_x, m_y;             // location, pixel or world
    double m_angle;              // angle, pixel or world
    double m_wWidth;             // width in pixel/world
                                 // updated during updateScreenCoordinates
    double m_screenX, m_screenY; // location, screen

    // Implements RegionShape::getShapeSpecificProperties.
    void getShapeSpecificProperties(Record& properties) const {
        properties.define(PROPTYPE, PROPTYPE_TEXT);
    }
    
    // Implements RegionShape::setShapeSpecificProperties.
    void setShapeSpecificProperties(const RecordInterface& properties) { }
    
    // Updates screen coords using world/pixel coords and the given WorldCanvas
    bool updateScreenCoordinates(WorldCanvasHolder& wch, String* err);
    
    // Update m_xMin, m_xMax, m_yMin, and m_yMax.  Note that this is only valid
    // the screen coordinates have been updated.
    void updateMinMax();
};


// Subclass of RegionShape used for drawing composite regions.  A composite
// region consists of zero or more children RegionShapes.  A RSComposite can
// either see its children as dependent, in which the parent controls the
// properties for each shape, or independent in which each child's properties
// must be set individually.  An RSComposite can have a custom name/type.
// NOTE: xMin, xMax, yMin, and yMax values are ONLY valid if all children
// shapes have the same coordinate system as the parent composite.
// The following properties are also available:
// <ul><li><b>RSComposite::PROPDEPENDENTCHILDREN</b> (<i>bool</i>):
//         whether the composite's children are dependent (true) or independent
//         (false).  <b>OPTIONAL</b>.</li>
//     <li><b>RSComposite::PROPCHILDREN</b> (<i>Record of Records</li>):
//         ordered list of children records.  Each field must have a unique
//         name (the specifics are irrelevant because it's the order that
//         matters) and the value must be a record that can be used to create a
//         RegionShape child.  <b>NOT STRICTLY REQUIRED, BUT A VALID COMPOSITE
//         HAS AT LEAST ONE CHILD.</b></li></ul>
class RSComposite : public RegionShape {
public:
    // Static Members //
    
    // Properties.  (See class descriptions of RegionShape and RSComposite.)
    // <group>
    static const String PROPDEPENDENTCHILDREN;
    static const String PROPCHILDREN;
    // </group>
    
    
    // Non-Static Members //
    
    // World constructor.  Initially empty composite.  The dependent flag
    // controls whether the children are dependent or independent.  If
    // customName is set, it will be returned on type().  The given world
    // system is only used for displaying/setting the center of the composite
    // and does not affect the coordinate systems of the children.
    RSComposite(MDirection::Types worldSys, bool dependent = true,
                String customName = "");
    
    // Pixel constructor.  Initially empty composite.  The dependent flag
    // controls whether the children are dependent or independent.  If
    // custonName is set, it will be returned on type().
    RSComposite(bool dependent = true, String customName = "");

    // Record Constructor.
    RSComposite(const RecordInterface& properties);
    
    // Destructor.
    ~RSComposite();
    
    // Adds the given shape(s) to this composite.  Note that in the case of
    // depdendent children, line properties, linethrough properties, and label
    // properties of the individual shapes are discarded in favor of those of
    // the parent RSComposite.
    // <group>
    void addShape(RegionShape* shape);
    void addShapes(const vector<RegionShape*> shapes);
    // </group>
    
    // Returns true if the children are dependent, false otherwise.
    bool childrenAreDependent() const;
    
    // Sets whether children are dependent or not.  NOTE: This should never be
    // called AFTER the composite has already been added to the
    // QtRegionShapeManager.  Preferably it should be done before adding any
    // children either.
    void setChildrenAreDependent(bool dependent = true);
    
    // Returns the composite's children.
    // <group>
    vector<RegionShape*> children();
    vector<const RegionShape*> children() const;
    // </group>
    
    
    // Overrides RegionShape methods in the case of dependent children.
    // <group>
    void setLineColor(const String& newColor, bool alsoSetLabel = true);
    void setLineWidth(double width);
    void setLineStyle(LineStyle style);
    void setLabel(const RegionShapeLabel& label);
    // </group>
    
    // Implements RegionShape::drawAndUpdateBoundingBox.
    bool drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err = NULL);

    // Implements RegionShape::type.
    String type() const;
    
    // Overrides RegionShape::oneWordType.
    String oneWordType() const { return "composite"; }
    
    // Returns 0 degrees pixel/screen in the given coordinate system (own
    // system if toSystem is empty, otherwise see the constraints listed in
    // RegionShape::coordParameterValues(String).
    double getAngle(String toSystem = "") const;
    
    // Gets composite-specific coordinates, to avoid dependence on
    // coordParameterValues() when the shape type is known.
    // If toSystem is empty, no conversion is done; otherwise see the
    // constraints listed in RegionShape::coordParameterValues(String).
    void getCompositeCoordinates(double& centerX, double& centerY,
                                 String toSystem = "") const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues() const;
    
    // Implements RegionShape::coordParameterValues.
    vector<double> coordParameterValues(String toSystem) const;
    
    // Implements RegionShape::coordParameterNames.
    vector<String> coordParameterNames() const;
    
    // Implements RegionShape::coordParameterPositions.
    vector<CoordinateParameterType> coordParameterTypes() const;
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals);
    
    // Implements RegionShape::setCoordParameters.
    void setCoordParameters(const vector<double>& vals, String valSystem);
    
    // Implements RegionShape::move.
    void move(double dx, double dy, String system = "");
    
    // Implements RegionShape::getHandle.
    RSHandle getHandle() const;
    
    // Implements RegionShape::optionNames.
    vector<String> optionNames() const;
    
    // Implements RegionShape::optionTypes.
    vector<OptionType> optionTypes() const;
    
    // Implements RegionShape::optionValues.
    vector<RSOption> optionValues() const;
    
    // Implements RegionShape::setOptions.
    void setOptionValues(const vector<RSOption>& options);

protected:
    String m_custName;             // optional custom name
    bool m_dependentChildren;      // whether children are dependent
    vector<RegionShape*> m_shapes; // children
    double m_centerX, m_centerY;   // center
    bool m_syncAngleDepChildren;   // flag for whether all children:
                                   // 1) are dependent
                                   // 2) have angles (rectangle, ellipse,
                                   //    vector, text)
                                   // 3) all have synched (same) angle
    double m_syncAngle;            // Sync angle (if applicable).
    bool m_applyChangesToChildren; // Whether or not to apply options changes
                                   // to children or not.
    
    // Implements RegionShape::getShapeSpecificProperties.
    void getShapeSpecificProperties(Record& properties) const;
    
    // Implements RegionShape::setShapeSpecificProperties.
    void setShapeSpecificProperties(const RecordInterface& properties);
    
    // Update m_xMin, m_xMax, m_yMin, and m_yMax.
    void updateMinMax();
    
private:
    // Adds the given child but does not perform upkeep.  Returns true if the
    // shape was added, false otherwise.
    bool addShapeHelper(RegionShape* shape);
    
    // Does upkeep after adding shapes.
    void addShapeUpkeep();
    
    // Gets the min/max of the child at the given index, in the composite's
    // coordinate system.
    void convertMinMax(double& xmin, double& xmax, double& ymin, double& ymax,
                       unsigned int index) const;
};

}

#endif /*REGIONSHAPES_H_*/
