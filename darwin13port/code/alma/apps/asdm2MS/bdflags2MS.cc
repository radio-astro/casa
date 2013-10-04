#include <iostream>
#include <sstream>
#include <bitset>
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

#include "ASDM.h"
#include "ConfigDescriptionTable.h"
#include "ConfigDescriptionRow.h"
#include "DataDescriptionTable.h"
#include "DataDescriptionRow.h"
#include "ExecBlockTable.h"
#include "ExecBlockRow.h"
#include "ScanTable.h"
#include "ScanRow.h"
#include "SpectralWindowTable.h"
#include "SpectralWindowRow.h"
#include "MainTable.h"
#include "MainRow.h"

using namespace asdm;

#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayLogical.h>

#include "CAxisName.h"
using namespace AxisNameMod;
#include "CProcessorType.h"
using namespace ProcessorTypeMod;

#include "SDMDataObject.h"
#include "SDMDataObjectReader.h"
#include "SDMDataObjectStreamReader.h"

using namespace asdmbinaries;

#include "asdm2MSGeneric.h"
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
using namespace boost;

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
using namespace boost::filesystem;

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/casts.hpp>

using namespace boost::lambda;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/assign/std/vector.hpp> // for 'operator+=()'
using namespace boost::assign; // bring 'operator+=()' into scope

bool verbose = true;

/*
** A simplistic tracing toolbox.
*/
bool debug = (getenv("ASDM_DEBUG") != NULL);
vector<char> logIndent;
#define LOGENTER(name) if (debug) {for_each(logIndent.begin(), logIndent.end(), cout << _1); logIndent.push_back('\t'); cout << #name ": entering" << endl;}
#define LOGEXIT(name)  if (debug) {logIndent.pop_back(); for_each(logIndent.begin(), logIndent.end(), cout << _1); cout << #name ": exiting" << endl;}
#define LOG(msg) if (debug) {for_each(logIndent.begin(), logIndent.end(), cout << _1); cout << msg << endl;}

/*
** Two string streams used all over the applications in general to prepare messages sent to the logging facilities.
*/ 
ostringstream errstream;
ostringstream infostream;

#include <casa/Logging/StreamLogSink.h>
#include <casa/Logging/LogSink.h>
using namespace casa;


string appName; // The name of the application.

/*
** Send the content of message on the logging facility if in verbose mode. 
*/
void info (const string& message) {  
  if (!verbose){
    return;
  }
  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::NORMAL));
}

/*
** Send the content of message on the logging facility in any case and then exit.
*/
void error(const string& message) {
  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::NORMAL));
  //os << LogIO::POST;
  exit(1);
}

/*
** A class to encode the exceptions occuring during the usage of one instance of class ProcessFlags.
*/
class ProcessFlagsException {
private :
  string message;

public : 
  ProcessFlagsException ();
  ProcessFlagsException (const string & message ) ;
  virtual ~ProcessFlagsException ();
  const string &getMessage();
};

ProcessFlagsException::ProcessFlagsException () : message("ProcessFlagsException") {;}

ProcessFlagsException::ProcessFlagsException (const string & message) : message("ProcessFlagsException: " + message) {;}

ProcessFlagsException::~ProcessFlagsException () {;}

const string& ProcessFlagsException::getMessage() {
  return message;
}
// end of class ProcessFlagsException

/*
** A generic class to implement a machine "consuming" elements of the "flags" attachments in the BDF parts.
** The parameter T is meant to handle the base type of the flags.
*/
template <typename T >
class BDFFlagConsumer {
private:
  const T* items_p;
  unsigned int numItems;
  unsigned int i0, i1;
  BDFFlagConsumer(){;}

public:
  BDFFlagConsumer(const T* items_p, unsigned int numItems) : items_p(items_p), numItems(numItems) {
    LOGENTER("BDFFlagConsumer::BDFFlagConsumer(const T* items_p, unsigned int numItems)");
    i0 = 0; i1 = 1;
    LOGEXIT("BDFFlagConsumer::BDFFlagConsumer(const T* items_p, unsigned int numItems)");
  }

  pair<const T* , const T*>  consume(unsigned int n) {
    LOGENTER("BDFFlgasConsumer::pair<const T* , const T*>  consume(unsigned int n)");
    if (i0 >= numItems) return make_pair<const T*, const T*>(NULL, NULL);
    i1 = i0 + n;
    if (i1 > numItems)
      return make_pair<const T*, const T*>(NULL, NULL);
    
    const T* p0 = items_p + i0;
    const T* p1 = items_p + i1;
    i0 = i1;
    LOGEXIT("BDFFlgasConsumer::pair<const T* , const T*>  consume(unsigned int n)");
    return make_pair<const T*, const T*>(p0, p1);
  }
}; // end of class BDFFlagConsumer.

/*
** A class whose instances are meant to be used as functors returning the evaluation of
** flags to be written into the MS given the mask flag provided on the command line.
**
*/
class MSFlagEval {
private:
  MSFlagEval() ;
  FLAGSTYPE mask;

public:
  MSFlagEval(FLAGSTYPE mask);
  ~MSFlagEval();
  char operator()(FLAGSTYPE);
};

MSFlagEval::MSFlagEval(FLAGSTYPE mask):mask(mask){;}
MSFlagEval::~MSFlagEval() {;}
char MSFlagEval::operator()(FLAGSTYPE f) { return ((f & mask) == 0) ? 0 : 1; }
// end of class MSFlagEval

