//# VBContinuumSubtractor.cc:  Subtract continuum from spectral line data
//# Copyright (C) 2004
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
//#
//#include <casa/Arrays/ArrayLogical.h>
//#include <casa/Arrays/ArrayMath.h>
//#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Cube.h>
//#include <casa/Arrays/MaskedArray.h>
//#include <casa/Arrays/MaskArrMath.h>
//#include <casa/Containers/Record.h>
//#include <casa/Containers/RecordFieldId.h>
//#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
//#include <casa/Quanta/MVTime.h>
//#include <casa/Quanta/QuantumHolder.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <msvis/MSVis/VBContinuumSubtractor.h>
#include <msvis/MSVis/VisBuffGroupAcc.h>
#include <msvis/MSVis/CalVisBuffer.h>
#include <scimath/Fitting/LinearFitSVD.h>
#include <scimath/Functionals/Polynomial.h>

//#include <algorithm>

namespace casa { //# NAMESPACE CASA - BEGIN

VBContinuumSubtractor::VBContinuumSubtractor():
  fitorder_p(-1),
  lofreq_p(-1.0),
  hifreq_p(-1.0),
  midfreq_p(-1.0),
  freqscale_p(1.0),
  maxAnt_p(-1),
  nHashes_p(0),
  ncorr_p(0)
{
}

VBContinuumSubtractor::VBContinuumSubtractor(const Double lofreq,
                                             const Double hifreq):
  fitorder_p(-1),
  lofreq_p(lofreq),
  hifreq_p(hifreq),
  midfreq_p(-1.0),
  freqscale_p(1.0),
  maxAnt_p(-1),
  nHashes_p(0),
  ncorr_p(0)
{
  midfreq_p = 0.5 * (lofreq + hifreq);
  freqscale_p = calcFreqScale();
}

void VBContinuumSubtractor::fit(VisBuffGroupAcc& vbga, const Int fitorder,
                                Cube<Complex>& coeffs, Cube<Bool>& coeffsOK)
{
  LogIO os(LogOrigin("VBContinuumSubtractor", "VBContinuumSubtractor()", WHERE));

  fitorder_p = fitorder;

  // Make the estimate
  // Initialize the baseline index
  maxAnt_p = 0;
  for(Int ibuf = 0; ibuf < vbga.nBuf(); ++ibuf)
    maxAnt_p = max(static_cast<Int>(maxAnt_p), max(vbga(ibuf).antenna2()));
  nHashes_p = (maxAnt_p * (maxAnt_p + 3)) / 2;  // Allows for autocorrs.

  ncorr_p = vbga(0).nCorr();

  LinearFitSVD<Float> fitter;
 
  // An nth order polynomial has n + 1 coefficients.
  coeffs.resize(ncorr_p, fitorder_p + 1, nHashes_p);
  // Calibrater wants coeffsOK to be a Cube, even though a Matrix would do for
  // VBContinuumSubtractor.  Let's see whether coeffsOK really has to have the
  // same shape as coeffs.
  //coeffsOK.resize(ncorr_p, fitorder_p + 1, nHashes_p);
  coeffsOK.resize(ncorr_p, 1, nHashes_p);
  coeffsOK.set(False);

  // Translate vbga to arrays for use by LinearFitSVD.
  // First count the total number of channels, and get the minimum and maximum
  // frequencies for scaling.
  uInt totnumchan = 0;
  hifreq_p = -1.0;
  lofreq_p = DBL_MAX;
  for(Int ibuf = 0; ibuf < vbga.nBuf(); ++ibuf){
    CalVisBuffer& cvb(vbga(ibuf));

    totnumchan += cvb.nChannel();
    getMinMaxFreq(cvb, lofreq_p, hifreq_p, False);
  }
  // The fitorder will actually be clamped on a baseline-by-baseline basis
  // because of flagging, but a summary note is in order here.
  if(static_cast<Int>(totnumchan) < fitorder_p)
    os << LogIO::WARN
       << "fitorder = " << fitorder_p
       << ", but only " << totnumchan << " channels were selected.\n"
       << "The polynomial order will be lowered accordingly."
       << LogIO::POST;
  // Scale frequencies to [-1, 1].
  midfreq_p = 0.5 * (lofreq_p + hifreq_p);
  freqscale_p = calcFreqScale();
  Vector<Float> freqs(totnumchan);
  uInt totchan = 0;
  for(Int ibuf = 0; ibuf < vbga.nBuf(); ++ibuf){
    CalVisBuffer& cvb(vbga(ibuf));
    Vector<Double> freq(cvb.frequency());
    uInt nchan = cvb.nChannel();

    for(uInt c = 0; c < nchan; ++c){
      freqs[totchan] = freqscale_p * (freq[c] - midfreq_p);
      ++totchan;
    }
  }

  Vector<Float> sigma(totnumchan);
  Vector<Float> unflaggedfreqs(totnumchan);
  Vector<Complex> vizzes(totnumchan);
  Vector<Float> floatvs(totnumchan);
  for(uInt corrind = 0; corrind < ncorr_p; ++corrind){
    for(uInt blind = 0; blind < nHashes_p; ++blind){
      // Fill sigma and vizzes with the baseline's values for all channels
      // being used in the fit.
      uInt totchan = 0;
      uInt totunflaggedchan = 0;
      sigma.resize(totnumchan);
      vizzes.resize(totnumchan);
      unflaggedfreqs.resize(totnumchan);
      for(Int ibuf = 0; ibuf < vbga.nBuf(); ++ibuf){
        CalVisBuffer& cvb(vbga(ibuf));
        uInt nchan = cvb.nChannel();
        Float sig;

        // 2/24/2011: VisBuffer doesn't (yet) have sigmaSpectrum, and I have
        // never seen it in an MS anyway.  Settle for 1/sqrt(weightSpectrum) if
        // it is available or sigmaMat otherwise.
        const Bool haveWS = cvb.existsWeightSpectrum();

        if(!haveWS)
          sig = cvb.sigmaMat()(corrind, blind);
        for(uInt c = 0; c < nchan; ++c){
          if(!cvb.flagCube()(corrind, c, blind)){
            unflaggedfreqs[totunflaggedchan] = freqs[totchan];
            if(haveWS){
              Double ws = cvb.weightSpectrum()(corrind, c, blind);

              sigma[totunflaggedchan] = ws != 0.0 ? 1.0 / sqrt(ws) : sig;
            }
            else
              sigma[totunflaggedchan] = sig;
            vizzes[totunflaggedchan] = cvb.visCube()(corrind, c, blind);
            ++totunflaggedchan;
          }
          ++totchan;
        }
      }
      if(totunflaggedchan > 0){
        coeffsOK(corrind, 0, blind) = True;

        sigma.resize(totunflaggedchan, True);
        //vizzes.resize(totunflaggedchan, True);
        floatvs.resize(totunflaggedchan);
        unflaggedfreqs.resize(totunflaggedchan, True);

        // perform least-squares fit
        // The way LinearFit is templated, "y" can be Complex, but at the cost
        // of "x" being Complex as well, and worse, sigma too.  It is better to
        // seperately fit the reals and imags.
        Int locFitOrd = min(fitorder_p, static_cast<Int>(totunflaggedchan));
        Polynomial<AutoDiff<Float> > pnom(locFitOrd);

        // Do reals.
        for(Int ordind = 0; ordind <= locFitOrd; ++ordind)       // Note <=.
          pnom.setCoefficient(ordind, 1.0);

        for(uInt c = 0; c < totunflaggedchan; ++c)
          floatvs[c] = vizzes[c].real();

        fitter.setFunction(pnom);
        Vector<Float> realsolution = fitter.fit(freqs, floatvs, sigma);

        // Do imags.
        for(Int ordind = 0; ordind <= locFitOrd; ++ordind)       // Note <=.
          pnom.setCoefficient(ordind, 1.0);

        for(uInt c = 0; c < totunflaggedchan; ++c)
          floatvs[c] = vizzes[c].imag();

        fitter.setFunction(pnom);
        Vector<Float> imagsolution = fitter.fit(freqs, floatvs, sigma);

        for(Int ordind = 0; ordind <= locFitOrd; ++ordind)       // Note <=.
          coeffs(corrind, ordind, blind) = Complex(realsolution[ordind],
                                                   imagsolution[ordind]);
        // Pad remaining orders (if any) with 0.0.  Note <=.
        for(Int ordind = locFitOrd + 1; ordind <= fitorder_p; ++ordind)
          coeffs(corrind, ordind, blind) = 0.0;

        // TODO: store uncertainties
      }
    }
  }
}

VBContinuumSubtractor::~VBContinuumSubtractor()
{}

void VBContinuumSubtractor::getMinMaxFreq(VisBuffer& vb,
                                          Double& minfreq,
                                          Double& maxfreq,
                                          const Bool initialize)
{
  const Vector<Double>& freq(vb.frequency());
  Int hichan = vb.nChannel() - 1;
  Int lochan = 0;

  if(initialize){
    maxfreq = -1.0;
    minfreq = DBL_MAX;
  }
  
  if(freq[hichan] < freq[lochan]){
    lochan = hichan;
    hichan = 0;
  }
  if(freq[hichan] > maxfreq)
    maxfreq = freq[hichan];
  if(freq[lochan] < minfreq)
    minfreq = freq[lochan];
}

Bool VBContinuumSubtractor::areFreqsInBounds(VisBuffer& vb,
                                             const Bool squawk) const
{
  Double maxfreq, minfreq;
  
  getMinMaxFreq(vb, minfreq, maxfreq);
  Bool result = minfreq >= lofreq_p && maxfreq <= hifreq_p;

  if(squawk && !result){
    LogIO os(LogOrigin("VBContinuumSubtractor", "areFreqsInBounds"));

    os << LogIO::WARN
       << "The frequency range [" << 1.0e-9 * minfreq << ", "
       << 1.0e-9 * maxfreq << "] (GHz)\n"
       << "is outside the one used for the continuum fit: ["
       << 1.0e-9 * lofreq_p << ", "
       << 1.0e-9 * hifreq_p << "] (GHz)"
       << LogIO::POST;
  }
  return result;
}

Bool VBContinuumSubtractor::doShapesMatch(VisBuffer& vb,
                                          LogIO& os, const Bool squawk) const
{
  Bool theydo = True;

  if(vb.nCorr() != static_cast<Int>(ncorr_p)){
    theydo = False;
    if(squawk)
      os << LogIO::SEVERE
         << "The supplied number of correlations, " << vb.nCorr()
         << ", does not match the expected " << ncorr_p
         << LogIO::POST;
  }
  // It's no longer the number of rows that matter but the maximum antenna number.
  // if(vb.nRow() != nrow_p){
  if(max(vb.antenna2()) > maxAnt_p){
    theydo = False;                     // Should it just flag unknown baselines?
    if(squawk)
      os << LogIO::SEVERE
         << "The fit is only valid for antennas with indices <= " << maxAnt_p
         << LogIO::POST;
  }
  return theydo;
}

// Do the subtraction
Bool VBContinuumSubtractor::apply(VisBuffer& vb,
                                  const MS::PredefinedColumns whichcol,
                                  const Cube<Complex>& coeffs,
                                  const Cube<Bool>& coeffsOK,
                                  const Bool doSubtraction,
                                  const Bool squawk)
{
  LogIO os(LogOrigin("VBContinuumSubtractor", "cont_subtracted"));

  if(!doShapesMatch(vb, os, squawk))
    return False;
    
  Bool ok = areFreqsInBounds(vb, squawk); // A Bool might be too Boolean here.

  if(!(whichcol == MS::DATA || whichcol == MS::MODEL_DATA ||
       whichcol == MS::CORRECTED_DATA)){
    if(squawk)
      os << LogIO::SEVERE
         << MS::columnName(whichcol) << " is not supported."
         << LogIO::POST;
    return False;
  }
  Cube<Complex>& viscube(vb.dataCube(whichcol));
  
  uInt nchan = vb.nChannel();
  Vector<Double> freqpow(fitorder_p + 1);           // sf**ordind
  freqpow[0] = 1.0;
  Vector<Double>& freq(vb.frequency());

  for(uInt c = 0; c < nchan; ++c){
    Double sf = freqscale_p * (freq[c] - midfreq_p);      // scaled frequency
  
    for(Int ordind = 1; ordind <= fitorder_p; ++ordind)
      freqpow[ordind] = sf * freqpow[ordind - 1];

    for(uInt blind = 0; blind < nHashes_p; ++blind){
      vb.flagRow()(blind) = false;

      for(uInt corrind = 0; corrind < ncorr_p; ++corrind){
        if(coeffsOK(corrind, 0, blind)){
          Complex cont = coeffs(corrind, 0, blind);

          vb.flagRow()(blind) = false;
          for(Int ordind = 1; ordind <= fitorder_p; ++ordind)
            cont += coeffs(corrind, ordind, blind) * freqpow[ordind];
          if(doSubtraction)
            viscube(corrind, c, blind) -= cont;
          else
            viscube(corrind, c, blind) = cont;

          // TODO: Adjust WEIGHT_SPECTRUM (create if necessary?), WEIGHT, and SIGMA.
        }
      }
    }
  }
  return ok;
}

} //# NAMESPACE CASA - END

