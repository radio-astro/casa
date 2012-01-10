
// -----------------------------------------------------------------------------

/*

CalStatsFitter.cc

Description:
------------
This file contains member functions for the CalStatsFitter class.

Classes:
--------
CalStatsFitter - This class contains the fit statistics static enums and
functions.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <calanalysis/CalAnalysis/CalStatsFitter.h>

// -----------------------------------------------------------------------------
// Start of casa namespace
// -----------------------------------------------------------------------------

namespace casa {

// -----------------------------------------------------------------------------
// Start of CalStatsFitter class
// -----------------------------------------------------------------------------

/*

CalStatsFitter

Description:
------------
This class contains the fit statistics static enums and functions.

Classes:
--------
CalStatsFitter - This class contains the fit statistics enums and functions.

Public static member functions:
-------------------------------
init - This member function initializes a CalStatsFitter::FIT structure.

Private static member functions (least-squares fitting):
--------------------------------------------------------
lsqFit - This member function calculates polynomial least-squares fits.

Private static member functions (robust fitting):
-------------------------------------------------
robustFit     - This member function calculates polynomial robust fits.
numDataWeight - This member function creates a new vector with multiple copies
                of elements from the original vector.
slope         - This member function calculates the robust slope.
brackFunc     - This member function is root-finding bracketing function used to
                determine the slope.
signum        - This member function calculates the signum function (scalar).
signum        - This member function calculates the signum function (vector).

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version created with enums ORDER, TYPE, and WEIGHT; public
              static member functions init() and fit(); and private static
              member functions lsqFit(), robustFit(), numDataWeight(), slope(),
              brackFunc(), signum() (scalar), and signum() (vector).
2011 Dec 11 - Nick Elias, NRAO
              Added data, data error, and model vectors to the FIT structure.
              Added dealloc() (pointer) and dealloc() (reference) public static
              member functions.
2011 Dec 21 - Nick Elias, NRAO
              Public static member functions init() and dealloc() removed because
              all of their duties are subsumed by the nested class FIT.

*/

// -----------------------------------------------------------------------------
// Start of CalStatsFitter public static member functions
// -----------------------------------------------------------------------------

