
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
2012 Jan 25 - Nick Elias, NRAO
              Logging capability added.  Error checking added.

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
oData           - This reference to a Cube<Double> instance contains the data.
oDataErr        - This reference to a Cube<Double> instance contains the data
                  errors.
oFlag           - This reference to a Cube<Bool> instance contains the flags.
oFeed           - This reference to a Vector<String> instance is the feed
                  abscissae.
oFrequency      - This reference to a Vector<Double> instance is the frequency
                  abscissae.
oTime           - This reference to a Vector<Double> instance is the time
                  abscissae.
eAxisIterUserID - This reference to a CalStats::AXIS enum contains either the
                  FREQUENCY or TIME iteration axes (user defined).

Outputs:
--------
None.

Modification history:
---------------------
2011 Dec 11 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Error checking added.

*/

// -----------------------------------------------------------------------------

CalStatsReal::CalStatsReal( const Cube<Double>& oData,
    const Cube<Double>& oDataErr, const Cube<Bool>& oFlag,
    const Vector<String>& oFeed, const Vector<Double>& oFrequency,
    const Vector<Double>& oTime, const CalStats::AXIS& eAxisIterUser )
    : CalStats() {

  // Create an instance of the CalStats base class constructor and copy its
  // state to this CalStatsReal instance

  CalStats* poCS;

  try {
    poCS = new CalStats( oData, oDataErr, oFlag, oFeed, oFrequency, oTime,
        eAxisIterUser );
  }
  catch ( AipsError oAE ) {
    throw( oAE );
  }

  oAxisIterID = IPosition( poCS->axisIterID() );
  eAxisNonIterID = poCS->axisNonIterID();

  oAxisIterFeed = Vector<String>( poCS->axisIterFeed() );
  oAxisIterUser = Vector<Double>( poCS->axisIterUser() );
  oAxisNonIter = Vector<Double>( poCS->axisNonIter() );

  oStatsShape = IPosition( poCS->statsShape() );

  poData = new Cube<Double>( poCS->data() );
  poDataErr = new Cube<Double>( poCS->dataErr() );
  poFlag = new Cube<Bool>( poCS->flag() );

  poDataIter = new ArrayIterator<Double>( *poData, oAxisIterID, False );
  poDataIter->reset();

  poDataErrIter = new ArrayIterator<Double>( *poDataErr, oAxisIterID, False );
  poDataErrIter->reset();

  poFlagIter = new ArrayIterator<Bool>( *poFlag, oAxisIterID, False );
  poFlagIter->reset();

  delete poCS;


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
oData           - This reference to a Cube<DComplex> instance contains the data.
oDataErr        - This reference to a Cube<Double> instance contains the data
                  errors (real and imaginary parts).
oFlag           - This reference to a Cube<Bool> instance contains the flags.
oFeed           - This reference to a Vector<String> instance is the feed
                  abscissae.
oFrequency      - This reference to a Vector<Double> instance is the frequency
                  abscissae.
oTime           - This reference to a Vector<Double> instance is the time
                  abscissae.
eAxisIterUserID - This reference to a CalStats::AXIS enum contains either the
                  FREQUENCY or TIME iteration axes (user defined).
bNorm           - This reference to a Bool variable contains the normalization
                  flag (True = normalize, False = don't normalize).

Outputs:
--------
None.

Modification history:
---------------------
2011 Nov 15 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Error checking added.
2012 Feb 15 - Nick Elias, NRAO
              Data error input parameter changed from DComplex to Double.

*/

// -----------------------------------------------------------------------------

CalStatsAmp::CalStatsAmp( const Cube<DComplex>& oData,
    const Cube<Double>& oDataErr, const Cube<Bool>& oFlag,
    const Vector<String>& oFeed, const Vector<Double>& oFrequency,
    const Vector<Double>& oTime, const CalStats::AXIS& eAxisIterUser,
    const Bool& bNorm ) : CalStats() {

  // Calculate the amplitudes and their errors

  Cube<Double> oAmp( amplitude( oData ) );

  Cube<Double> oAmpErr = oDataErr.copy();


  // Normalize the amplitudes and their errors, if selected.  The input flag
  // cube cannot be modified, so make a local copy.

  Cube<Bool> oFlagCopy( oFlag.copy() );

  if ( bNorm ) norm( oAmp, oAmpErr, oFlagCopy );


  // Create an instance of the CalStats base class constructor and copy its
  // state to this CalStatsAmp instance

  CalStats* poCS;

  try {
    poCS = new CalStats( oAmp, oAmpErr, oFlagCopy, oFeed, oFrequency, oTime,
        eAxisIterUser );
  }
  catch ( AipsError oAE ) {
    throw( oAE );
  }

  oAxisIterID = IPosition( poCS->axisIterID() );
  eAxisNonIterID = poCS->axisNonIterID();

  oAxisIterFeed = Vector<String>( poCS->axisIterFeed() );
  oAxisIterUser = Vector<Double>( poCS->axisIterUser() );
  oAxisNonIter = Vector<Double>( poCS->axisNonIter() );

  oStatsShape = IPosition( poCS->statsShape() );

  poData = new Cube<Double>( poCS->data() );
  poDataErr = new Cube<Double>( poCS->dataErr() );
  poFlag = new Cube<Bool>( poCS->flag() );

  poDataIter = new ArrayIterator<Double>( *poData, oAxisIterID, False );
  poDataIter->reset();

  poDataErrIter = new ArrayIterator<Double>( *poDataErr, oAxisIterID, False );
  poDataErrIter->reset();

  poFlagIter = new ArrayIterator<Bool>( *poFlag, oAxisIterID, False );
  poFlagIter->reset();

  delete poCS;


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

NB: All flags corresponding to amplitudes less then 1.0E-08 times the peak
amplitude (along the FREQUENCY axis) are updated to True.

Inputs:
-------
oAmp    - This reference to a Cube<Double> instance contains the amplitudes.
oAmpErr - This reference to a Cube<Double> instance contains the amplitude
          errors.
oFlag   - This reference to a Cube<Bool> instance contains the flags.

Outputs:
--------
oAmp    - This reference to a Cube<Double> instance contains the amplitudes.
oAmpErr - This reference to a Cube<Double> instance contains the amplitude
          errors.
oFlag   - This reference to a Cube<Bool> instance contains the flags.

Modification history:
---------------------
2011 Dec 11 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Logging capability added.

*/

// -----------------------------------------------------------------------------

void CalStatsAmp::norm( Cube<Double>& oAmp, Cube<Double>& oAmpErr,
    Cube<Bool>& oFlag ) {

  // Return if the length of the FREQUENCY axis is unity

  if ( oAmp.shape()[CalStats::FREQUENCY] <= 1 ) {
    LogIO log( LogOrigin( "CalStatsAmp", "norm", WHERE ) );
    log << LogIO::WARN
        << "Frequency axis has a dimension of 1, no normalization"
        << LogIO::POST;
    return;
  }


  // Create iterators for the amplitude, amplitude error, and flag cubes

  IPosition oIterShape( 2, (ssize_t) CalStats::FEED, (ssize_t) CalStats::TIME );

  ArrayIterator<Double> oAmpIter( oAmp, oIterShape, False );
  ArrayIterator<Double> oAmpErrIter( oAmpErr, oIterShape, False );
  ArrayIterator<Bool> oFlagIter( oFlag, oIterShape, False );


  // Normalize the amplitudes and their errors along the frequency axis

  while ( !oAmpIter.pastEnd() ) {

    uInt uiNumFrequency = oAmpIter.array().nelements();
    IPosition oShape( 1, uiNumFrequency );

    MaskedArray<Double> oAmpM( oAmpIter.array(), !oFlagIter.array() );
    Double dAmpMax = max( oAmpM.getCompressedArray() );

    Vector<Bool> oFlagV( oFlagIter.array().copy().reform(oShape) );
    Vector<Bool> oFlagLow( oAmpIter.array() <= (Double) 1.0E-08*dAmpMax );

    oFlagV = oFlagV || oFlagLow;
    oFlagIter.array() = oFlagV;

    Vector<Double> oAmpTemp( oAmpIter.array().copy().reform(oShape) );
    Vector<Double> oAmpErrTemp( oAmpErrIter.array().copy().reform(oShape) );

    for ( uInt f=0; f<uiNumFrequency; f++ ) {
      if ( !oFlagV[f] ) {
        oAmpTemp[f] /= dAmpMax;
	oAmpErrTemp[f] /= dAmpMax;
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
oData           - This reference to a Cube<DComplex> instance contains the data.
oDataErr        - This reference to a Cube<Double> instance contains the data
                  errors (real and imaginary parts).
oFlag           - This reference to a Cube<Bool> instance contains the flags.
oFeed           - This reference to a Vector<String> instance is the feed
                  abscissae.
oFrequency      - This reference to a Vector<Double> instance is the frequency
                  abscissae.
oTime           - This reference to a Vector<Double> instance is the time
                  abscissae.
eAxisIterUserID - This reference to a CalStats::AXIS enum contains either the
                  FREQUENCY or TIME iteration axes (user defined).
bUnwrap         - This reference to a Bool variable contains the unwrapping flag
                  (True = unwrap, False = don't unwrap).

Outputs:
--------
None.

Modification history:
---------------------
2011 Nov 15 - Nick Elias, NRAO
              Initial version. 
2011 Dec 11 - Nick Elias, NRAO
              Flag updates according to low amplitude was added.
2012 Jan 25 - Nick Elias, NRAO
              Error checking added.
2012 Feb 15 - Nick Elias, NRAO
              Data error input parameter changed from DComplex to Double.

*/

// -----------------------------------------------------------------------------

CalStatsPhase::CalStatsPhase( const Cube<DComplex>& oData,
    const Cube<Double>& oDataErr, const Cube<Bool>& oFlag,
    const Vector<String>& oFeed, const Vector<Double>& oFrequency,
    const Vector<Double>& oTime, const CalStats::AXIS& eAxisIterUser,
    const Bool& bUnwrap ) : CalStats() {

  // Calculate the phases and their initial errors (set to 0.0)

  Cube<Double> oPhase( phase(oData) );
  Cube<Double> oPhaseErr( oPhase.shape() );


  // Create the iterators.  The input flag cube is copied since it cannot be
  // modified.

  IPosition oIterShape( 2, (ssize_t) CalStats::FEED, (ssize_t) CalStats::TIME );

  ReadOnlyArrayIterator<DComplex> oDataIter( oData, oIterShape, False );
  ReadOnlyArrayIterator<Double> oDataErrIter( oDataErr, oIterShape, False );

  Cube<Bool> oFlagCopy( oFlag.copy() );
  ArrayIterator<Bool> oFlagIter( oFlagCopy, oIterShape, False );

  ArrayIterator<Double> oPhaseIter( oPhase, oIterShape, False );
  ArrayIterator<Double> oPhaseErrIter( oPhaseErr, oIterShape, False );


  // Calculate the phase errors.  They require dividing by amplitude.  Set the
  // flags according to low amplitudes (corresponding to phase errors greater
  // than M_PI).  All flagged phase errors are set to M_PI.  The iteration axes
  // are FEED and TIME.  A FREQUENCY for loop is located inside the FEED/TIME
  // iteration loop.

  while ( !oDataIter.pastEnd() ) {

    uInt uiNumFrequency = oDataIter.array().nelements();
    IPosition oShape( 1, uiNumFrequency );

    Vector<Double> oPhaseErrV( oShape );

    Vector<DComplex> oDataV( oDataIter.array().copy().reform(oShape) );
    Vector<Double> oDataErrV( oDataErrIter.array().copy().reform(oShape) );

    Vector<Double> oAmpV( amplitude(oDataV) );

    Vector<Bool> oFlagV( oFlagIter.array().copy().reform(oShape) );
    Vector<Bool> oFlagLow( oAmpV <= oDataErrV/((Double) M_PI) );

    oFlagV = oFlagV || oFlagLow;
    oFlagIter.array() = oFlagV;

    for ( uInt f=0; f<uiNumFrequency; f++ ) {
      if ( !oFlagV[f] ) {
        oPhaseErrV[f] = oDataErrV[f] / oAmpV[f];
      } else {
        oPhaseErrV[f] = (Double) M_PI;
      }
    }

    oPhaseErrIter.array() = oPhaseErrV;

    oDataIter.next(); oDataErrIter.next(); oFlagIter.next();
    oPhaseIter.next(); oPhaseErrIter.next();

  }


  // Unwrap the phases, if selected

  if ( bUnwrap ) unwrap( oPhase, oFrequency, oFlagCopy );


  // Create an instance of the CalStats base class constructor and copy its
  // state to this CalStatsPhase instance

  CalStats* poCS;

  try {
    poCS = new CalStats( oPhase, oPhaseErr, oFlagCopy, oFeed, oFrequency, oTime,
        eAxisIterUser );
  }
  catch ( AipsError oAE ) {
    throw( oAE );
  }

  oAxisIterID = IPosition( poCS->axisIterID() );
  eAxisNonIterID = poCS->axisNonIterID();

  oAxisIterFeed = Vector<String>( poCS->axisIterFeed() );
  oAxisIterUser = Vector<Double>( poCS->axisIterUser() );
  oAxisNonIter = Vector<Double>( poCS->axisNonIter() );

  oStatsShape = IPosition( poCS->statsShape() );

  poData = new Cube<Double>( poCS->data() );
  poDataErr = new Cube<Double>( poCS->dataErr() );
  poFlag = new Cube<Bool>( poCS->flag() );

  poDataIter = new ArrayIterator<Double>( *poData, oAxisIterID, False );
  poDataIter->reset();

  poDataErrIter = new ArrayIterator<Double>( *poDataErr, oAxisIterID, False );
  poDataErrIter->reset();

  poFlagIter = new ArrayIterator<Bool>( *poFlag, oAxisIterID, False );
  poFlagIter->reset();

  delete poCS;


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
oPhase     - This reference to an Cube<Double> instance contains the phases.
oFrequency - This reference to a Vector<Double> instance is the frequency
             abscissa.
oFlag      - This reference to a Cube<Bool> instance contains the flags.

Outputs:
--------
oPhase - This reference to an Cube<Double> instance contains the phases.

Modification history:
---------------------
2011 Nov 15 - Nick Elias, NRAO
              Initial stub version.
2012 Jan 25 - Nick Elias, NRAO
              Logging capability added.

*/

// -----------------------------------------------------------------------------

void CalStatsPhase::unwrap( Cube<Double>& oPhase,
    const Vector<Double>& oFrequency, const Cube<Bool>& oFlag ) {

  // Return if the length of the FREQUENCY axis is unity

  if ( oPhase.shape()[CalStats::FREQUENCY] <= 1 ) {
    LogIO log( LogOrigin( "CalStatsPhase", "unwrap", WHERE ) );
    log << LogIO::WARN
        << "Frequency axis has a dimension of 1, no unwrapping"
        << LogIO::POST;
    return;
  }


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
