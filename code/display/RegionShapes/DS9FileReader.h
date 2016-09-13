//# DS9FileReader.h: Implementation for DS9 region file reader etc.
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
#ifndef DS9FILEREADER_H_
#define DS9FILEREADER_H_

#include <QStringList>

#include <display/RegionShapes/RSFileReaderWriter.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <display/DisplayDatas/DrawingDisplayData.h>

#include <casa/namespace.h>

namespace casa {

// Contains common enums, constants, and methods for DS9 files.
	class DS9 {
	public:
		// Units.
		// <group>
		enum CoordinateUnit {
		    Degrees, Radians, PhysicalPixels, ImagePixels, HMS, DMS, ArcSec,
		    ArcMin, UNSET
		};

		static const String FILE_ARCMIN;
		static const String FILE_ARCSEC;
		static const String FILE_DEGREES;
		static const String FILE_DELIMITER;
		static const String FILE_DMS_D;
		static const String FILE_DMS_M;
		static const String FILE_DMS_S;
		static const String FILE_HMS_H;
		static const String FILE_HMS_M;
		static const String FILE_HMS_S;
		static const String FILE_IMAGE_PIXELS;
		static const String FILE_PHYSICAL_PIXELS;
		static const String FILE_RADIANS;

		static CoordinateUnit coordinateUnit(const String& unit) {
			if(unit == FILE_ARCMIN) return ArcMin;
			else if(unit == FILE_ARCSEC) return ArcSec;
			else if(unit == FILE_DEGREES) return Degrees;
			else if(unit == FILE_IMAGE_PIXELS) return ImagePixels;
			else if(unit == FILE_PHYSICAL_PIXELS) return PhysicalPixels;
			else if(unit == FILE_RADIANS) return Radians;

			else return UNSET;
		}

		static const QRegExp REGEXP_HDMS;
		static const QRegExp REGEXP_HMS;
		static const QRegExp REGEXP_DMS;
		static const QRegExp REGEXP_NUMSYS;
		// </group>


		// Coordinate systems.
		// <group>
		enum CoordinateSystem {
		    Physical, Image, FK4, FK5, Galactic, Ecliptic, Linear, Amplifier,
		    Detector
		};

		static CoordinateSystem defaultCoordinateSystem() {
			return Physical;
		}

		static const QString FILE_AMPLIFIER;
		static const QString FILE_B1950;
		static const QString FILE_DETECTOR;
		static const QString FILE_ECLIPTIC;
		static const QString FILE_FK4;
		static const QString FILE_FK5;
		static const QString FILE_GALACTIC;
		static const QString FILE_ICRS;
		static const QString FILE_IMAGE;
		static const QString FILE_J2000;
		static const QString FILE_LINEAR;
		static const QString FILE_PHYSICAL;

		static QStringList coordinateSystemFirstWords() {
			QStringList list;
			list << FILE_AMPLIFIER << FILE_B1950 << FILE_DETECTOR;
			list << FILE_ECLIPTIC << FILE_FK4 << FILE_FK5 << FILE_GALACTIC;
			list << FILE_ICRS << FILE_IMAGE << FILE_J2000 << FILE_LINEAR;
			list << FILE_PHYSICAL;
			return list;
		}

		static CoordinateSystem coordinateSystem(const QString& cs) {
			QString c = cs.toLower();
			if(c == FILE_AMPLIFIER) return Amplifier;
			else if(c == FILE_B1950 || c == FILE_FK4) return FK4;
			else if(c == FILE_DETECTOR) return Detector;
			else if(c == FILE_ECLIPTIC) return Ecliptic;
			else if(c == FILE_FK5 || c == FILE_J2000 || c == FILE_ICRS) return FK5;
			else if(c == FILE_GALACTIC) return Galactic;
			else if(c == FILE_IMAGE) return Image;
			else if(c == FILE_LINEAR) return Linear;
			else if(c == FILE_PHYSICAL) return Physical;

			else return defaultCoordinateSystem();
		}

