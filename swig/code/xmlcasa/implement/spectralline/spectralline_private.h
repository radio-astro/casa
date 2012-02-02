// Private part to spectralline_cmpt.h

casa::LogIO *_log;
casa::SplatalogueTable *_table;

spectralline(casa::SplatalogueTable* table);

bool _detached() const;

void _checkLowHigh(
	double& low, double& high, const vector<double> pair, const string label
) const ;
