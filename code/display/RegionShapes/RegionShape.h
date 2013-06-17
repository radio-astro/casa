//# RegionShape.h: Parent RegionShape class and helpers.
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
#ifndef REGIONSHAPE_H_
#define REGIONSHAPE_H_

#include <display/RegionShapes/RSUtils.qo.h>

#include <display/DisplayDatas/PassiveCachingDD.h>
#include <display/DisplayDatas/CachingDisplayMethod.h>
#include <display/Display/WorldCanvas.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>

#include <casa/namespace.h>

namespace casa {

	class PixelCanvas;

// Text label associated with a RegionShape.
	class RegionShapeLabel {
	public:
		// Static Members //

		// Defaults.
		// <group>
		static const String DEFAULT_FONT;
		static const int DEFAULT_POINTSIZE;
		// </group>


		// Constructor.
		RegionShapeLabel(String text = "");

		// Destructor.
		~RegionShapeLabel();


		// Sets this label's text to the given.
		void setText(const String& text);

		// Sets this label's font family to the given.  How it is handled when
		// being drawn depends on the underlying PixelCanvas implementation.
		void setFont(const String& font);

		// Sets the label's font point size to the given.
		void setSize(int size);

		// Sets the label's font color to the given.  Note: while there is no
		// formal specification for color format, Strings should be accepted in
		// common name format ("black") or hexadecimal format ("#000000").
		void setColor(const String& color);

		// Sets whether the label's font is italic or not.  Note: only
		// QtPixelCanvas currently supports bold.
		void setItalic(bool italic = true);

		// Sets whether the label's font is bold or not.  Note: only QtPixelCanvas
		// currently supports bold.
		void setBold(bool bold = true);

		// Sets the rotation angle to the given (in degrees).  This angle is in
		// pixel coordinates.  Rotation is counterclockwise.
		void setAngle(double angle);


		// Returns true if this label has empty text, false otherwise.
		bool isEmpty() const;

		// Returns the text for this label.
		const String& text() const;

		// Returns the font family for this label.
		const String& font() const;

		// Returns the point size for this label.
		int size() const;

		// Returns the color for this label.
		const String& color() const;

		// Returns true if this label's font is italic, false otherwise.
		bool isItalic() const;

		// Returns true if this label's font is bold, false otherwise.
		bool isBold() const;

		// Returns this label's angle.
		double angle() const;


		// Sets the font on the given PixelCanvas.  Note: bold/italic only
		// work for QtPixelCanvas.
		void setFont(PixelCanvas* pc) const;

		// Returns the result of pc->textWidth() after setting the font.
		int width(PixelCanvas* pc) const;

		// Returns the result of pc->textHeight() after setting the font.
		int height(PixelCanvas* pc) const;

		// Draws itself on the given pixel canvas and returns whether the operation
		// succeeded or not.  x and y are the screen pixel coordinates to draw the
		// label.  If bottom is false, the label is centered on (x, y); otherwise
		// the label is drawn with (x, y) at its bottom center point.
		bool draw(PixelCanvas* pc, int x, int y, bool bottom = false,
		          String* error = NULL);

