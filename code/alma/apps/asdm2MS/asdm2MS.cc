#include <iostream>
#include <sstream>
#include <vector>
#include <assert.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <ASDMAll.h>

#include "SDMBinData.h"
using namespace sdmbin;

#include <exception>
using namespace asdm;
#include "IllegalAccessException.h"

#include "UvwCoords.h"
#include "ASDM2MSFiller.h"

#include "measures/Measures/Stokes.h"
using namespace casa;

#include "CStokesParameter.h"
#include "CPolarizationType.h"
#include "CProcessorType.h"
#include "CProcessorSubType.h"

#include	"time.h"			/* <time.h> */
#if	defined(__sysv__)
#include	<sys/time.h>
#elif	defined(__bsd__)
#define		ftime	FTIME
#include	<sys/timeb.h>			/* from system */
#undef		ftime
extern	void	ftime( struct timeb * );	/* this is the funtion */
#else
#endif

void	myTimer( double *cpu_time ,		/* cpu timer */
		 double *real_time ,		/* real timer */
		 int  *mode )			/* the mode */
{
  clock_t	tc;				/* clock time */
  double	ct;				/* cpu time in seconds */
  double	rt = 0.0 ;           		/* real time in seconds */

#if	defined(__sysv__)
  struct timeval 	Tp;
  struct timezone	Tzp;
#elif	defined(__bsd__)
  struct timeb tr;				/* struct from ftime */
#else
#endif
  tc = clock( );				/* get clock time */
  ct = (double)(tc) / (double)CLOCKS_PER_SEC;	/* to seconds */
#if	defined(__sysv__)
  gettimeofday( &Tp, &Tzp );			/* get timeofday */
  rt = (double) Tp.tv_sec + 0.000001 * (double) Tp.tv_usec;
#elif	defined(__bsd__)
  ftime( &tr );				/* get real time */
  rt = (double) tr.time + 0.001 * (double) tr.millitm;	/* in seconds */
#else
#endif
  if (*mode) {					/* calculate difference */
    (*cpu_time)  = ct - (*cpu_time);		/* cpu time */
    (*real_time) = rt - (*real_time);		/* real time */
  } else {
    (*cpu_time)  = ct;			/* set cpu time */
    (*real_time) = rt;			/* set real time */
  }
}


//using namespace casa;



ASDM2MSFiller* msFiller;


void error(string message) {
  cout << message << endl;
  exit(1);
}

#include <iostream>
#include <sstream>

ostringstream err;
using namespace std;


//
// A class to describe Exceptions.
//
class ASDM2MSException {  
public:
  /**
   * An empty contructor.
   */
  ASDM2MSException();
  
  /**
   * A constructor with a message associated with the exception.
   * @param m a string containing the message.
   */
  ASDM2MSException(string m);
  
  /**
   * The destructor.
   */
  virtual ~ASDM2MSException();
  
  /**
   * Returns the message associated to this exception.
   * @return a string.
   */
  string getMessage() const;
  
protected:
  string message;
  
};

inline ASDM2MSException::ASDM2MSException() : message ("ASDM2MSException") {}
inline ASDM2MSException::ASDM2MSException(string m) : message(m) {}
inline ASDM2MSException::~ASDM2MSException() {}
inline string ASDM2MSException::getMessage() const {
  return "ASDM2MSException : " + message;
}

// A facility to get rid of blanks at start and end of a string.
// 
string lrtrim(std::string& s,const std::string& drop = " ")
{
  std::string r=s.erase(s.find_last_not_of(drop)+1);
  return r.erase(0,r.find_first_not_of(drop));
}




// These classes provide mappings from some ALMA Enumerations to their CASA counterparts.
class StokesMapper {
private :
  Stokes::StokesTypes* sa;

public :
  StokesMapper();
  ~StokesMapper();

  static Stokes::StokesTypes value(StokesParameterMod::StokesParameter s);
  Stokes::StokesTypes* to1DArray(const vector<StokesParameterMod::StokesParameter>& v);
}
;

StokesMapper::StokesMapper() {
  sa = 0;
}

StokesMapper::~StokesMapper() {
  if (sa)  delete[] sa;
}

Stokes::StokesTypes StokesMapper::value(StokesParameterMod::StokesParameter s) {
  switch (s) {
  case I : return Stokes::I; 
  case Q : return Stokes::Q; 
  case U : return Stokes::U; 
  case V : return Stokes::V; 
  case RR : return Stokes::RR; 
  case RL : return Stokes::RL; 
  case LR : return Stokes::LR; 
  case LL : return Stokes::LL; 
  case XX : return Stokes::XX; 
  case XY : return Stokes::XY; 
  case YX : return Stokes::YX; 
  case YY : return Stokes::YY; 
  case RX : return Stokes::RX; 
  case RY : return Stokes::RY; 
  case LX : return Stokes::LX; 
  case LY : return Stokes::LY; 
  case XR : return Stokes::XR; 
  case XL : return Stokes::XL; 
  case YR : return Stokes::YR; 
  case YL : return Stokes::YL; 
  case PP : return Stokes::PP; 
  case PQ : return Stokes::PQ; 
  case QP : return Stokes::QP; 
  case QQ : return Stokes::QQ; 
  case RCIRCULAR : return Stokes::RCircular; 
  case LCIRCULAR : return Stokes::LCircular; 
  case LINEAR : return Stokes::Linear; 
  case PTOTAL : return Stokes::Ptotal; 
  case PLINEAR : return Stokes::Plinear; 
  case PFTOTAL : return Stokes::PFtotal; 
  case PFLINEAR : return Stokes::PFlinear; 
  case PANGLE : return Stokes::Pangle;  
  }
  return Stokes::Undefined;
}

Stokes::StokesTypes* StokesMapper::to1DArray(const vector<StokesParameterMod::StokesParameter>& v) {
  if (v.size() == 0) return 0;

  if (sa) {
    delete[] sa;
    sa = 0;
  }

  sa = new Stokes::StokesTypes[v.size()];
  for (unsigned int i = 0; i < v.size(); i++) 
    sa[i] = value(v.at(i));

  return sa;
}

class PolTypeMapper {
private :
  vector<string> polType;

public :
  PolTypeMapper();
  ~PolTypeMapper();

  static char value(PolarizationTypeMod::PolarizationType p);
  vector<string> toStringVector(const vector<PolarizationTypeMod::PolarizationType>& v);
};


PolTypeMapper::PolTypeMapper() {
  ;
}

PolTypeMapper::~PolTypeMapper() {
  ;
}

char PolTypeMapper::value(PolarizationTypeMod::PolarizationType p) {
  return (CPolarizationType::name(p)).at(0);
}

vector<string> PolTypeMapper::toStringVector(const vector<PolarizationTypeMod::PolarizationType>& v) {
  polType.clear();
  for (unsigned int i = 0; i < v.size(); i++) {
    polType.push_back((CPolarizationType::name(v.at(i))));
  }
  return polType;
}

// These classes provide methods to convert from Vectors to Arrays.
class DConverter {
private :
  double* da;
  
  
public :
  ~DConverter();
  DConverter();
  double* to1DArray(vector<double>);
  double* to1DArray(vector<vector<double> > v);
  double* to1DArray(vector<Frequency>);
  double* to1DArray(vector<Angle>);
  double* to1DArray(vector<AngularRate>);
  double* to1DArray(vector<Speed>);
  double* to1DArray(vector<vector <Angle> >);
  double* to1DArray(double, double, double);
  double* to1DArray(Length, Length, Length);
};

DConverter::DConverter() {
  da = 0;
}

DConverter::~DConverter() {
  //cout << "DConverter destructor" << endl;
  if (da) {
    delete[] da;
    da = 0;
  }
  //cout << "DConverter has released space" << endl;
}

