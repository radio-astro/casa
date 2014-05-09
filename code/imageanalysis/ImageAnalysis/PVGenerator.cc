//# Copyright (C) 1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $

#include <imageanalysis/ImageAnalysis/PVGenerator.h>

#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <tables/Tables/PlainTable.h>

#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/ImageAnalysis/ImageMetaData.h>
#include <imageanalysis/ImageAnalysis/ImagePadder.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

#include <iomanip>

namespace casa {

const String PVGenerator::_class = "PVGenerator";

PVGenerator::PVGenerator(
	const SPCIIF image,
	const Record *const &regionRec,
	const String& chanInp, const String& stokes,
	const String& maskInp, const String& outname,
	const Bool overwrite
) : ImageTask<Float>(
		image, "", regionRec, "", chanInp, stokes,
		maskInp, outname, overwrite
	), _start(0), _end(0), _width(1), _unit("arcsec") {
	_construct();
}

PVGenerator::~PVGenerator() {}

void PVGenerator::setEndpoints(
	const std::pair<Double, Double>& start,
	const std::pair<Double, Double>& end
) {
	*_getLog() << LogOrigin(_class, __func__, WHERE);
	Double startx = start.first;
	Double starty = start.second;
	Double endx = end.first;
	Double endy = end.second;
	ThrowIf(
		startx == endx && starty == endy,
		"Start and end pixels must be different."
	);
	ThrowIf(
		startx < 2 || endx < 2 || starty < 2 || endy < 2,
		"Line segment end point positions must be contained in the image and be "
		"farther than two pixels from image edges. The pixel positions for "
		"the specified line segment are at " + _pairToString(start) + " and "
		+ _pairToString(end)
	);
	Vector<Int> dirAxes = _getImage()->coordinates().directionAxesNumbers();
	Int xShape = _getImage()->shape()[dirAxes[0]];
	Int yShape = _getImage()->shape()[dirAxes[1]];
	ThrowIf(
		startx > xShape-3 || endx > xShape-3
		|| starty > yShape-3 || endy > yShape-3,
		"Line segment end point positions must be contained in the image and must fall "
		"farther than two pixels from the image edges. The pixel positions for "
		"the specified line segment are at " + _pairToString(start) + " and "
		+ _pairToString(end)
	);
	_start.reset(new vector<Double>(2));
	_end.reset(new vector<Double>(2));
	(*_start)[0] = startx;
	(*_start)[1] = starty;
	(*_end)[0] = endx;
	(*_end)[1] = endy;
}

String PVGenerator::_pairToString(const std::pair<Double, Double>& p) {
	ostringstream os;
	os << "[" << p.first << ", " << p.second << "]";
	return os.str();
}

void PVGenerator::setEndpoints(
	const std::pair<Double, Double>& center, Double length,
	const Quantity& pa
) {
	ThrowIf(
		length <= 0,
		"Length must be positive"
	);
	setEndpoints(center, length*_increment(), pa);
}

void PVGenerator::setEndpoints(
	const std::pair<Double, Double>& center, const Quantity& length,
	const Quantity& pa
) {
	Vector<Double> centerV(2);
	const CoordinateSystem csys = _getImage()->coordinates();
	if (csys.isDirectionAbscissaLongitude()) {
		centerV[0] = center.first;
		centerV[1] = center.second;
	}
	else {
		centerV[0] = center.second;
		centerV[1] = center.first;
	}
	setEndpoints(
		csys.directionCoordinate().toWorld(centerV),
		length, pa
	);
}

void PVGenerator::setEndpoints(
	const MDirection& center, const Quantity& length,
	const Quantity& pa
) {
	ThrowIf(
		! pa.isConform("rad"),
		"Position angle must have angular units"
	);
	Quantity inc = _increment();
	ThrowIf(
		! length.isConform(inc),
		"Units of length are not conformant with direction axes units"
	);
	MDirection start = center;
	start.shiftAngle(length/2, pa);
	MDirection end = center;
	end.shiftAngle(length/2, pa - Quantity(180, "deg"));
	setEndpoints(start, end);
}

void PVGenerator::setEndpoints(
	const MDirection& center, Double length,
	const Quantity& pa
) {
	setEndpoints(center, length*_increment(), pa);
}

Quantity PVGenerator::_increment() const {
	const DirectionCoordinate dc = _getImage()->coordinates().directionCoordinate();
	Vector<String> units = dc.worldAxisUnits();
	ThrowIf(
		units[0] != units[1],
		"Cannot calculate the direction pixel increment because the"
		"axes have different units of " + units[0] + " and " + units[1]
	);
	return Quantity(fabs(dc.increment()[0]), units[0]);
}

void PVGenerator::setEndpoints(
	const MDirection& start, const MDirection& end
) {
	*_getLog() << LogOrigin(_class, __func__, WHERE);
	const DirectionCoordinate dc = _getImage()->coordinates().directionCoordinate();
	Vector<Double> sPixel = dc.toPixel(start);
	Vector<Double> ePixel = dc.toPixel(end);
	*_getLog() << LogIO::NORMAL << "Setting pixel end points "
		<< sPixel << ", " << ePixel << LogIO::POST;
	setEndpoints(
		std::make_pair(sPixel[0], sPixel[1]),
		std::make_pair(ePixel[0], ePixel[1])
	);
}

void PVGenerator::setWidth(uInt width) {
	ThrowIf(
		width % 2 == 0,
		"Width must be odd."
	);
	_width = width;
}

void PVGenerator::setWidth(const Quantity& q) {
	*_getLog() << LogOrigin(_class, __func__, WHERE);
	const DirectionCoordinate dc = _getImage()->coordinates().directionCoordinate();
	Quantity inc(fabs(dc.increment()[0]), dc.worldAxisUnits()[0]);
	ThrowIf(
		! q.isConform(inc),
		"Nonconformant units specified for quantity"
	);
	Double nPixels = (q/inc).getValue("");
	if (nPixels < 1) {
		nPixels = 1;
		*_getLog() << LogIO::NORMAL << "Using a width of 1 pixel or "
			<< inc.getValue(q.getUnit()) << q.getUnit() << LogIO::POST;
	}
	else if (near(fmod(nPixels, 2), 1.0)) {
		nPixels = floor(nPixels + 0.5);
	}
	else {
		nPixels = ceil(nPixels);
		if (near(fmod(nPixels, 2), 0.0)) {
			nPixels += 1;
		}
		Quantity qq = nPixels*inc;
		*_getLog() << LogIO::NORMAL << "Rounding width up to next odd number of pixels ("
			<< nPixels << "), or " << qq.getValue(q.getUnit()) << q.getUnit() << LogIO::POST;
	}
	setWidth((uInt)nPixels);
}

SPIIF PVGenerator::generate() const {
	*_getLog() << LogOrigin(_class, __func__, WHERE);

	ThrowIf(
		_start.get() == 0 || _end.get() == 0,
		"Start and/or end points have not been set"
	);
	SPIIF subImage(
		SubImageFactory<Float>::createImage(
			*_getImage(), "", *_getRegion(),
			_getMask(), False, False, False, _getStretch()
		 )
	);
	*_getLog() << LogOrigin(_class, __func__, WHERE);
	CoordinateSystem subCoords = subImage->coordinates();
	Vector<Int> dirAxes = subCoords.directionAxesNumbers();
	Int xAxis = dirAxes[0];
	Int yAxis = dirAxes[1];
	IPosition subShape = subImage->shape();
	IPosition origShape = _getImage()->shape();
	ThrowIf(
		subShape[xAxis] != origShape[xAxis]
		|| subShape[yAxis] != origShape[yAxis],
		"You are not permitted to make a region selection "
		"in the direction coordinate"
	);
	_checkWidth(subShape[xAxis], subShape[yAxis]);
	*_getLog() << LogOrigin(_class, __func__, WHERE);
	// get the PA of the end points
	vector<Double> start = *_start;
	vector<Double> end = *_end;
	Double paInRad = start[1] == end[1] ?
		start[0] < end[0]
		    ? 0 : C::pi
		: atan2(
			end[0] - start[0], end[1] - start[1]
		) - C::pi_2;
	Double halfwidth = (_width - 1)/2;
    if (_width > 1) {
        // check already done when setting the points if _width == 1
        Double angle1 = paInRad + C::pi/2;
        Double halfx = halfwidth*cos(angle1);
        Double halfy = halfwidth*sin(angle1);
        Vector<Double> xs(4);
        xs[0] = start[0] - halfx;
        xs[1] = start[0] + halfx;
        xs[2] = end[0] - halfx;
        xs[3] = end[0] + halfx;
        Vector<Double> ys(4);
        ys[0] = start[1] - halfy;
        ys[1] = start[1] + halfy;
        ys[2] = end[1] - halfy;
        ys[3] = end[1] + halfy;
        ThrowIf(
            min(xs) < 2 || max(xs) > subImage->shape()[xAxis] - 3
            || min(ys) < 2 || max(ys) > subImage->shape()[yAxis] - 3,
            "Error: specified end points and half width are such "
            "that chosen directional region falls outside or within "
            "two pixels of the edge of the image."
        );
    }

    {
    	// rotation occurs about the reference pixel, so move the reference pixel to be
    	// on the segment, near the midpoint so that the y value is an integer.
    	vector<Double> midpoint = (end + start)/2.0;
    	Double targety = int(midpoint[1]);
    	Double targetx = targety == midpoint[1]
    	    ? midpoint[0]
    	    : (
    	    	start[0]*(end[1] - targety) + end[0]*(targety - start[1])
    	    )/(end[1] - start[1]);
    	Vector<Double> newRefPix = subCoords.referencePixel();
    	newRefPix[dirAxes[0]] = targetx;
    	newRefPix[dirAxes[1]] = targety;
    	Vector<Double> newRefVal;
    	subCoords.toWorld(newRefVal, newRefPix);
    	subCoords.setReferencePixel(newRefPix);
    	subCoords.setReferenceValue(newRefVal);
    	subImage->setCoordinateInfo(subCoords);

    	// rotate the image through this angle, in the opposite direction.
    	*_getLog() << LogIO::NORMAL << "Rotating image by " << (paInRad*180/C::pi)
    		<< " degrees about direction coordinate pixel (" << targetx << ", " << targety
    		<< ") to align specified slice with the x axis" << LogIO::POST;
    }
	Vector<Double> worldStart, worldEnd;
	const DirectionCoordinate& dc1 = subCoords.directionCoordinate();
	dc1.toWorld(worldStart, Vector<Double>(start));
	dc1.toWorld(worldEnd, Vector<Double>(end));
	std::auto_ptr<DirectionCoordinate> rotCoord(
		dynamic_cast<DirectionCoordinate *>(
			dc1.rotate(Quantity(paInRad, "rad"))
		)
	);
	Vector<Double> startPixRot, endPixRot;
	rotCoord->toPixel(startPixRot, worldStart);
	rotCoord->toPixel(endPixRot, worldEnd);
	AlwaysAssert(abs(startPixRot[1] - endPixRot[1]) < 1e-6, AipsError);
	Double xdiff = fabs(end[0] - start[0]);
	Double ydiff = fabs(end[1] - start[1]);
	AlwaysAssert(
		abs(
			(endPixRot[0] - startPixRot[0])
			- sqrt(xdiff*xdiff + ydiff*ydiff)
		) < 1e-6, AipsError
	);
	Double padNumber = max(0.0, 1 - startPixRot[0]);
	padNumber = max(padNumber, -(startPixRot[1] - halfwidth - 1));
	SPIIF imageToRotate = subImage;
	Int nPixels = 0;
	if (padNumber > 0) {
		nPixels = (Int)padNumber + 1;
		*_getLog() << LogIO::NORMAL
			<< "Some pixels will fall outside the rotated image, so "
			<< "padding before rotating with " << nPixels << " pixels."
			<< LogIO::POST;
		ImagePadder padder(subImage);
		padder.setPaddingPixels(nPixels);
		SPIIF padded = padder.pad(True);
		imageToRotate = padded;
	}
	IPosition blc(subImage->ndim(), 0);
	IPosition trc = subShape - 1;

	// ensure we have enough real estate after the rotation
	blc[xAxis] = (Int)min(min(start[0], end[0]) - 1 - halfwidth, 0.0);
	blc[yAxis] = (Int)min(min(start[1], end[1]) - 1 - halfwidth, 0.0);
	trc[xAxis] = (Int)max(
		max(start[0], end[0]) + 1 + halfwidth,
		blc[xAxis] + (Double)subShape[xAxis] - 1
	) + nPixels;
	trc[yAxis] = (Int)max(
		max(start[1], end[1]) + 1 + halfwidth,
		(Double)subShape[yAxis] - 1
	) + nPixels;

	Record lcbox = LCBox(blc, trc, imageToRotate->shape()).toRecord("");
	std::tr1::shared_ptr<ImageInterface<Float> > rotated;
	if (paInRad == 0) {
		*_getLog() << LogIO::NORMAL << "Slice is along x-axis, no rotation necessary.";
		rotated.reset(
			new SubImage<Float>(
				SubImageFactory<Float>::createSubImage(
					*imageToRotate, lcbox, "", 0, False,
					AxesSpecifier(), False, True
				)
			)
		);
	}
	else {
		IPosition outShape = subShape;
		outShape[xAxis] = (Int)(endPixRot[0] + nPixels + 6);
		outShape[yAxis] = (Int)(startPixRot[1] + halfwidth) + nPixels + 6;
		ImageAnalysis ia(imageToRotate);
		rotated.reset(
			ia.rotate(
				"", outShape.asVector(), Quantity(paInRad, "rad"),
				lcbox, ""
			)
		);
	}

	// done with this pointer
	imageToRotate.reset();
	Vector<Double> origStartPixel = Vector<Double>(subShape.size(), 0);
	origStartPixel[xAxis] = start[0];
	origStartPixel[yAxis] = start[1];
	Vector<Double> origEndPixel = Vector<Double>(subShape.size(), 0);
	origEndPixel[xAxis] = end[0];
	origEndPixel[yAxis] = end[1];
	Vector<Double> startWorld = subCoords.toWorld(origStartPixel);
	Vector<Double> endWorld = subCoords.toWorld(origEndPixel);
	const CoordinateSystem& rotCoords = rotated->coordinates();

	Vector<Double> rotPixStart = rotCoords.toPixel(startWorld);
	Vector<Double> rotPixEnd = rotCoords.toPixel(endWorld);
	// sanity checks, can be removed when this is well tested and used without issue
	// The rotated start and end pixels should lie in the image
	IPosition rotShape = rotated->shape();
	for (uInt i=0; i<2 ;i++) {
		Int64 shape = i == 0 ? rotShape[xAxis] : rotShape[yAxis];
		AlwaysAssert(
			rotPixStart[i] > 0 && rotPixEnd[i] > 0
			&& rotPixStart[i] < shape - 1 && rotPixEnd[i] < shape - 1,
			AipsError
		);
	}

	// We've rotated to make the slice coincident with the x axis, therefore, the y axis
	// values of the endpoints should be equal
	AlwaysAssert(abs(rotPixStart[yAxis] - rotPixEnd[yAxis]) < 1e-6, AipsError);
	// the difference in the x axis coordinate of rotated endpoints should simply be
	// the distance between those points before rotation
	AlwaysAssert(
		abs(
			(rotPixEnd[xAxis] - rotPixStart[xAxis])
			- sqrt(xdiff*xdiff + ydiff*ydiff)
		) < 1e-6, AipsError
	);
	// CAS-6043, because it's possible for the above conditions to be true but the y values to still be
	// just a little different and on either side of the 0.5 pixel mark
	//rotPixEnd[yAxis] = rotPixStart[yAxis];
	// We have rotated so the position of the starting pixel x is smaller than
	// the ending pixel x.
	AlwaysAssert(rotPixStart[xAxis] < rotPixEnd[xAxis], AipsError);

	blc = IPosition(rotated->ndim(), 0);
	trc = rotated->shape() - 1;
	blc[xAxis] = (Int)(rotPixStart[xAxis] + 0.5);
	blc[yAxis] = (Int)(rotPixStart[yAxis] + 0.5 - halfwidth);
	trc[xAxis] = (Int)(rotPixEnd[xAxis] + 0.5);
	trc[yAxis] = (Int)(rotPixEnd[yAxis] + 0.5 + halfwidth);

	lcbox = LCBox(blc, trc, rotated->shape()).toRecord("");
	IPosition axes(1, yAxis);
	ImageCollapser<Float> collapser(
		"mean", rotated, &lcbox,
		"", axes, False, "", False
	);

	SPIIF collapsed = collapser.collapse();
	Vector<Double > newRefPix = rotCoords.referencePixel();
	newRefPix[xAxis] = rotPixStart[xAxis] - blc[xAxis];
	newRefPix[yAxis] = rotPixStart[yAxis] - blc[yAxis];
	CoordinateSystem collCoords = collapsed->coordinates();

	// to determine the pixel increment of the angular offset axis, get the
	// distance between the end points
	ImageMetaData md(collapsed);
	Vector<Int> dirShape = md.directionShape();
	AlwaysAssert(dirShape[1] == 1, AipsError);
	const DirectionCoordinate& dc = collCoords.directionCoordinate();
	Vector<Int> dirAxisNumbers = collCoords.directionAxesNumbers();
	Vector<Double> pixStart(2, 0);
	MVDirection collapsedStart = dc.toWorld(pixStart);
	Vector<Double> pixEnd(2, 0);
	pixEnd[0] = dirShape[0];
	MVDirection collapsedEnd = dc.toWorld(pixEnd);
	Quantity separation = collapsedEnd.separation(
		collapsedStart, dc.worldAxisUnits()[0]
	);
	// The new coordinate must have the same number of axes as the coordinate
	// it replaces, so 2 for the linear coordinate, we will remove the degenerate
	// axis later
	Vector<String> axisName(2, "Offset");
	Vector<String> axisUnit(2, _unit);
	Vector<Double> crval(2, 0);
	Vector<Double> cdelt(2, separation.getValue(axisUnit[0])/dirShape[0]);
	Matrix<Double> xform(2, 2, 1);
	xform(0, 1) = 0;
	xform(1, 0) = 0;
	Vector<Double> crpix(2, (dirShape[0] - 1)/2);
	LinearCoordinate lc(
		axisName, axisUnit, crval,
		cdelt, xform, crpix
	);
	collCoords.replaceCoordinate(
		lc, collCoords.directionCoordinateNumber()
	);
	TableRecord misc = collapsed->miscInfo();
	collapsed->coordinates().save(misc, "secondary_coordinates");
	collapsed->setMiscInfo(misc);
	collapsed->setCoordinateInfo(collCoords);
	IPosition keep, axisPath;
	uInt j = 0;
	for (uInt i=0; i<collapsed->ndim(); i++) {
		if ((Int)i != dirAxisNumbers[1]) {
			axisPath.append(IPosition(1, j));
			j++;
			if (collapsed->shape()[i] == 1) {
				keep.append(IPosition(1, i));
			}
		}
	}
	// now remove the degenerate linear axis
	SubImage<Float> cDropped = SubImageFactory<Float>::createSubImage(
		*collapsed, Record(), "", 0, False, AxesSpecifier(keep, axisPath),
		False, True
	);
	std::auto_ptr<ArrayLattice<Bool> > newMask;
	if (dynamic_cast<TempImage<Float> *>(collapsed.get())->hasPixelMask()) {
		// because the mask doesn't lose its degenerate axis when subimaging.
		Array<Bool> newArray = collapsed->pixelMask().get().reform(cDropped.shape());
		newMask.reset(new ArrayLattice<Bool>(cDropped.shape()));
		newMask->put(newArray);
	}
	return _prepareOutputImage(cDropped, 0, newMask.get());
}

void PVGenerator::setOffsetUnit(const String& s) {
	Quantity q(1, s);
	ThrowIf(
		! q.isConform("rad"),
		s + " is not a unit of angular measure"
	);
	_unit = s;
}

String PVGenerator::getClass() const {
	return _class;
}

void PVGenerator::_checkWidth(const Int64 xShape, const Int64 yShape) const {
	*_getLog() << LogOrigin(_class, __func__, WHERE);
	if (_width == 1) {
		return;
	}
	vector<Double> start = *_start;
	vector<Double> end = *_end;

	Double angle = (start[0] == end[0])
		? 0 : atan2((end[1] - start[1]),(end[0] - start[0])) + C::pi_2;
	Double halfwidth = (_width - 1)/2;

	Double delX = halfwidth * cos(angle);
	Double delY = halfwidth * sin(angle);
	if (
		start[0] - delX < 0 || start[0] + delX > xShape
		|| start[1] - delY < 0 || start[1] + delY > yShape
		|| end[0] - delX < 0 || end[0] + delX > xShape
		|| end[1] - delY < 0 || end[1] + delY > yShape
	) {
		*_getLog() << LogIO::WARN << "The half width chosen is too large "
			<< "to include all pixels along the chosen slice. The half "
			<< "width extends beyond the image edge(s) at some location(s)."
			<< LogIO::POST;
	}
}

}


