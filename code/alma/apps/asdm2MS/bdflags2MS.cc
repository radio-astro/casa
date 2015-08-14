#include <iostream>
#include <sstream>
#include <bitset>
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

#include "ASDMAll.h"

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

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/program_options.hpp>

#include <boost/assign/std/vector.hpp> // for 'operator+=()'
using namespace boost::assign; // bring 'operator+=()' into scope

bool verbose = true;

#include "CScanIntent.h"
using namespace ScanIntentMod;
#include "CSubscanIntent.h"
using namespace SubscanIntentMod;

/*
** A simplistic tracing toolbox.
*/
bool debug = (getenv("ASDM_DEBUG") != NULL);
vector<char> logIndent;
void hack01(char x) { cout << x; }
struct hack02 {
  string &acc;
  hack02(string &a) : acc(a) { }
  void operator( )(const map<string, unsigned int>::value_type &v) { acc += "\n\t* " + v.first; }
};
struct hack03 {
  DataDescriptionTable &table;
  vector<DataDescriptionRow*> &acc;
  hack03(DataDescriptionTable &t, vector<DataDescriptionRow*> &a) : table(t), acc(a) { }
  void operator( )(const Tag &tag) {
    acc.push_back( table.getRowByKey(tag) );
  }
};
struct hack04 {
  vector<string> &acc;
  hack04(vector<string> &v) : acc(v) { }
  void operator( )(const Tag &tag) {
    acc.push_back(tag.toString( ));
  }
};
struct hack05 {
  vector<pair<string, string> > &acc;
  hack05(vector<pair<string, string> > &a) : acc(a) { }
  void operator( )(const DataDescriptionRow *row) {
    acc.push_back(make_pair(row->getSpectralWindowId( ).toString( ),row->getPolOrHoloId( ).toString( )));
  }
};
struct hack06 {
  ostringstream &out;
  hack06(ostringstream &o) : out(o) { }
  void operator( )(AxisName ax) {
    out << CAxisName::toString(ax) << " ";
  }
};
#define LOGENTER(name) if (debug) {for_each(logIndent.begin(), logIndent.end(), hack01); logIndent.push_back('\t'); cout << #name ": entering" << endl;}
#define LOGEXIT(name)  if (debug) {logIndent.pop_back(); for_each(logIndent.begin(), logIndent.end(), hack01); cout << #name ": exiting" << endl;}
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
** Some typedefs for quite complex data structures used in many places 
*/
typedef pair <unsigned int, unsigned int> FLAG_SHAPE;
typedef vector<char> FLAG_V;
typedef pair<FLAG_SHAPE, FLAG_V> FLAG_CELL; 

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
    LOGENTER("BDFFlagConsumer::pair<const T* , const T*>  consume(unsigned int n)");
    if (i0 >= numItems) return make_pair<const T*, const T*>(NULL, NULL);
    i1 = i0 + n;
    if (i1 > numItems)
      return make_pair<const T*, const T*>(NULL, NULL);
    
    const T* p0 = items_p + i0;
    const T* p1 = items_p + i1;
    i0 = i1;
    LOGEXIT("BDFFlagsConsumer::pair<const T* , const T*>  consume(unsigned int n)");
    return make_pair(p0, p1);
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
  FLAGSTYPE ignore;

public:
  MSFlagEval(FLAGSTYPE mask, FLAGSTYPE ignore = 0 ); // 0 plays the role of a kind of neutral element as a matter of flagging.
  ~MSFlagEval();
  char operator()(FLAGSTYPE);
  MSFlagEval& operator=(const MSFlagEval& other);
};

MSFlagEval::MSFlagEval(FLAGSTYPE mask, FLAGSTYPE ignore):mask(mask),ignore(ignore) {;}
MSFlagEval::~MSFlagEval() {;}
char MSFlagEval::operator()(FLAGSTYPE f) { return ((f == ignore) || (f & mask & 0x0000FFFF) == 0) ? 0 : 1; }
MSFlagEval& MSFlagEval::operator=(const MSFlagEval& other) {
  if ( this != &other ) {
    mask = other.mask;
  }
  return *this;
}

// end of class MSFlagEval

/*
** A class to define a machine accumulating MS Flags cells and their descriptors as the BDF Flags are traversed 
** the BDFlag consumer.
*/
template <typename T>
class MSFlagAccumulator {
private:
  unsigned int numIntegration_;
  unsigned int numBAL_;
  unsigned int numDD_;

  unsigned int currIntegration_;
  unsigned int currBAL_;
  unsigned int currDD_;

  vector < vector < vector < FLAG_CELL > > > flagCell_vvv;  // The 1st axis ( left ) is TIME
                                                            // The 2nd axis ( middle ) is BAL or ANT
                                                            // The 3rd axis ( right ) is DD

  vector<T> MSFlags_v;

  vector< FLAG_SHAPE * > flagShapes_p_v;
  vector< FLAG_V * > flagValues_p_v;

  unsigned int numFlaggedRows;
  MSFlagAccumulator();

public:
  MSFlagAccumulator(unsigned int numIntegration,
		    unsigned int numBAL,
		    unsigned int numDD) {
    LOGENTER("MSFlagAccumulator::MSFlagAccumulator()");
    this->numBAL_ = numBAL;
    this->numIntegration_ = numIntegration;
    this->numDD_ = numDD;
    vector<FLAG_CELL> v(numDD_);
    vector <vector<FLAG_CELL> > vv(numBAL_, v);
    flagCell_vvv.assign(numIntegration_, vv);

    currIntegration_ = 0;
    currBAL_ = 0;
    currDD_ = 0;

    numFlaggedRows = 0;
    LOGEXIT("MSFlagAccumulator::MSFlagAccumulator()");
  }

