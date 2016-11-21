public: 
//bool open(const casacore::ImageInterface<casacore::Float>* inImage);

// The constructed object will manage the input pointer with a
// shared_ptr
image(casacore::ImageInterface<casacore::Float> * inImage);

image(casacore::ImageInterface<casacore::Complex> * inImage);

image(SHARED_PTR<casacore::ImageInterface<casacore::Float> > inImage);

image(SHARED_PTR<casacore::ImageInterface<casacore::Complex> > inImage);

//image(SHARED_PTR<casacore::ImageAnalysis> ia);

private:

typedef casacore::GaussianBeam Angular2DGaussian;

mutable casacore::LogIO _log;

// This class needs to be templated. For now, we maintain two pointers.
// At least one of which will be zero for a valid object state.
// SHARED_PTR<casacore::ImageInterface<casacore::Float> > _imageFloat;
// SHARED_PTR<casacore::ImageInterface<casacore::Complex> > _imageComplex;


// the image analysis object needs to be removed after decimation of that
// class is complete
//SHARED_PTR<casacore::ImageAnalysis> _image;
casa::SPIIF _imageF;
casa::SPIIC _imageC;
std::auto_ptr<casa::ImageStatsCalculator> _stats;

static const casacore::String _class;

// static casacore::Bool _openFuncsRegistered;

// Having private version of IS and IH means that they will
// only recreate storage images if they have to

// Prints an error message if the image DO is detached and returns true.
bool detached() const;

casac::record* recordFromQuantity(casacore::Quantity q);

casac::record* recordFromQuantity(const casacore::Quantum<casacore::Vector<casacore::Double> >& q);

template<class T> image* _adddegaxes(
	SPCIIT inImage,
	const std::string& outfile, bool direction,
	bool spectral, const std::string& stokes, bool linear,
	bool tabular, bool overwrite, bool silent
);

void _addHistory(
    const casacore::String& method, const vector<casacore::String>& keys, const vector<variant>& vals,
    const vector<casacore::String>& appendMsgs=vector<casacore::String>(),
    const std::set<casacore::String>& dontQuote=std::set<casacore::String>()
);

static String _quantityRecToString(const Record& q);

template <class T> image* _boxcar(
	SPCIIT myimage, const variant& region,
	const casac::variant& mask, const std::string& outfile, bool overwrite,
	bool stretch, int axis, int width, bool drop,
	const string& dmethod, const casacore::LogOrigin& lor
);

casacore::Quantity _casaQuantityFromVar(const ::casac::variant& theVar);

template<class T> image* _decimate(
	SPCIIT image, const string& outfile, int axis,
	int factor, casa::ImageDecimatorData::Function f,
	const SHARED_PTR<casacore::Record> region,
	const string& mask, bool overwrite, bool stretch,
	const vector<casacore::String>& msgs
) const;

std::pair<casa::SPIIF, casa::SPIIC> _fromarray(
    const string& outfile, const variant& pixels,
    const record& csys, bool linear, bool overwrite, bool log
);

template<class T> casacore::Record _getchunk(
	SPCIIT myimage,
	const std::vector<int>& blc, const std::vector<int>& trc,
	const std::vector<int>& inc, const std::vector<int>& axes,
	bool list, bool dropdeg
);

static casacore::String _getMask(const casac::variant& mask);

template <class T> casacore::Record _getprofile(
	SPCIIT myimage, int axis, const casacore::String& function,
	const casacore::String& unit, const casacore::Record& region,
	const casacore::String& mask, bool stretch,
	const casacore::String& spectype, const casacore::Quantity* const &restfreq,
	const casacore::String& frame, const casacore::String& logfile,
	const casacore::String& regionName
);

SHARED_PTR<casacore::Record> _getRegion(
	const variant& region, const bool nullIfEmpty,
	const std::string& otherImageName=""
) const;

template<class T> vector<string>  _handleMask(
	SPIIT myimage, const casacore::String& op,
	const vector<string>& name
);

template <class T> static image* _hanning(
	SPCIIT image, SHARED_PTR<const casacore::Record> region,
	const casacore::String& mask, const std::string& outfile, bool overwrite,
	bool stretch, int axis, bool drop,
	casa::ImageDecimatorData::Function dFunction,
	const std::vector<casac::variant> values
);

template<class T> SPIIT _imagecalc(
	const string& outfile, const string& pixels,
	bool overwrite, const string& imagemd
);

static casacore::String _inputsString(
	const std::vector<std::pair<casacore::String, casac::variant> >& inputs,
	const std::set<String>& dontQuote
);

static bool _isUnset(const variant& var);

// because public method name() is not const
casacore::String _name(bool strippath=false) const;

static vector<casacore::String> _newHistory(
	const string& method, const vector<casacore::String>& names,
	const vector<variant>& values, const std::set<String>& dontQuote=std::set<String>()
);

// the returned value of pixels will have either 0 or two elements, if 0 then the returned
// value of dir will be set
void _processDirection(
	casacore::Vector<casacore::Double>& pixels, casacore::MDirection& dir, const variant& inputDirection,
	const casacore::String& paramName
);

template<class T> void _putchunk(
	SPIIT image, const casac::variant& pixels,
	const vector<int>& blc, const vector<int>& inc,
	const bool list, const bool locking, const bool replicate
);

template <class T> static image* _regrid(
	casa::ImageRegridderBase<T>& regridder,
	const string& method, int decimate,	bool replicate,
	bool doRefChange, bool forceRegrid,
	bool specAsVelocity, bool stretch,
	bool dropDegenerateAxes, const casacore::LogOrigin& lor,
	const vector<casacore::String>& msgs
);

void _remove(bool verbose);

void _reset();

template<class T> SHARED_PTR<casacore::ImageInterface<T> > _subimage(
	SHARED_PTR<casacore::ImageInterface<T> > clone,
	const casacore::String& outfile, const casacore::Record& region,
	const casacore::String& mask, bool dropDegenerateAxes,
	bool overwrite, bool list, bool stretch, const vector<int>& keepaxes
);

static vector<double> _toDoubleVec(const variant& v);

template <class T> static SPIIT _twopointcorrelation(
	SPIIT myimage, const string& outfile,
	SHARED_PTR<casacore::Record> region, const casacore::String& mask,
	const casacore::IPosition& axes, const std::string& method,
	bool overwrite, bool stretch, const casacore::LogOrigin& origin,
    const vector<casacore::String>& msgs
);
