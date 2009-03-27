//#---------------------------------------------------------------------------
//# SDPol2.cc: Templated polarimetric functionality
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------

#include <singledish/SDPol.h>
#include <singledish/SDDefs.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/DataType.h>
#include <casa/Utilities/Assert.h>


using namespace casa;
using namespace asap;


template <class T>
Array<T> SDPolUtil::stokesData (Array<T>& rawData, Bool doLinear)
//
// Generate data for each Stokes parameter from the
// raw flags.  This is a lot of computational work and may
// not be worth the effort.
//
// Designed for use 
//   Bool or uChar for mask
//   Float for TSys
//
// Input arrays should be of shape
//    [nBeam,nIF,nPol,nChan]
//
{
   T* t;
   DataType type = whatType(t);
   AlwaysAssert(type==TpFloat || type==TpBool || type==TpUChar, AipsError);
//
   IPosition shapeIn = rawData.shape();
   const uInt nDim = shapeIn.nelements();
   uInt polAxis = 0;
   AlwaysAssert(nDim==asap::nAxes, AipsError);
//
   uInt nPol = shapeIn(asap::PolAxis);
   Array<T> stokesData;
//
   IPosition start(nDim,0);
   IPosition end(shapeIn-1);
   IPosition shapeOut = shapeIn;
//
   if (doLinear) {
      if (nPol==1) {
         stokesData.resize(shapeOut);
         stokesData = rawData;
      } else if (nPol==2 || nPol==4) {

// Set shape of output array

         if (nPol==2) {
            shapeOut(asap::PolAxis) = 1;
         } else {
            shapeOut(asap::PolAxis) = 4;
         }
         stokesData.resize(shapeOut);

// Get reference slices and assign/compute

         start(asap::PolAxis) = 0;
         end(asap::PolAxis) = 0;
         Array<T> M1In = rawData(start,end);
//
         start(asap::PolAxis) = 1;
         end(asap::PolAxis) = 1;
         Array<T> M2In = rawData(start,end);
//
         start(asap::PolAxis) = 0;
         end(asap::PolAxis) = 0;
         Array<T> M1Out = stokesData(start,end);         
         M1Out = SDPolUtil::andArrays (M1In, M2In);         // I
//
         if (nPol==4) {   
            start(asap::PolAxis) = 2;
            end(asap::PolAxis) = 2;
            Array<T> M3In = rawData(start,end);
//
            start(asap::PolAxis) = 3;
            end(asap::PolAxis) = 3;
            Array<T> M4In = rawData(start,end);
//
            start(asap::PolAxis) = 1;
            end(asap::PolAxis) = 1;
            Array<T> M2Out = stokesData(start,end);
            M2Out = M1Out;                                  // Q
//
            start(asap::PolAxis) = 2;
            end(asap::PolAxis) = 2;
            Array<T> M3Out = stokesData(start,end);
            M3Out = M3In;                                   // U
//
            start(asap::PolAxis) = 3;
            end(asap::PolAxis) = 3;
            Array<T> M4Out = stokesData(start,end);
            M4Out = M4In;                                   // V
         }
      } else {
         throw(AipsError("Can only handle 1,2 or 4 polarizations"));
      }
   } else {
      throw (AipsError("Only implemented for Linear polarizations"));
   }
//
   return stokesData;
}



template <class T>
Array<T> SDPolUtil::computeStokesDataForWriter (Array<T>& rawData, Bool doLinear)
//
// Generate data for each Stokes parameter from the
// raw flags.  This is a lot of computational work and may
// not be worth the effort.  This function is specifically
// for the SDWriter 
//
// Designed for use 
//   Bool or uChar for mask
//   Float for TSys
//
// Input arrays should be of shape
//    [nChan,nPol]   Bool/uChar
//    [nPol]         Float      
//
{
   T* t;
   DataType type = whatType(t);
   AlwaysAssert(type==TpFloat || type==TpBool || type==TpUChar, AipsError);
//
   IPosition shapeIn = rawData.shape();
   const uInt nDim = shapeIn.nelements();
   uInt polAxis = 0;
   if (type==TpFloat) {
      AlwaysAssert(nDim==1,AipsError);
   } else {
      AlwaysAssert(nDim==2,AipsError);
      polAxis = 1;
   }
//
   uInt nPol = shapeIn(polAxis);
   Array<T> stokesData;
//
   IPosition start(nDim,0);
   IPosition end(shapeIn-1);
   IPosition shapeOut = shapeIn;
//
   if (doLinear) {
      if (nPol==1) {
         stokesData.resize(shapeOut);
         stokesData = rawData;
      } else if (nPol==2 || nPol==4) {

// Set shape of output array

         if (nPol==2) {
            shapeOut(polAxis) = 1;
         } else {
            shapeOut(polAxis) = 4;
         }
         stokesData.resize(shapeOut);

// Get reference slices and assign/compute

         start(polAxis) = 0;
         end(polAxis) = 0;
         Array<T> M1In = rawData(start,end);
//
         start(polAxis) = 1;
         end(polAxis) = 1;
         Array<T> M2In = rawData(start,end);
//
         start(polAxis) = 0;
         end(polAxis) = 0;
         Array<T> M1Out = stokesData(start,end);         
         M1Out = SDPolUtil::andArrays (M1In, M2In);         // I
//
         if (nPol==4) {   
            start(polAxis) = 2;
            end(polAxis) = 2;
            Array<T> M3In = rawData(start,end);
//
            start(polAxis) = 3;
            end(polAxis) = 3;
            Array<T> M4In = rawData(start,end);
//
            start(polAxis) = 1;
            end(polAxis) = 1;
            Array<T> M2Out = stokesData(start,end);
            M2Out = M1Out;                                  // Q
//
            start(polAxis) = 2;
            end(polAxis) = 2;
            Array<T> M3Out = stokesData(start,end);
            M3Out = M3In;                                   // U
//
            start(polAxis) = 3;
            end(polAxis) = 3;
            Array<T> M4Out = stokesData(start,end);
            M4Out = M4In;                                   // V
         }
      } else {
         throw(AipsError("Can only handle 1,2 or 4 polarizations"));
      }
   } else {
      throw (AipsError("Only implemented for Linear polarizations"));
   }
//
   return stokesData;
}