double* DConverter::to1DArray(vector<double> v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i);

  return da;
}


double* DConverter::to1DArray(vector<vector<double> > v) {
  // Calculates the number of integers stored in v.
  int n = 0;
  for (unsigned int i = 0; i < v.size(); i++) n += v.at(i).size();

  // Now we can allocate and copy with a transposition.
  da = new double[n];
  int k = 0;
  for (unsigned int i=0; i < v.size(); i++)
    for (unsigned int j = 0; j < v.at(i).size(); j++)
      da[k++] = v.at(i).at(j);

  return da;

}

double* DConverter::to1DArray(vector<Frequency> v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i).get();

  return da;
}

double*  DConverter::to1DArray(vector<Angle> v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i).get();

  return da;
}


double*  DConverter::to1DArray(vector<Speed> v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i).get();

  return da;
}

double*  DConverter::to1DArray(vector<AngularRate> v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i).get();

  return da;
}

double* DConverter::to1DArray(vector<vector<Angle> > v) {
  // Calculates the number of integers stored in v.
  int n = 0;
  for (unsigned int i = 0; i < v.size(); i++) n += v.at(i).size();

  // Now we can allocate and copy with a transposition.
  da = new double[n];
  int k = 0;
  for (unsigned int i=0; i < v.size(); i++)
    for (unsigned int j = 0; j < v.at(i).size(); j++)
      da[k++] = v.at(i).at(j).get();

  return da;

}

double* DConverter::to1DArray(double e0, double e1, double e2) {
  da = new double[3];

  da[0]=e0;
  da[1]=e1;
  da[2]=e2; 
  
  return da;
}

double* DConverter::to1DArray(Length e0, Length e1, Length e2) {
  da = new double[3];

  da[0]=e0.get();
  da[1]=e1.get();
  da[2]=e2.get();
  
  return da;
}

class IConverter {
private :
  int* ia;  
  
public :
  ~IConverter();
  IConverter();
  int* to1DArray(vector<int>);
  int* to1DArray(vector<Tag>);
  int* to1DArray(vector<vector <int> >);
};

IConverter::IConverter() {
  ia = 0;
}

IConverter::~IConverter() {
  if (ia) {
    delete[] ia;
    ia = 0;
  }
  // cout << "IConverter has released space" << endl;
}

