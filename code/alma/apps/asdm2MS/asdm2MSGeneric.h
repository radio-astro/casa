#ifndef TableSAXReader_h
#define TableSAXReader_h
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <set>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

/**
 * A template functor which returns -|v|.
 * The operator '-' and the comparison with 0.0 ( < 0.0) must be defined on the type T.
 */
template<typename T>
struct negateFunctor {
public:
  T operator() (const T& v) {
    return (v < 0.0) ? v : -v;
  }
};

/**
 * A template function which returns a string representing the contents of a set
 * of elements of type T.
 * the operator << is expected to be defined on type T.
 * @param aSet the set.
 * @return the string containing the textual representation of the set.
 */
template<typename T>
std::string displaySet(const std::set<T> &aSet) {
  std::ostringstream oss;
  oss << "{";
  typename std::set<T>::const_iterator iter = aSet.begin();
  if (iter != aSet.end())
    oss << *iter++;
   
  while (iter != aSet.end())
    oss << "," << *iter++;  
  oss<< "}";
  return oss.str();
}

/**
 * Returns the intersection of two sets.
 *
 * @param T the base type of the two sets.
 * @param s1 the first set.
 * @param s2 the second set.
 * @return a set equal to the intersection of s1 and s2.
 */
template<typename T> 
set<T> SetAndSet(const set<T>& s1, const set<T>& s2) {
  set<T> result;
  typename set<T>::iterator iter1_s, iter2_s;
  for (iter1_s = s1.begin(); iter1_s != s1.end(); iter1_s++) {
    if ((iter2_s = s2.find(*iter1_s)) != s2.end())
      result.insert(*iter1_s);
  }
  return result;
}


template<typename T>
struct rowsInAScanbyTimeIntervalFunctor {
private:
  const vector<ScanRow *>&	scans;
  vector<T *>			result;
  
  /**
   * A function which returns true if and only there is at least
   * one element in the vector 'scans' for which the time interval
   * defined by its attributes startTime and endTime has a non empty
   * intersection with the time interval defined by the 'timeInterval'
   * attribute of the generic parameter of 'row' which is expected to 
   * have a method getTimeInterval which returns a TimeInterval object.
   *
   * @param row a pointer to an object of type T.
   * @param a const reference to a vector of pointers of ScanRow.
   * @param return a bool.
   */
  bool timeIntervalIntersectsAScan (T* row, const std::vector<asdm::ScanRow *>& scans) {
    bool result = false;
    
    int64_t currentScanStartTime, currentScanEndTime;
    int64_t rowStartTime, rowEndTime;
    for (std::vector<asdm::ScanRow *>::const_iterator iter = scans.begin(); iter != scans.end(); iter++) {
      currentScanStartTime = (*iter)->getStartTime().get();
      currentScanEndTime = (*iter)->getEndTime().get();
      
      rowStartTime = row->getTimeInterval().getStart().get();
      rowEndTime = rowStartTime + row->getTimeInterval().getDuration().get();
      if (max(currentScanStartTime, rowStartTime) < min(currentScanEndTime, rowEndTime))
	return true;
    }
    return result;
  }
  
public:
  rowsInAScanbyTimeIntervalFunctor(const std::vector<asdm::ScanRow *>& scans): scans(scans) {};
  const std::vector<T *> & operator() (const std::vector<T *>& rows, bool ignoreTime=false) {
    if (ignoreTime) return rows;
    
    result.clear();
    for (typename std::vector<T *>::const_iterator iter = rows.begin(); iter != rows.end(); iter++) {
      if (timeIntervalIntersectsAScan (*iter, scans))
	result.push_back(*iter);
    }
    return result;    
  }
};

template<typename T>
struct rowsInAScanbyTimeFunctor {
private:
  const vector<ScanRow *>&	scans;
  vector<T *>			result;

