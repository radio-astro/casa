//# NBody.cc: class which provides brute force n-body simulation and display
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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
#include <display/Display/AttValBase.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Exceptions/Error.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/WCPositionEvent.h>
#include <display/DisplayEvents/WCMotionEvent.h>
#include <display/DisplayEvents/WCRefreshEvent.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/NBody.h>
#include <display/DisplayDatas/Galaxy.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	NBody::NBody() {
		dataMin = 0.0;
		dataMax = 1.0;
		turnedOff = False;
		itsTimeStep = 2.0;
		itsDampingFactor = 0.99;
		itsNumSteps = 10;
		itsXSize = 400;
		itsYSize = 400;
		itsGalaxyListIter = new ListIter<void *> (&itsGalaxyList);
		setupStars();
	}

	NBody::~NBody() {
		cleanupStars();
		delete itsGalaxyListIter;
	}

	Vector<String> NBody::worldAxisNames() const {
		Vector<String> tmpVec;
		if (!turnedOff) {
			tmpVec.resize(2);
			tmpVec(0) = "X";
			tmpVec(1) = "Y";
		}
		return tmpVec;
	}

	Vector<String> NBody::worldAxisUnits() const {
		Vector<String> tmpVec;
		if (!turnedOff) {
			tmpVec.resize(2);
			tmpVec(0) = "cm";
			tmpVec(1) = "cm";
		}
		return tmpVec;
	}

	const Unit NBody::dataUnit() {
		Unit bla;
		return bla;
	}

	const RecordInterface& NBody::miscInfo() {
		return miscInfoDummy;
	}

	Bool NBody::linToWorld(Vector<Double>& world,
	                       const Vector<Double>& lin) {
		world.resize(lin.shape()(0));
		world = lin;
		return True;
	}

	Bool NBody::worldToLin(Vector<Double>& lin,
	                       const Vector<Double>& world) {
		lin.resize(world.shape()(0));
		lin = world;
		return True;
	}

	void  NBody::positionEH(const WCPositionEvent& ev) {
		WorldCanvas *wCanvas = ev.worldCanvas();

		if (!ev.keystate())
			return;

		// listen to certain keystrokes and mouse clicks
		switch ( ev.key() ) {

			// default is to do nothing
		case Display::K_q : {
			exit(0);
			break;
		}
		default:
			break;
		case Display::K_f : {
			drawMovie(*wCanvas, itsNumSteps, itsTimeStep, itsDampingFactor);
			break;
		}
		case Display::K_b : {
			drawMovie(*wCanvas, itsNumSteps, -itsTimeStep, 1.0/itsDampingFactor);
			break;
		}
		case Display::K_a : {
			drawMovie(*wCanvas, 10*itsNumSteps, itsTimeStep, itsDampingFactor);
			break;
		}
		case Display::K_d : {
			while (True) {
				drawMovie(*wCanvas, 10*itsNumSteps, itsTimeStep, itsDampingFactor);
				drawMovie(*wCanvas, 10*itsNumSteps, itsTimeStep, itsDampingFactor);
				drawMovie(*wCanvas, 10*itsNumSteps, itsTimeStep, itsDampingFactor);
				drawMovie(*wCanvas, 10*itsNumSteps, itsTimeStep, itsDampingFactor);
				drawImage(*wCanvas);
				cleanupStars();
				setupStars();
				break;
			}
		}
		case Display::K_p : {
			Galaxy *galaxy;
			itsGalaxyListIter->toStart();
			while ( !itsGalaxyListIter->atEnd() ) {
				galaxy = (Galaxy *) itsGalaxyListIter->getRight();
				galaxy->setPlotMode(Galaxy::POSITION);
				(*itsGalaxyListIter)++;
			}
			drawImage(*wCanvas);
			drawMovie(*wCanvas, 1, 0.0, 1.0);
			break;
		}
		case Display::K_v : {
			Galaxy *galaxy;
			itsGalaxyListIter->toStart();
			while ( !itsGalaxyListIter->atEnd() ) {
				galaxy = (Galaxy *) itsGalaxyListIter->getRight();
				galaxy->setPlotMode(Galaxy::VELOCITY);
				(*itsGalaxyListIter)++;
			}
			drawImage(*wCanvas);
			drawMovie(*wCanvas, 1, 0.0, 1.0);
			break;
		}
		case Display::K_Left : {
			Matrix<Double> rotMatrix(3,3);
			rotMatrix = Rot3D(1, C::pi/10.0);

			Galaxy *galaxy;
			itsGalaxyListIter->toStart();
			while ( !itsGalaxyListIter->atEnd() ) {
				galaxy = (Galaxy *) itsGalaxyListIter->getRight();
				galaxy->rotate(rotMatrix);
				(*itsGalaxyListIter)++;
			}
			drawMovie(*wCanvas, 1, 0.0, 1.0);
			break;
		}
		case Display::K_Right : {
			Matrix<Double> rotMatrix(3,3);
			rotMatrix = Rot3D(1, -C::pi/10.0);

			Galaxy *galaxy;
			itsGalaxyListIter->toStart();
			while ( !itsGalaxyListIter->atEnd() ) {
				galaxy = (Galaxy *) itsGalaxyListIter->getRight();
				galaxy->rotate(rotMatrix);
				(*itsGalaxyListIter)++;
			}
			drawMovie(*wCanvas, 1, 0.0, 1.0);
			break;
		}
		case Display::K_Up : {
			Matrix<Double> rotMatrix(3,3);
			rotMatrix = Rot3D(0, C::pi/10.0);

			Galaxy *galaxy;
			itsGalaxyListIter->toStart();
			while ( !itsGalaxyListIter->atEnd() ) {
				galaxy = (Galaxy *) itsGalaxyListIter->getRight();
				galaxy->rotate(rotMatrix);
				(*itsGalaxyListIter)++;
			}
			drawMovie(*wCanvas, 1, 0.0, 1.0);
			break;
		}
		case Display::K_Down : {
			Matrix<Double> rotMatrix(3,3);
			rotMatrix = Rot3D(0, -C::pi/10.0);

			Galaxy *galaxy;
			itsGalaxyListIter->toStart();
			while ( !itsGalaxyListIter->atEnd() ) {
				galaxy = (Galaxy *) itsGalaxyListIter->getRight();
				galaxy->rotate(rotMatrix);
				(*itsGalaxyListIter)++;
			}
			drawMovie(*wCanvas, 1, 0.0, 1.0);
			break;
		}
		case Display::K_r : {
			drawImage(*wCanvas);
			cleanupStars();
			setupStars();

			drawMovie(*wCanvas, 1, 0.0, 1.0);
			break;
		}
		} // of switch
	}

	void  NBody::motionEH(const WCMotionEvent &) {
	}

