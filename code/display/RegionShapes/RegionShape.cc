//# RegionShape.cc: Parent RegionShape class and helpers.
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
#include <QFont>

#include <display/RegionShapes/RegionShape.h>
#include <display/QtViewer/QtPixelCanvas.qo.h>
#include <display/Display/PixelCanvas.h>
#include <display/RegionShapes/RegionShapes.h>

namespace casa {

// REGIONSHAPELABEL DEFINITIONS //

	const String RegionShapeLabel::DEFAULT_FONT = "times";
	const int RegionShapeLabel::DEFAULT_POINTSIZE = 11;

	RegionShapeLabel::RegionShapeLabel(String text) : m_text(text),
		m_font(DEFAULT_FONT), m_pointSize(DEFAULT_POINTSIZE),
		m_color(RegionShape::DEFAULT_COLOR), m_italic(false), m_bold(false),
		m_angle(0) { }

	RegionShapeLabel::~RegionShapeLabel() { }


	void RegionShapeLabel::setText(const String& text) {
		m_text = text;
	}
	void RegionShapeLabel::setFont(const String& font) {
		if(!font.empty()) m_font = font;
	}
	void RegionShapeLabel::setSize(int size) {
		if(size > 0) m_pointSize = size;
	}
	void RegionShapeLabel::setColor(const String& color) {
		if(!color.empty()) m_color = color;
	}
	void RegionShapeLabel::setItalic(bool italic) {
		m_italic = italic;
	}
	void RegionShapeLabel::setBold(bool bold) {
		m_bold = bold;
	}
	void RegionShapeLabel::setAngle(double angle) {
		m_angle = angle;
	}

	bool RegionShapeLabel::isEmpty() const {
		return m_text.empty();
	}
	const String& RegionShapeLabel::text() const {
		return m_text;
	}
	const String& RegionShapeLabel::font() const {
		return m_font;
	}
	int RegionShapeLabel::size() const {
		return m_pointSize;
	}
	const String& RegionShapeLabel::color() const {
		return m_color;
	}
	bool RegionShapeLabel::isItalic() const {
		return m_italic;
	}
	bool RegionShapeLabel::isBold() const {
		return m_bold;
	}
	double RegionShapeLabel::angle() const {
		return m_angle;
	}

	void RegionShapeLabel::setFont(PixelCanvas* pc) const {
		if(pc == NULL) return;

		QtPixelCanvas* qpc = dynamic_cast<QtPixelCanvas*>(pc);
		if(qpc != NULL) {
			// in QtPixelCanvas it doesn't matter if font is empty or size is < 0
			qpc->setFont(m_font, m_pointSize, m_bold, m_italic);
		} else {
			if(!m_font.empty()) {
				if(m_pointSize > 0) pc->setFont(m_font, m_pointSize);
				else                pc->setFont(m_font);
			}
		}
	}

	int RegionShapeLabel::width(PixelCanvas* pc) const {
		setFont(pc);
		return pc->textWidth(m_text);
	}

	int RegionShapeLabel::height(PixelCanvas* pc) const {
		setFont(pc);
		return pc->textHeight(m_text);
	}

	bool RegionShapeLabel::draw(PixelCanvas* pc, int x, int y, bool bottom,
	                            String* error) {
		if(pc == NULL || x < 0 || y < 0 || m_text.empty()) {
			if(error) {
				String e;
				if(pc == NULL) e = "label: given a null PixelCanvas.";
				else if(x < 0 || y < 0) e = "label: given negative coordinates.";
				else e = "label: cannot draw empty text.";
				RSUtils::appendUniqueMessage(error, e);
			}
			return false;
		}

		pc->setColor(m_color);

		setFont(pc);

		pc->drawText(x, y, m_text, m_angle, bottom ? Display::AlignBottom :
		             Display::AlignCenter);

		return true;
	}


// REGIONSHAPE DEFINITIONS //

// Constructors/Destructors //