	private:
		String m_text;      // text
		String m_font;      // font family name
		int m_pointSize; // font point size
		String m_color;     // font color
		bool m_italic;      // is italic?
		bool m_bold;        // is bold?
		double m_angle;     // screen rotation angle
	};


// Parent class for all RegionShapes.  Contains operations and properties
// common to all RegionShapes, such as:
// <ul><li>Line color, width</li>
//     <li>Label (see RegionShapeLabel)</li>
//     <li>Linethrough</li>
//     <li>Minimum/maximum values, both world/pixel and screen</li>
//     <li>World coordinate system (if applicable)</li>
//     <li>Converting between different coordinate systems</li></ul>
// Also contains methods that must be implemented in subclasses:
// <ul><li>Type/name for display</li>
//     <li>Getting/setting coordinates</li>
//     <li>Getting/setting options</li>
//     <li>Moving the shape</li>
//     <li>Drawing on a world canvas</li></ul>
// RegionShapes can also be converted to and from a CASA record.  Fields in
// the record MUST adhere to the properties outlined below and in the shape
// subclasses.  While additional properties may be defined in the shape
// subclasses, properties at the RegionShape level are:
// <ul><li><b>RegionShape::PROPISWORLD</b> (<i>bool</i>): whether the shape has
//         a world system (true) or a pixel system (false).  Note: if true, the
//         world system MUST be defined using the following property.
//         <b>MUST ALWAYS BE DEFINED.</b></li>
//     <li><b>RegionShape::PROPWORLDSYSTEM</b> (<i>String</i>): if the shape
//         is in world coordinates, the world system.  MUST be able to be
//         converted to an MDirection::Types via MDirection::getType().
//         <b>MUST BE DEFINED FOR SHAPES WITH WORLD COORDINATES.</b></li>
//     <li><b>RegionShape::PROPLINE_COLOR</b> (<i>String</i>): line color.
//         <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPLINE_WIDTH</b> (<i>double</i>): line width.
//         <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPLINE_STYLE</b> (<i>String</i>): String
//         representation of line style.  NOTE: styles other than solid and
//         dashed are only supported by QtPixelCanvas.  <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPTEXT</b> (<i>String</i>): label text.
//         <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPTEXT_COLOR</b> (<i>String</i>): label color.
//         <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPTEXT_FONT</b> (<i>String</i>): label font.
//         <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPTEXT_SIZE</b> (<i>int</i>): label point size.
//         <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPTEXT_ITALIC</b> (<i>bool</i>): whether label
//         font is italic.  <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPTEXT_BOLD</b> (<i>bool</i>): whether label font
//         is bold.  <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPLINETHROUGH</b> (<i>bool</li>): whether the
//         shape has a linethrough or not.  <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPLINETHROUGH_COLOR</b> (<i>String</i>):
//         linethrough color.  <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPLINETHROUGH_WIDTH</b> (<i>double</i>):
//         linethrough width.  <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPLINETHROUGH_STYLE</b> (<i>String</i>): String
//         representation of linethrough style.  NOTE: styles other than solid
//         and dashed are only supported by QtPixelCanvas.
//         <b>OPTIONAL.</b></li>
//     <li><b>RegionShape::PROPCOORDINATES</b> (<i>Array&lt;double&gt;</i>):
//         This property will be used to call setCoordParameters, and so is
//         specific to the shape type.  <b>MUST BE DEFINED FOR SHAPE TYPES
//         OTHER THAN RSPolygon AND RSComposite.</b></li>
//     <li><b>RegionShape::PROPOPTIONS</b> (<i>Record</i>): the fields are set
//         in the order defined by each shape type's option methods.  Each
//         field's name should be unique (the specifics are irrelevant since it
//         is the order that matters), and the value should be one of the types
//         supported by RSOption.  <b>OPTIONAL.</b></li></ul>
// RegionShape also defines a shapeFromRecord() method that will create and
// return a new shape based on the given record.  This record has the
// following additional properties:
// <ul><li><b>RegionShape::PROPTYPE</b> (<i>String</i>): for defining which
//         type of shape is defined by the record.  Acceptable choices:
//         <ul><li>RegionShape::PROPTYPE_ELLIPSE</li>
//             <li>RegionShape::PROPTYPE_CIRCLE</li>
//             <li>RegionShape::PROPTYPE_RECTANGLE</li>
//             <li>RegionShape::PROPTYPE_POLYGON</li>
//             <li>RegionShape::PROPTYPE_LINE</li>
//             <li>RegionShape::PROPTYPE_VECTOR</li>
//             <li>RegionShape::PROPTYPE_MARKER</li>
//             <li>RegionShape::PROPTYPE_TEXT</li>
//             <li>RegionShape::PROPTYPE_COMPOSITE.</li></ul>
//         <b>MUST ALWAYS BE DEFINED.</b></li></ul>
	class RegionShape : public PassiveCachingDD {
	public:
		// Static Members //

		// The unit that all native coordinates are in.  Currently set to "deg".
		// If this is changed, DS9RegionFileWriter::convertValue() must also be
		// updated.
		static const String UNIT;

		// Value used for wrapping negative/positive unit values.  Currently set
		// to 360 for degrees.
		static const double UNIT_WRAPAROUND;

		// Properties.  See RegionShape description.
		// <group>
		static const String PROPISWORLD;
		static const String PROPWORLDSYSTEM;
		static const String PROPLINE_COLOR;
		static const String PROPLINE_WIDTH;
		static const String PROPLINE_STYLE;
		static const String PROPTEXT;
		static const String PROPTEXT_COLOR;
		static const String PROPTEXT_FONT;
		static const String PROPTEXT_SIZE;
		static const String PROPTEXT_ITALIC;
		static const String PROPTEXT_BOLD;
		static const String PROPLINETHROUGH;
		static const String PROPLINETHROUGH_COLOR;
		static const String PROPLINETHROUGH_WIDTH;
		static const String PROPLINETHROUGH_STYLE;
		static const String PROPCOORDINATES;
		static const String PROPOPTIONS;