		static QString coordinateSystem(CoordinateSystem c) {
			switch(c) {
			case Physical:
				return FILE_PHYSICAL;
			case Image:
				return FILE_IMAGE;
			case FK4:
				return FILE_FK4;
			case FK5:
				return FILE_FK5;
			case Galactic:
				return FILE_GALACTIC;
			case Ecliptic:
				return FILE_ECLIPTIC;
			case Linear:
				return FILE_LINEAR;
			case Amplifier:
				return FILE_AMPLIFIER;
			case Detector:
				return FILE_DETECTOR;

			default:
				return "";
			}
		}
		// </group>


		// Regions.
		// <group>
		enum RegionType {
		    Circle, Annulus, Ellipse, EllipseAnnulus, Box, BoxAnnulus, Polygon,
		    Line, Vector, Text, Ruler, CirclePoint, BoxPoint, DiamondPoint,
		    CrossPoint, XPoint, ArrowPoint, BoxCirclePoint, Compass, Projection,
		    Panda, EllipticalPanda, BoxPanda, Composite
		};

		// region types, first word
		static const QString FILE_ANNULUS;
		static const QString FILE_ARROW;     // only for "arrow point"
		static const QString FILE_BOX;
		static const QString FILE_BOXCIRCLE; // only for "boxcircle point"
		static const QString FILE_BPANDA;
		static const QString FILE_CIRCLE;
		static const QString FILE_COMPASS;
		static const QString FILE_COMPOSITE;
		static const QString FILE_CROSS;     // only for "cross point"
		static const QString FILE_DIAMOND;   // only for "diamond point"
		static const QString FILE_ELLIPSE;
		static const QString FILE_EPANDA;
		static const QString FILE_LINE;
		static const QString FILE_PANDA;
		static const QString FILE_POINT;
		static const QString FILE_POLYGON;
		static const QString FILE_PROJECTION;
		static const QString FILE_RULER;
		static const QString FILE_TEXT;
		static const QString FILE_VECTOR;
		static const QString FILE_X;         // only for "x point"

		static QStringList regionFirstWords() {
			QStringList list;
			list << FILE_ANNULUS << FILE_ARROW << FILE_BOX << FILE_BOXCIRCLE;
			list << FILE_BPANDA << FILE_CIRCLE << FILE_COMPASS << FILE_COMPOSITE;
			list << FILE_CROSS << FILE_DIAMOND << FILE_ELLIPSE << FILE_EPANDA;
			list << FILE_LINE << FILE_PANDA << FILE_POINT << FILE_POLYGON;
			list << FILE_PROJECTION << FILE_RULER << FILE_TEXT << FILE_VECTOR;
			list << FILE_X;
			return list;
		}

		static QString regionType(RegionType type) {
			switch(type) {
			case Circle:
				return FILE_CIRCLE;
			case Annulus:
				return FILE_ANNULUS;
			case Ellipse:
			case EllipseAnnulus:
				return FILE_ELLIPSE;
			case Box:
			case BoxAnnulus:
				return FILE_BOX;
			case Polygon:
				return FILE_POLYGON;
			case Line:
				return FILE_LINE;
			case Vector:
				return FILE_VECTOR;
			case Text:
				return FILE_TEXT;
			case Ruler:
				return FILE_RULER;
			case CirclePoint:
				return FILE_CIRCLE + " " + FILE_POINT;
			case BoxPoint:
				return FILE_BOX + " " + FILE_POINT;
			case DiamondPoint:
				return FILE_DIAMOND + " " + FILE_POINT;
			case CrossPoint:
				return FILE_CROSS + " " + FILE_POINT;
			case XPoint:
				return FILE_X + " " + FILE_POINT;
			case ArrowPoint:
				return FILE_ARROW + " " + FILE_POINT;
			case BoxCirclePoint:
				return FILE_BOXCIRCLE+ " " +FILE_POINT;
			case Compass:
				return FILE_COMPASS;
			case Projection:
				return FILE_PROJECTION;
			case Panda:
				return FILE_PANDA;
			case EllipticalPanda:
				return FILE_EPANDA;
			case BoxPanda:
				return FILE_BPANDA;
			case Composite:
				return FILE_COMPOSITE;

			default:
				return "";
			}
		}

