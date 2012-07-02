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
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/VectorSTLIterator.h>
#include <casa/BasicSL/String.h>
#include <scimath/Mathematics/MedianSlider.h>
#include <casa/Exceptions/Error.h>

#include "MathUtils.h"

using namespace casa;

float mathutil::statistics(const String& which,
                           const MaskedArray<Float>& data)
{
   String str(which);
   str.upcase();
   if (str.matches(String("MIN"))) {
      return min(data);
   } else if (str.matches(String("MAX"))) {
      return max(data);
   } else if (str.matches(String("SUMSQ"))) {
      return sumsquares(data);
   } else if (str.matches(String("SUM"))) {
      return sum(data);
   } else if (str.matches(String("MEAN"))) {
      return mean(data);
   } else if (str.matches(String("VAR"))) {
      return variance(data);
      } else if (str.matches(String("STDDEV"))) {
      return stddev(data);
   } else if (str.matches(String("AVDEV"))) {
      return avdev(data);
   } else if (str.matches(String("RMS"))) {
      uInt n = data.nelementsValid();
      return sqrt(sumsquares(data)/n);
   } else if (str.matches(String("MEDIAN"))) {
      return median(data);
   } else {
      String msg = str + " is not a valid type of statistics";
      throw(AipsError(msg));
   } 
   return 0.0;
}

IPosition mathutil::minMaxPos(const String& which,
                           const MaskedArray<Float>& data)
{
   Float minVal, maxVal;
   IPosition minPos(data.ndim(), 0), maxPos(data.ndim(), 0); 
   minMax(minVal, maxVal, minPos, maxPos, data);
   String str(which);
   str.upcase();
   if (str.contains(String("MIN"))) {
     return minPos;
   } else if (str.contains(String("MAX"))) {
     return maxPos;
   } else {
      String msg = str + " is not a valid type of statistics";
      throw(AipsError(msg));
   } 
   //return 0.0;
}

void mathutil::replaceMaskByZero(Vector<Float>& data, const Vector<Bool>& mask)
{
   for (uInt i=0; i<data.nelements(); i++) {
      if (!mask[i]) data[i] = 0.0;
   }
}


std::vector<std::string> mathutil::tovectorstring(const Vector<String>& in)
{
  std::vector<std::string> out;
  out.reserve(in.nelements());
  for (Array<String>::const_iterator it = in.begin(); it != in.end(); ++it) {
    out.push_back(*it);
  }
  return out;
}

Vector<String> mathutil::toVectorString(const std::vector<std::string>& in)
{
  Vector<String> out(in.size());
  Array<String>::iterator oit = out.begin();
  for (std::vector<std::string>::const_iterator it=in.begin() ;
       it != in.end(); ++it,++oit) {
    *oit = *it;
  }
  return out;
}

void mathutil::hanning(Vector<Float>& out, Vector<Bool>& outmask,
                       const Vector<Float>& in, const Vector<Bool>& mask,
                       Bool relaxed, Bool ignoreOther) {
  Vector< Vector<Float> > weights(8);
  Vector<Float> vals(3);
  vals = 0.0;weights[0] = vals;// FFF
  vals[0] = 1.0; vals[1] = 0.0; vals[2] = 0.0; weights[1] = vals;// TFF
  vals[0] = 0.0; vals[1] = 1.0; vals[2] = 0.0; weights[2] = vals;// FTF
  vals[0] = 1.0/3.0; vals[1] = 2.0/3.0; vals[2] = 0.0; weights[3] = vals;// TTF
  vals[0] = 0.0; vals[1] = 0.0; vals[2] = 1.0;weights[4] = vals;// FFT
  vals[0] = 0.5; vals[1] = 0.0; vals[2] = 0.5; weights[5] = vals;// TFT
  vals[0] = 0.0; vals[1] = 2.0/3.0; vals[2] = 1.0/3.0; weights[6] = vals;// FTT
  vals[0] = 0.25; vals[1] = 0.5; vals[2] = 0.25; weights[7] = vals;// TTT
  // Chris' case
  Vector<Bool> weighted(8);
  if (relaxed) {
    weighted = False;
    weighted[7] = True;

  } else {
    weighted = True;
    weighted[0] = False;
  }

  out.resize(in.nelements());
  outmask.resize(mask.nelements());
  // make special case for first and last
  /// ...here
  // loop from 1..n-2
  out.resize(in.nelements());
  out[0] = in[0];out[out.nelements()-1] = in[in.nelements()-1];
  outmask.resize(mask.nelements());
  outmask = False;
  uInt m;Vector<Float>* w;
  for (uInt i=1; i < out.nelements()-1;++i) {
    m = mask[i-1] + 2*mask[i] + 4*mask[i+1];
    w = &(weights[m]);
    if (weighted[m]) {
      out[i] = (*w)[0]*in[i-1] + (*w)[1]*in[i] + (*w)[2]*in[i+1];
      outmask[i] = True;
    } else { // mask it
      out[i] = in[i];//use arbitrary value
      outmask[i] = False;
    }
  }
}


void mathutil::runningMedian(Vector<Float>& out, Vector<Bool>& outflag,
                             const Vector<Float>& in, const Vector<Bool>& flag,
                             float width)
{
  Int hwidth = Int(width+0.5);
  Int fwidth = hwidth*2+1;
  out.resize(in.nelements());
  outflag.resize(flag.nelements());
  MedianSlider ms(hwidth);
  Slice sl(0, fwidth-1);
  Float medval = ms.add(const_cast<Vector<Float>& >(in)(sl), 
                  const_cast<Vector<Bool>& >(flag)(sl));
  uInt n = in.nelements();
  for (uInt i=hwidth; i<(n-hwidth); ++i) {
    // add data value
    out[i] = ms.add(in[i+hwidth], flag[i+hwidth]); 
    outflag[i] = (ms.nval() == 0);    
  }
  // replicate edge values from fisrt value with full width of values
  for (uInt i=0;i<hwidth;++i) {
    out[i] = out[hwidth];
    outflag[i] = outflag[hwidth];    
    out[n-1-i] = out[n-1-hwidth];
    outflag[n-1-i] = outflag[n-1-hwidth];    
  }
}
