// Private part to measures_impl.h

// Do a frame fill with a measure
bool doframe(const casa::MeasureHolder &in);
// Do a frame fill with a table name (e.g. comet)
bool doframe(const casa::String &in);

// Convert measure
bool measure(casa::String &error, casa::MeasureHolder &out,
		       const casa::MeasureHolder &in,
		       const casa::String &outref,
		       const casa::Record &off);

casa::Quantity
posangle (const casa::MDirection& md1, const casa::MDirection& md2);

casa::MeasureHolder
doptorv(const casa::String &rf, const casa::MeasureHolder &v);
casa::MeasureHolder
doptofreq(const casa::String &rf, const casa::MeasureHolder &v,
		    const casa::Quantity &rfq);

//# Data
casa::MeasFrame *frame_p;    // The globally used MeasFrame for this DO
casa::MeasComet *pcomet_p;   // The current comet class
casa::LogIO *itsLog;

casa::Quantity casaQuantityFromVar(const ::casac::variant& theVar);

bool casacRec2MeasureHolder(casa::MeasureHolder& mh, const ::casac::record& r);
casa::MeasureHolder casaMeasureHolderFromVar(const ::casac::variant& theVar);

casa::MDirection casaMDirectionFromVar(const ::casac::variant& theVar);
casa::MDoppler casaMDopplerFromVar(const ::casac::variant& theVar);
casa::MEpoch casaMEpochFromVar(const ::casac::variant& theVar);
casa::MFrequency casaMFrequencyFromVar(const ::casac::variant& theVar);
casa::MPosition casaMPositionFromVar(const ::casac::variant& theVar);
casa::MRadialVelocity casaMRadialVelocityFromVar(const ::casac::variant& theVar);
casa::MBaseline casaMBaselineFromVar(const ::casac::variant& theVar);
casa::Muvw casaMuvwFromVar(const ::casac::variant& theVar);
casa::MEarthMagnetic casaMEarthMagneticFromVar(const ::casac::variant& theVar);

casac::record* recordFromQuantity(const casa::Quantity q);
casac::record* recordFromQuantity(const casa::Quantum<casa::Vector<casa::Double> >& q);
