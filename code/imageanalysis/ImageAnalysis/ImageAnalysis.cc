//# ImageAnalysis.cc:  Image analysis and handling tool
//# Copyright (C) 1995-2007
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
//# $Id: ImageAnalysis.cc 20491 2009-01-16 08:33:56Z gervandiepen $
//   

// PLEASE DO *NOT* ADD ADDITIONAL METHODS TO THIS CLAS

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/fstream.h>
#include <casa/OS/Directory.h>
#include <casa/OS/File.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <components/ComponentModels/Flux.h>
#include <images/Images/ImageRegrid.h>
#include <imageanalysis/ImageAnalysis/ComponentImager.h>
#include <imageanalysis/ImageAnalysis/ImageTwoPtCorr.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/ImageDecomposer.h>
#include <imageanalysis/ImageAnalysis/ImageFactory.h>
#include <imageanalysis/ImageAnalysis/ImageSourceFinder.h>
#include <imageanalysis/ImageAnalysis/PixelValueManipulator.h>
#include <imageanalysis/ImageAnalysis/SepImageConvolver.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <lattices/LatticeMath/LatticeSlice1D.h>
#include <lattices/Lattices/MaskedLatticeIterator.h>
#include <lattices/Lattices/PixelCurve1D.h>
#include <lattices/LRegions/RegionType.h>

#include <casa/OS/PrecTimer.h>

#include <casa/namespace.h>

#include <iostream>
#include <memory>

using namespace std;

