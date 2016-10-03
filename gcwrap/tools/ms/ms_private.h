casacore::Bool detached();
casacore::Bool ready2write_();

casacore::MeasurementSet *itsMS, *itsOriginalMS;
casacore::MSSelection *itsMSS;
casacore::LogIO *itsLog;
casacore::MSSelector *itsSel;
casacore::MSFlagger *itsFlag;
casa::VisibilityIterator *itsVI;
casa::VisBuffer *itsVB;
casacore::Bool doingIterations_p;

void addephemcol(const casacore::MeasurementSet& appendedMS);
