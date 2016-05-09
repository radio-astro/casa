//# MomentCalcBase.h:
//# Copyright (C) 1997,1999,2000,2001,2002
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
//# $Id: MomentCalculator.h 20299 2008-04-03 05:56:44Z gervandiepen $

#ifndef IMAGEANALYSIS_MOMENTCALCBASE_H
#define IMAGEANALYSIS_MOMENTCALCBASE_H

namespace casa {

//# Forward Declarations
template <class T> class MomentsBase;
// <summary>
// Abstract base class for moment calculator classes
// </summary>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> <linkto class="MomentsBase">MomentsBase</linkto>
//   <li> <linkto class="ImageMoments">ImageMoments</linkto>
//   <li> <linkto class="MSMoments">MSMoments</linkto>
//   <li> <linkto class="LatticeApply">LatticeApply</linkto>
//   <li> <linkto class="LineCollapser">LineCollapser</linkto>
// </prerequisite>
//
// <synopsis>
//  This class, its concrete derived classes, and the classes LineCollapser,
//  ImageMoments, MSMoments, and LatticeApply are connected as follows.   LatticeApply offers 
//  functions so that the application programmer does not need to worry about how 
//  to optimally iterate through a Lattice; it deals with tiling and to a 
//  lesser extent memory.    LatticeApply functions are used by offering a class 
//  object to them that has a member function with a name and signature 
//  specified by an abstract base class that LatticeApply uses and the 
//  offered class inherits from.   Specifically, in this case, MomentCalcBase
//  inherits from LineCollapser and LatticeApply uses objects and methods of this
//  class (but does not inherit from it).  This defines the functions
//  <src>collapse</src> and <src>multiProcess</src> which operate on a vector
//  extracted from a Lattice.  The former returns one number, the latter a vector
//  of numbers from that profile.  MomentCalcBase is a base class for
//  for moment calculation and the <src>multiProcess</src>
//  functions are used to compute moments  (e.g., mean, sum, sum squared, 
//  intensity weighted velocity etc).
//
//  It is actually the concrete classes derived from MomentCalcBase (call them,
//  as a group, the MomentCalculator classes) that implement the <src>multiProcess</src> 
//  functions.  These derived classes allow different 
//  algorithms to be written with which moments of the vector can be computed. 
//
//  Now, so far, we have a LatticeApply function which iterates through Lattices,
//  extracts vectors, and offers them up to functions implemented in the derived 
//  MomentCalculator classes to compute the moments.   As well as that, we need some
//  class to actually construct the MomentCalculator classes and to feed them to 
//  LatticeApply.   This is the role of the ImageMoments or MSMoments classes.  
//  They are a high level 
//  class which takes control information from users specifying which moments they 
//  would like to calculate and how. They also provide the ancilliary masking lattice to 
//  the MomentCalculator constructors. The actual computational work is done by the 
//  MomentCalculator classes. So MomentsBase, MomentCalcBase and their derived 
//  MomentCalculator classes are really one unit; none of them are useful without 
//  the others.  The separation of functionality is caused by having the
//  LatticeApply class that knows all about optimally iterating through Lattices.
//
//  The coupling between these classes is done partly by the "friendship".   MomentsBase and 
//  its inheritances 
//  grant friendship to MomentCalcBase so that the latter has access to the private data and 
//  private functions of the formers.  MomentCalcBase then operates as an interface between 
//  its derived MomentCalculator classes and ImageMoments or MSMoments. It retrieves private data 
//  from these classes, and also activates private functions in these classes, on behalf 
//  of the MomentCalculator classes. The rest of the coupling is done via the constructors 
//  of the derived MomentCalculator classes.  
//
//  Finally, MomentCalcBase also has a number of protected functions that are common to its
//  derived classes (e.g. fitting, accumulating sums etc).  It also has protected
//  data that is common to all the MomentCalculator classes.  This protected data is accessed 
//  directly by name rather than with interface functions as there is too much of it.  Of 
//  course, since MomentCalcBase is an abstract base class, it is up to the MomentCalculator 
//  classes to give the MomentCalcBase protected data objects values.
//
//  For discussion about different moments and algorithms to compute them see the 
//  discussion in <linkto class="MomentsBase">MomentsBase</linkto>, 
//  <linkto class="ImageMoments">ImageMoments</linkto>, 
//  <linkto class="MSMoments">MSMoments</linkto> and also in
//  the derived classes documentation.
// </synopsis>
//
// <example>
//  Since MomentCalcBase is an abstract class, we defer code examples to
//  the derived classes.
// </example>
//
// <motivation>
// We were desirous of writing functions to optimally iterate through Lattices
// so that the application programmer did not have to know anything about tiling
// or memory if possible.   These are the LatticeApply functions. To incorporate 
// MomentsBase and its inheritances into this scheme required some of it to be shifted into 
// MomentCalcBase and its derived classes.
// </motivation>
//
// <note role=tip>
// Note that there are is assignment operator or copy constructor.
// Do not use the ones the system would generate either.
// </note>
//
// <todo asof="yyyy/mm/dd">
//  <li> Derive more classes !
// </todo>


template <class T> class MomentCalcBase : public LineCollapser<T,T> {
public:
    virtual ~MomentCalcBase();

    // Returns the number of failed fits if doing fitting
    virtual inline uInt nFailedFits() const { return nFailed_p; }

protected:

    // A number of private data members are kept here in the base class
    // as they are common to the derived classes.  Since this class
    // is abstract, they have to be filled by the derived classes.

    // CoordinateSystem
    CoordinateSystem cSys_p;

    // This vector is a container for all the possible moments that
    // can be calculated.  They are in the order given by the MomentsBase
    // enum MomentTypes
    Vector<T> calcMoments_p;
    Vector<Bool> calcMomentsMask_p;

    // This vector tells us which elements of the calcMoments_p vector
    // we wish to select
    Vector<Int> selectMoments_p;

    // Although the general philosophy of these classes is to compute
    // all the posisble moments and then select the ones we want,
    // some of them are too expensive to calculate unless they are
    // really wanted.  These are the median moments and those that
    // require a second pass.  These control Bools tell us whether
    // we really want to compute the expensive ones.
    Bool doMedianI_p, doMedianV_p, doAbsDev_p;

    // These vectors are used to transform coordinates between pixel and world
    Vector<Double> pixelIn_p, worldOut_p;

    // All computations involving Coordinate conversions are relatively expensive
    // These Bools signifies whether we need coordinate calculations or not for
    // the full profile, and for some occaisional calculations
    Bool doCoordProfile_p, doCoordRandom_p;

    // This vector houses the world coordinate values for the profile if it
    // was from a separable axis. This means this vector can be pre computed
    // just once, instead of working out the coordinates for each profile
    // (expensive).  It should only be filled if doCoordCalc_p is True
    Vector<Double> sepWorldCoord_p;

    // This Bool tells us whether we want to see all profiles plotted with the
    // Y range or whether they are to be scaled individually
    //Bool fixedYLimits_p;

    // When we are plotting, if we have asked to all profiles with the same
    // Y min and max, these are the values to use
    T yMinAuto_p, yMaxAuto_p;

    // This vector is used to hold the abcissa values when plotting profiles
    Vector<T> abcissa_p;

    // This string tells us the name of the moment axis (VELO or FREQ etc)
    String momAxisType_p;

    // This is the number of Gaussian fits that failed.
    uInt nFailed_p;

    // This scale factor is the increment along the moment axis
    // applied so that units for the Integrated moment are like
    // Jy/beam.km/s (or whatever is needed for the moment axis units)
    // For non-linear velocities (e.g. optical) this is approximate
    // only and is computed at the reference pixel
    Double integratedScaleFactor_p;

    // Accumulate statistical sums from a vector
    inline void accumSums(
        typename NumericTraits<T>::PrecisionType& s0,
        typename NumericTraits<T>::PrecisionType& s0Sq,
        typename NumericTraits<T>::PrecisionType& s1,
        typename NumericTraits<T>::PrecisionType& s2,
        Int& iMin, Int& iMax, T& dMin, T& dMax,
        Int i, T datum, Double coord
    ) const {
        // Accumulate statistical sums from this datum
        //
        // Input:
        //  i              Index
        //  datum          Pixel value
        //  coord          Coordinate value on moment axis
        // Input/output:
        //  iMin,max       index of dMin and dMax
        //  dMin,dMax      minimum and maximum value
        // Output:
        //  s0             sum (I)
        //  s0Sq           sum (I*I)
        //  s1             sum (I*v)
        //  s2             sum (I*v*v)
        typename NumericTraits<T>::PrecisionType dDatum = datum;
        s0 += dDatum;
        s0Sq += dDatum*dDatum;
        s1 += dDatum*coord;
        s2 += dDatum*coord*coord;
        if (datum < dMin) {
            iMin = i;
            dMin = datum;
        }
        if (datum > dMax) {
            iMax = i;
            dMax = datum;
        }
    }

    // Determine if the spectrum is pure noise
    uInt allNoise(T& dMean,
        const Vector<T>& data,
        const Vector<Bool>& mask,
        T peakSNR,
        T stdDeviation
    ) const;

    // Check validity of constructor inputs
    void constructorCheck(
        Vector<T>& calcMoments,
        Vector<Bool>& calcMomentsMask,
        const Vector<Int>& selectMoments,
        uInt nLatticeOut
    ) const;

    // Convert from <tt>T</tt> to <tt>Float</tt> for plotting
    inline Float convertT(T value) {
        return MomentsBase<T>::convertT(value);
    }

    // Convert from <tt>Float</tt> (from plotting) to a <tt>T</tt>
    inline T convertF(const Float value) {
        return MomentsBase<T>::convertF(value);
    }

    // Find out from the selectMoments array whether we want
    // to compute the more expensive moments
    void costlyMoments(
        MomentsBase<T>& iMom, Bool& doMedianI,
        Bool& doMedianV, Bool& doAbsDev
    ) const;

    // Return automatic/interactive switch from the ImageMoments object
    Bool doAuto(const MomentsBase<T>& iMom) const;

    // Return the Bool saying whether we need to compute coordinates
    // or not for the requested moments
    void doCoordCalc(
        Bool& doCoordProfile,
        Bool& doCoordRandom,
        const MomentsBase<T>& iMom
    ) const;

    // Return the Bool from the ImageMoments or MSMoments object saying whether we
    // are going to fit Gaussians to the profiles or not.
    Bool doFit(const MomentsBase<T>& iMom) const;

    // Find the next masked or unmasked point in a vector
    Bool findNextDatum(
        uInt& iFound, const uInt& n,
        const Vector<Bool>& mask, const uInt& iStart,
        const Bool& findGood
    ) const;

    // Fit a Gaussian to x and y arrays given guesses for the gaussian parameters
    Bool fitGaussian(
        uInt& nFailed, T& peak, T& pos,
        T& width, T& level, const Vector<T>& x,
        const Vector<T>& y, const Vector<Bool>& mask,
        T peakGuess, T posGuess, T widthGuess,
        T levelGuess
    ) const;

    // Return the fixed Y-plotting limits switch from the
    // ImageMoments or MSMoments object
    Bool fixedYLimits(const MomentsBase<T>& iMom) const;

    // Automatically fit a Gaussian to a spectrum, including finding the
    // starting guesses.
    Bool getAutoGaussianFit(
        uInt& nFailed, Vector<T>& gaussPars,
        const Vector<T>& x, const Vector<T>& y,
        const Vector<Bool>& mask, T peakSNR,
        T stdDeviation
    ) const;

    // Automatically work out a guess for the Gaussian parameters
    // Returns False if all pixels masked.
    Bool getAutoGaussianGuess(
        T& peakGuess, T& posGuess,
        T& widthGuess, T& levelGuess,
        const Vector<T>& x, const Vector<T>& y,
        const Vector<Bool>& mask
    ) const;

    // Interactively define a guess for a Gaussian fit, and then
    // do the fit.  Do this repeatedly  until the user is content.
    Bool getInterGaussianFit(
        uInt& nFailed, Vector<T>& gaussPars,
        LogIO& os, const Vector<T>& x,
        const Vector<T>& y, const Vector<Bool>& mask,
        const Bool fixedYLimits, const T yMinAuto,
        T yMaxAuto, const String& xLabel,
        const String& yLabel, const String& title
    ) const;

    // Interactively define a guess for the Gaussian parameters
    void getInterGaussianGuess(
        T& peakGuess, T& posGuess,
        T& widthGuess, Vector<Int>& window,
        Bool& reject, LogIO& os,
        Int nPts
    ) const;

    // Compute the world coordinate for the given moment axis pixel
    inline Double getMomentCoord(
        const MomentsBase<T>& iMom, Vector<Double>& pixelIn,
        Vector<Double>& worldOut, Double momentPixel,
        Bool asVelocity=False
    ) const {
        // Find the value of the world coordinate on the moment axis
        // for the given moment axis pixel value.
        //
        // Input
        //   momentPixel   is the index in the profile extracted from the data
        // Input/output
        //   pixelIn       Pixels to convert.  Must all be filled in except for
        //                 pixelIn(momentPixel).
        //   worldOut      Vector to hold result
        //
        // Should really return a Fallible as I don't check and see
        // if the coordinate transformation fails or not
        //
        // Should really check the result is True, but for speed ...
        pixelIn(iMom.momentAxis_p) = momentPixel;
        cSys_p.toWorld(worldOut, pixelIn);
        if (asVelocity) {
            Double velocity;
            cSys_p.spectralCoordinate().frequencyToVelocity(
                velocity, worldOut(iMom.worldMomentAxis_p)
            );
            return velocity;
        }
        return worldOut(iMom.worldMomentAxis_p);
    }

    // Examine a mask and determine how many segments of unmasked points
    // it consists of.
    void lineSegments (
        uInt& nSeg, Vector<uInt>& start,
        Vector<uInt>& nPts, const Vector<Bool>& mask
    ) const;

    // Resize an abcissa vector for plotting
    void makeAbcissa(
        Vector<T>& x, Int n
    ) const;

    // Return the moment axis from the ImageMoments object
    Int& momentAxis(MomentsBase<T>& iMom) const;

    // Return the name of the moment/profile axis
    String momentAxisName(
        const CoordinateSystem&,
        const MomentsBase<T>& iMom
    ) const;

    // Return the number of moments that the ImageMoments or MSMoments class can calculate
    uInt nMaxMoments() const;

    // Return the peak SNR for determination of all noise spectra from
    // the ImageMoments or MSMoments object
    T& peakSNR(MomentsBase<T>& iMom) const;

    // Return the selected pixel intensity range from the ImageMoments or MSMoments
    // object and the Bools describing whether it is inclusion or exclusion
    void selectRange(
        Vector<T>& pixelRange,
        Bool& doInclude,
        Bool& doExlude,
        MomentsBase<T>& iMom
    ) const;

    // The MomentCalculators compute a vector of all possible moments.
    // This function returns a vector which selects the desired moments from that
    // "all moment" vector.
    Vector<Int> selectMoments(MomentsBase<T>& iMom) const;

    // Fill the ouput moments array
    void setCalcMoments(
        const MomentsBase<T>& iMom, Vector<T>& calcMoments,
        Vector<Bool>& calcMomentsMask, Vector<Double>& pixelIn,
        Vector<Double>& worldOut, Bool doCoord,
        Double integratedScaleFactor, T dMedian,
        T vMedian, Int nPts,
        typename NumericTraits<T>::PrecisionType s0,
        typename NumericTraits<T>::PrecisionType s1,
        typename NumericTraits<T>::PrecisionType s2,
        typename NumericTraits<T>::PrecisionType s0Sq,
        typename NumericTraits<T>::PrecisionType sumAbsDev,
        T dMin, T dMax, Int iMin, Int iMax
    ) const;

    // Fill a string with the position of the cursor
    void setPosLabel(String& title, const IPosition& pos) const;

    // Install CoordinateSystem and SpectralCoordinate
    // in protected data members
    void setCoordinateSystem(
        CoordinateSystem& cSys, MomentsBase<T>& iMom
    );

    // Set up separable moment axis coordinate vector and
    // conversion vectors if not separable
    void setUpCoords(
        const MomentsBase<T>& iMom, Vector<Double>& pixelIn,
        Vector<Double>& worldOut, Vector<Double>& sepWorldCoord,
        LogIO& os, Double& integratedScaleFactor,
        const CoordinateSystem& cSys, Bool doCoordProfile,
        Bool doCoordRandom
    ) const;

    // Find some statistics from teh masked vector.
    // Returns False if no unmasked points.
    Bool stats(
        T& dMin, T& dMax, uInt& minPos, uInt& maxPos,
        T& mean, const Vector<T>& profile,
        const Vector<Bool>& mask
    ) const;

    // Return standard deviation of image from ImageMoments or MSMoments object
    T& stdDeviation(MomentsBase<T>& iMom) const;

    // Return the auto y min and max from the ImageMoments object
    void yAutoMinMax(
        T& yMin, T& yMax, MomentsBase<T>& iMom
    ) const;

protected:
    // Check if #pixels is indeed 1.
    virtual void init (uInt nOutPixelsPerCollapse);
};

}

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <imageanalysis/ImageAnalysis/MomentCalcBase.tcc>
#endif
#endif
