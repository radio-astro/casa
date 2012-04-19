
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

NB: There are a lot of get/set member functions.  Unfortunately, they could not
    be overloaded with the same names.

Nested classes:
---------------
OUTPUT<T> - This nested class contains the outputs for the
            CalAnalysis::stats<T>() template member function.

Class public member functions:
------------------------------
CalAnalysis  - This constructor gets information from the new format calibration
               table for further processing by the stats<T>() function.
~CalAnalysis - This destructor deallocates the internal memory of an instance.
calName      - This member function returns the new format calibration table
               name.
msName       - This member function returns the associated MS name.
visCal       - This member function returns the visibility calibration type.
parType      - This member function returns the parameter type ("Complex" or
               "Float").
polBasis     - This member function returns the polarization basis ("L" or "C").
field        - This member function returns the field numbers.
antenna      - This member function returns the antenna numbers.
time         - This member function returns the times.
feed         - This member function returns the feeds.
numspw       - This member function returns the number of spectral windows.
spw          - This member function returns the spectral windows.
numChannel   - This member function returns the number of channels for each
               spectral window.
freq         - This member function returns the frequencies for each spectral
               window.

Class template public member functions:
---------------------------------------
stats<T> - This member function is the main user interface for calculating the
           statistics for all iterations.  Allowed T: CalStats::NONE only
           returns the input data, CalStatsFitter::FIT calculates fit
           statistics, and CalStatsHist::HIST calculates histogram statistics.

Class template public static member functions:
----------------------------------------------
exists<T> - This member function determines whether a value appears in a vector.
unique<T> - This member function returns a unique vector from an input vector.

Class private member functions:
-------------------------------
calNameGet   - This member function gets the new format calibration table name
               from the new format calibration table.
calNameSet   - This member function sets the new format calibration table name
               private variable.
msNameGet    - This member function gets the associated MS name from the new
               format calibration table.
msNameSet    - This member function sets the associated MS name private
               variable.
visCalGet    - This member function gets the visibility calibration type from
               the new format calibration table.
visCalSet    - This member function sets the visibility calibration type private
               variable.
parTypeGet   - This member function gets the parameter type ("Complex" or
               "Float") from the new format calibration table.
parTypeSet   - This member function sets the parameter type ("Complex" or
               "Float") private variable.
polBasisGet  - This member function gets the polarization basis ("L" or "C")
               from the new format calibration table.
polBasisSet  - This member function sets the polarization basis ("L" or "C")
               private variable.
fieldGet     - This member function gets the field numbers from the new format
               calibration table.
fieldSet     - This member function sets the field numbers private variables.
fieldCheck   - This member function checks the input field vector and returns
               the fixed field vector.
antennaGet   - This member function gets the antenna numbers from the new format
               calibration table.
antennaSet   - This member function sets the antenna numbers private variables.
antennaCheck - This member function checks the input antenna vector and returns
               the fixed antenna vector.
timeGet      - This member function gets the times from the new format
               calibration table.
timeSet      - This member function sets the times private variables.
timeCheck    - This member function checks the time range and returns the
               corresponding time vector.
feedGet      - This member function gets the feeds from the new format
               calibration table.
feedSet      - This member function sets the feeds private variables.
feedCheck    - This member function checks the input feed vector and returns the
               fixed feed vector.
spwInfoGet   - This member function gets the spectral window information from
               the new format calibration table.
spwInfoSet   - This member function sets the spectral window information private
               variables.

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
              (generic), ~CalAnalysis(); template static public member function
              stats<T>(); template public member functions exists<T>() and
              unique<T>(); private member functions tableType(), polBasisGet(),
              feedCheck(), timeCheck(), spw_channel(), freq(); template private
              member functions parse<T>(), and select<T>(); and protected member
	      functions CalAnalysis() (default), CalAnalysis() (copy), and
              operator=().
2012 Feb 14 - Nick Elias, NRAO
              Updated this code to reflect changes in NewCalTabIter (now
              ROCTIter) and other classes.  Added the RAP enum.
2012 Mar 13 - Nick Elias, NRAO
              Public member function tableType() renamed to parTypeGet().
              Private member functions msNameGet() and visCalGet() added.
2012 Mar 14 - Nick Elias, NRAO
              Spectral window ID, start channel, and stop channel added to the
              nested OUTPUT<T> class.