  virtual ~MSFlagAccumulator() {;}
  void accumulate(unsigned int numChan, unsigned int numPol, const vector<T>& values) {
    LOGENTER("MSFlagAccumulator::accumulate(unsigned int numChan, unsigned int numPol, T* values)");
    //cout << numIntegration_ << ", " << numBAL_ << ", " << numDD_ << endl;
    //cout << currIntegration_ << ", " << currBAL_ << ", " << currDD_ << endl;
    vector<T> values_ = values;
    flagCell_vvv[currIntegration_][currBAL_][currDD_] = make_pair(make_pair(numChan, numPol), values_);
    LOGEXIT("MSFlagAccumulator::accumulate(unsigned int numChan, unsigned int numPol, T* values)");
  }

  unsigned int numIntegration() const {
    return numIntegration_;
  }

  unsigned int numBAL() const {
    return numBAL_;
  }

  unsigned int numDD() const {
    return numDD_;
  }

  void nextBAL() {
    LOGENTER("MSFlagAccumulator::nextBAL()");
    currBAL_++;
    LOGEXIT("MSFlagAccumulator::nextBAL()");
  }

  void resetBAL() {
    LOGENTER("MSFlagAccumulator::resetBAL()");
    currBAL_=0;
    LOGEXIT("MSFlagAccumulator::resetBAL()");
  }

  void nextDD() {
    LOGENTER("MSFlagAccumulator::nextDD()");
    currDD_++;
    LOGEXIT("MSFlagAccumulator::nextDD()");
  }

  void resetDD() {
    LOGENTER("MSFlagAccumulator::resetDD()");
    currDD_=0;
    LOGEXIT("MSFlagAccumulator::resetDD()");
  }

  void nextIntegration() {
    LOGENTER("MSFlagAccumulator::nextIntegration()");
    currIntegration_++;
    LOGEXIT("MSFlagAccumulator::nextIntegration()");
  }

  void resetIntegration() {
    LOGENTER("MSFlagAccumulator::resetIntegration()");
    currIntegration_=0;
    LOGEXIT("MSFlagAccumulator::resetIntegration()");
  }

  pair< vector<FLAG_SHAPE *>*,
	vector<FLAG_V *>* >  orderedByDDTIMBAL() {
    LOGENTER("MSFlagAccumulator::orderedByDDTIMBAL(bool MSORDER=true)");

    flagShapes_p_v.clear();
    flagShapes_p_v.resize(numIntegration_*numBAL_*numDD_);
    flagValues_p_v.clear();
    flagValues_p_v.resize(numIntegration_*numBAL_*numDD_);


    //cout << "Size of flagShapes_p_v = " << flagShapes_p_v.size() << endl;
    //cout << "numDD_ = " << numDD_ <<", numIntegration_ = " << numIntegration_ << ", numBAL_ = " << numBAL_ << endl;
    
    unsigned int k = 0;
    for (unsigned int iDD = 0; iDD < numDD_; iDD++) 
      for (unsigned int iIntegration = 0; iIntegration < numIntegration_; iIntegration++)
	for (unsigned int iBAL = 0; iBAL < numBAL_; iBAL++) {
	  flagShapes_p_v[k] = &(flagCell_vvv[iIntegration][iBAL][iDD].first);
	  flagValues_p_v[k] = &(flagCell_vvv[iIntegration][iBAL][iDD].second);
	  //cout << "iDD = " << iDD << ", iIntegration = " <<  iIntegration << ", iBAL = " << iBAL << ", k = " << k
	  //		 << "flagShape = [" << flagShapes_p_v[k]->first << "," << flagShapes_p_v[k]->second << "]" << endl;
	  k++;
	}

    LOGEXIT("MSFlagAccumulator::orderedByDDTIMBAL(bool MSORDER=true)");
    return make_pair< vector<FLAG_SHAPE *> *, vector<FLAG_V * > *> (&flagShapes_p_v, &flagValues_p_v);
  }

  void info (ostream& os) {
    LOGENTER("MSFlagAccumulator::info");
    os << "I have " << numIntegration_ * numBAL_ * numDD_  << " MS flag cells." << endl;
    LOGEXIT("MSFlagAccumulator::info");
  }

