//# VlaAsciiSink.cc:
//# Copyright (C) 1997,1999,2001,2002
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

#include <nrao/VLA/VlaAsciiSink.h>
#include <math.h>
#include <casa/iostream.h>

// Note from Wes to himself.  
// The continuum and spectral line CDA write routines are are probably broke.
// I need to redo them.  Look at Gareth's stuff in VlaRecord there's probably
// something to steal from there.

using namespace casa;

VlaAsciiSink::VlaAsciiSink() 
  :VlaSink() 
{
}

VlaAsciiSink::VlaAsciiSink(const String& a) 
  :VlaSink() 
{
  allocateFlags();
  setConstraints(a);
}

VlaAsciiSink::~VlaAsciiSink() {
}

Int VlaAsciiSink::writeVisibilityRecord(const Char* buffer) {
  Int rStat = 0;
  cerr << rca.dateMJAD << " " << rca.timeIAT << " " << rca.controlProgramID 
       << " " << sda.sourceName << endl;
  writeAscii(cout, rcaFlags);
  writeAscii(cout, sdaFlags);
  for (Int i=0; i<rca.numberOfAntennas; i++) {
    if (writeAntennae && writeAntennae[i])
      writeAscii(cout, ada[i], adaFlags);
  }
  writeAscii(cout, ccdaFlags);
  writeAscii(cout, slcdaFlags);
  return rStat;
}

Int VlaAsciiSink::writeAscii(ostream &oss, const RCAFlags* flags) {
  if (flags) {
    if (flags->dateMJADFlag) oss << rca.dateMJAD << " ";
    if (flags->timeIATFlag) oss << rca.timeIAT << " ";
    if (flags->numberOfAntennasFlag) oss << rca.numberOfAntennas << " ";
    if (flags->writeSomethingFlag) oss << endl;
  }
  return 0;
}