// refresh event handler used by WorldCanvasHolder. The event gets distributed
// over the eventhandlers installed on this displaydata. I am not sure
// it is necessary that refreshhandlers  can be installed on displaydatas, but
// atleast now all the events can be caught on a DisplayData. Contrary to
// position- and motion event, the refresh event distributed is identical to
// the one generated by the WorldCanvas and no restirction on the type of the
// event handler exists. After calling handler installed on the
// ImageDisplayData, the actual refresh of the WorldCanvasis done
	void  NBody::refreshEH(const WCRefreshEvent& ev) {
		WorldCanvas *wCanvas = ev.worldCanvas();
		PixelCanvas *pCanvas = wCanvas->pixelCanvas();
		itsXSize = pCanvas->width();
		itsYSize = pCanvas->height();
		setScale();
		drawImage(*wCanvas);
		drawMovie(*wCanvas, 1, 0.0, 1.0);
	}

	Bool NBody::sizeControl(WorldCanvasHolder& wcHolder,
	                        AttributeBuffer& holderBuf) {

		// Attributes have already been set by another DisplayData in this
		// sizeControl
		if ( holderBuf.exists("linXMin") ||
		        holderBuf.exists("linXMax") ||
		        holderBuf.exists("linYMin") ||
		        holderBuf.exists("linYMax") ||
		        holderBuf.exists("linXMinLimit") ||
		        holderBuf.exists("linXMaxLimit") ||
		        holderBuf.exists("linYMinLimit") ||
		        holderBuf.exists("linYMaxLimit") ||
		        holderBuf.exists("dataMin") ||
		        holderBuf.exists("dataMax") ) {
			// should not respond to requests
			turnedOff = True;
			return True;
		}

		WorldCanvas *wCanvas = wcHolder.worldCanvas();

		//  Bool integerOverSampling;
		//  Bool keepAspectInResample;
		//  wCanvas->getAttributeValue("integerOverSampling", integerOverSampling);
		//  wCanvas->getAttributeValue("keepAspectInResample", keepAspectInResample);
		Int linXMin = -1;
		Int linYMin = -1;
		Int linXMax = 1;
		Int linYMax = 1;
		Attribute resetAtt("resetCoordinates", False);
		wCanvas->setAttribute(resetAtt);


		holderBuf.add("linXMin", Double(linXMin));
		holderBuf.add("linXMax", Double(linXMax));
		holderBuf.add("linYMin", Double(linYMin));
		holderBuf.add("linYMax", Double(linYMax));
		holderBuf.add("linXMinLimit", Double(linXMin));
		holderBuf.add("linXMaxLimit", Double(linXMax));
		holderBuf.add("linYMinLimit", Double(linYMin));
		holderBuf.add("linYMaxLimit", Double(linYMax));

		// the min and max on the worldCanvas are always Double
		holderBuf.add("dataMin", Double(0));
		holderBuf.add("dataMax", Double(1));

		// we store the sizeControl state so we can check in the refreshEH if the
		// refreshEH can assume that the sizeControl has been done by this
		// ImageDisplayData.
		sizeControlBuf = holderBuf;

		return True;
	}

	void NBody::cleanup() {
	}

	Bool NBody::iDidSizeControl(WorldCanvas& wCanvas) {
		//turned off is set during sizeControl
		if (turnedOff) {
			return False;
		}

		AttributeBuffer wCanvasState;
		wCanvasState.add("linXMin", wCanvas.linXMin());
		wCanvasState.add("linXMax", wCanvas.linXMax());
		wCanvasState.add("linYMin", wCanvas.linYMin());
		wCanvasState.add("linYMax", wCanvas.linYMax());

		wCanvasState.add("linXMinLimit", wCanvas.linXMinLimit());
		wCanvasState.add("linXMaxLimit", wCanvas.linXMaxLimit());
		wCanvasState.add("linYMinLimit", wCanvas.linYMinLimit());
		wCanvasState.add("linYMaxLimit", wCanvas.linYMaxLimit());

		wCanvasState.add("dataMin", wCanvas.dataMin());
		wCanvasState.add("dataMax", wCanvas.dataMax());

		return wCanvasState.matches(sizeControlBuf);
	}

	Double NBody::getDataMin() {
		return dataMin;
	}

	Double NBody::getDataMax() {
		return dataMax;
	}

	uInt NBody::nelements(const WorldCanvasHolder &) const {
		return 1;
	}

	uInt NBody::nelements() const {
		return 1;
	}