/*
** A class to encode the description of a cell of MS Main Flag.
**
** The elements of the description are :
** 
** - the shape i.e. the pair (numChan, numCorr) 
** - the offset of the linearized sequence of values in that cell from the beginning of the unique array containing
**   all the values.
**
*/ 
class MSFlagCellDescriptor {
  pair<unsigned int, unsigned int> shape_;
  unsigned long int offset_;
private:
  MSFlagCellDescriptor();

public:
  virtual ~MSFlagCellDescriptor();
  MSFlagCellDescriptor(pair<unsigned int, unsigned int> shape, unsigned int);
  pair<unsigned int, unsigned int>& shape()  ;
  unsigned int offset() ;
};
  
MSFlagCellDescriptor::MSFlagCellDescriptor(pair<unsigned int, unsigned int> shape, unsigned int  offset):shape_(shape), offset_(offset) {;}
MSFlagCellDescriptor::~MSFlagCellDescriptor() {;}
pair<unsigned int, unsigned int>& MSFlagCellDescriptor::shape() {return shape_;}
unsigned int MSFlagCellDescriptor::offset() {return offset_;}
// end of class MSFlagDescriptor.

/*
** A class to define a machine accumulating MS Flags cells and their descriptors as the BDF Flags are traversed 
** the BDFlag consumer.
*/
template <typename T>
class MSFlagAccumulator {
private:
  vector<T> MSFlags_v;
  unsigned  int offset;

  vector<MSFlagCellDescriptor> MSFlagCellDescriptor_v;

  vector< pair<unsigned int, unsigned int> *> flagShapes_p_v;
  vector< T* > flagValues_p_v;

  vector<MSFlagCellDescriptor *> MSFlagCellDescriptor_p_v;
  vector<unsigned int> orderedByDD_v;
  vector<vector<unsigned int> >  orderedByBALDD_vv;
  vector<vector<vector<unsigned int> > > orderedByTimeBALDD_vvv;
  unsigned int numFlaggedRows;

public:
  MSFlagAccumulator() {
    LOGENTER("MSFlagAccumulator::MSFlagAccumulator()");
    offset = 0;
    numFlaggedRows = 0;
    LOGEXIT("MSFlagAccumulator::MSFlagAccumulator()");
  }

  virtual ~MSFlagAccumulator() {;}
  void accumulate(unsigned int numChan, unsigned int numPol, const vector<T>& values) {
    LOGENTER("MSFlagAccumulator::accumulate(unsigned int numChan, unsigned int numPol, T* values)");

    unsigned int offset = MSFlags_v.size();
    for_each(values.begin(), values.end(), bind(&vector<T>::push_back, var(MSFlags_v), _1));

    orderedByTimeBALDD_vvv.back().back().push_back(MSFlagCellDescriptor_v.size());
    MSFlagCellDescriptor_v.push_back(MSFlagCellDescriptor(make_pair<unsigned int, unsigned int>(numChan, ((numPol==3)? 4:numPol)),
							  offset));
    LOGEXIT("MSFlagAccumulator::accumulate(unsigned int numChan, unsigned int numPol, T* values)");
  }

  void BALStep() {
    LOGENTER("MSFlagAccumulator::BALStep()");
    orderedByTimeBALDD_vvv.back().push_back(orderedByDD_v);
    LOGEXIT("MSFlagAccumulator::BALStep()");
  }

  void timeStep() {
    LOGENTER("MSFlagAccumulator::TimeStep()");
    orderedByTimeBALDD_vvv.push_back(orderedByBALDD_vv);
    LOGEXIT("MSFlagAccumulator::TimeStep()");
  }

  pair< vector<pair <unsigned int, unsigned int> *>*,
	vector<T*>* >  cellDescriptors(bool MSORDER=true) {
    LOGENTER("MSFlagAccumulator::cellDescriptors(bool MSORDER=true)");

    flagShapes_p_v.clear();
    flagShapes_p_v.resize(MSFlagCellDescriptor_v.size());
    flagValues_p_v.clear();
    flagValues_p_v.resize(MSFlagCellDescriptor_v.size());

    if (MSORDER) {
      // Calculate the number of antennas from the supposedly constant size of vectors contained in orderedByTimeBALDD_vvv
      unsigned int numAntenna = (unsigned int) (1 + sqrt(1 + 4 * orderedByTimeBALDD_vvv.at(0).size())) / 2;

      unsigned int k = 0;
      for (unsigned int iTime = 0; iTime < orderedByTimeBALDD_vvv.size(); iTime++) {
	vector<vector<unsigned int> > & plane = orderedByTimeBALDD_vvv[iTime];
	unsigned int numBAL = plane.size();
	unsigned int numDD = plane[0].size();
	for (unsigned int iDD = 0; iDD < numDD; iDD++) {
	  // Firstly the autocorrelations. 
	  for (unsigned int iBAL = numBAL-numAntenna; iBAL < numBAL; iBAL++) {
	    flagShapes_p_v[k] = &((&MSFlagCellDescriptor_v[0] + plane[iBAL][iDD])->shape());
	    flagValues_p_v[k] = &MSFlags_v[0] + (&MSFlagCellDescriptor_v[0] + plane[iBAL][iDD])->offset();
	    k++;
	  }
	  
	  // Then the cross correlations (if any).
	  for (unsigned int iBAL = 0; iBAL < numBAL-numAntenna; iBAL++) {
	    flagShapes_p_v[k] = &((&MSFlagCellDescriptor_v[0] + plane[iBAL][iDD])->shape());
	    flagValues_p_v[k] = &MSFlags_v[0] + (&MSFlagCellDescriptor_v[0] + plane[iBAL][iDD])->offset();
	    k++;
	  }	  
	}
      }
    }
    else {
      transform(MSFlagCellDescriptor_v.begin(),
		MSFlagCellDescriptor_v.end(),
		flagShapes_p_v.begin(),
		&bind(&MSFlagCellDescriptor::shape, _1));
      transform(MSFlagCellDescriptor_v.begin(),
		MSFlagCellDescriptor_v.end(),
		flagValues_p_v.begin(),
		&(var(MSFlags_v[0])) + bind(&MSFlagCellDescriptor::offset, _1));
    }
    
    LOGEXIT("MSFlagAccumulator::cellDescriptors(bool MSORDER=true)");
    return make_pair< vector<pair <unsigned int, unsigned int> *>*,
      vector<T*>* >(&flagShapes_p_v, &flagValues_p_v);
  }

