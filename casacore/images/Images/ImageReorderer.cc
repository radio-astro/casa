/*
 * ImageReorderer.cc
 *
 *  Created on: May 7, 2010
 *      Author: dmehring
 */

#include <images/Images/FITSImage.h>
#include <images/Images/ImageReorderer.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/MIRIADImage.h>

namespace casa {

ImageReorderer::ImageReorderer(
	const String& imagename, const String& order, const String& outputImage
)
: _image(0), _order(IPosition()), _outputImage(outputImage) {
	_log = new LogIO();
	*_log << LogOrigin("ImageReorderer", "constructor");
	if (imagename.empty()) {
		*_log << "imagename cannot be blank" << LogIO::EXCEPTION;
	}
	if (_outputImage.empty()) {
		*_log << "output imagename cannot be blank" << LogIO::EXCEPTION;
	}
	File outputImageFile(_outputImage);
	switch(outputImageFile.getWriteStatus()) {
	case File::OVERWRITABLE:
		// fall through
	case File::NOT_OVERWRITABLE:
		*_log << "Requested output image " << _outputImage
			<< " already exists and will not be overwritten" << LogIO::EXCEPTION;
	case File::NOT_CREATABLE:
		*_log << "Requested ouptut image " << _outputImage
			<< " cannot be created. Perhaps a permissions issue?" << LogIO::EXCEPTION;
	}
	// Register the functions to create a FITSImage or MIRIADImage object.
	FITSImage::registerOpenFunction();
	MIRIADImage::registerOpenFunction();
	try {
		ImageUtilities::openImage(_image, imagename, *_log);
	}
	catch(AipsError err) {
		if (_image == 0) {
			throw(AipsError("Unable to open image '" + imagename + "'"));
		}
		RETHROW(err);
	}
	uInt intOrder = String::toInt(order);
	uInt naxes = _image->ndim();
	uInt raxes = uInt(log10(intOrder)) + 1;
	Bool firstZero = order.at(0,1) == "0";
	if (firstZero) {
		// first axis is axis number 0
		raxes++;
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
	_order = IPosition(naxes);
	uInt mag = 1;
	for (uInt i=1; i<_order.size(); i++) {
		mag *= 10;
	}
	for (uInt i=0; i<_order.size(); i++) {
		uInt index = intOrder/mag;
		if (index >= naxes) {
			*_log << "Image does not contained zero-based axis number " << index
				<< " but this was incorrectly specified in order parameter. "
				<< order << " All digits in the order parameter must be greater "
				<< "than or equal to zero and less than the number of image axes."
				<< LogIO::EXCEPTION;
		}
		for (uInt j=0; j<i; j++) {
			if (index == _order[j]) {
				*_log << "Axis number " << index
					<< " specified multiple times in order parameter "
					<< order << " . It can only be specified once."
					<< LogIO::EXCEPTION;
			}
		}
		_order[i] = index;
		intOrder -= index*mag;
		mag /= 10;
	}
}

PagedImage<Float>* ImageReorderer::reorder() const {
	// get the image data
	Array<Float> dataCopy;
	_image->get(dataCopy);
	reorderArray(dataCopy, _order);
	CoordinateSystem csys = _image->coordinates();
	CoordinateSystem newCsys = csys;
	Vector<Int> orderVector = _order.asVector();
	newCsys.transpose(orderVector, orderVector);
	IPosition shape = _image->shape();
	IPosition newShape(_order.size());
	for (uInt i=0; i<newShape.size(); i++) {
		newShape[i] = shape[_order[i]];
	}
	PagedImage<Float>* output = new PagedImage<Float>(TiledShape(newShape), newCsys, _outputImage);
	output->put(reorderArray(dataCopy, _order));
	return output;
}



ImageReorderer::~ImageReorderer() {
	delete _image;
	delete _log;
}

}
