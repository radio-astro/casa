//# tSubImage.cc: Test program for class SubImage
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

#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <measures/Measures/MDirection.h>

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/ImageAnalysis/ImagePadder.h>

#include <iomanip>

#include <tables/Tables/PlainTable.h>

namespace casa {

const String PVGenerator::_class = "PVGenerator";

PVGenerator::PVGenerator(
	const ImageInterface<Float> *const image,
	const Record *const &regionRec,
	const String& chanInp, const String& stokes,
	const String& maskInp, const String& outname,
	const Bool overwrite
) : ImageTask(
		image, "", regionRec, "", chanInp, stokes,
		maskInp, outname, overwrite
	), _start(0), _end(0), _halfwidth(0) {
	_construct();
}

PVGenerator::~PVGenerator() {}

void PVGenerator::setEndpoints(
	const Double startx, const Double starty,
	const Double endx, const Double endy
) {
	if (startx == endx && starty == endy) {
		*_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);
		*_getLog() << "Start and end pixels must be different."
			<< LogIO::EXCEPTION;
	}
	if (startx < 0 || endx < 0 || starty < 0 || endy < 0) {
		*_getLog() << "Pixel positions cannot fall below image BLC" << LogIO::EXCEPTION;
	}
	Vector<Int> dirAxes = _getImage()->coordinates().directionAxesNumbers();
	Int xShape = _getImage()->shape()[dirAxes[0]];
	Int yShape = _getImage()->shape()[dirAxes[1]];
	if (startx > xShape-1 || endx > xShape-1 || starty > yShape-1 || endy > yShape-1) {
		*_getLog() << "pixel positions cannot fall above image TRC" << LogIO::EXCEPTION;
	}

	_start.reset(new vector<Double>(2));
	_end.reset(new vector<Double>(2));
	(*_start)[0] = startx;
	(*_start)[1] = starty;
	(*_end)[0] = endx;
	(*_end)[1] = endy;
}

void PVGenerator::setHalfWidth(const Double halfwidth) {
	_halfwidth = halfwidth;
}

