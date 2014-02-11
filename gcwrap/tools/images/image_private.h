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
std::tr1::shared_ptr<casa::Record> _getRegion(const variant& region, const bool nullIfEmpty) const;

static vector<double> _toDoubleVec(const variant& v);

void _reset();

// the returned value of pixels will have either 0 or two elements, if 0 then the returned
// value of dir will be set
void _processDirection(
	casa::Vector<casa::Double>& pixels, casa::MVDirection& dir, const variant& inputDirection,
	const casa::String& paramName
);

template<class T, class U> casac::variant* _getchunk(
	const std::vector<int>& blc, const std::vector<int>& trc,
	const std::vector<int>& inc, const std::vector<int>& axes,
	const bool list, const bool dropdeg, const bool getmask
);

template<class T> bool _putchunk(
	T imageType,
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
	casa::PtrHolder<casa::ImageInterface<T> >& outimage,
	tr1::shared_ptr<const casa::ImageInterface<T> > inImage,
	const std::string& outfile, bool direction,
	bool spectral, const std::string& stokes, bool linear,
	bool tabular, bool overwrite, bool silent
);
