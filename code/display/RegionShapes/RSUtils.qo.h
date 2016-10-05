//# RSUtils.qo.h: Common utilities/constants for region shapes.
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
#ifndef RSUTILS_QO_H_
#define RSUTILS_QO_H_

#include <QtGui>

#include <casa/BasicSL/String.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/WorldCanvas.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MDirection.h>

#include <casa/namespace.h>

namespace casa {

	class QtColorWidget;

// Common utilities/constants for region shapes.
	class RSUtils {
	public:
		// Returns a new layout with the given parent, and calls setupLayout
		// on it;
		// <group>
		static QHBoxLayout* hlayout(QWidget* parent = NULL);
		static QVBoxLayout* vlayout(QWidget* parent = NULL);
		// </group>

		// Sets up the given layout by setting:
		// 1) margins to 0,
		// 2) spacing to 3.
		static void setupLayout(QLayout* layout);

		// Returns a horizontal or vertical line (QFrame).
		// <group>
		static QWidget* hline();
		static QWidget* vline();
		// </group>

		// Returns a new color widget.  If margins and spacing are given, they are
		// applied to the widget.  If a frame is given, the widget is inserted into
		// the frame.
		static QtColorWidget* colorWidget(bool showButton = false,
		                                  casacore::String setColor = "",
		                                  QWidget* parent = NULL);


		// Pixel coordinate system.  Used for converting between different
		// systems, etc.
		static const casacore::String PIXEL;


		// Returns whether or not the given coordinate system has a direction
		// coordinate or not.
		// <group>
		static bool hasDirectionCoordinate(const DisplayCoordinateSystem& cs);
		static bool hasDirectionCoordinate(const WorldCanvas* wc) {
			return wc != NULL && hasDirectionCoordinate(wc->coordinateSystem());
		}
		static bool hasDirectionCoordinate(const WorldCanvasHolder& wch) {
			return hasDirectionCoordinate(wch.worldCanvas()->coordinateSystem());
		}
		// </group>

		// Returns the world system type for the given coordinate system.
		static casacore::MDirection::Types worldSystem(const DisplayCoordinateSystem& cs);

		// Returns the world system type for the given WorldCanvas.
		static casacore::MDirection::Types worldSystem(const WorldCanvas* wc) {
			return worldSystem(wc->coordinateSystem());
		}

		// Returns the world system type for the given WorldCanvasHolder.
		static casacore::MDirection::Types worldSystem(const WorldCanvasHolder& wch) {
			return worldSystem(wch.worldCanvas()->coordinateSystem());
		}

		// Converts between coordinate systems.  The result has unit
		// RegionShape::UNIT.
		static bool convertWCS(const casacore::Quantum<casacore::Vector<double> >& from,
		                       casacore::MDirection::Types fromSys,
		                       casacore::Quantum<casacore::Vector<double> >& to,
		                       casacore::MDirection::Types toSys);

		// Converts the given coordinates between different systems, using the
		// given WorldCanvas and world system (if applicable).  casacore::Input world values
		// are expected to have unit RegionShape::UNIT; output world values always
		// have unit RegionShape::UNIT.  Methods that have a "wrap" flag will check
		// the sign of the values when converting between different world
		// coordinate systems, and wrap the values to have the same sign.  If
		// a casacore::String is given, it will be used to record errors when the method
		// returns false.
		// <group>
		static bool worldToPixel(const casacore::Quantum<casacore::Vector<double> >& worldX,
		                         const casacore::Quantum<casacore::Vector<double> >& worldY,
		                         casacore::Vector<double>& pixelX, casacore::Vector<double>& pixelY,
		                         WorldCanvasHolder& wch, casacore::MDirection::Types fromSys,
		                         bool wrap = true, casacore::String* error = NULL);

		static bool pixelToWorld(const casacore::Vector<double>& pixelX,
		                         const casacore::Vector<double>& pixelY,
		                         casacore::Quantum<casacore::Vector<double> >& worldX,
		                         casacore::Quantum<casacore::Vector<double> >& worldY,
		                         WorldCanvasHolder& wch, casacore::MDirection::Types toSys,
		                         bool wrap = true, casacore::String* error = NULL);

		static bool worldToLinear(const casacore::Quantum<casacore::Vector<double> >& worldX,
		                          const casacore::Quantum<casacore::Vector<double> >& worldY,
		                          casacore::Vector<double>& linearX, casacore::Vector<double>& linearY,
		                          WorldCanvasHolder& wch, casacore::MDirection::Types fromSys,
		                          bool wrap = true, casacore::String* error = NULL);

		static bool pixelToLinear(const casacore::Vector<double>& pixelX,
		                          const casacore::Vector<double>& pixelY,
		                          casacore::Vector<double>& linearX, casacore::Vector<double>& linearY,
		                          WorldCanvasHolder& wch, casacore::String* error = NULL);

