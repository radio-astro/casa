public: 
//bool open(const casa::ImageInterface<casa::Float>* inImage);

// The constructed object will manage the input pointer with a
// shared_ptr
image(casa::ImageInterface<casa::Float> * inImage);

image(casa::ImageInterface<casa::Complex> * inImage);

image(std::tr1::shared_ptr<casa::ImageInterface<casa::Float> > inImage);

image(std::tr1::shared_ptr<casa::ImageInterface<casa::Complex> > inImage);

image(std::tr1::shared_ptr<casa::ImageAnalysis> ia);

private:

typedef GaussianBeam Angular2DGaussian;

mutable casa::LogIO _log;

// This class needs to be templated. For now, we maintain two pointers.
// At least one of which will be zero for a valid object state.
// std::tr1::shared_ptr<casa::ImageInterface<casa::Float> > _imageFloat;
// std::tr1::shared_ptr<casa::ImageInterface<casa::Complex> > _imageComplex;


// the image analysis object needs to be removed after decimation of that
// class is complete
std::tr1::shared_ptr<casa::ImageAnalysis> _image;
std::auto_ptr<casa::ImageStatsCalculator> _stats;

static const casa::String _class;

// Having private version of IS and IH means that they will
// only recreate storage images if they have to

// Prints an error message if the image DO is detached and returns True.
bool detached() const;

casac::record* recordFromQuantity(casa::Quantity q);
casac::record* recordFromQuantity(const casa::Quantum<casa::Vector<casa::Double> >& q);
casa::Quantity _casaQuantityFromVar(const ::casac::variant& theVar);
std::tr1::shared_ptr<casa::Record> _getRegion(
	const variant& region, const bool nullIfEmpty,
	const std::string& otherImageName=""
) const;

static vector<double> _toDoubleVec(const variant& v);

void _reset();

// the returned value of pixels will have either 0 or two elements, if 0 then the returned
// value of dir will be set
void _processDirection(
	casa::Vector<casa::Double>& pixels, casa::MDirection& dir, const variant& inputDirection,
	const casa::String& paramName
);

template <class T> image* _boxcar(
	SPCIIT myimage, std::tr1::shared_ptr<const casa::Record> region,
	const casa::String& mask, const std::string& outfile, bool overwrite,
	bool stretch, int axis, int width, bool drop,
	casa::ImageDecimatorData::Function dFunction, const casa::LogOrigin& lor,
	const std::vector<casa::String> msgs
);

template<class T> casa::Record _getchunk(
	SPCIIT myimage,
	const std::vector<int>& blc, const std::vector<int>& trc,
	const std::vector<int>& inc, const std::vector<int>& axes,
	bool list, bool dropdeg
);

template <class T> casa::Record _getprofile(
	SPCIIT myimage, int axis, const casa::String& function,
	const casa::String& unit, const casa::Record& region,
	const casa::String& mask, bool stretch,
	const casa::String& spectype, const casa::Quantity* const &restfreq,
	const casa::String& frame
);

template<class T> void _putchunk(
	T imageType, SPIIT image,
	const casac::variant& pixels,
	const vector<int>& blc, const vector<int>& inc,
	const bool list, const bool locking, const bool replicate
);

template<class T> tr1::shared_ptr<casa::ImageInterface<T> > _subimage(
	std::tr1::shared_ptr<casa::ImageInterface<T> > clone,
	const casa::String& outfile, const casa::Record& region,
	const casa::String& mask, bool dropDegenerateAxes,
	bool overwrite, bool list, bool stretch
);

template<class T> image* _adddegaxes(
	SPCIIT inImage,
	const std::string& outfile, bool direction,
	bool spectral, const std::string& stokes, bool linear,
	bool tabular, bool overwrite, bool silent
);

template<class T> image* _decimate(
	SPCIIT image, const string& outfile, int axis,
	int factor, casa::ImageDecimatorData::Function f,
	const std::tr1::shared_ptr<casa::Record> region,
	const string& mask, bool overwrite, bool stretch,
	const vector<casa::String>& msgs
) const;

template <class T> static image* _hanning(
	SPCIIT image, std::tr1::shared_ptr<const casa::Record> region,
	const casa::String& mask, const std::string& outfile, bool overwrite,
	bool stretch, int axis, bool drop,
	casa::ImageDecimatorData::Function dFunction,
	const std::vector<casac::variant> values
);

static casa::String _inputsString(
	const std::vector<std::pair<casa::String, casac::variant> >& inputs
);

// because public method name() is not const
casa::String _name(bool strippath=false) const;

static vector<String> _newHistory(
	const string& method, const vector<String>& names,
	const vector<variant>& values
);

template <class T> static image* _regrid(
	ImageRegridderBase<T>& regridder,
	const string& method, int decimate,	bool replicate,
	bool doRefChange, bool forceRegrid,
	bool specAsVelocity, bool stretch,
	bool dropDegenerateAxes, const LogOrigin& lor,
	const vector<String>& msgs
);