Int VlaAsciiSink::writeAscii(ostream& oss, const SDAFlags* flags){
  if (flags) {
    if (flags->timeStampFlag) {
      oss << " " << rca.dateMJAD << " " << rca.timeIAT << " ";
    }
    if (flags->subarrayIDFlag) oss << sda.subarrayID;
    if (flags->sourceNameFlag) {
      oss << " "; oss.write(sda.sourceName.chars(),
			    sizeof(sda.sourceName));
    }
    if (flags->sourceNameQualifierFlag) oss << " " << sda.sourceNameQualifier;
    if (flags->observingProgramIDFlag) {
      oss << " "; 
      oss.write(sda.observingProgramID.chars(),
		sizeof(sda.observingProgramID));
    }
    if (flags->aipsIDFlag) {
      oss << " "; oss.write(sda.observingMode.chars(),
			    sizeof(sda.observingMode));
    }
    if (flags->observingModeFlag) {
      if (sda.observingMode[0] == ' ') {
	oss << 'N';
      } else {
	oss << sda.observingMode[0];
      }
    }
    if (flags->calibratorCodeFlag) oss << " " << sda.calibratorCode;
    if (flags->submodeFlag) {
      Int mySubmode = sda.submode;
      oss << " " << mySubmode;
    }
    if (flags->arrayStatusInformationFlag) {
      oss << " " << sda.arrayStatusInformation;
    }
    if (flags->numFreqChansPerBaselineFlag) {
      // oss << " " << sda.NumFreqChannelsPerBaseline;
      uShort chanA, chanB, chanC, chanD;
      uShort mask = 0x000f;
      chanD = sda.numFreqChansPerBaseline & mask;             // Channel D is least significant
      chanC = (sda.numFreqChansPerBaseline >> 4) & mask;
      chanB = (sda.numFreqChansPerBaseline >> 8) & mask;
      chanA = (sda.numFreqChansPerBaseline >> 12) & mask;     // Channel A is most significant
      oss << " " << chanA << " " << chanB << " " << chanC << " " << chanD;
    }
    if (flags->integrationTimeFlag) oss << " " << sda.integrationTime;
    if (flags->stopTimeFlag) oss << " " << sda.stopTime;
    if (flags->startTimeFlag) oss << " " << sda.startTime;
    if (flags->raFlag) oss << " " << sda.ra;
    if (flags->decFlag) oss << " " << sda.dec;
    if (flags->apparentRAFlag) oss << " " << sda.apparentRA;
    if (flags->apparentDecFlag) oss << " " << sda.apparentDec;
    for (Int i=0;i<4;i++){
      if (flags->sslosFlag[i]) {
	oss << " " << sda.sslos[i];
      }
    }
    for (Int i=0;i<4;i++) {
      if (flags->skyFreqFlag[i]) {
	oss << " " << sda.skyFreq[i];
      }
    }
    if (flags->iatEndOfIntegrationFlag) {
      oss << " " << sda.iatEndOfIntegration;
    }
    if (flags->lstEndOfIntegrationFlag) {
      oss << " " << sda.lstEndOfIntegration;
    }
    if (flags->iat4GeoCalculationsFlag) {
      oss << " " << sda.iat4GeoCalculations;
    }
    if (flags->currentSurfaceRefractivityFlag) {
      oss << " " << sda.currentSurfaceRefractivity;
    }
    if (flags->estimatedZenithAtmosPhaseFlag) {
      oss << " " << sda.estimatedZenithAtmosPhase;
    }
    if (flags->sinElevationFlag) {
      oss << " " << sda.sinElevation;
    }
    if (flags->cosElevationFlag) {
      oss << " " << sda.cosElevation;
    }
    if (flags->sinAzimuthFlag) {
      oss << " " << sda.sinAzimuth;
    }
    if (flags->cosAzimuthFlag) {
      oss << " " << sda.cosAzimuth;
    }
    if (flags->sinParallacticAngleFlag) {
      oss << " " << sda.sinParallacticAngle;
    }
    if (flags->cosParallacticAngleFlag) {
      oss << " " << sda.cosParallacticAngle;
    }
    if (flags->bandwidthCodesFlag) {
      oss << " " << sda.bandwidthCodes;
    }
    if (flags->frontendFilterCodesFlag) {
      oss << " " << sda.frontendFilterCodes;
    }
    if (flags->zeroSpacingFluxFlag) {
      oss << " " << sda.zeroSpacingFlux;
    }
    if (flags->uvLimitsFlag) {
      for (Int i=0;i<2;i++) {
	oss << " " << sda.uvLimits[i];
      }
    }
    if (flags->arrayControlBitsFlag) {
      oss << " " << sda.arrayControlBits;
    }
    for (Int i=0; i<5; i++){
      if (flags->weatherInfoFlag[i]) {
	oss << " " << sda.weatherInfo[i];
      }
    }
    for (Int i=0; i<4; i++){
      if (flags->radialVelocityFlag[i]) {
	oss << " " << sda.radialVelocity[i];
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->lineRestFreqFlag[i]) {
	oss << " " << sda.lineRestFreq[i];
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->velocityReferenceFrameFlag[i]) {
	oss << " "; oss.write(sda.velocityReferenceFrame[i].chars(), 2);
      }
    }
    if (flags->correlatorModeFlag) {
      oss << " " << sda.correlatorMode;
    }
    if (flags->arrayProcessorOptionsFlag) {
      oss << " " << sda.arrayProcessorOptions;
    }
    if (flags->epochYearFlag) {
      oss << " " << sda.epochYear;
    }
    for (Int i=0; i<4; i++) {
      if (flags->channelOffsetsFlag[i]) {
	oss << " " << sda.channelOffsets[i];
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->channelSepCodesFlag[i]) {
	oss << " " << sda.channelSepCodes[i];
      }
    }
    if (flags->writeSomethingFlag) {
      oss << endl;
    }
  }
  return 0;
}