  void info (ostream& os) {
    LOGENTER("MSFlagAccumulator::info");
    os << "I have " << MSFlagCellDescriptor_v.size() << " MS flag cell descriptors." << endl;
    LOGEXIT("MSFlagAccumulator::info");
  }

  void dump (ostream& os, bool MSORDER=true) {
    LOGENTER("MSFlagAccumulator::dump");
    pair< vector<pair <unsigned int, unsigned int>* >*,
      vector<T*>* > cds = cellDescriptors(MSORDER);
  
    vector<pair <unsigned int, unsigned int> *>* shapes_p_v_p = cds.first;
    vector<T*>* values_p_v_p = cds.second;
    unsigned int numCells = shapes_p_v_p->size();

    unsigned rn = 0;
    for (unsigned int i = 0; i < numCells; i++) {
      const T* values_p = values_p_v_p->at(i);
      os << rn++ << " - shape=[" << shapes_p_v_p->at(i)->first << "," << shapes_p_v_p->at(i)->second << "]" << " - values = [" ;
      for (unsigned int iChan = 0; iChan < shapes_p_v_p->at(i)->first ; iChan++) {
	os << "[";
	for (unsigned int iCorr = 0; iCorr < shapes_p_v_p->at(i)->second; iCorr++) {
	  cout << (int)(*values_p);
	  values_p++;
	  if (iCorr < shapes_p_v_p->at(i)->second - 1) os << ",";
	}
	os << "]";
	if (iChan <  shapes_p_v_p->at(i)->first - 1) os << ",";
      }
      os << "]" << endl;
    }
    LOGEXIT("MSFlagAccumulator::dump");
  }
}; // end of class MSFlagAccumulator


//
// A collection of declarations and functions used for the parsing of the 'scans' option.
//
//#include <boost/spirit.hpp>
#include <boost/spirit/include/classic.hpp>
//#include <boost/spirit/actor/assign_actor.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
//#include <boost/spirit/actor/push_back_actor.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
using namespace boost::spirit::classic;

vector<int> eb_v;
int allEbs = -1;
int ebNumber = allEbs;
int readEb = allEbs;

set<int> scan_s;
int scanNumber0, scanNumber1;

map<int, set<int> > eb_scan_m;

/*
** Inserts all the integer values in the range [scanNumber0, scanNumber1] in the set referred 
** to by the global variable scan_s.
** The two parameters begin and end are not used and here only to comply with the Spirit parser's convention.
*/
void fillScanSet(const char* begin, const char* end) {
  for (int i = scanNumber0; i < (scanNumber1+1); i++)
    scan_s.insert(i);
}

/*
** Inserts all the elements of the set referred to by the global variable scan_s into
** the set associated with the (int) key equal to the last value of the global vector eb_v
** in the global map eb_scan_m.
** The two parameters begin and end are not used and here only to comply with the Spirit parser's convention.
*/  
void mergeScanSet(const char* begin, const char* end) {
  int key = eb_v.back();
  eb_scan_m[key].insert(scan_s.begin(), scan_s.end());
}

/*
** Empties the global set scan_s.
** The two parameters begin and end are not used and here only to comply with the Spirit parser's convention.
*/
void clearScanSet(const char* begin, const char* end) {
  scan_s.clear();
}

/*
** Defines the grammar and the behaviour of a Spirit parser
** able to process a scan selection.
*/
struct eb_scan_selection : public grammar<eb_scan_selection> {
  template<typename ScannerT> struct definition {
    definition (eb_scan_selection const& self) {
      eb_scan_list   = eb_scan >> *(';' >> eb_scan);
      eb_scan	     = (eb[push_back_a(eb_v, ebNumber)][assign_a(ebNumber, allEbs)] >> scan_list)[&mergeScanSet][&clearScanSet];
      eb	     = !(int_p[assign_a(readEb)] >> ':')[assign_a(ebNumber, readEb)];
      scan_list	     = !((scan_selection >> *(',' >> scan_selection)));
      scan_selection = (int_p[assign_a(scanNumber0)][assign_a(scanNumber1)] >> !('~' >> int_p[assign_a(scanNumber1)]))
	[&fillScanSet]
	[assign_a(scanNumber0, -1)]
	[assign_a(scanNumber1, -1)];
    }
    rule<ScannerT> eb_scan_list, eb_scan, eb, scan_list, scan_selection;
    rule<ScannerT> const& start() const { return eb_scan_list ; }
  };
};


