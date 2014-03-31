#include <image_cmpt.h>

#include <iostream>
#include <sys/wait.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/fstream.h>
#include <casa/IO/STLIO.h>
#include <casa/Logging/LogFilter.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/OS/Directory.h>
#include <casa/OS/EnvVar.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Utilities/Assert.h>
#include <components/ComponentModels/SkyCompRep.h>

#include <images/Images/ImageConcat.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImageExprParse.h>
#include <images/Images/ImageFITSConverter.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageStatistics.h>
#include <images/Images/ImageSummary.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/LELImageCoord.h>
#include <images/Images/PagedImage.h>
#include <images/Images/RebinImage.h>
#include <images/Images/TempImage.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/WCLELMask.h>
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
#include <measures/Measures/Stokes.h>
#include <scimath/Fitting/LinearFitSVD.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/VectorKernel.h>
#include <tables/LogTables/NewFile.h>

#include <components/SpectralComponents/SpectralListFactory.h>

#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/ImageBoxcarSmoother.h>
#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/ImageAnalysis/ImageCropper.h>
#include <imageanalysis/ImageAnalysis/ImageDecimator.h>
#include <imageanalysis/ImageAnalysis/ImageFactory.h>
#include <imageanalysis/ImageAnalysis/ImageFFTer.h>
#include <imageanalysis/ImageAnalysis/ImageFitter.h>
#include <imageanalysis/ImageAnalysis/ImageHanningSmoother.h>
#include <imageanalysis/ImageAnalysis/ImageHistory.h>
#include <imageanalysis/ImageAnalysis/ImageMaskedPixelReplacer.h>
#include <imageanalysis/ImageAnalysis/ImagePadder.h>
#include <imageanalysis/ImageAnalysis/ImageProfileFitter.h>
#include <imageanalysis/ImageAnalysis/ImagePrimaryBeamCorrector.h>
#include <imageanalysis/ImageAnalysis/ImageRebinner.h>
#include <imageanalysis/ImageAnalysis/ImageRegridder.h>
#include <imageanalysis/ImageAnalysis/ImageStatsCalculator.h>
#include <imageanalysis/ImageAnalysis/ImageTransposer.h>
#include <imageanalysis/ImageAnalysis/PeakIntensityFluxDensityConverter.h>
#include <imageanalysis/ImageAnalysis/PVGenerator.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

#include <stdcasa/version.h>

#include <casa/namespace.h>

#include <memory>
#include <tr1/memory>

#include <stdcasa/cboost_foreach.h>
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;
using namespace std;

#define _ORIGIN LogOrigin(_class, __func__, WHERE)