// Set a restriction on an element
	void NBody::addElementRestriction(const uInt, Attribute &, Bool) {
	}

	AttributeBuffer *NBody::elementRestrictionBuffer(const uInt) {
		throw (AipsError("ImageDisplayData::elementRestrictionBuffer -"
		                 "index out of range"));
		return 0;
	}

// Set restrictions on an element.
	void NBody::addElementRestrictions(const uInt, AttributeBuffer &) {
	}


// Set a restriction on an element.
	void NBody::setElementRestriction(const uInt, Attribute &) {
	}

// Set restrictions on an element.
	void NBody::setElementRestrictions(const uInt, AttributeBuffer &) {
	}

	void NBody::removeElementRestriction(const uInt, const String &) {
	}

	Bool NBody::existElementRestriction(const uInt, const String &) {
		return False;
	}

	void  NBody::clearElementRestrictions(const uInt) {
	}

	void NBody::drawMovie(WorldCanvas& wCanvas, Int numSteps, Double timeStep,
	                      Double dampingFactor) {

		PixelCanvas *pCanvas = wCanvas.pixelCanvas();

//  pCanvas->setDrawFunction(Display::DFInvert);
		pCanvas->setDrawFunction(Display::DFXor);
		pCanvas->setColor("yellow");

		Galaxy *galaxy;
		for (Int i = 0; i < numSteps; i++) {
			// compute new position and velocity for all Galaxies
			itsGalaxyListIter->toStart();
			while ( !itsGalaxyListIter->atEnd() ) {
				galaxy = (Galaxy *) itsGalaxyListIter->getRight();
				galaxy->computeStep(itsGalaxyList, timeStep, dampingFactor);
				(*itsGalaxyListIter)++;
			}
			// apply new positions and velocity
			itsGalaxyListIter->toStart();
			while ( !itsGalaxyListIter->atEnd() ) {
				galaxy = (Galaxy *) itsGalaxyListIter->getRight();
				galaxy->update();
				(*itsGalaxyListIter)++;
			}
			// compute new positions and velocities for stars
			itsGalaxyListIter->toStart();
			while ( !itsGalaxyListIter->atEnd() ) {
				galaxy = (Galaxy *) itsGalaxyListIter->getRight();
				galaxy->applyForceToStars(itsGalaxyList, timeStep, dampingFactor);
				galaxy->draw(pCanvas);

				(*itsGalaxyListIter)++;
			}
		}
		pCanvas->setDrawFunction(Display::DFCopy);
	}

	void NBody::setupStars() {
		Vector<Double> galPos(3);
		galPos = 0.0;
		galPos(0) = -65.0/400.0*itsXSize;

		Vector<Double> galVel(3);
		galVel = 0.0;
		galVel(0) = 0.5;
		galVel(1) = -1.0;

		Galaxy *galaxy;
		galaxy = new Galaxy(galPos, galVel, 1.0, 8.0, 5, 5, C::pi/3.0, C::pi/2.0,
		                    itsXSize, itsYSize);
		itsGalaxyListIter->addRight((void *) galaxy);

		galPos = Double(-1.0)*(Array<Double>)galPos;
		galVel = Double(-1.0)*(Array<Double>)galVel;
		galaxy = new Galaxy(galPos, galVel, 1.0, 8.0, 5, 5, C::pi/3.0,
		                    C::pi/2.0, itsXSize, itsYSize);
		itsGalaxyListIter->addRight((void *) galaxy);
	}

	void NBody::cleanupStars() {
		Galaxy *galaxy;

		itsGalaxyListIter->toStart();
		while ( !itsGalaxyListIter->atEnd() ) {
			galaxy = (Galaxy *) itsGalaxyListIter->getRight();
			itsGalaxyListIter->removeRight();
			delete galaxy;
		}
	}

	void NBody::drawImage(WorldCanvas& wCanvas) {

		wCanvas.clear();

		Matrix<Float> dummyMat(10,10);

		dummyMat = 0.15;

		Vector<Double> blc(2);
		blc(0) = blc(1) = -1.0;

		Vector<Double> trc(2);
		trc(0) = trc(1) = 1.0;

		wCanvas.drawImage(blc, trc, dummyMat);
	}

	void NBody::setScale() {
		Galaxy *galaxy;

		itsGalaxyListIter->toStart();
		while ( !itsGalaxyListIter->atEnd() ) {
			galaxy = (Galaxy *) itsGalaxyListIter->getRight();
			galaxy->setScale(itsXSize, itsYSize);
			(*itsGalaxyListIter)++;
		}
	}

} //# NAMESPACE CASA - END

