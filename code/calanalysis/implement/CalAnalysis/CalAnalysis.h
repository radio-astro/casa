
// -----------------------------------------------------------------------------

/*

CalAnalysis.h

Description:
------------
This header file contains definitions for the CalStats class.

Classes:
--------
CalAnalysis - This class acts as the interface between the ROCTIter and CalStats
              classes.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Logging capability added.  Error checking added.
2012 Feb 14 - Nick Elias, NRAO
              Updated this code to reflect changes in NewCalTabIter (now
              ROCTIter) and other classes.

*/

// -----------------------------------------------------------------------------
// Start of define macro to prevent multiple loading
// -----------------------------------------------------------------------------

#ifndef CAL_ANALYSIS_H
#define CAL_ANALYSIS_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <casa/BasicSL/String.h>

#include <iostream>
#include <sstream>
using namespace std;

#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>

#include <casa/Containers/Block.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayMath.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableProxy.h>

#include <casa/Containers/Record.h>
#include <casa/Containers/RecordField.h>

#include <casa/Utilities/GenSort.h>

#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTIter.h>

#include <calanalysis/CalAnalysis/CalStats.h>
#include <calanalysis/CalAnalysis/CalStatsDerived.h>
#include <calanalysis/CalAnalysis/CalStatsFitter.h>

// -----------------------------------------------------------------------------
// Start of casa namespace
// -----------------------------------------------------------------------------

namespace casa {

// -----------------------------------------------------------------------------
// Start of CalAnalysis class definition
// -----------------------------------------------------------------------------

/*

CalAnalysis

Description:
------------
This class acts as the interface between the ROCTIter and CalAnalysis classes.

NB: I may replace msName(), parType(), polBasis(), and visCal() with a single
function whose second argument is the keyword name.

In a nutshell:
--------------
* The constructor gets the information from the new format calibration table.
* The stats<T>() function calculates statistics (the type depends on T) and
  returns the results.
  - The user can specify the field(s), antenna(s), time range, feeds, spectral
    windows, frequencies, and the second iteration axis for the CalStats class
    (time or frequency).
  - The inputs are checked and fixed, if possible.
  - The iteration loop goes over field, antenna1, and antenna2.  If a set of
    field, antenna1 and antenna2 numbers from an iteration is consistent with
    the inputs, statistics are calculated.  This is not the most efficient way,
    but the NewCalTable class doesn't have another way to access data and the
    time for each iteration is very fast.
  - For each iteration, the dimensions of the value, value error, and flag cubes
    provided by ROCTIter are feed x frequency(spw) x row(spw,time).  This shape
    is not useful for calculating statistics with CalStats, so the parse<T>()
    function slices and dices the cubes into feed x frequency x time.
  - The parsed cubes are further refined by the select<T>() function to include
    only the feeds, frequencies, and times selected by the input parameters.
  - The resulting cubes are fed to the CalStats class and its stats<T>()
    function calculates the desired statistics which are stored in a vector
    of OUTPUT<T> instances.

Nested classes:
---------------
OUTPUT<T> - This nested class contains the outputs for the
            CalAnalysis::stats<T>() template member function.

Class public member functions:
------------------------------
CalAnalysis  - This constructor gets information from the new format calibration
               table for further processing by the stats<T>() function.
~CalAnalysis - This destructor deallocates the internal memory of an instance.
msName       - This function returns the associated MS name.
parType      - This function returns the parameter type (Complex or Float).
polBasis     - This function returns the polarization basis (linear or
               circular).
visCal       - This function returns the visibility calibration type.

Class template public stats member functions:
---------------------------------------------
stats<T> - This member function is the main user interface for calculating the
           statistics for all iterations.  Allowed T: CalStats::NONE only
           returns the input data, CalStatsFitter::FIT calculates fit
           statistics, and CalStatsHist::HIST calculates histogram statistics.

Class template public static member functions:
----------------------------------------------
exists<T>       - This member function determines whether a value appears in a
                  vector.
uniqueNoSort<T> - This member function returns an unsorted and unique vector
                  from an input vector.

Class private member functions:
-------------------------------
feed        - This member function checks the input feed vector and returns the
              fixed feed vector.
time        - This member function checks the time range and returns the
              corresponding time vector.
spw_channel - This member functions checks the input spectral window and channel
              vectors and returns the fixed spectral window and channel vectors.
freq        - This member function creates the total frequency vector based on
              the spectral window and channel vectors.

Class template private member functions:
----------------------------------------
parse<T>  - This member function reshapes the cubes provided by class
            ROCTIter to dimensions required by class CalStats.
select<T> - This member function selects the desired feeds, frequencies, and
            times from an input cube.

Class protected member functions:
---------------------------------
CalAnalysis - This default constructor is unused by this class and unavailable
              when an instance is created.
CalAnalysis - This copy constructor is unused by this class and unavailable when
              an instance is created.
operator=   - This operator= function is unused by this class and unavailable
              when an instance is created.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version created with public member functions CalAnalysis()
              (generic), ~CalAnalysis(), tableType(), polBasis(); template
              public stats member function stats<T>(); class template public
              static functions exists<T>() and uniqueNoSort<T>(); private member
              functions feed(), time(), spw_channel(), freq(); template private
              member functions parse(), and select<T>(); and protected member
              functions CalAnalysis() (default), CalAnalysis() (copy), and
              operator=().
2012 Feb 14 - Nick Elias, NRAO
              Updated this code to reflect changes in NewCalTabIter (now
              ROCTIter) and other classes.  Added the RAP enum.
2012 Mar 13 - Nick Elias, NRAO
              Public member function tableType() renamed to parType().  Public
              member functions visCal() and msName() added.
2012 Mar 14 - Nick Elias, NRAO
              I added the spectral window ID, start channel, and stop channel
              to the nested OUTPUT<T> class.

*/

// -----------------------------------------------------------------------------

class CalAnalysis {

