#include <imagemetadata_cmpt.h>

#include <imageanalysis/ImageAnalysis/ImageMetaDataRW.h>

#include <stdcasa/version.h>

#include <casa/namespace.h>

using namespace std;

namespace casac {

const String imagemetadata::_class = "imagemetadata";

imagemetadata::imagemetadata() :
_log(new LogIO()), _header(0) {
try {
	} catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

imagemetadata::~imagemetadata() {}

bool imagemetadata::close() {
	try {
		_header.reset(0);
		_image.reset(0);
		return True;
	}
	catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
				<< LogIO::POST;
		RETHROW(x);
	}
}

bool imagemetadata::add(const string& key, const variant& value) {
	try {
		_exceptIfDetached();
		return _header->add(key, value);
	}
	catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
			<< LogIO::POST;
		RETHROW(x);
	}
}

bool imagemetadata::done() {
	return close();
}

variant* imagemetadata::get(const string& key) {
	try {
		_exceptIfDetached();
		return new variant(_header->getFITSValue(key));
	}
	catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: "
			<< x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

record* imagemetadata::list(bool verbose) {
	try {
		_exceptIfDetached();
		return fromRecord(_header->toRecord(verbose));
	}
	catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: "
			<< x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

bool imagemetadata::open(const std::string& infile) {
	try {
		if (_log.get() == 0) {
			_log.reset(new LogIO());
		}
		ImageInterface<Float> *x;
		ImageUtilities::openImage(x, infile, *_log);
		_image.reset(x);
		_header.reset(new ImageMetaDataRW<Float>(x));
		return True;

	} catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: "
			<< x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

bool imagemetadata::remove(const string& key, const variant& value) {
	try {
		_exceptIfDetached();
		if (String(key) == ImageMetaData<Float>::MASKS) {
			return _header->removeMask(value.toString());
		}
		else {
			return _header->remove(key);
		}
	}
	catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: "
			<< x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}


bool imagemetadata::set(const string& key, const variant& value) {
	try {
		_exceptIfDetached();
		return _header->set(key, value);
	}
	catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: "
			<< x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

void imagemetadata::_exceptIfDetached() const {
	if (_header.get() == 0) {
		throw AipsError("Tool is not attached to a metadata object. Call open() first.");
	}
}


} // casac namespace
