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
#include <casa/Logging/LogFilter.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/OS/Directory.h>
#include <casa/OS/EnvVar.h>
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
#include <images/Images/ComponentImager.h>
#include <images/Images/Image2DConvolver.h>
#include <images/Images/ImageConcat.h>
#include <images/Images/ImageConvolver.h>
#include <images/Images/ImageDecomposer.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImageExprParse.h>
#include <images/Images/ImageFFT.h>
#include <images/Images/ImageFITSConverter.h>
#include <images/Images/ImageHistograms.h>
#include <images/Images/ImageMoments.h>
#include <images/Images/ImageMetaData.h>
#include <images/Images/ImageOpener.h>
#include <images/Regions/ImageRegion.h>
#include <images/Images/ImageRegrid.h>
#include <images/Images/ImageSourceFinder.h>
#include <images/Images/ImageStatistics.h>
#include <images/Images/ImageSummary.h>
#include <images/Images/ImageTwoPtCorr.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/LELImageCoord.h>
#include <images/Images/PagedImage.h>
#include <images/Images/RebinImage.h>
#include <images/Regions/RegionManager.h>
#include <images/Images/SepImageConvolver.h>
#include <images/Images/SubImage.h>
#include <images/Images/TempImage.h>
#include <images/Regions/WCLELMask.h>
#include <images/Images/ImageAnalysis.h>
#include <lattices/LatticeMath/Fit2D.h>
#include <lattices/LatticeMath/LatticeFit.h>
#include <lattices/Lattices/LatticeAddNoise.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeRegion.h>
#include <lattices/Lattices/LatticeSlice1D.h>
#include <lattices/Lattices/LatticeUtilities.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LCSlicer.h>
#include <lattices/Lattices/MaskedLatticeIterator.h>
#include <lattices/Lattices/PixelCurve1D.h>
#include <lattices/Lattices/RegionType.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <scimath/Fitting/LinearFitSVD.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/VectorKernel.h>
#include <tables/LogTables/NewFile.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>

#include <casa/namespace.h>

#include <memory>

namespace casa { //# name space casa begins

ImageAnalysis::ImageAnalysis() :
	pImage_p(0), pStatistics_p(0), pHistograms_p(0),
			pOldStatsRegionRegion_p(0), pOldStatsMaskRegion_p(0),
			pOldHistRegionRegion_p(0), pOldHistMaskRegion_p(0) {

	// Register the functions to create a FITSImage or MIRIADImage object.
	FITSImage::registerOpenFunction();
	MIRIADImage::registerOpenFunction();

	itsLog = new LogIO();

}

ImageAnalysis::ImageAnalysis(const ImageInterface<Float>* inImage) :
	pImage_p(inImage->cloneII()), itsLog(new LogIO()), pStatistics_p(0),
	pHistograms_p(0), pOldStatsRegionRegion_p(0),
	pOldStatsMaskRegion_p(0), pOldHistRegionRegion_p(0),
	pOldHistMaskRegion_p(0) {}

ImageAnalysis::ImageAnalysis(ImageInterface<Float>* inImage, const Bool cloneInputPointer) :
itsLog(new LogIO()), pStatistics_p(0), pHistograms_p(0),
	pOldStatsRegionRegion_p(0),
	pOldStatsMaskRegion_p(0), pOldHistRegionRegion_p(0),
	pOldHistMaskRegion_p(0) {
	pImage_p = cloneInputPointer ? inImage->cloneII() : inImage;
}

ImageAnalysis::~ImageAnalysis() {
  
        if (pImage_p != 0) {
	  if((pImage_p->isPersistent()) && ((pImage_p->imageType()) == "PagedImage")){
	    ImageOpener::ImageTypes type = ImageOpener::imageType(pImage_p->name());
	    if (type == ImageOpener::AIPSPP) {
	      (static_cast<PagedImage<Float> *>(pImage_p))->table().relinquishAutoLocks(True);
	      (static_cast<PagedImage<Float> *>(pImage_p))->table().unlock();
	    }
	  }
    
    
	  delete pImage_p;
	  pImage_p = 0;
	}
	deleteHistAndStats();
}

Bool ImageAnalysis::toRecord(RecordInterface& rec) {

	if (pImage_p != 0) {
		String err;
		return pImage_p->toRecord(err, rec);

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
		if (itsLog == 0)
			itsLog = new LogIO();
		if (pImage_p != 0) {
			delete pImage_p;
			*itsLog << LogOrigin("ImageAnalysis", "fromRecord");
			*itsLog << LogIO::WARN
					<< "Image is already open, disconnecting first"
					<< LogIO::POST;
			pImage_p = 0;
		}
		pImage_p = new TempImage<Float> ();
		retval = pImage_p->fromRecord(err, rec);

	}

	return retval;

}

Bool ImageAnalysis::open(const String& infile) {
	Bool rstat = True;

	if (itsLog == 0)
		itsLog = new LogIO();

	*itsLog << LogOrigin("ImageAnalysis", "open");

	// Check whether infile exists
	if (infile.empty()) {
		*itsLog << LogIO::WARN << "File string is empty" << LogIO::POST;
		return false;
	}
	File thefile(infile);
	if (!thefile.exists()) {
		*itsLog << LogIO::WARN << "File [" << infile << "] does not exist."
				<< LogIO::POST;
		return false;
	}

	// Generally used if the image is already closed !b
	if (pImage_p != 0) {
		delete pImage_p;
		*itsLog << LogIO::WARN << "Image is already open, closing first"
				<< LogIO::POST;
		pImage_p = 0;
	}

	// Open input image.  We don't handle an Image tool because
	// we would get a bit confused as to who owns the pointer
	ImageUtilities::openImage(pImage_p, infile, *itsLog);

	// Ensure that we reconstruct the statistics and histograms objects
	deleteHistAndStats();
	return rstat;
}

Bool ImageAnalysis::detached() {
	if (pImage_p == 0)
		return True;
	return False;

}

Bool ImageAnalysis::addnoise(const String& type, const Vector<Double>& pars,
		Record& region, const Bool zeroIt) {
	bool rstat(False);
	*itsLog << LogOrigin("ImageAnalysis", "addnoise");

	Record *pRegion = &region;

	// Make SubImage
	String mask;
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(pRegion)),
		mask, itsLog, True
	);

	// Zero subimage if requested
	if (zeroIt)
		subImage.set(0.0);

	// Do it
	Random::Types typeNoise = Random::asType(type);
	LatticeAddNoise lan(typeNoise, pars);
	lan.add(subImage);
	//
	deleteHistAndStats();
	rstat = true;

	return rstat;
}

ImageInterface<Float> *
ImageAnalysis::imagecalc(const String& outfile, const String& expr,
		const Bool overwrite) {

	*itsLog << LogOrigin("ImageAnalysis", "imagecalc");

	Record regions;

	// Check output file name
	if (!outfile.empty() && !overwrite) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outfile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	// Get LatticeExprNode (tree) from parser.  Substitute possible
	// object-id's by a sequence number, while creating a
	// LatticeExprNode for it.  Convert the GlishRecord containing
	// regions to a PtrBlock<const ImageRegion*>.
	if (expr.empty()) {
		*itsLog << "You must specify an expression" << LogIO::EXCEPTION;
	}

	Block<LatticeExprNode> temps;
	String exprName;
	// String newexpr = substituteOID (temps, exprName, expr);
	String newexpr = expr;
	PtrBlock<const ImageRegion*> tempRegs;
	makeRegionBlock(tempRegs, regions, *itsLog);
	LatticeExprNode node = ImageExprParse::command(newexpr, temps, tempRegs);

	// Get the shape of the expression
	const IPosition shapeOut = node.shape();

	// Get the CoordinateSystem of the expression
	const LELAttribute attr = node.getAttribute();
	const LELLattCoordBase* lattCoord = &(attr.coordinates().coordinates());
	if (!lattCoord->hasCoordinates() || lattCoord->classname()
			!= "LELImageCoord") {
		*itsLog << "Images in expression have no coordinates"
				<< LogIO::EXCEPTION;
	}
	const LELImageCoord* imCoord =
			dynamic_cast<const LELImageCoord*> (lattCoord);

	AlwaysAssert (imCoord != 0, AipsError);
	CoordinateSystem cSysOut = imCoord->coordinates();

	// Create LatticeExpr create mask if needed
	LatticeExpr<Float> latEx(node);

	// Construct output image - an ImageExpr or a PagedImage
	if (outfile.empty()) {
		pImage_p = new ImageExpr<Float> (latEx, exprName);
		if (pImage_p == 0) {
			*itsLog << "Failed to create ImageExpr" << LogIO::EXCEPTION;
		}
	} else {
		*itsLog << LogIO::NORMAL << "Creating image `" << outfile
				<< "' of shape " << shapeOut << LogIO::POST;
		pImage_p = new PagedImage<Float> (shapeOut, cSysOut, outfile);
		if (pImage_p == 0) {
			*itsLog << "Failed to create PagedImage" << LogIO::EXCEPTION;
		}

		// Make mask if needed, and copy data and mask
		if (latEx.isMasked()) {
			String maskName("");
			makeMask(*pImage_p, maskName, False, True, *itsLog, True);
		}
		LatticeUtilities::copyDataAndMask(*itsLog, *pImage_p, latEx);
	}

	// Copy miscellaneous stuff over
	pImage_p->setMiscInfo(imCoord->miscInfo());
	pImage_p->setImageInfo(imCoord->imageInfo());
	//
	if (expr.contains("spectralindex")) {
		pImage_p->setUnits("");
	} else if (expr.contains(Regex("pa\\(*"))) {
		pImage_p->setUnits("deg");
		Vector<Int> newstokes(1);
		newstokes = Stokes::Pangle;
		StokesCoordinate scOut(newstokes);
		CoordinateSystem cSys = pImage_p->coordinates();
		Int iStokes = cSys.findCoordinate(Coordinate::STOKES, -1);
		cSys.replaceCoordinate(scOut, iStokes);
		pImage_p->setCoordinateInfo(cSys);
	} else {
		pImage_p->setUnits(imCoord->unit());
	}

	// Logger not yet available
	//    pImage_p->appendLog(imCoord->logger());

	// Delete the ImageRegions (by using an empty GlishRecord).
	makeRegionBlock(tempRegs, Record(), *itsLog);

	return pImage_p;

}

ImageInterface<Float> *
ImageAnalysis::imageconcat(const String& outfile,
		const Vector<String>& inFiles, const Int axis, const Bool relax,
		const Bool tempclose, const Bool overwrite) {
	*itsLog << LogOrigin("ImageAnalysis", "imageconcat");

	// There could be wild cards embedded in our list so expand them out
	Vector<String> expInNames = Directory::shellExpand(inFiles, False);
	if (expInNames.nelements() <= 1) {
		*itsLog << "You must give at least two valid input images"
				<< LogIO::EXCEPTION;
	}
	*itsLog << LogIO::NORMAL << "Number of expanded file names = "
			<< expInNames.nelements() << LogIO::POST;

	// Verify output file
	String outFile(outfile);
	if (!outFile.empty() && !overwrite) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	// Find spectral axis of first image
	PtrHolder<ImageInterface<Float> > im;
	ImageUtilities::openImage(im, expInNames(0), *itsLog);

	//   PagedImage<Float> im(expInNames(0), MaskSpecifier(True));
	CoordinateSystem cSys = im.ptr()->coordinates();
	Int iAxis = axis;
	if (iAxis < 0) {
		iAxis = CoordinateUtil::findSpectralAxis(cSys);
		if (iAxis < 0) {
			*itsLog << "Could not find a spectral axis in first input image"
					<< LogIO::EXCEPTION;
		}
	}

	// Create concatenator.  Use holder so if exceptions, the ImageConcat
	// object gets cleaned up
	uInt axis2 = uInt(iAxis);
	PtrHolder<ImageConcat<Float> > pConcat(new ImageConcat<Float> (axis2,
			tempclose));

	// Set first image
	pConcat.ptr()->setImage(*(im.ptr()), relax);

	// Set the other images.  We may run into the open file limit.
	for (uInt i = 1; i < expInNames.nelements(); i++) {
		Bool doneOpen = False;
		try {
			PtrHolder<ImageInterface<Float> > im2;
			ImageUtilities::openImage(im2, expInNames(i), *itsLog);
			//         PagedImage<Float> im2(expInNames(i), MaskSpecifier(True));
			doneOpen = True;
			pConcat.ptr()->setImage(*(im2.ptr()), relax);
		} catch (AipsError x) {
			if (!doneOpen) {
				*itsLog << "Failed to open file " << expInNames(i) << endl;
				*itsLog
						<< "This may mean you have too many files open simultaneously"
						<< endl;
				*itsLog
						<< "Try using tempclose=T in the imageconcat constructor"
						<< LogIO::EXCEPTION;
			} else {
				*itsLog << x.getMesg() << LogIO::EXCEPTION;
			}
		}
	}
	//
	if (!outFile.empty()) {
		// Construct output image and give it a mask if needed
		pImage_p = new PagedImage<Float> (pConcat.ptr()->shape(),
				pConcat.ptr()->coordinates(), outFile);
		if (!pImage_p) {
			*itsLog << "Failed to create PagedImage" << LogIO::EXCEPTION;
		}
		*itsLog << LogIO::NORMAL << "Creating image '" << outfile
				<< "' of shape " << pImage_p->shape() << LogIO::POST;
		//
		if (pConcat.ptr()->isMasked()) {
			String maskName("");
			makeMask(*pImage_p, maskName, False, True, *itsLog, True);
		}

		// Copy to output
		LatticeUtilities::copyDataAndMask(*itsLog, *pImage_p, *(pConcat.ptr()));
		ImageUtilities::copyMiscellaneous(*pImage_p, *(pConcat.ptr()));
	} else {
		pImage_p = pConcat.ptr()->cloneII();
	}
	return pImage_p;
}

Bool ImageAnalysis::imagefromarray(const String& outfile,
		Array<Float> & pixelsArray, const Record& csys, const Bool linear,
		const Bool overwrite, const Bool log) {

	Bool rstat = False;
	try {
		*itsLog << LogOrigin("ImageAnalysis", "imagefromarray");

		String error;
		if (csys.nfields() != 0) {
			PtrHolder<CoordinateSystem> cSys(makeCoordinateSystem(csys,
					pixelsArray.shape()));
			CoordinateSystem* pCS = cSys.ptr();
			if (!make_image(error, outfile, *pCS, pixelsArray.shape(), *itsLog,
					log, overwrite)) {
				*itsLog << error << LogIO::EXCEPTION;
			}
		} else {
			// Make default CoordinateSystem
			CoordinateSystem cSys = CoordinateUtil::makeCoordinateSystem(
					pixelsArray.shape(), linear);
			centreRefPix(cSys, pixelsArray.shape());
			if (!make_image(error, outfile, cSys, pixelsArray.shape(), *itsLog,
					log, overwrite)) {
				*itsLog << error << LogIO::EXCEPTION;
			}
		}

		// Fill image
		pImage_p->putSlice(pixelsArray, IPosition(pixelsArray.ndim(), 0),
				IPosition(pixelsArray.ndim(), 1));
		rstat = True;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
	}
	return rstat;
}

Bool ImageAnalysis::imagefromascii(const String& outfile, const String& infile,
		const Vector<Int>& shape, const String& sep, const Record& csys,
		const Bool linear, const Bool overwrite) {
	// The glish code ignored sep (assumed to be ' ') so will we to
	Bool rstat = False;

	try {
		*itsLog << LogOrigin("ImageAnalysis", "imagefromascii");

		Path filePath(infile);
		String fileName = filePath.expandedName();

		ifstream inFile(fileName.c_str());
		if (!inFile) {
			*itsLog << LogIO::SEVERE << "Cannot open " << infile << LogIO::POST;
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
					*itsLog << LogIO::SEVERE << "Length of line " << iline
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
		rstat = imagefromarray(outfile, pixels, csys, linear, overwrite);

	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
	}
	return rstat;
}

Bool ImageAnalysis::imagefromfits(const String& outfile,
		const String& fitsfile, const Int whichrep, const Int whichhdu,
		const Bool zeroBlanks, const Bool overwrite) {
	Bool rstat = False;
	try {
		*itsLog << LogOrigin("ImageAnalysis", "imagefromfits");

		// Check output file
		if (!overwrite && !outfile.empty()) {
			NewFile validfile;
			String errmsg;
			if (!validfile.valueOK(outfile, errmsg)) {
				*itsLog << errmsg << LogIO::EXCEPTION;
			}
		}
		//
		if (whichrep < 0) {
			*itsLog	<< "The Coordinate Representation index must be non-negative"
				<< LogIO::EXCEPTION;
		}
		//
		ImageInterface<Float>* pOut = 0;
		String error;
		ImageFITSConverter::FITSToImage(pOut, error, outfile, fitsfile,
						whichrep, whichhdu, HostInfo::memoryFree() / 1024, overwrite,
						zeroBlanks);
		//
		if (pOut == 0) {
			*itsLog << error << LogIO::EXCEPTION;
		}
		pImage_p = pOut;
		rstat = True;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
	}
	return rstat;
}

Bool ImageAnalysis::imagefromimage(const String& outfile, const String& infile,
		Record& region, const String& Mask, const bool dropdeg,
		const bool overwrite) {
	Bool rstat = False;
	try {
		*itsLog << LogOrigin("ImageAnalysis", "imagefromimage");

		// Open
		PtrHolder<ImageInterface<Float> > inImage;
		ImageUtilities::openImage(inImage, infile, *itsLog);
		ImageInterface<Float>* pInImage = inImage.ptr();
		//
		// Convert region from Glish record to ImageRegion.
		// Convert mask to ImageRegion and make SubImage.
		//
		AxesSpecifier axesSpecifier;
		if (dropdeg) {
			axesSpecifier = AxesSpecifier(False);
		}
		SubImage<Float> subImage = SubImage<Float>::createSubImage(
			*pInImage, *(ImageRegion::tweakedRegionRecord(&region)),
			Mask, itsLog, True, axesSpecifier
		);

		// Create output image

		if (outfile.empty()) {
			pImage_p = new SubImage<Float> (subImage);
			rstat = True;
		} else {
			if (!overwrite) {
				NewFile validfile;
				String errmsg;
				if (!validfile.valueOK(outfile, errmsg)) {
					*itsLog << errmsg << LogIO::EXCEPTION;
				}
			}
			//
			*itsLog << LogIO::NORMAL << "Creating image '" << outfile
					<< "' of shape " << subImage.shape() << LogIO::POST;
			pImage_p = new PagedImage<Float> (subImage.shape(),
					subImage.coordinates(), outfile);
			if (pImage_p == 0) {
				*itsLog << "Failed to create PagedImage" << LogIO::EXCEPTION;
			}
			ImageUtilities::copyMiscellaneous(*pImage_p, *pInImage);

			// Make output mask if required

			if (subImage.isMasked()) {
				String maskName("");
				makeMask(*pImage_p, maskName, False, True, *itsLog, True);
			}

			// Copy data and mask

			LatticeUtilities::copyDataAndMask(*itsLog, *pImage_p, subImage);
			rstat = True;
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
	}
	return rstat;
}

Bool ImageAnalysis::imagefromshape(const String& outfile,
		const Vector<Int>& shapeV, const Record& coordinates,
		const Bool linear, const Bool overwrite, const Bool log) {
	Bool rstat = False;
	try {
		*itsLog << LogOrigin("ImageAnalysis", "imagefromshape");

		// Some protection
		if (shapeV.nelements() == 0) {
			*itsLog << "The shape is invalid" << LogIO::EXCEPTION;
		}
		for (uInt i = 0; i < shapeV.nelements(); i++) {
			if (shapeV(i) <= 0) {
				*itsLog << "The shape is invalid" << LogIO::EXCEPTION;
			}
		}

		// Make with supplied CoordinateSystem if record not empty
		String error;
		if (coordinates.nfields() > 0) { //
			PtrHolder<CoordinateSystem> pCS(makeCoordinateSystem(coordinates,
					shapeV));
			if (!make_image(error, outfile, *(pCS.ptr()), shapeV, *itsLog, log,
					overwrite)) {
				*itsLog << error << LogIO::EXCEPTION;
			}
		} else {
			// Make default CoordinateSystem
			CoordinateSystem cSys = CoordinateUtil::makeCoordinateSystem(
					shapeV, linear);
			centreRefPix(cSys, shapeV);
			if (!make_image(error, outfile, cSys, shapeV, *itsLog, log,
					overwrite)) {
				*itsLog << error << LogIO::EXCEPTION;
			}
		}
		pImage_p->set(0.0);
		rstat = True;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
	}
	return rstat;
}

Bool ImageAnalysis::adddegaxes(const String& outfile, PtrHolder<ImageInterface<
		Float> >& outImage, const Bool direction, const Bool spectral,
		const String& stokes, const Bool linear, const Bool tabular,
		const Bool overwrite) {

	*itsLog << LogOrigin("ImageAnalysis", "adddegaxes");

	String outFile(outfile);
	String sStokes(stokes);

	ImageUtilities::addDegenerateAxes(*itsLog, outImage, *pImage_p, outFile,
			direction, spectral, sStokes, linear, tabular, overwrite);

	return True;

}

ImageInterface<Float> *
ImageAnalysis::convolve(
	const String& outFile, Array<Float>& kernelArray,
	const String& kernelFileName, const Double in_scale, Record& region,
	String& mask, const Bool overwrite, const Bool, const Bool stretch
) {

	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

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
		if (!validfile.valueOK(outFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&region)),
		mask, itsLog, False, AxesSpecifier(), stretch
	);

	// Create output image
	IPosition outShape = subImage.shape();
	PtrHolder<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*itsLog << LogIO::NORMAL << "Creating (temp)image of shape "
				<< outShape << LogIO::POST;
		imOut.set(new TempImage<Float> (outShape, subImage.coordinates()));
	}
	else {
		*itsLog << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << outShape << LogIO::POST;
		imOut.set(new PagedImage<Float> (outShape, subImage.coordinates(),
				outFile));
	}
	ImageInterface<Float>* pImOut = imOut.ptr()->cloneII();

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
		if (kernelArray.nelements() > 1) {
			aic.convolve(*itsLog, *pImOut, subImage, kernelArray, scaleType,
					scale, copyMisc);
		}
		else {
			*itsLog << "Kernel array dimensions are invalid"
					<< LogIO::EXCEPTION;
		}
	}
	else {
		if (!Table::isReadable(kernelFileName)) {
			*itsLog << LogIO::SEVERE << "kernel image " << kernelFileName
					<< " is not available " << LogIO::POST;
			return 0;
		}
		PagedImage<Float> kernelImage(kernelFileName);
		aic.convolve(*itsLog, *pImOut, subImage, kernelImage, scaleType, scale,
				copyMisc, warnOnly);
	}
	return pImOut;
}

Record*
ImageAnalysis::boundingbox(const Record& Region) {
	*itsLog << LogOrigin("ImageAnalysis", "boundingbox");
	// Find the bounding box of this region
	Record tmpR(Region);
	const ImageRegion* pRegion = ImageRegion::fromRecord(
		0, pImage_p->coordinates(), pImage_p->shape(),
		*ImageRegion::tweakedRegionRecord(&tmpR)
	);
	LatticeRegion latRegion = pRegion->toLatticeRegion(pImage_p->coordinates(),
			pImage_p->shape());
	//
	Slicer sl = latRegion.slicer();
	IPosition blc(sl.start()); // 1-rel for Glish
	IPosition trc(sl.end());
	IPosition inc(sl.stride());
	IPosition length(sl.length());
	RecordDesc outRecDesc;
	outRecDesc.addField("blc", TpArrayInt);
	outRecDesc.addField("trc", TpArrayInt);
	outRecDesc.addField("inc", TpArrayInt);
	outRecDesc.addField("bbShape", TpArrayInt);
	outRecDesc.addField("regionShape", TpArrayInt);
	outRecDesc.addField("imageShape", TpArrayInt);
	outRecDesc.addField("blcf", TpString);
	outRecDesc.addField("trcf", TpString);
	Record *outRec = new Record(outRecDesc);
	outRec->define("blc", blc.asVector());
	outRec->define("trc", trc.asVector());
	outRec->define("inc", inc.asVector());
	outRec->define("bbShape", (trc - blc + 1).asVector());
	outRec->define("regionShape", length.asVector());
	outRec->define("imageShape", pImage_p->shape().asVector());
	//
	CoordinateSystem cSys(pImage_p->coordinates());
	outRec->define("blcf", CoordinateUtil::formatCoordinate(blc, cSys)); // 0-rel for use in C++
	outRec->define("trcf", CoordinateUtil::formatCoordinate(trc, cSys));
	return outRec;
}

String ImageAnalysis::brightnessunit() {
	String rstat;
	*itsLog << LogOrigin("ImageAnalysis", "brightnessunit");
	rstat = pImage_p->units().getName();
	return rstat;
}