		static QString pointType(RegionType type) {
			switch(type) {
			case CirclePoint:
				return FILE_CIRCLE;
			case BoxPoint:
				return FILE_BOX;
			case DiamondPoint:
				return FILE_DIAMOND;
			case CrossPoint:
				return FILE_CROSS;
			case XPoint:
				return FILE_X;
			case ArrowPoint:
				return FILE_ARROW;
			case BoxCirclePoint:
				return FILE_BOXCIRCLE;

			default:
				return "";
			}
		}
		// </group>

		// DS9 defaults.
		// <group>
		static const int MARKER_SIZE;
		static const int ARROW_SIZE;
		// </group>

		// Miscellaneous.
		// <group>
		static const QString FILE_COMMENT;
		static const QString FILE_COMPOSITE_OR;
		static const QString FILE_EQUAL;
		static const QString FILE_GLOBAL;
		static const QString FILE_LINESEP;
		static const QString FILE_MINUS;
		static const QString FILE_PLUS;
		static const QString FILE_TEXT_END1;
		static const QString FILE_TEXT_END2;
		static const QString FILE_TEXT_END3;
		static const QString FILE_TEXT_START1;
		static const QString FILE_TEXT_START2;
		static const QString FILE_TEXT_START3;
		// </group>
	};


// Class to represent a single coordinate (value + unit).
	class DS9Coordinate {
	public:
		// Constructor which takes a system and a unit.
		DS9Coordinate(DS9::CoordinateSystem s, DS9::CoordinateUnit u = DS9::UNSET);

		// Destructor.
		~DS9Coordinate();


		// Sets the coordinate system to the given.
		void set(DS9::CoordinateSystem system);

		// Sets the coordinate unit to the given.
		void set(DS9::CoordinateUnit unit);

		// Sets the single value to the given.
		void set(double value);

		// Sets the HMS/DMS value to the given.  minusZero is used when the first
		// value is zero, but the whole value is negative.
		void set(double val1, double val2, double val3, bool minusZero = false);

		// Returns the coordinate system.
		DS9::CoordinateSystem system() const;

		// Returns the unit.
		DS9::CoordinateUnit unit() const;

		// Returns the first value.  No conversion is used.
		double value() const;

		// Returns the value(s).  Size will be 1 for normal coordinates, 3 for
		// HMS/DMS.  No conversion is used.
		vector<double> values() const;

		// Returns 1 for normal coordinates or 3 for HMS/DMS.
		unsigned int size() const;

		// Returns true if this coordinate is valid, false otherwise.  If the
		// coordinate is invalid (such as an unset unit), the method will attempt
		// to fix it.
		bool isValid() const;

		// Returns a String version for use with DS9Region::toPrintString.
		String toPrintString() const;

		// Returns the value in degrees.
		double toDegrees() const;

	private:
		DS9::CoordinateSystem m_system; // coordinate system
		DS9::CoordinateUnit m_unit;     // coordinate unit
		vector<double> m_values;        // values
		bool m_minusZero;               // minusZero flag for HMS/DMS
	};


// Holds information for read DS9 regions.  A DS9Region consists of a type,
// a list of coordinates, a coordinate system, and a set of properties.
// Composite regions also have a list of children regions.  For simplicity,
// properties are either bool properties or String properties.  All properties
// have defaults which are set during construction.
	class DS9Region { /*: public RFRegion*/
		//friend class DS9RegionFileReader;

	public:
		// Public Static Members/Methods //