	RegionShape::RegionShape(): m_isWorld(false), m_worldSystem(MDirection::J2000) {
		initDefaults();
	}

	RegionShape::RegionShape(MDirection::Types worldSystem) : m_isWorld(true),
		m_worldSystem(worldSystem) {
		initDefaults();
	}

	RegionShape::RegionShape(const RecordInterface& properties) : m_isWorld(false),
		m_worldSystem(MDirection::J2000) {
		initDefaults();
		setProperties(properties);
	}

	RegionShape::~RegionShape() { }


// Public Methods //

	const Unit RegionShape::dataUnit() const {
		if(m_isWorld) return UNIT;
		else return "_";
	}

	Display::DisplayDataType RegionShape::classType() {
		return Display::Annotation;
	}

	CachingDisplayMethod* RegionShape::newDisplayMethod(WorldCanvas* wc,
	        AttributeBuffer* wchAttr, AttributeBuffer* ddAttr,
	        CachingDisplayData* dd) {
		return new RegionShapeDM(*this, wc, wchAttr, ddAttr, dd);
	}

	String RegionShape::lineColor() const {
		return m_color;
	}
	void RegionShape::setLineColor(const String& newColor, bool setLabel) {
		if(newColor != m_color || (setLabel && newColor != m_label.color())) {
			m_color = newColor;
			if(setLabel) m_label.setColor(newColor);
			refresh(true);
		}
	}

	double RegionShape::lineWidth() const {
		return m_width;
	}
	void RegionShape::setLineWidth(double width) {
		if(width != m_width && width > 0) {
			m_width = width;
			refresh(true);
		}
	}

	RegionShape::LineStyle RegionShape::lineStyle() const {
		return m_style;
	}
	void RegionShape::setLineStyle(LineStyle style) {
		if(style != m_style) {
			m_style = style;
			refresh(true);
		}
	}

	String RegionShape::text() const {
		return m_label.text();
	}
	void RegionShape::setText(const String& text) {
		if(text != m_label.text()) {
			m_label.setText(text);
			refresh(true);
		}
	}

	const RegionShapeLabel& RegionShape::label() const {
		return m_label;
	}
	void RegionShape::setLabel(const RegionShapeLabel& label) {
		m_label = label;
		refresh(true);
	}

	bool RegionShape::linethrough() const {
		return m_lt;
	}
	String RegionShape::linethroughColor() const {
		return m_ltColor;
	}
	double RegionShape::linethroughWidth() const {
		return m_ltWidth;
	}
	RegionShape::LineStyle RegionShape::linethroughStyle() const {
		return m_ltStyle;
	}

	void RegionShape::setLinethrough(bool lthrough, String color, double width,
	                                 LineStyle style) {
		if(m_lt != lthrough || color != m_ltColor ||
		        (width != m_ltWidth && width > 0) || style != m_ltStyle) {
			m_lt = lthrough;
			m_ltColor = color;
			if(width > 0) m_ltWidth = width;
			m_ltStyle = style;
			refresh(true);
		}
	}

	double RegionShape::xMin() const {
		return m_xMin;
	}
	double RegionShape::xMax() const {
		return m_xMax;
	}
	double RegionShape::yMin() const {
		return m_yMin;
	}
	double RegionShape::yMax() const {
		return m_yMax;
	}

	double RegionShape::screenXMin() const {
		return m_screenXMin;
	}
	double RegionShape::screenXMax() const {
		return m_screenXMax;
	}
	double RegionShape::screenYMin() const {
		return m_screenYMin;
	}
	double RegionShape::screenYMax() const {
		return m_screenYMax;
	}

	bool RegionShape::draw(WorldCanvasHolder& wch) {
		if(!RSUtils::hasDirectionCoordinate(wch)) {
			m_lastWCH = NULL;
			m_drawingError = "Cannot draw without a direction coordinate.";
			return false;
		}

		m_lastWCH = &wch;
		m_drawingError = "";
		m_hadDrawingError = !drawAndUpdateBoundingBox(wch, &m_drawingError);
		return !m_hadDrawingError;
	}

