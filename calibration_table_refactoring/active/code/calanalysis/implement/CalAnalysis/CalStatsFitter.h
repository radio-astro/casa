
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
              all of their duties are subsumed by the nested class FIT (it used
              used to be a structure).

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
      TYPE_INIT=-1,LSQ=0, ROBUST
    } TYPE;

    // Weight enums
    typedef enum WEIGHT {
      WEIGHT_INIT=-1,NO=False, YES=True
    } WEIGHT;

    // FIT nested class
    class FIT {
      public:
        ORDER eOrder;           // Fit order
        TYPE eType;             // Fit type
        WEIGHT eWeight;         // Fit weight
        Bool bValid;            // Fit validity boolean
        Vector<Float> oPars;    // Fit parameters
        Matrix<Double> oCovars; // Fit parameter covariances
        Vector<Float> oModel;   // Fit model
        Vector<Float> oRes;     // Fit residuals
        Float fRedChi2;         // Fit reduced chi^2
        FIT( void );
        FIT( const FIT& oFit );
        ~FIT( void );
        FIT& operator=( const FIT& oFit );
    };

    // User interface to all fitting capabilities
    static FIT& fit( const Vector<Float>& oAbs, const Vector<Float>& oData,
        const Vector<Float>& oDataErr, Vector<Bool>& oFlag, const ORDER& eOrder,
        const TYPE& eType, const WEIGHT& eWeight );

  private:

    // Least-squares fitting function
    static FIT& lsqFit( const Vector<Float>& oAbs, const Vector<Float>& oData,
        const Vector<Float>& oDataErr, Vector<Bool>& oFlag, const ORDER& eOrder,
        const WEIGHT& eWeight );

    // Robust fitting function
    static FIT& robustFit( const Vector<Float>& oAbs,
        const Vector<Float>& oData, const Vector<Float>& oDataErr,
        Vector<Bool>& oFlag, const ORDER& eOrder, const WEIGHT& eWeight,
        const Float& fTrim );

    // Functions required to calculate a robust linear fit
    static Vector<Float>& numDataWeight( const Vector<Float>& oVector,
        const Vector<Float>& oDataErr );
    static Float& slope( const Vector<Float>& oAbs, const Vector<Float>& oData,
        const Float& fSlope, const Float& fSlopeErr, const Float& fFudge,
        const uInt& uiNumSlope, const uInt& uiNumIter );
    static Float& brackFunc( const Vector<Float>& oAbs,
        const Vector<Float>& oData, const Float& fSlope );

    // Signum functions (also required to calculate a robust linear fit)
    static Float& signum( const Float& fValue );
    static Vector<Float>& signum( const Vector<Float>& oValue );

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
