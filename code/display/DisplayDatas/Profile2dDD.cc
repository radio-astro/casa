//# Profile2dDD.cc: 2d Profile DisplayData
//# Copyright (C) 2003,2004
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
#include <casa/BasicMath/Math.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Slicer.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/MaskedLattice.h>
#include <casa/BasicSL/String.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <lattices/Lattices/LatticeStatistics.h>
#include <display/DisplayDatas/LatticePADD.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/Profile2dDD.h>
#include <display/DisplayDatas/Profile2dDM.h>
#include <display/DisplayEvents/CrosshairEvent.h>
#include <display/DisplayEvents/RectRegionEvent.h>
#include <display/DisplayEvents/DDModEvent.h>
#include <measures/Measures/MeasTable.h>

#include <iostream>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

// Default construtor
	Profile2dDD::Profile2dDD() :
		itsDD(0),
		itsIsRegionProfile(False),
		itsTrackingState(True),
		itsDependentAxis(-1),
		itsYAxisInc(1),
		itsData(),
		itsMask(),
		itsParamColor(0),
		itsParamLineWidth(0),
		itsParamLineStyle(0),
		itsParamAutoscale(0),
		itsParamShowRestFrequency(0),
		itsParamRegionStatType(0),
		itsParamRegionXRadius(0),
		itsParamRegionYRadius(0) {
		setCaching(False);
		itsCurrentBlc.resize(2);
		itsCurrentTrc.resize(2);
		itsPixelPosition.resize(3);
		itsWorldPosition.resize(3);
		itsDefaultAxisLabels.resize(2);
		constructParameters();
	}

	Profile2dDD::Profile2dDD(LatticePADisplayData<Float>* pDD) :
		itsDD(0),
		itsIsRegionProfile(False),
		itsTrackingState(True),
		itsDependentAxis(-1),
		itsYAxisInc(1),
		itsData(),
		itsMask(),
		itsParamColor(0),
		itsParamLineWidth(0),
		itsParamLineStyle(0),
		itsParamAutoscale(0),
		itsParamShowRestFrequency(0),
		itsParamRegionStatType(0),
		itsParamRegionXRadius(0),
		itsParamRegionYRadius(0) {
		setCaching(False);
		itsCurrentBlc.resize(2);
		itsCurrentTrc.resize(2);
		itsPixelPosition.resize(3);
		itsWorldPosition.resize(3);
		itsDefaultAxisLabels.resize(2);
		constructParameters();

		// attach the data to this profile2dDD
		attachDD(pDD);
	}

	Profile2dDD::~Profile2dDD() {
		destructParameters();
		detachDD();
	}

	Bool Profile2dDD::attachDD(LatticePADisplayData<Float>* pDD) {
		// Attach a Display Data to this Profile2dDD
		if (itsDD) {
			throw(AipsError("Profile2dDD::attachDD - A Display Data is already attached"));
			return False;
		}

		// check data shape
		if (pDD->coordinateSystem().nWorldAxes() < 3) {
			throw(AipsError("Profile2dDD::attachDD - data has less then three axes"));
			return False;
		}
		if (pDD->nelements() < 2) {
			throw(AipsError("Profile2dDD::attachDD - data has less than 2 pixels on profile axis"));
			return False;
		}
		// check for unsupported coordinate types
		Int coordNum, axisInCoordinate;
		pDD->coordinateSystem().findWorldAxis(coordNum, axisInCoordinate, 2);
		if (pDD->coordinateSystem().type(coordNum) != Coordinate::SPECTRAL &&
		        pDD->coordinateSystem().type(coordNum) != Coordinate::DIRECTION &&
		        pDD->coordinateSystem().type(coordNum) != Coordinate::STOKES ) {
			//      pDD->coordinateSystem().type(coordNum) != Coordinate::LINEAR &&
			//      pDD->coordinateSystem().type(coordNum) != Coordinate::TABULAR) {
			throw(AipsError("Profile2dDD::attachDD - Coordinate type of profile axis not supported"));
			return False;
		}
		// Integrity checks complete
		itsDD = pDD;

		// set the increment value for the coordinate on Y Axis
		Double range(itsDD->getDataMax() - itsDD->getDataMin());
		// this creates 10000 'steps' between data min and data max
		itsYAxisInc = range/10000;

		// create new coordinate system for this profile2dDD
		createCoordinateSystem();

		// Get hold of parent ML (do not delete pointer)
		std::tr1::shared_ptr<const MaskedLattice<Float> > pLat = itsDD->maskedLattice();

		// set min and max X values for this DisplayData
		itsCurrentBlc[0] = 0;
		itsCurrentTrc[0] = Double(pLat->shape()(profileAxis()));
		// set min and max Y values for this DisplayData
		itsCurrentBlc[1] = itsDD->getDataMin();
		itsCurrentTrc[1] = itsDD->getDataMax();

		// set min and max RegionRadius values.
		itsParamRegionXRadius->setMinimum(0);
		itsParamRegionYRadius->setMinimum(0);
		itsParamRegionXRadius->setDefaultValue(0);
		itsParamRegionYRadius->setDefaultValue(0);
		itsParamRegionXRadius->setMaximum(pLat->shape()(itsDD->displayAxes()[0]));
		itsParamRegionYRadius->setMaximum(pLat->shape()(itsDD->displayAxes()[1]));

		// Attach Profile2dDD as an event handler to the input
		// DisplayData itsDD
		itsDD->addMotionEventHandler(static_cast<WCMotionEH*>(this));
		itsDD->addPositionEventHandler(static_cast<WCPositionEH*>(this));
		// listen to DisplayEvents (eg, RectRegionEvent)
		itsDD->addDisplayEventHandler(static_cast<DisplayEH*>(this));

		return True;
	}

	void Profile2dDD::detachDD() {
		if (!itsDD) {
			throw(AipsError("Profile2dDD::dettachDD - No Display Data attached"));
		}
		itsDD->removeMotionEventHandler(*static_cast<WCMotionEH*>(this));
		itsDD->removePositionEventHandler(*static_cast<WCPositionEH*>(this));
		itsDD->removeDisplayEventHandler(*static_cast<DisplayEH*>(this));
		itsDD = 0;
		itsData = 0;
		itsMask = 0;
	}

	Bool Profile2dDD::createCoordinateSystem()
