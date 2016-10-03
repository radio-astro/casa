// Private part to quanta_cmpt.h


casacore::Quantity casaQuantityFromVar(const ::casac::variant& theVar);
casacore::QuantumHolder quantumHolderFromVar(const ::casac::variant& theVar);
casac::record* recordFromQuantity(casacore::Quantity q);
casac::record* recordFromQuantity(const casacore::Quantum<casacore::Vector<casacore::Double> >& q);

bool qcompare(const ::casac::variant& v, const casacore::Quantity a);

// casacore::List known units static casacore::Record mapit(const casacore::String &tp);
static casacore::Record mapit(const casacore::String &tp);

// Give a constant named by the string
static casacore::Quantity constants(const casacore::String &in);

// Make time format from casacore::String array
static casacore::Int makeFormT(const casacore::Vector<casacore::String> &in);

// Make angle format from casacore::String array
static casacore::Int makeFormA(const casacore::Vector<casacore::String> &in);

// Add a unit name entry to table
static void mapInsert(casacore::Record &out,
		      const casacore::String &type,
		      const std::map<casacore::String, casacore::UnitName> &mp);

// dopcv - doppler value conversion
casacore::Quantity
dopcv(const casacore::Quantity val, const casacore::Quantity arg);

// frqcv - freq converter
casacore::Quantity
frqcv(const casacore::Quantity val, const casacore::Quantity arg);

// tfreq - table freq formatter
casacore::Vector<casacore::String>
tfreq(const casacore::Quantum<casacore::Vector<casacore::Double> > &val,
      const casacore::Vector<casacore::Int> &arg,
      const casacore::String &form, const casacore::Bool form2);

//// Some useless classes???
// unitv - unit(vector)
casacore::Quantum<casacore::Array<casacore::Double> >
unitv(const casacore::Array<casacore::Double> v, const casacore::String &unitname);

// quant
casacore::Array<casacore::QuantumHolder>
quant(const casacore::Array<casacore::QuantumHolder> a);

// Data
casacore::LogIO *itsLog;
