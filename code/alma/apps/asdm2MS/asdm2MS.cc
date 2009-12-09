#define DDPRIORITY 1
#include <iostream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <cmath>

#include <boost/algorithm/string.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/algorithm/string.hpp>
using namespace boost;

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <ASDMAll.h>

#include "SDMBinData.h"
using namespace sdmbin;

#include <exception>
using namespace asdm;
#include "IllegalAccessException.h"

#include "UvwCoords.h"
#include "ASDM2MSFiller.h"

#include "measures/Measures/Stokes.h"
#include "measures/Measures/MFrequency.h"
using namespace casa;

#include "CBasebandName.h"
#include "CFrequencyReferenceCode.h"
#include "CPolarizationType.h"
#include "CProcessorSubType.h"
#include "CProcessorType.h"
#include "CScanIntent.h"
#include "CSubscanIntent.h"
#include "CStokesParameter.h"

#include "Name2Table.h"
#include "ASDMVerbatimFiller.h"

#include "SDMDataObjectReader.h"
#include "SDMDataObject.h"

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
string appName;
bool verbose = true;

//LogIO os;

#include <casa/Logging/StreamLogSink.h>
#include <casa/Logging/LogSink.h>

void info (const string& message) {
  
  if (!verbose){
	  return;
  }
  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::NORMAL));
}

void error(const string& message) {
	LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::SEVERE));
  //os << LogIO::POST;
  exit(1);
}

#include <iostream>
#include <sstream>

ostringstream errstream;
ostringstream infostream;
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

class FrequencyReferenceMapper {

public :
  FrequencyReferenceMapper();
  ~FrequencyReferenceMapper();

  static MFrequency::Types value(FrequencyReferenceCodeMod::FrequencyReferenceCode frc);
}
  ;

FrequencyReferenceMapper::FrequencyReferenceMapper() {
  ;
}

FrequencyReferenceMapper::~FrequencyReferenceMapper() {
  ;
}

MFrequency::Types FrequencyReferenceMapper::value(FrequencyReferenceCodeMod::FrequencyReferenceCode frc) {
  switch (frc) {
  case LABREST : 
    errstream.str("");
    errstream << " Can't map FrequencyReferenceCode::LABREST to an MFrequency::Types" << endl;
    error(errstream.str());
    break;
    
  case FrequencyReferenceCodeMod::LSRD : return MFrequency::LSRD; 
  case FrequencyReferenceCodeMod::LSRK : return MFrequency::LSRK; 
  case FrequencyReferenceCodeMod::BARY : return MFrequency::BARY;
  case FrequencyReferenceCodeMod::REST : return MFrequency::REST;
  case FrequencyReferenceCodeMod::GEO  : return MFrequency::GEO; 
  case FrequencyReferenceCodeMod::GALACTO : return MFrequency::GALACTO;
  case FrequencyReferenceCodeMod::TOPO : return MFrequency::TOPO; 
  }
  return MFrequency::TOPO; // Never happens.
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
  double* to1DArray(const vector<double>&);
  double* to1DArray(const vector<vector<double> >& v);
  double* to1DArray(const vector<Frequency>&);
  double* to1DArray(const vector<Angle>&);
  double* to1DArray(const vector<AngularRate>&);
  double* to1DArray(const vector<Length>&);
  double* to1DArray(const vector<Speed>&);
  double* to1DArray(const vector<vector <Angle> >&);
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

double* DConverter::to1DArray(const vector<double>& v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i);

  return da;
}


double* DConverter::to1DArray(const vector<vector<double> >& v) {
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

double* DConverter::to1DArray(const vector<Frequency>& v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i).get();

  return da;
}

double*  DConverter::to1DArray(const vector<Angle>& v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i).get();

  return da;
}

double*  DConverter::to1DArray(const vector<Length>& v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i).get();

  return da;
}


double*  DConverter::to1DArray(const vector<Speed>& v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i).get();

  return da;
}

double*  DConverter::to1DArray(const vector<AngularRate>& v) {
  da = new double[v.size()];
  for (unsigned int i=0; i < v.size(); i++)
    da[i] = v.at(i).get();

  return da;
}

