//# MWCPositionVelocityTool.cc: MultiWorldCanvas position velocity tool
//# Copyright (C) 2012
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

#include <iostream>
#include <iomanip>
#include <casa/Arrays/ArrayMath.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/MWCPositionVelocityTool.h>
#include <casa/OS/Time.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	MWCPositionVelocityTool::MWCPositionVelocityTool( Display::KeySym keysym, Bool ) :
		MultiWCTool(keysym), itsActive(False), itsEqualUnits(False), itsRaIndex(-1), itsDecIndex(-1) {
	}

	MWCPositionVelocityTool::~MWCPositionVelocityTool() {
		reset();
	}

	void MWCPositionVelocityTool::keyPressed(const WCPositionEvent &ev) {
		itsX1=itsX2=ev.pixX();
		itsY1=itsY2=ev.pixY();
		itsCurrentWC = ev.worldCanvas();
		itsActive = True;
		itsRaIndex  = -1;
		itsDecIndex = -1;
		itsEqualUnits=True;

		// extract the axis names and units
		Vector<String> aXisNames=itsCurrentWC->worldAxisNames();
		Vector<String> unitNames=itsCurrentWC->worldAxisUnits();

		// identify RA and DEC axis
		for (Int index=0; index < (Int)aXisNames.size(); index++) {
			if (aXisNames(index).contains("scension") && (index < 2))
				itsRaIndex=index;
			if (aXisNames(index).contains("eclination") && (index < 2))
				itsDecIndex=index;
		}

		// check for equal units
		if (unitNames.size()>1) {
			if (!unitNames(0).compare(unitNames(1)) && unitNames(0).size()>0)
				itsEqualUnits=True;
			else
				itsEqualUnits=False;
		}
	}

	void MWCPositionVelocityTool::moved(const WCMotionEvent &ev, const viewer::region::region_list_type & /*selected_regions*/) {
		if (!itsActive) return;
		itsX2 = ev.pixX();
		itsY2 = ev.pixY();
		refresh();
	}

	void MWCPositionVelocityTool::keyReleased(const WCPositionEvent &ev) {
		if(!itsActive) return;
		if(itsCurrentWC!=ev.worldCanvas()) {
			refresh();
			return;
		}

		reset();
	}

	void MWCPositionVelocityTool::draw(const WCRefreshEvent&/*ev*/, const viewer::region::region_list_type & /*selected_regions*/) {
		// draws a line from initial to final mouse position,
		// when the button is down.
		if (!itsActive) return;
		PixelCanvas *pCanvas = itsCurrentWC->pixelCanvas();
		if(!pCanvas) return;

		Vector<Double> world1(2), world2(2);
		Vector<Double> pix1(2),   pix2(2);
		Vector<Double> diff(2);
		Double allDiff;
		String unit("");

		// get the position of the start- and end-points -- we will be rotating the cube about the mid-point of the line
		pix1(0) = (Double)itsX1;
		pix1(1) = (Double)itsY1;
		pix2(0) = (Double)itsX2;
		pix2(1) = (Double)itsY2;

		//std::cout << "draw\t\t" << pix1 << ", " << pix2 << std::endl;

		// determine the positions in world coordinates
		if ( ! itsCurrentWC->pixToWorld(world1, pix1) || !itsCurrentWC->pixToWorld(world2, pix2)) {

			// if one of the positions could
			// NOT be determined, just draw a line between
			// the start and end point
			pCanvas->setColor(drawColor());
			pCanvas->setDrawFunction(Display::DFCopy);
			pCanvas->setLineWidth(2);
			pCanvas->drawLine(itsX1, itsY1, itsX2, itsY2);
			return;
		}

		// pixToWorld sometimes seems to add a dimension
		Vector<Double> world3(world2.size(), 0.0);
		Vector<Double>   pix3(world2.size(), 0.0);

		// get the corner point in world-coordinates
		world3(0) = world1(0);
		world3(1) = world2(1);
		for (unsigned int index=2; index<world2.size(); index++)
			world3(index) = world2(index);

		// get the corner point in pixel-coordinates
		if (!itsCurrentWC->worldToPix(pix3, world3)) {

			// if one the third positions could
			// NOT be determined, just draw a line between
			// the start and end point
			pCanvas->setColor(drawColor());
			pCanvas->setDrawFunction(Display::DFCopy);
			pCanvas->setLineWidth(2);
			pCanvas->drawLine(itsX1, itsY1, itsX2, itsY2);
			return;
		}

		if (fabs(pix3(0))>10000.0 || fabs(pix3(1))>10000.0) {
			// if one the third positions could
			// NOT be determined, just draw a line between
			// the start and end point
			pCanvas->setColor(drawColor());
			pCanvas->setDrawFunction(Display::DFCopy);
			pCanvas->setLineWidth(2);
			pCanvas->drawLine(itsX1, itsY1, itsX2, itsY2);
			return;
		}

		itsX3=Int(pix3(0)+0.5);
		itsY3=Int(pix3(1)+0.5);

		// compute the differences;
		// correct for declination and convert
		// to arcsec if possible
		String dText;
		diff(0) = fabs(world1(0)-world2(0));
		diff(1) = fabs(world1(1)-world2(1));
		if (itsRaIndex > -1 && itsDecIndex > -1) {
			diff(0) = diff(0)*3600.0*180.0/C::pi;
			diff(1) = diff(1)*3600.0*180.0/C::pi;
			diff(itsRaIndex) = diff(itsRaIndex) * cos(world3(itsDecIndex));
			unit = "\"";
		}

		// prepare the drawing
		pCanvas->setColor(drawColor());
		pCanvas->setDrawFunction(Display::DFCopy);
		pCanvas->setLineWidth(1);

		// draw the corner lines
		pCanvas->drawLine(itsX1, itsY1, itsX3, itsY3);
		pCanvas->drawLine(itsX2, itsY2, itsX3, itsY3);

		// write the distance one corner line
		Int tPosX, tPosY, dX, dY;
		Double angle1;
		Double dist1, dist2;
		ostringstream tt, uu;
		tPosX = Int((itsX1+itsX3)/2.0+0.5);
		tPosY = Int((itsY1+itsY3)/2.0+0.5);
		tt << std::setiosflags(ios::scientific) << std::setiosflags(ios::fixed) << std::setprecision(4) << diff(1) << unit;
		dText=String(tt.str());
		if (abs(Double(itsX3-itsX1))>0.1)
			angle1 = C::pi/2.0+atan(Double(itsY3-itsY1)/Double(itsX3-itsX1));
		else
			angle1=0.0;
		//cout << "Angle1: "<< (angle1/C::pi*180.0) << endl;
		dX=Int(20.0*cos(angle1)+0.5);
		dY=Int(20.0*sin(angle1)+0.5);
		dist1 = (tPosX-dX-itsX2)*(tPosX-dX-itsX2) + (tPosY-dY-itsY2)*(tPosY-dY-itsY2);
		dist2 = (tPosX+dX-itsX2)*(tPosX+dX-itsX2) + (tPosY+dY-itsY2)*(tPosY+dY-itsY2);
		if (dist1 < dist2) {
			tPosX += dX;
			tPosY += dY;
		} else {
			tPosX -= dX;
			tPosY -= dY;
		}
		pCanvas->drawText(tPosX, tPosY, dText, Display::AlignCenter);

		// write the distance to the second corner line
		tPosX = Int((itsX2+itsX3)/2.0+0.5);
		tPosY = Int((itsY2+itsY3)/2.0+0.5);
		uu << std::setiosflags(ios::scientific) << std::setiosflags(ios::fixed) << std::setprecision(4) << diff(0) << unit;
		dText=String(uu.str());
		if (abs(Double(itsX3-itsX2))>0.1)
			angle1 = C::pi/2.0+atan(Double(itsY3-itsY2)/Double(itsX3-itsX2));
		else
			angle1=0.0;
		//cout << "Angle2: "<< (angle1/C::pi*180.0) << endl;
		dX=Int(20.0*cos(angle1)+0.5);
		dY=Int(20.0*sin(angle1)+0.5);
		dist1 = (tPosX-dX-itsX1)*(tPosX-dX-itsX1) + (tPosY-dY-itsY1)*(tPosY-dY-itsY1);
		dist2 = (tPosX+dX-itsX1)*(tPosX+dX-itsX1) + (tPosY+dY-itsY1)*(tPosY+dY-itsY1);
		if (dist1 < dist2) {
			tPosX += dX;
			tPosY += dY;
		} else {
			tPosX -= dX;
			tPosY -= dY;
		}
		pCanvas->drawText(tPosX, tPosY, dText, Display::AlignCenter);

		// check whether the units in x and y are the same
		if (itsEqualUnits) {
			// draw the direct line between the start and the end point
			// compute write the total distance to the line
			allDiff = sqrt(diff(0)*diff(0) + diff(1)*diff(1));

			pCanvas->setLineWidth(2);
			pCanvas->drawLine(itsX1, itsY1, itsX2, itsY2);

			ostringstream ss;
			tPosX = Int((itsX1+itsX2)/2.0+0.5);
			tPosY = Int((itsY1+itsY2)/2.0+0.5);
			ss <<  std::setiosflags(ios::scientific) << std::setiosflags(ios::fixed) << std::setprecision(4) << allDiff << unit;
			dText=String(ss.str());
			if (abs(Double(itsX2-itsX1))>0.1)
				angle1 = C::pi/2.0+atan(Double(itsY2-itsY1)/Double(itsX2-itsX1));
			else
				angle1=0.0;
			//cout << "Angle3: "<< (angle1/C::pi*180.0) << endl;
			dX=Int(20.0*cos(angle1)+0.5);
			dY=Int(20.0*sin(angle1)+0.5);
			dist1 = (tPosX-dX-itsX3)*(tPosX-dX-itsX3) + (tPosY-dY-itsY3)*(tPosY-dY-itsY3);
			dist2 = (tPosX+dX-itsX3)*(tPosX+dX-itsX3) + (tPosY+dY-itsY3)*(tPosY+dY-itsY3);
			if (dist1 < dist2) {
				tPosX += dX;
				tPosY += dY;
			} else {
				tPosX -= dX;
				tPosY -= dY;
			}
			pCanvas->drawText(tPosX, tPosY, dText, Display::AlignCenter);
		}
	}

	void MWCPositionVelocityTool::reset(Bool skipRefresh) {
		Bool wasActive=itsActive;
		itsActive=False;	// erases pan line (if any) on refresh
		if(wasActive && !skipRefresh) refresh();
	}

} //# NAMESPACE CASA - END