/*

CalStatsFitter::fit

Description:
------------
This member function is the fitting interface of this class.

NB: If there are no errors, the data error vector should contain all 1.0
elements.

NB: There is no robust quadratic fit available.

NB: The trim factor required for member function robustFit() is set to 5.0.  It
is not part of the argument list for this member function.

Inputs:
-------
oAbs     - This Vector<Float> object contains the abscissae.
oData    - This Vector<Float> object contains the data.
oDataErr - This Vector<Float> object contains the data errors.
oFlag    - This Vector<Bool> object contains the flags.
eOrder   - This CalStatsFitter::ORDER enum contains the fit order
           (CalStatsFitter::AVERAGE = average fit, CalStatsFitter::LINEAR =
           linear fit, CalStatsFitter::QUADRATIC = quadratic fit).
eType    - This CalStatsFitter::TYPE enum contains the fit type
           (CalStatsFitter::LSQ = least squares,
           CalStatsFitter::ROBUST = robust).
eWeight  - This CalStatsFitter::WEIGHT enum contains the weight flag
           (CalStatsFitter::YES = apply weights, CalStatsFitter::NO = don't apply
           weights).

Outputs:
--------
oFlag - This Vector<Bool> object contains the flags.
The reference to the FIT structure, returned via the function value.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStatsFitter::FIT& CalStatsFitter::fit( const Vector<Float>& oAbs,
    const Vector<Float>& oData, const Vector<Float>& oDataErr,
    Vector<Bool>& oFlag, const ORDER& eOrder, const TYPE& eType,
    const WEIGHT& eWeight ) {

  // Initialize the pointer to the CalStatsFitter::FIT structure

  CalStatsFitter::FIT* poFit;


  // Calculate the desired fit and populate the pointer to the FIT structure

  switch ((uInt) eType) {

    case (uInt) LSQ:
      poFit = new CalStatsFitter::FIT(
          lsqFit( oAbs, oData, oDataErr, oFlag, eOrder, eWeight ) );
      poFit->eOrder = eOrder;
      poFit->eType = eType;
      poFit->eWeight = eWeight;
      break;

    case (uInt) ROBUST:
      switch ((uInt) eOrder) {
        case (uInt) AVERAGE:
        case (uInt) LINEAR:
          poFit = new CalStatsFitter::FIT(
              robustFit( oAbs, oData, oDataErr, oFlag, eOrder, eWeight, 5.0 ) );
          poFit->eOrder = eOrder;
          poFit->eType = eType;
          poFit->eWeight = eWeight;
	  break;
        case (uInt) QUADRATIC:
        default:
          break;
      }
      break;

    default:
      break;

  }


  // Return the reference to the FIT structure

  return( *poFit );

}

// -----------------------------------------------------------------------------
// End of CalStatsFitter public static member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalStatsFitter private static member functions
// -----------------------------------------------------------------------------

/*

CalStatsFitter::lsqFit

Description:
------------
This member function calculates polynomial least-squares fits.

NB: The linear (coefficients times basis functions) SVD fit is performed by
class LinearFitSVD in LinearFitSVD.h.  This class also requires the Polynomial
class in Polynomial.h and the AutoDiff class in AutoDiff.h.

Inputs:
-------
oAbs     - This Vector<Float> object contains the abscissae.
oData    - This Vector<Float> object contains the data.
oDataErr - This Vector<Float> object contains the data errors.
oFlag    - This Vector<Bool> object contains the flags.
eOrder   - This CalStatsFitter::ORDER enum determines the fit order
           (CalStatsFitter::AVERAGE = average fit,
           CalStatsFitter::LINEAR = linear fit,
           CalStatsFitter::QUADRATIC = quadratic fit).
eWeight  - This CalStatsFitter::WEIGHT enum is the weight flag
           (CalStatsFitter::YES = apply weights, CalStatsFitter::NO = don't
           apply weights).

Outputs:
--------
oFlag - This Vector<Bool> object contains the flags.
The reference to the FIT structure, returned via the function value.  If an
error occurrs the FIT structure will contain only the initial values and no
error message will be printed.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version.
2011 Dec 21 - Nick Elias, NRAO
              Converted Float fit to Double fit and scaled abscissa, to minimize
              round-off errors.

*/

// -----------------------------------------------------------------------------