		static const String PROPTYPE;
		static const String PROPTYPE_ELLIPSE;
		static const String PROPTYPE_CIRCLE;
		static const String PROPTYPE_RECTANGLE;
		static const String PROPTYPE_POLYGON;
		static const String PROPTYPE_LINE;
		static const String PROPTYPE_VECTOR;
		static const String PROPTYPE_MARKER;
		static const String PROPTYPE_TEXT;
		static const String PROPTYPE_COMPOSITE;
		// </group>

		// Creates a record of the correct type from the given record (see
		// RegionShape class description), or NULL if a fatal error occurred.
		static RegionShape* shapeFromRecord(const RecordInterface& properties,
		                                    String& error);

		// Different types for coordinate parameters.
		enum CoordinateParameterType {
		    POSITION, SIZE, ANGLE, PIXEL
		};

		std::string errorMessage( ) const { return ""; }

		// Different types for options.
		enum OptionType {
		    STRING, STRINGCHOICE, STRINGARRAY, STRINGCHOICEARRAY, DOUBLE, BOOL
		};

		// Different line styles.  NOTE: styles other than solid and dashed are
		// ONLY supported by QtPixelCanvas.  Custom line styles are used for
		// custom dash lists.
		enum LineStyle {
		    SOLID, DASHED, DOTTED
		};

		// Returns all available line styles.
		static vector<LineStyle> allLineStyles() {
			static vector<LineStyle> v(3);
			v[0] = SOLID;
			v[1] = DASHED;
			v[2] = DOTTED;
			return v;
		}

		// Convert between line style enum and String.
		// <group>
		static String convLineStyle(LineStyle style);
		static LineStyle convLineStyle(String style);
		// </group>

		// Defaults.
		// <group>
		static const String DEFAULT_COLOR;
		static const double DEFAULT_WIDTH;
		static const LineStyle DEFAULT_STYLE;
		// <group>


		// Non-Static Members //

		// World Constructor.  Note: implementing subclasses MUST supply values
		// for m_xMin, m_xMax, m_yMin, and m_yMax, or override xMin(), xMax(),
		// yMin(), and yMax().
		RegionShape(MDirection::Types worldSys);

		// Pixel Constructor.  Note: implementing subclasses MUST supply values
		// for m_xMin, m_xMax, m_yMin, and m_yMax, or override xMin(), xMax(),
		// yMin(), and yMax().
		RegionShape();

		// Record Constructor.
		RegionShape(const RecordInterface& properties);

		// Destructor
		virtual ~RegionShape();


		// PassiveCachingDD methods //

		// Implements PassiveCachingDD::dataUnit.
		const Unit dataUnit() const;
		const IPosition dataShape() const {
			return IPosition( );
		}
		const uInt dataDim() const {
			return 0;
		}
		std::vector<int> displayAxes( ) const {
			return std::vector<int>( );
		}

		// Implements PassiveCachingDD::classType.
		Display::DisplayDataType classType();

		// Implements PassiveCachingDD::newDisplayMethod.
		CachingDisplayMethod* newDisplayMethod(WorldCanvas* wc,
		                                       AttributeBuffer* wchAttr, AttributeBuffer* ddAttr,
		                                       CachingDisplayData* dd);


		// Common RegionShape methods //

		// Gets/sets this shape's line color.
		// <group>
		virtual String lineColor() const;
		virtual void setLineColor(const String& newColor, bool alsoSetLabel= true);
		// </group>

		// Gets/sets this shape's line width.
		// <group>
		virtual double lineWidth() const;
		virtual void setLineWidth(double width);
		// </group>

		// Gets/sets this shape's line style.  See RegionShape::LineStyle.
		// <group>
		virtual LineStyle lineStyle() const;
		virtual void setLineStyle(LineStyle style);
		// </group>

		// Gets/sets this shape's label's text.
		// <group>
		virtual String text() const;
		virtual void setText(const String& text);
		// </group>

		// Gets/sets this shape's label.
		// <group>
		virtual const RegionShapeLabel& label() const;
		virtual void setLabel(const RegionShapeLabel& label);
		// </group>