  void dump (ostream& os, bool MSORDER=true) {
    LOGENTER("MSFlagAccumulator::dump");

    pair< vector<FLAG_SHAPE *>*,
	vector<FLAG_V *>* >cds = orderedByDDTIMBAL(MSORDER);
  
    vector<FLAG_SHAPE *>* shapes_p_v_p = cds.first;
    vector<FLAG_V*>* values_p_v_p = cds.second;
    unsigned int numCell = shapes_p_v_p->size();
    // cout << numCell << endl;
    unsigned rn = 0;
    for (unsigned int i = 0; i < numCell; i++) {
      os << rn++ << " - shape=[" << shapes_p_v_p->at(i)->first << "," << shapes_p_v_p->at(i)->second << "]" << endl;
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

  accumulator.resetDD();
  BOOST_FOREACH (SDMDataObject::Baseband bab, basebands) {
    const vector<SDMDataObject::SpectralWindow>& spws = bab.spectralWindows();
    BOOST_FOREACH(SDMDataObject::SpectralWindow spw, spws) {
      vector<StokesParameter>::const_iterator ppIter, ppBegin, ppEnd;
      ppIter = ppBegin = sameAntenna ? spw.sdPolProducts().begin() : spw.crossPolProducts().begin();
      ppEnd = sameAntenna ? spw.sdPolProducts().end() : spw.crossPolProducts().end();
      unsigned int numPolProducts = ppEnd - ppBegin; 
      
      if (ddIter == dataDescriptions.end())
	throw ProcessFlagsException ("The number of DataDescription declared in the ConfigDescription table does not fit with the number of Basebands and SpectralWindows declared in the Main header ot the BDF");
      
      // If we are with auto correlations and 4 polarizations; don't forget that only three (e.g. XX XY YY) are stored in the BDF while 4 must be stored in the MS (XX, XY, YX, YY).
      unsigned int flagsCellNumPolProducts =  (numPolProducts == 3 && sameAntenna) ? 4 : numPolProducts;


      //cout << ddIter->first << "(" << spw.numSpectralPoint() << ")," << ddIter->second << "(" <<  numPolProducts<< ") - " << endl;
      //ddIter++;      
      //cout << " Flags[" << spw.numSpectralPoint() << ", " << numPolProducts << "]";
      vector<char>  MSFlagsCell(spw.numSpectralPoint()*flagsCellNumPolProducts, (char) 0);
      if (numFlags) {
	// cout << "flagsCellNumPolProducts = " << flagsCellNumPolProducts  << "..."  << endl;
	pair<const FLAGSTYPE*, const FLAGSTYPE*> range  = consumer.consume(numPolProducts);
	unsigned int k = 0;
	for (unsigned int i = 0; i < spw.numSpectralPoint(); i++){
	  unsigned int kk = 0;
	  for (const FLAGSTYPE* p = range.first; p != range.second; p++){
	    MSFlagsCell[k] = flagEval(*p) ; k++;
	    kk++;
	    if ((flagsCellNumPolProducts == 4) && sameAntenna && ( kk == 1 )) { // If we are in a case like XX XY YX, don't forget to repeat the value of index 1 .
	      MSFlagsCell[k] =  MSFlagsCell[k-1]; k++;
	    }
	    //cout << "k= " << k ;
	    //cout << endl;
	  }
	}
	// cout << "About to exit if numFlags" << endl;
      }
      accumulator.accumulate(spw.numSpectralPoint(), flagsCellNumPolProducts, MSFlagsCell);
      accumulator.nextDD();
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
		 MSFlagAccumulator<char>&               accumulator) {

  LOGENTER("traverseANT");
  accumulator.resetBAL();
  for (unsigned int i = 0; i < antennas.size() ; i++) {
    traverseBAB(true, basebands, dataDescriptions, flagsPair, consumer, flagEval, accumulator);
    accumulator.nextBAL();
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

  LOGENTER("traverseBAL");
  accumulator.resetBAL();
  for (unsigned int i = 0; i < antennas.size(); i++)
    for (unsigned int j = i+1; j < antennas.size(); j++) {      
      traverseBAB(false, basebands, dataDescriptions, flagsPair, consumer, flagEval, accumulator);
      accumulator.nextBAL();
    }
  LOGEXIT("traverseBAL");
}

void traverseALMACorrelatorFlagsAxes(const vector<SDMDataObject::Baseband>&	basebands,
				     const vector<string>&			antennas,
				     const vector<pair<string, string> >&	dataDescriptions,
				     const pair<unsigned int, const FLAGSTYPE*>& flagsPair,
				     MSFlagEval&                               flagEval,
				     CorrelationModeMod::CorrelationMode        correlationMode,
				     MSFlagAccumulator<char>&                   autoAccumulator,
				     MSFlagAccumulator<char>&                   crossAccumulator) {
  LOGENTER("traverseALMACorrelatorFlagsAxes");
  
  const FLAGSTYPE*	flags_p	 = flagsPair.second;
  unsigned int		numFlags = flagsPair.first;
  BDFFlagConsumer<FLAGSTYPE> consumer(flags_p, numFlags);

  // Attention the next two calls must be done in *this* order (BAL then ANT) !!!!
  if (correlationMode != CorrelationModeMod::AUTO_ONLY) 
    traverseBAL(basebands, antennas, dataDescriptions, flagsPair, consumer, flagEval, crossAccumulator);
  if (correlationMode != CorrelationModeMod::CROSS_ONLY) 
    traverseANT(basebands, antennas, dataDescriptions, flagsPair, consumer, flagEval, autoAccumulator);

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
  
  accumulator.resetIntegration();
  for (unsigned int iTime = 0; iTime < numTime; iTime++) {
    //cout << "iTime = " << iTime << endl;
    traverseANT(basebands, antennas, dataDescriptions, flagsPair, consumer, flagEval, accumulator);
    accumulator.nextIntegration();
  }  
  LOGEXIT("traverseALMARadiometerFlagsAxes");
}

bool isNotNull(char f){
  return f != 0;
}

/**
 * 
 * Populates one cell of the columns FLAG and FLAG_ROW from the content of what's been found in the BDF flags attachment.
 *
 * 
 * @parameter flagsShape_p a pointer to the shape of the flag in the MS Main row number iRow0.
 * @parameter flag_v_p a pointer to a vector of flags as a flattened version of the 2D natural represenation of flags. 
 * @parameter iRow0 the row number in the MS Main table
 * @parameter flag the column FLAG in the MS Main table
 * @parameter flagRow the column FLAG_ROW in the MS Main table
 */
bool  putCell( FLAG_SHAPE* flagShape_p,
	       FLAG_V* flag_v_p,
	       uInt iRow0,
	       ArrayColumn<Bool>& flag,
	       ScalarColumn<Bool> flagRow) {
  LOGENTER("putCell");
  uInt numChan = flagShape_p->first;
  uInt numCorr = flagShape_p->second;
  
  bool cellFlagged = false;
  bool flagged = false;
  
  Matrix<Bool> flagCell(IPosition(2, numCorr, numChan));
  if (debug) {
    cout << "irow0 = " << iRow0 << endl;
    cout << "expectin g a cell of shape numCorr=" << numCorr << ", numChan=" << numChan << endl;
    cout << "actual shape is " << flag.shape(iRow0) << endl;
  }
  flag.get((uInt)iRow0, flagCell);
  bool allSet = true;
  unsigned int notNullBDF =  count_if(flag_v_p->begin(), flag_v_p->end(), isNotNull);
  //if (notNullBDF) cout << "row " << iRow0 << " - putCell in front of " << flag_v_p->size() << " elements of which " << notNullBDF << " are not null";
  int notNull = 0;
  int k = 0;
  for (uInt i = 0;  i < numChan; i++) {
    for (uInt j = 0; j < numCorr; j++) {
      flagged = flag_v_p->at(k++) != (char) 0;
      if (flagged) notNull++;
      flagCell(j, i) = flagged; // flagCell(j, i) || flagged;    // Let's OR the content of flag with what's found in the BDF flags.
      cellFlagged = cellFlagged || flagged;
      allSet = allSet && flagged;
    }
  }
  flag.put((uInt)iRow0, flagCell);
  //if (notNull)  cout << "row " << iRow0 <<  " - putCell counted actually" << notNull << " non null flags so that cellFlagged = " << cellFlagged << endl;

  flagRow.put((uInt)iRow0, flagRow.get((uInt)iRow0) || allSet);  // Let's OR the content of flagRow with what's found in the BDF flags.
  
  LOGEXIT("putCell");
  return cellFlagged;  
}

pair<uInt, uInt> mergeAndPut(CorrelationModeMod::CorrelationMode correlationMode,
			     MSFlagAccumulator<char>& autoAccumulator,
			     MSFlagAccumulator<char>& crossAccumulator,
			     uInt iRow0,
			     ArrayColumn<Bool>& flag,
			     ScalarColumn<Bool> flagRow) {
  LOGENTER("mergeAnPut");

  unsigned int	numIntegration = autoAccumulator.numIntegration();
  unsigned int	numANT	       = autoAccumulator.numBAL();
  unsigned int  numBAL         = crossAccumulator.numBAL();
  unsigned int	numDD	       = autoAccumulator.numDD();

  if (numIntegration != crossAccumulator.numIntegration()	||
      (numANT * (numANT - 1 )) / 2 != numBAL			||
      numDD			   != crossAccumulator.numDD() )
    throw ProcessFlagsException("The accumulators of cross and auto data flags do not have compatible dimensions");

  pair< vector<FLAG_SHAPE * >*, vector<FLAG_V * >* > autoFlagPair  = autoAccumulator.orderedByDDTIMBAL();
  pair< vector<FLAG_SHAPE * >*, vector<FLAG_V * >* > crossFlagPair = crossAccumulator.orderedByDDTIMBAL();

  vector<pair <unsigned int, unsigned int> *>* autoFlagShapes_p_v_p = autoFlagPair.first;
  vector<vector<char> *>* autoFlagValues_p_v_p = autoFlagPair.second;

  vector<pair <unsigned int, unsigned int> *>* crossFlagShapes_p_v_p = crossFlagPair.first;
  vector<vector<char> *>* crossFlagValues_p_v_p = crossFlagPair.second;

  unsigned int		kAuto	       = 0;
  unsigned int		kCross	       = 0;

  unsigned int		numFlaggedRows = 0; // Maintain a count of the number of flagged rows.

  //
  // For each Data Description...
  for (unsigned int iDD = 0; iDD < numDD; iDD++) {
    //
    // ... put the flags for auto correlation firstly
    if (correlationMode != CorrelationModeMod::CROSS_ONLY) {
      if (debug) cout << "AUTO " << numIntegration << ", " << numDD << ", " << iDD << ", " << autoFlagShapes_p_v_p->at(0)->first << ", " << autoFlagShapes_p_v_p->at(0)->second << endl;
      for (unsigned int iTIMEANT = 0; iTIMEANT < numIntegration * numANT; iTIMEANT++) {
	if (putCell(autoFlagShapes_p_v_p->at(kAuto),
		    autoFlagValues_p_v_p->at(kAuto),
		    iRow0,
		    flag, 
		    flagRow))
	  numFlaggedRows++;
	//if (count_if(autoFlagValues_p_v_p->at(kAuto)->begin(), autoFlagValues_p_v_p->at(kAuto)->end(), isNotNull) != 0)
	//  numFlaggedRows++;
	
	iRow0++;
	kAuto++;
      }
      //cout << "AUTO - numFlaggedRows " << numFlaggedRows << endl;
    }
  }

  //
  // For each Data Description...
  for (unsigned int iDD = 0; iDD < numDD; iDD++) {
    //
    // ... put the flags for cross correlation then
    if (correlationMode != CorrelationModeMod::AUTO_ONLY) {
      if (debug) cout << "CROSS " << numIntegration << ", " << iDD << ", " << crossFlagShapes_p_v_p->at(0)->first << ", " << crossFlagShapes_p_v_p->at(0)->second << endl;
      for (unsigned int iTIMEBAL = 0; iTIMEBAL < numIntegration * numBAL; iTIMEBAL++) {
	if (putCell(crossFlagShapes_p_v_p->at(kCross),
		    crossFlagValues_p_v_p->at(kCross),
		    iRow0,
		    flag, 
		    flagRow))
	  numFlaggedRows++;
	
	iRow0++;
	kCross++;
      }
      //cout << "CROSS - numFlaggedRows " << numFlaggedRows << endl;
    }
  }
  LOGEXIT("mergeAndPut");
  return make_pair(iRow0, numFlaggedRows);
}

pair<uInt, uInt> put(MSFlagAccumulator<char>& accumulator,
		     uInt iRow0,
		     ArrayColumn<Bool>& flag,
		     ScalarColumn<Bool> flagRow) {
  LOGENTER("put");
  pair< vector<FLAG_SHAPE * >*,
	vector<FLAG_V * >* > cds = accumulator.orderedByDDTIMBAL();
  
  vector<pair <unsigned int, unsigned int> *>* shapes_p_v_p = cds.first;
  vector<vector<char> *>* values_p_v_p = cds.second;
  unsigned int numCells = shapes_p_v_p->size();
  
  uInt numFlaggedRows = 0;
  //cout << "numCells = " << numCells << endl;
  for (unsigned int i = 0; i < numCells; i++) {
    FLAG_V * values_p = values_p_v_p->at(i);
    int numChan = shapes_p_v_p->at(i)->first;
    int numCorr = shapes_p_v_p->at(i)->second;

    bool cellFlagged = false;
    bool flagged = false;

    char* p = &(values_p->at(0));
    Matrix<Bool> flagCell(IPosition(2, numCorr, numChan));
    //cout << "Trying to retrieve a " << numCorr << " x " << numChan << " matrix of flags"; 
    //cout << "in a cell shaped as follows " << flag.shape((uInt) iRow0) << " at row " << iRow0 << endl;
    flag.get((uInt)iRow0, flagCell);
    
    bool allSet = true;
    for (uInt i = 0;  i < numChan; i++)
      for (uInt j = 0; j < numCorr; j++) {
	flagged = (*p & 0x0000FFFF) != 0;
	flagCell(j, i) = flagged ; // flagCell(j, i) || flagged;    // Let's OR the content of flag with what's found in the BDF flags.
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
  return make_pair(iRow0, numFlaggedRows);
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

/**
 * This function tries to calculate the sizes of the successives slices of the BDF 
 * to be processed given a) the overall size of the BDF and b) the approximative maximum
 * size that one wants for one slice.
 * @paramater BDFsize the overall size of the BDF expressed in bytes,
 * @parameter approxSizeInMemory the approximative size that one wants for one slice, expressed in byte.
 *
 * \par Note:
 * It tries to avoid a last slice, corresponding to 
 * the remainder in the euclidean division BDFsize / approxSizeInMemory.
 */
vector<uint64_t> sizeInMemory(uint64_t BDFsize, uint64_t approxSizeInMemory) {
  if (debug) cout << "sizeInMemory: entering" << endl;
  vector<uint64_t> result;
  uint64_t Q = BDFsize / approxSizeInMemory;
  if (Q == 0) { 
    result.push_back(BDFsize);
  }
  else {
    result.resize(Q, approxSizeInMemory);
    unsigned int R = BDFsize % approxSizeInMemory;
    if ( R > (Q * approxSizeInMemory / 5) )  {
      result.push_back(R); 
    }
    else {
      while (R > 0) 
	for (unsigned int i = 0; R >0 && i < result.size(); i++) {
	  result[i]++ ; R--;
	}
    }
  }
  if (debug) cout << "sizeInMemory: exiting" << endl;
  return result;
} 

int main (int argC, char * argV[]) {
  LOGENTER("int main (int argC, char * argV[])");
  string dsName;
  string msName;
  bitset<32> flagmask;
  map<string, unsigned int> abbrev2bitpos;

  appName = string(argV[0]);

  uint64_t bdfSliceSizeInMb = 500; // The default size of the BDF slice hold in memory.

  // Load the BDF flags abbreviations.
  loadBDFlags(abbrev2bitpos);
  
  string abbrevList;
  hack02 hack02_instance(abbrevList);
  for_each (abbrev2bitpos.begin(), abbrev2bitpos.end(), hack02_instance);

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
      ("scans,s", po::value<string>(), "processes only the scans specified in the option's value. This value is a semicolon separated list of scan specifications. A scan specification consists in an exec bock index followed by the character ':' followed by a comma separated list of scan indexes or scan index ranges. A scan index is relative to the exec block it belongs to. Scan indexes are 1-based while exec blocks's are 0-based. \"0:1\" or \"2:2~6\" or \"0:1,1:2~6,8;2:,3:24~30\" \"1,2\" are valid values for the option. \"3:\" alone will be interpreted as 'all the scans of the exec block#3'. An scan index or a scan index range not preceded by an exec block index will be interpreted as 'all the scans with such indexes in all the exec blocks'.  By default all the scans are considered.")
      ("wvr-corrected-data", po::value<bool>()->default_value(false), "must be set to True (resp. False) whenever the MS to be populated contains corrected (resp. uncorrected) data (default==false)");
    
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
      infostream << "Consequently the following flag mask will be used : " << flagmask << "(" << std::hex << flagmask.to_ulong() << ")" << std::dec << endl;
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
  // Selection of the kind of data - uncorrected or corrected - to consider.
  //
  infostream.str("");
  bool processUncorrectedData = true;
  if (vm.count("wvr-corrected-data")) {
    processUncorrectedData = !vm["wvr-corrected-data"].as< bool >();
  }
  infostream << "only " << (processUncorrectedData ? "uncorrected" : "corrected") << " data will be considered." << endl;
  info(infostream.str());
  
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
  // ArrayColumn<Complex>	data;
  ArrayColumn<Bool>	flag;
  ScalarColumn<Bool>	flagRow;
  

  try {
    mainTable = Table (msName, Table::Update);
    // data.attach(mainTable, "DATA");
    flag.attach(mainTable, "FLAG");
    flagRow.attach(mainTable, "FLAG_ROW");
  }
  catch (AipsError e) {
    errstream.str("");
    errstream << e.getMesg() << endl;
    error(errstream.str());
  }

  // Regular expressions for the correct sequences of axes in the flags in the case of ALMA data.
  boost::regex ALMACorrelatorFlagsAxesRegex("(BAL )?ANT (BAB )?(POL )?");
  boost::regex ALMARadiometerFlagsAxesRegex("(TIM ANT )?(BAB BIN POL )?");


  ConfigDescriptionTable &	cfgT = ds.getConfigDescription();
  DataDescriptionTable &	ddT  = ds.getDataDescription();
  
  SDMDataObjectStreamReader sdosr;
  SDMDataObjectReader sdor;


  MSFlagEval flagEval(flagmask.to_ulong(), processUncorrectedData ? 3 : 0); // If we process uncorrected data we ignore the combination
                                                                            // 3 = WVR_APC | INTEGRATION_FULLY_BLANKED

  uInt	iASDMRow    = 0;	// Row index in the ASDM Main table.

  //
  //
  // Consider only the Main rows whose execBlockId and scanNumber attributes correspond to the selection and which
  // contain the data with appropriate atmospheric phase correction characteristics.
  //
  AtmPhaseCorrectionMod::AtmPhaseCorrection queriedAPC = processUncorrectedData ? AtmPhaseCorrectionMod::AP_UNCORRECTED : AtmPhaseCorrectionMod::AP_CORRECTED;
  vector<MainRow*> v;
  vector<int32_t> mainRowIndex; 
  const vector<MainRow *>& temp = ds.getMain().get();

  for ( vector<MainRow *>::const_iterator iter_v = temp.begin(); iter_v != temp.end(); iter_v++) {
    map<int, set<int> >::iterator iter_m = selected_eb_scan_m.find((*iter_v)->getExecBlockId().getTagValue());
    if ( iter_m != selected_eb_scan_m.end() && iter_m->second.find((*iter_v)->getScanNumber()) != iter_m->second.end() ) {
      bool toBeProcessed  = cfgT.getRowByKey((*iter_v)->getConfigDescriptionId())->getProcessorType() == RADIOMETER ; // RADIOMETER data are always put in both (UN/CORRECTED) MS.
      if (!toBeProcessed) {
	vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > apc_v = cfgT.getRowByKey((*iter_v)->getConfigDescriptionId())->getAtmPhaseCorrection();
	toBeProcessed =   find(apc_v.begin(), apc_v.end(), queriedAPC) != apc_v.end();
      }
      if (toBeProcessed) {
	mainRowIndex.push_back(iter_v - temp.begin());
	v.push_back(*iter_v);
      }
    }
  }

  infostream.str("");
  infostream << "The dataset has " << temp.size() << " main(s)...";
  infostream << v.size() << " of them in the selected exec blocks / scans." << endl;
  info(infostream.str());

  uInt	iMSRow	    = 0;	// Row index in the MS Main table.
  uInt	iMSRowBegin = 0;	// Index of the first row in the MS Main table of the slice corresponding to one row in the ASDM Main table.
  uInt	iMSRowEnd   = 0;	// Index of the last row in the MS Main table of the slice corresponding to one row in the ASDM Main table.

  iMSRowBegin = iMSRow;
  unsigned int iASDMIndex = 0;
  BOOST_FOREACH (MainRow * mR, v) {
    // 
    // Retrieve metadata informations.
    //

    unsigned int numIntegration = mR->getNumIntegration();

    ConfigDescriptionRow * cfgR = cfgT.getRowByKey(mR->getConfigDescriptionId());

    vector<Tag> antennaTags = cfgR->getAntennaId();
    vector<string> antennas;
    hack04 hack04_instance(antennas);
    for_each(antennaTags.begin(), antennaTags.end(), hack04_instance);

    vector<Tag> dataDescriptionTags = cfgR->getDataDescriptionId();
    vector<DataDescriptionRow *> ddRs;
    hack03 hack03_instance(ddT,ddRs);
    for_each(dataDescriptionTags.begin(), dataDescriptionTags.end(), hack03_instance);

    unsigned int numDD = ddRs.size();

    vector<pair<string, string> > dataDescriptions;
    hack05 hack05_instance(dataDescriptions);
    for_each(ddRs.begin(),
	     ddRs.end(),
	     hack05_instance);

    string bdfPath = dsName+"/ASDMBinary/"+replace_all_copy(replace_all_copy(mR->getDataUID().getEntityId().toString(), "/", "_"), ":", "_");
    if (debug) cout << "BDF " << bdfPath << endl;
    ProcessorType pt = cfgR->getProcessorType();
    uInt numFlaggedRows = 0;
    try {
      infostream.str("");
      infostream << "ASDM Main row #" << mainRowIndex[iASDMIndex]
		 << " (scan #" << mR->getScanNumber()
		 <<", subscan #" <<  mR->getSubscanNumber()
		 <<", " << CProcessorType::toString(pt)
		 <<", " << mR->getConfigDescriptionId().toString() << ")"
		 << " - BDF '" << bdfPath << "' - Size " << mR->getDataSize() << " bytes." <<  endl;
      info(infostream.str());

      //
      // Check that the triple (execBlockId,scanNumber, subscanNumber) refers to an existing entry in the subscan table
      // if it's not the case ignore this row of the Main table to have a behaviour similar to the filler's one.
      //
      ASDM&			ds	   = mR -> getTable() . getContainer();
      ScanRow*		scanR_p	   = ds.getScan().getRowByKey(mR -> getExecBlockId(),	mR -> getScanNumber());
      vector<ScanIntent>	scanIntent = scanR_p -> getScanIntent();
      SubscanRow*		sscanR_p   = ds.getSubscan().getRowByKey(mR -> getExecBlockId(),
									 mR -> getScanNumber(),
									 mR -> getSubscanNumber());
      if (sscanR_p == 0) {
	infostream << "Could not find a row in the Subscan table for the following key value (execBlockId="
		   << mR->getExecBlockId().toString()
		  <<", scanNumber="<< mR->getScanNumber()
		  <<", subscanNum=" << mR->getSubscanNumber() << "). Aborting. "
		  << endl;
	info(infostream.str());
	continue;  // goto the next main row.
      }	  
      // End of check

      switch (pt) {
      case ProcessorTypeMod::CORRELATOR :
	{  
	  sdosr.open(bdfPath);
	  const SDMDataObject::BinaryPart & flagsBP = sdosr.dataStruct().flags();
	  const vector<AxisName> & flagsAxes = flagsBP.axes();
	  ostringstream oss;
	  hack06 hack06_instance(oss);
	  for_each(flagsAxes.begin(), flagsAxes.end(), hack06_instance);
	  // Check the validity of the sequence of flags axes.
	  if (!regex_match(oss.str(), ALMACorrelatorFlagsAxesRegex)) {
	    throw ProcessFlagsException("'" + oss.str() + "' is not a valid sequence of flags axes for an ALMA correlator.");
	  }

	  uint32_t		N			 = mR->getNumIntegration();
	  uint64_t		bdfSize			 = mR->getDataSize();
	  vector<uint64_t>	actualSizeInMemory(sizeInMemory(bdfSize, bdfSliceSizeInMb*1024*1024));
	  int32_t		numberOfMSMainRows	 = 0;
	  int32_t		numberOfIntegrations	 = 0;
	  int32_t		numberOfReadIntegrations = 0;

	  CorrelationModeMod::CorrelationMode correlationMode = sdosr.correlationMode();
	  unsigned int numBAL = antennas.size() * (antennas.size() - 1) / 2;

	  for (unsigned int j = 0; j < actualSizeInMemory.size(); j++) {
	    if (debug) cout << "PLATOON" << endl;
	    numberOfIntegrations = min(actualSizeInMemory[j] / (bdfSize / N), N); // The min to prevent a possible excess when there are very few bytes in the BDF.
	    if (debug) cout << "------------> " << numberOfIntegrations << " , " << actualSizeInMemory[j] << endl;

	    if (numberOfIntegrations) {
	      MSFlagAccumulator<char> autoAccumulator(numberOfIntegrations, antennas.size(), numDD);
	      MSFlagAccumulator<char> crossAccumulator(numberOfIntegrations, numBAL, numDD);
	      if ( correlationMode != CorrelationModeMod::CROSS_ONLY )
		autoAccumulator.resetIntegration();
	      if ( correlationMode != CorrelationModeMod::AUTO_ONLY )
		crossAccumulator.resetIntegration();

	      for (int iIntegration = 0; iIntegration < numberOfIntegrations; iIntegration++) {
		//cout << "iIntegration = " << iIntegration << endl;
		const FLAGSTYPE * flags_p;
		unsigned int numFlags = sdosr.getSubset().flags(flags_p);
		pair<unsigned int, const FLAGSTYPE *> flagsPair(numFlags, flags_p);
		traverseALMACorrelatorFlagsAxes(sdosr.dataStruct().basebands(),
						antennas,
						dataDescriptions,
						flagsPair,
						flagEval,
						correlationMode,
						autoAccumulator,
						crossAccumulator);
		if ( correlationMode != CorrelationModeMod::CROSS_ONLY )
		  autoAccumulator.nextIntegration();
		if ( correlationMode != CorrelationModeMod::AUTO_ONLY )
		  crossAccumulator.nextIntegration();
	      }
	      //cout << "About to call mergeAndPut" << endl;
	      //cout << "iMSRow = " << iMSRow << endl;
	      pair<uInt, uInt> putReturn = mergeAndPut(correlationMode,
						       autoAccumulator,
						       crossAccumulator,
						       iMSRow,
						       flag,
						       flagRow);
	      //cout << "Back from  mergeAndPut" << endl;
	      iMSRow = putReturn.first;
	      numFlaggedRows += putReturn.second;
	      numberOfReadIntegrations += numberOfIntegrations;

	      infostream.str("");

	      infostream << "ASDM Main row #" << mainRowIndex[iASDMIndex] << " - " << numberOfReadIntegrations   << "/" << N << " integrations done so far.";
	      info(infostream.str());
	    }
	  }

	  if (debug) cout << "REMAINING" << endl;
	  uint32_t numberOfRemainingIntegrations = N - numberOfReadIntegrations;
	  //cout << "--->" << numberOfRemainingIntegrations << endl;
	  if (numberOfRemainingIntegrations) {
	    MSFlagAccumulator<char> autoAccumulator(numberOfRemainingIntegrations, antennas.size(), numDD);
	    MSFlagAccumulator<char> crossAccumulator(numberOfRemainingIntegrations, numBAL, numDD);
	    if ( correlationMode != CorrelationModeMod::CROSS_ONLY )
	      autoAccumulator.resetIntegration();
	    if ( correlationMode != CorrelationModeMod::AUTO_ONLY )
	      crossAccumulator.resetIntegration(); 
	    for (int iIntegration = 0; iIntegration < numberOfRemainingIntegrations; iIntegration++) {
	      // cout << "iIntegration = " << iIntegration << endl; 
	      const FLAGSTYPE * flags_p;
	      unsigned int numFlags = sdosr.getSubset().flags(flags_p);
	      pair<unsigned int, const FLAGSTYPE *> flagsPair(numFlags, flags_p);
	      traverseALMACorrelatorFlagsAxes(sdosr.dataStruct().basebands(),
					      antennas,
					      dataDescriptions,
					      flagsPair,
					      flagEval,
					      correlationMode,
					      autoAccumulator,
					      crossAccumulator);
	      if ( correlationMode != CorrelationModeMod::CROSS_ONLY )
		autoAccumulator.nextIntegration();
	      if ( correlationMode != CorrelationModeMod::AUTO_ONLY )
		crossAccumulator.nextIntegration();
	    }
	    pair<uInt, uInt> putReturn = mergeAndPut(correlationMode,
						     autoAccumulator,
						     crossAccumulator,
						     iMSRow,
						     flag,
						     flagRow);
	    iMSRow = putReturn.first;
	    numFlaggedRows += putReturn.second;
	    numberOfReadIntegrations += numberOfIntegrations;
	    
	    infostream.str("");
	    
	    infostream << "ASDM Main row #" << mainRowIndex[iASDMIndex]   << " - " << numberOfReadIntegrations << "/" << N << " integrations done so far.";
	    info(infostream.str());
	  }
	  sdosr.close();
	}	
	break;
 
      case ProcessorTypeMod::RADIOMETER :
	{
	  const SDMDataObject& sdo = sdor.read(bdfPath);
	  const SDMDataObject::BinaryPart & flagsBP = sdo.dataStruct().flags();
	  const vector<AxisName> & flagsAxes = flagsBP.axes();
	  ostringstream oss;
	  hack06 hack06_instance(oss);
	  for_each(flagsAxes.begin(), flagsAxes.end(), hack06_instance);
	  // Check the validity of the sequence of flags axes.
	  if (!regex_match(oss.str(), ALMARadiometerFlagsAxesRegex)) 
	    throw ProcessFlagsException("'" + oss.str() + "' is not a valid sequence of flags axes for an ALMA radiometer.");
	  
	  unsigned int numTime = sdo.numTime();
	  const FLAGSTYPE * flags_p;
	  unsigned int numFlags = sdo.tpDataSubset().flags(flags_p);
	  pair<unsigned int, const FLAGSTYPE *> flagsPair(numFlags, flags_p);
	  MSFlagAccumulator<char> accumulator( numTime, antennas.size(), numDD);

	  if (numTime != numIntegration) {
	    infostream << "(the number of integrations actually read in the BDF (numTime = " << numTime << ") is different from the value announced in the Main table (numIntegration = " << numIntegration
		       << "). Using " << numTime << ")";
	  }

	  traverseALMARadiometerFlagsAxes(numTime, sdo.dataStruct().basebands(), antennas, dataDescriptions, flagsPair, flagEval, accumulator);
	  infostream.str("");
	  
	  infostream << "ASDM Main row #" << mainRowIndex[iASDMIndex] << " - " << numTime  << "/" << numTime << " integrations done so far.";
	  info(infostream.str());

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
      infostream.str("");
      infostream << "ASDM Main row #" << mainRowIndex[iASDMIndex]
		 << " (scan #" << mR->getScanNumber()
		 <<", subscan #" <<  mR->getSubscanNumber()
		 <<", " << CProcessorType::toString(pt)
		 <<", " << mR->getConfigDescriptionId().toString() << ")"
		 << " - BDF '" << bdfPath << "' - Size " << mR->getDataSize() << " bytes,  produced " << numFlaggedRows  << " flagged rows in the MS Main table rows " << iMSRowBegin << " to " << (iMSRow - 1) << endl << endl << endl;
      info(infostream.str());
      iMSRowBegin = iMSRow;
    }
    catch (AipsError e) {
      info(infostream.str());
      infostream.str("");
      infostream << e.getMesg() << endl;
      info(infostream.str());
      exit(1);
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
    iASDMIndex++;
  }
  mainTable.flush(); 

  LOGEXIT("int main (int argC, char * argV[])");
}
