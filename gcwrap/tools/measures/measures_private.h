// Private part to measures_impl.h

// Do a frame fill with a measure
bool doframe(const casacore::MeasureHolder &in);
// Do a frame fill with a table name (e.g. comet)
bool doframe(const casacore::String &in);

// Convert measure
bool measure(casacore::String &error, casacore::MeasureHolder &out,
		       const casacore::MeasureHolder &in,
		       const casacore::String &outref,
		       const casacore::Record &off);

casacore::Quantity
posangle (const casacore::MDirection& md1, const casacore::MDirection& md2);

casacore::MeasureHolder
doptorv(const casacore::String &rf, const casacore::MeasureHolder &v);
casacore::MeasureHolder
doptofreq(const casacore::String &rf, const casacore::MeasureHolder &v,
		    const casacore::Quantity &rfq);

//# Data
casacore::MeasFrame *frame_p;    // The globally used casacore::MeasFrame for this DO
casacore::MeasComet *pcomet_p;   // The current comet class
casacore::LogIO *itsLog;

casacore::Quantity casaQuantityFromVar(const ::casac::variant& theVar);

bool casacRec2MeasureHolder(casacore::MeasureHolder& mh, const ::casac::record& r);
casacore::MeasureHolder casaMeasureHolderFromVar(const ::casac::variant& theVar);

casacore::MDirection casaMDirectionFromVar(const ::casac::variant& theVar);
casacore::MDoppler casaMDopplerFromVar(const ::casac::variant& theVar);
casacore::MEpoch casaMEpochFromVar(const ::casac::variant& theVar);
casacore::MFrequency casaMFrequencyFromVar(const ::casac::variant& theVar);
casacore::MPosition casaMPositionFromVar(const ::casac::variant& theVar);
casacore::MRadialVelocity casaMRadialVelocityFromVar(const ::casac::variant& theVar);
casacore::MBaseline casaMBaselineFromVar(const ::casac::variant& theVar);
casacore::Muvw casaMuvwFromVar(const ::casac::variant& theVar);
casacore::MEarthMagnetic casaMEarthMagneticFromVar(const ::casac::variant& theVar);
