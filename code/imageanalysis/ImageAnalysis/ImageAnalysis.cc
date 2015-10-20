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
#include <casa/BasicSL/String.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/fstream.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/OS/Directory.h>
#include <casa/OS/File.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Regex.h>
#include <measures/Measures/MeasureHolder.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MPosition.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/GaussianShape.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/SkyCompRep.h>
#include <components/ComponentModels/TwoSidedShape.h>
#include <components/SpectralComponents/SpectralElement.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/GaussianConvert.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <imageanalysis/ImageAnalysis/ComponentImager.h>
#include <imageanalysis/ImageAnalysis/ImageConvolver.h>
#include <images/Images/ImageExprParse.h>
#include <images/Regions/WCEllipsoid.h>
#include <imageanalysis/ImageAnalysis/ImageMoments.h>
#include <images/Images/ImageOpener.h>
#include <images/Regions/ImageRegion.h>
#include <images/Images/ImageRegrid.h>
#include <images/Images/ImageStatistics.h>
#include <imageanalysis/ImageAnalysis/ImageTwoPtCorr.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/LELImageCoord.h>
#include <images/Images/PagedImage.h>
#include <images/Regions/RegionManager.h>
#include <imageanalysis/ImageAnalysis/SepImageConvolver.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/TempImage.h>
#include <images/Regions/WCLELMask.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/ImageDecomposer.h>
#include <imageanalysis/ImageAnalysis/ImageFactory.h>
#include <imageanalysis/ImageAnalysis/ImageSourceFinder.h>
#include <imageanalysis/ImageAnalysis/PixelValueManipulator.h>
#include <lattices/LatticeMath/LatticeFit.h>
#include <lattices/LatticeMath/LatticeAddNoise.h>
#include <lattices/LEL/LatticeExprNode.h>
#include <lattices/LEL/LatticeExprNode.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/LRegions/LatticeRegion.h>
#include <lattices/LatticeMath/LatticeSlice1D.h>
#include <lattices/Lattices/LatticeUtilities.h>
#include <lattices/LRegions/LCBox.h>
#include <lattices/LRegions/LCSlicer.h>
#include <lattices/Lattices/MaskedLatticeIterator.h>
#include <lattices/Lattices/PixelCurve1D.h>
#include <lattices/LRegions/RegionType.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <scimath/Fitting/LinearFitSVD.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/VectorKernel.h>
#include <tables/LogTables/NewFile.h>
#include <images/Images/MIRIADImage.h>

#include <casa/OS/PrecTimer.h>

#include <casa/namespace.h>

#include <iostream>
#include <memory>

using namespace std;

namespace casa { //# name space casa begins

ImageAnalysis::ImageAnalysis() :
	_imageFloat(), _imageComplex(),
			imageMomentsProgressMonitor(0){

	// Register the functions to create a FITSImage or MIRIADImage object.
	FITSImage::registerOpenFunction();
	MIRIADImage::registerOpenFunction();

	_log.reset(new LogIO());

}

ImageAnalysis::ImageAnalysis(SPIIF image) :
	_imageFloat(image),_imageComplex(), _log(new LogIO()),
				imageMomentsProgressMonitor(0) {}


ImageAnalysis::ImageAnalysis(SPIIC image) :
	_imageFloat(),_imageComplex(image), _log(new LogIO()),
				imageMomentsProgressMonitor(0) {}


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

Bool ImageAnalysis::fromRecord(const RecordInterface& rec, const String& name) {
	Bool retval = False;
	String err;
	if (name != "") {
		TempImage<Float> tempim;
		retval = tempim.fromRecord(err, rec);
		if (retval) {
			{
				PagedImage<Float> diskim(tempim.shape(), tempim.coordinates(),
						name);
				diskim.copyData(tempim);
				// go out of context hence flush to disk
			}
			retval = open(name);
		}
	} else {
		if (_log.get() == 0)
			_log.reset(new LogIO());
		if (_imageFloat.get() != 0) {
			*_log << LogOrigin("ImageAnalysis", "fromRecord");
			*_log << LogIO::WARN
					<< "Image is already open, disconnecting first"
					<< LogIO::POST;
		}
		_imageFloat.reset(new TempImage<Float> ());
		retval = _imageFloat->fromRecord(err, rec);

	}
	return retval;
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

void ImageAnalysis::addnoise(
	const String& type, const Vector<Double>& pars,
	const Record& region, const Bool zeroIt,
	const std::pair<Int, Int> *const &seeds
) {
	_onlyFloat(__func__);
	*_log << LogOrigin(className(), __func__);
	String mask;
	SHARED_PTR<SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRW(
		*_imageFloat, region, mask, _log.get()
	);
	if (zeroIt) {
		subImage->set(0.0);
	}
	Random::Types typeNoise = Random::asType(type);
	SHARED_PTR<LatticeAddNoise> lan(
		seeds
		? new LatticeAddNoise(typeNoise, pars, seeds->first, seeds->second)
		: new LatticeAddNoise(typeNoise, pars)
	);
	lan->add(*subImage);
	//deleteHist();
}

Bool ImageAnalysis::imagefromascii(const String& outfile, const String& infile,
		const Vector<Int>& shape, const String& sep, const Record& csys,
		const Bool linear, const Bool overwrite) {
	try {
		*_log << LogOrigin("ImageAnalysis", "imagefromascii");

		Path filePath(infile);
		String fileName = filePath.expandedName();

		ifstream inFile(fileName.c_str());
		if (!inFile) {
			*_log << LogIO::SEVERE << "Cannot open " << infile << LogIO::POST;
			return false;
		}

		IPosition shp(shape);
		uInt n = shp.product();
		uInt nx = shp(0);
		Vector<Float> a(n, 0.0);
		int idx = 0;
		string line;
		string *line2 = new string[2 * nx];
		uInt iline = 0;
		uInt nl = 1;
		while (nl > 0) {
			getline(inFile, line, '\n');
			nl = split(line, line2, 2 * nx, sep);
			if (nl > 0) {
				if (nl != nx) {
					*_log << LogIO::SEVERE << "Length of line " << iline
							<< " is " << nl << " but should be " << nx
							<< LogIO::POST;
					return false;
				}
				for (uInt i = 0; i < nx; i++) {
					a[idx + i] = atof(line2[i].c_str());
				}
				idx += nx;
				iline += 1;
			}
		}
		delete [] line2;
		Vector<Float> vec(n);
		for (uInt i = 0; i < n; i++)
			vec[i] = a[i];
		Array<Float> pixels(vec.reform(IPosition(shape)));
		_imageComplex.reset();
		_imageFloat = ImageFactory::imageFromArray(outfile, pixels, csys, linear, overwrite);
	} catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		return False;
	}
	return True;
}

Bool ImageAnalysis::imagefromimage(const String& outfile, const String& infile,
		Record& region, const String& Mask, const bool dropdeg,
		const bool overwrite) {
	Bool rstat = False;
	try {
		*_log << LogOrigin(className(), __func__);

		// Open
		PtrHolder<ImageInterface<Float> > inImage;
		ImageUtilities::openImage(inImage, infile);
		//
		// Convert region from Glish record to ImageRegion.
		// Convert mask to ImageRegion and make SubImage.
		//
		AxesSpecifier axesSpecifier;
		if (dropdeg) {
			axesSpecifier = AxesSpecifier(False);
		}
		SHARED_PTR<SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRW(
			*inImage, region,
			Mask, _log.get(), axesSpecifier
		);

		if (outfile.empty()) {
			_imageFloat = subImage;
			rstat = True;
		}
		else {
			if (!overwrite) {
				NewFile validfile;
				String errmsg;
				if (!validfile.valueOK(outfile, errmsg)) {
					*_log << errmsg << LogIO::EXCEPTION;
				}
			}
			*_log << LogIO::NORMAL << "Creating image '" << outfile
					<< "' of shape " << subImage->shape() << LogIO::POST;
			_imageFloat.reset(new PagedImage<Float> (subImage->shape(),
					subImage->coordinates(), outfile));
			if (_imageFloat.get() == 0) {
				*_log << "Failed to create PagedImage" << LogIO::EXCEPTION;
			}
			ImageUtilities::copyMiscellaneous(*_imageFloat, *inImage);

			// Make output mask if required

			if (subImage->isMasked()) {
				String maskName("");
				ImageMaskAttacher::makeMask(*_imageFloat, maskName, False, True, *_log, True);
			}

			LatticeUtilities::copyDataAndMask(*_log, *_imageFloat, *subImage);
			rstat = True;
		}
	}
	catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
	}
	return rstat;
}

