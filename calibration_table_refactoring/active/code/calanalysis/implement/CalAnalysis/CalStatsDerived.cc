
// -----------------------------------------------------------------------------

/*

CalStatsDerived.cc

Description:
------------
This file contains the member functions of the classes derived from CalStats.

Classes:
--------
CalStatsReal  - This class feeds real data to the CalStats base class.
CalStatsAmp   - This class converts complex data to amplitudes and initializes
                the CalStats base class.
CalStatsPhase - This class converts complex data to phases and initializes the
                CalStats base class.

Inhertited classes:
-------------------
CalStats - This class calculates statistics on CASA caltables.

Modification history:
---------------------
2011 Nov 15 - Nick Elias, NRAO
              Initial version created with classes CalStatsAmp and
              CalStatsPhase.
2011 Dec 11 - Nick Elias, NRAO
              Class CalStatsReal added.

*/

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <calanalysis/CalAnalysis/CalStatsDerived.h>

// -----------------------------------------------------------------------------
// Start of casa namespace
// -----------------------------------------------------------------------------

namespace casa {

// -----------------------------------------------------------------------------
// Start of CalStatsReal class
// -----------------------------------------------------------------------------

/*

CalStatsReal

Description:
------------
This class feeds real data to the CalStats base class.

Inhertited classes:
-------------------
CalStats - This class calculates statistics of new CASA caltables.

Class public member functions:
------------------------------
CalStatsReal  - This constructor feeds real data to the CalStats base class.
~CalStatsReal - This destructor deallocates the internal memory of an instance.

2011 Dec 11 - Nick Elias, NRAO
              Initial version created with public member functions
              CalStatsReal() and ~CalStatsReal().

*/

// -----------------------------------------------------------------------------
// Start of CalStatsReal public member functions
// -----------------------------------------------------------------------------

/*

CalStatsReal::CalStatsReal

Description:
------------
This class feeds real data to the CalStats base class..

NB: The FEED axis is always included as an iteration axis by default because one
cannot perform a fit along it.  The other iteration axis is defined by the user.

NB: I do not employ the direct call to the corresponding CalStats constructor.
I initialize an instance of this CalStatsReal class by creating a local instance
of CalStats and copying its state to the state of this CalStatsReal instance.
Yes, it's not as elegant, but it is easy to read and required for CalStatsAmp
and CalStatsPhase classes, anyway.

Inputs:
-------
oData           - This Cube<Float> object contains the data.
oDataErr        - This Cube<Float> object contains the data errors.
oFlag           - This Cube<Bool> object contains the flags.
oFeed           - This Vector<String> object is the feed abscissae.
oFrequency      - This Vector<Float> object is the frequency abscissae.
oTime           - This Vector<Float> object is the time abscissae.
eAxisIterUserID - This CalStats::AXIS enum contains either the FREQUENCY or TIME
                  iteration axes (user defined).

Outputs:
--------
None.

Modification history:
---------------------
2011 Dec 11 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStatsReal::CalStatsReal( const Cube<Float>& oData,
    const Cube<Float>& oDataErr, const Cube<Bool>& oFlag,
    const Vector<String>& oFeed, const Vector<Float>& oFrequency,
    const Vector<Float>& oTime, const CalStats::AXIS& eAxisIterUser )
    : CalStats() {

  // Create an instance of the CalStats base class constructor and copy its
  // state to this CalStatsReal instance

  CalStats oCS( oData, oDataErr, oFlag, oFeed, oFrequency, oTime,
      eAxisIterUser );

  oAxisIterID = IPosition( oCS.axisIterID() );
  eAxisNonIterID = oCS.axisNonIterID();

  oAxisIterFeed = Vector<String>( oCS.axisIterFeed() );
  oAxisIterUser = Vector<Float>( oCS.axisIterUser() );
  oAxisNonIter = Vector<Float>( oCS.axisNonIter() );

  oStatsShape = IPosition( oCS.statsShape() );

  poData = new Cube<Float>( oCS.data() );
  poDataErr = new Cube<Float>( oCS.dataErr() );
  poFlag = new Cube<Bool>( oCS.flag() );

  poDataIter = new ArrayIterator<Float>( *poData, oAxisIterID, False );
  poDataIter->reset();

  poDataErrIter = new ArrayIterator<Float>( *poDataErr, oAxisIterID, False );
  poDataErrIter->reset();

  poFlagIter = new ArrayIterator<Bool>( *poFlag, oAxisIterID, False );
  poFlagIter->reset();


  // Return

  return;

}

// -----------------------------------------------------------------------------

/*

CalStatsReal::~CalStatsReal

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
2011 Dec 11 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStatsReal::~CalStatsReal( void ) {}

// -----------------------------------------------------------------------------
// End of CalStatsReal public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// End of CalStatsReal class
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalStatsAmp class
// -----------------------------------------------------------------------------

/*

CalStatsAmp

Description:
------------
This class converts complex data to amplitudes and initializes the CalStats base
class.

Inhertited classes:
-------------------
CalStats - This class calculates statistics of new CASA caltables.

CalStatsAmp public member functions:
------------------------------------
CalStatsAmp  - This generic constructor converts complex data to amplitudes and
               initializes the CalStats base class.
~CalStatsAmp - This destructor deallocates the internal memory of an instance.

CalStatsAmp public static member functions:
-------------------------------------------
norm - This member function normalizes the amplitudes their errors.

Modification history:
---------------------
2011 Nov 15 - Nick Elias, NRAO
              Initial version created with public member functions are
              CalStatsAmp() and ~CalStatsAmp(); and public static member
              function is norm().

*/

// -----------------------------------------------------------------------------
// Start of CalStatsAmp public member functions
// -----------------------------------------------------------------------------

/*

CalStatsAmp::CalStatsAmp

Description:
------------
This class converts complex data and their errors to amplitudes and their errors
and initializes the CalStats base class.

NB: The FEED axis is always included as an iteration axis by default because one
cannot perform a fit along it.  The other iteration axis is defined by the user.

NB: If normalization is selected, it is performed along the FREQUENCY axis.

NB: The default CalStats constructor is called.

NB: The are no input data real-imaginary cross correlations available, so the
amplitude errors do not include them.

Inputs:
-------
oData           - This Cube<Float> object contains the data.
oDataErr        - This Cube<Float> object contains the data errors.
oFlag           - This Cube<Bool> object contains the flags.
oFeed           - This Vector<String> object is the feed abscissae.
oFrequency      - This Vector<Float> object is the frequency abscissae.
oTime           - This Vector<Float> object is the time abscissae.
eAxisIterUserID - This CalStats::AXIS enum contains either the FREQUENCY or TIME
                  iteration axes (user defined).
bNorm           - This Bool contains the normalization flag (True = normalize,
                  False = don't normalize).

Outputs:
--------
None.

Modification history:
---------------------
2011 Nov 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStatsAmp::CalStatsAmp( const Cube<Complex>& oData,
    const Cube<Complex>& oDataErr, const Cube<Bool>& oFlag,
    const Vector<String>& oFeed, const Vector<Float>& oFrequency,
    const Vector<Float>& oTime, const CalStats::AXIS& eAxisIterUser,
    const Bool& bNorm ) : CalStats() {

  // Calculate the amplitudes

  Cube<Float> oAmp( amplitude( oData ) );


  // Calculate the amplitude errors

  Cube<Float> oR( real( oData ) );
  Cube<Float> oErrR( real( oDataErr ) );

  Cube<Float> oI( imag( oData ) );
  Cube<Float> oErrI( imag( oDataErr ) );

  Cube<Float> oPhase( phase( oData ) );
  Cube<Float> oCos( cos( oPhase ) );
  Cube<Float> oSin( sin( oPhase ) );

  Cube<Float> oAmpErr( sqrt( square(oCos*oErrR) + square(oSin*oErrI) ) );


  // Normalize the amplitudes and their errors, if selected.  The input flag
  // cube cannot be modified, so make a local copy.

  Cube<Bool> oFlagCopy( oFlag );

  if ( bNorm ) norm( oAmp, oAmpErr, oFlagCopy );


  // Create an instance of the CalStats base class constructor and copy its
  // state to this CalStatsAmp instance

  CalStats oCS( oAmp, oAmpErr, oFlagCopy, oFeed, oFrequency, oTime,
      eAxisIterUser );

  oAxisIterID = IPosition( oCS.axisIterID() );
  eAxisNonIterID = oCS.axisNonIterID();

  oAxisIterFeed = Vector<String>( oCS.axisIterFeed() );
  oAxisIterUser = Vector<Float>( oCS.axisIterUser() );
  oAxisNonIter = Vector<Float>( oCS.axisNonIter() );

  oStatsShape = IPosition( oCS.statsShape() );

  poData = new Cube<Float>( oCS.data() );
  poDataErr = new Cube<Float>( oCS.dataErr() );
  poFlag = new Cube<Bool>( oCS.flag() );

  poDataIter = new ArrayIterator<Float>( *poData, oAxisIterID, False );
  poDataIter->reset();

  poDataErrIter = new ArrayIterator<Float>( *poDataErr, oAxisIterID, False );
  poDataErrIter->reset();

  poFlagIter = new ArrayIterator<Bool>( *poFlag, oAxisIterID, False );
  poFlagIter->reset();


  // Return

  return;

}

// -----------------------------------------------------------------------------

/*

CalStatsAmp::~CalStatsAmp

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
2011 Nov 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStatsAmp::~CalStatsAmp( void ) {}

// -----------------------------------------------------------------------------
// End of CalStatsAmp public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalStatsAmp public static member functions
// -----------------------------------------------------------------------------

/*

CalStatsAmp::norm

Description:
------------
This member function normalizes the amplitudes and their errors.

NB: The normalization is applied only along the frequency axis.

NB: The normalization is applied only when the frequency axis length is greater
than 1.

NB: The normalization is applied only when the number of unflagged frequencies
is greater than 1.

Inputs:
-------
oAmp    - This reference to a Cube<Float> obtain contains the amplitudes.
oAmpErr - This reference to a Cube<Float> obtain contains the amplitude errors.
oFlag   - This Cube<Bool> object contains the flags.

Outputs:
--------
oAmp    - This reference to a Cube<Float> obtain contains the amplitudes.
oAmpErr - This reference to a Cube<Float> obtain contains the amplitude errors.
oFlag   - This Cube<Bool> object contains the flags.

Modification history:
---------------------
2011 Dec 11 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalStatsAmp::norm( Cube<Float>& oAmp, Cube<Float>& oAmpErr,
    Cube<Bool>& oFlag ) {

  // Return if the length of the FREQUENCY axis is unity or if all of the
  // amplitudes are flagged

  if ( oAmp.shape()[CalStats::FREQUENCY] <= 1 ) return;

  if ( allEQ( oFlag, True ) ) return;


  // Create iterators for the amplitude, amplitude error, and flag cubes

  IPosition oIterShape( 2, (ssize_t) CalStats::FEED, (ssize_t) CalStats::TIME );

  ArrayIterator<Float> oAmpIter( oAmp, oIterShape, False );
  ArrayIterator<Float> oAmpErrIter( oAmpErr, oIterShape, False );
  ArrayIterator<Bool> oFlagIter( oFlag, oIterShape, False );


  // Normalize the amplitudes and their errors along the frequency axis

  while ( !oAmpIter.pastEnd() ) {

    uInt uiNumFrequency = oAmpIter.array().nelements();
    IPosition oShape( 1, uiNumFrequency );

    MaskedArray<Float> oAmpM( oAmpIter.array(), !oFlagIter.array() );
    Float fAmpMax = max( oAmpM.getCompressedArray() );

    Vector<Bool> oFlagV( oFlagIter.array().copy().reform(oShape) );
    Vector<Bool> oFlagLow( oAmpIter.array() <= (Float) 1.0E-08*fAmpMax );

    oFlagV = oFlagV || oFlagLow;
    oFlagIter.array() = oFlagV;

    Vector<Float> oAmpTemp( oAmpIter.array().copy().reform(oShape) );
    Vector<Float> oAmpErrTemp( oAmpErrIter.array().copy().reform(oShape) );

    for ( uInt f=0; f<uiNumFrequency; f++ ) {
      if ( !oFlagV[f] ) {
        oAmpTemp[f] /= fAmpMax;
	oAmpErrTemp[f] /= fAmpMax;
      }
    }

    oAmpIter.array() = oAmpTemp;
    oAmpErrIter.array() = oAmpErrTemp;

    oAmpIter.next(); oAmpErrIter.next(); oFlagIter.next();

  }


  // Return

  return;

}

// -----------------------------------------------------------------------------
// End of CalStatsAmp public static member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// End of CalStatsAmp class
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalStatsPhase class
// -----------------------------------------------------------------------------

/*

CalStatsPhase

Description:
------------
This class converts complex data to phases and initializes the CalStats base
class.

Inhertited classes:
-------------------
CalStats - This class calculates statistics of new CASA caltables.

Class public member functions:
------------------------------
CalStatsPhase  - This generic constructor converts complex data to amplitudes
                 and initializes the CalStats base class.
~CalStatsPhase - This destructor deallocates the internal memory of an instance.

CalStatsPhase public static member functions:
---------------------------------------------
unwrap - This member function unwraps the phases.

Modification history:
---------------------
2011 Nov 15 - Nick Elias, NRAO
              Initial version created with public member functions are
              CalStatsPhase() and ~CalStatsPhase(); and public static member
              function is unwrap().

*/

// -----------------------------------------------------------------------------
// Start of CalStatsPhase public member functions
// -----------------------------------------------------------------------------

/*

CalStatsPhase::CalStatsPhase

Description:
------------
This class converts complex data and their errors to phases and their errors and
initializes the CalStats base class.

NB: The FEED axis is always included as an iteration axis by default because one
cannot perform a fit along it.  The other iteration axis is defined by the user.

NB: If unwrapping is selected, it is performed along the FREQUENCY axis.

NB: All flags corresponding to amplitudes less then 1.0E-08 times the peak
amplitude (along the FREQUENCY axis) are updated to True.

NB: The default CalStats constructor is called.

NB: The are no input data real-imaginary cross correlations available, so the
amplitude errors do not include them.

Inputs:
-------
oData           - This Cube<Float> object contains the data.
oDataErr        - This Cube<Float> object contains the data errors.
oFlag           - This Cube<Bool> object contains the flags.
oFeed           - This Vector<String> object is the feed abscissae.
oFrequency      - This Vector<Float> object is the frequency abscissae.
oTime           - This Vector<Float> object is the time abscissae.
eAxisIterUserID - This CalStats::AXIS enum contains either the FREQUENCY or TIME
                  iteration axes (user defined).
bUnwrap         - This Bool contains the unwrapping flag (True = unwrap, False =
                  don't unwrap).

Outputs:
--------
None.

Modification history:
---------------------
2011 Nov 15 - Nick Elias, NRAO
              Initial version. 
2011 Dec 11 - Nick Elias, NRAO
              Flag updates according to low amplitude was added.

*/

// -----------------------------------------------------------------------------

CalStatsPhase::CalStatsPhase( const Cube<Complex>& oData,
    const Cube<Complex>& oDataErr, const Cube<Bool>& oFlag,
    const Vector<String>& oFeed, const Vector<Float>& oFrequency,
    const Vector<Float>& oTime, const CalStats::AXIS& eAxisIterUser,
    const Bool& bUnwrap ) : CalStats() {

  // Calculate the phases and create its iterator

  IPosition oIterShape( 2, (ssize_t) CalStats::FEED, (ssize_t) CalStats::TIME );

  Cube<Float> oPhase( phase(oData) );
  ArrayIterator<Float> oPhaseIter( oPhase, oIterShape, False );


  // Create iterators for the input data, data error, flag , and phase error
  // cubes.  The input cubes cannot be modified, so copies are made.  Create the
  // phase error cube (initialized to M_PI) and its iterator.

  ReadOnlyArrayIterator<Complex> oDataIter( oData, oIterShape, False );

  ReadOnlyArrayIterator<Complex> oDataErrIter( oDataErr, oIterShape, False );

  Cube<Bool> oFlagCopy( oFlag );
  ArrayIterator<Bool> oFlagIter( oFlagCopy, oIterShape, False );

  Cube<Float> oPhaseErr( oPhase.shape(), (Float) M_PI );
  ArrayIterator<Float> oPhaseErrIter( oPhaseErr, oIterShape, False );


  // Calculate the phase errors.  They require dividing by amplitude.  Update
  // flags according to low amplitudes (corresponding to phase errors greater
  // than M_PI).  All flagged phase errors remain M_PI.  The iteration axes are
  // FEED and TIME.  A FREQUENCY for loop is located inside the FEED/TIME
  // iteration loop. 

  while ( !oDataIter.pastEnd() ) {

    uInt uiNumFrequency = oDataIter.array().nelements();
    IPosition oShape( 1, uiNumFrequency );

    Vector<Complex> oDataV( oDataIter.array().copy().reform(oShape) );
    Vector<Complex> oDataErrV( oDataErrIter.array().copy().reform(oShape) );

    Vector<Float> oR( real(oDataV) );
    Vector<Float> oRErr( real(oDataErrV) );

    Vector<Float> oI( imag(oDataV) );
    Vector<Float> oIErr( imag(oDataErrV) );

    Vector<Float> oPhaseV( oPhaseIter.array().copy().reform(oShape) );
    Vector<Float> oPhaseErrV( oPhaseErrIter.array().copy().reform(oShape) );

    Vector<Float> oCos( cos(oPhaseV) );
    Vector<Float> oSin( sin(oPhaseV) );

    Vector<Float> oAmp( sqrt(square(oR)+square(oI)) );

    Vector<Float> oMetricR = pow( oSin*oRErr, 2 );
    Vector<Float> oMetricI = pow( oCos*oIErr, 2 );
    Vector<Float> oMetric = sqrt( oMetricR+oMetricI );

    Vector<Bool> oFlagV( oFlagIter.array().copy().reform(oShape) );
    Vector<Bool> oFlagLow( oAmp <= oMetric/((Float) M_PI) );

    oFlagV = oFlagV || oFlagLow;
    oFlagIter.array() = oFlagV;

    for ( uInt f=0; f<uiNumFrequency; f++ ) {
      if ( !oFlagV[f] ) oPhaseErrV[f] = oMetric[f] / oAmp[f];
    }

    oPhaseErrIter.array() = oPhaseErrV;

    oDataIter.next(); oDataErrIter.next(); oFlagIter.next();
    oPhaseIter.next(); oPhaseErrIter.next();

  }


  // Unwrap the phases, if selected

  if ( bUnwrap ) unwrap( oPhase, oFrequency, oFlagCopy );


  // Create an instance of the CalStats base class constructor and copy its
  // state to this CalStatsPhase instance

  CalStats oCS( oPhase, oPhaseErr, oFlagCopy, oFeed, oFrequency, oTime,
      eAxisIterUser );

  oAxisIterID = IPosition( oCS.axisIterID() );
  eAxisNonIterID = oCS.axisNonIterID();

  oAxisIterFeed = Vector<String>( oCS.axisIterFeed() );
  oAxisIterUser = Vector<Float>( oCS.axisIterUser() );
  oAxisNonIter = Vector<Float>( oCS.axisNonIter() );

  oStatsShape = IPosition( oCS.statsShape() );

  poData = new Cube<Float>( oCS.data() );
  poDataErr = new Cube<Float>( oCS.dataErr() );
  poFlag = new Cube<Bool>( oCS.flag() );

  poDataIter = new ArrayIterator<Float>( *poData, oAxisIterID, False );
  poDataIter->reset();

  poDataErrIter = new ArrayIterator<Float>( *poDataErr, oAxisIterID, False );
  poDataErrIter->reset();

  poFlagIter = new ArrayIterator<Bool>( *poFlag, oAxisIterID, False );
  poFlagIter->reset();


  // Return

  return;

}

// -----------------------------------------------------------------------------

/*

CalStatsPhase::~CalStatsPhase

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
2011 Nov 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStatsPhase::~CalStatsPhase( void ) {}

// -----------------------------------------------------------------------------
// End of CalStatsPhase public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalStatsPhase public static member functions
// -----------------------------------------------------------------------------

/*

CalStatsPhase::unwrap

Description:
------------
This member function unwraps the phases.

NB: The unwrapping is applied only along the frequency axis.

NB: The unwrapping is applied only when the frequency axis length is greater
than 1.

NB: The unwrapping is applied only when the number of unflagged frequencies
is greater than 1.

Inputs:
-------
oPhase     - This reference to an Cube<Float> obtain contains the phases.
oFrequency - This Vector<Float> object is the frequency abscissa.
oFlag      - This Cube<Bool> object contains the flags.

Outputs:
--------
oPhase - This reference to an Cube<Float> obtain contains the phases.

Modification history:
---------------------
2011 Nov 15 - Nick Elias, NRAO
              Initial stub version.

*/

// -----------------------------------------------------------------------------

void CalStatsPhase::unwrap( Cube<Float>& oPhase,
    const Vector<Float>& oFrequency, const Cube<Bool>& oFlag ) {

  // Return if the length of the FREQUENCY axis is unity or if all of the phases
  // are flagged

  if ( oPhase.shape()[CalStats::FREQUENCY] <= 1 ) return;

  if ( allEQ( oFlag, True ) ) return;


  // Unwrap the phases along the frequency axis

  cout << oPhase << endl << flush;     // To eliminate warning messages
  cout << oFrequency << endl << flush; // To eliminate warning messages
  cout << oFlag << endl << flush;      // To eliminate warning messages


  // Return

  return;

}

// -----------------------------------------------------------------------------
// End of CalStatsPhase public static member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// End of CalStatsPhase class
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------
