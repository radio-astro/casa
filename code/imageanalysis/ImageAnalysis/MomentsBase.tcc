//# MomentsBase.cc:  base class for moment generator
//# Copyright (C) 1995,1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: MomentsBase.tcc 20652 2009-07-06 05:04:32Z Malte.Marquarding $
//   

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordFieldId.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/Quantum.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/LinearSearch.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <lattices/LatticeMath/LatticeStatsBase.h>
#include <tables/LogTables/NewFile.h>

#include <casa/sstream.h>
#include <casa/iomanip.h>

namespace casa {

template <class T> 
MomentsBase<T>::MomentsBase(
    LogIO &os, Bool overWriteOutput, Bool showProgressU
) : os_p(os), showProgress_p(showProgressU),
    overWriteOutput_p(overWriteOutput) {
    UnitMap::putUser("pix",UnitVal(1.0), "pixel units");
}

template <class T> 
MomentsBase<T>::~MomentsBase ()
{
  // do nothing
}

template <class T>
Bool MomentsBase<T>::setMoments(const Vector<Int>& momentsU)
//
// Assign the desired moments
//
{
   if (!goodParameterStatus_p) {
      error_p = "Internal class status is bad";
      return False;
   }

   moments_p.resize(0);
   moments_p = momentsU;


// Check number of moments

   uInt nMom = moments_p.nelements();
   if (nMom == 0) {
      error_p = "No moments requested";
      goodParameterStatus_p = False;
      return False;
   } else if (nMom > NMOMENTS) {
      error_p = "Too many moments specified";
      goodParameterStatus_p = False;
      return False;
   }

   for (uInt i=0; i<nMom; i++) {
      if (moments_p(i) < 0 || moments_p(i) > NMOMENTS-1) {
         error_p = "Illegal moment requested";
         goodParameterStatus_p = False;
         return False;
      }
   }
   return True;
}

template <class T>
Bool MomentsBase<T>::setWinFitMethod(const Vector<Int>& methodU)
//
// Assign the desired windowing and fitting methods
//
{

   if (!goodParameterStatus_p) {
      error_p = "Internal class status is bad";
      return False;
   }

// No extra methods set

   if (methodU.nelements() == 0) return True;


// Check legality

   for (uInt i = 0; i<uInt(methodU.nelements()); i++) {
      if (methodU(i) < 0 || methodU(i) > NMETHODS-1) {
         error_p = "Illegal method given";
         goodParameterStatus_p = False;
         return False;
      }
   }


// Assign Boooools

   linearSearch(doWindow_p, methodU, Int(WINDOW), methodU.nelements());
   linearSearch(doFit_p, methodU, Int(FIT), methodU.nelements());
   linearSearch(doAuto_p, methodU, Int(INTERACTIVE), methodU.nelements());
   doAuto_p  = (!doAuto_p);

   return True;
}

template <class T>
Bool MomentsBase<T>::setSmoothMethod(const Vector<Int>& smoothAxesU,
                                      const Vector<Int>& kernelTypesU,
                                      const Vector<Double>& kernelWidthsU)
{
   const uInt n = kernelWidthsU.nelements();
   Vector<Quantum<Double> > t(n);
   for (uInt i=0; i<n; i++) {
      t(i) = Quantum<Double>(kernelWidthsU(i),String("pix"));
   }
   return setSmoothMethod(smoothAxesU, kernelTypesU, t);
}

template <class T>  
void MomentsBase<T>::setInExCludeRange(
    const Vector<T>& includeU, const Vector<T>& excludeU
) {
   ThrowIf(
       ! goodParameterStatus_p,
       "Internal class status is bad"
   );
   _setIncludeExclude(
       selectRange_p, noInclude_p, noExclude_p,
       includeU, excludeU
   );
}

template <class T> void MomentsBase<T>::setSnr(
    const T& peakSNRU, const T& stdDeviationU
) {
//
// Assign the desired snr.  The default assigned in
// the constructor is 3,0
//
   ThrowIf(
       ! goodParameterStatus_p,
       "Internal class status is bad"
   );
   peakSNR_p = peakSNRU <= 0.0 ? T(3.0) : peakSNRU;
   stdDeviation_p = stdDeviationU <= 0.0 ? 0.0 : stdDeviationU;
} 




template <class T>
Bool MomentsBase<T>::setSmoothOutName(const String& smoothOutU) 
//
// Assign the desired smoothed image output file name
// 
{ 
   if (!goodParameterStatus_p) {
      error_p = "Internal class status is bad";
      return False;
   }
//
   if (!overWriteOutput_p) {
      NewFile x;
      String error;
      if (!x.valueOK(smoothOutU, error)) {
         return False;
      }
   }
//
   smoothOut_p = smoothOutU;  
   return True;
}

template <class T>
void MomentsBase<T>::setVelocityType(MDoppler::Types velocityType)
{
   velocityType_p = velocityType;
}



template <class T>
Vector<Int> MomentsBase<T>::toMethodTypes (const String& methods)
// 
// Helper function to convert a string containing a list of desired smoothed kernel types
// to the correct <src>Vector<Int></src> required for the <src>setSmooth</src> function.
// 
// Inputs:
//   methods     SHould contain some of "win", "fit", "inter"
//
{
   Vector<Int> methodTypes(3);
   if (!methods.empty()) {
      String tMethods = methods;
      tMethods.upcase();

      Int i = 0;
      if (tMethods.contains("WIN")) {
         methodTypes(i) = WINDOW;
         i++;
      }
      if (tMethods.contains("FIT")) {
         methodTypes(i) = FIT;
         i++;
      }
      if (tMethods.contains("INTER")) {
         methodTypes(i) = INTERACTIVE;
         i++;
      }
      methodTypes.resize(i, True);
   } else {
      methodTypes.resize(0);
   }
   return methodTypes;
} 

template <class T> void MomentsBase<T>::_checkMethod () {

    // Only can have the median coordinate under certain conditions
    Bool found;
    if(
        linearSearch(
            found, moments_p, Int(MEDIAN_COORDINATE), moments_p.nelements()
        ) != -1
    ) {
        Bool noGood = False;
        if (doWindow_p || doFit_p || doSmooth_p) {
            noGood = True;
        }
        else {
            if (noInclude_p && noExclude_p) {
                noGood = True;
            }
            else {
                if (selectRange_p(0)*selectRange_p(1) < T(0)) {
                    noGood = True;
                }
            }
        }
        ThrowIf(
            noGood,
            "Request for the median coordinate moment, but it is only "
            "available with the basic (no smooth, no window, no fit) method "
            "and a pixel range that is either all positive or all negative"
        );
    }
    // Now check all the silly methods
    const Bool doInter = (!doAuto_p);
    if (
        ! (
            (
                ! doSmooth_p && ! doWindow_p && ! doFit_p
                && (noInclude_p &&  noExclude_p) && ! doInter
            ) || (
                doSmooth_p && ! doWindow_p && ! doFit_p
                && (! noInclude_p || ! noExclude_p) && ! doInter
            ) || (
                ! doSmooth_p && ! doWindow_p && ! doFit_p
                && (! noInclude_p || ! noExclude_p) && ! doInter
            ) || (
                doSmooth_p &&  doWindow_p && ! doFit_p
                && (noInclude_p &&  noExclude_p) &&  doInter
            ) || (
                ! doSmooth_p &&  doWindow_p && ! doFit_p
                && (noInclude_p &&  noExclude_p) &&  doInter
            ) || (
                doSmooth_p &&  doWindow_p && ! doFit_p
                && (noInclude_p &&  noExclude_p) && ! doInter
            ) || (
                ! doSmooth_p && doWindow_p && ! doFit_p
                && (noInclude_p &&  noExclude_p) && ! doInter
            ) || (
                ! doSmooth_p &&  doWindow_p &&  doFit_p
                && (noInclude_p &&  noExclude_p) &&  doInter
            ) || (
                ! doSmooth_p &&  doWindow_p &&  doFit_p
                && (noInclude_p &&  noExclude_p) && ! doInter
            ) || (
                doSmooth_p &&  doWindow_p &&  doFit_p
                && (noInclude_p &&  noExclude_p) && doInter
            ) || (
                doSmooth_p &&  doWindow_p &&  doFit_p
                && (noInclude_p &&  noExclude_p) && ! doInter
            ) || (
                ! doSmooth_p && ! doWindow_p && doFit_p
                && (noInclude_p &&  noExclude_p) &&  doInter
            ) || (
                ! doSmooth_p && ! doWindow_p && doFit_p
                && (noInclude_p &&  noExclude_p) && !doInter
            )
        )
    ) {
        ostringstream oss;
        oss << "Invalid combination of methods requested." << endl;
        oss << "Valid combinations are: " << endl << endl;
        oss <<  "Smooth    Window      Fit   in/exclude   Interactive " << endl;
        oss <<  "-----------------------------------------------------" << endl;
        // Basic method. Just use all the data
        oss <<  "  N          N         N        N            N       " << endl;
        // Smooth and clip, or just clip
        oss <<  "  Y/N        N         N        Y            N       " << endl << endl;
        // Direct interactive window selection with or without smoothing
        oss <<  "  Y/N        Y         N        N            Y       " << endl;
        // Automatic windowing via Bosma's algorithm with or without smoothing
        oss <<  "  Y/N        Y         N        N            N       " << endl;
        // Windowing by fitting Gaussians (selecting +/- 3-sigma) automatically or interactively
        // with or without out smoothing
        oss <<  "  Y/N        Y         Y        N            Y/N     " << endl;
        // Interactive and automatic Fitting of Gaussians and the moments worked out
        // directly from the fits
        oss <<  "  N          N         Y        N            Y/N     " << endl << endl;

        oss <<  "Request was" << endl << endl;
        oss << "  " << (doSmooth_p ? "Y" : "N");
        oss << "          " << (doWindow_p ? "Y" : "N");
        oss << "         " << (doFit_p ? "Y" : "N");
        oss << "        " << (noInclude_p && noExclude_p ? "Y" : "N");
        oss << "            " << (doInter ? "Y" : "N");
        oss <<  endl;
        oss <<  "-----------------------------------------------------" << endl;
        ThrowCc(oss.str());
   }


    // Tell them what they are getting
    os_p << endl << endl
        << "***********************************************************************" << endl;
    os_p << LogIO::NORMAL << "You have selected the following methods" << endl;
    if (doWindow_p) {
        os_p << "The window method" << endl;
        if (doFit_p) {
            if (doInter) {
                os_p << "   with window selection via interactive Gaussian fitting" << endl;
            }
            else {
                os_p << "   with window selection via automatic Gaussian fitting" << endl;
            }
        }
        else {
            if (doInter) {
                os_p << "   with interactive direct window selection" << endl;
            }
            else {
                os_p << "   with automatic window selection via the converging mean (Bosma) algorithm" << endl;
            }
        }
        if (doSmooth_p) {
            os_p << "   operating on the smoothed image.  The moments are still" << endl;
            os_p << "   evaluated from the unsmoothed image" << endl;
        }
        else {
            os_p << "   operating on the unsmoothed image" << endl;
        }
    }
    else if (doFit_p) {
        if (doInter) {
            os_p << "The interactive Gaussian fitting method" << endl;
        }
        else {
            os_p << "The automatic Gaussian fitting method" << endl;
        }
        os_p << "   operating on the unsmoothed data" << endl;
        os_p << "   The moments are evaluated from the fits" << endl;
    }
    else if (doSmooth_p) {
        os_p << "The smooth and clip method.  The moments are evaluated from" << endl;
        os_p << "   the masked unsmoothed image" << endl;
    }
    else {
        if (noInclude_p && noExclude_p) {
            os_p << "The basic method" << endl;
        }
        else {
            os_p << "The basic clip method" << endl;
        }
    }
    os_p << endl << endl << LogIO::POST;
}

template <class T> Bool MomentsBase<T>::_setOutThings(
    String& suffix, Unit& momentUnits,
    const Unit& imageUnits, const String& momentAxisUnits,
    const Int moment, Bool convertToVelocity
) {
    // Set the output image suffixes and units
    //
    // Input:
    //   momentAxisUnits
    //                The units of the moment axis
    //   moment       The current selected moment
    //   imageUnits   The brightness units of the input image.
    //   convertToVelocity
    //                The moment axis is the spectral axis and
    //                world coordinates must be converted to km/s
    // Outputs:
    //   momentUnits  The brightness units of the moment image. Depends upon moment type
    //   suffix       suffix for output file name
    //   Bool         True if could set units for moment image, false otherwise
    String temp;
    auto goodUnits = True;
    auto goodImageUnits = ! imageUnits.getName().empty();
    auto goodAxisUnits = ! momentAxisUnits.empty();

    if (moment == AVERAGE) {
        suffix = ".average";
        temp = imageUnits.getName();
        goodUnits = goodImageUnits;
    }
    else if (moment == INTEGRATED) {
        suffix = ".integrated";
        temp = imageUnits.getName() + "." + momentAxisUnits;
        if (convertToVelocity) {
            temp = imageUnits.getName() + String(".km/s");
        }
        goodUnits = (goodImageUnits && goodAxisUnits);
    }
    else if (moment == WEIGHTED_MEAN_COORDINATE) {
        suffix = ".weighted_coord";
        temp = momentAxisUnits;
        if (convertToVelocity) {
            temp = String("km/s");
        }
        goodUnits = goodAxisUnits;
    }
    else if (moment == WEIGHTED_DISPERSION_COORDINATE) {
        suffix = ".weighted_dispersion_coord";
        temp = momentAxisUnits + "." + momentAxisUnits;
        if (convertToVelocity) {
            temp = String("km/s");
        }
        goodUnits = goodAxisUnits;
    }
    else if (moment == MEDIAN) {
        suffix = ".median";
        temp = imageUnits.getName();
        goodUnits = goodImageUnits;
    }
    else if (moment == STANDARD_DEVIATION) {
        suffix = ".standard_deviation";
        temp = imageUnits.getName();
        goodUnits = goodImageUnits;
    }
    else if (moment == RMS) {
        suffix = ".rms";
        temp = imageUnits.getName();
        goodUnits = goodImageUnits;
    }
    else if (moment == ABS_MEAN_DEVIATION) {
        suffix = ".abs_mean_dev";
        temp = imageUnits.getName();
        goodUnits = goodImageUnits;
    }
    else if (moment == MAXIMUM) {
        suffix = ".maximum";
        temp = imageUnits.getName();
        goodUnits = goodImageUnits;
    }
    else if (moment == MAXIMUM_COORDINATE) {
        suffix = ".maximum_coord";
        temp = momentAxisUnits;
        if (convertToVelocity) {
            temp = String("km/s");
        }
        goodUnits = goodAxisUnits;
    }
    else if (moment == MINIMUM) {
        suffix = ".minimum";
        temp = imageUnits.getName();
        goodUnits = goodImageUnits;
    }
    else if (moment == MINIMUM_COORDINATE) {
        suffix = ".minimum_coord";
        temp = momentAxisUnits;
        if (convertToVelocity) {
            temp = String("km/s");
        }
        goodUnits = goodAxisUnits;
    }
    else if (moment == MEDIAN_COORDINATE) {
        suffix = ".median_coord";
        temp = momentAxisUnits;
        if (convertToVelocity) {
            temp = String("km/s");
        }
        goodUnits = goodAxisUnits;
    }
    if (goodUnits) {
        momentUnits.setName(temp);
    }
    return goodUnits;
}

template <class T> void MomentsBase<T>::_setIncludeExclude(
    Vector<T>& range, Bool& noInclude, Bool& noExclude,
    const Vector<T>& include, const Vector<T>& exclude
) {
    // Take the user's data inclusion and exclusion data ranges and
    // generate the range and Booleans to say what sort it is
    //
    // Inputs:
    //   include   Include range given by user. Zero length indicates
    //             no include range
    //   exclude   Exclude range given by user. As above.
    //   os        Output stream for reporting
    // Outputs:
    //   noInclude If True user did not give an include range
    //   noExclude If True user did not give an exclude range
    //   range     A pixel value selection range.  Will be resized to
    //             zero length if both noInclude and noExclude are True
    //   Bool      True if successfull, will fail if user tries to give too
    //             many values for includeB or excludeB, or tries to give
    //             values for both

    noInclude = True;
    range.resize(0);
    if (include.size() == 0) {
        // do nothing
    }
    else if (include.size() == 1) {
        range.resize(2);
        range(0) = -abs(include(0));
        range(1) =  abs(include(0));
         noInclude = False;
    }
    else if (include.nelements() == 2) {
        range.resize(2);
        range(0) = min(include(0),include(1));
        range(1) = max(include(0),include(1));
        noInclude = False;
    }
    else {
        ThrowCc("Too many elements for argument include");
    }
    noExclude = True;
    if (exclude.size() == 0) {
        // do nothing
    }
    else if (exclude.nelements() == 1) {
        range.resize(2);
        range(0) = -abs(exclude(0));
        range(1) =  abs(exclude(0));
        noExclude = False;
    }
    else if (exclude.nelements() == 2) {
        range.resize(2);
        range(0) = min(exclude(0),exclude(1));
        range(1) = max(exclude(0),exclude(1));
        noExclude = False;
    }
    else {
        ThrowCc("Too many elements for argument exclude");
    }
    if (! noInclude && ! noExclude) {
        ThrowCc("You can only give one of arguments include or exclude");
    }
}

template <class T> CoordinateSystem MomentsBase<T>::_makeOutputCoordinates (
    IPosition& outShape, const CoordinateSystem& cSysIn,
    const IPosition& inShape, Int momentAxis, Bool removeAxis
) {
    CoordinateSystem cSysOut;
    cSysOut.setObsInfo(cSysIn.obsInfo());

    // Find the Coordinate corresponding to the moment axis

    Int coord, axisInCoord;
    cSysIn.findPixelAxis(coord, axisInCoord, momentAxis);
    const Coordinate& c = cSysIn.coordinate(coord);

    // Find the number of axes

    if (removeAxis) {
        // Shape with moment axis removed
        uInt dimIn = inShape.size();
        uInt dimOut = dimIn - 1;
        outShape.resize(dimOut);
        uInt k = 0;
        for (uInt i=0; i<dimIn; ++i) {
            if (Int(i) != momentAxis) {
                outShape(k) = inShape(i);
                ++k;
            }
        }
        if (c.nPixelAxes()==1 && c.nWorldAxes()==1) {
            // We can physically remove the coordinate and axis
            for (uInt i=0; i<cSysIn.nCoordinates(); ++i) {
                // If this coordinate is not the moment axis coordinate,
                // and it has not been virtually removed in the input
                // we add it to the output.  We don't cope with transposed
                // CoordinateSystems yet.
                auto pixelAxes = cSysIn.pixelAxes(i);
                auto worldAxes = cSysIn.worldAxes(i);
                if (
                    Int(i) != coord && pixelAxes[0] >= 0
                    && worldAxes[0] >= 0
                ) {
                    cSysOut.addCoordinate(cSysIn.coordinate(i));
                }
            }
        }
        else {
            // Remove just world and pixel axis but not the coordinate
            cSysOut = cSysIn;
            Int worldAxis = cSysOut.pixelAxisToWorldAxis(momentAxis);
            cSysOut.removeWorldAxis(worldAxis, cSysIn.referenceValue()(worldAxis));
        }
    }
    else {
        // Retain the Coordinate and give the moment axis  shape 1.
        outShape.resize(0);
        outShape = inShape;
        outShape(momentAxis) = 1;
        cSysOut = cSysIn;
    }
    return cSysOut;
}

}

