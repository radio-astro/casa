//# DS9FileWriter.cc: DS9 implementation of RegionFileWriter classes.
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

#include <display/RegionShapes/DS9FileWriter.qo.h>
#include <display/RegionShapes/RegionShapes.h>
#include <display/RegionShapes/QtSingleRegionShape.qo.h>
#include <display/RegionShapes/RSUtils.qo.h>

#include <imageanalysis/Annotations/AnnAnnulus.h>
#include <imageanalysis/Annotations/AnnCenterBox.h>
#include <imageanalysis/Annotations/AnnCircle.h>
#include <imageanalysis/Annotations/AnnEllipse.h>
#include <imageanalysis/Annotations/AnnLine.h>
#include <imageanalysis/Annotations/AnnPolygon.h>
#include <imageanalysis/Annotations/AnnRectBox.h>
#include <imageanalysis/Annotations/AnnRegion.h>
#include <imageanalysis/Annotations/AnnRotBox.h>
#include <imageanalysis/Annotations/AnnSymbol.h>
#include <imageanalysis/Annotations/AnnText.h>
#include <imageanalysis/Annotations/AnnVector.h>

#include <iomanip>

namespace casa {

// DS9FILEWRITER DEFINITIONS //

	DS9FileWriter::DS9FileWriter() {
		resetOptions();
	}

	DS9FileWriter::~DS9FileWriter() { }


	QWidget* DS9FileWriter::optionsWidget() const {
		return new QtDS9RFWOptions();
	}

	void DS9FileWriter::setOptions(const QWidget* widget) {
		const QtDS9RFWOptions* opts = dynamic_cast<const QtDS9RFWOptions*>(widget);
		if(opts == NULL) return;

		m_custCoordSys = opts->isCustomCoordinateSystem();
		if(m_custCoordSys) {
			m_pixelCoordSys = opts->isPixelCoordinateSystem();
			m_coordSys = opts->getCoordinateSystem();
		}
		m_posUnits = opts->getPositionUnits();
		m_sizeUnits = opts->getSizeUnits();
		m_comments = opts->getComments();
		m_precision = opts->getPrecision();
	}

	bool DS9FileWriter::write(const vector<RegionShape*>& shapes)const {
		QFile file(m_filename.c_str());
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			setError("Could not open file " + m_filename, true);
			return false;
		}

		bool success = true;
		stringstream errors;
		QTextStream out(&file);

		success &= writeHeader(out, errors);
		success &= writeGlobals(out, errors);

		for(unsigned int i = 0; i < shapes.size(); i++)
			if(shapes[i] != NULL)
				success &= writeShape(out, errors, shapes[i]);

		resetOptions();
		setError(errors.str(), false);
		return success;
	}

