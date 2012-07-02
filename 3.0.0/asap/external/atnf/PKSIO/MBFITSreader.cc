//#---------------------------------------------------------------------------
//# MBFITSreader.cc: ATNF single-dish RPFITS reader.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
//# Mark Calabretta, ATNF
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
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: mcalabre@atnf.csiro.au.
//#        Postal address: Dr. Mark Calabretta,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id$
//#---------------------------------------------------------------------------
//# The MBFITSreader class reads single dish RPFITS files (such as Parkes
//# Multibeam MBFITS files).
//#
//# Original: 2000/07/28 Mark Calabretta
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/MBFITSreader.h>
#include <atnf/PKSIO/PKSMBrecord.h>

#include <RPFITS.h>

#include <casa/math.h>
#include <casa/iostream.h>
#include <casa/stdio.h>
#include <casa/stdlib.h>
#include <casa/string.h>
#include <unistd.h>

using namespace std;

// Numerical constants.
const double PI = 3.141592653589793238462643;
const double TWOPI = 2.0 * PI;

//------------------------------------------------- MBFITSreader::MBFITSreader

// Default constructor.

MBFITSreader::MBFITSreader(
        const int retry,
        const int interpolate)
{
  cRetry = retry;
  if (cRetry > 10) {
    cRetry = 10;
  }

  cInterp = interpolate;
  if (cInterp < 0 || cInterp > 2) {
    cInterp = 1;
  }

  // Initialize pointers.
  cBeams     = 0x0;
  cIFs       = 0x0;
  cNChan     = 0x0;
  cNPol      = 0x0;
  cHaveXPol  = 0x0;
  cStartChan = 0x0;
  cEndChan   = 0x0;
  cRefChan   = 0x0;

  cVis = new float[2*4*8163];

  cBeamSel   = 0x0;
  cIFSel     = 0x0;
  cChanOff   = 0x0;
  cXpolOff   = 0x0;
  cBuffer    = 0x0;
  cPosUTC    = 0x0;

  cMBopen = 0;
  jstat = -3;
}

//------------------------------------------------ MBFITSreader::~MBFITSreader

// Destructor.

MBFITSreader::~MBFITSreader()
{
  close();
}

//--------------------------------------------------------- MBFITSreader::open

// Open the RPFITS file for reading.

int MBFITSreader::open(
        char *rpname,
        int  &nBeam,
        int* &beams,
        int  &nIF,
        int* &IFs,
        int* &nChan,
        int* &nPol,
        int* &haveXPol,
        int  &haveBase,
        int  &haveSpectra,
        int  &extraSysCal)
{
  if (cMBopen) {
    close();
  }

  strcpy(names_.file, rpname);

  // Open the RPFITS file.
  rpfitsin_(&jstat, cVis, weight, &baseline, &ut, &u, &v, &w, &flag, &bin,
            &if_no, &sourceno);

  if (jstat) {
    fprintf(stderr, "Failed to open MBFITS file: %s\n", rpname);
    return 1;
  }

  cMBopen = 1;

  // Tell RPFITSIN that we want the OBSTYPE card.
  int j;
  param_.ncard = 1;
  for (j = 0; j < 80; j++) {
    names_.card[j] = ' ';
  }
  strncpy(names_.card, "OBSTYPE", 7);

  // Read the first header.
  jstat = -1;
  rpfitsin_(&jstat, cVis, weight, &baseline, &ut, &u, &v, &w, &flag, &bin,
            &if_no, &sourceno);

  if (jstat) {
    fprintf(stderr, "Failed to read MBFITS header: %s\n", rpname);
    close();
    return 1;
  }

  // Mopra data has some peculiarities.
  cMopra = strncmp(names_.instrument, "ATMOPRA", 7) == 0;

  // Tidbinbilla data has some more.
  cTid = strncmp(names_.sta, "tid", 3) == 0;
  if (cTid) {
    // Telescope position is stored in the source table.
    cInterp = 0;
  }


  // Find the maximum beam number.
  cNBeam = 0;
  for (int iBeam = 0; iBeam < anten_.nant; iBeam++) {
    if (anten_.ant_num[iBeam] > cNBeam) {
      cNBeam = anten_.ant_num[iBeam];
    }
  }

  if (cNBeam <= 0) {
    fprintf(stderr, "Couldn't determine number of beams.\n");
    close();
    return 1;
  }

  // Construct the beam mask.
  cBeams = new int[cNBeam];
  for (int iBeam = 0; iBeam < cNBeam; iBeam++) {
    cBeams[iBeam] = 0;
  }

  // ...beams present in the data.
  for (int iBeam = 0; iBeam < anten_.nant; iBeam++) {
    cBeams[anten_.ant_num[iBeam] - 1] = 1;
  }

  // Passing back the address of the array allows PKSFITSreader::select() to
  // modify its elements directly.
  nBeam = cNBeam;
  beams = cBeams;


  // Number of IFs.
  cNIF = if_.n_if;
  cIFs = new int[cNIF];
  for (int iIF = 0; iIF < cNIF; iIF++) {
    cIFs[iIF] = 1;
  }

  // Passing back the address of the array allows PKSFITSreader::select() to
  // modify its elements directly.
  nIF = cNIF;
  IFs = cIFs;


  // Number of channels and polarizations.
  cNChan    = new int[cNIF];
  cNPol     = new int[cNIF];
  cHaveXPol = new int[cNIF];
  cGetXPol  = 0;

  int maxProd = 0;
  for (int iIF = 0; iIF < cNIF; iIF++) {
    cNChan[iIF] = if_.if_nfreq[iIF];
    cNPol[iIF]  = if_.if_nstok[iIF];
    cNChan[iIF] -= cNChan[iIF]%2;

    // Do we have cross-polarization data?
    if ((cHaveXPol[iIF] = cNPol[iIF] > 2)) {
      // Cross-polarization data is handled separately.
      cNPol[iIF] = 2;

      // Default is to get it if we have it.
      cGetXPol = 1;
    }

    // Maximum number of spectral products in any IF.
    int nProd = if_.if_nfreq[iIF] * if_.if_nstok[iIF];
    if (maxProd < nProd) maxProd = nProd;
  }

  // Is the vis array declared by RPFITS.h large enough?
  if (8*8193 < maxProd) {
    // Need to allocate more memory for RPFITSIN.
    cVis = new float[2*maxProd];
  }

  nChan    = cNChan;
  nPol     = cNPol;
  haveXPol = cHaveXPol;


  // Default channel range selection.
  cStartChan = new int[cNIF];
  cEndChan   = new int[cNIF];
  cRefChan   = new int[cNIF];

  for (int iIF = 0; iIF < cNIF; iIF++) {
    cStartChan[iIF] = 1;
    cEndChan[iIF] = cNChan[iIF];
    cRefChan[iIF] = cNChan[iIF]/2 + 1;
  }

  cGetSpectra = 1;


  // No baseline parameters in MBFITS.
  haveBase = 0;

  // Always have spectra in MBFITS.
  haveSpectra = cHaveSpectra = 1;


  // Integration cycle time (s).
  cIntTime = param_.intime;

  // Can't deduce binning mode till later.
  cNBin = 0;


  // Read the first syscal record.
  if (rpget(1, cEOS)) {
    fprintf(stderr, "Error: Failed to read first syscal record.\n");
    close();
    return 1;
  }

  // Additional information for Parkes Multibeam data?
  extraSysCal = (sc_.sc_ant > anten_.nant);


  cFirst = 1;
  cEOF = 0;
  cFlushing = 0;

  return 0;
}

