/*
 * ImageReorderer.cc
 *
 *  Created on: May 7, 2010
 *      Author: dmehring
 */

#include <imageanalysis/ImageAnalysis/ImageTransposer.h>

#include <images/Images/ImageAnalysis.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>

#include <memory>

namespace casa {

const String ImageTransposer::_class = "ImageReorderer";

ImageTransposer::ImageTransposer(
	const ImageInterface<Float> *const &image, const String& order, const String& outputImage
)
: _log(new LogIO), _image(image->cloneII()), _order(Vector<Int>(0)), _outputImage(outputImage) {
	LogOrigin origin("ImageReorderer", String(__FUNCTION__) + "_1");
	*_log << origin;
	_construct();
	*_log << origin;
	Regex intRegex("^[0-9]+$");
	if (order.matches(intRegex)) {
		_order = _getOrder(order);
	}
	else {
		*_log << "Incorrect order specification " << order
			<< ". All characters must be digits." << LogIO::EXCEPTION;
	}
}

ImageTransposer::ImageTransposer(
	const ImageInterface<Float> *const &image, const Vector<String> order, const String& outputImage
)
: _log(new LogIO), _image(image->cloneII()), _order(Vector<Int>()), _outputImage(outputImage) {
	LogOrigin origin(_class, String(__FUNCTION__) + "_2");
	*_log << origin;
	_construct();
	*_log << origin;
	Vector<String> orderCopy = order;
	_order = _image->coordinates().getWorldAxesOrder(orderCopy, True);
	*_log << "Old to new axis mapping is " << _order << LogIO::NORMAL;
}

ImageTransposer::ImageTransposer(
	const ImageInterface<Float> *const &image, uInt order, const String& outputImage
)
: _log(new LogIO), _image(image->cloneII()), _order(Vector<Int>()), _outputImage(outputImage) {
	LogOrigin origin(_class, String(__FUNCTION__) + "_3");
	*_log << origin;
	_construct();
	*_log << origin;
	_order = _getOrder(order);
}

ImageInterface<Float>* ImageTransposer::transpose() const {
	*_log << LogOrigin(_class, __FUNCTION__);
	// get the image data
	Array<Float> dataCopy = _image->get();

	CoordinateSystem csys = _image->coordinates();
	CoordinateSystem newCsys = csys;
	newCsys.transpose(_order, _order);
	IPosition shape = _image->shape();
	IPosition newShape(_order.size());
	for (uInt i=0; i<newShape.size(); i++) {
		newShape[i] = shape[_order[i]];
	}
	std::auto_ptr<ImageInterface<Float> > output(
		new TempImage<Float>(TiledShape(newShape), newCsys)
	);
	output->put(reorderArray(dataCopy, _order));
	if (_image->hasPixelMask()) {
		std::auto_ptr<Lattice<Bool> > maskLattice(
			_image->pixelMask().clone()
		);
		Array<Bool> maskCopy = maskLattice->get();
		dynamic_cast<TempImage<Float> *>(output.get())->attachMask(
			ArrayLattice<Bool>(reorderArray(maskCopy, _order))
		);
	}
	ImageUtilities::copyMiscellaneous(*output, *_image);
	if (! _outputImage.empty()) {
		ImageAnalysis ia(output.get());
		Record empty;
		output.reset(
			ia.subimage(_outputImage, empty, "", False, False)
		);
	}
	return output.release();
}

ImageTransposer::~ImageTransposer() {}

void ImageTransposer::_construct() {
	*_log << LogOrigin(_class, __FUNCTION__);
	if (! _outputImage.empty()) {
		File outputImageFile(_outputImage);
		switch(outputImageFile.getWriteStatus()) {
		case File::CREATABLE:
			// handle just to avoid compiler warning
			break;
		case File::OVERWRITABLE:
			// fall through to NOT_OVERWRITABLE
			// FIXME add caller specified Bool overwrite flag
		case File::NOT_OVERWRITABLE:
			*_log << "Requested output image " << _outputImage
			<< " already exists and will not be overwritten" << LogIO::EXCEPTION;
		case File::NOT_CREATABLE:
			*_log << "Requested output image " << _outputImage
			<< " cannot be created. Perhaps a permissions issue?" << LogIO::EXCEPTION;
		}
	}
}

Vector<Int> ImageTransposer::_getOrder(uInt order) const {
	*_log << LogOrigin(_class, String(__FUNCTION__));
	uInt naxes = _image->ndim();
	uInt raxes = uInt(log10(order)) + 1;
	if (naxes != raxes) {
		istringstream is;
		is >> order;
		if (! String(is.str()).contains("0")) {
			raxes++;
		}
	}
	if (raxes != naxes) {
		*_log << "Image has " << naxes << " axes but " << raxes
				<< " were given for reordering. Number of axes to reorder must match the number of image axes"
				<< LogIO::EXCEPTION;
	}
	if (raxes > 10) {
		*_log << "Only images with less than 10 axes can currently be reordered. This image has "
				<< naxes << " axes" << LogIO::EXCEPTION;
	}
	Vector<Int> myorder(naxes);
	uInt mag = 1;
	for (uInt i=1; i<myorder.size(); i++) {
		mag *= 10;
	}
	uInt scratchOrder = order;
	for (uInt i=0; i<myorder.size(); i++) {
		uInt index = scratchOrder/mag;
		if (index >= naxes) {
			*_log << "Image does not contain zero-based axis number " << index
					<< " but this was incorrectly specified in order parameter. "
					<< order << " All digits in the order parameter must be greater "
					<< "than or equal to zero and less than the number of image axes."
					<< LogIO::EXCEPTION;
		}
		for (uInt j=0; j<i; j++) {
			if ((Int)index == myorder[j]) {
				cout << "*** blah!" << endl;
				*_log << "Axis number " << index
						<< " specified multiple times in order parameter "
						<< order << " . It can only be specified once."
						<< LogIO::EXCEPTION;
			}
		}
		myorder[i] = index;
		scratchOrder -= index*mag;
		mag /= 10;
	}
	return myorder;
}

Vector<Int> ImageTransposer::_getOrder(const String& order) const {
	return _getOrder(String::toInt(order));
}

}