	Record RegionShape::getProperties() const {
		Record props;

		bool b = isWorld();
		props.define(PROPISWORLD, b);
		if(b) props.define(PROPWORLDSYSTEM, MDirection::showType(worldSystem()));

		props.define(PROPLINE_COLOR, lineColor());
		props.define(PROPLINE_WIDTH, lineWidth());
		props.define(PROPLINE_STYLE, convLineStyle(lineStyle()));

		props.define(PROPTEXT, text());
		const RegionShapeLabel& l = label();
		props.define(PROPTEXT_COLOR, l.color());
		props.define(PROPTEXT_FONT, l.font());
		props.define(PROPTEXT_SIZE, l.size());
		props.define(PROPTEXT_ITALIC, l.isItalic());
		props.define(PROPTEXT_BOLD, l.isBold());

		props.define(PROPLINETHROUGH, linethrough());
		props.define(PROPLINETHROUGH_COLOR, linethroughColor());
		props.define(PROPLINETHROUGH_WIDTH, linethroughWidth());
		props.define(PROPLINETHROUGH_STYLE, convLineStyle(linethroughStyle()));

		props.define(PROPCOORDINATES, Vector<double>(coordParameterValues()));

		vector<RSOption> o = optionValues();
		Record opts;
		for(unsigned int i = 0; i < o.size(); i++) {
			String s = String::toString(i);
			if(o[i].isString())      opts.define(s, o[i].asString());
			else if(o[i].isBool())   opts.define(s, o[i].asBool());
			else if(o[i].isDouble()) opts.define(s, o[i].asDouble());
			else if(o[i].isStringArray())
				opts.define(s, Vector<String>(o[i].asStringArray()));
		}
		if(opts.nfields() > 0) props.defineRecord(PROPOPTIONS, opts);

		getShapeSpecificProperties(props);
		return props;
	}