int* IConverter::to1DArray(vector<int> v) {
  ia = new int[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    ia[i] = v.at(i);

  return ia;
}


int* IConverter::to1DArray(vector<Tag> v) {
  ia = new int[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    ia[i] = v.at(i).getTagValue();

  return ia;
}

int* IConverter::to1DArray(vector<vector<int> > v) {
  // Calculates the number of integers stored in v.
  int n = 0;
  for (unsigned int i = 0; i < v.size(); i++) n += v.at(i).size();

  // Now we can allocate and copy with a transposition.
  ia = new int[n];
  int k = 0;
  for (unsigned int i=0; i < v.size(); i++)
    for (unsigned int j = 0; j < v.at(i).size(); j++)
      ia[k++] = v.at(i).at(j);

  return ia;
}


class SConverter {
private :
  char* sa;  
  char* *ssa;
  
public :
  SConverter();
  ~SConverter(); 
  char* to1DCharArray(vector<string>);
  char* to1DCharArray(vector<vector <string> >);
  char** to1DCStringArray(vector<string >);

  template<class Enum, class EnumHelper> char** to1DCStringArray(vector<Enum> vEnum) {
    // Convert the vector of Enum into a vector of string.
    vector<string> v;
    for (unsigned int i = 0; i < vEnum.size(); i++)
      v.push_back(EnumHelper::name(vEnum.at(i)));

    // Calculates the number of chars to store the resulting C-strings (including terminating zeros)
    int n = 0;
    for (unsigned int i = 0; i < v.size(); i++)
      n += v.at(i).size() + 1;
    
    // Reserve the space to store all the C-Strings
    sa = new char[n+1];
    
    // Reserve the space to store the array of pointers onto these C-Strings
    ssa = new char*[v.size() +1];
    
    int k = 0;
    for (unsigned int i = 0; i < v.size(); i++) {
      ssa[i] = &sa[k];
      for (unsigned int j = 0; j < v.at(i).size(); j++)
	sa[k++] = v.at(i).at(j);
      sa[k++] = 0;
    }
    ssa[v.size()] = 0;
    return ssa;
  }
};

SConverter::SConverter() {
  sa = 0;
  ssa = 0;
}

SConverter::~SConverter() {
  //cout << "SConverter destructor" << endl;
  if (sa) {
    delete[] sa;
    sa = 0;
  }

  if (ssa) {
    delete[] ssa;
    ssa = 0;
  }
  //cout << "SConverter has released space" << endl;
}

char* SConverter::to1DCharArray(vector<string> v) {
  // Calculates the number of char to store in result.
  int n = 0;
  for (unsigned int i = 0; i < v.size(); i++)
    n += v.at(i).size();

  // Reserve the appropriate number of chars.
  sa = new char[n+1];
  sa[n] = 0; // Not really useful but...who knows...

  int k = 0;
  for (unsigned int i=0; i < v.size(); i++)
    for (unsigned int j=0; j < v.at(i).size(); j++)
      sa[k++] = v.at(i).at(j);

  return sa;
}

char** SConverter::to1DCStringArray(vector<string> v) {
  // Calculates the number of chars to store the resulting C-strings (including terminating zeros)
  int n = 0;
  for (unsigned int i = 0; i < v.size(); i++)
    n += v.at(i).size() + 1;

  // Reserve the space to store all the C-Strings
  sa = new char[n+1];

  // Reserve the space to store the array of pointers onto these C-Strings
  ssa = new char*[v.size() +1];

  int k = 0;
  for (unsigned int i = 0; i < v.size(); i++) {
    ssa[i] = &sa[k];
    for (unsigned int j = 0; j < v.at(i).size(); j++)
      sa[k++] = v.at(i).at(j);
    sa[k++] = 0;
  }
  ssa[v.size()] = 0;
  return ssa;
}


char* SConverter::to1DCharArray(vector< vector<string> > v) {
  // Calculates the number of char to store in result.
  int n = 0;
  for (unsigned int i = 0; i < v.size(); i++)
    for (unsigned int j = 0; j < v.at(i).size(); j++)
      n += v.at(i).at(j).size();

  // Reserve the appropriate number of chars.
  sa = new char[n+1];
  sa[n] = 0; // Not really useful but...who knows...

  int l = 0;
  for (unsigned int i=0; i < v.size(); i++)
    for (unsigned int j = 0; j < v.at(i).size(); j++)
      for (unsigned int k = 0; k < v.at(i).at(j).size(); k++)
	sa[l++] = v.at(i).at(j).at(k);

  return sa;
}

class  CConverter {
private :
  double* ra;
  double* ia;

    
public :
  ~CConverter();
  CConverter();
  double* to1DArrayR(vector<asdm::Complex>);
  double* to1DArrayI(vector<asdm::Complex>);
  double* to1DArrayR(vector< vector<asdm::Complex> >);
  double* to1DArrayI(vector< vector<asdm::Complex> >);

};


CConverter::CConverter() {
  ra = 0;
  ia = 0;
}

CConverter::~CConverter() {
  if (ra != 0) {
    delete[] ra;
    ra = 0;
  }

  if (ia != 0) {
    delete[] ia;
    ia = 0;
  }
  // cout << "CConverter has released space" << endl;
}


double* CConverter::to1DArrayR(vector<asdm::Complex> v) {
  ra = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    ra[i] = v.at(i).getReal();

  return ra;
}

double* CConverter::to1DArrayI(vector<asdm::Complex> v) {
  ia = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    ia[i] = v.at(i).getImg();

  return ia;
}


double* CConverter::to1DArrayR(vector< vector<asdm::Complex> > v) {
  // Calculates the number of integers stored in v.
  int n = 0;
  for (unsigned int i = 0; i < v.size(); i++) n += v.at(i).size();

  // Now we can allocate and copy with a transposition.
  ra = new double[n];
  int k = 0;
  for (unsigned int i=0; i < v.size(); i++)
    for (unsigned int j = 0; j < v.at(i).size(); j++)
      ra[k++] = v.at(i).at(j).getReal();
  
  return ra;
}


double* CConverter::to1DArrayI(vector< vector<asdm::Complex> > v) {
  // Calculates the number of integers stored in v.
  int n = 0;
  for (unsigned int i = 0; i < v.size(); i++) n += v.at(i).size();

  // Now we can allocate and copy with a transposition.
  ra = new double[n];
  int k = 0;
  for (unsigned int i=0; i < v.size(); i++)
    for (unsigned int j = 0; j < v.at(i).size(); j++)
      ra[k++] = v.at(i).at(j).getImg();
  
  return ra;
}


class ComplexDataFilter {

public:
  ComplexDataFilter();
  virtual ~ComplexDataFilter();
  virtual float* to4Pol(int numPol, int numChan, float* cdata);

private:
  vector<float *> storage; 
};

ComplexDataFilter::ComplexDataFilter() {
  ;
}


ComplexDataFilter::~ComplexDataFilter() {
  for (unsigned int i = 0; i < storage.size(); i++) 
    delete[] storage.at(i);

  // cout << "In ~ComplexDataFilter" << endl;
}

float *ComplexDataFilter::to4Pol(int numCorr, int numChan, float* cdata) {
  // Do nothing if numCorr != 3
  if (numCorr != 3) return cdata;

  
  // Increase the storage size by appending nul chars.
  float* filtered = new float[ 2 * 4 * numChan ];

  storage.push_back(filtered);
  
  for (int i = 0; i < numChan; i++) {
    // The 1st row goes to the first row.
    filtered[ 8 * i ]     = cdata[ 6 * i ] ;
    filtered[ 8 * i + 1 ] = cdata[ 6 * i + 1 ] ;

    // The second row goes the second row.
    filtered [ 8 * i + 2 ] = cdata[ 6 * i + 2 ] ;
    filtered [ 8 * i + 3 ] = cdata[ 6 * i + 3 ] ;

    // The second row's conjugate goes to the third row.
    filtered [ 8 * i + 4 ] = cdata[ 6 * i + 2 ] ;
    filtered [ 8 * i + 5 ] = - cdata[ 6 * i + 3 ] ;

    // The third row goes to the third row.
    filtered [ 8 * i + 6 ] = cdata[ 6 * i + 4 ] ;
    filtered [ 8 * i + 7 ] = cdata[ 6 * i + 5 ] ;    
  }



  return filtered;
}


void usage(char* command) {
  cout << "Usage : " << command << " DataSetName" << endl;
}

//
// A number of EnumSet to encode the different selection criteria.
//
EnumSet<CorrelationMode>         es_cm;
EnumSet<SpectralResolutionType>  es_srt;
EnumSet<TimeSampling>            es_ts;
Enum<CorrelationMode>            e_query_cm; 
EnumSet<AtmPhaseCorrection>      es_query_apc;    

//
// By default the resulting MS will not contain compressed columns
// unless the 'compress' option has been given.
// 
Bool                             withCompression = false;


/**
 * The main function.
 */
int main(int argc, char *argv[]) {

  string dummy;

  
  //   Process command line options and parameters.
  
  try {


    // Declare the supported options.

    po::options_description generic("Converts an ASDM dataset into a CASA measurement set.\n"
				    "Usage : asdm2MS [options] asdm-directory\n"
				    "Allowed options:");
    generic.add_options()
      ("help", "produces help message.")
      ("icm",  po::value<string>()->default_value("all"), "specifies the correlation mode to be considered on input. A quoted string containing a sequence of 'ao' 'co' 'ac' 'all' separated by whitespaces is expected")
      ("isrt", po::value<string>()->default_value("all"), "specifies the spectral resolution type to be considered on input. A quoted string containing a sequence of 'fr' 'ca' 'bw' 'all' separated by whitespaces is expected")
      ("its",  po::value<string>()->default_value("all"), "specifies the time sampling (INTEGRATION and/or SUBINTEGRATION)  to be considered on input. A quoted string containing a sequence of 'i' 'si' 'all' separated by whitespaces is expected")  
      ("ocm",  po::value<string>()->default_value("co"),  "output data for correlation mode AUTO_ONLY (ao) or CROSS_ONLY (co) or CROSS_AND_AUTO (ca)")
      ("compression,c", "produces compressed columns in the resulting measurement set (not set by default)")
      ;

    // Hidden options, will be allowed both on command line and
    // in config file, but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()
      ("asdm-directory", po::value< string >(), "asdm directory")
      ;

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(hidden);
    
    po::positional_options_description p;
    p.add("asdm-directory", -1);
    
    // Parse the command line and retrieve the options and parameters.

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);

    po::notify(vm);
    

    // Help ? displays help's content and don't go further.

    if (vm.count("help")) {
      cout << generic << "\n" ;
      return 1;
    }
    
    istringstream iss;
    string token;

    // Selection of correlation mode of data to be considered on input.

    string icm_opt = vm["icm"].as< string >();
    iss.clear();
    iss.str(icm_opt);
    
    while (iss >> token) {
      if (token.compare("co") == 0) 
	es_cm.fromString("CROSS_ONLY", false);
      else if (token.compare("ao") == 0)
	es_cm.fromString("AUTO_ONLY", false);
      else if (token.compare("ac") == 0)
	es_cm.fromString("CROSS_AND_AUTO", false);
      else if (token.compare("all") == 0)
	es_cm.fromString("CROSS_ONLY AUTO_ONLY CROSS_AND_AUTO", false);
      else {
	cout << "Token '" << token << "' invalid for --icm option." << endl;
	cout << generic << endl;
      }
    }
    cout << "icm is \"" << icm_opt << "\" (\"" << es_cm.toString() << "\")" << endl;


    // Selection of spectral resolution type of data to be considered.

    string isrt_opt = vm["isrt"].as< string >();
    iss.clear();
    iss.str(isrt_opt);

    while (iss >> token) {
      if (token.compare("fr") == 0)
	es_srt.fromString("FULL_RESOLUTION", false);
      else if (token.compare("ca") == 0)
	es_srt.fromString("CHANNEL_AVERAGE", false);
      else if (token.compare("bw") == 0)
	es_srt.fromString("BASEBAND_WIDE", false);
      else if (token.compare("all") == 0)
	es_srt.fromString("FULL_RESOLUTION CHANNEL_AVERAGE BASEBAND_WIDE", false);
      else { 
	cout << "Token '" << token << "' invalid for --isrt option." << endl;
	cout << generic << endl;
      }
    }
    cout << "isrt is \"" << isrt_opt << "\" (\"" << es_srt.toString() << "\")" << endl;

    // Selection of the time sampling of data to be considered (integration and/or subintegration)

    string its_opt = vm["its"].as < string >();
    iss.clear();
    iss.str(its_opt);

    while ( iss >> token ) {
      if (token.compare("i") == 0)
	es_ts.fromString("INTEGRATION",false);
      else if (token.compare("si") == 0)
	es_ts.fromString("SUBINTEGRATION", false);
      else if (token.compare("all") == 0)
	es_ts.fromString("INTEGRATION SUBINTEGRATION", false);
      else {
	cout << "Token '" << token << "' invalid for its option." << endl;
	cout << generic << endl;
      }
    }
    cout << "its is \"" << its_opt << "\" (\"" << es_ts.toString() << "\")" << endl;

    // Selection of the correlation mode of data to be produced in the measurement set.

    string ocm_opt = vm["ocm"].as< string >();
    if ( ocm_opt.compare("co") == 0 )
      e_query_cm = CROSS_ONLY;
    else if ( ocm_opt.compare("ao") == 0 )
      e_query_cm = AUTO_ONLY;
    else if ( ocm_opt.compare("ca") == 0 )
      e_query_cm = CROSS_AND_AUTO;
    else {
      cout << generic << "\n" ;
      return 1;
    }

    cout << "ocm is \"" << ocm_opt << "\" (\"" << e_query_cm.str() << "\")\n";

    if (vm.count("asdm-directory")) {
      dummy = vm["asdm-directory"].as< string >() ;
      cout << "asdm-directory is '" << dummy << "'" << endl;
    }
    else {
      cout << generic << "\n" << endl;
      return 0;
    }


    // Does the user want compressed columns in the resulting MS ?

    if ((withCompression = (vm.count("compression") != 0))) {
      cout << "the resulting MS will contain compressed columns" << endl;
    }
  }
  catch (exception& e) {
    cout << e.what() << "\n";
  }
  
  es_query_apc.fromString("AP_CORRECTED AP_UNCORRECTED", true);
  
  //
  // Try to open an ASDM dataset whose name has been passed as a parameter on the command line
  //
  string dsName = lrtrim(dummy);
  if ( dsName.at(dsName.size()-1) == '/' ) dsName.erase(dsName.size()-1);


  double cpu_time_parse_xml  = 0.0;
  double real_time_parse_xml = 0.0;
  int mode;
  mode = 0; myTimer(&cpu_time_parse_xml, &real_time_parse_xml, &mode);

  ASDM* ds = new ASDM();

  try {
    cout << "Taking the dataset '" << dsName << "' as input." << endl;
    ds->setFromFile(dsName);
  }
  catch (ConversionException e) {
    cout << e.getMessage() << endl;
    exit(-1);
  }
  catch (exception e) {
    cout << e.what() << endl;
    exit(-1);
  }
  catch (...) {
    cout << "Uncaught exception !" << endl;
  }
  
  mode = 1; myTimer(&cpu_time_parse_xml, &real_time_parse_xml, &mode);
  cout << "Time spent parsing the ASDM medata :" << cpu_time_parse_xml << " s." << endl;
    
  //
  // Determine what kind of data complex (DATA column) or float (FLOAT_DATA) will be
  // stored in the measurement set by using the method isDataComplex on the first row of 
  // the ASDM Dataset. 
  // This method called on all remaining rows of the ASDM should return the same result
  // otherwise the filling process will stop.
  //
  SDMBinData sdmBinData(ds, dsName);

  // Define the SDM Main table subset of rows to be accepted
  sdmBinData.select( es_cm, es_srt, es_ts);   

  // Define the subset of data to be extracted from the BLOBs
  sdmBinData.selectDataSubset(e_query_cm, es_query_apc);

  cout << "e_query_cm = " << e_query_cm.str() << endl;

  bool complexData =  sdmBinData.isComplexData();

  cout << "The resulting measurement set will contain a '" << ((complexData) ? "DATA" : "FLOAT_DATA") << "' column" << endl; 

  //
  // Create a Measurement Set.


  // The name of the measurement set is the name of the ASDM dataset ...
  ostringstream ost;

  ost << dsName; 

  // ... prefixed with .compressed if compression has been requested ...
  if (withCompression)
    ost << ".compressed";

  // ... and prefixed with .ms in all cases.
  ost << ".ms";

  cout << "About to create a new measurement set '" << ost.str() << "'" << endl;
  msFiller = new ASDM2MSFiller(ost.str().c_str(), 0.0, false, (Bool) complexData, withCompression);


  //
  // Firstly convert the basic tables.
  //
  // For purpose of simplicity we assume that in all ASDM basic tables having a Tag identifier
  // these Tag values are forming a sequence of integer 0 -> table.size() - 1. If that's not the case, the
  // program aborts.
  //
  //
  // Antenna table conversion.
  //  
  AntennaTable& aT = ds->getAntenna();

  // We need the ExecBlockTable here to compute the antenna position with a site dependant logic.
  // This table will be used later to fill the MS Observation table.
  //
  ExecBlockTable& execBlockT = ds->getExecBlock(); 

  //
  // Write the Antenna table.
  // 
  // (That part needs the Station table)
  { 
    AntennaRow*   r   = 0;
    ExecBlockRow* reb = 0;

    int nAntenna = aT.size();
    cout << "The dataset has " << nAntenna << " antenna(s)...";
    for (int i = 0; i < nAntenna; i++) {
      if ((r = aT.getRowByKey(Tag(i, TagType::Antenna))) == 0){
	err.str("");
	err << "Problem while reading the Antenna table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl;
	error(err.str());
      }

      // We assume that wether the ExecBlock table has one row or that all ExecBlocks come from the same site.
      reb = execBlockT.get().at(0);

      // The MS Antenna position is defined as the sum of the ASDM station position and
      // of the ASDM Antenna position after applying to it a coordinate system transformation.
      // Since the ASDM Antenna position is 0,0,0 for now, we only use the ASDM station position.
      vector<Length> position = r->getStationUsingStationId()->getPosition();
      double xPosition = position.at(0).get();
      double yPosition = position.at(1).get();
      double zPosition = position.at(2).get();

      msFiller->addAntenna(r->getName().c_str(),
			   r->getStationUsingStationId()->getName().c_str(),
			   xPosition,
			   yPosition,
			   zPosition,
			   // 		     r->getXPosition().get() + reb->getSiteLongitude().get(),
			   // 		     r->getYPosition().get() + reb->getSiteLatitude().get(),
			   // 		     r->getZPosition().get() + reb->getSiteAltitude().get(),
			   
			   r->getXOffset().get(),
			   r->getYOffset().get(),
			   r->getZOffset().get(),
			   (float)r->getDishDiameter().get());
    }
    if (nAntenna) cout << "converted in " << msFiller->ms()->antenna().nrow() << " antenna(s)  in the measurement set." ;
    cout << endl;
  }

 

  SpectralWindowTable& spwT = ds->getSpectralWindow();  
  //
  // Read/Write the SpectralWindow table.
  //
  {

    SpectralWindowRow* r = 0;
    int nSpectralWindow = spwT.size();
    cout << "The dataset has " << nSpectralWindow << " spectral window(s)..."; 
    
    for (int i = 0; i < nSpectralWindow; i++) {
      if ((r = spwT.getRowByKey(Tag(i, TagType::SpectralWindow))) == 0) {
	err.str("");
	(err << "Problem while reading the SpectralWindow table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(err.str());
      }

      int numAssoc = 0;

      int* assocSpwId = 0;
      IConverter assocSpwIdConverter;
      if (r->isAssocSpectralWindowIdExists()) {
	numAssoc = r->getAssocSpectralWindowId().size();
	assocSpwId = assocSpwIdConverter.to1DArray(r->getAssocSpectralWindowId());
      }
      
      char** assocNature = 0;
      SConverter assocNatureConverter;
      if (r->isAssocNatureExists()) {
	assocNature = assocNatureConverter.to1DCStringArray<SpectralResolutionType, CSpectralResolutionType>(r->getAssocNature());
      }
      
      /**
       * This interpolation is due to the trick/workaround proposed by rlucas to fill the 4 potentially large arrays
       * contained in a SpectralWindowRow in such a way that their XML representation stay under the 64K (thanks Oracle).
       **/
      
      DConverter chanFreqConverter, chanWidthConverter, effectiveBwConverter, resolutionConverter;
      // 1stly Transform the asdm vectors into plain c++ arrays.
      double* chanFreq1D    = chanFreqConverter.to1DArray(r->getChanFreq());
      double* chanWidth1D   = chanWidthConverter.to1DArray(r->getChanWidth());
      double* effectiveBw1D = effectiveBwConverter.to1DArray(r->getEffectiveBw());
      double* resolution1D  = resolutionConverter.to1DArray(r->getResolution());
      
      // 2ndly Computes values with indexes in [1, size-2] by interpolation assuming that in each array the elements form
      // an arithmetic sequence.
      double chanFreqStep    = (chanFreq1D[r->getNumChan()-1]    - chanFreq1D[0])    / (r->getNumChan() - 1);
      double chanWidthStep   = (chanWidth1D[r->getNumChan()-1]   - chanWidth1D[0])   / (r->getNumChan() - 1);
      double effectiveBwStep = (effectiveBw1D[r->getNumChan()-1] - effectiveBw1D[0]) / (r->getNumChan() - 1);      
      double resolutionStep  = (resolution1D[r->getNumChan()-1]  - resolution1D[0])  / (r->getNumChan() - 1);

      for (int i = 1; i < r->getNumChan() - 1; i++) {
	chanFreq1D[i]   = chanFreq1D[i-1]    + chanFreqStep;
	chanWidth1D[i]  = chanWidth1D[i-1]   + chanWidthStep;
	effectiveBw1D[i]= effectiveBw1D[i-1] + effectiveBwStep;
	resolution1D[i] = resolution1D[i-1]  + resolutionStep;
      }

      /**
       * End of interpolation trick.
       **/

      int bbcNo = -1;
      if (r->isBasebandNameExists()) bbcNo = r->getBasebandName();
      msFiller->addSpectralWindow(r->getNumChan(),
				  r->isNameExists()?r->getName().c_str():"",
				  (r->getRefFreq()).get(),
				  chanFreq1D,  //DConverter().to1DArray(r->getChanFreq()),
				  chanWidth1D, //DConverter().to1DArray(r->getChanWidth()),
				  r->isMeasFreqRefExists()?r->getMeasFreqRef():0,
				  effectiveBw1D, //DConverter().to1DArray(r->getEffectiveBw()),
				  resolution1D,//DConverter().to1DArray(r->getResolution()),
				  (r->getTotBandwidth()).get(),
				  r->getNetSideband(),
				  r->isBasebandNameExists()?r->getBasebandName():-1, 
				  r->isIfConvChainExists()?r->getIfConvChain():0,
				  r->isFreqGroupExists()?r->getFreqGroup():0,
				  r->isFreqGroupNameExists()?r->getFreqGroupName().c_str():"",
				  numAssoc,
				  assocSpwId,
				  assocNature);
    }

    if (nSpectralWindow) cout << "converted in " << msFiller->ms()->spectralWindow().nrow() << " spectral window(s) in the measurement set.";
    cout << endl;

  }

  //
  // Read/Write the polarization table
  //
  Stokes::StokesTypes linearCorr[] = { Stokes::XX, Stokes::XY, Stokes::YX, Stokes::YY };
  int corrProduct1[] = { 0, 0 };
  int corrProduct2[] = { 0, 0, 1, 1};
  int corrProduct4[] = { 0, 0, 0, 1, 1, 0, 1, 1 };
			 
  vector<int> polarizationMap;
  PolarizationTable& polT = ds->getPolarization();  
  try {
    PolarizationRow* r = 0;
    int nPolarization = polT.size();
    cout << "The dataset has " << nPolarization << " polarization(s)..."; 
    
    for (int i = 0; i < nPolarization; i++) {
      if ((r=polT.getRowByKey(Tag(i, TagType::Polarization))) == 0) {
	err.str("");
	(err << "Problem while reading the Polarization table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(err.str());
      }
      
      int numCorr = r->getNumCorr();
      if (numCorr < 1 || numCorr > 4) {
	ostringstream oss ;
	oss << "a polarization row cannot be processed due to  'numCorr = " << numCorr << "'.";
	throw ASDM2MSException(oss.str());
      }

      Stokes::StokesTypes * corrType;
      StokesMapper stokesMapper;
      if (numCorr != 3) {
	corrType = stokesMapper.to1DArray(r->getCorrType());
      }
      else {
	numCorr  = 4;
	corrType = linearCorr;
      }
      
      
      int* corrProduct = 0;
      switch (numCorr) {
      case 1: corrProduct = corrProduct1; break;
      case 2: corrProduct = corrProduct2; break;
      case 4: corrProduct = corrProduct4; break;
      }

      polarizationMap.push_back(msFiller->addUniquePolarization(numCorr,
							  corrType,
							  corrProduct
							  ));
    }
    if (nPolarization) cout << "converted in " << msFiller->ms()->polarization().nrow() << " polarization(s) the measurement set." ;
    cout << endl;
  }
  catch (ASDM2MSException e) {
    cout << e.getMessage() << endl;
    exit(-1);
  }


  //
  // Load the DataDescription table.
  vector<int> dataDescriptionMap;
  DataDescriptionTable& ddT = ds->getDataDescription();
  {
    DataDescriptionRow* r = 0;
    int nDataDescription = ddT.size();
    cout << "The dataset has " << nDataDescription << " data description(s)...";
    for (int i = 0; i < nDataDescription; i++) {
      if ((r=ddT.getRowByKey(Tag(i, TagType::DataDescription))) == 0) {
	err.str("");
	(err << "Problem while reading the DataDescription table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(err.str());
      }
      dataDescriptionMap.push_back(msFiller->addUniqueDataDescription(r->getSpectralWindowId().getTagValue(),
								polarizationMap.at(r->getPolOrHoloId().getTagValue())));
    }
    if (nDataDescription) cout << "converted in " << msFiller->ms()->dataDescription().nrow() << " data description(s)  in the measurement set." ;
    cout << endl;
  }


  //
  // Load the Feed table
  // Issues :
  //    - time (epoch) : at the moment it takes directly the time as it is stored in the ASDM.
  //    - focusLength (in AIPS++) is no defined.


  FeedTable& feedT = ds->getFeed();
  {
    FeedRow* r = 0;
    int nFeed = feedT.size();
    cout << "The dataset has " << nFeed << " feed(s)...";
    vector<FeedRow *> v = feedT.get();
    for (int i = 0; i < nFeed; i++) {
      r = v.at(i);
      // For now we just adapt the types of the time related informations and compute a mid-time.
      //
      double interval = ((double) r->getTimeInterval().getDuration().get()) / ArrayTime::unitsInASecond ;
      double time =  ((double) r->getTimeInterval().getStart().get()) / ArrayTime::unitsInASecond + interval/2.0;

      double xyzPositionArray[] =  {0.0, 0.0, 0.0};
      double* xyzPosition = xyzPositionArray;
      DConverter xyzPositionConverter;
      if (r->isXPositionExists() && r->isYPositionExists() && r->isZPositionExists() ) {
	xyzPosition = xyzPositionConverter.to1DArray(r->getXPosition(),r->getYPosition(), r->getZPosition());
      }

      msFiller->addFeed(r->getAntennaId().getTagValue(),
			r->getFeedId(),
			r->getSpectralWindowId().getTagValue(),
			time,
			interval,
			r->getNumReceptor(), 
			-1,             // We ignore the beamId array
			DConverter().to1DArray(r->getBeamOffset()),
			PolTypeMapper().toStringVector(r->getPolarizationTypes()),
			CConverter().to1DArrayR(r->getPolResponse()),
			CConverter().to1DArrayI(r->getPolResponse()),
			xyzPosition,
			DConverter().to1DArray(r->getReceptorAngle()));
    }
    if (nFeed) cout << "converted in " << msFiller->ms()->feed().nrow() << " feed(s) in the measurement set." ;
    cout << endl;
  }

  // Load the Field table.
  // Issues :
  // - only processes the case with numPoly == 0 at the moment.

  FieldTable& fieldT = ds->getField();
  
  {
    FieldRow* r = 0;
    int nField = fieldT.size();
    cout << "The dataset has " << nField << " field(s)...";

    double delayDir[2];
    double phaseDir[2];
    double referenceDir[2];

    for (int i = 0; i < nField; i++) {
      if ((r=fieldT.getRowByKey(Tag(i, TagType::Field))) == 0) {
	err.str("");
	(err << "Problem while reading the Field table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(err.str());
      }

      delayDir[0] = DConverter().to1DArray(r->getDelayDir())[0];
      delayDir[1] = DConverter().to1DArray(r->getDelayDir())[1];
      
      phaseDir[0] = DConverter().to1DArray(r->getPhaseDir())[0];
      phaseDir[1] = DConverter().to1DArray(r->getPhaseDir())[1];
      
      referenceDir[0] = DConverter().to1DArray(r->getReferenceDir())[0];
      referenceDir[1] = DConverter().to1DArray(r->getReferenceDir())[1];

      int sourceId = -1;
      if (r->isSourceIdExists()) sourceId = r->getSourceId();

      msFiller->addField( (const char*)r->getFieldName().c_str(),
		    (const char*)r->getCode().c_str(),
		    ((double) r->getTime().get()) / ArrayTime::unitsInASecond,
		    delayDir,
		    phaseDir,
		    referenceDir,
		    sourceId);
    }  
    if (nField) cout << "converted in " << msFiller->ms()->field().nrow() << "  field(s) in the measurement set." ;
    cout << endl;
  }


  // Load the FlagCmd table
  FlagCmdTable& flagCmdT  = ds->getFlagCmd();


  {
    FlagCmdRow* r = 0;
    int nFlagCmd = flagCmdT.size();
    cout << "The dataset has " << nFlagCmd << " flag(s)...";
    vector<FlagCmdRow *> v = flagCmdT.get();
    for (int i = 0; i < nFlagCmd; i++) {
      r = v.at(i);
      // For now we just adapt the types of the time related informations and compute a mid-time.
      //
      double interval = ((double) r->getTimeInterval().getDuration().get()) / ArrayTime::unitsInASecond ;
      double time =  ((double) r->getTimeInterval().getStart().get()) / ArrayTime::unitsInASecond + interval/2.0;
      
      msFiller->addFlagCmd(time,
		     interval,
		     r->getType().c_str(),
		     r->getReason().c_str(),
		     r->getLevel(),
		     r->getSeverity(),
		     r->getApplied() ? 1 : 0,
		     r->getCommand().c_str());
    }
    if (nFlagCmd) cout << "converted in " << msFiller->ms()->flagCmd().nrow() << " in the measurement set." ;
    cout << endl;}
  

  // Load the History table
  // Issues :
  // - use executeBlockId for observationId ...to be discussed with Francois.
  // - objectId : not taken into account (it's a string while the MS expects an int).
  HistoryTable& historyT = ds->getHistory();
  {
    HistoryRow* r = 0;
    vector<HistoryRow *> v = historyT.get();
    int nHistory = historyT.size();
    cout << "The dataset has " << nHistory << " history(s)...";


    for (int i = 0; i < nHistory; i++) {
      r = v.at(i);
      double time =  ((double) r->getTime().get()) / ArrayTime::unitsInASecond ;
      msFiller->addHistory(time,
		     r->getExecBlockId().getTagValue(),   
		     r->getMessage().c_str(),
		     r->getPriority().c_str(),
		     r->getOrigin().c_str(),
		     -1,
		     r->getApplication().c_str(),
		     r->getCliCommand().c_str(),
		     r->getAppParms().c_str());
    }
    if (nHistory) cout << "converted in " << msFiller->ms()->history().nrow() << " history(s) in the measurement set" ;
    cout << endl;
  }


  // Build the MS Observation table with the content of ASDM ExecBlock table.
  // 
  {
    ExecBlockRow* r = 0;
    int nExecBlock = execBlockT.size();
    cout << "The dataset has " << nExecBlock << " execBlock(s)...";
    vector<ExecBlockRow *> v = execBlockT.get();
    
    for (int i = 0; i < nExecBlock; i++) {
      r = v.at(i);
      
      ArrayTimeInterval ati = r->getTimeInterval();
      double startTime      = ati.getStartInMJD()*86400;
      ArrayTime endArrayTime = ArrayTime(ati.getStart().get() + ati.getDuration().get());

      double endTime        = endArrayTime.getMJD()*86400;

      msFiller->addObservation((const char*) r->getTelescopeName().c_str(),
			 startTime,
			 endTime,
			 (const char*) r->getObserverName().c_str(),
			 (r->getObservingLog().size() == 0) ? ((const char**) 0): ((const char**) SConverter().to1DCStringArray(r->getObservingLog())),
			 (const char *) "ALMA",
			 (const char**) 0,
			 (const char*) "T.B.D.",
			 r->isReleaseDateExists() ? r->getReleaseDate().getMJD():0.0
			 );
    } 
    if (nExecBlock) cout << "converted in " << msFiller->ms()->observation().nrow() << " observation(s) in the measurement set" ;
    cout << endl;
  }

  // Load the Pointing table
  // Issues :
  // - pointingModelId , phaseTracking, sourceOffset and overTheTop not taken into account.

#if 0
  PointingTable& pointingT = ds->getPointing(); 
  {
    PointingRow* r = 0;
    int nPointing = pointingT.size();
    cout << "The dataset has " << nPointing << " pointing(s)..."; 
    vector<PointingRow *> v = pointingT.get();

    for (int i = 0; i < nPointing; i++) {
      r = v.at(i);
      
      // For now we just adapt the types of the time related informations and compute a mid-time.
      //
      double interval = ((double) r->getTimeInterval().getDuration().get()) / ArrayTime::unitsInASecond ;
      double time =  ((double) r->getTimeInterval().getStart().get()) / ArrayTime::unitsInASecond + interval/2.0;

      double* pointingOffset = 0;
      pointingOffset =  DConverter().to1DArray(r->getOffset());

      msFiller->addPointing(r->getAntennaId().getTagValue(),
		      time,
		      interval,
		      r->isNameExists()?r->getName().c_str(): "",
		      DConverter().to1DArray(r->getPointingDirection()),
		      DConverter().to1DArray(r->getTarget()),
		      pointingOffset,
		      DConverter().to1DArray(r->getEncoder()),
		      r->getPointingTracking() ? 1 :0);

    }

    if (nPointing) cout << "converted in " << msFiller->ms()->pointing().nrow() << " pointing(s) in the measurement set." ;
    cout << endl;
  }
#else
  PointingTable& pointingT = ds->getPointing();
  int nPointing = pointingT.size();
  cout << "The dataset has " << nPointing << " pointing(s)...";
  if (nPointing > 0) {
    PointingRow* r = 0;
    vector<PointingRow *> v = pointingT.get();
    
    int    *antenna_id_      = new int[nPointing];
    double *time_            = new double[nPointing];
    double *interval_        = new double[nPointing];
    string * namestr_        = new string[nPointing];
    char  **name_            = new char*[nPointing];
    double *direction_       = new double[2 * nPointing];
    double *target_          = new double[2 * nPointing];
    double *pointing_offset_ = new double[2 * nPointing];
    double *encoder_         = new double[2 * nPointing];
    bool   *tracking_        = new bool[nPointing];

    for (int i = 0; i < nPointing; i++) {
      r = v.at(i);

      antenna_id_[i]          = r->getAntennaId().getTagValue();
      interval_[i]            = ((double) r->getTimeInterval().getDuration().get()) / ArrayTime::unitsInASecond ;
      time_[i]                = ((double) r->getTimeInterval().getStart().get()) / ArrayTime::unitsInASecond + time_[i]/2.0;
      namestr_[i]             = r->isNameExists()?r->getName().c_str(): "";
      name_[i]                = const_cast<char *>(namestr_[i].data());
      direction_[2*i]         = r->getPointingDirection().at(0).at(0).get();
      direction_[2*i+1]       = r->getPointingDirection().at(0).at(1).get(); 
      target_[2*i]            = r->getTarget().at(0).at(0).get();
      target_[2*i+1]          = r->getTarget().at(0).at(1).get();
      pointing_offset_[2*i]   = r->getOffset().at(0).at(0).get();
      pointing_offset_[2*i+1] = r->getOffset().at(0).at(1).get();
      encoder_[2*i]           = r->getEncoder().at(0).get();
      encoder_[2*i+1]         = r->getEncoder().at(1).get();

      tracking_[i]            = r->getPointingTracking();
    }
    
    msFiller->addPointingSlice(nPointing,
			       antenna_id_,
			       time_,
			       interval_,
			       name_,
			       direction_,
			       target_,
			       pointing_offset_,
			       encoder_,
			       tracking_);
			       
    delete[] antenna_id_;
    delete[] time_;
    delete[] interval_;
    delete[] namestr_;
    delete[] name_;
    delete[] direction_;
    delete[] target_;
    delete[] pointing_offset_;
    delete[] encoder_;
    delete[] tracking_;
    cout << "converted in " << msFiller->ms()->pointing().nrow() << " pointing(s) in the measurement set." ;
  }
  cout << endl;
#endif

  // Load the processor table
  //
  ProcessorTable& processorT = ds->getProcessor();
  {
    ProcessorRow* r = 0;
    int nProcessor = processorT.size();
    cout << "The dataset has " << nProcessor << " processor(s)..."; 
    
    for (int i = 0; i < nProcessor; i++) {
      if ((r=processorT.getRowByKey(Tag(i, TagType::Processor))) == 0) {
	err.str("");
	(err << "Problem while reading the Processor table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(err.str());
      }
      
      msFiller->addProcessor((CProcessorType::name(r->getType())).c_str(),
		       r->getSubType().c_str(),
		       -1,    // Since there is no typeId in the ASDM.
		       r->getAlmaCorrelatorModeId().getTagValue());
    }
    if (nProcessor) cout << "converted in " << msFiller->ms()->processor().nrow() << " processor(s) in the measurement set." ;
    cout << endl;
  }
  

  // Load the source table
  //
  SourceTable& sourceT = ds->getSource();
  {
    SourceRow* r = 0;
    int nSource = sourceT.size();
    cout << "The dataset has " << nSource << " source(s)...";

    vector<SourceRow *> v = sourceT.get();
    for (int i = 0; i < nSource; i++) {
      r = v.at(i);
      // For now we just adapt the types of the time related informations and compute a mid-time.
      //
      double interval = ((double) r->getTimeInterval().getDuration().get()) / ArrayTime::unitsInASecond ;
      double time =  r->getTimeInterval().getStartInMJD()*86400 + interval / 2.0 ;

      double* position = 0;
      DConverter positionConverter;
      if (r->isPositionExists()){
	vector<Length> vp = r->getPosition();
	position = positionConverter.to1DArray(vp.at(0), vp.at(1), vp.at(2));
      } 
				
      double* restFrequency = 0;
      DConverter restFrequencyConverter;
      if (r->isRestFrequencyExists()) {
	restFrequency = restFrequencyConverter.to1DArray(r->getRestFrequency());
      }

      double* sysVel = 0;
      DConverter sysVelConverter;
      if (r->isSysVelExists()) {
	sysVel = sysVelConverter.to1DArray(r->getSysVel());
      }

      
      char** transition = 0;
      SConverter transitionConverter;
      if (r->isTransitionExists()) {
	transition = transitionConverter.to1DCStringArray(r->getTransition());
      }
      

      msFiller->addSource(r->getSourceId(),
		    time,
		    interval,
		    r->getSpectralWindowId().getTagValue(),
		    r->getNumLines(),
		    r->getSourceName().c_str(),
		    r->isCalibrationGroupExists() ? r->getCalibrationGroup() : 0,
		    r->getCode().c_str(),
		    DConverter().to1DArray(r->getDirection()),
		    position,
		    DConverter().to1DArray(r->getProperMotion()),
		    (const char **)transition,
		    restFrequency,
		    sysVel);
    }
    if (nSource) cout << "converted in " << msFiller->ms()->source().nrow() <<" source(s) in the measurement set." ;
    cout << endl;
  }  

#if 0

  //
  // Load the weather table
  WeatherTable& weatherT = ds->getWeather();

  {
    WeatherRow* r = 0;

    
    vector<WeatherRow*> v = weatherT.get();

    int nWeather = weatherT.size();
    cout << "The dataset has " << nWeather << " weather(s)...";
 
    for (int i = 0; i < nWeather; i++) {
      r = v.at(i);      
      double interval = ((double) r->getTimeInterval().getDuration().get()) / ArrayTime::unitsInASecond ;
      double time =  ((double) r->getTimeInterval().getStart().get());
      vector<WeatherRow*> v = weatherT.get();
      
      int nWeather = weatherT.size(); / ArrayTime::unitsInASecond + interval/2.0;
      /* 
	 msFiller->addWeather(Integer::parseInt(r->getAntennaId().getId()),
	 time,
	 interval,
	 r->getH2o(),
	 r->getH2oFlag(),
	 r->getPressure(),
	 r->getPressureFlag(),
	 r->getRelHumidity(),
	 r->getRelHumidityFlag(),
	 r->getTemperature(),
	 r->getTemperatureFlag(),
	 r->getWindDirection(),
	 r->getWindDirectionFlag(),
	 r->getWindSpeed(),
	 r->getWindSpeedFlag(),
	 r->getWindMax(),
	 r->getWindMaxFlag(),
	 r->isDewPointExists()?r->getDewPoint():-1.0,
	 r->isDewPointExists()?r->getDewPointFlag():true);
      */
    }
    if (nWeather) cout << "successfully copied them into the measurement set.";
    cout << endl;
  }
#endif

  // And then finally process the state and the main table in parallel.
  //
  
  // Some variables for time measurements.
  double cpu_time_asdm_overall  = 0.0;
  double real_time_asdm_overall = 0.0;
  
  double cpu_time_aips_overall  = 0.0;
  double real_time_aips_overall = 0.0;


#if 1 
  {
    MainTable& mainT = ds->getMain();
    StateTable& stateT = ds->getState();

    //    SDMBinaryData*                   sdmbdPtr=0;
    MainRow* r = 0;
    int nMain = mainT.size();
    vector<MainRow*> v = mainT.get();

    double cpu_time_asdm  = 0.0;
    double real_time_asdm = 0.0;

    double cpu_time_aips  = 0.0;
    double real_time_aips = 0.0;

    int mode;
    


    // Consider integration and subintegration


    // Prepare an UVW coordinate engine
    UvwCoords uvwCoords(ds);
    const VMSData *vmsDataPtr = 0;
    
    try {
      for (int i = 0; i < nMain; i++) {
	mode = 0; myTimer(&cpu_time_asdm, &real_time_asdm, &mode);
	r = v.at(i);

	mode = 1; myTimer(&cpu_time_asdm, &real_time_asdm, &mode);
	cpu_time_asdm_overall  += cpu_time_asdm;
	real_time_asdm_overall += real_time_asdm;
	
	if(sdmBinData.acceptMainRow(r)){
	  //	  cout << "Processing ASDM Main row #" << i+1 << endl;
	  mode = 0; myTimer(&cpu_time_asdm, &real_time_asdm, &mode);
	  vmsDataPtr = sdmBinData.getDataCols();

	  mode = 1; myTimer(&cpu_time_asdm, &real_time_asdm, &mode);
	  cpu_time_asdm_overall  += cpu_time_asdm;
	  real_time_asdm_overall += real_time_asdm;
	  

	  // Firstly populate the State table...
	  mode = 0; myTimer(&cpu_time_aips, &real_time_aips, &mode);
	  vector<int> msStateId;
	  for (unsigned int iState = 0; iState < vmsDataPtr->v_msState.size(); iState++) {
	    const sdmbin::MSState& msState = vmsDataPtr->v_msState.at(iState);
	    int retId = msFiller->addUniqueState(msState.sig,
					   msState.ref,
					   msState.cal,
					   msState.load,
					   msState.subscanNum,
					   msState.obsMode.c_str(),
					   false);
	    msStateId.push_back(retId);
	  }
	  
	  mode = 1; myTimer(&cpu_time_aips, &real_time_aips, &mode);
	  cpu_time_aips_overall  += cpu_time_aips;
	  real_time_aips_overall += real_time_aips;
	  
	  mode = 0; myTimer(&cpu_time_aips, &real_time_aips, &mode);
	  
	  // Then populate the Main table.
	  ComplexDataFilter filter; // To process the case numCorr == 3
	  cout << "ASDM Main table row #" << i+1
	       << " will be transformed into " << vmsDataPtr->v_antennaId1.size()
	       << " MS Main table rows" << endl;
	  
	  if (vmsDataPtr->v_antennaId1.size() == 0) {
	    cout << "No MS data produced for ASDM table row #" << i+1 << endl;
	    continue;
	  }


	  vector<vector<unsigned int> > filteredShape = vmsDataPtr->vv_dataShape;
	  for (unsigned int ipart = 0; ipart < vmsDataPtr->vv_dataShape.size(); ipart++) {
	    if (filteredShape.at(ipart).at(0) == 3)
	      filteredShape.at(ipart).at(0) = 4;
	  }
	  
	  vector<int> filteredDD;
	  for (unsigned int idd = 0; idd < vmsDataPtr->v_dataDescId.size(); idd++)
	    filteredDD.push_back(dataDescriptionMap.at(vmsDataPtr->v_dataDescId.at(idd)));
	  vector<float *> uncorrectedData;
	  vector<float *> correctedData;

	  ComplexDataFilter cdf;
	  map<AtmPhaseCorrectionMod::AtmPhaseCorrection, float*>::const_iterator iter;

	  for (unsigned int iData = 0; iData < vmsDataPtr->v_m_data.size(); iData++) {
	    if ((iter=vmsDataPtr->v_m_data.at(iData).find(AtmPhaseCorrectionMod::AP_UNCORRECTED)) != vmsDataPtr->v_m_data.at(iData).end())
	      uncorrectedData.push_back(cdf.to4Pol(vmsDataPtr->vv_dataShape.at(iData).at(0),
						   vmsDataPtr->vv_dataShape.at(iData).at(1),
						   iter->second));
	    else 
	      uncorrectedData.push_back(0);

	    if ((iter=vmsDataPtr->v_m_data.at(iData).find(AtmPhaseCorrectionMod::AP_CORRECTED)) != vmsDataPtr->v_m_data.at(iData).end())
	      correctedData.push_back(cdf.to4Pol(vmsDataPtr->vv_dataShape.at(iData).at(0),
						 vmsDataPtr->vv_dataShape.at(iData).at(1),
						 iter->second));	    
	    else
	      correctedData.push_back(0);	    
	  }
	  
	  /* compute the UVW */
#if 1

	  vector<double> uvw(3*vmsDataPtr->v_time.size());
	  
	  vector<casa::Vector<casa::Double> > v_uvw;
	  uvwCoords.uvw_bl(r, vmsDataPtr->v_timeCentroid, e_query_cm, 
			   sdmbin::SDMBinData::dataOrder(),
			   v_uvw);

	  int k = 0;
	  for (unsigned int i = 0; i < v_uvw.size(); i++) {
	    uvw[k++] = v_uvw[i][0]; 
	    uvw[k++] = v_uvw[i][1];
	    uvw[k++] = v_uvw[i][2];
	  } 
#else
	  vector<double> uvw(3*vmsDataPtr->v_time.size(), 0.);
#endif
	  msFiller->addData(complexData,
			    (vector<double>&) vmsDataPtr->v_time, // this is already time midpoint
			    (vector<int>&) vmsDataPtr->v_antennaId1,
			    (vector<int>&) vmsDataPtr->v_antennaId2,
			    (vector<int>&) vmsDataPtr->v_feedId1,
			    (vector<int>&) vmsDataPtr->v_feedId2,
			    filteredDD,
			    vmsDataPtr->processorId,
			    (vector<int>&)vmsDataPtr->v_fieldId,
			    (vector<double>&) vmsDataPtr->v_interval,
			    (vector<double>&) vmsDataPtr->v_exposure,
			    (vector<double>&) vmsDataPtr->v_timeCentroid,
			    (int) r->getScanNumber(), 
			    0,                                               // Array Id
			    (int) r->getExecBlockId().getTagValue(), // Observation Id
			    (vector<int>&)msStateId,
			    uvw,
			    filteredShape, // vmsDataPtr->vv_dataShape after filtering the case numCorr == 3
			    uncorrectedData,
			    correctedData,
			    (vector<unsigned int>&)vmsDataPtr->v_flag);
	  mode = 1; myTimer(&cpu_time_aips, &real_time_aips, &mode);
	  cpu_time_aips_overall  += cpu_time_aips;
	  real_time_aips_overall += real_time_aips;	
	}else{
	  cout <<"No data retrieved in the SDM row #" << i+1 << " (" << sdmBinData.reasonToReject(r) <<")" << endl;
	}      
      }
    }
    catch ( IllegalAccessException& e) {
      cout << e.getMessage() << endl;
    }
    catch ( SDMDataObjectReaderException& e ) {
      cout << e.getMessage() << endl;
      exit (-1);
    }
    catch ( exception & e) {
      cout << e.what() << endl;
      exit (-1);
    }
    
    cout << "The dataset has "  << stateT.size() << " state(s)..." ;
    if (stateT.size()) 
      cout << "converted in " << msFiller->ms()->state().nrow() << " state(s) in the measurement set";
    cout << endl;

    cout << "The dataset has " << mainT.size() << " main(s)...";
    if (mainT.size())
      cout << "converted in " << msFiller->ms()->nrow() << " main(s) in the measurement set";
    cout << endl;
  }
#endif

  msFiller->end(0.0);

  cout << "Overall cpu time spent in ASDM methods to read/process the ASDM Main table : cpu = " << cpu_time_asdm_overall  << " s." << endl;
  cout << "Overall cpu time spent in AIPS methods to fill the MS Main table : cpu = " << cpu_time_aips_overall  << " s." << endl;

  delete msFiller;
  delete ds;
  return 0;
}