Bool ImageAnalysis::calc(const String& expr) {

	*itsLog << LogOrigin("ImageAnalysis", "calc");
	Record regions;

	// Get LatticeExprNode (tree) from parser
	// Convert the GlishRecord containing regions to a
	// PtrBlock<const ImageRegion*>.
	if (expr.empty()) {
		*itsLog << "You must specify an expression" << LogIO::EXCEPTION;
		return False;
	}

	Block<LatticeExprNode> temps;
	//  String exprName;
	//  String newexpr = substituteOID (temps, exprName, expr);
	String newexpr = expr;
	PtrBlock<const ImageRegion*> tempRegs;
	makeRegionBlock(tempRegs, regions, *itsLog);
	LatticeExprNode node = ImageExprParse::command(newexpr, temps, tempRegs);

	// Delete the ImageRegions (by using an empty GlishRecord)
	makeRegionBlock(tempRegs, Record(), *itsLog);

	// Get the shape of the expression and check it matches that
	// of the output image
	if (!node.isScalar()) {
		const IPosition shapeOut = node.shape();
		if (!pImage_p->shape().isEqual(shapeOut)) {
			*itsLog << LogIO::SEVERE
					<< "The shape of the expression does not conform " << endl;
			*itsLog << "with the shape of the output image" << LogIO::POST;
			*itsLog << "Expression shape = " << shapeOut << endl;
			*itsLog << "Image shape      = " << pImage_p->shape()
					<< LogIO::EXCEPTION;
		}
	}

	// Get the CoordinateSystem of the expression and check it matches
	// that of the output image
	if (!node.isScalar()) {
		const LELAttribute attr = node.getAttribute();
		const LELLattCoordBase* lattCoord = &(attr.coordinates().coordinates());
		if (!lattCoord->hasCoordinates() || lattCoord->classname()
				!= "LELImageCoord") {
			// We assume here that the output coordinates are ok
			*itsLog << LogIO::WARN
					<< "Images in expression have no coordinates"
					<< LogIO::POST;
		} else {
			const LELImageCoord* imCoord =
					dynamic_cast<const LELImageCoord*> (lattCoord);
			AlwaysAssert (imCoord != 0, AipsError);
			const CoordinateSystem& cSysOut = imCoord->coordinates();
			if (!pImage_p->coordinates().near(cSysOut)) {
				// Since the output image has coordinates, and the shapes
				// have conformed, just issue a warning
				*itsLog << LogIO::WARN
						<< "The coordinates of the expression do not conform "
						<< endl;
				*itsLog << "with the coordinates of the output image" << endl;
				*itsLog << "Proceeding with output image coordinates"
						<< LogIO::POST;
			}
		}
	}
	// Make a LatticeExpr and see if it is masked
	Bool exprIsMasked = node.isMasked();
	if (exprIsMasked) {
		if (!pImage_p->isMasked()) {
			// The image does not have a default mask set.  So try and make it one.
			String maskName("");
			makeMask(*pImage_p, maskName, True, True, *itsLog, True);
		}
	}
	// Evaluate the expression and fill the output image and mask
	if (node.isScalar()) {
		LatticeExprNode node2 = toFloat(node);
		// If the scalar value is masked, there is nothing
		// to do.
		if (!exprIsMasked) {
			Float value = node2.getFloat();
			if (pImage_p->isMasked()) {
				// We implement with a LEL expression of the form
				// iif(mask(image)", value, image)
				LatticeExprNode node3 = iif(mask(*pImage_p), node2, *pImage_p);
				pImage_p->copyData(LatticeExpr<Float> (node3));
			} else {
				// Just set all values to the scalar. There is no mask to
				// worry about.
				pImage_p->set(value);
			}
		}
	} else {
		if (pImage_p->isMasked()) {
			// We implement with a LEL expression of the form
			// iif(mask(image)", expr, image)
			LatticeExprNode node3 = iif(mask(*pImage_p), node, *pImage_p);
			pImage_p->copyData(LatticeExpr<Float> (node3));
		} else {
			// Just copy the pixels from the expression to the output.
			// There is no mask to worry about.
			pImage_p->copyData(LatticeExpr<Float> (node));
		}
	}
	// Ensure that we reconstruct the statistics and histograms objects
	// now that the data have changed
	deleteHistAndStats();
	//
	return True;
}

Bool ImageAnalysis::calcmask(const String& mask, Record& regions,
		const String& maskName, const Bool makeDefault) {

	*itsLog << LogOrigin("ImageAnalysis", "calcmask");

	String expr = mask;

	// Get LatticeExprNode (tree) from parser
	// Convert the GlishRecord containing regions to a
	// PtrBlock<const ImageRegion*>.
	if (expr.empty()) {
		*itsLog << "You must specify an expression" << LogIO::EXCEPTION;
		return False;
	}
	Block<LatticeExprNode> temps;
	//String exprName;
	//can't use $this in expression; so be it
	//String newexpr = substituteOID (temps, exprName, expr);
	String newexpr = expr;
	PtrBlock<const ImageRegion*> tempRegs;
	makeRegionBlock(tempRegs, regions, *itsLog);
	LatticeExprNode node = ImageExprParse::command(newexpr, temps, tempRegs);

	// Delete the ImageRegions (by using an empty GlishRecord).
	makeRegionBlock(tempRegs, Record(), *itsLog);

	// Make sure the expression is Boolean
	DataType type = node.dataType();
	if (type != TpBool) {
		*itsLog << "The expression type must be Boolean" << LogIO::EXCEPTION;
	}

	// Get the shape of the expression and check it matches that
	// of the output image.  We don't check that the Coordinates
	// match as that would be an un-necessary restriction.
	if (!node.isScalar()) {
		const IPosition shapeOut = node.shape();
		if (!pImage_p->shape().isEqual(shapeOut)) {
			*itsLog << LogIO::SEVERE
					<< "The shape of the expression does not conform " << endl;
			*itsLog << "with the shape of the output image" << LogIO::POST;
			*itsLog << "Expression shape = " << shapeOut << endl;
			*itsLog << "Image shape      = " << pImage_p->shape()
					<< LogIO::EXCEPTION;
		}
	}

	// Make mask and get hold of its name.   Currently new mask is forced to
	// be default because of other problems.  Cannot use the usual makeMask
	// function because I cant attach/make it default until the expression
	// has been evaluated
	if (pImage_p->canDefineRegion()) {
		// Generate mask name if not given
		String maskName2 = maskName;
		if (maskName.empty())
			maskName2 = pImage_p->makeUniqueRegionName(String("mask"), 0);
		// Make the mask if it does not exist
		if (!pImage_p->hasRegion(maskName2, RegionHandler::Masks)) {
			pImage_p->makeMask(maskName2, True, False);
			*itsLog << LogIO::NORMAL << "Created mask `" << maskName2 << "'"
					<< LogIO::POST;
			//
			ImageRegion iR = pImage_p->getRegion(maskName2,
					RegionHandler::Masks);
			LCRegion& mask = iR.asMask();
			if (node.isScalar()) {
				Bool value = node.getBool();
				mask.set(value);
			} else {
				mask.copyData(LatticeExpr<Bool> (node));
			}
		} else {
			// Access pre-existing mask.
			ImageRegion iR = pImage_p->getRegion(maskName2,
					RegionHandler::Masks);
			LCRegion& mask2 = iR.asMask();
			if (node.isScalar()) {
				Bool value = node.getBool();
				mask2.set(value);
			} else {
				mask2.copyData(LatticeExpr<Bool> (node));
			}
		}
		if (makeDefault)
			pImage_p->setDefaultMask(maskName2);
	} else {
		*itsLog << "Cannot make requested mask for this type of image" << endl;
		*itsLog << "It is probably an ImageExpr or SubImage"
				<< LogIO::EXCEPTION;
	}
	//
	return True;

}

ImageInterface<Float>* ImageAnalysis::continuumsub(
	const String& outline, const String& outcont,
	Record& region, const Vector<Int>& channels, const String& pol,
	const Int in_fitorder, const Bool overwrite
) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	// Form virtual image according to region argument and find
	// coordinate system
	String leoutfile("");
	String lemask("");
	Bool ledropdeg(False);
	Bool leoverwrite(False);
	Bool lelist(False);
	std::auto_ptr<ImageInterface<Float> > subim(
		subimage(leoutfile, region, lemask,
		ledropdeg, leoverwrite, lelist)
	);
	if (!subim.get()) {
		*itsLog << LogIO::SEVERE
				<< "Could not form subimage in specified region."
				<< LogIO::POST;
		return 0;
	}

	const CoordinateSystem& cSys = subim->coordinates();

	// Spectral axis
	Vector<Int> spectralPixelAxis;
	Int foundSpectral = cSys.findCoordinate(Coordinate::SPECTRAL);
	if (!foundSpectral) {
		*itsLog << LogIO::SEVERE << "No Spectral axis in this image"
				<< LogIO::POST;
		return 0;
	}
	spectralPixelAxis = cSys.pixelAxes(foundSpectral);
	// Check non-degeneracy of spectral axis
	IPosition imshape = subim->shape();
	if (imshape[spectralPixelAxis[0]] == 1) {
		*itsLog << LogIO::WARN
				<< "There is only one channel in the selected region."
				<< LogIO::POST;
		return 0;
	}

	// If requested, select additionally on Stokes axis
	Record fitregion;
	if (pol.size() != 0) {

		Record myRegion;
		const CoordinateSystem& cSys = pImage_p->coordinates();
		Int stokesIndex = cSys.findCoordinate(Coordinate::STOKES);
		if ((stokesIndex < 0)) {
			*itsLog << LogIO::SEVERE << "No Stokes axis in this image"
					<< LogIO::POST;
			return 0;
		}
		StokesCoordinate stokesCoord = cSys.stokesCoordinate(stokesIndex);
		Int whichPolPix;
		if (!stokesCoord.toPixel(whichPolPix, Stokes::type(pol))) {
			*itsLog << LogIO::SEVERE << "Selected polarization " << pol
					<< " not in image" << LogIO::POST;
			return 0;
		}

		Vector<Int> stokesPixelAxis = cSys.pixelAxes(stokesIndex);

		// Now create box region to select only on the Stokes
		// axis. Pretty hard work.
		int nDim = (subim->shape()).size();
		Vector<Double> blc(nDim);
		blc.set(0);
		Vector<Double> trc(nDim);
		for (int k = 0; k < nDim; ++k) {
			trc[k] = subim->shape()(k) - 1;
		}
		blc[stokesPixelAxis[0]] = (Double) whichPolPix;
		trc[stokesPixelAxis[0]] = (Double) whichPolPix;

		LCBox leregion(blc, trc, subim->shape());
		fitregion = Record(leregion.toRecord(""));
		if (fitregion.nfields() < 1) {
			*itsLog << LogIO::SEVERE << "Failed to form valid Stokes fitregion"
					<< LogIO::POST;
			return 0;
		}
	}

	// Create OTF mask from given channels and axis
	String mask("");
	if (in_fitorder < 0) {
		*itsLog << LogIO::SEVERE << "Fit order must be non-negative"
				<< LogIO::POST;
		return 0;
	}
	Int fitorder = in_fitorder;
	Int ncchan = channels.nelements();
	if (ncchan != 0) {
		// Check order
		if (ncchan == 1) {
			*itsLog
					<< "Only one continuum channel specified; forcing fitorder=0."
					<< LogIO::POST;
			fitorder = 0;
		}
		// Make mask
		ostringstream oss;
		oss << "indexin(";
		//It is really just a scalar
		//for (uInt j = 0; j < spectralPixelAxis.size(); j++) {
		// oss <<  spectralPixelAxis[0]+1 << " ";
		oss << spectralPixelAxis[0] << " ";
		//}
		//DAMN ...its 1 based....arrrgh...
		//SDJ Not anymore!!! (June 10, 2008)
		oss << ", [";
		for (uInt j = 0; j < (channels.size() - 1); j++) {
			//oss << channels[j]+1 << ", ";
			oss << channels[j] << ", ";
		}
		//oss << channels[channels.size()-1]+1 << " ";
		oss << channels[channels.size() - 1] << " ";
		oss << "])";
		mask = oss.str();
	}
	*itsLog << "image.continuumsub mask is: *" << mask << "*" << LogIO::POST;
	//if(mask == "[]")
	//mask = "";
	// Do fit and subtraction
	std::string sigmafile = "";
	//Now lets keep this pImage and put the subImage as the pImage
	ImageAnalysis ia(subim.get());
	ImageInterface<Float>* rstat = ia.fitpolynomial(
		outline, outcont,
		sigmafile, spectralPixelAxis[0], fitorder, fitregion, mask,
		overwrite
	);
	//revert this object back to its original image
	if (!rstat) {
		*itsLog << LogIO::SEVERE << "fitpolynomial failed" << LogIO::POST;
	}

	// Return Image tool to fitted image

	return rstat;
}

casa::Quantity ImageAnalysis::convertflux(
		Bool& fakeBeam, const Quantity& value,
		const Quantity& majorAxis, const Quantity& minorAxis,
		const String& type, const Bool toPeak, Bool suppressNoBeamWarnings
	) {
	*itsLog << LogOrigin("ImageAnalysis", "convertflux");
	Quantum<Double> valueOut;
	fakeBeam = False;
	const Unit& brightnessUnit = pImage_p->units();
	const ImageInfo& info = pImage_p->imageInfo();
	const CoordinateSystem& cSys = pImage_p->coordinates();
	//
	Vector<Quantity> beam = info.restoringBeam();
	//
	if (majorAxis.getValue() > 0.0 && minorAxis.getValue() > 0.0) {
		Unit rad("rad");
		if (!(majorAxis.getFullUnit() == rad) || !(minorAxis.getFullUnit()
				== rad)) {
			*itsLog << "The major and minor axes must be angular"
					<< LogIO::EXCEPTION;
		}
	} else {
		*itsLog << "The major and minor axes must both be positive"
				<< LogIO::EXCEPTION;
	}
	//
	Int afterCoord = -1;
	Int iC = cSys.findCoordinate(Coordinate::DIRECTION, afterCoord);
	if (iC < 0) {
		*itsLog << "No DirectionCoordinate - cannot convert flux density"
				<< LogIO::EXCEPTION;
	}
	const DirectionCoordinate& dirCoord = cSys.directionCoordinate(iC);
	ComponentType::Shape shape = ComponentType::shape(type);

	if(brightnessUnit.getName().contains("/beam") && beam.size() != 3) {
		beam = ImageUtilities::makeFakeBeam(*itsLog, cSys, suppressNoBeamWarnings);
		fakeBeam = True;
	}

	SkyCompRep skyComp;
	valueOut = (toPeak)
		? skyComp.integralToPeakFlux(
			dirCoord, shape, value,
			brightnessUnit, majorAxis, minorAxis, beam
		)
		: skyComp.peakToIntegralFlux(
			dirCoord, shape, value,
			majorAxis, minorAxis, beam
		);

	return valueOut;
}

ImageInterface<Float>* ImageAnalysis::convolve2d(
	const String& outFile, const Vector<Int>& axes,
	const String& type, const Quantity& majorKernel,
	const Quantity& minorKernel,
	const Quantity& paKernel, Double scale,
	Record& Region, const String& mask, const Bool overwrite,
	const Bool stretch
) {
	*itsLog << LogOrigin("ImageAnalysis", "convolve2d");
    if (majorKernel < minorKernel) {
    	*itsLog << "Major axis "
    		<< " is less than minor axis " << LogIO::EXCEPTION;
    }
	String kernel(type);
	Bool autoScale;
	if (scale > 0) {
		autoScale = False;
	} else {
		autoScale = True;
		scale = 1.0;
	}
	// Check output file
	if (!overwrite && !outFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p, *(ImageRegion::tweakedRegionRecord(&Region)),
		mask, itsLog, False, AxesSpecifier(), stretch
	);

	// Convert inputs
	Vector<Int> axes2(axes);
	if (axes2.nelements() != 2) {
		*itsLog << "You must give two axes to convolve" << LogIO::EXCEPTION;
	}
	IPosition axes3(axes2);
	VectorKernel::KernelTypes kernelType = VectorKernel::toKernelType(kernel);
	Vector<Quantum<Double> > parameters(3);
	parameters(0) = majorKernel;
	parameters(1) = minorKernel;
	parameters(2) = paKernel;

	// Create output image and mask
	IPosition outShape = subImage.shape();
	PtrHolder<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*itsLog << LogIO::NORMAL << "Creating (temp)image of shape "
				<< outShape << LogIO::POST;
		imOut.set(new TempImage<Float> (outShape, subImage.coordinates()));
	} else {
		*itsLog << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << outShape << LogIO::POST;
		imOut.set(
			new PagedImage<Float> (
				outShape, subImage.coordinates(), outFile
			)
		);
	}

	ImageInterface<Float>* pImOut = imOut.ptr()->cloneII();

	// Make the convolver
        try {
          Image2DConvolver<Float> ic;
          ic.convolve(
              *itsLog, *pImOut, subImage, kernelType, axes3,
              parameters, autoScale, scale, True
          );
        }
        catch ( AipsError &e ) {
          pImOut->unlock() ;
          delete pImOut ;
          throw e ;
        }

	// Return image
	return pImOut;
}

CoordinateSystem ImageAnalysis::coordsys(const Vector<Int>& pixelAxes) {

	*itsLog << LogOrigin("ImageAnalysis", "coordsys");

	// Recover CoordinateSytem into a Record
	Record rec;
	CoordinateSystem cSys = pImage_p->coordinates();
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
					*itsLog << "Pixel axis " << axes(i) + 1
							<< " has been removed" << LogIO::EXCEPTION;
				}
			} else {
				*itsLog << "Specified pixel axis " << axes(i) + 1
						<< " is not a valid pixelaxis" << LogIO::EXCEPTION;
			}
		}
		//
		// Find mapping.  Says where world axis i in cSys is in cSys2
		//
		Vector<Int> worldAxisMap, worldAxisTranspose;
		Vector<Bool> refChange;
		if (!cSys2.worldMap(worldAxisMap, worldAxisTranspose, refChange, cSys)) {
			*itsLog << "Error finding world map because "
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
					*itsLog << "World axis corresponding to pixel axis "
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
	} else {
		cSys2 = cSys;
	}

	// Return coordsys object
	return cSys2;
}

CoordinateSystem ImageAnalysis::csys(const Vector<Int>& axes) {

	//No clue why this was done...just keeping it
	return coordsys(axes);

}

Record *
ImageAnalysis::coordmeasures(Quantity& intensity, Record& direction,
		Record& frequency, Record& velocity, const Vector<Double>& pixel) {
	Record *r = 0;

	*itsLog << LogOrigin("ImageAnalysis", "coordmeasures");

	CoordinateSystem cSys = pImage_p->coordinates();

	Vector<Double> vpixel(pixel);
	if (pixel.size() == 0) {
		vpixel = cSys.referencePixel();
	}

	String format("m");
	r = new Record(toWorldRecord(vpixel, format));

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
Matrix<Float> ImageAnalysis::decompose(Record& Region, const String& mask,
		const Bool simple, const Double Threshold, const Int nContour,
		const Int minRange, const Int nAxis, const Bool fit,
		const Double maxrms, const Int maxRetry, const Int maxIter,
		const Double convCriteria) {

  Matrix<Int> blcs;
  Matrix<Int> trcs;
  return decompose(blcs, trcs, Region, mask, simple, Threshold, nContour, minRange,
		   nAxis, fit, maxrms, maxRetry, maxIter, convCriteria);
}
Matrix<Float> ImageAnalysis::decompose(
	Matrix<Int>& blcs, Matrix<Int>& trcs, Record& Region, const String& mask,
	const Bool simple, const Double Threshold, const Int nContour,
	const Int minRange, const Int nAxis, const Bool fit,
	const Double maxrms, const Int maxRetry, const Int maxIter,
	const Double convCriteria, const Bool stretch
) {

	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	if (Threshold < 0) {
		*itsLog << "Threshold cannot be negative " << LogIO::EXCEPTION;
	}

	Float threshold(Threshold);

	AxesSpecifier axesSpec(False);
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p, *(ImageRegion::tweakedRegionRecord(&Region)), mask,
		itsLog, False, axesSpec, stretch
	);
	// Make finder
	ImageDecomposer<Float> decomposer(subImage);

	/*
	 * We no longer allow negative thresholds.
	// Set auto-threshold at 5-sigma
	if (threshold <= 0.0) {
		LatticeStatistics<Float> stats(subImage);
		Array<Double> out;
		//Bool ok = stats.getSigma (out, True); //what did this do?
		// Bool ok = stats.getStatistic (out,LatticeStatsBase::SIGMA);
		threshold = 5.0 * out(IPosition(subImage.ndim(), 0));
	}
	*/
	// Do it
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

Record ImageAnalysis::deconvolvecomponentlist(Record& compList) {

	Record retval;
	*itsLog << LogOrigin("ImageAnalysis", "decovolvecomponentlist");

	String error;
	ComponentList cl;
	if (!cl.fromRecord(error, compList)) {
		*itsLog << "Can not  convert input parameter to ComponentList "
				<< error << LogIO::EXCEPTION;
		return retval;
	}

	uInt n = cl.nelements();
	Vector<SkyComponent> list(n);
	for (uInt i = 0; i < n; i++) {
		list(i) = cl.component(i);
	}

	// Do we have a beam ?
	ImageInfo ii = pImage_p->imageInfo();
	Vector<Quantum<Double> > beam = ii.restoringBeam();
	if (beam.nelements() != 3) {
		*itsLog << "This image does not have a restoring beam"
				<< LogIO::EXCEPTION;
	}
	//
	const CoordinateSystem cSys = pImage_p->coordinates();
	Int dirCoordinate = cSys.findCoordinate(Coordinate::DIRECTION);
	if (dirCoordinate == -1) {
		*itsLog
				<< "This image does not contain a DirectionCoordinate - cannot deconvolve"
				<< LogIO::EXCEPTION;
	}
	DirectionCoordinate dirCoord = cSys.directionCoordinate(dirCoordinate);

	// Loop over components and deconvolve
	n = list.nelements();
	// Vector<SkyComponent> listOut(n);
	ComponentList outCL;
	for (uInt i = 0; i < n; ++i) {
		// listOut(i) = deconvolveSkyComponent(*itsLog, list(i), beam, dirCoord);
		outCL.add(ImageUtilities::deconvolveSkyComponent(*itsLog, list(i), beam, dirCoord));
	}
	if (outCL.nelements() > 0)
		if (!outCL.toRecord(error, retval)) {
			*itsLog << "Cannot  convert deconvolved ComponentList to Record"
					<< error << LogIO::EXCEPTION;

		}
	//
	return retval;
}

Bool ImageAnalysis::remove(Bool verbose)
{
  *itsLog << LogOrigin("ImageAnalysis", "remove");
  Bool rstat(False);

  // Let's see if it exists.  If it doesn't, then the user has
  // deleted it, or its a readonly expression
  if (!pImage_p->isPersistent()) {
    *itsLog << LogIO::WARN
            << "This image tool is not associated with a persistent disk file. It cannot be deleted"
            << LogIO::POST;
    return False;
  }
  Bool strippath(False);
  String fileName = pImage_p->name(strippath);
  if (fileName.empty()) {
    *itsLog << LogIO::WARN << "Filename is empty or does not exist."
            << LogIO::POST;
    return False;
  }
  File f(fileName);
  if (!f.exists()) {
    *itsLog << LogIO::WARN << fileName << " does not exist." << LogIO::POST;
    return False;
  }

  // OK the file exists. Close ourselves first.  This deletes
  // the temporary persistent image as well, if any and destroys
  // the DDs associated with this image (they reference the image
  // and will prevent us from deleting it)
  if (pImage_p != 0) {
    *itsLog << (verbose ? LogIO::NORMAL : LogIO::DEBUG1)
            << "Detaching from image" << LogIO::POST;
    delete pImage_p;
  }
  pImage_p = 0;
  deleteHistAndStats();

  // Now try and blow it away.  If it's open, tabledelete won't delete it.
  String message;
  if (Table::canDeleteTable(message, fileName, True)) {
    try {
      Table::deleteTable(fileName, True);
      rstat = True;
      *itsLog << (verbose ? LogIO::NORMAL : LogIO::DEBUG1)
              << "deleted table " << fileName << LogIO::POST;
    } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Failed to delete file " << fileName
              << " because " << x.getMesg() << LogIO::POST;
    };
  } else {
    *itsLog << LogIO::SEVERE << "Cannot delete file " << fileName
            << " because " << message << LogIO::POST;
  }

  return rstat;
}

