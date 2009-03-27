//#---------------------------------------------------------------------------
//# PKSSDWriter.h: Class to write Parkes multibeam data to an SDFITS file.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
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
//# Original: 2000/07/21, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSSDWRITER_H
#define ATNF_PKSSDWRITER_H

#include <atnf/PKSIO/PKSwriter.h>
#include <atnf/PKSIO/SDFITSwriter.h>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>

// <summary>
// Class to write Parkes multibeam data to an SDFITS file.
// </summary>

#include <casa/namespace.h>
class PKSSDwriter : public PKSwriter
{
  public:
    // Default constructor.
    PKSSDwriter();

    // Destructor.
    virtual ~PKSSDwriter();

    // Create the SDFITS file and write static data.
    virtual Int create(
        const String sdName,
        const String observer,
        const String project,
        const String antName,
        const Vector<Double> antPosition,
        const String obsMode,
        const Float  equinox,
        const String dopplerFrame,
        const Vector<uInt> nChan,
        const Vector<uInt> nPol,
        const Vector<Bool> haveXPol,
        const Bool   haveBase,
        const String fluxUnit);

    // Write the next data record.
    virtual Int write(
        const Int             scanNo,
        const Int             cycleNo,
        const Double          mjd,
        const Double          interval,
        const String          fieldName,
        const String          srcName,
        const Vector<Double>  srcDir,
        const Vector<Double>  srcPM,
        const Double          srcVel,
        const String          obsMode,
        const Int             IFno,
        const Double          refFreq,
        const Double          bandwidth,
        const Double          freqInc,
        //const Double          restFreq,
        const Vector<Double>  restFreq,
        const Vector<Float>   tcal,
        const String          tcalTime,
        const Float           azimuth,
        const Float           elevation,
        const Float           parAngle,
        const Float           focusAxi,
        const Float           focusTan,
        const Float           focusRot,
        const Float           temperature,
        const Float           pressure,
        const Float           humidity,
        const Float           windSpeed,
        const Float           windAz,
        const Int             refBeam,
        const Int             beamNo,
        const Vector<Double>  direction,
        const Vector<Double>  scanRate,
        const Vector<Float>   tsys,
        const Vector<Float>   sigma,
        const Vector<Float>   calFctr,
        const Matrix<Float>   baselin,
        const Matrix<Float>   basesub,
        const Matrix<Float>   &spectra,
        const Matrix<uChar>   &flagged,
        const Complex         xCalFctr,
        const Vector<Complex> &xPol);

    // Close the SDFITS file.
    virtual void close();

  private:
    // Masks declaration in parent class.
    Vector<uInt> cHaveXPol;

    SDFITSwriter cSDwriter;
};

#endif
