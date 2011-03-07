//#---------------------------------------------------------------------------
//# MathUtilities.cc: General math operations
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

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/VectorSTLIterator.h>
#include <casa/BasicSL/String.h>

//#include "MathUtils.h"
#include <singledish/MathUtils.h>

using namespace casa;

float mathutil::statistics(const String& which,  
			   const MaskedArray<Float>& data)
{
   String str(which);
   str.upcase();
   if (str.contains(String("MIN"))) {
      return min(data); 
   } else if (str.contains(String("MAX"))) {
      return max(data);
   } else if (str.contains(String("SUMSQ"))) {
      return sumsquares(data);
   } else if (str.contains(String("SUM"))) {
      return sum(data);
   } else if (str.contains(String("MEAN"))) {
      return mean(data);
   } else if (str.contains(String("VAR"))) {
      return variance(data); 
   } else if (str.contains(String("STDDEV"))) {
      return stddev(data);
   } else if (str.contains(String("AVDEV"))) {
      return avdev(data);
   } else if (str.contains(String("RMS"))) {
      uInt n = data.nelementsValid();
      return sqrt(sumsquares(data)/n);
   } else if (str.contains(String("MED"))) {
      return median(data);
   }
}
  

void mathutil::replaceMaskByZero(Vector<Float>& data, const Vector<Bool>& mask)
{
   for (uInt i=0; i<data.nelements(); i++) {
      if (!mask[i]) data[i] = 0.0;
   }
}


void mathutil::scanBoundaries(Vector<uInt>& startInt,
			      Vector<uInt>& endInt,
			      const Vector<Int>& scanIDs) 
  // Find integrations start and end for each Scan
{
  uInt nInt = scanIDs.nelements();
  startInt.resize(nInt);
  endInt.resize(nInt);

  startInt(0) = 0;
  uInt j = 0;
  Int currScanID = scanIDs(0);
  for (uInt i=0; i<nInt; i++) {
    if (scanIDs(i) != currScanID) {  
      endInt(j) = i-1;
      currScanID = scanIDs(i);

      j += 1;
      startInt(j) = i;
      if (i==nInt-1) {
	endInt(j) = i;
      }   
    } else {
      if (i==nInt-1) endInt(j) = i;
    }
  }
  startInt.resize(j+1,True);
  endInt.resize(j+1,True);
}

std::vector<std::string> mathutil::tovectorstring(const Vector<String>& in) 
{
  std::vector<std::string> out;
  VectorSTLIterator<String> it(in);
  for (uInt i=0; it != in.end(); ++it,++i) {
    out.push_back(*it);
  }
  return out;
}

Vector<String> mathutil::toVectorString(const std::vector<std::string>& in) 
{
  Vector<String> out(in.size());
  uInt i=0;
  std::vector<std::string>::const_iterator it;
  for (it=in.begin();it != in.end();++it,++i) {
    out[i] = casa::String(*it);
  }
  return out;
}