		static bool linearToScreen(const casacore::Vector<double>& linearX,
		                           const casacore::Vector<double>& linearY,
		                           casacore::Vector<double>& screenX, casacore::Vector<double>& screenY,
		                           WorldCanvasHolder& wch, casacore::String* error = NULL);

		static bool worldToScreen(const casacore::Quantum<casacore::Vector<double> >& worldX,
		                          const casacore::Quantum<casacore::Vector<double> >& worldY,
		                          casacore::Vector<double>& screenX, casacore::Vector<double>& screenY,
		                          WorldCanvasHolder& wch, casacore::MDirection::Types fromSys,
		                          bool wrap = true, casacore::String* error = NULL) {
			casacore::Vector<double> linX(worldX.getValue().size()),
			       linY(worldY.getValue().size());
			return worldToLinear(worldX, worldY, linX, linY, wch, fromSys, wrap,
			                     error) && linearToScreen(linX,linY,screenX,screenY,wch,error);
		}

		static bool pixelToScreen(const casacore::Vector<double>& pixelX,
		                          const casacore::Vector<double>& pixelY,
		                          casacore::Vector<double>& screenX, casacore::Vector<double>& screenY,
		                          WorldCanvasHolder& wch, casacore::String* error = NULL) {
			casacore::Vector<double> linX(pixelX.size()), linY(pixelY.size());
			return pixelToLinear(pixelX, pixelY, linX, linY, wch, error) &&
			       linearToScreen(linX, linY, screenX, screenY, wch, error);
		}

		static bool screenToLinear(const casacore::Vector<double>& screenX,
		                           const casacore::Vector<double>& screenY,
		                           casacore::Vector<double>& linearX, casacore::Vector<double>& linearY,
		                           WorldCanvasHolder& wch, casacore::String* error = NULL);

		static bool linearToWorld(const casacore::Vector<double>& linearX,
		                          const casacore::Vector<double>& linearY,
		                          casacore::Quantum<casacore::Vector<double> >& worldX,
		                          casacore::Quantum<casacore::Vector<double> >& worldY,
		                          WorldCanvasHolder& wch, casacore::MDirection::Types toSys,
		                          const vector<int>& xSign,const vector<int>& ySign,
		                          bool wrap = true, casacore::String* error = NULL);

		static bool linearToWorld(const casacore::Vector<double>& linearX,
		                          const casacore::Vector<double>& linearY,
		                          casacore::Quantum<casacore::Vector<double> >& worldX,
		                          casacore::Quantum<casacore::Vector<double> >& worldY,
		                          WorldCanvasHolder& wch, casacore::MDirection::Types toSys,
		                          casacore::String* error = NULL) {
			return linearToWorld(linearX, linearY, worldX, worldY, wch, toSys,
			                     vector<int>(), vector<int>(), false, error);
		}

		static bool linearToPixel(const casacore::Vector<double>& linearX,
		                          const casacore::Vector<double>& linearY,
		                          casacore::Vector<double>& pixelX, casacore::Vector<double>& pixelY,
		                          WorldCanvasHolder& wch, casacore::String* error = NULL);

		static bool screenToWorld(const casacore::Vector<double>& screenX,
		                          const casacore::Vector<double>& screenY,
		                          casacore::Quantum<casacore::Vector<double> >& worldX,
		                          casacore::Quantum<casacore::Vector<double> >& worldY,
		                          WorldCanvasHolder& wch, casacore::MDirection::Types toSys,
		                          casacore::String* error = NULL) {
			casacore::Vector<double> linX(screenX.size()), linY(screenY.size());
			return screenToLinear(screenX, screenY, linX, linY, wch) &&
			       linearToWorld(linX, linY, worldX, worldY, wch, toSys,
			                     vector<int>(), vector<int>(), false, error);
		}

		static bool screenToWorld(const casacore::Vector<double>& screenX,
		                          const casacore::Vector<double>& screenY,
		                          casacore::Quantum<casacore::Vector<double> >& worldX,
		                          casacore::Quantum<casacore::Vector<double> >& worldY,
		                          WorldCanvasHolder& wch, casacore::MDirection::Types toSys,
		                          const vector<int>& xSign, const vector<int>& ySign,
		                          casacore::String* error = NULL) {
			casacore::Vector<double> linX(screenX.size()), linY(screenY.size());
			return screenToLinear(screenX, screenY, linX, linY, wch) &&
			       linearToWorld(linX, linY, worldX, worldY, wch, toSys,
			                     xSign, ySign, true, error);
		}

