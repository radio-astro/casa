// Private part to quanta_cmpt.h


casa::Quantity casaQuantityFromVar(const ::casac::variant& theVar);
casa::QuantumHolder quantumHolderFromVar(const ::casac::variant& theVar);
casac::record* recordFromQuantity(casa::Quantity q);
casac::record* recordFromQuantity(const casa::Quantum<casa::Vector<casa::Double> >& q);

bool qcompare(const ::casac::variant& v, const casa::Quantity a);

// List known units static Record mapit(const String &tp);
static casa::Record mapit(const casa::String &tp);

// Give a constant named by the string
static casa::Quantity constants(const casa::String &in);

// Make time format from String array
static casa::Int makeFormT(const casa::Vector<casa::String> &in);

// Make angle format from String array
static casa::Int makeFormA(const casa::Vector<casa::String> &in);

// Add a unit name entry to table
static void mapInsert(casa::Record &out,
		      const casa::String &type,
		      const std::map<casa::String, casa::UnitName> &mp);

// dopcv - doppler value conversion
casa::Quantity
dopcv(const casa::Quantity val, const casa::Quantity arg);

// frqcv - freq converter
casa::Quantity
frqcv(const casa::Quantity val, const casa::Quantity arg);

// tfreq - table freq formatter
casa::Vector<casa::String>
tfreq(const casa::Quantum<casa::Vector<casa::Double> > &val,
      const casa::Vector<casa::Int> &arg,
      const casa::String &form, const casa::Bool form2);

//// Some useless classes???
// unitv - unit(vector)
casa::Quantum<casa::Array<casa::Double> >
unitv(const casa::Array<casa::Double> v, const casa::String &unitname);

// quant
casa::Array<casa::QuantumHolder>
quant(const casa::Array<casa::QuantumHolder> a);

// Data
casa::LogIO *itsLog;