Int VlaAsciiSink::writeAscii(ostream& oss, const AntennaDataArea& ada,
			     const ADAFlags* flags){
  if (flags) {
    Int i = 0;
    if (flags->timeStampFlag) {
      oss << " " << rca.dateMJAD << " " << rca.timeIAT;
    }
    if (flags->antennaIDFlag) {
      oss << " " << ada.antennaID;
    }
    if (flags->dcsAddressFlag) {
      oss << " " << ada.dcsAddress;
    }
    if (flags->antennaControlBitsFlag) {
      oss << " " << ada.antennaControlBits;
    }
    if (flags->ifStatusFlag) {
      for (i=0; i<2; i++) {
	oss << " " << (Int)ada.ifStatus[i];
      }
    }
    if (flags->nominalSensitivityFlag) {
      for (i=0; i<4; i++) {
	oss << " " << ada.nominalSensitivity[i];
      }
    }
    if (flags->peculiarDelayFlag) {
      for (i=0; i<4; i++) {
	oss << " " << ada.peculiarDelay[i];
      }
    }
    if (flags->peculiarPhaseFlag) {
      oss << " " << ada.peculiarPhase;
    }
    if (flags->totalDelayAtEpochFlag) {
      oss << " " << ada.totalDelayAtEpoch;
    }
    if (flags->uFlag) {
      oss << " " << ada.u;
    }
    if (flags->vFlag) {
      oss << " " << ada.v;
    }
    if (flags->wFlag) {
      oss << " " << ada.w;
    }
    if (flags->bxFlag) {
      oss << " " << ada.bx;
    }
    if (flags->byFlag) {
      oss << " " << ada.by;
    }
    if (flags->bzFlag) {
      oss << " " << ada.bz;
    }
    if (flags->baFlag) {
      oss << " " << ada.ba;
    }
    if (flags->frontendTsysFlag) {
      for (i=0; i<4; i++) {
	oss << " " << ada.frontendTsys[i];
      }
    }
    if (flags->backendTsysFlag) {
      for (i=0; i<4; i++) {
	oss << " " << ada.backendTsys[i];
      }
    }
    if (flags->writeSomethingFlag) {
      oss << endl;
    }
  }
  return 0;
}

Int VlaAsciiSink::writeAscii(ostream &oss, const ContinuumCDAFlags *flags) {
  Int nAnts = rca.numberOfAntennas;
  if (flags) {
    for (Int vlaIF=0; vlaIF<2; vlaIF++) {
      VLAContinuumDatum* continuum = cda[vlaIF]->correlatorData.continuum;
      if (flags->dataFlag) {
	for (Int i=0; i<nAnts; i++) {
	  if (*(writeAntennae+i)) {
	    for (Int j=i+1; j<nAnts; j++) {
	      if (*(writeAntennae+j)) {
		oss << *(antIds + i) << " " << *(antIds + j) << " ";
		if (flags->headerFlag) {
		  oss << continuum->header(i,j,0) << " ";
		  oss << continuum->header(i,j,1) << " ";
		}
		if (flags->AwAFlag) {
		  for (Int m=0; m<3; m++) {
		    oss << continuum->dataAwA(i,j,m) << " ";
		  }
		}
		if (flags->CwCFlag) {
		  for (Int m=0; m<3; m++) {
		    oss << continuum->dataCwC(i,j,m) << " ";
		  }
		}
		if (flags->AwCFlag) {
		  for (Int m=0; m<3; m++)
		    oss << continuum->dataAwC(i,j,m) << " ";
		}
		if (flags->CwAFlag) {
		  for (Int m=0; m<3; m++) {
		    oss << continuum->dataCwA(i,j,m) << " ";
		  }
		}
		if(flags->writeSomethingFlag) {
		  oss << endl;
		}
	      }
	    }
	  }
	}
      }
    }
  }
  return 0;
}

Int VlaAsciiSink::writeAscii(ostream& oss, const SpectralLineCDAFlags* flags){
  Int numAntennas = rca.numberOfAntennas;
  if (flags) {
    for (Int vlaIF=0; vlaIF<4; vlaIF++) {
      if (flags->channelFlag) {
	VLASpectralLine* chanData = cda[vlaIF]->correlatorData.spectralLine;
	if (flags->baseLineFlag) {
	  Int nels (chanData->header.nelements());
	  oss << " " << chanData->header(nels-1);
	}
	for (Int i=0; i<numAntennas; i++) {
	  if (*(writeAntennae+i)) {
	    for (Int j=i+1; j<numAntennas; j++) {
	      if (*(writeAntennae+j)) {
		if (flags->allChannelsFlag) {
		  for (Int k=0; k<chanData->numberOfChannels; k++) {
		    oss << chanData->data(i,j,k);
		  }
		} else {
		  for (Int k=0; k<flags->channelCount; k++) {
		    oss << chanData->data(i,j,flags->channelID[k]);
		  }
		}
	      }
	    }
	  }
	}
	if (flags->writeSomethingFlag) {
	  oss << endl;
	}
      }
    }
  }
  return 0;
}
// Local Variables: 
// compile-command: "gmake OPTLIB=1 VlaAsciiSink"
// End: 