double* DConverter::to1DArray(const vector<vector<Angle> >& v) {
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

//
//  A collection of functions to combine target and offset
//  into a pointing direction in the Pointing table.
//
/**
 *
 * From a Fortran 90 subroutine, written by Didier Despois (1980)
 * and revised by Michel Perault (1984).
 *
 * Computes a coodinates conversion matrix for a rotation defined
 * by Euler angles psi, the and phi (values in radians).
 *
 * The product of the euclidean coordinates of a vector in the original
 * base by this matrix gives the coordinates in the rotated base.
 * 
 */
void eulmat(double psi, double the, double phi, vector<vector<double> >& mat) {
  double cpsi, spsi, cthe, sthe, cphi, sphi;
  double x1, x2;

  cpsi = cos(psi);
  spsi = sin(psi);
  cthe = cos(the);
  sthe = sin(the);
  cphi = cos(phi);
  sphi = sin(phi);

  x1 = spsi*cthe;
  x2 = cpsi * cthe; 
  mat.at(0).at(0) =  cpsi * cphi - x1 * sphi;
  mat.at(0).at(1) =  spsi * cphi + x2 * sphi;
  mat.at(0).at(2) =  sthe * sphi;

  mat.at(1).at(0) = -cpsi * sphi - x1 * cphi;
  mat.at(1).at(1) = -spsi * sphi + x2 * cphi;
  mat.at(1).at(2) = sthe * cphi;

  mat.at(2).at(0) =  spsi * sthe;
  mat.at(2).at(1) =  -cpsi * sthe;
  mat.at(2).at(2) =  cthe;
}

/**
 * Performs a product Matrix x Vector.
 *
 * Attention ! vFactor , mFactor and vResult must be correctly
 * sized ! And also vFactor and vResult must be different objects.
 */
void matvec (const vector<vector<double> >& mFactor, const vector<double>& vFactor, vector<double>& vResult) {
  vResult.at(0) = vFactor.at(0)*mFactor.at(0).at(0) + vFactor.at(1)*mFactor.at(0).at(1) + vFactor.at(2)*mFactor.at(0).at(2);
  vResult.at(1) = vFactor.at(0)*mFactor.at(1).at(0) + vFactor.at(1)*mFactor.at(1).at(1) + vFactor.at(2)*mFactor.at(1).at(2);
  vResult.at(2) = vFactor.at(0)*mFactor.at(2).at(0) + vFactor.at(1)*mFactor.at(2).at(1) + vFactor.at(2)*mFactor.at(2).at(2);
}

/**
 * Spherical to cartesian conversion.
 * Radius assumed to be equal to 1.
 *
 * Attention ! a and x must be correctly sized !
 */
void rect(const vector<double>& s, vector<double>& x) {
  x.at(0) = cos(s.at(0)) * cos(s.at(1));
  x.at(1) = sin(s.at(0)) * cos(s.at(1));
  x.at(2) = sin(s.at(1));
}

/**
 * Cartesian to spherical conversion.
 * Radius assumed to be equal to 1.
 *
 * Attention ! a and x must be correctly sized !
 */
void spher(const vector<double>& x, vector<double>& s) {
  s.at(0) = atan2(x.at(1) , x.at(0));
  s.at(1) = atan2(x.at(2), sqrt(x.at(0)*x.at(0) + x.at(1)*x.at(1)));
}

/**
 * Reorder the collection of spectral windows ids.
 *
 * Given a dataset 'ds', the statement 'ds.spectralWindowTable.get()' returns
 * a vector of pointers on instances of 'SpectralWindowRow' and 
 * defines implicitely an ordered collection of Tag (of type TagType::SpectralWindow) instances
 * obtained by using 'getSpectralWindowId()' on each instance.
 *
 * This method partitions this ordered collection into two collections
 * based on the criterium : does the Tag appear in at least one row
 * of the DataDescription table in the SpectralWindowId attribute or not.
 *
 * The methods returns a vector of (SpactralWindow) Tag  obtained by appending the collections
 * of (SpectralWindow) Tag which appear in the DataDescription table
 * followed by the collection of (SpectralWindow) Tag which does not appear
 * in the DataDescription table. The int value which associated with a Tag
 * is the order number of the insertion of the pair (Tag, int) in the map.
 * 
 */
vector<Tag> reorderSwIds(const ASDM& ds) {
  vector<SpectralWindowRow *> swRs = ds.getSpectralWindow().get();
  vector<DataDescriptionRow *> ddRs = ds.getDataDescription().get();
  map<Tag, bool> isInDD;

  for (vector<SpectralWindowRow *>::size_type  i = 0; i < swRs.size(); i++) isInDD[swRs[i]->getSpectralWindowId()] = false;
  for (vector<DataDescriptionRow *>::size_type i = 0; i < ddRs.size(); i++) isInDD[ddRs[i]->getSpectralWindowId()] = true;

  vector<Tag> swIdsDD, swIdsNoDD;
  for (map<Tag, bool>::iterator iter = isInDD.begin(); iter != isInDD.end(); ++iter)
    if (iter->second) swIdsDD.push_back(iter->first);
    else swIdsNoDD.push_back(iter->first);

  vector<Tag> result (swIdsDD.begin(), swIdsDD.end());
  for (vector<Tag>::size_type i = 0; i < swIdsNoDD.size(); i++)
    result.push_back(swIdsNoDD[i]);

  return result;
}
  
void usage(char* command) {
  cout << "Usage : " << command << " DataSetName" << endl;
}

template<class T>
void checkVectorSize(const string& vectorAttrName,
		     const vector<T>& vectorAttr,
		     const string& sizeAttrName,
		     unsigned int sizeAttr,
		     const string& tableName,
		     unsigned int rowNumber) {
  if (vectorAttr.size() != sizeAttr) {
    errstream.str("");
    errstream << "In the '"
	      << tableName 
	      << " table, at row #"
	      << rowNumber
	      << ", I found '"
	      << vectorAttrName
	      << "' with a size of '"
	      << vectorAttr.size() 
	      << "', I was expecting it to be equal to the size of '"
	      << sizeAttrName
	      <<"' which is '"
	      <<"'"
	      <<sizeAttr
	      <<"'. I can't go further."
	      << endl;
    error(errstream.str());
  }
}

EnumSet<AtmPhaseCorrection> apcLiterals(const ASDM& ds, const string& asdmBinaryPath) {
  EnumSet<AtmPhaseCorrection> result;

  vector<MainRow *> mRs = ds.getMain().get();
  
  for (unsigned int i = 0; i < mRs.size(); i++) {
    ConfigDescriptionRow * configDescriptionRow = mRs.at(i)->getConfigDescriptionUsingConfigDescriptionId();
    vector<AtmPhaseCorrection> apc = configDescriptionRow -> getAtmPhaseCorrection();
    for (unsigned int i = 0; i < apc.size(); i++)
      result.set(apc.at(i));
  }
  /*
    for (unsigned int i = 0; i < mRs.size(); i++) {
    string binaryFileName = replace_all_copy(mRs.at(i)->getDataOid().getEntityId().toString(), "/", "_");
    replace_all(binaryFileName, ":", "_");
    try {
    SDMDataObjectReader sdmDataObjectReader;
    vector<AtmPhaseCorrection> apc = sdmDataObjectReader.read(asdmBinaryPath+"/"+binaryFileName).dataStruct().apc();
    for (unsigned int i = 0; i < apc.size(); i++)
    result.set(apc.at(i));

    sdmDataObjectReader.done();
    }
    catch (SDMDataObjectReaderException& e1) {
    cout << e1.getMessage() << endl;
    }
    catch (SDMDataObjectException& e2) {
    cout << e2.getMessage() << endl;
    }
    }
  */
  return result;
}

bool hasCorrectedData(const EnumSet<AtmPhaseCorrection>& es) {
  return es[AP_CORRECTED];
}

bool hasUncorrectedData(const EnumSet<AtmPhaseCorrection>& es) {
  return es[AP_UNCORRECTED];
}
		    
//
// A number of EnumSet to encode the different selection criteria.
//
EnumSet<CorrelationMode>         es_cm;
EnumSet<SpectralResolutionType>  es_srt;
EnumSet<TimeSampling>            es_ts;
Enum<CorrelationMode>            e_query_cm; 
EnumSet<AtmPhaseCorrection>      es_query_apc;    

// An EnumSet to store the different values of AtmPhaseCorrection present
// in the binary data (apc in datastruct).
//
EnumSet<AtmPhaseCorrection>      es_apc;

//
// By default the resulting MS will not contain compressed columns
// unless the 'compress' option has been given.
// 
Bool                             withCompression = false;


/**
 * The main function.
 */
int main(int argc, char *argv[]) {

  string dsName;
  string msNamePrefix;
  string msNameExtension;
  boost::filesystem::path msPath;

  appName = string(argv[0]);
  ofstream ofs;

  LogSinkInterface& lsif = LogSink::globalSink();

  //   Process command line options and parameters.
  po::variables_map vm;
  try {


    // Declare the supported options.

    po::options_description generic("Converts an ASDM dataset into a CASA measurement set.\n"
				    "Usage : " + appName +" [options] asdm-directory [ms-directory-prefix]\n\n"
				    "Command parameters: \n"
				    " asdm-directory : the pathname to the ASDM dataset to be converted \n"
				    " ms-directory-prefix : the prefix of the pathname(s) of the measurement set(s ) to be created,\n"
				    " this prefis is completed by a suffix to form the name(s) of the resulting measurement set(s), \n"
				    " this suffix depends on the selected options (see options compression and wvr-correction) \n"
				    ".\n\n"
				    "Allowed options:");
    generic.add_options()
      ("help", "produces help message.")
      ("icm",  po::value<string>()->default_value("all"), "specifies the correlation mode to be considered on input. A quoted string containing a sequence of 'ao' 'co' 'ac' 'all' separated by whitespaces is expected")
      ("isrt", po::value<string>()->default_value("all"), "specifies the spectral resolution type to be considered on input. A quoted string containing a sequence of 'fr' 'ca' 'bw' 'all' separated by whitespaces is expected")
      ("its",  po::value<string>()->default_value("all"), "specifies the time sampling (INTEGRATION and/or SUBINTEGRATION)  to be considered on input. A quoted string containing a sequence of 'i' 'si' 'all' separated by whitespaces is expected")  
      ("ocm",  po::value<string>()->default_value("ca"),  "output data for correlation mode AUTO_ONLY (ao) or CROSS_ONLY (co) or CROSS_AND_AUTO (ca)")
      ("compression,c", "produces compressed columns in the resulting measurement set (not set by default). When this option is selected the string '-compressed' is inserted in the pathname of the resulting measurement set.")
      ("asis", po::value<string>(), "creates verbatim copies of the ASDM tables in the output measurement set. The value given to this option must be a quoted string containing a list of table names separated by space characters; the wildcard character '*' is allowed in table names.")
      ("wvr-corrected-data", po::value<string>()->default_value("no"),  "specifies wich values are considered in the ASDM binary data to fill the DATA column in the MAIN table of the MS. Expected values for this option are 'no' for the uncorrected data (this is the default), 'yes' for the corrected data and 'both' for corrected and uncorrected data. In the latter case, two measurement sets are created, one containing the uncorrected data and the other one, whose name is suffixed by '-wvr-corrected', containing the corrected data.")
      ("logfile,l", po::value<string>(), "specifies the log filename. If the option is not used then the logged informations are written to the standard error stream.")
      ("verbose,v", "logs numerous informations as the filler is working.")
      ("revision,r", "logs information about the revision of this application.");


    // Hidden options, will be allowed both on command line and
    // in config file, but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()
      ("asdm-directory", po::value< string >(), "asdm directory")
      ;
    hidden.add_options()
      ("ms-directory-prefix", po::value< string >(), "ms directory prefix")
      ;

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(hidden);
    
    po::positional_options_description p;
    p.add("asdm-directory", 1);
    p.add("ms-directory-prefix", 1);
    
    // Parse the command line and retrieve the options and parameters.
    po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);

    po::notify(vm);
    
    // Where do the log messages should go ?
    if (vm.count("logfile")) {
      //LogSinkInterface *theSink;
      ofs.open(vm["logfile"].as<string>().c_str(), ios_base::app);
      LogSinkInterface *theSink = new casa::StreamLogSink(&ofs);
      LogSink::globalSink(theSink);
    }

    // Help ? displays help's content and don't go further.

    if (vm.count("help")) {
      errstream.str("");
      errstream << generic << "\n" ;
      error(errstream.str());
    }

    // Revision ? displays revision's info and don't go further.
    if (vm.count("revision")) {
      errstream.str("");
      errstream << "$Id: asdm2MS.cpp,v 1.34 2009/11/24 13:38:53 mcaillat Exp $" << "\n" ;
      error(errstream.str());
    }

    // Verbose or quiet ?
    verbose = vm.count("verbose") > 0;
    
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
	errstream.str("");
	errstream << "Token '" << token << "' invalid for --icm option." << endl;
	errstream << generic << endl;
	error(errstream.str());
      }
    }

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
	errstream.str("");
	errstream << "Token '" << token << "' invalid for --isrt option." << endl;
	errstream << generic;
	error(errstream.str());
      }
    }


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
	errstream.str("");
	errstream << "Token '" << token << "' invalid for its option." << endl;
	errstream << generic ;
	error(errstream.str());
      }
    }

    // Selection of the correlation mode of data to be produced in the measurement set.

    string ocm_opt = vm["ocm"].as< string >();
    if ( ocm_opt.compare("co") == 0 )
      e_query_cm = CROSS_ONLY;
    else if ( ocm_opt.compare("ao") == 0 )
      e_query_cm = AUTO_ONLY;
    else if ( ocm_opt.compare("ca") == 0 )
      e_query_cm = CROSS_AND_AUTO;
    else {
      errstream.str("");
      errstream << "Token '" << ocm_opt << "' invalid for ocm option." << endl;
      errstream << generic ;
      error(errstream.str());
    }

    if (vm.count("asdm-directory")) {
      string dummy = vm["asdm-directory"].as< string >();
      dsName = lrtrim(dummy) ;
      if (boost::algorithm::ends_with(dsName,"/")) dsName.erase(dsName.size()-1);
    }
    else {
      errstream.str("");
      errstream << generic ;
      error(errstream.str());
    }

    if (vm.count("ms-directory-prefix")) {
      string dummyMSName = vm["ms-directory-prefix"].as< string >();
      dummyMSName = lrtrim(dummyMSName);
      if (boost::algorithm::ends_with(dummyMSName, "/")) dummyMSName.erase(dummyMSName.size()-1);
      boost::filesystem::path msPath(lrtrim(dummyMSName));
      string msDirectory = msPath.branch_path().string();
      msDirectory = lrtrim(msDirectory);
      if (msDirectory.size() == 0) msDirectory = ".";
      msNamePrefix = msDirectory + "/" + boost::filesystem::basename(lrtrim(dummyMSName));
      msNameExtension = boost::filesystem::extension(lrtrim(dummyMSName));
    }
    else {
      msNamePrefix = dsName;
      msNameExtension = ".ms";
    }

    // Does the user want compressed columns in the resulting MS ?

    if ((withCompression = (vm.count("compression") != 0))) {
      infostream.str("");
      infostream << "Compressed columns in the resulting MS(s) : Yes" ;
      info(infostream.str());
    }
    else {
      infostream.str("");
      infostream << "Compressed columns in the resulting MS(s) : No" ;
      info(infostream.str());
    }

    // WVR uncorrected and|or corrected data required ?
    string wvr_corrected_data = vm["wvr-corrected-data"].as<string>();
    if (wvr_corrected_data.compare("no") == 0)
      es_query_apc.fromString("AP_UNCORRECTED");
    else if (wvr_corrected_data.compare("yes") == 0)
      es_query_apc.fromString("AP_CORRECTED");
    else if (wvr_corrected_data.compare("both") == 0)
      es_query_apc.fromString("AP_CORRECTED AP_UNCORRECTED");
    else {
      errstream.str("");
      errstream << "Token '" << wvr_corrected_data << "' invalid for wvr-corrected-data." << endl;
      errstream << generic;
      error(errstream.str());
    }
  }
  catch (std::exception& e) {
    errstream.str("");
    errstream << e.what();
    error(errstream.str());
  }

  //
  // Try to open an ASDM dataset whose name has been passed as a parameter on the command line
  //
  if ( dsName.at(dsName.size()-1) == '/' ) dsName.erase(dsName.size()-1);


  double cpu_time_parse_xml  = 0.0;
  double real_time_parse_xml = 0.0;
  int mode;
  mode = 0; myTimer(&cpu_time_parse_xml, &real_time_parse_xml, &mode);

  ASDM* ds = new ASDM();

  try {
    infostream.str("");
    infostream << "Input ASDM dataset : " << dsName << endl;
    info(infostream.str());
    ds->setFromFile(dsName);
  }
  catch (ConversionException e) {
    errstream.str("");
    errstream << e.getMessage();
    error(errstream.str());
  }
  catch (std::exception e) {
    errstream.str("");
    errstream << e.what();
    error(errstream.str());
  }
  catch (...) {
    errstream.str("");
    errstream << "Uncaught exception !" << endl;
    error(errstream.str());
  }
  

  mode = 1; myTimer(&cpu_time_parse_xml, &real_time_parse_xml, &mode);
  infostream.str("");
  infostream << "Time spent parsing the ASDM medata : " << cpu_time_parse_xml << " s.";
  info(infostream.str());

  //
  // What are the apc literals present in the binary data.
  //
  es_apc = apcLiterals(*ds, dsName+"/ASDMBinary");
 
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

  //
  // Report the selection's parameters.
  //
  infostream.str("");
  infostream << "Correlation modes requested : " << e_query_cm.str() << endl;
  infostream << "Spectral resolution types requested : " << es_srt.str() << endl;
  infostream << "Time sampling requested : " << es_ts.str() << endl;
  infostream << "WVR uncorrected and|or corrected data requested : " << es_query_apc.str() << endl;
  info(infostream.str());

  //
  // Shall we have Complex or Float data ?
  //
  bool complexData;
  try {
    complexData =  sdmBinData.isComplexData();
  }
  catch (Error & e) {
    errstream.str("");
    errstream << e.getErrorMessage();
    error(errstream.str());
  }

  //
  // Prepare a map AtmPhaseCorrection -> name of measurement set.
  // Three cases are possible :
  // only AP_CORRECTED -> MS name is suffixed with "-wvr-corrected",
  // only AP_UNCORRECTED -> MS name has no particular suffix,
  // AP_CORRECTED and AP_UNCORRECTED -> 2 MSs whith names defined by the two above rules.
  //
  map<AtmPhaseCorrection, string> msNames;
  if (hasCorrectedData(es_apc) && es_query_apc[AP_CORRECTED]) {
    msNames[AP_CORRECTED] = msNamePrefix + "-wvr-corrected";
  }
  if (hasUncorrectedData(es_apc) && es_query_apc[AP_UNCORRECTED]) {
    msNames[AP_UNCORRECTED] = msNamePrefix;
  }
      
  if (msNames.size() == 0) {
    //
    // no MS can be produced due to the selection parameters values.
    // 
    infostream.str("");
    infostream << "No measurement set can be produced with  your selection criteria on '" << dsName << "'" << endl;
    info(infostream.str());
    delete ds;
    exit(1);
  }
  else {
    //
    // OK, we are going to produce at least one MS.
    // If the '--compression' option has been used then append a suffix ".compressed" to
    // the MS name.
    // And eventually always suffix with ".ms".
    //
    for (map<AtmPhaseCorrection, string>::iterator iter=msNames.begin(); iter != msNames.end(); ++iter) {
      if (withCompression)
	iter->second = iter->second + ".compressed";
      iter->second +=  msNameExtension;
    }
  }

  infostream.str("");
  infostream << "The resulting measurement set will contain a '" << ((complexData) ? "DATA" : "FLOAT_DATA") << "' column" << endl; 
  info(infostream.str());