		// Gets/sets this shape's linethrough properties.
		// <group>
		virtual bool linethrough() const;
		virtual String linethroughColor() const;
		virtual double linethroughWidth() const;
		virtual LineStyle linethroughStyle() const;
		virtual void setLinethrough(bool linethrough, String color = "red",
		                            double width = 1, LineStyle style = SOLID);
		// </group>

		// Min/max methods for world/pixel coordinates.  Note that some shapes
		// (RSMarker, RSText) may have identical mins/maxes since they only
		// take up space in screen pixels.
		// <group>
		virtual double xMin() const;
		virtual double xMax() const;
		virtual double yMin() const;
		virtual double yMax() const;
		// </group>

		// Bounding box methods.  Returns the min and max of the screen
		// coordinates.  Note: these are only valid after a call to
		// drawAndUpdateBoundingBox!
		// <group>
		virtual double screenXMin() const;
		virtual double screenXMax() const;
		virtual double screenYMin() const;
		virtual double screenYMax() const;
		// </group>

		// Returns whether this shape's coordinates are in world or pixel.
		virtual bool isWorld() const {
			return m_isWorld;
		}

		// If this shape is in world coordinates, returns the system used.
		// Undefined for pixel coordinates.
		virtual MDirection::Types worldSystem() const {
			return m_worldSystem;
		}

		// Set world or pixel system.
		virtual void setIsWorld(bool isWorld) {
			m_isWorld = isWorld;
		}

		// Set world system.
		virtual void setWorldSystem(MDirection::Types sys) {
			m_worldSystem = sys;
		}

		// Returns this shape's type (color + type).
		virtual String title() const {
			return lineColor() + " " + type();
		}

		// Checks whether the given WorldCanvasHolder has a direction coordinate.
		// Updates the last WorldCanvasHolder seen to the given, then calls
		// drawAndUpdateBoundingBox which actually draws this shape onto the
		// world canvas.
		bool draw(WorldCanvasHolder& wch);

		// Returns whether there was an error during the last call to draw() or
		// not.
		virtual bool hadDrawingError() const {
			return m_hadDrawingError;
		}

		// Returns the error that occurred during the last call to draw().
		const String& lastDrawingError() const {
			return m_drawingError;
		}

		// Returns this shape's properties as a Record.
		Record getProperties() const;

		// Synonym.
		Record toRecord() const {
			return getProperties();
		}

		// Sets this shape's properties from the given Record.
		void setProperties(const RecordInterface& properties);


		// Pure Virtual Methods //

		// Draw this shape on the given world canvas and update the bounding box
		// protected variables m_screenXMin, m_screenXMax, m_screenYMin,
		// and m_screenYMax.  NOTE: implementing classes should be sure to set the
		// line style of the pixel canvas back to Display::LSSolid when finished.
		// The protected method resetLineStyle can be used for this purpose.
		virtual bool drawAndUpdateBoundingBox(WorldCanvasHolder& wch,
		                                      String* error = NULL) = 0;

		// Returns the name/type of this shape, for displaying to the user.
		virtual String type() const = 0;
		// Pure virtual function from DisplayData...
		String dataType() const {
			return type( );
		}


		// Can be overridden by children.  Returns a one-word type.
		virtual String oneWordType() const {
			return type();
		}

		// Returns the current values for the coordinate parameters.  The
		// number and order depend on the shape type.  For world systems
		// (RegionShape::isWorld()), these coordinates have unit RegionShape::UNIT
		// in the system RegionShape::worldSystem().
		virtual vector<double> coordParameterValues() const = 0;

		// Returns the current values for the coordinate parameters in the given
		// world system, by using the last WorldCanvasHolder for conversions if
		// needed.  The number and order depend on the shape type.  toSystem
		// indivates which system the given values are in and should either be:
		// 1) RSUtils::PIXEL for pixel coordinates, or
		// 2) a value that can be converted to a MDirection::Types via
		//    MDirection::getType().
		// Returned coordinates have unit RegionShape::UNIT for world systems.
		virtual vector<double> coordParameterValues(String toSystem) const = 0;

		// Returns the name for the coordinate parameters, in the same order
		// as coordParameterValues().
		virtual vector<String> coordParameterNames() const = 0;

		// Returns a vector indicating the types of the coordinate parameters,
		// in the same order as coordParameterValues().
		virtual vector<CoordinateParameterType> coordParameterTypes() const = 0;