//---------------------------------------------------- MBFITSreader::getHeader

// Get parameters describing the data.

int MBFITSreader::getHeader(
        char   observer[32],
        char   project[32],
        char   telescope[32],
        double antPos[3],
        char   obsType[32],
        float  &equinox,
        char   radecsys[32],
        char   dopplerFrame[32],
        char   datobs[32],
        double &utc,
        double &refFreq,
        double &bandwidth)
{
  if (!cMBopen) {
    fprintf(stderr, "An MBFITS file has not been opened.\n");
    return 1;
  }

  sprintf(observer,  "%-16.16s", names_.rp_observer);
  sprintf(project,   "%-16.16s", names_.object);
  sprintf(telescope, "%-16.16s", names_.instrument);

  // Observatory coordinates (ITRF), in m.
  antPos[0] = doubles_.x[0];
  antPos[1] = doubles_.y[0];
  antPos[2] = doubles_.z[0];

  // This is the only sure way to identify the telescope, maybe.
  if (strncmp(names_.sta, "MB0", 3) == 0) {
    // Parkes Multibeam.
    sprintf(telescope, "%-16.16s", "ATPKSMB");
    antPos[0] = -4554232.087;
    antPos[1] =  2816759.046;
    antPos[2] = -3454035.950;
  } else if (strncmp(names_.sta, "HOH", 3) == 0) {
    // Parkes HOH receiver.
    sprintf(telescope, "%-16.16s", "ATPKSHOH");
    antPos[0] = -4554232.087;
    antPos[1] =  2816759.046;
    antPos[2] = -3454035.950;
  } else if (strncmp(names_.sta, "CA0", 3) == 0) {
    // An ATCA antenna, use the array centre position.
    sprintf(telescope, "%-16.16s", "ATCA");
    antPos[0] = -4750915.837;
    antPos[1] =  2792906.182;
    antPos[2] = -3200483.747;
  } else if (strncmp(names_.sta, "MOP", 3) == 0) {
    // Mopra.
    sprintf(telescope, "%-16.16s", "ATMOPRA");
    antPos[0] = -4682768.630;
    antPos[1] =  2802619.060;
    antPos[2] = -3291759.900;
  } else if (strncmp(names_.sta, "HOB", 3) == 0) {
    // Hobart.
    sprintf(telescope, "%-16.16s", "HOBART");
    antPos[0] = -3950236.735;
    antPos[1] =  2522347.567;
    antPos[2] = -4311562.569;
  } else if (strncmp(names_.sta, "CED", 3) == 0) {
    // Ceduna.
    sprintf(telescope, "%-16.16s", "CEDUNA");
    antPos[0] = -3749943.657;
    antPos[1] =  3909017.709;
    antPos[2] = -3367518.309;
  } else if (strncmp(names_.sta, "tid", 3) == 0) {
    // DSS.
    sprintf(telescope, "%-16.16s", "DSS-43");
    antPos[0] = -4460894.727;
    antPos[1] =  2682361.530;
    antPos[2] = -3674748.424;
  }

  // Observation type.
  int j;
  for (j = 0; j < 31; j++) {
    obsType[j] = names_.card[11+j];
    if (obsType[j] == '\'') break;
  }
  obsType[j] = '\0';

  // Coordinate frames.
  equinox = 2000.0f;
  strcpy(radecsys, "FK5");
  strcpy(dopplerFrame, "TOPOCENT");

  // Time at start of observation.
  sprintf(datobs, "%-10.10s", names_.datobs);
  utc = ut;

  // Spectral parameters.
  refFreq   = doubles_.if_freq[0];
  bandwidth = doubles_.if_bw[0];

  return 0;
}

