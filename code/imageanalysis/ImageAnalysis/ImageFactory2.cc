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

#include <imageanalysis/ImageAnalysis/ImageFactory.h>

#include <images/Images/ImageFITSConverter.h>
#include <images/Images/ImageUtilities.h>
#include <imageanalysis/IO/CasaImageOpener.h>

namespace casa {

SPIIF ImageFactory::floatImageFromShape(
		const String& outfile, const Vector<Int>& shape,
		const Record& csys, Bool linear,
		Bool overwrite, Bool verbose,
		const vector<std::pair<LogOrigin, String> > *const &msgs
) {
	return _fromShape<Float>(
			outfile, shape, csys, linear,
			overwrite, verbose, msgs
	);
}

SPIIC ImageFactory::complexImageFromShape(
		const String& outfile, const Vector<Int>& shape,
		const Record& csys, Bool linear,
		Bool overwrite, Bool verbose,
		const vector<std::pair<LogOrigin, String> > *const &msgs
) {
	return _fromShape<Complex>(
			outfile, shape, csys, linear,
			overwrite, verbose, msgs
	);
}

void ImageFactory::_centerRefPix(
	CoordinateSystem& csys, const IPosition& shape
) {
	Int after = -1;
	Int iS = csys.findCoordinate(Coordinate::STOKES, after);
	Int sP = -1;
	if (iS >= 0) {
		Vector<Int> pixelAxes = csys.pixelAxes(iS);
		sP = pixelAxes(0);
	}
	Vector<Double> refPix = csys.referencePixel();
	for (Int i = 0; i < Int(refPix.nelements()); i++) {
		if (i != sP)
			refPix(i) = Double(shape(i) / 2);
	}
	csys.setReferencePixel(refPix);
}

CoordinateSystem* ImageFactory::_makeCoordinateSystem(
    const Record& coordinates, const IPosition& shape
) {
    std::unique_ptr<CoordinateSystem> pCS;
    if (coordinates.nfields() == 1) { 
        // must be a record as an element
        Record tmp(coordinates.asRecord(RecordFieldId(0)));
        pCS.reset(CoordinateSystem::restore(tmp, ""));
    } 
    else {
        pCS.reset(CoordinateSystem::restore(coordinates, ""));
    }
    // Fix up any body longitude ranges...
    String errMsg;
    ThrowIf(
        ! CoordinateUtil::cylindricalFix(*pCS, errMsg, shape),
        errMsg
    ); 
    return pCS.release();
}

SHARED_PTR<TempImage<Complex> > ImageFactory::complexFromFloat(
	SPCIIF realPart, const Array<Float>& imagPart
) {
	SHARED_PTR<TempImage<Complex> > newImage(
		new TempImage<Complex>(
			TiledShape(realPart->shape()),
			realPart->coordinates()
		)
	);
	{
		Array<Bool> mymask = realPart->getMask();
		if (realPart->hasPixelMask()) {
			mymask = mymask && realPart->pixelMask().get();
		}
		if (! allTrue(mymask)) {
			newImage->attachMask(ArrayLattice<Bool>(mymask));
		}
	}
	ImageUtilities::copyMiscellaneous(*newImage, *realPart);
	newImage->put(makeComplex(realPart->get(), imagPart));
	return newImage;
}

SHARED_PTR<TempImage<Float> > ImageFactory::floatFromComplex(
	SPCIIC complexImage, ComplexToFloatFunction function
) {
	SHARED_PTR<TempImage<Float> > newImage(
		new TempImage<Float>(
			TiledShape(complexImage->shape()),
			complexImage->coordinates()
		)
	);
	{
		Array<Bool> mymask = complexImage->getMask();
		if (complexImage->hasPixelMask()) {
			mymask = mymask && complexImage->pixelMask().get();
		}
		if (! allTrue(mymask)) {
			newImage->attachMask(ArrayLattice<Bool>(mymask));
		}
	}
	ImageUtilities::copyMiscellaneous(*newImage, *complexImage);
	switch (function) {
	case REAL:
		newImage->put(real(complexImage->get()));
		break;
	case IMAG:
		newImage->put(imag(complexImage->get()));
		break;
	default:
		ThrowCc("Logic Error: Unhandled function");
	}
	return newImage;
}

pair<SPIIF, SPIIC> ImageFactory::fromFile(const String& infile) {
    ThrowIf(
        infile.empty(), "File name is empty"
    );
    File thefile(infile);
    ThrowIf(
        ! thefile.exists(),
        "File " + infile + " does not exist."
    );
    unique_ptr<LatticeBase> latt(CasaImageOpener::openImage(infile));
    ThrowIf (! latt, "Unable to open lattice");
    DataType dataType = latt->dataType();
    pair<SPIIF, SPIIC> ret(nullptr, nullptr);
    if (isReal(dataType)) {
        if (dataType != TpFloat) {
            ostringstream os;
            os << dataType;
            LogIO log;
            log << LogOrigin(className(), __func__);
            log << LogIO::WARN << "Converting " << os.str() << " precision pixel values "
                << "to float precision in CASA image" << LogIO::POST;
        }
        return pair<SPIIF, SPIIC>(
            SPIIF(dynamic_cast<ImageInterface<Float> *>(latt.release())),
            SPIIC(nullptr)
        );
    }
    else if (isComplex(dataType)) {
        if (dataType != TpComplex) {
            ostringstream os;
            os << dataType;
            LogIO log;
            log << LogOrigin(className(), __func__);
            log << LogIO::WARN << "Converting " << os.str() << " precision pixel values "
                << "to complex float precision in CASA image" << LogIO::POST;
        }
        return pair<SPIIF, SPIIC>(
            SPIIF(nullptr),
            SPIIC(dynamic_cast<ImageInterface<Complex> *>(latt.release()))
        );
    }
    ostringstream os;
    os << dataType;
    throw AipsError("unsupported image data type " + os.str());
}

SPIIF ImageFactory::fromFITS(
    const String& outfile, const String& fitsfile,
    const Int whichrep, const Int whichhdu,
    const Bool zeroBlanks, const Bool overwrite
) {
    _checkOutfile(outfile, overwrite);
    ThrowIf(
        whichrep < 0,
        "The Coordinate Representation index must be non-negative"
    );
    ImageInterface<Float> *x = nullptr;
    String error;
    Bool rval = ImageFITSConverter::FITSToImage(
        x, error, outfile, fitsfile, whichrep, whichhdu,
        HostInfo::memoryFree() / 1024, overwrite, zeroBlanks
    );
    SPIIF pOut(x);
    ThrowIf(! rval || ! pOut, error);
    return pOut;
}

void ImageFactory::_checkOutfile(const String& outfile, Bool overwrite) {
    if (! overwrite && ! outfile.empty()) {
        NewFile validfile;
        String errmsg;
        ThrowIf(
            ! validfile.valueOK(outfile, errmsg), errmsg
        );
    }
}

}

