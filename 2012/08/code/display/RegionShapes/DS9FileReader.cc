//# DS9FileReader.cc: Implementation for DS9 region file reader etc.
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
#include <QFile>
#include <QTextStream>

#include <display/RegionShapes/DS9FileReader.h>
#include <display/RegionShapes/RegionShapes.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

#include <fstream>

namespace casa {

// DS9COORDINATE DEFINITIONS //

DS9Coordinate::DS9Coordinate(DS9::CoordinateSystem s, DS9::CoordinateUnit u) :
    m_system(s), m_unit(u) { }

DS9Coordinate::~DS9Coordinate() { }

void DS9Coordinate::set(DS9::CoordinateSystem system) { m_system = system; }

void DS9Coordinate::set(DS9::CoordinateUnit unit) { m_unit = unit; }

void DS9Coordinate::set(double value) { m_values.resize(1, value); }

void DS9Coordinate::set(double val1, double val2, double val3, bool mz) {
    m_values.resize(3);
    m_values[0] = val1; m_values[1] = val2; m_values[2] = val3;
    m_minusZero = mz;
}

DS9::CoordinateSystem DS9Coordinate::system() const { return m_system; }

DS9::CoordinateUnit DS9Coordinate::unit() const { return m_unit; }

vector<double> DS9Coordinate::values() const { return m_values; }

unsigned int DS9Coordinate::size() const { return m_values.size(); }

double DS9Coordinate::value() const {
    if(m_values.size() >= 1) return m_values[0];
    else return 0.;
}

bool DS9Coordinate::isValid() const {
    bool b = false;
    if(m_values.size() != 1 && m_values.size() != 3) return false;
    if(m_values.size() == 1) {
        b = m_unit == DS9::Degrees || m_unit == DS9::Radians ||
            m_unit == DS9::PhysicalPixels || m_unit == DS9::ImagePixels ||
            m_unit == DS9::ArcSec || m_unit == DS9::ArcMin ||
            m_unit == DS9::UNSET;
    } else {
        b = m_unit == DS9::HMS || m_unit == DS9::DMS;
    }

    if(b && m_unit == DS9::UNSET) {
        switch(m_system) {
        case DS9::Image:
            const_cast<DS9::CoordinateUnit&>(m_unit) = DS9::ImagePixels; break;
        case DS9::Physical:
            const_cast<DS9::CoordinateUnit&>(m_unit) = DS9::PhysicalPixels;
            break;
        case DS9::FK4: case DS9::FK5: case DS9::Galactic:
        case DS9::Ecliptic: case DS9::Amplifier: case DS9::Detector:
            const_cast<DS9::CoordinateUnit&>(m_unit) = DS9::Degrees; break;

        default: break;
        }
    }

    return b;
}

double DS9Coordinate::toDegrees() const {
    if(!isValid()) return 0;

    double deg = 0;
    switch(m_unit) {
    case DS9::Degrees: deg = m_values[0]; break;
    case DS9::Radians: deg = m_values[0] * 180 / C::pi; break;
    case DS9::HMS:
        if(m_values[0] < 0 || (m_values[0] == 0 && m_minusZero))
            deg = (m_values[0]*15) - (m_values[1]/4) - (m_values[2]/240);
        else deg = (m_values[0]*15) + (m_values[1]/4) + (m_values[2]/240);
        break;
    case DS9::DMS:
        if(m_values[0] < 0 || (m_values[0] == 0 && m_minusZero))
            deg = m_values[0] - (m_values[1] / 60) - (m_values[2] / 3600);
        else deg = m_values[0] + (m_values[1] / 60) + (m_values[2] / 3600);
        break;
    case DS9::ArcSec: deg = m_values[0] / 3600; break;
    case DS9::ArcMin: deg = m_values[0] / 60; break;
    default: return 0;
    }
    
    return deg;
}

String DS9Coordinate::toPrintString() const {
    if(!isValid()) return "?";

    stringstream ss;

    if(m_values.size() == 1) ss << m_values[0];
    else {
        ss << m_values[0];
        if(m_unit == DS9::HMS) ss << DS9::FILE_HMS_H;
        else ss << DS9::FILE_DMS_D;
        ss << m_values[1];
        if(m_unit == DS9::HMS) ss << DS9::FILE_HMS_M;
        else ss << DS9::FILE_DMS_M;
        ss << m_values[2];
        if(m_unit == DS9::HMS) ss << DS9::FILE_HMS_S;
        else ss << DS9::FILE_DMS_S;
    }

    switch(m_unit) {
    case DS9::Degrees: ss << DS9::FILE_DEGREES; break;
    case DS9::Radians: ss << DS9::FILE_RADIANS; break;
    case DS9::PhysicalPixels: ss << DS9::FILE_PHYSICAL_PIXELS; break;
    case DS9::ImagePixels: ss << DS9::FILE_IMAGE_PIXELS; break;
    case DS9::ArcSec: ss << DS9::FILE_ARCSEC; break;
    case DS9::ArcMin: ss << DS9::FILE_ARCMIN; break;

    default: break;
    }

    return ss.str();
}


// DS9REGION DEFINITIONS //

DS9Region::DS9Region(DS9::RegionType type, DS9::CoordinateSystem coordSys) :
        m_type(type), m_system(coordSys) {
    // set default properties
    vector<String> props = properties();
    String prop;
    for(unsigned int i = 0; i < props.size(); i++) {
        prop = props[i];
        if(isBoolProperty(prop)) define(prop, defaultBoolValue(prop));
        else define(prop, defaultStringValue(prop));
    }
}

DS9Region::~DS9Region() { }


String DS9Region::toPrintString() const {
    stringstream ss;

    ss << DS9::coordinateSystem(m_system).toStdString();
    ss << DS9::FILE_LINESEP.toStdString() << " ";
    if(m_props.isDefined(PROP_INCLUDE) && !m_props.asBool(PROP_INCLUDE))
        ss << DS9::FILE_MINUS.toStdString();
    ss << DS9::regionType(m_type).toStdString() << " ";

    for(unsigned int i = 0; i < m_coords.size(); i++)
        ss << m_coords[i].toPrintString() << " ";

    if(m_props.isDefined(PROP_COMPOSITE) && m_props.asBool(PROP_COMPOSITE))
        ss << DS9::FILE_COMPOSITE_OR.toStdString() << " ";

    if(m_props.nfields() > 0) ss << DS9::FILE_COMMENT.toStdString();
    
    // special compass case
    if(m_type == DS9::Compass) {
        // compass=<coord sys> <nlabel> <elabel> [0|1] [0|1]
        if(m_props.isDefined(PROP_COMPASS))
            ss << ' ' << PROP_COMPASS << DS9::FILE_EQUAL.toStdString() <<
                  m_props.asString(PROP_COMPASS);
        if(m_props.isDefined(PROP_COMPASS_NLABEL))
            ss << " {" << m_props.asString(PROP_COMPASS_NLABEL) << '}';
        if(m_props.isDefined(PROP_COMPASS_ELABEL))
            ss << " {" << m_props.asString(PROP_COMPASS_ELABEL) << '}';
        if(m_props.isDefined(PROP_LINE))
            ss << ' ' << m_props.asString(PROP_LINE);
    }

    vector<String> props = properties();
    String prop;
    for(unsigned int i = 0; i < props.size(); i++) {
        prop = props[i];
        if(m_props.isDefined(prop)) {
            if(prop == PROP_COMPASS || prop == PROP_COMPASS_NLABEL ||
               prop == PROP_COMPASS_ELABEL || prop == PROP_LINE) continue;            
            if(prop == PROP_LINE && m_type != DS9::Line &&
               m_type != DS9::Compass) continue;
            if(prop == PROP_VECTOR && m_type != DS9::Vector) continue;
            if(prop == PROP_RULER && m_type != DS9::Ruler) continue;
            if(prop == PROP_INCLUDE) continue;
            if(prop == PROP_TEXTANGLE && m_type != DS9::Text) continue;
            if(prop == PROP_TEXT && m_props.asString(prop).empty()) continue;
            if(prop == PROP_WIDTH && QString(
                    m_props.asString(prop).c_str()).toDouble() <= 0) continue;
            if((prop == PROP_SOURCE || prop == PROP_BACKGROUND) &&
                !m_props.asBool(prop)) continue;
            if(prop == PROP_COMPOSITE) continue;
            if(prop == PROP_TAG && m_props.asString(PROP_TAG).empty())
                continue;
            if(prop == PROP_MARKER_SIZE && m_type != DS9::CirclePoint &&
               m_type != DS9::BoxPoint && m_type != DS9::DiamondPoint &&
               m_type != DS9::CrossPoint && m_type != DS9::XPoint &&
               m_type != DS9::ArrowPoint && m_type != DS9::BoxCirclePoint)
                continue;

            ss << ' ';
            if(prop == PROP_SOURCE || prop == PROP_BACKGROUND) {
                if(m_props.asBool(prop)) ss << prop;
            } else if(prop == PROP_MARKER_SIZE) {
                ss << m_props.asString(prop);
            } else if(m_props.dataType(prop)== TpBool&& prop != PROP_INCLUDE) {
                ss << prop << DS9::FILE_EQUAL.toStdString();
                ss << m_props.asBool(prop);
            } else if(m_props.dataType(prop) == TpString) {
                ss << prop << DS9::FILE_EQUAL.toStdString();
                if(prop == PROP_TEXT)
                    ss << "{" << m_props.asString(prop) << "}";
                else if(prop == PROP_FONT)
                    ss << '"' << m_props.asString(prop) << '"';
                else ss << m_props.asString(prop);
            }
        }
    }

    return ss.str();
}

String DS9Region::name() const {
    return DS9::regionType(m_type).toStdString();
}

RegionShape* DS9Region::toRegionShape() const {
    if(!const_cast<DS9Region*>(this)->checkCoordinates()) {
        setError("Invalid coordinates.", true);
        return NULL;
    }

    if(m_system == DS9::Linear || m_system == DS9::Amplifier ||
       m_system == DS9::Detector) {
        setError("Coordinate system not yet supported.", true);
        return NULL;
    }
    
    MDirection::Types t = MDirection::GALACTIC;
    if(m_system == DS9::FK4)           t = MDirection::B1950;
    else if(m_system == DS9::FK5)      t = MDirection::J2000;
    else if(m_system == DS9::Galactic) t = MDirection::GALACTIC;
    else if(m_system == DS9::Ecliptic) t = MDirection::ECLIPTIC;
    bool isWorld = m_system == DS9::FK4 || m_system == DS9::FK5 ||
                   m_system == DS9::Galactic || m_system == DS9::Ecliptic;
    
    RegionShape* s = NULL;
    
    switch(m_type) {
    case DS9::Ruler: case DS9::Compass: case DS9::Projection: case DS9::Panda:
    case DS9::EllipticalPanda: case DS9::BoxPanda:
        setError("Shape type not yet supported.", true);
        return NULL;
        
    // TODO more shapes
        
    case DS9::Composite: {
        RSComposite* c = isWorld ? (new RSComposite(t, false)) :
                                   (new RSComposite(false));
        for(unsigned int i = 0; i < m_compositeRegions.size(); i++) {
            s = m_compositeRegions[i].toRegionShape();
            if(s != NULL) c->addShape(s);
            else setError("One or more composite regions were invalid.",
                          false);
        }
        s = c;
        break; }
    
    case DS9::BoxAnnulus: {
        // angles not yet supported
        double x = isWorld ? m_coords[0].toDegrees() : m_coords[0].value(),
               y = isWorld ? m_coords[1].toDegrees() : m_coords[1].value();
        RSComposite* c = isWorld ? (new RSComposite(t, true, "box annulus")):
                                   (new RSComposite(true, "box annulus"));
        RSRectangle* r;
        double a = m_coords[m_coords.size() - 1].value();
        if(isWorld) a = 180 - a;
        while(a >= 360)  a -= 360;
        while(a < 0) a += 360;
        
        double w, h;
        for(unsigned int i = 2; i < m_coords.size() - 2; i += 2) {
            w = isWorld ? m_coords[i].toDegrees() : m_coords[i].value();
            h = isWorld ? m_coords[i+1].toDegrees():m_coords[i+1].value();
            r = isWorld ? (new RSRectangle(x, y, w, h, t, a)) :
                          (new RSRectangle(x, y, w, h, a));
            c->addShape(r);
        }
        s = c;
        break; }
    
    case DS9::Annulus: {
        double x = isWorld ? m_coords[0].toDegrees() : m_coords[0].value(),
               y = isWorld ? m_coords[1].toDegrees() : m_coords[1].value();
        RSComposite* c = isWorld ? (new RSComposite(t, true, "annulus")) :
                                   (new RSComposite(true, "annulus"));
        RSCircle* e;
        double r;
        for(unsigned int i = 2; i < m_coords.size(); i++) {
            r = isWorld ? m_coords[i].toDegrees() : m_coords[i].value();
            e = isWorld ? (new RSCircle(x, y, r, t)) :
                          (new RSCircle(x, y, r));
            c->addShape(e);
        }
        s = c;
        break; }
        
    case DS9::EllipseAnnulus: {
        double x = isWorld ? m_coords[0].toDegrees() : m_coords[0].value(),
               y = isWorld ? m_coords[1].toDegrees() : m_coords[1].value(),
               a = m_coords[m_coords.size() - 1].value();
               
        if(isWorld) a = 180 - a;        
        while(a >= 360) a -= 360;
        while(a < 0) a += 360;
               
        RSComposite* c = isWorld? (new RSComposite(t, true,"ellipse annulus")):
                                  (new RSComposite(true, "ellipse annulus"));
        RSEllipse* e;
        double r1, r2;
        for(unsigned int i = 2; i < m_coords.size() - 2; i += 2) {
            r1 = isWorld ? m_coords[i].toDegrees() : m_coords[i].value();
            r2 = isWorld ? m_coords[i+1].toDegrees():m_coords[i+1].value();
            e = isWorld ? (new RSEllipse(x, y, r1, r2, t, a)) :
                          (new RSEllipse(x, y, r1, r2, a));
            c->addShape(e);
        }
        s = c;
        break; }

    case DS9::Line: {
        bool a1 = false, a2 = false;
        if(m_props.isDefined(PROP_LINE)) {
            QStringList spl = QString(m_props.asString(
                    PROP_LINE).c_str()).split(QRegExp("\\s+"));
            a1 = spl[0] != "0"; a2 = spl[1] != "0";
        }
        int size = DS9::ARROW_SIZE;
        if(isWorld) {
            s = new RSLine(m_coords[0].toDegrees(), m_coords[1].toDegrees(),
                           m_coords[2].toDegrees(), m_coords[3].toDegrees(),
                           t, size, a1, a2);
        } else {
            s = new RSLine(m_coords[0].value(), m_coords[1].value(),
                           m_coords[2].value(), m_coords[3].value(), size, a1,
                           a2);
        }
        break; }
    
    case DS9::Vector: {
        bool arrow = false;
        if(m_props.isDefined(PROP_VECTOR)) arrow = m_props.asBool(PROP_VECTOR);
        int size = DS9::ARROW_SIZE;
        
        double a = m_coords[3].value();
        if(isWorld) a = 180 - a;
        while(a >= 360) a -= 360;
        while(a < 0) a += 360;
        
        if(isWorld) {            
            s = new RSVector(m_coords[0].toDegrees(), m_coords[1].toDegrees(),
                             m_coords[2].toDegrees(), a, t, size, arrow);
        } else {
            s = new RSVector(m_coords[0].value(), m_coords[1].value(),
                             m_coords[2].value(), a, size, arrow);
        }
        break; }

    case DS9::CirclePoint: case DS9::BoxPoint: case DS9::DiamondPoint:
    case DS9::CrossPoint: case DS9::XPoint: case DS9::BoxCirclePoint:
    case DS9::ArrowPoint: {
        vector<Display::Marker> m(1, Display::Circle);
        if(m_type == DS9::BoxPoint) m[0] = Display::Square;
        else if(m_type == DS9::DiamondPoint) m[0] = Display::Diamond;
        else if(m_type == DS9::CrossPoint) m[0] = Display::Cross;
        else if(m_type == DS9::XPoint) m[0] = Display::X;
        else if(m_type == DS9::BoxCirclePoint) {
            m[0] = Display::Square;
            m.push_back(Display::Circle);
        } else if(m_type == DS9::ArrowPoint) {
            m[0] = Display::X;
            setError("Arrow point not supported. Switching to X point.",false);
        }

        int size = DS9::MARKER_SIZE;
        if(m_props.isDefined(PROP_MARKER_SIZE))
            size = QString(m_props.asString(PROP_MARKER_SIZE).c_str()).toInt();

        if(isWorld) {
            s = new RSMarker(m_coords[0].toDegrees(), m_coords[1].toDegrees(),
                             m, size, t);
        } else {
            s = new RSMarker(m_coords[0].value(), m_coords[1].value(), m,size);
        }
        break; }

    case DS9::Circle: {
        if(isWorld)
            s = new RSCircle(m_coords[0].toDegrees(), m_coords[1].toDegrees(),
                             m_coords[2].toDegrees(), t);
        else
            s = new RSCircle(m_coords[0].value(), m_coords[1].value(),
                             m_coords[2].value());
        break; }

    case DS9::Ellipse: {
        double a = m_coords[4].value();
        if(isWorld) a = 180 - a;
        while(a >= 360) a -= 360;
        while(a < 0) a += 360;
        if(isWorld) {            
            s = new RSEllipse(m_coords[0].toDegrees(), m_coords[1].toDegrees(),
                              m_coords[2].toDegrees(), m_coords[3].toDegrees(),
                              t, a);
        } else {
            s = new RSEllipse(m_coords[0].value(), m_coords[1].value(),
                              m_coords[2].value(), m_coords[3].value(), a);
        }
        break; }

    case DS9::Box: {
        double a = m_coords[4].value();
        if(isWorld) a = 180 - a;
        while(a >= 360) a -= 360;
        while(a < 0) a += 360;
        if(isWorld) {
            s = new RSRectangle(m_coords[0].toDegrees(),
                    m_coords[1].toDegrees(), m_coords[2].toDegrees(),
                    m_coords[3].toDegrees(), t, a);
        } else {
            s = new RSRectangle(m_coords[0].value(), m_coords[1].value(),
                                m_coords[2].value(), m_coords[3].value(), a);
        }
        break; }

    case DS9::Text: {
        if(!m_props.isDefined(PROP_TEXT)||m_props.asString(PROP_TEXT).empty()){
            setError("Text is empty.", true);
            return NULL;
        }
        double a = 0;
        if(m_props.isDefined(PROP_TEXTANGLE)) {
            a = QString(m_props.asString(PROP_TEXTANGLE).c_str()).toDouble();
            if(isWorld) a = 180 - a;
            while(a >= 360) a -= 360;
            while(a < 0) a += 360;
        }

        if(isWorld) {
            s = new RSText(m_coords[0].toDegrees(), m_coords[1].toDegrees(),
                           m_props.asString(PROP_TEXT), t, a);
        } else {
            s = new RSText(m_coords[0].value(), m_coords[1].value(),
                           m_props.asString(PROP_TEXT), a);
        }
        // the actual text will be taken care of below
        break; }

    case DS9::Polygon: {
        if(isWorld) {
            unsigned int n = m_coords.size() / 2;
            Vector<double> x(n), y(n);
            for(unsigned int i = 0; i < m_coords.size() - 1; i += 2) {
                x(i / 2) = m_coords[i].toDegrees();
                y(i / 2) = m_coords[i + 1].toDegrees();
            }
            s = new RSPolygon(x, y, t);
        } else {
            unsigned int n = m_coords.size() / 2;
            Vector<double> x(n), y(n);
            for(unsigned int i = 0; i < m_coords.size() - 1; i += 2) {
                x(i / 2) = m_coords[i].value();
                y(i / 2) = m_coords[i + 1].value();
            }
            s = new RSPolygon(x, y);
        }

        break; }
    }

    if(s == NULL) return s;

    // text/label
    RegionShapeLabel label(!m_props.isDefined(PROP_TEXT) ? "" :
                           m_props.asString(PROP_TEXT));
    if(m_props.isDefined(PROP_FONT)) {
        QStringList split = QString(m_props.asString(
                                    PROP_FONT).c_str()).split(QRegExp("\\s+"));
        label.setFont(split[0].toStdString());
        label.setSize(split[1].toInt());
        if(split[2] == "bold") label.setBold(true);
        else if(split[2] == "italic" || split[2] == "italics")
            label.setItalic(true);
    }
    if(m_props.isDefined(PROP_TEXTANGLE)) {
        double angle = QString(m_props.asString(
                PROP_TEXTANGLE).c_str()).toDouble();
        if(angle != 0) label.setAngle(angle);
    }
    s->setLabel(label);

    // properties
    if(m_props.isDefined(PROP_COLOR))
        s->setLineColor(m_props.asString(PROP_COLOR), true);
    double width = 1;
    if(m_props.isDefined(PROP_WIDTH)) {
        width = QString(m_props.asString(PROP_WIDTH).c_str()).toDouble();
        s->setLineWidth(width);
    }
    
    bool background = (!m_props.isDefined(PROP_SOURCE) ||
                       !m_props.asBool(PROP_SOURCE)) ||
                      (m_props.isDefined(PROP_BACKGROUND) &&
                       m_props.asBool(PROP_BACKGROUND));    
    bool dash = (m_props.isDefined(PROP_DASH) && m_props.asBool(PROP_DASH)) ||
                background;
    bool dot = false;
    if(m_props.isDefined(PROP_DASHLIST)) {
        unsigned int d= QString(m_props.asString(PROP_DASHLIST).c_str()).split(
                        QRegExp("\\s+"))[0].toUInt();
        if(d < 5) dot = true;
    }
    
    RegionShape::LineStyle style = dash ? (dot ? RegionShape::DOTTED :
                                   RegionShape::DASHED) : RegionShape::SOLID;
    
    if(m_props.isDefined(PROP_INCLUDE)) {
        s->setLinethrough(!m_props.asBool(PROP_INCLUDE), "red", width, style);
    }
    
    s->setLineStyle(style);
    
    return s;
}

DS9::RegionType DS9Region::type() const { return m_type; }

bool DS9Region::setProperties(const RecordInterface& props) {
    for(unsigned int i = 0; i < props.nfields(); i++) {
        if(!isProperty(props.name(i))) return false;
        if(isBoolProperty(props.name(i))) {
            if(props.dataType(i) != TpBool) return false;
        } else if(props.dataType(i) != TpString ||
                  !valueIsValid(props.name(i), props.asString(i), m_type))
            return false;
    }

    for(unsigned int i = 0; i < props.nfields(); i++) {
        if(props.dataType(i) == TpBool) {
            if(!define(props.name(i), props.asBool(i)))   // shouldn't happen
                return false;
        } else if(props.dataType(i) == TpString) {
            if(!define(props.name(i), props.asString(i))) // shouldn't happen
                return false;
        }
    }

    return true;
}

bool DS9Region::define(const String& property, bool value) {
    if(isBoolProperty(property)) {
        m_props.define(property, value);
        return true;
    } else return false;
}

bool DS9Region::define(const String& property, const String& value) {
    if(isProperty(property) && !isBoolProperty(property) &&
       valueIsValid(property, value, m_type)) {
        m_props.define(property, value);
        return true;
    } else return false;
}

bool DS9Region::isDefined(const String& property) {
    return m_props.isDefined(property);
}

bool DS9Region::boolValue(const String& property) {
    if(isBoolProperty(property) && m_props.isDefined(property))
        return m_props.asBool(property);
    else return false;
}

String DS9Region::stringValue(const String& property) {
    if(isProperty(property) && !isBoolProperty(property) &&
       m_props.isDefined(property)) return m_props.asString(property);
    else return "";
}

void DS9Region::pushCoordinate(const DS9Coordinate& coord) {
    m_coords.push_back(coord);
}

void DS9Region::pushCompositeRegion(const DS9Region& region) {
    if(m_type == DS9::Composite) m_compositeRegions.push_back(region);
}

bool DS9Region::checkCoordinates() {
    unsigned int n = m_coords.size();
    switch(m_type) {
    case DS9::Text: case DS9::CirclePoint: case DS9::BoxPoint:
    case DS9::DiamondPoint: case DS9::CrossPoint: case DS9::XPoint:
    case DS9::ArrowPoint: case DS9::BoxCirclePoint: return n == 2;
    
    case DS9::Circle: case DS9::Compass: case DS9::Composite: return n == 3;
    
    case DS9::Line: case DS9::Ruler: case DS9::Vector: return n == 4;
    
    case DS9::Annulus: return n >= 4;
    
    case DS9::Ellipse: case DS9::Box: case DS9::Projection: return n == 5;
    
    case DS9::Polygon: return n >= 6;

    case DS9::EllipseAnnulus: case DS9::BoxAnnulus:
        if(n < 5) return false;
        else if(n == 5) {
            if(m_type == DS9::EllipseAnnulus) m_type = DS9::Ellipse;
            else if(m_type == DS9::BoxAnnulus) m_type = DS9::Box;
            return true;
        } else return n % 2 == 1; // n must be odd

    case DS9::Panda: return n == 8;
    
    case DS9::EllipticalPanda: case DS9::BoxPanda: return n == 9;

    default: return false;
    }
}

bool DS9Region::checkProperties() {
    switch(m_type) {
    case DS9::Text:  return m_props.isDefined(PROP_TEXT) &&
                            !m_props.asString(PROP_TEXT).empty();
    case DS9::Ruler: return m_props.isDefined(PROP_RULER);

    default: return true;
    }
}

const RFError& DS9Region::lastError() const { return m_lastError; }

void DS9Region::setError(const String& error, bool isFatal) const {
    const_cast<RFError&>(m_lastError).set(error, isFatal);
}


// DS9FILEREADER DEFINITIONS //

DS9FileReader::DS9FileReader() { }

DS9FileReader::~DS9FileReader() { }


bool DS9FileReader::read(vector<RegionShape*>& shapes) {
    bool success = true;
    stringstream invalid;

    if(m_filename.empty()) {
        success = false;
        invalid << "No file has been set to read!";
        setError(invalid.str(), true);
        return success;
    }

    QFile file(m_filename.c_str());
    if(!file.exists() || !file.open(QIODevice::ReadOnly)) {
        success = false;
        invalid << "Cannot open file: '" << m_filename << "'.";
        setError(invalid.str(), true);
        return success;
    }
    
    QTextStream text(&file);
    QString line = text.readLine();
    QStringList lines;
    while(!line.isNull()) {
        lines = line.split(DS9::FILE_LINESEP, QString::SkipEmptyParts);
        for(int ind = 0; ind < lines.size(); ind++) {
        line = lines[ind].trimmed();
        
        if(line.startsWith(DS9::FILE_COMMENT)) {
            // check for special case for newer DS9 format where what SHOULD be
            //   text(coords) # properties
            //   vector(coords) # properties
            //   ruler(coords) # properties
            //   compass(coords) # properties
            //   projection(coords) # properties
            //   composite(coords) # properties
            // for some unknown and utterly stupid reason has become
            //   # text(coords) properties
            //   # vector(coords) properties
            //   # ruler(coords) properties
            //   # compass(coords) properties
            //   # projection(coords) properties
            //   # composite(coords) properties

            QRegExp dlm("[\\s(),#]");
            QStringList list = line.split(dlm, QString::SkipEmptyParts);
            if(list.size() > 0) {
            list[0] = list[0].toLower();
            if((list.size() >= 4 && list[0] == DS9::FILE_TEXT) ||
               (list.size() >= 6 && list[0] == DS9::FILE_VECTOR) ||
               (list.size() >= 6 && list[0] == DS9::FILE_RULER) ||
               (list.size() >= 5 && list[0] == DS9::FILE_COMPASS) ||
               (list.size() >= 7 && list[0] == DS9::FILE_PROJECTION) ||
               (list.size() >= 5 && list[0] == DS9::FILE_COMPOSITE)) {
                unsigned int size = 0;
                if(list[0] == DS9::FILE_TEXT) size = 2;
                else if(list[0] == DS9::FILE_COMPASS ||
                        list[0] == DS9::FILE_COMPOSITE) size = 3;
                else if(list[0] == DS9::FILE_VECTOR ||
                        list[0] == DS9::FILE_RULER) size = 4;
                else if(list[0] == DS9::FILE_PROJECTION) size = 5;
                bool ok;
                bool valid = size > 0;
                for(unsigned int i = 0; i < size && valid; i++) {
                    list[i + 1].toDouble(&ok);
                    if(!ok) {
                        ok = list[i + 1].indexOf(DS9::REGEXP_HDMS) == 0 ||
                             list[i + 1].indexOf(DS9::REGEXP_HMS) == 0 ||
                             list[i + 1].indexOf(DS9::REGEXP_DMS) == 0 ||
                             list[i + 1].indexOf(DS9::REGEXP_NUMSYS) == 0;
                    }
                    if(!ok) valid = false;
                }
                if(valid) {
                    // rearrange appropriately
                    int index = line.indexOf(list[0]);
                    for(unsigned int i = 0; i < size; i++)
                        index = line.indexOf(list[i+1], index+ list[i].size());

                    index = line.indexOf(dlm, index + 1);
                    
                    while(line[index].isSpace() || line[index] == '(' ||
                          line[index] == ')' || line[index] == ',' ||
                          line[index] == '#') index++;
                    
                    QString nl = list[0] + "(";
                    for(unsigned int i = 0; i < size; i++)
                        nl += list[i + 1] + ((i < size - 1) ? "," : ") ");
                    nl += DS9::FILE_COMMENT + " " +
                          line.right(line.size() - index);
                    line = nl;
                }
            }
            }
        }
        
        if(!line.startsWith(DS9::FILE_COMMENT)) // skip comments
            success &= processLine(line, invalid);
        }
        line = text.readLine();
    }

    file.close();
    
    // combine composite shapes as necessary
    bool inComposite = false;
    for(unsigned int i = 0; i < m_regions.size(); i++) {
        if(inComposite) {
            // make sure there are no nested composites
            if(m_regions[i].type() == DS9::Composite) {
                m_regions.erase(m_regions.begin() + i);
                i--;
                continue;
            }
            
            // add this region to the composite
            m_regions[i - 1].pushCompositeRegion(m_regions[i]);
            
            // check whether to continue the composite
            if(m_regions[i].isDefined(DS9Region::PROP_COMPOSITE)) {
                if(!m_regions[i].boolValue(DS9Region::PROP_COMPOSITE))
                    inComposite = false;
            } else inComposite = false;
            
            // remove from main list
            m_regions.erase(m_regions.begin() + i);
            i--;
        } else {
            if(m_regions[i].type() == DS9::Composite) inComposite = true;
        }
    }
    
    // convert regions to shapes
    shapes.clear();
    RegionShape* shape;
    for(unsigned int i = 0; i < m_regions.size(); i++) {
        shape = m_regions[i].toRegionShape();
        if(shape == NULL) {
            RSUtils::appendUniqueMessage(invalid, m_regions[i].name() + ": " +
                                         m_regions[i].lastError().error());
            success = false;
        } else {
            if(!m_regions[i].lastError().error().empty()) {
                RSUtils::appendUniqueMessage(invalid, m_regions[i].name() +
                                   ": " + m_regions[i].lastError().error());
                success = false;
            }
            shapes.push_back(shape);
        }
    }
    
    // Reset members
    m_nextSystem.second = false;
    m_regions.clear();
    m_globals = Record();
    
    setError(invalid.str());
    return success;
}


// Private Methods //

bool DS9FileReader::processLine(const QString& line, stringstream& invalid) {
    if(line.isNull() || line.isEmpty() || line.startsWith(DS9::FILE_COMMENT))
        return true;

    bool success = true;

    QStringList list = line.split(DS9::FILE_COMMENT);
    QString command = list[0].trimmed();
    QString comment = list.size() > 1 ? list[1].trimmed() : "";
    for(int i = 2; i < list.size(); i++)
        comment += "#" + list[i];

    QStringList cmdlist = command.split(QRegExp("[\\s+(),]"),
                                        QString::SkipEmptyParts);

    QStringList regfw = DS9::regionFirstWords();

    if(regfw.contains(cmdlist[0], Qt::CaseInsensitive)) {
        success = processRegion(cmdlist, comment, invalid);
    } else if(DS9::coordinateSystemFirstWords().contains(cmdlist[0],
                                                        Qt::CaseInsensitive)) {
        success = processCoordSys(command, invalid);
    } else if(cmdlist[0].toLower() == DS9::FILE_GLOBAL) {
        success = processGlobal(command, invalid);
    } else {
        // check for +region or -region case
        if((cmdlist[0] == DS9::FILE_PLUS || cmdlist[0] == DS9::FILE_MINUS) &&
           cmdlist.size() >= 2 && regfw.contains(cmdlist[1],
                                                 Qt::CaseInsensitive)) {
            bool include = cmdlist[0] == DS9::FILE_PLUS;
            cmdlist.removeFirst();
            success = processRegion(cmdlist, comment, invalid, include);
        } else if(cmdlist[0].startsWith(DS9::FILE_PLUS) ||
                  cmdlist[0].startsWith(DS9::FILE_MINUS)) {
            bool include = cmdlist[0].startsWith(DS9::FILE_PLUS);
            if(include)
                cmdlist[0] = cmdlist[0].right(cmdlist[0].size() -
                                              DS9::FILE_PLUS.size());
            else
                cmdlist[0] = cmdlist[0].right(cmdlist[0].size() -
                                              DS9::FILE_MINUS.size());
            success = processRegion(cmdlist, comment, invalid, include);
        } else {
            success = false;
            invalid << "\n* Unknown command: \"" << line.toStdString() << '"';
        }
    }

    return success;
}

bool DS9FileReader::processRegion(QStringList& line, QString& comment,
        stringstream& invalid, bool include) {
    DS9::CoordinateSystem coordSys = DS9::defaultCoordinateSystem();
    if(m_nextSystem.second) coordSys = m_nextSystem.first;

    QString first = line[0].toLower();
    DS9::RegionType type = DS9::Circle;
    if(first == DS9::FILE_CIRCLE) {
        if(line.size() >= 2 && line[1].toLower() == DS9::FILE_POINT) {
            type = DS9::CirclePoint;
            line.removeFirst();
        } else type = DS9::Circle;
        line.removeFirst();
    } else if(first == DS9::FILE_ANNULUS) {
        type = DS9::Annulus;
        line.removeFirst();
    } else if(first == DS9::FILE_ELLIPSE) {
        type = DS9::EllipseAnnulus; // will be put to a normal ellipse
                                    // later if applicable
        line.removeFirst();
    } else if(first == DS9::FILE_BOX) {
        if(line.size() >= 2 && line[1].toLower() == DS9::FILE_POINT) {
            type = DS9::BoxPoint;
            line.removeFirst();
        } else type = DS9::BoxAnnulus; // will be put to a normal box later
                                       // if applicable
        line.removeFirst();
    } else if(first == DS9::FILE_POLYGON) {
        type = DS9::Polygon;
        line.removeFirst();
    } else if(first == DS9::FILE_LINE) {
        type = DS9::Line;
        line.removeFirst();
    } else if(first == DS9::FILE_VECTOR) {
        type = DS9::Vector;
        line.removeFirst();
    } else if(first == DS9::FILE_TEXT) {
        type = DS9::Text;
        line.removeFirst();
    } else if(first == DS9::FILE_RULER) {
        type = DS9::Ruler;
        line.removeFirst();
    } else if(first == DS9::FILE_POINT) {
        if(!readPointType(comment, type)) {
            invalid << "\n* Given point type is invalid!";
            return false;
        }
        line.removeFirst();
    } else if(first == DS9::FILE_DIAMOND) {
        if(line.size() < 2 || line[1].toLower() != DS9::FILE_POINT) {
            invalid << "\n* \"" << DS9::FILE_DIAMOND.toStdString();
            invalid << "\" not followed by \"";
            invalid << DS9::FILE_POINT.toStdString() << "\"!";
            return false;
        }
        type = DS9::DiamondPoint;
        line.removeFirst(); line.removeFirst();
    } else if(first == DS9::FILE_CROSS) {
        if(line.size() < 2 || line[1].toLower() != DS9::FILE_POINT) {
            invalid << "\n* \"" << DS9::FILE_CROSS.toStdString();
            invalid << "\" not followed by \"";
            invalid << DS9::FILE_POINT.toStdString() << "\"!";
            return false;
        }
        type = DS9::CrossPoint;
        line.removeFirst(); line.removeFirst();
    } else if(first == DS9::FILE_X) {
        if(line.size() < 2 || line[1].toLower() != DS9::FILE_POINT) {
            invalid << "\n* \"" << DS9::FILE_X.toStdString();
            invalid << "\" not followed by \"";
            invalid << DS9::FILE_POINT.toStdString() << "\"!";
            return false;
        }
        type = DS9::XPoint;
        line.removeFirst(); line.removeFirst();
    } else if(first == DS9::FILE_ARROW) {
        if(line.size() < 2 || line[1].toLower() != DS9::FILE_POINT) {
            invalid << "\n* \"" << DS9::FILE_ARROW.toStdString();
            invalid << "\" not followed by \"";
            invalid << DS9::FILE_POINT.toStdString() << "\"!";
            return false;
        }
        type = DS9::ArrowPoint;
        line.removeFirst(); line.removeFirst();
    } else if(first == DS9::FILE_BOXCIRCLE) {
        if(line.size() < 2 || line[1].toLower() != DS9::FILE_POINT) {
            invalid << "\n* \"" << DS9::FILE_BOXCIRCLE.toStdString();
            invalid << "\" not followed by \"";
            invalid << DS9::FILE_POINT.toStdString() << "\"!";
            return false;
        }
        type = DS9::BoxCirclePoint;
        line.removeFirst(); line.removeFirst();
    } else if(first == DS9::FILE_COMPASS) {
        type = DS9::Compass;
        line.removeFirst();
    } else if(first == DS9::FILE_PROJECTION) {
        type = DS9::Projection;
        line.removeFirst();
    } else if(first == DS9::FILE_PANDA) {
        type = DS9::Panda;
        line.removeFirst();
    } else if(first == DS9::FILE_EPANDA) {
        type = DS9::EllipticalPanda;
        line.removeFirst();
    } else if(first == DS9::FILE_BPANDA) {
        type = DS9::BoxPanda;
        line.removeFirst();
    } else if(first == DS9::FILE_COMPOSITE) {
        type = DS9::Composite;
        line.removeFirst();
    } else {
        invalid << "\n* Unknown region type.";
        return false;
    }

    DS9Region region(type, coordSys);

    if(m_globals.nfields() > 0) {
        if(!region.setProperties(m_globals)) {
            invalid << "\n* Invalid global properties!";
        }
    }

    region.define(DS9Region::PROP_INCLUDE, include);
    
    int orInd;
    if((orInd = line.indexOf(DS9::FILE_COMPOSITE_OR)) >= 0) {
        region.define(DS9Region::PROP_COMPOSITE, true);
        line.removeAt(orInd);
    } else if((orInd = comment.indexOf(DS9::FILE_COMPOSITE_OR)) >= 0) {
        region.define(DS9Region::PROP_COMPOSITE, true);
        comment.remove(orInd, DS9::FILE_COMPOSITE_OR.size());
    }

    int numCoords = 2;
    if(type == DS9::Circle || type == DS9::Compass || type == DS9::Composite)
        numCoords = 3;
    else if(type == DS9::Annulus || type == DS9::Line || type == DS9::Ruler ||
            type == DS9::Vector)
        numCoords = 4;
    else if(type == DS9::Ellipse || type == DS9::EllipseAnnulus ||
            type == DS9::Box || type == DS9::BoxAnnulus ||
            type == DS9::Projection) numCoords = 5;
    else if(type == DS9::Polygon) numCoords = 6;
    else if(type == DS9::Panda) numCoords = 8;
    else if(type == DS9::EllipticalPanda || type == DS9::BoxPanda)
        numCoords = 9;

    if(line.size() < numCoords) {
        invalid << "\n* Type \"" << DS9::regionType(type).toStdString();
        invalid << "\" requires at least " << numCoords << " coordinates, but";
        invalid << " only " << line.size() << " are provided.";
        return false;
    }

    stringstream ss;
    ss << "[" << DS9::FILE_HMS_H << DS9::FILE_HMS_M << DS9::FILE_HMS_S;
    ss << DS9::FILE_DMS_D << DS9::FILE_DMS_M << DS9::FILE_DMS_S << "]";
    QRegExp nmbr("["+DS9::FILE_PLUS+DS9::FILE_MINUS+"]?\\d+(\\.\\d+)?"),
            nnbr(ss.str().c_str());
    DS9::CoordinateUnit unit;
    int i1 = -1, i2 = -1, i3 = -1;
    double tmp = 0, tmp2 = 0, tmp3 = 0;
    String tmp4;
    DS9Coordinate coord(coordSys);
    QStringList nums;
    bool br;
    QString text;
    for(int i = 0; i < line.size(); i++) {
        br = false;
        if(numCoords == 0) {
            switch(type) {
            case DS9::Circle: case DS9::Ellipse: case DS9::Box: case DS9::Line:
            case DS9::Vector: case DS9::Ruler: case DS9::CirclePoint:
            case DS9::BoxPoint: case DS9::DiamondPoint: case DS9::CrossPoint:
            case DS9::XPoint: case DS9::ArrowPoint: case DS9::BoxCirclePoint:
            case DS9::Compass: case DS9::Projection: case DS9::Panda:
            case DS9::EllipticalPanda: case DS9::BoxPanda: case DS9::Composite:
                br = true; break;

            case DS9::Annulus: case DS9::EllipseAnnulus: case DS9::BoxAnnulus:
                break;

            case DS9::Text:
                // The remaining strings had better be the text
                nums = line;
                for(int j = 0; j < i; j++) nums.removeFirst();
                text = nums.join(" ");
                i1 = text.indexOf(DS9::FILE_TEXT_START1);
                i2 = text.indexOf(DS9::FILE_TEXT_START2);
                i3 = text.indexOf(DS9::FILE_TEXT_START3);

                if(i1 >= 0 && (i2 < 0 || i1 < i2) && (i3 < 0 || i1 < i3)) {
                    i2 = text.indexOf(DS9::FILE_TEXT_END1, i1);
                    if(i2 >= 0)
                        region.define(DS9Region::PROP_TEXT,
                               text.left(i2).right(i2 - i1 - 1).toStdString());
                } else if(i2 >= 0&& (i1 < 0 || i2 < i1)&&(i3 < 0 || i2 < i3)) {
                    i3 = text.indexOf(DS9::FILE_TEXT_END2, i2);
                    if(i3 >= 0)
                        region.define(DS9Region::PROP_TEXT,
                               text.left(i3).right(i3 - i2 - 1).toStdString());
                } else if(i3 >= 0&& (i1 < 0 || i3 < i1)&&(i2 < 0 || i3 < i2)) {
                    i2 = text.indexOf(DS9::FILE_TEXT_END3, i3);
                    if(i2 >= 0)
                        region.define(DS9Region::PROP_TEXT,
                               text.left(i2).right(i2 - i3 - 1).toStdString());
                }
                br = true;
                break;

            default: break;
            }
        }

        if(br) break; // don't care about other coordinates

        if(line[i].isEmpty() || (!line[i][0].isDigit() &&
           !line[i].startsWith(DS9::FILE_MINUS) &&
           !line[i].startsWith(DS9::FILE_PLUS))) {
            invalid << "\n* Given coordinate \"" << line[i].toStdString();
            invalid << "\" is not a valid number!";
            continue;
        }

        i1 = line[i].indexOf(DS9::REGEXP_HDMS);
        i2 = line[i].indexOf(DS9::REGEXP_HMS);
        i3 = line[i].indexOf(DS9::REGEXP_DMS);
        if(i1 >= 0 || i2 >= 0 || i3 >= 0) {
            if(i1 >= 0) {
                // 1-based odd is hms, so 0-based even is hms
                // ...UNLESS the system is galactic (why can't they be
                // consistent???) in which case they're both DMS
                if(m_nextSystem.second && m_nextSystem.first == DS9::Galactic)
                    unit = DS9::DMS;
                else
                    unit = i % 2 == 0 ? DS9::HMS : DS9::DMS;
                nums = line[i].split(DS9::FILE_DELIMITER.c_str());
                tmp =  nums[0].toDouble();
                tmp2 = nums[1].toDouble();
                tmp3 = nums[2].toDouble();
            } else {
                unit = i2 >= 0 ? DS9::HMS : DS9::DMS;
                nums = line[i].split(nnbr);
                tmp =  nums[0].toDouble();
                tmp2 = nums[1].toDouble();
                tmp3 = nums[2].toDouble();
            }

            coord = DS9Coordinate(coordSys, unit);
            coord.set(tmp, tmp2, tmp3, nums[0].startsWith(DS9::FILE_MINUS));
            
        } else if(line[i].indexOf(DS9::REGEXP_NUMSYS) == 0) {
            stringstream ss2(line[i].toStdString());

            ss2 >> tmp;
            ss2 >> tmp4;

            if(tmp4.empty()) unit = DS9::UNSET;
            else             unit = DS9::coordinateUnit(tmp4);
            
            coord = DS9Coordinate(coordSys, unit);
            coord.set(tmp);

        } else if(line[i].indexOf(nmbr) == 0) {
            tmp = line[i].toDouble();
            
            coord = DS9Coordinate(coordSys, DS9::UNSET);
            coord.set(tmp);

        } else {
            invalid << "\n* Given coordinate \"" << line[i].toStdString();
            invalid << "\" is not valid!";
            continue;
        }

        if(!coord.isValid()) {
            invalid << "\n* Given coordinate \"" << line[i].toStdString();
            invalid << "\" is not valid!";
            continue;
        }

        region.pushCoordinate(coord);

        if(numCoords > 0) numCoords--;
    }

    // here is where ellipses and boxes are given their proper types
    if(!region.checkCoordinates()) {
        invalid << "\n* Given coordinates are invalid for the region type \"";
        invalid << DS9::regionType(type).toStdString() << "\"!";
        return false;
    }

    bool success = true;
    if(!comment.isEmpty())
        if(!processComment(region, comment, invalid)) success = false;

    if(!region.checkProperties()) {
        invalid << "\n* Given properties are invalid for the region type \"";
        invalid << DS9::regionType(type).toStdString() << "\"!";
        success = false;
    }

    m_regions.push_back(region);

    return success;
}

bool DS9FileReader::processCoordSys(QString& line, stringstream& invld) {
    m_nextSystem.first = DS9::coordinateSystem(line);
    m_nextSystem.second = true;
    return true;
}

bool DS9FileReader::processGlobal(QString& line, stringstream& invalid) {
    int i = line.indexOf(DS9::FILE_GLOBAL) + DS9::FILE_GLOBAL.size();
    while(line[i].isSpace()) i++;
    line = line.right(line.size() - i).trimmed();

    Record record(Record::Variable);
    if(!readProperties(record, line)) {
        invalid << "\n* Invalid properties in globals!";
        return false;
    }
    for(unsigned int i = 0; i < record.nfields(); i++) {
        if(record.dataType(i) == TpBool)
            m_globals.define(record.name(i), record.asBool(i));
        else if(record.dataType(i) == TpString)
            m_globals.define(record.name(i), record.asString(i));
    }
    return true;
}

bool DS9FileReader::processComment(DS9Region& region, QString& comment,
                                         stringstream& invalid) {
    Record record(Record::Variable);
    if(!readProperties(record, comment) || !region.setProperties(record)) {
        invalid << "\n* Invalid properties in comment!";
        return false;
    } else return true;
}

bool DS9FileReader::readPointType(QString& cmt, DS9::RegionType& type) {
    int i = cmt.indexOf(DS9::FILE_POINT, 0, Qt::CaseInsensitive);
    if(i < 0 || i >= cmt.size()) return false;

    int j = cmt.indexOf(DS9::FILE_EQUAL, i + DS9::FILE_POINT.size());
    if(j < 0 || j >= cmt.size()) return false;

    // if there's anything between point and =, invalid
    for(int index = i + DS9::FILE_POINT.size(); index < j; index++)
        if(!cmt[index].isSpace()) return false;

    for(; j < cmt.size(); j++)
        if(!cmt[j].isSpace()) break;
    if(j == cmt.size()) return false; // nothing after =

    int k = cmt.indexOf(QRegExp("\\s+"), j + 1);
    if(k < 0 || k >= cmt.size()) k = cmt.size();

    QString str = cmt.right(cmt.size() - j - 1).left(k - j - 1).toLower();
    if(str == DS9::FILE_CIRCLE) type = DS9::CirclePoint;
    else if(str == DS9::FILE_BOX) type = DS9::BoxPoint;
    else if(str == DS9::FILE_DIAMOND) type = DS9::DiamondPoint;
    else if(str == DS9::FILE_CROSS) type = DS9::CrossPoint;
    else if(str == DS9::FILE_X) type = DS9::XPoint;
    else if(str == DS9::FILE_ARROW) type = DS9::ArrowPoint;
    else if(str == DS9::FILE_BOXCIRCLE) type = DS9::BoxCirclePoint;
    else return false;

    if(k < cmt.size() && i > 0)
        cmt = cmt.left(i) + " " + cmt.right(cmt.size() - k - 1);
    if(k == cmt.size()) cmt = cmt.left(i);
    else cmt = cmt.right(cmt.size() - k - 1);

    return true;
}

bool DS9FileReader::readProperties(Record& rec, QString& line) {
    QStringList list = line.split(QRegExp("\\s+"));
    int j, k;
    QString qprop, qval, tmp;
    String prop, val;
    
    // check for special compass case
    if(list.size() >= 5 && list[0].startsWith(DS9Region::PROP_COMPASS.c_str(),
                                              Qt::CaseInsensitive)) {
        bool valid = true;
        k = 0;
        // compass=<coord sys> <n label> <e label> [0|1] [0|1]
        if((j = list[0].indexOf(DS9::FILE_EQUAL))> 0 && j< list[0].size() - 1){
            // compass=<coord sys>
            k = 1;
        } else if(j == list[0].size() - 1) {
            // compass= <coord sys>
            k = 2;
            if(list.size() < 6) valid = false;
        } else if(j < 0) {
            if(list[1].startsWith(DS9::FILE_EQUAL)) {
                if(list[1].size() == DS9::FILE_EQUAL.size()) {
                    // compass = <coord sys>
                    k = 3;
                    if(list.size() < 7) valid = false;
                } else {
                    // compass =<coord sys>
                    k = 2;
                    if(list.size() < 6) valid = false;
                }
            } else valid = false;
        }
        if(valid) {
            list.insert(k, DS9Region::PROP_COMPASS_NLABEL.c_str());
            
            k++;
            if(list[k].startsWith(DS9::FILE_TEXT_START1) ||
               list[k].startsWith(DS9::FILE_TEXT_START2) ||
               list[k].startsWith(DS9::FILE_TEXT_START3)) {
                QString end = DS9::FILE_TEXT_END1;
                if(list[k].startsWith(DS9::FILE_TEXT_START2))
                    end = DS9::FILE_TEXT_END2;
                else if(list[k].startsWith(DS9::FILE_TEXT_START3))
                    end = DS9::FILE_TEXT_END3;
                while(!list[k].endsWith(end) && k < list.size()) k++;
            }
            k++;
            
            list.insert(k, DS9Region::PROP_COMPASS_ELABEL.c_str());
            
            k++;
            if(list[k].startsWith(DS9::FILE_TEXT_START1) ||
               list[k].startsWith(DS9::FILE_TEXT_START2) ||
               list[k].startsWith(DS9::FILE_TEXT_START3)) {
                QString end = DS9::FILE_TEXT_END1;
                if(list[k].startsWith(DS9::FILE_TEXT_START2))
                    end = DS9::FILE_TEXT_END2;
                else if(list[k].startsWith(DS9::FILE_TEXT_START3))
                    end = DS9::FILE_TEXT_END3;
                while(!list[k].endsWith(end) && k < list.size()) k++;
            }
            k++;
            
            list.insert(k, DS9Region::PROP_LINE.c_str());
        }
    }
    
    for(int i = 0; i < list.size(); i++) {
        if((j = list[i].indexOf(DS9::FILE_EQUAL))> 0 && j< list[i].size() - 1){
            // [property=value] case
            qprop = list[i].left(j); prop = qprop.toStdString();
            if(!DS9Region::isProperty(prop)) continue;
            
            // special ruler/line/dashlist case
            if((prop == DS9Region::PROP_RULER || prop== DS9Region::PROP_LINE ||
                prop == DS9Region::PROP_DASHLIST) && i < list.size() - 1) {
                list[i] += " " + list[i + 1];
                list.removeAt(i + 1);
            }
            
            qval = list[i].right(list[i].size() - j - 1);

            if(DS9Region::isBoolProperty(prop)) {
                if(qval != "0" && qval != "1") continue;
                rec.define(prop, qval == "1");
            } else {
                if(qval.startsWith(DS9::FILE_TEXT_START1) ||
                   qval.startsWith(DS9::FILE_TEXT_START2) ||
                   qval.startsWith(DS9::FILE_TEXT_START3)) {
                    if(qval.startsWith(DS9::FILE_TEXT_START1)) {
                        tmp = DS9::FILE_TEXT_END1;
                        k = DS9::FILE_TEXT_START1.size();
                    } else if(qval.startsWith(DS9::FILE_TEXT_START2)) {
                        tmp = DS9::FILE_TEXT_END2;
                        k = DS9::FILE_TEXT_START2.size();
                    } else {
                        tmp = DS9::FILE_TEXT_END3;
                        k = DS9::FILE_TEXT_START3.size();
                    }
                    k += prop.size() + DS9::FILE_EQUAL.size();

                    for(j = i; j < list.size(); j++) {
                        if(list[j].indexOf(tmp, i == j ? k : 0) >= 0) break;
                    }
                    k -= prop.size() + DS9::FILE_EQUAL.size();
                    // j is now the index of the word containing the ending }"'
                    if(j == list.size()) continue;
                    else if(i == j) {
                        qval.remove(0, k);
                        qval.remove(qval.size() - tmp.size(), tmp.size());
                        val = qval.toStdString();
                        rec.define(prop, val);
                    } else {
                        qval.remove(0, k);
                        // all but last
                        for(i++; i < j; i++) qval += " " + list[i];
                        if(list[i].endsWith(tmp)) {
                            qval += " " + list[i];
                            qval.remove(qval.size() - tmp.size(), tmp.size());
                            val = qval.toStdString();
                            rec.define(prop, val);
                        } else {
                            qval += " " + list[i].left(list[i].indexOf(tmp));
                            list[i] = list[i].right(list[i].size() -
                                    (list[i].indexOf(tmp) + tmp.size()));
                            i--;
                            val = qval.toStdString();
                            rec.define(prop, val);
                        }
                    }

                } else {
                    val = qval.toStdString();
                    rec.define(prop, val);
                }
            }
        } else if(j == list[i].size() - 1) {
            // [property=] [value] case
            if(i == list.size() - 1) continue;
            // sort of cheating/inefficient..
            list[i] += list[i + 1];
            list.removeAt(i + 1);
            i--;
        } else if(j < 0) {
            qprop = list[i]; prop = qprop.toStdString();
            if(!DS9Region::isProperty(prop)) {
                // could be [marker size] case
                bool valid;
                int size = qprop.toInt(&valid);
                if(!valid) continue;
                else rec.define(DS9Region::PROP_MARKER_SIZE,
                                String::toString(size));
            } else {
                if(i == list.size() - 1 || list[i + 1] != DS9::FILE_EQUAL) {
                    // [property] case
                    if(!DS9Region::isBoolProperty(prop)) continue;
                    rec.define(prop, true);
                } else {
                    // [property] [=] case
                    if(i == list.size() - 2) continue;
                    // sort of cheating/inefficient..
                    list[i] += list[i + 1] + list[i + 2];
                    list.removeAt(i + 1);
                    list.removeAt(i + 1);
                    i--;
                }
            }
        }
    }

    return true;
}


// Static Members //

const String DS9::FILE_ARCMIN =          "'";
const String DS9::FILE_ARCSEC =          "\"";
const String DS9::FILE_DEGREES =         "d";
const String DS9::FILE_DELIMITER =       ":";
const String DS9::FILE_DMS_D =           "d";
const String DS9::FILE_DMS_M =           "m";
const String DS9::FILE_DMS_S =           "s";
const String DS9::FILE_HMS_H =           "h";
const String DS9::FILE_HMS_M =           "m";
const String DS9::FILE_HMS_S =           "s";
const String DS9::FILE_IMAGE_PIXELS =    "i";
const String DS9::FILE_PHYSICAL_PIXELS = "p";
const String DS9::FILE_RADIANS =         "r";

const int DS9::MARKER_SIZE = 11;
const int DS9::ARROW_SIZE = 7;

const QString DS9::FILE_ANNULUS =    "annulus";
const QString DS9::FILE_ARROW =      "arrow";
const QString DS9::FILE_BOX =        "box";
const QString DS9::FILE_BOXCIRCLE =  "boxcircle";
const QString DS9::FILE_BPANDA =     "bpanda";
const QString DS9::FILE_CIRCLE =     "circle";
const QString DS9::FILE_COMPASS =    "compass";
const QString DS9::FILE_COMPOSITE =  "composite";
const QString DS9::FILE_CROSS =      "cross";
const QString DS9::FILE_DIAMOND =    "diamond";
const QString DS9::FILE_ELLIPSE =    "ellipse";
const QString DS9::FILE_EPANDA =     "epanda";
const QString DS9::FILE_LINE =       "line";
const QString DS9::FILE_PANDA =      "panda";
const QString DS9::FILE_POINT =      "point";
const QString DS9::FILE_POLYGON =    "polygon";
const QString DS9::FILE_PROJECTION = "projection";
const QString DS9::FILE_RULER =      "ruler";
const QString DS9::FILE_TEXT =       "text";
const QString DS9::FILE_VECTOR =     "vector";
const QString DS9::FILE_X =          "x";

const QString DS9::FILE_AMPLIFIER = "amplifier";
const QString DS9::FILE_B1950 =     "b1950";
const QString DS9::FILE_DETECTOR =  "detector";
const QString DS9::FILE_ECLIPTIC =  "ecliptic";
const QString DS9::FILE_FK4 =       "fk4";
const QString DS9::FILE_FK5 =       "fk5";
const QString DS9::FILE_GALACTIC =  "galactic";
const QString DS9::FILE_ICRS =      "icrs";
const QString DS9::FILE_IMAGE =     "image";
const QString DS9::FILE_J2000 =     "j2000";
const QString DS9::FILE_LINEAR =    "linear";
const QString DS9::FILE_PHYSICAL =  "physical";

const QString DS9::FILE_COMMENT =      "#";
const QString DS9::FILE_COMPOSITE_OR = "||";
const QString DS9::FILE_EQUAL =        "=";
const QString DS9::FILE_GLOBAL =       "global";
const QString DS9::FILE_LINESEP =      ";";
const QString DS9::FILE_MINUS =        "-";
const QString DS9::FILE_PLUS =         "+";
const QString DS9::FILE_TEXT_END1 =    "}";
const QString DS9::FILE_TEXT_END2 =    "\"";
const QString DS9::FILE_TEXT_END3 =    "'";
const QString DS9::FILE_TEXT_START1 =  "{";
const QString DS9::FILE_TEXT_START2 =  "\"";
const QString DS9::FILE_TEXT_START3 =  "'";

const QRegExp DS9::REGEXP_HDMS("[" + DS9::FILE_MINUS + DS9::FILE_PLUS +
                        "]?\\d+(\\.\\d+)?" + DS9::FILE_DELIMITER.c_str() +
                        "\\d+(\\.\\d+)?" + DS9::FILE_DELIMITER.c_str() +
                        "\\d+(\\.\\d+)?");
const QRegExp DS9::REGEXP_HMS("[" + DS9::FILE_MINUS + DS9::FILE_PLUS +
                       "]?\\d+(\\.\\d+)?" + DS9::FILE_HMS_H.c_str() +
                       "\\d+(\\.\\d+)?" + DS9::FILE_HMS_M.c_str() +
                       "\\d+(\\.\\d+)?" + DS9::FILE_HMS_S.c_str());
const QRegExp DS9::REGEXP_DMS("[" + DS9::FILE_MINUS + DS9::FILE_PLUS +
                       "]?\\d+(\\.\\d+)?" + DS9::FILE_DMS_D.c_str() +
                       "\\d+(\\.\\d+)?" + DS9::FILE_DMS_M.c_str() +
                       "\\d+(\\.\\d+)?" + DS9::FILE_DMS_S.c_str());
const QRegExp DS9::REGEXP_NUMSYS("[" + DS9::FILE_PLUS + DS9::FILE_MINUS +
                                 "]?\\d+(\\.\\d+)?\\D+");

const String DS9Region::PROP_BACKGROUND =     "background";
const String DS9Region::PROP_COLOR =          "color";
const String DS9Region::PROP_COMPASS =        "compass";
const String DS9Region::PROP_COMPASS_ELABEL = "elabel";
const String DS9Region::PROP_COMPASS_NLABEL = "nlabel";
const String DS9Region::PROP_COMPOSITE =      "composite";
const String DS9Region::PROP_DASH =           "dash";
const String DS9Region::PROP_DASHLIST =       "dashlist";
const String DS9Region::PROP_DELETE =         "delete";
const String DS9Region::PROP_EDIT =           "edit";
const String DS9Region::PROP_FIXED =          "fixed";
const String DS9Region::PROP_HIGHLITE =       "highlite";
const String DS9Region::PROP_FONT =           "font";
const String DS9Region::PROP_INCLUDE =        "include";
const String DS9Region::PROP_LINE =           "line";
const String DS9Region::PROP_MARKER_SIZE =    "size";
const String DS9Region::PROP_MOVE =           "move";
const String DS9Region::PROP_ROTATE =         "rotate";
const String DS9Region::PROP_RULER =          "ruler";
const String DS9Region::PROP_SELECT =         "select";
const String DS9Region::PROP_SOURCE =         "source";
const String DS9Region::PROP_TAG =            "tag";
const String DS9Region::PROP_TEXT =           "text";
const String DS9Region::PROP_TEXTANGLE =      "textangle";
const String DS9Region::PROP_VECTOR =         "vector";
const String DS9Region::PROP_WIDTH =          "width";

}