#if DDPRIORITY
  // The data returned by getDataCols will be ordered in priority by Data Descriptions.
  sdmBinData.setPriorityDataDescription();
#endif


  //
  // Create the measurement set(s). 
  map<AtmPhaseCorrection, ASDM2MSFiller*> msFillers;
  try {
    for (map<AtmPhaseCorrection, string>::iterator iter = msNames.begin(); iter != msNames.end(); ++iter) {
      msFillers[iter->first] = new ASDM2MSFiller(msNames[iter->first].c_str(),
						 0.0,
						 false,
						 (Bool) complexData,
						 withCompression);
      info("About to create a filler for the measurement set '" + msNames[iter->first] + "'");
    }
  }
  catch(AipsError & e) {
    errstream.str("");
    errstream << e.getMesg();
    error(errstream.str());
  }
  catch (std::exception & e) {
    errstream.str("");
    errstream << e.what();
    error(errstream.str());
  }

  //--info("About to create a new measurement set '" + ost.str() + "'");

  msFiller = msFillers.begin()->second;
  //msFiller = new ASDM2MSFiller(ost.str().c_str(), 0.0, false, (Bool) complexData, withCompression);


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
  AntennaTable& antennaT = ds->getAntenna();


  //
  // Write the Antenna table.
  // 
  // (This part needs the Station table)
  //
  // At the same time, we populate a map ASDM Station Tag -> MS ANTENNA ID
  // which will be useful when the Weather table will be converted.
  //
  unsigned int numTrueAntenna;
  { 
    AntennaRow*   r   = 0;

    int nAntenna = antennaT.size();
    infostream.str("");
    infostream << "The dataset has " << nAntenna << " antenna(s)...";
    info(infostream.str());
    
    for (int i = 0; i < nAntenna; i++) {
      if ((r = antennaT.getRowByKey(Tag(i, TagType::Antenna))) == 0){
	errstream.str("");
	errstream << "Problem while reading the Antenna table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl;
	error(errstream.str());
      }

      // We assume that wether the ExecBlock table has one row or that all ExecBlocks come from the same site.
      // reb = execBlockT.get().at(0);

      // The MS Antenna position is defined as the sum of the ASDM station position and
      // of the ASDM Antenna position after applying to it a coordinate system transformation.
      // Since the ASDM Antenna position is 0,0,0 for now, we only use the ASDM station position.
      vector<Length> position = r->getStationUsingStationId()->getPosition();
      double xPosition = position.at(0).get();
      double yPosition = position.at(1).get();
      double zPosition = position.at(2).get();

      vector<Length> offset = r->getOffset();
      double xOffset = offset.at(0).get();
      double yOffset = offset.at(1).get();
      double zOffset = offset.at(2).get();
      
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	int antenna_id = iter->second->addAntenna(r->getName().c_str(),
						  r->getStationUsingStationId()->getName().c_str(),
						  xPosition,
						  yPosition,
						  zPosition,
						  xOffset,
						  yOffset,
						  zOffset,
						  (float)r->getDishDiameter().get());	
      }
    }
    numTrueAntenna = msFillers.begin()->second->ms()->antenna().nrow();
    if (nAntenna) {
      infostream.str("");
      infostream << "converted in " << numTrueAntenna << " antenna(s)  in the measurement set(s)." ;
      info(infostream.str());
    }
  }
  
  SpectralWindowTable& spwT = ds->getSpectralWindow();  
  //
  // Read/Write the SpectralWindow table.
  //
  vector<Tag> reorderedSwIds = reorderSwIds(*ds); // The vector of Spectral Window Tags in the order they will be inserted in the MS.
  map<int, int> swIdx2Idx ;                       // A map which associates old and new index of Spectral Windows before/after reordering.
  for (vector<Tag>::size_type i = 0; i != reorderedSwIds.size() ; i++) swIdx2Idx[reorderedSwIds[i].getTagValue()] = i;
  
  try {
    
    SpectralWindowRow* r = 0;
    int nSpectralWindow = spwT.size();
    
    infostream.str("");
    infostream << "The dataset has " << nSpectralWindow << " spectral window(s)..."; 
    info(infostream.str());
    
    for (vector<Tag>::size_type i = 0; i < reorderedSwIds.size(); i++) {
      if ((r = spwT.getRowByKey(reorderedSwIds[i])) == 0) {
	errstream.str("");
	(errstream << "Problem while reading the SpectralWindow table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(errstream.str());
      }
      
      
      /*
       * associated spectral windows and and natures.
       */
      
      unsigned int numAssocValues = 0;
      if (r->isNumAssocValuesExists()) numAssocValues = r->getNumAssocValues();
      
      // Test the simultaneous presence or absence of assocNature and assoSpectralWindowId
      if ((r->isAssocNatureExists() && !r->isAssocSpectralWindowIdExists()) ||
	  (!r->isAssocNatureExists() && r->isAssocSpectralWindowIdExists())) {
	errstream.str("");
	errstream << "Only one of the attributes assocSpectralWindowId and assocNature is present. Can't go further."
		  << endl;
	error(errstream.str());
      }
      
      int* assocSpectralWindowId_ = 0;
      IConverter assocSpwIdConverter;
      vector<Tag> assocSpectralWindowId;

      char** assocNature_ = 0;
      SConverter assocNatureConverter;
      vector<SpectralResolutionType> assocNature;

      if (r->isAssocSpectralWindowIdExists()) { // it exists then the assocNature exists also, given the test
	                                        // which is done before.
	assocSpectralWindowId = r->getAssocSpectralWindowId();
	if (numAssocValues != assocSpectralWindowId.size()) {
	  infostream.str("");
	  infostream << "The size of assocSpectralWindowId ('"
		     << assocSpectralWindowId.size()
		     << "') is not equal to the value announced in numAssocValues ('"
		     << numAssocValues
		     << "'). Ignoring the difference and sending the full vector assocSpectralWindowId to the filler" 
	    ;
	  info(infostream.str());
	}
	numAssocValues = assocSpectralWindowId.size();
	assocSpectralWindowId_ = assocSpwIdConverter.to1DArray(assocSpectralWindowId);

	// Take into account the re ordering of the spectral window indices.
	for (unsigned int iAssocSw = 0; iAssocSw < numAssocValues; iAssocSw++)
	  assocSpectralWindowId_[iAssocSw] =  swIdx2Idx[assocSpectralWindowId_[iAssocSw]];
	
	assocNature = r->getAssocNature();
	if (assocNature.size() != assocSpectralWindowId.size()) {
	  infostream.str("");
	  infostream << "The size of assocNature ('"
		     << assocNature.size() 
		     << "') is not equal to the size of assocSpectralWindowId ('"
		     << assocSpectralWindowId.size()
		     << "'). Ignoring the difference and sending the full assocNature vector to the filler.";
	  info(infostream.str());
	}
	assocNature_ = assocNatureConverter.to1DCStringArray<SpectralResolutionType, CSpectralResolutionType>(r->getAssocNature());
      }
      
      
      /* Processing the chanFreqXXX
       *
       * Either (chanFreqStart, chanFreqStep) or (chanFreqArray) must be present
       * with a priority to chanFreqArray if both are present. If chanFreqArray
       * is present it *must* have a size equal to numChan otherwiser EXIT.
       */
      bool chanStartAndStep = r->isChanFreqStartExists() && r->isChanFreqStepExists();
      bool chanArray = r->isChanFreqArrayExists();
      if (!chanStartAndStep && !chanArray) {
	errstream.str("");
	errstream << "Did not find (chanFreqStart, chanFreqStep) nor chanFreqArray. Can't go further";
	error(errstream.str());
      }
      
      double* chanFreq1D = (double *) 0;
      DConverter chanFreqConverter;
      vector<Frequency> chanFreqArray;
      Frequency chanFreqStart, chanFreqStep;
      if (chanArray) { // Frequency channels are specified by an array.
	chanFreqArray = r->getChanFreqArray();
	if (chanFreqArray.size() != (unsigned int)r->getNumChan()) {
	  errstream.str("");
	  errstream << "Size of chanFreqArray ('"
		    << chanFreqArray.size()
		    << "') is not equal to numChan ('"
		    << r->getNumChan()
		    << "'). Can't go further.";
	  error(errstream.str());
	}
      }
      else { // Frequency channels are specified by a (start, step) pair.
	chanFreqStart = r->getChanFreqStart();
	chanFreqStep  = r->getChanFreqStep();
	for (int i = 0; i < r->getNumChan(); i++)
	  chanFreqArray.push_back(chanFreqStart + i * chanFreqStep);
      }
      chanFreq1D = chanFreqConverter.to1DArray(chanFreqArray);
      
      /* Processing the chanWidthXXX
       *
       * Either chanWidth or chanWidthArray must be present
       * with a priority to chanWidthArray if both are present. If chanWidthArray
       * is present it *must* have a size equal to numChan otherwiser EXIT.
       */
      if (!r->isChanWidthExists() && !r->isChanWidthArrayExists()) {
	errstream.str("");
	errstream << "Did not find chanWidth nor chanWidthArray. Can't go further";
	error(errstream.str());
      }
      
      double* chanWidth1D = (double *) 0;
      DConverter chanWidthConverter;
      vector<Frequency> chanWidthArray;
      if (r->isChanWidthArrayExists()) { // Frequency channels widths are specified by an array.
	chanWidthArray = r->getChanWidthArray();
	if (chanWidthArray.size() != (unsigned int) r->getNumChan()) {
	  errstream.str("");
	  errstream << "Size of chanWidthArray ('"
		    << chanWidthArray.size()
		    << "') is not equal to numChan ('"
		    << r->getNumChan()
		    << "'). Can't go further.";
	  error(errstream.str());
	}
      }
      else { // Frequency channels widths are specified by a constant value.
	chanWidthArray.resize(r->getNumChan());
	chanWidthArray.assign(chanWidthArray.size(), r->getChanWidth());
      }
      chanWidth1D = chanWidthConverter.to1DArray(chanWidthArray);
      
      
      /* Processing the effectiveBwXXX
       *
       * Either effectiveBw or effectiveBwArray must be present
       * with a priority to effectiveBwArray if both are present. If effectiveBwArray
       * is present it *must* have a size equal to numChan otherwiser EXIT.
       */
      if (!r->isEffectiveBwExists() && !r->isEffectiveBwArrayExists()) {
	errstream.str("");
	errstream << "Did not find effectiveBw nor effectiveBwArray. Can't go further";
	error(errstream.str());
      }
      
      double* effectiveBw1D = (double *) 0;
      DConverter effectiveBwConverter;
      vector<Frequency> effectiveBwArray;
      if (r->isEffectiveBwArrayExists()) { // Effective BWs are specified by an array.
	effectiveBwArray = r->getEffectiveBwArray();
	if (effectiveBwArray.size() != (unsigned int) r->getNumChan()) {
	  errstream.str("");
	  errstream << "Size of effectiveBwArray ('"
		    << effectiveBwArray.size()
		    << "') is not equal to numChan ('"
		    << r->getNumChan()
		    << "'). Can't go further." ;
	  error(errstream.str());
	}
      }
      else { // Effective BWs are specified by a constant value.
	effectiveBwArray.resize(r->getNumChan());
	effectiveBwArray.assign(effectiveBwArray.size(), r->getEffectiveBw());
      }
      effectiveBw1D = effectiveBwConverter.to1DArray(effectiveBwArray);
      
      
      /* Processing the resolutionXXX
       *
       * Either resolution or resolutionArray must be present
       * with a priority to resolutionArray if both are present. If resolutionArray
       * is present it *must* have a size equal to numChan otherwiser EXIT.
       */
      if (!r->isResolutionExists() && !r->isResolutionArrayExists()) {
	errstream.str("");
	errstream << "Did not find resolution nor resolutionArray. Can't go further";
	error(errstream.str());
      }
      
      double* resolution1D = (double *) 0;
      DConverter resolutionConverter;
      vector<Frequency> resolutionArray;
      if (r->isResolutionArrayExists()) { // Resolutions are specified by an array.
	resolutionArray = r->getResolutionArray();
	if (resolutionArray.size() != (unsigned int) r->getNumChan()) {
	  errstream.str("");
	  errstream << "Size of resolutionArray ('"
		    << resolutionArray.size()
		    << "') is not equal to numChan ('"
		    << r->getNumChan()
		    << "'). Can't go further.";
	  error(errstream.str());
	}
      }
      else { // Resolutions are specified by a constant value.
	resolutionArray.resize(r->getNumChan());
	resolutionArray.assign(resolutionArray.size(), r->getResolution());
      }
      resolution1D = resolutionConverter.to1DArray(resolutionArray);
      
      int bbcNo = -1;
      bbcNo = r->getBasebandName();
      
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	iter->second->addSpectralWindow(r->getNumChan(),
					r->isNameExists()?r->getName().c_str():"",
					(r->getRefFreq()).get(),
					chanFreq1D,  
					chanWidth1D, 
					r->isMeasFreqRefExists()?FrequencyReferenceMapper::value(r->getMeasFreqRef()):MFrequency::TOPO,
					effectiveBw1D, 
					resolution1D,
					(r->getTotBandwidth()).get(),
					r->getNetSideband(),
					r->getBasebandName(),
					0,  // IFConvChain set to 0.
					r->isFreqGroupExists()?r->getFreqGroup():0,
					r->isFreqGroupNameExists()?r->getFreqGroupName().c_str():"",
					numAssocValues,
					assocSpectralWindowId_,
					assocNature_);      
      }      
    }
    if (nSpectralWindow) {
      infostream.str("");
      infostream << "converted in " << msFillers.begin()->second->ms()->spectralWindow().nrow() << " spectral window(s) in the measurement set(s).";
      info(infostream.str());
    }	
    
  }
  catch (IllegalAccessException& e) {
    errstream.str("");
    errstream << e.getMessage();
    error(errstream.str());
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
    infostream.str("");
    infostream << "The dataset has " << nPolarization << " polarization(s)..."; 
    info(infostream.str());

    for (int i = 0; i < nPolarization; i++) {
      if ((r=polT.getRowByKey(Tag(i, TagType::Polarization))) == 0) {
	errstream.str("");
	(errstream << "Problem while reading the Polarization table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(errstream.str());
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


      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	polarizationMap.push_back(iter->second->addUniquePolarization(numCorr,
								      corrType,
								      corrProduct
								      ));
      }

      if (nPolarization) {
	infostream.str("");
	infostream << "converted in " << msFillers.begin()->second->ms()->polarization().nrow() << " polarization(s)." ;
	info(infostream.str());
      }
    }
  }
  catch (ASDM2MSException e) {
    errstream.str("");
    errstream << e.getMessage();
    error(errstream.str());
  }
  
    
  //
  // Load the DataDescription table.
  vector<int> dataDescriptionMap;
  DataDescriptionTable& ddT = ds->getDataDescription();
  {
    DataDescriptionRow* r = 0;
    int nDataDescription = ddT.size();
    infostream.str("");
    infostream << "The dataset has " << nDataDescription << " data description(s)...";
    info(infostream.str());

    for (int i = 0; i < nDataDescription; i++) {
      if ((r=ddT.getRowByKey(Tag(i, TagType::DataDescription))) == 0) {
	errstream.str("");
	(errstream << "Problem while reading the DataDescription table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(errstream.str());
      }
      
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	dataDescriptionMap.push_back(msFiller->addUniqueDataDescription(swIdx2Idx[r->getSpectralWindowId().getTagValue()],
									polarizationMap.at(r->getPolOrHoloId().getTagValue())));
      }
    }
    if (nDataDescription) {
      infostream.str("");
      infostream << "converted in " << msFillers.begin()->second->ms()->dataDescription().nrow() << " data description(s)  in the measurement set(s)." ;
      info(infostream.str());
    }
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
    infostream.str("");
    infostream << "The dataset has " << nFeed << " feed(s)...";
    info(infostream.str());

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
      if (r->isPositionExists()) {
	vector<Length> position = r->getPosition();
	if (position.size() != 3) {
	  errstream.str("");
	  errstream << "The size of attribute position ('" 
		    << position.size()
		    << "') is not equal to 3. Can't go further."
		    << endl;
	  error(errstream.str());
	}
	xyzPosition = xyzPositionConverter.to1DArray(position.at(0),
						     position.at(1), 
						     position.at(2));
      }
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	iter->second->addFeed(r->getAntennaId().getTagValue(),
			      r->getFeedId(),
			      swIdx2Idx[r->getSpectralWindowId().getTagValue()],
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
    }
    if (nFeed) {
      infostream.str("");
      infostream <<  "converted in " << msFillers.begin()->second->ms()->feed().nrow() << " feed(s) in the measurement set." ;
      info(infostream.str());
    }
  }
  
  // Load the Field table.
  // Issues :
  // - only processes the case with numPoly == 0 at the moment.

  FieldTable& fieldT = ds->getField();
  
  try {
    FieldRow* r = 0;
    int nField = fieldT.size();
    infostream.str("");
    infostream << "The dataset has " << nField << " field(s)...";
    info(infostream.str());

    double delayDir[2];
    double phaseDir[2];
    double referenceDir[2];

    for (int i = 0; i < nField; i++) {
      if ((r=fieldT.getRowByKey(Tag(i, TagType::Field))) == 0) {
	errstream.str("");
	(errstream << "Problem while reading the Field table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(errstream.str());
      }

      delayDir[0] = DConverter().to1DArray(r->getDelayDir())[0];
      delayDir[1] = DConverter().to1DArray(r->getDelayDir())[1];
      
      phaseDir[0] = DConverter().to1DArray(r->getPhaseDir())[0];
      phaseDir[1] = DConverter().to1DArray(r->getPhaseDir())[1];
      
      referenceDir[0] = DConverter().to1DArray(r->getReferenceDir())[0];
      referenceDir[1] = DConverter().to1DArray(r->getReferenceDir())[1];

      int sourceId = -1;
      if (r->isSourceIdExists()) sourceId = r->getSourceId();

      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	iter->second->addField( (const char*)r->getFieldName().c_str(),
				(const char*)r->getCode().c_str(),
				r->isTimeExists() ? ((double) r->getTime().get()) / ArrayTime::unitsInASecond : 0.0,
				delayDir,
				phaseDir,
				referenceDir,
				r->isSourceIdExists()?r->getSourceId():0);
      }
    }  
    if (nField) {
      infostream.str("");
      infostream << "converted in " << msFillers.begin()->second->ms()->field().nrow() << "  field(s) in the measurement set(s)." ;
      info(infostream.str());
    }
  }
  catch (IllegalAccessException& e) {
    cout << e.getMessage();
  }


  // Load the FlagCmd table
  FlagCmdTable& flagCmdT  = ds->getFlagCmd();


  {
    FlagCmdRow* r = 0;
    int nFlagCmd = flagCmdT.size();
    
    infostream.str("");
    infostream << "The dataset has " << nFlagCmd << " flag(s)...";
    info(infostream.str());

    vector<FlagCmdRow *> v = flagCmdT.get();
    for (int i = 0; i < nFlagCmd; i++) {
      r = v.at(i);
      // For now we just adapt the types of the time related informations and compute a mid-time.
      //
      double interval = ((double) r->getTimeInterval().getDuration().get()) / ArrayTime::unitsInASecond ;
      double time =  ((double) r->getTimeInterval().getStart().get()) / ArrayTime::unitsInASecond + interval/2.0;
      
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	iter->second->addFlagCmd(time,
				 interval,
				 r->getType().c_str(),
				 r->getReason().c_str(),
				 r->getLevel(),
				 r->getSeverity(),
				 r->getApplied() ? 1 : 0,
				 r->getCommand().c_str());
      }
    }
    if (nFlagCmd) {
      infostream.str("");
      infostream << "converted in " << msFillers.begin()->second->ms()->flagCmd().nrow() << " in the measurement set." ;
      info(infostream.str());
    }
  }

  // Load the History table
  // Issues :
  // - use executeBlockId for observationId ...to be discussed with Francois.
  // - objectId : not taken into account (it's a string while the MS expects an int).
  HistoryTable& historyT = ds->getHistory();
  {
    HistoryRow* r = 0;
    vector<HistoryRow *> v = historyT.get();
    int nHistory = historyT.size();
    infostream.str("");
    infostream << "The dataset has " << nHistory << " history(s)...";
    info(infostream.str());

    for (int i = 0; i < nHistory; i++) {
      r = v.at(i);
      double time =  ((double) r->getTime().get()) / ArrayTime::unitsInASecond ;
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	iter->second->addHistory(time,
				 r->getExecBlockId().getTagValue(),   
				 r->getMessage().c_str(),
				 r->getPriority().c_str(),
				 r->getOrigin().c_str(),
				 -1,
				 r->getApplication().c_str(),
				 r->getCliCommand().c_str(),
				 r->getAppParms().c_str());
      }
    }
    if (nHistory) {
      infostream.str("");
      infostream << "converted in " << msFillers.begin()->second->ms()->history().nrow() << " history(s) in the measurement set(s)." ;
      info(infostream.str());
    }
  }

  // Build the MS Observation table with the content of ASDM ExecBlock table.
  // 
  ExecBlockTable& execBlockT = ds->getExecBlock(); 
  {
    ExecBlockRow* r = 0;
    int nExecBlock = execBlockT.size();
    infostream.str("");
    infostream << "The dataset has " << nExecBlock << " execBlock(s)...";
    info(infostream.str());

    vector<ExecBlockRow *> v = execBlockT.get();
    
    for (int i = 0; i < nExecBlock; i++) {
      r = v.at(i);
      
      double startTime      = r->getStartTime().getMJD()*86400;
      double endTime        = r->getEndTime().getMJD()*86400;

      vector<string> observingLog;
      observingLog.push_back(r->getObservingLog());
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	iter->second->addObservation((const char*) r->getTelescopeName().c_str(),
				     startTime,
				     endTime,
				     (const char*) r->getObserverName().c_str(),
				     (const char**) SConverter().to1DCStringArray(observingLog),
				     (const char *) "ALMA",
				     (const char**) 0,
				     (const char*) "T.B.D.",
				     r->isReleaseDateExists() ? r->getReleaseDate().getMJD():0.0
				     );
      }
    } 
    if (nExecBlock) {
      infostream.str("");
      infostream << "converted in " << msFillers.begin()->second->ms()->observation().nrow() << " observation(s) in the measurement set(s)." ;
      info(infostream.str());
    }
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
  infostream.str("");
  infostream << "The dataset has " << nPointing << " pointing(s)...";
  info(infostream.str());

  if (nPointing > 0) {

    // Check some assertions.
    //
    // All rows of ASDM-Pointing must have their attribute usePolynomials equal to false
    // and their numTerm attribute equal to 1. Use the opportunity of this check
    // to compute the number of rows to be created in the MS-Pointing by summing
    // all the numSample attributes values.
    //
    int numMSPointingRows = 0;
    vector<PointingRow *> v = pointingT.get();
    for (unsigned int i = 0; i < v.size(); i++) {
      if (v[i]->getUsePolynomials()) {
	errstream.str("");
	errstream << "Found usePolynomials equal to true at row #" << i <<". Can't go further"<<endl;
	error(errstream.str());
      }

      if (v[i]->getNumTerm() != 1) {
	errstream.str("");
	errstream << "Found numTerm equal to " << v[i]->getNumTerm() <<" at row #" << i <<" and I was expecting 1. Can't go further" << endl;
	error(errstream.str());
      }

      numMSPointingRows += v[i]->getNumSample();
    }

    //
    // Ok now we have verified the assertions and we know the number of rows
    // to be created into the MS-Pointing, we can proceed.

    PointingRow* r = 0;
    int    *antenna_id_      = new int[numMSPointingRows];
    double *time_            = new double[numMSPointingRows];
    double *interval_        = new double[numMSPointingRows];
    double *direction_       = new double[2 * numMSPointingRows];
    double *target_          = new double[2 * numMSPointingRows];
    double *pointing_offset_ = new double[2 * numMSPointingRows];
    double *encoder_         = new double[2 * numMSPointingRows];
    bool   *tracking_        = new bool[numMSPointingRows];

    int iMSPointingRow = 0;
    for (int i = 0; i < nPointing; i++) {     // Each row in the ASDM-Pointing ...
      r = v.at(i);

      // Let's prepare some values.
      int antennaId = r->getAntennaId().getTagValue();
      
      double time = 0.0, interval = 0.0;
      if (!r->isSampledTimeIntervalExists()) { // If no sampledTimeInterval then
	                                       // then compute the first value of MS TIME and INTERVAL.
	interval   = ((double) r->getTimeInterval().getDuration().get()) / ArrayTime::unitsInASecond / r->getNumSample();
	time = ((double) r->getTimeInterval().getStart().get()) / ArrayTime::unitsInASecond + interval / 2.0;
      }

      //
      // The size of each vector below 
      // should be checked against numSample !!!
      //
      int numSample = r->getNumSample();
      const vector<vector<Angle> > encoder = r->getEncoder();
      checkVectorSize<vector<Angle> >("encoder", encoder, "numSample", (unsigned int) numSample, "Pointing", (unsigned int)i);

      const vector<vector<Angle> > pointingDirection = r->getPointingDirection();
      checkVectorSize<vector<Angle> >("pointingDirection", pointingDirection, "numSample", (unsigned int) numSample, "Pointing", (unsigned int) i);

      const vector<vector<Angle> > target = r->getTarget();
      checkVectorSize<vector<Angle> >("target", target, "numSample", (unsigned int) numSample, "Pointing", (unsigned int) i);

      const vector<vector<Angle> > offset = r->getOffset();
      checkVectorSize<vector<Angle> >("offset", offset, "numSample", (unsigned int) numSample, "Pointing", (unsigned int) i);

      bool   pointingTracking = r->getPointingTracking();
 
      //
      // Prepare some data structures and values required to compute the
      // (MS) direction.
      vector<double> cartesian1(3, 0.0);
      vector<double> cartesian2(3, 0.0);
      vector<double> spherical1(2, 0.0);
      vector<double> spherical2(2, 0.0);
      vector<vector<double> > matrix3x3;
      for (unsigned int i = 0; i < 3; i++) {
	matrix3x3.push_back(cartesian1); // cartesian1 is used here just as a way to get a 1D vector of size 3.
      }
      double PSI = M_PI_2;
      double THETA;
      double PHI;
      
      for (int j = 0 ; j < r->getNumSample(); j++) { // ... must be expanded in numSample MS-Pointing rows.
	// ANTENNA_ID
	antenna_id_[iMSPointingRow]          = antennaId;

	// TIME and INTERVAL
	if (r->isSampledTimeIntervalExists()) { //if sampledTimeInterval is present use its values.	                                         
	  vector<ArrayTimeInterval> timeInterval = r->getSampledTimeInterval();
	  // Here the size of timeInterval will have to be checked against numSample !!
	  interval_[iMSPointingRow] = ((double) timeInterval.at(j).getDuration().get()) / ArrayTime::unitsInASecond ;
	  time_[iMSPointingRow] = ((double) timeInterval.at(j).getStart().get()) / ArrayTime::unitsInASecond
	    + interval_[iMSPointingRow]/2;	  
	}
	else {                                     // otherwise compute TIMEs and INTERVALs from the first values.
	  interval_[iMSPointingRow]            = interval;
	  time_[iMSPointingRow]                = time + j*interval;
	} 

	// DIRECTION
#if 0
	direction_[2*iMSPointingRow]  = pointingDirection.at(j).at(0).get();
	direction_[2*iMSPointingRow+1]= pointingDirection.at(j).at(1).get();
#else
	THETA = target.at(j).at(1).get();
	PHI   = -M_PI_2 - target.at(j).at(0).get();
	spherical1[0] = offset.at(j).at(0).get();
	spherical1[1] = offset.at(j).at(1).get();
	rect(spherical1, cartesian1);
	eulmat(PSI, THETA, PHI, matrix3x3);
	matvec(matrix3x3, cartesian1, cartesian2);
	spher(cartesian2, spherical2);
	direction_[2*iMSPointingRow]  = spherical2[0];
	direction_[2*iMSPointingRow+1]= spherical2[1];
#endif

	// TARGET
	target_[2*iMSPointingRow]     = target.at(j).at(0).get();
	target_[2*iMSPointingRow+1]   = target.at(j).at(1).get();

	// POINTING_OFFSET
	pointing_offset_[2*iMSPointingRow]   = offset.at(j).at(0).get();
	pointing_offset_[2*iMSPointingRow+1] = offset.at(j).at(1).get();

	// ENCODER
	encoder_[2*iMSPointingRow]           = encoder.at(j).at(0).get();
	encoder_[2*iMSPointingRow+1]         = encoder.at(j).at(1).get();

	// TRACKING
	tracking_[iMSPointingRow] = pointingTracking;

	// increment the row number in MS Pointing.
	iMSPointingRow++;
      }
    }
   
    for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	 iter != msFillers.end();
	 ++iter) {
      iter->second->addPointingSlice(numMSPointingRows,
				     antenna_id_,
				     time_,
				     interval_,
				     direction_,
				     target_,
				     pointing_offset_,
				     encoder_,
				     tracking_);
    }
			       
    delete[] antenna_id_;
    delete[] time_;
    delete[] interval_;
    delete[] direction_;
    delete[] target_;
    delete[] pointing_offset_;
    delete[] encoder_;
    delete[] tracking_;
  }

  if (nPointing) {
    infostream.str("");
    infostream << "converted in " << msFillers.begin()->second->ms()->pointing().nrow() << " pointing(s) in the measurement set." ;
    info(infostream.str()); 
  }
