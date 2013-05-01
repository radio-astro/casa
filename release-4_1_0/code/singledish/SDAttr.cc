//#---------------------------------------------------------------------------
//# SDAttr.cc: A collection of attributes for different telescopes
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

#include <singledish/SDAttr.h>
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Exceptions.h>
#include <casa/Quanta/QC.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>

#include <measures/Measures/MEpoch.h>

#include <scimath/Mathematics/InterpolateArray1D.h>

using namespace casa;
using namespace asap;

SDAttr::SDAttr ()
{
   initData();
}

SDAttr::SDAttr(const SDAttr& other)
{
   initData();                                 // state just private 'static' data
}

SDAttr& SDAttr::operator=(const SDAttr& other) 
{
  if (this != &other) {
    ;                                      // state just private 'static' data
  }
  return *this;
}

SDAttr::~SDAttr()
{;}


Float SDAttr::diameter (Instrument inst)  const
{
   Float D = 1.0;
   switch (inst) {
      case ATMOPRA:
        {
           D = 22.0;
        }
        break;
      case ATPKSMB:
      case ATPKSHOH:
        {
           D = 64.0;
        }
        break;
      case TIDBINBILLA:
        {
           D = 70.0;
        }
        break;
      case CEDUNA:
        {
           D = 30.0;
        }
        break;
      case HOBART:
        {
           D = 26.0;
        }
        break;
      default:
        {
            throw(AipsError("Unknown instrument"));
        }
   }
//
   return D;
}

Vector<Float> SDAttr::beamEfficiency (Instrument inst, const MEpoch& dateObs, const Vector<Float>& freqs) const
{

// Look at date where appropriate

   MVTime t(dateObs.getValue());
   uInt year = t.year();
//
   Vector<Float> facs(freqs.nelements(),1.0);
   switch (inst) {
      case ATMOPRA:
        {
           if (year<2003) {
              cerr << "There is no beam efficiency data from before 2003 - using 2003 data" << endl;
              facs = interp (freqs/1.0e9f, MopEtaBeamX_, MopEtaBeam2003Y_); 
           } else if (year==2003) {
              cerr << "Using beam efficiency data from 2003" << endl;
              facs = interp (freqs/1.0e9f, MopEtaBeamX_, MopEtaBeam2003Y_); 
           } else {
              cerr << "Using beam efficiency data from 2004" << endl;
              facs = interp (freqs/1.0e9f, MopEtaBeamX_, MopEtaBeam2004Y_); 
           }
        }
        break;
      default:
        {
           cerr << "No beam efficiency data for this instrument - assuming unity" << endl;
        }
   }
//
   return facs;
}

Vector<Float> SDAttr::apertureEfficiency (Instrument inst, const MEpoch& dateObs, const Vector<Float>& freqs) const
{

// Look at date where appropriate

   MVTime t(dateObs.getValue());
   uInt year = t.year();
//
   Vector<Float> facs(freqs.nelements(),1.0);
   switch (inst) {
      case ATMOPRA:
        {
           if (year<2004) {
              cerr << "There is no aperture efficiency data from before 2004 - using 2004 data" << endl;
              facs = interp (freqs/1.0e9f, MopEtaApX_, MopEtaAp2004Y_);
           } else {
              cerr << "Using aperture efficiency data from 2004" << endl;
              facs = interp (freqs/1.0e9f, MopEtaApX_, MopEtaAp2004Y_);
           }
        }
        break;
      case TIDBINBILLA:
        {
            facs = interp (freqs/1.0e9f, TidEtaApX_, TidEtaApY_);
        }
        break;
      default:
        {
           cerr << "No aperture efficiency data for this instrument - assuming unity" << endl;
        }
   }
   return facs;
}

Vector<Float> SDAttr::JyPerK (Instrument inst, const MEpoch& dateObs, const Vector<Float>& freqs) const
{

// FInd what we need

   Vector<Float> etaAp = apertureEfficiency (inst, dateObs, freqs);
   Float D = diameter(inst);

// Compute it

   Vector<Float> facs(freqs.nelements(),1.0);
   for (uInt i=0; i<freqs.nelements(); i++) {
      facs(i) = SDAttr::findJyPerK (etaAp(i), D);
   }
//
   return facs;
}