//
// We are going to construct a DisplayCoordinateSystem for this
// DD.  It holds a coordinate for the profile axis (X on plot)
// and a LinearCoordinate for the intensity (Y on plot).
// It may hold extra coordinates if available (e.g. a SpectralCoordinate
// needs a DirectionCoordinate for frame conversions).
//
	{

// Retrieve CoordinateSystem from parent CS

		const DisplayCoordinateSystem& cSysDD(itsDD->coordinateSystem());

// Make empty output CS

		DisplayCoordinateSystem cSysOut;

// Copy the ObsInfo

		cSysOut.setObsInfo (cSysDD.obsInfo());

// Retrieve profile (pixel) axis (2) from input data and add its coordinate to the output CS

		const Int profilePixelAxis = 2;
		Int profileCoordNum, axisInProfileCoordinate;
		cSysDD.findPixelAxis(profileCoordNum, axisInProfileCoordinate, profilePixelAxis);
		if (profileCoordNum==-1) {                                                   // Should not happen
			throw(AipsError("Profile2dDD::init - pixel axis 2 does not exist"));
		}
		if (cSysDD.type(profileCoordNum)==Coordinate::DIRECTION) {
			Vector<Int> pixelAxes = cSysDD.pixelAxes(profileCoordNum);
			itsDependentAxis = 1;
			if (pixelAxes(1)==profilePixelAxis) itsDependentAxis = 0;
		}
		cSysOut.addCoordinate(cSysDD.coordinate(profileCoordNum));

// Create a linear coordinate for Y axis (Intensity) and add it to the output CS

		LinearCoordinate lc(1);
		Vector<String> name(1, "Intensity");
		Vector<String> unit(1, itsDD->dataUnit().getName());
		lc.setWorldAxisNames(name);
		lc.overwriteWorldAxisUnits(unit);
		Vector<Double> inc(1, itsYAxisInc);
		lc.setIncrement(inc);
		cSysOut.addCoordinate(lc);

// If the profile axis is a SpectralCoordinate, we need to add any DirectionCoordinate
// to support frame conversions

		if (cSysDD.type(profileCoordNum)==Coordinate::SPECTRAL) {
			Int afterCoord = -1;
			Int extraCoordNum = cSysDD.findCoordinate(Coordinate::DIRECTION, afterCoord);
			if (extraCoordNum>=0) {
				cSysOut.addCoordinate (cSysDD.coordinate(extraCoordNum));
			}

// Locate rest frequency

			itsParamShowRestFrequency->setEditable(True);
			itsRestFrequency = cSysDD.spectralCoordinate(profileCoordNum).restFrequency();
		} else {
			itsParamShowRestFrequency->setEditable(False);
			itsRestFrequency = 0;
		}


// We have created a CS with all of its axes intact and upto 3 Coordinates.
// Now we are going to remove all but the axes relevant to the X and Y of the plot.

		Vector<Int> axes;
		Vector<Double> worldReplacement;
		if (cSysOut.coordinate(0).nWorldAxes() > 1) {

// First remove all but the profile axis from coordinate 0 (the profile coordinate)

			Vector<Int> tAxes = cSysOut.worldAxes(0);
			axes.resize(tAxes.nelements()-1);
			uInt j = 0;
			for (uInt i=0; i<tAxes.nelements(); i++) {
				if (Int(i) != axisInProfileCoordinate) {          // Because first coordinate this index is ok
					axes[j] = tAxes[i];
					j++;
				}
			}

// Remove pixel and world axes other than the profile axis.  Use the
// reference value for replacement for now.

			cSysOut.removeAxes(worldReplacement, axes, True);
		}


// Now remove all axes associated with the third coordinate, if any.  All we
// will do with this coordinate is access its ObsInfo, not its axes.

		if (cSysOut.nCoordinates()==3) {
			axes.resize();
			axes = cSysOut.worldAxes(2);
			cSysOut.removeAxes(worldReplacement, axes, True);
		}

// Set the coordinate system of this DD (which will live in ActiveCaching2D) and the
// axis labeller with copies (initially) of this CS

		updateCoordinateSys(cSysOut);

// set default axis labels

		itsDefaultAxisLabels[0] = itsAxisLabeller.xAxisText();
		itsDefaultAxisLabels[1] = itsAxisLabeller.yAxisText();
//
		return True;
	}

	void Profile2dDD::constructParameters() {

// Profile color

		itsParamColor = new DParameterColorChoice("profilecolor", "Profile Color",
		        "Color of Profile","", True);

		itsParamLineWidth = new DParameterRange<Float>("profilelinewidth", "Profile Line Width", "Line Width",
		        0, 5, 0.1, 0.5,0.5, "", True, False);

		Vector<String> choices(3);
		Vector<Int> values(3);
		// profile line style
		for (uInt i=0; i<values.nelements(); i++) {
			values[i] = i;
		}
		choices[0] = "Solid";
		choices[1] = "Dashed";
		choices[2] = "Double Dashed";
		itsParamLineStyle = new DParameterMapKeyChoice("profilelinestyle", "Line Style", "Line Style",
		        choices, values, choices(0), choices(0), "");

// Autoscale switch (on or off)

		itsParamAutoscale = new DParameterSwitch("profileautoscale", "Auto Scale Y values",
		        "Auto Scale Y Axis vales to min and max", False, False, "");

		itsParamShowRestFrequency = new DParameterSwitch("profileshowrestfrequency", "Show Rest Frequency",
		        "Show Rest Frequency", False, False, "");

// Statistic type for region calculations

		itsRegionCalcChoices.resize(11);
		values.resize(itsRegionCalcChoices.nelements());

		itsRegionCalcChoices[0] = "Sum";
		values[0] = LatticeStatsBase::SUM;
//
		itsRegionCalcChoices[1] = "Sum Sq.";
		values[1] = LatticeStatsBase::SUMSQ;
//
		itsRegionCalcChoices[2] = "Median";
		values[2] = LatticeStatsBase::MEDIAN;
//
		itsRegionCalcChoices[3] = "Med. Abs Dev.";
		values[3] = LatticeStatsBase::MEDABSDEVMED;
//
		itsRegionCalcChoices[4]   = "Quartile";
		values[4] = LatticeStatsBase::QUARTILE;
//
		itsRegionCalcChoices[5] = "Min";
		values[5] = LatticeStatsBase::MIN;
//
		itsRegionCalcChoices[6] = "Max";
		values[6] = LatticeStatsBase::MAX;
//
		itsRegionCalcChoices[7] = "Mean";
		values[7] = LatticeStatsBase::MEAN;
//
		itsRegionCalcChoices[8] = "Variance";
		values[8] = LatticeStatsBase::VARIANCE;
//
		itsRegionCalcChoices[9] = "Sigma";
		values[9] = LatticeStatsBase::SIGMA;
//
		itsRegionCalcChoices[10] = "RMS";
		values[10] = LatticeStatsBase::RMS;
//
		itsParamRegionStatType = new DParameterMapKeyChoice("profileregionstattype", "Region Stat. Type",
		        "Region statistic type", itsRegionCalcChoices, values,
		        itsRegionCalcChoices(7), itsRegionCalcChoices(7),
		        "Region_options");

// X and Y radius around a position (to create a region)

		itsParamRegionXRadius = new DParameterRange<Int>("profileregionxradius", "+- Width",
		        "Create region of +- X around point of profile",
		        0, 5, 0, 0,0, "Region_options", False, True);

		itsParamRegionYRadius = new DParameterRange<Int>("profileregionyradius", "+- Height",
		        "Create region of +- Y around point of profile",
		        0, 5, 0, 0,0, "Region_options", False, True);
	}

	void Profile2dDD::destructParameters() {
		delete itsParamColor;
		delete itsParamLineWidth;
		delete itsParamLineStyle;
		delete itsParamAutoscale;
		delete itsParamShowRestFrequency;
		delete itsParamRegionStatType;
		delete itsParamRegionXRadius;
		delete itsParamRegionYRadius;
	}

	Bool Profile2dDD::canLabelAxes() const {
		bool axisLabelling = false;
		if (itsAxisLabeller.axisLabelSwitch()) {
			axisLabelling = true;
		}
		return axisLabelling;
	}

	Bool Profile2dDD::labelAxes(const WCRefreshEvent &ev) {
		if (!itsAxisLabeller.axisLabelSwitch()) {
			return False;
		}
//
		try {
			itsAxisLabeller.draw(ev);
		} catch (const AipsError &x) {
			if (&x) { // use x to avoid compiler warning
				throw(AipsError(x.getMesg()));
				return False;
			}
		}
		return True;
	}

	void Profile2dDD::operator()(const WCMotionEvent &ev) {
		// (dk note: this can be confusing.  These events come from the
		// _underlying Image DD's_ canvas; they cause a profile refresh.
		// In contrast, WCMotionEvents from this DD's _own_ canvas find their way
		// (via a different wierd route) to this DD's showPosition() routine...)

		if (!itsDD) {
			return; // do nothing if no data is attached
		}
		if (!itsTrackingState) {
			return; // do nothing if tracking is off
		}
		if (!itsDD->conformsTo(ev.worldCanvas())) return;
		// Event was not from a canvas where itsDD currently applies.

		// draw the profile for the new position
		if (getPointProfile(ev.world())) {

			refresh();
			// send DDModEvent
			sendDDModEvent();
		}

	}

	void Profile2dDD::operator()(const WCPositionEvent &ev) {
		if (ev.keystate()) {
			if (ev.key() == Display::K_space) {
				// if SPACE pressed, switch the tracking state
				itsTrackingState = !itsTrackingState;
			}
		}
	}

	void Profile2dDD::handleEvent(DisplayEvent &ev) {
		if (!itsDD) {
			return; // do nothing if no data is attached
		}

// send to parent

		ActiveCaching2dDD::handleEvent(ev);

// check for crosshair event

		CrosshairEvent *cev = dynamic_cast<CrosshairEvent *>(&ev);
		if (cev) {
			if (!itsDD->conformsTo(cev->worldCanvas())) return;
			// Event was not from a canvas where itsDD is valid.

			if (!cev->evtype().compare("up")) {
				return;
			}

// get world coordinate of event

			Vector<Double> world(2);
			Vector<Double> pix(2);
			pix(0) = cev->pixX();
			pix(1) = cev->pixY();
			cev->worldCanvas()->pixToWorld(world, pix);

// get profile at that world coord and refresh if needed

			if (getPointProfile(world)) {
				refresh();

// send DDModEvent

				sendDDModEvent();
			}
			return;
		}

// check for RectRegion event

		RectRegionEvent *rev = dynamic_cast<RectRegionEvent *>(&ev);
		if (rev) {

			if (!itsDD->conformsTo(rev->worldCanvas())) return;
			// Event was not from a canvas where itsDD is valid.

// get pixel coordinate of corners

			Vector<Double> pixBlc(2), pixTrc(2);
			for(Int i=0; i<2; i++) {
				pixBlc[i] = rev->pixBlc()[i];
				pixTrc[i] = rev->pixTrc()[i];
			}

// convert pixel coordinates to world

			Vector<Double> worldBlc, worldTrc;
			rev->worldCanvas()->pixToWorld(worldBlc, pixBlc);
			rev->worldCanvas()->pixToWorld(worldTrc, pixTrc);
			Vector<Double> fworldBlc, fpixelBlc, fworldTrc, fpixelTrc;
			itsDD->getFullCoord(fworldBlc, fpixelBlc, worldBlc);
			itsDD->getFullCoord(fworldTrc, fpixelTrc, worldTrc);

// get profile of the region and refresh if needed

			if (getRegionProfile(fpixelBlc, fpixelTrc)) {
				refresh();

// send DDModEvent

				sendDDModEvent();
			}
			return;
		}
	}

	void Profile2dDD::sendDDModEvent() {
// send DDModEvent

		Record rec;
		getProfileAsRecord(rec);
		DDModEvent ev(this, &rec);
		ActiveCaching2dDD::handleEvent(ev);
	}


	Bool Profile2dDD::sizeControl(WorldCanvasHolder& wch, AttributeBuffer& buf) {

		if(!wch.isCSmaster(this)) return False;
		// Return if another DD has already done size control.

		WorldCanvas *wc = wch.worldCanvas();
		static String unZoom = "resetCoordinates", zoomT = "manualZoomTrc";
		if(profileAutoscale() && !wc->existsAttribute(zoomT)) {

			// Autoscale is on (and no prior zoom order already exists):
			// place a zoom order on the WC to scale Y axis to current data.

			Vector<Double> zoomBlc(2), zoomTrc(2);
			if(wc->existsAttribute(unZoom)) {
				zoomBlc[0]=itsPixelBlc[0];	// Set X zoom to maximum if
				zoomTrc[0]=itsPixelTrc[0];
			}	// unzoom order already exists.
			else {
				zoomBlc[0]=wc->linXMin();		// Otherwise, leave it at
				zoomTrc[0]=wc->linXMax();
			}	// the current WC setting.

			// Compute data range for Y scaling.
			// (dk note: this does what the old version did (with a bit more safety
			// and efficiency), but I think the alignment between data and mask is
			// still very suspect, esp. on unzoom.  No time to fix it now, though).

			Int   d0 = Int(zoomBlc[0] +.5),
			      xlen = Int(zoomTrc[0]-zoomBlc[0] +.5),
			      mlen = itsMask.nelements(),
			      dlen = itsData.shape()[0];
			Bool nomask = mlen==0;
			mlen = nomask?  xlen : min(mlen,xlen);
			Double dmin=1e80, dmax=-1e80;

			for (Int im=0, id=d0;   im<mlen && id<dlen;   im++, id++) {
				if((nomask||itsMask[im]) && id>=0) {
					Double d = itsData(id,1);
					if(dmin>d) dmin=d;
					if(dmax<d) dmax=d;
				}
			}

			if(dmin<dmax) {	// Good Y zoom values: send the zoom order.

				zoomBlc[1]=dmin;
				zoomTrc[1]=dmax;
				wc->setZoomRectangleLCS(zoomBlc, zoomTrc);
			}
		}


		// Leave the rest of size control to the base class.  (please)

		Bool ok = ActiveCaching2dDD::sizeControl(wch, buf);

		buf.getValue("linXMin", itsCurrentBlc[0]);	// (Maintain our own record
		buf.getValue("linYMin", itsCurrentBlc[1]);	// of final zoom window).
		buf.getValue("linXMax", itsCurrentTrc[0]);
		buf.getValue("linYMax", itsCurrentTrc[1]);

		return ok;

	}



	void Profile2dDD::getDrawData(Matrix<Double>& data, const Bool world) {

// This function only returns the portion of the data
// array that needs drawing. Eg. when canvas is zoomed

		if (!itsData.nelements()) {
			return;
		}
		IPosition shape(2);
		shape(0) = Int(profileXMax() - profileXMin() + 0.5);
		shape(1) = 2;
		data.resize(shape);
		for (uInt i=0; i < uInt(shape(0)); i++) {
			data(i, 0) = Double(i) + profileXMin();
			data(i, 1) = itsData(i+Int(profileXMin()+0.5), 1);
		}

// convert to world coordinates if world variable is True

		if (world) {
			Vector<Double> pixelCoord(2), worldCoord(2);
			for (uInt i=0; i < uInt(shape(0)); i++) {
				pixelCoord[0] = data(i, 0);
				pixelCoord[1] = data(i, 1);
				itsCoordinateSystem.toWorld(worldCoord, pixelCoord);
				data(i, 0) = worldCoord[0];
				data(i, 1) = worldCoord[1];
			}
		}
	}

	void Profile2dDD::getMaskData(Vector<Bool>& mask) {
// This function only returns the mask for the portion of the data
// array that is currently drawn.

		if (!itsMask.nelements()) {
			return;
		}
		uInt length = uInt(profileXMax()+0.5) - uInt(profileXMin()+0.5);
		mask.resize(length);
		for (uInt i=0; i < length; i++) {
			mask[i] = itsMask(i+Int(profileXMin()+0.5));
		}
	}

	void Profile2dDD::getProfileAsRecord(Record &rec) {
		Matrix<Double> worldData, pixelData;
		Vector<Double> fworld, fpixel;
		Vector<Bool> maskData;
		getDrawData(worldData, True); // get data in world coords
		getDrawData(pixelData); // get data in pixel coords
		getMaskData(maskData);
		itsDD->getFullCoord(fworld, fpixel, itsWorldPosition);

		// Create (main) data record structure
		RecordDesc dataRecDesc;
		dataRecDesc.addField("position", TpRecord); // position data
		dataRecDesc.addField("region", TpRecord); // position data
		dataRecDesc.addField("profile", TpRecord); // profile data
		Record dataRec(dataRecDesc);

		// create data-position record structure
		RecordDesc positionRecDesc;
		positionRecDesc.addField("pixel", TpRecord);
		positionRecDesc.addField("world", TpRecord);
		positionRecDesc.addField("intensity", TpRecord);
		Record positionRec(positionRecDesc);

		// create data-position-pixel record structure
		RecordDesc posPixelRecDesc;
		posPixelRecDesc.addField("numeric", TpArrayDouble);
		posPixelRecDesc.addField("string", TpString);
		Record posPixelRec(posPixelRecDesc);
		posPixelRec.define("numeric", fpixel);
		String tmpPixPos;
		String pPos = itsDD->showPosition(itsWorldPosition, False, True);
		pPos.gsub("\n"," ");		// replace newlines with spaces.
		posPixelRec.define("string", pPos);

		// create data-position-world record structure
		RecordDesc posWorldRecDesc;
		posWorldRecDesc.addField("numeric", TpArrayDouble);
		posWorldRecDesc.addField("string", TpString);
		Record posWorldRec(posWorldRecDesc);
		posWorldRec.define("numeric", fworld);
		String wPos = itsDD->showPosition(itsWorldPosition, True, True);
		wPos.gsub("\n"," ");		// replace newlines with spaces.
		posWorldRec.define("string", wPos);

		// create data-position-intensity record structure
		RecordDesc posIntensRecDesc;
		//  posIntensRecDesc.addField("value", TpDouble);
		posIntensRecDesc.addField("value", TpString);
		posIntensRecDesc.addField("mask", TpBool);
		Record posIntensRec(posIntensRecDesc);

		if (itsData.nelements() > 0) {
			posIntensRec.define("value", itsDD->showValue(itsWorldPosition));
		}
		if (itsMask.nelements() > 0) {
			Bool mask = itsMask(Int(fpixel(profileAxis())+0.5));
			posIntensRec.define("mask", mask);
		}

		// create data-region record structure
		RecordDesc regionRecDesc;
		regionRecDesc.addField("pixelblc", TpArrayDouble);
		regionRecDesc.addField("pixeltrc", TpArrayDouble);
		regionRecDesc.addField("stat", TpString);
		Record regionRec(regionRecDesc);
		Vector<Double> pixelBlc, pixelTrc;
		regionDimensions(pixelBlc, pixelTrc);
		regionRec.define("pixelblc", pixelBlc);
		regionRec.define("pixeltrc", pixelTrc);
		regionRec.define("stat", itsParamRegionStatType->value());

		// create data-profile record structure
		RecordDesc profileRecDesc;
		profileRecDesc.addField("cs", TpRecord);
		profileRecDesc.addField("x", TpRecord);
		profileRecDesc.addField("y", TpRecord);
		Record profileRec(profileRecDesc);

		// create data-profile-CS record structure
		Record profCSRec;
		DisplayCoordinateSystem cs(itsAxisLabeller.coordinateSystem());

		// remove the profile linear axis (coordinate 1, axis 0)

		cs.removePixelAxis(1, 0);

		// set the reference pixel for zoomed profiles.
		Vector<Double> ref(1, profileXMin());
		cs.setReferencePixel(ref);
		// save the cs to the record
		cs.save(profCSRec, "cs");

		// create data-profile-x record structure
		RecordDesc profXRecDesc;
		profXRecDesc.addField("pixel", TpRecord);
		profXRecDesc.addField("world", TpRecord);
		Record profXRec(profXRecDesc);

		// create data-profile-x-pixel record structure
		RecordDesc profXPixRecDesc;
		profXPixRecDesc.addField("abs", TpArrayDouble);
		// profXPixRecDesc.addField("rel", TpArrayDouble);
		Record profXPixRec(profXPixRecDesc);
		profXPixRec.define("abs", pixelData.column(0));

		// create data-profile-x-world record structure
		RecordDesc profXWorldRecDesc;
		profXWorldRecDesc.addField("abs", TpArrayDouble);
		//  profXWorldRecDesc.addField("rel", TpArrayDouble);
		Record profXWorldRec(profXWorldRecDesc);
		profXWorldRec.define("abs", worldData.column(0));

		// create data-profile-y record structure
		RecordDesc profYRecDesc;
		profYRecDesc.addField("data", TpArrayDouble);
		profYRecDesc.addField("mask", TpArrayBool);
		profYRecDesc.addField("unit", TpString);
		Record profYRec(profYRecDesc);
		profYRec.define("data", worldData.column(1));
		profYRec.define("unit", itsCoordinateSystem.worldAxisUnits()(1));
		if (maskData.nelements() != 0) {
			profYRec.define("mask", maskData);
		}

		// Compile the data record
		positionRec.defineRecord("pixel", posPixelRec);
		positionRec.defineRecord("world", posWorldRec);
		positionRec.defineRecord("intensity", posIntensRec);
		profileRec.defineRecord("cs", profCSRec);
		profXRec.defineRecord("pixel", profXPixRec);
		profXRec.defineRecord("world", profXWorldRec);
		profileRec.defineRecord("x", profXRec);
		profileRec.defineRecord("y", profYRec);
		if (isRegionProfile()) {
			dataRec.defineRecord("region", regionRec);
		} else {
			dataRec.defineRecord("position", positionRec);
		}
		dataRec.defineRecord("profile", profileRec);

		rec = dataRec;
	}

	Bool Profile2dDD::getRegionProfile( Vector<Double> &fpixelBlc,
	                                    Vector<Double> &fpixelTrc) {
		if (itsDependentAxis != -1) {

// Cannot perform region calcs if profiling an axis from a
// Direction Coordinate

			return False;
		}

// integrity checks

		if (fpixelBlc.nelements() == 0 || fpixelTrc.nelements() == 0 ||
		        fpixelBlc.nelements() > 4 || fpixelTrc.nelements() > 4) {
			throw(AipsError("Profile2dDD::getRegionProfile - invalid region dimensions"));
		}

// Crop regions outside of shape boundary

		if (cropRegion(fpixelBlc, fpixelTrc)) {

// set new Y Axis Label (include stat type)

			if (!itsIsRegionProfile) {
				String newYLabel = String(itsParamRegionStatType->value()) +
				                   " " + itsDefaultAxisLabels(1);
				itsAxisLabeller.setYAxisText(newYLabel);
				itsAxisLabeller.invalidate();
			}
			// set the global region variables to the current values.
			itsRegionBlc.resize();
			itsRegionTrc.resize();
			itsRegionBlc = fpixelBlc;
			itsRegionTrc = fpixelTrc;
		} else {
			return False;
		}
		itsIsRegionProfile = True;

// get the required slice from the data

		IPosition shape = itsDD->maskedLattice()->shape();
		vector<int> axes = itsDD->displayAxes();
		IPosition start(shape.nelements()), end(shape.nelements());

		for (uInt i=0; i < shape.nelements(); i++) {
			if (Int(i) == profileAxis()) {
				start(i) = 0;
				end(i) = shape(profileAxis())-1;
			} else {
				start(i) = Int(fpixelBlc(i)+0.5);
				end(i) = Int(fpixelTrc(i)+0.5);
			}
		}

// Get the statistic

		Slicer slice(start, end, Slicer::endIsLast);
		SubLattice<Float> subLat (*itsDD->maskedLattice(), slice, False);
		LatticeStatistics<Float> latticeStats(subLat, False);
		Vector<Int> cursorAxes(2, 0);
		cursorAxes[1] = 1;
		if (!latticeStats.setAxes(cursorAxes)) {
			throw(AipsError("Profile2dDD::getRegionProfile - " + latticeStats.errorMessage()));
		}

// Get the required statistic.

		Array<Float> array;
		latticeStats.getConvertedStatistic(array, regionStatType());
		if (array.nelements()==0) {
			throw(AipsError("Profile2dDD::getRegionProfile - No elements in statistics array"));
			return False;
		}

// Insert the statistics into the data array itsData

		IPosition pos(1);
		for (Int i=0; i < shape(profileAxis()); i++) {
			pos = i;
			itsData(i, 1) = array(pos) / itsYAxisInc;
		}

// masks are currently not supported with region calculations (24/03/03)

		itsMask.resize();

		return True;
	}

	Bool Profile2dDD::getPointProfile(const Vector<Double> &world) {
		Bool ret = True;
		Vector<Double> fworld,fpixel;
		if (!itsDD->getFullCoord(fworld, fpixel, world)) {
			return False;
		}

// check if Region Mode is active
// Also need to ensure there are no dependent axis (since region
// calcs cannot currently be performed on Direction Coordinates)

		if (itsDependentAxis == -1 && (regionXRadius() || regionYRadius())) {
			Vector<Double> fpixelBlc(fpixel.nelements()), fpixelTrc(fpixel.nelements());
			fpixelBlc = fpixel;
			fpixelTrc = fpixel;
			fpixelBlc(itsDD->displayAxes()[0]) -= regionXRadius();
			fpixelBlc(itsDD->displayAxes()[1]) -= regionYRadius();
			fpixelTrc(itsDD->displayAxes()[0]) += regionXRadius();
			fpixelTrc(itsDD->displayAxes()[1]) += regionYRadius();
			return getRegionProfile(fpixelBlc, fpixelTrc);
		} else {
			// this is definitely a single point profile, not a region profile.
			if (itsIsRegionProfile) {
				itsAxisLabeller.setYAxisText(itsDefaultAxisLabels(1));
				itsAxisLabeller.invalidate();
			}
		}
		itsIsRegionProfile = False;

		// obtain current pixel position
		itsWorldPosition.resize();
		itsWorldPosition = world;
		itsPixelPosition.resize(world.nelements());
		for (uInt i=0; i < world.nelements(); i++) {
			for (uInt j=0; j < fworld.nelements(); j++) {
				if (world(i) == fworld(j)) {
					itsPixelPosition(i) = fpixel(j);
					continue;
				}
			}
		}
		// set up start position and shape to slice
		uInt length = fpixel.nelements();
		vector<int> displayAxes = itsDD->displayAxes();
		IPosition start(length), shape(itsDD->maskedLattice()->shape());
		for (uInt i=0; i< length; i++) {
			if (Int(i) == profileAxis()) {
				start(i) = 0; // this starts profile from the 1st channel
			} else {
				start(i) = Int(fpixel(i) + 0.5);
				shape(i) = 1;
			}
		}

		// create stores and get slice
		static Array<Float> dataSlice;
		static Array<Bool> maskSlice;
		dataSlice.resize();
		maskSlice.resize();
		try {
			itsDD->maskedLattice()->getSlice(dataSlice,start,shape,True);
			itsDD->maskedLattice()->getMaskSlice(maskSlice, start, shape, True);
		} catch (const AipsError &x) {
			if (&x) { // use x to avoid compiler warning
				return False;
			}
		}
		// copy data from dataSlice to itsData
		static Vector<Float> tmp0;
		tmp0.resize();
		tmp0 = dataSlice.nonDegenerate();
		itsData.resize(tmp0.nelements(),2);
		for (uInt i=0; i< tmp0.nelements(); i++) {
			itsData(i,0) = Double(i);
			itsData(i,1) = Double(tmp0(i)) / itsYAxisInc;
		}

		// copy data from maskSlice to itsMask
		if (itsDD->maskedLattice()->isMasked()) {
			static Vector<Bool> tmp1;
			tmp1.resize();
			tmp1 = maskSlice.nonDegenerate();
			itsMask.resize(tmp1.nelements());
			for (uInt i=0; i< tmp0.nelements(); i++) {
				itsMask(i) = tmp1(i);
			}
		} else {
			itsMask = 0;
		}
		return ret;
	}

	Bool Profile2dDD::cropRegion(Vector<Double> &fpixelBlc,
	                             Vector<Double> &fpixelTrc) {
		IPosition shape = itsDD->maskedLattice()->shape();

		// Crop regions outside of shape boundary
		Int axis;
		for (uInt i=0; i < 2; i++) {
			axis = itsDD->displayAxes()[i];
			if (fpixelBlc(axis) <= 0) fpixelBlc(axis) = 0;
			if (fpixelTrc(axis) <= 0) fpixelTrc(axis) = 0;
			if (fpixelBlc(axis) >= shape(axis)) fpixelBlc(axis) = shape(axis)-1;
			if (fpixelTrc(axis) >= shape(axis)) fpixelTrc(axis) = shape(axis)-1;
		}
		if (fpixelBlc(itsDD->displayAxes()[0]) == fpixelTrc(itsDD->displayAxes()[0]) ||
		        fpixelBlc(itsDD->displayAxes()[1]) == fpixelTrc(itsDD->displayAxes()[1])) {
			// the entire region is outside the image area.
			return False;
		}
		return True;
	}


	Bool Profile2dDD::updateCoordinateSys(DisplayCoordinateSystem &cs) {

// update cs of axis labeller

		itsAxisLabeller.setCoordinateSystem(cs);
		itsAxisLabeller.setAxisLabelSwitch(True);

// setup scales for plot

		Vector<Double> linblc(2, 0), lintrc(2, 0);
		linblc(0) = 0;
		lintrc(0) = Double(itsDD->maskedLattice()->shape()(profileAxis()));

// set y min/max to the min max data value

		linblc(1) = itsDD->getDataMin() / itsYAxisInc;
		lintrc(1) = itsDD->getDataMax() / itsYAxisInc;

// update cs of this DD

		setCoordinateSystem(cs, linblc, lintrc);
//
		return True;
	}

	const Unit Profile2dDD::dataUnit() {
		return Unit("");
	}

	String Profile2dDD::showValue(const Vector<Double> &/*world*/) {
		return "";
	}

	void Profile2dDD::setDefaultOptions() {
		ActiveCaching2dDD::setDefaultOptions();
		itsAxisLabeller.setDefaultOptions();
		// Set local default options
		itsParamColor->setValue(itsParamColor->defaultValue());
		itsParamLineWidth->setValue(itsParamLineWidth->defaultValue());
		itsParamLineStyle->setValue(itsParamLineStyle->defaultValue());
		itsParamAutoscale->setValue(itsParamAutoscale->defaultValue());
		itsParamShowRestFrequency->setValue(itsParamShowRestFrequency->defaultValue());
		itsParamRegionStatType->setValue(itsParamRegionStatType->defaultValue());
		itsParamRegionXRadius->setValue(itsParamRegionXRadius->defaultValue());
		itsParamRegionYRadius->setValue(itsParamRegionYRadius->defaultValue());
	}

	Bool Profile2dDD::setOptions(Record &rec, Record &recOut) {

		Bool ret = ActiveCaching2dDD::setOptions(rec, recOut);

		// Make local changes
		Bool localchange = False;

		if (rec.isDefined("axislabelpixelworld")) {
			// if the axis labels are to change from world -> pixel we don't
			// want the y axis to change to pixel because it would have no
			// meaning. We just want the x axis to show pixel coordinates. We
			// do this by setting a new coordinate system on the axis
			// labeller. This cs will contain a linear coordinate with two
			// world axes.

			DisplayCoordinateSystem tmpCS;
			if (rec.asString(0) == "pixel") {

				String xname = itsCoordinateSystem.coordinate(0).worldAxisNames()(0);
				String yname = itsCoordinateSystem.coordinate(1).worldAxisNames()(0);
				Vector<String> names(2);
				names(0) = xname;
				names(1) = yname;

				String xunits = itsCoordinateSystem.coordinate(0).worldAxisUnits()(0);
				String yunits = itsCoordinateSystem.coordinate(1).worldAxisUnits()(0);
				Vector<String> units(2);
				units(0) = xunits;
				units(1) = yunits;

				LinearCoordinate lc(2);
				lc.setWorldAxisNames(names);
				lc.overwriteWorldAxisUnits(units);
				Vector<Double> inc(2);
				inc(0) = 1;
				inc(1) = itsYAxisInc;
				lc.setIncrement(inc);
				tmpCS.addCoordinate(lc);
				itsAxisLabeller.setCoordinateSystem(tmpCS);
				itsAxisLabeller.invalidate();

				// change the axis label to display 'pixels'
				String xAxisText = xname + " (pixels)";
				itsAxisLabeller.setXAxisText(xAxisText);

			} else if (rec.asString(0) == "world") {
				String xname = itsCoordinateSystem.coordinate(0).worldAxisNames()(0);
				String xunits = itsCoordinateSystem.coordinate(0).worldAxisUnits()(0);
				itsAxisLabeller.setCoordinateSystem(itsCoordinateSystem);
				itsAxisLabeller.unsetXAxisText();
				itsAxisLabeller.invalidate();
			}

			localchange = True;

		} else {
			ret = itsAxisLabeller.setOptions(rec, recOut) || ret;
		}

		localchange = itsParamColor->fromRecord(rec) || localchange;
		localchange = itsParamLineWidth->fromRecord(rec) || localchange;
		localchange = itsParamLineStyle->fromRecord(rec) || localchange;
		localchange = itsParamAutoscale->fromRecord(rec) || localchange;
		localchange = itsParamShowRestFrequency->fromRecord(rec) || localchange;
		Bool statType = itsParamRegionStatType->fromRecord(rec);
		localchange = itsParamRegionXRadius->fromRecord(rec) || localchange;
		localchange = itsParamRegionYRadius->fromRecord(rec) || localchange;

		// if the stat type has changed AND the profile is a region profile,
		// then change the label and recalculate the profile
		if (statType && itsIsRegionProfile) {
			String newYLabel = String(itsParamRegionStatType->value()) +
			                   " " + itsDefaultAxisLabels(1);
			itsAxisLabeller.setYAxisText(newYLabel);
			itsAxisLabeller.invalidate();
			Vector<Double> regionBlc, regionTrc;
			regionDimensions(regionBlc, regionTrc);
			getRegionProfile(regionBlc, regionTrc);
			refresh();
			// send DDModEvent
			sendDDModEvent();
		}
		return (ret || localchange || statType);
	}

	Record Profile2dDD::getOptions() {
		Record rec = ActiveCaching2dDD::getOptions();
		Record axisRec = itsAxisLabeller.getOptions();
		if (rec.isDefined("aspect")) {
			rec.removeField("aspect");
		}
		// Merge fields of axisRec into rec
		rec.merge(axisRec);

		// Add local options to rec
		itsParamColor->toRecord(rec);
		itsParamLineWidth->toRecord(rec);
		itsParamLineStyle->toRecord(rec);
		itsParamAutoscale->toRecord(rec);
		itsParamShowRestFrequency->toRecord(rec);
		itsParamRegionStatType->toRecord(rec);
		itsParamRegionXRadius->toRecord(rec);
		itsParamRegionYRadius->toRecord(rec);
		return rec;
	}

	void Profile2dDD::regionDimensions(Vector<Double> &regionBlc,
	                                   Vector<Double> &regionTrc) {
		regionBlc = itsRegionBlc;
		regionTrc = itsRegionTrc;
	}

	const Int Profile2dDD::profileAxis() {
		if (itsDD) {
			return itsDD->displayAxes()[2];
		} else {
			throw(AipsError("No data attached"));
		}
	}


	CachingDisplayMethod *Profile2dDD::newDisplayMethod(
	    WorldCanvas *worldCanvas,
	    AttributeBuffer *wchAttributes,
	    AttributeBuffer *ddAttributes,
	    CachingDisplayData *dd) {
		return new Profile2dDM(worldCanvas, wchAttributes, ddAttributes, dd);
	}

	AttributeBuffer Profile2dDD::optionsAsAttributes() {
		AttributeBuffer buffer = ActiveCaching2dDD::optionsAsAttributes();
		buffer.set(itsParamColor->name(), profileColor());
		buffer.set(itsParamLineWidth->name(), profileLineWidth());
		buffer.set(itsParamLineStyle->name(), profileLineStyle());
		buffer.set(itsParamAutoscale->name(), profileAutoscale());
		buffer.set(itsParamShowRestFrequency->name(), showRestFrequency());
		buffer.set(itsParamRegionStatType->name(), regionStatType());
		buffer.set(itsParamRegionXRadius->name(), regionXRadius());
		buffer.set(itsParamRegionYRadius->name(), regionYRadius());
		return buffer;
	}

	void Profile2dDD::notifyUnregister(WorldCanvasHolder &wcHolder,
	                                   Bool ignoreRefresh) {
		// disconnected from WC[H]--be sure drawlists it may be
		// holding for us are deleted.
		itsAxisLabeller.invalidate();
		ActiveCaching2dDD::notifyUnregister(wcHolder, ignoreRefresh);
	}

	Profile2dDD::Profile2dDD(const Profile2dDD &o ) : ActiveCaching2dDD(o), WCMotionEH(o), WCPositionEH(o){
		// MUST IMPLEMENT
	}

	void Profile2dDD::operator=(const Profile2dDD &) {
		// MUST IMPLEMENT
	}

} //# NAMESPACE CASA - END