/*
** The following functions drive the processing of the BDF flags
*/
void traverseBAB(bool					sameAntenna,
		 const vector<SDMDataObject::Baseband>& basebands,
		 const vector<pair<string, string> >&	dataDescriptions,
		 const pair<unsigned			int, const FLAGSTYPE*> & flagsPair,
		 BDFFlagConsumer<FLAGSTYPE>&		consumer, 
		 MSFlagEval&				flagEval,
		 MSFlagAccumulator<char>&               accumulator ) {

  LOGENTER("traverseBAB");
  vector<pair<string, string> >::const_iterator ddIter = dataDescriptions.begin();

  vector<SDMDataObject::Baseband>::const_iterator babIter = basebands.begin();
  const vector<SDMDataObject::SpectralWindow>& spw = babIter->spectralWindows();
  vector<SDMDataObject::SpectralWindow>::const_iterator spwIter = spw.begin(); 

  unsigned int		 numFlags = flagsPair.first;

  BOOST_FOREACH (SDMDataObject::Baseband bab, basebands) {
    const vector<SDMDataObject::SpectralWindow>& spws = bab.spectralWindows();
    BOOST_FOREACH(SDMDataObject::SpectralWindow spw, spws) {
      vector<StokesParameter>::const_iterator ppIter, ppBegin, ppEnd;
      ppIter = ppBegin = sameAntenna ? spw.sdPolProducts().begin() : spw.crossPolProducts().begin();
      ppEnd = sameAntenna ? spw.sdPolProducts().end() : spw.crossPolProducts().end();
      unsigned int numPolProducts = ppEnd - ppBegin; 

     if (ddIter == dataDescriptions.end())
	throw ProcessFlagsException ("The number of DataDescription declared in the ConfigDescription table does not fit with the number of Basebands and SpectralWindows declared in the Main header ot the BDF");
     //cout << ddIter->first << "(" << spw.numSpectralPoint() << ")," << ddIter->second << "(" <<  numPolProducts<< ") - ";
     ddIter++;
     
     //cout << " Flags[" << spw.numSpectralPoint() << ", " << numPolProducts << "]";
     vector<char>  MSFlagsCell(spw.numSpectralPoint()*numPolProducts, (char) 0);
     if (numFlags) {
       pair<const FLAGSTYPE*, const FLAGSTYPE*> range  = consumer.consume(numPolProducts);
       unsigned int k = 0;
       for (unsigned int i = 0; i < spw.numSpectralPoint(); i++){
	 for (const FLAGSTYPE* p = range.first; p != range.second; p++) 
	   MSFlagsCell[k++] = flagEval(*p) ; 
       }
     }
     accumulator.accumulate(spw.numSpectralPoint(), numPolProducts, MSFlagsCell);
     //cout << endl;
    }
  }
  LOGEXIT("traverseBAB");
}

void traverseANT(const vector<SDMDataObject::Baseband>& basebands,
		 const vector<string>&			antennas,
		 const vector<pair<string, string> >&	dataDescriptions,
		 const pair<unsigned			int, const FLAGSTYPE*> &			flagsPair,
		 BDFFlagConsumer<FLAGSTYPE>&            consumer, 
		 MSFlagEval&				flagEval, 
		 MSFlagAccumulator<char>&                   accumulator) {

  LOGENTER("traverseANT");
  for (unsigned int i = 0; i < antennas.size() ; i++) {
    accumulator.BALStep();
    traverseBAB(true, basebands, dataDescriptions, flagsPair, consumer, flagEval, accumulator);
  }
  LOGEXIT("traverseANT");
}

void traverseBAL(const vector<SDMDataObject::Baseband>& basebands,
		 const vector<string>&			antennas,
		 const vector<pair<string, string> >&	dataDescriptions,
		 const pair<unsigned int, const FLAGSTYPE*> &			flagsPair,
		 BDFFlagConsumer<FLAGSTYPE>&                   consumer, 
		 MSFlagEval& flagEval,
		 MSFlagAccumulator<char>&                   accumulator) {

  LOGENTER("traverseBAL")
  for (unsigned int i = 0; i < antennas.size(); i++)
    for (unsigned int j = i+1; j < antennas.size(); j++) {
      accumulator.BALStep();
      traverseBAB(false, basebands, dataDescriptions, flagsPair, consumer, flagEval, accumulator);
    }
  LOGEXIT("traverseBAL")
}

void traverseALMACorrelatorFlagsAxes(const vector<SDMDataObject::Baseband>&	basebands,
				     const vector<string>&			antennas,
				     const vector<pair<string, string> >&	dataDescriptions,
				     const pair<unsigned int, const FLAGSTYPE*>& flagsPair,
				     MSFlagEval&                               flagEval,
				     MSFlagAccumulator<char>&                   accumulator) {
  LOGENTER("traverseALMACorrelatorFlagsAxes");
  
  const FLAGSTYPE*	flags_p	 = flagsPair.second;
  unsigned int		numFlags = flagsPair.first;
  BDFFlagConsumer<FLAGSTYPE> consumer(flags_p, numFlags);
  accumulator.timeStep();
  // Attention the next two calls must be done in *this* order (BAL then ANT) !!!!
  traverseBAL(basebands, antennas, dataDescriptions, flagsPair, consumer, flagEval, accumulator);
  traverseANT(basebands, antennas, dataDescriptions, flagsPair, consumer, flagEval, accumulator);

  LOGEXIT("traverseALMACorrelatorFlagsAxes");
}

