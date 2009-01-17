//# LatticePADMVector.cc: Class for drawing axis bound lattice elements as vector fields
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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
//# $Id$

#include <casa/aips.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayDatas/LatticeAsVector.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageRegion.h>
#include <lattices/Lattices/MaskedLattice.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/BasicMath/Math.h>
#include <casa/Quanta/Unit.h>

#include <display/DisplayDatas/LatticePADMVector.h>


namespace casa { //# NAMESPACE CASA - BEGIN

template <class T>
LatticePADMVector<T>::LatticePADMVector(const uInt xAxis,
                                        const uInt yAxis, const uInt mAxis,
                                        const IPosition fixedPos,
                                        LatticePADisplayData<T>* arDat)
 : LatticePADisplayMethod<T>(xAxis, yAxis, mAxis, fixedPos, arDat)
{}

template <class T>
LatticePADMVector<T>::LatticePADMVector(const uInt xAxis,
                                     const uInt yAxis, 
                                     LatticePADisplayData<T>* arDat)
 : LatticePADisplayMethod<T>(xAxis, yAxis, arDat)
{}

template <class T>
LatticePADMVector<T>::~LatticePADMVector() 
{}

template <class T>
uInt LatticePADMVector<T>::dataDrawSelf(WorldCanvas *wCanvas,
                                        const Vector<Double> &blc,
                                        const Vector<Double> &trc,
                                        const IPosition &start,
                                        const IPosition &shape,
                                        const IPosition &stride,
                                        const Bool usePixelEdges) 
//
// blc and trc are world coordinates.  they will be 2-D
// if there is no dependency on any third axis. If there
// is (e.g. ra/freq/dec) then they will be 3-D
//
// start and shape are appropriate to the SubImage if the 
// user has applied an image region.
// 
{
   uInt drawListNumber = wCanvas->newList();
   LatticeAsVector<T>* lav = dynamic_cast<LatticeAsVector<T>*>(parentDisplayData());
   LogIO os(LogOrigin("LatticePADMVector", "drawDataSelf", WHERE));
   T* dummy;
   DataType dataType = whatType(dummy);

// Get options

   Bool debias = lav->itsDebias;
   Float variance = lav->itsVar;
   if (variance<=0.0) {
      if (debias) {
         os << LogIO::WARN <<  
              "Variance is illegal (non-positive) - amplitude will not be debiased" << LogIO::POST;
         debias = False;
      }
   }
   Float scale = lav->itsScale;
   Int incX = lav->itsIncX;
   Int incY = lav->itsIncY;
   Bool arrow = lav->itsArrow;
   Float barb = lav->itsBarb;
   Float rotation = lav->itsRotation * C::pi / 180.0;      // radians
   Bool constAmp = lav->itsConstAmp;
   if (constAmp) {
      if (debias) {
         os << LogIO::WARN <<  
              "Debiasing request ignored for constant amplitude" << LogIO::POST;
      }
      debias = False;
   }

// Find scale factor to convert to radians if Float data

    Float angleConversionFactor = 1.0;
    if (dataType == TpFloat) {
       Unit units = lav->itsUnits;
       Unit rad("rad");
       if (units==rad) {
          angleConversionFactor = units.getValue().getFac();
       } else {
          os << LogIO::WARN << "Units of image are not angular, assuming degrees" << LogIO::POST;
          angleConversionFactor = C::pi / 180.0;
       }
    }

// If we are holding the sky, p.a. is defined N->E (+y -> +x).  Else +x -> +y
// in world coordinate frame

   Vector<Int> displayAxes = lav->displayAxes();
   const CoordinateSystem cSys = lav->coordinateSystem();
   Vector<Double> inc = cSys.increment();
//
   Bool holdsOneSkyAxis = False;
   Vector<Int> tAxes(2); tAxes(0) = displayAxes(0); tAxes(1) = displayAxes(1);
   if (!CoordinateUtil::holdsSky (holdsOneSkyAxis, cSys, tAxes)) {
      angleConversionFactor *= -1.0;
      rotation += C::pi_2;
   }

// Make some effort to see if this image is a Complex polarization
// for which phase type should be "polarimetric"

   Bool isPol = False;
   if (dataType==TpComplex) {
      Vector<Stokes::StokesTypes> whichStokes;
      Int stokes = CoordinateUtil::findStokesAxis(whichStokes, cSys);
      if (stokes >=0) {
         if (whichStokes.nelements()==1) {
            if (whichStokes(0)==Stokes::Plinear || whichStokes(0)==Stokes::PFlinear) isPol = True;
         }
      }
   }
//
   String phaseType = lav->itsPhaseType;
   Float phasePolarity = 2.0;
   if (phaseType==String("normal")) {
      phasePolarity = 1.0;
//
      if (isPol) {
         os << LogIO::WARN << "The image appears to be a Complex polarization type" << endl;
         os << "If this is correct, you should probably set the phase type to 'polarimetric'" << LogIO::POST;
      }
   } else if (phaseType==String("polarimetric")) {
      phasePolarity = 2.0;
//
      if (!isPol) {
         os << LogIO::WARN << "The image does not appear to be a Complex polarization type" << endl;
         os << "If this is correct, you should probably set the phase type to 'normal'" << LogIO::POST;
      }
//
      if (arrow) {
         os << LogIO::WARN << "Arrow heads are meaningless for Polarimetric displays" << LogIO::POST;
      }
      arrow = False;
   }

//
  try {

// Set options

    wCanvas->setColor(lav->itsColor);
    wCanvas->pixelCanvas()->setLineWidth(lav->itsLineWidth);

// Get data; should put inc{X,Y} into stride but then need to
// subimage CoordinateSYstem as well

    Matrix<T> datMatrix;
    Matrix<Bool> datMask;
    dataGetSlice(datMatrix, datMask, start, shape, stride);

// Draw vector map

    if (constAmp && dataType==TpComplex) {

// If we wish to discard the amplitude from Complex data, make
// amplitude and call Float version.  We must set phasePol to
// 1.0 otherwise it would get applied again

       Matrix<Float> datMatrix2 = getAmplitude(datMatrix) / phasePolarity;
       phasePolarity = 1.0;
       angleConversionFactor = 1.0;
       wCanvas->drawVectorMap(blc, trc, datMatrix2, datMask,
                              angleConversionFactor, phasePolarity, 
                              debias, variance, incX, incY, scale, 
                              arrow, barb, rotation, inc(displayAxes(0)),
                              inc(displayAxes(1)), usePixelEdges);
    } else {
       wCanvas->drawVectorMap(blc, trc, datMatrix, datMask,
                              angleConversionFactor, phasePolarity, 
                              debias, variance, incX, incY, scale, 
                              arrow, barb, rotation, inc(displayAxes(0)),
                              inc(displayAxes(1)), usePixelEdges);
    }
  } catch (const AipsError &x) {
    wCanvas->endList();
    if (wCanvas->validList(drawListNumber)) {
      wCanvas->deleteList(drawListNumber);
    }
    throw(AipsError(x));
  } 
//
  wCanvas->endList();
  return drawListNumber;
}

} //# NAMESPACE CASA - END