Bool ImageAnalysis::fft(
	const String& realOut, const String& imagOut,
	const String& ampOut, const String& phaseOut, const Vector<Int>& axes,
	Record& Region, const String& mask, const Bool stretch
) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	// Validate outfiles
	if (realOut.empty() && imagOut.empty() && ampOut.empty()
			&& phaseOut.empty()) {
		*itsLog << LogIO::WARN << "You did not request any output images"
				<< LogIO::POST;
		return False;
	}
	//
	String errmsg;
	if (!realOut.empty()) {
		NewFile validFileReal;
		if (!validFileReal.valueOK(realOut, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}
	//
	if (!imagOut.empty()) {
		NewFile validFileImag;
		if (!validFileImag.valueOK(imagOut, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}
	//
	if (!ampOut.empty()) {
		NewFile validFileAmp;
		if (!validFileAmp.valueOK(ampOut, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}
	//
	if (!phaseOut.empty()) {
		NewFile validFilePhase;
		if (!validFilePhase.valueOK(phaseOut, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p, *(ImageRegion::tweakedRegionRecord(&Region)),
		mask, itsLog, False, AxesSpecifier(), stretch
	);

	// Do the FFT
	ImageFFT fft;
	if (axes.size() == 0) {
		*itsLog << LogIO::NORMAL << "FFT the sky" << LogIO::POST;
		fft.fftsky(subImage);
	} else {
		// Set vector of bools specifying axes
		Vector<Int> intAxes(axes);
		Vector<Bool> which(subImage.ndim(), False);
		for (uInt i = 0; i < intAxes.nelements(); i++)
			which(intAxes(i)) = True;
		//
		*itsLog << LogIO::NORMAL << "FFT axes " << intAxes + 1 << LogIO::POST;
		fft.fft(subImage, which);
	}

	// Write output files
	String maskName("");
	if (!realOut.empty()) {
		*itsLog << LogIO::NORMAL << "Creating image '" << realOut << "'"
				<< LogIO::POST;
		PagedImage<Float> realOutIm(subImage.shape(), subImage.coordinates(),
				realOut);
		if (subImage.isMasked())
			makeMask(realOutIm, maskName, False, True, *itsLog, True);
		fft.getReal(realOutIm);
	}
	if (!imagOut.empty()) {
		*itsLog << LogIO::NORMAL << "Creating image '" << imagOut << "'"
				<< LogIO::POST;
		PagedImage<Float> imagOutIm(subImage.shape(), subImage.coordinates(),
				imagOut);
		if (subImage.isMasked())
			makeMask(imagOutIm, maskName, False, True, *itsLog, True);
		fft.getImaginary(imagOutIm);
	}
	if (!ampOut.empty()) {
		*itsLog << LogIO::NORMAL << "Creating image '" << ampOut << "'"
				<< LogIO::POST;
		PagedImage<Float> ampOutIm(subImage.shape(), subImage.coordinates(),
				ampOut);
		if (subImage.isMasked())
			makeMask(ampOutIm, maskName, False, True, *itsLog, True);
		fft.getAmplitude(ampOutIm);
	}
	if (!phaseOut.empty()) {
		*itsLog << LogIO::NORMAL << "Creating image '" << phaseOut << "'"
				<< LogIO::POST;
		PagedImage<Float> phaseOutIm(subImage.shape(), subImage.coordinates(),
				phaseOut);
		if (subImage.isMasked())
			makeMask(phaseOutIm, maskName, False, True, *itsLog, True);
		fft.getPhase(phaseOutIm);
	}
	return True;
}

Record ImageAnalysis::findsources(const int nMax, const double cutoff,
		Record& Region, const String& mask, const Bool point, const Int width,
		const Bool absFind) {

	*itsLog << LogOrigin("ImageAnalysis", "findsources");

	AxesSpecifier axesSpec(False);
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p, *(ImageRegion::tweakedRegionRecord(&Region)),
		mask, itsLog, False, axesSpec
	);

	// Make finder
	ImageSourceFinder<Float> sf(subImage);

	// Find them
	ComponentList list = sf.findSources(*itsLog, nMax, cutoff, absFind, point,
			width);
	String error;
	Record listOut;
	if (!list.toRecord(error, listOut)) {
		*itsLog << "Can not convert output ComponentList to Record " << error
				<< LogIO::EXCEPTION;
	}

	return listOut;
}

ImageInterface<Float> *
ImageAnalysis::fitpolynomial(const String& residFile, const String& fitFile,
		const String& sigmaFile, const Int axis, const Int order,
		Record& Region, const String& mask, const Bool overwrite) {

	*itsLog << LogOrigin("ImageAnalysis", "fitpolynomial");

	*itsLog << "Mask is *" << mask << "*" << LogIO::POST;
	Int baseline = order;

	// Verify output file
	if (!overwrite && !residFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(residFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	// Make SubImages from input image
	ImageRegion* pRegionRegion = 0;
	ImageRegion* pMaskRegion = 0;
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		pRegionRegion, pMaskRegion,
		*pImage_p, *(ImageRegion::tweakedRegionRecord(&Region)),
		mask, 0, False
	);
	delete pMaskRegion;
	IPosition imageShape = subImage.shape();

	// Make subimage from input error image
	SubImage<Float>* pSubSigmaImage = 0;
	if (!sigmaFile.empty()) {
		PagedImage<Float> sigmaImage(sigmaFile);
		if (!sigmaImage.shape().conform(pImage_p->shape())) {
			*itsLog << "image and sigma images must have same shape"
					<< LogIO::EXCEPTION;
		}
		if (Region.nfields() > 0) {
			ImageRegion* pR = ImageRegion::fromRecord(
				itsLog, sigmaImage.coordinates(),
				sigmaImage.shape(), Region
			);
			pSubSigmaImage = new SubImage<Float> (sigmaImage, *pR, False);
			delete pR;
		} else {
			pSubSigmaImage = new SubImage<Float> (sigmaImage, False);
		}
	}

	// Find spectral axis if not given.
	CoordinateSystem cSys = subImage.coordinates();
	Int pAxis = CoordinateUtil::findSpectralAxis(cSys);
	Int axis2;
	if (axis < 0) {
		if (pAxis != -1) {
			axis2 = pAxis;
		} else {
			axis2 = subImage.ndim() - 1;
		}
	} else {
		axis2 = axis;
	}

	// Create output residual image (returned as an Image tool)
	// Create with no mask
	PtrHolder<ImageInterface<Float> > residImage;
	ImageInterface<Float>* pResid = 0;
	if (makeExternalImage(residImage, residFile, cSys, imageShape, subImage,
			*itsLog, overwrite, True, False))
		pResid = residImage.ptr()->cloneII();

	// Create optional disk image holding fit
	// Create with no mask
	PtrHolder<ImageInterface<Float> > fitImage;
	ImageInterface<Float>* pFit = 0;
	if (makeExternalImage(fitImage, fitFile, cSys, imageShape, subImage,
			*itsLog, overwrite, False, False))
		pFit = fitImage.ptr();

	// Make fitter
	Polynomial<AutoDiff<Float> > poly(baseline);
	LinearFitSVD<Float> fitter;
	fitter.setFunction(poly);

	// Fit
	LatticeFit::fitProfiles(pFit, pResid, subImage, pSubSigmaImage, fitter,
			axis2, True);
	if (pSubSigmaImage)
		delete pSubSigmaImage;

	// Copy mask from input image so that we exclude the OTF mask
	// in the output.  The OTF mask is just used to select what we fit
	// but should not be copied to the output
	SubImage<Float>* pSubImage2 = 0;
	if (pRegionRegion) {
		pSubImage2 = new SubImage<Float> (*pImage_p, *pRegionRegion, True);
	} else {
		pSubImage2 = new SubImage<Float> (*pImage_p, True);
	}
	delete pRegionRegion;

	if (pSubImage2->hasPixelMask()) {
		Lattice<Bool>& pixelMaskIn = pSubImage2->pixelMask();
		String maskNameResid;
		makeMask(*pResid, maskNameResid, False, True, *itsLog, True);
		{
			Lattice<Bool>& pixelMaskOut = pResid->pixelMask();
			pixelMaskOut.copyData(pixelMaskIn);
		}
		if (pFit) {
			String maskNameFit;
			makeMask(*pFit, maskNameFit, False, True, *itsLog, True);
			{
				Lattice<Bool>& pixelMaskOut = pFit->pixelMask();
				pixelMaskOut.copyData(pixelMaskIn);
			}
		}
	}
	delete pSubImage2;

	// Return residual image
	return pResid;
}

Bool ImageAnalysis::getchunk(Array<Float>& pixels, Array<Bool>& pixelMask,
		const Vector<Int>& blc, const Vector<Int>& trc, const Vector<Int>& inc,
		const Vector<Int>& axes, const Bool list, const Bool dropdeg,
		const Bool getmask) {

	// Recover some pixels from the image from a simple strided box
	*itsLog << LogOrigin("ImageAnalysis", "getchunk");

	IPosition iblc = IPosition(Vector<Int> (blc));
	IPosition itrc = IPosition(Vector<Int> (trc));
	IPosition imshape = pImage_p->shape();

	// Verify region.
	IPosition iinc = IPosition(inc.size());
	for (uInt i = 0; i < inc.size(); i++) {
		iinc(i) = inc[i];
	}
	LCBox::verify(iblc, itrc, iinc, imshape);
	if (list) {
		*itsLog << LogIO::NORMAL << "Selected bounding box " << iblc << " to "
				<< itrc << LogIO::POST;
	}

	// Get the chunk.  The mask is not returned. Leave that to getRegion
	IPosition curshape = (itrc - iblc + iinc) / iinc;
	Slicer sl(iblc, itrc, iinc, Slicer::endIsLast);
	SubImage<Float> subImage(*pImage_p, sl);
	//
	IPosition iAxes = IPosition(Vector<Int> (axes));
	if (getmask) {
		LatticeUtilities::collapse(pixels, pixelMask, iAxes, subImage, dropdeg);
		return True;
	} else {
		LatticeUtilities::collapse(pixels, iAxes, subImage, dropdeg);
		return True;
	}

}

const ImageInterface<Float>* ImageAnalysis::getImage() const {
	return pImage_p;
}


Bool ImageAnalysis::getregion(
	Array<Float>& pixels, Array<Bool>& pixelmask,
	Record& Region, const Vector<Int>& axes, const String& Mask,
	const Bool list, const Bool dropdeg, const Bool getmask,
	const bool extendMask
) {
	// Recover some pixels and their mask from a region in the image
	*itsLog << LogOrigin("ImageAnalysis", "getregion");

	// Get the region
	pixels.resize(IPosition(0, 0));
	pixelmask.resize(IPosition(0, 0));

	// Drop degenerate axes
	IPosition iAxes = IPosition(Vector<Int> (axes));

    SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p, *(ImageRegion::tweakedRegionRecord(&Region)),
		Mask, (list ? itsLog : 0), False, AxesSpecifier(),
		extendMask
	);
	if (getmask) {
        LatticeUtilities::collapse(pixels, pixelmask, iAxes, subImage, dropdeg);
	} else {
		LatticeUtilities::collapse(pixels, iAxes, subImage, dropdeg);
	}
    return True;
 }

Record*
ImageAnalysis::getslice(const Vector<Double>& x, const Vector<Double>& y,
		const Vector<Int>& axes, const Vector<Int>& coord, const Int npts,
		const String& method) {
	*itsLog << LogOrigin("ImageAnalysis", "getslice");

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
	LatticeSlice1D<Float> slicer(*pImage_p, method2);
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

ImageInterface<Float>* ImageAnalysis::hanning(
	const String& outFile, Record& Region,
	const String& mask, const Int axis, const Bool drop,
	const Bool overwrite, const Bool extendMask
) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	// Validate outfile
	if (!overwrite && !outFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	// Deal with axis
	Int iAxis = axis;
	if (iAxis < 0) {
		iAxis = CoordinateUtil::findSpectralAxis(pImage_p->coordinates());
		if (iAxis < 0) {
			*itsLog << "Could not find a spectral axis in input image"
					<< LogIO::EXCEPTION;
		}
	} else {
		if (iAxis > Int(pImage_p->ndim()) - 1) {
			*itsLog << "Specified axis of " << iAxis + 1
					<< "is greater than input image dimension of "
					<< pImage_p->ndim() << LogIO::EXCEPTION;
		}
	}

	ImageRegion* pRegionRegion = 0;
	ImageRegion* pMaskRegion = 0;
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		pRegionRegion, pMaskRegion,
		*pImage_p, *(ImageRegion::tweakedRegionRecord(&Region)), mask,
		itsLog, False, AxesSpecifier(), extendMask
	);
	IPosition blc(subImage.ndim(), 0);
	if (pRegionRegion) {
		LatticeRegion latRegion = pRegionRegion->toLatticeRegion(
			pImage_p->coordinates(), pImage_p->shape()
		);
		blc = latRegion.slicer().start();
	}
	delete pRegionRegion;
	delete pMaskRegion;

	// Work out shape of output image
	IPosition inShape(subImage.shape());
	IPosition outShape(inShape);
	if (drop) {
		outShape(iAxis) = inShape(iAxis) / 2;
		if (inShape(iAxis) % 2 == 0) {
			outShape(iAxis) = outShape(iAxis) - 1;
		}
	}
	*itsLog << LogIO::NORMAL << "Output image shape = " << outShape
			<< LogIO::POST;

	// Create output image coordinates.  Account for region selection and if
	// we drop every other point, the first output point is centred on
	// the second input pixel.
	Vector<Float> cInc(pImage_p->ndim(), 1.0);
	Vector<Float> cBlc(blc.nelements());
	for (uInt i = 0; i < cBlc.nelements(); i++)
		cBlc(i) = Float(blc(i));
	if (drop) {
		cInc(iAxis) = 2.0;
		cBlc(iAxis) += 1.0;
	}
	CoordinateSystem cSys = pImage_p->coordinates().subImage(
		cBlc, cInc, outShape.asVector()
	);

	// Make output image and mask if needed
	PtrHolder<ImageInterface<Float> > imOut;
	Bool isMasked = False;
	if (outFile.empty()) {
		*itsLog << LogIO::NORMAL << "Creating (temp)image '" << outFile
				<< "' of shape " << outShape << LogIO::POST;
		imOut.set(new TempImage<Float> (outShape, cSys));
	} else {
		*itsLog << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << outShape << LogIO::POST;
		imOut.set(new PagedImage<Float> (outShape, cSys, outFile));
	}
	ImageInterface<Float>* pImOut = imOut.ptr()->cloneII();
	if (subImage.isMasked()) {
		String maskName("");
		isMasked = makeMask(*pImOut, maskName, False, True, *itsLog, True);
	}

	// Create input image iterator
	IPosition inTileShape = subImage.niceCursorShape();
	TiledLineStepper inNav(subImage.shape(), inTileShape, iAxis);
	RO_MaskedLatticeIterator<Float> inIter(subImage, inNav);

	// Iterate by profile and smooth
	//Int nProfiles =
	//  subImage.shape().product()/inIter.vectorCursor().nelements();
	//    ProgressMeter clock(0.0, Double(nProfiles), "Hanning smooth", "Profiles smoothed", "", "", True, max(1,Int(nProfiles/20)));
	//    Double meterValue = 0.0;

	//
	IPosition outSliceShape(pImOut->ndim(), 1);
	outSliceShape(iAxis) = pImOut->shape()(iAxis);
	Array<Float> slice(outSliceShape);
	//
	IPosition inSliceShape(subImage.ndim(), 1);
	inSliceShape(iAxis) = subImage.shape()(iAxis);
	Array<Bool> maskIn(inSliceShape);
	Array<Bool> maskOut(outSliceShape);
	Lattice<Bool>* pMaskOut = 0;
	if (isMasked) {
		pMaskOut = &pImOut->pixelMask();
		if (!pMaskOut->isWritable()) {
			*itsLog << LogIO::WARN
					<< "The output image has a mask but it is not writable"
					<< endl;
			*itsLog << LogIO::WARN
					<< "So the mask will not be transferred to the output"
					<< LogIO::POST;
			isMasked = False;
		}
	}
	//
	while (!inIter.atEnd()) {
		if (isMasked) {
			inIter.getMask(maskIn, False);
			hanning_smooth(slice, maskOut, inIter.vectorCursor(), maskIn, True);
			pMaskOut->putSlice(maskOut, inIter.position());
		} else {
			hanning_smooth(slice, maskOut, inIter.vectorCursor(), maskIn, False);
		}
		pImOut->putSlice(slice, inIter.position());
		//
		inIter++;
		// meterValue += 1.0;
		// clock.update(meterValue);
	}
	ImageUtilities::copyMiscellaneous(*pImOut, *pImage_p);

	// Return handle to new file
	return pImOut;
}

Vector<Bool> ImageAnalysis::haslock() {
	Vector<Bool> rstat;
	*itsLog << LogOrigin("ImageAnalysis", "haslock");

	rstat.resize(2);
	rstat[0] = pImage_p->hasLock(FileLocker::Read);
	rstat[1] = pImage_p->hasLock(FileLocker::Write);
	return rstat;
}

Bool ImageAnalysis::histograms(
	Record& histout, const Vector<Int>& axes,
	Record& regionRec, const String& sMask, const Int nbins,
	const Vector<Double>& includepix, const Bool gauss,
	const Bool cumu, const Bool log, const Bool list,
	const String&, const Int nx, const Int ny,
	const Vector<Int>& size, const Bool force,
	const Bool disk, const Bool extendMask
) {

	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	ImageRegion* pRegionRegion = 0;
	ImageRegion* pMaskRegion = 0;

	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		pRegionRegion, pMaskRegion, *pImage_p,
		*(ImageRegion::tweakedRegionRecord(&regionRec)),
		sMask, itsLog, False, AxesSpecifier(), extendMask
	);

	// Make new object only if we need to.
	Bool forceNewStorage = force;
	if (pHistograms_p != 0 && oldHistStorageForce_p != disk) {
		forceNewStorage = True;
	}
	if (forceNewStorage) {
		delete pHistograms_p;
		pHistograms_p = 0;
		delete pOldHistRegionRegion_p;
		pOldHistRegionRegion_p = 0;
		delete pOldHistMaskRegion_p;
		pOldHistMaskRegion_p = 0;
		//
		pHistograms_p = new ImageHistograms<Float> (
			subImage, *itsLog, True, disk
		);
	}
	else {
		if (pHistograms_p == 0) {
			// We are here if this is the first time or the image has changed
			pHistograms_p = new ImageHistograms<Float> (
				subImage, *itsLog, True, disk
			);
		}
		else {
			// We already have a histogram object.  We only have to set
			// the new image (which will force the accumulation image
			// to be recomputed) if the region has changed.  If the image itself
			// changed, pHistograms_p will already have been set to 0
			pHistograms_p->resetError();
			if (
				haveRegionsChanged(pRegionRegion, pMaskRegion,
				pOldHistRegionRegion_p, pOldHistMaskRegion_p)
			) {
				pHistograms_p->setNewImage(subImage);
			}
		}
	}

	// Assign old regions to current regions
	delete pOldHistRegionRegion_p;
	pOldHistRegionRegion_p = 0;
	delete pOldHistMaskRegion_p;
	pOldHistMaskRegion_p = 0;
	//
	pOldHistRegionRegion_p = pRegionRegion;
	pOldHistMaskRegion_p = pMaskRegion;
	oldHistStorageForce_p = disk;

	// Set cursor axes
	Vector<Int> tmpaxes(axes);
	if (!pHistograms_p->setAxes(tmpaxes)) {
		*itsLog << pHistograms_p->errorMessage() << LogIO::EXCEPTION;
	}

	// Set number of bins
	if (!pHistograms_p->setNBins(nbins)) {
		*itsLog << pHistograms_p->errorMessage() << LogIO::EXCEPTION;
	}

	// Set pixel include ranges
	Vector<Float> tmpinclude(includepix.size());
	for (uInt i = 0; i < includepix.size(); i++) {
		tmpinclude[i] = includepix[i];
	}
	if (!pHistograms_p->setIncludeRange(tmpinclude)) {
		*itsLog << pHistograms_p->errorMessage() << LogIO::EXCEPTION;
	}

	// Plot the gaussian ?
	if (!pHistograms_p->setGaussian(gauss)) {
		*itsLog << pHistograms_p->errorMessage() << LogIO::EXCEPTION;
	}

	// Set form of histogram
	if (!pHistograms_p->setForm(log, cumu)) {
		*itsLog << pHistograms_p->errorMessage() << LogIO::EXCEPTION;
	}

	// List statistics as well ?
	if (!pHistograms_p->setStatsList(list)) {
		*itsLog << pHistograms_p->errorMessage() << LogIO::EXCEPTION;
	}

	// Make plots
	String pgdevice("/NULL");
	PGPlotter plotter;
	if (!pgdevice.empty()) {
		//      try {
		plotter = PGPlotter(pgdevice, 2, 100, size[0], size[1]);
		//      } catch (AipsError x) {
		//	*itsLog << LogIO::SEVERE << "Exception: " << x.getMesg() << LogIO::POST;
		//	return False;
		//      }
		Vector<Int> nxy(2);
		nxy(0) = nx;
		nxy(1) = ny;
		if (nx < 0 || ny < 0) {
			nxy.resize(0);
		}
		if (!pHistograms_p->setPlotting(plotter, nxy)) {
			*itsLog << pHistograms_p->errorMessage() << LogIO::EXCEPTION;
		}
	}

	if (plotter.isAttached()) {
		if (!pHistograms_p->display()) {
			*itsLog << pHistograms_p->errorMessage() << LogIO::EXCEPTION;
		}
		pHistograms_p->closePlotting();
	}

	// If OK recover the histogram into the Glish record
	Array<Float> values, counts;
	if (!pHistograms_p->getHistograms(values, counts)) {
		*itsLog << pHistograms_p->errorMessage() << LogIO::EXCEPTION;
	}
	//
	histout.define(RecordFieldId("values"), values);
	histout.define(RecordFieldId("counts"), counts);
	return True;
}

Vector<String> ImageAnalysis::history(const Bool list, const Bool browse) {
	*itsLog << LogOrigin("ImageAnalysis", "history");

	//
	if (browse) {
		*itsLog << "Table browsing is not implemented yet!" << LogIO::POST;
	}
	Vector<String> t;
	LoggerHolder& logger = pImage_p->logger();
	//
	uInt i = 1;
	for (LoggerHolder::const_iterator iter = logger.begin(); iter
			!= logger.end(); iter++, i++) {
		if (list) {
			if (!(iter->location()).empty()) {
				*itsLog << LogOrigin(iter->location());
			} else {
				*itsLog << LogOrigin("ImageAnalysis", "history");
			}
			*itsLog << endl << iter->message() << endl << LogIO::POST;
		} else {
			if (i > t.nelements()) {
				t.resize(t.nelements() + 100, True);
			}
			t(i - 1) = iter->message();
		}
	}
	if (list)
		*itsLog << LogIO::POST;
	//
	if (!list) {
		t.resize(i - 1, True);
	}
	return t;
}

ImageInterface<Float> *
ImageAnalysis::insert(const String& infile, Record& Region,
		const Vector<double>& locatePixel) {

	*itsLog << LogOrigin("ImageAnalysis", "insert");

	Bool doRef;
	if (locatePixel.size() == 0) {
		doRef = True;
	} else {
		doRef = False;
	}
	Int dbg = 0;

	// Open input image
	ImageInterface<Float>* pInImage = 0;
	ImageUtilities::openImage(pInImage, infile, *itsLog);

	// Create region and subImage for input image
	const ImageRegion* pRegion = ImageRegion::fromRecord(
		itsLog, pInImage->coordinates(),
		pInImage->shape(), Region
	);
	SubImage<Float> inSub(*pInImage, *pRegion);
	delete pRegion;

	// Generate output pixel location
	const IPosition inShape = inSub.shape();
	const IPosition outShape = pImage_p->shape();
	const uInt nDim = pImage_p->ndim();
	Vector<Double> outPix(nDim);
	const uInt nDim2 = locatePixel.nelements();
	//
	if (doRef) {
		outPix.resize(0);
	} else {
		for (uInt i = 0; i < nDim; i++) {
			if (i < nDim2) {
				//	  outPix[i] = locatePixel[i] - 1.0;              // 1 -> 0 rel
				outPix[i] = locatePixel[i];
			} else {
				outPix[i] = (outShape(i) - inShape(i)) / 2.0; // Centrally located
			}
		}
	}

	// Insert
	ImageRegrid<Float> ir;
	ir.showDebugInfo(dbg);
	ir.insert(*pImage_p, outPix, inSub);
	delete pInImage;

	// Make sure hist and stats are redone
	deleteHistAndStats();

	return pImage_p;

}


Bool ImageAnalysis::ispersistent() {
	*itsLog << LogOrigin("ImageAnalysis", "ispersistent");

	return pImage_p->isPersistent();

}

Bool ImageAnalysis::lock(const Bool writelock, const Int nattempts) {

	*itsLog << LogOrigin("ImageAnalysis", "lock");

	FileLocker::LockType locker = FileLocker::Read;
	if (writelock)
		locker = FileLocker::Write;
	uInt n = max(0, nattempts);
	return pImage_p->lock(locker, n);
}

Bool ImageAnalysis::makecomplex(const String& outFile, const String& imagFile,
		Record& Region, const Bool overwrite) {

	*itsLog << LogOrigin("ImageAnalysis", "makecomplex");

	// Check output file
	if (!overwrite && !outFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	// Open images and check consistency
	PagedImage<Float> imagImage(imagFile);
	//
	const IPosition realShape = pImage_p->shape();
	const IPosition imagShape = imagImage.shape();
	if (!realShape.isEqual(imagShape)) {
		*itsLog << "Image shapes are not identical" << LogIO::EXCEPTION;
	}
	//
	CoordinateSystem cSysReal = pImage_p->coordinates();
	CoordinateSystem cSysImag = imagImage.coordinates();
	if (!cSysReal.near(cSysImag)) {
		*itsLog << "Image Coordinate systems are not conformant" << LogIO::POST;
	}

	String mask;
	SubImage<Float> subRealImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&Region)),
		mask, itsLog, False
	);
	SubImage<Float> subImagImage = SubImage<Float>::createSubImage(
		imagImage,
		*(ImageRegion::tweakedRegionRecord(&Region)),
		mask, 0, False
	);

	// LEL node
	LatticeExprNode node(formComplex(subRealImage, subImagImage));
	LatticeExpr<Complex> expr(node);
	//
	PagedImage<Complex> outImage(realShape, cSysReal, outFile);
	outImage.copyData(expr);
	ImageUtilities::copyMiscellaneous(outImage, *pImage_p);
	return True;
}

Vector<String> ImageAnalysis::maskhandler(const String& op,
		const Vector<String>& namesIn) {
	*itsLog << LogOrigin("ImageAnalysis", "maskhandler");

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
			pImage_p->setDefaultMask(String(""));
		} else {
			pImage_p->setDefaultMask(namesIn(0));
		}
	} else if (OP.contains(String("DEF"))) {
		// Return default mask
		namesOut.resize(1);
		namesOut(0) = pImage_p->getDefaultMask();
		hasOutput = True;
	} else if (OP.contains(String("DEL"))) {
		// Delete mask(s)
		if (n <= 0) {
			*itsLog << "You have not supplied any mask names"
					<< LogIO::EXCEPTION;
		}
		for (uInt i = 0; i < n; i++) {
			pImage_p->removeRegion(namesIn(i), RegionHandler::Masks, False);
		}
	} else if (OP.contains(String("REN"))) {
		// Rename masks
		if (n != 2) {
			*itsLog << "You must give two mask names" << LogIO::EXCEPTION;
		}
		pImage_p->renameRegion(namesIn(1), namesIn(0), RegionHandler::Masks,
				False);
	} else if (OP.contains(String("GET"))) {
		// Get names of all masks
		namesOut.resize(0);
		namesOut = pImage_p->regionNames(RegionHandler::Masks);
		hasOutput = True;
	} else if (OP.contains(String("COP"))) {
		// Copy mask;  maskIn maskOut  or imageIn:maskIn maskOut
		if (n != 2) {
			*itsLog << "You must give two mask names" << LogIO::EXCEPTION;
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
			*itsLog << "Illegal number of mask names" << LogIO::EXCEPTION;
		}
		//
		if (pImage_p->hasRegion(maskOut, RegionHandler::Any)) {
			*itsLog << "The mask " << maskOut << " already exists in image "
					<< pImage_p->name() << LogIO::EXCEPTION;
		}

		// Create new mask in output
		pImage_p->makeMask(maskOut, True, False);

		// Copy masks
		ImageInterface<Float>* pImIn = 0;
		if (external) {
			pImIn = new PagedImage<Float> (nameIn);
			if (pImIn->shape() != pImage_p->shape()) {
				*itsLog << "Images have different shapes" << LogIO::EXCEPTION;
			}
		} else {
			pImIn = pImage_p;
		}
		//
		AxesSpecifier axesSpecifier;
		ImageUtilities::copyMask(*pImage_p, *pImIn, maskOut, maskIn,
				axesSpecifier);
		//
		if (external) {
			delete pImIn;
			pImIn = 0;
		}
	} else {
		*itsLog << "Unknown operation" << LogIO::EXCEPTION;
	}

	// Make sure hist and stats are redone
	deleteHistAndStats();

	if (hasOutput)
		return namesOut;
	return Vector<String> (0);
}

Record ImageAnalysis::miscinfo() {
	*itsLog << LogOrigin("ImageAnalysis", "miscinfo");

	Record tmp = pImage_p->miscInfo();
	return tmp;
}

Bool ImageAnalysis::modify(
	Record& Model, Record& Region, const String& mask,
	const Bool subtract, const Bool list, const Bool extendMask) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	String error;
	ComponentList cL;
	if (!cL.fromRecord(error, Model)) {
		*itsLog << LogIO::SEVERE
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
		*itsLog << "There are no components in the model componentlist"
				<< LogIO::EXCEPTION;
	}

	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&Region)),
		mask,  (list ? itsLog : 0), True, AxesSpecifier(), extendMask
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
	ComponentImager::project(subImage, cl);

	// Ensure that we reconstruct the statistics and histograms objects
	// now that the data have changed
	deleteHistAndStats();

	return True;
}