void traverseALMARadiometerFlagsAxes(unsigned int				numTime,
				     const vector<SDMDataObject::Baseband>&	basebands,
				     const vector<string>&			antennas,
				     const vector<pair<string, string> >&	dataDescriptions,
				     const pair<unsigned int, const FLAGSTYPE*> &                      flagsPair,
				     MSFlagEval&                               flagEval,
				     MSFlagAccumulator<char>&                   accumulator) {
  LOGENTER("traverseALMARadiometerFlagsAxes");

  const FLAGSTYPE*	flags_p	 = flagsPair.second;
  unsigned int		numFlags = flagsPair.first;
  BDFFlagConsumer<FLAGSTYPE> consumer(flags_p, numFlags);

  for (unsigned int i = 0; i < numTime; i++) {
    accumulator.timeStep();
    traverseANT(basebands, antennas, dataDescriptions, flagsPair, consumer, flagEval, accumulator);
  }  
  LOGEXIT("traverseALMARadiometerFlagsAxes");
}

pair<uInt, uInt> put(MSFlagAccumulator<char>& accumulator,
		     uInt iRow0,
		     ArrayColumn<Bool>& flag,
		     ScalarColumn<Bool> flagRow) {
  LOGENTER("put");
  pair< vector<pair <unsigned int, unsigned int>* >*,
	vector<char*>* > cds = accumulator.cellDescriptors(true);
  
  vector<pair <unsigned int, unsigned int> *>* shapes_p_v_p = cds.first;
  vector<char*>* values_p_v_p = cds.second;
  unsigned int numCells = shapes_p_v_p->size();
  
  uInt numFlaggedRows = 0;
  for (unsigned int i = 0; i < numCells; i++) {
    const char* values_p = values_p_v_p->at(i);
    int numChan = shapes_p_v_p->at(i)->first;
    int numCorr = shapes_p_v_p->at(i)->second;

    bool cellFlagged = false;
    bool flagged = false;

    const char* p = values_p;
    Matrix<Bool> flagCell(IPosition(2, numCorr, numChan));
    flag.get((uInt)iRow0, flagCell);
    
    bool allSet = true;
    for (uInt i = 0;  i < numChan; i++)
      for (uInt j = 0; j < numCorr; j++) {
	flagged = (*p & 0x0000FFFF) != 0;
	flagCell(j, i) = flagCell(j, i) || flagged;    // Let's OR the content of flag with what's found in the BDF flags.
	cellFlagged = cellFlagged || flagged;
	allSet = allSet && flagged;
	p++;
      }
    
    if (cellFlagged) numFlaggedRows ++;
    
    flag.put((uInt)iRow0, flagCell);
    flagRow.put((uInt)iRow0, flagRow.get((uInt)iRow0) || allSet);  // Let's OR the content of flagRow with what's found in the BDF flags.
    iRow0++;
  }

  LOGEXIT("put");
  return make_pair<uInt, uInt> (iRow0, numFlaggedRows);
}

void loadBDFlags(map<string, unsigned int>& abbrev2bitpos) {
  LOGENTER("void loadBDFlags(map<string, unsigned int>& abbrev2bitpos");

  string bdflagsFilename = "bdflags.abbrev.txt";
  char * rootDir_p;
  if ((rootDir_p = getenv("CASAPATH")) != 0) {
    string rootPath(rootDir_p);
    vector<string> rootPathElements;
    split(rootPathElements, rootPath, is_any_of(" "));
    string bdflagsPath = rootPathElements[0];
    if (!ends_with(bdflagsPath, "/")) bdflagsPath+="/";
    bdflagsPath+="data/alma/asdm/";
    bdflagsPath+=bdflagsFilename;
    
    if (!exists(path(bdflagsPath))) {
    errstream.str("");
    errstream << "The file '" << bdflagsPath << "' containing the collection of BDF flags can't be found." << endl;
    error(errstream.str());      
    }

    ifstream bdflagsAbbrev;
    bdflagsAbbrev.clear(ifstream::failbit | ifstream::badbit );
    string line;
    unsigned int bitposition = 0;  
    try {
      bdflagsAbbrev.open(bdflagsPath.c_str(), fstream::in);
      while (! bdflagsAbbrev.eof()) {
	getline(bdflagsAbbrev, line);
	if (line.size() > 0)
	  abbrev2bitpos[line] = bitposition++;
      }
      bdflagsAbbrev.close();
    }
    catch (ifstream::failure e) {
      errstream.str("");
      errstream << "I/O error with the file containing the collection of BDF flags, the message was '" << e.what() << "'." << endl;
      error(errstream.str());
    }
  }
  else {
    errstream.str("");
    errstream << "The environment variable CASAPATH does not seem to be defined; I can't locate the collections of BDF Flags" << endl;
    error(errstream.str());
  }
  LOGEXIT("void loadBDFlags(map<string, unsigned int>& abbrev2bitpos");
}

typedef map<string, unsigned int> s2ui;
typedef pair<const string, unsigned int> s_ui; 

