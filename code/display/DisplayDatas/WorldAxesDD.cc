//# WorldAxesDD.cc: world axis labelling DisplayData
//# Copyright (C) 2000,2001,2003,2004
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

#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <display/DisplayEvents/WCRefreshEvent.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/WorldAxesDD.h>
#include <display/DisplayDatas/WorldAxesDM.h>
#include <display/Display/DParameterChoice.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MPosition.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	WorldAxesDD::WorldAxesDD() :
		AxesDisplayData() {
		installDefaultOptions();

		Vector<String> vstring;

		// spectral preferences
		vstring.resize(5);
		vstring(0) = "km/s";
		vstring(1) = "m/s";
		vstring(2) = "GHz";
		vstring(3) = "MHz";
		vstring(4) = "Hz";
		itsParamSpectralUnit
		    = new DParameterChoice("spectralunit", "Spectral unit", "",
		                           vstring, vstring(2), vstring(2), "Axis_units");

		vstring.resize(3);
		vstring(0) = "radio";
		vstring(1) = "optical";
		vstring(2) = "true";

		itsParamVelocityType
		    = new DParameterChoice("velocitytype", "Velocity type", "",
		                           vstring, vstring(0), vstring(0), "Axis_units");

	}

	WorldAxesDD::~WorldAxesDD() {
		delete itsParamSpectralUnit;
		delete itsParamVelocityType;
	}

	void WorldAxesDD::setDefaultOptions() {
		AxesDisplayData::setDefaultOptions();
		installDefaultOptions();
	}

	Bool WorldAxesDD::setOptions(Record& rec, Record& recOut) {
		Bool ret = AxesDisplayData::setOptions(rec, recOut);

		Bool localchange = False;

		// spectral preferences

		localchange = (itsParamVelocityType->fromRecord(rec) || localchange);
		localchange = (itsParamSpectralUnit->fromRecord(rec) || localchange);

		return (ret || localchange);
	}

	Record WorldAxesDD::getOptions() {
		Record rec = AxesDisplayData::getOptions();

		// context: Axis_units
		itsParamSpectralUnit->toRecord(rec);
		itsParamVelocityType->toRecord(rec);

		return rec;
	}

	CachingDisplayMethod* WorldAxesDD::newDisplayMethod(
	    WorldCanvas* worldCanvas,
	    AttributeBuffer* wchAttributes,
	    AttributeBuffer* ddAttributes,
	    CachingDisplayData* dd) {
		return new WorldAxesDM(worldCanvas, wchAttributes, ddAttributes, dd);
	}

	AttributeBuffer WorldAxesDD::optionsAsAttributes() {
		AttributeBuffer buffer = AxesDisplayData::optionsAsAttributes();

		// context: Axis_units
		buffer.set(itsParamSpectralUnit->name(), itsParamSpectralUnit->value());
		buffer.set(itsParamVelocityType->name(), itsParamVelocityType->value());

		return buffer;
	}

	WorldAxesDD::WorldAxesDD(const WorldAxesDD &o) : AxesDisplayData(o) {
	}

	void WorldAxesDD::operator=(const WorldAxesDD& ) {
	}

	void WorldAxesDD::installDefaultOptions() {
	}

	String WorldAxesDD::axisText(const WorldCanvas *wc, const uInt axisNo) const {
		//cerr << "WADD::axisText(wc, " << axisNo << ") called." << endl;
		const DisplayCoordinateSystem &wcCS = wc->coordinateSystem();
		//cerr << "wc csys has " << wcCS.nCoordinates() << " coordinates." << endl;
		//cerr << "  of which No = " << wcCS.findCoordinate(Coordinate::SPECTRAL)
		//     << " is spectral. " << endl;

		String base;
		String base0 = wcCS.worldAxisNames()(axisNo);
		String prefUnit = wcCS.worldAxisUnits()(axisNo);

		Int coordinate, axisInCoordinate;

		wcCS.findWorldAxis(coordinate, axisInCoordinate, axisNo);
		Coordinate::Type ctype = wcCS.type(coordinate);

		if (ctype == Coordinate::SPECTRAL) {
			const SpectralCoordinate& dcoord = wcCS.spectralCoordinate(coordinate);

			MFrequency::Types ctype;
			MEpoch epoch;
			MPosition position;
			MDirection direction;

			dcoord.getReferenceConversion(ctype, epoch, position, direction);

			String freqType = MFrequency::showType(ctype);

			Unit pVU(spectralUnit());
			Unit HZ("Hz");
			Unit KMS("km/s");

			if (pVU == KMS) {
				prefUnit = spectralUnit();
				base = freqType + String(" ") + velocityType() + String(" velocity (") +
				       prefUnit + String(")");
			} else if (pVU == HZ) {
				//base = freqType + String(" ") + base0 + String(" (") + prefUnit + String(")");
				base = base0 + String(" (") + spectralUnit() + String(")");
			} else {
				base = freqType + String(" ") + base0 + String(" (") + prefUnit + String(")");
			}
		} else {
			base = "";
		}
		return base;
	}

	String WorldAxesDD::xAxisText(const WorldCanvas* wc) const {
		//cerr << "X Unit = " << wc->coordinateSystem().worldAxisUnits()[0] << "..." << endl;
		//cerr << "spectralUnit() = " << spectralUnit() << endl;
		//if (Unit("Hz") == Unit(spectralUnit())) {
		//  cerr << "spectralUnit() is conformant with Hz." << endl;
		//}
		String tAxisText = axisText(wc, 0);
		if (tAxisText == "") {
			return AxesDisplayData::xAxisText(wc);
		} else {
			return tAxisText;
		}
	}

	String WorldAxesDD::yAxisText(const WorldCanvas* wc) const {
		//cerr << "Y Unit = " << wc->coordinateSystem().worldAxisUnits()[1] << "..." << endl;
		String tAxisText = axisText(wc, 1);
		if (tAxisText == "") {
			return AxesDisplayData::yAxisText(wc);
		} else {
			return tAxisText;
		}
	}



} //# NAMESPACE CASA - END

