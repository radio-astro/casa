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

Record* ImageAnalysis::coordmeasures(
	Quantum<Float>& intensity, Record& direction,
	Record& frequency, Record& velocity, const Vector<Double>& pixel
) {
	_onlyFloat(__func__);
	Record *r = 0;

	*_log << LogOrigin("ImageAnalysis", "coordmeasures");

	CoordinateSystem cSys = _imageFloat->coordinates();

	Vector<Double> vpixel(pixel);
	if (pixel.size() == 0) {
		vpixel = cSys.referencePixel();
	}

	String format("m");
	ImageMetaData imd(_imageFloat);
	r = new Record(imd.toWorld(vpixel, format));

	Vector<Int> ipixel(vpixel.size());
	convertArray(ipixel, vpixel);

	//    Record *pR = this->pixelvalue(ipixel);
	Bool offImage;
	Quantum<Double> value;
	Bool mask(False);
	PixelValueManipulator<Float> pvm(_imageFloat, nullptr, "");
	pvm.pixelValue(offImage, intensity, mask, ipixel);
	if (offImage)
		return r;

	r->define(RecordFieldId("mask"), mask);

	if (r->isDefined("direction")) {
		direction = r->asRecord("direction");
	}
	if (r->isDefined("spectral")) {
		Record specRec = r->asRecord("spectral");
		if (specRec.isDefined("frequency")) {
			frequency = specRec.asRecord("frequency");
		}
		if (specRec.isDefined("radiovelocity")) {
			velocity = specRec.asRecord("radiovelocity");
		}
	}
	return r;
}

Matrix<Float> ImageAnalysis::decompose(
	Record& Region, const String& mask,
	const Bool simple, const Double Threshold, const Int nContour,
	const Int minRange, const Int nAxis, const Bool fit,
	const Double maxrms, const Int maxRetry, const Int maxIter,
	const Double convCriteria
) {
	_onlyFloat(__func__);

	Matrix<Int> blcs;
	Matrix<Int> trcs;
	return decompose(
		blcs, trcs, Region, mask, simple, Threshold, nContour, minRange,
		nAxis, fit, maxrms, maxRetry, maxIter, convCriteria
	);
}

Matrix<Float> ImageAnalysis::decompose(
	Matrix<Int>& blcs, Matrix<Int>& trcs, Record& Region, const String& mask,
	const Bool simple, const Double Threshold, const Int nContour,
	const Int minRange, const Int nAxis, const Bool fit,
	const Double maxrms, const Int maxRetry, const Int maxIter,
	const Double convCriteria, const Bool stretch
) {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", __func__);

	if (Threshold < 0) {
		*_log << "Threshold cannot be negative " << LogIO::EXCEPTION;
	}

	Float threshold(Threshold);

	AxesSpecifier axesSpec(False);
	SHARED_PTR<const SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRO(
		*_imageFloat, Region, mask,	_log.get(), axesSpec, stretch
	);
	// Make finder
	ImageDecomposer<Float> decomposer(*subImage);
	decomposer.setDeblend(!simple);
	decomposer.setDeblendOptions(threshold, nContour, minRange, nAxis);
	decomposer.setFit(fit);
	decomposer.setFitOptions(maxrms, maxRetry, maxIter, convCriteria);
	decomposer.decomposeImage();
	decomposer.printComponents();
	Block<IPosition> blcspos(decomposer.numRegions());
	Block<IPosition> trcspos(decomposer.numRegions());
	decomposer.boundRegions(blcspos, trcspos);
	if(blcspos.nelements() >0){
	  blcs.resize(blcspos.nelements(), blcspos[0].asVector().nelements());
	  trcs.resize(trcspos.nelements(), trcspos[0].asVector().nelements());
	  for (uInt k=0; k < blcspos.nelements(); ++k){
	    blcs.row(k)=blcspos[k].asVector();
	    trcs.row(k)=trcspos[k].asVector();
	  }
	}
	// As yet no methods to put the results into an output container
	// (Note: component list can be output as a Matrix.)
	Matrix<Float> cl = decomposer.componentList();
	return cl;

}