// Private Methods //

	void DS9FileWriter::resetOptions() const {
		// setup default options
		const_cast<bool&>(m_custCoordSys) = false;
		const_cast<bool&>(m_pixelCoordSys) = false;
		const_cast<String&>(m_coordSys) = CSYS_J2000;
		const_cast<String&>(m_posUnits) = UNIT_DEGREES;
		const_cast<String&>(m_sizeUnits) = UNIT_DEGREES;
		const_cast<int&>(m_precision) = -1;
		const_cast<String&>(m_comments) = DEFAULT_COMMENTS;
	}

	bool DS9FileWriter::writeHeader(QTextStream& file,
	                                stringstream& /*errors*/) const {
		if(m_comments.empty()) return true;
		QStringList comments = QString(m_comments.c_str()).split('\n',
		                       QString::SkipEmptyParts);
		for(int i = 0; i < comments.size(); i++)
			file << DS9::FILE_COMMENT << ' ' << comments[i] << '\n';
		return true;
	}

	bool DS9FileWriter::writeGlobals(QTextStream& file,stringstream& /*errors*/) const {
		file << DS9::FILE_GLOBAL;

		vector<String> props = globalProperties();
		for(unsigned int i = 0; i < props.size(); i++) {
			file << ' ' << props[i].c_str() << DS9::FILE_EQUAL;
			if(DS9Region::isBoolProperty(props[i]))
				file << DS9Region::defaultBoolValue(props[i]);
			else {
				if(props[i] == DS9Region::PROP_FONT) file << DS9::FILE_TEXT_START2;
				file << DS9Region::defaultStringValue(props[i]).c_str();
				if(props[i] == DS9Region::PROP_FONT) file << DS9::FILE_TEXT_END2;
			}
		}
		file << "\n";

		if(m_custCoordSys)
			file << DS9::coordinateSystem(coordinateSystem(m_coordSys)) << '\n';

		return true;
	}

	bool DS9FileWriter::writeShape(QTextStream& file, stringstream& errors,
	                               const RegionShape* shape, bool isComposite) const {
		if(shape == NULL) return true;

		// special marker case
		const RSMarker* marker = dynamic_cast<const RSMarker*>(shape);
		if(marker != NULL) {
			bool wasConv = false;
			vector<RSMarker*> convMarkers = convertMarker(marker, wasConv);
			if(wasConv) {
				for(unsigned int i = 0; i < convMarkers.size(); i++) {
					writeShape(file, errors, convMarkers[i]);
					delete convMarkers[i];
				}
				RSUtils::appendUniqueMessage(errors, "Marker type not supported by"
				                             " DS9. Converting to one or more DS9-friendly markers.");
				return false;
			}

			// otherwise marker (should be!) DS9-friendly
		}

		DS9::RegionType type;
		if(!regionType(shape, type, errors)) return false;

		const RSCircle* c = dynamic_cast<const RSCircle*>(shape);
		const RSEllipse* e = dynamic_cast<const RSEllipse*>(shape);
		const RSRectangle* r = dynamic_cast<const RSRectangle*>(shape);
		const RSPolygon* p = dynamic_cast<const RSPolygon*>(shape);
		const RSVector* v = dynamic_cast<const RSVector*>(shape);
		const RSLine* l = dynamic_cast<const RSLine*>(shape);
		const RSMarker* m = dynamic_cast<const RSMarker*>(shape);
		const RSText* t = dynamic_cast<const RSText*>(shape);
		const RSComposite* cp = dynamic_cast<const RSComposite*>(shape);

		if(c == NULL && e == NULL && r == NULL && p == NULL && v == NULL &&
		        l == NULL && m == NULL && t == NULL && cp == NULL) {
			RSUtils::appendUniqueMessage(errors, "Unknown shape type! (Really "
			                             "shouldn't happen.)");
			return false;
		}

		bool convertCoords = m_custCoordSys;
		bool toPixel = (m_custCoordSys && m_pixelCoordSys) ||
		               (!m_custCoordSys && !shape->isWorld());
		String convertSys = toPixel ? RSUtils::PIXEL : m_coordSys,
		       pUnit      = toPixel ? UNIT_IPIXELS : m_posUnits,
		       sUnit      = toPixel ? UNIT_IPIXELS : m_sizeUnits;
		if(!convertCoords) {
			if(!shape->isWorld()) file << DS9::coordinateSystem(DS9::Image);
			else {
				if(isValidDS9System(shape->worldSystem()))
					file << DS9::coordinateSystem(coordinateSystem(
					                                  MDirection::showType(shape->worldSystem())));
				else {
					convertCoords = true;
					convertSys = CSYS_J2000;
					file << DS9::coordinateSystem(coordinateSystem(convertSys));
				}
			}
			file << DS9::FILE_LINESEP << ' ';
		}
		if(!convertCoords) convertSys = "";

		if(shape->linethrough()) file << DS9::FILE_MINUS;

		if(m == NULL) file << DS9::regionType(type);
		else          file << DS9::FILE_POINT;

		// coordinates
		int markerPxHeight = 0;
		double textAngle = 0;
		if(c != NULL) {
			double x, y, r;
			c->getCircleCoordinates(x, y, r, convertSys);
			file << "(";
			file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision) << ","
			     << convertValue(y, sgUnit(pUnit,1,convertSys), m_precision) << ","
			     << convertValue(r, sUnit, m_precision);
			file << ")";

		} else if(e != NULL) {
			double x, y, xrad, yrad, angle;
			e->getEllipseCoordinates(x, y, xrad, yrad, angle, convertSys);
			file << "(";
			file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision) << ","
			     << convertValue(y, sgUnit(pUnit,1,convertSys), m_precision) << ","
			     << convertValue(xrad, sUnit, m_precision) << ","
			     << convertValue(yrad, sUnit, m_precision) << ",";
			// compensate for stupid DS9 angles
			if(!toPixel) {
				angle = 180 - angle;
				if(angle < 0) angle += 360;
			}
			file << convertValue(angle, "", m_precision);
			file << ")";

		} else if(r != NULL) {
			double x, y, width, height, angle;
			r->getRectangleCoordinates(x, y, width, height, angle, convertSys);
			file << "(";
			file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision) << ","
			     << convertValue(y, sgUnit(pUnit,1,convertSys), m_precision) << ","
			     << convertValue(width, sUnit, m_precision) << ","
			     << convertValue(height, sUnit, m_precision) << ",";
			// compensate for stupid DS9 angles
			if(!toPixel) {
				angle = 180 - angle;
				if(angle < 0) angle += 360;
			}
			file << angle;
			file << ")";

		} else if(p != NULL) {
			vector<double> x, y;
			p->getPolygonCoordinates(x, y, convertSys);
			unsigned int n = x.size();
			if(y.size() < x.size()) n = y.size();
			file << "(";
			for(unsigned int i = 0; i < n; i++) {
				file << convertValue(x[i], sgUnit(pUnit,0,convertSys),m_precision)
				     << ","
				     << convertValue(y[i], sgUnit(pUnit,1,convertSys),m_precision);
				if(i < n - 1) file << ",";
			}
			file << ")";

		} else if(v != NULL) {
			double x, y, angle, length;
			int temp;
			v->getVectorCoordinates(x, y, angle, length, temp, convertSys);
			file << "(";
			file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision) << ","
			     << convertValue(y, sgUnit(pUnit,1,convertSys), m_precision) << ","
			     << convertValue(length, sUnit, m_precision) << ",";
			// compensate for stupid DS9 angles
			if(!toPixel) {
				angle = 180 - angle;
				if(angle < 0) angle += 360;
			}
			file << convertValue(angle, "", m_precision);
			file << ")";

		} else if(l != NULL) {
			double x1, y1, x2, y2;
			int temp;
			l->getLineCoordinates(x1, y1, x2, y2, temp, convertSys);
			file << "(";
			file << convertValue(x1, sgUnit(pUnit,0,convertSys),m_precision) << ","
			     << convertValue(y1, sgUnit(pUnit,1,convertSys),m_precision) << ","
			     << convertValue(x2, sgUnit(pUnit,0,convertSys),m_precision) << ","
			     << convertValue(y2, sgUnit(pUnit,1,convertSys),m_precision);
			file << ")";

		} else if(m != NULL) {
			double x, y;
			m->getMarkerCoordinates(x, y, markerPxHeight, convertSys);
			file << "(";
			file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision) << ","
			     << convertValue(y, sgUnit(pUnit,1,convertSys), m_precision);
			file << ")";

		} else if(t != NULL) {
			double x, y;
			t->getTextCoordinates(x, y, textAngle, convertSys);
			file << "(";
			file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision) << ","
			     << convertValue(y, sgUnit(pUnit,1,convertSys), m_precision);
			file << ")";
			// compensate for stupid DS9 angles
			if(!toPixel) {
				textAngle = 180 - textAngle;
				if(textAngle < 0) textAngle += 360;
			}

		} else if(cp != NULL) {
			file << "(";
			if(type == DS9::Annulus) {
				vector<const RegionShape*> children = cp->children();
				double x, y, xrad;
				c = dynamic_cast<const RSCircle*>(children[0]);
				c->getCircleCoordinates(x, y, xrad, convertSys);
				file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision)
				     << ","
				     << convertValue(y, sgUnit(pUnit,1,convertSys), m_precision)
				     << "," << convertValue(xrad, sUnit, m_precision);
				for(unsigned int i = 1; i < children.size(); i++) {
					c = dynamic_cast<const RSCircle*>(children[i]);
					c->getCircleCoordinates(x, y, xrad, convertSys);
					file << "," << convertValue(xrad, sUnit, m_precision);
				}

			} else if(type == DS9::EllipseAnnulus) {
				vector<const RegionShape*> children = cp->children();
				double x, y, xrad, yrad, angle;
				e = dynamic_cast<const RSEllipse*>(children[0]);
				e->getEllipseCoordinates(x, y, xrad, yrad, angle, convertSys);
				file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision)
				     << ","
				     << convertValue(y, sgUnit(pUnit,1,convertSys), m_precision)
				     << "," << convertValue(xrad, sUnit, m_precision) << ","
				     << convertValue(yrad, sUnit, m_precision);
				for(unsigned int i = 1; i < children.size(); i++) {
					e = dynamic_cast<const RSEllipse*>(children[i]);
					e->getEllipseCoordinates(x, y, xrad, yrad, angle, convertSys);
					file << "," << convertValue(xrad, sUnit, m_precision)
					     << "," << convertValue(yrad, sUnit, m_precision);
				}
				// compensate for stupid DS9 angles
				if(!toPixel) {
					angle = 180 - angle;
					if(angle < 0) angle += 360;
				}
				file << "," << convertValue(angle, "", m_precision);

			} else if(type == DS9::BoxAnnulus) {
				vector<const RegionShape*> children = cp->children();
				double x, y, width, height, angle;
				r = dynamic_cast<const RSRectangle*>(children[0]);
				r->getRectangleCoordinates(x, y, width, height, angle, convertSys);
				file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision)
				     << ","
				     << convertValue(y, sgUnit(pUnit,1,convertSys), m_precision)
				     << "," << convertValue(width, sUnit, m_precision) << ","
				     << convertValue(height, sUnit, m_precision);
				for(unsigned int i = 1; i < children.size(); i++) {
					r = dynamic_cast<const RSRectangle*>(children[i]);
					r->getRectangleCoordinates(x,y,width,height,angle,convertSys);
					file << "," << convertValue(width, sUnit, m_precision)
					     << "," << convertValue(height, sUnit, m_precision);
				}
				// compensate for stupid DS9 angles
				if(!toPixel) {
					angle = 180 - angle;
					if(angle < 0) angle += 360;
				}
				file << "," << angle;

			} else {
				double x, y;
				cp->getCompositeCoordinates(x, y, convertSys);
				file << convertValue(x, sgUnit(pUnit,0,convertSys), m_precision)
				     << ","
				     <<convertValue(y,sgUnit(pUnit,1,convertSys),m_precision)<<",";
				if(m_pixelCoordSys) file << 0;
				else {
					double angle = cp->getAngle(convertSys);
					// compensate for stupid DS9 angles
					if(!toPixel) {
						angle = 180 - angle;
						if(angle < 0) angle += 360;
					}
					file << angle;
				}
			}
			file << ")";
		}

		if(isComposite || type == DS9::Composite)
			file << ' ' << DS9::FILE_COMPOSITE_OR;

		stringstream props;
		String equals(DS9::FILE_EQUAL.toStdString());

		// shape-specific properties
		bool hadProps = false;
		if(c != NULL) {

		} else if(e != NULL) {

		} else if(r != NULL) {

		} else if(p != NULL) {

		} else if(v != NULL) {
			props << DS9Region::PROP_VECTOR << equals << v->arrow();
			hadProps = true;
		} else if(l != NULL) {
			props << DS9Region::PROP_LINE << equals << l->p1Arrow() << ' '
			      << l->p2Arrow();
			hadProps = true;
		} else if(m != NULL) {
			props << DS9::FILE_POINT.toStdString() << equals
			      << DS9::pointType(type).toStdString() << ' ' <<markerPxHeight;
			hadProps = true;
		} else if(t != NULL) {
			props << DS9Region::PROP_TEXTANGLE << equals
			      << QString("%1").arg(textAngle,0,'f',m_precision).toStdString();
			hadProps = true;
		} else if(cp != NULL) {
			if(type == DS9::Annulus) {

			} else if(type == DS9::EllipseAnnulus) {

			} else if(type == DS9::BoxAnnulus) {

			} else {
				props << DS9Region::PROP_COMPOSITE << equals
				      << cp->childrenAreDependent();
				hadProps = true;
			}
		}

		// general properties
		if(shape->lineColor() != DS9Region::defaultStringValue(
		            DS9Region::PROP_COLOR)) {
			if(hadProps) props << ' ';
			props << DS9Region::PROP_COLOR << equals << shape->lineColor();
			hadProps = true;
		}
		stringstream ss;
		// ds9 gets confused by real font names, so just use helvetica..
		ss << "helvetica " << (int)(shape->label().size() + 0.5) << ' ';
		if(shape->label().isBold())        ss << "bold";
		else if(shape->label().isItalic()) ss << "italic";
		else                               ss << "normal";
		String font(ss.str());
		if(!shape->text().empty() &&
		        font != DS9Region::defaultStringValue(DS9Region::PROP_FONT)) {
			if(hadProps) props << ' ';
			props << DS9Region::PROP_FONT << equals
			      << DS9::FILE_TEXT_START2.toStdString() << font
			      << DS9::FILE_TEXT_END2.toStdString();
			hadProps = true;
		}
		if(!shape->text().empty()) {
			if(hadProps) props << ' ';
			props << DS9Region::PROP_TEXT << equals
			      << DS9::FILE_TEXT_START1.toStdString() << shape->text()
			      << DS9::FILE_TEXT_END1.toStdString();
			hadProps = true;
		}
		ss.str("");
		ss << (int)(shape->lineWidth() + 0.5);
		String width(ss.str());
		if(width != DS9Region::defaultStringValue(DS9Region::PROP_WIDTH)) {
			if(hadProps) props << ' ';
			props << DS9Region::PROP_WIDTH << equals << width;
			hadProps = true;
		}

		bool dashed = shape->lineStyle() != RegionShape::SOLID;
		if(dashed != DS9Region::defaultBoolValue(DS9Region::PROP_DASH)) {
			if(hadProps) props << ' ';
			props << DS9Region::PROP_DASH << equals << dashed;
		}
		if(dashed) {
			switch(shape->lineStyle()) {
			case RegionShape::DASHED:
				if(hadProps) props << ' ';
				props << DS9Region::PROP_DASHLIST << equals;
				props << DS9Region::defaultStringValue(DS9Region::PROP_DASHLIST);
				break;

			case RegionShape::DOTTED:
				if(hadProps) props << ' ';
				props << DS9Region::PROP_DASHLIST << equals;
				props << "3 3";
				break;

			default:
				break;
			}
		}

		if(hadProps) file << ' '<< DS9::FILE_COMMENT <<' ' << props.str().c_str();

		// composite children
		file << '\n';
		bool success = true;
		if(cp != NULL && type == DS9::Composite) {
			vector<const RegionShape*> children = cp->children();
			for(unsigned int i = 0; i < children.size(); i++) {
				// all but last child have composite ||
				success &= writeShape(file, errors, children[i],
				                      i < children.size() - 1);
			}
		}

		return success;
	}