//-------------------------------------------------- MBFITSreader::getFreqInfo

// Get frequency parameters for each IF.

int MBFITSreader::getFreqInfo(
        int     &nIF,
        double* &startFreq,
        double* &endFreq)
{
  // This is RPFITS - can't do it!
  return 1;
}

//---------------------------------------------------- MBFITSreader::findRange

// Find the range of the data selected in time and position.

int MBFITSreader::findRange(
        int    &nRow,
        int    &nSel,
        char   dateSpan[2][32],
        double utcSpan[2],
        double* &positions)
{
  // This is RPFITS - can't do it!
  return 1;
}

//--------------------------------------------------------- MBFITSreader::read

// Read the next data record.

int MBFITSreader::read(
        PKSMBrecord &MBrec)
{
  int beamNo = -1;
  int haveData, status;
  PKSMBrecord *iMBuff = 0x0;

  if (!cMBopen) {
    fprintf(stderr, "An MBFITS file has not been opened.\n");
    return 1;
  }

  // Positions recorded in the input records do not coincide with the midpoint
  // of the integration and hence the input must be buffered so that true
  // positions may be interpolated.
  //
  // On the first call nBeamSel buffers of length nBin, are allocated and
  // filled, where nBin is the number of time bins.
  //
  // The input records for binned, single beam data with multiple simultaneous
  // IFs are ordered by IF within each integration rather than by bin number
  // and hence are not in time order.  No multibeam data exists with
  // nBin > 1 but the likelihood that the input records would be in beam/IF
  // order and the requirement that output records be in time order would
  // force an elaborate double-buffering system and we do not support it.
  //
  // Once all buffers are filled, the next record for each beam pertains to
  // the next integration and should contain new position information allowing
  // the proper position for each spectrum in the buffer to be interpolated.
  // The buffers are then flushed in time order.  For single beam data there
  // is only one buffer and reads from the MBFITS file are suspended while the
  // flush is in progress.  For multibeam data each buffer is of unit length
  // so the flush completes immediately and the new record takes its place.

  haveData = 0;
  while (!haveData) {
    int iBeamSel = -1, iIFSel = -1;

    if (!cFlushing) {
      if (cEOF) {
        return -1;
      }

      // Read the next record.
      if ((status = rpget(0, cEOS)) == -1) {
        // EOF.
        cEOF = 1;
        cFlushing = 1;
        cFlushBin = 0;
        cFlushIF  = 0;

#ifdef PKSIO_DEBUG
        printf("End-of-file detected, flushing last scan.\n");
#endif

      } else if (status) {
        // IO error.
        return 1;

      } else {
        if (cFirst) {
          // First data; cBeamSel[] stores the buffer index for each beam.
          cNBeamSel = 0;
          cBeamSel = new int[cNBeam];

          for (int iBeam = 0; iBeam < cNBeam; iBeam++) {
            if (cBeams[iBeam]) {
              // Buffer offset for this beam.
              cBeamSel[iBeam] = cNBeamSel++;
            } else {
              // Signal that the beam is not selected.
              cBeamSel[iBeam] = -1;
            }
          }

          // Set up bookkeeping arrays for IFs.
          cIFSel   = new int[cNIF];
          cChanOff = new int[cNIF];
          cXpolOff = new int[cNIF];

          int simulIF = 0;
          int maxChan = 0;
          int maxXpol = 0;

          for (int iIF = 0; iIF < cNIF; iIF++) {
            if (cIFs[iIF]) {
              // Buffer index for each IF within each simultaneous set.
              cIFSel[iIF] = 0;

              // Array offsets for each IF within each simultaneous set.
              cChanOff[iIF] = 0;
              cXpolOff[iIF] = 0;

              // Look for earlier IFs in the same simultaneous set.
              for (int jIF = 0; jIF < iIF; jIF++) {
                if (!cIFs[jIF]) continue;

                if (if_.if_simul[jIF] == if_.if_simul[iIF]) {
                  // Got one, increment indices.
                  cIFSel[iIF]++;

                  cChanOff[iIF] += cNChan[jIF] * cNPol[jIF];
                  if (cHaveXPol[jIF]) {
                    cXpolOff[iIF] += 2 * cNChan[jIF];
                  }
                }
              }

              // Maximum number of selected IFs in any simultaneous set.
              simulIF = max(simulIF, cIFSel[iIF]+1);

              // Maximum memory required for any simultaneous set.
              maxChan = max(maxChan, cChanOff[iIF] + cNChan[iIF]*cNPol[iIF]);
              if (cHaveXPol[iIF]) {
                maxXpol = max(maxXpol, cXpolOff[iIF] + 2*cNChan[iIF]);
              }

            } else {
              // Signal that the IF is not selected.
              cIFSel[iIF] = -1;
            }
          }

          // Check for binning mode observations.
          if (param_.intbase > 0.0f) {
            cNBin = int((cIntTime / param_.intbase) + 0.5);

            // intbase sometimes contains rubbish.
            if (cNBin == 0) {
              cNBin = 1;
            }
          } else {
            cNBin = 1;
          }

          if (cNBin > 1 && cNBeamSel > 1) {
            fprintf(stderr, "Cannot handle binning mode for multiple "
                            "beams.\n");
            close();
            return 1;
          }

          // Allocate buffer data storage.
          int nBuff = cNBeamSel * cNBin;
          cBuffer = new PKSMBrecord[nBuff];

          // Allocate memory for spectral arrays.
          for (int ibuff = 0; ibuff < nBuff; ibuff++) {
            cBuffer[ibuff].setNIFs(simulIF);
            cBuffer[ibuff].allocate(0, maxChan, maxXpol);
          }

          cPosUTC = new double[cNBeamSel];

          cFirst = 0;
          cScanNo  = 1;
          cCycleNo = 0;
          cUTC = 0.0;
          cStaleness = new int[cNBeamSel];
          for (int iBeamSel = 0; iBeamSel < cNBeamSel; iBeamSel++) {
            cStaleness[iBeamSel] = 0;
          }
        }

        // Check for end-of-scan.
        if (cEOS) {
          cScanNo++;
          cCycleNo = 0;
          cUTC = 0.0;
        }

        // Apply beam selection.
        beamNo = int(baseline / 256.0);
        iBeamSel = cBeamSel[beamNo-1];
        if (iBeamSel < 0) continue;

        // Sanity check (mainly for MOPS).
        if (if_no > cNIF) continue;

        // Apply IF selection.
        iIFSel = cIFSel[if_no - 1];
        if (iIFSel < 0) continue;

        sprintf(cDateObs, "%-10.10s", names_.datobs);

        // Change-of-day; note that the ut variable from RPFITS.h is global
        // and will be preserved between calls to this function.
        if (ut < cUTC - 85800.0) {
          ut += 86400.0;
        }

        // New integration cycle?
        if (ut > cUTC) {
          cCycleNo++;
          cUTC = ut + 0.0001;
        }

        if (cNBin > 1) {
          // Binning mode: correct the time.
          ut += param_.intbase * (bin - (cNBin + 1)/2.0);
        }

        // Compute buffer number.
        iMBuff = cBuffer + iBeamSel;
        if (cNBin > 1) iMBuff += cNBeamSel*(bin-1);

        if (cCycleNo < iMBuff->cycleNo) {
          // Note that if the first beam and IF are not both selected cEOS
          // will be cleared by rpget() when the next beam/IF is read.
          cEOS = 1;
        }

        // Begin flush cycle?
        if (cEOS || (iMBuff->nIF && ut > iMBuff->utc + 0.0001)) {
          cFlushing = 1;
          cFlushBin = 0;
          cFlushIF  = 0;
        }

#ifdef PKSIO_DEBUG
        printf(" In:%4d%4d%3d%3d\n", cScanNo, cCycleNo, beamNo, if_no);
        if (cEOS) printf("Start of new scan, flushing previous scan.\n");
#endif
      }
    }


    if (cFlushing) {
      // Find the oldest integration to flush, noting that the last
      // integration cycle may be incomplete.
      beamNo = 0;
      int cycleNo = 0;
      for (; cFlushBin < cNBin; cFlushBin++) {
        for (iBeamSel = 0; iBeamSel < cNBeamSel; iBeamSel++) {
          iMBuff = cBuffer + iBeamSel + cNBeamSel*cFlushBin;

          // iMBuff->nIF is set to zero (below) to signal that all IFs in
          // an integration have been flushed.
          if (iMBuff->nIF) {
            if (cycleNo == 0 || iMBuff->cycleNo < cycleNo) {
              beamNo  = iMBuff->beamNo;
              cycleNo = iMBuff->cycleNo;
            }
          }
        }

        if (beamNo) {
          // Found an integration to flush.
          break;
        }
      }

      if (beamNo) {
        iBeamSel = cBeamSel[beamNo-1];
        iMBuff = cBuffer + iBeamSel + cNBeamSel*cFlushBin;

        // Find the IF to flush.
        for (; cFlushIF < iMBuff->nIF; cFlushIF++) {
          if (iMBuff->IFno[cFlushIF]) break;
        }

      } else {
        // Flush complete.
        cFlushing = 0;
        if (cEOF) {
          return -1;
        }

        // The last record read must have been the first of a new cycle.
        beamNo = int(baseline / 256.0);
        iBeamSel = cBeamSel[beamNo-1];

        // Compute buffer number.
        iMBuff = cBuffer + iBeamSel;
        if (cNBin > 1) iMBuff += cNBeamSel*(bin-1);
      }
    }


    if (cFlushing && cFlushBin == 0 && cFlushIF == 0 && cInterp) {
      // Interpolate the beam position at the start of the flush cycle.
#ifdef PKSIO_DEBUG
      printf("Doing position interpolation for beam %d.\n", iMBuff->beamNo);
#endif

      double prevRA  = iMBuff->ra;
      double prevDec = iMBuff->dec;
      double prevUTC = cPosUTC[iBeamSel];

      if (!cEOF && !cEOS) {
        // The position is measured by the control system at a time returned
        // by RPFITSIN as the 'w' visibility coordinate.  The ra and dec,
        // returned as the 'u' and 'v' visibility coordinates, must be
        // interpolated to the integration time which RPFITSIN returns as
        // 'ut', this usually being a second or two later.
        //
        // Note that the time recorded as the 'w' visibility coordinate
        // cycles through 86400 back to 0 at midnight, whereas that in 'ut'
        // continues to increase past 86400.

        double thisRA  = u;
        double thisDec = v;
        double thisUTC = w;

        if (thisUTC < prevUTC) {
          // Must have cycled through midnight.
          thisUTC += 86400.0;
        }

        // Guard against RA cycling through 24h in either direction.
        if (fabs(thisRA - prevRA) > PI) {
          if (thisRA < prevRA) {
            thisRA += TWOPI;
          } else {
            thisRA -= TWOPI;
          }
        }

        // The control system at Mopra typically does not update the
        // positions between successive integration cycles at the end of a
        // scan (nor are they flagged).  In this case we use the previously
        // computed rates, even if from the previous scan since these are
        // likely to be a better guess than anything else.

        double dUTC = thisUTC - prevUTC;

        // Scan rate for this beam.
        if (dUTC > 0.0) {
          iMBuff->raRate  = (thisRA  - prevRA)  / dUTC;
          iMBuff->decRate = (thisDec - prevDec) / dUTC;

          if (cInterp == 2) {
            // Use the same interpolation scheme as the original pksmbfits
            // client.  This incorrectly assumed that (thisUTC - prevUTC) is
            // equal to the integration time and interpolated by computing a
            // weighted sum of the positions before and after the required
            // time.

            double utc = iMBuff->utc;
            if (utc - prevUTC > 100.0) {
              // Must have cycled through midnight.
              utc -= 86400.0;
            }

            double tw1 = 1.0 - (utc - prevUTC) / iMBuff->exposure;
            double tw2 = 1.0 - (thisUTC - utc) / iMBuff->exposure;
            double gamma = (tw2 / (tw1 + tw2)) * dUTC / (utc - prevUTC);

            iMBuff->raRate  *= gamma;
            iMBuff->decRate *= gamma;
          }

          cStaleness[iBeamSel] = 0;

        } else {
          // Issue warnings.
          int nch = 0;
          fprintf(stderr, "WARNING, scan %d,%n cycle %d: Position ",
            iMBuff->scanNo, &nch, iMBuff->cycleNo);

          if (dUTC < 0.0) {
            fprintf(stderr, "timestamp went backwards!\n");
          } else {
            if (thisRA != prevRA || thisDec != prevDec) {
              fprintf(stderr, "changed but timestamp unchanged!\n");
            } else {
              fprintf(stderr, "and timestamp unchanged!\n");
            }
          }

          cStaleness[iBeamSel]++;
          fprintf(stderr, "%-*s Using stale scan rate, staleness = %d "
            "cycle%s.\n", nch, "WARNING,", cStaleness[iBeamSel],
            (cStaleness[iBeamSel] == 1) ? "" : "s");

          if (thisRA != prevRA || thisDec != prevDec) {
            if (iMBuff->raRate == 0.0 && iMBuff->decRate == 0.0) {
              fprintf(stderr, "%-*s But the previous rate was zero!  "
                "Position will be inaccurate.\n", nch, "WARNING,");
            }
          }
        }
      }

      // Compute the position of this beam for all bins.
      for (int idx = 0; idx < cNBin; idx++) {
        int jbuff = iBeamSel + cNBeamSel*idx;

        cBuffer[jbuff].raRate  = iMBuff->raRate;
        cBuffer[jbuff].decRate = iMBuff->decRate;

        double dutc = cBuffer[jbuff].utc - prevUTC;
        if (dutc > 100.0) {
          // Must have cycled through midnight.
          dutc -= 86400.0;
        }

        cBuffer[jbuff].ra  = prevRA  + cBuffer[jbuff].raRate  * dutc;
        cBuffer[jbuff].dec = prevDec + cBuffer[jbuff].decRate * dutc;
        if (cBuffer[jbuff].ra < 0.0) {
          cBuffer[jbuff].ra += TWOPI;
        } else if (cBuffer[jbuff].ra > TWOPI) {
          cBuffer[jbuff].ra -= TWOPI;
        }
      }
    }


    if (cFlushing) {
      // Copy buffer location out one IF at a time.
      MBrec.extract(*iMBuff, cFlushIF);
      haveData = 1;

#ifdef PKSIO_DEBUG
      printf("Out:%4d%4d%3d%3d\n", MBrec.scanNo, MBrec.cycleNo, MBrec.beamNo,
        MBrec.IFno[0]);
#endif

      // Signal that this IF in this buffer location has been flushed.
      iMBuff->IFno[cFlushIF] = 0;

      if (cFlushIF == iMBuff->nIF - 1) {
        // Signal that all IFs in this buffer location have been flushed.
        iMBuff->nIF = 0;
      } else {
        // Carry on flushing the other IFs.
        continue;
      }

      // Has the whole buffer been flushed?
      if (cFlushBin == cNBin - 1) {
        if (cEOS || cEOF) {
          // Stop cEOS being set when the next integration is read.
          iMBuff->cycleNo = 0;

          // Carry on flushing other buffers.
          cFlushIF = 0;
          continue;
        }

        cFlushing = 0;

        beamNo = int(baseline / 256.0);
        iBeamSel = cBeamSel[beamNo-1];

        // Compute buffer number.
        iMBuff = cBuffer + iBeamSel;
        if (cNBin > 1) iMBuff += cNBeamSel*(bin-1);
      }
    }

    if (!cFlushing) {
      // Buffer this MBrec.
      if (cCycleNo == 1 && iMBuff->IFno[0]) {
        // Sanity check on the number of IFs in the new scan.
        if (if_.n_if != cNIF) {
          fprintf(stderr, "WARNING, scan %d has %d IFs instead of %d, "
            "continuing.\n", cScanNo, if_.n_if, cNIF);
        }
      }

      iMBuff->scanNo  = cScanNo;
      iMBuff->cycleNo = cCycleNo;

      // Times.
      strncpy(iMBuff->datobs, cDateObs, 10);
      iMBuff->utc = ut;
      iMBuff->exposure = param_.intbase;

      // Source identification.
      sprintf(iMBuff->srcName, "%-16.16s",
              names_.su_name + (sourceno-1)*16);
      iMBuff->srcRA  = doubles_.su_ra[sourceno-1];
      iMBuff->srcDec = doubles_.su_dec[sourceno-1];

      // Rest frequency of the line of interest.
      iMBuff->restFreq = doubles_.rfreq;
      if (strncmp(names_.instrument, "ATPKSMB", 7) == 0) {
        // Fix the HI rest frequency recorded for Parkes multibeam data.
        double reffreq  = doubles_.freq;
        double restfreq = doubles_.rfreq;
        if ((restfreq == 0.0 || fabs(restfreq - reffreq) == 0.0) &&
             fabs(reffreq - 1420.40575e6) < 100.0) {
          iMBuff->restFreq = 1420.40575e6;
        }
      }

      // Observation type.
      int j;
      for (j = 0; j < 15; j++) {
        iMBuff->obsType[j] = names_.card[11+j];
        if (iMBuff->obsType[j] == '\'') break;
      }
      iMBuff->obsType[j] = '\0';

      // Beam-dependent parameters.
      iMBuff->beamNo = beamNo;

      // Beam position at the specified time.
      if (cTid) {
        // Tidbinbilla data.
        iMBuff->ra  = doubles_.su_ra[sourceno-1];
        iMBuff->dec = doubles_.su_dec[sourceno-1];
      } else {
        iMBuff->ra  = u;
        iMBuff->dec = v;
      }
      cPosUTC[iBeamSel] = w;

      // IF-dependent parameters.
      int iIF = if_no - 1;
      int startChan = cStartChan[iIF];
      int endChan   = cEndChan[iIF];
      int refChan   = cRefChan[iIF];

      int nChan = abs(endChan - startChan) + 1;

      iIFSel = cIFSel[iIF];
      iMBuff->nIF++;
      iMBuff->IFno[iIFSel]  = if_no;
      iMBuff->nChan[iIFSel] = nChan;
      iMBuff->nPol[iIFSel]  = cNPol[iIF];

      iMBuff->fqRefPix[iIFSel] = doubles_.if_ref[iIF];
      iMBuff->fqRefVal[iIFSel] = doubles_.if_freq[iIF];
      iMBuff->fqDelt[iIFSel]   =
        if_.if_invert[iIF] * fabs(doubles_.if_bw[iIF] /
          (if_.if_nfreq[iIF] - 1));

      // Adjust for channel selection.
      if (iMBuff->fqRefPix[iIFSel] != refChan) {
        iMBuff->fqRefVal[iIFSel] +=
          (refChan - iMBuff->fqRefPix[iIFSel]) *
            iMBuff->fqDelt[iIFSel];
        iMBuff->fqRefPix[iIFSel] = refChan;
      }

      if (endChan < startChan) {
        iMBuff->fqDelt[iIFSel] = -iMBuff->fqDelt[iIFSel];
      }


      // System temperature.
      int iBeam = beamNo - 1;
      int scq = sc_.sc_q;
      float TsysPol1 = sc_.sc_cal[scq*iBeam + 3];
      float TsysPol2 = sc_.sc_cal[scq*iBeam + 4];
      iMBuff->tsys[iIFSel][0] = TsysPol1*TsysPol1;
      iMBuff->tsys[iIFSel][1] = TsysPol2*TsysPol2;

      // Calibration factor; may be changed later if the data is recalibrated.
      if (scq > 14) {
        // Will only be present for Parkes Multibeam or LBA data.
        iMBuff->calfctr[iIFSel][0] = sc_.sc_cal[scq*iBeam + 14];
        iMBuff->calfctr[iIFSel][1] = sc_.sc_cal[scq*iBeam + 15];
      } else {
        iMBuff->calfctr[iIFSel][0] = 0.0f;
        iMBuff->calfctr[iIFSel][1] = 0.0f;
      }

      // Cross-polarization calibration factor (unknown to MBFITS).
      for (int j = 0; j < 2; j++) {
        iMBuff->xcalfctr[iIFSel][j] = 0.0f;
      }

      // Baseline parameters (unknown to MBFITS).
      iMBuff->haveBase = 0;

      // Data (always present in MBFITS).
      iMBuff->haveSpectra = 1;

      // Flag:  bit 0 set if off source.
      //        bit 1 set if loss of sync in A polarization.
      //        bit 2 set if loss of sync in B polarization.
      unsigned char rpflag =
        (unsigned char)(sc_.sc_cal[scq*iBeam + 12] + 0.5f);

      // The baseline flag may be set independently.
      if (rpflag == 0) rpflag = flag;

      // Copy and scale data.
      int inc = 2 * if_.if_nstok[iIF];
      if (endChan < startChan) inc = -inc;

      float TsysF;
      iMBuff->spectra[iIFSel] = iMBuff->spectra[0] + cChanOff[iIF];
      iMBuff->flagged[iIFSel] = iMBuff->flagged[0] + cChanOff[iIF];

      float *spectra = iMBuff->spectra[iIFSel];
      unsigned char *flagged = iMBuff->flagged[iIFSel];
      for (int ipol = 0; ipol < cNPol[iIF]; ipol++) {
        if (sc_.sc_cal[scq*iBeam + 3 + ipol] > 0.0f) {
          // The correlator has already applied the calibration.
          TsysF = 1.0f;
        } else {
          // The correlator has normalized cVis[k] to a Tsys of 500K.
          TsysF = iMBuff->tsys[iIFSel][ipol] / 500.0f;
        }

        int k = 2 * (if_.if_nstok[iIF]*(startChan - 1) + ipol);
        for (int ichan = 0; ichan < nChan; ichan++) {
          *(spectra++) = TsysF * cVis[k];
          *(flagged++) = rpflag;
          k += inc;
        }
      }

      if (cHaveXPol[iIF]) {
        int k = 2 * (3*(startChan - 1) + 2);
        iMBuff->xpol[iIFSel] = iMBuff->xpol[0] + cXpolOff[iIF];
        float *xpol = iMBuff->xpol[iIFSel];
        for (int ichan = 0; ichan < nChan; ichan++) {
          *(xpol++) = cVis[k];
          *(xpol++) = cVis[k+1];
          k += inc;
        }
      }


      // Parallactic angle.
      iMBuff->parAngle = sc_.sc_cal[scq*iBeam + 11];

      // Calibration factor applied to the data by the correlator.
      if (scq > 14) {
        // Will only be present for Parkes Multibeam or LBA data.
        iMBuff->tcal[iIFSel][0] = sc_.sc_cal[scq*iBeam + 14];
        iMBuff->tcal[iIFSel][1] = sc_.sc_cal[scq*iBeam + 15];
      } else {
        iMBuff->tcal[iIFSel][0] = 0.0f;
        iMBuff->tcal[iIFSel][1] = 0.0f;
      }

      if (sc_.sc_ant <= anten_.nant) {
        // No extra syscal information present.
        iMBuff->extraSysCal = 0;
        iMBuff->azimuth   = 0.0f;
        iMBuff->elevation = 0.0f;
        iMBuff->parAngle  = 0.0f;
        iMBuff->focusAxi  = 0.0f;
        iMBuff->focusTan  = 0.0f;
        iMBuff->focusRot  = 0.0f;
        iMBuff->temp      = 0.0f;
        iMBuff->pressure  = 0.0f;
        iMBuff->humidity  = 0.0f;
        iMBuff->windSpeed = 0.0f;
        iMBuff->windAz    = 0.0f;
        strcpy(iMBuff->tcalTime, "                ");
        iMBuff->refBeam = 0;

      } else {
        // Additional information for Parkes Multibeam data.
        int iOff = scq*(sc_.sc_ant - 1) - 1;
        iMBuff->extraSysCal = 1;
        iMBuff->azimuth   = sc_.sc_cal[iOff + 2];
        iMBuff->elevation = sc_.sc_cal[iOff + 3];
        iMBuff->parAngle  = sc_.sc_cal[iOff + 4];
        iMBuff->focusAxi  = sc_.sc_cal[iOff + 5] * 1e-3;
        iMBuff->focusTan  = sc_.sc_cal[iOff + 6] * 1e-3;
        iMBuff->focusRot  = sc_.sc_cal[iOff + 7];
        iMBuff->temp      = sc_.sc_cal[iOff + 8];
        iMBuff->pressure  = sc_.sc_cal[iOff + 9];
        iMBuff->humidity  = sc_.sc_cal[iOff + 10];
        iMBuff->windSpeed = sc_.sc_cal[iOff + 11];
        iMBuff->windAz    = sc_.sc_cal[iOff + 12];

        char *tcalTime = iMBuff->tcalTime;
        sprintf(tcalTime, "%-16.16s", (char *)(&sc_.sc_cal[iOff+13]));

#ifndef AIPS_LITTLE_ENDIAN
        // Do byte swapping on the ASCII date string.
        for (int j = 0; j < 16; j += 4) {
          char ctmp;
          ctmp = tcalTime[j];
          tcalTime[j]   = tcalTime[j+3];
          tcalTime[j+3] = ctmp;
          ctmp = tcalTime[j+1];
          tcalTime[j+1] = tcalTime[j+2];
          tcalTime[j+2] = ctmp;
        }
#endif

        // Reference beam number.
        float refbeam = sc_.sc_cal[iOff + 17];
        if (refbeam > 0.0f || refbeam < 100.0f) {
          iMBuff->refBeam = int(refbeam);
        } else {
          iMBuff->refBeam = 0;
        }
      }
    }
  }

  return 0;
}