		// Properties.
		// <group>
		static const String PROP_BACKGROUND;
		static const String PROP_COLOR;
		static const String PROP_COMPASS;
		static const String PROP_COMPASS_ELABEL;
		static const String PROP_COMPASS_NLABEL;
		static const String PROP_COMPOSITE;
		static const String PROP_DASH;
		static const String PROP_DASHLIST;
		static const String PROP_DELETE;
		static const String PROP_EDIT;
		static const String PROP_FIXED;
		static const String PROP_FONT;
		static const String PROP_HIGHLITE;
		static const String PROP_INCLUDE;
		static const String PROP_LINE;
		static const String PROP_MARKER_SIZE;
		static const String PROP_MOVE;
		static const String PROP_ROTATE;
		static const String PROP_RULER;
		static const String PROP_SELECT;
		static const String PROP_SOURCE;
		static const String PROP_TAG;
		static const String PROP_TEXT;
		static const String PROP_TEXTANGLE;
		static const String PROP_VECTOR;
		static const String PROP_WIDTH;
		// </group>

		// Note: when adding new properties:
		// 1) add to properties(), DS9RegionFileWriter::globalProperties() (if
		//    necessary)
		// 2) add to isBoolProperty()
		// 3) for strings, add to valueIsValid
		// 4) add to defaultBoolValue() or defaultStringValue()
		// 5) if special printing is required, edit toPrintString,
		//    DS9RegionFileWriter::writeGlobals()
		// 6) if special input is required, edit
		//    DS9RegionFileReader::readProperties()

		// Returns all valid properties.
		static vector<String> properties() {
			static vector<String> v(26);
			v[0] = PROP_INCLUDE;
			v[1] = PROP_TEXT;
			v[2] = PROP_COLOR;
			v[3] = PROP_FONT;
			v[4] = PROP_SELECT;
			v[5] = PROP_EDIT;
			v[6] = PROP_MOVE;
			v[7] = PROP_ROTATE;
			v[8] = PROP_DELETE;
			v[9] = PROP_FIXED;
			v[10] = PROP_LINE;
			v[11] = PROP_RULER;
			v[12] = PROP_SOURCE;
			v[13] = PROP_BACKGROUND;
			v[14] = PROP_TEXTANGLE;
			v[15] = PROP_WIDTH;
			v[16] = PROP_MARKER_SIZE;
			v[17] = PROP_HIGHLITE;
			v[18] = PROP_TAG;
			v[19] = PROP_VECTOR;
			v[20] = PROP_COMPASS;
			v[21] = PROP_COMPASS_NLABEL;
			v[22] = PROP_COMPASS_ELABEL;
			v[23] = PROP_COMPOSITE;
			v[24] = PROP_DASH;
			v[25] = PROP_DASHLIST;
			return v;
		}

		// Returns true if the given String is a valid property, false otherwise.
		static bool isProperty(const String& prp) {
			static vector<String> v = properties();
			for(unsigned int i = 0; i < v.size(); i++) if(v[i] == prp) return true;
			return false;
		}

		// Returns true if the given String is a valid bool property, false
		// otherwise.
		static bool isBoolProperty(const String& property) {
			if(!isProperty(property)) return false;
			else return property != PROP_TEXT && property != PROP_COLOR &&
				            property != PROP_FONT && property != PROP_LINE &&
				            property != PROP_RULER && property != PROP_TEXTANGLE &&
				            property != PROP_WIDTH && property != PROP_MARKER_SIZE &&
				            property != PROP_TAG && property != PROP_COMPASS &&
				            property != PROP_COMPASS_NLABEL &&
				            property != PROP_COMPASS_ELABEL &&
				            property != PROP_DASHLIST;
		}