  public:

    // Real/Amplitude/Phase enums.
    typedef enum RAP {
      INIT=-1, REAL=0, AMPLITUDE, PHASE
    } RAP;

    // OUTPUT nested class (allowed T: CalStats::NONE, CalStatsFitter::FIT,
    // or CalStatsHist::HIST), used to hold the vector output of stats<T>().
    template <typename T>
    class OUTPUT {
      public:
        uInt uiField;
        uInt uiAntenna1;
        uInt uiAntenna2;
	Vector<uInt> oSPW;
	Vector<uInt> oStartChan;
	Vector<uInt> oStopChan;
        Matrix<CalStats::OUT<T> > oOut;
    };

    // Generic constructor
    CalAnalysis( const String& oTableName );

    // Destructor
    virtual ~CalAnalysis( void );

    // Get main table keywords
    String& parType( const String& oTableName );
    String& polBasis( const String& oTableName );
    String& visCal( const String& oTableName );
    String& msName( const String& oTableName );

    // Calculate statistics for the specified fields, antennas, time range,
    // feeds, spectral windows, and channels (allowed T: CalStats::NONE gets
    // data without calculating statistics, CalStatsFitter::FIT calculates fits,
    // and CalStatsHist::HIST calculates histogram statistics).
    template <typename T>
    Vector<OUTPUT<T> >& stats( const Vector<uInt>& oFieldIn,
        const Vector<uInt>& oAntenna1In, const Vector<uInt>& oAntenna2In,
        const Double& dStartTimeIn, const Double& dStopTimeIn,
        const Vector<String>& oFeedIn, const Vector<uInt>& oSPWIn,
        const Vector<uInt>& oStartChannelIn, const Vector<uInt>& oStopChannelIn,
        const CalStats::AXIS& eAxisIterUserID, const RAP& eRAP,
        const Double& dJumpMax, const CalStats::ARG<T>& oArg );

    // Function to determine whether a value is present in an array
    template <typename T>
    static Bool& exists( const T& tValue, const Vector<T>& oValue );

    // Function to return unsorted unique values of a vector
    template <typename T>
    static Vector<T>& uniqueNoSort( const Vector<T>& oVector );

  private:

    NewCalTable* poNCT;
    ROCTIter* poNCTIter;

    String oParType;
    
    uInt uiNumFeed;
    Vector<String> oFeed;