Float SDAttr::findJyPerK (Float etaAp, Float D)
//
// Converts K -> Jy
// D in m
//
{
   Double kb = QC::k.getValue(Unit(String("erg/K")));
   Float gA = C::pi * D * D / 4.0;
   return (2.0 * 1.0e19 * kb / etaAp / gA);
}


Vector<Float> SDAttr::gainElevationPoly (Instrument inst) const
{

// Look at date where appropriate

   switch (inst) {
      case TIDBINBILLA:
        {
           return TidGainElPoly_.copy();
        }
        break;
      default:
        {
           Vector<Float> t;
           return t.copy();
        }
   }
}

FeedPolType SDAttr::feedPolType (Instrument inst) const
{
   FeedPolType type = UNKNOWNFEED;
   switch (inst) {
      case ATMOPRA:
      case ATPKSMB:
      case ATPKSHOH:
        {
           type = LINEAR;
        }
        break;
      case TIDBINBILLA:
        {
           type = CIRCULAR;
        }
        break;
      default:
        {
           type = UNKNOWNFEED;
        }
   }
   return type;
}





// Private

Vector<Float> SDAttr::interp (const Vector<Float>& xOut, const Vector<Float>& xIn, 
                              const Vector<Float>& yIn) const
{
   Int method = 1;                         // Linear
   Vector<Float> yOut;
   Vector<Bool> mOut;
//
   Vector<Bool> mIn(xIn.nelements(),True);
//
   InterpolateArray1D<Float,Float>::interpolate(yOut, mOut, xOut,
                                                 xIn, yIn, mIn,
                                                 method, True, True);
//
   return yOut;
}

void SDAttr::initData () 
//
// Mopra data from Mopra web page
// Tid  data from Tid web page
// X in GHz
//
{

// Beam efficiency

   MopEtaBeamX_.resize(3);
   MopEtaBeamX_(0) = 86.0;
   MopEtaBeamX_(1) = 100.0;
   MopEtaBeamX_(2) = 115.0;
//
   MopEtaBeam2003Y_.resize(3);
   MopEtaBeam2003Y_(0) = 0.39;
   MopEtaBeam2003Y_(1) = 0.37;
   MopEtaBeam2003Y_(2) = 0.37;                // replicated from (1)
//
   MopEtaBeam2004Y_.resize(3);
   MopEtaBeam2004Y_(0) = 0.49;
   MopEtaBeam2004Y_(1) = 0.44;
   MopEtaBeam2004Y_(2) = 0.42;

// Aperture efficiency

   MopEtaApX_.resize(2);
   MopEtaApX_(0) = 86.0;
   MopEtaApX_(1) = 115.0;
//
   MopEtaAp2004Y_.resize(2);
   MopEtaAp2004Y_(0) = 0.33;
   MopEtaAp2004Y_(1) = 0.24;
//
   TidEtaApX_.resize(2);
   TidEtaApY_.resize(2);
   TidEtaApX_(0) = 18.0;
   TidEtaApX_(1) = 26.5;
   TidEtaApY_(0) = 0.4848;
   TidEtaApY_(1) = 0.4848;

// Gain elevation correction polynomial coefficients (for elevation in degrees)

   TidGainElPoly_.resize(3);
   TidGainElPoly_(0) = 3.58788e-1;
   TidGainElPoly_(1) = 2.87243e-2;
   TidGainElPoly_(2) = -3.219093e-4;
}


Instrument SDAttr::convertInstrument(const String& instrument,
                                     Bool throwIt)
{
   String t(instrument);
   t.upcase();

   // The strings are what SDReader returns, after cunning interrogation
   // of station names... :-(
   Instrument inst = asap::UNKNOWNINST;
   if (t==String("DSS-43")) {
      inst = TIDBINBILLA;
   } else if (t==String("ATPKSMB")) {
      inst = ATPKSMB;
   } else if (t==String("ATPKSHOH")) {
      inst = ATPKSHOH;
   } else if (t==String("ATMOPRA")) {
      inst = ATMOPRA;
   } else if (t==String("CEDUNA")) {
      inst = CEDUNA;
   } else if (t==String("HOBART")) {
      inst = HOBART;
   } else {
     if (throwIt) {
       throw AipsError("Unrecognized instrument - use function scan.set_instrument to set");
     }
   }
   return inst;
}