int main (int argC, char * argV[]) {
  LOGENTER("int main (int argC, char * argV[])");
  string dsName;
  string msName;
  bitset<32> flagmask;
  map<string, unsigned int> abbrev2bitpos;

  appName = string(argV[0]);

  // Load the BDF flags abbreviations.
  loadBDFlags(abbrev2bitpos);
  
  string abbrevList;
  for_each (abbrev2bitpos.begin(), abbrev2bitpos.end(), abbrevList += "\n\t* "+bind(&s2ui::value_type::first, _1));

  po::variables_map vm;

  try {
    string flagcondDoc = "specifies the list of flagging conditions to consider. The list must be a white space separated list of valid flagging conditions names. Note that the flag names can be shortened as long as there is no ambiguity (i.e. \"FFT, SI\" is valid and will be interpreted as \"FFT_OVERFLOW , SIGMA_OVERFLOW\"). If no flagging condition is provided the application exits immediately. A flag is set at the appropriate location in the MS Main row whenever at least one of the flagging conditions present in the option --flagcond is read at the relevant location in the relevant BDF file. The flagging conditions are :\n" + abbrevList + "\n\n. Note that the special value \"ALL\" to set all the flagging conditions.";
    po::options_description generic("Generates MS Flag information from the flagging conditions contained in the BDF files of an ASDM dataset.\n\n"
				    "Usage : \n" + appName + "[options] asdm-directory ms-directory \n\n"
				    "Command parameters : \n"
				    "asdm-directory : the pathname to the ASDM dataset to read \n"
				    "ms-directory : the pathname of the MS where the flagging information will be written. \n"
				    "\n\n"
				    "Allowed options:");
    generic.add_options()
      ("help", "produces help message.")
      ("flagcond,f", po::value<string>()->default_value(""), flagcondDoc.c_str())
      ("scans,s", po::value<string>(), "processes only the scans specified in the option's value. This value is a semicolon separated list of scan specifications. A scan specification consists in an exec bock index followed by the character ':' followed by a comma separated list of scan indexes or scan index ranges. A scan index is relative to the exec block it belongs to. Scan indexes are 1-based while exec blocks's are 0-based. \"0:1\" or \"2:2~6\" or \"0:1,1:2~6,8;2:,3:24~30\" \"1,2\" are valid values for the option. \"3:\" alone will be interpreted as 'all the scans of the exec block#3'. An scan index or a scan index range not preceded by an exec block index will be interpreted as 'all the scans with such indexes in all the exec blocks'.  By default all the scans are considered.");
    
    po::options_description hidden("Hidden options");
    hidden.add_options()
      ("asdm-directory", po::value<string>(), "asdm directory")
      ("ms-directory", po::value<string>(), "ms directory")
      ;
    
    po::options_description cmdline_options;
    cmdline_options.add(generic).add(hidden);

    po::positional_options_description p;
    p.add("asdm-directory", 1);
    p.add("ms-directory", 1);
    
    // Parse the command line and retrieve the options and parameters.
    po::store(po::command_line_parser(argC, argV).options(cmdline_options).positional(p).run(), vm);
    po::notify(vm);

    // Help ? displays help's content and don't go further.
    if (vm.count("help")) {
      errstream.str("");
      errstream << generic << "\n" ;
      error(errstream.str());
    }

    // What's the dataset to use ?
    if (vm.count("asdm-directory")) {
      string dummy = vm["asdm-directory"].as< string >();
      dsName = trim_copy(dummy) ;
      if (boost::algorithm::ends_with(dsName,"/")) dsName.erase(dsName.size()-1);
    }
    else {
      errstream.str("");
      errstream << generic ;
      error(errstream.str());
    }

    // Which MS the flagging informations
    // should be written into ?
    if (vm.count("ms-directory")) {
      string dummy = vm["ms-directory"].as< string >();
      msName = trim_copy(dummy) ;
      if (boost::algorithm::ends_with(msName,"/")) msName.erase(msName.size()-1);
    }
    else {
      errstream.str("");
      errstream << generic ;
      error(errstream.str());
    }
    
    if (vm.count("flagcond")) {
      istringstream iss;
      string token;
      
      flagmask.reset();

      string flagcond_opt = vm["flagcond"].as<string>();
      //
      // Stop here if no flag mask was given.
      //
      if (trim_copy(flagcond_opt).size() == 0) {
	infostream.str("");
	infostream << "No flagging condition specified." << endl;
	info(infostream.str());
	exit (0);
      }

      infostream.str("");      
      infostream << "These BDF flagging conditions have been specified :" ;
      iss.clear();
      iss.str(flagcond_opt);
      while (iss >> token) {
	if ( to_upper_copy(token) == "ALL") {
	  BOOST_FOREACH ( s_ui p, abbrev2bitpos ) {
	    flagmask.set(p.second, true);
	    infostream << " " << p.first;
	  }
	  break;
	}
	else {
	  unsigned hits = 0;
	  unsigned int bitpos = 0;
	  BOOST_FOREACH ( s_ui p , abbrev2bitpos ) {
	    if (starts_with(p.first, to_upper_copy(trim_copy(token)))) {
	      bitpos = p.second;
	      hits++;
	      infostream << " " <<  p.first;
	    }
	  }
	  switch (hits) {
	  case 0 :
	    errstream.str("");
	    errstream << "'" << token << "' does not correspond to any BDF flagging condition." << endl;
	    error(errstream.str());
	    break;
	  case 1 :
	    flagmask.set(bitpos);
	    break;
	  default :
	    errstream.str("");
	    errstream << "'" << token << "' is too ambiguous. Please provide more characters." << endl;
	    error(errstream.str());
	    break;
	  }
	}
      }
      info(infostream.str());
      infostream.str("");
      infostream << "Consequently the following flag mask will be used : " << flagmask << endl;
      info(infostream.str());
    }
  }
  catch (std::exception& e) {
    errstream.str("");
    errstream << e.what();
    error(errstream.str());
  }

  //
  // The tables will be loaded in memory only on demand.
  ASDMParseOptions parseOpt; parseOpt.loadTablesOnDemand(true);

  ASDM ds;

  // Open the dataset.
  try {
    ds.setFromFile(dsName, parseOpt);
  }
  catch (ConversionException e) {
    cout << e.getMessage() << endl;
    exit (1);
  }
  catch (...) {
    cout << "Unexpected error." << endl;
    exit (1);
  }  

  //
  // Reject the dataset if it does not contain ALMA data.
  //
  // In order to decide if it contains ALMA data, let's consider simply
  // the first row of the ExecBlock table.
  //
  const vector<ExecBlockRow *>& ebRs = ds.getExecBlock().get();
  if (ebRs.size() == 0) {
    errstream.str("");
    errstream << "The execblock table has no row; I can't determine the origin of the data." << endl;
    error(errstream.str());
  }
  ExecBlockRow * ebR = ebRs[0]; 
  string telescopeName = to_upper_copy(trim_copy(ebR->getTelescopeName()));
  vector<string> telescopeNames;
  telescopeNames += "ALMA", "OSF", "AOS";
  if (find(telescopeNames.begin(), telescopeNames.end(), telescopeName) == telescopeNames.end()) {
    errstream.str("");
    errstream << "This dataset announces telescopeName == '" << telescopeName << "', which is not ALMA. Flags can't be processed." << endl;
    error(errstream.str());
  }
  
  //
  // Selection of the scans to consider.
  //
  vector<ScanRow *>	scanRow_v	       = ds.getScan().get();
  map<int, set<int> > all_eb_scan_m;
  for (vector<ScanRow *>::size_type i = 0; i < scanRow_v.size(); i++)
    all_eb_scan_m[scanRow_v[i]->getExecBlockId().getTagValue()].insert(scanRow_v[i]->getScanNumber());

  vector<ScanRow *>	selectedScanRow_v;
  map<int, set<int> >   selected_eb_scan_m;

  string scansOptionInfo;
  if (vm.count("scans")) {
    string scansOptionValue = vm["scans"].as< string >();
    eb_scan_selection ebs;

    int status = parse(scansOptionValue.c_str(), ebs, space_p).full;

    if (status == 0) {
      errstream.str("");
      errstream << "'" << scansOptionValue << "' is an invalid scans selection." << endl;
      error(errstream.str());
    }

    vector<ScanRow *> scanRow_v = ds.getScan().get();
    map<int, set<int> >::iterator iter_m = eb_scan_m.find(-1);

    if (iter_m != eb_scan_m.end())
      for (map<int, set<int> >::iterator iterr_m = all_eb_scan_m.begin(); iterr_m != all_eb_scan_m.end(); iterr_m++)
	if ((iter_m->second).empty())
	  selected_eb_scan_m[iterr_m->first] = iterr_m->second;
	else
	  selected_eb_scan_m[iterr_m->first] = SetAndSet<int>(iter_m->second, iterr_m->second);

    for (map<int, set<int> >::iterator iterr_m = all_eb_scan_m.begin(); iterr_m != all_eb_scan_m.end(); iterr_m++)
      if ((iter_m=eb_scan_m.find(iterr_m->first)) != eb_scan_m.end())
	if ((iter_m->second).empty())
	  selected_eb_scan_m[iterr_m->first].insert((iterr_m->second).begin(), (iterr_m->second).end());
	else {
	  set<int> s = SetAndSet<int>(iter_m->second, iterr_m->second);
	  selected_eb_scan_m[iterr_m->first].insert(s.begin(), s.end());
	}

    ostringstream	oss;
    oss << "The following scans will be processed : " << endl;
    for (map<int, set<int> >::const_iterator iter_m = selected_eb_scan_m.begin(); iter_m != selected_eb_scan_m.end(); iter_m++) {
      oss << "eb#" << iter_m->first << " -> " << displaySet<int>(iter_m->second) << endl;

      Tag		execBlockTag  = Tag(iter_m->first, TagType::ExecBlock);
      for (set<int>::const_iterator iter_s = iter_m->second.begin();
	   iter_s		     != iter_m->second.end();
	   iter_s++)
	selectedScanRow_v.push_back(ds.getScan().getRowByKey(execBlockTag, *iter_s));

    }

    scansOptionInfo = oss.str();
  }
  else {
    selectedScanRow_v = ds.getScan().get();
    selected_eb_scan_m = all_eb_scan_m;
    scansOptionInfo = "All scans of all exec blocks will be processed \n";
  }
  infostream.str("");
  infostream << scansOptionInfo;
  info(infostream.str());


  // Open the MS MAIN table of the measurement set in Update mode.
  
  Table mainTable;
  ArrayColumn<Complex>	data;
  ArrayColumn<Bool>	flag;
  ScalarColumn<Bool>	flagRow;
  

  try {
    mainTable = Table (msName, Table::Update);
    data.attach(mainTable, "DATA");
    flag.attach(mainTable, "FLAG");
    flagRow.attach(mainTable, "FLAG_ROW");
  }
  catch (AipsError e) {
    errstream.str("");
    errstream << e.getMesg() << endl;
    error(errstream.str());
  }

  // Regular expressions for the correct sequences of axes in the flags in the case of ALMA data.
  boost::regex ALMACorrelatorFlagsAxesRegex("BAL ANT (BAB )?(POL )?");
  boost::regex ALMARadiometerFlagsAxesRegex("(TIM ANT )?");

  // Process the row of the Main table.
  const vector<MainRow*> & mRs = ds.getMain().get();

  ConfigDescriptionTable &	cfgT = ds.getConfigDescription();
  DataDescriptionTable &	ddT  = ds.getDataDescription();
  
  SDMDataObjectStreamReader sdosr;
  SDMDataObjectReader sdor;

  MSFlagEval flagEval(flagmask.to_ulong());

  uInt iASDMRow = 0; // Row index in the ASDM Main table.
  uInt iMSRow   = 0; // Row index in the MS Main table.
  BOOST_FOREACH (MainRow * mR, mRs) {
    // 
    // Retrieve metadata informations.
    //
    ConfigDescriptionRow * cfgR = cfgT.getRowByKey(mR->getConfigDescriptionId());

    vector<Tag> antennaTags = cfgR->getAntennaId();
    vector<string> antennas;
    for_each(antennaTags.begin(), antennaTags.end(), bind(&vector<string>::push_back, var(antennas), bind(&Tag::toString, _1)));

    vector<Tag> dataDescriptionTags = cfgR->getDataDescriptionId();
    vector<DataDescriptionRow *> ddRs;
    for_each(dataDescriptionTags.begin(), dataDescriptionTags.end(), bind(&vector<DataDescriptionRow*>::push_back, var(ddRs), bind(&DataDescriptionTable::getRowByKey, var(ddT), _1)));

    vector<pair<string, string> > dataDescriptions;
    for_each(ddRs.begin(),
	     ddRs.end(),
	     bind(&vector<pair<string, string> >::push_back,
		  var(dataDescriptions),
		  bind(&make_pair<string, string>,
		       bind(&Tag::toString,
			    bind(&DataDescriptionRow::getSpectralWindowId, *_1)),
		       bind(&Tag::toString,
			    bind(&DataDescriptionRow::getPolOrHoloId, *_1)))));

    string bdfPath = dsName+"/ASDMBinary/"+replace_all_copy(replace_all_copy(mR->getDataUID().getEntityId().toString(), "/", "_"), ":", "_");
    ProcessorType pt = cfgR->getProcessorType();
    uInt numFlaggedRows = 0;
    try {
      infostream.str("");
      infostream << "ASDM Main row #" << iASDMRow++ << " , "<< CProcessorType::toString(pt) << " data contained in " << bdfPath ;

      switch (pt) {
      case ProcessorTypeMod::CORRELATOR :
	{  
	  sdosr.open(bdfPath);
	  const SDMDataObject::BinaryPart & flagsBP = sdosr.dataStruct().flags();
	  const vector<AxisName> & flagsAxes = flagsBP.axes();
	  ostringstream oss;
	  for_each(flagsAxes.begin(), flagsAxes.end(), oss << bind(&CAxisName::toString, _1) << " ");    
	  // Check the validity of the sequence of flags axes.
	  if (!regex_match(oss.str(), ALMACorrelatorFlagsAxesRegex)) {
	    throw ProcessFlagsException("'" + oss.str() + "' is not a valid sequence of flags axes for an ALMA correlator.");
	  }

	  MSFlagAccumulator<char> accumulator;
	  while (sdosr.hasSubset()) {
	    const FLAGSTYPE * flags_p;
	    unsigned int numFlags = sdosr.getSubset().flags(flags_p);
	    pair<unsigned int, const FLAGSTYPE *> flagsPair(numFlags, flags_p);
	   
	    traverseALMACorrelatorFlagsAxes(sdosr.dataStruct().basebands(),
					    antennas,
					    dataDescriptions,
					    flagsPair,
					    flagEval,
					    accumulator);
	  }
	  //	  accumulator.dump(cout, true);
	  pair<uInt, uInt> putReturn = put(accumulator, iMSRow, flag, flagRow);
	  iMSRow = putReturn.first;
	  numFlaggedRows = putReturn.second;
	  sdosr.close();
	}	
	break;
 	
      case ProcessorTypeMod::RADIOMETER :
	{
	  const SDMDataObject& sdo = sdor.read(bdfPath);
	  const SDMDataObject::BinaryPart & flagsBP = sdo.dataStruct().flags();
	  const vector<AxisName> & flagsAxes = flagsBP.axes();
	  ostringstream oss;
	  for_each(flagsAxes.begin(), flagsAxes.end(), oss << bind(&CAxisName::toString, _1) << " ");
	  // Check the validity of the sequence of flags axes.
	  if (!regex_match(oss.str(), ALMARadiometerFlagsAxesRegex)) 
	    throw ProcessFlagsException("'" + oss.str() + "' is not a valid sequence of flags axes for an ALMA radiometer.");
	  
	  unsigned int numTime = sdo.numTime();
	  const FLAGSTYPE * flags_p;
	  unsigned int numFlags = sdo.tpDataSubset().flags(flags_p);
	  pair<unsigned int, const FLAGSTYPE *> flagsPair(numFlags, flags_p);
	  MSFlagAccumulator<char> accumulator;
	  traverseALMARadiometerFlagsAxes(numTime, sdo.dataStruct().basebands(), antennas, dataDescriptions, flagsPair, flagEval, accumulator);
	  sdor.done();
	  pair<uInt, uInt> putReturn = put(accumulator, iMSRow, flag, flagRow);
	  //	  accumulator.dump(cout, true);
	  iMSRow = putReturn.first;
	  numFlaggedRows = putReturn.second;
	}
	break;
	
      default:
	throw ProcessFlagsException("Unrecognized processor type.");
      }
      infostream << " , " << numFlaggedRows << " flagged rows." << endl;
      info(infostream.str());
    }
    catch (AipsError e) {
      info(infostream.str());
      infostream.str("");
      infostream << e.getMesg() << endl;
      info(infostream.str());
    }
    catch (ProcessFlagsException e) {
      info(infostream.str());
      infostream.str("");
      infostream << e.getMessage() << " , bdf path = " << bdfPath << ", processor type = " << CProcessorType::toString(pt) << endl;
      info(infostream.str());
    }
    catch (SDMDataObjectStreamReaderException e) {
      info(infostream.str());
      infostream.str("");
      infostream << e.getMessage() << endl;
      info(infostream.str());
    }
  }
  mainTable.flush(); 

  LOGEXIT("int main (int argC, char * argV[])");
}