		static bool screenToPixel(const casacore::Vector<double>& screenX,
		                          const casacore::Vector<double>& screenY,
		                          casacore::Vector<double>& pixelX, casacore::Vector<double>& pixelY,
		                          WorldCanvasHolder& wch, casacore::String* error = NULL) {
			casacore::Vector<double> linX(screenX.size()), linY(screenY.size());
			return screenToLinear(screenX, screenY, linX, linY, wch, error) &&
			       linearToPixel(linX, linY, pixelX, pixelY, wch, error);
		}
		// </group>


		// Appends the given message to the given stream, if the stream does not
		// already contain an identical message.  Messages are newline-separated.
		// <group>
		static void appendUniqueMessage(stringstream& ss, const casacore::String& message);
		static void appendUniqueMessage(stringstream* ss, const casacore::String& message) {
			if(ss != NULL) appendUniqueMessage(*ss, message);
		}
		static void appendUniqueMessage(casacore::String& ss, const casacore::String& message);
		static void appendUniqueMessage(casacore::String* ss, const casacore::String& message) {
			if(ss != NULL) appendUniqueMessage(*ss, message);
		}
		// </group>
	};


// A widget that lets the user select a color: either one from a list, or a
// custom color.
	class QtColorWidget : public QHBoxLayout {
		Q_OBJECT

	public:
		// Constructor that uses default colors.  If setText is nonempty, the
		// chooser is set to the given.  If showButton is true, a "pick" button
		// is shown for picking colors.
		QtColorWidget(bool showButton = false, casacore::String setText = "",
		              QWidget* parent = NULL);

		// Constructor that uses the given colors.  If setText is nonempty, the
		// chooser is set to the given.  If showButton is true, a "pick" button
		// is shown for picking colors.
		QtColorWidget(const vector<casacore::String>& colors, bool showButton = false,
		              casacore::String setText = "", QWidget* parent = NULL);

		// Destructor.
		~QtColorWidget();

		// Returns the color that the user has chosen.  This will either be from
		// the initial list, or in "#000000" form (see QColor::name()).
		casacore::String getColor() const;

		// Sets the displayed color to the given.  If the color is in the colors
		// list, that index will be selected -- otherwise it will be entered in
		// the custom color box.
		void setColor(const casacore::String& color);


		// Returns default colors.
		static vector<casacore::String> defaultColors() {
			static vector<casacore::String> v(9);
			v[0] = "white";
			v[1] = "black";
			v[2] = "red";
			v[3] = "green";
			v[4] = "blue";
			v[5] = "cyan";
			v[6] = "magenta";
			v[7] = "yellow";
			v[8] = "gray";
			return v;
		}

	private:
		// Color chooser.
		QComboBox* m_chooser;

		// Custom color.
		QLineEdit* m_edit;

		// Picker button.
		QPushButton* m_button;

		// Initializes GUI members.
		void init(const vector<casacore::String>& colors, const casacore::String& setText,
		          bool showButton);

	private slots:
		// For when the user picks a different color in the chooser.
		void colorChanged(int index);

		// For when the user clicks the "pick" button.
		void colorPick();
	};


// Convenience class for the different units available for coordinates/sizes.
// Two modes: quantum and HMS/DMS (distinguished by the "isQuantum" flag).
// <ol><li>casacore::Quantum: consists of a value and a unit, stored in "val".</li>
//     <li>HMS/DMS: consists of three values and two flags.  Hours/degrees
//         are stored in "hOrD", minutes are stored in "min", and seconds are
//         stored in "sec".  The "isHMS" flag distinguishes between HMS and
//         DMS.  The "isMinusZero" flag is used when "hOrD" is zero, but the
//         whole value is negative.</li></ol>
	class RSValue {
	public:
		RSValue(double d = 0) : isQuantum(true), isHMS(false), isMinusZero(false),
			val(d, DEG), hOrD(0), min(0), sec(0) { }

		RSValue(double d, casacore::Unit u) : isQuantum(true), isHMS(false),
			isMinusZero(false), val(d, u), hOrD(0), min(0), sec(0) { }

		RSValue(bool hms, long hd, long m, double s, bool minus = false) :
			isQuantum(false), isHMS(hms), isMinusZero(minus), val(0, "_"),
			hOrD(hd), min(m), sec(s) { }

		~RSValue() { }

		bool isQuantum;
		bool isHMS;
		bool isMinusZero;

		casacore::Quantum<double> val;
		long hOrD;
		long min;
		double sec;


		// casacore::Conversion methods/constants //

		// Units constants.
		// <group>
		static const casacore::String DEG;
		static const casacore::String RAD;
		static const casacore::String ARCSEC;
		static const casacore::String ARCMIN;
		static const casacore::String HMS;
		static const casacore::String DMS;
		// </group>