ImageInterface<Float> *
ImageAnalysis::convolve(
	const String& outFile, Array<Float>& kernelArray,
	const String& kernelFileName, const Double in_scale, Record& region,
	String& mask, const Bool overwrite, const Bool, const Bool stretch
) {
	_onlyFloat(__func__);
	*_log << LogOrigin(className(), __func__);

	//Need to deal with the string part
	//    String kernelFileName(kernel.toString());
	if (mask == "[]") {
		mask = "";
	}
	Bool autoScale;
	Double scale(in_scale);

	if (scale > 0) {
		autoScale = False;
	}
	else {
		autoScale = True;
		scale = 1.0;
	}

	// Check output file name
	if (!outFile.empty() && !overwrite) {
		NewFile validfile;
		String errmsg;
		ThrowIf(
			!validfile.valueOK(outFile, errmsg), errmsg
		);
	}

	SHARED_PTR<const SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRO(
		*_imageFloat, region, mask, _log.get(), AxesSpecifier(), stretch
	);

	// Create output image
	IPosition outShape = subImage->shape();
	std::unique_ptr<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*_log << LogIO::NORMAL << "Creating (temp)image of shape "
				<< outShape << LogIO::POST;
		imOut.reset(new TempImage<Float> (outShape, subImage->coordinates()));
	}
	else {
		*_log << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << outShape << LogIO::POST;
		imOut.reset(new PagedImage<Float> (outShape, subImage->coordinates(),
				outFile));
	}
	//ImageInterface<Float>* pImOut = imOut.ptr()->cloneII();

	// Make the convolver
	ImageConvolver<Float> aic;
	Bool copyMisc = True;
	Bool warnOnly = True;
	ImageConvolver<Float>::ScaleTypes scaleType(ImageConvolver<Float>::NONE);
	if (autoScale) {
		scaleType = ImageConvolver<Float>::AUTOSCALE;
	}
	else {
		scaleType = ImageConvolver<Float>::SCALE;
	}
	if (kernelFileName.empty()) {
		ThrowIf(
			kernelArray.nelements() <= 1,
			"Kernel array dimensions are invalid"
		);
		aic.convolve(
			*_log, *imOut, *subImage, kernelArray, scaleType,
			scale, copyMisc
		);
	}
	else {
		ThrowIf(
			! Table::isReadable(kernelFileName),
			"kernel image " + kernelFileName
			+ " is not available "
		);
		PagedImage<Float> kernelImage(kernelFileName);
		aic.convolve(
			*_log, *imOut, *subImage, kernelImage, scaleType,
			scale, copyMisc, warnOnly
		);
	}
	return imOut.release();
}

