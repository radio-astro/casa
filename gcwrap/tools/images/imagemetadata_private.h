/* Private parts */
public:

private:

std::auto_ptr<casacore::LogIO> _log;

std::auto_ptr<casa::ImageMetaDataRW> _header;

static const casacore::String _class;

void _exceptIfDetached() const;