		// Converts the value in the given QString in the given units to the given
		// RSValue.  The output RSValue is in degrees.  fromUnits should either be:
		// 1) HMS or DMS constants,
		// 2) a relevant Unit.
		static bool convertBetween(const QString& from, const casacore::String& fromUnits,
		                           RSValue& to);

		// Converts the value in the given RSValue to the given QString, using the
		// given precision for the doubles.
		static bool convertBetween(const RSValue& from, QString& to,
		                           int precision = -1);

		// Converts the value in the given RSValue to the give RSValue which uses
		// the given units.  toUnits should either be:
		// 1) HMS or DMS constants,
		// 2) a relevant Unit.
		static bool convertBetween(const RSValue& from, RSValue& to,
		                           const casacore::String& toUnits);
	};


// Convenience class for a casacore::String, bool, or double.
	class RSOption {
	public:
		// casacore::String constructor.
		RSOption(const casacore::String& str);

		// bool constructor.
		RSOption(bool b = false);

		// double constructor.
		RSOption(double d);

		// casacore::String vector constructor.
		RSOption(const vector<casacore::String>& v);

		// Destructor.
		~RSOption();

		// Type methods.
		// <group>
		bool isString() const;
		bool isBool() const;
		bool isDouble() const;
		bool isStringArray() const;
		// </group>

		// Value methods.
		// <group>
		const casacore::String& asString() const;
		bool asBool() const;
		double asDouble() const;
		const vector<casacore::String>& asStringArray() const;
		// </group>

		// Operators.
		// <group>
		bool operator==(const RSOption& other);
		bool operator!=(const RSOption& other);
		RSOption& operator=(const casacore::String& str);
		RSOption& operator=(bool b);
		RSOption& operator=(double d);
		RSOption& operator=(const vector<casacore::String>& v);
		// </group>

	private:
		bool m_isString;
		casacore::String m_string;
		bool m_isBool;
		bool m_bool;
		bool m_isDouble;
		double m_double;
		bool m_isStringArray;
		vector<casacore::String> m_stringArray;
	};


// A "handle" is a four-point structure (usually a rectangle) that describes
// the boundaries in screen pixels that a RegionShape takes on a canvas.  In
// the future, this will be used for selecting/editing/moving/resizing shapes
// on the canvas using the mouse.
	class RSHandle {
	public:
		// Defaults.
		// <group>
		static const int DEFAULT_MARKER_HEIGHT;
		static const casacore::String DEFAULT_MARKER_COLOR;
		static const Display::Marker DEFAULT_MARKER_TYPE;
		// </group>

		// Constructor which makes an invalid handle.
		RSHandle();

		// Constructor which takes x and y vectors.  x and y MUST be length 4 or
		// the handle is invalid.
		RSHandle(const vector<double>& x, const vector<double>& y,
		         int markerHeight = DEFAULT_MARKER_HEIGHT,
		         const casacore::String& markerColor = DEFAULT_MARKER_COLOR,
		         Display::Marker markerType = DEFAULT_MARKER_TYPE);

		// Constructor which takes x and y Vectors.  x and y MUST be length 4 or
		// the handle is invalid.
		RSHandle(const casacore::Vector<double>& x, const casacore::Vector<double>& y,
		         int markerHeight = DEFAULT_MARKER_HEIGHT,
		         const casacore::String& markerColor = DEFAULT_MARKER_COLOR,
		         Display::Marker markerType = DEFAULT_MARKER_TYPE);

		// Destructor.
		~RSHandle();

		// Gets/sets the marker height/color/type.
		// <group>
		int getMarkerHeight() const {
			return m_markerHeight;
		}
		void setMarkerHeight(int height);
		casacore::String getMarkerColor() const {
			return m_markerColor;
		}
		void setMarkerColor(const casacore::String& color);
		Display::Marker getMarkerType() const {
			return m_markerType;
		}
		void setMarkerType(Display::Marker type);
		// </group>

		// Returns whether the handle is valid (has four valid points) or not.
		bool isValid() const {
			return m_isValid;
		}

		// Returns true if the handle is valid and the given point is inside.
		bool containsPoint(double x, double y) const;

		// Gets the handle vertices coordinates and returns whether the operation
		// succeeded or not (i.e. if the handle is valid).  If it succeeded, x and
		// y will be resize to be size 4 if necessary.
		bool getVertices(vector<double>& x, vector<double>& y) const;

		// Draws the handles on the given canvas and returns whether the operation
		// succeeded or not (i.e. if the handle is valid).  If valid, each of the
		// four points is drawn as a marker.
		bool draw(PixelCanvas* canvas) const;

	private:
		bool m_isValid;
		vector<double> m_x, m_y;
		int m_markerHeight;
		casacore::String m_markerColor;
		Display::Marker m_markerType;
	};

}

#endif /* RSUTILS_H_ */