// QTDS9RFWOPTIONS DEFINITIONS //

	QtDS9RFWOptions::QtDS9RFWOptions() : QWidget() {
		setupUi(this);

		// setup coordSystem
		coordSystem->addItem(DS9FileWriter::CSYS_INDIVIDUAL.c_str());
		coordSystem->addItem(DS9FileWriter::CSYS_IMAGE.c_str());
		coordSystem->addItem(DS9FileWriter::CSYS_B1950.c_str());
		coordSystem->addItem(DS9FileWriter::CSYS_J2000.c_str());
		coordSystem->addItem(DS9FileWriter::CSYS_GALACTIC.c_str());
		coordSystem->addItem(DS9FileWriter::CSYS_ECLIPTIC.c_str());

		// setup positionUnits
		positionUnits->addItem(DS9FileWriter::UNIT_DEGREES.c_str());
		positionUnits->addItem(DS9FileWriter::UNIT_RADIANS.c_str());
		positionUnits->addItem(DS9FileWriter::PUNIT_SEXAGESIMAL.c_str());
		positionUnits->addItem(DS9FileWriter::PUNIT_HMS.c_str());
		positionUnits->addItem(DS9FileWriter::PUNIT_DMS.c_str());

		// setup sizeUnits
		sizeUnits->addItem(DS9FileWriter::UNIT_DEGREES.c_str());
		sizeUnits->addItem(DS9FileWriter::UNIT_RADIANS.c_str());
		sizeUnits->addItem(DS9FileWriter::SUNIT_ARCSEC.c_str());
		sizeUnits->addItem(DS9FileWriter::SUNIT_ARCMIN.c_str());

		coordSystem->setCurrentIndex(0);
		coordinateSystemChanged(0);

		// setup commentEdit
		commentEdit->setPlainText(DS9FileWriter::DEFAULT_COMMENTS.c_str());

		connect(coordSystem, SIGNAL(currentIndexChanged(int)),
		        SLOT(coordinateSystemChanged(int)));
	}

	QtDS9RFWOptions::~QtDS9RFWOptions() { }


	bool QtDS9RFWOptions::isCustomCoordinateSystem() const {
		return coordSystem->currentIndex() > 0;
	}

	bool QtDS9RFWOptions::isPixelCoordinateSystem() const {
		return coordSystem->currentIndex() == 1;
	}

	String QtDS9RFWOptions::getCoordinateSystem() const {
		return coordSystem->currentText().toStdString();
	}

	String QtDS9RFWOptions::getPositionUnits() const {
		return positionUnits->currentText().toStdString();
	}

	String QtDS9RFWOptions::getSizeUnits() const {
		return sizeUnits->currentText().toStdString();
	}

	int QtDS9RFWOptions::getPrecision() const {
		return precisionSpinner->value();
	}

	String QtDS9RFWOptions::getComments() const {
		return commentEdit->toPlainText().toStdString();
	}


	void QtDS9RFWOptions::coordinateSystemChanged(int index) {
		bool en = index != 1;
		positionUnits->setEnabled(en);
		sizeUnits->setEnabled(en);
	}