2012 Apr 03 - Nick Elias, NRAO
              Private member function calNameGet() added.  Public member
              functions calName(), msName(), visCal(), parType(), and polBasis()
              added.
2012 Apr 04 - Nick Elias, NRAO
              Private member functions calNameSet(), msNameSet(), visCalSet(),
              parTypeSet(), polBasisSet(), feedGet(), feedSet(), timeGet(),
              timeSet(), spwGet(), and spwSet() added.  Public member functions
              feed(), time(), spw(), and numspw() added.
2012 Apr 17 - Nick Elias, NRAO
              Nested class CalAnalysis::SPW_INFO() added.  Private member
              functions fieldGet(), fieldSet(), fieldCheck(), antennaGet(),
              antennaSet(), antennaCheck(), spwInfoGet(), and spwInfoSet()
              added.  Public member functions field(), antenna(), numChannel(),
              and freq() added.  Private member functions spwGet(), spwSet(),
              and spw_channel() removed.

*/

// -----------------------------------------------------------------------------

class CalAnalysis {

  public:

    // Real/Amplitude/Phase enums
    typedef enum RAP {
      INIT=-1, REAL=0, AMPLITUDE, PHASE
    } RAP;

    // SPW_INFO nested class
    class SPW_INFO {
      public:
        Bool bValid;
        uInt uiNumSPW;
        Vector<uInt> oSPW;
        Vector<uInt> oNumChannel;
        Vector<Vector<Double> > voFreq;
        SPW_INFO( const String& oTableName );
        SPW_INFO( const SPW_INFO& oSPWInfoIn );
        SPW_INFO( void );
        ~SPW_INFO( void );
        SPW_INFO& subset( const Vector<uInt>& oSPWIn,
            const Vector<uInt>& oStartChannelIn,
            const Vector<uInt>& oStopChannelIn );
        Bool& freq( const Vector<uInt>& oSPWIn,
            const Vector<uInt>& oStartChannelIn,
            const Vector<uInt>& oStopChannelIn, Vector<Double>& oFreqIn );
        Bool spwCheck( const Vector<uInt>& oSPWIn, Vector<uInt>& oSPWOut );
        SPW_INFO& operator=( const SPW_INFO& oSPWInfoIn );
    };

    // OUTPUT nested class (allowed T: CalStats::NONE, CalStatsFitter::FIT,
    // or CalStatsHist::HIST), used to hold the vector output of stats<T>()
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

    // Return the calibration table name and keyword private variables
    String& calName( void ) const;
    String& msName( void ) const;
    String& visCal( void ) const;
    String& parType( void ) const;
    String& polBasis( void ) const;

    // Get the fields, antennas, times, and feeds
    // spectral windows
    Vector<uInt>& field( void ) const;
    Vector<uInt>& antenna( void ) const;
    Vector<Double>& time( void ) const;
    Vector<String>& feed( void ) const;

    // Get the number of spectral windows, spectral windows, number of channels
    // for each spectral window, and frequencies for each spectral window
    uInt& numspw( void ) const;
    Vector<uInt>& spw( void ) const;
    Vector<uInt>& numChannel( void ) const;
    Vector<Vector<Double> > freq( void ) const;

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

    // Function to return sorted unique values of a vector
    template <typename T>
    static Vector<T>& unique( const Vector<T>& oVector );

  private:

    // Classes for new-format calibration table and iterator
    NewCalTable* poNCT;
    ROCTIter* poNCTIter;
    
    // Get calibration table name and set the private variable
    String oCalName;
    String& calNameGet( const String& oTableName );
    void calNameSet( const String& oCalNameIn );

    // Get associated MS name and set the private variable
    String oMSName;
    String& msNameGet( const String& oTableName );
    void msNameSet( const String& oMSNameIn );

    // Get visibility calibration type and set the private variable
    String oVisCal;
    String& visCalGet( const String& oTableName );
    void visCalSet( const String& oVisCalIn );

    // Get parameter column type and set the private variable
    String oParType;
    String& parTypeGet( const String& oTableName );
    void parTypeSet( const String& oParTypeIn );

    // Get polarization basis and set the private variable
    String oPolBasis;
    String& polBasisGet( const String& oTableName );
    void polBasisSet( const String& oPolBasisIn );

