//# DSWorldMarker.cc : Implementation of a world coords DSMarker
//# Copyright (C) 1998,1999,2000,2001,2002,2003
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
//# $Id:

#include <display/DisplayShapes/DSWorldMarker.h>
#include <display/DisplayShapes/DSPixelMarker.h>
#include <display/DisplayShapes/DSScreenMarker.h>

#include <casa/Quanta/UnitMap.h>
#include <casa/Containers/List.h>
#include <display/Display/WorldCanvas.h>
#include <casa/Arrays/ArrayMath.h>

#include <display/Display/DisplayCoordinateSystem.h>
namespace casa { //# NAMESPACE CASA - BEGIN

	DSWorldMarker::DSWorldMarker() :
		DSMarker(),
		itsPD(0),
		itsWC(0),
		itsWorldCenter(0)  {

		// Default cons. We know nothing about anything.

	}

	DSWorldMarker::DSWorldMarker(const Record& settings, PanelDisplay* pd) :
		DSMarker(),
		itsPD(pd),
		itsWC(0),
		itsWorldCenter(0) {

		setOptions(settings);

	}

	DSWorldMarker::DSWorldMarker(DSScreenMarker& other, PanelDisplay* pd) :
		DSMarker(),
		itsPD(pd),
		itsWC(0),
		itsWorldCenter(0) {

		Record shapeSettings = other.getRawOptions();
		DSMarker::setOptions(shapeSettings);
		updateWCent();


	}

	DSWorldMarker::DSWorldMarker(DSPixelMarker& other, PanelDisplay* pd) :
		DSMarker(),
		itsPD(pd),
		itsWC(0),
		itsWorldCenter(0) {

		Record shapeSettings = other.getRawOptions();
		DSMarker::setOptions(shapeSettings);
		updateWCent();
	}

	DSWorldMarker::~DSWorldMarker() {

	}

	void DSWorldMarker::recalculateScreenPosition() {

		Vector<Double> world(2);
		world(0) = itsWorldCenter(0).getValue();
		world(1) = itsWorldCenter(1).getValue();
		Vector<Double> pix(2);
		if (itsWC) {
			itsWC->worldToPix(pix, world);
		} else {
			// I don't think this will stay a throw...
			throw(AipsError("DSWorldMarker couldn't update screen position since "
			                "it didn't have a valid worldcanvas"));
		}

		DSMarker::setCenter(Float(pix(0)), Float(pix(1)));

	}


	Bool DSWorldMarker::setOptions(const Record& settings) {
		Bool localChange = False;
		Record toSet = settings;

		if (settings.isDefined("coords")) {
			if (settings.asString("coords") != "world") {
				throw(AipsError("I (DSWorldMarker) was expecting an option record which"
				                " had coords == \'world\'. Please use a \'lock\' "
				                "function"
				                " to change my co-ord system"));
			}
		}

		if (settings.isDefined("center")) {
			if (!itsWC) {

				itsWC = chooseWCFromWorldPoint(settings, itsPD);
				if (!itsWC) {
					throw (AipsError("There was an error constructing a world canvas "
					                 "marker. The supplied record had a \'center\' field,"
					                 " but based on the \'PanelDisplay\' provided, no"
					                 " suitable worldcanvas could be found"));
				}
			}

			if (settings.dataType("center") == TpRecord) {
				localChange = True;
				Record centerField = settings.subRecord("center");

				if (centerField.nfields() != 2) {
					throw(AipsError("Bad record to setoptions() of DSWorldMarker, since "
					                "the "
					                "field \'center\' must be composed of two subfields"
					                " (x, y)"));
				}

				Record x = centerField.subRecord(0);
				Record y = centerField.subRecord(1);

				QuantumHolder xh, yh;
				String error;

				xh.fromRecord(error,x);

				if (error.length() != 0) {
					throw(AipsError("Bad record to setoptions() of DSWorldMarker, since "
					                "extracting the \'x\' (first element) from the center"
					                " field caused an error: " + error));
				}

				yh.fromRecord(error,y);
				if (error.length() != 0) {
					throw(AipsError("Bad record to setoptions() of DSWorldMarker, since "
					                "extracting the \'y\' (second element) from the center"
					                " field caused an error: " + error));
				}


				// TODO are units compatible with current CS units?

				itsWorldCenter.resize(2);
				itsWorldCenter(0) = xh.asQuantumDouble();
				itsWorldCenter(1) = yh.asQuantumDouble();

				toSet.removeField("center");
			} else {
				throw(AipsError("Bad data type for setOptions, field \'center\'"));
			}
		}

		if (localChange) {
			recalculateScreenPosition();
		}

		if (DSMarker::setOptions(toSet)) {
			localChange = True;
		}

		return localChange;
	}