Record ImageAnalysis::maxfit(Record& Region, const Bool doPoint,
		const Int width, const Bool absFind, const Bool list) {

	*itsLog << LogOrigin("ImageAnalysis", "maxfit");

	SkyComponent sky; // Output
	Vector<Double> absPixel; // Output

	// Make subimage
	ImageRegion* pRegionRegion = 0;
	ImageRegion* pMaskRegion = 0;
	AxesSpecifier axesSpec(False); // drop degenerate
	String mask;
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		pRegionRegion, pMaskRegion, *pImage_p,
		*(ImageRegion::tweakedRegionRecord(&Region)),
		mask, itsLog, False, axesSpec
	);
	Vector<Float> blc;
	if (pRegionRegion) {
		blc = pRegionRegion->asLCSlicer().blc();
	} else {
		blc.resize(subImage.ndim());
		blc = 0.0;
	}
	delete pRegionRegion;
	delete pMaskRegion;

	// Find it
	ImageSourceFinder<Float> sf(subImage);
	Double cutoff = 0.1;
	sky
			= sf.findSourceInSky(*itsLog, absPixel, cutoff, absFind, doPoint,
					width);
	//absPixel += 1.0;

	// modify to show dropped degenerate axes values???
	if (list) {
		*itsLog << LogIO::NORMAL << "Brightness     = " << sky.flux().value()
				<< " " << sky.flux().unit().getName() << LogIO::POST;
		CoordinateSystem cSys = subImage.coordinates();
		*itsLog << "World Axis Units: " << cSys.worldAxisUnits() << LogIO::POST;
		Vector<Double> wPix;
		if (!cSys.toWorld(wPix, absPixel)) {
			*itsLog << LogIO::WARN
					<< "Failed to convert to absolute world coordinates "
					<< cSys.errorMessage() << LogIO::POST;
		} else {
			*itsLog << "Absolute world = " << wPix << LogIO::POST;
		}
		Vector<Double> wRel = wPix.copy();
		cSys.makeWorldRelative(wRel);
		*itsLog << "Relative world = " << wRel << LogIO::POST;
		*itsLog << LogIO::NORMAL << "Absolute pixel = " << absPixel << endl;
		Vector<Double> pRel = absPixel.copy();
		cSys.makePixelRelative(pRel);
		*itsLog << "Relative pixel = " << pRel << LogIO::POST;
	}

	ComponentList mycomp;
	mycomp.add(sky);

	String error;
	Record outrec;
	if (!mycomp.toRecord(error, outrec)) {
		*itsLog << LogIO::SEVERE
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
	const String& smoothout, const String& pgdevice, const Int nx,
	const Int ny, const Bool yind, const Bool overwrite,
	const Bool removeAxis, const Bool stretchMask
) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	// check that we can write to smoothout if specified
	if (!smoothout.empty() and !overwrite) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(smoothout, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	// Note that the user may give the strings (method & kernels)
	// as either vectors of strings or one string with separators.
	// Hence the code below that deals with it.   Also in image.g we therefore
	// give the default value as a blank string rather than a null vector.

	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&Region)),
		mask, itsLog, False, AxesSpecifier(), stretchMask
	);
	// Create ImageMoments object
	ImageMoments<Float> momentMaker(subImage, *itsLog, overwrite, True);

	// Set which moments to output
	if (!momentMaker.setMoments(whichmoments + 1)) {
		*itsLog << momentMaker.errorMessage() << LogIO::EXCEPTION;
	}

	// Set moment axis
	if (axis >= 0) {
		if (!momentMaker.setMomentAxis(axis)) {
			*itsLog << momentMaker.errorMessage() << LogIO::EXCEPTION;
		}
	}
	// Set moment methods
	if (method.nelements() > 0 && method(0) != "") {
		String tmp;
		for (uInt i = 0; i < method.nelements(); i++) {
			tmp += method(i) + " ";
		}
		Vector<Int> intmethods = momentMaker.toMethodTypes(tmp);
		if (!momentMaker.setWinFitMethod(intmethods)) {
			*itsLog << momentMaker.errorMessage() << LogIO::EXCEPTION;
		}
	}
	// Set smoothing
	if (kernels.nelements() >= 1 && kernels(0) != "" && smoothaxes.size() >= 1
			&& kernelwidths.nelements() >= 1) {
		String tmp;
		for (uInt i = 0; i < kernels.nelements(); i++) {
			tmp += kernels(i) + " ";
		}
		//
		Vector<Int> intkernels = VectorKernel::toKernelTypes(kernels);
		Vector<Int> intaxes(smoothaxes);
		if (!momentMaker.setSmoothMethod(intaxes, intkernels, kernelwidths)) {
			*itsLog << momentMaker.errorMessage() << LogIO::EXCEPTION;
		}
	}

	// Set pixel include/exclude range
	if (!momentMaker.setInExCludeRange(includepix, excludepix)) {
		*itsLog << momentMaker.errorMessage() << LogIO::EXCEPTION;
	}

	// Set SNR cutoff
	if (!momentMaker.setSnr(peaksnr, stddev)) {
		*itsLog << momentMaker.errorMessage() << LogIO::EXCEPTION;
	}
	// Set velocity type
	if (!velocityType.empty()) {
		MDoppler::Types velType;
		if (!MDoppler::getType(velType, velocityType)) {
			*itsLog << LogIO::WARN << "Illegal velocity type, using RADIO"
					<< LogIO::POST;
			velType = MDoppler::RADIO;
		}
		momentMaker.setVelocityType(velType);
	}

	// Set output names
	if (smoothout != "" && !momentMaker.setSmoothOutName(smoothout)) {
		*itsLog << momentMaker.errorMessage() << LogIO::EXCEPTION;
	}
	// Set plotting attributes
	PGPlotter plotter;
	if (!pgdevice.empty()) {
		//      try {
		plotter = PGPlotter(pgdevice);
		//      } catch (AipsError x) {
		//          *itsLog << LogIO::SEVERE << "Exception: " << x.getMesg() << LogIO::POST;
		//          return False;
		//      }
		Vector<Int> nxy(2);
		nxy(0) = nx;
		nxy(1) = ny;
		if (nx < 0 || ny < 0)
			nxy.resize(0);
		if (!momentMaker.setPlotting(plotter, nxy, yind)) {
			*itsLog << momentMaker.errorMessage() << LogIO::EXCEPTION;
		}
	}
	// If no file name given for one moment image, make TempImage.
	// Else PagedImage results
	Bool doTemp = False;
	if (out.empty() && whichmoments.nelements() == 1)
		doTemp = True;

	// Create moments
	PtrBlock<MaskedLattice<Float>*> images;
	if (!momentMaker.createMoments(images, doTemp, out, removeAxis)) {
		*itsLog << momentMaker.errorMessage() << LogIO::EXCEPTION;
	}
	momentMaker.closePlotting();

	// Return handle of first image
	ImageInterface<Float>* pIm =
			dynamic_cast<ImageInterface<Float>*> (images[0]);

	// Clean up pointer block except for the one pointed by pIm
	for (uInt i = 1; i < images.nelements(); i++)
		delete images[i];
	//
	return pIm;
}

String ImageAnalysis::name(const Bool strippath) {
	*itsLog << LogOrigin("ImageAnalysis", "name");
	return pImage_p->name(strippath);
}

Record*
ImageAnalysis::pixelvalue(const Vector<Int>& pixel) {
	*itsLog << LogOrigin("ImageAnalysis", "pixelvalue");

	//

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
		*itsLog << error << LogIO::POST;
	} else {
		outRec->defineRecord("value", qr);
	}
	outRec->define("pixel", pos);
	return outRec;
}

void ImageAnalysis::pixelValue(Bool& offImage, Quantum<Double>& value,
		Bool& mask, Vector<Int>& pos) const {
	//
	const IPosition imShape = pImage_p->shape();
	const Vector<Double> refPix = pImage_p->coordinates().referencePixel();
	const uInt nDim = pImage_p->ndim();
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
	IPosition shp(pImage_p->ndim(), 1);
	Array<Float> pixels = pImage_p->getSlice(iPos, shp);
	Array<Bool> maskPixels = pImage_p->getMaskSlice(iPos, shp);
	Unit units = pImage_p->units();
	//

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

Bool ImageAnalysis::putchunk(const Array<Float>& pixelsArray,
		const Vector<Int>& blc, const Vector<Int>& inc, const Bool list,
		const Bool locking, const Bool replicate) {
	*itsLog << LogOrigin("ImageAnalysis", "putchunk");

	//
	IPosition imageShape = pImage_p->shape();
	uInt ndim = imageShape.nelements();
	if (pixelsArray.ndim() > ndim) {
		*itsLog << "Pixels array has more axes than the image!"
				<< LogIO::EXCEPTION;
	}

	// Verify blc value. Fill in values for blc and inc.  trc set to shape-1
	IPosition iblc = IPosition(Vector<Int> (blc));
	IPosition itrc;
	IPosition iinc(inc.size());
	for (uInt i = 0; i < inc.size(); i++)
		iinc(i) = inc[i];
	LCBox::verify(iblc, itrc, iinc, imageShape);

	// Create two slicers; one describing the region defined by blc + shape-1
	// with extra axes given length 1. The other we extend with the shape
	IPosition len = pixelsArray.shape();
	len.resize(ndim, True);
	for (uInt i = pixelsArray.shape().nelements(); i < ndim; i++) {
		len(i) = 1;
		itrc(i) = imageShape(i) - 1;
	}
	Slicer sl(iblc, len, iinc, Slicer::endIsLength);
	if (sl.end() + 1 > imageShape) {
		*itsLog << "Pixels array, including inc, extends beyond edge of image."
				<< LogIO::EXCEPTION;
	}
	Slicer sl2(iblc, itrc, iinc, Slicer::endIsLast);

	//
	if (list) {
		*itsLog << LogIO::NORMAL << "Selected bounding box " << sl.start()
				<< " to " << sl.end() << LogIO::POST;
	}

	// Put the pixels
	if (pixelsArray.ndim() == ndim) {
		set_cache(pixelsArray.shape());
		if (replicate) {
			LatticeUtilities::replicate(*pImage_p, sl2, pixelsArray);
		} else {
			pImage_p->putSlice(pixelsArray, iblc, iinc);
		}
	} else {
		// Pad with extra degenerate axes if necessary (since it is somewhat
		// costly).
		Array<Float> pixelsref(pixelsArray.addDegenerate(ndim
				- pixelsArray.ndim()));
		set_cache(pixelsref.shape());
		if (replicate) {
			LatticeUtilities::replicate(*pImage_p, sl2, pixelsref);
		} else {
			pImage_p->putSlice(pixelsref, iblc, iinc);
		}
	}
	// Ensure that we reconstruct the statistics and histograms objects
	// now that the data have changed
	deleteHistAndStats();

	Bool rstat = True;

	if (locking) {
		rstat = unlock();
	}
	return rstat;
}

Bool ImageAnalysis::putregion(const Array<Float>& pixels,
		const Array<Bool>& mask, Record& region, const Bool list,
		const Bool usemask, const Bool, const Bool replicateArray) {
	*itsLog << LogOrigin("ImageAnalysis", "putregion");

	// used to verify array dimension
	uInt img_ndim = pImage_p->shape().asVector().nelements();

	// Checks on pixels dimensions
	Vector<Int> p_shape = pixels.shape().asVector();
	uInt p_ndim = p_shape.size();
	if (p_ndim > img_ndim) {
		*itsLog << "Pixels array has more axes than the image"
				<< LogIO::EXCEPTION;
		return False;
	}
	//    if (p_ndim == 0) {
	//  *itsLog << "The pixels array is empty" << LogIO::EXCEPTION;
	//}
	for (uInt i = 0; i < p_ndim; i++) {
		if (p_shape(i) <= 0) {
			*itsLog << "The shape of the pixels array is invalid"
					<< LogIO::EXCEPTION;
			return False;
		}
	}

	// Checks on pixelmask dimensions
	Vector<Int> m_shape = mask.shape().asVector();
	uInt m_ndim = m_shape.size();
	if (m_ndim > img_ndim) {
		*itsLog << "Mask array has more axes than the image"
				<< LogIO::EXCEPTION;
	}
	//if (m_ndim == 0) {
	//  *itsLog << "The pixelmask array is empty" << LogIO::EXCEPTION;
	//  return False;
	//}
	for (uInt i = 0; i < m_ndim; i++) {
		if (m_shape(i) <= 0) {
			*itsLog << "The shape of the pixelmask array is invalid"
					<< LogIO::EXCEPTION;
			return False;
		}
	}

	// Warning, an empty Array comes through the Glish tasking system
	// as shape = [0], ndim = 1, nelements = 0
	IPosition dataShape;
	uInt dataDim = 0;
	uInt pixelElements = pixels.nelements();
	uInt maskElements = mask.nelements();

	//
	if (pixelElements != 0 && maskElements != 0) {
		if (!pixels.shape().isEqual(mask.shape())) {
			*itsLog << "Pixels and mask arrays have different shapes"
					<< LogIO::EXCEPTION;
		}
		if (pixelElements != 0) {
			dataShape = pixels.shape();
			dataDim = pixels.ndim();
		} else {
			dataShape = mask.shape();
			dataDim = mask.ndim();
		}
	} else if (pixelElements != 0) {
		dataShape = pixels.shape();
		dataDim = pixels.ndim();
	} else if (maskElements != 0) {
		dataShape = mask.shape();
		dataDim = mask.ndim();
	} else {
		*itsLog << "Pixels and mask arrays are both zero length"
				<< LogIO::EXCEPTION;
	}

	// Make region.  If the region extends beyond the image, it is
	// truncated here.

	const ImageRegion* pRegion = ImageRegion::fromRecord(
		(list ? itsLog : 0), pImage_p->coordinates(), pImage_p->shape(),
		*ImageRegion::tweakedRegionRecord(&region)
	);
	LatticeRegion latRegion = pRegion->toLatticeRegion(pImage_p->coordinates(),
			pImage_p->shape());
	// The pixels array must be same shape as the bounding box of the
	// region for as many axes as there are in the pixels array.  We
	// pad with degenerate axes for missing axes. If the region
	// dangled over the edge, it will have been truncated and the
	// array will no longer be the correct shape and we get an error.
	// We could go to the trouble of fishing out the bit that doesn't
	// fall off the edge.
	for (uInt i = 0; i < dataDim; i++) {
		if (dataShape(i) != latRegion.shape()(i)) {
			if (!(i == dataDim - 1 && dataShape(i) == 1)) {
				ostringstream oss;
				oss << "Data array shape (" << dataShape
						<< ") including inc, does not"
						<< " match the shape of the region bounding box ("
						<< latRegion.shape() << ")" << endl;
				*itsLog << String(oss) << LogIO::EXCEPTION;
			}
		}
	}

	// If our image doesn't have a mask, try and make it one.
	if (maskElements > 0) {
		if (!pImage_p->hasPixelMask()) {
			String maskName("");
			makeMask(*pImage_p, maskName, True, True, *itsLog, list);
		}
	}
	Bool useMask2 = usemask;
	if (!pImage_p->isMasked())
		useMask2 = False;

	// Put the mask first
	if (maskElements > 0 && pImage_p->hasPixelMask()) {
		Lattice<Bool>& maskOut = pImage_p->pixelMask();
		if (maskOut.isWritable()) {
			if (dataDim == img_ndim) {
				if (replicateArray) {
					LatticeUtilities::replicate(maskOut, latRegion.slicer(),
							mask);
				} else {
					maskOut.putSlice(mask, latRegion.slicer().start());
				}
			} else {
				*itsLog << LogIO::NORMAL
						<< "Padding mask array with degenerate axes"
						<< LogIO::POST;
				Array<Bool> maskref(mask.addDegenerate(img_ndim - mask.ndim()));
				if (replicateArray) {
					LatticeUtilities::replicate(maskOut, latRegion.slicer(),
							maskref);
				} else {
					maskOut.putSlice(maskref, latRegion.slicer().start());
				}
			}
		} else {
			*itsLog
					<< "The mask is not writable. Probably an ImageExpr or SubImage"
					<< LogIO::EXCEPTION;
		}
	}

	// Get the mask and data from disk if we need it
	IPosition pixelsShape = pixels.shape();
	Array<Bool> oldMask;
	Array<Float> oldData;
	Bool deleteOldMask, deleteOldData, deleteNewData;
	const Bool* pOldMask = 0;
	const Float* pOldData = 0;
	const Float* pNewData = 0;
	if (pixelElements > 0 && useMask2) {
		if (pixels.ndim() != img_ndim) {
			pixelsShape.append(IPosition(img_ndim - pixels.ndim(), 1));
		}
		oldData = pImage_p->getSlice(latRegion.slicer().start(), pixelsShape,
				False);
		oldMask = pImage_p->getMaskSlice(latRegion.slicer().start(),
				pixelsShape, False);
		pOldData = oldData.getStorage(deleteOldData); // From disk
		pOldMask = oldMask.getStorage(deleteOldMask); // From disk
		pNewData = pixels.getStorage(deleteNewData); // From user
	}

	// Put the pixels
	if (dataDim == img_ndim) {
		if (pixelElements > 0) {
			if (useMask2) {
				Bool deleteNewData2;
				Array<Float> pixels2(pixelsShape);
				Float* pNewData2 = pixels2.getStorage(deleteNewData2);
				for (uInt i = 0; i < pixels2.nelements(); i++) {
					pNewData2[i] = pNewData[i]; // Value user gives
					if (!pOldMask[i])
						pNewData2[i] = pOldData[i]; // Value on disk
				}
				pixels2.putStorage(pNewData2, deleteNewData2);
				if (replicateArray) {
					LatticeUtilities::replicate(*pImage_p, latRegion.slicer(),
							pixels2);
				} else {
					pImage_p->putSlice(pixels2, latRegion.slicer().start());
				}
			} else {
				if (replicateArray) {
					LatticeUtilities::replicate(*pImage_p, latRegion.slicer(),
							pixels);
				} else {
					pImage_p->putSlice(pixels, latRegion.slicer().start());
				}
			}
		}
	} else {
		if (pixelElements > 0) {
			*itsLog << LogIO::NORMAL
					<< "Padding pixels array with degenerate axes"
					<< LogIO::POST;
			//
			if (useMask2) {
				Bool deleteNewData2;
				Array<Float> pixels2(pixelsShape);
				Float* pNewData2 = pixels2.getStorage(deleteNewData2);
				for (uInt i = 0; i < pixels2.nelements(); i++) {
					pNewData2[i] = pNewData[i]; // Value user gives
					if (!pOldMask[i])
						pNewData2[i] = pOldData[i]; // Value on disk
				}
				pixels2.putStorage(pNewData2, deleteNewData2);
				if (replicateArray) {
					LatticeUtilities::replicate(*pImage_p, latRegion.slicer(),
							pixels2);
				} else {
					pImage_p->putSlice(pixels2, latRegion.slicer().start());
				}
			} else {
				Array<Float> pixelsref(pixels.addDegenerate(img_ndim
						- pixels.ndim()));
				if (replicateArray) {
					LatticeUtilities::replicate(*pImage_p, latRegion.slicer(),
							pixelsref);
				} else {
					pImage_p->putSlice(pixelsref, latRegion.slicer().start());
				}
			}
		}
	}

	if (pOldMask != 0)
		oldMask.freeStorage(pOldMask, deleteOldMask);
	if (pOldData != 0)
		oldData.freeStorage(pOldData, deleteOldData);
	if (pNewData != 0)
		pixels.freeStorage(pNewData, deleteNewData);
	delete pRegion;

	// Ensure that we reconstruct the statistics and histograms objects
	// now that the data have changed
	deleteHistAndStats();

	return unlock();

}

ImageInterface<Float>* ImageAnalysis::rebin(
	const String& outFile, const Vector<Int>& factors,
	Record& Region, const String& mask, const Bool dropdeg,
	const Bool overwrite, const Bool extendMask
) {

	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	// Validate outfile
	if (!overwrite && !outFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	// Convert region from Glish record to ImageRegion. Convert mask
	// to ImageRegion and make SubImage.
	AxesSpecifier axesSpecifier;
	if (dropdeg)
		axesSpecifier = AxesSpecifier(False);
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&Region)),
		mask, itsLog, False, axesSpecifier, extendMask
	);

	// Convert binning factors
	IPosition factors2(subImage.ndim());
	for (uInt i = 0; i < factors.nelements(); i++) {
		if (factors(i) <= 0) {
			*itsLog << "Binning factors must be positive" << LogIO::EXCEPTION;
		}
		factors2[i] = max(1, factors[i]);
	}

	// Create rebinner
	RebinImage<Float> binIm(subImage, factors2);
	IPosition outShape = binIm.shape();
	CoordinateSystem cSysOut = binIm.coordinates();

	// Create the image and mask
	PtrHolder<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*itsLog << LogIO::NORMAL << "Creating (temp)image of shape "
				<< outShape << LogIO::POST;
		imOut.set(new TempImage<Float> (outShape, cSysOut));
	} else {
		*itsLog << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << outShape << LogIO::POST;
		imOut.set(new PagedImage<Float> (outShape, cSysOut, outFile));
	}
	ImageInterface<Float>* pImOut = imOut.ptr()->cloneII();
	String maskName("");
	makeMask(*pImOut, maskName, True, True, *itsLog, True);

	// Do the work
	LatticeUtilities::copyDataAndMask(*itsLog, *pImOut, binIm);

	// Copy miscellaneous things over
	ImageUtilities::copyMiscellaneous(*pImOut, binIm);

	// Return image
	return pImOut;
}