Record* ImageAnalysis::boundingbox(
	const Record& region
) const {
	*_log << LogOrigin(className(), __func__);
	const CoordinateSystem csys = _imageFloat
		? _imageFloat->coordinates()
		: _imageComplex->coordinates();
	const IPosition shape = _imageFloat
		? _imageFloat->shape()
		: _imageComplex->shape();
	// Find the bounding box of this region
	const ImageRegion* pRegion = ImageRegion::fromRecord(
		0, csys, shape, region
	);
	LatticeRegion latRegion = pRegion->toLatticeRegion(
		csys, shape
	);
	Slicer sl = latRegion.slicer();
	IPosition blc(sl.start());
	IPosition trc(sl.end());
	IPosition inc(sl.stride());
	IPosition length(sl.length());
	std::unique_ptr<Record> outRec(new Record());
	outRec->define("blc", blc.asVector());
	outRec->define("trc", trc.asVector());
	outRec->define("inc", inc.asVector());
	outRec->define("bbShape", (trc - blc + 1).asVector());
	outRec->define("regionShape", length.asVector());
	outRec->define("imageShape", shape.asVector());
	outRec->define("blcf", CoordinateUtil::formatCoordinate(blc, csys)); // 0-rel for use in C++
	outRec->define("trcf", CoordinateUtil::formatCoordinate(trc, csys));
	return outRec.release();
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
	Quantity& intensity, Record& direction,
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
	r = new Record(toworld(vpixel, format));

	Vector<Int> ipixel(vpixel.size());
	convertArray(ipixel, vpixel);

	//    Record *pR = this->pixelvalue(ipixel);
	Bool offImage;
	Quantum<Double> value;
	Bool mask(False);
	pixelValue(offImage, intensity, mask, ipixel);
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

Bool ImageAnalysis::remove(Bool verbose)
{
	_onlyFloat(__func__);
  *_log << LogOrigin(className(), __func__);
  Bool rstat(False);

  // Let's see if it exists.  If it doesn't, then the user has
  // deleted it, or its a readonly expression
  if (!_imageFloat->isPersistent()) {
    *_log << LogIO::WARN
            << "This image tool is not associated with a persistent disk file. It cannot be deleted"
            << LogIO::POST;
    return False;
  }
  Bool strippath(False);
  String fileName = _imageFloat->name(strippath);
  if (fileName.empty()) {
    *_log << LogIO::WARN << "Filename is empty or does not exist."
            << LogIO::POST;
    return False;
  }
  File f(fileName);
  if (!f.exists()) {
    *_log << LogIO::WARN << fileName << " does not exist." << LogIO::POST;
    return False;
  }

  // OK the file exists. Close ourselves first.  This deletes
  // the temporary persistent image as well, if any and destroys
  // the DDs associated with this image (they reference the image
  // and will prevent us from deleting it)
  if (_imageFloat.get() != 0) {
    *_log << (verbose ? LogIO::NORMAL : LogIO::DEBUG1)
            << "Detaching from image" << LogIO::POST;
  }
  _imageFloat.reset();
  //deleteHist();

  // Now try and blow it away.  If it's open, tabledelete won't delete it.
  String message;
  if (Table::canDeleteTable(message, fileName, True)) {
    try {
      Table::deleteTable(fileName, True);
      rstat = True;
      *_log << (verbose ? LogIO::NORMAL : LogIO::DEBUG1)
              << "deleted table " << fileName << LogIO::POST;
    } catch (AipsError x) {
      *_log << LogIO::SEVERE << "Failed to delete file " << fileName
              << " because " << x.getMesg() << LogIO::POST;
    };
  } else {
    *_log << LogIO::SEVERE << "Cannot delete file " << fileName
            << " because " << message << LogIO::POST;
  }

  return rstat;
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

Bool ImageAnalysis::insert(
	const String& infile, Record& Region,
	const Vector<double>& locatePixel, Bool verbose
) {
	_onlyFloat(__func__);
	*_log << LogOrigin(className(), __func__);
	Bool doRef;
	if (locatePixel.size() == 0) {
		doRef = True;
	}
	else {
		doRef = False;
	}
	Int dbg = 0;

	ImageInterface<Float>* pInImage = 0;
	ImageUtilities::openImage(pInImage, infile);
	std::unique_ptr<ImageInterface<Float> > inImage(pInImage);
	// Create region and subImage for image to be inserted
	std::unique_ptr<const ImageRegion> pRegion(
		ImageRegion::fromRecord(
			verbose ? _log.get() : 0, pInImage->coordinates(),
			pInImage->shape(), Region
		)
	);
	SubImage<Float> inSub(*pInImage, *pRegion);
	// Generate output pixel location
	const IPosition inShape = inSub.shape();
	const IPosition outShape = _imageFloat->shape();
	const uInt nDim = _imageFloat->ndim();
	Vector<Double> outPix(nDim);
	const uInt nDim2 = locatePixel.nelements();
	//
	if (doRef) {
		outPix.resize(0);
	}
	else {
		for (uInt i = 0; i < nDim; i++) {
			if (i < nDim2) {
				//	  outPix[i] = locatePixel[i] - 1.0;              // 1 -> 0 rel
				outPix[i] = locatePixel[i];
			}
			else {
				outPix[i] = (outShape(i) - inShape(i)) / 2.0; // Centrally located
			}
		}
	}
	// Insert
	ImageRegrid<Float> ir;
	ir.showDebugInfo(dbg);
	ir.insert(*_imageFloat, outPix, inSub);
	return True;
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

Record ImageAnalysis::maxfit(
	Record& Region, const Bool doPoint,
	const Int width, const Bool absFind, const Bool list
) {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", __func__);
	//SkyComponent sky; // Output
	Vector<Double> absPixel; // Output
	CountedPtr<ImageRegion> pRegionRegion, pMaskRegion;
	AxesSpecifier axesSpec(False); // drop degenerate
	String mask;
	/*SHARED_PTR<const SubImage<Float> >*/ auto subImage = SubImageFactory<Float>::createSubImageRO(
		pRegionRegion, pMaskRegion, *_imageFloat, Region, mask, _log.get(), axesSpec
	);
	*_log << LogOrigin("ImageAnalysis", __func__);

    pRegionRegion = nullptr;
    pMaskRegion = nullptr;
	// Find it
	ImageSourceFinder<Float> sf(*subImage);
	Double cutoff = 0.1;
	auto sky = sf.findSourceInSky(*_log, absPixel, cutoff, absFind, doPoint, width);
	// modify to show dropped degenerate axes values???
	if (list) {
		*_log << LogIO::NORMAL << "Brightness     = " << sky.flux().value()
				<< " " << sky.flux().unit().getName() << LogIO::POST;
		/*CoordinateSystem*/ const auto& cSys = subImage->coordinates();
		*_log << "World Axis Units: " << cSys.worldAxisUnits() << LogIO::POST;
		Vector<Double> wPix;
		if (!cSys.toWorld(wPix, absPixel)) {
			*_log << LogIO::WARN
					<< "Failed to convert to absolute world coordinates "
					<< cSys.errorMessage() << LogIO::POST;
		} else {
			*_log << "Absolute world = " << wPix << LogIO::POST;
		}
		Vector<Double> wRel = wPix.copy();
		cSys.makeWorldRelative(wRel);
		*_log << "Relative world = " << wRel << LogIO::POST;
		*_log << LogIO::NORMAL << "Absolute pixel = " << absPixel << endl;
		Vector<Double> pRel = absPixel.copy();
		cSys.makePixelRelative(pRel);
		*_log << "Relative pixel = " << pRel << LogIO::POST;
	}
	ComponentList mycomp;
	mycomp.add(sky);

	String error;
	Record outrec;
	if (!mycomp.toRecord(error, outrec)) {
		*_log << LogIO::SEVERE
				<< "Cannot convert SkyComponent to output record"
				<< LogIO::POST;
	}
	return outrec;
}

ImageInterface<Float> * ImageAnalysis::moments(
        const Vector<Int>& whichmoments, const Int axis,
        Record& Region, const String& mask, const Vector<String>& method,
        const Vector<Int>& smoothaxes, const Vector<String>& kernels,
        const Vector<Quantity>& kernelwidths, const Vector<Float>& includepix,
        const Vector<Float>& excludepix, const Double peaksnr,
        const Double stddev, const String& velocityType, const String& out,
        const String& smoothout, const Bool overwrite,
        const Bool removeAxis, const Bool stretchMask
) {
    _onlyFloat(__func__);
    *_log << LogOrigin(className(), __func__);
    // check that we can write to smoothout if specified
    if (! smoothout.empty() and !overwrite) {
        NewFile validfile;
        String errmsg;
        ThrowIf(
            ! validfile.valueOK(smoothout, errmsg), errmsg
        );
    }
    // Note that the user may give the strings (method & kernels)
    // as either vectors of strings or one string with separators.
    // Hence the code below that deals with it.   Also in image.g we therefore
    // give the default value as a blank string rather than a null vector.
    String tmpImageName;
    Record r;
    std::unique_ptr<ImageInterface<Float> > pIm;
    try {
        SPCIIF x;
        if (_imageFloat->imageType() != PagedImage<Float>::className()) {
            Path tmpImage = File::newUniqueName (".", "moments.scratch.image");
            tmpImageName = tmpImage.baseName();
            *_log << LogIO::NORMAL << "Calculating moments of non-paged images can be notoriously slow, "
                << "so converting to a CASA temporary paged image named "
                << tmpImageName  << " first which will be written to the current directory" << LogIO::POST;
            x = SubImageFactory<Float>::createImage(
                *_imageFloat, tmpImageName, r, "", False,
                False, True, False
            );
            x = SubImageFactory<Float>::createSubImageRO(
                *x, Region, mask, _log.get(),
                AxesSpecifier(), stretchMask
            );
        }
        else {
            x = SubImageFactory<Float>::createSubImageRO(
                    *_imageFloat, Region, mask, _log.get(),
                    AxesSpecifier(), stretchMask
            );
        }
        // Create ImageMoments object
        ImageMoments<Float> momentMaker(*x, *_log, overwrite, True);
        if ( imageMomentsProgressMonitor != nullptr ){
            momentMaker.setProgressMonitor( imageMomentsProgressMonitor );
        }
        // Set which moments to output
        if (!momentMaker.setMoments(whichmoments + 1)) {
            *_log << momentMaker.errorMessage() << LogIO::EXCEPTION;
        }
        // Set moment axis
        if (axis >= 0) {
            momentMaker.setMomentAxis(axis);
        }
        if (x->imageInfo().hasMultipleBeams()) {
            const CoordinateSystem& csys = x->coordinates();
            if (csys.hasPolarizationCoordinate() && axis == csys.polarizationAxisNumber()) {
                *_log << LogIO::WARN << "This image has multiple beams and you determining "
                        << " moments along the polarization axis. Interpret your results carefully"
                        << LogIO::POST;
            }
        }
        // Set moment methods
        if (method.nelements() > 0 && method(0) != "") {
            String tmp;
            for (uInt i = 0; i < method.nelements(); ++i) {
                tmp += method(i) + " ";
            }
            Vector<Int> intmethods = momentMaker.toMethodTypes(tmp);
            ThrowIf(
                ! momentMaker.setWinFitMethod(intmethods),
                momentMaker.errorMessage()
            );
        }
        // Set smoothing
        if (
            kernels.nelements() >= 1 && kernels(0) != "" && smoothaxes.size() >= 1
            && kernelwidths.nelements() >= 1
        ) {
            String tmp;
            for (uInt i = 0; i < kernels.nelements(); i++) {
                tmp += kernels(i) + " ";
            }
            //
            Vector<Int> intkernels = VectorKernel::toKernelTypes(kernels);
            ThrowIf(
                ! momentMaker.setSmoothMethod(
                    smoothaxes, intkernels, kernelwidths
                ), momentMaker.errorMessage()
            );
        }
        // Set pixel include/exclude range
        ThrowIf(
                ! momentMaker.setInExCludeRange(includepix, excludepix),
                momentMaker.errorMessage()
        );
        // Set SNR cutoff
        ThrowIf(
                ! momentMaker.setSnr(peaksnr, stddev),
                momentMaker.errorMessage()
        );
        // Set velocity type
        if (!velocityType.empty()) {
            MDoppler::Types velType;
            if (!MDoppler::getType(velType, velocityType)) {
                *_log << LogIO::WARN << "Illegal velocity type, using RADIO"
                        << LogIO::POST;
                velType = MDoppler::RADIO;
            }
            momentMaker.setVelocityType(velType);
        }
        // Set output names
        ThrowIf(
            smoothout != "" && ! momentMaker.setSmoothOutName(smoothout),
            momentMaker.errorMessage()
        );
        // If no file name given for one moment image, make TempImage.
        // Else PagedImage results
        Bool doTemp = out.empty() && whichmoments.nelements() == 1;
        // Create moments
        std::vector<std::unique_ptr<MaskedLattice<Float> > > images =
            momentMaker.createMoments(doTemp, out, removeAxis);
        momentMaker.closePlotting();
        // Return handle of first image
        pIm.reset(
            dynamic_cast<ImageInterface<Float>*> (images[0].release())
        );
    }
    catch (const AipsError& x) {
        if (! tmpImageName.empty()) {
            Directory dir(tmpImageName);
            if (dir.exists()) {
                dir.removeRecursive(False);
            }
        }
        RETHROW(x);
    }
    if (! tmpImageName.empty()) {
        Directory dir(tmpImageName);
        if (dir.exists()) {
            dir.removeRecursive(False);
        }
    }
    return pIm.release();
}

void ImageAnalysis::setMomentsProgressMonitor( ImageMomentsProgressMonitor* progressMonitor ){
	_onlyFloat(__func__);
	imageMomentsProgressMonitor = progressMonitor;
}

Record*
ImageAnalysis::pixelvalue(const Vector<Int>& pixel) {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", "pixelvalue");

	Bool offImage;
	Quantum<Double> value;
	Bool mask;
	Vector<Int> pos(pixel);
	ImageAnalysis::pixelValue(offImage, value, mask, pos);
	if (offImage)
		return new Record();

	RecordDesc outRecDesc;
	outRecDesc.addField("mask", TpBool);
	outRecDesc.addField("value", TpRecord);
	outRecDesc.addField("pixel", TpArrayInt);
	Record *outRec = new Record(outRecDesc);
	outRec->define("mask", mask);
	String error;
	QuantumHolder qh(value);
	Record qr;
	if (!qh.toRecord(error, qr)) {
		*_log << error << LogIO::POST;
	} else {
		outRec->defineRecord("value", qr);
	}
	outRec->define("pixel", pos);
	return outRec;
}

void ImageAnalysis::pixelValue(Bool& offImage, Quantum<Double>& value,
		Bool& mask, Vector<Int>& pos) const {
	_onlyFloat(__func__);
	const IPosition imShape = _imageFloat->shape();
	const Vector<Double> refPix = _imageFloat->coordinates().referencePixel();
	const uInt nDim = _imageFloat->ndim();
	//
	if (pos.size() == 1 && pos[0] == -1) { // check for default input parameter
		pos.resize(nDim);
		for (uInt i = 0; i < nDim; i++) {
			pos[i] = Int(refPix(i) + 0.5);
		}
	}
	//
	IPosition iPos = IPosition(pos);
	const uInt nPix = iPos.nelements();
	iPos.resize(nDim, True);

	// Discard extra pixels, add ref pixel for missing ones
	offImage = False;
	for (uInt i = 0; i < nDim; i++) {
		if ((i + 1) > nPix) {
			iPos(i) = Int(refPix(i) + 0.5);
		} else {
			if (iPos(i) < 0 || iPos(i) > (imShape(i) - 1))
				offImage = True;
		}
	}
	if (offImage)
		return;
	//
	IPosition shp(_imageFloat->ndim(), 1);
	Array<Float> pixels = _imageFloat->getSlice(iPos, shp);
	Array<Bool> maskPixels = _imageFloat->getMaskSlice(iPos, shp);
	Unit units = _imageFloat->units();
	if (pos.nelements() != iPos.nelements()) {
		pos.resize(iPos.nelements());
	}
	uInt n = pos.nelements();
	for (uInt i = 0; i < n; i++) {
		pos(i) = iPos(i);
	}
	value = Quantum<Double> (Double(pixels(shp - 1)), units);
	mask = maskPixels(shp - 1);
}

Bool ImageAnalysis::rename(const String& name, const Bool overwrite) {
	_onlyFloat(__func__);
	*_log << LogOrigin(className(), __func__);

	if (!ispersistent()) {
		*_log << LogIO::WARN
				<< "This image tool is not associated with a persistent disk file. It cannot be renamed"
				<< LogIO::POST;
		return False;
	}
	if (name.size() == 0) {
		*_log << LogIO::WARN << "Empty name" << LogIO::POST;
		return False;
	}

	String oldName = _imageFloat->name(False);
	if (oldName.size() == 0) {
		return False;
	}

	// Make sure we don't rename ourselves to ourselves
	if (oldName == name) {
		*_log << LogIO::WARN
				<< "Given name is already the name of the disk file associated with this image tool"
				<< LogIO::POST;
		return False;
	}

	// Let's see if it exists.  If it doesn't, then the user has deleted it
	File file(oldName);
	if (file.isSymLink()) {
		file = File(SymLink(file).followSymLink());
	}
	if (!file.exists()) {
		*_log << LogIO::WARN
				<< "The disk file associated with this image tool appears to have been deleted"
				<< LogIO::POST;
		return False;
	}

	// Make sure target image name does not already exist
	if (!overwrite) {
		File nfile(name);
		if (nfile.isSymLink()) {
			nfile = File(SymLink(nfile).followSymLink());
		}
		if (nfile.exists()) {
			*_log << LogIO::WARN << "There is already a file with the name "
					<< name << LogIO::POST;
			return False;
		}
	}

	// OK we passed the tests.  Close deletes temporary persistent image
	if (_imageFloat.get() != 0) {
		*_log << LogIO::NORMAL << "Detaching from image" << LogIO::POST;
		_imageFloat.reset();

	}

	// Now try and move it
	Bool follow(True);
	if (file.isRegular(follow)) {
		RegularFile(file).move(name, overwrite);
	} else if (file.isDirectory(follow)) {
		Directory(file).move(name, overwrite);
	} else if (file.isSymLink()) {
		SymLink(file).copy(name, overwrite);
	} else {
		*_log << LogIO::POST << "Failed to rename file " << oldName << " to "
				<< name << LogIO::POST;
		return False;
	}

	*_log << LogIO::NORMAL << "Successfully renamed file " << oldName
			<< " to " << name << LogIO::POST;

	// Reopen ourprivate with the new file
	if (!open(name)) {
		*_log << LogIO::WARN << "Failed to open renamed file" << LogIO::POST;
	}

	return True;

}

ImageInterface<Float>* ImageAnalysis::sepconvolve(
	const String& outFile, const Vector<Int>& smoothaxes,
	const Vector<String>& kernels,
	const Vector<Quantity>& kernelwidths, Double scale, Record& pRegion,
	const String& mask, const Bool overwrite, const Bool extendMask
) {
	_onlyFloat(__func__);
	*_log << LogOrigin("ImageAnalysis", __func__);

	Bool autoScale(False);
	if (scale < 0) {
		autoScale = True;
		scale = 1.0;
	}

	// Checks
	if (smoothaxes.nelements() == 0) {
		*_log << "You have not specified any axes to convolve"
				<< LogIO::EXCEPTION;
	}
	ThrowIf(
		smoothaxes.nelements() != kernels.nelements()
		|| smoothaxes.nelements() != kernelwidths.nelements(),
		"You must give the same number of axes, kernels and widths"
	);

	SHARED_PTR<const SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRO(
		*_imageFloat, pRegion, mask, _log.get(), AxesSpecifier(), extendMask
	);

	// Create convolver
	SepImageConvolver<Float> sic(*subImage, *_log, True);

	// Handle inputs.
	Bool useImageShapeExactly = False;
	for (uInt i = 0; i < smoothaxes.nelements(); i++) {
		VectorKernel::KernelTypes type = VectorKernel::toKernelType(kernels(i));
		sic.setKernel(uInt(smoothaxes(i)), type, kernelwidths(i), autoScale,
				useImageShapeExactly, scale);
		*_log << LogIO::NORMAL << "Axis " << smoothaxes(i)
				<< " : kernel shape = " << sic.getKernelShape(uInt(smoothaxes(
				i))) << LogIO::POST;
	}

	// Make output image  - leave it until now incase there are
	// errors in VectorKernel
	PtrHolder<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*_log << LogIO::NORMAL << "Creating (temp)image of shape "
				<< subImage->shape() << LogIO::POST;
		imOut.set(new TempImage<Float> (subImage->shape(),
				subImage->coordinates()));
	}
	else {
		if (!overwrite) {
			NewFile validfile;
			String errmsg;
			if (!validfile.valueOK(outFile, errmsg)) {
				*_log << errmsg << LogIO::EXCEPTION;
			}
		}
		*_log << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << subImage->shape() << LogIO::POST;
		imOut.set(
			new PagedImage<Float> (
				subImage->shape(), subImage->coordinates(), outFile
			)
		);
	}
	ImageInterface<Float>* pImOut = imOut.ptr()->cloneII();
	ImageUtilities::copyMiscellaneous(*pImOut, *_imageFloat);

	sic.convolve(*pImOut);

	return pImOut;
}

Bool ImageAnalysis::setcoordsys(const Record& coordinates) {
	*_log << LogOrigin(className(), __func__);
	ThrowIf(
		coordinates.nfields() == 0,
		"CoordinateSystem is empty"
	);
	Bool ok = False;
	if (_imageFloat) {
		PtrHolder<CoordinateSystem> cSys(
			makeCoordinateSystem(
				coordinates, _imageFloat->shape()
			)
		);
		ok = _imageFloat->setCoordinateInfo(*(cSys.ptr()));
	}
	else if (_imageComplex) {
		PtrHolder<CoordinateSystem> cSys(
			makeCoordinateSystem(
				coordinates, _imageComplex->shape()
			)
		);
		ok = _imageComplex->setCoordinateInfo(*(cSys.ptr()));
	}
	else {
		ThrowCc("No image is defined");
	}
	ThrowIf(!ok, "Failed to set CoordinateSystem");
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

Record ImageAnalysis::summary(
	const String& doppler, const Bool list,
	const Bool pixelorder, const Bool verbose
) {
	if (_imageFloat) {
		return _summary(
			*_imageFloat, doppler, list, pixelorder, verbose
		);
	}
	else {
		return _summary(
			*_imageComplex, doppler, list, pixelorder, verbose
		);
	}
}

Bool ImageAnalysis::tofits(
	const String& fitsfile, const Bool velocity,
	const Bool optical, const Int bitpix, const Double minpix,
	const Double maxpix, Record& pRegion, const String& mask,
	const Bool overwrite, const Bool dropDeg, const Bool,
	const Bool dropStokes, const Bool stokesLast, const Bool wavelength,
	const Bool airWavelength, const String& origin, const Bool stretch,
	const Bool history
) {
	_onlyFloat(__func__);
	*_log << LogOrigin(className(), __func__);
	String error;
	// Check output file
	if (!overwrite && !fitsfile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(fitsfile, errmsg)) {
			*_log << errmsg << LogIO::EXCEPTION;
		}
	}
	// The SubImage that goes to the FITSCOnverter no longer will know
	// the name of the parent mask, so spit it out here
	if (_imageFloat->isMasked()) {
		*_log << LogIO::NORMAL << "Applying mask of name '"
				<< _imageFloat->getDefaultMask() << "'" << LogIO::POST;
	}
	IPosition keepAxes;
	if (!dropDeg) {
		if (dropStokes) {
			CoordinateSystem cSys = _imageFloat->coordinates();
			if (cSys.findCoordinate(Coordinate::STOKES) >= 0
					&& cSys.nCoordinates() > 1) {
				// Stokes axis exists and its not the only one
				Vector<String> cNames = cSys.worldAxisNames();
				keepAxes = IPosition(cNames.size() - 1);
				uInt j = 0;
				for (uInt i = 0; i < cNames.size(); i++) {
					if (cNames(i) != "Stokes") { // not Stokes?
						keepAxes(j) = i; // keep it
						j++;
					}
				}
			}
			//else: nothing to drop
		}
	}
	AxesSpecifier axesSpecifier;
	if (dropDeg) { // just drop all degenerate axes
		axesSpecifier = AxesSpecifier(False);
	}
	else if (!keepAxes.empty()) { // specify which axes to keep
		axesSpecifier = AxesSpecifier(keepAxes);
	}
	SHARED_PTR<const SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRO(
		*_imageFloat, pRegion, mask, _log.get(), axesSpecifier, stretch
	);
    // FIXME remove when the casacore interface has been updated to const
    SPIIF myclone(subImage->cloneII());
    if (
		! ImageFITSConverter::ImageToFITS(
			error, *myclone, fitsfile,
			HostInfo::memoryFree() / 1024,
			velocity, optical,
			bitpix, minpix, maxpix, overwrite,
			False, //  deglast default
			False, //  verbose default
			stokesLast,	wavelength,
			airWavelength, // for airWavelength=True
			origin,
			history
		)
	) {
		*_log << error << LogIO::EXCEPTION;
	}
	return True;
}

Bool ImageAnalysis::toASCII(
	const String& outfile, Record& region,
	const String& mask, const String& sep,
	const String& format, const Double maskvalue,
	const Bool overwrite, const Bool extendMask
) {
	_onlyFloat(__func__);
	// sep is hard-wired as ' ' which is what imagefromascii expects
	*_log << LogOrigin("ImageAnalysis", "toASCII");

	String outFileStr(outfile);
	// Check output file name

	if (outFileStr.empty()) {
		Bool strippath(true);
		outFileStr = _imageFloat->name(strippath);
		outFileStr = outFileStr + ".ascii";
	}

	if (!overwrite) { // quit with warning if file exists and overwrite=false
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFileStr, errmsg)) {
			*_log << errmsg << LogIO::EXCEPTION;
		}
	}

	Path filePath(outFileStr);
	String fileName = filePath.expandedName();

	ofstream outFile(fileName.c_str());
	ThrowIf(! outFile, "Cannot open file " + outfile);

	PixelValueManipulator<Float> pvm(
		_imageFloat, &region, mask
	);
	pvm.setVerbosity(ImageTask<Float>::QUIET);
	pvm.setStretch(extendMask);
	Record ret = pvm.get();

	Array<Float> pixels = ret.asArrayFloat("values");
	Array<Bool> pixmask = ret.asArrayBool("mask");
	IPosition shp = pixels.shape();
	IPosition vshp = pixmask.shape();
	uInt nx = shp(0);
	uInt n = shp.product();
	uInt nlines = 0;
	if (nx > 0) {
		nlines = n / nx;
	}
	IPosition vShape(1, n);
	Vector<Float> vpixels(pixels.reform(vShape));
	if (pixmask.size() > 0) {
		Vector<Bool> vmask(pixmask.reform(vShape));
		for (uInt i = 0; i < n; i++) {
			if (vmask[i] == false)
				vpixels[i] = (float) maskvalue;
		}
	}
	//
	int idx = 0;
	uInt nline = 0;
	char nextentry[128];
	while (nline < nlines) {
		string line;
		for (uInt i = 0; i < nx - 1; i++) {
			sprintf(nextentry, (format + "%s").c_str(), vpixels[idx + i],
					sep.c_str());
			line += nextentry;
		}
		sprintf(nextentry, format.c_str(), vpixels[idx + nx - 1]);
		line += nextentry;
		outFile << line.c_str() << endl;
		//
		idx += nx;
		nline += 1;
	}
	return True;
}