		// Sets the coordinate parameter values to the given, in the same order
		// as coordParameterValues().  For world systems (RegionShape::isWorld()),
		// the values must be in unit RegionShape::UNIT and system
		// RegionShape::worldSystem().
		virtual void setCoordParameters(const vector<double>& vals) = 0;

		// Sets the coordinate parameter values to the given, in the same order
		// as coordParameterValues().  valSystem indicates which system the given
		// values are in and should either be:
		// 1) RSUtils::PIXEL for pixel coordinates, or
		// 2) a value that can be converted to a MDirection::Types via
		//    MDirection::getType().
		// Values must be in unit RegionShape::UNIT for world systems.
		virtual void setCoordParameters(const vector<double>& vals,
		                                String valSystem) = 0;

		// Moves this shape the given delta x and delta y in the given system
		// (empty for shape's native system).
		virtual void move(double dx, double dy, String system = "") = 0;

		// Returns a RSHandle (four points in screen coords that are like a
		// bounding box for the shape) for this shape.  If the shape has not
		// previously been drawn on a canvas, this may return an invalid RSHandle.
		// The handle does NOT include the label.
		virtual RSHandle getHandle() const = 0;

		// Returns the names for options.
		virtual vector<String> optionNames() const = 0;

		// Returns the types for options.
		virtual vector<OptionType> optionTypes() const = 0;

		// For options that are string choices, return the available choices.
		// May return an empty vector if there are no string choices.
		virtual vector<vector<String> > optionChoices() const {
			return vector<vector<String> >();
		}

		// Returns the current values for options.
		virtual vector<RSOption> optionValues() const = 0;

		// Sets the options to the given.
		virtual void setOptionValues(const vector<RSOption>& options) = 0;

	protected:
		// Common RegionShape Members //

		// Whether this shape uses world or pixel coordinates.
		bool m_isWorld;

		// World system (undefined if m_isWorld is false).
		MDirection::Types m_worldSystem;

		// Line color.
		String m_color;

		// Line width.
		double m_width;

		// Line style.
		LineStyle m_style;

		// Label.
		RegionShapeLabel m_label;

		// Linethrough properties.
		// <group>
		bool m_lt;
		String m_ltColor;
		double m_ltWidth;
		LineStyle m_ltStyle;
		// </group>

		// Bounding box, in world/pixel coords.
		double m_xMin, m_xMax, m_yMin, m_yMax;

		// Bounding box, in screen pixels.
		double m_screenXMin, m_screenXMax, m_screenYMin, m_screenYMax;

		// Last seen WorldCanvasHolder.  Should only be used by
		// coordParameterValues.
		WorldCanvasHolder* m_lastWCH;

		// For keeping track of drawing/conversion errors.
		// <group>
		bool m_hadDrawingError;
		String m_drawingError;
		// </group>


		// Pure Virtual Methods //

		// Puts shape-specific properties in the given record.  MUST at least set
		// PROP_TYPE.
		virtual void getShapeSpecificProperties(Record& properties) const = 0;

		// Sets shape-specific properties from the given record.
		virtual void setShapeSpecificProperties(const RecordInterface& props) = 0;


		// Common RegionShape Methods //

		// Sets the line properties (color, width, style) of this shape for the
		// given PixelCanvas.
		void setLineProperties(PixelCanvas* pc);

		// Sets the linethrough properties (color, width, style) of this shape for
		// the given PixelCanvas.
		void setLinethroughProperties(PixelCanvas* pc);

		// Used for resetting the line style on the given PixelCanvas back to
		// solid, in case future uses are expecting solid lines.
		void resetLineStyle(PixelCanvas* pc) const;

		// Convenience access to RSUtils conversion methods.
		// <group>
		bool worldToPixel(const Quantum<Vector<double> >& worldX,
		                  const Quantum<Vector<double> >& worldY,
		                  Vector<double>& pixelX, Vector<double>& pixelY,
		                  WorldCanvasHolder& wch, bool wrap = true,
		                  String* error = NULL) const {
			return RSUtils::worldToPixel(worldX, worldY, pixelX, pixelY, wch,
			                             m_worldSystem, wrap, error);
		}

		bool pixelToWorld(const Vector<double>& pixelX,
		                  const Vector<double>& pixelY,
		                  Quantum<Vector<double> >& worldX,
		                  Quantum<Vector<double> >& worldY, WorldCanvasHolder& wch,
		                  bool wrap = true, String* error = NULL) const {
			return RSUtils::pixelToWorld(pixelX, pixelY, worldX, worldY, wch,
			                             m_worldSystem, wrap, error);
		}