    uInt uiNumTime;
    Vector<Double> oTime;

    uInt uiNumSPW;
    Vector<uInt> oSPW;

    uInt uiNumFreq;
    Vector<uInt> oNumFreq;
    Vector<Double> oFreq;

    // These functions check the inputs against possible values and return
    // unsorted unique values
    Bool feed( const Vector<String>& oFeedIn, Vector<String>& oFeedOut ) const;
    Bool time( const Double& dStartTimeIn, const Double& dStopTimeIn,
        Vector<Double>& oTimeOut ) const;
    Bool spw_channel( const Vector<uInt>& oSPWIn,
        const Vector<uInt>& oStartChannelIn, const Vector<uInt>& oStopChannelIn,
        Vector<uInt>& oSPWOut, Vector<uInt>* aoChannelOut ) const;
    Bool freq( const Vector<uInt>& oSPWIn,
        const Vector<uInt>* const aoChannelIn, Vector<Double>& oFreqOut ) const;

    // This function sorts the input feed x frequency(spw) x row(spw,time) cube
    // (from ROCTIter) to feed x frequency x time (for CalStats)
    template <typename T>
    Cube<T>& parse( const Cube<T>& oCubeIn ) const;

    // This function takes the parsed cube and selects the feeds, frequencies,
    // and times
    template <typename T>
    Cube<T>& select( const Cube<T>& oCubeIn, const Vector<String>& oFeedOut,
        const Vector<Double>& oFreqOut, const Vector<Double>& oTimeOut ) const;

  protected:

    // Unused constructors
    CalAnalysis( void );
    CalAnalysis( const CalAnalysis& oCalAnalysis );