#endif

  // Load the processor table
  //
  ProcessorTable& processorT = ds->getProcessor();
  {
    ProcessorRow* r = 0;
    int nProcessor = processorT.size();

    infostream.str("");
    infostream << "The dataset has " << nProcessor << " processor(s)...";
    info(infostream.str());
    
    for (int i = 0; i < nProcessor; i++) {
      if ((r=processorT.getRowByKey(Tag(i, TagType::Processor))) == 0) {
	errstream.str("");
	(errstream << "Problem while reading the Processor table, the row with key = Tag(" << i << ") does not exist.Aborting." << endl);
	error(errstream.str());
      }

      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	iter->second->addProcessor((CProcessorType::name(r->getProcessorType())).c_str(),
				   (CProcessorSubType::name(r->getProcessorSubType())).c_str(),
				   -1,    // Since there is no typeId in the ASDM.
				   r->getModeId().getTagValue());
      }
    }
    if (nProcessor) {
      infostream.str("");
      infostream << "converted in " << msFillers.begin()->second->ms()->processor().nrow() << " processor(s) in the measurement set." ;
      info(infostream.str());
    } 
  }
  
  // Load the source table
  //
  SourceTable& sourceT = ds->getSource();
  try {
    SourceRow* r = 0;
    int nSource = sourceT.size();
    infostream.str("");
    infostream << "The dataset has " << nSource << " source(s)...";
    info(infostream.str());

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
      
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	iter->second->addSource(r->getSourceId(),
				time,
				interval,
				swIdx2Idx[r->getSpectralWindowId().getTagValue()],
				r->isNumLinesExists() ? r->getNumLines() : 0,
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
    }
    if (nSource) {
      infostream.str("");
      infostream << "converted in " << msFillers.begin()->second->ms()->source().nrow() <<" source(s) in the measurement set." ;
      info(infostream.str());
    }
  }  
  catch (IllegalAccessException& e) {
    errstream.str("");
    error(errstream.str());
  }

