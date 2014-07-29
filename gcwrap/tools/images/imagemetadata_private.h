/* Private parts */
public:

private:

std::auto_ptr<casa::LogIO> _log;

std::auto_ptr<casa::ImageMetaDataRW> _header;

static const casa::String _class;

void _exceptIfDetached() const;