    // Get field numbers and set the private variables
    uInt uiNumField; Vector<uInt> oField;
    Vector<uInt>& fieldGet( const String& oTableName );
    void fieldSet( const Vector<uInt>& oFieldIn );
    Bool fieldCheck( const Vector<uInt>& oFieldIn,
        Vector<uInt>& oFieldOut ) const;

    // Get antenna numbers and set the private variables
    uInt uiNumAntenna; Vector<uInt> oAntenna;
    Vector<uInt>& antennaGet( const String& oTableName );
    void antennaSet( const Vector<uInt>& oAntennaIn );
    Bool antennaCheck( const Vector<uInt>& oAntennaIn,
        Vector<uInt>& oAntennaOut ) const;

    // Get the times and set the private variables
    uInt uiNumTime; Vector<Double> oTime;
    Vector<Double>& timeGet( const String& oTableName );
    void timeSet( const Vector<Double>& oTimeIn );
    Bool timeCheck( const Double& dStartTimeIn, const Double& dStopTimeIn,
        Vector<Double>& oTimeOut ) const;

    // Get the feeds and set the private variables
    uInt uiNumFeed; Vector<String> oFeed;
    Vector<String>& feedGet( const String& oTableName );
    void feedSet( const Vector<String>& oFeedIn );
    Bool feedCheck( const Vector<String>& oFeedIn,
        Vector<String>& oFeedOut ) const;

    // Get the spectral window information and set the private variables
    SPW_INFO oSPWInfo; uInt uiNumFreq; Vector<Double> oFreq;
    SPW_INFO& spwInfoGet( const String& oTableName );
    void spwInfoSet( const SPW_INFO& oSPWInfoIn );

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
// Start of CalAnalysis template public member functions
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

  // Initialize the output vector containing statistics for each field ID,
  // antenna 1, and antenna 2

  uInt uiNumOutput = 0;

  Vector<CalAnalysis::OUTPUT<T> >* poOutput =
      new Vector<CalAnalysis::OUTPUT<T> >();


  // Check the fields and create a new field vector

  Vector<uInt> oFieldNew;

  if ( !fieldCheck( oFieldIn, oFieldNew ) ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "One or more invalid fields" << LogIO::POST;
    return( *poOutput );
  }


  // Check the antennas and create the new antenna vectors

  Vector<uInt> oAntenna1New;
  Vector<uInt> oAntenna2New;

  if ( !antennaCheck( oAntenna1In, oAntenna1New ) ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "One or more invalid antenna 1" << LogIO::POST;
    return( *poOutput );
  }

  if ( !antennaCheck( oAntenna2In, oAntenna2New ) ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "One or more invalid antenna 2" << LogIO::POST;
    return( *poOutput );
  }


  // Check the time range and create the corresponding time vector

  Vector<Double> oTimeNew;

  if ( !timeCheck( dStartTimeIn, dStopTimeIn, oTimeNew ) ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid start and/or stop times" << LogIO::POST;
    return( *poOutput );
  }


  // Check the feeds and create the new feed vector

  Vector<String> oFeedNew;

  if ( !feedCheck( oFeedIn, oFeedNew ) ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid feed(s)" << LogIO::POST;
    return( *poOutput );
  }


  // Check the spectral window info and create the new spectral window instance

  Vector<Double> oFreqNew;

  if ( !oSPWInfo.freq( oSPWIn, oStartChannelIn, oStopChannelIn, oFreqNew ) ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "Invalid spectral window information" << LogIO::POST;
  }


  // Check the user-defined iteration axis and jump parameter