namespace casa { //# name space casa begins

ImageAnalysis::ImageAnalysis() :
	_imageFloat(), _imageComplex() {

	// Register the functions to create a FITSImage or MIRIADImage object.
	FITSImage::registerOpenFunction();
	MIRIADImage::registerOpenFunction();

	_log.reset(new LogIO());

}

ImageAnalysis::ImageAnalysis(SPIIF image) :
	_imageFloat(image),_imageComplex(), _log(new LogIO()) {
}

ImageAnalysis::ImageAnalysis(SPIIC image) :
	_imageFloat(),_imageComplex(image), _log(new LogIO()) {}

ImageAnalysis::~ImageAnalysis() {
	if (_imageFloat) {
		_destruct(*_imageFloat);
	}
	if (_imageComplex) {
		_destruct(*_imageComplex);
	}
	//deleteHist();
}

Bool ImageAnalysis::toRecord(RecordInterface& rec) {
	_onlyFloat(__func__);
	if (_imageFloat.get() != 0) {
		String err;
		return _imageFloat->toRecord(err, rec);

	}

	return False;
}

Bool ImageAnalysis::open(const String& infile) {
	// Generally used if the image is already closed !b
	if (_imageFloat || _imageComplex) {
		*_log << LogIO::WARN << "Another image is already open, closing first"
				<< LogIO::POST;
		// The pointer does explicitly need to be reset for proper destruction
		// of the image esp if the image trying to be opened is the same
		// as the image stored in the pre-existing pointer.
		_imageFloat.reset();
		_imageComplex.reset();
	}
	pair<SPIIF, SPIIC> ret = ImageFactory::fromFile(infile);
	_imageFloat = ret.first;
	_imageComplex = ret.second;
	return True;
}

Bool ImageAnalysis::detached() {
	return _imageFloat.get() == 0 && _imageComplex.get() == 0;
}

void ImageAnalysis::calc(const String& expr, Bool verbose) {
	*_log << LogOrigin(className(), __func__);
	ThrowIf(expr.empty(), "You must specify an expression");
	Record regions;
	Block<LatticeExprNode> temps;
	PtrBlock<const ImageRegion*> tempRegs;
	PixelValueManipulator<Float>::makeRegionBlock(tempRegs, regions);
	LatticeExprNode node = ImageExprParse::command(expr, temps, tempRegs);
	DataType type = node.dataType();
	Bool isReal = casa::isReal(type);
	ostringstream os;
	os << type;
	ThrowIf(
		! isReal && ! isComplex(type),
		"Unsupported node data type " + os.str()
	);
	ThrowIf(
		_imageComplex && isReal,
		"Resulting image is real valued but"
		"the attached image is complex valued"
	);
	ThrowIf(
		_imageFloat && isComplex(type),
		"Resulting image is complex valued but"
		"the attached image is real valued"
	);
	PixelValueManipulator<Float>::makeRegionBlock(tempRegs, Record());
	if (verbose) {
		*_log << LogIO::WARN << "Overwriting pixel values "
			<< "of the currently attached image"
			<< LogIO::POST;
	}
	if (_imageFloat) {
		_calc(_imageFloat, node);
	}
	else {
		_calc(_imageComplex, node);
	}
}

Bool ImageAnalysis::calcmask(
	const String& mask, Record& regions,
	const String& maskName, const Bool makeDefault
) {
	*_log << LogOrigin(className(), __func__);

	// Get LatticeExprNode (tree) from parser
	// Convert the GlishRecord containing regions to a
	// PtrBlock<const ImageRegion*>.
	ThrowIf(
		mask.empty(),
		"You must specify an expression"
	);
	Block<LatticeExprNode> temps;
	PtrBlock<const ImageRegion*> tempRegs;
	PixelValueManipulator<Float>::makeRegionBlock(tempRegs, regions);
	LatticeExprNode node = ImageExprParse::command(mask, temps, tempRegs);

	// Delete the ImageRegions
	PixelValueManipulator<Float>::makeRegionBlock(tempRegs, Record());

	// Make sure the expression is Boolean
	DataType type = node.dataType();
	ThrowIf(
		type != TpBool,
		"The expression type must be Boolean"
	);

	Bool res = _imageFloat
		? _calcmask(
			_imageFloat, node, maskName, makeDefault
		)
		: _calcmask(
			_imageComplex, node, maskName, makeDefault
		);
	return res;
}

CoordinateSystem ImageAnalysis::coordsys(const Vector<Int>& pixelAxes) {
	*_log << LogOrigin(className(), __func__);

	// Recover CoordinateSytem into a Record
	Record rec;
	CoordinateSystem cSys = _imageFloat
		? _imageFloat->coordinates()
		: _imageComplex->coordinates();
	CoordinateSystem cSys2;

	// Fish out the coordinate of the desired axes
	uInt j = 0;
	if (pixelAxes.size() > 0 && !(pixelAxes.size() == 1 && pixelAxes[0] == -1)) {
		Vector<Int> axes(pixelAxes);
		//
		const Int nPixelAxes = cSys.nPixelAxes();
		Vector<uInt> coordinates(cSys.nCoordinates(), uInt(0));
		Int coord, axisInCoord;
		for (uInt i = 0; i < axes.nelements(); i++) {
			if (axes(i) >= 0 && axes(i) < nPixelAxes) {
				cSys.findPixelAxis(coord, axisInCoord, uInt(axes(i)));
				if (coord != -1) {
					coordinates(coord)++;
					// Copy desired coordinate (once)
					if (coordinates(coord) == 1)
						cSys2.addCoordinate(cSys.coordinate(coord));
				} else {
					// Axis removed.  Better give up.
					*_log << "Pixel axis " << axes(i) + 1
							<< " has been removed" << LogIO::EXCEPTION;
				}
			} else {
				*_log << "Specified pixel axis " << axes(i) + 1
						<< " is not a valid pixelaxis" << LogIO::EXCEPTION;
			}
		}
		//
		// Find mapping.  Says where world axis i in cSys is in cSys2
		//
		Vector<Int> worldAxisMap, worldAxisTranspose;
		Vector<Bool> refChange;
		if (!cSys2.worldMap(worldAxisMap, worldAxisTranspose, refChange, cSys)) {
			*_log << "Error finding world map because "
					<< cSys2.errorMessage() << LogIO::EXCEPTION;
		}
		//
		// Generate list of world axes to keep
		//
		Vector<Int> keepList(cSys.nWorldAxes());
		Vector<Double> worldReplace;
		j = 0;
		//
		for (uInt i = 0; i < axes.nelements(); i++) {
			if (axes(i) >= 0 && axes(i) < nPixelAxes) {
				Int worldAxis = cSys.pixelAxisToWorldAxis(uInt(axes(i)));
				if (worldAxis >= 0) {
					keepList(j++) = worldAxisMap(worldAxis);
				} else {
					*_log << "World axis corresponding to pixel axis "
							<< axes(i) + 1 << " has been removed"
							<< LogIO::EXCEPTION;
				}
			}
		}

		// Remove unwanted world (and pixel) axes.  Better would be to just
		// remove the pixel axes and leave the world axes there...
		if (j > 0) {
			keepList.resize(j, True);
			CoordinateUtil::removeAxes(cSys2, worldReplace, keepList, False);
		}

		// Copy the ObsInfo
		cSys2.setObsInfo(cSys.obsInfo());
	}
	else {
		cSys2 = cSys;
	}

	// Return coordsys object
	return cSys2;
}

SPCIIC ImageAnalysis::getComplexImage() const {
	ThrowIf(
		_imageFloat,
		"This object currently holds a Float valued image. Use "
		"getImage instead"
	);
	ThrowIf(
		! _imageComplex,
		"This image does not hold a valid Complex valued image"
	);
	return _imageComplex;
}

SPIIC ImageAnalysis::getComplexImage() {
	ThrowIf(
		_imageFloat,
		"This object currently holds a Float valued image. Use "
		"getImage instead"
	);
	ThrowIf(
		! _imageComplex,
		"This image does not hold a valid Complex valued image"
	);
	return _imageComplex;
}

SPIIF ImageAnalysis::getImage() {
	ThrowIf(
		_imageComplex,
		"This object currently holds a Complex valued image. Use "
		"getComplexImage instead"
	);
	ThrowIf(
		! _imageFloat,
		"This image does not hold a valid Float valued image"
	);
	return _imageFloat;
}

SPCIIF ImageAnalysis::getImage() const {
	ThrowIf(
		_imageComplex,
		"This object currently holds a Complex valued image. Use "
		"getComplexImage instead"
	);
	ThrowIf(
		! _imageFloat,
		"This image does not hold a valid Float valued image"
	);
	return _imageFloat;
}

void ImageAnalysis::_onlyFloat(const String& method) const {
	ThrowIf(! _imageFloat, "Method " + method + " only supports Float valued images");
}

}