//-------------------------------------------------------- MBFITSreader::rpget

// Read the next data record from the RPFITS file.

int MBFITSreader::rpget(int syscalonly, int &EOS)
{
  EOS = 0;

  int retries = 0;

  // Allow 10 read errors.
  int numErr = 0;

  jstat = 0;
  while (numErr < 10) {
    int lastjstat = jstat;
    rpfitsin_(&jstat, cVis, weight, &baseline, &ut, &u, &v, &w, &flag, &bin,
              &if_no, &sourceno);

    switch(jstat) {
    case -1:
      // Read failed; retry.
      numErr++;
      fprintf(stderr, "RPFITS read failed - retrying.\n");
      jstat = 0;
      break;

    case 0:
      // Successful read.
      if (lastjstat == 0) {
        if (baseline == -1) {
          // Syscal data.
          if (syscalonly) {
            return 0;
          }

        } else {
          if (!syscalonly) {
            return 0;
          }
        }
      }

      // Last operation was to read header or FG table; now read data.
      break;

    case 1:
      // Encountered header while trying to read data; read it.
      EOS = 1;
      jstat = -1;
      break;

    case 2:
      // End of scan; read past it.
      jstat = 0;
      break;

    case 3:
      // End-of-file; retry applies to real-time mode.
      if (retries++ >= cRetry) {
        return -1;
      }

      sleep(10);
      jstat = 0;
      break;

    case 4:
      // Encountered FG table while trying to read data; read it.
      jstat = -1;
      break;

    case 5:
      // Illegal data at end of block after close/reopen operation; retry.
      jstat = 0;
      break;

    default:
      // Shouldn't reach here.
      fprintf(stderr, "Unrecognized RPFITSIN return code: %d (retrying)\n",
              jstat);
      jstat = 0;
      break;
    }
  }

  fprintf(stderr, "RPFITS read failed too many times.\n");
  return 2;
}

//-------------------------------------------------------- MBFITSreader::close

// Close the input file.

void MBFITSreader::close(void)
{
  if (cMBopen) {
    jstat = 1;
    rpfitsin_(&jstat, cVis, weight, &baseline, &ut, &u, &v, &w, &flag, &bin,
              &if_no, &sourceno);

    if (cBeams)     delete [] cBeams;
    if (cIFs)       delete [] cIFs;
    if (cNChan)     delete [] cNChan;
    if (cNPol)      delete [] cNPol;
    if (cHaveXPol)  delete [] cHaveXPol;
    if (cStartChan) delete [] cStartChan;
    if (cEndChan)   delete [] cEndChan;
    if (cRefChan)   delete [] cRefChan;

    if (cVis)       delete [] cVis;

    if (cBeamSel)   delete [] cBeamSel;
    if (cIFSel)     delete [] cIFSel;
    if (cChanOff)   delete [] cChanOff;
    if (cXpolOff)   delete [] cXpolOff;
    if (cBuffer)    delete [] cBuffer;
    if (cPosUTC)    delete [] cPosUTC;

    cMBopen = 0;
  }
}