ImageInterface<Float>* PVGenerator::generate(const Bool wantReturn) const {
	*_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);

	if (_start.get() == 0 || _end.get() == 0) {
		*_getLog() << "Start and/or end points have not been set" << LogIO::EXCEPTION;
	}
	std::auto_ptr<ImageInterface<Float> > myClone(_getImage()->cloneII());
	SubImage<Float> subImage = SubImageFactory<Float>::createSubImage(
		*myClone, *_getRegion(), _getMask(),
		_getLog().get(), False, AxesSpecifier(), _getStretch()
	);
	*_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);
	const CoordinateSystem& subCoords = subImage.coordinates();
	Vector<Int> dirAxes = subCoords.directionAxesNumbers();
	Int xAxis = dirAxes[0];
	Int yAxis = dirAxes[1];
	IPosition subShape = subImage.shape();
	IPosition origShape = _getImage()->shape();
	if (
		subShape[xAxis] != origShape[xAxis]
		|| subShape[yAxis] != origShape[yAxis]
	) {
		*_getLog() << "You are not permitted to make a region selection "
			<< "in the direction coordinate" << LogIO::EXCEPTION;
	}
	_checkWidth(subShape[xAxis], subShape[yAxis]);
	*_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);
	// get the PA of the end points
	vector<Double> start = *_start;
	vector<Double> end = *_end;
	Double paInRad = start[1] == end[1] ?
		start[0] < end[0]
		    ? 0 : C::pi
		: atan2(
			end[0] - start[0], end[1] - start[1]
		) - C::pi_2;
	// rotate the image through this angle, in the opposite direction.
	*_getLog() << LogIO::NORMAL << "Rotating image by "
		<< (paInRad*180/C::pi)
		<< " degrees to align specified slice with the x axis" << LogIO::POST;

	// rotation occurs about the reference pixel. Calculate the bounds of the
	// output image.
	Vector<Double> refPix = subCoords.referencePixel();
	Double refX = refPix[xAxis];
	Double refY = refPix[yAxis];
	Double startXDiff = start[0] - refX;
	Double startYDiff = start[1] - refY;
	Double endXDiff = end[0] - refX;
	Double endYDiff = end[1] - refY;


	Double startXRot = startXDiff*cos(paInRad) - startYDiff*sin(paInRad) + refX;
	Double startYRot = startXDiff*sin(paInRad) + startYDiff*cos(paInRad) + refY;
	Double endXRot = endXDiff*cos(paInRad) - endYDiff*sin(paInRad) + refX;
	Double endYRot = endXDiff*sin(paInRad) + endYDiff*cos(paInRad) + refY;

	AlwaysAssert(abs(startYRot - endYRot) < 1e-6, AipsError);
	Double xdiff = fabs(end[0] - start[0]);
	Double ydiff = fabs(end[1] - start[1]);
	AlwaysAssert(
		abs(
			(endXRot - startXRot)
			- sqrt(xdiff*xdiff + ydiff*ydiff)
		) < 1e-6, AipsError
	);
	Double padNumber = max(0.0, 1 - startXRot);
	padNumber = max(padNumber, -(startYRot - _halfwidth - 1));
	ImageInterface<Float> *imageToRotate = &subImage;
	std::auto_ptr<ImageInterface<Float> > padded;
	Int nPixels = 0;
	if (padNumber > 0) {
		nPixels = (Int)padNumber + 1;
		*_getLog() << LogIO::NORMAL
			<< "Some pixels will fall outside the rotated image, so "
			<< "padding before rotating with " << nPixels << " pixels."
			<< LogIO::POST;
		ImagePadder padder(&subImage);
		padder.setPaddingPixels(nPixels);
		padded.reset(padder.pad(True));
		imageToRotate = padded.get();
	}
	IPosition outShape = subShape;
	outShape[xAxis] = (Int)(endXRot) + nPixels + 3;
	outShape[yAxis] = (Int)(startYRot + _halfwidth) + nPixels + 3;

	IPosition blc(subImage.ndim(), 0);
	IPosition trc = subShape - 1;

	// ensure we have enough real estate after the rotation
	blc[xAxis] = (Int)max(min(start[0], end[0]) - 1 - _halfwidth, 0.0);
	blc[yAxis] = (Int)max(min(start[1], end[1]) - 1 - _halfwidth, 0.0);
	trc[xAxis] = (Int)min(
		max(start[0], end[0]) + 1 + _halfwidth,
		(Double)subShape[xAxis] - 1
	) + nPixels;
	trc[yAxis] = (Int)min(
		max(start[1], end[1]) + 1 + _halfwidth,
		(Double)subShape[yAxis] - 1
	) + nPixels;

	Record lcbox = LCBox(blc, trc, imageToRotate->shape()).toRecord("");
	ImageAnalysis ia(imageToRotate);
	std::auto_ptr<ImageInterface<Float> > rotated(
		ia.rotate(
			"", outShape.asVector(), Quantity(paInRad, "rad"),
			lcbox, ""
		)
	);
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
	// We have rotated so the position of the starting pixel x is smaller than
	// the ending pixel x.
	AlwaysAssert(rotPixStart[xAxis] < rotPixEnd[xAxis], AipsError);

	blc = IPosition(rotated->ndim(), 0);
	trc = rotated->shape() - 1;
	blc[xAxis] = (Int)(rotPixStart[xAxis] + 0.5);
	blc[yAxis] = (Int)(rotPixStart[yAxis] + 0.5 - _halfwidth);
	trc[xAxis] = (Int)(rotPixEnd[xAxis] + 0.5);
	trc[yAxis] = (Int)(rotPixEnd[yAxis] + 0.5 + _halfwidth);

	lcbox = LCBox(blc, trc, rotated->shape()).toRecord("");
	IPosition axes(1, yAxis);
	ImageCollapser collapser(
		"mean", rotated.get(), "", &lcbox,
		"", "", "", "", axes, "", False
	);
	std::auto_ptr<ImageInterface<Float> > collapsed(collapser.collapse(True));
	Vector<Double > newRefPix = rotCoords.referencePixel();
	newRefPix[xAxis] = rotPixStart[xAxis] - blc[xAxis];
	newRefPix[yAxis] = rotPixStart[yAxis] - blc[yAxis];

	CoordinateSystem collCoords = collapsed->coordinates();
	collCoords.setReferencePixel(newRefPix);
	Vector<Double> refVal = collCoords.referenceValue();
	refVal[xAxis] = startWorld[xAxis];
	refVal[yAxis] = startWorld[yAxis];
	collCoords.setReferenceValue(refVal);
	collapsed->setCoordinateInfo(collCoords);
	std::auto_ptr<ImageInterface<Float> > outImage = _prepareOutputImage(collapsed.get());

	if (wantReturn) {
		return outImage.release();
	}
	else {
		return 0;
	}
}

String PVGenerator::getClass() const {
	return _class;
}

void PVGenerator::_checkWidth(const Int64 xShape, const Int64 yShape) const {
	*_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);
	if (_halfwidth == 0) {
		return;
	}
	vector<Double> start = *_start;
	vector<Double> end = *_end;

	Double angle = (start[0] == end[0])
		? 0 : atan2((end[1] - start[1]),(end[0] - start[0])) + C::pi_2;
	Double delX = _halfwidth * cos(angle);
	Double delY = _halfwidth * sin(angle);
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