	void RegionShape::setProperties(const RecordInterface& p) {
		if(p.isDefined(PROPISWORLD) && p.dataType(PROPISWORLD) == TpBool)
			setIsWorld(p.asBool(PROPISWORLD));

		if(p.isDefined(PROPWORLDSYSTEM)&& p.dataType(PROPWORLDSYSTEM)== TpString) {
			MDirection::Types sys;
			if(MDirection::getType(sys, p.asString(PROPWORLDSYSTEM)))
				setWorldSystem(sys);
		}

		if(p.isDefined(PROPLINE_COLOR) && p.dataType(PROPLINE_COLOR) == TpString)
			setLineColor(p.asString(PROPLINE_COLOR));

		if(p.isDefined(PROPLINE_WIDTH) && p.dataType(PROPLINE_WIDTH) == TpDouble)
			setLineWidth(p.asDouble(PROPLINE_WIDTH));

		if(p.isDefined(PROPLINE_STYLE) && p.dataType(PROPLINE_STYLE) == TpString)
			setLineStyle(convLineStyle(p.asString(PROPLINE_STYLE)));

		RegionShapeLabel l = label();
		if(p.isDefined(PROPTEXT) && p.dataType(PROPTEXT) == TpString)
			l.setText(p.asString(PROPTEXT));
		if(p.isDefined(PROPTEXT_COLOR) && p.dataType(PROPTEXT_COLOR) == TpString)
			l.setColor(p.asString(PROPTEXT_COLOR));
		if(p.isDefined(PROPTEXT_FONT) && p.dataType(PROPTEXT_FONT) == TpString)
			l.setFont(p.asString(PROPTEXT_FONT));
		if(p.isDefined(PROPTEXT_SIZE) && p.dataType(PROPTEXT_SIZE) == TpInt)
			l.setSize(p.asInt(PROPTEXT_SIZE));
		if(p.isDefined(PROPTEXT_ITALIC) && p.dataType(PROPTEXT_ITALIC) == TpBool)
			l.setItalic(p.asBool(PROPTEXT_ITALIC));
		if(p.isDefined(PROPTEXT_BOLD) && p.dataType(PROPTEXT_BOLD) == TpBool)
			l.setBold(p.asBool(PROPTEXT_BOLD));
		setLabel(l);

		bool b = linethrough();
		if(p.isDefined(PROPLINETHROUGH) && p.dataType(PROPLINETHROUGH) == TpBool)
			b = p.asBool(PROPLINETHROUGH);
		String s = linethroughColor();
		if(p.isDefined(PROPLINETHROUGH_COLOR)&& p.dataType(PROPLINETHROUGH_COLOR)==
		        TpString) s = p.asString(PROPLINETHROUGH_COLOR);
		double d = linethroughWidth();
		if(p.isDefined(PROPLINETHROUGH_WIDTH)&& p.dataType(PROPLINETHROUGH_WIDTH)==
		        TpDouble) d = p.asDouble(PROPLINETHROUGH_WIDTH);
		LineStyle ls = linethroughStyle();
		if(p.isDefined(PROPLINETHROUGH_STYLE)&& p.dataType(PROPLINETHROUGH_STYLE)==
		        TpString) ls = convLineStyle(p.asString(PROPLINETHROUGH_STYLE));
		setLinethrough(b, s, d, ls);

		if(p.isDefined(PROPCOORDINATES) &&
		        p.dataType(PROPCOORDINATES) == TpArrayDouble) {
			vector<double> coords;
			p.asArrayDouble(PROPCOORDINATES).tovector(coords);
			setCoordParameters(coords);
		}

		if(p.isDefined(PROPOPTIONS) && p.dataType(PROPOPTIONS) == TpRecord) {
			vector<RSOption> opts;
			const RecordInterface& o = p.asRecord(PROPOPTIONS);
			for(unsigned int i = 0; i < o.nfields(); i++) {
				if(o.dataType(i) == TpString)
					opts.push_back(RSOption(o.asString(i)));
				else if(o.dataType(i) == TpBool)
					opts.push_back(RSOption(o.asBool(i)));
				else if(o.dataType(i) == TpDouble)
					opts.push_back(RSOption(o.asDouble(i)));
				else if(o.dataType(i) == TpArrayString) {
					vector<String> v;
					o.asArrayString(i).tovector(v);
					opts.push_back(RSOption(v));
				}
			}
			setOptionValues(opts);
		}

		setShapeSpecificProperties(p);
	}


// Protected Methods //

	void RegionShape::setLineProperties(PixelCanvas* pc) {
		if(pc == NULL) return;

		String c = lineColor();
		if(c.empty()) {
			setLineColor("green");
			c = "green";
		}
		pc->setColor(c);

		double w = lineWidth();
		if(w < 1) {
			setLineWidth(1);
			w = 1;
		}
		pc->setLineWidth(w);

		LineStyle s = lineStyle();
		if(s == SOLID)       pc->setLineStyle(Display::LSSolid);
		else if(s == DASHED) pc->setLineStyle(Display::LSDashed);
		else {
			QtPixelCanvas* qpc = dynamic_cast<QtPixelCanvas*>(pc);
			if(qpc != NULL) {
				if(s == DOTTED) qpc->setQtLineStyle(Qt::DotLine);
			}
		}
	}

	void RegionShape::setLinethroughProperties(PixelCanvas* pc) {
		if(pc == NULL || !m_lt) return;

		String c = linethroughColor();
		if(c.empty()) {
			setLinethrough(true, "red", linethroughWidth(), linethroughStyle());
			c = "red";
		}
		pc->setColor(c);

		double w = linethroughWidth();
		if(w < 1) {
			setLinethrough(true, c, 1, linethroughStyle());
			w = 1;
		}
		pc->setLineWidth(w);

		LineStyle s = linethroughStyle();
		if(s == SOLID)       pc->setLineStyle(Display::LSSolid);
		else if(s == DASHED) pc->setLineStyle(Display::LSDashed);
		else {
			QtPixelCanvas* qpc = dynamic_cast<QtPixelCanvas*>(pc);
			if(qpc != NULL) {
				if(s == DOTTED) qpc->setQtLineStyle(Qt::DotLine);
			}
		}
	}

