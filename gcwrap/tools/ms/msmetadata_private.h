std::unique_ptr<casacore::MSMetaData> _msmd;
std::unique_ptr<casacore::MeasurementSet> _ms;
std::unique_ptr<casacore::LogIO> _log;

msmetadata(const casacore::MeasurementSet * const & s, const float maxcache);

std::set<casacore::uInt> _antsForObsID(casacore::Int obsid) const;

void _checkAntennaId(int id, bool throwIfNegative) const;

void _checkArrayId(int id, bool throwIfNegative) const;

void _checkFieldId(int id, bool throwIfNegative) const;

void _checkObsId(int id, bool throwIfNegative) const;

void _checkSpwId(int id, bool throwIfNegative) const;

void _checkPolId(int id, bool throwIfNegative) const;

std::vector<std::string> _fieldNames(const std::set<int>& ids);

std::set<casacore::ScanKey> _getScanKeys(int scan, int obsid, int arrayid) const;

std::set<casacore::ScanKey> _getScanKeys(const vector<int>& scan, int obsid, int arrayid) const;

// return the intersection of the elements in v and s. In the returned vector, elements
// have the same ordering as in the input vector, v.
template <class T> static vector<T> _intersection(const vector<T>& v, const std::set<T>& s);

// inserts intersection of two sets at the end of vector r. The newly inserted items will be
// ordered (although the entire contents of r may not be).
template <class T> static void _intersection(
    vector<T>& r, const std::set<T>& s1, const std::set<T>& s2
);

bool _isAttached(const bool throwExceptionIfNotAttached=true) const;

void _handleException(const casacore::AipsError& x) const;

static std::vector<double> _setDoubleToVectorDouble(const std::set<casacore::Double>& inset);

static std::vector<int> _setUIntToVectorInt(const std::set<casacore::uInt>& inset);

static std::vector<casacore::uInt> _setUIntToVectorUInt(const std::set<casacore::uInt>& inset);

static std::vector<int> _setIntToVectorInt(const std::set<casacore::Int>& inset);

static std::vector<std::string> _setStringToVectorString(const std::set<casacore::String>& inset);

static std::vector<std::string> _vectorStringToStdVectorString(const std::vector<casacore::String>& inset);

static std::vector<casacore::String> _vectorStdStringToVectorString(const std::vector<std::string>& inset);

static std::vector<int> _vectorUIntToVectorInt(const std::vector<casacore::uInt>& inset);

static std::vector<uint> _vectorIntToVectorUInt(const std::vector<casacore::Int>& inset);

friend msmetadata* ms::metadata(const float maxcache);

void _init(const casacore::MeasurementSet *const &ms, const float maxcache);

template <class T>
static std::set<T> _idsFromExpansion(
	const std::map<casacore::String, std::set<T> >& mymap, const casacore::String& matchString
);

static std::set<int> _idsFromExpansion(
	const std::map<casacore::String, std::set<uint> >& mymap, const casacore::String& matchString
);

static std::vector<casacore::String> _match(
	const vector<casacore::String>& candidates, const casacore::String& matchString
);

static std::string _escapeExpansion(const casacore::String& stringToEscape);
