
// -----------------------------------------------------------------------------

/*

CalStatsFitter.h

Description:
------------
This header file contains definitions for the CalStatsFitter class.

Classes:
--------
CalStatsFitter - This class contains the fit statistics static enums and
functions.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version created.
2012 Jan 25 - Nick Elias, NRAO
              Error checking added.

*/

// -----------------------------------------------------------------------------
// Start of define macro to prevent multiple loading
// -----------------------------------------------------------------------------

#ifndef CAL_STATS_FITTER_H
#define CAL_STATS_FITTER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <casa/aips.h>

#include <casa/Exceptions/Error.h>

#include <casa/Arrays/Vector.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>

#include <casa/Arrays/MaskedArray.h>

#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Fitting/LinearFitSVD.h>

// -----------------------------------------------------------------------------
// Start of casa namespace
// -----------------------------------------------------------------------------

namespace casa {

// -----------------------------------------------------------------------------
// Start of CalStatsFitter class definition
// -----------------------------------------------------------------------------

/*

CalStatsFitter

Description:
------------
This class contains the fit statistics static enums and functions.

NB: At present, this class acts as a namespace for static functions.

Classes:
--------
CalStatsFitter - This class contains the fit statistics enums and functions.

Class public static member functions:
-------------------------------------
fit        - This member function is the fitting interface of this class.
orderName  - This function returns the string corresponding to the
             CalStatsFitter::ORDER enum.
typeName   - This function returns the string corresponding to the
             CalStatsFitter::TYPE enum.
weightName - This function returns the string corresponding to the
             CalStatsFitter::WEIGHT enum.

Class private static member functions (least-squares fitting):
--------------------------------------------------------------
lsqFit - This member function calculates polynomial least-squares fits.

Class private static member functions (robust fitting):
-------------------------------------------------------
robustFit - This member function calculates polynomial robust fits.
slope     - This member function calculates the robust slope.
brackFunc - This member function is root-finding bracketing function used to
            determine the slope.
signum    - This member function calculates the signum function (scalar).
signum    - This member function calculates the signum function (vector).
theil     - This member function estimates the slope and slope error using
            Theil's method.

Modification history:
---------------------
2011 Dec 08 - Nick Elias, NRAO
              Initial version created with enums ORDER, TYPE, and WEIGHT; public
              static member functions init() and fit(); and private static
              member functions lsqFit(), robustFit(), numDataWeight(), slope(),
              brackFunc(), signum() (scalar), and signum() (vector).
2011 Dec 11 - Nick Elias, NRAO
              Added value, value error, and model vectors to the FIT structure.
              Added dealloc() (pointer) and dealloc() (reference) public static
              member functions.
2011 Dec 21 - Nick Elias, NRAO
              Public static member functions init() and dealloc() removed
              because all of their duties are subsumed by the nested class
              FIT (it used used to be a structure).
2012 Jan 24 - Nick Elias, NRAO
              Private static member function theil() added.  Private static
              member function numDataWeight() removed because initial robust
              estimates of fit parameters (before final "trimmed" least squares)
              no longer employ weighting.
2012 Mar 06 - Nick Elias, NRAO
              Static public member functions orderName(), typeName(), and
              weightName() added.
2012 Mar 15 - Nick Elias, NRAO
              Public members dResVar and dResMean added to the nested FIT class.

*/

// -----------------------------------------------------------------------------

class CalStatsFitter {

  public:

    // Order enums
    typedef enum ORDER {
      ORDER_INIT=-1, AVERAGE=0, LINEAR, QUADRATIC
    } ORDER;

    // Type enums
    typedef enum TYPE {
      TYPE_INIT=-1, LSQ=0, ROBUST
    } TYPE;

    // Weight enums
    typedef enum WEIGHT {
      WEIGHT_INIT=-1, NO=False, YES=True
    } WEIGHT;

    // FIT nested class
    class FIT {
      public:
        ORDER eOrder;           // Fit order
        TYPE eType;             // Fit type
        WEIGHT eWeight;         // Fit weight
        Bool bValid;            // Fit validity boolean
        Vector<Double> oPars;   // Fit parameters
        Matrix<Double> oCovars; // Fit parameter covariances
        Vector<Double> oModel;  // Fit model
        Vector<Double> oRes;    // Fit residuals
        Double dResVar;         // Fit variance of residuals
        Double dResMean;        // Fit mean of residuals
        Double dRedChi2;        // Fit reduced chi^2
        FIT( void );
        FIT( const FIT& oFit );
        ~FIT( void );
        FIT& operator=( const FIT& oFit );
    };

    // User interface to all fitting capabilities
    static FIT& fit( const Vector<Double>& oAbs, const Vector<Double>& oValue,
        const Vector<Double>& oValueErr, Vector<Bool>& oFlag,
        const ORDER& eOrder, const TYPE& eType, const WEIGHT& eWeight );

    // The enum names
    static String& orderName( const ORDER& eOrder );
    static String& typeName( const TYPE& eOrder );
    static String& weightName( const WEIGHT& eOrder );

  private:

    // Least-squares fitting function
    static FIT& lsqFit( const Vector<Double>& oAbs,
        const Vector<Double>& oValue, const Vector<Double>& oValueErr,
        Vector<Bool>& oFlag, const ORDER& eOrder, const WEIGHT& eWeight );

    // Robust fitting function
    static FIT& robustFit( const Vector<Double>& oAbs,
        const Vector<Double>& oValue, const Vector<Double>& oValueErr,
        Vector<Bool>& oFlag, const ORDER& eOrder, const WEIGHT& eWeight,
        const Double& dTrim );

    // Functions required to calculate a robust linear fit
    static Double& slope( const Vector<Double>& oAbs,
        const Vector<Double>& oValue, const Double& dSlope,
        const Double& dSlopeErr, const Double& dFudge, const uInt& uiNumSlope,
        const uInt& uiNumIter );
    static Double& brackFunc( const Vector<Double>& oAbs,
        const Vector<Double>& oValue, const Double& dSlope );

    // Signum functions (also required to calculate a robust linear fit)
    static Double& signum( const Double& dValue );
    static Vector<Double>& signum( const Vector<Double>& oValue );

    // Function for calculating slope estimate using Theil's method
    static void theil( const Vector<Double>& oAbs, const Vector<Double>& oValue,
        Double& dSlope, Double& dSlopeErr );

};

// -----------------------------------------------------------------------------
// End of CalStatsFitter class definition
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------
// End of define macro to prevent multiple loading
// -----------------------------------------------------------------------------
