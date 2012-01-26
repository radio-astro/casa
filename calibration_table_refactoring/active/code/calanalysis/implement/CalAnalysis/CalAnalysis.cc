
// -----------------------------------------------------------------------------

/*

CalAnalysis.cc

Description:
------------
This file contains member functions for the CalAnalysis class.

Classes:
--------
CalAnalysis - This class acts as the interface between the NewCalTabIter and
              CalAnalysis classes.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Logging capability added.  Error checking added.

*/

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <calanalysis/CalAnalysis/CalAnalysis.h>

// -----------------------------------------------------------------------------
// Start of casa namespace
// -----------------------------------------------------------------------------

namespace casa {

// -----------------------------------------------------------------------------
// Start of CalAnalysis class
// -----------------------------------------------------------------------------

/*

CalAnalysis

Description:
------------
This class acts as the interface between the NewCalTabIter and CalAnalysis
classes.

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
  - For each iteration, the dimensions of the data, data error, and flag cubes
    provided by NewCalTabIter are feed x frequency(spw) x row(spw,time).  This
    shape is not useful for calculating statistics with CalStats, so the
    parse<T>() function slices and dices the cubes into feed x frequency x
    time.
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
tableType    - This function returns the table type (Complex or Float).
polBasis     - This function returns the polarization basis (linear, circular,
               or scalar).

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
            NewCalTabIter to dimensions prefered by class CalStats.
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

*/

// -----------------------------------------------------------------------------
// Start of CalAnalysis public member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::CalAnalysis

Description:
------------
This constructor gets information from the new format calibration table for
further processing by the stats<T>() function.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Error checking added.

*/

// -----------------------------------------------------------------------------

CalAnalysis::CalAnalysis( const String& oTableName ) {

  // Reconstitute the new calibration table in memory

  try {
    poNCT = new NewCalTable( oTableName, Table::Old, Table::Memory );
  }
  catch ( AipsError oAE ) {
    throw( oAE );
  }


  // Create the iterator for the new calibration table

  Block<String> oColIter( 3 );
  oColIter[0] = String( "ANTENNA2" );
  oColIter[1] = String( "ANTENNA1" );
  oColIter[2] = String( "FIELD_ID" );

  poNCTIter = new NewCalTabIter( *poNCT, oColIter );
  poNCTIter->reset();


  // Get the table type and polarization basis

  oTableType = tableType( oTableName );
  oPolBasis = polBasis( oTableName );


  // Create the feed vector

  uiNumFeed = poNCTIter->param().shape()[0];
  oFeed = Vector<String>( uiNumFeed, "" );

  if ( uiNumFeed == 1 ) {
    oFeed[0] = "S";
  } else {
    if ( oPolBasis == 'L' ) {
      oFeed[0] = "X";
      oFeed[1] = "Y";
    } else {
      oFeed[0] = "L";
      oFeed[1] = "R";
    }
  }


  // Get the unique and unsorted time vector and spectral window vector

  oTime = uniqueNoSort<Double>( poNCTIter->time() );
  uiNumTime = oTime.nelements();

  oSPW = uniqueNoSort<Int>( poNCTIter->spw() );
  uiNumSPW = oSPW.nelements();


  // Get the CHAN_FREQ column from the spectral window subtable

  String oSPWName( oTableName + "/SPECTRAL_WINDOW" );
  NewCalTable::CalSpectralWindow oCalSPW( oSPWName, Table::Old );

  ROArrayColumn<Double> oFreqACD( ROMSSpWindowColumns(oCalSPW).chanFreq() );


  // Get the number of frequencies for each spectral window and the total number
  // of frequencies

  oNumFreq = Vector<uInt>( uiNumSPW );
  for ( uInt s=0; s<uiNumSPW; s++ ) oNumFreq[s] = oFreqACD(s).nelements();

  uiNumFreq = 0;
  for ( uInt s=0; s<uiNumSPW; s++ ) uiNumFreq += oNumFreq[s];


  // Create the total frequency vector

  oFreq = Vector<Double>( uiNumFreq );

  for ( uInt s=0,fMin=0,fMax=0; s<uiNumSPW; s++ ) {

    if ( s > 0 ) fMin += oNumFreq[s-1];
    fMax += oNumFreq[s];

    Vector<Double> oFreqTemp( oFreqACD(s) );
    for ( uInt f=fMin; f<fMax; f++ ) oFreq[f] = oFreqTemp[f-fMin];

  }


  // Return

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::~CalAnalysis

Description:
------------
This destructor deallocates the internal memory of an instance.

Inputs:
-------
None.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalAnalysis::~CalAnalysis( void ) {

  // Deallocate the memory of the private variables

  delete poNCT;
  delete poNCTIter;


  // Return

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::tableType

Description:
------------
This function returns the table type (Complex or Float).

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the String instance containing the table type (Complex or
Float), returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::tableType( const String& oTableName ) {

  // Get the record containing the main table keywords

  Table oTable( oTableName );
  TableProxy oTP( oTable );
  Record oR( oTP.getKeywordSet( String("") ) );


  // Get the table type (Complex or Float) and return it

  uInt uiIndex = oR.fieldNumber( String( "Type" ) );
  String* poCalType = new String( RecordFieldPtr<String>(oR,uiIndex).get() );

  return( *poCalType );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::polBasis

Description:
------------
This function returns the polarization basis (linear, circular, or scalar).

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the String instance containing the polarization basis (linear,
circular, or scalar), returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::polBasis( const String& oTableName ) {

  // Get the record containing the main table keywords

  Table oTable( oTableName );
  TableProxy oTP( oTable );
  Record oR( oTP.getKeywordSet( String("") ) );


  // Get the polarization basis (Linear, Circular, or Scalar), make it upper
  // case, and keep only the initial letter

  uInt uiIndex = oR.fieldNumber( String( "PolBasis" ) );
  String* poPolBasis = new String( RecordFieldPtr<String>(oR,uiIndex).get() );

  poPolBasis->upcase();
  *poPolBasis = poPolBasis->operator[](0);


  // Return the polarization basis

  return( *poPolBasis );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis private member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::feed

Description:
------------
This member function checks the input feed vector and returns the fixed feed
vector.

Inputs:
-------
oFeedIn - This reference to a Vector<String> instance contains the feeds ("R"
          and "L" for circular, "X" and "Y" for linear, "S" for scalar).

Outputs:
--------
oFeedOut - This reference to the Vector<String> instance contains the checked
           and fixed feeds.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::feed( const Vector<String>& oFeedIn,
    Vector<String>& oFeedOut ) const {

  // Check the input feed values

  for ( uInt f=0; f<oFeedIn.nelements(); f++ ) {
    if ( !exists<String>( oFeedIn[f], oFeed ) ) return( False );
  }


  // Get the unique and unsorted feed vector

  if ( oFeedOut.nelements() != 0 ) oFeedOut.resize();
  oFeedOut = uniqueNoSort<String>( oFeedIn );


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::time

Description:
------------
This member function checks the time range and returns the corresponding time
vector.

Inputs:
-------
dStartTimeIn - This reference to a Double variable contains the start time.
dStopTimeIn  - This reference to a Double variable contains the stop time.

Outputs:
--------
oTimeOut - This reference to the Vector<Double> instance containing the time
           stamps.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::time( const Double& dStartTimeIn, const Double& dStopTimeIn,
    Vector<Double>& oTimeOut ) const {

  // Check the start and stop times

  if ( dStartTimeIn > oTime[uiNumTime-1] || dStopTimeIn < oTime[0] ) {
    return( False );
  }


  // Get the unique and unsorted time values

  if ( oTimeOut.nelements() != 0 ) oTimeOut.resize();

  for ( uInt t=0,tOut=0; t<uiNumTime; t++ ) {
    if ( oTime[t] >= dStartTimeIn && oTime[t] <= dStopTimeIn ) {
      oTimeOut.resize( ++tOut, True );
      oTimeOut[tOut-1] = oTime[t];
    }
  }


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::spw_channel

Description:
------------
This member functions checks the input spectral window and channel vectors and
returns the fixed spectral window and channel vectors.

NB: Each element of the Vector<Int> array must correspond to an element in the
spectral window vector.  If there is no correspondence, the function stops and
returns False.

Inputs:
-------
oSPWIn      - This reference to a Vector<Int> instance contains the spectral
              window IDs.
aoChannelIn - This array of Vector<Int> instances containing the channel
              numbers.  Each element of the array corresponds to an element of
              oSPWIn.

Outputs:
--------
oSPWIn       - This Vector<Int> instance contains the checked spectral window
               IDs.
aoChannelOut - This array of Vector<Int> instances containing the checked
               channel numbers.  Each element of the array corresponds to an
               element of oSPWIn.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::spw_channel( const Vector<Int>& oSPWIn,
    const Vector<Int>* const aoChannelIn, Vector<Int>& oSPWOut,
    Vector<Int>* aoChannelOut ) const {

  // Check the input spectral window and channel values

  for ( uInt s=0; s<oSPWIn.nelements(); s++ ) {

    if ( !exists<Int>( oSPWIn[s], oSPW ) ) return( False );

    for ( uInt c=0; c<aoChannelIn[s].nelements(); c++ ) {
      if ( aoChannelIn[s][c] < 0 || aoChannelIn[s][c] >= (Int) oNumFreq[s] ) {
        return( False );
      }
    }

  }


  // Get the unique and unsorted spectral window values.  If there are duplicate
  // values, return False.

  if ( oSPWOut.nelements() != 0 ) oSPWOut.resize();
  oSPWOut = uniqueNoSort<Int>( oSPWIn );

  if ( oSPWOut.nelements() != oSPWIn.nelements() ) return( False );


  // Get the unique and unsorted channel values

  if ( aoChannelOut == NULL ) {
    aoChannelOut = new Vector<Int> [oSPWOut.nelements()];
  }

  for ( uInt s=0; s<oSPWOut.nelements(); s++ ) {

    if ( aoChannelOut[s].nelements() > 0 ) aoChannelOut[s].resize();
    aoChannelOut[s] = aoChannelIn[s];

    uInt uiNumChannelOut = GenSort<Int>::sort( aoChannelOut[s],
        Sort::Ascending, Sort::NoDuplicates );
    aoChannelOut[s].resize( uiNumChannelOut, True );

  }


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::freq

Description:
------------
This member function creates the total frequency vector based on the spectral
window and channel vectors.

NB: The spectral window and channel vectors should already be checked by the
CalAnalysis::spw_channel() function before feeding them to this function.

Inputs:
-------
oSPWIn      - This reference to a Vector<Int> instance contains the spectral
              window IDs.
aoChannelIn - This array of Vector<Int> instances containing the channel
              numbers.  Each element of the array corresponds to an element of
              oSPWIn.

Outputs:
--------
oFreqOut - This reference to the Vector<Double> instance contains the
           frequencies.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::freq( const Vector<Int>& oSPWIn,
    const Vector<Int>* const aoChannelIn, Vector<Double>& oFreqOut ) const {

  // Get the frequency values

  if ( oFreqOut.nelements() != 0 ) oFreqOut.resize();

  uInt uiNumFreqOut = 0;

  for ( uInt s=0,fStart=0; s<oSPWIn.nelements(); s++ ) {

    for ( uInt c=0; c<aoChannelIn[s].nelements(); c++ ) {
      oFreqOut.resize( ++uiNumFreqOut, True );
      oFreqOut[uiNumFreqOut-1] = oFreq[aoChannelIn[s][c]+fStart];
    }

    fStart += oNumFreq[s];

  }


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis private member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis protected member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::CalAnalysis (default)

Description:
------------
This default constructor is unused by this class and unavailable when an
instance is created.

Inputs:
-------
None.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalAnalysis::CalAnalysis( void ) {}

// -----------------------------------------------------------------------------

/*

CalAnalysis::CalAnalysis (copy)

Description:
------------
This copy constructor is unused by this class and unavailable when an instance
is created.

Inputs:
-------
oCalAnalysis - A reference to a CalAnalysis instance.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalAnalysis::CalAnalysis( const CalAnalysis& oCalAnalysis ) {

  // Overwrite this instance and return.  This code will bomb.  I have written
  // it in this way to keep the compiler from spewing warning messages about
  // unused variables.

  *this = oCalAnalysis;

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::operator=

Description:
------------
This operator= function is unused by this class and unavailable when an instance
is created.

Inputs:
-------
oCalAnalysis - A reference to a CalAnalysis instance.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalAnalysis& CalAnalysis::operator=( const CalAnalysis& oCalAnalysis ) {

  // Copy the input instance and return it.  This code will bomb.  I have
  // written it in this way to keep the compiler from spewing warning messages
  // about unused variables.

  CalAnalysis* poCalAnalysis = new CalAnalysis( oCalAnalysis );

  return( *poCalAnalysis );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis protected member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// End of CalAnalysis class
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------