CalStatsFitter::FIT& CalStatsFitter::lsqFit( const Vector<Float>& oAbs,
    const Vector<Float>& oData, const Vector<Float>& oDataErr,
    Vector<Bool>& oFlag, const ORDER& eOrder, const WEIGHT& eWeight ) {

  // Get the length of the input arrays

  uInt uiNumData = oAbs.nelements();


  // Eliminate the flagged abscissae, data, and data errors

  MaskedArray<Float> oAbsM( oAbs, !oFlag );
  Vector<Float> oAbsC( oAbsM.getCompressedArray() );

  MaskedArray<Float> oDataM( oData, !oFlag );
  Vector<Float> oDataC( oDataM.getCompressedArray() );

  MaskedArray<Float> oDataErrM( oDataErr, !oFlag );
  Vector<Float> oDataErrC( oDataErrM.getCompressedArray() );


  // Form the Double vectors

  uInt uiNumDataC = oAbsC.nelements();

  Vector<Double> oAbsD( uiNumDataC );
  for ( uInt d=0; d<uiNumDataC; d++ ) oAbsD[d] = oAbsC[d];

  Vector<Double> oDataD( uiNumDataC );
  for ( uInt d=0; d<uiNumDataC; d++ ) oDataD[d] = oDataC[d];

  Vector<Double> oDataErrD( uiNumDataC );
  for ( uInt d=0; d<uiNumDataC; d++ ) oDataErrD[d] = oDataErrC[d];


  // Check to see if there are a sufficient number of data.  The absolute
  // minimum number of data is allowed, which may or may not be a good idea
  // (TBD).

  Int iDF = ((Int) uiNumDataC) - ((Int) eOrder);

  if ( iDF <= 0 ) {
    CalStatsFitter::FIT* poFitError = new CalStatsFitter::FIT();
    return( *poFitError );
  }


  // Initialize the SVD linear fit object

  LinearFitSVD<Double> oFitter;

  oFitter.asSVD( True );
  oFitter.asWeight( (Bool) eWeight );


  // Initialize the basis functions and feed them to the SVD linear fit object

  Polynomial< AutoDiff<Double> > oBasisFuncs( (uInt) eOrder );

  for ( uInt o=0; o<=(uInt)eOrder; o++ ) oBasisFuncs[o] = 1.0;

  oFitter.setFunction( oBasisFuncs );


  // Calculate the fit, including abscissa scaling (to eliminate round off)

  Vector<Double> oParsD( (uInt) eOrder+1 );

  Double dFactor = abs( max(oAbsD) ) + abs( min(oAbsD) );
  oAbsD /= dFactor;

  Bool bValid = oFitter.fit( oParsD, oAbsD, oDataD, oDataErrD );


  // Initialize the pointer to the CalStatsFitter::FIT structure

  CalStatsFitter::FIT* poFit = new CalStatsFitter::FIT();


  // Populate the pointer to the FIT structure, including abscissa, parameter,
  // and covariance rescaling

  if ( bValid ) {

    poFit->bValid = bValid;

    poFit->oPars = Vector<Float>( (uInt) eOrder+1 );
    for ( uInt o=0; o<=(uInt)eOrder; o++ ) {
      poFit->oPars[o] = (Float) oParsD[o] / pow(dFactor,(Int)o);
    }

    poFit->oCovars = Matrix<Double>( oFitter.compuCovariance() );
    for ( uInt o1=0; o1<=(uInt)eOrder; o1++ ) {
      for ( uInt o2=0; o2<=(uInt)eOrder; o2++ ) {
        poFit->oCovars(o1,o2) /= pow(dFactor,(Int)o1) * pow(dFactor,(Int)o2);
      }
    }

    poFit->oModel = Vector<Float>( uiNumData, (Float) 0.0 );
    for ( uInt o=0; o<=(uInt)eOrder; o++ ) {
      poFit->oModel += poFit->oPars[o] * pow(oAbs,(Int)o);
    }

    poFit->oRes = Vector<Float>( oData - poFit->oModel );

    poFit->fRedChi2 = oFitter.chiSquare() / ((Float) iDF);

  }


  // Return the reference to the FIT structure

  return( *poFit );

}

// -----------------------------------------------------------------------------