#if 1
  //
  // Load the weather table
  WeatherTable& weatherT = ds->getWeather();

  {
    WeatherRow* r = 0;
    
    vector<WeatherRow*> v = weatherT.get();

    int nWeather = weatherT.size();
    infostream.str("");
    infostream << "The dataset has " << nWeather << " weather(s)...";
    info(infostream.str());

    for (int i = 0; i < nWeather; i++) {
      r = v.at(i);      
      double interval = ((double) r->getTimeInterval().getDuration().get()) / ArrayTime::unitsInASecond ;
      double time =  ((double) r->getTimeInterval().getStart().get()) + interval / 2.0 ;
        
      DConverter position2DArray;
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	iter->second->addWeather(-1,
				 time,
				 interval,
				 r->getPressure().get(),
				 r->getPressureFlag(),
				 r->getRelHumidity().get(),
				 r->getRelHumidityFlag(),
				 r->getTemperature().get(),
				 r->getTemperatureFlag(),
				 r->getWindDirection().get(),
				 r->getWindDirectionFlag(),
				 r->getWindSpeed().get(),
				 r->getWindSpeedFlag(),
				 r->isDewPointExists(),
				 r->isDewPointExists()?r->getDewPoint().get():-1.0,
				 r->isDewPointFlagExists()?r->getDewPointFlag():true,
				 r->getStationId().getTagValue(),
				 position2DArray.to1DArray(r->getStationUsingStationId()->getPosition()));
      }
    }
    if (nWeather) {
      infostream.str("");
      infostream << "converted in " << msFillers.begin()->second->ms()->weather().nrow() <<" weather(s) in the measurement set." ;
      info(infostream.str());
    }
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
    const VMSData *vmsDataPtr = 0;
    
    try {
      // Prepare an UVW coordinate engine
      UvwCoords uvwCoords(ds);
      
      for (int i = 0; i < nMain; i++) {
	mode = 0; myTimer(&cpu_time_asdm, &real_time_asdm, &mode);
	r = v.at(i);

	mode = 1; myTimer(&cpu_time_asdm, &real_time_asdm, &mode);
	cpu_time_asdm_overall  += cpu_time_asdm;
	real_time_asdm_overall += real_time_asdm;
	
	if(sdmBinData.acceptMainRow(r)){
	  mode = 0; myTimer(&cpu_time_asdm, &real_time_asdm, &mode);
	  vmsDataPtr = sdmBinData.getDataCols();

	  mode = 1; myTimer(&cpu_time_asdm, &real_time_asdm, &mode);
	  cpu_time_asdm_overall  += cpu_time_asdm;
	  real_time_asdm_overall += real_time_asdm;
	  

	  // Firstly populate the State table...
	  mode = 0; myTimer(&cpu_time_aips, &real_time_aips, &mode);
	  ScanRow* scanR = ds->getScan().getRowByKey(r->getExecBlockId(),
						     r->getScanNumber());
	  vector<ScanIntent> scanIntent = scanR->getScanIntent();
	  
	  vector<int> msStateId;
	  for (unsigned int iState = 0; iState < vmsDataPtr->v_msState.size(); iState++) {
							      
	    const sdmbin::MSState& msState = vmsDataPtr->v_msState.at(iState);
	    SubscanRow* sscanR = ds->getSubscan().getRowByKey(r->getExecBlockId(),
							      r->getScanNumber(),
							      msState.subscanNum);
	    SubscanIntent subscanIntent = sscanR->getSubscanIntent();
	    string obs_mode;
	    if (scanIntent.size() > 0) {
	      obs_mode = CScanIntent::name(scanIntent.at(0))+"."+CSubscanIntent::name(subscanIntent);

	      for (unsigned int iScanIntent = 1; iScanIntent < scanIntent.size(); iScanIntent++) {
		obs_mode += ",";
		obs_mode +=  CScanIntent::name(scanIntent.at(iScanIntent))+"."+CSubscanIntent::name(subscanIntent);
	      }
	    }

	    bool pushed = false;
	    for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
		 iter != msFillers.end();
		 ++iter) {
	      int retId = iter->second->addUniqueState(msState.sig,
						       msState.ref,
						       msState.cal,
						       msState.load,
						       msState.subscanNum,
						       obs_mode.c_str(), //msState.obsMode.c_str(),
						       false);
	      if (!pushed) {
		msStateId.push_back(retId);
		pushed = true;
	      }
	    }
	  }
	  mode = 1; myTimer(&cpu_time_aips, &real_time_aips, &mode);
	  cpu_time_aips_overall  += cpu_time_aips;
	  real_time_aips_overall += real_time_aips;
	  
	  mode = 0; myTimer(&cpu_time_aips, &real_time_aips, &mode);
	  
	  // Then populate the Main table.
	  ComplexDataFilter filter; // To process the case numCorr == 3
	  
	  if (vmsDataPtr->v_antennaId1.size() == 0) {
	    infostream.str("");
	    infostream << "No MS data produced for ASDM table row #" << i+1 << endl;
	    info(infostream.str());
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

	  /* compute the UVW */

	  vector<double> uvw(3*vmsDataPtr->v_time.size());
	  // cout << "3 * v_time.size = " << 3*vmsDataPtr->v_time.size() << endl;
	  
	  vector<casa::Vector<casa::Double> > vv_uvw;
#if DDPRIORITY
	  // cout << "About to compute UVW with DD priority" << endl;
	  uvwCoords.uvw_bl(r, sdmBinData.timeSequence(), e_query_cm, 
			   sdmbin::SDMBinData::dataOrder(),
			   vv_uvw);
	  // cout << "computed UVW with DD priority" << endl;
#else
	  // cout << "About to compute UVW without DD priority" << endl;
	  uvwCoords.uvw_bl(r, vmsDataPtr->v_timeCentroid, e_query_cm, 
			   sdmbin::SDMBinData::dataOrder(),
			   vv_uvw);
	  // cout << "computed UVW without DD priority" << endl;
#endif
	  // cout << "vv_uvw size = " << vv_uvw.size() << endl;

	  // cout << "About to linearize uvw " << endl;
	  int k = 0;
	  for (unsigned int iUvw = 0; iUvw < vv_uvw.size(); iUvw++) {
	    uvw[k++] = vv_uvw[iUvw](0); 
	    uvw[k++] = vv_uvw[iUvw](1);
	    uvw[k++] = vv_uvw[iUvw](2);
	  } 

	  ComplexDataFilter cdf;
	  map<AtmPhaseCorrectionMod::AtmPhaseCorrection, float*>::const_iterator iter;

	  vector<double>	correctedTime;
	  vector<int>		correctedAntennaId1;
	  vector<int>		correctedAntennaId2;
	  vector<int>		correctedFeedId1;
	  vector<int>		correctedFeedId2;
	  vector<int>		correctedFieldId;
	  vector<double>	correctedInterval;
	  vector<double>	correctedExposure;
	  vector<double>	correctedTimeCentroid;
	  vector<int>		correctedMsStateId;
	  vector<double>	correctedUvw ;
	  vector<unsigned int>	correctedFlag;

	  for (unsigned int iData = 0; iData < vmsDataPtr->v_m_data.size(); iData++) {

	    if ((msFillers.find(AP_UNCORRECTED) != msFillers.end()) &&
		(iter=vmsDataPtr->v_m_data.at(iData).find(AtmPhaseCorrectionMod::AP_UNCORRECTED)) != vmsDataPtr->v_m_data.at(iData).end()){
	      uncorrectedData.push_back(cdf.to4Pol(vmsDataPtr->vv_dataShape.at(iData).at(0),
						   vmsDataPtr->vv_dataShape.at(iData).at(1),
						   iter->second));
	    }
	    
	    if ((msFillers.find(AP_CORRECTED) != msFillers.end()) &&
		(iter=vmsDataPtr->v_m_data.at(iData).find(AtmPhaseCorrectionMod::AP_CORRECTED)) != vmsDataPtr->v_m_data.at(iData).end()){
	      correctedTime.push_back(vmsDataPtr->v_time.at(iData));
	      correctedAntennaId1.push_back(vmsDataPtr->v_antennaId1.at(iData));
	      correctedAntennaId2.push_back(vmsDataPtr->v_antennaId2.at(iData));
	      correctedFeedId1.push_back(vmsDataPtr->v_feedId1.at(iData));
	      correctedFeedId2.push_back(vmsDataPtr->v_feedId2.at(iData));
	      correctedFieldId.push_back(vmsDataPtr->v_fieldId.at(iData));
	      correctedInterval.push_back(vmsDataPtr->v_interval.at(iData));
	      correctedExposure.push_back(vmsDataPtr->v_exposure.at(iData));
	      correctedTimeCentroid.push_back(vmsDataPtr->v_timeCentroid.at(iData));
	      correctedMsStateId.push_back(msStateId.at(iData));
	      correctedUvw.push_back(vv_uvw.at(iData)(0));
	      correctedUvw.push_back(vv_uvw.at(iData)(1));
	      correctedUvw.push_back(vv_uvw.at(iData)(2));
	      correctedData.push_back(cdf.to4Pol(vmsDataPtr->vv_dataShape.at(iData).at(0),
						 vmsDataPtr->vv_dataShape.at(iData).at(1),
						 iter->second));
	      correctedFlag.push_back(vmsDataPtr->v_flag.at(iData));
	    }
	  }
	  
	  if (uncorrectedData.size() > 0 && (msFillers.find(AP_UNCORRECTED) != msFillers.end())) {
	    infostream.str("");
	    infostream << "ASDM Main table row #" << i+1
		       << " will be transformed into " << vmsDataPtr->v_antennaId1.size()
		       << " rows in the wvr uncorrected MS Main table." << endl;
	    info(infostream.str());
	    msFillers[AP_UNCORRECTED]->addData(complexData,
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
					       (vector<unsigned int>&)vmsDataPtr->v_flag);
	    mode = 1; myTimer(&cpu_time_aips, &real_time_aips, &mode);
	    cpu_time_aips_overall  += cpu_time_aips;
	    real_time_aips_overall += real_time_aips;
	  }

	  if (correctedData.size() > 0 && (msFillers.find(AP_CORRECTED) != msFillers.end())) {
	    infostream.str("");
	    infostream << "ASDM Main table row #" << i+1
		       << " will be transformed into " << correctedAntennaId1.size()
		       << " rows in the wvr corrected MS Main table." << endl;
	    info(infostream.str());

	    msFillers[AP_CORRECTED]->addData(complexData,
					     correctedTime, // this is already time midpoint
					     correctedAntennaId1, 
					     correctedAntennaId2,
					     correctedFeedId1,
					     correctedFeedId2,
					     filteredDD,
					     vmsDataPtr->processorId,
					     correctedFieldId,
					     correctedInterval,
					     correctedExposure,
					     correctedTimeCentroid,
					     (int) r->getScanNumber(), 
					     0,                                               // Array Id
					     (int) r->getExecBlockId().getTagValue(), // Observation Id
					     correctedMsStateId,
					     correctedUvw,
					     filteredShape, // vmsDataPtr->vv_dataShape after filtering the case numCorr == 3
					     correctedData,
					     correctedFlag);
	    mode = 1; myTimer(&cpu_time_aips, &real_time_aips, &mode);
	    cpu_time_aips_overall  += cpu_time_aips;
	    real_time_aips_overall += real_time_aips;
	  }
	}else{
	  infostream.str("");
	  infostream <<"No data retrieved in the SDM row #" << i+1 << " (" << sdmBinData.reasonToReject(r) <<")" << endl;
	  info(infostream.str());
	}      
      }
    }
    catch ( IllegalAccessException& e) {
      errstream.str("");
      errstream << e.getMessage();
      error(errstream.str());
    }
    catch ( SDMDataObjectReaderException& e ) {
      errstream.str("");
      errstream << e.getMessage();
      error(errstream.str());
    }
    catch ( std::exception & e) {
      errstream.str("");
      errstream << e.what();
      error(errstream.str());      
    }
    catch (Error & e) {
      errstream.str("");
      errstream << e.getErrorMessage();
      error(errstream.str());
    }

    infostream.str("");
    infostream << "The dataset has "  << stateT.size() << " state(s)..." ;
    info(infostream.str());

    if (stateT.size()) {
      infostream.str("");
      infostream << "converted in " << msFiller->ms()->state().nrow() << " state(s) in the measurement set.";
      info(infostream.str());
    }


    infostream.str("");
    infostream << "The dataset has " << mainT.size() << " main(s)...";
    info(infostream.str());


    if (mainT.size()) {
      for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
	   iter != msFillers.end();
	   ++iter) {
	string kindOfData = (iter->first == AP_UNCORRECTED) ? "wvr uncorrected" : "wvr corrected";
	infostream.str("");
	infostream << "converted in " << iter->second->ms()->nrow() << " main(s) rows in the measurement set containing the " << kindOfData << " data.";
	info(infostream.str());
      }
    }
  }
#endif
  
  // Do we also want to store the verbatim copies of some tables of the ASDM dataset ?
  if (vm.count("asis")) {
    istringstream iss;
    iss.str(vm["asis"].as<string>());
    string word;
    vector<string> tablenames;
    while (iss>>word)
      tablenames.push_back(word);
    
    ASDMVerbatimFiller avf(const_cast<casa::MS*>(msFillers.begin()->second->ms()), Name2Table::find(tablenames, verbose));
    avf.fill(*ds);
  }
  
  for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
       iter != msFillers.end();
       ++iter)
    iter->second->end(0.0);
  
  infostream.str("");
  infostream << "Overall cpu time spent in ASDM methods to read/process the ASDM Main table : cpu = " << cpu_time_asdm_overall  << " s." << endl;
  info(infostream.str());
  
  infostream.str("");
  infostream << "Overall cpu time spent in AIPS methods to fill the MS Main table : cpu = " << cpu_time_aips_overall  << " s." << endl;
  info(infostream.str());
  
  for (map<AtmPhaseCorrection, ASDM2MSFiller*>::iterator iter = msFillers.begin();
       iter != msFillers.end();
       ++iter)
    delete iter->second;
  delete ds;
  return 0;
}