	Record DSWorldMarker::getOptions() {
		Record toReturn;
		toReturn = DSMarker::getOptions();

		if (toReturn.isDefined("center")) {
			toReturn.removeField("center");

			if (itsWC) {
				QuantumHolder x(itsWorldCenter(0));
				QuantumHolder y(itsWorldCenter(1));
				Record subX, subY;
				String error;
				x.toRecord(error, subX);

				if (error.length() != 0) {
					throw(AipsError("Problem outputting center (x value)"));
				}

				y.toRecord(error, subY);

				if (error.length() != 0) {
					throw(AipsError("Problem outputting center (y value)"));
				}

				Record subCenter;
				subCenter.defineRecord("x", subX);
				subCenter.defineRecord("y", subY);

				toReturn.defineRecord("center", subCenter);
			} else {
				throw(AipsError("Problem during getoptions. Center is defined, but no"
				                " valid WC. That should never happen"));
			}
		}

		// Shouldn't happen (should never be defined) .. but why not
		if (toReturn.isDefined("coords")) {
			toReturn.removeField("coords");
		}
		toReturn.define("coords", "world");

		return toReturn;
	}

	void DSWorldMarker::draw(PixelCanvas* pc) {
		if (itsWC) {
			Vector<Float> cent = getCenter();

			if ( itsWC->inDrawArea(Int(cent(0)+0.5), Int(cent(1)+0.5)) ) {
				DSMarker::draw(pc);
			}

		}
	}


	void DSWorldMarker::updateWCent() {
		Vector<Float> pixelCenter = DSMarker::getCenter();

		if (pixelCenter.nelements() != 2) {
			throw(AipsError("Error updating world co-ordinates - the pixelCenter"
			                " I return was invalid"));
		}

		if (!itsWC) {

			itsWC = chooseWCFromPixPoint(pixelCenter(0), pixelCenter(1), itsPD);
			if (!itsWC) {
				throw(AipsError("There was an error constructing a world canvas "
				                "marker. The first time I got information about "
				                "the center"
				                " of the shape, I couldn't find an appropriate "
				                "WorldCanvas (based on the supplied PanelDisplay)"
				                " to lock it to!"));
			}
		}

		const Vector<String>& units = itsWC->coordinateSystem().worldAxisUnits();

		Vector<Double> dblpix(pixelCenter.nelements());
		convertArray(dblpix, pixelCenter);
		Vector<Double> world;

		Bool suc = itsWC->pixToWorld(world, dblpix);

		if (!suc) {
			throw(AipsError("Had a problem going pix->world in WCMarker."));
		}

		if (world.nelements() != 2) {
			throw(AipsError("I (DSWorldMarker) am not yet sure how to handle "
			                "a conversion to world co-ords if I get returned !=2"
			                " co-ordinates. Sorry"));
		}

		Quantity x(world(0), units(0));
		Quantity y(world(1), units(1));

		itsWorldCenter.resize(2);
		itsWorldCenter(0) = x;
		itsWorldCenter(1) = y;

	}


	void DSWorldMarker::move(const Float& dX, const Float& dY) {
		DSMarker::move(dX, dY);
		updateWCent();
	}

	void DSWorldMarker::setCenter(const Float& xPos, const Float& yPos) {
		if (!itsWC && itsPD) {

			// We haven't yet selected a WC - probably because we didn't know
			// the center yet
			DSMarker::setCenter(xPos, yPos);

			itsWC = chooseWCFromPixPoint(xPos, yPos, itsPD);
			if (!itsWC) {
				throw(AipsError("There was an error constructing a world canvas "
				                "marker. The first time I got information about "
				                "the center"
				                " of the shape, I couldn't find an appropriate "
				                "WorldCanvas (based on the supplied PanelDisplay)"
				                " to lock it to!"));

			} else {

				updateWCent();
			}
		} else {
			DSMarker::setCenter(xPos, yPos);
			updateWCent();
		}

	}



} //# NAMESPACE CASA - END