		// Returns true if the given value is valid for the given String property
		// and region type, false otherwise.
		static bool valueIsValid(const String& property, const String& value,
		                         DS9::RegionType type) {
			if(property == PROP_COLOR) {
				if(value == "white"   || value == "black" || value == "red" ||
				        value == "green"   || value == "blue"  || value == "cyan" ||
				        value == "magenta" || value == "yellow" || value == "gray" ||
				        value == "grey") return true;
				QString v(value.c_str());
				return (v.size() == 7 &&
				        v.indexOf(QRegExp("#(?:\\d|[A-F]|[a-f]){6}")) == 0) ||
				       (v.size() == 6 &&
				        v.indexOf(QRegExp("(?:\\d|[A-F]|[a-f]){6}")) == 0);
			} else if(property == PROP_FONT) {
				QStringList split = QString(value.c_str()).split(QRegExp("\\s+"));
				if(split.size() < 3) return false;
				bool valid;
				split[1].toInt(&valid);
				if(!valid) return false;
				return split[2] == "bold" || split[2] == "normal" ||
				       split[2] == "italic" || split[2] == "italics";
			} else if(property == PROP_LINE) {
				QStringList split = QString(value.c_str()).split(QRegExp("\\s+"));
				if(split.size() < 2) return false;
				return (split[0] == "0" || split[0] == "1") &&
				       (split[1] == "0" || split[1] == "1");
			} else if(property == PROP_DASHLIST) {
				QStringList split = QString(value.c_str()).split(QRegExp("\\s+"));
				if(split.size() < 2) return false;
				bool valid;
				split[0].toUInt(&valid);
				if(!valid) return false;
				split[1].toUInt(&valid);
				return valid;
			} else if(property == PROP_RULER) {
				QStringList split = QString(value.c_str()).split(QRegExp("\\s+"));
				if(split.size() < 2) return false;
				split[0] = split[0].toLower();
				split[1] = split[1].toLower();
				return DS9::coordinateSystemFirstWords().contains(split[0]) &&
				       (split[1] == "image"   || split[1] == "physical" ||
				        split[1] == "degrees" || split[1] == "arcmin" ||
				        split[1] == "arcsec");
			} else if(property == PROP_TEXTANGLE) {
				bool valid;
				QString(value.c_str()).toDouble(&valid);
				return type == DS9::Text && valid;
			} else if(property == PROP_WIDTH) {
				bool valid;
				QString(value.c_str()).toDouble(&valid);
				return valid;
			} else if(property == PROP_MARKER_SIZE) {
				bool valid;
				QString(value.c_str()).toInt(&valid);
				return valid;
			} else if(property == PROP_COMPASS) {
				return DS9::coordinateSystemFirstWords().contains(value.c_str(),
				        Qt::CaseInsensitive);
			} else return !isBoolProperty(property);
		}

		// Returns the default value for the given bool property.
		static bool defaultBoolValue(const String& property) {
			if(property == PROP_INCLUDE)         return true;
			else if(property == PROP_SELECT)     return true;
			else if(property == PROP_EDIT)       return true;
			else if(property == PROP_MOVE)       return true;
			else if(property == PROP_ROTATE)     return true;
			else if(property == PROP_DELETE)     return true;
			else if(property == PROP_FIXED)      return false;
			else if(property == PROP_SOURCE)     return true;
			else if(property == PROP_BACKGROUND) return false;
			else if(property == PROP_HIGHLITE)   return true;
			else if(property == PROP_VECTOR)     return true;
			else if(property == PROP_COMPOSITE)  return false;
			else if(property == PROP_DASH)       return false;
			else return false;
		}

		// Returns the default value for the given String property.
		static String defaultStringValue(const String& property) {
			if(property == PROP_TEXT)                return "";
			else if(property == PROP_COLOR)          return "green";
			else if(property == PROP_FONT)           return "helvetica 10 normal";
			else if(property == PROP_LINE)           return "0 0";
			else if(property == PROP_RULER)          return "pixels";
			else if(property == PROP_TEXTANGLE)      return "0";
			else if(property == PROP_WIDTH)          return "1";
			else if(property == PROP_MARKER_SIZE)
				return String::toString(DS9::MARKER_SIZE);
			else if(property == PROP_TAG)            return "";
			else if(property == PROP_COMPASS)        return "image";
			else if(property == PROP_COMPASS_NLABEL) return "N";
			else if(property == PROP_COMPASS_ELABEL) return "E";
			else if(property == PROP_DASHLIST)       return "8 3";
			else return "";
		}


		// Non-Static //

		// Constructor, which types a type and a coordinate system.
		DS9Region(DS9::RegionType type, DS9::CoordinateSystem coordSys);

		// Destructor.
		~DS9Region();


		// Returns a human-readable representation of this region.
		String toPrintString() const;

		// Returns a name.
		String name() const;

		// Converts this region to a RegionShape.  Not all DS9 region types are
		// supported and thus this may return NULL.  For unsupported regions, see
		// cookbook documentation.
		RegionShape* toRegionShape() const;