		bool worldToLinear(const Quantum<Vector<double> >& worldX,
		                   const Quantum<Vector<double> >& worldY,
		                   Vector<double>& linearX, Vector<double>& linearY,
		                   WorldCanvasHolder& wch, bool wrap = true,
		                   String* error = NULL) const {
			return RSUtils::worldToLinear(worldX, worldY, linearX, linearY, wch,
			                              m_worldSystem, wrap, error);
		}

		bool worldToScreen(const Quantum<Vector<double> >& worldX,
		                   const Quantum<Vector<double> >& worldY,
		                   Vector<double>& screenX, Vector<double>& screenY,
		                   WorldCanvasHolder& wch, bool wrap = true,
		                   String* error = NULL) const {
			Vector<double> linX(worldX.getValue().size()),
			       linY(worldY.getValue().size());
			return RSUtils::worldToLinear(worldX, worldY, linX, linY, wch,
			                              m_worldSystem, wrap, error) &&
			       RSUtils::linearToScreen(linX, linY, screenX, screenY,wch,error);
		}

		bool linearToWorld(const Vector<double>& linearX,
		                   const Vector<double>& linearY,
		                   Quantum<Vector<double> >& worldX,
		                   Quantum<Vector<double> >& worldY,
		                   WorldCanvasHolder& wch, String* error = NULL) const {
			return RSUtils::linearToWorld(linearX, linearY, worldX, worldY, wch,
			                              m_worldSystem, vector<int>(),
			                              vector<int>(), false, error);
		}

		bool linearToWorld(const Vector<double>& linearX,
		                   const Vector<double>& linearY,
		                   Quantum<Vector<double> >& worldX,
		                   Quantum<Vector<double> >& worldY,
		                   WorldCanvasHolder& wch, const vector<int>& xSign,
		                   const vector<int>& ySign, String* error = NULL) const {
			return RSUtils::linearToWorld(linearX, linearY, worldX, worldY, wch,
			                              m_worldSystem, xSign, ySign, true, error);
		}

		bool screenToWorld(const Vector<double>& screenX,
		                   const Vector<double>& screenY,
		                   Quantum<Vector<double> >& worldX,
		                   Quantum<Vector<double> >& worldY,
		                   WorldCanvasHolder& wch, String* error = NULL) const {
			Vector<double> linX(screenX.size()), linY(screenY.size());
			return RSUtils::screenToLinear(screenX, screenY, linX,linY,wch,error)&&
			       RSUtils::linearToWorld(linX, linY, worldX, worldY, wch,
			                              m_worldSystem, vector<int>(),
			                              vector<int>(), false, error);
		}

		bool screenToWorld(const Vector<double>& screenX,
		                   const Vector<double>& screenY,
		                   Quantum<Vector<double> >& worldX,
		                   Quantum<Vector<double> >& worldY,
		                   WorldCanvasHolder& wch, const vector<int>& xSign,
		                   const vector<int>& ySign, String* error = NULL) const {
			Vector<double> linX(screenX.size()), linY(screenY.size());
			return RSUtils::screenToLinear(screenX, screenY, linX,linY,wch,error)&&
			       RSUtils::linearToWorld(linX, linY, worldX, worldY, wch,
			                              m_worldSystem, xSign, ySign, true,error);
		}
		// </group>

	private:
		// Sets default values where needed for all member variables, EXCEPT
		// m_isWorld and m_worldSystem.  Intended to be called from a constructor.
		void initDefaults();
	};


// Implementation of CachingDisplayMethod to be used by RegionShapes.
// Basically just a call to the shape's draw method.
	class RegionShapeDM : public CachingDisplayMethod {
	public:
		// Constructor that takes the RegionShape to draw, as well as the other
		// parameters for the CachingDisplayMethod constructor.
		RegionShapeDM(RegionShape& shape, WorldCanvas* wc, AttributeBuffer* wcAttr,
		              AttributeBuffer* ddAttr, CachingDisplayData* dd);

		// Destructor.
		~RegionShapeDM();

		// Implementation of CachingDisplayMethod::drawIntoList.
		Bool drawIntoList(Display::RefreshReason reason, WorldCanvasHolder& wch);

	private:
		RegionShape& m_shape; // Parent shape.
	};

}

#endif /* REGIONSHAPE_H_ */
