/* Private parts for coordsys */

public :

void setcoordsys(casa::CoordinateSystem &acsys);

private:

mutable casa::PtrHolder<casa::LogIO>  _log;
casa::PtrHolder<casa::CoordinateSystem> _csys;
casa::String _imageName;

// private constructor to make components on the fly
coordsys(const casa::CoordinateSystem * inCS);

// Inter convert absolute and relative world or pixel coordinates
casa::Record absRelRecord (casa::LogIO& os,
			   const casa::RecordInterface& recIn,
			   casa::Bool isWorld, casa::Bool absToRel);

// Add default coordinates to CS
void addCoordinate (casa::CoordinateSystem& cSys, casa::Bool direction,
		    casa::Bool spectral,
		    const casa::Vector<casa::String>& stokes,
		    casa::Int linear, casa::Bool tabular);

// Copy the world axes of in to out
void copyWorldAxes (casa::Vector<casa::Double>& out,
		    const casa::Vector<casa::Double>& in, casa::Int c) const;

// Convert record of measures to world coordinate vector
casa::Vector<casa::Double>
measuresToWorldVector (const casa::RecordInterface& rec) const;

// Convert world coordinate to measures and stick in record
casa::Record worldVectorToMeasures(const casa::Vector<casa::Double>& world,
				   casa::Int c, casa::Bool abs) const;

// Find coordinate of desired type
casa::Int findCoordinate (casa::Coordinate::Type type, casa::Bool warn) const;

// Convert a record holding some mixture of numeric, measures, quantity, string
// to a vector of doubles
void recordToWorldVector (casa::Vector<casa::Double>& world,
			  casa::String& type,
			  casa::Int c, const casa::RecordInterface& rec) const;

// Convert a vector of world to a record holding some mixture
// of numeric, measures, quantity, string
casa::Record worldVectorToRecord (const casa::Vector<casa::Double>& world,
				  casa::Int c, const casa::String& format,
				  casa::Bool isAbsolute,
				  casa::Bool showAsAbsolute);

// Convert Quantum to record
casa::Record quantumToRecord (casa::LogIO& os,
			      const casa::Quantum<casa::Double>& value) const;

// Set DirectionCoordinate reference code
void setDirectionCode (const casa::String& code, casa::Bool adjust);

// Set SpectralCoordinate reference code
void setSpectralCode (const casa::String& code, casa::Bool adjust);

// Convert world String to world vector double.  World vector must be length
// cSys.nWorldAxes()
casa::Vector<casa::Double>
stringToWorldVector (casa::LogIO& os, const casa::Vector<casa::String>& world,
		     const casa::Vector<casa::String>& worldAxisUnits) const;

// Convert user coordinate type string to enum
casa::Coordinate::Type stringToType(const casa::String& typeIn) const;

// absolute pixel to absolute world
casa::Record toWorldRecord (const casa::Vector<casa::Double>& pixel, const casa::String& format);

// Add missing values or tim excessive
void trim (casa::Vector<casa::Double>& in,
	   const casa::Vector<casa::Double>& replace) const;

// Convert a vector of quantum doubles in a record to a vector of
// double applying specified units
casa::Vector<casa::Double>
quantumVectorRecordToVectorDouble (const casa::RecordInterface& recQ,
				   const casa::Vector<casa::String>& units) const;

casa::Vector<casa::Double> 
quantumRecordToVectorDouble (const casa::RecordInterface& recQ,
			     const casa::Vector<casa::String>& units) const;

casa::Bool checkAbsRel(casac::variant& value, casa::Bool shouldBeAbs);

casa::Record * coordinateValueToRecord(const casac::variant& value,
				       casa::Bool isWorld, casa::Bool isAbs,
				       casa::Bool first);

int isValueWorld(casac::variant& value, int shouldBeWorld,
		 casa::Bool verbose=true);

std::vector<double> toPixel(const ::casac::variant& value);

void _setup(const casa::String& method);
