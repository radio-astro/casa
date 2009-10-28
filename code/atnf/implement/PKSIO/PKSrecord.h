//#---------------------------------------------------------------------------
//# PKSrecord.h: Class to store an MBFITS single-dish data record.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2008
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
//# $Id: PKSrecord.h,v 1.1 2008-11-17 06:46:00 cal103 Exp $
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
    Complex         xCalFctr;
    Vector<Complex> xPol;
    Int             polNo ;
};

#endif
