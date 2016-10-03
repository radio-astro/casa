/* Private parts for coordsys */

public :

void setcoordsys(casacore::CoordinateSystem &acsys);

private:

mutable casacore::PtrHolder<casacore::LogIO>  _log;
casacore::PtrHolder<casacore::CoordinateSystem> _csys;
casacore::String _imageName;

// private constructor to make components on the fly
coordsys(const casacore::CoordinateSystem * inCS);

// Inter convert absolute and relative world or pixel coordinates
casacore::Record absRelRecord (casacore::LogIO& os,
			   const casacore::RecordInterface& recIn,
			   casacore::Bool isWorld, casacore::Bool absToRel);

// Add default coordinates to CS
void addCoordinate (casacore::CoordinateSystem& cSys, casacore::Bool direction,
		    casacore::Bool spectral,
		    const casacore::Vector<casacore::String>& stokes,
		    casacore::Int linear, casacore::Bool tabular);

// Copy the world axes of in to out
void copyWorldAxes (casacore::Vector<casacore::Double>& out,
		    const casacore::Vector<casacore::Double>& in, casacore::Int c) const;

// Convert record of measures to world coordinate vector
casacore::Vector<casacore::Double>
measuresToWorldVector (const casacore::RecordInterface& rec) const;

// Convert world coordinate to measures and stick in record
casacore::Record worldVectorToMeasures(const casacore::Vector<casacore::Double>& world,
				   casacore::Int c, casacore::Bool abs) const;

// Find coordinate of desired type
casacore::Int findCoordinate (casacore::Coordinate::Type type, casacore::Bool warn) const;

// Convert a record holding some mixture of numeric, measures, quantity, string
// to a vector of doubles
void recordToWorldVector (casacore::Vector<casacore::Double>& world,
			  casacore::String& type,
			  casacore::Int c, const casacore::RecordInterface& rec) const;

// Convert a vector of world to a record holding some mixture
// of numeric, measures, quantity, string
casacore::Record worldVectorToRecord (const casacore::Vector<casacore::Double>& world,
				  casacore::Int c, const casacore::String& format,
				  casacore::Bool isAbsolute,
				  casacore::Bool showAsAbsolute);

// Convert casacore::Quantum to record
casacore::Record quantumToRecord (casacore::LogIO& os,
			      const casacore::Quantum<casacore::Double>& value) const;

// Set casacore::DirectionCoordinate reference code
void setDirectionCode (const casacore::String& code, casacore::Bool adjust);

// Set casacore::SpectralCoordinate reference code
void setSpectralCode (const casacore::String& code, casacore::Bool adjust);

// Convert world casacore::String to world vector double.  World vector must be length
// cSys.nWorldAxes()
casacore::Vector<casacore::Double>
stringToWorldVector (casacore::LogIO& os, const casacore::Vector<casacore::String>& world,
		     const casacore::Vector<casacore::String>& worldAxisUnits) const;

// Convert user coordinate type string to enum
casacore::Coordinate::Type stringToType(const casacore::String& typeIn) const;

// absolute pixel to absolute world
casacore::Record toWorldRecord (const casacore::Vector<casacore::Double>& pixel, const casacore::String& format);

// Add missing values or tim excessive
void trim (casacore::Vector<casacore::Double>& in,
	   const casacore::Vector<casacore::Double>& replace) const;

// Convert a vector of quantum doubles in a record to a vector of
// double applying specified units
casacore::Vector<casacore::Double>
quantumVectorRecordToVectorDouble (const casacore::RecordInterface& recQ,
				   const casacore::Vector<casacore::String>& units) const;

casacore::Vector<casacore::Double>
quantumRecordToVectorDouble (const casacore::RecordInterface& recQ,
			     const casacore::Vector<casacore::String>& units) const;

casacore::Bool checkAbsRel(casac::variant& value, casacore::Bool shouldBeAbs);

casacore::Record * coordinateValueToRecord(const casac::variant& value,
				       casacore::Bool isWorld, casacore::Bool isAbs,
				       casacore::Bool first);

int isValueWorld(casac::variant& value, int shouldBeWorld,
		 casacore::Bool verbose=true);

std::vector<double> toPixel(const ::casac::variant& value);

void _setup(const casacore::String& method);