ImageInterface<Float>* ImageAnalysis::_regrid(
	const String& outFile, const Vector<Int>& inshape,
	const CoordinateSystem& coordinates, const Vector<Int>& inaxes,
	Record& Region, const String& mask, const String& methodU,
	const Int decimate, const Bool replicate,
	const Bool doRefChange, const Bool dropDegenerateAxes,
	const Bool overwrite, const Bool forceRegrid,
	const Bool extendMask
) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	Int dbg = 0;

	String method2 = methodU;
	method2.upcase();

	// Validate outfile
	if (!overwrite && !outFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	Vector<Int> tmpShape;
	Vector<Int> tmpShape2;
	if (inshape.size() == 1 && inshape[0] == -1) {
		tmpShape = pImage_p->shape().asVector();
		tmpShape2.resize(tmpShape.size());
		if (dropDegenerateAxes) {
			int j = 0;
			for (uInt i = 0; i < tmpShape.size(); i++) {
				if (tmpShape[i] != 1) {
					tmpShape2[j] = tmpShape[i];
					j++;
				}
			}
			tmpShape2.resize(j);
			tmpShape = tmpShape2;
		}
	}
	else {
		tmpShape = inshape;
	}

	// Convert region from Glish record to ImageRegion. Convert mask
	// to ImageRegion and make SubImage.
	AxesSpecifier axesSpecifier;
	if (dropDegenerateAxes)
		axesSpecifier = AxesSpecifier(False);
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&Region)),
		mask, itsLog, False, axesSpecifier, extendMask
	);

	// Deal with axes
	Vector<Int> axes(inaxes);
	IPosition axes2(axes);
	//cout << "axes2 " << axes2 << endl;
	Vector<Int> shape(tmpShape);
	IPosition outShape(shape);

	// Make CoordinateSystem from user given
	CoordinateSystem cSysFrom = subImage.coordinates();
	CoordinateSystem pCSTo(
		coordinates.nCoordinates() != 0
		? coordinates
		: subImage.coordinates()
	);
	pCSTo.setObsInfo(cSysFrom.obsInfo());

	// Now build a CS which copies the user specified Coordinate for
	// axes to be regridded and the input image Coordinate for axes not
	// to be regridded
	CoordinateSystem cSys = ImageRegrid<Float>::makeCoordinateSystem(*itsLog,
		pCSTo, cSysFrom, axes2
	);
	if (cSys.nPixelAxes() != outShape.nelements()) {
		*itsLog
				<< "The number of pixel axes in the output shape and Coordinate System must be the same"
				<< LogIO::EXCEPTION;
	}

	// Create the image and mask
	PtrHolder<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*itsLog << LogIO::NORMAL << "Creating (temp)image of shape "
				<< outShape << LogIO::POST;
		imOut.set(new TempImage<Float> (outShape, cSys));
	} else {
		*itsLog << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << outShape << LogIO::POST;
		imOut.set(new PagedImage<Float> (outShape, cSys, outFile));
	}
	ImageInterface<Float>* pImOut = imOut.ptr()->cloneII();
	pImOut->set(0.0);
	ImageUtilities::copyMiscellaneous(*pImOut, subImage);
	String maskName("");
	makeMask(*pImOut, maskName, True, True, *itsLog, True);
	//
	Interpolate2D::Method method = Interpolate2D::stringToMethod(methodU);
	IPosition dummy;
	ImageRegrid<Float> ir;
	ir.showDebugInfo(dbg);
	ir.disableReferenceConversions(!doRefChange);
	ir.regrid(
		*pImOut, method, axes2, subImage,
		replicate, decimate, True,
		forceRegrid
	);

	// Cleanup and return image
	return pImOut;
}

ImageInterface<Float>* ImageAnalysis::regrid(
	const String& outFile, const Vector<Int>& inshape,
	const Record& coordinates, const Vector<Int>& inaxes,
	Record& Region, const String& mask,
	const String& methodU, const Int decimate,
	const Bool replicate, const Bool doRefChange,
	const Bool dropDegenerateAxes, const Bool overwrite,
	const Bool forceRegrid, const Bool specAsVelocity,
	const Bool extendMask
) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	// must deal with default shape and dropDegenerateAxes
	Vector<Int> tmpShape;
	Vector<Int> tmpShape2;
	if (inshape.size() == 1 && inshape[0] == -1) {
		tmpShape = pImage_p->shape().asVector();
		tmpShape2.resize(tmpShape.size());
		if (dropDegenerateAxes) {
			int j = 0;
			for (uInt i = 0; i < tmpShape.size(); i++) {
				if (tmpShape[i] != 1) {
					tmpShape2[j] = tmpShape[i];
					j++;
				}
			}
			tmpShape2.resize(j);
			tmpShape = tmpShape2;
		}
	} else {
		tmpShape = inshape;
	}

	std::auto_ptr<CoordinateSystem> csys(
		coordinates.nfields() > 0
		? makeCoordinateSystem(coordinates, IPosition(tmpShape))
		: new CoordinateSystem()
	);

	Bool regridByVel = False;
	if (
		specAsVelocity && pImage_p->coordinates().hasSpectralAxis()
		&& csys->hasSpectralAxis()
	) {
		if (inaxes.size() == 0) {
			regridByVel = True;
		}
		else {
			Int specAxis = pImage_p->coordinates().spectralAxisNumber();
			for (uInt i=0; i<inaxes.size(); i++) {
				if (inaxes[i] == specAxis) {
					regridByVel = True;
					break;
				}
			}
		}
	}
	if (regridByVel) {
		return _regridByVelocity(
			outFile, inshape, *csys, inaxes,
			Region, mask, methodU, decimate,
			replicate, doRefChange, dropDegenerateAxes,
			overwrite, forceRegrid, extendMask
		);
	}
	else {
		return _regrid(
			outFile, inshape, *csys, inaxes, Region, mask, methodU,
			decimate, replicate, doRefChange, dropDegenerateAxes, overwrite,
			forceRegrid, extendMask
		);
	}
}

ImageInterface<Float>* ImageAnalysis::_regridByVelocity(
	const String& outfile, const Vector<Int>& shape,
	const CoordinateSystem& csysTemplate, const Vector<Int>& axes,
    Record& region, const String& mask,
    const String& method, const Int decimate,
    const Bool replicate, const Bool doref,
    const Bool dropdeg, const Bool overwrite,
    const Bool force, const Bool extendMask
) const {
	std::auto_ptr<CoordinateSystem> csys(
		dynamic_cast<CoordinateSystem *>(csysTemplate.clone())
	);
	SubImage<Float> maskedClone = SubImage<Float>::createSubImage(
		*pImage_p, Record(), mask, 0, False,
		AxesSpecifier(), extendMask
	);
	std::auto_ptr<CoordinateSystem> coordClone(
		dynamic_cast<CoordinateSystem *>(maskedClone.coordinates().clone())
	);
	const SpectralCoordinate saveSpecCoord = csys->spectralCoordinate();

	for (uInt i=0; i<2; i++) {
		CoordinateSystem *cs = i == 0 ? csys.get() : coordClone.get();
		// create and replace the coordinate system's spectral coordinate with
		// a linear coordinate which describes the velocity axis. In this way
		// we can regrid by velocity.
		Int specCoordNum = cs->spectralCoordinateNumber();
		SpectralCoordinate specCoord = cs->spectralCoordinate();
		Double freqRefVal = specCoord.referenceValue()[0];
		Double velRefVal;
		if (! specCoord.frequencyToVelocity(velRefVal, freqRefVal)) {
			*itsLog << LogIO::SEVERE << "Unable to determine reference velocity";
		}
		Double vel0, vel1;
		if (
			! specCoord.pixelToVelocity(vel0, 0.0)
			|| ! specCoord.pixelToVelocity(vel1, 1.0)
		) {
			*itsLog << LogIO::SEVERE << "Unable to determine velocity increment";
		}
		Matrix<Double> pc(1, 1, 0);
		pc.diagonal() = 1.0;
		LinearCoordinate lin(
			Vector<String>(1, "velocity"),
			specCoord.worldAxisUnits(),
			Vector<Double>(1, velRefVal),
			Vector<Double>(1, vel1 - vel0),
			pc, specCoord.referencePixel()
		);
		if (! cs->replaceCoordinate(lin, specCoordNum)) {
			*itsLog << "Unable to replace spectral with linear coordinate";
		}
	}
	maskedClone.setCoordinateInfo(*coordClone);
	ImageAnalysis newIA(&maskedClone);
	// do not pass the mask info in, the subimage is already masked
	std::auto_ptr<ImageInterface<Float> > outImage(
		newIA._regrid(
			outfile, shape, *csys, axes, region, "", method,
			decimate, replicate, doref, dropdeg, overwrite,
			force, False
		)
	);
	// replace the temporary linear coordinate with the saved spectral coordinate
	std::auto_ptr<CoordinateSystem> newCoords(
		dynamic_cast<CoordinateSystem *>(outImage->coordinates().clone())
	);
	if (
		! newCoords->replaceCoordinate(
			saveSpecCoord,
			maskedClone.coordinates().linearCoordinateNumber()
		)
	) {
		*itsLog << LogIO::SEVERE
			<< "Unable to replace coordinate for velocity regridding";
	}
	outImage->setCoordinateInfo(*newCoords);
	return outImage.release();
}

ImageInterface<Float>* ImageAnalysis::rotate(
	const String& outFile, const Vector<Int>& shape,
	const Quantity& pa, Record& Region, const String& mask,
	const String& methodU, const Int decimate,
	const Bool replicate, const Bool dropdeg,
	const Bool overwrite, const Bool extendMask
) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	Int dbg = 0;

	// Validate outfile
	if (!overwrite && !outFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	Vector<Int> tmpShape;
	Vector<Int> tmpShape2;
	if (shape.size() == 1 && shape[0] == -1) {
		tmpShape = pImage_p->shape().asVector();
		tmpShape2.resize(tmpShape.size());
		if (dropdeg) {
			int j = 0;
			for (uInt i = 0; i < tmpShape.size(); i++) {
				if (tmpShape[i] != 1) {
					tmpShape2[j] = tmpShape[i];
					j++;
				}
			}
			tmpShape2.resize(j);
			tmpShape = tmpShape2;
		}
	}
	else {
		tmpShape = shape;
	}

	//
	// Only handles Direction or Linear coordinate
	//
	String method2 = methodU;
	method2.upcase();

	// Convert region from Glish record to ImageRegion. Convert mask
	// to ImageRegion and make SubImage.
	AxesSpecifier axesSpecifier;
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&Region)),
		mask, itsLog, False, axesSpecifier, extendMask
	);

	// Get image coordinate system
	CoordinateSystem cSysFrom = subImage.coordinates();
	CoordinateSystem cSysTo = cSysFrom;

	// We automatically find a DirectionCoordinate or LInearCoordinate
	// These must hold *only* 2 axes at this point (restriction in ImageRegrid)
	Int after = -1;
	Int dirInd = -1;
	Int linInd = -1;
	uInt coordInd = 0;
	Vector<Int> pixelAxes;

	dirInd = cSysTo.findCoordinate(Coordinate::DIRECTION, after);
	if (dirInd < 0) {
		after = -1;
		linInd = cSysTo.findCoordinate(Coordinate::LINEAR, after);
		if (linInd >= 0) {
			pixelAxes = cSysTo.pixelAxes(linInd);
			coordInd = linInd;
			*itsLog << "Rotating LinearCoordinate holding axes " << pixelAxes
					+ 1 << LogIO::POST;
		}
	}
	else {
		pixelAxes = cSysTo.pixelAxes(dirInd);
		coordInd = dirInd;
		*itsLog << "Rotating DirectionCoordinate holding axes " << pixelAxes
				+ 1 << LogIO::POST;
	}

	if (pixelAxes.nelements() == 0) {
		*itsLog << "Could not find a Direction or Linear coordinate to rotate"
				<< LogIO::EXCEPTION;
	}
	else if (pixelAxes.nelements() != 2) {
		*itsLog << "Coordinate to rotate must hold exactly two axes"
				<< LogIO::EXCEPTION;
	}

	// Get Linear Transform
	const Coordinate& coord = cSysTo.coordinate(coordInd);
	Matrix<Double> xf = coord.linearTransform();

	// Generate rotation matrix components
	Double angleRad = pa.getValue(Unit("rad"));
	Matrix<Double> rotm(2, 2);
	Double s = sin(-angleRad);
	Double c = cos(-angleRad);
	rotm(0, 0) = c;
	rotm(0, 1) = s;
	rotm(1, 0) = -s;
	rotm(1, 1) = c;

	// Create new linear transform matrix
	Matrix<Double> xform(2, 2);
	xform(0, 0) = rotm(0, 0) * xf(0, 0) + rotm(0, 1) * xf(1, 0);
	xform(0, 1) = rotm(0, 0) * xf(0, 1) + rotm(0, 1) * xf(1, 1);
	xform(1, 0) = rotm(1, 0) * xf(0, 0) + rotm(1, 1) * xf(1, 0);
	xform(1, 1) = rotm(1, 0) * xf(0, 1) + rotm(1, 1) * xf(1, 1);

	// Apply new linear transform matrix to coordinate
	if (cSysTo.type(coordInd) == Coordinate::DIRECTION) {
		DirectionCoordinate c = cSysTo.directionCoordinate(coordInd);
		c.setLinearTransform(xform);
		cSysTo.replaceCoordinate(c, coordInd);
	}
	else {
		LinearCoordinate c = cSysTo.linearCoordinate(coordInd);
		c.setLinearTransform(xform);
		cSysTo.replaceCoordinate(c, coordInd);
	}

	// Determine axes to regrid to new coordinate system
	IPosition axes2(pixelAxes);
	IPosition outShape(tmpShape);

	// Now build a CS which copies the user specified Coordinate for
	// axes to be regridded and the input image Coordinate for axes
	// not to be regridded
	CoordinateSystem cSys = ImageRegrid<Float>::makeCoordinateSystem(*itsLog,
			cSysTo, cSysFrom, axes2);
	if (cSys.nPixelAxes() != outShape.nelements()) {
		*itsLog
				<< "The number of pixel axes in the output shape and Coordinate System must be the same"
				<< LogIO::EXCEPTION;
	}

	// Create the image and mask
	PtrHolder<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*itsLog << LogIO::NORMAL << "Creating (temp)image of shape "
				<< outShape << LogIO::POST;
		imOut.set(new TempImage<Float> (outShape, cSys));
	}
	else {
		*itsLog << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << outShape << LogIO::POST;
		imOut.set(new PagedImage<Float> (outShape, cSys, outFile));
	}
	ImageInterface<Float>* pImOut = imOut.ptr()->cloneII();
	pImOut->set(0.0);
	ImageUtilities::copyMiscellaneous(*pImOut, subImage);
	String maskName("");
	makeMask(*pImOut, maskName, True, True, *itsLog, True);
	//
	Interpolate2D::Method method = Interpolate2D::stringToMethod(methodU);
	IPosition dummy;
	ImageRegrid<Float> ir;
	ir.showDebugInfo(dbg);
	Bool forceRegrid = False;
	ir.regrid(
		*pImOut, method, axes2, subImage, replicate,
		decimate, True, forceRegrid
	);

	// Return image
	return pImOut;

}

Bool ImageAnalysis::rename(const String& name, const Bool overwrite) {

	*itsLog << LogOrigin("ImageAnalysis", "rename");

	if (!ispersistent()) {
		*itsLog << LogIO::WARN
				<< "This image tool is not associated with a persistent disk file. It cannot be renamed"
				<< LogIO::POST;
		return False;
	}
	if (name.size() == 0) {
		*itsLog << LogIO::WARN << "Empty name" << LogIO::POST;
		return False;
	}

	Bool strippath(False);
	String oldName = this->name(strippath);
	if (oldName.size() == 0) {
		return False;
	}

	// Make sure we don't rename ourselves to ourselves
	if (oldName == name) {
		*itsLog << LogIO::WARN
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
		*itsLog << LogIO::WARN
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
			*itsLog << LogIO::WARN << "There is already a file with the name "
					<< name << LogIO::POST;
			return False;
		}
	}

	// OK we passed the tests.  Close deletes temporary persistent image
	if (pImage_p != 0) {
		*itsLog << LogIO::NORMAL << "Detaching from image" << LogIO::POST;
		delete pImage_p;
	}
	pImage_p = 0;
	deleteHistAndStats();

	// Now try and move it
	Bool follow(True);
	if (file.isRegular(follow)) {
		RegularFile(file).move(name, overwrite);
	} else if (file.isDirectory(follow)) {
		Directory(file).move(name, overwrite);
	} else if (file.isSymLink()) {
		SymLink(file).copy(name, overwrite);
	} else {
		*itsLog << LogIO::POST << "Failed to rename file " << oldName << " to "
				<< name << LogIO::POST;
		return False;
	}

	*itsLog << LogIO::NORMAL << "Successfully renamed file " << oldName
			<< " to " << name << LogIO::POST;

	// Reopen ourprivate with the new file
	if (!open(name)) {
		*itsLog << LogIO::WARN << "Failed to open renamed file" << LogIO::POST;
	}

	return True;

}

Bool ImageAnalysis::replacemaskedpixels(
	const String& pixels, Record& pRegion,
	const String& maskRegion, const Bool updateMask,
	const Bool list, const Bool extendMask
) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);
	if (pixels.empty()) {
		*itsLog << "You must specify an expression" << LogIO::EXCEPTION
				<< LogIO::POST;
	}
	// Whine about no mask if appropriate.
	if (maskRegion.empty() && !pImage_p->isMasked()) {
		*itsLog << "This image does not have a mask - no action taken"
				<< LogIO::WARN << LogIO::POST;
		return False;
	}
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&pRegion)),
		maskRegion, (list ? itsLog : 0), True,
		AxesSpecifier(), extendMask
	);

	// See if we can write to ourselves
	if (!subImage.isWritable()) {
		*itsLog
				<< "This image is not writable.  It is probably a reference or expression virtual image"
				<< LogIO::EXCEPTION;
	}

	// Get LatticeExprNode (tree) from parser.
	// Convert the GlishRecord containing regions to a
	// PtrBlock<const ImageRegion*>.
	Block<LatticeExprNode> temps;
	String newexpr = pixels;
	//  String newexpr = substituteOID (temps, exprName, pixels);
	Record tempRegions;
	PtrBlock<const ImageRegion*> tempRegs;
	makeRegionBlock(tempRegs, tempRegions, *itsLog);
	LatticeExprNode node = ImageExprParse::command(newexpr, temps, tempRegs);
	// Delete the ImageRegions (by using an empty GlishRecord).
	makeRegionBlock(tempRegs, Record(), *itsLog);
	// Create the LEL expression we need.  It's like  replace(lattice, pixels)
	// where pixels is an expression itself.
	LatticeExprNode node2 = replace(subImage, node);
	// Do it
	subImage.copyData(LatticeExpr<Float> (node2));

	// Update the mask if desired
	if (updateMask) {
		Lattice<Bool>& mask = subImage.pixelMask();
		LatticeExprNode node(iif(!mask, True, mask));
		LatticeExpr<Bool> expr(node);
		mask.copyData(expr);
	}
	// Ensure that we reconstruct the statistics and histograms objects
	// now that the data/mask have changed
	deleteHistAndStats();

	return True;
}

Record ImageAnalysis::restoringbeam() {
	Record rstat;
	*itsLog << LogOrigin("ImageAnalysis", "restoringbeam");

	ImageInfo info = pImage_p->imageInfo();
	Record iRec;
	String error;
	if (!info.toRecord(error, iRec)) {
		*itsLog << LogIO::SEVERE
				<< "Failed to convert ImageInfo to a record because " << error
				<< LogIO::POST;
		return rstat;
	} else {
		if (iRec.isDefined("restoringbeam")) {
			rstat = iRec.asRecord("restoringbeam");
		}
	}
	return rstat;
}

ImageInterface<Float>* ImageAnalysis::sepconvolve(
	const String& outFile, const Vector<Int>& smoothaxes,
	const Vector<String>& kernels,
	const Vector<Quantity>& kernelwidths, Double scale, Record& pRegion,
	const String& mask, const Bool overwrite, const Bool extendMask
) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	Bool autoScale(False);
	if (scale < 0) {
		autoScale = True;
		scale = 1.0;
	}

	// Checks
	if (smoothaxes.nelements() == 0) {
		*itsLog << "You have not specified any axes to convolve"
				<< LogIO::EXCEPTION;
	}
	if (smoothaxes.nelements() != kernels.nelements() || smoothaxes.nelements()
			!= kernelwidths.nelements()) {
		*itsLog << "You must give the same number of axes, kernels and widths"
				<< LogIO::EXCEPTION;
	}

	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&pRegion)),
		mask, itsLog, False, AxesSpecifier(), extendMask
	);

	// Create convolver
	SepImageConvolver<Float> sic(subImage, *itsLog, True);

	// Handle inputs.
	Bool useImageShapeExactly = False;
	Vector<Int> smoothaxes2(smoothaxes);
	for (uInt i = 0; i < smoothaxes2.nelements(); i++) {
		VectorKernel::KernelTypes type = VectorKernel::toKernelType(kernels(i));
		sic.setKernel(uInt(smoothaxes2(i)), type, kernelwidths(i), autoScale,
				useImageShapeExactly, scale);
		*itsLog << LogIO::NORMAL << "Axis " << smoothaxes2(i)
				<< " : kernel shape = " << sic.getKernelShape(uInt(smoothaxes2(
				i))) << LogIO::POST;
	}

	// Make output image  - leave it until now incase there are
	// errors in VectorKernel
	PtrHolder<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*itsLog << LogIO::NORMAL << "Creating (temp)image of shape "
				<< subImage.shape() << LogIO::POST;
		imOut.set(new TempImage<Float> (subImage.shape(),
				subImage.coordinates()));
	}
	else {
		if (!overwrite) {
			NewFile validfile;
			String errmsg;
			if (!validfile.valueOK(outFile, errmsg)) {
				*itsLog << errmsg << LogIO::EXCEPTION;
			}
		}
		*itsLog << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << subImage.shape() << LogIO::POST;
		imOut.set(
			new PagedImage<Float> (
				subImage.shape(),
				subImage.coordinates(), outFile
			)
		);
	}
	ImageInterface<Float>* pImOut = imOut.ptr()->cloneII();
	ImageUtilities::copyMiscellaneous(*pImOut, *pImage_p);

	sic.convolve(*pImOut);

	return pImOut;
}

Bool ImageAnalysis::set(const String& lespixels, const Int pixelmask,
		Record& p_Region, const Bool list) {

	*itsLog << LogOrigin("ImageAnalysis", "set");
	String pixels(lespixels);
	Bool setPixels(True);
	if (pixels.length() == 0) {
		setPixels = False;
		pixels = "0.0";
	}
	Bool setMask(False);
	Bool mask(True);
	if (pixelmask != -1) {
		setMask = True;
		mask = (pixelmask > 0 ? True : False);
	}
	Record tempRegions;

	if (!setPixels && !setMask) {
		*itsLog << LogIO::WARN << "Nothing to do" << LogIO::POST;
		return False;
	}

	// Try and make a mask if we need one.
	if (setMask && !pImage_p->isMasked()) {
		String maskName("");
		makeMask(*pImage_p, maskName, True, True, *itsLog, list);
	}

	// Make region and subimage
	Record *tmpRegion = new Record(p_Region);
	const ImageRegion* pRegion = ImageRegion::fromRecord(
		(list ? itsLog : 0), pImage_p->coordinates(), pImage_p->shape(),
		*(ImageRegion::tweakedRegionRecord(tmpRegion))
	);
	delete tmpRegion;
	SubImage<Float> subImage(*pImage_p, *pRegion, True);

	// Set the pixels
	if (setPixels) {
		// Get LatticeExprNode (tree) from parser
		// Convert the GlishRecord containing regions to a
		// PtrBlock<const ImageRegion*>.
		if (pixels.empty()) {
			*itsLog << "You must specify an expression" << LogIO::EXCEPTION;
		}
		Block<LatticeExprNode> temps;
		String exprName;
		//String newexpr = substituteOID (temps, exprName, pixels);
		String newexpr = pixels;
		PtrBlock<const ImageRegion*> tempRegs;
		makeRegionBlock(tempRegs, tempRegions, *itsLog);
		LatticeExprNode node =
				ImageExprParse::command(newexpr, temps, tempRegs);
		// Delete the ImageRegions (by using an empty GlishRecord).
		makeRegionBlock(tempRegs, Record(), *itsLog);
		// We must have a scalar expression
		if (!node.isScalar()) {
			*itsLog << "The pixels expression must be scalar"
					<< LogIO::EXCEPTION;
		}
		if (node.isInvalidScalar()) {
			*itsLog << "The scalar pixels expression is invalid"
					<< LogIO::EXCEPTION;
		}
		LatticeExprNode node2 = toFloat(node);
		// if region==T (good) set value given by pixel expression, else
		// leave the pixels as they are
		LatticeRegion region = subImage.region();
		LatticeExprNode node3(iif(region, node2.getFloat(), subImage));
		subImage.copyData(LatticeExpr<Float> (node3));
	}
	// Set the mask
	if (setMask) {
		Lattice<Bool>& pixelMask = subImage.pixelMask();
		LatticeRegion region = subImage.region();
		// if region==T (good) set value given by "mask", else
		// leave the pixelMask as it is
		LatticeExprNode node4(iif(region, mask, pixelMask));
		pixelMask.copyData(LatticeExpr<Bool> (node4));
	}
	// Ensure that we reconstruct the statistics and histograms objects
	// now that the data/mask have changed

	deleteHistAndStats();

	return True;
}