Record ImageAnalysis::deconvolvecomponentlist(
	const Record& compList, const Int channel, const Int polarization
) {
	_onlyFloat(__func__);
	Record retval;
	*_log << LogOrigin(className(), __func__);

	String error;
	ComponentList cl;
	if (! cl.fromRecord(error, compList)) {
		*_log << "Can not  convert input parameter to ComponentList "
				<< error << LogIO::EXCEPTION;
		return retval;
	}

	uInt n = cl.nelements();
	Vector<SkyComponent> list(n);
	for (uInt i = 0; i < n; i++) {
		list(i) = cl.component(i);
	}

	// Do we have a beam ?
	GaussianBeam beam = _imageFloat->imageInfo().restoringBeam(channel, polarization);
	if (beam.isNull()) {
		*_log << "This image does not have a restoring beam"
				<< LogIO::EXCEPTION;
	}
	const CoordinateSystem cSys = _imageFloat->coordinates();
	Int dirCoordinate = cSys.findCoordinate(Coordinate::DIRECTION);
	if (dirCoordinate == -1) {
		*_log
		<< "This image does not contain a DirectionCoordinate - cannot deconvolve"
		<< LogIO::EXCEPTION;
	}
	// Loop over components and deconvolve
	n = list.nelements();
	ComponentList outCL;
	for (uInt i = 0; i < n; ++i) {
		outCL.add(SkyComponentFactory::deconvolveSkyComponent(*_log, list(i), beam));
	}
	if (outCL.nelements() > 0) {
		if (!outCL.toRecord(error, retval)) {
			*_log << "Cannot  convert deconvolved ComponentList to Record"
					<< error << LogIO::EXCEPTION;
		}
	}
	return retval;
}

Record ImageAnalysis::findsources(const int nMax, const double cutoff,
		Record& Region, const String& mask, const Bool point, const Int width,
		const Bool absFind) {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", "findsources");

	AxesSpecifier axesSpec(False);
	SHARED_PTR<const SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRO(
		*_imageFloat, Region, mask, _log.get(), axesSpec
	);
	// Make finder
	ImageSourceFinder<Float> sf(*subImage);

	// Find them
	ComponentList list = sf.findSources(*_log, nMax, cutoff, absFind, point,
			width);
	String error;
	Record listOut;
	if (!list.toRecord(error, listOut)) {
		*_log << "Can not convert output ComponentList to Record " << error
				<< LogIO::EXCEPTION;
	}
	return listOut;
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

Record*
ImageAnalysis::getslice(const Vector<Double>& x, const Vector<Double>& y,
		const Vector<Int>& axes, const Vector<Int>& coord, const Int npts,
		const String& method) {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", "getslice");

	Vector<Float> xPos;
	Vector<Float> yPos;
	Vector<Float> distance;
	Vector<Float> pixels;
	Vector<Bool> pixelMask;

	// Construct PixelCurve.  FIll in defaults for x, y vectors
	PixelCurve1D curve(x, y, npts);

	// Set coordinates
	IPosition iCoord = IPosition(Vector<Int> (coord));
	IPosition iAxes = IPosition(Vector<Int> (axes));

	// Get the Slice
	LatticeSlice1D<Float>::Method method2 =
			LatticeSlice1D<Float>::stringToMethod(method);
	LatticeSlice1D<Float> slicer(*_imageFloat, method2);
	slicer.getSlice(pixels, pixelMask, curve, iAxes(0), iAxes(1), iCoord);

	// Get slice locations
	uInt axis0, axis1;
	slicer.getPosition(axis0, axis1, xPos, yPos, distance);

	RecordDesc outRecDesc;
	outRecDesc.addField("pixel", TpArrayFloat);
	outRecDesc.addField("mask", TpArrayBool);
	outRecDesc.addField("xpos", TpArrayFloat);
	outRecDesc.addField("ypos", TpArrayFloat);
	outRecDesc.addField("distance", TpArrayFloat);
	outRecDesc.addField("axes", TpArrayInt);
	Record *outRec = new Record(outRecDesc);
	outRec->define("pixel", pixels);
	outRec->define("mask", pixelMask);
	outRec->define("xpos", xPos);
	outRec->define("ypos", yPos);
	outRec->define("distance", distance);
	outRec->define("axes", Vector<Int> (axis0, axis1));
	return outRec;
}

Vector<Bool> ImageAnalysis::haslock() {
	_onlyFloat(__func__);
	Vector<Bool> rstat;
	*_log << LogOrigin("ImageAnalysis", "haslock");

	rstat.resize(2);
	rstat[0] = _imageFloat->hasLock(FileLocker::Read);
	rstat[1] = _imageFloat->hasLock(FileLocker::Write);
	return rstat;
}

Bool ImageAnalysis::ispersistent() {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", "ispersistent");
	return _imageFloat->isPersistent();
}

Bool ImageAnalysis::lock(const Bool writelock, const Int nattempts) {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", "lock");

	FileLocker::LockType locker = FileLocker::Read;
	if (writelock)
		locker = FileLocker::Write;
	uInt n = max(0, nattempts);
	return _imageFloat->lock(locker, n);
}

Bool ImageAnalysis::makecomplex(const String& outFile, const String& imagFile,
		Record& Region, const Bool overwrite) {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", "makecomplex");

	// Check output file
	if (!overwrite && !outFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*_log << errmsg << LogIO::EXCEPTION;
		}
	}

	// Open images and check consistency
	PagedImage<Float> imagImage(imagFile);
	//
	const IPosition realShape = _imageFloat->shape();
	const IPosition imagShape = imagImage.shape();
	if (!realShape.isEqual(imagShape)) {
		*_log << "Image shapes are not identical" << LogIO::EXCEPTION;
	}
	//
	CoordinateSystem cSysReal = _imageFloat->coordinates();
	CoordinateSystem cSysImag = imagImage.coordinates();
	if (!cSysReal.near(cSysImag)) {
		*_log << "Image Coordinate systems are not conformant" << LogIO::POST;
	}

	String mask;
	SHARED_PTR<const SubImage<Float> > subRealImage = SubImageFactory<Float>::createSubImageRO(
		*_imageFloat, Region, mask, _log.get()
	);
	SHARED_PTR<const SubImage<Float> > subImagImage = SubImageFactory<Float>::createSubImageRO(
		imagImage, Region, mask, 0
	);

	// LEL node
	LatticeExprNode node(formComplex(*subRealImage, *subImagImage));
	LatticeExpr<Complex> expr(node);
	//
	PagedImage<Complex> outImage(realShape, cSysReal, outFile);
	outImage.copyData(expr);
	ImageUtilities::copyMiscellaneous(outImage, *_imageFloat);
	return True;
}

