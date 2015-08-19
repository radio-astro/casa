std::unique_ptr<casa::MSMetaData> _msmd;
std::unique_ptr<casa::MeasurementSet> _ms;
std::unique_ptr<casa::LogIO> _log;

msmetadata(const casa::MeasurementSet * const & s, const float maxcache);

void _checkAntennaId(int id, bool throwIfNegative) const;

void _checkArrayId(int id, bool throwIfNegative) const;

void _checkFieldId(int id, bool throwIfNegative) const;

void _checkObsId(int id, bool throwIfNegative) const;

void _checkSpwId(int id, bool throwIfNegative) const;

void _checkPolId(int id, bool throwIfNegative) const;

std::vector<std::string> _fieldNames(const std::set<int>& ids);

std::set<casa::ScanKey> _getScanKeys(int scan, int obsid, int arrayid) const;

std::set<casa::ScanKey> _getScanKeys(const vector<int>& scan, int obsid, int arrayid) const;

bool _isAttached(const bool throwExceptionIfNotAttached=true) const;

void _handleException(const casa::AipsError& x) const;

static std::vector<double> _setDoubleToVectorDouble(const std::set<casa::Double>& inset);

static std::vector<int> _setUIntToVectorInt(const std::set<casa::uInt>& inset);

static std::vector<int> _setIntToVectorInt(const std::set<casa::Int>& inset);

static std::vector<std::string> _setStringToVectorString(const std::set<casa::String>& inset);

static std::vector<std::string> _vectorStringToStdVectorString(const std::vector<casa::String>& inset);

static std::vector<casa::String> _vectorStdStringToVectorString(const std::vector<std::string>& inset);

static std::vector<int> _vectorUIntToVectorInt(const std::vector<casa::uInt>& inset);

static std::vector<uint> _vectorIntToVectorUInt(const std::vector<casa::Int>& inset);

friend msmetadata* ms::metadata(const float maxcache);

void _init(const casa::MeasurementSet *const &ms, const float maxcache);

template <class T>
static std::set<T> _idsFromExpansion(
	const std::map<casa::String, std::set<T> >& mymap, const casa::String& matchString
);

static std::set<int> _idsFromExpansion(
	const std::map<casa::String, std::set<uint> >& mymap, const casa::String& matchString
);

static std::vector<casa::String> _match(
	const vector<casa::String>& candidates, const casa::String& matchString
);

static std::string _escapeExpansion(const casa::String& stringToEscape);
