casacore::Bool detached();
casacore::Bool ready2write_();

casacore::Bool checkinit();
casacore::Vector<casacore::Int> convertCorrToInt(casa::vi::VisBuffer2* vb2);
casacore::Vector<casacore::Int> getspectralwindows();
casacore::Vector<casacore::Int> getifrnumbers();
casacore::Vector<casacore::Int> getbaselines(casa::vi::VisBuffer2* vb2);

casacore::Vector<casacore::Int> addgaps(
        casacore::Vector<casacore::Int> ifrnums, casacore::Int gap);
void convertPoln(casacore::Cube<casacore::Complex>& inputcube, 
        casa::vi::VisBuffer2* vb);
void completeMissingIfrs(casacore::Vector<casacore::Int>& inputvec, 
        casacore::Record& rec, casacore::String field);
casacore::Array<casacore::Int> getFlagCount(
        casacore::Array<casacore::Bool> flagcube, casacore::Bool ifraxis);
casacore::String getSpwExpr();

// for getdata2 axis_info:
casacore::Vector<casacore::String> getCorrAxis(casa::vi::VisBuffer2* vb2);
casacore::Record getFreqAxis();
casacore::Record getIfrAxis();
void addTimeAxis(casacore::Record& rec);
void incTimeAxis(casacore::IPosition& cubeshape);
// Add data to return record (getdata2):
void getitem(casacore::String item, casa::vi::VisBuffer2* vb2,
        casacore::Record& addToRec, bool ifraxis);
template <typename T> void getIfrArray(casacore::Array<T>& inputarray, 
        casa::vi::VisBuffer2* vb2);
void getInfoOptions(casacore::Vector<casacore::Bool> info_options, 
        casacore::Record& rec);
template <typename T> void addArrayToRec(casacore::Array<T>& inputarray, 
        casacore::Record& rec, casacore::String field,
        casacore::Bool ifraxis=casacore::False);
template <typename T> void addVectorToRec(casacore::Vector<T>& inputvec,
        casacore::Record& rec, casacore::String field);
// If averaging requested
void getAveragedValues(casacore::Vector<casacore::String> fieldnames, 
        casacore::Record& rec);
void getWeightSum(casacore::Array<casacore::Float>& weight);
void getAvgSigma(casacore::Array<casacore::Float>& sigma);

// for putdata2
bool allowPut(casacore::String fieldname);
void putitem(casacore::uInt fieldId, casa::vi::VisBuffer2* vb2,
        casacore::Record& inputRecord, casacore::Int startrow,
        casacore::Int subchunk);
template <typename T> void ifrToArray(casacore::Array<T>& ifrarray,
        casa::vi::VisBuffer2* vb2);


casacore::MeasurementSet *itsMS, *itsOriginalMS, *itsSelectedMS;
casacore::MSSelector *itsSel;
casacore::LogIO *itsLog;
casacore::MSSelection *itsMSS;
casa::VisibilityIterator *itsVI;
casa::vi::VisibilityIterator2 *itsVI2;

casacore::Bool doingIterations_p;
casacore::Bool doingAveraging_p;
casacore::Bool initSel_p;
casacore::Bool maxrows_p;
casacore::Int nAnt1_p;
casacore::String polnExpr_p;
casacore::String chanselExpr_p;
casacore::Vector<casacore::Int> wantedpol_p;
casacore::Vector<casacore::Int> ifrnumbers_p;
std::vector<int> chansel_p; // nchan, start, width, inc

void addephemcol(const casacore::MeasurementSet& appendedMS);