namespace casac {

const String image::_class = "image";

image::image() :
_log(), _image(new ImageAnalysis()) {}

// private ImageInterface constructor for on the fly components
// The constructed object will take over management of the provided pointer
// using a shared_ptr

image::image(casa::ImageInterface<casa::Float> *inImage) :
	_log(), _image(new ImageAnalysis(std::tr1::shared_ptr<ImageInterface<Float> >(inImage))) {
	try {
		_log << _ORIGIN;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::image(ImageInterface<Complex> *inImage) :
	_log(), _image(new ImageAnalysis(std::tr1::shared_ptr<ImageInterface<Complex> >(inImage))) {
	try {
		_log << _ORIGIN;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::image(SPIIF inImage) :
	_log(), _image(new ImageAnalysis(inImage)) {
	try {
		_log << _ORIGIN;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::image(SPIIC inImage) :
	_log(), _image(new ImageAnalysis(inImage)) {
	try {
		_log << _ORIGIN;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::image(std::tr1::shared_ptr<ImageAnalysis> ia) :
	_log(), _image(ia) {
	try {
		_log << _ORIGIN;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image::~image() {}

Bool isunset(const ::casac::variant &theVar) {
	Bool rstat(False);
	if ((theVar.type() == ::casac::variant::BOOLVEC) && (theVar.size() == 0))
		rstat = True;
	return rstat;
}

::casac::record*
image::torecord() {
	::casac::record *rstat = new ::casac::record();
	_log << LogOrigin("image", "torecord");
	if (detached())
		return rstat;
	try {
		Record rec;
		if (!_image->toRecord(rec)) {
			_log << "Could not convert to record " << LogIO::EXCEPTION;
		}

		// Put it in a ::casac::record
		delete rstat;
		rstat = 0;
		rstat = fromRecord(rec);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;

}
bool image::fromrecord(const record& imrecord, const string& outfile) {
	try {
		_log << _ORIGIN;
		auto_ptr<Record> tmpRecord(toRecord(imrecord));
		if (_image.get() == 0) {
			_image.reset(new ImageAnalysis());
		}
		if (
			! _image->fromRecord(*tmpRecord, casa::String(outfile))
		) {
			_log << "Failed to create a valid image from this record"
					<< LogIO::EXCEPTION;
		}
		return True;
	} catch (AipsError x) {
		RETHROW(x);
	}
}

bool image::addnoise(const std::string& type, const std::vector<double>& pars,
		const ::casac::record& region, const bool zeroIt) {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return False;
		}

		Record *pRegion = toRecord(region);
		_image->addnoise(type, pars, *pRegion, zeroIt);

		_stats.reset(0);
		return True;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

// FIXME need to support region records as input
image * image::collapse(
	const string& function, const variant& axes,
	const string& outfile, const variant& region, const string& box,
	const string& chans, const string& stokes, const string& mask,
	const bool overwrite, const bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		return 0;
	}
	try {
		IPosition myAxes;
		if (axes.type() == variant::INT) {
			myAxes = IPosition(1, axes.toInt());
		}
		else if (axes.type() == variant::INTVEC) {
			myAxes = IPosition(axes.getIntVec());
		}
		else if (
			axes.type() == variant::STRINGVEC
			|| axes.type() == variant::STRING
		) {
			Vector<String> axVec = (axes.type() == variant::STRING)
				? Vector<String> (1, axes.getString())
				: toVectorString(axes.toStringVec());
			myAxes = IPosition(
				_image->isFloat()
				? _image->getImage()->coordinates().getWorldAxesOrder(
					axVec, False
				)
				: _image->getComplexImage()->coordinates().getWorldAxesOrder(
					axVec, False
				)
			);
			for (
				IPosition::iterator iter = myAxes.begin();
				iter != myAxes.end(); iter++
			) {
				ThrowIf(
					*iter < 0,
					"At least one specified axis does not exist"
				);
			}
		}
		else {
			ThrowCc("Unsupported type for parameter axes");
		}
		String regStr = region.type() == variant::STRING ? region.toString() : "";
		std::auto_ptr<Record> regRec(
			region.type() == variant::RECORD
			? toRecord(region.getRecord()) : 0
		);
		String aggString = function;
		aggString.trim();
		aggString.downcase();
		if (aggString == "avdev") {
			_log << "avdev currently not supported. Let us know if you have a need for it"
				<< LogIO::EXCEPTION;
		}
		if (_image->isFloat()) {
			ImageCollapser<Float> collapser(
				aggString, _image->getImage(), regStr, regRec.get(), box,
				chans, stokes, mask, myAxes, outfile, overwrite
			);
			collapser.setStretch(stretch);
			return new image(collapser.collapse(True));
		}
		else {
			ImageCollapser<casa::Complex> collapser(
				aggString, _image->getComplexImage(), regStr, regRec.get(), box,
				chans, stokes, mask, myAxes, outfile, overwrite
			);
			collapser.setStretch(stretch);
			return new image(collapser.collapse(True));
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::imagecalc(
	const string& outfile, const string& pixels,
	const bool overwrite
) {
	try {
		std::tr1::shared_ptr<ImageAnalysis> ia(new ImageAnalysis());
        ia->imagecalc(outfile, pixels, overwrite);
        return  new image(ia);
	}
	catch (const AipsError& x) {
		RETHROW(x);
	}
}

image* image::imageconcat(
	const string& outfile, const variant& infiles,
	const int axis, const bool relax, const bool tempclose,
	const bool overwrite
) {
	try {
		image *rstat(0);
		Vector<String> inFiles;
		if (infiles.type() == variant::BOOLVEC) {
			inFiles.resize(0); // unset
		}
		else if (infiles.type() == variant::STRING) {
			sepCommaEmptyToVectorStrings(inFiles, infiles.toString());
		}
		else if (infiles.type() == variant::STRINGVEC) {
			inFiles = toVectorString(infiles.toStringVec());
		}
		else {
			_log << "Unrecognized infiles datatype" << LogIO::EXCEPTION;
		}
		ImageAnalysis ia;
		rstat = new image(
			ia.imageconcat(
				outfile, inFiles, axis,	relax, tempclose, overwrite
			)
		);
		if(!rstat)
			throw AipsError("Unable to create image");
		return rstat;
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fromarray(const std::string& outfile,
		const ::casac::variant& pixels, const ::casac::record& csys,
		const bool linear, const bool overwrite, const bool log) {
	try {
		_reset();

		_log << _ORIGIN;

		Vector<Int> shape = pixels.arrayshape();
		uInt ndim = shape.size();
		if (ndim == 0) {
			_log << "The pixels array is empty" << LogIO::EXCEPTION;
		}
		for (uInt i = 0; i < ndim; i++) {
			if (shape(i) <= 0) {
				_log << "The shape of the pixels array is invalid"
						<< LogIO::EXCEPTION;
			}
		}

		Array<Float> pixelsArray;
		if (pixels.type() == variant::DOUBLEVEC) {
			std::vector<double> pixelVector = pixels.getDoubleVec();
			Vector<Int> shape = pixels.arrayshape();
			pixelsArray.resize(IPosition(shape));
			Vector<Double> localpix(pixelVector);
			convertArray(pixelsArray, localpix.reform(IPosition(shape)));
		} else if (pixels.type() == ::casac::variant::INTVEC) {
			vector<int> pixelVector = pixels.getIntVec();
			Vector<Int> shape = pixels.arrayshape();
			pixelsArray.resize(IPosition(shape));
			Vector<Int> localpix(pixelVector);
			convertArray(pixelsArray, localpix.reform(IPosition(shape)));
		} else {
			_log << "pixels is not understood, try using an array "
					<< LogIO::EXCEPTION;
			return False;
		}

		auto_ptr<Record> coordinates(toRecord(csys));
		if (
			_image->imagefromarray(
				outfile, pixelsArray, *coordinates,
				linear, overwrite, log
			)
		) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error creating image from array");
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

void image::_reset() {
	_image.reset(new ImageAnalysis());
	//_imageFloat.reset();
	//_imageComplex.reset();
	_stats.reset(0);
}

bool image::fromascii(const string& outfile, const string& infile,
		const vector<int>& shape, const string& sep, const record& csys,
		const bool linear, const bool overwrite) {
	try {
		_log << _ORIGIN;

		if (infile == "") {
			_log << LogIO::SEVERE << "infile must be specified" << LogIO::POST;
			return false;
		}
		if (shape.size() == 1 && shape[0] == -1) {
			_log << LogIO::SEVERE << "Image shape must be specified"
					<< LogIO::POST;
			return false;
		}

		_reset();
		auto_ptr<Record> coordsys(toRecord(csys));
		if (
			_image->imagefromascii(
				outfile, infile, Vector<Int> (shape),
				sep, *coordsys, linear, overwrite
			)
		) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error creating image from ascii.");
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fromfits(const std::string& outfile, const std::string& fitsfile,
		const int whichrep, const int whichhdu, const bool zeroBlanks,
		const bool overwrite) {
	try {
		_reset();
		_log << _ORIGIN;
		return _image->imagefromfits(outfile, fitsfile, whichrep, whichhdu,
				zeroBlanks, overwrite);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fromimage(const string& outfile, const string& infile,
		const record& region, const variant& mask, const bool dropdeg,
		const bool overwrite) {
	try {
		_reset();

		_log << _ORIGIN;
		String theMask;
		if (mask.type() == variant::BOOLVEC) {
			theMask = "";
		} else if (mask.type() == variant::STRING) {
			theMask = mask.toString();
		} else if (mask.type() == variant::RECORD) {
			_log << LogIO::SEVERE
					<< "Don't support region masking yet, only valid LEL "
					<< LogIO::POST;
			return False;
		} else {
			_log << LogIO::SEVERE
					<< "Mask is not understood, try a valid LEL string "
					<< LogIO::POST;
			return False;
		}

		auto_ptr<Record> regionPtr(toRecord(region));
		return _image->imagefromimage(outfile, infile, *regionPtr, theMask,
				dropdeg, overwrite);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fromshape(
	const string& outfile, const vector<int>& shape,
	const record& csys, const bool linear, const bool overwrite,
	const bool log, const string& type
) {
	try {
		LogOrigin lor("image", __func__);
		_log << lor;
		_reset();
		auto_ptr<Record> coordinates(toRecord(csys));
        String mytype = type;
        mytype.downcase();
        ThrowIf(
            ! mytype.startsWith("f") && ! mytype.startsWith("c"),
            "Input parm type must start with either 'f' or 'c'"
        );
		vector<std::pair<LogOrigin, String> > msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__;
			msgs.push_back(make_pair(lor, os.str()));
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("shape", shape));
			inputs.push_back(make_pair("csys", csys));
			inputs.push_back(make_pair("linear", linear));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("log", log));
			inputs.push_back(make_pair("type", type));
			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(make_pair(lor, os.str()));
		}
        if (mytype.startsWith("f")) {
            SPIIF myfloat = ImageFactory::floatImageFromShape(
                outfile, shape, *coordinates,
                linear, overwrite, log, &msgs
            );
            _image.reset(new ImageAnalysis(myfloat));
        }
        else {
            SPIIC mycomplex = ImageFactory::complexImageFromShape(
                outfile, shape, *coordinates,
                linear, overwrite, log, &msgs
            );
            _image.reset(new ImageAnalysis(mycomplex));
        }
       	return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::image *
image::adddegaxes(
	const std::string& outfile, bool direction,
	bool spectral, const std::string& stokes, bool linear,
	bool tabular, bool overwrite, bool silent
) {
	try {
        _log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		if (_image->isFloat()) {
			SPCIIF image = _image->getImage();
			return _adddegaxes(
				image,
				outfile, direction, spectral, stokes, linear,
				tabular, overwrite, silent
			);
		}
		else {
			SPCIIC image = _image->getComplexImage();
			return _adddegaxes(
				image,
				outfile, direction, spectral, stokes,
				linear, tabular, overwrite, silent
			);
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

template<class T> image* image::_adddegaxes(
	SPCIIT inImage,
	const std::string& outfile, bool direction,
		bool spectral, const std::string& stokes, bool linear,
		bool tabular, bool overwrite, bool silent
) {
	_log << _ORIGIN;
	PtrHolder<ImageInterface<T> > outimage;
	ImageUtilities::addDegenerateAxes(
		_log, outimage, *inImage, outfile,
		direction, spectral, stokes, linear, tabular, overwrite, silent
	);
	ImageInterface<T> *outPtr = outimage.ptr();
	outimage.clear(False);
	return new image(outPtr);
}

::casac::image* image::convolve(
	const string& outFile, const variant& kernel,
	const double in_scale, const record& region,
	const variant& vmask, const bool overwrite,
	const bool stretch, const bool async
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}

		Array<Float> kernelArray;
		String kernelFileName = "";
		if (kernel.type() == variant::DOUBLEVEC) {
			vector<double> kernelVector = kernel.toDoubleVec();
			Vector<Int> shape = kernel.arrayshape();
			kernelArray.resize(IPosition(shape));
			Vector<Double> localkern(kernelVector);
			convertArray(kernelArray, localkern.reform(IPosition(shape)));
		}
		else if (kernel.type() == variant::INTVEC) {
			vector<int> kernelVector = kernel.toIntVec();
			Vector<Int> shape = kernel.arrayshape();
			kernelArray.resize(IPosition(shape));
			Vector<Int> localkern(kernelVector);
			convertArray(kernelArray, localkern.reform(IPosition(shape)));
		}
		else if (
			kernel.type() == variant::STRING
			|| kernel.type() == variant::STRINGVEC
		) {
			kernelFileName = kernel.toString();
		}
		else {
			_log << "kernel is not understood, try using an array or an image "
				<< LogIO::EXCEPTION;
		}

		String theMask;
		Record *theMaskRegion;
		if (vmask.type() == variant::BOOLVEC) {
			theMask = "";
		}
		else if (
			vmask.type() == variant::STRING
			|| vmask.type() == variant::STRINGVEC
		) {
			theMask = vmask.toString();
		}
		else if (vmask.type() == variant::RECORD) {
			variant localvar(vmask);
			theMaskRegion = toRecord(localvar.asRecord());
			_log << "Don't support region masking yet, only valid LEL "
				<< LogIO::EXCEPTION;
		}
		else {
			_log << "Mask is not understood, try a valid LEL string "
				<< LogIO::EXCEPTION;
		}

		Record *Region = toRecord(region);
		return new image(
			_image->convolve(
				outFile, kernelArray,
				kernelFileName, in_scale, *Region,
				theMask, overwrite, async, stretch
			)
		);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::boundingbox(const record& region) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		Record *Region = toRecord(region);
		return fromRecord(*_image->boundingbox(*Region));
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::boxcar(
	const string& outfile, const variant& region,
	const variant& vmask, int axis, int width, bool drop,
	const string& dmethod,
	bool overwrite, bool stretch
) {
	LogOrigin lor(_class, __func__);
	_log << lor;
	if (detached()) {
		throw AipsError("Unable to create image");
	}
	try {
		std::tr1::shared_ptr<const Record> myregion = _getRegion(
			region, True
		);
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (axis < 0) {
			const CoordinateSystem csys = _image->isFloat()
				? _image->getImage()->coordinates()
				: _image->getComplexImage()->coordinates();
			ThrowIf(
				! csys.hasSpectralAxis(),
				"Axis not specified and image has no spectral coordinate"
			);
			axis = csys.spectralAxisNumber(False);
		}
        ImageDecimatorData::Function dFunction = ImageDecimatorData::NFUNCS;
        if (drop) {
            String mymethod = dmethod;
            mymethod.downcase();
            if (mymethod.startsWith("m")) {
                dFunction = ImageDecimatorData::MEAN;
            }
            else if (mymethod.startsWith("c")) {
                dFunction = ImageDecimatorData::COPY;
            }
            else {
                ThrowCc(
                    "Value of dmethod must be "
                    "either 'm'(ean) or 'c'(opy)"
                );
            }
        }
		vector<String> msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__ << "() on image " << _name();
			msgs.push_back(os.str());
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("region", region));
			inputs.push_back(make_pair("mask", vmask));
			inputs.push_back(make_pair("axis", axis));
			inputs.push_back(make_pair("width", width));
			inputs.push_back(make_pair("drop", drop));
			inputs.push_back(make_pair("dmethod", dmethod));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("stretch", stretch));
			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(os.str());
		}
		if (_image->isFloat()) {
			SPCIIF image = _image->getImage();
			return _boxcar(
				image, myregion, mask, outfile,
				overwrite, stretch, axis, width, drop,
				dFunction, lor, msgs
			);
		}
		else {
			SPCIIC image = _image->getComplexImage();
			return _boxcar(
				image, myregion, mask, outfile,
				overwrite, stretch, axis, width, drop,
				dFunction, lor, msgs
			);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

template <class T> image* image::_boxcar(
	SPCIIT myimage, std::tr1::shared_ptr<const Record> region,
	const String& mask, const string& outfile, bool overwrite,
	bool stretch, int axis, int width, bool drop,
	ImageDecimatorData::Function dFunction, const LogOrigin& lor,
	const vector<String> msgs
) {
	ImageBoxcarSmoother<T> smoother(
		myimage, region.get(), mask, outfile, overwrite
	);
	smoother.setAxis(axis);
	smoother.setDecimate(drop);
	smoother.setStretch(stretch);
	if (drop) {
		smoother.setDecimationFunction(dFunction);
	}
	smoother.setWidth(width);
	smoother.addHistory(lor, msgs);
	return new image(smoother.smooth());
}

std::string image::brightnessunit() {
	std::string rstat;
	try {
		_log << _ORIGIN;
		if (!detached()) {
			rstat = _image->brightnessunit();
		} else {
			rstat = "";
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::calc(const std::string& expr) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return False;
		}
		_image->calc(expr);
		_stats.reset(0);
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::calcmask(
	const string& mask, const string& maskName, bool makeDefault
) {
	bool rstat(false);
	try {
		_log << _ORIGIN;
		if (detached()) {
			return rstat;
		}
		Record region;
		rstat = _image->calcmask(mask, region, maskName, makeDefault);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::close() {
	try {
		_log << _ORIGIN;
		_image.reset();
		_stats.reset(0);
		return True;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::continuumsub(
	const string& outline, const string& outcont,
	const record& region, const vector<int>& channels,
	const string& pol, const int in_fitorder, const bool overwrite
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		Record * leRegion = toRecord(region);
		Vector<Int> theChannels(channels);
		if (theChannels.size() == 1 && theChannels[0] == -1) {
			theChannels.resize(0);
		}
		std::tr1::shared_ptr<ImageInterface<Float> > theResid(
			_image->continuumsub(
				outline, outcont, *leRegion, theChannels,
				pol, in_fitorder, overwrite
			)
		);
		if (!theResid) {
			_log << "continuumsub failed " << LogIO::EXCEPTION;
		}
		return new ::casac::image(theResid);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::convertflux(
	const variant& qvalue, const variant& major,
	const variant& minor,  const string& /*type*/,
	const bool toPeak,
	const int channel, const int polarization
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		casa::Quantity value = casaQuantity(qvalue);
		casa::Quantity majorAxis = casaQuantity(major);
		casa::Quantity minorAxis = casaQuantity(minor);
		Bool noBeam = False;
		PeakIntensityFluxDensityConverter converter(_image->getImage());
		converter.setSize(
			Angular2DGaussian(majorAxis, minorAxis, casa::Quantity(0, "deg"))
		);
		converter.setBeam(channel, polarization);
		return recordFromQuantity(
			toPeak
			? converter.fluxDensityToPeakIntensity(noBeam, value)
			: converter.peakIntensityToFluxDensity(noBeam, value)
		);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: "
			<< x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

image* image::convolve2d(
	const string& outFile, const vector<int>& axes,
	const string& type, const variant& major, const variant& minor,
	const variant& pa, const double in_scale, const record& region,
	const variant& vmask, const bool overwrite, const bool stretch,
	const bool targetres, const record& beam
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			throw AipsError("Unable to create image");
		}
		UnitMap::putUser("pix", UnitVal(1.0), "pixel units");
		Record *Region = toRecord(region);
		String mask = vmask.toString();

		if (mask == "[]") {
			mask = "";
		}
		String kernel(type);
		casa::Quantity majorKernel;
		casa::Quantity minorKernel;
		casa::Quantity paKernel;
		_log << _ORIGIN;
		if (! beam.empty()) {
			if (! String(type).startsWith("g") && ! String(type).startsWith("G")) {
				_log << "beam can only be given with a gaussian kernel" << LogIO::EXCEPTION;
			}
			if (
				! major.toString(False).empty()
				|| ! minor.toString(False).empty()
				|| ! pa.toString(False).empty()
			) {
				_log << "major, minor, and/or pa may not be specified if beam is specified"
					<< LogIO::EXCEPTION;
			}
			if (beam.size() != 3) {
				_log << "If given, beam must have exactly three fields"
					<< LogIO::EXCEPTION;
			}
			if (beam.find("major") == beam.end()) {
				_log << "Beam must have a 'major' field" << LogIO::EXCEPTION;
			}
			if (beam.find("minor") == beam.end()) {
				_log << "Beam must have a 'minor' field" << LogIO::EXCEPTION;
			}
			if (
				beam.find("positionangle") == beam.end()
				&& beam.find("pa") == beam.end()) {
				_log << "Beam must have a 'positionangle' or 'pa' field" << LogIO::EXCEPTION;
			}
			std::auto_ptr<Record> nbeam(toRecord(beam));

			for (uInt i=0; i<3; i++) {
				String key = i == 0
					? "major"
					: i == 1
					    ? "minor"
					    : beam.find("pa") == beam.end()
					        ? "positionangle"
					        : "pa";
				casa::Quantity x;
				DataType type = nbeam->dataType(nbeam->fieldNumber(key));
				String err;
				QuantumHolder z;
				Bool success;
				if (type == TpString) {
					success = z.fromString(err, nbeam->asString(key));
				}
				else if (type == TpRecord) {
					success = z.fromRecord(err, nbeam->asRecord(key));
				}
				else {
					throw AipsError("Unsupported data type for beam");
				}
				if (! success) {
					throw AipsError("Error converting beam to Quantity");
				}
				if (key == "major") {
					majorKernel = z.asQuantity();
				}
				else if (key == "minor") {
					minorKernel = z.asQuantity();
				}
				else {
					paKernel = z.asQuantity();
				}
			}
		}
		else {
			majorKernel = _casaQuantityFromVar(major);
			minorKernel = _casaQuantityFromVar(minor);
			paKernel = _casaQuantityFromVar(pa);
		}
		_log << _ORIGIN;

		Vector<Int> Axes(axes);
		if (Axes.size() == 0) {
			Axes.resize(2);
			Axes[0] = 0;
			Axes[1] = 1;
		}
		return new image(
			_image->convolve2d(
				outFile, Axes, type, majorKernel, minorKernel,
				paKernel, in_scale, *Region, mask, overwrite,
				stretch, targetres
			)
		);

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: "
			<< x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

::casac::coordsys *
image::coordsys(const std::vector<int>& pixelAxes) {
	::casac::coordsys *rstat = 0;
	_log << _ORIGIN;

	try {
		if (detached())
			return rstat;

		// Return coordsys object
		rstat = new ::casac::coordsys();
		CoordinateSystem csys = _image->coordsys(Vector<Int> (pixelAxes));
		rstat->setcoordsys(csys);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

image* image::decimate(
	const string& outfile, int axis, int factor, const string& method,
	const variant& region, const string& mask, bool overwrite, bool stretch
) {
	ThrowIf(
		detached(),
		"Tool is not attached to an image"
	);
	try {
		ThrowIf(
			axis < 0,
			"The value of axis cannot be negative"
		);
		ThrowIf(
			factor < 0,
			"The value of factor cannot be negative"
		);
		String mymethod = method;
		mymethod.downcase();
		ImageDecimatorData::Function f;
		if (mymethod.startsWith("c")) {
			f = ImageDecimatorData::COPY;
		}
		else if (mymethod.startsWith("m")) {
			f = ImageDecimatorData::MEAN;
		}
		else {
			ThrowCc("Unsupported decimation method " + method);
		}
		std::tr1::shared_ptr<Record> regPtr(_getRegion(region, True));
		vector<String> msgs;
		{
			msgs.push_back("Ran ia.decimate() on image " + _name());
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("axis", axis));
			inputs.push_back(make_pair("factor", factor));
			inputs.push_back(make_pair("method", method));
			inputs.push_back(make_pair("region", region));
			inputs.push_back(make_pair("mask", mask));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("stretch", stretch));
			msgs.push_back("ia.decimate" + _inputsString(inputs));
		}
		if (_image->isFloat()) {
			SPCIIF myim = _image->getImage();
			return _decimate(
				myim, outfile, axis, factor, f,
				regPtr, mask, overwrite, stretch, msgs
			);
		}
		else {
			SPCIIC myim = _image->getComplexImage();
			return _decimate(
				myim, outfile, axis, factor, f,
				regPtr, mask, overwrite, stretch, msgs
			);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

String image::_inputsString(const vector<std::pair<String, variant> >& inputs) {
	String out = "(";
	String quote;
	vector<std::pair<String, variant> >::const_iterator begin = inputs.begin();
	vector<std::pair<String, variant> >::const_iterator iter = begin;
	vector<std::pair<String, variant> >::const_iterator end = inputs.end();
	while (iter != end) {
		if (iter != begin) {
			out += ", ";
		}
		quote = iter->second.type() == variant::STRING ? "'" : "";
		out += iter->first + "=" + quote;
		out += iter->second.toString();
		out += quote;
		iter++;
	}
	out += ")";
	return out;
}

template <class T> image* image::_decimate(
	const SPCIIT myimage,
	const string& outfile, int axis, int factor,
	ImageDecimatorData::Function f,
	const std::tr1::shared_ptr<Record> region,
	const string& mask, bool overwrite, bool stretch,
	const vector<String>& msgs
) const {
	ImageDecimator<T> decimator(
		myimage, region.get(),
			mask, outfile, overwrite
	);
	decimator.setFunction(f);
	decimator.setAxis(axis);
	decimator.setFactor(factor);
	decimator.setStretch(stretch);
	decimator.addHistory(
		LogOrigin("image", __func__), msgs
	);
	SPIIT out = decimator.decimate();
	return new image(out);
}

::casac::record*
image::coordmeasures(const std::vector<double>&pixel) {
	::casac::record *rstat = 0;
	try {
		_log << _ORIGIN;
		if (detached())
			return rstat;

		casa::Record theDir;
		casa::Record theFreq;
		casa::Record theVel;
		casa::Record* retval;
		casa::Quantity theInt;
		Vector<Double> vpixel;
		if (!(pixel.size() == 1 && pixel[0] == -1)) {
			vpixel = pixel;
		}
		retval = _image->coordmeasures(theInt, theDir, theFreq, theVel, vpixel);

		String error;
		Record R;
		if (QuantumHolder(theInt).toRecord(error, R)) {
			retval->defineRecord(RecordFieldId("intensity"), R);
		} else {
			_log << LogIO::SEVERE << "Could not convert intensity to record. "
					<< error << LogIO::POST;
		}
		rstat = fromRecord(*retval);

	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

::casac::record*
image::decompose(const ::casac::record& region, const ::casac::variant& vmask,
	const bool simple, const double Threshold, const int nContour,
	const int minRange, const int nAxis, const bool fit,
	const double maxrms, const int maxRetry, const int maxIter,
	const double convCriteria, const bool stretch
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		if (Threshold < 0) {
			throw AipsError(
				"Threshold = " + String::toString(Threshold)
				+ ". You must specify a nonnegative threshold"
			);
		}
		Record *Region = toRecord(region);
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Matrix<Int> blcs;
		Matrix<Int> trcs;

		Matrix<Float> cl = _image->decompose(
			blcs, trcs, *Region, mask, simple, Threshold,
			nContour, minRange, nAxis, fit, maxrms,
			maxRetry, maxIter, convCriteria, stretch
		);

		casa::Record outrec1;
		outrec1.define("components", cl);
		outrec1.define("blc", blcs);
		outrec1.define("trc", trcs);
		return fromRecord(outrec1);

	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::deconvolvecomponentlist(
	const record& complist, const int channel, const int polarization
) {
	_log << _ORIGIN;
	if (detached()) {
		return 0;
	}
	try {
		std::auto_ptr<Record> compList(toRecord(complist));
		return fromRecord(
			_image->deconvolvecomponentlist(
				*compList, channel, polarization
			)
		);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::deconvolvefrombeam(
	const ::casac::variant& source,
	const ::casac::variant& beam
) {

	try {
		_log << _ORIGIN;
		Vector<casa::Quantity> sourceParam, beamParam;
		Angular2DGaussian mySource;
		if (
			! toCasaVectorQuantity(source, sourceParam)
			|| (sourceParam.nelements() == 0)
			|| sourceParam.nelements() > 3
		) {
			throw(AipsError("Cannot understand source values"));
		}
		else {
			if (sourceParam.nelements() == 1) {
				sourceParam.resize(3, True);
				sourceParam[1] = sourceParam[0];
				sourceParam[2] = casa::Quantity(0, "deg");
			}
			else if (sourceParam.nelements() == 2) {
				sourceParam.resize(3, True);
				sourceParam[2] = casa::Quantity(0, "deg");
			}
			mySource = Angular2DGaussian(
				sourceParam[0], sourceParam[1], sourceParam[2]
			);
		}
		if (
			! toCasaVectorQuantity(beam, beamParam)
			|| (beamParam.nelements() == 0)) {
			throw(AipsError("Cannot understand beam values"));
		}
		else {
			if (beamParam.nelements() == 1) {
				beamParam.resize(3, True);
				beamParam[1] = beamParam[0];
				beamParam[2] = casa::Quantity(0.0, "deg");
			}
			if (beamParam.nelements() == 2) {
				beamParam.resize(3, True);
				beamParam[2] = casa::Quantity(0.0, "deg");
			}
		}
		GaussianBeam myBeam(beamParam[0], beamParam[1], beamParam[2]);
		Bool success = False;
		Angular2DGaussian decon;
		Bool retval = False;
		try {
			retval = myBeam.deconvolve(decon, mySource);
			success = True;
		}
		catch (const AipsError& x) {
			retval = False;
			success = False;
		}
		Record deconval = decon.toRecord();
		deconval.defineRecord("pa", deconval.asRecord("positionangle"));
		deconval.removeField("positionangle");
		deconval.define("success", success);
		Record outrec1;
		outrec1.define("return", retval);
		outrec1.defineRecord("fit", deconval);
		return fromRecord(outrec1);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}


record* image::beamforconvolvedsize(
	const variant& source, const variant& convolved
) {

	try {
		_log << _ORIGIN;
		Vector<casa::Quantity> sourceParam, convolvedParam;
		if (
			! toCasaVectorQuantity(source, sourceParam)
			|| sourceParam.size() != 3
		) {
			throw(AipsError("Cannot understand source values"));
		}
		if (
			! toCasaVectorQuantity(convolved, convolvedParam)
			&& convolvedParam.size() != 3
		) {
			throw(AipsError("Cannot understand target values"));
		}
		Angular2DGaussian mySource(sourceParam[0], sourceParam[1], sourceParam[2]);
		GaussianBeam myConvolved(convolvedParam[0], convolvedParam[1], convolvedParam[2]);
		GaussianBeam neededBeam;
		try {
			if (mySource.deconvolve(neededBeam, myConvolved)) {
				// throw without a message here, it will be caught
				// in the associated catch block and a new error will
				// be thrown with the appropriate message.
				throw AipsError();
			}
		}
		catch (const AipsError& x) {
			ostringstream os;
			os << "Unable to reach target resolution of "
				<< myConvolved << " Input source "
				<< mySource << " is probably too large.";
			throw AipsError(os.str());
		}
		Record ret;
		QuantumHolder qh(neededBeam.getMajor());
		ret.defineRecord("major", qh.toRecord());
		qh = QuantumHolder(neededBeam.getMinor());
		ret.defineRecord("minor", qh.toRecord());
		qh = QuantumHolder(neededBeam.getPA());
		ret.defineRecord("pa", qh.toRecord());
		return fromRecord(ret);
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::commonbeam() {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		ImageInfo myInfo = _image->getImage()->imageInfo();
		if (! myInfo.hasBeam()) {
			throw AipsError("This image has no beam(s).");
		}
		GaussianBeam beam;
		if (myInfo.hasSingleBeam()) {
			_log << LogIO::WARN
				<< "This image only has one beam, so just returning that"
				<< LogIO::POST;
			beam = myInfo.restoringBeam();

		}
		else {
			// multiple beams in this image
			beam = myInfo.getBeamSet().getCommonBeam();
		}
		beam.setPA(casa::Quantity(beam.getPA("deg", True), "deg"));
		Record x = beam.toRecord();
		x.defineRecord("pa", x.asRecord("positionangle"));
		x.removeField("positionangle");
		return fromRecord(x);

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}

}

bool image::remove(const bool finished, const bool verbose) {
	try {
		_log << _ORIGIN;

		if (detached()) {
			return False;
		}
		_stats.reset(0);

		if (_image->remove(verbose)) {
			// Now done the image tool if desired.
			if (finished) {
				done();
			}
			return True;
		}
		throw AipsError("Error removing image.");
	} catch (const AipsError &x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::removefile(const std::string& filename) {
	bool rstat(false);
	try {
		_log << LogOrigin("image", "removefile");

		String fileName(filename);
		if (fileName.empty()) {
			_log << LogIO::WARN << "Empty filename" << LogIO::POST;
			return rstat;
		}
		File f(fileName);
		if (!f.exists()) {
			_log << LogIO::WARN << fileName << " does not exist."
					<< LogIO::POST;
			return rstat;
		}

		// Now try and blow it away.  If it's open, tabledelete won't delete it.
		String message;
		if (Table::canDeleteTable(message, fileName, True)) {
			Table::deleteTable(fileName, True);
			rstat = true;
		} else {
			_log << LogIO::WARN << "Cannot delete file " << fileName
					<< " because " << message << LogIO::POST;
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::done(const bool remove, const bool verbose) {
	try {
		_log << _ORIGIN;
		// resetting _stats must come before the table removal or the table
		// removal will fail
		_stats.reset(0);

		if (remove && !detached()) {
			if (!_image->remove(verbose)) {
				_log << LogIO::WARN << "Failed to remove image file"
						<< LogIO::POST;
			}
		}
		_image.reset();
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::fft(
	const string& realOut, const string& imagOut,
	const string& ampOut, const string& phaseOut,
	const std::vector<int>& axes, const record& region,
	const variant& vmask, const bool stretch,
	const string& complexOut
) {
	try {
		_log << LogOrigin(_class, __func__);
		if (detached()) {
			return false;
		}

		std::tr1::shared_ptr<Record> myregion(toRecord(region));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Vector<uInt> leAxes(0);
		if (
			axes.size() > 1
			|| (axes.size() == 1 && axes[0] >= 0)
		) {
			leAxes.resize(axes.size());
			for (uInt i=0; i<axes.size(); i++) {
				ThrowIf(
					axes[i] < 0,
					"None of the elements of axes may be less than zero"
				);
				leAxes[i] = axes[i];
			}
		}
		if (_image->isFloat()) {
			ImageFFTer<Float> ffter(
				_image->getImage(),
				myregion.get(), mask, leAxes
			);
			ffter.setStretch(stretch);
			ffter.setReal(realOut);
			ffter.setImag(imagOut);
			ffter.setAmp(ampOut);
			ffter.setPhase(phaseOut);
			ffter.setComplex(complexOut);
			ffter.fft();
		}
		else {
			ImageFFTer<Complex> ffter(
				_image->getComplexImage(),
				myregion.get(), mask, leAxes
			);
			ffter.setStretch(stretch);
			ffter.setReal(realOut);
			ffter.setImag(imagOut);
			ffter.setAmp(ampOut);
			ffter.setPhase(phaseOut);
			ffter.setComplex(complexOut);
			ffter.fft();
		}
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::findsources(const int nMax, const double cutoff,
		const ::casac::record& region, const ::casac::variant& vmask,
		const bool point, const int width, const bool absFind) {
	::casac::record *rstat = 0;
	try {
		_log << LogOrigin("image", "findsources");
		if (detached())
			return rstat;

		Record *Region = toRecord(region);
		String mask = vmask.toString();
		if (mask == "[]")
			mask = "";

		Record listOut = _image->findsources(nMax, cutoff, *Region, mask,
				point, width, absFind);
		rstat = fromRecord(listOut);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

record* image::fitprofile(const string& box, const variant& region,
	const string& chans, const string& stokes, const int axis,
	const variant& vmask, int ngauss, const int poly,
	const string& estimates, const int minpts, const bool multifit,
	const string& model, const string& residual, const string& amp,
	const string& amperr, const string& center, const string& centererr,
	const string& fwhm, const string& fwhmerr,
	const string& integral, const string& integralerr, const bool stretch,
	const bool logResults, const variant& pampest,
	const variant& pcenterest, const variant& pfwhmest,
	const variant& pfix, const variant& gmncomps,
    const variant& gmampcon, const variant& gmcentercon,
    const variant& gmfwhmcon, const vector<double>& gmampest,
    const vector<double>& gmcenterest, const vector<double>& gmfwhmest,
    const variant& gmfix, const string& spxtype, const vector<double>& spxest,
    const vector<bool>& spxfix, const variant& div, const string& spxsol,
    const string& spxerr, const string& logfile,
    const bool append, const variant& pfunc,
    const vector<double>& goodamprange,
    const vector<double>& goodcenterrange,
    const vector<double>& goodfwhmrange, const variant& sigma,
    const string& outsigma
) {
	_log << LogOrigin(_class, __func__);
	if (detached()) {
		return 0;
	}
	try {
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		String regionName;
		std::tr1::shared_ptr<Record> regionPtr = _getRegion(region, True);
		if (ngauss < 0) {
			_log << LogIO::WARN
				<< "ngauss < 0 is meaningless. Setting ngauss = 0 "
				<< LogIO::POST;
			ngauss = 0;
		}
		vector<double> mygoodamps = toVectorDouble(goodamprange, "goodamprange");
		if (mygoodamps.size() > 2) {
			_log << "Too many elements in goodamprange" << LogIO::EXCEPTION;
		}
		vector<double> mygoodcenters = toVectorDouble(goodcenterrange, "goodcenterrange");
		if (mygoodcenters.size() > 2) {
			_log << "Too many elements in goodcenterrange" << LogIO::EXCEPTION;
		}
		vector<double> mygoodfwhms = toVectorDouble(goodfwhmrange, "goodcenterrange");
		if (mygoodfwhms.size() > 2) {
			_log << "Too many elements in goodfwhmrange" << LogIO::EXCEPTION;
		}
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		std::auto_ptr<Array<Float> > sigmaArray(0);
		std::auto_ptr<PagedImage<Float> > sigmaImage(0);
		if (sigma.type() == variant::STRING) {
			String sigmaName = sigma.toString();
			if (! sigmaName.empty()) {
				sigmaImage.reset(new PagedImage<Float>(sigmaName));
			}
		}
		else if (
			sigma.type() == variant::DOUBLEVEC
			|| sigma.type() == variant::INTVEC
		) {
			sigmaArray.reset(new Array<Float>());
			vector<double> sigmaVector = sigma.getDoubleVec();
			Vector<Int> shape = sigma.arrayshape();
			sigmaArray->resize(IPosition(shape));
			convertArray(
				*sigmaArray,
				Vector<Double>(sigmaVector).reform(IPosition(shape))
			);
		}
		else if (sigma.type() == variant::BOOLVEC) {
			// nothing to do
		}
		else {
			_log << LogIO::SEVERE
				<< "Unrecognized type for sigma. Use either a string (image name) or a numpy array"
				<< LogIO::POST;
			return 0;
		}
		String myspxtype;
		vector<double> plpest, ltpest;
		vector<bool> plpfix, ltpfix;
		if (! spxtype.empty()) {
			myspxtype = String(spxtype);
			myspxtype.downcase();
			if (myspxtype == "plp") {
				plpest = spxest;
				plpfix = spxfix;
			}
			else if (myspxtype == "ltp") {
				ltpest = spxest;
				ltpfix = spxfix;
			}
			else {
				throw AipsError("Unsupported value for spxtype");
			}
		}
		SpectralList spectralList = SpectralListFactory::create(
			_log, pampest, pcenterest, pfwhmest, pfix, gmncomps,
			gmampcon, gmcentercon, gmfwhmcon, gmampest,
			gmcenterest, gmfwhmest, gmfix, pfunc, plpest, plpfix,
			ltpest, ltpfix
		);
		if (! estimates.empty() && spectralList.nelements() > 0) {
			_log << "You cannot specify both an "
				<< "estimates file and set estimates "
				<< "directly. You may only do one or "
				<< "the either (or neither in which "
				<< "case you must specify ngauss and/or poly)"
				<< LogIO::EXCEPTION;
		}
		ImageProfileFitter fitter(
			_image->getImage(), regionName, regionPtr.get(),
			box, chans, stokes, mask, axis,
			ngauss, estimates, spectralList
		);
		fitter.setDoMultiFit(multifit);
		if (poly >= 0) {
			fitter.setPolyOrder(poly);
		}
		fitter.setModel(model);
		fitter.setResidual(residual);
		fitter.setAmpName(amp);
		fitter.setAmpErrName(amperr);
		fitter.setCenterName(center);
		fitter.setCenterErrName(centererr);
		fitter.setFWHMName(fwhm);
		fitter.setFWHMErrName(fwhmerr);
		fitter.setIntegralName(integral);
		fitter.setIntegralErrName(integralerr);
		fitter.setMinGoodPoints(minpts > 0 ? minpts : 0);
		fitter.setStretch(stretch);
		fitter.setLogResults(logResults);
		if (! logfile.empty()) {
			fitter.setLogfile(logfile);
			fitter.setLogfileAppend(append);
		}
		if (mygoodamps.size() == 2) {
			fitter.setGoodAmpRange(mygoodamps[0], mygoodamps[1]);
		}
		if (mygoodcenters.size() == 2) {
			fitter.setGoodCenterRange(mygoodcenters[0], mygoodcenters[1]);
		}
		if (mygoodfwhms.size() == 2) {
			fitter.setGoodFWHMRange(mygoodfwhms[0], mygoodfwhms[1]);
		}
		if (sigmaImage.get()) {
			fitter.setSigma(sigmaImage.get());
		}
		else if (sigmaArray.get()) {
			fitter.setSigma(*sigmaArray);
		}
		if (! outsigma.empty()) {
			if (sigmaImage.get() || sigmaArray.get()) {
				fitter.setOutputSigmaImage(outsigma);
			}
			else {
				_log << LogIO::WARN
					<< "outsigma specified but no sigma image "
					<< "or array specified. outsigma will be ignored"
					<< LogIO::POST;
			}
		}
		if (plpest.size() > 0 || ltpest.size() > 0) {
			variant::TYPE t = div.type();
			if (div.type() == variant::BOOLVEC) {
				fitter.setAbscissaDivisor(0);
			}
			else if (t == variant::INT || t == variant::DOUBLE) {
				fitter.setAbscissaDivisor(div.toDouble());
			}
			else if (t == variant::STRING || t == variant::RECORD) {
				fitter.setAbscissaDivisor(casaQuantity(div));
			}
			else {
				throw AipsError("Unsupported type " + div.typeString() + " for div");
			}
			if (! myspxtype.empty()) {
				if (myspxtype == "plp") {
					fitter.setPLPName(spxsol);
					fitter.setPLPErrName(spxerr);
				}
				else if (myspxtype == "ltp") {
					fitter.setLTPName(spxsol);
					fitter.setLTPErrName(spxerr);
				}
			}
		}
		return fromRecord(fitter.fit());
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::transpose(
	const std::string& outfile,
	const variant& order
) {
	try {
		_log << LogOrigin("image", __func__);

		if (detached()) {
			throw AipsError("No image specified to transpose");
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		std::auto_ptr<ImageTransposer> transposer(0);
		switch(order.type()) {
		case variant::INT:
			transposer.reset(
				new ImageTransposer(
					_image->getImage(),
					order.toInt(), outfile
				)
			);
			break;
		case variant::STRING:
			transposer.reset(
				new ImageTransposer(
					_image->getImage(),
					order.toString(), outfile
				)
			);
			break;
		case variant::STRINGVEC:
			{
				Vector<String> orderVec = toVectorString(order.toStringVec());
				transposer.reset(
					new ImageTransposer(
						_image->getImage(), orderVec,
						outfile
					)
				);
			}
			break;
		default:
			_log << "Unsupported type for order parameter " << order.type()
					<< ". Supported types are a non-negative integer, a single "
					<< "string containing all digits or a list of strings which "
					<< "unambiguously match the image axis names."
					<< LogIO::EXCEPTION;
		}
		
		image *rstat =new image(
			transposer->transpose()
		);
		if(!rstat)
			throw AipsError("Unable to transpose image");
		return rstat;
	} catch (const AipsError& x) {
		RETHROW(x);
	}
}

::casac::record* image::fitcomponents(
		const string& box, const variant& region, const variant& chans,
		const string& stokes, const variant& vmask,
		const vector<double>& in_includepix,
		const vector<double>& in_excludepix, const string& residual,
		const string& model, const string& estimates,
		const string& logfile, const bool append,
		const string& newestimates, const string& complist,
		bool overwrite, bool dooff, double offset,
		bool fixoffset, bool stretch, const variant& rms,
		const variant& noisefwhm
) {
	if (detached()) {
		return 0;
	}
	LogOrigin lor(_class, __func__);
	_log << lor;

	try {
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		int num = in_includepix.size();
		Vector<Float> includepix(num);
		num = in_excludepix.size();
		Vector<Float> excludepix(num);
		convertArray(includepix, Vector<Double> (in_includepix));
		convertArray(excludepix, Vector<Double> (in_excludepix));
		if (includepix.size() == 1 && includepix[0] == -1) {
			includepix.resize();
		}
		if (excludepix.size() == 1 && excludepix[0] == -1) {
			excludepix.resize();
		}
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
        SPCIIF image = _image->getImage();
		ImageFitterResults::CompListWriteControl writeControl = complist.empty()
			? ImageFitterResults::NO_WRITE
			: overwrite
				? ImageFitterResults::OVERWRITE
				: ImageFitterResults::WRITE_NO_REPLACE;
		String sChans;
		if (chans.type() == variant::BOOLVEC) {
			// for some reason which eludes me, the default variant type is boolvec
			sChans = "";
		}
		else if (chans.type() == variant::STRING) {
			sChans = chans.toString();
		}
		else if (chans.type() == variant::INT) {
			sChans = String::toString(chans.toInt());
		}
		else {
			ThrowCc(
				"Unsupported type for chans. chans must "
				"be either an integer or a string"
			);
		}
		std::tr1::shared_ptr<Record> regionRecord = _getRegion(region, True);
		vector<String> msgs;
		Bool doImages = ! residual.empty() || ! model.empty();

		ImageFitter fitter(
			image, "", regionRecord.get(), box, sChans,
			stokes, mask, estimates, newestimates, complist
		);
		if (includepix.size() == 1) {
			fitter.setIncludePixelRange(
				std::make_pair<Float, Float>(includepix[0],includepix[0])
			);
		}
		else if (includepix.size() == 2) {
			fitter.setIncludePixelRange(
				std::make_pair<Float, Float>(includepix[0],includepix[1])
			);
		}
		if (excludepix.size() == 1) {
			fitter.setExcludePixelRange(
				std::make_pair<Float, Float>(excludepix[0],excludepix[0])
			);
		}
		else if (excludepix.size() == 2) {
			fitter.setExcludePixelRange(
				std::make_pair<Float, Float>(excludepix[0],excludepix[1])
			);
		}
		fitter.setWriteControl(writeControl);
		fitter.setStretch(stretch);
		fitter.setModel(model);
		fitter.setResidual(residual);
		if (! logfile.empty()) {
			fitter.setLogfile(logfile);
			fitter.setLogfileAppend(append);
		}
		if (dooff) {
			fitter.setZeroLevelEstimate(offset, fixoffset);
		}
		casa::Quantity myrms = (rms.type() == variant::DOUBLE || rms.type() == variant::INT)
			? casa::Quantity(rms.toDouble(), _image->brightnessunit())
			: _casaQuantityFromVar(rms);
		if (myrms.getValue() > 0) {
			fitter.setRMS(myrms);
		}
		variant::TYPE noiseType = noisefwhm.type();
		if (noiseType == variant::DOUBLE || noiseType == variant::INT) {
			fitter.setNoiseFWHM(noisefwhm.toDouble());
		}
		else if (noiseType == variant::BOOLVEC) {
			fitter.clearNoiseFWHM();
		}
		else if (
			noiseType == variant::STRING || noiseType == variant::RECORD
		) {
			if (noiseType == variant::STRING && noisefwhm.toString().empty()) {
				fitter.clearNoiseFWHM();
			}
			else {
				fitter.setNoiseFWHM(_casaQuantityFromVar(noisefwhm));
			}
		}
		else {
			ThrowCc(
				"Unsupported data type for noisefwhm: " + noisefwhm.typeString()
			);
		}
		if (doImages) {
			std::vector<String> names;
			names += "box", "region", "chans", "stokes", "mask", "includepix",
				"excludepix", "residual", "model", "estimates", "logfile",
				"append", "newestimates", "complist", "dooff", "offset",
				"fixoffset", "stretch", "rms", "noisefwhm";
			std::vector<variant> values;
			values += box, region, chans, stokes, vmask, in_includepix, in_excludepix,
				residual, model, estimates, logfile, append, newestimates, complist,
				dooff, offset, fixoffset, stretch, rms, noisefwhm;
			String fname = String("ia.") + String(__func__);
			fitter.addHistory(lor, fname, names, values);
		}
		std::pair<ComponentList, ComponentList> compLists = fitter.fit();
		return fromRecord(fitter.getOutputRecord());

	}
	catch (const AipsError& x) {
		FluxRep<Double>::clearAllowedUnits();
		_log << "Exception Reported: " << x.getMesg()
			<< LogIO::EXCEPTION;
		RETHROW(x);
	}
}

variant* image::getchunk(
	const std::vector<int>& blc, const std::vector<int>& trc,
	const std::vector<int>& inc, const std::vector<int>& axes,
	bool list, bool dropdeg, bool getmask
) {
	try {

		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		if (_image->isFloat()) {
			return _getchunk<Float, double>(
				blc, trc, inc, axes, list, dropdeg, getmask
			);
		}
		else {
			return _getchunk<Complex, std::complex<double> > (
				blc, trc, inc, axes, list, dropdeg, getmask
			);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

template<class T, class U> casac::variant* image::_getchunk(
	const vector<int>& blc, const vector<int>& trc,
	const vector<int>& inc, const vector<int>& axes,
	bool list, bool dropdeg, bool getmask
) {
	Array<T> pixels;
	Array<Bool> pixelMask;
	Vector<Int> iaxes(axes);
	// if default value change it to empty vector
	if (iaxes.size() == 1 && iaxes[0] < 0) {
		iaxes.resize();
	}
	_image->getchunk(
		pixels, pixelMask, Vector<Int>(blc), Vector<Int>(trc),
		Vector<Int>(inc), iaxes, list, dropdeg, getmask
	);
	std::vector<int> s_shape;
	if (getmask) {
		std::vector<bool> s_pixelmask;
		pixelMask.tovector(s_pixelmask);
		pixelMask.shape().asVector().tovector(s_shape);
		return new variant(s_pixelmask, s_shape);
	}
	else {
		pixels.shape().asVector().tovector(s_shape);
		std::vector<U> d_pixels;
		foreach_(U v, pixels) {
			d_pixels.push_back(v);
		}
		return new variant(d_pixels, s_shape);
	}
}

image* image::pbcor(
	const variant& pbimage, const string& outfile,
	const bool overwrite, const string& box,
	const variant& region, const string& chans,
	const string& stokes, const string& mask,
	const string& mode, const float cutoff,
	const bool stretch
) {
	if (detached()) {
		throw AipsError("Unable to create image");
		return 0;
	}
	try {
		_log << _ORIGIN;
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		Array<Float> pbPixels;
        SPCIIF pb_ptr;
		if (pbimage.type() == variant::DOUBLEVEC) {
			Vector<Int> shape = pbimage.arrayshape();
			pbPixels.resize(IPosition(shape));
			Vector<Double> localpix(pbimage.getDoubleVec());
			casa::convertArray(pbPixels, localpix.reform(IPosition(shape)));
		}
		else if (pbimage.type() == variant::STRING) {
            ImageInterface<Float>* pb;
            ImageUtilities::openImage(pb, pbimage.getString());
			if (pb == 0) {
				_log << "Unable to open primary beam image " << pbimage.getString()
					<< LogIO::EXCEPTION;
			}
            pb_ptr.reset(pb);
		}
		else {
			_log << "Unsupported type " << pbimage.typeString()
				<< " for pbimage" << LogIO::EXCEPTION;
		}

		String regionString = "";
		auto_ptr<Record> regionRecord(0);
		if (region.type() == variant::STRING || region.size() == 0) {
			regionString = (region.size() == 0) ? "" : region.toString();
		}
		else if (region.type() == variant::RECORD) {
			regionRecord.reset(toRecord(region.clone()->asRecord()));
		}
		else {
			_log << "Unsupported type for region " << region.type()
				<< LogIO::EXCEPTION;
		}
		String modecopy = mode;
		modecopy.downcase();
		modecopy.trim();
		if (! modecopy.startsWith("d") && ! modecopy.startsWith("m")) {
			throw AipsError("Unknown mode " + mode);
		}
		ImagePrimaryBeamCorrector::Mode myMode = modecopy.startsWith("d")
			? ImagePrimaryBeamCorrector::DIVIDE
			: ImagePrimaryBeamCorrector::MULTIPLY;
		Bool useCutoff = cutoff >= 0.0;
        SPCIIF shImage = _image->getImage();
        std::auto_ptr<ImagePrimaryBeamCorrector> pbcor(
			(!pb_ptr)
			? new ImagePrimaryBeamCorrector(
				shImage, pbPixels, regionRecord.get(),
				regionString, box, chans, stokes, mask, outfile, overwrite,
				cutoff, useCutoff, myMode
			)
			: new ImagePrimaryBeamCorrector(
				shImage, pb_ptr, regionRecord.get(),
				regionString, box, chans, stokes, mask, outfile, overwrite,
				cutoff, useCutoff, myMode
			)
		);
		pbcor->setStretch(stretch);
        std::tr1::shared_ptr<ImageInterface<Float> > corrected(pbcor->correct(True));
		return new image(corrected);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::variant* image::getregion(
	const ::casac::record& region, const std::vector<int>& axes,
	const ::casac::variant& mask, const bool list, const bool dropdeg,
	const bool getmask, const bool stretch
) {
	// Recover some pixels and their mask from a region in the image
	try {
		_log << LogOrigin(_class, __func__);
		if (detached()) {
			return false;
		}
		Array<Float> pixels;
		Array<Bool> pixelmask;

		Record *Region = toRecord(region);
		String Mask;
		if (mask.type() == ::casac::variant::BOOLVEC) {
			Mask = "";
		}
		else if (
			mask.type() == ::casac::variant::STRING
			|| mask.type() == ::casac::variant::STRINGVEC
		) {
			Mask = mask.toString();
		}
		else {
			_log << LogIO::WARN
				<< "Only LEL string handled for mask...region is yet to come"
				<< LogIO::POST;
			Mask = "";
		}
		Vector<Int> iaxes(axes);
		// if default value change it to empty vector
		if (iaxes.size() == 1 && iaxes[0] < 0) {
			iaxes.resize();
		}
		_image->getregion(
			pixels, pixelmask, *Region, iaxes,
			Mask, list, dropdeg, getmask, stretch
		);
		if (getmask) {
			std::vector<bool> s_pixelmask;
			std::vector<int> s_shape;
			pixelmask.tovector(s_pixelmask);
			pixels.shape().asVector().tovector(s_shape);
			return new ::casac::variant(s_pixelmask, s_shape);
		}
		else {
			std::vector<int> s_shape;
			pixels.shape().asVector().tovector(s_shape);
			std::vector<double> d_pixels(pixels.nelements());
			int i(0);
			for (
				Array<Float>::iterator iter = pixels.begin();
				iter != pixels.end(); iter++
			) {
				d_pixels[i++] = *iter;
			}
			return new ::casac::variant(d_pixels, s_shape);
		}
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::getslice(const std::vector<double>& x, const std::vector<double>& y,
		const std::vector<int>& axes, const std::vector<int>& coord,
		const int npts, const std::string& method) {
	::casac::record *rstat = 0;
	try {
		_log << _ORIGIN;
		if (detached()) {
			return rstat;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		// handle default coord
		std::vector<int> ncoord(coord);
		if (ncoord.size() == 1 && ncoord[0] == -1) {
			int n = _image->getImage()->ndim();
			ncoord.resize(n);
			for (int i = 0; i < n; i++) {
				//ncoord[i]=i;
				ncoord[i] = 0;
			}
		}

		Record *outRec = _image->getslice(Vector<Double> (x),
				Vector<Double> (y), Vector<Int> (axes), Vector<Int> (ncoord),
				npts, method);
		rstat = fromRecord(*outRec);
		delete outRec;
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

image* image::hanning(
	const string& outfile, const variant& region,
	const variant& vmask, int axis, bool drop,
	bool overwrite, bool /* async */, bool stretch,
    const string& dmethod
) {
	LogOrigin lor(_class, __func__);
	_log << lor;
	if (detached()) {
		throw AipsError("Unable to create image");
	}
	try {
		std::tr1::shared_ptr<const Record> myregion = _getRegion(
			region, True
		);
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (axis < 0) {
			const CoordinateSystem csys = _image->isFloat()
				? _image->getImage()->coordinates()
				: _image->getComplexImage()->coordinates();
			ThrowIf(
				! csys.hasSpectralAxis(),
				"Axis not specified and image has no spectral coordinate"
			);
			axis = csys.spectralAxisNumber(False);
		}
        ImageDecimatorData::Function dFunction = ImageDecimatorData::NFUNCS;
        if (drop) {
            String mymethod = dmethod;
            mymethod.downcase();
            if (mymethod.startsWith("m")) {
                dFunction = ImageDecimatorData::MEAN;
            }
            else if (mymethod.startsWith("c")) {
                dFunction = ImageDecimatorData::COPY;
            }
            else {
                ThrowCc(
                    "Value of dmethod must be "
                    "either 'm'(ean) or 'c'(opy)"
                );
            }
        }
		vector<variant> values;
		values += outfile, region, vmask, axis, drop, overwrite, stretch, dmethod;
		if (_image->isFloat()) {
			SPCIIF image = _image->getImage();
			return _hanning(
				image, myregion, mask, outfile,
				overwrite, stretch, axis, drop,
				dFunction, values
			);
		}
		else {
			SPCIIC image = _image->getComplexImage();
			return _hanning(
				image, myregion, mask, outfile,
				overwrite, stretch, axis, drop,
				dFunction, values
			);
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

template <class T> image* image::_hanning(
	SPCIIT myimage, std::tr1::shared_ptr<const Record> region,
	const String& mask, const string& outfile, bool overwrite,
	bool stretch, int axis, bool drop,
	ImageDecimatorData::Function dFunction,
	const std::vector<casac::variant> values
) {
	ImageHanningSmoother<T> smoother(
		myimage, region.get(), mask, outfile, overwrite
	);
	smoother.setAxis(axis);
	smoother.setDecimate(drop);
	smoother.setStretch(stretch);
	if (drop) {
		smoother.setDecimationFunction(dFunction);
	}
	vector<String> names;
	names += "outfile", "region", "mask", "axis",
		"drop", "overwrite", "stretch", "dmethod";
	smoother.addHistory(
		LogOrigin(_class, __func__), "ia.hanning",
		names, values
	);
	return new image(smoother.smooth());
}

std::vector<bool> image::haslock() {
	std::vector<bool> rstat;
	try {
		_log << LogOrigin("image", __func__);
		if (detached())
			return rstat;

		_image->haslock().tovector(rstat);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

record* image::histograms(
	const vector<int>& axes,
	const record& region, const variant& mask,
	const int nbins, const vector<double>& includepix,
	const bool gauss, const bool cumu, const bool log, const bool list,
	const bool force, const bool disk,
	const bool /* async */, bool stretch
) {
	_log << LogOrigin(_class, __func__);
	if (detached()) {
		return 0;
	}
	try {
		std::auto_ptr<Record> regionRec(toRecord(region));
		String Mask;
		if (mask.type() == variant::BOOLVEC) {
			Mask = "";
		}
		else if (
			mask.type() == variant::STRING
			|| mask.type() == variant::STRINGVEC
		) {
			Mask = mask.toString();
		}
		else {
			_log << LogIO::WARN
					<< "Only LEL string handled for mask...region is yet to come"
					<< LogIO::POST;
			Mask = "";
		}
		Vector<Int> naxes;
		if (!(axes.size() == 1 && axes[0] == -1)) {
			naxes.resize(axes.size());
			naxes = Vector<Int> (axes);
		}
		Vector<Double> includePix;
		if (!(includepix.size() == 1 && includepix[0] == -1)) {
			includePix.resize(includepix.size());
			includePix = Vector<Double> (includepix);
		}
        return fromRecord(
        	_image->histograms(
        		naxes, *regionRec, Mask, nbins, includePix,
        		gauss, cumu, log, list, force, disk, stretch
        	)
        );
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

std::vector<std::string> image::history(bool list) {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return vector<string>();
		}
		if (_image->isFloat()) {
			SPIIF im = _image->getImage();
			ImageHistory<Float> hist(im);
			return fromVectorString(hist.get(list));
		}
		else {
			SPIIC im = _image->getComplexImage();
			ImageHistory<Complex> hist(im);
			return fromVectorString(hist.get(list));
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::insert(
	const std::string& infile, const ::casac::record& region,
	const std::vector<double>& locate, bool verbose
) {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return 0;
		}

		Vector<Double> locatePixel(locate);
		if (locatePixel.size() == 1 && locatePixel[0] < 0) {
			locatePixel.resize(0);
		}
		std::auto_ptr<Record> Region(toRecord(region));
		if (_image->insert(infile, *Region, locatePixel, verbose)) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error inserting image.");
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::isopen() {
	try {
		_log << _ORIGIN;

		if (_image.get() != 0 && !_image->detached()) {
			return True;
		} else {
			return False;
		}
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::ispersistent() {
	bool rstat(false);
	try {
		_log << LogOrigin("image", "ispersistent");
		if (detached())
			return rstat;

		rstat = _image->ispersistent();
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::lock(const bool writelock, const int nattempts) {
	bool rstat(false);
	try {
		_log << LogOrigin("image", "lock");
		if (detached())
			return rstat;
		rstat = _image->lock(writelock, nattempts);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::makecomplex(const std::string& outFile,
		const std::string& imagFile, const ::casac::record& region,
		const bool overwrite) {
	bool rstat(false);
	try {
		_log << LogOrigin("image", "makecomplex");
		if (detached())
			return rstat;

		Record *Region = toRecord(region);

		rstat = _image->makecomplex(outFile, imagFile, *Region, overwrite);
		delete Region;

	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

// bool image::makefloat(const std::string& outFile,
// 		      const std::string& compFile,
// 		      const std::string& op,
// 		      const bool overwrite) {
// 	bool rstat(false);
// 	try {
// 		_log << LogOrigin("image", "makefloat");

// 		rstat = ImageAnalysis::makeFloat(outFile, compFile, _log, op, overwrite);

// 	} catch (AipsError x) {
// 		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
// 				<< LogIO::POST;
// 		RETHROW(x);
// 	}
// 	return rstat;
// }

std::vector<std::string> image::maskhandler(const std::string& op,
		const std::vector<std::string>& name) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return std::vector<string>(0);
		}

		Vector<String> namesOut;
		Vector<String> namesIn = toVectorString(name);
		namesOut = _image->maskhandler(op, namesIn);
		if (namesOut.size() == 0) {
			namesOut.resize(1);
			namesOut[0] = "T";
		}
		std::vector<string> rstat = fromVectorString(namesOut);
		_stats.reset(0);
		return rstat;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::miscinfo() {
	::casac::record *rstat = 0;
	try {
		_log << LogOrigin("image", "miscinfo");
		if (detached())
			return rstat;

		rstat = fromRecord(_image->miscinfo());
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::modify(
	const ::casac::record& model, const ::casac::record& region,
	const ::casac::variant& vmask, const bool subtract, const bool list,
	const bool /* async */, bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		return false;
	}
	try {
		String error;
		std::auto_ptr<Record> Model(toRecord(model));
		std::auto_ptr<Record> Region(toRecord(region));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (
			_image->modify(
				*Model, *Region, mask, subtract,
				list, stretch
			)
		) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error modifying image.");
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::maxfit(const ::casac::record& region, const bool doPoint,
		const int width, const bool absFind, const bool list) {
	::casac::record *rstat = 0;
	try {
		_log << LogOrigin("image", "maxfit");
		if (detached())
			return rstat;

		Record *Region = toRecord(region);
		rstat = fromRecord(_image->maxfit(*Region, doPoint, width, absFind,
				list));

		delete Region;

	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

::casac::image *
image::moments(
	const std::vector<int>& moments, const int axis,
	const ::casac::record& region, const ::casac::variant& vmask,
	const std::vector<std::string>& in_method,
	const std::vector<int>& smoothaxes,
	const ::casac::variant& smoothtypes,
	const std::vector<double>& smoothwidths,
	const std::vector<double>& d_includepix,
	const std::vector<double>& d_excludepix, const double peaksnr,
	const double stddev, const std::string& velocityType,
	const std::string& out, const std::string& smoothout,
	const std::string& pgdevice, const int nx, const int ny,
	const bool yind, const bool overwrite, const bool removeAxis,
	const bool stretch, const bool /* async */
) {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return 0;
		}
		UnitMap::putUser("pix", UnitVal(1.0), "pixel units");
		Vector<Int> whichmoments(moments);
		std::auto_ptr<Record> Region(toRecord(region));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Vector<String> method = toVectorString(in_method);

		Vector<String> kernels;
		if (smoothtypes.type() == ::casac::variant::BOOLVEC) {
			kernels.resize(0); // unset
		}
		else if (smoothtypes.type() == ::casac::variant::STRING) {
			sepCommaEmptyToVectorStrings(kernels, smoothtypes.toString());
		}
		else if (smoothtypes.type() == ::casac::variant::STRINGVEC) {
			kernels = toVectorString(smoothtypes.toStringVec());
		}
		else {
			_log << LogIO::WARN << "Unrecognized smoothtypes datatype"
				<< LogIO::POST;
		}
		int num = kernels.size();

		Vector<Quantum<Double> > kernelwidths(num);
		Unit u("pix");
		for (int i = 0; i < num; i++) {
			kernelwidths[i] = casa::Quantity(smoothwidths[i], u);
		}
		Vector<Float> includepix;
		num = d_includepix.size();
		if (!(num == 1 && d_includepix[0] == -1)) {
			includepix.resize(num);
			for (int i = 0; i < num; i++)
				includepix[i] = d_includepix[i];
		}
		Vector<Float> excludepix;
		num = d_excludepix.size();
		if (!(num == 1 && d_excludepix[0] == -1)) {
			excludepix.resize(num);
			for (int i = 0; i < num; i++)
				excludepix[i] = d_excludepix[i];
		}
		std::tr1::shared_ptr<ImageInterface<Float> > outIm(
			_image->moments(
				whichmoments, axis,
				*Region, mask, method, Vector<Int> (smoothaxes), kernels,
				kernelwidths, includepix, excludepix, peaksnr, stddev,
				velocityType, out, smoothout, pgdevice, nx, ny, yind,
				overwrite, removeAxis, stretch
			)
		);
		return new ::casac::image(outIm);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

std::string image::name(const bool strippath) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return "none";
		}
		return _name(strippath);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

String image::_name(bool strippath) const {
	if (_image->isFloat()) {
		return _image->getImage()->name(strippath);
	}
	else {
		return _image->getComplexImage()->name(strippath);
	}
}

bool image::open(const std::string& infile) {
	try {
		_reset();

		_log << _ORIGIN;
        return _image->open(infile);

	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::pad(
	const string& outfile, int npixels, double value, bool padmask,
	bool overwrite, const variant& region, const string& box,
	const string& chans, const string& stokes, const string& mask,
	bool  stretch, bool wantreturn
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		if (npixels <= 0) {
			_log << "Value of npixels must be greater than zero" << LogIO::EXCEPTION;
		}
		std::tr1::shared_ptr<Record> regionPtr = _getRegion(region, True);

		ImagePadder padder(
			_image->getImage(), regionPtr.get(), box,
			chans, stokes, mask, outfile, overwrite
		);
		padder.setStretch(stretch);
		padder.setPaddingPixels(npixels, value, padmask);
		tr1::shared_ptr<ImageInterface<Float> > out(padder.pad(wantreturn));
		if (wantreturn) {
			return new image(out);
		}
		return 0;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::crop(
	const string& outfile, const vector<int>& axes,
	bool overwrite, const variant& region, const string& box,
	const string& chans, const string& stokes, const string& mask,
	bool  stretch, bool wantreturn
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
        if (axes.size() > 0) {
            std::set<int> saxes(axes.begin(), axes.end());
	    	if (*saxes.begin() < 0) {
			    _log << "All axes values must be >= 0" << LogIO::EXCEPTION;
		    }
        }
        std::set<uInt> saxes(axes.begin(), axes.end());
		std::tr1::shared_ptr<Record> regionPtr = _getRegion(region, True);

		ImageCropper<Float> cropper(
			_image->getImage(), regionPtr.get(), box,
			chans, stokes, mask, outfile, overwrite
		);
		cropper.setStretch(stretch);
        cropper.setAxes(saxes);
        std::tr1::shared_ptr<ImageInterface<Float> > out(cropper.crop(wantreturn));
		if (wantreturn) {
			return new image(out);
		}
		return 0;

	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
image::pixelvalue(const std::vector<int>& pixel) {
	::casac::record *rstat = 0;
	try {
		_log << _ORIGIN;
		if (detached())
			return rstat;

		Record *outRec = _image->pixelvalue(Vector<Int> (pixel));
		rstat = fromRecord(*outRec);
		delete outRec;
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::putchunk(
	const variant& pixels,
	const vector<int>& blc, const vector<int>& inc,
	const bool list, const bool locking, const bool replicate
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return false;
		}
		if (_image->isFloat()) {
			Float f;
			return _putchunk(
				f, pixels, blc, inc, list, locking, replicate
			);
		}
		else {
			if (
				pixels.type() == variant::COMPLEXVEC
			) {
				Array<Complex> pixelsArray;
				std::vector<std::complex<double> > pixelVector = pixels.getComplexVec();
				Vector<Int> shape = pixels.arrayshape();
				pixelsArray.resize(IPosition(shape));
				Vector<std::complex<double> > localpix(pixelVector);
				casa::convertArray(pixelsArray, localpix.reform(IPosition(shape)));
				if (
					_image->putchunk(
						pixelsArray, Vector<Int> (blc), Vector<Int> (inc),
						list, locking, replicate
					)
				) {
					_stats.reset(0);
					return True;
				}
				ThrowCc("Error putting chunk");
			}
			else {
				Complex c;
				return _putchunk(
					c, pixels, blc, inc, list, locking, replicate
				);
			}
		}
		/*
		if (_image->isFloat()) {
			Array<Float> pixelsArray;
			if (pixels.type() == ::casac::variant::DOUBLEVEC) {
				std::vector<double> pixelVector = pixels.getDoubleVec();
				Vector<Int> shape = pixels.arrayshape();
				pixelsArray.resize(IPosition(shape));
				Vector<Double> localpix(pixelVector);
				casa::convertArray(pixelsArray, localpix.reform(IPosition(shape)));
			}
			else if (pixels.type() == ::casac::variant::INTVEC) {
				std::vector<int> pixelVector = pixels.getIntVec();
				Vector<Int> shape = pixels.arrayshape();
				pixelsArray.resize(IPosition(shape));
				Vector<Int> localpix(pixelVector);
				casa::convertArray(pixelsArray, localpix.reform(IPosition(shape)));
			}
			else {
				ThrowCc(
					"Unsupported type for pixels parameter. It "
					"must be either a vector of doubles or ints"
				);
			}
			if (
				_image->putchunk(
					pixelsArray, Vector<Int> (blc), Vector<Int> (inc),
					list, locking, replicate
				)
			) {
				_stats.reset(0);
				return True;
			}
		}
		else {

		}
		throw AipsError("Error putting chunk.");
		*/
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

template<class T> bool image::_putchunk(
	T imageType, const variant& pixels,
	const vector<int>& blc, const vector<int>& inc,
	const bool list, const bool locking, const bool replicate
) {
	Array<T> pixelsArray;
	Vector<Int> shape = pixels.arrayshape();

	if (pixels.type() == variant::DOUBLEVEC) {
		std::vector<double> pixelVector = pixels.getDoubleVec();
		pixelsArray.resize(IPosition(shape));
		Vector<Double> localpix(pixelVector);
		casa::convertArray(pixelsArray, localpix.reform(IPosition(shape)));
	}
	else if (pixels.type() == variant::INTVEC) {
		std::vector<int> pixelVector = pixels.getIntVec();
		pixelsArray.resize(IPosition(shape));
		Vector<Int> localpix(pixelVector);
		casa::convertArray(pixelsArray, localpix.reform(IPosition(shape)));
	}

	else {
		String types = casa::whatType(&imageType) == TpFloat
			? "doubles or ints"
			: "complexes, doubles, or ints";
		ThrowCc(
			"Unsupported type for pixels parameter. It "
			"must be either a vector of " + types
		);
	}
	if (
		_image->putchunk(
			pixelsArray, Vector<Int> (blc), Vector<Int> (inc),
			list, locking, replicate
		)
	) {
		_stats.reset(0);
		return True;
	}
	ThrowCc("Error putting chunk");
}

bool image::putregion(const ::casac::variant& v_pixels,
		const ::casac::variant& v_pixelmask, const ::casac::record& region,
		const bool list, const bool usemask, const bool locking,
		const bool replicateArray) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return False;
		}
		// create Array<Float> pixels
		Array<Float> pixels;
		if (isunset(v_pixels)) {
			// do nothing
		} else if (v_pixels.type() == ::casac::variant::DOUBLEVEC) {
			std::vector<double> pixelVector = v_pixels.getDoubleVec();
			Vector<Int> shape = v_pixels.arrayshape();
			pixels.resize(IPosition(shape));
			Vector<Double> localpix(pixelVector);
			casa::convertArray(pixels, localpix.reform(IPosition(shape)));
		} else if (v_pixels.type() == ::casac::variant::INTVEC) {
			std::vector<int> pixelVector = v_pixels.getIntVec();
			Vector<Int> shape = v_pixels.arrayshape();
			pixels.resize(IPosition(shape));
			Vector<Int> localpix(pixelVector);
			casa::convertArray(pixels, localpix.reform(IPosition(shape)));
		} else {
			_log << LogIO::SEVERE
					<< "pixels is not understood, try using an array "
					<< LogIO::POST;
			return False;
		}

		// create Array<Bool> mask
		Array<Bool> mask;
		if (isunset(v_pixelmask)) {
			// do nothing
		} else if (v_pixelmask.type() == ::casac::variant::DOUBLEVEC) {
			std::vector<double> maskVector = v_pixelmask.getDoubleVec();
			Vector<Int> shape = v_pixelmask.arrayshape();
			mask.resize(IPosition(shape));
			Vector<Double> localmask(maskVector);
			casa::convertArray(mask, localmask.reform(IPosition(shape)));
		} else if (v_pixelmask.type() == ::casac::variant::INTVEC) {
			std::vector<int> maskVector = v_pixelmask.getIntVec();
			Vector<Int> shape = v_pixelmask.arrayshape();
			mask.resize(IPosition(shape));
			Vector<Int> localmask(maskVector);
			casa::convertArray(mask, localmask.reform(IPosition(shape)));
		} else if (v_pixelmask.type() == ::casac::variant::BOOLVEC) {
			std::vector<bool> maskVector = v_pixelmask.getBoolVec();
			Vector<Int> shape = v_pixelmask.arrayshape();
			mask.resize(IPosition(shape));
			Vector<Bool> localmask(maskVector);
			// casa::convertArray(mask,localmask.reform(IPosition(shape)));
			mask = localmask.reform(IPosition(shape));
		} else {
			_log << LogIO::SEVERE
					<< "mask is not understood, try using an array "
					<< LogIO::POST;
			return False;
		}

		if (pixels.size() == 0 && mask.size() == 0) {
			_log << "You must specify at least either the pixels or the mask"
					<< LogIO::POST;
			return False;
		}

		std::auto_ptr<Record> theRegion(toRecord(region));
		if (
			_image->putregion(
				pixels, mask, *theRegion, list, usemask,
				locking, replicateArray
			)
		) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error putting region.");

	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::pv(
	const string& outfile, const variant& start,
	const variant& end, const variant& center, const variant& length,
	const variant& pa, const variant& width, const string& unit,
	const bool overwrite, const variant& region, const string& chans,
	const string& stokes, const string& mask, const bool stretch,
	const bool wantreturn
) {
	if (detached()) {
		return 0;
	}
	try {
		_log << _ORIGIN;
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		std::tr1::shared_ptr<casa::MDirection> startMD, endMD, centerMD;
		Vector<Double> startPix, endPix, centerPix;
		std::tr1::shared_ptr<casa::Quantity> lengthQ;
		Double lengthD = 0;
		if (! start.empty() && ! end.empty()) {
			ThrowIf(
				! center.empty() || ! length.empty()
				|| ! pa.empty(),
				"None of center, length, nor pa may be specified if start and end are specified"
			);
			ThrowIf(
				start.type() != end.type(),
				"start and end must be the same data type"
			);
			casa::MDirection dir;
			_processDirection(startPix, dir, start, String("start"));
			if (startPix.size() == 0) {
				startMD.reset(new casa::MDirection(dir));
			}
			_processDirection(endPix, dir, end, "end");
			if (endPix.size() == 0) {
				endMD.reset(new casa::MDirection(dir));
			}
		}
		else if (
			! center.empty() && ! length.empty()
			&& ! pa.empty()
		) {
			ThrowIf(
				! start.empty() || ! end.empty(),
				"Neither start nor end may be specified "
				"if center, length, and pa are specified"
			);
			casa::MDirection dir;
			_processDirection(centerPix, dir, center, "center");
			if (centerPix.size() == 0) {
				centerMD.reset(new casa::MDirection(dir));
			}
			if (length.type() == variant::INT || length.type() == variant::DOUBLE) {
				lengthD = length.toDouble();
			}
			else {
				lengthQ.reset(
					new casa::Quantity(_casaQuantityFromVar(variant(length)))
				);
			}
		}
		else {
			ThrowCc(
				"Either both of start and end or all three of "
				"center, width, and pa must be specified"
			);
		}
		_log << _ORIGIN;

		uInt intWidth = 0;
		casa::Quantity qWidth;
		if (width.type() == variant::INT) {
			intWidth = width.toInt();
			ThrowIf(
				intWidth % 2 == 0 || intWidth < 1,
				"width must be an odd integer >= 1"
			);
		}
		else if (
			width.type() == variant::STRING
			|| width.type() == variant::RECORD
		) {
			qWidth = _casaQuantityFromVar(width);
		}
		else if (width.type() == variant::BOOLVEC && width.empty()) {
			intWidth = 1;
		}
		else {
			ThrowCc("Unsupported data type for width " + width.typeString());
		}
		if (outfile.empty() && ! wantreturn) {
			_log << LogIO::WARN << "outfile was not specified and wantreturn is false. "
				<< "The resulting image will be inaccessible" << LogIO::POST;
		}
		std::tr1::shared_ptr<Record> regionPtr = _getRegion(region, True);
		PVGenerator pv(
			_image->getImage(), regionPtr.get(),
			chans, stokes, mask, outfile, overwrite
		);
		if (startPix.size() == 2) {
			pv.setEndpoints(
				make_pair(startPix[0], startPix[1]),
				make_pair(endPix[0], endPix[1])
			);
		}
		else if (startMD) {
			pv.setEndpoints(*startMD, *endMD);
		}
		else if (centerMD) {
			if (lengthQ) {
				pv.setEndpoints(
					*centerMD, *lengthQ, _casaQuantityFromVar(variant(pa))
				);
			}
			else {
				pv.setEndpoints(
					*centerMD, lengthD, _casaQuantityFromVar(variant(pa))
				);
			}
		}
		else {
			if (lengthQ) {
				pv.setEndpoints(
					make_pair(centerPix[0], centerPix[1]),
					*lengthQ, _casaQuantityFromVar(variant(pa))
				);
			}
			else {
				pv.setEndpoints(
						make_pair(centerPix[0], centerPix[1]),
					lengthD, _casaQuantityFromVar(variant(pa))
				);
			}
		}
		if (intWidth == 0) {
			pv.setWidth(qWidth);
		}
		else {
			pv.setWidth(intWidth);
		}
		pv.setStretch(stretch);
		pv.setOffsetUnit(unit);
		_log << _ORIGIN;
		SPIIF out = pv.generate();
		image *ret = wantreturn ? new image(out) : 0;
		return ret;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

void image::_processDirection(
	Vector<Double>& pixel, casa::MDirection& dir,
	const variant& inputDirection, const String& paramName
) {
	variant::TYPE myType = inputDirection.type();
	ThrowIf(
		(
			myType == variant::INTVEC
			|| myType == variant::DOUBLEVEC
			|| myType == variant::STRINGVEC
		) &&
		inputDirection.size() != 2,
		"If specified as an array, " + paramName
		+ " must have exactly two elements"
	);
	pixel.resize(0);
	if (myType == variant::INTVEC || myType == variant::DOUBLEVEC) {
		pixel = Vector<Double>(_toDoubleVec(inputDirection));
	}
	else if(myType == variant::STRINGVEC) {
		vector<string> x = inputDirection.toStringVec();
		casa::Quantity q0 = _casaQuantityFromVar(variant(x[0]));
		casa::Quantity q1 = _casaQuantityFromVar(variant(x[1]));
		dir = casa::MDirection(q0, q1);
	}
	else if (myType == variant::STRING) {
		string parts[3];
		split(inputDirection.toString(), parts, 3, Regex("[, \n\t\r\v\f]+"));
		casa::MDirection::Types frame;
		casa::MDirection::getType(frame, parts[0]);
		dir = casa::MDirection::getType(frame, parts[0])
			? casa::MDirection(
				_casaQuantityFromVar(parts[1]),
				_casaQuantityFromVar(parts[2]), frame
			)
			: casa::MDirection(
				_casaQuantityFromVar(parts[0]),
				_casaQuantityFromVar(parts[1])
			);
	}
	else {
		ThrowCc("Unsupported type for " + paramName);
	}
}

image* image::rebin(
	const std::string& outfile, const std::vector<int>& bin,
	const variant& region, const ::casac::variant& vmask,
	bool dropdeg, bool overwrite, bool /* async */,
	bool stretch, bool crop
) {
	LogOrigin lor(_class, __func__);
	_log << lor;
	ThrowIf(
		detached(), "Unable to create image"
	);
	Vector<Int> mybin(bin);
	ThrowIf(
		anyTrue(mybin <= 0),
		"All binning factors must be positive."
	);
	try {
		vector<String> msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__;
			msgs.push_back(os.str());
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("bin", bin));
			inputs.push_back(make_pair("region", region));
			inputs.push_back(make_pair("mask", vmask));
			inputs.push_back(make_pair("dropdeg", dropdeg));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("stretch", stretch));
			inputs.push_back(make_pair("crop", crop));

			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(os.str());
		}
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (_image->isFloat()) {
			SPIIF myfloat = _image->getImage();
			ImageRebinner<Float> rebinner(
				myfloat, _getRegion(region, True).get(),
				mask, outfile, overwrite
			);
			rebinner.setFactors(mybin);
			rebinner.setStretch(stretch);
			rebinner.setDropDegen(dropdeg);
			rebinner.addHistory(lor, msgs);
			rebinner.setCrop(crop);
			return new image(rebinner.rebin());
		}
		else {
			SPIIC myComplex = _image->getComplexImage();
			ImageRebinner<Complex> rebinner(
				myComplex, _getRegion(region, True).get(),
				mask, outfile, overwrite
			);
			rebinner.setFactors(mybin);
			rebinner.setStretch(stretch);
			rebinner.setDropDegen(dropdeg);
			rebinner.addHistory(lor, msgs);
			rebinner.setCrop(crop);
			return new image(rebinner.rebin());
		}
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::regrid(
	const string& outfile, const vector<int>& inshape,
	const record& csys, const vector<int>& inaxes,
	const variant& region, const variant& vmask,
	const string& method, const int decimate, const bool replicate,
	const bool doRefChange, const bool dropDegenerateAxes,
	const bool overwrite, const bool forceRegrid,
	const bool specAsVelocity, const bool /* async */,
	const bool stretch
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
		        throw AipsError("Unable to create image");
			return 0;
		}
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		auto_ptr<Record> csysRec(toRecord(csys));
		auto_ptr<CoordinateSystem> coordinates(CoordinateSystem::restore(*csysRec, ""));
		ThrowIf (
			! coordinates.get(),
			"Invalid specified coordinate system record."
		);
		std::tr1::shared_ptr<Record> regionPtr(_getRegion(region, True));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Vector<Int> axes;
		if (!((inaxes.size() == 1) && (inaxes[0] == -1))) {
			axes = inaxes;
		}
		ImageRegridder regridder(
			_image->getImage(), regionPtr.get(),
			mask, outfile, overwrite, *coordinates,
			IPosition(axes), IPosition(inshape), dropDegenerateAxes
		);
		regridder.setMethod(method);
		regridder.setDecimate(decimate);
		regridder.setReplicate(replicate);
		regridder.setDoRefChange(doRefChange);
		regridder.setForceRegrid(forceRegrid);
		regridder.setSpecAsVelocity(specAsVelocity);
		regridder.setStretch(stretch);
		return new image(regridder.regrid());
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::image* image::rotate(
	const std::string& outfile, const std::vector<int>& inshape,
	const ::casac::variant& inpa, const ::casac::record& region,
	const ::casac::variant& vmask, const std::string& method,
	const int decimate, const bool replicate, const bool dropdeg,
	const bool overwrite, const bool /* async */, const bool stretch
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
		        throw AipsError("Unable to create image");
			return 0;
		}
		Vector<Int> shape(inshape);
		Quantum<Double> pa(_casaQuantityFromVar(inpa));
		std::auto_ptr<Record> Region(toRecord(region));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		std::tr1::shared_ptr<ImageInterface<Float> > pImOut(
			_image->rotate(
				outfile, shape, pa, *Region, mask, method,
				decimate, replicate, dropdeg, overwrite, stretch
			)
		);
		return new image(pImOut);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::rename(const std::string& name, const bool overwrite) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return False;
		}

		if (_image->rename(name, overwrite)) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error renaming image.");
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::replacemaskedpixels(
	const variant& pixels,
	const variant& region, const variant& vmask,
	const bool updateMask, const bool list,
	const bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		return False;
	}
	try {
		std::tr1::shared_ptr<Record> regionPtr = _getRegion(region, True);

		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		vector<std::pair<LogOrigin, String> > msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__;
			msgs.push_back(make_pair(_ORIGIN, os.str()));
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("pixels", pixels));
			inputs.push_back(make_pair("region", region));
			inputs.push_back(make_pair("mask", vmask));
			inputs.push_back(make_pair("update", updateMask));
			inputs.push_back(make_pair("list", list));
			inputs.push_back(make_pair("stretch", stretch));
			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(make_pair(_ORIGIN, os.str()));
		}
		if (_image->isFloat()) {
			SPIIF myfloat = _image->getImage();
			ImageMaskedPixelReplacer<Float> impr(
				myfloat, regionPtr.get(), mask
			);
			impr.setStretch(stretch);
			impr.replace(pixels.toString(), updateMask, list);
			_image.reset(new ImageAnalysis(myfloat));
			ImageHistory<Float> hist(myfloat);
			hist.addHistory(msgs);
		}
		else {
			SPIIC mycomplex = _image->getComplexImage();
			ImageMaskedPixelReplacer<Complex> impr(
				mycomplex, regionPtr.get(), mask
			);
			impr.setStretch(stretch);
			impr.replace(pixels.toString(), updateMask, list);
			_image.reset(new ImageAnalysis(mycomplex));
			ImageHistory<Complex> hist(mycomplex);
			hist.addHistory(msgs);
		}
		_stats.reset(0);
		return True;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

record*
image::restoringbeam(int channel, int polarization) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		if (_image->isFloat()) {
			return fromRecord(
				_image->getImage()->imageInfo().beamToRecord(
					channel, polarization
				)
			);
		}
		else {
			return fromRecord(
				_image->getComplexImage()->imageInfo().beamToRecord(
					channel, polarization
				)
			);
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::image* image::sepconvolve(
	const std::string& outFile, const std::vector<int>& axes,
	const std::vector<std::string>& types,
	const ::casac::variant& widths,
	const double Scale, const ::casac::record& region,
	const ::casac::variant& vmask, const bool overwrite,
	const bool /* async */, const bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		throw AipsError("Unable to create image");
		return 0;
	}
	try {
		UnitMap::putUser("pix", UnitVal(1.0), "pixel units");
		std::auto_ptr<Record> pRegion(toRecord(region));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		Vector<Int> smoothaxes(axes);
		Vector<String> kernels = toVectorString(types);

		int num = 0;
		Vector<Quantum<Double> > kernelwidths;
		if (widths.type() == ::casac::variant::INTVEC) {
			std::vector<int> widthsIVec = widths.toIntVec();
			num = widthsIVec.size();
			std::vector<double> widthsVec(num);
			for (int i = 0; i < num; i++)
				widthsVec[i] = widthsIVec[i];
			kernelwidths.resize(num);
			Unit u("pix");
			for (int i = 0; i < num; i++) {
				kernelwidths[i] = casa::Quantity(widthsVec[i], u);
			}
		}
		else if (widths.type() == ::casac::variant::DOUBLEVEC) {
			std::vector<double> widthsVec = widths.toDoubleVec();
			num = widthsVec.size();
			kernelwidths.resize(num);
			Unit u("pix");
			for (int i = 0; i < num; i++) {
				kernelwidths[i] = casa::Quantity(widthsVec[i], u);
			}
		}
		else if (widths.type() == ::casac::variant::STRING || widths.type()
				== ::casac::variant::STRINGVEC) {
			toCasaVectorQuantity(widths, kernelwidths);
			num = kernelwidths.size();
		}
		else {
			_log << LogIO::WARN << "Unrecognized kernelwidth datatype"
					<< LogIO::POST;
			return 0;
		}
		if (kernels.size() == 1 && kernels[0] == "") {
			kernels.resize(num);
			for (int i = 0; i < num; i++) {
				kernels[i] = "gauss";
			}
		}
		if (
			smoothaxes.size() == 0 || ((smoothaxes.size() == 1)
			&& (smoothaxes[0] = -1))
		) {
			smoothaxes.resize(num);
			for (int i = 0; i < num; i++) {
				smoothaxes[i] = i;
			}
		}
		std::tr1::shared_ptr<ImageInterface<Float> > pImOut(
			_image->sepconvolve(outFile,
				smoothaxes, kernels, kernelwidths,
				Scale, *pRegion, mask, overwrite,
				stretch
			)
		);
		return new image(pImOut);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::set(const ::casac::variant& vpixels, const int pixelmask,
		const ::casac::record& region, const bool list) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return False;
		}

		String pixels = vpixels.toString();
		if (pixels == "[]")
			pixels = "";
		std::auto_ptr<Record> pRegion(toRecord(region));

		if (pixels == "" && pixelmask == -1) {
			_log << LogIO::WARN
					<< "You must specify at least either the pixels or the mask to set"
					<< LogIO::POST;
			return False;
		}
		if (_image->set(pixels, pixelmask, *pRegion, list)) {
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error setting pixel values.");
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::setbrightnessunit(const std::string& unit) {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return False;
		}
		Bool res = _image->isFloat()
			?  _image->getImage()->setUnits(Unit(unit))
			: _image->getComplexImage()->setUnits(Unit(unit));
		ThrowIf(! res, "Unable to set brightness unit");
		_stats.reset(0);
		return True;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::setcoordsys(const ::casac::record& csys) {
	bool rstat(false);
	try {
		_log << LogOrigin("image", "setcoordsys");
		if (detached())
			return rstat;

		Record *coordinates = toRecord(csys);
		rstat = _image->setcoordsys(*coordinates);
		delete coordinates;
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::sethistory(const std::string& origin,
		const std::vector<std::string>& history) {
	try {
		if (detached()) {
			return false;
		}
		if ((history.size() == 1) && (history[0].size() == 0)) {
			LogOrigin lor("image", "sethistory");
			_log << lor << "history string is empty" << LogIO::POST;
		} else {
			if(_image->isFloat()) {
				ImageHistory<Float> hist(_image->getImage());
				hist.addHistory(origin, history);
			}
			else {
				ImageHistory<Complex> hist(_image->getComplexImage());
				hist.addHistory(origin, history);
			}
		}
		return True;
	}
	catch (const AipsError& x) {
		LogOrigin lor("image", __func__);
		_log << lor << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::setmiscinfo(const ::casac::record& info) {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return false;
		}
		std::auto_ptr<Record> tmp(toRecord(info));
		Bool res = _image->isFloat()
			? _image->getImage()->setMiscInfo(*tmp)
			: _image->getComplexImage()->setMiscInfo(*tmp);
		return res;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

std::vector<int> image::shape() {
	std::vector<int> rstat(0);
	_log << _ORIGIN;
	if (detached()) {
		return rstat;
	}
	try {
		rstat = _image->isFloat()
			? _image->getImage()->shape().asVector().tovector()
			: _image->getComplexImage()->shape().asVector().tovector();
		return rstat;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::setrestoringbeam(
	const variant& major, const variant& minor,
	const variant& pa, const record& beam,
	const bool deleteIt, const bool log,
	const int channel, const int polarization
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return false;
		}
		std::auto_ptr<Record> rec(toRecord(beam));
		if (
			_image->setrestoringbeam(
				_casaQuantityFromVar(major), _casaQuantityFromVar(minor),
				_casaQuantityFromVar(pa), *rec, deleteIt,
				log, channel, polarization
			)
		) {
			_log << LogOrigin("image", __func__);
			_stats.reset(0);
			return True;
		}
		throw AipsError("Error setting restoring beam.");
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

record* image::statistics(
	const vector<int>& axes, const record& region,
	const variant& mask, const vector<string>& plotstats,
	const vector<double>& includepix,
	const vector<double>& excludepix, const string& plotter,
	const int nx, const int ny, const bool list, const bool force,
	const bool disk, const bool robust, const bool verbose,
	const bool /* async */, const bool stretch, const string& logfile,
	const bool append
) {
	_log << _ORIGIN;
	if (detached()) {
		_log << "Image not attached" << LogIO::POST;
		return 0;
	}
	try {
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		std::auto_ptr<Record> regionRec(
			region.empty() ? 0 : toRecord(region)
		);
		String mtmp = mask.toString();
		if (mtmp == "false" || mtmp == "[]") {
			mtmp = "";
		}
		Vector<String> plotStats = toVectorString(plotstats);
		if (plotStats.size() == 0) {
			plotStats.resize(2);
			plotStats[0] = "mean";
			plotStats[1] = "sigma";
		}
		Vector<Int> tmpaxes(axes);
		if (tmpaxes.size() == 1 && tmpaxes[0] == -1) {
			tmpaxes.resize(0);
		}
		Vector<Float> tmpinclude;
		Vector<Float> tmpexclude;
		if (
			!(
				includepix.size() == 1
				&& includepix[0] == -1
			)
		) {
			tmpinclude.resize(includepix.size());
			for (uInt i=0; i<includepix.size(); i++) {
				tmpinclude[i] = includepix[i];
			}
		}
		if (!(excludepix.size() == 1 && excludepix[0] == -1)) {
			tmpexclude.resize(excludepix.size());
			for (uInt i = 0; i < excludepix.size(); i++) {
				tmpexclude[i] = excludepix[i];
			}
		}
		if (verbose) {
			_log << LogIO::NORMAL << "Determining stats for image "
				<< _name(True) << LogIO::POST;
		}
		Record ret;
		if (force || _stats.get() == 0) {
			_stats.reset(
				new ImageStatsCalculator(
					_image->getImage(), regionRec.get(), mtmp, verbose
				)
			);
		}
		else {
			_stats->setMask(mtmp);
			std::auto_ptr<Record> regionRec2(toRecord(region));
			_stats->setRegion(*regionRec2);
		}
		_stats->setPlotStats(plotStats);
		_stats->setAxes(tmpaxes);
		_stats->setIncludePix(tmpinclude);
		_stats->setPlotter(plotter);
		_stats->setExcludePix(tmpexclude);
		_stats->setNXNY(nx, ny);
		_stats->setList(list);
		_stats->setForce(force);
		_stats->setDisk(disk);
		_stats->setRobust(robust);
		_stats->setVerbose(verbose);
		_stats->setStretch(stretch);
		_stats->setLogfile(logfile);
		_stats->setLogfileAppend(append);
		return fromRecord(_stats->calculate());
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::twopointcorrelation(
	const std::string& outfile,
	const ::casac::record& region, const ::casac::variant& vmask,
	const std::vector<int>& axes, const std::string& method,
	const bool overwrite, const bool stretch
) {
	_log << _ORIGIN;
	if (detached()) {
		return false;
	}
	try {
		String outFile(outfile);
		std::auto_ptr<Record> Region(toRecord(region));
		String mask = vmask.toString();
		if (mask == "[]")
			mask = "";
		Vector<Int> iAxes;
		if (!(axes.size() == 1 && axes[0] == -1)) {
			iAxes = axes;
		}
		return _image->twopointcorrelation(
			outFile, *Region, mask, iAxes,
			method, overwrite, stretch
		);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::image* image::subimage(
	const string& outfile, const variant& region,
	const variant& vmask, const bool dropDegenerateAxes,
	const bool overwrite, const bool list, const bool stretch,
	const bool wantreturn
) {
	try {
		_log << _ORIGIN;
		ThrowIf(
			detached(),
			"Unable to create image"
		);
		std::tr1::shared_ptr<Record> regionRec = _getRegion(region, False);
		String regionStr = region.type() == variant::STRING
			? region.toString()
			: "";
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}
		if (outfile.empty() && ! wantreturn) {
			_log << LogIO::WARN << "outfile was not specified and wantreturn is false. "
				<< "The resulting image will be inaccessible" << LogIO::POST;
		}

		tr1::shared_ptr<ImageAnalysis> ia;
		if (_image->isFloat()) {
			ia.reset(
				new ImageAnalysis(
					_subimage<Float>(
						std::tr1::shared_ptr<ImageInterface<Float> >(
							_image->getImage()->cloneII()
						),
						outfile, *regionRec, mask, dropDegenerateAxes,
						overwrite, list, stretch
					)
				)
			);
		}
		else {
			ia.reset(
				new ImageAnalysis(
					_subimage<Complex>(
						std::tr1::shared_ptr<ImageInterface<Complex> >(
							_image->getComplexImage()->cloneII()
						),
						outfile, *regionRec, mask, dropDegenerateAxes,
						overwrite, list, stretch
					)
				)
			);
		}
		image *res = wantreturn ? new image(ia) : 0;
		return res;
	}
	catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

template<class T> SPIIT image::_subimage(
	SPIIT clone,
	const String& outfile, const Record& region,
	const String& mask, bool dropDegenerateAxes,
	bool overwrite, bool list, bool stretch
) {
	return SPIIT(
		SubImageFactory<T>::createImage(
			*clone, outfile, region,
			mask, dropDegenerateAxes, overwrite, list, stretch
		)
	);
}

record* image::summary(
	const string& doppler, const bool list,
	const bool pixelorder, const bool verbose
) {
	try {
		_log << _ORIGIN;
		if (detached()) {
			return 0;
		}
		return fromRecord(
			_image->summary(doppler, list, pixelorder, verbose)
		);
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::tofits(
	const std::string& fitsfile, const bool velocity,
	const bool optical, const int bitpix, const double minpix,
	const double maxpix, const ::casac::record& region,
	const ::casac::variant& vmask, const bool overwrite,
	const bool dropdeg, const bool deglast, const bool dropstokes,
	const bool stokeslast, const bool wavelength, const bool airwavelength,
	const bool /* async */, const bool stretch,
	const bool history
) {
	_log << _ORIGIN;
	if (detached()) {
		return false;
	}
	try {
		if (fitsfile.empty()) {
			_log << "fitsfile must be specified" << LogIO::EXCEPTION;
		}
		if (fitsfile == "." || fitsfile == "..") {
			_log << "Invalid fitsfile name " << fitsfile << LogIO::EXCEPTION;
		}
		std::auto_ptr<Record> pRegion(toRecord(region));
		String mask = vmask.toString();
		if (mask == "[]") {
			mask = "";
		}

		String origin;
		{
			ostringstream buffer;
			buffer << "CASA ";
			VersionInfo::report(buffer);
			origin = String(buffer);
		}
		return _image->tofits(
			fitsfile, velocity, optical, bitpix, minpix,
			maxpix, *pRegion, mask, overwrite, dropdeg,
			deglast, dropstokes, stokeslast, wavelength,
			airwavelength, origin, stretch, history
		);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::toASCII(const std::string& outfile, const ::casac::record& region,
		const ::casac::variant& mask, const std::string& sep,
		const std::string& format, const double maskvalue, const bool overwrite,
		const bool stretch) {
	// sep is hard-wired as ' ' which is what imagefromascii expects
	_log << _ORIGIN;
	if (detached()) {
		return False;
	}

	try {
		String Mask;
		if (mask.type() == ::casac::variant::BOOLVEC) {
			Mask = "";
		}
		else if (
			mask.type() == ::casac::variant::STRING
			|| mask.type() == ::casac::variant::STRINGVEC
		) {
			Mask = mask.toString();
		}
		std::auto_ptr<Record> pRegion(toRecord(region));
		return _image->toASCII(
			outfile, *pRegion, Mask, sep, format,
			maskvalue, overwrite, stretch
		);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

std::string image::type() {
	return "image";
}

//std::vector<double>
::casac::record*
image::topixel(const ::casac::variant& value) {
	//  std::vector<double> rstat;
	::casac::record *rstat = 0;
	try {
		_log << LogOrigin("image", __func__);
		if (detached())
			return rstat;

		Vector<Int> bla;
		CoordinateSystem cSys = _image->coordsys(bla);
		::casac::coordsys mycoords;
		//NOT using _image->toworld as most of the math is in casac namespace
		//in coordsys...should revisit this when casac::coordsys is cleaned
		mycoords.setcoordsys(cSys);
		rstat = mycoords.topixel(value);

	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

::casac::record*
image::toworld(const ::casac::variant& value, const std::string& format) {
	::casac::record *rstat = 0;
	try {
		_log << LogOrigin("image", __func__);
		if (detached())
			return rstat;

		Vector<Double> pixel;
		if (isunset(value)) {
			pixel.resize(0);
		} else if (value.type() == ::casac::variant::DOUBLEVEC) {
			pixel = value.getDoubleVec();
		} else if (value.type() == ::casac::variant::INTVEC) {
			variant vcopy = value;
			Vector<Int> ipixel = vcopy.asIntVec();
			Int n = ipixel.size();
			pixel.resize(n);
			for (int i = 0; i < n; i++)
				pixel[i] = ipixel[i];
		} else if (value.type() == ::casac::variant::RECORD) {
			::casac::variant localvar(value);
			Record *tmp = toRecord(localvar.asRecord());
			if (tmp->isDefined("numeric")) {
				pixel = tmp->asArrayDouble("numeric");
			} else {
				_log << LogIO::SEVERE << "unsupported record type for value"
						<< LogIO::EXCEPTION;
				return rstat;
			}
			delete tmp;
		} else {
			_log << LogIO::SEVERE << "unsupported data type for value"
					<< LogIO::EXCEPTION;
			return rstat;
		}
		rstat = fromRecord(_image->toworld(pixel, format));

	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::unlock() {
	try {
		_log << LogOrigin("image", __func__);
		if (detached()) {
			return False;
		}
		if (_image->isFloat()) {
			_image->getImage()->unlock();
		}
		else {
			_image->getComplexImage()->unlock();
		}
		return True;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::detached() const {
	if ( _image.get() == 0 || _image->detached()) {
		_log <<  _ORIGIN;
		_log << LogIO::SEVERE
			<< "Image is detached - cannot perform operation." << endl
			<< "Call image.open('filename') to reattach." << LogIO::POST;
		return True;
	}
	return False;
}

::casac::record*
image::setboxregion(const std::vector<double>& blc,
		const std::vector<double>& trc, const bool frac,
		const std::string& infile) {
	casac::record* rstat = 0;
	try {

		_log << _ORIGIN;
		_log << LogIO::WARN
			<< "THIS METHOD IS DEPRECATED AND WILL BE REMOVED. USE rg.box() INSTEAD."
			<< LogIO::POST;
		if (detached()) {
			return rstat;
		}

		Record tempR(_image->setboxregion(Vector<Double> (blc),
				Vector<Double> (trc), frac, infile));
		rstat = fromRecord(tempR);

	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return rstat;
}

bool image::maketestimage(
	const std::string& outfile, const bool overwrite
) {
	try {
		_reset();
		_log << _ORIGIN;
		return _image->maketestimage(outfile, overwrite);
	} catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimagefromimage(
	const string& infile, const string& outfile,
	const record& region, const variant& vmask,
	const bool dropdeg, const bool overwrite
) {
	try {
		image *rstat(0);
		std::auto_ptr<ImageAnalysis> newImage(new ImageAnalysis());
		_log << _ORIGIN;

		String mask;

		if (vmask.type() == variant::BOOLVEC) {
			mask = "";
		}
		else if (
			vmask.type() == variant::STRING
			|| vmask.type() == variant::STRINGVEC
		) {
			mask = vmask.toString();
		}
		else if (vmask.type() == ::casac::variant::RECORD) {
			_log << LogIO::SEVERE
					<< "Don't support region masking yet, only valid LEL "
					<< LogIO::POST;
			throw AipsError("Unable to create image");
			return 0;
		}
		else {
			_log << LogIO::SEVERE
					<< "Mask is not understood, try a valid LEL string "
					<< LogIO::POST;
			throw AipsError("Unable to create image");
			return 0;
		}

		std::auto_ptr<Record> regionPtr(toRecord(region));
        std::tr1::shared_ptr<ImageInterface<Float> >outIm(
			newImage->newimage(
				infile, outfile,*regionPtr,
				mask, dropdeg, overwrite
			)
		);
		if (outIm) {
			rstat = new image(outIm);
		} else {
			rstat = new image();
		}
		if(!rstat)
			throw AipsError("Unable to create image");
		return rstat;

	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimagefromfile(const std::string& fileName) {
	try {
		image *rstat(0);
		std::auto_ptr<ImageAnalysis> newImage(new ImageAnalysis());
		_log << _ORIGIN;
		std::tr1::shared_ptr<ImageInterface<Float> > outIm(
			newImage->newimagefromfile(fileName)
		);
		if (outIm.get() != 0) {
			rstat =  new image(outIm);
		} else {
			rstat =  new image();
		}
		if(!rstat)
			throw AipsError("Unable to create image");
		return rstat;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimage(const string& fileName) {
	image *rstat = newimagefromfile(fileName);
	if (!rstat)
			throw AipsError("Unable to create image");
	return rstat;
}

image* image::newimagefromarray(
	const string& outfile, const variant& pixels,
	const record& csys, const bool linear,
	const bool overwrite, const bool log
) {
	try {
		auto_ptr<ImageAnalysis> newImage(
			new ImageAnalysis()
		);
		_log << _ORIGIN;
		// Some protection.  Note that a Glish array, [], will
		// propagate through to here to have ndim=1 and shape=0
		Vector<Int> shape = pixels.arrayshape();
		uInt ndim = shape.size();
		if (ndim == 0) {
			_log << "The pixels array is empty" << LogIO::EXCEPTION;
		}
		for (uInt i = 0; i < ndim; i++) {
			if (shape(i) <= 0) {
				_log << "The shape of the pixels array is invalid"
						<< LogIO::EXCEPTION;
			}
		}
		Array<Float> pixelsArray;
		if (pixels.type() == variant::DOUBLEVEC) {
			vector<double> pixelVector = pixels.getDoubleVec();
			Vector<Int> shape = pixels.arrayshape();
			pixelsArray.resize(IPosition(shape));
			Vector<Double> localpix(pixelVector);
			convertArray(pixelsArray, localpix.reform(IPosition(shape)));
		}
		else if (pixels.type() == ::casac::variant::INTVEC) {
			vector<int> pixelVector = pixels.getIntVec();
			Vector<Int> shape = pixels.arrayshape();
			pixelsArray.resize(IPosition(shape));
			Vector<Int> localpix(pixelVector);
			convertArray(pixelsArray, localpix.reform(IPosition(shape)));
		}
		else {
			_log << LogIO::SEVERE
					<< "pixels is not understood, try using an array "
					<< LogIO::POST;
			return new image();
		}
		auto_ptr<Record> coordinates(toRecord(csys));
        tr1::shared_ptr<ImageInterface<Float> > outIm(
			newImage->newimagefromarray(
				outfile, pixelsArray, *coordinates,
				linear, overwrite, log
			)
		);
        ThrowIf(
            ! outIm,
            "Unable to create image"
        );
		return new image(outIm);
	}
	catch (AipsError x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimagefromshape(
	const string& outfile, const vector<int>& shape,
	const record& csys, bool linear,
	bool overwrite, bool log, const string& type
) {
	try {
        LogOrigin lor("image", __func__);
		_log << lor;
		auto_ptr<Record> coordinates(toRecord(csys));
        String mytype = type;
        mytype.downcase();
        ThrowIf(
            ! mytype.startsWith("f") && ! mytype.startsWith("c"),
            "Input parm type must start with either 'f' or 'c'"
        );
		vector<std::pair<LogOrigin, String> > msgs;
		{
			ostringstream os;
			os << "Ran ia." << __func__;
			msgs.push_back(make_pair(lor, os.str()));
			vector<std::pair<String, variant> > inputs;
			inputs.push_back(make_pair("outfile", outfile));
			inputs.push_back(make_pair("shape", shape));
			inputs.push_back(make_pair("csys", csys));
			inputs.push_back(make_pair("linear", linear));
			inputs.push_back(make_pair("overwrite", overwrite));
			inputs.push_back(make_pair("log", log));
			inputs.push_back(make_pair("type", type));
			os.str("");
			os << "ia." << __func__ << _inputsString(inputs);
			msgs.push_back(make_pair(lor, os.str()));
		}
        if (mytype.startsWith("f")) {
            SPIIF myfloat = ImageFactory::floatImageFromShape(
                outfile, shape, *coordinates,
                linear, overwrite, log, &msgs
            );
            return new image(myfloat);
        }
        else {
            SPIIC mycomplex = ImageFactory::complexImageFromShape(
                outfile, shape, *coordinates,
                linear, overwrite, log, &msgs
            );
            return new image(mycomplex);
        }

/*

		image *rstat(0);
		auto_ptr<ImageAnalysis> newImage(new ImageAnalysis());
		_log << _ORIGIN;
		auto_ptr<Record> coordinates(toRecord(csys));
        std::tr1::shared_ptr<ImageInterface<Float> > outIm(
			newImage->newimagefromshape(
				outfile, Vector<Int>(shape), *coordinates,
				linear, overwrite, log
			)
		);
        rstat = outIm.get() ? new image(outIm) : new image(outIm);
		ThrowIf(
			rstat == 0, "Unable to create image"
		);
		return rstat;
*/
    }
    catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

image* image::newimagefromfits(
	const string& outfile, const string& fitsfile,
	const int whichrep, const int whichhdu,
	const bool zeroBlanks, const bool overwrite
) {
	try {
		image *rstat(0);
		auto_ptr<ImageAnalysis> newImage(new ImageAnalysis());
		_log << _ORIGIN;
        tr1::shared_ptr<ImageInterface<Float> > outIm(
			newImage->newimagefromfits(
				outfile,
				fitsfile, whichrep, whichhdu, zeroBlanks, overwrite
			)
		);
		if (outIm.get() != 0) {
			rstat = new image(outIm);
		} else {
			rstat = new image();
		}
		if(!rstat)
			throw AipsError("Unable to create image");
		return rstat;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

::casac::variant*
image::makearray(const double v, const std::vector<int>& shape) {
	int ndim = shape.size();
	int nelem = 1;
	for (int i = 0; i < ndim; i++)
		nelem *= shape[i];
	std::vector<double> element(nelem);
	for (int i = 0; i < nelem; i++)
		element[i] = v;
	return new ::casac::variant(element, shape);
}

casac::record*
image::recordFromQuantity(const casa::Quantity q) {
	::casac::record *r = 0;
	try {
		_log << LogOrigin("image", "recordFromQuantity");
		String error;
		casa::Record R;
		if (QuantumHolder(q).toRecord(error, R)) {
			r = fromRecord(R);
		} else {
			_log << LogIO::SEVERE << "Could not convert quantity to record."
					<< LogIO::POST;
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return r;
}

::casac::record*
image::recordFromQuantity(const Quantum<Vector<Double> >& q) {
	::casac::record *r = 0;
	try {
		_log << LogOrigin("image", "recordFromQuantity");
		String error;
		casa::Record R;
		if (QuantumHolder(q).toRecord(error, R)) {
			r = fromRecord(R);
		} else {
			_log << LogIO::SEVERE << "Could not convert quantity to record."
					<< LogIO::POST;
		}
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
	return r;
}

casa::Quantity image::_casaQuantityFromVar(const ::casac::variant& theVar) {
	try {
		_log << _ORIGIN;
		casa::QuantumHolder qh;
		String error;
		if (
			theVar.type() == ::casac::variant::STRING
			|| theVar.type() == ::casac::variant::STRINGVEC
		) {
			ThrowIf(
				!qh.fromString(error, theVar.toString()),
				"Error " + error + " in converting quantity "
			);
		}
		else if (theVar.type() == ::casac::variant::RECORD) {
			//NOW the record has to be compatible with QuantumHolder::toRecord
			::casac::variant localvar(theVar); //cause its const
			auto_ptr<Record> ptrRec(toRecord(localvar.asRecord()));
			ThrowIf(
				!qh.fromRecord(error, *ptrRec),
				"Error " + error + " in converting quantity "
			);
		}
        else if (theVar.type() == variant::BOOLVEC) {
            return casa::Quantity();
        }
		return qh.asQuantity();
	}
	catch (const AipsError& x) {
		_log << LogOrigin("image", __func__);
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool image::isconform(const string& other) {
	_log << _ORIGIN;

	if (detached()) {
		return False;
	}
	try {
		ThrowIf(
			! _image->isFloat(),
			"This method only supports Float valued images"
		);
		ImageInterface<Float> *oth = 0;
		ImageUtilities::openImage(oth, String(other));
		if (oth == 0) {
			throw AipsError("Unable to open image " + other);
		}
		std::auto_ptr<ImageInterface<Float> > x(oth);
        std::tr1::shared_ptr<const ImageInterface<Float> > mine = _image->getImage();
		if (mine->shape().isEqual(x->shape()) && mine->coordinates().near(
				x->coordinates())) {
			Vector<String> mc = mine->coordinates().worldAxisNames();
			Vector<String> oc = x->coordinates().worldAxisNames();
			if (mc.size() != oc.size()) {
				return False;
			}
			for (uInt i = 0; i < mc.size(); i++) {
				if (mc[i] != oc[i]) {
					return False;
				}
			}
			return True;
		}
		return False;
	} catch (const AipsError& x) {
		_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

std::tr1::shared_ptr<Record> image::_getRegion(
	const variant& region, const bool nullIfEmpty
) const {
	switch (region.type()) {
	case variant::BOOLVEC:
		return std::tr1::shared_ptr<Record>(nullIfEmpty ? 0 : new Record());
	case variant::STRING:
		return std::tr1::shared_ptr<Record>(
			region.toString().empty()
				? nullIfEmpty ? 0 : new Record()
				: new Record(
					CasacRegionManager::regionFromString(
						_image->getImage()->coordinates(),
						region.toString(), _name(False),
						_image->getImage()->shape()
					)
				)
		);
	case variant::RECORD:
		{
			std::tr1::shared_ptr<variant> clon(region.clone());
			return std::tr1::shared_ptr<Record>(
				nullIfEmpty && region.size() == 0
					? 0
					: toRecord(
						std::tr1::shared_ptr<variant>(region.clone())->asRecord()
					)
			);
		}
	default:
		ThrowCc("Unsupported type for region " + region.typeString());
	}
}

vector<double> image::_toDoubleVec(const variant& v) {
	variant::TYPE type = v.type();
	ThrowIf(
		type != variant::INTVEC && type != variant::LONGVEC
		&& type != variant::DOUBLEVEC,
		"variant is not a numeric array"
	);
	vector<double> output;
	if (type == variant::INTVEC || type == variant::LONGVEC) {
		Vector<Int> x = v.toIntVec();
		std::copy(x.begin(), x.end(), std::back_inserter(output));
	}
	if (type == variant::DOUBLEVEC) {
		Vector<Double> x = v.toDoubleVec();
		std::copy(x.begin(), x.end(), std::back_inserter(output));
	}
	return output;
}



} // casac namespace