Vector<String> ImageAnalysis::maskhandler(const String& op,
		const Vector<String>& namesIn) {
	_onlyFloat(__func__);
	*_log << LogOrigin(className(), __func__);

	Vector<String> namesOut;
	Bool hasOutput;

	//
	String OP = op;
	OP.upcase();
	const uInt n = namesIn.nelements();
	hasOutput = False;
	//
	if (OP.contains(String("SET"))) {
		// Set new default mask.  Empty means unset default mask
		if (n == 0) {
			_imageFloat->setDefaultMask(String(""));
		} else {
			_imageFloat->setDefaultMask(namesIn(0));
		}
	} else if (OP.contains(String("DEF"))) {
		// Return default mask
		namesOut.resize(1);
		namesOut(0) = _imageFloat->getDefaultMask();
		hasOutput = True;
	} else if (OP.contains(String("DEL"))) {
		// Delete mask(s)
		if (n <= 0) {
			*_log << "You have not supplied any mask names"
					<< LogIO::EXCEPTION;
		}
		for (uInt i = 0; i < n; i++) {
			_imageFloat->removeRegion(namesIn(i), RegionHandler::Masks, False);
		}
	} else if (OP.contains(String("REN"))) {
		// Rename masks
		if (n != 2) {
			*_log << "You must give two mask names" << LogIO::EXCEPTION;
		}
		_imageFloat->renameRegion(namesIn(1), namesIn(0), RegionHandler::Masks,
				False);
	} else if (OP.contains(String("GET"))) {
		// Get names of all masks
		namesOut.resize(0);
		namesOut = _imageFloat->regionNames(RegionHandler::Masks);
		hasOutput = True;
	} else if (OP.contains(String("COP"))) {
		// Copy mask;  maskIn maskOut  or imageIn:maskIn maskOut
		if (n != 2) {
			*_log << "You must give two mask names" << LogIO::EXCEPTION;
		}
		Vector<String> mask2 = stringToVector(namesIn(0), ':');
		const uInt n2 = mask2.nelements();
		//
		String maskOut = namesIn(1);
		String maskIn, nameIn;
		Bool external = False;
		if (n2 == 1) {
			external = False;
			maskIn = mask2(0);
		} else if (n2 == 2) {
			external = True;
			nameIn = mask2(0);
			maskIn = mask2(1);
		} else {
			*_log << "Illegal number of mask names" << LogIO::EXCEPTION;
		}
		//
		if (_imageFloat->hasRegion(maskOut, RegionHandler::Any)) {
			*_log << "The mask " << maskOut << " already exists in image "
					<< _imageFloat->name() << LogIO::EXCEPTION;
		}

		// Create new mask in output
		_imageFloat->makeMask(maskOut, True, False);

		// Copy masks
		ImageInterface<Float>* pImIn = 0;
		if (external) {
			pImIn = new PagedImage<Float> (nameIn);
			if (pImIn->shape() != _imageFloat->shape()) {
				*_log << "Images have different shapes" << LogIO::EXCEPTION;
			}
		} else {
			pImIn = _imageFloat.get();
		}
		//
		AxesSpecifier axesSpecifier;
		ImageUtilities::copyMask(*_imageFloat, *pImIn, maskOut, maskIn,
				axesSpecifier);
		//
		if (external) {
			delete pImIn;
			pImIn = 0;
		}
	} else {
		*_log << "Unknown operation" << LogIO::EXCEPTION;
	}
	if (hasOutput)
		return namesOut;
	return Vector<String> (0);
}