// DS9FILEWRITER STATIC CONSTANTS //

	const String DS9FileWriter::CSYS_INDIVIDUAL   = "Use Individual Shapes'";
	const String DS9FileWriter::CSYS_IMAGE        = "IMAGE (Pixel)";
	const String DS9FileWriter::CSYS_B1950        =
	    MDirection::showType(MDirection::B1950);
	const String DS9FileWriter::CSYS_J2000        =
	    MDirection::showType(MDirection::J2000);
	const String DS9FileWriter::CSYS_GALACTIC     =
	    MDirection::showType(MDirection::GALACTIC);
	const String DS9FileWriter::CSYS_ECLIPTIC     =
	    MDirection::showType(MDirection::ECLIPTIC);
	const String DS9FileWriter::UNIT_IPIXELS      = "pixels";
	const String DS9FileWriter::UNIT_DEGREES      = "degrees";
	const String DS9FileWriter::UNIT_RADIANS      = "radians";
	const String DS9FileWriter::PUNIT_SEXAGESIMAL = "sexagesimal";
	const String DS9FileWriter::PUNIT_HMS         = "H:M:S";
	const String DS9FileWriter::PUNIT_DMS         = "D:M:S";
	const String DS9FileWriter::SUNIT_ARCSEC      = "arc sec";
	const String DS9FileWriter::SUNIT_ARCMIN      = "arc min";
	const String DS9FileWriter::DEFAULT_COMMENTS  =
	    "Region file format: DS9 version 4.1\nGenerated by CASA "
	    "[http://casa.nrao.edu]";


	bool DS9FileWriter::regionType(const RegionShape* shape,
	                               DS9::RegionType& type, stringstream& errors) {
		if(shape == NULL) {
			RSUtils::appendUniqueMessage(errors, "Given null shape. (Shouldn't "
			                             "happen.)");
			return false;
		}

		const RSMarker* m;
		const RSComposite* cp;
		if(dynamic_cast<const RSCircle*>(shape) != NULL) type = DS9::Circle;
		else if(dynamic_cast<const RSEllipse*>(shape) != NULL) type = DS9::Ellipse;
		else if(dynamic_cast<const RSRectangle*>(shape) != NULL) type = DS9::Box;
		else if(dynamic_cast<const RSPolygon*>(shape) != NULL) type = DS9::Polygon;
		else if(dynamic_cast<const RSVector*>(shape) != NULL) type = DS9::Vector;
		else if(dynamic_cast<const RSLine*>(shape) != NULL) type = DS9::Line;
		else if((m = dynamic_cast<const RSMarker*>(shape)) != NULL) {
			// NOTE: this method is expecting the RSMarkers to already be converted
			// to a DS9-friendly format.
			vector<Display::Marker> markers = m->markers();
			if(markers.size() == 0) {
				RSUtils::appendUniqueMessage(errors, "Empty marker. (Shouldn't "
				                             "happen.)");
				return false;
			} else if(markers.size() == 1) {
				switch(markers[0]) {
				case Display::Cross:
					type = DS9::CrossPoint;
					break;
				case Display::X:
					type = DS9::XPoint;
					break;
				case Display::Diamond:
					type = DS9::DiamondPoint;
					break;
				case Display::Circle:
					type = DS9::CirclePoint;
					break;
				case Display::Square:
					type = DS9::BoxPoint;
					break;

				default:
					RSUtils::appendUniqueMessage(errors, "Point type not valid for"
					                             " DS9. (Shouldn't happen.)");
					return false;
				}
			} else if(markers.size() == 2) {
				if((markers[0]== Display::Square && markers[1]== Display::Circle)||
				        (markers[0]== Display::Circle && markers[1]== Display::Square))
					type = DS9::BoxCirclePoint;
				else {
					RSUtils::appendUniqueMessage(errors, "Compound point type not "
					                             "valid for DS9. (Shouldn't happen.)");
					return false;
				}
			} else {
				RSUtils::appendUniqueMessage(errors, "Compound point type not "
				                             "valid for DS9. (Shouldn't happen.)");
				return false;
			}
		} else if(dynamic_cast<const RSText*>(shape) != NULL) type = DS9::Text;
		else if((cp = dynamic_cast<const RSComposite*>(shape)) != NULL) {
			vector<const RegionShape*> children = cp->children();
			if(children.size() == 0) {
				RSUtils::appendUniqueMessage(errors, "Empty composite. (Shouldn't "
				                             "happen.)");
				return false;
			} else if(cp->childrenAreDependent()) {
				bool isAnn(false);
				bool isWorld(false);
				MDirection::Types world(MDirection::J2000);
				double x(0), y(0), x2(0), y2(0), temp(0);

				// annulus
				const RSCircle* c = dynamic_cast<const RSCircle*>(children[0]);
				isAnn = c != NULL && c->isWorld() == cp->isWorld() &&
				        (!c->isWorld() || c->worldSystem() == cp->worldSystem());
				if(isAnn) {
					isWorld = c->isWorld();
					if(isWorld) world = c->worldSystem();
					c->getCircleCoordinates(x, y, temp);

					for(unsigned int i = 1; isAnn && i < children.size(); i++) {
						c = dynamic_cast<const RSCircle*>(children[i]);
						if(c == NULL) {
							isAnn = false;
							break;
						}
						if(c->isWorld() != isWorld ||
						        (isWorld && c->worldSystem() != world)) {
							isAnn = false;
							break;
						}
						c->getCircleCoordinates(x2, y2, temp);
						if(x != x2 || y != y2) {
							isAnn = false;
							break;
						}
					}
					if(isAnn) {
						type = DS9::Annulus;
						return true;
					}
				}

				// elliptical annulus
				const RSEllipse* e = dynamic_cast<const RSEllipse*>(children[0]);
				isAnn = e != NULL && e->isWorld() == cp->isWorld() &&
				        (!e->isWorld() || e->worldSystem() == cp->worldSystem());
				if(isAnn) {
					double angle, angle2;
					isWorld = e->isWorld();
					if(isWorld) world = e->worldSystem();
					e->getEllipseCoordinates(x, y, temp, temp, angle);

					for(unsigned int i = 1; isAnn && i < children.size(); i++) {
						c = dynamic_cast<const RSCircle*>(children[i]);
						if(c != NULL) {
							isAnn = false;
							break;
						}
						e = dynamic_cast<const RSEllipse*>(children[i]);
						if(e == NULL) {
							isAnn = false;
							break;
						}
						if(e->isWorld() != isWorld ||
						        (isWorld && e->worldSystem() != world)) {
							isAnn = false;
							break;
						}
						e->getEllipseCoordinates(x2, y2, temp, temp, angle2);
						if(x != x2 || y != y2 || angle != angle2) {
							isAnn = false;
							break;
						}
					}
					if(isAnn) {
						type = DS9::EllipseAnnulus;
						return true;
					}
				}

				// box annulus
				const RSRectangle* r=dynamic_cast<const RSRectangle*>(children[0]);
				isAnn = r != NULL && r->isWorld() == cp->isWorld() &&
				        (!r->isWorld() || r->worldSystem() == cp->worldSystem());
				if(isAnn) {
					double angle, angle2;
					isWorld = r->isWorld();
					if(isWorld) world = r->worldSystem();
					r->getRectangleCoordinates(x, y, temp, temp, angle);

					for(unsigned int i = 1; isAnn && i < children.size(); i++) {
						r = dynamic_cast<const RSRectangle*>(children[i]);
						if(r == NULL) {
							isAnn = false;
							break;
						}
						if(r->isWorld() != isWorld ||
						        (isWorld && r->worldSystem() != world)) {
							isAnn = false;
							break;
						}
						r->getRectangleCoordinates(x2, y2, temp, temp, angle2);
						if(x != x2 || y != y2 || angle != angle2) {
							isAnn = false;
							break;
						}
					}
					if(isAnn) {
						type = DS9::BoxAnnulus;
						return true;
					}
				}
				if(!isAnn) type = DS9::Composite;
			} else type = DS9::Composite;

		} else {
			RSUtils::appendUniqueMessage(errors, "Unknown shape type. (Shouldn't "
			                             "happen.)");
			return false;
		}

		return true;
	}

	bool DS9FileWriter::regionType(const AnnRegion* shape,
	                               DS9::RegionType& type, stringstream& errors) {
		if(shape == NULL) {
			RSUtils::appendUniqueMessage(errors, "Given null annotation. (Shouldn't "
			                             "happen.)");
			return false;
		}

		const AnnSymbol* m;
		//const RSComposite* cp;
		if(dynamic_cast<const AnnCircle*>(shape) != NULL) type = DS9::Circle;
		else if(dynamic_cast<const AnnEllipse*>(shape) != NULL) type = DS9::Ellipse;
		else if(dynamic_cast<const AnnRectBox*>(shape) != NULL) type = DS9::Box;
		else if(dynamic_cast<const AnnPolygon*>(shape) != NULL) type = DS9::Polygon;
		else if(dynamic_cast<const AnnVector*>(shape) != NULL) type = DS9::Vector;
		else if(dynamic_cast<const AnnLine*>(shape) != NULL) type = DS9::Line;
		else if((m = dynamic_cast<const AnnSymbol*>(shape)) != NULL) {
			switch(m->getSymbol( )) {
			case AnnSymbol::X:
				type = DS9::XPoint;
				break;
			case AnnSymbol::CIRCLE:
				type = DS9::CirclePoint;
				break;
			case AnnSymbol::DIAMOND:
				type = DS9::DiamondPoint;
				break;
			case AnnSymbol::SQUARE:
				type = DS9::BoxPoint;
				break;
			default:
				RSUtils::appendUniqueMessage(errors, "symbol type not valid for DS9; defaulting to CrossPoint");
				type = DS9::CrossPoint;
				break;
			}
		} else if(dynamic_cast<const AnnText*>(shape) != NULL) type = DS9::Text;
		else {
			RSUtils::appendUniqueMessage(errors, "Unknown shape type. (Shouldn't "
			                             "happen.)");
			return false;
		}

		return true;
	}


	vector<RSMarker*> DS9FileWriter::convertMarker(const RSMarker* marker,
	        bool& conversionWasNeeded) {
		conversionWasNeeded = false;
		if(marker == NULL) return vector<RSMarker*>();

		vector<RSMarker*> ms;
		RSMarker* m;
		bool newCompound = false;
		Record props = marker->getProperties();

		vector<Display::Marker> markers = marker->markers();
		if(markers.size() == 0) return ms;
		else if(markers.size() == 1) {
			Display::Marker nm = markers[0];
			if(nm != Display::Cross && nm != Display::X && nm!= Display::Diamond &&
			        nm != Display::Circle && nm != Display::Square) {
				nm = Display::X;
				conversionWasNeeded = true;
				m = new RSMarker(props);
				m->setMarker(nm);
			} else m = new RSMarker(*marker);
			ms.push_back(m);
		} else if(markers.size() == 2) {
			if((markers[0] == Display::Circle && markers[1] == Display::Square) ||
			        (markers[0] == Display::Square && markers[1] == Display::Circle)) {
				ms.push_back(new RSMarker(*marker));
			} else newCompound = true;
		} else newCompound = true;

		if(newCompound) {
			conversionWasNeeded = true;
			Display::Marker nm;
			vector<Display::Marker> nms;
			bool setLabel = true;
			for(unsigned int i = 0; i < markers.size(); i++) {
				nm = markers[i];
				if(nm!=Display::Cross && nm!=Display::X && nm!=Display::Diamond &&
				        nm != Display::Circle && nm != Display::Square) nm = Display::X;
				nms.resize(1);
				nms[0] = nm;
				if(nm == Display::Circle || nm == Display::Square) {
					if((i < markers.size() - 1) && ((nm == Display::Circle &&
					                                 markers[i + 1] == Display::Square) || (nm == Display::Square
					                                         && markers[i + 1] == Display::Circle))) {
						// boxcircle case
						nms.push_back(markers[i + 1]);
						i++;
					}
				}
				m = new RSMarker(props);
				m->setMarkers(nms);
				if(!setLabel) m->setText("");
				ms.push_back(m);
				setLabel = false;
			}
		}

		return ms;
	}

	QString DS9FileWriter::convertValue(double value, const String& toUnits,
	                                    int precision) {
		if(toUnits.empty())
			return QString("%1").arg(value, 0, 'f', precision);
		else if(toUnits == UNIT_IPIXELS)
			return QString("%1" + QString(DS9::FILE_IMAGE_PIXELS.c_str())).arg(
			           value, 0, 'f', precision);

		RSValue from(value, RegionShape::UNIT);
		RSValue to;

		String unit = toUnits;
		if(toUnits == UNIT_DEGREES)      unit = RSValue::DEG;
		else if(toUnits == UNIT_RADIANS) unit = RSValue::RAD;
		else if(toUnits == PUNIT_HMS)    unit = RSValue::HMS;
		else if(toUnits == PUNIT_DMS)    unit = RSValue::DMS;
		else if(toUnits == SUNIT_ARCSEC) unit = RSValue::ARCSEC;
		else if(toUnits == SUNIT_ARCMIN) unit = RSValue::ARCMIN;

		if(!RSValue::convertBetween(from, to, unit))
			return QString("%1").arg(value, 0, 'f', precision);

		stringstream ss;
		if(toUnits == PUNIT_HMS || toUnits == PUNIT_DMS) {
			if(to.hOrD == 0 && to.isMinusZero) ss << '-';
			ss << to.hOrD;
			if(toUnits == PUNIT_HMS) ss << DS9::FILE_HMS_H.c_str();
			else                     ss << DS9::FILE_DMS_D.c_str();
			ss << to.min;
			if(toUnits == PUNIT_HMS) ss << DS9::FILE_HMS_M.c_str();
			else                     ss << DS9::FILE_DMS_M.c_str();
			ss << QString("%1").arg(to.sec, 0, 'f', precision).toStdString();
			if(toUnits == PUNIT_HMS) ss << DS9::FILE_HMS_S.c_str();
			else                     ss << DS9::FILE_DMS_S.c_str();
		} else {
			ss << QString("%1").arg(to.val.getValue(), 0, 'f',
			                        precision).toStdString();

			if(toUnits == UNIT_DEGREES)      ss << DS9::FILE_DEGREES.c_str();
			else if(toUnits == UNIT_RADIANS) ss << DS9::FILE_RADIANS.c_str();
			else if(toUnits == SUNIT_ARCSEC) ss << DS9::FILE_ARCSEC.c_str();
			else if(toUnits == SUNIT_ARCMIN) ss << DS9::FILE_ARCMIN.c_str();
		}

		return QString(ss.str().c_str());
	}


	static stringstream static_errors;
	bool DS9FileWriter::writeHeader( QTextStream &out ) const {
		bool success = true;
		static_errors.str("");
		static_errors.clear( );

		success &= writeHeader(out, static_errors);
		success &= writeGlobals(out, static_errors);
		setError(static_errors.str(), false);
		return success;
	}

	bool DS9FileWriter::writeFooter( QTextStream & ) const {
		return false;
	}

	bool DS9FileWriter::write( QTextStream &, AnnRegion * ) const {
		return false;
	}

}