	void RegionShape::resetLineStyle(PixelCanvas* pc) const {
		if(pc != NULL) pc->setLineStyle(Display::LSSolid);
	}


// Private Methods //

	void RegionShape::initDefaults() {
		m_color = DEFAULT_COLOR;
		m_width = DEFAULT_WIDTH;
		m_style = DEFAULT_STYLE;

		m_lt = false;
		m_ltColor = "red";
		m_ltWidth = DEFAULT_WIDTH;
		m_ltStyle = DEFAULT_STYLE;

		m_xMin = m_xMax = m_yMin = m_yMax = m_screenXMin = m_screenXMax =
		                                        m_screenYMin = m_screenYMax = 0;

		m_lastWCH = NULL;

		m_hadDrawingError = false;

	}


// Static Members //

	const String RegionShape::UNIT = "deg";
	const double RegionShape::UNIT_WRAPAROUND = 360.0;

	const String RegionShape::PROPISWORLD                 = "isworld";
	const String RegionShape::PROPWORLDSYSTEM             = "worldsys";
	const String RegionShape::PROPLINE_COLOR              = "line_color";
	const String RegionShape::PROPLINE_WIDTH              = "line_width";
	const String RegionShape::PROPLINE_STYLE              = "line_style";
	const String RegionShape::PROPTEXT                    = "text";
	const String RegionShape::PROPTEXT_COLOR              = "text_color";
	const String RegionShape::PROPTEXT_FONT               = "text_font";
	const String RegionShape::PROPTEXT_SIZE               = "text_size";
	const String RegionShape::PROPTEXT_ITALIC             = "text_italic";
	const String RegionShape::PROPTEXT_BOLD               = "text_bold";
	const String RegionShape::PROPLINETHROUGH             = "linethrough";
	const String RegionShape::PROPLINETHROUGH_COLOR       = "ltcolor";
	const String RegionShape::PROPLINETHROUGH_WIDTH       = "ltwidth";
	const String RegionShape::PROPLINETHROUGH_STYLE       = "ltstyle";
	const String RegionShape::PROPCOORDINATES             = "coordinates";
	const String RegionShape::PROPOPTIONS                 = "options";

	const String RegionShape::PROPTYPE                    = "type";
	const String RegionShape::PROPTYPE_ELLIPSE            = "type_ellipse";
	const String RegionShape::PROPTYPE_CIRCLE             = "type_circle";
	const String RegionShape::PROPTYPE_RECTANGLE          = "type_rectangle";
	const String RegionShape::PROPTYPE_POLYGON            = "type_polygon";
	const String RegionShape::PROPTYPE_LINE               = "type_line";
	const String RegionShape::PROPTYPE_VECTOR             = "type_vector";
	const String RegionShape::PROPTYPE_MARKER             = "type_marker";
	const String RegionShape::PROPTYPE_TEXT               = "type_text";
	const String RegionShape::PROPTYPE_COMPOSITE          = "type_composite";