/*

CalStatsFitter::robustFit

Description:
------------
This member function calculates polynomial robust fits.

NB: Unlike member function lsqFit(), this member function does not calculate
quadratic fits.

NB: All outliers will be trimmed.  This trimming will manifest itself in the
input/output abscissae, data, and data errors.

Algorithm:
----------
* Calculate a least-squares fit to get an initial estimate of the fit
  parameters using member function lsqFit().
* In the robust fitting scheme, new abscissae and data vectors are created when
  weighting is selected.
  - Calculate the robust weights (1/error).
  - Normalize the weights to the minimum weight.
  - Round the normalized weights.
  - Each normalized and rounded weight represents the number of times the
    corresponding abscissa and datum must appear in the new vectors.
  - These new vectors are created by the numDataWeight() member function.
* Using the initial estimate of the fit parameters (and new abscissae and data
  vectors if weighting is selected), calculate the robust fit parameters by
  minimizing the absolute deviation.
  - If the order is zero, the median is calculated (in ArrayMath.h).  Forming
    the median is essentially a sorting problem.
  - If the order is one, the slope and intercept are calculated.  The algorithm
    may be found in "Numerical Recipes in C" (in the first edition, section 14.6
    on pages 562-563).  Forming the slope and intercept is essentially a root
    finding problem performed by the slope() member function.  The median (in
    ArrayMath.h) is used as well.  Some input parameters of the slope() member
    function are set by this member function and do not appear in its input
    parameter list.
      # The search range for the robust slope 10.0 times the standard deviation
        of the initial least-squares fit.
      # The number of test slopes within an iteration is set to 20.
      # The number of iterations is set to 30.
* The robust model and residuals are calculated.
* A boolean flag vector is formed.  False elements correspond to absolute
  residuals less than fTrim (an input parameter of this member function) times
  the mean absolute deviation (avdev() function in ArrayMath.h) of the
  residuals.  True elements, of course, correspond to the opposite case.
* The abscissae, data, and data error vectors have outlier points removed by
  creating masked arrays (MaskedArray class in MaskedArray.h) using the flag
  vector.
* The masked arrays are converted back to vectors without outliers.
* Calculate a "trimmed" least-squares fit to get the final estimate of the fit
  parameters and their covariance matrix.

Inputs:
-------
oAbs     - This Vector<Float> object contains the abscissae.
oData    - This Vector<Float> object contains the data.
oDataErr - This Vector<Float> object contains the data errors.
oFlag    - This Vector<Bool> object contains the flags.
eOrder   - This CalStatsFitter::ORDER enum contains the fit order
           (CalStatsFitter::AVERAGE = average fit, CalStatsFitter::LINEAR =
           linear fit).
eWeight  - This CalStatsFitter::WEIGHT enum contains the weight flag
           (CalStatsFitter::YES = apply weights, CalStatsFitter::NO = don't apply
           weights).
fTrim    - This Float contains the dimensionless trim factor.

Outputs:
--------
oFlag - This Vector<Bool> object contains the flags.
The reference to the FIT structure containing the trimmed least-squares fit,
returned via the function value.  If an error occurrs the FIT structure will
contain only the initial values and no error message will be printed.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStatsFitter::FIT& CalStatsFitter::robustFit( const Vector<Float>& oAbs,
    const Vector<Float>& oData, const Vector<Float>& oDataErr,
    Vector<Bool>& oFlag, const ORDER& eOrder, const WEIGHT& eWeight,
    const Float& fTrim ) {

  // Calculate the least squares fit as an initial estimate to the MAD
  // minimization robust fit

  CalStatsFitter::FIT* poFitLSQ = new CalStatsFitter::FIT(
      lsqFit( oAbs, oData, oDataErr, oFlag, eOrder, eWeight ) );

  if ( !poFitLSQ->bValid ) return( *poFitLSQ );


  // If selected apply the robust weighting, otherwise do not.  See the ICD of
  // this member function for more details.

  Vector<Float> oAbsNew( oAbs );
  Vector<Float> oDataNew( oData );

  if ( (Bool) eWeight ) {
    oAbsNew = numDataWeight( oAbsNew, oDataErr );
    oDataNew = numDataWeight( oDataNew, oDataErr );
  }


  // Minimizing the mean absolute deviations, calculate the robust fit
  // parameters

  Float f1, f0;
  Float fSlopeLSQ, fSlopeErrLSQ; // Declaration here is a kludge

  CalStatsFitter::FIT* poFitError = new CalStatsFitter::FIT();

  switch ((uInt) eOrder) {

    case (uInt) AVERAGE:
      f1 = 0.0;
      f0 = median( oDataNew );
      break;

    case (uInt) LINEAR:
      fSlopeLSQ = poFitLSQ->oPars( 1 );
      fSlopeErrLSQ = (Float) sqrt( poFitLSQ->oCovars( 1, 1 ) );
      try {
        f1 = slope( oAbsNew, oDataNew, fSlopeLSQ, fSlopeErrLSQ, 10.0, 20, 30 );
	f0 = median( oDataNew - f1*oAbsNew );
      }
      catch( char* acMsg ) {
        return( *poFitError );
      }
      break;

    default:
      return( *poFitError );
      break;

  }


  // Deallocate the memory for the initial estimates

  delete poFitLSQ;


  // Trim the data to make them more robust.  See the ICD of this member
  // function for more details.

  Vector<Float> oModel = Vector<Float>( f1*oAbs + f0 );
  Vector<Float> oRes = Vector<Float>( oData - oModel );

  Vector<Bool> oFlagTrim( fabs(oRes) >= fTrim*avdev(oRes) );
  oFlag = oFlag || oFlagTrim;


  // Calculate the "trimmed" least-squares fit 

  CalStatsFitter::FIT* poFitTrim;
  poFitTrim = new CalStatsFitter::FIT(
      lsqFit( oAbs, oData, oDataErr, oFlag, eOrder, eWeight ) );


  // Return the reference to the FIT structure (trimmed least-squares fit)

  return( *poFitTrim );

}

// -----------------------------------------------------------------------------

/*

CalStatsFitter::numDataWeight

Description:
------------
This member function creates a new vector with multiple copies of elements from
the original vector.

Weighted Algorithm:
-------------------
* Form the weights (1.0 divided by the data errors).
* Divide the weights by the minimum weight.
* Round the normalized weights to their nearest integer.  Each value represents
  the number of times the corresponding element in the old vector appears in the
  new vector.
* Sum the normalized weights, which is the size of the new vector.
* For each element in the original data, put the new number of them into the new
  data array.  For example, if an element in the data array is 1.5 and its
  corresponding number is 3, put three 1.5 values into the new data array.

Inputs:
-------
oVector  - This Vector<Float> object contains a vector to be weighted.
oDataErr - This Vector<Float> object contains the data errors.

Outputs:
--------
The reference to the new Vector<Float> object, returned via the function value.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<Float>& CalStatsFitter::numDataWeight( const Vector<Float>& oVector,
    const Vector<Float>& oDataErr ) {

  // Calculate the weights, normalize them to the minimum weight, and round each
  // of them to the nearest integer.  Each value represents the number of times
  // the corresponding element of the old vector appears in the new vector.

  Vector<Float> oWeight( (Float) 1.0 / oDataErr );
  oWeight = floor( oWeight/min(oWeight) + (Float) 0.5 );


  // Form the new vector whose size is the sum of the normalized and rounded
  // weights

  uInt uiNumVector = oVector.nelements();
  uInt uiNumVectorNew = (uInt) sum( oWeight );

  Vector<Float>* poVectorNew = new Vector<Float>( uiNumVectorNew );

  for ( uInt v=0,vNew=0; v<uiNumVector; v++ ) {
    for ( uInt vW=0; vW<(uInt)oWeight[v]; vW++,vNew++ ) {
      (*poVectorNew)[vNew] = oVector[v];
    }
  }


  // Return the reference to the new vector

  return( *poVectorNew );

}

// -----------------------------------------------------------------------------

/*

CalStatsFitter::slope

Description:
------------
This member function calculates the robust slope.

NB: For the stopping criterion to work reliably, the number of slopes input
parameter should be set to at least ~ 10.

Algorithm:
----------
* Set the initial minimum and maximum slope search range based on the estimated
  slope and its error times the dimensionless fudge factor.
  - Example: slope = 5, slope error = 1, fudge factor = 2 --> initial slope
    search range is 3 to 7.
* Outer iteration loop:
  - Divide up the slope range into "subslopes" according to the number of slopes
    input parameter (2 = bisection, but a value greater than ~ 10 should be used
    for the stopping criterion to work reliably).
  - Calculate the bracketing function (brackFunc() member function) for each
    subslope.
  - Inner subslope loop:
    # Look for a sign change in the bracketing function between the minimum
      subslope of the iteration and the present subslope (scalar signum()
      member function).
    # If there is no sign change, go to the next subslope.
    # If there is a sign change, reset the maximum slope search range to the
      present subslope, reset the minimum slope search range to the previous
      subslope, and break out of the subslope loop.
  - If this is the first iteration, check if the slope has been bracketed.  If
    not, throw an exception.
  - Check for convergence.  I only do this when more than half of the iterations
    have been completed because I want to insure that the average slope is a
    reasonable value.  This strategy is not strictly optimum, since more
    iterations will always be calculated than necessary.  Given the reliability
    and the nominal sizes of vectors containing calibration, I don't expect it
    to be an issue.

Inputs:
-------
oAbs       - This Vector<Float> object contains the abscissae.
oData      - This Vector<Float> object contains the data.
fSlope     - This Float variable contains the initial slope estimate.
fSlopeErr  - This Float variable contains the initial slope error estimate.
fFudge     - This Float variable contains the dimensionless fudge factor.
uiNumSlope - This uInt variable contains the number of subslopes per iteration.
uiNumIter  - This uInt variable contains the number of iterations.

Outputs:
--------
The reference to the Float variable that contains the robust slope, returned via
the function value.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Float& CalStatsFitter::slope( const Vector<Float>& oAbs,
    const Vector<Float>& oData, const Float& fSlope, const Float& fSlopeErr,
    const Float& fFudge, const uInt& uiNumSlope, const uInt& uiNumIter ) {

  // Initialize the slope range

  Float fSlopeMin = fSlope - fFudge*fSlopeErr;
  Float fSlopeMax = fSlope + fFudge*fSlopeErr;


  // Perform the bisections iteratively to refine the robust slope range bracket

  for ( uInt i=0; i<uiNumIter; i++ ) {

    Bool bFlag = False;

    Float fSlopeInt = (fSlopeMax-fSlopeMin) / ((Float) uiNumSlope);

    Vector<Float> oIndgen( uiNumSlope+1 ); indgen( oIndgen );
    Vector<Float> oSlopes( oIndgen*fSlopeInt + fSlopeMin );

    Float fValueMin = brackFunc( oAbs, oData, oSlopes[0] );

    for ( uInt s=1; s<=uiNumSlope; s++ ) {
      Float fValue = brackFunc( oAbs, oData, oSlopes[s] );
      if ( fValue == 0.0 ) return( oSlopes[s] );
      if ( signum( fValue ) != signum( fValueMin ) ) {
        bFlag = True;
	fSlopeMin = oSlopes[s] - fSlopeInt;
	fSlopeMax = oSlopes[s];
	break;
      }
    }

    if ( !bFlag && i==0 ) {
      throw( AipsError( "Robust slope has not been bracketed." ) );
    }

    if ( i >= uiNumIter/2 ) {
      Float fSlopeDiff = fabs( fSlopeMax - fSlopeMin );
      Float fSlopeAve = fabs( 0.5 * ( fSlopeMax + fSlopeMin ) );
      if ( fSlopeDiff/fSlopeAve <= 1.0E-08 ) break;
    }

  }


  // Calculate the robust slope estimate (average of the last range bracket) and
  // return the reference to it

  Float* pfSlopeEst;
  *pfSlopeEst = 0.5 * ( fSlopeMin + fSlopeMax );

  return( *pfSlopeEst );

}

// -----------------------------------------------------------------------------

/*

CalStatsFitter::brackFunc

Description:
------------
This member function is root-finding bracketing function used to determine the
slope.

NB: The algorithm may be found in "Numerical Recipes in C" (in the first
edition, section 14.6 on pages 562-563).

Inputs:
-------
oAbs   - This Vector<Float> object contains the abscissae.
oData  - This Vector<Float> object contains the data.
fSlope - This Float variable contains the slope estimate.

Outputs:
--------
The reference to the Float variable containing the bracketing function value,
returned via the function value.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Float& CalStatsFitter::brackFunc( const Vector<Float>& oAbs,
    const Vector<Float>& oData, const Float& fSlope ) {

  // Calculate the median (from Arraymath.h), which corresponds to the y offset

  Float fOffset = median( oData - fSlope*oAbs );


  // Calculate the bracketing function value and return the reference to it

  Float* pfBrackFunc;
  *pfBrackFunc = sum( oAbs * signum( oData - fSlope*oAbs - fOffset ) );

  return( *pfBrackFunc );

}

// -----------------------------------------------------------------------------

/*

CalStatsFitter::signum (scalar)

Description:
------------
This member function calculates the signum function.

Inputs:
-------
fValue - This Float variable contains the input value.

Outputs:
--------
The reference to the Float variable containing the signum value, returned via
the function value.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Float& CalStatsFitter::signum( const Float& fValue ) {

  // Calculate the signum value

  Float* pfSignum;

  if ( fValue == 0.0 ) {
    *pfSignum = 0.0;
  } else if ( fValue > 0.0 ) {
    *pfSignum = 1.0;
  } else {
    *pfSignum = -1.0;
  }


  // Return the reference to the signum value

  return( *pfSignum );

}

// -----------------------------------------------------------------------------

/*

CalStatsFitter::signum (vector)

Description:
------------
This member function calculates the signum function.

Inputs:
-------
oValue - This Vector<Float> object contains the input vector values.

Outputs:
--------
The reference to the Vector<Float>& object containing the signum values,
returned via the function value.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<Float>& CalStatsFitter::signum( const Vector<Float>& oValue ) {

  // Calculate the signum values for all of the input vector values

  uInt uiNumValue = oValue.nelements();
  Vector<Float>* poSignum = new Vector<Float>( uiNumValue );

  for ( uInt v=0; v<uiNumValue; v++ ) {
    (*poSignum)[v] = signum( oValue[v] );
  }


  // Return the reference to the vector that contains the signum values

  return( *poSignum );

}

// -----------------------------------------------------------------------------
// End of CalStatsFitter private static member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// End of CalStatsFitter class
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------