  if ( eAxisIterUserID != CalStats::FREQUENCY &&
       eAxisIterUserID != CalStats::TIME ) {
    LogIO log( LogOrigin( "CalAnalysis", "stats<T>()", WHERE ) );
    log << LogIO::WARN << "User-defined iteration axis must be frequency/time"
        << LogIO::POST;
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

    if ( !exists<uInt>( poNCTIter->field()[0], oFieldNew ) ||
         !exists<uInt>( poNCTIter->antenna1()[0], oAntenna1New ) ||
         !exists<uInt>( poNCTIter->antenna2()[0], oAntenna2New ) ) {
      poNCTIter->next();
      continue;
    }

    poOutput->resize( ++uiNumOutput, True );

    poOutput->operator[](uiNumOutput-1).uiField = poNCTIter->field()[0];
    poOutput->operator[](uiNumOutput-1).uiAntenna1 = poNCTIter->antenna1()[0];
    poOutput->operator[](uiNumOutput-1).uiAntenna2 = poNCTIter->antenna2()[0];
    poOutput->operator[](uiNumOutput-1).oSPW = oSPWInfo.oSPW;
    poOutput->operator[](uiNumOutput-1).oStartChan = oStartChannelIn;
    poOutput->operator[](uiNumOutput-1).oStopChan = oStopChannelIn;

    Cube<Double> oParamD;
    Cube<DComplex> oParamDC;

    if ( oParType == String("Float") ) {
      Cube<Float> oParamF = select<Float>( parse<Float>(poNCTIter->fparam()),
          oFeedNew, oFreqNew, oTimeNew );
      oParamD.resize( oParamF.shape(), 0.0 );
      convertArray<Double,Float>( oParamD, oParamF.copy() );
    } else {
      Cube<Complex> oParamC = select<Complex>(
          parse<Complex>(poNCTIter->cparam()), oFeedNew, oFreqNew, oTimeNew );
      oParamDC.resize( oParamC.shape(), 0.0 );
      convertArray<DComplex,Complex>( oParamDC, oParamC.copy() );
    }

    Cube<Float> oParamErr = select<Float>( parse<Float>(poNCTIter->paramErr()),
        oFeedNew, oFreqNew, oTimeNew );
    Cube<Double> oParamErrD( oParamErr.shape(), 0.0 );
    convertArray<Double,Float>( oParamErrD, oParamErr );

    Cube<Bool> oFlag = select<Bool>( parse<Bool>(poNCTIter->flag()), oFeedNew,
        oFreqNew, oTimeNew );

    CalStats* poCS = NULL;

    try {

      switch ((uInt) eRAP) {
        case (uInt) REAL:
          poCS = (CalStats*) new CalStatsReal( oParamD, oParamErrD, oFlag,
              oFeedNew, oFreqNew, oTimeNew, eAxisIterUserID );
          break;
	case (uInt) AMPLITUDE:
	  poCS = (CalStats*) new CalStatsAmp( oParamDC, oParamErrD, oFlag,
              oFeedNew, oFreqNew, oTimeNew, eAxisIterUserID, True );
          break;
	case (uInt) PHASE:
	  poCS = (CalStats*) new CalStatsPhase( oParamDC, oParamErrD, oFlag,
              oFeedNew, oFreqNew, oTimeNew, eAxisIterUserID, True,
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
// End of CalAnalysis template public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis template static public member functions
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

CalAnalysis::unique<T>

Description:
------------
This member function returns a unique sorted vector from an input vector.

Inputs:
-------
oVector - This reference to a Vector<T> instance contains the values.

Outputs:
--------
The reference to the unique sorted Vector<T> instance, returned via the function
value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <typename T>
Vector<T>& CalAnalysis::unique( const Vector<T>& oVector ) {

  // Initialize the unique vector

  Vector<T>* poVectorUnique = new Vector<T>();


  // Form the unique vector

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


  // Sort the unique vector

  Sort::Order eOrder = Sort::Ascending;
  Int iOptions = Sort::QuickSort;

  GenSort<T>::sort( *poVectorUnique, eOrder, (int) iOptions );


  // Return the unique sorted vector

  return( *poVectorUnique );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis template static public member functions
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

  uInt uiNumFreq = sum( oSPWInfo.oNumChannel );

  Cube<T>* poCubeOut = new Cube<T>( uiNumFeed, uiNumFreq, uiNumTime, (T) 0 );


  // Put input cube data (whose dimensions are feed x freq(spw) x row(spw,time)
  // into output cube data (whose dimensions are feed x freq x time) using the
  // Slicer() class

  uInt uiFreqStart = 0;

  for ( uInt s=0; s<oSPWInfo.uiNumSPW; s++ ) {

    IPosition oInStart( 3, 0, 0, s*uiNumTime );
    IPosition oInDelta( 3, uiNumFeed, oSPWInfo.oNumChannel[s], uiNumTime );
    Slicer oIn( oInStart, oInDelta );

    if ( s > 0 ) uiFreqStart += oSPWInfo.oNumChannel[s-1];

    IPosition oOutStart( 3, 0, uiFreqStart, 0 );
    IPosition oOutDelta( 3, uiNumFeed, oSPWInfo.oNumChannel[s], uiNumTime );
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