  /**
   * A template function which checks if there is at least one element scan of the vector scans for which
   * the time  contained by returned by row->getTime() is embedded in the time range defined in scan. 
   * Returns true there is such a scan.
   */
  bool timeIsInAScan(T* row, const vector<ScanRow *>& scans) {
    bool result = false;
    
    int64_t currentScanStartTime, currentScanEndTime;
    int64_t rowTime;
    rowTime = row->getTime().get();
    for (vector<ScanRow *>::const_iterator iter = scans.begin(); iter != scans.end(); iter++) {
      currentScanStartTime = (*iter)->getStartTime().get();
      currentScanEndTime = (*iter)->getEndTime().get();
      if ((currentScanStartTime <= rowTime) && (rowTime < currentScanEndTime))
	return true;
    }
    return result;
  }
  
public:
  rowsInAScanbyTimeFunctor(const vector<ScanRow *>& scans): scans(scans) {};
  const vector<T *> & operator() (const vector<T *>& rows, bool ignoreTime=false) {
    if (ignoreTime) return rows;

    result.clear();
    for (typename vector<T *>::const_iterator iter = rows.begin(); iter != rows.end(); iter++) {
      if (timeIsInAScan (*iter, scans))
	result.push_back(*iter);
    }

    return result;    
  }
};

/**
 * A boolean template functor which returns the value of the expression x.size() < y.
 * T is expected to have a method size returning an numerical value defined on it. 
 */
template<typename T>
struct size_lt {
public: 
  size_lt(unsigned int y) : y(y) {}
  bool operator()(vector<T>& x) {return x.size() < y;}

private:
  unsigned int  y;
};

/**
 * A template function which returns a string from an (expectedly) enumeration and its associated 
 * helper class. (see <..>/code/alma/implement/Enumerations)
 */
template<typename Enum, typename CEnum> 
string stringValue(Enum literal) {
  return CEnum::name(literal);
}


/**
 * template function meant to return a value of type BasicType out of a value expected to be of one of the Physical Quantities type (Pressure, Speed ....)
 * the type PhysicalQuantity is expected to have a method get returning a valye castable into BasicType.
 */
template<typename PhysicalQuantity, typename BasicType> 
BasicType basicTypeValue (PhysicalQuantity value) {
  return (BasicType) value.get();
}


/* ------------------------------------- TableSaxParser - beginning ------------------------------------------*/

#if defined(__APPLE__)
#include <mach-o/dyld.h>
std::string getexepath() {
  char path[1024];
  uint32_t size = sizeof(path);
  return  (_NSGetExecutablePath(path, &size) == 0) ? std::string(path) : "";
}
#else
#include <limits.h>
#include <unistd.h>
std::string getexepath() {
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  return std::string( result, (count > 0) ? count : 0);
}
#endif

#include <casa/Logging/StreamLogSink.h>
#include <casa/Logging/LogSink.h>


#include "ConversionException.h"
#include "CAtmPhaseCorrection.h"
#include "ASDM.h"
#include "ASDM2MSFiller.h"

#define CONTEXT_P ((ParserContext<T, R, RFilter> *)myContext_p)
#define V2CTX_P(v_p) ((ParserContext<T, R, RFilter> *) v_p)

template<class T, class R, class RFilter>
  struct ParserContext {
  public:
    enum StatesEnum {START, IN_TABLE, IN_ENTITY, AFTER_ENTITY, IN_CONTAINER_ENTITY, AFTER_CONTAINER_ENTITY, IN_ROW, AFTER_ROW, IN_ATTRIBUTE, END};
    asdm::ASDM*			asdm_p;
    unsigned int		maxNumberOfRowsInMem;
    boost::shared_ptr<R>	row_sp;
    std::vector<boost::shared_ptr<R> >  rows;
    RFilter*                    rFilter_p;
    bool                        ignoreTime;
    void (*tableFiller_f_p) (const vector<R*>&, map<AtmPhaseCorrectionMod::AtmPhaseCorrection, ASDM2MSFiller*>&);
    std::map<AtmPhaseCorrectionMod::AtmPhaseCorrection, ASDM2MSFiller*>* msFillers_m_p;
    const xmlChar*		topLevelElement_p;
    const xmlChar*		entityElement_p;
    const xmlChar*		containerEntityElement_p;
    const xmlChar*		rowElement_p;
    int				depth;
    string			currentElement;
    string			currentValue;
    StatesEnum			state;
    bool			verbose;
    bool                        debug;
  };

