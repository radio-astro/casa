//#---------------------------------------------------------------------------
//# PKSrecord.h: Class to store an MBFITS single-dish data record.
//#---------------------------------------------------------------------------
//# livedata - processing pipeline for single-dish, multibeam spectral data.
//# Copyright (C) 2000-2009, Australia Telescope National Facility, CSIRO
//#
//# This file is part of livedata.
//#
//# livedata is free software: you can redistribute it and/or modify it under
//# the terms of the GNU General Public License as published by the Free
//# Software Foundation, either version 3 of the License, or (at your option)
//# any later version.
//#
//# livedata is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with livedata.  If not, see <http://www.gnu.org/licenses/>.
//#
//# Correspondence concerning livedata may be directed to:
//#        Internet email: mcalabre@atnf.csiro.au
//#        Postal address: Dr. Mark Calabretta
//#                        Australia Telescope National Facility, CSIRO
//#                        PO Box 76
//#                        Epping NSW 1710
//#                        AUSTRALIA
//#
//# http://www.atnf.csiro.au/computing/software/livedata.html
//# $Id: PKSrecord.h,v 1.2 2009-09-29 07:33:39 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2008/11/14, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSRECORD_H
#define ATNF_PKSRECORD_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

// <summary>
// Class to store an MBFITS single-dish data record.
// </summary>

// Essentially just a struct used as a function argument.
class PKSrecord
{
  public:
    Int             scanNo;
    Int             cycleNo;
    Double          mjd;
    Double          interval;
    String          fieldName;
    String          srcName;
    Vector<Double>  srcDir;
    Vector<Double>  srcPM;
    Double          srcVel;
    String          obsType;
    Int             IFno;
    Double          refFreq;
    Double          bandwidth;
    Double          freqInc;
    Int             nchan;
    Vector<Double>  restFreq;
    Vector<Float>   tcal;
    String          tcalTime;
    Float           azimuth;
    Float           elevation;
    Float           parAngle;
    Float           focusAxi;
    Float           focusTan;
    Float           focusRot;
    Float           temperature;
    Float           pressure;
    Float           humidity;
    Float           windSpeed;
    Float           windAz;
    Int             refBeam;
    Int             beamNo;
    Vector<Double>  direction;
    Int             pCode;
    Float           rateAge;
    Vector<Double>  scanRate;
    Float           paRate;
    Vector<Float>   tsys;
    Vector<Float>   sigma;
    Vector<Float>   calFctr;
    Matrix<Float>   baseLin;
    Matrix<Float>   baseSub;
    Matrix<Float>   spectra;
    Matrix<uChar>   flagged;
    uInt            flagrow;
    Complex         xCalFctr;
    Vector<Complex> xPol;
    Int             polNo ;
    Int             srcType ;
};

#endif