Bool ImageAnalysis::setbrightnessunit(const String& unit) {

	*itsLog << LogOrigin("ImageAnalysis", "setbrightnessunit");

	return pImage_p->setUnits(Unit(unit));
}

Bool ImageAnalysis::setcoordsys(const Record& coordinates) {
	*itsLog << LogOrigin("ImageAnalysis", "setcoordsys");
	if (coordinates.nfields() == 0) {
		*itsLog << "CoordinateSystem is empty" << LogIO::EXCEPTION;
		return False;
	}
	PtrHolder<CoordinateSystem> cSys(makeCoordinateSystem(coordinates,
			pImage_p->shape()));
	Bool ok = pImage_p->setCoordinateInfo(*(cSys.ptr()));
	if (!ok) {
		*itsLog << "Failed to set CoordinateSystem" << LogIO::EXCEPTION;
	}
	return ok;
}

Bool ImageAnalysis::sethistory(const String& origin,
		const Vector<String>& History) {
	LogOrigin lor;
	if (origin.empty()) {
		lor = LogOrigin("ImageAnalysis", "sethistory");
	} else {
		lor = LogOrigin(origin);
	}
	*itsLog << lor << LogIO::POST;

	LoggerHolder& log = pImage_p->logger();
	// 
	// Make sure we can write into the history table if needed
	//
	log.reopenRW();
	LogSink& sink = log.sink();
	for (uInt i = 0; i < History.nelements(); i++) {
		if (History(i).length() > 0) {
			LogMessage msg(History(i), lor);
            sink.postLocally(msg);
		}
	}
	return True;
}

Bool ImageAnalysis::setmiscinfo(const Record& info) {
	*itsLog << LogOrigin("ImageAnalysis", "setmiscinfo");

	return pImage_p->setMiscInfo(info);
}

Vector<Int> ImageAnalysis::shape() {

	return pImage_p->shape().asVector();
}

Bool ImageAnalysis::setrestoringbeam(const Quantity& major,
		const Quantity& minor, const Quantity& pa, const Record& rec,
		const bool deleteIt, const bool log) {
	*itsLog << LogOrigin("ImageAnalysis", "setrestoringbeam");

	// It is  either  delete and ignore everthing else
	// or put in the info from record
	// else the 3 quantities.
	ImageInfo ii = pImage_p->imageInfo();
	if (deleteIt) {
		if (log) {
			*itsLog << LogIO::NORMAL << "Deleting restoring beam"
					<< LogIO::POST;
		}
		ii.removeRestoringBeam();
	} else if (rec.nfields() != 0) {
		String error;
		Record rec2;
		rec2.defineRecord("restoringbeam", rec);
		ii.setRestoringBeam(rec2);
	} else { //NEED WAY TO SPECIFY UNSET PA
		if (major.getValue() != 0 && minor.getValue() != 0) {
			ii.setRestoringBeam(major, minor, pa);
		} else {
			Vector<Quantum<Double> > b = ii.restoringBeam();
			if (b.nelements() == 3) {
				Quantity bmajor, bminor, bpa;
				if (major.getValue() != 0) {
					bmajor = major;
				} else {
					bmajor = b(0);
				}
				if (minor.getValue() != 0) {
					bminor = minor;
				} else {
					bminor = b(1);
				}
				if (pa.getValue() != 0) {
					bpa = pa;
				} else {
					bpa = b(2);
				}
				ii.setRestoringBeam(bmajor, bminor, bpa);
			} else {
				*itsLog
						<< "This image does not have a restoring beam that can be "
						<< "used to set missing input parameters"
						<< LogIO::POST;
			}
		}
	}
	if (log) {
		Vector<Quantum<Double> > b = ii.restoringBeam();
		if (b.nelements() == 3) {
			*itsLog << LogIO::NORMAL << "Set restoring beam" << endl;
			{
				ostringstream oss;
				oss << b(0);
				*itsLog << "  Major          : " << String(oss) << endl;
			}
			{
				ostringstream oss;
				oss << b(1);
				*itsLog << "  Minor          : " << String(oss) << endl;
			}
			{
				ostringstream oss;
				oss << b(2);
				*itsLog << "  Position Angle : " << String(oss) << endl;
			}
			itsLog->post();
		}
	}
	pImage_p->setImageInfo(ii);
	return True;
}

Bool ImageAnalysis::statistics(
	Record& statsout, const Vector<Int>& axes,
	Record& regionRec, const String& mask, const Vector<String>& plotstats,
	const Vector<Float>& includepix, const Vector<Float>& excludepix,
	const String&, const Int nx, const Int ny, const Bool list,
	const Bool force, const Bool disk, const Bool robust,
	const Bool verbose
) {
	String pgdevice("/NULL");
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	ImageRegion* pRegionRegion = 0;
	ImageRegion* pMaskRegion = 0;
	String mtmp = mask;
	if (mtmp == "false" || mtmp == "[]")
		mtmp = "";
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		pRegionRegion, pMaskRegion, *pImage_p,
		*(ImageRegion::tweakedRegionRecord(&regionRec)),
		mtmp,  (verbose ? itsLog : 0), False
	);
	{
        /*
		// block for debug only
		//cout << "mask " << subImage.getMask() << endl;
		Array<Bool> mymask = subImage.getMask();
		IPosition shape = mymask.shape();
		IPosition index = shape-1;
		uInt j=0;
		while(True) {
			for (uInt i=0; i<shape(0); i++) {
				index[0] = i;
				cout << mymask(index) << " ";
			}
			cout << index << endl;
			for (j=1; j<shape.size(); j++) {
				if (index[j] == 0) {
					index[j] = shape[j]-1;
					cout << endl;
				}
			else {
				index[j]--;
				break;
			}

			}
			if (j == shape.size()) {
				break;
			}
		}
		cout << "pixel mask " << endl;
		mymask = subImage.pixelMask().get();
		shape = mymask.shape();
		index = shape-1;
		j=0;
		while(True) {
			for (uInt i=0; i<shape(0); i++) {
				index[0] = i;
				cout << mymask(index) << " ";
			}
			cout << index << endl;
			for (j=1; j<shape.size(); j++) {
				if (index[j] == 0) {
					index[j] = shape[j]-1;
					cout << endl;
				}
			else {
				index[j]--;
				break;
			}

			}
			if (j == shape.size()) {
				break;
			}
		}
        */
	}
	// Reset who is logging stuff.
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	// Find BLC of subimage in pixels and world coords, and output the
	// information to the logger.
	// NOTE: ImageStatitics can't do this because it only gets the subimage
	//       not a region and the full image.
	IPosition blc(subImage.ndim(), 0);
	IPosition trc(subImage.shape() - 1);
	if (pRegionRegion != 0) {
		LatticeRegion latRegion = pRegionRegion->toLatticeRegion(
				pImage_p->coordinates(), pImage_p->shape());
		Slicer sl = latRegion.slicer();
		blc = sl.start();
		trc = sl.end();
	}
	// for precision
	CoordinateSystem cSys = pImage_p->coordinates();
	Bool hasDirectionCoordinate = (cSys.findCoordinate(Coordinate::DIRECTION) >= 0);
	Int precis = -1;
	if (hasDirectionCoordinate) {
		DirectionCoordinate dirCoord = cSys.directionCoordinate(0);
		Vector<String> dirUnits = dirCoord.worldAxisUnits();
		Vector<Double> dirIncs = dirCoord.increment();
		for (uInt i=0; i< dirUnits.size(); i++) {
			Quantity inc(dirIncs[i], dirUnits[i]);
			inc.convert("s");
			Int newPrecis = abs(int(floor(log10(inc.getValue()))));
			precis = (newPrecis > 2 && newPrecis > precis) ? newPrecis : precis;
		}
	}

	String blcf, trcf;
	blcf = CoordinateUtil::formatCoordinate(blc, cSys, precis);
	trcf = CoordinateUtil::formatCoordinate(trc, cSys, precis);

	if (list) {
		// Only write to the logger if the user wants it displayed.
		*itsLog << LogOrigin("ImageAnalysis", "statistics") << LogIO::NORMAL;
		*itsLog << "Regions --- " << LogIO::POST;
		*itsLog << "         -- bottom-left corner (pixel) [blc]:  " << blc
				<< LogIO::POST;
		*itsLog << "         -- top-right corner (pixel) [trc]:    " << trc
				<< LogIO::POST;
		*itsLog << "         -- bottom-left corner (world) [blcf]: " << blcf
				<< LogIO::POST;
		*itsLog << "         -- top-right corner (world) [trcf]:   " << trcf
				<< LogIO::POST;

	}

	// Make new statistics object only if we need to.    This code is getting
	// a bit silly. I should rework it somewhen.
	Bool forceNewStorage = force;
	if (pStatistics_p != 0) {
		if (disk != oldStatsStorageForce_p)
			forceNewStorage = True;
	}

	if (forceNewStorage) {
		delete pStatistics_p;
		pStatistics_p = 0;
		if (verbose) {
			pStatistics_p = new ImageStatistics<Float> (subImage, *itsLog,
					True, disk);
		} else {
			pStatistics_p = new ImageStatistics<Float> (subImage, True, disk);
		}
	} else {
		if (pStatistics_p == 0) {
			// We are here if this is the first time or the image has
			// changed (pStatistics_p is deleted then)

			if (verbose) {
				pStatistics_p = new ImageStatistics<Float> (subImage, *itsLog, False, disk);
			}
			else {
				pStatistics_p = new ImageStatistics<Float> (subImage, False, disk);
			}
		} else {
			// We already have a statistics object.  We only have to set
			// the new image (which will force the accumulation image
			// to be recomputed) if the region has changed.  If the image itself
			// changed, pStatistics_p will already have been set to 0

			Bool reMake = (verbose && !pStatistics_p->hasLogger()) || (!verbose
					&& pStatistics_p->hasLogger());
			if (reMake) {
				delete pStatistics_p;
				pStatistics_p = 0;
				if (verbose) {
					pStatistics_p = new ImageStatistics<Float> (subImage,
							*itsLog, True, disk);

				} else {
					pStatistics_p = new ImageStatistics<Float> (subImage, True,
							disk);
				}
			} else {
				pStatistics_p->resetError();
				if (haveRegionsChanged(pRegionRegion, pMaskRegion,
						pOldStatsRegionRegion_p, pOldStatsMaskRegion_p)) {
					pStatistics_p->setNewImage(subImage);
				}
			}
		}
	}

	pStatistics_p->setPrecision(precis);
	pStatistics_p->setBlc(blc);

	// Assign old regions to current regions
	delete pOldStatsRegionRegion_p;
	pOldStatsRegionRegion_p = 0;
	delete pOldStatsMaskRegion_p;
	pOldStatsMaskRegion_p = 0;

	pOldStatsRegionRegion_p = pRegionRegion;
	pOldStatsMaskRegion_p = pMaskRegion;
	oldStatsStorageForce_p = disk;

	// Set cursor axes
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);
	Vector<Int> tmpaxes(axes);
	if (!pStatistics_p->setAxes(tmpaxes)) {
		*itsLog << pStatistics_p->errorMessage() << LogIO::EXCEPTION;
	}

	// Set pixel include/exclude ranges
	//std::cerr << "include/exclude" << includepix.size() << " " << excludepix.size() << std::endl;
	if (!pStatistics_p->setInExCludeRange(includepix, excludepix, False)) {
		*itsLog << pStatistics_p->errorMessage() << LogIO::EXCEPTION;
	}

	// Tell what to list
	if (!pStatistics_p->setList(list)) {
		*itsLog << pStatistics_p->errorMessage() << LogIO::EXCEPTION;
	}

	// What to plot
	Vector<Int> statsToPlot = LatticeStatsBase::toStatisticTypes(plotstats);

	// Recover statistics
	Array<Double> npts, sum, sumsquared, min, max, mean, sigma;
	Array<Double> rms, fluxDensity, med, medAbsDevMed, quartile;
	Bool ok = True;

	Bool trobust(robust);
	if (!trobust) {
		for (uInt i = 0; i < statsToPlot.nelements(); i++) {
			if (statsToPlot(i) == Int(LatticeStatsBase::MEDIAN) || statsToPlot(
					i) == Int(LatticeStatsBase::MEDABSDEVMED) || statsToPlot(i)
					== Int(LatticeStatsBase::QUARTILE)) {
				trobust = True;
			}
		}
	}
	if (trobust) {
		ok = pStatistics_p->getStatistic(med, LatticeStatsBase::MEDIAN)
				&& pStatistics_p->getStatistic(medAbsDevMed,
						LatticeStatsBase::MEDABSDEVMED)
				&& pStatistics_p->getStatistic(quartile,
						LatticeStatsBase::QUARTILE);
	}
	if (ok) {
		ok = pStatistics_p->getStatistic(npts, LatticeStatsBase::NPTS)
				&& pStatistics_p->getStatistic(sum, LatticeStatsBase::SUM)
				&& pStatistics_p->getStatistic(sumsquared,
						LatticeStatsBase::SUMSQ)
				&& pStatistics_p->getStatistic(min, LatticeStatsBase::MIN)
				&& pStatistics_p->getStatistic(max, LatticeStatsBase::MAX)
				&& pStatistics_p->getStatistic(mean, LatticeStatsBase::MEAN)
				&& pStatistics_p->getStatistic(sigma, LatticeStatsBase::SIGMA)
				&& pStatistics_p->getStatistic(rms, LatticeStatsBase::RMS);
	}
	if (!ok) {
		*itsLog << pStatistics_p->errorMessage() << LogIO::EXCEPTION;
	}
	Bool ok2 = pStatistics_p->getStatistic(fluxDensity, LatticeStatsBase::FLUX);
	Record retval;
	retval.define(RecordFieldId("npts"), npts);
	retval.define(RecordFieldId("sum"), sum);
	retval.define(RecordFieldId("sumsq"), sumsquared);
	retval.define(RecordFieldId("min"), min);
	retval.define(RecordFieldId("max"), max);
	retval.define(RecordFieldId("mean"), mean);
	if (trobust) {
		retval.define(RecordFieldId("median"), med);
		retval.define(RecordFieldId("medabsdevmed"), medAbsDevMed);
		retval.define(RecordFieldId("quartile"), quartile);
	}
	retval.define(RecordFieldId("sigma"), sigma);
	retval.define(RecordFieldId("rms"), rms);
	if (ok2)
		retval.define(RecordFieldId("flux"), fluxDensity);

	retval.define(RecordFieldId("blc"), blc.asVector());
	retval.define(RecordFieldId("blcf"), blcf);

	retval.define(RecordFieldId("trc"), trc.asVector());
	retval.define(RecordFieldId("trcf"), trcf);

	String tmp;
	IPosition minPos, maxPos;
	if (pStatistics_p->getMinMaxPos(minPos, maxPos)) {
		if (minPos.nelements() > 0 && maxPos.nelements() > 0) {
			retval.define(RecordFieldId("minpos"), (blc + minPos).asVector());
			tmp = CoordinateUtil::formatCoordinate(blc + minPos, cSys, precis);
			retval.define(RecordFieldId("minposf"), tmp);
			retval.define(RecordFieldId("maxpos"), (blc + maxPos).asVector());
			tmp = CoordinateUtil::formatCoordinate(blc + maxPos, cSys, precis);
			retval.define(RecordFieldId("maxposf"), tmp);
		}
	}

	statsout = retval;

	// Make plots
	PGPlotter plotter;
	Vector<Int> nxy(2);
	if (!pgdevice.empty()) {
		//      try {
		plotter = PGPlotter(pgdevice);
		//      } catch (AipsError x) {
		//          *itsLog << LogIO::SEVERE << "Exception: " << x.getMesg() << LogIO::POST;
		//          return False;
		//      }
		nxy(0) = nx;
		nxy(1) = ny;
		if (nx < 0 || ny < 0) {
			nxy.resize(0);
		}

		if (!pStatistics_p->setPlotting(plotter, statsToPlot, nxy)) {
			*itsLog << pStatistics_p->errorMessage() << LogIO::EXCEPTION;
		}
	}

	if (list || !pgdevice.empty()) {
		pStatistics_p->showRobust(trobust);
		if (!pStatistics_p->display()) {
			*itsLog << pStatistics_p->errorMessage() << LogIO::EXCEPTION;
		}
	}
	pStatistics_p->closePlotting();

	return True;
}

Bool ImageAnalysis::twopointcorrelation(const String& outFile,
		Record& theRegion, const String& mask, const Vector<Int>& axes1,
		const String& method, const Bool overwrite) {

	*itsLog << LogOrigin("ImageAnalysis", "twopointcorrelation");

	// Validate outfile
	if (!overwrite && !outFile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	AxesSpecifier axesSpecifier;
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&theRegion)),
		mask, itsLog, False, axesSpecifier
	);

	// Deal with axes and shape
	Vector<Int> axes2(axes1);
	CoordinateSystem cSysIn = subImage.coordinates();
	IPosition axes = ImageTwoPtCorr<Float>::setUpAxes(IPosition(axes2), cSysIn);
	IPosition shapeOut = ImageTwoPtCorr<Float>::setUpShape(subImage.shape(),
			axes);

	// Create the output image and mask
	PtrHolder<ImageInterface<Float> > imOut;
	if (outFile.empty()) {
		*itsLog << LogIO::NORMAL << "Creating (temp)image of shape "
				<< shapeOut << LogIO::POST;
		imOut.set(new TempImage<Float> (shapeOut, cSysIn));
	} else {
		*itsLog << LogIO::NORMAL << "Creating image '" << outFile
				<< "' of shape " << shapeOut << LogIO::POST;
		imOut.set(new PagedImage<Float> (shapeOut, cSysIn, outFile));
	}
	ImageInterface<Float>* pImOut = imOut.ptr();
	String maskName("");
	makeMask(*pImOut, maskName, True, True, *itsLog, True);

	// Do the work.  The Miscellaneous items and units are dealt with
	// by function ImageTwoPtCorr::autoCorrelation
	ImageTwoPtCorr<Float> twoPt;
	Bool showProgress = True;
	LatticeTwoPtCorr<Float>::Method m = LatticeTwoPtCorr<Float>::fromString(
			method);
	twoPt.autoCorrelation(*pImOut, subImage, axes, m, showProgress);

	return True;
}

