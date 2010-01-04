
casa::MeasurementSet *itsMS;
bool hasValidMS_p;
casa::Imager *itsImager;
casa::LogIO *itsLog;
casa::PGPlotter *itsPlotter;
bool mrvFromString(casa::MRadialVelocity &,  const casa::String &);
bool mdFromString(casa::MDirection &,  const casa::String &);
     bool mpFromString(casa::MPosition &,  const casa::String &);