		// Returns the region type.
		DS9::RegionType type() const;

		// Sets/adds the given properties to this region's.  Returns false if an
		// error occured.
		bool setProperties(const RecordInterface& properties);

		// Defines the given bool property with the given value.  Returns false if
		// the given property is invalid.
		bool define(const String& property, bool value);

		// Defines the given String property with the given value.  Returns false
		// if the given property or value is invalid.
		bool define(const String& property, const String& value);

		// Returns true if the given property is defined, false otherwise.
		bool isDefined(const String& property);

		// Returns the value for the given bool property.
		bool boolValue(const String& property);

		// Returns the String value for the given String property.
		String stringValue(const String& property);

		// Adds the given coordinate to the region.
		void pushCoordinate(const DS9Coordinate& coord);

		// Adds the given region to this composite region (does nothing for non-
		// composite regions).
		void pushCompositeRegion(const DS9Region& region);

		// Returns true if this region's coordinates are valid, false otherwise.
		// If the coordinates are invalid, this method will attempt to fix them
		// first.
		bool checkCoordinates();

		// Returns true if this region's properties are valid, false otherwise.
		// If the properties are invalid, this method will attempt to fix them
		// first.
		bool checkProperties();

		// Returns the last error found during toRegionShape().  This error is only
		// valid if toRegionShape() returns NULL which indicates an error was
		// encountered.
		const RFError& lastError() const;

	private:
		DS9::RegionType m_type;               // Region type.
		DS9::CoordinateSystem m_system;       // Region coordinate system.
		vector<DS9Coordinate> m_coords;       // Region coordinates.
		Record m_props;                       // Region properties.
		vector<DS9Region> m_compositeRegions; // Composite children.

		// Last encountered error during toRegionShape.
		RFError m_lastError;

		// Convenience method for setting the last error during toRegionShape.
		void setError(const String& error, bool isFatal = false) const;
	};


// Implementation of RSFileReader for DS9 regions.
	class DS9FileReader : public RSFileReader {
	public:
		// Constructor.
		DS9FileReader();

		// Destructor.
		~DS9FileReader();


		// RSFileReader methods //

		// Implements RSFileReader::read.
		bool read(vector<RegionShape*>& shapes);

	private:
		// Whether a coordinate system has been set in the file, and what it is.
		pair<DS9::CoordinateSystem, bool> m_nextSystem;

		// Read regions.
		vector<DS9Region> m_regions;

		// Current read global properties.
		Record m_globals;


		// Processes the given line and returns whether an error occurred or not.
		// If an error occurred, the details are appended to invalid.
		bool processLine(const QString& line, stringstream& invalid);

		// Processes the given region and returns whether an error occurred or not.
		// If an error occurred, the details are appended to invalid.  line should
		// contain the comma-, parentheses-, or whitespace-separated text before
		// the comment symbol, while "comment" should contain all text after the
		// comment symbol.  The include flag is used to set the include property.
		bool processRegion(QStringList& line, QString& comment,
		                   stringstream& invalid, bool include = true);

		// Processes the given coordinate system and returns whether an error
		// occurred or not.  If an error occurred, the details are appended to
		// invalid.
		bool processCoordSys(QString& line, stringstream& invalid);

		// Processes the given global line and returns whether an error occurred or
		// not.  If an error occurred, the details are appended to invalid.
		bool processGlobal(QString& line, stringstream& invalid);

		// Processes the given comment into properties for the given region, and
		// returns whether an error occurred or not.  If an error occurred, the
		// details are appended to invalid.
		bool processComment(DS9Region& region, QString& comment,
		                    stringstream& invalid);

		// For "point" regions.  Parses the given comment for a point=type property
		// and places the correct type into "type".  Returns whether the operation
		// succeeded or not.
		bool readPointType(QString& comment, DS9::RegionType& type);

		// Reads properties in the given line into the given record.
		bool readProperties(Record& record, QString& line);
	};

}

#endif /*DS9FILEREADER_H_*/
