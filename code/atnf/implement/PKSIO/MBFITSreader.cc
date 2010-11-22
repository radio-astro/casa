//#---------------------------------------------------------------------------
//# MBFITSreader.cc: ATNF single-dish RPFITS reader.
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
//# $Id: MBFITSreader.cc,v 19.57 2009-10-30 06:34:36 cal103 Exp $
//#---------------------------------------------------------------------------
//# The MBFITSreader class reads single dish RPFITS files (such as Parkes
//# Multibeam MBFITS files).
//#
//# Original: 2000/07/28 Mark Calabretta
//#---------------------------------------------------------------------------

#include <atnf/pks/pks_maths.h>
#include <atnf/PKSIO/MBFITSreader.h>
#include <atnf/PKSIO/MBrecord.h>

#include <casa/Logging/LogIO.h>

#include <casa/math.h>
#include <casa/iostream.h>
#include <casa/stdio.h>
#include <casa/stdlib.h>
#include <casa/string.h>
#include <unistd.h>

#include <RPFITS.h>

using namespace std;

// Numerical constants.
const double PI = 3.141592653589793238462643;
const double TWOPI = 2.0 * PI;
const double HALFPI = PI / 2.0;
const double R2D = 180.0 / PI;

// Class name
const string className = "MBFITSreader" ;

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

  cVis = 0x0;
  cWgt = 0x0;

  cBeamSel   = 0x0;
  cIFSel     = 0x0;
  cChanOff   = 0x0;
  cXpolOff   = 0x0;
  cBuffer    = 0x0;
  cPosUTC    = 0x0;

  cMBopen = 0;

  // Tell RPFITSIN not to report errors directly.
  //iostat_.errlun = -1;
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
  const string methodName = "open()" ;
  LogIO os( LogOrigin( className, methodName, WHERE ) ) ;

  if (cMBopen) {
    close();
  }

  strcpy(names_.file, rpname);

  // Open the RPFITS file.
  int jstat = -3;
  if (rpfitsin(jstat)) {
    sprintf(cMsg, "Failed to open MBFITS file\n%s", rpname);
    //os << LogIO::SEVERE << cMsg << LogIO::POST ;
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
  if (rpfitsin(jstat)) {
    sprintf(cMsg, "Failed to read MBFITS header in file\n"
                  "%s", rpname);
    //os << LogIO::SEVERE << cMsg << LogIO::POST ;
    close();
    return 1;
  }

  // Mopra data has some peculiarities.
  cMopra = strncmp(names_.instrument, "ATMOPRA", 7) == 0;

  // Non-ATNF data may not store the position in (u,v,w).
  if (strncmp(names_.sta, "tid", 3) == 0) {
    sprintf(cMsg, "Found Tidbinbilla data");
    cSUpos = 1;
  } else if (strncmp(names_.sta, "HOB", 3) == 0) {
    sprintf(cMsg, "Found Hobart data");
    cSUpos = 1;
  } else if (strncmp(names_.sta, "CED", 3) == 0) {
    sprintf(cMsg, "Found Ceduna data");
    cSUpos = 1;
  } else {
    cSUpos = 0;
  }

  if (cSUpos) {
    strcat(cMsg, ", using telescope position\n         from SU table.");
    os << LogIO::WARN << cMsg << LogIO::POST ;
    cInterp = 0;
  }

  // Mean scan rate (for timestamp repairs).
  cNRate = 0;
  cAvRate[0] = 0.0;
  cAvRate[1] = 0.0;
  cCode5 = 0;


  // Find the maximum beam number.
  cNBeam = 0;
  for (int iBeam = 0; iBeam < anten_.nant; iBeam++) {
    if (anten_.ant_num[iBeam] > cNBeam) {
      cNBeam = anten_.ant_num[iBeam];
    }
  }

  if (cNBeam <= 0) {
    os << LogIO::SEVERE << "Couldn't determine number of beams." << LogIO::POST ;
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
    // Guard against dubious beam numbers, e.g. zeroes in
    // 1999-09-29_1632_024848p14_071b.hpf and the four scans following.
    // Note that the actual beam number is decoded from the 'baseline' random
    // parameter for each spectrum and is only used for beam selection.
    int beamNo = anten_.ant_num[iBeam];
    if (beamNo != iBeam+1) {
      char sta[8];
      strncpy(sta, names_.sta+(8*iBeam), 8);
      char *cp = sta + 7;
      while (*cp == ' ') *(cp--) = '\0';

      sprintf(cMsg,
        "RPFITSIN returned beam number %2d for AN table\n"
        "entry %2d with name '%.8s'", beamNo, iBeam+1, sta);

      char text[8];
      sprintf(text, "MB%2.2d", iBeam+1);
      cp = cMsg + strlen(cMsg);
      if (strncmp(sta, text, 8) == 0) {
        beamNo = iBeam + 1;
        sprintf(cp, "; using beam number %2d.", beamNo);
      } else {
        sprintf(cp, ".");
      }

      os << LogIO::WARN << cMsg << LogIO::POST ;
    }

    if (0 < beamNo && beamNo <= cNBeam) {
      cBeams[beamNo-1] = 1;
    }
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

  // Allocate memory for RPFITSIN subroutine arguments.
  if (cVis) delete [] cVis;
  if (cWgt) delete [] cWgt;
  cVis = new float[2*maxProd];
  cWgt = new float[maxProd];

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
    os << LogIO::SEVERE << "Failed to read first syscal record." << LogIO::POST ;
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
        char   bunit[32],
        float  &equinox,
        char   radecsys[32],
        char   dopplerFrame[32],
        char   datobs[32],
        double &utc,
        double &refFreq,
        double &bandwidth)
{
  const string methodName = "getHeader()" ;
  LogIO os( LogOrigin( className, methodName, WHERE ) ) ;

  if (!cMBopen) {
    os << LogIO::SEVERE << "An MBFITS file has not been opened." << LogIO::POST ;
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

    // ATCA-104.  Updated position at epoch 2007/06/24 from Chris Phillips.
    // antPos[0] = -4751640.182; // ± 0.008
    // antPos[1] =  2791700.322; // ± 0.006
    // antPos[2] = -3200490.668; // ± 0.007
    //
  } else if (strncmp(names_.sta, "MOP", 3) == 0) {
    // Mopra.  Updated position at epoch 2007/06/24 from Chris Phillips.
    sprintf(telescope, "%-16.16s", "ATMOPRA");
    antPos[0] = -4682769.444; // ± 0.009
    antPos[1] =  2802618.963; // ± 0.006
    antPos[2] = -3291758.864; // ± 0.008

  } else if (strncmp(names_.sta, "HOB", 3) == 0) {
    // Hobart.
    sprintf(telescope, "%-16.16s", "HOBART");
    antPos[0] = -3950236.735;
    antPos[1] =  2522347.567;
    antPos[2] = -4311562.569;

  } else if (strncmp(names_.sta, "CED", 3) == 0) {
    // Ceduna.  Updated position at epoch 2007/06/24 from Chris Phillips.
    sprintf(telescope, "%-16.16s", "CEDUNA");
    antPos[0] = -3753443.168; // ± 0.017
    antPos[1] =  3912709.794; // ± 0.017
    antPos[2] = -3348067.060; // ± 0.016

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

  // Brightness unit.
  sprintf(bunit, "%-16.16s", names_.bunit);
  if (strcmp(bunit, "JY") == 0) {
    bunit[1] = 'y';
  } else if (strcmp(bunit, "JY/BEAM") == 0) {
    strcpy(bunit, "Jy/beam");
  }

  // Coordinate frames.
  equinox = 2000.0f;
  strcpy(radecsys, "FK5");
  strcpy(dopplerFrame, "TOPOCENT");

  // Time at start of observation.
  sprintf(datobs, "%-10.10s", names_.datobs);
  utc = cUTC;

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

// Read the next data record (if you're feeling lucky).

int MBFITSreader::read(
        MBrecord &MBrec)
{
  const string methodName = "read()" ;
  LogIO os( LogOrigin( className, methodName, WHERE ) ) ;

  int beamNo = -1;
  int haveData, pCode = 0, status;
  double raRate = 0.0, decRate = 0.0, paRate = 0.0;
  MBrecord *iMBuff = 0x0;

  if (!cMBopen) {
    os << LogIO::SEVERE << "An MBFITS file has not been opened." << LogIO::POST ;
    return 1;
  }

  // Positions recorded in the input records usually do not coincide with the
  // midpoint of the integration and hence the input must be buffered so that
  // true positions may be interpolated.
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
      pCode = 0;
      if ((status = rpget(0, cEOS)) == -1) {
        // EOF.
        cEOF = 1;
        cFlushing = 1;
        cFlushBin = 0;
        cFlushIF  = 0;

#ifdef PKSIO_DEBUG
        os << LogIO::DEBUGGING << "\nEnd-of-file detected, flushing last cycle.\n" << LogIO::POST ;
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

          int maxChan = 0;
          int maxXpol = 0;

          cSimulIF = 0;
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
              cSimulIF = max(cSimulIF, cIFSel[iIF]+1);

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
            os << LogIO::SEVERE << "Cannot handle binning mode for multiple beams.\nSelect a single beam for input." << LogIO::POST ;
            close();
            return 1;
          }

          // Allocate buffer data storage; the MBrecord constructor zeroes
          // class members such as cycleNo that are tested in the first pass
          // below.
          int nBuff = cNBeamSel * cNBin;
          cBuffer = new MBrecord[nBuff];

          // Allocate memory for spectral arrays.
          for (int ibuff = 0; ibuff < nBuff; ibuff++) {
            cBuffer[ibuff].setNIFs(cSimulIF);
            cBuffer[ibuff].allocate(0, maxChan, maxXpol);

            // Signal that this IF in this buffer has been flushed.
            for (int iIF = 0; iIF < cSimulIF; iIF++) {
              cBuffer[ibuff].IFno[iIF] = 0;
            }
          }

          cPosUTC = new double[cNBeamSel];

          cFirst = 0;
          cScanNo  = 1;
          cCycleNo = 0;
          cPrevUTC = -1.0;
        }

        // Check for end-of-scan.
        if (cEOS) {
          cScanNo++;
          cCycleNo = 0;
          cPrevUTC = -1.0;
        }

        // Apply beam and IF selection before the change-of-day test to allow
        // a single selected beam and IF to be handled in binning-mode.
        beamNo = int(cBaseline / 256.0);
        if (beamNo == 1) {
          // Store the position of beam 1 for grid convergence corrections.
          cRA0  = cU;
          cDec0 = cV;
        }
        iBeamSel = cBeamSel[beamNo-1];
        if (iBeamSel < 0) continue;

        // Sanity check (mainly for MOPS).
        if (cIFno > cNIF) continue;

        // Apply IF selection; iIFSel == 0 for the first selected IF, == 1
        // for the second, etc.
        iIFSel = cIFSel[cIFno - 1];
        if (iIFSel < 0) continue;


        if (cNBin > 1) {
          // Binning mode: correct the time.
          cUTC += param_.intbase * (cBin - (cNBin + 1)/2.0);
        }

        // Check for change-of-day.
        double cod = 0.0;
        if ((cUTC + 86400.0) < (cPrevUTC + 600.0)) {
          // cUTC should continue to increase past 86400 during a single scan.
          // However, if the RPFITS file contains multiple scans that straddle
          // midnight then cUTC can jump backwards from the end of one scan to
          // the start of the next.
#ifdef PKSIO_DEBUG
          char buf[256] ;
          sprintf(buf, "Change-of-day on cUTC: %.1f -> %.1f\n", cPrevUTC, cUTC);
          os << LogIO::DEBUGGING << buf << LogIO::POST ;
#endif
          // Can't change the recorded value of cUTC directly (without also
          // changing dateobs) so change-of-day must be recorded separately as
          // an offset to be applied when comparing integration timestamps.
          cod = 86400.0;

        } 

        if ((cUTC+cod) < cPrevUTC - 1.0) {
          if (cBin == 1 && iIFSel) {
            // Multiple-IF, binning-mode data is only partially time ordered.
#ifdef PKSIO_DEBUG
            fprintf(stderr, "New IF in multiple-IF, binning-mode data.\n");
#endif
            cCycleNo -= cNBin;
            cPrevUTC = -1.0;

          } else {
            // All other data should be fully time ordered.
            sprintf(cMsg,
              "Cycle %d:%03d-%03d, UTC went backwards from\n"
              "%.1f to %.1f!  Incrementing day number,\n"
              "positions may be unreliable.", cScanNo, cCycleNo,
              cCycleNo+1, cPrevUTC, cUTC);
            //logMsg(cMsg);
            os << LogIO::WARN << cMsg << LogIO::POST ;
            cUTC += 86400.0;
          }
        }

        // New integration cycle?
        if ((cUTC+cod) > cPrevUTC) {
          cCycleNo++;
          cPrevUTC = cUTC + 0.0001;
        }

        sprintf(cDateObs, "%-10.10s", names_.datobs);
        cDateObs[10] = '\0';

        // Compute buffer number.
        iMBuff = cBuffer + iBeamSel;
        if (cNBin > 1) iMBuff += cNBeamSel*(cBin-1);

        if (cCycleNo < iMBuff->cycleNo) {
          // Note that if the first beam and IF are not both selected cEOS
          // will be cleared by rpget() when the next beam/IF is read.
          cEOS = 1;
        }

        // Begin flush cycle?
        if (cEOS || (iMBuff->nIF && (cUTC+cod) > (iMBuff->utc+0.0001))) {
          cFlushing = 1;
          cFlushBin = 0;
          cFlushIF  = 0;
        }

#ifdef PKSIO_DEBUG
        char rel = '=';
        double dt = utcDiff(cUTC, cW);
        if (dt < 0.0) {
          rel = '<';
        } else if (dt > 0.0) {
          rel = '>';
        }

        sprintf(buf, "\n In:%4d%4d%3d%3d  %.3f %c %.3f (%+.3fs) - "
          "%sflushing\n", cScanNo, cCycleNo, beamNo, cIFno, cUTC, rel, cW, dt,
          cFlushing ? "" : "not ");
        os << LogIO::DEBUGGING << buf << LogIO::POST ;
        if (cEOS) {
          sprintf(buf, "Start of new scan, flushing previous scan.\n");
          os << LogIO::DEBUGGING << buf << LogIO::POST ;
        }
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

          // iMBuff->nIF is decremented (below) and if zero signals that all
          // IFs in an integration have been flushed.
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

        // Start with the first IF in the next bin.
        cFlushIF = 0;
      }

      if (beamNo) {
        iBeamSel = cBeamSel[beamNo-1];
        iMBuff = cBuffer + iBeamSel + cNBeamSel*cFlushBin;

        // Find the IF to flush.
        for (; cFlushIF < cSimulIF; cFlushIF++) {
          if (iMBuff->IFno[cFlushIF]) break;
        }

      } else {
        // Flush complete.
        cFlushing = 0;
        if (cEOF) {
          return -1;
        }

        // The last record read must have been the first of a new cycle.
        beamNo = int(cBaseline / 256.0);
        iBeamSel = cBeamSel[beamNo-1];

        // Compute buffer number.
        iMBuff = cBuffer + iBeamSel;
        if (cNBin > 1) iMBuff += cNBeamSel*(cBin-1);
      }
    }


    if (cInterp && cFlushing == 1) {
      // Start of flush cycle, interpolate the beam position.
      //
      // The position is measured by the control system at a time returned by
      // RPFITSIN as the 'w' visibility coordinate.  The ra and dec, returned
      // as the 'u' and 'v' visibility coordinates, must be interpolated to
      // the integration time which RPFITSIN returns as 'cUTC', this usually
      // being a second or two later.  The interpolation method used here is
      // based on the scan rate.
      //
      // "This" RA, Dec, and UTC refers to the position currently stored in
      // the buffer marked for output (iMBuff).  This position is interpolated
      // to the midpoint of that integration using either
      //   a) the rate currently sitting in iMBuff, which was computed from
      //      the previous integration, otherwise
      //   b) from the position recorded in the "next" integration which is
      //      currently sitting in the RPFITS commons,
      // so that the position timestamps straddle the midpoint of the
      // integration and is thereby interpolated rather than extrapolated.
      //
      // At the end of a scan, or if the next position has not been updated
      // or its timestamp does not advance sufficiently, the most recent
      // determination of the scan rate will be used for extrapolation which
      // is quantified by the "rate age" measured in seconds beyond the
      // interval defined by the position timestamps.

      // At this point, iMBuff contains cU, cV, cW, parAngle and focusRot
      // stored from the previous call to rpget() for this beam (i.e. "this"),
      // and also raRate, decRate and paRate computed from that integration
      // and the previous one.
      double thisRA  = iMBuff->ra;
      double thisDec = iMBuff->dec;
      double thisUTC = cPosUTC[iBeamSel];
      double thisPA  = iMBuff->parAngle + iMBuff->focusRot;

#ifdef PKSIO_DEBUG
      sprintf(buf, "This (%d) ra, dec, UTC: %9.4f %9.4f %10.3f %9.4f\n",
        iMBuff->cycleNo, thisRA*R2D, thisDec*R2D, thisUTC, thisPA*R2D);
      os << LogIO::DEBUGGING << buf << LogIO::POST ;
#endif

      if (cEOF || cEOS) {
        // Use rates from the last cycle.
        raRate  = iMBuff->raRate;
        decRate = iMBuff->decRate;
        paRate  = iMBuff->paRate;

      } else {
        if (cW == thisUTC) {
          // The control system at Mopra typically does not update the
          // positions between successive integration cycles at the end of a
          // scan (nor are they flagged).  In this case we use the previously
          // computed rates, even if from the previous scan since these are
          // likely to be a better guess than anything else.
          raRate  = iMBuff->raRate;
          decRate = iMBuff->decRate;
          paRate  = iMBuff->paRate;

          if (cU == thisRA && cV == thisDec) {
            // Position and timestamp unchanged.
            pCode = 1;

          } else if (fabs(cU-thisRA) < 0.0001 && fabs(cV-thisDec) < 0.0001) {
            // Allow small rounding errors (seen infrequently).
            pCode = 1;

          } else {
            // (cU,cV) are probably rubbish (not yet seen in practice).
            pCode = 2;
            cU = thisRA;
            cV = thisDec;
          }

#ifdef PKSIO_DEBUG
          sprintf(buf, "Next (%d) ra, dec, UTC: %9.4f %9.4f %10.3f "
            "(0.000s)\n", cCycleNo, cU*R2D, cV*R2D, cW);
          os << LogIO::DEBUGGING << buf << LogIO::POST ;
#endif

        } else {
          double nextRA  = cU;
          double nextDec = cV;

          // Check and, if necessary, repair the position timestamp,
          // remembering that pCode refers to the NEXT cycle.
          pCode = fixw(cDateObs, cCycleNo, beamNo, cAvRate, thisRA, thisDec,
                       thisUTC, nextRA, nextDec, cW);
          if (pCode > 0) pCode += 3;
          double nextUTC = cW;

#ifdef PKSIO_DEBUG
          sprintf(buf, "Next (%d) ra, dec, UTC: %9.4f %9.4f %10.3f "
            "(%+.3fs)\n", cCycleNo, nextRA*R2D, nextDec*R2D, nextUTC,
            utcDiff(nextUTC, thisUTC));
          os << LogIO::DEBUGGING << buf << LogIO::POST ;
#endif

          // Compute the scan rate for this beam.
          double dUTC = utcDiff(nextUTC, thisUTC);
          if ((0.0 < dUTC) && (dUTC < 600.0)) {
            scanRate(cRA0, cDec0, thisRA, thisDec, nextRA, nextDec, dUTC,
                     raRate, decRate);

            // Update the mean scan rate.
            cAvRate[0] = (cAvRate[0]*cNRate +  raRate) / (cNRate + 1);
            cAvRate[1] = (cAvRate[1]*cNRate + decRate) / (cNRate + 1);
            cNRate++;

            // Rate of change of position angle.
            if (sc_.sc_ant <= anten_.nant) {
              paRate = 0.0;
            } else {
              int iOff = sc_.sc_q * (sc_.sc_ant - 1) - 1;
              double nextPA = sc_.sc_cal[iOff + 4] + sc_.sc_cal[iOff + 7];
              double paDiff = nextPA - thisPA;
              if (paDiff > PI) {
                paDiff -= TWOPI;
              } else if (paDiff < -PI) {
                paDiff += TWOPI;
              }
              paRate = paDiff / dUTC;
            }

            if (cInterp == 2) {
              // Use the same interpolation scheme as the original pksmbfits
              // client.  This incorrectly assumed that (nextUTC - thisUTC) is
              // equal to the integration time and interpolated by computing a
              // weighted sum of the positions before and after the required
              // time.

              double utc = iMBuff->utc;
              double tw1 = 1.0 - utcDiff(utc, thisUTC) / iMBuff->exposure;
              double tw2 = 1.0 - utcDiff(nextUTC, utc) / iMBuff->exposure;
              double gamma = (tw2 / (tw1 + tw2)) * dUTC / (utc - thisUTC);

              // Guard against RA cycling through 24h in either direction.
              if (fabs(nextRA - thisRA) > PI) {
                if (nextRA < thisRA) {
                  nextRA += TWOPI;
                } else {
                  nextRA -= TWOPI;
                }
              }

              raRate  = gamma * (nextRA  - thisRA)  / dUTC;
              decRate = gamma * (nextDec - thisDec) / dUTC;
            }

          } else {
            if (cCycleNo == 2 && fabs(utcDiff(cUTC,cW)) < 600.0) {
              // thisUTC (i.e. cW for the first cycle) is rubbish, and
              // probably the position as well (extremely rare in practice,
              // e.g. 97-12-19_1029_235708-18_586e.hpf which actually has the
              // t/1000 scaling bug in the first cycle).
              iMBuff->pCode = 3;
              thisRA  = cU;
              thisDec = cV;
              thisUTC = cW;
              raRate  = 0.0;
              decRate = 0.0;
              paRate  = 0.0;

            } else {
              // cW is rubbish and probably (cU,cV), and possibly the
              // parallactic angle and everything else as well (rarely seen
              // in practice, e.g. 97-12-09_0743_235707-58_327c.hpf and
              // 97-09-01_0034_123717-42_242b.hpf, the latter with bad
              // parallactic angle).
              pCode = 3;
              cU = thisRA;
              cV = thisDec;
              cW = thisUTC;
              raRate  = iMBuff->raRate;
              decRate = iMBuff->decRate;
              paRate  = iMBuff->paRate;
            }
          }
        }
      }


      // Choose the closest rate determination.
      if (cCycleNo == 1) {
        // Scan containing a single integration.
        iMBuff->raRate  = 0.0;
        iMBuff->decRate = 0.0;
        iMBuff->paRate  = 0.0;

      } else {
        double dUTC = iMBuff->utc - cPosUTC[iBeamSel];

        if (dUTC >= 0.0) {
          // In HIPASS/ZOA, the position timestamp, which should always occur
          // on the whole second, normally precedes an integration midpoint
          // falling on the half-second.  Consequently, positive ages are
          // always half-integral.
          dUTC = utcDiff(iMBuff->utc, cW);
          if (dUTC > 0.0) {
            iMBuff->rateAge = dUTC;
          } else {
            iMBuff->rateAge = 0.0f;
          }

          iMBuff->raRate  =  raRate;
          iMBuff->decRate = decRate;
          iMBuff->paRate  =  paRate;

        } else {
          // In HIPASS/ZOA, negative ages occur when the integration midpoint,
          // occurring on the whole second, precedes the position timestamp.
          // Thus negative ages are always an integral number of seconds.
          // They have only been seen to occur sporadically in the period
          // 1999/05/31 to 1999/11/01, e.g. 1999-07-26_1821_005410-74_007c.hpf
          //
          // In recent (2008/10/07) Mopra data, small negative ages (~10ms,
          // occasionally up to ~300ms) seem to be the norm, with both the
          // position timestamp and integration midpoint falling close to but
          // not on the integral second.
          if (cCycleNo == 2) {
            // We have to start with something!
            iMBuff->rateAge = dUTC;

          } else {
            // Although we did not record the relevant position timestamp
            // explicitly, it can easily be deduced.
            double w = iMBuff->utc - utcDiff(cUTC, iMBuff->utc) -
                       iMBuff->rateAge;
            dUTC = utcDiff(iMBuff->utc, w);

            if (dUTC > 0.0) {
              iMBuff->rateAge = 0.0f;
            } else {
              iMBuff->rateAge = dUTC;
            }
          }

          iMBuff->raRate  =  raRate;
          iMBuff->decRate = decRate;
          iMBuff->paRate  =  paRate;
        }
      }

#ifdef PKSIO_DEBUG
      double avRate = sqrt(cAvRate[0]*cAvRate[0] + cAvRate[1]*cAvRate[1]);
      sprintf(buf, "RA, Dec, Av & PA rates: %8.4f %8.4f %8.4f %8.4f "
        "pCode %d\n", raRate*R2D, decRate*R2D, avRate*R2D, paRate*R2D, pCode);
      os << LogIO::DEBUGGING << buf << LogIO::POST ;
#endif


      // Compute the position of this beam for all bins.
      for (int idx = 0; idx < cNBin; idx++) {
        int jbuff = iBeamSel + cNBeamSel*idx;

        cBuffer[jbuff].raRate  = iMBuff->raRate;
        cBuffer[jbuff].decRate = iMBuff->decRate;
        cBuffer[jbuff].paRate  = iMBuff->paRate;

        double dUTC = utcDiff(cBuffer[jbuff].utc, thisUTC);
        if (dUTC > 100.0) {
          // Must have cycled through midnight.
          dUTC -= 86400.0;
        }

        applyRate(cRA0, cDec0, thisRA, thisDec,
          cBuffer[jbuff].raRate, cBuffer[jbuff].decRate, dUTC,
          cBuffer[jbuff].ra, cBuffer[jbuff].dec);

#ifdef PKSIO_DEBUG
        sprintf(buf, "Intp (%d) ra, dec, UTC: %9.4f %9.4f %10.3f (pCode, "
          "age: %d %.1fs)\n", iMBuff->cycleNo, cBuffer[jbuff].ra*R2D,
          cBuffer[jbuff].dec*R2D, cBuffer[jbuff].utc, iMBuff->pCode,
          iMBuff->rateAge);
        os << LogIO::DEBUGGING << buf << LogIO::POST ;
#endif
      }

      cFlushing = 2;
    }


    if (cFlushing) {
      // Copy buffer location out one IF at a time.
      MBrec.extract(*iMBuff, cFlushIF);
      haveData = 1;

#ifdef PKSIO_DEBUG
      sprintf(buf, "Out:%4d%4d%3d%3d\n", MBrec.scanNo, MBrec.cycleNo,
        MBrec.beamNo, MBrec.IFno[0]);
      os << LogIO::DEBUGGING << buf << LogIO::POST ;
#endif

      // Signal that this IF in this buffer location has been flushed.
      iMBuff->IFno[cFlushIF] = 0;

      iMBuff->nIF--;
      if (iMBuff->nIF == 0) {
        // All IFs in this buffer location have been flushed.  Stop cEOS
        // being set when the next integration is read.
        iMBuff->cycleNo = 0;

      } else {
        // Carry on flushing the other IFs.
        continue;
      }

      // Has the whole buffer been flushed?
      if (cFlushBin == cNBin - 1) {
        if (cEOS || cEOF) {
          // Carry on flushing other buffers.
          cFlushIF = 0;
          continue;
        }

        cFlushing = 0;

        beamNo = int(cBaseline / 256.0);
        iBeamSel = cBeamSel[beamNo-1];

        // Compute buffer number.
        iMBuff = cBuffer + iBeamSel;
        if (cNBin > 1) iMBuff += cNBeamSel*(cBin-1);
      }
    }

    if (!cFlushing) {
      // Buffer this MBrec.
      if ((cScanNo > iMBuff->scanNo) && iMBuff->IFno[0]) {
        // Sanity check on the number of IFs in the new scan.
        if (if_.n_if != cNIF) {
          sprintf(cMsg, "Scan %d has %d IFs instead of %d, "
            "continuing.", cScanNo, if_.n_if, cNIF);
          os << LogIO::WARN << cMsg << LogIO::POST ;
        }
      }

      // Sanity check on incomplete integrations within a scan.
      if (iMBuff->nIF && (iMBuff->cycleNo != cCycleNo)) {
        // Force the incomplete integration to be flushed before proceeding.
        cFlushing = 1;
        continue;
      }

#ifdef PKSIO_DEBUG
      sprintf(buf, "Buf:%4d%4d%3d%3d\n", cScanNo, cCycleNo, beamNo, cIFno);
      os << LogIO::DEBUGGING << buf << LogIO::POST ;
#endif

      // Store IF-independent parameters only for the first IF of a new cycle,
      // particularly because this is the only one for which the scan rates
      // are computed above.
      int firstIF = (iMBuff->nIF == 0);
      if (firstIF) {
        iMBuff->scanNo  = cScanNo;
        iMBuff->cycleNo = cCycleNo;

        // Times.
        strcpy(iMBuff->datobs, cDateObs);
        iMBuff->utc = cUTC;
        iMBuff->exposure = param_.intbase;

        // Source identification.
        sprintf(iMBuff->srcName, "%-16.16s",
                names_.su_name + (cSrcNo-1)*16);
        iMBuff->srcName[16] = '\0';
        iMBuff->srcRA  = doubles_.su_ra[cSrcNo-1];
        iMBuff->srcDec = doubles_.su_dec[cSrcNo-1];

        // Rest frequency of the line of interest.
        iMBuff->restFreq = doubles_.rfreq;
        if (strncmp(names_.instrument, "ATPKSMB", 7) == 0) {
          // Fix the HI rest frequency recorded for Parkes multibeam data.
          double reffreq  = doubles_.freq;
          double restfreq = doubles_.rfreq;
          if ((restfreq == 0.0 || fabs(restfreq - reffreq) == 0.0) &&
               fabs(reffreq - 1420.405752e6) < 100.0) {
            iMBuff->restFreq = 1420.405752e6;
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
        if (cSUpos) {
          // Non-ATNF data that does not store the position in (u,v,w).
          iMBuff->ra  = doubles_.su_ra[cSrcNo-1];
          iMBuff->dec = doubles_.su_dec[cSrcNo-1];
        } else {
          iMBuff->ra  = cU;
          iMBuff->dec = cV;
        }
        cPosUTC[iBeamSel] = cW;
        iMBuff->pCode = pCode;

        // Store rates for next time.
        iMBuff->raRate  =  raRate;
        iMBuff->decRate = decRate;
        iMBuff->paRate  =  paRate;
      }

      // IF-dependent parameters.
      int iIF = cIFno - 1;
      int startChan = cStartChan[iIF];
      int endChan   = cEndChan[iIF];
      int refChan   = cRefChan[iIF];

      int nChan = abs(endChan - startChan) + 1;

      iIFSel = cIFSel[iIF];
      if (iMBuff->IFno[iIFSel] == 0) {
        iMBuff->nIF++;
        iMBuff->IFno[iIFSel] = cIFno;
      } else {
        // Integration cycle written to the output file twice (the only known
        // example is 1999-05-22_1914_000-031805_03v.hpf).
        sprintf(cMsg, "Integration cycle %d:%d, beam %2d, \n"
                      "IF %d was duplicated.", cScanNo, cCycleNo-1,
                      beamNo, cIFno);
        os << LogIO::WARN << cMsg << LogIO::POST ;
      }
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
      if (rpflag == 0) rpflag = cFlag;

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


      // Calibration factor applied to the data by the correlator.
      if (scq > 14) {
        // Will only be present for Parkes Multibeam or LBA data.
        iMBuff->tcal[iIFSel][0] = sc_.sc_cal[scq*iBeam + 14];
        iMBuff->tcal[iIFSel][1] = sc_.sc_cal[scq*iBeam + 15];
      } else {
        iMBuff->tcal[iIFSel][0] = 0.0f;
        iMBuff->tcal[iIFSel][1] = 0.0f;
      }

      if (firstIF) {
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
          tcalTime[16] = '\0';

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
  }

  return 0;
}

//-------------------------------------------------------- MBFITSreader::rpget

// Read the next data record from the RPFITS file.

int MBFITSreader::rpget(int syscalonly, int &EOS)
{
  const string methodName = "rpget()" ;
  LogIO os( LogOrigin( className, methodName, WHERE ) ) ;

  EOS = 0;

  int retries = 0;

  // Allow 10 read errors.
  int numErr = 0;

  int jstat = 0;
  while (numErr < 10) {
    int lastjstat = jstat;

    switch(rpfitsin(jstat)) {
    case -1:
      // Read failed; retry.
      numErr++;
      os << LogIO::WARN << "RPFITS read failed - retrying." << LogIO::POST ;
      jstat = 0;
      break;

    case 0:
      // Successful read.
      if (lastjstat == 0) {
        if (cBaseline == -1) {
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
      sprintf(cMsg, "Unrecognized RPFITSIN return code: %d "
                    "(retrying).", jstat);
      os << LogIO::WARN << cMsg << LogIO::POST ;
      jstat = 0;
      break;
    }
  }

  os << LogIO::SEVERE << "RPFITS read failed too many times." << LogIO::POST ;
  return 2;
}

//----------------------------------------------------- MBFITSreader::rpfitsin

// Wrapper around RPFITSIN that reports errors.  Returned RPFITSIN subroutine
// arguments are captured as MBFITSreader member variables.

int MBFITSreader::rpfitsin(int &jstat)

{
  rpfitsin_(&jstat, cVis, cWgt, &cBaseline, &cUTC, &cU, &cV, &cW, &cFlag,
            &cBin, &cIFno, &cSrcNo);

  // Handle messages from RPFITSIN.
/**
  if (names_.errmsg[0] != ' ') {
    int i;
    for (i = 80; i > 0; i--) {
      if (names_.errmsg[i-1] != ' ') break;
    }

    sprintf(cMsg, "WARNING: Cycle %d:%03d, RPFITSIN reported -\n"
                  "         %.*s", cScanNo, cCycleNo, i, names_.errmsg);
    logMsg(cMsg);
  }
**/
  return jstat;
}

//------------------------------------------------------- MBFITSreader::fixPos

// Check and, if necessary, repair a position timestamp.
//
// Problems with the position timestamp manifest themselves via the scan rate:
//
//   1) Zero scan rate pairs, 1997/02/28 to 1998/01/07
//
//      These occur because the position timestamp for the first integration
//      of the pair is erroneous; the value recorded is t/1000, where t is the
//      true value.
//        Earliest known: 97-02-28_1725_132653-42_258a.hpf
//          Latest known: 98-01-02_1923_095644-50_165c.hpf
//        (time range chosen to encompass observing runs).
//
//   2) Slow-fast scan rate pairs (0.013 - 0.020 deg/s),
//        1997/03/28 to 1998/01/07.
//
//      The UTC position timestamp is 1.0s later than it should be (never
//      earlier), almost certainly arising from an error in the telescope
//      control system.
//        Earliest known: 97-03-28_0150_010420-74_008d.hpf
//          Latest known: 98-01-04_1502_065150-02_177c.hpf
//        (time range chosen to encompass observing runs).
//
//   3) Slow-fast scan rate pairs (0.015 - 0.018 deg/s),
//        1999/05/20 to 2001/07/12 (HIPASS and ZOA),
//        2001/09/02 to 2001/12/04 (HIPASS and ZOA),
//        2002/03/28 to 2002/05/13 (ZOA only),
//        2003/04/26 to 2003/06/09 (ZOA only).
//        Earliest known: 1999-05-20_1818_175720-50_297e.hpf
//          Latest known: 2001-12-04_1814_065531p14_173e.hpf (HIPASS)
//                        2003-06-09_1924_352-085940_-6c.hpf (ZOA)
//
//      Caused by the Linux signalling NaN problem.  IEEE "signalling" NaNs
//      are silently transformed to "quiet" NaNs during assignment by setting
//      bit 22.  This affected RPFITS because of its use of VAX-format
//      floating-point numbers which, with their permuted bytes, may sometimes
//      appear as signalling NaNs.
//
//      The problem arose when the linux correlator came online and was
//      fixed with a workaround to the RPFITS library (repeated episodes
//      are probably due to use of an older version of the library).  It
//      should not have affected the data significantly because of the
//      low relative error, which ranges from 0.0000038 to 0.0000076, but
//      it is important for the computation of scan rates which requires
//      taking the difference of two large UTC timestamps, one or other
//      of which will have 0.5s added to it.
//
// The return value identifies which, if any, of these problems was repaired.

int MBFITSreader::fixw(
  const char *datobs,
  int    cycleNo,
  int    beamNo,
  double avRate[2],
  double thisRA,
  double thisDec,
  double thisUTC,
  double nextRA,
  double nextDec,
  float &nextUTC)
{
  if (strcmp(datobs, "2003-06-09") > 0) {
    return 0;

  } else if (strcmp(datobs, "1998-01-07") <= 0) {
    if (nextUTC < thisUTC && (nextUTC + 86400.0) > (thisUTC + 600.0)) {
      // Possible scaling problem.
      double diff = nextUTC*1000.0 - thisUTC;
      if (0.0 < diff && diff < 600.0) {
        nextUTC *= 1000.0;
        return 1;
      } else {
        // Irreparable.
        return -1;
      }
    }

    if (cycleNo > 2) {
      if (beamNo == 1) {
        // This test is only reliable for beam 1.
        double dUTC = nextUTC - thisUTC;
        if (dUTC < 0.0) dUTC += 86400.0;

        // Guard against RA cycling through 24h in either direction.
        if (fabs(nextRA - thisRA) > PI) {
          if (nextRA < thisRA) {
            nextRA += TWOPI;
          } else {
            nextRA -= TWOPI;
          }
        }

        double  dRA = (nextRA  - thisRA) * cos(nextDec);
        double dDec =  nextDec - thisDec;
        double  arc = sqrt(dRA*dRA + dDec*dDec);

        double averate = sqrt(avRate[0]*avRate[0] + avRate[1]*avRate[1]);
        double diff1 = fabs(averate - arc/(dUTC-1.0));
        double diff2 = fabs(averate - arc/dUTC);
        if ((diff1 < diff2) && (diff1 < 0.05*averate)) {
          nextUTC -= 1.0;
          cCode5 = cycleNo;
          return 2;
        } else {
          cCode5 = 0;
        }

      } else {
        if (cycleNo == cCode5) {
          nextUTC -= 1.0;
          return 2;
        }
      }
    }

  } else if ((strcmp(datobs, "1999-05-20") >= 0 &&
              strcmp(datobs, "2001-07-12") <= 0) ||
             (strcmp(datobs, "2001-09-02") >= 0 &&
              strcmp(datobs, "2001-12-04") <= 0) ||
             (strcmp(datobs, "2002-03-28") >= 0 &&
              strcmp(datobs, "2002-05-13") <= 0) ||
             (strcmp(datobs, "2003-04-26") >= 0 &&
              strcmp(datobs, "2003-06-09") <= 0)) {
    // Signalling NaN problem, e.g. 1999-07-26_1839_011106-74_009c.hpf.
    // Position timestamps should always be an integral number of seconds.
    double resid = nextUTC - int(nextUTC);
    if (resid == 0.5) {
      nextUTC -= 0.5;
      return 3;
    }
  }

  return 0;
}

//-------------------------------------------------------- MBFITSreader::close

// Close the input file.

void MBFITSreader::close(void)
{
  if (cMBopen) {
    int jstat = 1;
    rpfitsin_(&jstat, cVis, cWgt, &cBaseline, &cUTC, &cU, &cV, &cW, &cFlag,
              &cBin, &cIFno, &cSrcNo);

    if (cBeams)     delete [] cBeams;
    if (cIFs)       delete [] cIFs;
    if (cNChan)     delete [] cNChan;
    if (cNPol)      delete [] cNPol;
    if (cHaveXPol)  delete [] cHaveXPol;
    if (cStartChan) delete [] cStartChan;
    if (cEndChan)   delete [] cEndChan;
    if (cRefChan)   delete [] cRefChan;

    if (cVis) delete [] cVis;
    if (cWgt) delete [] cWgt;

    if (cBeamSel)   delete [] cBeamSel;
    if (cIFSel)     delete [] cIFSel;
    if (cChanOff)   delete [] cChanOff;
    if (cXpolOff)   delete [] cXpolOff;
    if (cBuffer)    delete [] cBuffer;
    if (cPosUTC)    delete [] cPosUTC;

    cMBopen = 0;
  }
}

//-------------------------------------------------------------------- utcDiff

// Subtract two UTCs (s) allowing for any plausible number of cycles through
// 86400s, returning a result in the range [-43200, +43200]s.

double MBFITSreader::utcDiff(double utc1, double utc2)
{
  double diff = utc1 - utc2;

  if (diff > 43200.0) {
    diff -= 86400.0;
    while (diff > 43200.0) diff -= 86400.0;
  } else if (diff < -43200.0) {
    diff += 86400.0;
    while (diff < -43200.0) diff += 86400.0;
  }

  return diff;
}

//------------------------------------------------------- scanRate & applyRate

// Compute and apply the scan rate corrected for grid convergence.  (ra0,dec0)
// are the coordinates of the central beam, assumed to be the tracking centre.
// The rate computed in RA will be a rate of change of angular distance in the
// direction of increasing RA at the position of the central beam.  Similarly
// for declination.  Angles in radian, time in s.

void MBFITSreader::scanRate(
  double ra0,
  double dec0,
  double ra1,
  double dec1,
  double ra2,
  double dec2,
  double dt,
  double &raRate,
  double &decRate)
{
  // Transform to a system where the central beam lies on the equator at 12h.
  eulerx(ra1, dec1, ra0+HALFPI, -dec0, -HALFPI, ra1, dec1);
  eulerx(ra2, dec2, ra0+HALFPI, -dec0, -HALFPI, ra2, dec2);

  raRate  = (ra2  - ra1)  / dt;
  decRate = (dec2 - dec1) / dt;
}


void MBFITSreader::applyRate(
  double ra0,
  double dec0,
  double ra1,
  double dec1,
  double raRate,
  double decRate,
  double dt,
  double &ra2,
  double &dec2)
{
  // Transform to a system where the central beam lies on the equator at 12h.
  eulerx(ra1, dec1, ra0+HALFPI, -dec0, -HALFPI, ra1, dec1);

  ra2  = ra1  + (raRate  * dt);
  dec2 = dec1 + (decRate * dt);

  // Transform back.
  eulerx(ra2, dec2, -HALFPI, dec0, ra0+HALFPI, ra2, dec2);
}

//--------------------------------------------------------------------- eulerx

void MBFITSreader::eulerx(
  double lng0,
  double lat0,
  double phi0,
  double theta,
  double phi,
  double &lng1,
  double &lat1)

// Applies the Euler angle based transformation of spherical coordinates.
//
//     phi0  Longitude of the ascending node in the old system, radians.  The
//           ascending node is the point of intersection of the equators of
//           the two systems such that the equator of the new system crosses
//           from south to north as viewed in the old system.
//
//    theta  Angle between the poles of the two systems, radians.  THETA is
//           positive for a positive rotation about the ascending node.
//
//      phi  Longitude of the ascending node in the new system, radians.

{
  // Compute intermediaries.
  double lng0p  = lng0 - phi0;
  double slng0p = sin(lng0p);
  double clng0p = cos(lng0p);
  double slat0  = sin(lat0);
  double clat0  = cos(lat0);
  double ctheta = cos(theta);
  double stheta = sin(theta);

  double x = clat0*clng0p;
  double y = clat0*slng0p*ctheta + slat0*stheta;

  // Longitude in the new system.
  if (x != 0.0 || y != 0.0) {
    lng1 = phi + atan2(y, x);
  } else {
    // Longitude at the poles in the new system is consistent with that
    // specified in the old system.
    lng1 = phi + lng0p;
  }
  lng1 = fmod(lng1, TWOPI);
  if (lng1 < 0.0) lng1 += TWOPI;

  lat1 = asin(slat0*ctheta - clat0*stheta*slng0p);
}
