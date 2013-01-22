std::auto_ptr<casa::MSMetaData> _msmd;
std::auto_ptr<casa::MeasurementSet> _ms;
std::auto_ptr<casa::LogIO> _log;

std::vector<std::string> _fieldNames(const std::set<uint>& ids);

msmetadata(const casa::MeasurementSet * const & s, const bool preload, const float maxcache);

bool _isAttached(const bool throwExceptionIfNotAttached=true) const;

void _handleException(const casa::AipsError& x) const;

static std::vector<double> _setDoubleToVectorDouble(const std::set<casa::Double>& inset);

static std::vector<int> _setUIntToVectorInt(const std::set<casa::uInt>& inset);

static std::vector<std::string> _setStringToVectorString(const std::set<casa::String>& inset);

static std::vector<std::string> _vectorStringToStdVectorString(const std::vector<casa::String>& inset);

static std::vector<int> _vectorUIntToVectorInt(const std::vector<casa::uInt>& inset);

static std::vector<uint> _vectorIntToVectorUInt(const std::vector<casa::Int>& inset);

friend msmetadata* ms::metadata(const bool preload, const float maxcache);

void _init(const casa::MeasurementSet *const &ms, const bool preload, const float maxcache);