ImageInterface<Float> *
ImageAnalysis::subimage(const String& outfile, Record& Region,
		const String& mask, const Bool dropDegenerateAxes,
		const Bool overwrite, const Bool list, const Bool extendMask) {

	*itsLog << LogOrigin("ImageAnalysis", "subimage");
	// Copy a portion of the image
	// Verify output file
	if (!overwrite && !outfile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outfile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	AxesSpecifier axesSpecifier;
	if (dropDegenerateAxes) {
		axesSpecifier = AxesSpecifier(False);
	}
	std::auto_ptr<SubImage<Float> >subImage(
		new SubImage<Float>(
			SubImage<Float>::createSubImage(
				*pImage_p,
				*(ImageRegion::tweakedRegionRecord(&Region)),
				mask, itsLog, True, axesSpecifier, extendMask
			)
		)
	);

	if (outfile.empty()) {
		return subImage.release();
	} else {
		// Make the output image
		if (list) {
			*itsLog << LogIO::NORMAL << "Creating image '" << outfile
					<< "' of shape " << subImage->shape() << LogIO::POST;
		}
		std::auto_ptr<PagedImage<Float> > outImage(
			new PagedImage<Float> (
				subImage->shape(),
				subImage->coordinates(), outfile
			)
		);
		ImageUtilities::copyMiscellaneous(*outImage, *pImage_p);

		// Make output mask if required
		if (subImage->isMasked()) {
			String maskName("");
			makeMask(*outImage, maskName, False, True, *itsLog, list);
		}

		// Copy data and mask
		LatticeUtilities::copyDataAndMask(*itsLog, *outImage, *subImage);

		// Return handle
		return outImage.release();
	}
}

Vector<String> ImageAnalysis::summary(Record& header, const String& doppler,
		const Bool list, const Bool pixelorder) {

	*itsLog << LogOrigin("ImageAnalysis", "summary");

	Vector<String> messages;
	Record retval;
	ImageSummary<Float> s(*pImage_p);
	//
	MDoppler::Types velType;
	if (!MDoppler::getType(velType, doppler)) {
		*itsLog << LogIO::WARN << "Illegal velocity type, using RADIO"
				<< LogIO::POST;
		velType = MDoppler::RADIO;
	}
	//
	if (list) {
		messages = s.list(*itsLog, velType, False);
	} else {
		// Write messages to local sink only so we can fish them out again
		LogFilter filter;
		LogSink sink(filter, False);
		LogIO osl(sink);
		messages = s.list(osl, velType, True);
	}
	//
	Vector<String> axes = s.axisNames(pixelorder);
	Vector<Double> crpix = s.referencePixels(False); // 0-rel
	Vector<Double> crval = s.referenceValues(pixelorder);
	Vector<Double> cdelt = s.axisIncrements(pixelorder);
	Vector<String> axisunits = s.axisUnits(pixelorder);
	//
	retval.define(RecordFieldId("ndim"), Int(s.ndim()));
	retval.define(RecordFieldId("shape"), s.shape().asVector());
	retval.define(RecordFieldId("tileshape"), s.tileShape().asVector());
	retval.define(RecordFieldId("axisnames"), axes);
	retval.define(RecordFieldId("refpix"), crpix);
	retval.define(RecordFieldId("refval"), crval);
	retval.define(RecordFieldId("incr"), cdelt);
	retval.define(RecordFieldId("axisunits"), axisunits);
	retval.define(RecordFieldId("unit"), s.units().getName());
	retval.define(RecordFieldId("hasmask"), s.hasAMask());
	retval.define(RecordFieldId("defaultmask"), s.defaultMaskName());
	retval.define(RecordFieldId("masks"), s.maskNames());
	retval.define(RecordFieldId("imagetype"), s.imageType());
	//
	ImageInfo info = pImage_p->imageInfo();
	Record iRec;
	String error;
	if (!info.toRecord(error, iRec)) {
		*itsLog << LogIO::SEVERE
				<< "Failed to convert ImageInfo to a record because "
				<< LogIO::EXCEPTION;
		*itsLog << LogIO::SEVERE << error << LogIO::POST;
	} else {
		if (iRec.isDefined("restoringbeam")) {
			retval.defineRecord(RecordFieldId("restoringbeam"), iRec);
		}
	}
	header = retval;
	return messages;

}

Bool ImageAnalysis::tofits(const String& fitsfile, const Bool velocity,
			   const Bool optical, const Int bitpix, const Double minpix,
			   const Double maxpix, Record& pRegion, const String& mask,
			   const Bool overwrite, const Bool dropDeg, const Bool,
			   const Bool dropStokes, const Bool stokesLast, const Bool wavelength,
			   const Bool airWavelength, const String& origin) {

	*itsLog << LogOrigin("ImageAnalysis", "tofits");

	//
	// Convert image to FITS
	//
	String error;

	// Check output file
	if (!overwrite && !fitsfile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(fitsfile, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	// The SubImage that goes to the FITSCOnverter no longer will know
	// the name of the parent mask, so spit it out here
	if (pImage_p->isMasked()) {
		*itsLog << LogIO::NORMAL << "Applying mask of name '"
				<< pImage_p->getDefaultMask() << "'" << LogIO::POST;
	}

	IPosition keepAxes;
	if (!dropDeg) {
		if (dropStokes) {
			CoordinateSystem cSys = pImage_p->coordinates();
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
	} else if (!keepAxes.empty()) { // specify which axes to keep
		axesSpecifier = AxesSpecifier(keepAxes);
	}

	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*pImage_p,
		*(ImageRegion::tweakedRegionRecord(&pRegion)),
		mask, itsLog, False, axesSpecifier
	);

	Bool ok = ImageFITSConverter::ImageToFITS(error, subImage, fitsfile,
						  HostInfo::memoryFree() / 1024, 
						  velocity, optical, 
						  bitpix, minpix, maxpix, overwrite, 
						  False, //  deglast default
						  False, //  verbose default
						  stokesLast, 
						  wavelength,
						  airWavelength, // for airWavelength=True
						  origin
						  );
	if (!ok)
		*itsLog << error << LogIO::EXCEPTION;

	return ok;

}

Bool ImageAnalysis::toASCII(const String& outfile, Record& region,
		const String& mask, const String& sep, const String& format,
		const Double maskvalue, const Bool overwrite) {
	// sep is hard-wired as ' ' which is what imagefromascii expects
	*itsLog << LogOrigin("ImageAnalysis", "toASCII");

	String outFileStr(outfile);
	// Check output file name

	if (outFileStr.empty()) {
		Bool strippath(true);
		outFileStr = pImage_p->name(strippath);
		outFileStr = outFileStr + ".ascii";
	}

	if (!overwrite) { // quit with warning if file exists and overwrite=false
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outFileStr, errmsg)) {
			*itsLog << errmsg << LogIO::EXCEPTION;
		}
	}

	Path filePath(outFileStr);
	String fileName = filePath.expandedName();

	ofstream outFile(fileName.c_str());
	if (!outFile) {
		*itsLog << "Cannot open file " << outfile << LogIO::EXCEPTION;
	}

	Vector<Int> axes;
	Array<Float> pixels;
	Array<Bool> pixmask;
	getregion(pixels, pixmask, region, axes, mask, False, False, False);

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
	//
	return True;
}

Vector<Double> ImageAnalysis::topixel(Record&) {

	/*  std::vector<double> rstat;
	 *itsLog << LogOrigin("ImageAnalysis", "topixel");

	 CoordinateSystem cSys = pImage_p->coordinates();
	 ::casac::coordsys mycoords;
	 mycoords.setcoordsys(cSys);
	 rstat = mycoords.topixel(value);

	 */
	//getting bored now....
	//This need to be implemented when coordsys::topixel is
	//refactored into the casa
	// name space...right now this is sitting in the casac namespace
	*itsLog << LogOrigin("ImageAnalysis", "topixel");
	*itsLog << LogIO::EXCEPTION << "This function is not implemented "
			<< LogIO::POST;

	Vector<Double> leGarbageTotal;
	return leGarbageTotal;

}

Record ImageAnalysis::toworld(const Vector<Double>& value, const String& format) {

	*itsLog << LogOrigin("ImageAnalysis", "toworld");
	Record bla(toWorldRecord(value, format));
	return bla;

}

Bool ImageAnalysis::unlock() {
	*itsLog << LogOrigin("ImageAnalysis", "unlock");

	pImage_p->unlock();
	return True;
}

Bool ImageAnalysis::deleteHistAndStats() {
	Bool rstat = False;
	*itsLog << LogOrigin("ImageAnalysis", "deleteHistAndStats");
	try {
		if (pStatistics_p != 0) {
			delete pStatistics_p;
			pStatistics_p = 0;
		}
		if (pHistograms_p != 0) {
			delete pHistograms_p;
			pHistograms_p = 0;
		}
		if (pOldStatsRegionRegion_p != 0) {
			delete pOldStatsRegionRegion_p;
			pOldStatsRegionRegion_p = 0;
		}
		if (pOldStatsMaskRegion_p != 0) {
			delete pOldStatsMaskRegion_p;
			pOldStatsMaskRegion_p = 0;
		}
		if (pOldHistRegionRegion_p != 0) {
			delete pOldHistRegionRegion_p;
			pOldHistRegionRegion_p = 0;
		}
		if (pOldHistMaskRegion_p != 0) {
			delete pOldHistMaskRegion_p;
			pOldHistMaskRegion_p = 0;
		}
		rstat = True;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
	}
	return rstat;
}

Bool ImageAnalysis::makeMask(ImageInterface<Float>& out, String& maskName,
		Bool init, Bool makeDefault, LogIO& os, Bool list) {
	os << LogOrigin("ImageAnalysis", __FUNCTION__);
	if (out.canDefineRegion()) {

		// Generate mask name if not given
		if (maskName.empty())
			maskName = out.makeUniqueRegionName(String("mask"), 0);

		// Make the mask if it does not exist
		if (!out.hasRegion(maskName, RegionHandler::Masks)) {
			out.makeMask(maskName, True, makeDefault, init, True);
			if (list) {
				if (init) {
					os << LogIO::NORMAL << "Created and initialized mask `"
							<< maskName << "'" << LogIO::POST;
				} else {
					os << LogIO::NORMAL << "Created mask `" << maskName << "'"
							<< LogIO::POST;
				}
			}
		}
		return True;
	} else {
		os << LogIO::WARN
				<< "Cannot make requested mask for this type of image" << endl;
		return False;
	}
}

Bool ImageAnalysis::haveRegionsChanged(ImageRegion* pNewRegionRegion,
		ImageRegion* pNewMaskRegion, ImageRegion* pOldRegionRegion,
		ImageRegion* pOldMaskRegion) const {
	Bool regionChanged = (pNewRegionRegion != 0 && pOldRegionRegion != 0
			&& (*pNewRegionRegion) != (*pOldRegionRegion)) || (pNewRegionRegion
			== 0 && pOldRegionRegion != 0) || (pNewRegionRegion != 0
			&& pOldRegionRegion == 0);
	Bool maskChanged = (pNewMaskRegion != 0 && pOldMaskRegion != 0
			&& (*pNewMaskRegion) != (*pOldMaskRegion)) || (pNewMaskRegion == 0
			&& pOldMaskRegion != 0) || (pNewMaskRegion != 0 && pOldMaskRegion
			== 0);
	return (regionChanged || maskChanged);
}

void ImageAnalysis::makeRegionBlock(PtrBlock<const ImageRegion*>& regions,
		const Record& Regions, LogIO& ) {

	// Trying to mimick  a glishRegion by a Record of Records

	for (uInt j = 0; j < regions.nelements(); j++) {
		delete regions[j];
	}
	regions.resize(0, True, True);
	uInt nreg = Regions.nfields();
	if (nreg > 0) {
		regions.resize(nreg);
		regions.set(static_cast<ImageRegion*> (0));
		for (uInt i = 0; i < nreg; i++) {
			regions[i] = ImageRegion::fromRecord(Regions.asRecord(i), "");
		}
	}
}

Bool ImageAnalysis::make_image(String &error, const String& outfile,
		const CoordinateSystem& cSys, const IPosition& shape, LogIO& os,
		Bool log, Bool overwrite) {
	// Verify outfile
	if (!overwrite && !outfile.empty()) {
		NewFile validfile;
		String errmsg;
		if (!validfile.valueOK(outfile, errmsg)) {
			error = errmsg;
			return False;
		}
	}
	//
	error = "";
	if (pImage_p != 0) {
		delete pImage_p;
		pImage_p = 0;
	}

	// This function is generally only called for creating new images,
	// but you never know, so add statistic/histograms protection
	deleteHistAndStats();

	uInt ndim = shape.nelements();
	if (ndim != cSys.nPixelAxes()) {
		error = "Supplied CoordinateSystem and image shape are inconsistent";
		return False;
	}
	//
	if (outfile.empty()) {
		pImage_p = new TempImage<Float> (shape, cSys);
		if (pImage_p == 0) {
			error = "Failed to create TempImage";
			return False;
		}
		if (log) {
			os << LogIO::NORMAL << "Creating (temp)image of shape "
					<< pImage_p->shape() << LogIO::POST;
		}
	} else {
		pImage_p = new PagedImage<Float> (shape, cSys, outfile);
		if (pImage_p == 0) {
			error = "Failed to create PagedImage";
			return False;
		}
		if (log) {
			os << LogIO::NORMAL << "Creating image '" << outfile
					<< "' of shape " << pImage_p->shape() << LogIO::POST;
		}
	}
	return True;
}

Bool ImageAnalysis::makeExternalImage(PtrHolder<ImageInterface<Float> >& image,
		const String& fileName, const CoordinateSystem& cSys,
		const IPosition& shape, const ImageInterface<Float>& inImage,
		LogIO& os, Bool overwrite, Bool allowTemp, Bool copyMask)
{
	if (fileName.empty()) {
		if (allowTemp) {
			os << LogIO::NORMAL << "Creating (Temp)Image '" << " of shape "
					<< shape << LogIO::POST;
			image.set(new TempImage<Float> (shape, cSys));
		}
	} else {
		if (!overwrite) {
			NewFile validfile;
			String errmsg;
			if (!validfile.valueOK(fileName, errmsg)) {
				os << errmsg << LogIO::EXCEPTION;
			}
		}
		os << LogIO::NORMAL << "Creating image '" << fileName << "' of shape "
				<< shape << LogIO::POST;
		image.set(new PagedImage<Float> (shape, cSys, fileName));
	}

	// See if we made something
	ImageInterface<Float>* pIm = image.ptr();
	if (pIm) {
		ImageUtilities::copyMiscellaneous(*pIm, inImage);
		//
		if (copyMask && inImage.isMasked()) {
			String maskName("");
			makeMask(*pIm, maskName, False, True, os, True);
			Lattice<Bool>& pixelMaskOut = pIm->pixelMask();
			// The input image may be a subimage with a pixel mask and
			// a region mask, so use getMaskSlice to get its mask
			LatticeIterator<Bool> maskIter(pixelMaskOut);
			for (maskIter.reset(); !maskIter.atEnd(); maskIter++) {
				maskIter.rwCursor() = inImage.getMaskSlice(maskIter.position(),
						maskIter.cursorShape());
			}
		}
		return True;
	} else {
		return False;
	}
}

CoordinateSystem* ImageAnalysis::makeCoordinateSystem(
		const Record& coordinates, const IPosition& shape) const {
	*itsLog << LogOrigin("ImageAnalysis", "makeCoordinateSystem");
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
		*itsLog << LogOrigin("ImageAnalysis", "makeCoordinateSystem");
		*itsLog << LogIO::WARN << errMsg << LogIO::POST;
	}
	//
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

void ImageAnalysis::set_cache(const IPosition &chunk_shape) const {
	if (pImage_p == 0) {
		return;
	}
	//
	if (chunk_shape.nelements() != last_chunk_shape_p.nelements()
			|| chunk_shape != last_chunk_shape_p) {
		ImageAnalysis *This = (ImageAnalysis *) this;
		This->last_chunk_shape_p.resize(chunk_shape.nelements());
		This->last_chunk_shape_p = chunk_shape;

		// Assume that we will keep getting similar sized chunks filling up
		// the whole image.
		IPosition shape(pImage_p->shape());
		IPosition blc(shape.nelements());
		blc = 0;
		IPosition axisPath(shape.nelements());
		for (uInt i = 0; i < axisPath.nelements(); i++) {
			axisPath(i) = i;
		}
		pImage_p->setCacheSizeFromPath(chunk_shape, blc, shape, axisPath);
	}
}

// the public version of the function
Bool ImageAnalysis::deconvolveFromBeam(Quantity& majorFit, Quantity& minorFit,
		Quantity& paFit, Bool& successFit, const Vector<Quantity>& beam) {

	return ImageUtilities::deconvolveFromBeam(majorFit, minorFit, paFit, successFit, *itsLog, beam);

}

void ImageAnalysis::hanning_smooth(Array<Float>& out, Array<Bool>& maskOut,
		const Vector<Float>& in, const Array<Bool>& maskIn, Bool isMasked) const {
	const uInt nIn = in.nelements();
	const uInt nOut = out.nelements();
	Bool deleteOut, deleteIn, deleteMaskIn, deleteMaskOut;
	//
	const Float* pDataIn = in.getStorage(deleteIn);
	const Bool* pMaskIn = 0;
	if (isMasked) {
		pMaskIn = maskIn.getStorage(deleteMaskIn);
	}
	//
	Float* pDataOut = out.getStorage(deleteOut);
	Bool* pMaskOut = 0;
	if (isMasked) {
		pMaskOut = maskOut.getStorage(deleteMaskOut);
	}

	// Zero masked points.
	Float* pData = 0;
	if (isMasked) {
		pData = new Float[in.nelements()];
		for (uInt i = 0; i < nIn; i++) {
			pData[i] = pDataIn[i];
			if (!pMaskIn[i])
				pData[i] = 0.0;
		}
	} else {
		pData = (Float*) pDataIn;
	}

	// Smooth
	if (nIn != nOut) {
		// Dropping every other pixel.  First output pixel is centred
		// on the second input pixel.   We discard the last input pixel
		// if the input spectrum is of an even number of pixels
		if (isMasked) {
			Int j = 1;
			for (uInt i = 0; i < nOut; i++) {
				pDataOut[i] = 0.25 * (pData[j - 1] + pData[j + 1]) + 0.5
						* pData[j];
				pMaskOut[i] = pMaskIn[j];
				j += 2;
			}
		} else {
			Int j = 1;
			for (uInt i = 0; i < nOut; i++) {
				pDataOut[i] = 0.25 * (pData[j - 1] + pData[j + 1]) + 0.5
						* pData[j];
				j += 2;
			}
		}
	} else {
		// All pixels
		if (isMasked) {
			for (uInt i = 1; i < nIn - 1; i++) {
				pDataOut[i] = 0.25 * (pData[i - 1] + pData[i + 1]) + 0.5
						* pData[i];
				pMaskOut[i] = pMaskIn[i];
			}
			pMaskOut[0] = pMaskIn[0];
			pMaskOut[nIn - 1] = pMaskIn[nIn - 1];
		} else {
			for (uInt i = 1; i < nIn - 1; i++) {
				pDataOut[i] = 0.25 * (pData[i - 1] + pData[i + 1]) + 0.5
						* pData[i];
			}
		}
		pDataOut[0] = 0.5 * (pData[0] + pData[1]);
		pDataOut[nIn - 1] = 0.5 * (pData[nIn - 2] + pData[nIn - 1]);
	}
	//
	if (isMasked) {
		delete[] pData;
		maskOut.putStorage(pMaskOut, deleteMaskOut);
		maskIn.freeStorage(pMaskIn, deleteMaskIn);
	}
	in.freeStorage(pDataIn, deleteIn);
	out.putStorage(pDataOut, deleteOut);
}

Record ImageAnalysis::setregion(const Vector<Int>& blc, const Vector<Int>& trc,
		const String& infile) {
	Vector<Double> Blc(blc.size());
	Vector<Double> Trc(trc.size());
	for (uInt i = 0; i < blc.size(); i++)
		Blc[i] = blc[i];
	for (uInt i = 0; i < trc.size(); i++)
		Trc[i] = trc[i];
	return ImageAnalysis::setboxregion(Blc, Trc, false, infile);
}

Record ImageAnalysis::setboxregion(const Vector<Double>& blc, const Vector<
		Double>& trc, const Bool frac, const String& infile) {
	*itsLog << LogOrigin("ImageAnalysis", "setboxregion");

	// create Region
	Record rec;
	rec.define("isRegion", Int(RegionType::LC));
	rec.define("name", "LCBox");
	rec.define("comment", "");

	Vector<Int> latticeShapeVec;
	if (infile != "") { // get region shape from infile
		ImageInterface<Float>* pImage_p_tmp;
		ImageUtilities::openImage(pImage_p_tmp, infile, *itsLog);
		latticeShapeVec = pImage_p_tmp->shape().asVector();
	} else {
		latticeShapeVec = this->shape();
	}
	rec.define("shape", latticeShapeVec);

	uInt inBoxDim = blc.size();
	if (trc.size() != inBoxDim) {
		*itsLog << LogIO::WARN << "blc and trc must have the same dimensions"
				<< LogIO::POST;
		return rec;
	}

	uInt imgDim = latticeShapeVec.size();
	Vector<Float> itsBlc(imgDim);
	Vector<Float> itsTrc(imgDim);
	if (inBoxDim == 1 && blc[0] < 0) {
		for (unsigned long i = 0; i < imgDim; i++) {
			itsBlc(i) = 0;
			itsTrc(i) = latticeShapeVec[i];
		}
	} else if (inBoxDim <= imgDim) {
		for (unsigned long i = 0; i < inBoxDim; i++) {
			if (frac == true) {
				itsBlc(i) = blc[i] * latticeShapeVec[i];
				itsTrc(i) = trc[i] * latticeShapeVec[i];
			} else {
				itsBlc(i) = blc[i];
				itsTrc(i) = trc[i];
			}
		}
		for (unsigned long i = inBoxDim; i < imgDim; i++) {
			itsBlc(i) = 0;
			itsTrc(i) = latticeShapeVec[i];
		}
	} else {// inBoxDim > imgDim (ignore extra values)
		for (unsigned long i = 0; i < imgDim; i++) {
			if (frac == true) {
				itsBlc(i) = blc[i] * latticeShapeVec[i];
				itsTrc(i) = trc[i] * latticeShapeVec[i];
			} else {
				itsBlc(i) = blc[i];
				itsTrc(i) = trc[i];
			}
		}
	}

	// Write 0-relative.
	rec.define("oneRel", False);
	rec.define("blc", itsBlc);
	rec.define("trc", itsTrc);

	return rec;
}

bool ImageAnalysis::maketestimage(const String& outfile, const Bool overwrite,
		const String& imagetype) {
	bool rstat(false);
	*itsLog << LogOrigin("ImageAnalysis", "maketestimage");
	String var = EnvironmentVariable::get("CASAPATH");
	if (var.empty()) {
		var = EnvironmentVariable::get("AIPSPATH");
	}
		if (!var.empty()) {
			String fields[4];
			Int num = split(var, fields, 4, String(" "));
			String fitsfile;
			if (num >= 1) {
				if (imagetype.contains("cube"))
					fitsfile = fields[0] + "/data/demo/Images/test_image.fits";
				else if (imagetype.contains("2d"))
					fitsfile = fields[0]
							+ "/data/demo/Images/imagetestimage.fits";
				int whichrep(0);
				int whichhdu(0);
				Bool zeroblanks = False;
				rstat = ImageAnalysis::imagefromfits(outfile, fitsfile,
						whichrep, whichhdu, zeroblanks, overwrite);
			}
			else {
				*itsLog << LogIO::EXCEPTION << "Bad environment variable";
			}
		}
		else {
			*itsLog << LogIO::EXCEPTION << "Environment variable undefined, can't get data path";
		}

	return rstat;
}

ImageInterface<Float> *
ImageAnalysis::newimage(const String& infile, const String& outfile,
		Record& region, const String& Mask, const bool dropdeg,
		const bool overwrite) {
	ImageInterface<Float>* outImage = 0;
		*itsLog << LogOrigin("ImageAnalysis", "newimage");

		// Open
		PtrHolder<ImageInterface<Float> > inImage;
		ImageUtilities::openImage(inImage, infile, *itsLog);
		ImageInterface<Float>* pInImage = inImage.ptr();

		AxesSpecifier axesSpecifier;
		if (dropdeg)
			axesSpecifier = AxesSpecifier(False);
		SubImage<Float> subImage = SubImage<Float>::createSubImage(
			*pInImage,
			*(ImageRegion::tweakedRegionRecord(&region)),
			Mask, itsLog, True, axesSpecifier
		);

		// Create output image
		if (outfile.empty()) {
			outImage = new SubImage<Float> (subImage);
		} else {
			if (!overwrite) {
				NewFile validfile;
				String errmsg;
				if (!validfile.valueOK(outfile, errmsg)) {
					*itsLog << errmsg << LogIO::EXCEPTION;
				}
			}
			//
			*itsLog << LogIO::NORMAL << "Creating image '" << outfile
					<< "' of shape " << subImage.shape() << LogIO::POST;
			outImage = new PagedImage<Float> (subImage.shape(),
					subImage.coordinates(), outfile);
			if (outImage == 0) {
				*itsLog << "Failed to create PagedImage" << LogIO::EXCEPTION;
			}
			ImageUtilities::copyMiscellaneous(*outImage, *pInImage);

			// Make output mask if required
			if (subImage.isMasked()) {
				String maskName("");
				makeMask(*outImage, maskName, False, True, *itsLog, True);
			}

			// Copy data and mask
			LatticeUtilities::copyDataAndMask(*itsLog, *outImage, subImage);
		}

	return outImage;
}

ImageInterface<Float> *
ImageAnalysis::newimagefromfile(const String& fileName) {
	ImageInterface<Float>* outImage = 0;
	if (itsLog == 0)
		itsLog = new LogIO();

		*itsLog << LogOrigin("ImageAnalysis", "newimagefromfile");

		// Check whether infile exists
		if (fileName.empty()) {
			*itsLog << LogIO::WARN << "File string is empty" << LogIO::POST;
			return outImage;
		}
		File thefile(fileName);
		if (!thefile.exists()) {
			*itsLog << LogIO::WARN << "File " << fileName << " does not exist."
					<< LogIO::POST;
			return outImage;
		}

		// Open
		PtrHolder<ImageInterface<Float> > inImage;
		ImageUtilities::openImage(inImage, fileName, *itsLog);
		outImage = (inImage.ptr())->cloneII();
		if (outImage == 0) {
			*itsLog << "Failed to create image tool" << LogIO::EXCEPTION;
		}

	return outImage;
}

ImageInterface<Float> *
ImageAnalysis::newimagefromarray(const String& outfile,
		Array<Float> & pixelsArray, const Record& csys, const Bool linear,
		const Bool overwrite, const Bool log) {
	ImageInterface<Float>* outImage = 0;

	try {
		*itsLog << LogOrigin("ImageAnalysis", "newimagefromarray");

		// Verify outfile
		if (!overwrite && !outfile.empty()) {
			NewFile validfile;
			String errmsg;
			if (!validfile.valueOK(outfile, errmsg)) {
				*itsLog << LogIO::WARN << errmsg << LogIO::POST;
				return outImage;
			}
		}
		// Some protection
		if (pixelsArray.ndim() == 0) {
			*itsLog << "The pixels array is empty" << LogIO::EXCEPTION;
		}
		for (uInt i = 0; i < pixelsArray.ndim(); i++) {
			if (pixelsArray.shape()(i) <= 0) {
				*itsLog << "The shape of the pixels array is invalid"
						<< LogIO::EXCEPTION;
			}
		}

		CoordinateSystem cSys;
		if (csys.nfields() != 0) {
			// Make with supplied CoordinateSystem if record not empty
			PtrHolder<CoordinateSystem> pCS(makeCoordinateSystem(csys,
					pixelsArray.shape()));
			cSys = *(pCS.ptr());
		} else {
			// Make default CoordinateSystem
			cSys = CoordinateUtil::makeCoordinateSystem(pixelsArray.shape(),
					linear);
			centreRefPix(cSys, pixelsArray.shape());
		}

		uInt ndim = (pixelsArray.shape()).nelements();
		if (ndim != cSys.nPixelAxes()) {
			*itsLog << LogIO::SEVERE
					<< "Supplied CoordinateSystem and image shape are inconsistent"
					<< LogIO::POST;
			return outImage;
		}
		//

		if (outfile.empty()) {
			outImage = new TempImage<Float> (IPosition(pixelsArray.shape()),
					cSys);
			if (outImage == 0) {
				*itsLog << LogIO::SEVERE << "Failed to create TempImage"
						<< LogIO::POST;
				return outImage;
			}
			if (log) {
				*itsLog << LogIO::NORMAL << "Creating (temp)image of shape "
						<< outImage->shape() << LogIO::POST;
			}
		} else {
			outImage = new PagedImage<Float> (IPosition(pixelsArray.shape()),
					cSys, outfile);
			if (outImage == 0) {
				*itsLog << LogIO::SEVERE << "Failed to create PagedImage"
						<< LogIO::POST;
				return outImage;
			}
			if (log) {
				*itsLog << LogIO::NORMAL << "Creating image '" << outfile
						<< "' of shape " << outImage->shape() << LogIO::POST;
			}
		}

		// Fill image
		outImage->putSlice(pixelsArray, IPosition(pixelsArray.ndim(), 0),
				IPosition(pixelsArray.ndim(), 1));
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
	}
	return outImage;
}

ImageInterface<Float> *
ImageAnalysis::newimagefromshape(const String& outfile,
		const Vector<Int>& shapeV, const Record& coordinates,
		const Bool linear, const Bool overwrite, const Bool log) {
	ImageInterface<Float>* outImage = 0;

	try {
		*itsLog << LogOrigin("ImageAnalysis", "newimagefromshape");

		// Verify outfile
		if (!overwrite && !outfile.empty()) {
			NewFile validfile;
			String errmsg;
			if (!validfile.valueOK(outfile, errmsg)) {
				*itsLog << LogIO::WARN << errmsg << LogIO::POST;
				return outImage;
			}
		}
		// Some protection
		if (shapeV.nelements() == 0) {
			*itsLog << "The shape is invalid" << LogIO::EXCEPTION;
		}
		for (uInt i = 0; i < shapeV.nelements(); i++) {
			if (shapeV(i) <= 0) {
				*itsLog << "The shape is invalid" << LogIO::EXCEPTION;
			}
		}

		CoordinateSystem cSys;
		if (coordinates.nfields() > 0) {
			// Make with supplied CoordinateSystem if record not empty
			PtrHolder<CoordinateSystem> pCS(makeCoordinateSystem(coordinates,
					shapeV));
			cSys = *(pCS.ptr());
		} else {
			// Make default CoordinateSystem
			cSys = CoordinateUtil::makeCoordinateSystem(shapeV, linear);
			centreRefPix(cSys, shapeV);
		}

		uInt ndim = shapeV.nelements();
		if (ndim != cSys.nPixelAxes()) {
			*itsLog << LogIO::SEVERE
					<< "Supplied CoordinateSystem and image shape are inconsistent"
					<< LogIO::POST;
			return outImage;
		}
		//
		if (outfile.empty()) {
			outImage = new TempImage<Float> (IPosition(shapeV), cSys);
			if (outImage == 0) {
				*itsLog << LogIO::SEVERE << "Failed to create TempImage"
						<< LogIO::POST;
				return outImage;
			}
			if (log) {
				*itsLog << LogIO::NORMAL << "Creating (temp)image of shape "
						<< outImage->shape() << LogIO::POST;
			}
		} else {
			outImage = new PagedImage<Float> (IPosition(shapeV), cSys, outfile);
			if (outImage == 0) {
				*itsLog << LogIO::SEVERE << "Failed to create PagedImage"
						<< LogIO::POST;
				return outImage;
			}
			if (log) {
				*itsLog << LogIO::NORMAL << "Creating image '" << outfile
						<< "' of shape " << outImage->shape() << LogIO::POST;
			}
		}
		outImage->set(0.0);
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
	}
	return outImage;
}

ImageInterface<Float> *
ImageAnalysis::newimagefromfits(const String& outfile, const String& fitsfile,
		const Int whichrep, const Int whichhdu, const Bool zeroBlanks,
		const Bool overwrite) {
	ImageInterface<Float>* outImage = 0;
		*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

		// Check output file
		if (!overwrite && !outfile.empty()) {
			NewFile validfile;
			String errmsg;
			if (!validfile.valueOK(outfile, errmsg)) {
				*itsLog << errmsg << LogIO::EXCEPTION;
			}
		}
		//
		if (whichrep < 0) {
			*itsLog
					<< "The Coordinate Representation index must be non-negative"
					<< LogIO::EXCEPTION;
		}
		//
		ImageInterface<Float>* pOut = 0;
		String error;
		ImageFITSConverter::FITSToImage(pOut, error, outfile, fitsfile,
				whichrep, whichhdu, HostInfo::memoryFree() / 1024, overwrite,
				zeroBlanks);
		if (pOut == 0) {
			*itsLog << error << LogIO::EXCEPTION;
		}
		outImage = pOut->cloneII();
		delete pOut;
		if (outImage == 0) {
			*itsLog << "Failed to create image tool" << LogIO::EXCEPTION;
		}

	return outImage;
}


Record*
ImageAnalysis::echo(Record& v, const bool godeep) {

	Record *inrec = new Record(v);
	if (godeep) {
		// FIXME ummm why is this commented out? confusing...
		/*
		 String error("");
		 MeasureHolder mh;
		 if (! mh.fromRecord(error,*inrec)) {
		 *itsLog << LogIO::WARN << error
		 << "\nInput record cannot be stored in a MeasureHolder"
		 << LogIO::POST;
		 } else {
		 Record outRec;
		 if (! mh.toRecord(error, outRec) ) {
		 *itsLog << LogIO::WARN << error
		 << "\nOutput record cannot be generated from MeasureHolder"
		 << LogIO::POST;
		 } else {
		 retval=fromRecord(outRec);
		 }
		 }
		 */
	}
	return inrec;
}


Bool ImageAnalysis::getSpectralAxisVal(const String& specaxis,
		Vector<Float>& specVal, const CoordinateSystem& cs,
		const String& xunits, const String& specFrame, const String& restValue) {
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	CoordinateSystem cSys=cs;
	if(specFrame != ""){
		String errMsg;
		if(!CoordinateUtil::setSpectralConversion(errMsg, cSys, specFrame)){
			//cerr << "Failed to convert with error: " << errMsg << endl;
			*itsLog << LogIO::WARN << "Failed to convert with error: " << errMsg << LogIO::POST;
		}
	}
	if (restValue!=""){
		String errMsg;
	   Quantity restQuant;
	   Bool ok = Quantity::read(restQuant, restValue);
	   if (!ok) {
	   	errMsg = "Can not convert value to rest wavelength/frequency: " + restValue;
	      //os << errorMsg << LogIO::EXCEPTION;
	   	*itsLog << LogIO::WARN << errMsg << LogIO::POST;
	   }
	   else if (restQuant.getValue() > 0 && restQuant.getUnit().empty()){
	   	errMsg = "Can not retrieve unit for rest wavelength/frequency in: " + restValue;
	      //os << errorMsg << LogIO::EXCEPTION;
	   	*itsLog << LogIO::WARN << errMsg << LogIO::POST;
	   }
	   if (!CoordinateUtil::setRestFrequency (errMsg, cSys,
	   	                                    restQuant.getUnit(), restQuant.getValue())) {
	   	//os << errorMsg << LogIO::EXCEPTION;
	   	*itsLog << LogIO::WARN << errMsg << LogIO::POST;
	   }
	}

	Int specAx = cSys.findCoordinate(Coordinate::SPECTRAL);
	Vector<Double> pix(specVal.nelements());
	indgen(pix);
	SpectralCoordinate specCoor = cSys.spectralCoordinate(specAx);
	Vector<Double> xworld(pix.nelements());
	String axis = specaxis;
	axis.downcase();
	Bool ok = False;

	if (axis.contains("velo") || axis.contains("freq") || axis.contains("wave")) { // need a conversion

		// first convert from pixels to frequencies
		Vector<String> tmpstr(1);
		Vector<Double> fworld(pix.nelements());
		if (xunits == String("")) {
		       tmpstr[0] = String("GHz");
		} else if(axis.contains("freq")) {
		       tmpstr[0] = xunits;
		}
		specCoor.setWorldAxisUnits(tmpstr);
		ok = True;
		for (uInt k = 0; k < pix.nelements(); ++k) {
		       ok = ok && specCoor.toWorld(fworld[k], pix[k]);
		}

		// then, if necessary, from frequencies to velocity or wavelength
		if (ok && axis.contains("velo")) {
			ok = False;
			if (axis.contains("optical")) { // optical velocity
				specCoor.setVelocity(xunits, MDoppler::OPTICAL);
			} else if (axis.contains("radio")) { // radio velocity
				specCoor.setVelocity(xunits, MDoppler::RADIO);
			} else { // true relativistic velocity (default)
				specCoor.setVelocity(xunits, MDoppler::RELATIVISTIC);
			}
			ok = specCoor.frequencyToVelocity(xworld, fworld);
		} else if(ok && axis.contains("air wave")) {
		        ok = False;
			specCoor.setWavelengthUnit(xunits);
			ok = specCoor.frequencyToAirWavelength(xworld, fworld);
		} else if(ok && axis.contains("wave")) {
		        ok = False;
			specCoor.setWavelengthUnit(xunits);
			ok = specCoor.frequencyToWavelength(xworld, fworld);
		} else {
			xworld = fworld;
		}
	} else { // no conversion necessary
		xworld = pix;
		ok = True;
	}

	if (!ok)
		return False;

	convertArray(specVal, xworld);
	//cout << xworld << endl;
	return True;
}


Bool ImageAnalysis::getFreqProfile(const Vector<Double>& xy,
				   Vector<Float>& zxaxisval, Vector<Float>& zyaxisval,
				   const String& xytype,
				   const String& specaxis, const Int&,
				   const Int&, const Int&,
				   const String& xunits, const String& specFrame,
				   const Int& whichQuality, const String& restValue) {

	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);
	if (xy.size() != 2) {
		*itsLog << "input xy vector must have exactly two elements. It has "
			<< xy.size() << "." << LogIO::EXCEPTION;
	}
	String whatXY = xytype;
	Vector<Double> xypix(2);
	xypix = 0.0;
	whatXY.downcase();
	CoordinateSystem cSys = pImage_p->coordinates();
	Int which = cSys.findCoordinate(Coordinate::DIRECTION);
	if (which < 0){
		*itsLog << LogIO::WARN << "Image: " << pImage_p->name()
				<< " does not have a DIRECTION coordinate system!" << LogIO::POST;
		return False;
	}

	// if necessary, convert from
	// world to pixel coordinates
	if (whatXY.contains("wor")) {
		const DirectionCoordinate& dirCoor = cSys.directionCoordinate(which);
		if (!dirCoor.toPixel(xypix, xy))
			return False;
	}
	else {
		if (xy.nelements() != 2)
			return False;
		xypix = xy;
	}

	// create container to define the corners
	IPosition blc(pImage_p->ndim(), 0);
	IPosition trc(pImage_p->ndim(), 0);

	// set the right index in the quality-coordinate
	Int qualAx = cSys.findCoordinate(Coordinate::QUALITY);
	if (qualAx>-1){
		Int pixQualAx = cSys.pixelAxes(qualAx)[0];
		blc(pixQualAx) = whichQuality;
		trc(pixQualAx) = whichQuality;
	}

	// make sure the pixel is insed the mage
	if ((xypix(0) < 0) || (xypix(0) > pImage_p->shape()(0)) || (xypix(1) < 0)
		  || (xypix(1) > pImage_p->shape()(1))) {
		return False;
	}

	// set the directional position in pixels
	Vector<Int> dirPixelAxis = cSys.pixelAxes(which);
	blc[dirPixelAxis(0)] = Int(xypix(0) + 0.5); // note: pixel _center_ is at integer values
	trc[dirPixelAxis(0)] = Int(xypix(0) + 0.5);
	blc[dirPixelAxis(1)] = Int(xypix(1) + 0.5);
	trc[dirPixelAxis(1)] = Int(xypix(1) + 0.5);

	// set the spectral index to extract the entire spectrum
	Int specAx = cSys.findCoordinate(Coordinate::SPECTRAL);
	Vector<Bool> zyaxismask;
	trc[cSys.pixelAxes(specAx)[0]] = pImage_p->shape()(cSys.pixelAxes(specAx)[0]) - 1;

	// extract the data and the mask
	zyaxisval.resize();
	zyaxisval = pImage_p->getSlice(blc, trc - blc + 1, True);
	zyaxismask = pImage_p->getMaskSlice(blc, trc - blc + 1, True);

	// apply the mask
	// FIXME: I dont think it makes
	// sense to just set these values to 0.0
	// masked spectral points should be eliminated.
	for (uInt kk = 0; kk < zyaxisval.nelements(); ++kk) {
		if (!zyaxismask[kk]) {
			zyaxisval[kk] = 0.;
		}

		// check for error arrays
		if (whichQuality > 0){
			if (zyaxisval[kk] < 0.){
				*itsLog << LogIO::WARN << "Value set to 0.0, sqrt(<0.0) not allowed!" << LogIO::POST;
				zyaxisval[kk] = 0.0;
			}
			else{
				zyaxisval[kk] = sqrt(zyaxisval[kk]);
			}
		}
	}

	// get the spectral values
	zxaxisval.resize(zyaxisval.nelements());
	return getSpectralAxisVal(specaxis, zxaxisval, cSys, xunits, specFrame, restValue);
}