Record ImageAnalysis::miscinfo() {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", "miscinfo");

	Record tmp = _imageFloat->miscInfo();
	return tmp;
}

Bool ImageAnalysis::modify(
	Record& Model, Record& Region, const String& mask,
	const Bool subtract, const Bool list, const Bool extendMask) {
	_onlyFloat(__func__);
	*_log << LogOrigin(className(), __func__);

	String error;
	ComponentList cL;
	if (!cL.fromRecord(error, Model)) {
		*_log << LogIO::SEVERE
				<< "Failed to transform model record to ComponentList "
				<< error << LogIO::POST;
		return False;
	}
	int nelem = cL.nelements();
	Vector<SkyComponent> mod(nelem);
	for (int i = 0; i < nelem; i++) {
		mod[i] = cL.component(i);
	}

	const uInt n = mod.nelements();
	if (n == 0) {
		*_log << "There are no components in the model componentlist"
				<< LogIO::EXCEPTION;
	}

	SHARED_PTR<SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRW(
		*_imageFloat, Region, mask,  (list ? _log.get() : 0),
        AxesSpecifier(), extendMask
	);

	// Allow for subtraction/addition
	ComponentList cl;
	for (uInt i = 0; i < n; i++) {
		SkyComponent sky = mod(i);
		if (subtract)
			sky.flux().scaleValue(-1.0);
		cl.add(sky);
	}

	// Do it
	ComponentImager::project(*subImage, cl);

	return True;
}

Bool ImageAnalysis::twopointcorrelation(
	const String& outFile,
	Record& theRegion, const String& mask, const Vector<Int>& axes1,
	const String& method, const Bool overwrite, const Bool stretch
) {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", __func__);

	// Validate outfile
	if (!overwrite && !outFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*_log << errmsg << LogIO::EXCEPTION;
		}
	}

	AxesSpecifier axesSpecifier;
	SHARED_PTR<const SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRO(
		*_imageFloat, theRegion, mask, _log.get(), axesSpecifier, stretch
	);

	// Deal with axes and shape
	Vector<Int> axes2(axes1);
	CoordinateSystem cSysIn = subImage->coordinates();
	IPosition axes = ImageTwoPtCorr<Float>::setUpAxes(IPosition(axes2), cSysIn);
	IPosition shapeOut = ImageTwoPtCorr<Float>::setUpShape(subImage->shape(),
			axes);

	// Create the output image and mask
	PtrHolder<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*_log << LogIO::NORMAL << "Creating (temp)image of shape "
				<< shapeOut << LogIO::POST;
		imOut.set(new TempImage<Float> (shapeOut, cSysIn));
	} else {
		*_log << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << shapeOut << LogIO::POST;
		imOut.set(new PagedImage<Float> (shapeOut, cSysIn, outFile));
	}
	ImageInterface<Float>* pImOut = imOut.ptr();
	String maskName("");
	ImageMaskAttacher::makeMask(*pImOut, maskName, True, True, *_log, True);

	// Do the work.  The Miscellaneous items and units are dealt with
	// by function ImageTwoPtCorr::autoCorrelation
	ImageTwoPtCorr<Float> twoPt;
	Bool showProgress = True;
	LatticeTwoPtCorr<Float>::Method m = LatticeTwoPtCorr<Float>::fromString(
			method);
	twoPt.autoCorrelation(*pImOut, *subImage, axes, m, showProgress);

	return True;
}

void ImageAnalysis::centreRefPix(CoordinateSystem& cSys, const IPosition& shape) const {
	Int after = -1;
	Int iS = cSys.findCoordinate(Coordinate::STOKES, after);
	Int sP = -1;
	if (iS >= 0) {
		Vector<Int> pixelAxes = cSys.pixelAxes(iS);
		sP = pixelAxes(0);
	}
	Vector<Double> refPix = cSys.referencePixel();
	for (Int i = 0; i < Int(refPix.nelements()); i++) {
		if (i != sP)
			refPix(i) = Double(shape(i) / 2);
	}
	cSys.setReferencePixel(refPix);
}

void ImageAnalysis::_onlyFloat(const String& method) const {
	ThrowIf(! _imageFloat, "Method " + method + " only supports Float valued images");
}

}