CoordinateSystem* ImageAnalysis::makeCoordinateSystem(
		const Record& coordinates, const IPosition& shape) const {
	*_log << LogOrigin("ImageAnalysis", "makeCoordinateSystem");
	CoordinateSystem* pCS = 0;
	if (coordinates.nfields() == 1) { // must be a record as an element
		Record tmp(coordinates.asRecord(RecordFieldId(0)));
		pCS = CoordinateSystem::restore(tmp, "");

	} else {
		pCS = CoordinateSystem::restore(coordinates, "");
	}

	// Fix up any body longitude ranges...
	String errMsg;
	if (!CoordinateUtil::cylindricalFix(*pCS, errMsg, shape)) {
		*_log << LogOrigin("ImageAnalysis", "makeCoordinateSystem");
		*_log << LogIO::WARN << errMsg << LogIO::POST;
	}
	return pCS;
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

ImageInterface<Float> *
ImageAnalysis::newimage(const String& infile, const String& outfile,
		Record& region, const String& Mask, const bool dropdeg,
		const bool overwrite) {
	ImageInterface<Float>* outImage = 0;
		*_log << LogOrigin(className(), __func__);

		// Open
		PtrHolder<ImageInterface<Float> > inImage;
		ImageUtilities::openImage(inImage, infile);

		AxesSpecifier axesSpecifier;
		if (dropdeg)
			axesSpecifier = AxesSpecifier(False);
		SHARED_PTR<SubImage<Float> > subImage = SubImageFactory<Float>::createSubImageRW(
			*inImage, region, Mask, _log.get(), axesSpecifier
		);

		// Create output image
		if (outfile.empty()) {
			outImage = new SubImage<Float> (*subImage);
		}
		else {
			if (!overwrite) {
				NewFile validfile;
				String errmsg;
				if (!validfile.valueOK(outfile, errmsg)) {
					*_log << errmsg << LogIO::EXCEPTION;
				}
			}
			//
			*_log << LogIO::NORMAL << "Creating image '" << outfile
					<< "' of shape " << subImage->shape() << LogIO::POST;
			outImage = new PagedImage<Float> (subImage->shape(),
					subImage->coordinates(), outfile);
			if (outImage == 0) {
				*_log << "Failed to create PagedImage" << LogIO::EXCEPTION;
			}
			ImageUtilities::copyMiscellaneous(*outImage, *inImage);

			// Make output mask if required
			if (subImage->isMasked()) {
				String maskName("");
				ImageMaskAttacher::makeMask(*outImage, maskName, False, True, *_log, True);
			}

			// Copy data and mask
			LatticeUtilities::copyDataAndMask(*_log, *outImage, *subImage);
		}

	return outImage;
}

// These should really go in a coordsys inside the casa name space

Record ImageAnalysis::toworld(
    const Vector<Double>& pixel, const String& format, Bool doVelocity
) const {
	*_log << LogOrigin(className(), __func__);
	Vector<Double> pixel2 = pixel.copy();
	CoordinateSystem itsCSys = _imageFloat->coordinates();
	{
		Vector<Double> replace = itsCSys.referencePixel();
		const Int nIn = pixel2.nelements();
		const Int nOut = replace.nelements();
		Vector<Double> out(nOut);
		for (Int i = 0; i < nOut; i++) {
			if (i > nIn - 1) {
				out(i) = replace(i);
			} else {
				out(i) = pixel2(i);
			}
		}
		pixel2.assign(out);
	}

	// Convert to world

	Vector<Double> world;
	Record rec;
	if (itsCSys.toWorld(world, pixel2)) {
		rec = _worldVectorToRecord(world, -1, format, True, True, doVelocity);
	}
	else {
		*_log << itsCSys.errorMessage() << LogIO::EXCEPTION;
	}
	return rec;
}

Record ImageAnalysis::_worldVectorToRecord(const Vector<Double>& world, Int c,
    const String& format, Bool isAbsolute, Bool showAsAbsolute, Bool doVelocity
) const
// World vector must be in the native units of cSys
// c = -1 means world must be length cSys.nWorldAxes
// c > 0 means world must be length cSys.coordinate(c).nWorldAxes()
// format from 'n,q,s,m'
{
	*_log << LogOrigin(className(), __func__);
	String ct = upcase(format);
	Vector<String> units;
	CoordinateSystem itsCSys = _imageFloat->coordinates();
	if (c < 0) {
		units = itsCSys.worldAxisUnits();
	} else {
		units = itsCSys.coordinate(c).worldAxisUnits();
	}
	AlwaysAssert(world.nelements()==units.nelements(),AipsError);
	Record rec;
	if (ct.contains(String("N"))) {
		rec.define("numeric", world);
	}
	if (ct.contains(String("Q"))) {
		String error;
		Record recQ1, recQ2;
		for (uInt i = 0; i < world.nelements(); i++) {
			Quantum<Double> worldQ(world(i), Unit(units(i)));
			QuantumHolder h(worldQ);
			if (!h.toRecord(error, recQ1))
				*_log << error << LogIO::EXCEPTION;
			recQ2.defineRecord(i, recQ1);
		}
		rec.defineRecord("quantity", recQ2);
	}
	if (ct.contains(String("S"))) {
		Vector<Int> worldAxes;
		if (c < 0) {
			worldAxes.resize(world.nelements());
			indgen(worldAxes);
		} else {
			worldAxes = itsCSys.worldAxes(c);
		}
		Coordinate::formatType fType = Coordinate::SCIENTIFIC;
		Int prec = 8;
		String u;
		Int coord, axisInCoord;
		Vector<String> fs(world.nelements());
		for (uInt i = 0; i < world.nelements(); i++) {
			itsCSys.findWorldAxis(coord, axisInCoord, i);
			if (itsCSys.type(coord) == Coordinate::DIRECTION || itsCSys.type(
					coord) == Coordinate::STOKES) {
				fType = Coordinate::DEFAULT;
			} else {
				fType = Coordinate::SCIENTIFIC;
			}
			u = "";
			fs(i) = itsCSys.format(u, fType, world(i), worldAxes(i),
					isAbsolute, showAsAbsolute, prec);
			if ((u != String("")) && (u != String(" "))) {
				fs(i) += String(" ") + u;
			}
		}

		rec.define("string", fs);
	}
	if (ct.contains(String("M"))) {
		Record recM = _worldVectorToMeasures(world, c, isAbsolute, doVelocity);
		rec.defineRecord("measure", recM);
	}
	return rec;
}

Record ImageAnalysis::_worldVectorToMeasures(
    const Vector<Double>& world, Int c,
	Bool abs, Bool doVelocity
) const {
	LogIO os(LogOrigin(className(), __func__));

	uInt directionCount, spectralCount, linearCount, stokesCount, tabularCount;
	directionCount = spectralCount = linearCount = stokesCount = tabularCount
			= 0;

	CoordinateSystem itsCSys = _imageFloat->coordinates();

	// Loop over desired Coordinates

	Record rec;
	String error;
	uInt s, e;
	if (c < 0) {
		AlwaysAssert(world.nelements()==itsCSys.nWorldAxes(), AipsError);
		s = 0;
		e = itsCSys.nCoordinates();
	} else {
		AlwaysAssert(world.nelements()==itsCSys.coordinate(c).nWorldAxes(), AipsError);
		s = c;
		e = c + 1;
	}
	for (uInt i = s; i < e; i++) {
		// Find the world axes in the CoordinateSystem that this coordinate belongs to

		const Vector<Int>& worldAxes = itsCSys.worldAxes(i);
		const uInt nWorldAxes = worldAxes.nelements();
		Vector<Double> world2(nWorldAxes);
		const Coordinate& coord = itsCSys.coordinate(i);
		Vector<String> units = coord.worldAxisUnits();
		Bool none = True;

		// Fill in missing world axes if all coordinates specified

		if (c < 0) {
			for (uInt j = 0; j < nWorldAxes; j++) {
				if (worldAxes(j) < 0) {
					world2(j) = coord.referenceValue()(j);
				} else {
					world2(j) = world(worldAxes(j));
					none = False;
				}
			}
		} else {
			world2 = world;
			none = False;
		}
		if (itsCSys.type(i) == Coordinate::LINEAR || itsCSys.type(i)
				== Coordinate::TABULAR) {
			if (!none) {
				Record linRec1, linRec2;
				for (uInt k = 0; k < world2.nelements(); k++) {
					Quantum<Double> value(world2(k), units(k));
					QuantumHolder h(value);
					if (!h.toRecord(error, linRec1))
						os << error << LogIO::EXCEPTION;
					linRec2.defineRecord(k, linRec1);
				}
				//
				if (itsCSys.type(i) == Coordinate::LINEAR) {
					rec.defineRecord("linear", linRec2);
				} else if (itsCSys.type(i) == Coordinate::TABULAR) {
					rec.defineRecord("tabular", linRec2);
				}
			}
			//
			if (itsCSys.type(i) == Coordinate::LINEAR)
				linearCount++;
			if (itsCSys.type(i) == Coordinate::TABULAR)
				tabularCount++;
		} else if (itsCSys.type(i) == Coordinate::DIRECTION) {
			if (!abs) {
				os
						<< "It is not possible to have a relative MDirection measure"
						<< LogIO::EXCEPTION;
			}
			AlwaysAssert(worldAxes.nelements()==2,AipsError);
			//
			if (!none) {

				// Make an MDirection and stick in record

				Quantum<Double> t1(world2(0), units(0));
				Quantum<Double> t2(world2(1), units(1));
				MDirection direction(t1, t2,
						itsCSys.directionCoordinate(i).directionType());
				//
				MeasureHolder h(direction);
				Record dirRec;
				if (!h.toRecord(error, dirRec)) {
					os << error << LogIO::EXCEPTION;
				} else {
					rec.defineRecord("direction", dirRec);
				}
			}
			directionCount++;
		} else if (itsCSys.type(i) == Coordinate::SPECTRAL) {
			if (!abs) {
				os
						<< "It is not possible to have a relative MFrequency measure"
						<< LogIO::EXCEPTION;
			}
			AlwaysAssert(worldAxes.nelements()==1,AipsError);
			if (!none) {

				// Make an MFrequency and stick in record

				Record specRec, specRec1;
				Quantum<Double> t1(world2(0), units(0));
				const SpectralCoordinate& sc0 = itsCSys.spectralCoordinate(i);
				MFrequency frequency(t1, sc0.frequencySystem());
				MeasureHolder h(frequency);
				if (!h.toRecord(error, specRec1)) {
					os << error << LogIO::EXCEPTION;
				}
                else {
					specRec.defineRecord("frequency", specRec1);
				}
				if (doVelocity) {
                    SpectralCoordinate sc(sc0);

				    // Do velocity conversions and stick in MDOppler
				    // Radio

				    sc.setVelocity(String("km/s"), MDoppler::RADIO);
				    Quantum<Double> velocity;
				    if (!sc.frequencyToVelocity(velocity, frequency)) {
					    os << sc.errorMessage() << LogIO::EXCEPTION;
				    }
                    else {
					    MDoppler v(velocity, MDoppler::RADIO);
					    MeasureHolder h(v);
					    if (!h.toRecord(error, specRec1)) {
						    os << error << LogIO::EXCEPTION;
					    }
                        else {
						    specRec.defineRecord("radiovelocity", specRec1);
					    }
				    }

				    // Optical

				    sc.setVelocity(String("km/s"), MDoppler::OPTICAL);
				    if (!sc.frequencyToVelocity(velocity, frequency)) {
					    os << sc.errorMessage() << LogIO::EXCEPTION;
				    }
                    else {
					    MDoppler v(velocity, MDoppler::OPTICAL);
					    MeasureHolder h(v);
					    if (!h.toRecord(error, specRec1)) {
						    os << error << LogIO::EXCEPTION;
					    }
                        else {
						    specRec.defineRecord("opticalvelocity", specRec1);
					    }
				    }

				    // beta (relativistic/true)

				    sc.setVelocity(String("km/s"), MDoppler::BETA);
				    if (!sc.frequencyToVelocity(velocity, frequency)) {
					    os << sc.errorMessage() << LogIO::EXCEPTION;
				    }
                    else {
					    MDoppler v(velocity, MDoppler::BETA);
					    MeasureHolder h(v);
					    if (!h.toRecord(error, specRec1)) {
						    os << error << LogIO::EXCEPTION;
					    }
                        else {
						    specRec.defineRecord("betavelocity", specRec1);
					    }
				    }
                }

				rec.defineRecord("spectral", specRec);
			}
			spectralCount++;
		}
        else if (itsCSys.type(i) == Coordinate::STOKES) {
			if (!abs) {
				os << "It makes no sense to have a relative Stokes measure"
						<< LogIO::EXCEPTION;
			}
			AlwaysAssert(worldAxes.nelements()==1,AipsError);
			//
			if (!none) {
				const StokesCoordinate& coord0 = itsCSys.stokesCoordinate(i);
				StokesCoordinate coord(coord0); // non-const
				String u;
				String s = coord.format(u, Coordinate::DEFAULT, world2(0), 0,
						True, True, -1);
				rec.define("stokes", s);
			}
			stokesCount++;
		}
        else {
			os << "Cannot handle Coordinates of type " << itsCSys.showType(i)
					<< LogIO::EXCEPTION;
		}
	}
	if (directionCount > 1) {
		os << LogIO::WARN
				<< "There was more than one DirectionCoordinate in the "
				<< LogIO::POST;
		os << LogIO::WARN << "CoordinateSystem.  Only the last one is returned"
				<< LogIO::POST;
	}
	if (spectralCount > 1) {
		os << LogIO::WARN
				<< "There was more than one SpectralCoordinate in the "
				<< LogIO::POST;
		os << LogIO::WARN << "CoordinateSystem.  Only the last one is returned"
				<< LogIO::POST;
	}
	if (stokesCount > 1) {
		os << LogIO::WARN << "There was more than one StokesCoordinate in the "
				<< LogIO::POST;
		os << LogIO::WARN << "CoordinateSystem.  Only the last one is returned"
				<< LogIO::POST;
	}
	if (linearCount > 1) {
		os << LogIO::WARN << "There was more than one LinearCoordinate in the "
				<< LogIO::POST;
		os << LogIO::WARN << "CoordinateSystem.  Only the last one is returned"
				<< LogIO::POST;
	}
	if (tabularCount > 1) {
		os << LogIO::WARN
				<< "There was more than one TabularCoordinate in the "
				<< LogIO::POST;
		os << LogIO::WARN << "CoordinateSystem.  Only the last one is returned"
				<< LogIO::POST;
	}
	return rec;
}

void ImageAnalysis::_onlyFloat(const String& method) const {
	ThrowIf(! _imageFloat, "Method " + method + " only supports Float valued images");
}

}