    // Unused operator=() function
    CalAnalysis& operator=( const CalAnalysis& oCalAnalysis );

};

// -----------------------------------------------------------------------------
// End of CalAnalysis class definition
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis::stats<T> template public fit member function
// -----------------------------------------------------------------------------

/*

CalAnalysis::stats<T>

Description:
------------
This member function is the main user interface for calculating the statistics
for all iterations.  Allowed T: CalStats::NONE only returns the input data,
CalStatsFitter::FIT calculates fit statistics, and CalStatsHist::HIST calculates
histogram statistics.

NB: This class assumes that the inputs are formulated correctly by a separate
calibration selection function or class.

NB: Scans are not used by this class.  This class assumes that scans have
already been converted to times.

NB: Iteration is used, since NewCalTable does not allow selecting specific data
directly.  It is not the most efficient way to get specific data, but given the
typical file sizes speed will not be an issue.

NB: The stats<T>() function calculates statistics (the type depends on T) and
    returns the results.
  - The user can specify the field(s), antenna(s), time range, feeds, spectral
    windows, frequencies, and the second iteration axis for the CalStats class
    (time or frequency).
  - The inputs are checked and fixed, if possible.
  - The iteration loop goes over field, antenna1, and antenna2.  If a set of
    field, antenna1 and antenna2 numbers from an iteration is consistent with
    the inputs, statistics are calculated.  This is not the most efficient way,
    but the ROCTIter class doesn't have another way to access data and the time
    for each iteration is very fast.
  - For each iteration, the dimensions of the value, value error, and flag cubes
    provided by ROCTIter are feed x frequency(spw) x row(spw,time).  This shape
    is not useful for calculating statistics with CalStats, so the parse<T>()
    function slices and dices the cubes into feed x frequency x time.
  - The parsed cubes are further refined by the select<T>() function to include
    only the feeds, frequencies, and times selected by the input parameters.
  - The resulting cubes are fed to the CalStats class and its stats<T>()
    function calculates the desired statistics which are stored in a vector
    of OUTPUT<T> instances.

Inputs:
-------
oFieldIn        - This reference to a Vector<uInt> instance contains the field
                  IDs.
oAntenna1In     - This reference to a Vector<uInt> instance contains the antenna
                  1 IDs.  This vector must be the same length as oAntenna2In,
                  i.e., the vectors together comprise the baselines.
oAntenna2In     - This reference to a Vector<uInt> instance contains the antenna
                  2 IDs.  This vector must be the same length as oAntenna1In,
                  i.e., the vectors together comprise the baselines.
dStartTimeIn    - This reference to a Double variable contains the start time.
dStopTimeIn     - This reference to a Double variable contains the stop time.
oFeedIn         - This reference to a Vector<String> instance contains the feeds
                  ("R" and "L" for circular, "X" and "Y" for linear, "S" for
                  scalar).
oSPWIn          - This reference to a Vector<uInt> instance contains the
                  spectral window IDs.
oStartChannelIn - This reference to a Vector<uInt> instance contains the start
                  channels.  Each element of the array corresponds to an element
                  of oSPWIn.
oStopChannelIn  - This reference to a Vector<uInt> instance contains the start
                  channels.  Each element of the array corresponds to an element
                  of oSPWIn.
eAxisIterUserID - This reference to a CalStats::AXIS enum contains either the
                  FREQUENCY or TIME iteration axes (user defined).
eRAP            - This reference to a CalAnalysis::RAP enum contains either
                  REAL, AMPLITUDE, or PHASE.
dJumpMax        - This reference to a Double variable contains the maximum
                  deviation from +/- M_PI for adjacent points to be unwrapped
                  by +/- 2.0*M_PI (in radians).  This parameter is always used
                  when the specified iteration axis is CalStats::FREQUENCY
                  (unwrapping along the CalStats::TIME axis).  If the specified
                  iteration axis is CalStats::TIME (unwrapping along the
                  CalStats::FREQUENCY axis), this parameter selects the type of
                  unwrapping (dJumpMax==0.0 --> group-delay unwrapping, dJumpMax
                  != 0.0 --> simple unwrapping).
oArg<T>         - This reference to a CalStats::ARG<T> instance contains the
                  extra input parameters.

Outputs:
--------
The reference to the Vector<CalAnalysis::OUTPUT<T> > instance containing the
statistics, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Logging capability added.
2012 Feb 14 - Nick Elias, NRAO
              Updated this code to reflect changes in NewCalTabIter (now
              ROCTIter) and other classes.  Added a RAP enum as an input
              parameter.
2012 Apr 02 - Nick Elias, NRAO
              Input parameter dJumpMax added.

*/

// -----------------------------------------------------------------------------

template <typename T>
Vector<CalAnalysis::OUTPUT<T> >& CalAnalysis::stats(
    const Vector<uInt>& oFieldIn, const Vector<uInt>& oAntenna1In,
    const Vector<uInt>& oAntenna2In, const Double& dStartTimeIn,
    const Double& dStopTimeIn, const Vector<String>& oFeedIn,
    const Vector<uInt>& oSPWIn, const Vector<uInt>& oStartChannelIn,
    const Vector<uInt>& oStopChannelIn, const CalStats::AXIS& eAxisIterUserID,
    const RAP& eRAP, const Double& dJumpMax, const CalStats::ARG<T>& oArg ) {

  // Initialize the output vector containing statistics versus field ID, antenna
  // 1, and antenna 2

  uInt uiNumOutput = 0;

  Vector<CalAnalysis::OUTPUT<T> >* poOutput =
      new Vector<CalAnalysis::OUTPUT<T> >();


  // Check the inputs

  if ( oAntenna1In.nelements() != oAntenna2In.nelements() ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Antenna ID vectors have different lengths"
        << LogIO::POST;
    return( *poOutput );
  }

  if ( dStartTimeIn > dStopTimeIn ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Start time is larger than the stop time"
        << LogIO::POST;
    return( *poOutput );
  }

  if ( eAxisIterUserID != CalStats::FREQUENCY &&
       eAxisIterUserID != CalStats::TIME ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "User-defined iteration axis must be frequency/time"
        << LogIO::POST;
    return( *poOutput );
  }


  // Check the feeds, time range, spectral windows, channels, and maximum jump
  // parameter and create the temporary vectors.  The temporary total frequency
  // vector is also created.

  Vector<String> oFeedTemp = Vector<String>();
  Bool bFeed = feed( oFeedIn, oFeedTemp );
  if ( !bFeed ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid feed ID(s)" << LogIO::POST;
    return( *poOutput );
  }

  Vector<Double> oTimeTemp = Vector<Double>();
  Bool bTime = time( dStartTimeIn, dStopTimeIn, oTimeTemp );
  if ( !bTime ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid time range" << LogIO::POST;
    return( *poOutput );
  }

  Vector<uInt> oSPWTemp = Vector<uInt>();
  Vector<uInt>* aoChannelTemp = new Vector<uInt> [oSPWIn.nelements()];
  Bool bSPW_Channel = spw_channel( oSPWIn, oStartChannelIn, oStopChannelIn,
      oSPWTemp, aoChannelTemp );
  if ( !bSPW_Channel ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid spectral window(s) and/or channels"
        << LogIO::POST;
    return( *poOutput );
  }

  Vector<Double> oFreqTemp = Vector<Double>();
  Bool bFreq = freq( oSPWTemp, aoChannelTemp, oFreqTemp );
  if ( !bFreq ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid frequencies" << LogIO::POST;
    return( *poOutput );
  }

  if ( dJumpMax < 0.0 ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid maximum jump parameter" << LogIO::POST;
    return( *poOutput );
  }


  // Calculate statistics for each field, antenna1, and antenna2 that are
  // found in the input parameters

  while ( !poNCTIter->pastEnd() ) {

    if ( !exists<uInt>( poNCTIter->field()[0], oFieldIn ) ||
         !exists<uInt>( poNCTIter->antenna1()[0], oAntenna1In ) ||
         !exists<uInt>( poNCTIter->antenna2()[0], oAntenna2In ) ) {
      poNCTIter->next();
      continue;
    }

    poOutput->resize( ++uiNumOutput, True );

    poOutput->operator[](uiNumOutput-1).uiField = poNCTIter->field()[0];
    poOutput->operator[](uiNumOutput-1).uiAntenna1 = poNCTIter->antenna1()[0];
    poOutput->operator[](uiNumOutput-1).uiAntenna2 = poNCTIter->antenna2()[0];
    poOutput->operator[](uiNumOutput-1).oSPW = oSPWTemp;
    poOutput->operator[](uiNumOutput-1).oStartChan = oStartChannelIn;
    poOutput->operator[](uiNumOutput-1).oStopChan = oStopChannelIn;

    Cube<Double> oParamD;
    Cube<DComplex> oParamDC;

    if ( oParType == String("Float") ) {
      Cube<Float> oParamF = select<Float>( parse<Float>(poNCTIter->fparam()),
          oFeedTemp, oFreqTemp, oTimeTemp );
      oParamD.resize( oParamF.shape(), 0.0 );
      convertArray<Double,Float>( oParamD, oParamF.copy() );
    } else {
      Cube<Complex> oParamC = select<Complex>(
          parse<Complex>(poNCTIter->cparam()), oFeedTemp, oFreqTemp,
          oTimeTemp );
      oParamDC.resize( oParamC.shape(), 0.0 );
      convertArray<DComplex,Complex>( oParamDC, oParamC.copy() );
    }

    Cube<Float> oParamErr = select<Float>( parse<Float>(poNCTIter->paramErr()),
        oFeedTemp, oFreqTemp, oTimeTemp );
    Cube<Double> oParamErrD( oParamErr.shape(), 0.0 );
    convertArray<Double,Float>( oParamErrD, oParamErr );

    Cube<Bool> oFlag = select<Bool>( parse<Bool>(poNCTIter->flag()), oFeedTemp,
        oFreqTemp, oTimeTemp );

    CalStats* poCS = NULL;

    try {

      switch ((uInt) eRAP) {
        case (uInt) REAL:
          poCS = (CalStats*) new CalStatsReal( oParamD, oParamErrD, oFlag,
              oFeedTemp, oFreqTemp, oTimeTemp, eAxisIterUserID );
          break;
	case (uInt) AMPLITUDE:
	  poCS = (CalStats*) new CalStatsAmp( oParamDC, oParamErrD, oFlag,
              oFeedTemp, oFreqTemp, oTimeTemp, eAxisIterUserID, True );
          break;
	case (uInt) PHASE:
	  poCS = (CalStats*) new CalStatsPhase( oParamDC, oParamErrD, oFlag,
              oFeedTemp, oFreqTemp, oTimeTemp, eAxisIterUserID, True,
              dJumpMax );
          break;
        default:
          throw( AipsError( "Invalid parameter (REAL, AMPLITUDE, or PHASE)" ) );
      }

      poOutput->operator[](uiNumOutput-1).oOut = poCS->stats<T>( oArg );
      delete poCS;

    }

    catch ( AipsError oAE ) {
      LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
      log << LogIO::WARN << oAE.getMesg()
          << ", iteration (field,antenna1,antenna2) = (" << poNCTIter->field()
          << "," << poNCTIter->antenna1() << "," << poNCTIter->antenna2()
          << "), continuing ..." << LogIO::POST;
      poOutput->operator[](uiNumOutput-1).oOut = CalStats::OUT<T>();
    }

    poNCTIter->next();

  }


  // Reset the new calibration table iterator

  poNCTIter->reset();


  // Return the reference to the Vector<CalAnalysis::OUTPUT<T> > instance

  return( *poOutput );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis::stats<T> template public fit member function
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis static template public member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::exists<T>

Description:
------------
This member function determines whether a value appears in a vector.

Inputs:
-------
tValue - This reference to a T variable contains the desired value.
oValue - This reference to a Vector<T> instance contains the values.

Outputs:
--------
The reference to the existance Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <typename T>
Bool& CalAnalysis::exists( const T& tValue, const Vector<T>& oValue ) {

  // Determine whether the value is present in an array and return the boolean

  Bool* pbValue = new Bool( False );

  for ( uInt v=0; v<oValue.nelements(); v++ ) {
    if ( tValue == oValue[v] ) *pbValue = True;
  }

  return( *pbValue );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::uniqueNoSort<T>

Description:
------------
This member function returns an unsorted and unique vector from an input vector.

Inputs:
-------
oVector - This reference to a Vector<T> instance contains the values.

Outputs:
--------
The reference to the unsorted and unique Vector<T> instance, returned via the
function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <typename T>
Vector<T>& CalAnalysis::uniqueNoSort( const Vector<T>& oVector ) {

  // Initialize the unique and unsorted vector

  Vector<T>* poVectorUnique = new Vector<T>();


  // Form the unique and unsorted vector

  for ( uInt v1=0; v1<oVector.nelements(); v1++ ) {

    Bool bDupe = False;

    for ( uInt v2=0; v2<v1; v2++ ) {
      if ( oVector[v1] == oVector[v2] ) {
        bDupe = True;
	break;
      }
    }

    if ( !bDupe ) {
      poVectorUnique->resize( poVectorUnique->nelements()+1, True );
      poVectorUnique->operator[](poVectorUnique->nelements()-1) = oVector[v1];
    }

  }


  // Return the unique and unsorted vector

  return( *poVectorUnique );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis static template public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis template private member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::parse<T>

Description:
------------
This member function reshapes the cubes provided by class ROCTIter to dimensions
required by class CalStats.

Inputs:
-------
oCubeIn - This reference to a  Cube<T> instance contains the values dimensioned
          by feed x frequency(spw) x row(spw,time).

Outputs:
--------
The reference to the Cube<T> instance dimensioned feed x frequency x time.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <typename T>
Cube<T>& CalAnalysis::parse( const Cube<T>& oCubeIn ) const {

  // Initialize the output cube

  Cube<T>* poCubeOut = new Cube<T>( uiNumFeed, uiNumFreq, uiNumTime, (T) 0 );


  // Put input cube data (whose dimensions are feed x freq(spw) x row(spw,time)
  // into output cube data (whose dimensions are feed x freq x time) using the
  // Slicer() class

  uInt uiFreqStart = 0;

  for ( uInt s=0; s<uiNumSPW; s++ ) {

    IPosition oInStart( 3, 0, 0, s*uiNumTime );
    IPosition oInDelta( 3, uiNumFeed, oNumFreq[s], uiNumTime );
    Slicer oIn( oInStart, oInDelta );

    if ( s > 0 ) uiFreqStart += oNumFreq[s-1];

    IPosition oOutStart( 3, 0, uiFreqStart, 0 );
    IPosition oOutDelta( 3, uiNumFeed, oNumFreq[s], uiNumTime );
    Slicer oOut( oOutStart, oOutDelta );

    poCubeOut->operator()(oOut) = oCubeIn(oIn);

  }


  // Return the output cube

  return( *poCubeOut );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::select<T>

Description:
------------
This member function selects the desired feeds, frequencies, and times from an
input cube.

NB: Cubes feed to this function should be processed by CalAnalysis::parse<T>
first.

NB: I chose the brute force method.  It is not elegant, but it will handle all
types of selections, including pathological ones.  Given the expected sizes of
new calibration tables, speed should not be an issue.

Inputs:
-------
oCubeIn  - This reference to a Cube<T> instance contains the values dimensioned
           by feed x frequency x time.
oFeedOut - This reference to a Vector<Int> instance contains the desired feeds.
oFreqOut - This reference to a Vector<Double> instance contains the desired
           frequencies.
oTimeOut - This reference to a Vector<Double> instance contains the desired
           times.

Outputs:
--------
The reference to the Cube<T> instance dimensioned feed x frequency x time,
returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <typename T>
Cube<T>& CalAnalysis::select( const Cube<T>& oCubeIn,
    const Vector<String>& oFeedOut, const Vector<Double>& oFreqOut,
    const Vector<Double>& oTimeOut ) const {

  // Check the feed inputs (may be redundant, see calling functions) and create
  // the feed map

  Vector<uInt> oFeedMap( oFeedOut.nelements(), 0 );

  for ( uInt pOut=0; pOut<oFeedOut.nelements(); pOut++ ) {
    Bool bFeed = False;
    for ( uInt p=0; p<oFeed.nelements(); p++ ) {
      if ( oFeedOut[pOut] == oFeed[p] ) {
        bFeed = True;
        oFeedMap[pOut] = p;
	break;
      }
    }
    if ( !bFeed ) throw( AipsError( "Invalid feed" ) );
  }


  // Check the frequency inputs (may be redundant, see calling functions) and
  // create the frequency map

  Vector<uInt> oFreqMap( oFreqOut.nelements(), 0 );

  for ( uInt fOut=0; fOut<oFreqOut.nelements(); fOut++ ) {
    Bool bFreq = False;
    for ( uInt f=0; f<oFreq.nelements(); f++ ) {
      if ( oFreqOut[fOut] == oFreq[f] ) {
        bFreq = True;
        oFreqMap[fOut] = f;
	break;
      }
    }
    if ( !bFreq ) throw( AipsError( "Invalid frequency" ) );
  }


  // Check the time inputs (may be redundant, see calling functions) and create
  // the time map

  Vector<uInt> oTimeMap( oTimeOut.nelements(), 0 );

  for ( uInt tOut=0; tOut<oTimeOut.nelements(); tOut++ ) {
    Bool bTime = False;
    for ( uInt t=0; t<oTime.nelements(); t++ ) {
      if ( oTimeOut[tOut] == oTime[t] ) {
        bTime = True;
        oTimeMap[tOut] = t;
	break;
      }
    }
    if ( !bTime ) throw( AipsError( "Invalid time" ) );
  }


  // Get the desired data from the input cube and put them into the output cube

  Cube<T>* poCubeOut = new Cube<T>( oFeedOut.nelements(), oFreqOut.nelements(),
      oTimeOut.nelements(), (T) 0 );

  for ( uInt pOut=0; pOut<oFeedOut.nelements(); pOut++ ) {
    uInt p = oFeedMap[pOut];
    for ( uInt fOut=0; fOut<oFreqOut.nelements(); fOut++ ) {
      uInt f = oFreqMap[fOut];
      for ( uInt tOut=0; tOut<oTimeOut.nelements(); tOut++ ) {
        uInt t = oTimeMap[tOut];
        poCubeOut->operator()(pOut,fOut,tOut) = oCubeIn(p,f,t);
      }
    }
  }


  // Return the output cube

  return( *poCubeOut );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis template private member functions
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------
// End of define macro to prevent multiple loading
// -----------------------------------------------------------------------------
