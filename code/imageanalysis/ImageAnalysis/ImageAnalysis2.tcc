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

#include <imageanalysis/ImageAnalysis/ImageFFT.h>
#include <imageanalysis/ImageAnalysis/ImageMaskAttacher.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/LatticeUtilities.h>

namespace casa {

template<class T> void ImageAnalysis::_calc(
    SHARED_PTR<ImageInterface<T> > image,
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

template<class T> void ImageAnalysis::_destruct(ImageInterface<T>& image) {
	if((image.isPersistent()) && ((image.imageType()) == "PagedImage")) {
		ImageOpener::ImageTypes type = ImageOpener::imageType(image.name());
		if (type == ImageOpener::AIPSPP) {
			Table::relinquishAutoLocks(True);
			(static_cast<PagedImage<T>& >(image)).table().unlock();
		}
	}
}

} // end of  casa namespace