	RegionShape* RegionShape::shapeFromRecord(const RecordInterface& props,
	        String& error) {
		Record p(props);
		// check that required fields are present
		if(!p.isDefined(PROPISWORLD) || p.dataType(PROPISWORLD) != TpBool) {
			error = "Property '" + PROPISWORLD + "' must be defined as a bool.";
			return NULL;
		}
		MDirection::Types t;
		if(p.asBool(PROPISWORLD) && (!p.isDefined(PROPWORLDSYSTEM) ||
		                             p.dataType(PROPWORLDSYSTEM) != TpString || !MDirection::getType(t,
		                                     p.asString(PROPWORLDSYSTEM)))) {
			error = "Property '" + PROPWORLDSYSTEM + "' must be a String "
			        "representation of an MDirection::Types.";
			return NULL;
		}
		if(!p.asBool(PROPISWORLD) && p.isDefined(PROPWORLDSYSTEM))
			p.removeField(PROPWORLDSYSTEM);
		if(!p.isDefined(PROPTYPE) || p.dataType(PROPTYPE) != TpString) {
			error = "Property '" + PROPTYPE + "' must be defined as a String.";
			return NULL;
		}

		String type = p.asString(PROPTYPE);
		p.removeField(PROPTYPE);
		if(type != PROPTYPE_ELLIPSE && type != PROPTYPE_CIRCLE && type !=
		        PROPTYPE_RECTANGLE && type != PROPTYPE_POLYGON && type != PROPTYPE_LINE
		        && type != PROPTYPE_VECTOR && type != PROPTYPE_MARKER && type !=
		        PROPTYPE_TEXT && type != PROPTYPE_COMPOSITE) {
			error = "Unknown value for property '" + PROPTYPE + "'";
			return NULL;
		}

		if(type != PROPTYPE_POLYGON && type != PROPTYPE_COMPOSITE && (!p.isDefined(
		            PROPCOORDINATES) || p.dataType(PROPCOORDINATES) != TpArrayDouble)) {
			error = "Property '" + PROPCOORDINATES + "' must be defined as a "
			        "double array.";
			return NULL;
		}

		if(p.isDefined(PROPCOORDINATES) && (type == PROPTYPE_POLYGON || type ==
		                                    PROPTYPE_COMPOSITE)) p.removeField(PROPCOORDINATES);

		RegionShape* s = NULL;

		if(type == PROPTYPE_ELLIPSE) s = new RSEllipse(p);
		else if(type == PROPTYPE_CIRCLE) s = new RSCircle(p);
		else if(type == PROPTYPE_RECTANGLE) s = new RSRectangle(p);
		else if(type == PROPTYPE_POLYGON) s = new RSPolygon(p);
		else if(type == PROPTYPE_LINE) s = new RSLine(p);
		else if(type == PROPTYPE_VECTOR) s = new RSVector(p);
		else if(type == PROPTYPE_MARKER) s = new RSMarker(p);
		else if(type == PROPTYPE_TEXT) s = new RSText(p);
		else if(type == PROPTYPE_COMPOSITE) s = new RSComposite(p);

		if(s == NULL) error = "Unknown error occurred.  (Shouldn't happen.)";

		return s;
	}

	String RegionShape::convLineStyle(LineStyle style) {
		switch(style) {
		case SOLID:
			return "solid";
		case DASHED:
			return "dashed";
		case DOTTED:
			return "dotted";

		default:
			return "";
		}
	}

	RegionShape::LineStyle RegionShape::convLineStyle(String style) {
		for(unsigned int i= 0; i < style.size(); i++) style[i] = tolower(style[i]);
		if(style == "solid")       return SOLID;
		else if(style == "dashed") return DASHED;
		else if(style == "dotted") return DOTTED;

		else return SOLID;
	}


	const String RegionShape::DEFAULT_COLOR = "green";
	const double RegionShape::DEFAULT_WIDTH = 1.0;
	const RegionShape::LineStyle RegionShape::DEFAULT_STYLE = RegionShape::SOLID;


// REGIONSHAPEDM DEFINITIONS //

	RegionShapeDM::RegionShapeDM(RegionShape& rs, WorldCanvas* wc,
	                             AttributeBuffer* wchAttr, AttributeBuffer* ddAttr,
	                             CachingDisplayData* dd): CachingDisplayMethod(wc, wchAttr, ddAttr, dd),
		m_shape(rs) { }

	RegionShapeDM::~RegionShapeDM() { }

	Bool RegionShapeDM::drawIntoList(Display::RefreshReason /*reason*/,
	                                 WorldCanvasHolder& wch) {
		return m_shape.draw(wch);
	}

}
