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

// PLEASE DO *NOT* ADD ADDITIONAL METHODS TO THIS CLASS

#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>

#include <casa/Logging/LogFilter.h>
#include <imageanalysis/ImageAnalysis/ImageFFT.h>
#include <imageanalysis/ImageAnalysis/ImageMaskAttacher.h>
#include <imageanalysis/ImageAnalysis/ImageMetaData.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/ImageSummary.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/LatticeUtilities.h>

namespace casa {

template<class T> void ImageAnalysis::_calc(
	std::tr1::shared_ptr<ImageInterface<T> > image,
	const LatticeExprNode& node
) {

	// Get the shape of the expression and check it matches that
	// of the output image
	if (! node.isScalar()) {
		const IPosition shapeOut = node.shape();
		if (! image->shape().isEqual(shapeOut)) {
			ostringstream os;
			os << "The shape of the expression does not conform "
				<< "with the shape of the output image"
				<< "Expression shape = " << shapeOut
				<< "Image shape = " << image->shape();
			throw AipsError(os.str());
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
			*_log << LogIO::WARN
					<< "Images in expression have no coordinates"
					<< LogIO::POST;
		}
		else {
			const LELImageCoord* imCoord =
					dynamic_cast<const LELImageCoord*> (lattCoord);
			AlwaysAssert (imCoord != 0, AipsError);
			const CoordinateSystem& cSysOut = imCoord->coordinates();
			if (! image->coordinates().near(cSysOut)) {
				// Since the output image has coordinates, and the shapes
				// have conformed, just issue a warning
				*_log << LogIO::WARN
						<< "The coordinates of the expression do not conform "
						<< endl;
				*_log << "with the coordinates of the output image" << endl;
				*_log << "Proceeding with output image coordinates"
						<< LogIO::POST;
			}
		}
	}

	// Make a LatticeExpr and see if it is masked
	Bool exprIsMasked = node.isMasked();
	if (exprIsMasked) {
		if (! image->isMasked()) {
			// The image does not have a default mask set.  So try and make it one.
			String maskName("");
			ImageMaskAttacher::makeMask(*image, maskName, True, True, *_log, True);
		}
	}

	// Evaluate the expression and fill the output image and mask
	if (node.isScalar()) {
		LatticeExprNode node2;

		if (isReal(node.dataType())) {
			node2 = toFloat(node);
		}
		else {
			node2 = toComplex(node);
		}
		// If the scalar value is masked, there is nothing
		// to do.
		if (! exprIsMasked) {
			if (image->isMasked()) {
				// We implement with a LEL expression of the form
				// iif(mask(image)", value, image)
				LatticeExprNode node3 = iif(mask(*image), node2, *image);
				image->copyData(LatticeExpr<T> (node3));
			}
			else {
				// Just set all values to the scalar. There is no mask to
				// worry about.
				if (isReal(node.dataType())) {
					Float value = node2.getFloat();
					_imageFloat->set(value);
				}
				else {
					Complex value = node2.getComplex();
					_imageComplex->set(value);
				}
			}
		}
	}
	else {
		if (image->isMasked()) {
			// We implement with a LEL expression of the form
			// iif(mask(image)", expr, image)
			LatticeExprNode node3 = iif(mask(*image), node, *image);
			image->copyData(LatticeExpr<T> (node3));
		}
		else {
			// Just copy the pixels from the expression to the output.
			// There is no mask to worry about.
			image->copyData(LatticeExpr<T> (node));
		}
	}
}

template<class T> Bool ImageAnalysis::_calcmask(
    std::tr1::shared_ptr<ImageInterface<T> > image,
    const LatticeExprNode& node,
    const String& maskName, const Bool makeDefault
) {
	*_log << LogOrigin(className(), __FUNCTION__);
	// Get the shape of the expression and check it matches that
	// of the output image.  We don't check that the Coordinates
	// match as that would be an un-necessary restriction.
	if (
		!node.isScalar()
		&& ! image->shape().isEqual(node.shape())
	) {
		ostringstream os;
		os << "The shape of the expression does not conform "
			<< "with the shape of the output image"
			<< "Expression shape = " << node.shape()
			<< "Image shape      = " << image->shape();
		ThrowCc(os.str());
	}

	ThrowIf (
		! image->canDefineRegion(),
		"Cannot make requested mask for this type of image"
		"It is of type" + image->imageType()
	);
	// Make mask and get hold of its name.   Currently new mask is forced to
	// be default because of other problems.  Cannot use the usual ImageMaskAttacher<Float>::makeMask
	// function because I cant attach/make it default until the expression
	// has been evaluated
	// Generate mask name if not given
	String maskName2 = maskName;
	if (maskName.empty()) {
		maskName2 = image->makeUniqueRegionName(
			String("mask"), 0
		);
	}
	// Make the mask if it does not exist
	if (!image->hasRegion(maskName2, RegionHandler::Masks)) {
		image->makeMask(maskName2, True, False);
		*_log << LogIO::NORMAL << "Created mask `" << maskName2 << "'"
			<< LogIO::POST;
		ImageRegion iR = image->getRegion(
			maskName2, RegionHandler::Masks
		);
		LCRegion& mask = iR.asMask();
		if (node.isScalar()) {
			Bool value = node.getBool();
			mask.set(value);
		}
		else {
			mask.copyData(LatticeExpr<Bool> (node));
		}
	}
	else {
		// Access pre-existing mask.
		ImageRegion iR = image->getRegion(
			maskName2,
			RegionHandler::Masks
		);
		LCRegion& mask2 = iR.asMask();
		if (node.isScalar()) {
			Bool value = node.getBool();
			mask2.set(value);
		}
		else {
			mask2.copyData(LatticeExpr<Bool> (node));
		}
	}
	if (makeDefault) {
		image->setDefaultMask(maskName2);
	}
	return True;
}

template<class T> void ImageAnalysis::_destruct(ImageInterface<T>& image) {
	if((image.isPersistent()) && ((image.imageType()) == "PagedImage")) {
		ImageOpener::ImageTypes type = ImageOpener::imageType(image.name());
		if (type == ImageOpener::AIPSPP) {
			Table::relinquishAutoLocks(True);
			(static_cast<PagedImage<T>& >(image)).table().unlock();
		}
	}
}

template<class T> Bool ImageAnalysis::_getchunk(
	Array<T>& pixels, Array<Bool>& pixelMask,
	const ImageInterface<T>& image,
	const Vector<Int>& blc, const Vector<Int>& trc, const Vector<Int>& inc,
	const Vector<Int>& axes, const Bool list, const Bool dropdeg,
	const Bool getmask
) {
	*_log << LogOrigin(className(), __FUNCTION__);

	IPosition iblc = IPosition(Vector<Int> (blc));
	IPosition itrc = IPosition(Vector<Int> (trc));
	IPosition imshape = image.shape();

	// Verify region.
	IPosition iinc = IPosition(inc.size());
	for (uInt i = 0; i < inc.size(); i++) {
		iinc(i) = inc[i];
	}
	LCBox::verify(iblc, itrc, iinc, imshape);
	if (list) {
		*_log << LogIO::NORMAL << "Selected bounding box " << iblc << " to "
				<< itrc << LogIO::POST;
	}

	// Get the chunk.  The mask is not returned. Leave that to getRegion
	IPosition curshape = (itrc - iblc + iinc) / iinc;
	Slicer sl(iblc, itrc, iinc, Slicer::endIsLast);
	SubImage<T> subImage(image, sl);
	IPosition iAxes = IPosition(Vector<Int> (axes));
	if (getmask) {
		LatticeUtilities::collapse(pixels, pixelMask, iAxes, subImage, dropdeg);
		return True;
	} else {
		LatticeUtilities::collapse(pixels, iAxes, subImage, dropdeg);
		return True;
	}
}


template<class T> std::tr1::shared_ptr<ImageInterface<T> > ImageAnalysis::_imagecalc(
	const LatticeExprNode& node, const IPosition& shape,
	const CoordinateSystem& csys, const LELImageCoord* const imCoord,
	const String& outfile,
	const Bool overwrite, const String& expr
) {
	*_log << LogOrigin(className(), __FUNCTION__);

	// Create LatticeExpr create mask if needed
	LatticeExpr<T> latEx(node);
	std::tr1::shared_ptr<ImageInterface<T> > image;
	String exprName;
	// Construct output image - an ImageExpr or a PagedImage
	if (outfile.empty()) {
		image.reset(new ImageExpr<T> (latEx, exprName));
		ThrowIf(
			! image,
			"Failed to create ImageExpr"
		);
	}
	else {
		*_log << LogIO::NORMAL << "Creating image `" << outfile
			<< "' of shape " << shape << LogIO::POST;
		try {
			image.reset(new PagedImage<T> (shape, csys, outfile));
		}
		catch (const TableError& te) {
			if (overwrite) {
				*_log << LogIO::SEVERE << "Caught TableError. This often means "
					<< "the table you are trying to overwrite has been opened by "
					<< "another method and so cannot be overwritten at this time. "
					<< "Try closing it and rerunning" << LogIO::POST;
				RETHROW(te);
			}
		}
		ThrowIf(
			! image,
			"Failed to create PagedImage"
		);

		// Make mask if needed, and copy data and mask
		if (latEx.isMasked()) {
			String maskName("");
			ImageMaskAttacher::makeMask(*image, maskName, False, True, *_log, True);
		}
		LatticeUtilities::copyDataAndMask(*_log, *image, latEx);
	}

	// Copy miscellaneous stuff over
	image->setMiscInfo(imCoord->miscInfo());
	image->setImageInfo(imCoord->imageInfo());
	if (expr.contains("spectralindex")) {
		image->setUnits("");
	}
	else if (expr.contains(Regex("pa\\(*"))) {
		image->setUnits("deg");
		Vector<Int> newstokes(1);
		newstokes = Stokes::Pangle;
		StokesCoordinate scOut(newstokes);
		CoordinateSystem cSys = image->coordinates();
		Int iStokes = cSys.findCoordinate(Coordinate::STOKES, -1);
		cSys.replaceCoordinate(scOut, iStokes);
		image->setCoordinateInfo(cSys);
	}
	else {
		image->setUnits(imCoord->unit());
	}
	return image;
}


template<class T> Bool ImageAnalysis::_putchunk(
	ImageInterface<T>& image,
	const Array<T>& pixelsArray, const Vector<Int>& blc,
	const Vector<Int>& inc, const Bool list,
	const Bool locking, const Bool replicate
) {
	*_log << LogOrigin(className(), __FUNCTION__);
	IPosition imageShape = image.shape();
	uInt ndim = imageShape.nelements();
	ThrowIf(
		pixelsArray.ndim() > ndim,
		"Pixels array has more axes than the image!"
	);

	// Verify blc value. Fill in values for blc and inc.  trc set to shape-1
	IPosition iblc = IPosition(Vector<Int> (blc));
	IPosition itrc;
	IPosition iinc(inc.size());
	for (uInt i = 0; i < inc.size(); i++) {
		iinc(i) = inc[i];
	}
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
	ThrowIf(
		sl.end() + 1 > imageShape,
		"Pixels array, including inc, extends beyond edge of image."
	);
	Slicer sl2(iblc, itrc, iinc, Slicer::endIsLast);

	if (list) {
		*_log << LogIO::NORMAL << "Selected bounding box " << sl.start()
			<< " to " << sl.end() << LogIO::POST;
	}

	// Put the pixels
	if (pixelsArray.ndim() == ndim) {
		set_cache(pixelsArray.shape());
		if (replicate) {
			LatticeUtilities::replicate(image, sl2, pixelsArray);
		}
		else {
			image.putSlice(pixelsArray, iblc, iinc);
		}
	}
	else {
		// Pad with extra degenerate axes if necessary (since it is somewhat
		// costly).
		Array<T> pixelsref(
			pixelsArray.addDegenerate(
				ndim - pixelsArray.ndim()
			)
		);
		set_cache(pixelsref.shape());
		if (replicate) {
			LatticeUtilities::replicate(image, sl2, pixelsref);
		}
		else {
			image.putSlice(pixelsref, iblc, iinc);
		}
	}
	// Ensure that we reconstruct the  histograms objects
	// now that the data have changed
	deleteHist();

	Bool rstat = True;

	if (locking) {
		rstat = unlock();
	}
	return rstat;
}

template<class T> Bool ImageAnalysis::_setrestoringbeam(
	std::tr1::shared_ptr<ImageInterface<T> > image,
	const Quantity& major, const Quantity& minor,
	const Quantity& pa, const Record& rec,
	const bool deleteIt, const bool log,
    Int channel, Int polarization
) {
	*_log << LogOrigin(className(), __FUNCTION__);
	ImageInfo ii = image->imageInfo();
	if (deleteIt) {
		if (log) {
			if (ii.hasMultipleBeams() && (channel >= 0 || polarization >= 0)) {
				*_log << LogIO::WARN
					<< "Delete ignores any channel and/or polarization specification "
					<< "All per plane beams are being deleted" << LogIO::POST;
			}
			*_log << LogIO::NORMAL << "Deleting restoring beam(s)"
				<< LogIO::POST;
		}
		ii.removeRestoringBeam();
		if (! image->setImageInfo(ii)) {
			*_log << LogIO::POST << "Failed to remove restoring beam" << LogIO::POST;
			return False;
		}
		deleteHist();
		return True;
	}
	Quantity bmajor, bminor, bpa;
	if (rec.nfields() != 0) {
		String error;
		// instantiating this object will do implicit consistency checks
		// on the passed-in record
		GaussianBeam beam = GaussianBeam::fromRecord(rec);

		bmajor = beam.getMajor();
		bminor = beam.getMinor();
		bpa = beam.getPA(True);
	}
	else {
		bmajor = major;
		bminor = minor;
		bpa = pa;
	}
	if (bmajor.getValue() == 0 || bminor.getValue() == 0) {
		GaussianBeam currentBeam = ii.restoringBeam(channel, polarization);
		if (! currentBeam.isNull()) {
			bmajor = major.getValue() == 0 ? currentBeam.getMajor() : major;
			bminor = minor.getValue() == 0 ? currentBeam.getMinor() : minor;
			bpa = pa.isConform("rad") ? pa : Quantity(0, "deg");
		}
		else {
			if (ii.hasMultipleBeams()) {
				*_log
					<< "This image does not have a corresponding per plane "
					<< "restoring beam that can be "
					<< "used to set missing input parameters"
					<< LogIO::POST;
			}
			else {
				*_log
					<< "This image does not have a restoring beam that can be "
					<< "used to set missing input parameters"
					<< LogIO::POST;
			}
			return False;
		}
	}
	if (ii.hasMultipleBeams()) {
		if (channel < 0 && polarization < 0) {
			if (log) {
				*_log << LogIO::WARN << "This image has per plane beams"
					<< "but no plane (channel/polarization) was specified. All beams will be set "
					<< "equal to the specified beam." << LogIO::POST;
			}
			ImageMetaData md(image);
			ii.setAllBeams(
				md.nChannels(), md.nStokes(),
				GaussianBeam(bmajor, bminor, bpa)
			);
		}
		else {
			ii.setBeam(channel, polarization, bmajor, bminor, bpa);
		}
	}
	else if (channel >= 0 || polarization >= 0) {
		if (ii.restoringBeam().isNull()) {
			if (log) {
				*_log << LogIO::NORMAL << "This iamge currently has no beams of any kind. "
					<< "Since channel and/or polarization were specified, "
					<< "a set of per plane beams, each equal to the specified beam, "
					<< "will be created." << LogIO::POST;
			}
			ImageMetaData md(image);
			ii.setAllBeams(
				md.nChannels(), md.nStokes(),
				GaussianBeam(bmajor, bminor, bpa)
			);
		}
		else {
			if (log) {
				*_log << LogIO::WARN << "Channel and/or polarization has "
					<< "been specified, but this image has a single (global restoring "
					<< "beam. This beam will not be altered. If you really want to modify "
					<< "the global beam, rerun setting both channel and "
					<< "polarization less than zero" << LogIO::POST;
			}
			return False;
		}
	}
	else {
		if (log) {
			*_log << LogIO::NORMAL
				<< "Setting (gloal) restoring beam." << LogIO::POST;
		}
		ii.setRestoringBeam(GaussianBeam(bmajor, bminor, bpa));
	}
	if (! image->setImageInfo(ii)) {
		*_log << LogIO::POST << "Failed to set restoring beam" << LogIO::POST;
		return False;
	}
	if (log) {
		*_log << LogIO::NORMAL << "Beam parameters:"
			<< "  Major          : " << bmajor.getValue() << " " << bmajor.getUnit() << endl
			<< "  Minor          : " << bminor.getValue() << " " << bminor.getUnit() << endl
			<< "  Position Angle : " << bpa.getValue() << " " << bpa.getUnit() << endl
			<< LogIO::POST;
	}
	deleteHist();
	return True;
}

template<class T> Record ImageAnalysis::_summary(
	const ImageInterface<T>& image,
	const String& doppler, const Bool list,
	const Bool pixelorder, const Bool verbose
) {
	*_log << LogOrigin(className(), __FUNCTION__);
	Vector<String> messages;
	Record retval;
	ImageSummary<T> s(image);
	MDoppler::Types velType;
	if (!MDoppler::getType(velType, doppler)) {
		*_log << LogIO::WARN << "Illegal velocity type, using RADIO"
				<< LogIO::POST;
		velType = MDoppler::RADIO;
	}

	if (list) {
		messages = s.list(*_log, velType, False, verbose);
	}
	else {
		// Write messages to local sink only so we can fish them out again
		LogFilter filter;
		LogSink sink(filter, False);
		LogIO osl(sink);
		messages = s.list(osl, velType, True);
	}
	retval.define("messages", messages);
	Vector<String> axes = s.axisNames(pixelorder);
	Vector<Double> crpix = s.referencePixels(False); // 0-rel
	Vector<Double> crval = s.referenceValues(pixelorder);
	Vector<Double> cdelt = s.axisIncrements(pixelorder);
	Vector<String> axisunits = s.axisUnits(pixelorder);

	retval.define("ndim", Int(s.ndim()));
	retval.define("shape", s.shape().asVector());
	retval.define("tileshape", s.tileShape().asVector());
	retval.define("axisnames", axes);
	retval.define("refpix", crpix);
	retval.define("refval", crval);
	retval.define("incr", cdelt);
	retval.define("axisunits", axisunits);
	retval.define("unit", s.units().getName());
	retval.define("hasmask", s.hasAMask());
	retval.define("defaultmask", s.defaultMaskName());
	retval.define("masks", s.maskNames());
	retval.define("imagetype", s.imageType());

	ImageInfo info = image.imageInfo();
	Record iRec;
	String error;
	Bool ok = info.toRecord(error, iRec);
	if (! ok) {
		*_log << LogIO::SEVERE
				<< "Failed to convert ImageInfo to a record because "
				<< LogIO::EXCEPTION;
		*_log << LogIO::SEVERE << error << LogIO::POST;
	}
	else if (iRec.isDefined("restoringbeam")) {
		retval.defineRecord("restoringbeam", iRec.asRecord("restoringbeam"));
	}
	else if (iRec.isDefined("perplanebeams")) {
		retval.defineRecord("perplanebeams", info.beamToRecord(-1, -1));
	}
	return retval;
}


} // end of  casa namespace
