
casacore::MeasurementSet *itsMS;
bool hasValidMS_p;
casa::Imager *itsImager;
casacore::LogIO *itsLog;
bool mrvFromString(casacore::MRadialVelocity &,  const casacore::String &);
bool mdFromString(casacore::MDirection &,  const casacore::String &);
     bool mpFromString(casacore::MPosition &,  const casacore::String &);