template <class	T, class R, class RFilter> 
  class TableSAXReader {

  typedef void (*TableFiller)(const vector<R*>&, map<AtmPhaseCorrectionMod::AtmPhaseCorrection, ASDM2MSFiller*>&);

 public:
  /**
   * An empty contructor.
   */
  TableSAXReader(bool verbose, RFilter& rFilter, TableFiller tableFiller_f_p, std::map<AtmPhaseCorrectionMod::AtmPhaseCorrection, ASDM2MSFiller*>& msFillers_m) {

    LogSinkInterface& lsif = LogSink::globalSink();

    // The top level element name can be derived from the template parameter T.
    myContext.asdm_p		       = &asdm;
    myContext.maxNumberOfRowsInMem     = 100000;
    myContext.rFilter_p                = &rFilter;
    myContext.tableFiller_f_p          = tableFiller_f_p;
    myContext.msFillers_m_p            = &msFillers_m;
    myContext.state		       = ParserContext<T, R, RFilter>::START;
    topLevelElement_s		       = T::name()+"Table";
    myContext.topLevelElement_p	       = (const xmlChar *) topLevelElement_s.c_str(); 
    myContext.entityElement_p	       = (const xmlChar *) "Entity";
    myContext.containerEntityElement_p = (const xmlChar *)"ContainerEntity";
    myContext.rowElement_p	       = (const xmlChar *) "row"; 
    myContext.depth                    = 0;
    myContext.verbose		       = verbose;
    myContext.debug                    = getenv("ASDM_DEBUG") != NULL;
  }
    
  /**
   * The destructor.
   */
  ~TableSAXReader() {;}
    
  /**
   * It will be used as a functor.
   */
  void operator() (const string&  asdmDirectory, bool ignoreTime) {
    myContext.ignoreTime = ignoreTime;
    string tablePath = asdmDirectory + "/"+ T::name() + ".xml";
    xmlSAXUserParseFile(&myHandler, &myContext, tablePath.c_str());
  }
    
  /**
   * Defines the action to perform when an event "opening tag" occurs.
   *
   */ 
  static void start_element_callback(void *v_p, const xmlChar *name, const xmlChar **attrs) {
    const xmlChar* expectedElement;
    typename ParserContext<T, R, RFilter>::StatesEnum nextState;

    if (V2CTX_P(v_p)->debug) TableSAXReader<T, R, RFilter>::enterElementInfo(V2CTX_P(v_p), name);
    V2CTX_P(v_p)->depth++; 
     
    switch (V2CTX_P(v_p)->state) {
      // We are right before the toplevel element of the XML document.
    case ParserContext<T, R, RFilter>::START :
      expectedElement = V2CTX_P(v_p)->topLevelElement_p;
      nextState = ParserContext<T, R, RFilter>::IN_TABLE;
      break;

      // We are right after the toplevel element of the XML document.
    case ParserContext<T, R, RFilter>::IN_TABLE :
      expectedElement = V2CTX_P(v_p)->entityElement_p;
      nextState = ParserContext<T, R, RFilter>::IN_ENTITY;
      break;

      // We are right after the <Entity.../> element.
    case ParserContext<T, R, RFilter>::AFTER_ENTITY :
      expectedElement = V2CTX_P(v_p)->containerEntityElement_p;
      nextState = ParserContext<T, R, RFilter>::IN_CONTAINER_ENTITY;
      break;

      // We are right after the <ContainerEntity.../> element or a <row>..</row> element.
    case ParserContext<T, R, RFilter>::AFTER_ROW :
    case ParserContext<T, R, RFilter>::AFTER_CONTAINER_ENTITY :
      {
	T& tableRef = (T&) V2CTX_P(v_p)->asdm_p->getTable(T::name());
	V2CTX_P(v_p)->row_sp = boost::shared_ptr<R>(tableRef.newRow());
	expectedElement = V2CTX_P(v_p)->rowElement_p;
	nextState = ParserContext<T, R, RFilter>::IN_ROW;
      }
      break;
    
      // We are inside a <row>...</row> but outside any element contained in that element.
    case ParserContext<T, R, RFilter>::IN_ROW :
      {
	expectedElement = NULL;
	V2CTX_P(v_p)->currentElement = (char *) name;
	nextState = ParserContext<T, R, RFilter>::IN_ATTRIBUTE;
      }
      break;
    
      // We are inside a element contained in a <row>...</row> element.
    case ParserContext<T, R, RFilter>::IN_ATTRIBUTE:
      break;
    
      // Otherwise we have a problem.
    default : 
      string message = "Unexpected '" + string((char *) name) + "'.";
      error(message);
    };
  
    checkOpeningElement(name, expectedElement);
    V2CTX_P(v_p)->state = nextState;
  }

  /**
   * Defines the action to perform when an event "closing tag" occurs.
   *
   */
  static void end_element_callback(void *v_p, const xmlChar* name) {
    typename ParserContext<T, R, RFilter>::StatesEnum nextState;
    typename ParserContext<T, R, RFilter>::StatesEnum currentState = V2CTX_P(v_p)->state;

    V2CTX_P(v_p)->depth--;
    if (V2CTX_P(v_p)->debug) TableSAXReader<T, R, RFilter>::exitElementInfo(v_p, name);    
      
    switch (currentState) {
      // We are right after a  <row>..</row>
    case ParserContext<T, R, RFilter>::AFTER_ROW :
      checkClosingElement(name, V2CTX_P(v_p)->topLevelElement_p);
      nextState = ParserContext<T, R, RFilter>::END;
      
      // Possibly write some remainings rows by applying tableFiller on the vector. 
      if (V2CTX_P(v_p)->rows.size() > 0) {
	std::vector<R*> tmp(V2CTX_P(v_p)->rows.size());
	for (unsigned int iR = 0; iR < tmp.size(); iR++) tmp[iR] = V2CTX_P(v_p)->rows[iR].get();
	const std::vector<R*>& filteredRows = (*(V2CTX_P(v_p)->rFilter_p))(tmp, V2CTX_P(v_p)->ignoreTime);
	if (filteredRows.size() > 0) {
	  (*(V2CTX_P(v_p)->tableFiller_f_p))(filteredRows , *(V2CTX_P(v_p)->msFillers_m_p));
	  //
	  // Log a minimum of information about what has just happened.
	  //
	  if (V2CTX_P(v_p)->verbose){
	    std::ostringstream oss;
	    oss.str("");
	    oss << "Appended " << filteredRows.size() << " rows to the MS SYSPOWER table." << endl;
	    LogSink::postGlobally(LogMessage(oss.str(), LogOrigin((boost::filesystem::path(getexepath())).filename(), WHERE),
					     LogMessage::NORMAL
					     )
				  );	
	  }
	}
	//
	// Empty the vector before reading new rows.
	//
	V2CTX_P(v_p)->rows.clear();
      }
      break;
	
      // We were in an <Entity/> element.
      case ParserContext<T, R, RFilter>::IN_ENTITY :
      checkClosingElement(name, V2CTX_P(v_p)->entityElement_p);
      nextState = ParserContext<T, R, RFilter>::AFTER_ENTITY;
      break;
	
      // We were in a <ContainerEntity/> element.
    case ParserContext<T, R, RFilter>::IN_CONTAINER_ENTITY:
      checkClosingElement(name, V2CTX_P(v_p)->containerEntityElement_p);
      nextState = ParserContext<T, R, RFilter>::AFTER_CONTAINER_ENTITY;
      break;
	
      // We were in a <row>..</row> element.
    case ParserContext<T, R, RFilter>::IN_ROW :
      checkClosingElement(name, V2CTX_P(v_p)->rowElement_p); 
      nextState = ParserContext<T, R, RFilter>::AFTER_ROW;

      // Push the last parsed row into the vector.
      V2CTX_P(v_p)->rows.push_back(V2CTX_P(v_p)->row_sp);

      // If the size of this vector is equal to the maximum number allowed to reside in memory
      // then proceed by applying tableFiller on on the vector and after that clear the vector.
      if (V2CTX_P(v_p)->rows.size() == V2CTX_P(v_p)->maxNumberOfRowsInMem) {
	std::vector<R*> tmp(V2CTX_P(v_p)->rows.size());
	for (unsigned int iR = 0; iR < tmp.size(); iR++) tmp[iR] = V2CTX_P(v_p)->rows[iR].get();
	const std::vector<R*>& filteredRows = (*(V2CTX_P(v_p)->rFilter_p))(tmp, V2CTX_P(v_p)->ignoreTime);
	if (filteredRows.size() > 0) {
	  (*(V2CTX_P(v_p)->tableFiller_f_p))(filteredRows , *(V2CTX_P(v_p)->msFillers_m_p));
	  //
	  // Log a minimum of information about what has just happened.
	  //
	  if (V2CTX_P(v_p)->verbose){
	    std::ostringstream oss;
	    oss.str("");
	    oss << "Appended " << filteredRows.size() << " rows to the MS SYSPOWER table." << endl;
	    LogSink::postGlobally(LogMessage(oss.str(), LogOrigin((boost::filesystem::path(getexepath())).filename(), WHERE),
					     LogMessage::NORMAL
					     )
				  );	
	  }
	}
	V2CTX_P(v_p)->rows.clear();
      }
      break;
	
      // We were in an element located in a <row>..</row>, which is normally an attribute. 
    case ParserContext<T, R, RFilter>::IN_ATTRIBUTE :
      checkClosingElement(name, (const xmlChar*) V2CTX_P(v_p)->currentElement.c_str()); 
      nextState = ParserContext<T, R, RFilter>::IN_ROW;
      V2CTX_P(v_p)->row_sp.get()->fromText(V2CTX_P(v_p)->currentElement, V2CTX_P(v_p)->currentValue);
      break;
	
    default : 
      string message = "Unexpected '" + string((char *) name) + "'.";
      error(message);
    }
      
    V2CTX_P(v_p)->state = nextState;
  }

  /**
   * Defines the action to be performed while parsing any text outside of 
   * a tag. Normally his occurs only when an attribute is parsed.
   *
   */
  static void characters_callback (void *		v_p,
				   const xmlChar * 	ch,
				   int			len) {
    V2CTX_P(v_p)->currentValue = string((char * ) ch, len);
  }
    
    
 private:
  bool			verbose;
  ParserContext<T, R, RFilter>		myContext ;
  string			topLevelElement_s;
  asdm::ASDM			asdm;
  static xmlSAXHandler	myHandler; 
  static xmlSAXHandler        initSAXHandler() {
    xmlSAXHandler		handler = { 0 };
    handler.startElement		= start_element_callback; 
    handler.endElement		= end_element_callback; 
    handler.characters		= characters_callback;
    return handler;
  }

  static void error(const string & message) {
    throw asdm::ConversionException(message, T::name());
  }
    
  static void unexpectedOpeningElement(const xmlChar *name, const xmlChar *expectedName) {
    string message = "Unexpected opening tag '" + string((const char *)  name) + "', I was expecting '" + string((const char*) expectedName) +"'.";
    error(message);
  }
    
  static void unexpectedClosingElement(const xmlChar *name) {
    string message = "Unexpected closing tag '" + string((const char *) name) + "'.";
    error(message);
  }
    
  static void checkOpeningElement(const xmlChar* name, const xmlChar* expectedName) {
    if (expectedName && xmlStrcmp(name, expectedName)) unexpectedOpeningElement(name, expectedName);
  }
    
  static void checkClosingElement(const xmlChar* name, const xmlChar* expectedName) {
    if (xmlStrcmp(name, expectedName)) unexpectedClosingElement(name);
  }
    
  static void enterElementInfo(void *v_p, const xmlChar* name) {
    cout << "Enter '" << name << "' (depth = " << V2CTX_P(v_p)->depth << ")" << endl;
  }
    
  static void exitElementInfo(void *v_p, const xmlChar* name) {
    cout << "Exit from '" << name << "' (depth = " << V2CTX_P(v_p)->depth << ")" << endl;
  }

}; // end TableSAXReader
  
template<class T, class R, class RFilter> xmlSAXHandler TableSAXReader<T, R, RFilter>::myHandler = TableSAXReader<T, R, RFilter>::initSAXHandler();

/* ------------------------------------- TableSaxParser - end ------------------------------------------*/  
#endif
