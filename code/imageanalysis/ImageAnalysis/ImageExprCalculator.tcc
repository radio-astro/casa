#include "ImageExprCalculator.h"


#include <casa/BasicSL/String.h>
#include <images/Images/ImageBeamSet.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageProxy.h>

#include <imageanalysis/ImageTypedefs.h>
#include <imageanalysis/ImageAnalysis/ImageFactory.h>
#include <imageanalysis/ImageAnalysis/PixelValueManipulator.h>

using namespace std;

namespace casa {

template<class T> ImageExprCalculator<T>::ImageExprCalculator(
	const String& expression, const String& outname, Bool overwrite
) : _expr(expression), _copyMetaDataFromImage(), _outname(outname),
	_overwrite(overwrite), _log() {
	ThrowIf(_expr.empty(), "You must specify an expression");
	if (! outname.empty() && ! overwrite) {
		//NewFile validfile;
		String errmsg;
		ThrowIf(
			! NewFile().valueOK(outname, errmsg), errmsg
		);
	}
}

template<class T> SPIIT ImageExprCalculator<T>::compute() const {
	_log << LogOrigin(getClass(), __func__);
	Record regions;

	// Get LatticeExprNode (tree) from parser.  Substitute possible
	// object-id's by a sequence number, while creating a
	// LatticeExprNode for it.  Convert the GlishRecord containing
	// regions to a PtrBlock<const ImageRegion*>.

	Block<LatticeExprNode> temps;
	String exprName;
	PtrBlock<const ImageRegion*> tempRegs;
	_makeRegionBlock(tempRegs, regions);
	LatticeExprNode node = ImageExprParse::command(_expr, temps, tempRegs);
	_checkImages();
	// Get the shape of the expression
	const IPosition shape = node.shape();

	// Get the CoordinateSystem of the expression
	const LELAttribute attr = node.getAttribute();
	const LELLattCoordBase* lattCoord = &(attr.coordinates().coordinates());
	ThrowIf(
		! lattCoord->hasCoordinates()
		|| lattCoord->classname() != "LELImageCoord",
		"Images in expression have no coordinates"
	);
	const LELImageCoord* imCoord =
		dynamic_cast<const LELImageCoord*> (lattCoord);
	AlwaysAssert (imCoord != 0, AipsError);
	CoordinateSystem csys = imCoord->coordinates();
	//DataType type = node.dataType();
	SPIIT computedImage = _imagecalc(
		node, shape, csys, imCoord
	);
	// Delete the ImageRegions (by using an empty Record).
	_makeRegionBlock(tempRegs, Record());
	return computedImage;
}

template<class T> void ImageExprCalculator<T>::compute2(
	SPIIT image, const String& expr, Bool verbose
) {
	LogIO log;
    log << LogOrigin("ImageExprCalculator", __func__);
    ThrowIf(expr.empty(), "You must specify an expression");
    Record regions;
    Block<LatticeExprNode> temps;
    PtrBlock<const ImageRegion*> tempRegs;
    PixelValueManipulator<T>::makeRegionBlock(tempRegs, regions);
    auto node = ImageExprParse::command(expr, temps, tempRegs);
    auto type = node.dataType();
    Bool isReal = casa::isReal(type);
    ostringstream os;
    os << type;
    ThrowIf(
        ! isReal && ! isComplex(type),
        "Unsupported node data type " + os.str()
    );
    ThrowIf(
    	isComplex(image->dataType()) && isReal,
        "Resulting image is real valued but"
        "the attached image is complex valued"
    );
    ThrowIf(
        casa::isReal(image->dataType()) && isComplex(type),
        "Resulting image is complex valued but"
        "the attached image is real valued"
    );
    if (verbose) {
        log << LogIO::WARN << "Overwriting pixel values "
            << "of the currently attached image"
            << LogIO::POST;
    }
    _calc(image, node);
}

template<class T> void ImageExprCalculator<T>::_calc(
    SHARED_PTR<ImageInterface<T> > image,
    const LatticeExprNode& node
) {
    // Get the shape of the expression and check it matches that
    // of the output image
    if (! node.isScalar() && ! image->shape().isEqual(node.shape())) {
    	// const auto shapeOut = node.shape();
        //if (! image->shape().isEqual(shapeOut)) {
            ostringstream os;
            os << "The shape of the expression does not conform "
                << "with the shape of the output image"
                << "Expression shape = " << node.shape()
                << "Image shape = " << image->shape();
            throw AipsError(os.str());
        //}
    }
    // Get the CoordinateSystem of the expression and check it matches
    // that of the output image
    LogIO mylog;
    if (! node.isScalar()) {
        const auto attr = node.getAttribute();
        const auto lattCoord = &(attr.coordinates().coordinates());
        if (!lattCoord->hasCoordinates() || lattCoord->classname()
                != "LELImageCoord") {
            // We assume here that the output coordinates are ok
            mylog << LogIO::WARN
                    << "Images in expression have no coordinates"
                    << LogIO::POST;
        }
        else {
            const auto imCoord =
                    dynamic_cast<const LELImageCoord*> (lattCoord);
            AlwaysAssert (imCoord != 0, AipsError);
            const auto& cSysOut = imCoord->coordinates();
            if (! image->coordinates().near(cSysOut)) {
                // Since the output image has coordinates, and the shapes
                // have conformed, just issue a warning
                mylog << LogIO::WARN
                        << "The coordinates of the expression do not conform "
                        << endl;
                mylog << "with the coordinates of the output image" << endl;
                mylog << "Proceeding with output image coordinates"
                        << LogIO::POST;
            }
        }
    }
    // Make a LatticeExpr and see if it is masked
    Bool exprIsMasked = node.isMasked();
    if (exprIsMasked) {
        if (! image->isMasked()) {
            // The image does not have a default mask set.  So try and make it one.
            String maskName = "";
            ImageMaskAttacher::makeMask(*image, maskName, True, True, mylog, True);
        }
    }

    // Evaluate the expression and fill the output image and mask
    if (node.isScalar()) {
        LatticeExprNode node2 = isReal(node.dataType())
        	? toFloat(node) : toComplex(node);
        // If the scalar value is masked, there is nothing
        // to do.
        if (! exprIsMasked) {
            if (image->isMasked()) {
                // We implement with a LEL expression of the form
                // iif(mask(image)", value, image)
                auto node3 = iif(mask(*image), node2, *image);
                image->copyData(LatticeExpr<T> (node3));
            }
            else {
                // Just set all values to the scalar. There is no mask to
                // worry about.
                image->copyData(LatticeExpr<T> (node2));
            }
        }
    }
    else {
        if (image->isMasked()) {
            // We implement with a LEL expression of the form
            // iif(mask(image)", expr, image)
            auto node3 = iif(mask(*image), node, *image);
            image->copyData(LatticeExpr<T> (node3));
        }
        else {
            // Just copy the pixels from the expression to the output.
            // There is no mask to worry about.
            image->copyData(LatticeExpr<T> (node));
        }
    }
}

template<class T> SPIIT ImageExprCalculator<T>::_imagecalc(
	const LatticeExprNode& node, const IPosition& shape,
	const CoordinateSystem& csys, const LELImageCoord* const imCoord
) const {
	_log << LogOrigin(getClass(), __func__);

	// Create LatticeExpr create mask if needed
	LatticeExpr<T> latEx(node);
	SPIIT image;
	String exprName;
	// Construct output image - an ImageExpr or a PagedImage
	if (_outname.empty()) {
		image.reset(new ImageExpr<T> (latEx, exprName));
		ThrowIf(! image, "Failed to create ImageExpr");
	}
	else {
		_log << LogIO::NORMAL << "Creating image `" << _outname
			<< "' of shape " << shape << LogIO::POST;
		try {
			image.reset(new PagedImage<T> (shape, csys, _outname));
		}
		catch (const TableError& te) {
			ThrowIf(
				_overwrite,
				"Caught TableError. This often means "
				"the table you are trying to overwrite has been opened by "
				"another method and so cannot be overwritten at this time. "
				"Try closing it and rerunning"
			);
		}
		ThrowIf(! image, "Failed to create PagedImage");

		// Make mask if needed, and copy data and mask
		if (latEx.isMasked()) {
			String maskName("");
			ImageMaskAttacher::makeMask(*image, maskName, False, True, _log, True);
		}
		LatticeUtilities::copyDataAndMask(_log, *image, latEx);
	}

	// Copy miscellaneous stuff over
    auto copied = False;
    Unit unit;
	if (! _copyMetaDataFromImage.empty()) {
        // do nonexistant file handling here because users want a special message
        ThrowIf(
            ! File(_copyMetaDataFromImage).isReadable(),
            "Cannot access " + _copyMetaDataFromImage
            + " so cannot copy its metadata to output image"
        );
        auto mypair = ImageFactory::fromFile(_copyMetaDataFromImage);
        if (mypair.first || mypair.second) {
            if (mypair.first) {
                image->setMiscInfo(mypair.first->miscInfo());
		        image->setImageInfo(mypair.first->imageInfo());
		        image->setCoordinateInfo(mypair.first->coordinates());
		        unit = mypair.first->units();
            } 
            else {
                image->setMiscInfo(mypair.second->miscInfo());
		        image->setImageInfo(mypair.second->imageInfo());
		        image->setCoordinateInfo(mypair.second->coordinates());
		        unit = mypair.second->units();
            }
            copied = True;
        }
	}
    if (! copied) {
		image->setMiscInfo(imCoord->miscInfo());
		image->setImageInfo(imCoord->imageInfo());
	}
	if (_expr.contains("spectralindex")) {
		image->setUnits("");
	}
	else if (_expr.contains(Regex("pa\\(*"))) {
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
		image->setUnits(copied ? unit : imCoord->unit());
	}
	return image;
}

template<class T> void ImageExprCalculator<T>::_makeRegionBlock(
	PtrBlock<const ImageRegion*>& regions,
	const Record& Regions
) {
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

template<class T> void ImageExprCalculator<T>::_checkImages() const {
	auto images = ImageExprParse::getImageNames();
	if (images.size() <= 1) {
		return;
	}
	unique_ptr<String> unit;
	unique_ptr<ImageBeamSet> beamSet;
	unique_ptr<Vector<String>> axisNames;
	for (auto& image: images) {
		if (File(image).exists()) {
			try {
				ImageProxy myImage(image, "", vector<ImageProxy>());
				if (myImage.getLattice()) {
					auto myUnit = myImage.unit();
					if (unit) {
						if (myUnit != *unit) {
							_log << LogIO::WARN << "image units are not the same: '"
								<< *unit << "' vs '" << myUnit << "'" << LogIO::POST;
							break;
						}
					}
					else {
						unit.reset(new String(myUnit));
					}
					ImageBeamSet mybs = myImage.imageInfoObject().getBeamSet();
					if (beamSet) {
						if (mybs != *beamSet) {
							ostringstream oss;
							oss << "image beams are not the same: " << mybs << " vs " << *beamSet;
							_log << LogIO::WARN << oss.str() << LogIO::POST;
							break;
						}
					}
					else {
						beamSet.reset(new ImageBeamSet(mybs));
					}
					auto myAxes = myImage.coordSysObject().worldAxisNames();
					if (axisNames) {
						if (myAxes.size() != axisNames->size()) {
							_log << LogIO::WARN << "Number of axes in input images differs"
								<< LogIO::POST;
							break;
						}
						auto iter = begin(myAxes);
						for (const auto& axis: *axisNames) {
							if (axis != *iter) {
								_log << LogIO::WARN << "Axes ordering and/or axes names "
									<< "in input images differs:" << *axisNames << " vs "
									<< myAxes << LogIO::POST;
								break;
							}
							++iter;
						}
					}
					else {
						axisNames.reset(new Vector<String>(myAxes));
					}

				}
			}
			catch (const AipsError&) {}
		}
	}
}

}