Bool ImageAnalysis::getFreqProfile(
		const Vector<Double>& x, const Vector<Double>& y,
		Vector<Float>& zxaxisval, Vector<Float>& zyaxisval,
		const String& xytype, const String& specaxis,
		const Int& whichStokes, const Int& whichTabular,
		const Int& whichLinear, const String& xunits,
		const String& specFrame, const Int &combineType,
		const Int& whichQuality, const String& restValue)
{
	*itsLog << LogOrigin("ImageAnalysis", __FUNCTION__);

	Vector<Double> xy(2);
	xy[0] = 0;
	xy[1] = 0;
	Int n = x.nelements();
	RegionManager regMan;
	ImageRegion* imagreg = 0;
	CoordinateSystem cSys = pImage_p->coordinates();
	Array<Float> dataArr;
	Array<Bool>  maskArr;

	// the x- and y-array must
	// have at least one element
	if (n < 1) {
		return False;
	}

	// for a point extraction,
	// call the another method
	if (n == 1) {
		xy[0] = x[0];
		xy[1] = y[0];
		return getFreqProfile(xy, zxaxisval, zyaxisval, xytype, specaxis,
				whichStokes, whichTabular, whichLinear, xunits, specFrame, whichQuality, restValue);
	}

	// n > 1, i.e. region to average over is a rectangle or polygon
	// identify the relevant axes (SPECTRAL and DIRECTIONAL)
	Int specAx = cSys.findCoordinate(Coordinate::SPECTRAL);
	Int pixSpecAx = cSys.pixelAxes(specAx)[0];
	Int nchan = pImage_p->shape()(pixSpecAx);
	Int which = cSys.findCoordinate(Coordinate::DIRECTION);
	if (which < 0){
		*itsLog << LogIO::WARN << "Image: " << pImage_p->name()
				<< " does not have a DIRECTION coordinate system!" << LogIO::POST;
		return False;
	}
	Vector<Int> dirPixelAxis = cSys.pixelAxes(which);

	// create the image region for
	// a rectangle
	if (n == 2) { // rectangle
		Vector<Quantity> blc(2);
		Vector<Quantity> trc(2);
		if (xytype.contains("wor")) {

			blc(0) = Quantity(x[0], "rad");
			blc(1) = Quantity(y[0], "rad");
			trc(0) = Quantity(x[1], "rad");
			trc(1) = Quantity(y[1], "rad");
			Vector<Int> pixax(2);
			pixax(0) = dirPixelAxis[0];
			pixax(1) = dirPixelAxis[1];
			imagreg = regMan.wbox(blc, trc, pixax, cSys);
		}
	}

	// create the image region for
	// a polygon
	if (n > 2) {
		Vector<Quantity> xvertex(n);
		Vector<Quantity> yvertex(n);
		for (Int k = 0; k < n; ++k) {
			xvertex[k] = Quantity(x[k], "rad");
			yvertex[k] = Quantity(y[k], "rad");
		}
		Vector<Int> pixax(2);
		pixax(0) = dirPixelAxis[0];
		pixax(1) = dirPixelAxis[1];
		imagreg = regMan.wpolygon(xvertex, yvertex, pixax, cSys, "abs");
	}

	// extract the subimage
	// and the corresponding mask
	if (imagreg != 0) {
		try{
			SubImage<Float> subim(*pImage_p, *imagreg, False);
			maskArr = subim.getMask();
			dataArr = subim.get();
		} catch (AipsError x) {
			*itsLog << "Error in extraction: " << x.getMesg() << LogIO::POST;
			return False;
		}
	}
	else {
		return False;
	}

	IPosition blc(cSys.nPixelAxes(),0);
	IPosition trc(cSys.nPixelAxes(),0);

	//FIXME only the I image for now
	//Int polAx  = cSys.findCoordinate(Coordinate::STOKES);
	//if (polAx >= 0) {
	//	Int pixPolAx = cSys.pixelAxes(polAx)[0];
	//	blc(pixPolAx) = 0;
	//	trc(pixPolAx) = 0;
	//}

	try{
		// set the right index in the quality-coordinate
		Int qualAx = cSys.findCoordinate(Coordinate::QUALITY);
		if (qualAx>-1){
			Int pixQualAx = cSys.pixelAxes(qualAx)[0];
			blc(pixQualAx) = whichQuality;
			trc(pixQualAx) = whichQuality;
		}

		//x-y plane
		blc(dirPixelAxis[0]) = 0;
		blc(dirPixelAxis[1]) = 0;
		trc(dirPixelAxis[0]) = dataArr.shape()(dirPixelAxis[0]) - 1;
		trc(dirPixelAxis[1]) = dataArr.shape()(dirPixelAxis[1]) - 1;
		zyaxisval.resize(nchan);
		zyaxisval = 0.0;

		// branch the various
		// combine types
		switch (combineType)
		{
		case 0:
			// combine with average
			for (Int k = 0; k < nchan; ++k) {
				blc(pixSpecAx) = k;
				trc(pixSpecAx) = k;
				MaskedArray<Float> planedat(dataArr(blc, trc), maskArr(blc, trc));
				if (planedat.nelementsValid() >0)
					zyaxisval(k) = mean(planedat);
			}
			break;
		case 1:
			// combine with median
			for (Int k = 0; k < nchan; ++k) {
				blc(pixSpecAx) = k;
				trc(pixSpecAx) = k;
				MaskedArray<Float> planedat(dataArr(blc, trc), maskArr(blc, trc));
				if (planedat.nelementsValid() >0)
					zyaxisval(k) = median(planedat);
			}
			break;
		case 2:
			// combine with sum
			for (Int k = 0; k < nchan; ++k) {
				blc(pixSpecAx) = k;
				trc(pixSpecAx) = k;
				MaskedArray<Float> planedat(dataArr(blc, trc), maskArr(blc, trc));
				if (planedat.nelementsValid() >0)
					zyaxisval(k) = sum(planedat);
			}
			break;
		case 3:
			// combine with variance
			for (Int k = 0; k < nchan; ++k) {
				blc(pixSpecAx) = k;
				trc(pixSpecAx) = k;
				MaskedArray<Float> planedat(dataArr(blc, trc), maskArr(blc, trc));
				if (planedat.nelementsValid() >0)
					zyaxisval(k) = variance(planedat);
			}
			break;
		case 4:
			// combine with stddev
			for (Int k = 0; k < nchan; ++k) {
				blc(pixSpecAx) = k;
				trc(pixSpecAx) = k;
				MaskedArray<Float> planedat(dataArr(blc, trc), maskArr(blc, trc));
				if (planedat.nelementsValid() >0)
					zyaxisval(k) = stddev(planedat);
			}
			break;
		case 5:
			// combine with the square root of the sum of squares
			for (Int k = 0; k < nchan; ++k) {
				blc(pixSpecAx) = k;
				trc(pixSpecAx) = k;
				MaskedArray<Float> planedat(dataArr(blc, trc), maskArr(blc, trc));
				if (planedat.nelementsValid() >0){
					zyaxisval(k) = sum(planedat);
					if (zyaxisval(k) < 0.0) {
						zyaxisval(k) = 0.0;
						*itsLog << LogIO::WARN << "Value set to 0.0, sqrt(<0.0) not allowed!" << LogIO::POST;
					}
					else {
						zyaxisval(k) = sqrt(zyaxisval(k));
					}
					//zyaxisval(k) = sqrt(sum(planedat));
				}
			}
			break;
		case 6:
			// combine with the average square root of the sum of squares
			Float fnpix;
			for (Int k = 0; k < nchan; ++k) {
				blc(pixSpecAx) = k;
				trc(pixSpecAx) = k;
				MaskedArray<Float> planedat(dataArr(blc, trc), maskArr(blc, trc));
				fnpix = Float(planedat.nelementsValid());
				if (fnpix >0.0){
					zyaxisval(k) = sum(planedat);
					if (zyaxisval(k) < 0.0) {
						zyaxisval(k) = 0.0;
						*itsLog << LogIO::WARN << "Value set to 0.0, sqrt(<0.0) not allowed!" << LogIO::POST;
					}
					else {
						zyaxisval(k) = sqrt(zyaxisval(k)) / fnpix;
					}
					//zyaxisval(k) = sqrt(sum(planedat)) / fnpix;
				}
			}
			break;
		default:
			// default is average
			for (Int k = 0; k < nchan; ++k) {
				blc(pixSpecAx) = k;
				trc(pixSpecAx) = k;
				MaskedArray<Float> planedat(dataArr(blc, trc), maskArr(blc, trc));
				if (planedat.nelementsValid() >0)
					zyaxisval(k) = mean(planedat);
			}
		}
	} catch (AipsError x) {
		*itsLog << "Error in extraction: " << x.getMesg() << LogIO::POST;
		return False;
	}

	// get the spectral values
	zxaxisval.resize(zyaxisval.nelements());
	return getSpectralAxisVal(specaxis, zxaxisval, cSys, xunits, specFrame, restValue);
}

// These should really go in a coordsys inside the casa name space

Record ImageAnalysis::toWorldRecord(const Vector<Double>& pixel,
		const String& format) const {

	*itsLog << LogOrigin("ImageAnalysis", "toWorldRecord");
	//
	Vector<Double> pixel2 = pixel.copy();
	//   if (pixel2.nelements()>0) pixel2 -= 1.0;        // 0-rel
	CoordinateSystem itsCSys = pImage_p->coordinates();
	trim(pixel2, itsCSys.referencePixel());

	// Convert to world

	Vector<Double> world;
	Record rec;
	if (itsCSys.toWorld(world, pixel2)) {
		rec = worldVectorToRecord(world, -1, format, True, True);
	}
	else {
		*itsLog << itsCSys.errorMessage() << LogIO::EXCEPTION;
	}
	return rec;
}

Record ImageAnalysis::worldVectorToRecord(const Vector<Double>& world, Int c,
		const String& format, Bool isAbsolute, Bool showAsAbsolute) const
//
// World vector must be in the native units of cSys
// c = -1 means world must be length cSys.nWorldAxes
// c > 0 means world must be length cSys.coordinate(c).nWorldAxes()
// format from 'n,q,s,m'
//
{
	*itsLog << LogOrigin("ImageAnalysis", "worldVectorToRecord");
	String ct = upcase(format);
	Vector<String> units;
	CoordinateSystem itsCSys = pImage_p->coordinates();
	if (c < 0) {
		units = itsCSys.worldAxisUnits();
	} else {
		units = itsCSys.coordinate(c).worldAxisUnits();
	}
	AlwaysAssert(world.nelements()==units.nelements(),AipsError);
	//
	Record rec;
	if (ct.contains(String("N"))) {
		rec.define("numeric", world);
	}
	//
	if (ct.contains(String("Q"))) {
		String error;
		Record recQ1, recQ2;
		//
		for (uInt i = 0; i < world.nelements(); i++) {
			Quantum<Double> worldQ(world(i), Unit(units(i)));
			QuantumHolder h(worldQ);
			if (!h.toRecord(error, recQ1))
				*itsLog << error << LogIO::EXCEPTION;
			recQ2.defineRecord(i, recQ1);
		}
		rec.defineRecord("quantity", recQ2);
	}
	//
	if (ct.contains(String("S"))) {
		Vector<Int> worldAxes;
		if (c < 0) {
			worldAxes.resize(world.nelements());
			indgen(worldAxes);
		} else {
			worldAxes = itsCSys.worldAxes(c);
		}
		//
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
			//
			u = "";
			fs(i) = itsCSys.format(u, fType, world(i), worldAxes(i),
					isAbsolute, showAsAbsolute, prec);
			if ((u != String("")) && (u != String(" "))) {
				fs(i) += String(" ") + u;
			}
		}

		rec.define("string", fs);
	}
	//
	if (ct.contains(String("M"))) {
		Record recM = worldVectorToMeasures(world, c, isAbsolute);
		rec.defineRecord("measure", recM);
	}
	//
	return rec;
}

Record ImageAnalysis::worldVectorToMeasures(const Vector<Double>& world, Int c,
		Bool abs) const {
	LogIO os(LogOrigin("ImageAnalysis", "worldVectorToMeasures(...)"));

	//
	uInt directionCount, spectralCount, linearCount, stokesCount, tabularCount;
	directionCount = spectralCount = linearCount = stokesCount = tabularCount
			= 0;

	CoordinateSystem itsCSys = pImage_p->coordinates();

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
	//
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
		//
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
			//
			if (!none) {

				// Make an MFrequency and stick in record

				Record specRec, specRec1;
				Quantum<Double> t1(world2(0), units(0));
				const SpectralCoordinate& sc0 = itsCSys.spectralCoordinate(i);
				MFrequency frequency(t1, sc0.frequencySystem());
				//
				MeasureHolder h(frequency);
				if (!h.toRecord(error, specRec1)) {
					os << error << LogIO::EXCEPTION;
				} else {
					specRec.defineRecord("frequency", specRec1);
				}
				//
				SpectralCoordinate sc(sc0);

				// Do velocity conversions and stick in MDOppler
				// Radio

				sc.setVelocity(String("km/s"), MDoppler::RADIO);
				Quantum<Double> velocity;
				if (!sc.frequencyToVelocity(velocity, frequency)) {
					os << sc.errorMessage() << LogIO::EXCEPTION;
				} else {
					MDoppler v(velocity, MDoppler::RADIO);
					MeasureHolder h(v);
					if (!h.toRecord(error, specRec1)) {
						os << error << LogIO::EXCEPTION;
					} else {
						specRec.defineRecord("radiovelocity", specRec1);
					}
				}

				// Optical

				sc.setVelocity(String("km/s"), MDoppler::OPTICAL);
				if (!sc.frequencyToVelocity(velocity, frequency)) {
					os << sc.errorMessage() << LogIO::EXCEPTION;
				} else {
					MDoppler v(velocity, MDoppler::OPTICAL);
					MeasureHolder h(v);
					if (!h.toRecord(error, specRec1)) {
						os << error << LogIO::EXCEPTION;
					} else {
						specRec.defineRecord("opticalvelocity", specRec1);
					}
				}

				// beta (relativistic/true)

				sc.setVelocity(String("km/s"), MDoppler::BETA);
				if (!sc.frequencyToVelocity(velocity, frequency)) {
					os << sc.errorMessage() << LogIO::EXCEPTION;
				} else {
					MDoppler v(velocity, MDoppler::BETA);
					MeasureHolder h(v);
					if (!h.toRecord(error, specRec1)) {
						os << error << LogIO::EXCEPTION;
					} else {
						specRec.defineRecord("betavelocity", specRec1);
					}
				}

				// Fill spectral record

				rec.defineRecord("spectral", specRec);
			}
			spectralCount++;
		} else if (itsCSys.type(i) == Coordinate::STOKES) {
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
		} else {
			os << "Cannot handle Coordinates of type " << itsCSys.showType(i)
					<< LogIO::EXCEPTION;
		}
	}
	//
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
	//
	return rec;
}

void ImageAnalysis::trim(Vector<Double>& inout, const Vector<Double>& replace) const {
	const Int nIn = inout.nelements();
	const Int nOut = replace.nelements();
	Vector<Double> out(nOut);
	for (Int i = 0; i < nOut; i++) {
		if (i > nIn - 1) {
			out(i) = replace(i);
		} else {
			out(i) = inout(i);
		}
	}
	inout.resize(nOut);
	inout = out;
}

/// When CoordSys is refactored the above should be removed cleanly


} // end of  casa namespace
