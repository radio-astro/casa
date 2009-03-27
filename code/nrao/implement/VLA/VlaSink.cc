//# VlaSink.cc:
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

#include <nrao/VLA/VlaSink.h>
#include <string.h> // for memcpy
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Regex.h>
#include <math.h>
#include <casa/fstream.h>
#include <casa/sstream.h>

VlaSink::VlaSink()
  :antIds(0),
   writeAntennae(0),
   rcaFlags(0),
   sdaFlags(0),
   adaFlags(0),
   ccdaFlags(0),
   slcdaFlags(0)
{
  for (Int i=0; i<4; i++) cda[i] = 0;
}

VlaSink::~VlaSink() {
  for (Int i=0;i<4;i++){
    if (cda[i]) {
      if (cda[i]->antennaIDs && i==0) {
	delete [] cda[i]->antennaIDs;
      }
      if (cda[i]->continuum) {
	if (cda[i]->correlatorData.continuum) {
	  delete [] cda[i]->correlatorData.continuum;
	}
      } else {
	if(cda[i]->correlatorData.spectralLine) {
	  delete cda[i]->correlatorData.spectralLine;
	}
      }
      delete cda[i];
    }
  }
  if (antIds) delete [] antIds;
  if (rcaFlags) delete rcaFlags;
  if (sdaFlags) delete sdaFlags;
  if (adaFlags) delete adaFlags;
  if (ccdaFlags) delete ccdaFlags;
  if (slcdaFlags) delete slcdaFlags;
  if (writeAntennae) delete [] writeAntennae;
}

Int VlaSink::write(const Char* buffer) { 
  fillFromBuffer(buffer);
  Int rStat = ok();
  if(rStat > 0) rStat = writeVisibilityRecord(buffer);
  return rStat;
}
  
void VlaSink::setConstraints(const String& filename) {
  ifstream flag_file(filename.chars());
  if (flag_file) {
    Char buffer[80];
    String keyword;
    while (flag_file.getline(buffer, 80)){
      if (strlen(buffer)){
	istringstream input(buffer);
        input >> keyword;
	const char* keywordChars = keyword.chars();
	if (strstr(keywordChars, "#")) {
	  // do nothing it's a comment
	} else if (strstr(keywordChars, "RCA.")){
	  setFlags(*rcaFlags, buffer);
	} else if (strstr(keywordChars, "SDA.")){
	  setFlags(*sdaFlags, buffer);
	} else if (strstr(keywordChars, "ADA.")){
	  setFlags(*adaFlags, buffer);
	} else if (strstr(keywordChars, "CCDA.")){
	  setFlags(*ccdaFlags, buffer);
	} else if (strstr(keywordChars, "SLCDA.")){
	  setFlags (*slcdaFlags, buffer);
	} else if (strstr(keywordChars, "ALL.")){
	  dataSelectionCriteria.setValues(buffer);
	} else if (strstr(keywordChars, "DEFAULT")){
	  Char defaultChar;
	  istringstream input(buffer);
          input >> keyword >> defaultChar;
	  Bool default_value(True);
	  switch (defaultChar) {
	  case 'F' :
	  case '0' :
	    default_value = False;
	    break;
	  default :
	    default_value = True;
	    break;
	  }
	  setFlags(*rcaFlags, default_value);
	  setFlags(*sdaFlags, default_value);
	  setFlags(*adaFlags, default_value);
	  setFlags(*ccdaFlags, default_value);
	  setFlags(*slcdaFlags, default_value);
	} else {
	  cerr << "VlaSink::setConstraints:  Error Can\'t parse " 
	       << buffer << endl;
	}
      }
    }
  }
  return;
}

void VlaSink::fillFromBuffer(const Char* buffer) {
  // Fill the RCA record, offset is usually zero
  rca.logicalRecordLength    = vlaI42Int(buffer);
  rca.formatType             = vlaI22Short(buffer+4);
  rca.formatRevision         = vlaI22Short(buffer+6);
  rca.dateMJAD               = vlaI42Int(buffer+8);
  rca.timeIAT                = vlaI42Int(buffer+12);
  rca.controlProgramID       = vlaAS2String(buffer+16, 8); 
  rca.ptr2SDA                = 2*vlaI42Int(buffer+24);
  rca.ptr2ADA                = 2*vlaI42Int(buffer+28);
  rca.lengthOfADA            = 2*vlaI22Short(buffer+32);
  rca.numberOfAntennas       = vlaI22Short(buffer+34);
  rca.ptr2FirstCDA           = 2*vlaI42Int(buffer+36);
  rca.numWordsFirstBRH       = vlaI22Short(buffer+40);
  rca.numWordsFirstBR        = vlaI22Short(buffer+42);
  rca.ptr2SecondCDA          = 2*vlaI42Int(buffer+44);
  rca.numWordsSecondBRH      = vlaI22Short(buffer+48);
  rca.numWordsSecondBR       = vlaI22Short(buffer+50);
  rca.ptr2ThirdCDA           = 2*vlaI42Int(buffer+52);
  rca.numWordsThirdBRH       = vlaI22Short(buffer+56);
  rca.numWordsThirdBR        = vlaI22Short(buffer+58);
  rca.ptr2FourthCDA          = 2*vlaI42Int(buffer+60);
  rca.numWordsFourthBRH      = vlaI22Short(buffer+64);
  rca.numWordsFourthBR       = vlaI22Short(buffer+66);
  rca.recordSizePerBlockSize = vlaI22Short(buffer+68);
   
  // Assign the subarray data areas
  const Char *sdabuf = buffer + rca.ptr2SDA;
  sda.subarrayID             = vlaI22Short(sdabuf);
  sda.sourceName             = vlaAS2String(sdabuf+2, 16);
  sda.sourceNameQualifier    = vlaI22Short(sdabuf+18);
  sda.arrayConfiguration     = vlaAS2String(sdabuf+20, 2);
  sda.observingProgramID     = vlaAS2String(sdabuf+22, 6);
  sda.aipsID                 = vlaI22Short(sdabuf+28);
  sda.observingMode          = vlaAS2String(sdabuf+30, 4);
  sda.calibratorCode         = vlaByte2Char(sdabuf+32);
  sda.submode                = vlaByte2uChar(sdabuf+33);
  sda.arrayStatusInformation = vlaI22Short(sdabuf+34);
  sda.numFreqChansPerBaseline = vlaI22Short(sdabuf+36);
  sda.integrationTime         = vlaI22Short(sdabuf+38);
  sda.stopTime                = vlaFP2Float(sdabuf+40);
  sda.startTime               = vlaFP2Float(sdabuf+44);
  sda.ra                      = vlaDP2Double(sdabuf+48);
  sda.dec                     = vlaDP2Double(sdabuf+56);
  sda.apparentRA              = vlaDP2Double(sdabuf+64);
  sda.apparentDec             = vlaDP2Double(sdabuf+74);
  Int offset(80);
  for (Int i=0; i<4; i++) {
    sda.sslos[i] = vlaDP2Double(sdabuf+offset);
    offset += 8;      // sizeof modcomp DP
  }
  offset = 112;
  for (Int i=0; i<4; i++) {
    sda.skyFreq[i] = vlaDP2Double(sdabuf+offset);
    offset += 8;      // sizeof modcomp DP
  }
  sda.iatEndOfIntegration        = vlaDP2Double(sdabuf + 144);
  sda.lstEndOfIntegration        = vlaDP2Double(sdabuf + 152);
  sda.iat4GeoCalculations        = vlaDP2Double(sdabuf + 160);
  sda.currentSurfaceRefractivity = vlaFP2Float(sdabuf + 168);
  sda.estimatedZenithAtmosPhase  = vlaFP2Float(sdabuf + 172);
  sda.sinElevation               = vlaFP2Float(sdabuf + 176);
  sda.cosElevation               = vlaFP2Float(sdabuf + 180);
  sda.sinAzimuth                 = vlaFP2Float(sdabuf + 184);
  sda.cosAzimuth                 = vlaFP2Float(sdabuf + 188);
  sda.cosParallacticAngle        = vlaFP2Float(sdabuf + 192);
  sda.sinParallacticAngle        = vlaFP2Float(sdabuf + 196);
  sda.bandwidthCodes             = vlaI22Short(sdabuf + 200);
  sda.frontendFilterCodes        = vlaI22Short(sdabuf + 202);
  sda.recirculatorControlCodes   = vlaI22Short(sdabuf + 204);
  sda.zeroSpacingFlux            = vlaFP2Float(sdabuf + 206);
  offset = 210;
  for (Int i=0; i<2; i++) {
    sda.uvLimits[i] = vlaFP2Float(sdabuf+offset);
    offset += 4;      // sizeof modcomp FP
  }
  sda.arrayControlBits           = vlaI42Int(sdabuf + 218);
  offset = 222;
  for (Int i=0; i<5; i++) {
    sda.weatherInfo[i] = vlaFP2Float(sdabuf+offset);
    offset += 4;      // sizeof modcomp FP
  }
  offset = 242;
  for (Int i=0; i<4; i++) {
    sda.radialVelocity[i] = vlaDP2Double(sdabuf+offset);
    offset += 8;      // sizeof modcomp DP
  }
  offset = 274;
  for (Int i=0; i<4; i++) {
    sda.lineRestFreq[i] = vlaDP2Double(sdabuf+offset);
    offset += 8;      // sizeof modcomp DP
  }
  offset = 306;
  for (Int i=0; i<4; i++) {
    sda.velocityReferenceFrame[i] = vlaAS2String(sdabuf+offset, 2);
    offset += 2;      // sizeof modcomp DP
  }
  sda.correlatorMode        = vlaAS2String(sdabuf+314, 4);
  sda.arrayProcessorOptions = vlaAS2String(sdabuf+318, 4);
  sda.epochYear             = vlaI22Short(sdabuf+322);
  offset = 324;
  for (Int i=0; i<4; i++) {
    sda.channelOffsets[i] = vlaI22Short(sdabuf+offset);
    offset += 2;      // sizeof modcomp short
  }
  offset = 332;
  for(Int i=0; i<4; i++) {
    sda.channelSepCodes[i] = vlaI22Short(sdabuf+offset);
    offset += 2;      // sizeof modcomp short
  }
  //Assign the ADA
  Int* antIDs = new Int[rca.numberOfAntennas];
  for(Int i=0;i<rca.numberOfAntennas;i++){
    assignADA(ada[i], buffer + rca.ptr2ADA + i*rca.lengthOfADA);
    antIDs[i] = ada[i].antennaID;
  }
  //Assign the CDA
  for (Int i=0; i<4; i++) {
    if (cda[i]) {
      if (cda[i]->antennaIDs && i == 0) {
	delete [] cda[i]->antennaIDs;
	cda[i]->antennaIDs = 0;
      }
      if (cda[i]->correlatorData.continuum) {
	delete [] cda[i]->correlatorData.continuum;
	cda[i]->correlatorData.continuum = 0;
      }
      if (cda[i]->correlatorData.spectralLine) {
	delete cda[i]->correlatorData.spectralLine;
	cda[i]->correlatorData.spectralLine = 0;
      }
      delete cda[i];
    }
    cda[i]              = new CorrelatorDataArea;
    cda[i]->continuum   = True;
    cda[i]->correlatorData.continuum = 0;
    // cda[i]->correlatorData.spectralLine = 0;
    cda[i]->numAntennas = rca.numberOfAntennas;
    cda[i]->dateMJAD    = rca.dateMJAD;
    cda[i]->timeIAT     = rca.timeIAT;
    cda[i]->antennaIDs  = antIDs;
    if (isContinuum() || isHolography()) {
      assignContinuum(*cda[i], buffer + getCDAOffset(i));
    } else {
      cda[i]->correlatorData.spectralLine = new VLASpectralLine;
      uShort chans = 
	vlaCompNibble2uShort((const Char *)&sda.numFreqChansPerBaseline, i);
      cda[i]->correlatorData.spectralLine->numberOfChannels =
	(chans == 0) ? 0 : (1 << chans);
      assignSpectralLine(*cda[i], buffer + getCDAOffset(i),
			 getCDAHeader(i), getCDAData(i));
    }
  }
}

Int VlaSink::ok() {
  Int r_status(dataSelectionCriteria.
	       timeRangeOK((Double)rca.dateMJAD +
			   ((Double)rca.timeIAT)/(86400.0*19.2)));
  if(r_status > 0) {
    r_status = dataSelectionCriteria.sourceOK(sda.sourceName.chars());
    if(r_status) {
      r_status = dataSelectionCriteria.
	observingModeOK(sda.observingMode.chars());
      if(r_status){
	r_status = dataSelectionCriteria.subarrayOK(sda.subarrayID);
	if(r_status){
	  r_status = dataSelectionCriteria.
	    programIDOK(sda.observingProgramID.chars());
	  if(r_status)
	    dataSelectionCriteria.antennasOK(rca.numberOfAntennas,
					     antIds, writeAntennae);
	}
      }
    }
  }
  return r_status;
}

void VlaSink::allocateFlags() {
  rcaFlags = new RCAFlags;
  sdaFlags = new SDAFlags;
  adaFlags = new ADAFlags;
  ccdaFlags = new ContinuumCDAFlags;
  slcdaFlags = new SpectralLineCDAFlags;
  //
  initFlags(*rcaFlags, False);
  initFlags(*sdaFlags, False);
  initFlags(*adaFlags, False);
  initFlags(*ccdaFlags, False);
  initFlags(*slcdaFlags, False);
  //
  writeAntennae = new Int[27];
  for(uInt i=0; i<27; i++) writeAntennae[i] = 1;
  return;
}

// The vla2 routines were stolen from Gareth's VlaRecord.cc.  I've modified
// them so we get Shorts instead of Ints, but that's pretty much about it.
// Byte swapping is going to be needed for the endianess of the various
// boxes we need to support.

uShort VlaSink::vlaCompNibble2uShort(const Char* s, uInt nibble) {
  uShort value;
  memcpy (&value, s, 2);
  return ((value >> ((3-nibble)*4)) & 0xf);
}

Char VlaSink::vlaByte2Char(const Char* s) {
  Char value;
  memcpy (&value, s, 1);
  return value;
}

uChar VlaSink::vlaByte2uChar(const Char* s) {
  uChar value;
  memcpy (&value, s, 1);
  return value;
}

uShort VlaSink::vlaByte2uShort(const Char* s) {
  uChar value;
  memcpy (&value, s, 1);
  return (uShort)value;
}

Short VlaSink::vlaByte2Short(const Char* s) {
  uChar value;
  memcpy (&value, s, 1);
  return (Short)value;
}

uShort VlaSink::vlaI22uShort(const Char* s) {
  uShort value;
  memcpy (&value, s, 2);
  return value;
}

Short  VlaSink::vlaI22Short(const Char* s) {
  Short value;
  memcpy (&value, s, 2);
  return value;
}

Int    VlaSink::vlaI42Int(const Char* s) {
  Int value;
  memcpy (&value, s, 4);
  return value;
}

uInt   VlaSink::vlaI42uInt(const Char* s) {
  uInt value;
  memcpy (&value, s, 4);
  return value;
}

String VlaSink::vlaAS2String(const Char* s, uInt nBytes) {
  return String(s, nBytes).before(Regex(" *$"));
}

// These two functions are adapted from the Classic AIPS routines ZRM2RL.C and
// ZDM2DL.C.
//
// They have been adapted to work on a single value at a time.  The following
// variables are left as defines, as a reminder to take care of non-network
// ordered architectures at a later date.

#define Z_bytflp 0
#define Z_spfrmt 1
#define Z_dpfrmt 1

Float  VlaSink::vlaFP2Float(const Char* s) {
  Int test;
  uInt sign, exponent, mantissa, temp;
  union u_tag {
    Float r4;
    uInt u4;
    Short u2[2];
  } what;
/*--------------------------------------------------------------------*/
                                        /* routine works IEEE, VAX F  */
                                        /* Get Modcomp value.         */
  memcpy (&what, s, 4);
                                      /* swap words if needed       */
  if (Z_bytflp > 1) {
      Short sitemp = what.u2[0];
      what.u2[0] = what.u2[1];
      what.u2[1] = sitemp;
  }
                                      /* Get as unsigned int.       */
  temp = what.u4;
                                      /* Mask out sign bit.         */
  sign = 0x80000000 & temp;
                                      /* If negative, 2's           */
                                      /* complement the whole word. */
  if (sign == 0x80000000) temp = (~temp) + 1;
                                      /* Correct for exponent bias. */
  switch (Z_spfrmt) {
                                      /* IEEE (bias = -130?).       */
  case 1:
    test = ((0x7fc00000 & temp) >> 22) - 130;
    break;
                                      /* VAX F (bias = -128).       */
  case 2:
    test = ((0x7fc00000 & temp) >> 22) - 128;
    break;
  }

  exponent = test << 23;
  mantissa = (0x001fffff & temp) << 2;
  what.u4 = sign | exponent | mantissa;
                                      /* Overflow.                  */
  if (test > 255)
    what.u4 = ~0x0;
                                      /* Underflow.                 */
  else if (test < 1)
    what.u4 = 0;
                                      /* Store result.              */
  return what.r4;
}

Double VlaSink::vlaDP2Double(const Char* s) {
  Int test;
  uInt sign, exponent, mantissa, temp, templo, bits, xsign;
  union u_tag {
    Double r8; uInt u4[2];
    Short u2[4];
  } what;
/*--------------------------------------------------------------------*/
                                        /* routine works IEEE, VAX G  */
                                        /* Get Modcomp value.         */
  memcpy (&what, s, 8);
                                        /* swap words if needed       */
  if (Z_bytflp > 1) {
    Short sitemp = what.u2[0];
    what.u2[0] = what.u2[1];
    what.u2[1] = sitemp;
    sitemp = what.u2[2];
    what.u2[2] = what.u2[3];
    what.u2[3] = sitemp;
  }
                                        /* Get hi as unsigned int.    */
  temp = what.u4[0];
                                        /* Mask out sign bit.         */
  xsign = 0x80000000;
  sign = xsign & temp;
                                        /* If negative, 2's           */
                                        /* complement the whole word. */
  if (sign != 0) {
    templo = what.u4[1];
    what.u4[1] = ~templo + 1;
    temp = (~temp);
                                        /* If msb of lo word are      */
                                        /* unchanged, add 1 to high   */
                                        /* word.                      */
    if ((templo & 0x80000000) == (what.u4[1] & 0x80000000))
      temp = temp + 1;
  }
                                        /* Correct for exponent bias  */
                                        /* and trap for 0.            */
  switch (Z_dpfrmt) {

  case 1:
                                        /* IEEE (bias = 766?).        */
    test = ((0x7fc00000 & temp) >> 22) + 766;
    if (test == 766) test = 0;
    break;

  case 3:
                                        /* VAX G (bias = 768).        */
    test = ((0x7fc00000 & temp) >> 22) + 768;
    if (test == 768) test = 0;
    break;
  }

  exponent = test << 20;
  mantissa = (0x001fffff & temp);
                                        /* Move lsb to next word.     */
  bits = (mantissa & 0x1) << 31;
                                        /* Shift high mantissa.       */
  mantissa = mantissa >> 1;
                                        /* Shift low mantissa.        */
  what.u4[1] = what.u4[1] >> 1;
                                        /* Lsb from hi word.          */
  what.u4[1] = what.u4[1] | bits;
  what.u4[0] = sign | exponent | mantissa;
                                        /* swap words if needed       */
  if (Z_bytflp > 1) {
    Short sitemp = what.u2[0];
    what.u2[0] = what.u2[2];
    what.u2[2] = sitemp;
    sitemp = what.u2[1];
    what.u2[1] = what.u2[3];
    what.u2[3] = sitemp;
  }
                                        /* Store result.              */
  return what.r8;
}


Int VlaSink::getCDAOffset(Int vlaIF) {
  Int offset = -1;
  switch (vlaIF) {
  case 0 :
    offset = rca.ptr2FirstCDA;
    break;
  case 1 :
    offset = rca.ptr2SecondCDA;
    break;
  case 2 :
    offset = rca.ptr2ThirdCDA;
    break;
  case 3 :
    offset = rca.ptr2FourthCDA;
    break;
  }
  return offset;
}

Int VlaSink::getCDAHeader(Int vlaIF)
{
  Int offset = -1;
  switch(vlaIF) {
  case 0 :
    offset = 2*rca.numWordsFirstBRH;
    break;
  case 1 :
    offset = 2*rca.numWordsSecondBRH;
    break;
  case 2 :
    offset = 2*rca.numWordsThirdBRH;
    break;
  case 3 :
    offset = 2*rca.numWordsFourthBRH;
    break;
  }
  return offset;
}

Int VlaSink::getCDAData(Int vlaIF)
{
  Int offset(-1);
  switch(vlaIF){
  case 0 :
    offset = 2*rca.numWordsFirstBR;
    break;
  case 1 :
    offset = 2*rca.numWordsSecondBR;
    break;
  case 2 :
    offset = 2*rca.numWordsThirdBR;
    break;
  case 3 :
    offset = 2*rca.numWordsFourthBR;
    break;
  }
  return offset;
}

void VlaSink::assignADA(AntennaDataArea& ada, const Char* buffer) {
  ada.antennaID             = vlaByte2Short(buffer);
  ada.dcsAddress            = vlaByte2uShort(buffer +1);
  ada.antennaControlBits    = vlaI42Int(buffer + 2);
  ada.ifStatus[0]           = vlaByte2Char(buffer + 6);
  ada.ifStatus[1]           = vlaByte2Char(buffer + 7);
  Int offset(8);
  for (Int i=0; i<4; i++) {
    ada.nominalSensitivity[i] = vlaFP2Float(buffer+offset);
    offset += 4;      // sizeof modcomp FP
  }
  offset = 24;
  for (Int i=0; i<4; i++) {
    ada.peculiarDelay[i] = vlaFP2Float(buffer+offset);
    offset += 4;      // sizeof modcomp FP
  }
  ada.peculiarPhase     = vlaDP2Double(buffer+40);
  ada.totalDelayAtEpoch = vlaDP2Double(buffer+48);
  ada.u                 = vlaFP2Float(buffer+56);
  ada.v                 = vlaFP2Float(buffer+60);
  ada.w                 = vlaFP2Float(buffer+64);
  ada.bx                = vlaDP2Double(buffer+68);
  ada.by                = vlaDP2Double(buffer+76);
  ada.bz                = vlaDP2Double(buffer+84);
  ada.ba                = vlaFP2Float(buffer+92);
  offset = 96;
  for (Int i=0; i<4; i++) {
    ada.frontendTsys[i] = vlaFP2Float(buffer+offset);
    offset += 4;      // sizeof modcomp FP
  }
  offset = 112;
  for (Int i=0; i<4; i++) {
    ada.backendTsys[i] = vlaFP2Float(buffer+offset);
    offset += 4;      // sizeof modcomp FP
  }
  return; 
}

void VlaSink::assignContinuum(CorrelatorDataArea &cda, const Char *buffer) {  
  VLAContinuumDatum* continuum = cda.correlatorData.continuum =
    new VLAContinuumDatum[rca.numberOfAntennas*rca.numberOfAntennas];
  Int offset = 0;
  cda.continuum = True;
  for (Int i=0; i<rca.numberOfAntennas; i++) {
    assignContinuumDatum(*(continuum + i*rca.numberOfAntennas + i),
			 buffer+offset);
    offset += sizeof(VLAContinuumDatum);
  }
  for (Int i=0; i<rca.numberOfAntennas; i++) {
    for (Int j=i+1; j<rca.numberOfAntennas; j++) {
      offset += assignContinuumDatum(*continuum, buffer+offset);
    }
  }
  return;
}

Int VlaSink::assignContinuumDatum(VLAContinuumDatum& datum, 
				  const Char* buffer) {
  Int offset = 0;
  datum.dataAwA.resize(rca.numberOfAntennas, rca.numberOfAntennas, 3);
  datum.dataCwC.resize(rca.numberOfAntennas, rca.numberOfAntennas, 3);
  datum.dataAwC.resize(rca.numberOfAntennas, rca.numberOfAntennas, 3);
  datum.dataCwA.resize(rca.numberOfAntennas, rca.numberOfAntennas, 3);
  datum.header.resize(rca.numberOfAntennas, rca.numberOfAntennas, 2);
  
  for (Int i=0; i < rca.numberOfAntennas; i++){
    for (Int j=i; j < rca.numberOfAntennas; j++) {
      for (Int ih=0; ih<2; ih++) {
	datum.header(i, j, ih) = vlaI22Short(buffer+offset);
	offset += 2;
      }
      //
      // Scale factor is in the first word of the header.  Decode it and apply
      // the scaling to the raw data.
      //
      uInt g((datum.header(i,j,0) >> 16) & 0x1f);
      Float scaleFactor(pow(2.0, -(Double)g));
      for (Int m=0; m<3; m++) {
	datum.dataAwA(i,j,m) = scaleFactor*vlaI22Short(buffer+offset);
	offset += 2;
      }
      for (Int m=0; m<3; m++) {
	datum.dataCwC(i,j,m) = scaleFactor*vlaI22Short(buffer+offset);
	offset += 2;
      }
      for (Int m=0; m<3; m++) {
	datum.dataAwC(i,j,m) = scaleFactor*vlaI22Short(buffer+offset);
	offset += 2;
      }
      for (Int m=0; m<3; m++) {
	datum.dataCwA(i,j,m) = scaleFactor*vlaI22Short(buffer+offset);
	offset += 2;
      }
    }
  }
  return offset;
}

void VlaSink::assignSpectralLine(CorrelatorDataArea& cda, const Char* buffer,
                                 const Int sizeBRH, const Int sizeBR) {  
  cda.continuum = False;
  //    int NumCorrelations(cda.correlatorData.spectralLine->numberOfChannels*
  // 		       rca.numberOfAntennas*(rca.numberOfAntennas-1)/2);
  
  cda.correlatorData.spectralLine->
    header.resize(cda.correlatorData.spectralLine->numberOfChannels);
  cda.correlatorData.spectralLine->
    data.resize(cda.numAntennas, cda.numAntennas,
		cda.correlatorData.spectralLine->numberOfChannels);
  Int flags = (cda.correlatorData.spectralLine->numberOfChannels/16 > 0) ? 1 :
    cda.correlatorData.spectralLine->numberOfChannels/16;
  for (Int i=0;i<cda.numAntennas;i++) {
    for (Int j=0; j<flags; j++) {
      cda.correlatorData.spectralLine->header(j) = 
	vlaI22Short(buffer + 2*i*(sizeBR+sizeBRH) + 2*j);
    }
    Int nels = cda.correlatorData.spectralLine->header.nelements();
    uInt g = (cda.correlatorData.spectralLine->header(nels-2) >> 16) & 0x1f;
    Float scaleFactor = pow(2.0, -(Double)g);
    for (Int j=0; j<cda.numAntennas; j++) {
      for (Int m=j; m<cda.correlatorData.spectralLine->numberOfChannels; m++){
	Short rPart = vlaI22Short(buffer
				  + 2*(i*cda.numAntennas)*(sizeBR+sizeBRH)
				  + 2*j*(sizeBR+sizeBRH) + 4*m);
	Short iPart = vlaI22Short(buffer
				  + 2*(i*cda.numAntennas)*(sizeBR+sizeBRH)
				  + 2*j*(sizeBR+sizeBRH) + 4*m + 2);
	cda.correlatorData.spectralLine->data(i,j,m) = 
	  scaleFactor*Complex(rPart, iPart);
      }
    }
  }
  return;
}

Bool VlaSink::isContinuum() {
  return (sda.numFreqChansPerBaseline == 0);
}

Bool VlaSink::isHolography() {
  return (sda.observingMode[0] == 'H');
}

void VlaSink::setFlags(RCAFlags& rca, Char* buffer) {
  String keyword;
  Char val;
  Bool flag = False;
  istringstream input(buffer);
  input >> keyword >> val;
  const char* keywordChars = keyword.chars();
  if (val == 'T' || val == '1') {
    flag = True;
  }
  if (!strcasecmp(keywordChars, "RCA.LogicalRecordLength")) {
    rca.logicalRecordLengthFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.FormatType")) {
    rca.formatTypeFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.FormatRevision")) {
    rca.formatRevisionFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.DateMJAD")) {
    rca.dateMJADFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.TimeIAT")) {
    rca.timeIATFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.ControlProgramID")) {
    rca.controlProgramIDFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.Ptr2SDA")) {
    rca.ptr2SDAFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.Ptr2ADA")) {
    rca.ptr2ADAFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.LengthOfADA")) {
    rca.lengthOfADAFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.NumberOfAntennas")) {
    rca.numberOfAntennasFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.Ptr2FirstCDA")) {
    rca.ptr2FirstCDAFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.NumWordsFirstBRH")) {
    rca.numWordsFirstBRHFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.NumWordsFirstBR")) {
    rca.numWordsFirstBRFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.Ptr2SecondCDA")) {
    rca.ptr2SecondCDAFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.NumWordsSecondBRH")) {
    rca.numWordsSecondBRHFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.NumWordsSecondBR")) {
    rca.numWordsSecondBRFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.Ptr2ThirdCDA")) {
    rca.ptr2ThirdCDAFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.NumWordsThirdBRH")) {
    rca.numWordsThirdBRHFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.NumWordsThirdBR")) {
    rca.numWordsThirdBRFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.Ptr2FourthCDA")) {
    rca.ptr2FourthCDAFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.NumWordsFourthBRH")) {
    rca.numWordsFourthBRHFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.NumWordsFourthBR")) {
    rca.numWordsFourthBRFlag = flag;
  }
  if (!strcasecmp(keywordChars, "RCA.RecordSizePerBlockSize")) {
    rca.recordSizePerBlockSizeFlag = flag;
  }
  if (flag) {
    rca.writeSomethingFlag = True;
  }
  return;
}

void VlaSink::setFlags(SDAFlags& sda, Char* buffer) {
  String keyword;
  Char val;
  Bool flag = False;
  istringstream input(buffer);
  input >> keyword >> val;
  const char* keywordChars = keyword.chars();
  if (val == 'T' || val == '1') {
    flag = True;
  }
  if (!strcasecmp(keywordChars, "SDA.TimeStamp")) {
    sda.timeStampFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SubarrayID")) {
    sda.subarrayIDFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SourceName")) {
    sda.sourceNameFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SourceNameQualifier")) {
    sda.sourceNameQualifierFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ArrayConfiguration")) {
    sda.arrayConfigurationFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ObservingProgramID")) {
    sda.observingProgramIDFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.AIPSID")) {
    sda.aipsIDFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ObservingMode")) {
    sda.observingModeFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.CalibratorCode")) {
    sda.calibratorCodeFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.Submode")) {
    sda.submodeFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ArrayStatusInformation")) {
    sda.arrayStatusInformationFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.NumFreqChannelsPerBaseline")) {
    sda.numFreqChansPerBaselineFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.IntegrationTime")) {
    sda.integrationTimeFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.StopTime")) {
    sda.stopTimeFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.StartTime")) {
    sda.startTimeFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.RA")) {
    sda.raFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.Dec")) {
    sda.decFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ApparentRA")) {
    sda.apparentRAFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ApparentDec")) {
    sda.apparentDecFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SSLOs[1]")) {
    sda.sslosFlag[0] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SSLOs[2]")) {
    sda.sslosFlag[1] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SSLOs[3]")) {
    sda.sslosFlag[2] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SSLOs[4]")) {
    sda.sslosFlag[3] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SkyFreq[1]")) {
    sda.skyFreqFlag[0] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SkyFreq[2]")) {
    sda.skyFreqFlag[1] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SkyFreq[3]")) {
    sda.skyFreqFlag[2] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SkyFreq[4]")) {
    sda.skyFreqFlag[3] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.IATEndOfIntegration")) {
    sda.iatEndOfIntegrationFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.LSTEndOfIntegration")) {
    sda.lstEndOfIntegrationFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.IAT4GeoCalculations")) {
    sda.iat4GeoCalculationsFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.CurrentSurfaceRefractivity")) {
    sda.currentSurfaceRefractivityFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.EstimatedZenithAtmosPhase")) {
    sda.estimatedZenithAtmosPhaseFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SinElevation")) {
    sda.sinElevationFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.CosElevation")) {
    sda.cosElevationFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SinAzimuth")) {
    sda.sinAzimuthFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.CosAzimuth")) {
    sda.cosAzimuthFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.CosParallacticAngle")) {
    sda.cosParallacticAngleFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.SinParallacticAngle")) {
    sda.sinParallacticAngleFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.BandwidthCodes")) {
    sda.bandwidthCodesFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.FrontendFilterCodes")) {
    sda.frontendFilterCodesFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.RecirculatorControlCodes")) {
    sda.recirculatorControlCodesFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ZeroSpacingFlux")) {
    sda.zeroSpacingFluxFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.UVLimits")) {
    sda.uvLimitsFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ArrayControlBits")) {
    sda.arrayControlBitsFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.WeatherInfo[1]")) {
    sda.weatherInfoFlag[0] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.WeatherInfo[2]")) {
    sda.weatherInfoFlag[1] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.WeatherInfo[3]")) {
    sda.weatherInfoFlag[2] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.WeatherInfo[4]")) {
    sda.weatherInfoFlag[3] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.WeatherInfo[5]")) {
    sda.weatherInfoFlag[4] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.RadialVelocity[1]")) {
    sda.radialVelocityFlag[0] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.RadialVelocity[2]")) {
    sda.radialVelocityFlag[1] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.RadialVelocity[3]")) {
    sda.radialVelocityFlag[2] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.RadialVelocity[4]")) {
    sda.radialVelocityFlag[3] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.LineRestFreq[1]")) {
    sda.lineRestFreqFlag[0] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.LineRestFreq[2]")) {
    sda.lineRestFreqFlag[1] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.LineRestFreq[3]")) {
    sda.lineRestFreqFlag[2] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.LineRestFreq[4]")) {
    sda.lineRestFreqFlag[3] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.VelocityReferenceFrame[1]")) {
    sda.velocityReferenceFrameFlag[0] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.VelocityReferenceFrame[2]")) {
    sda.velocityReferenceFrameFlag[1] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.VelocityReferenceFrame[3]")) {
    sda.velocityReferenceFrameFlag[2] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.VelocityReferenceFrame[4]")) {
    sda.velocityReferenceFrameFlag[3] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.CorrelatorMode")) {
    sda.correlatorModeFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ArrayProcessorOptions")) {
    sda.arrayProcessorOptionsFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.EpochYear")) {
    sda.epochYearFlag = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ChannelOffsets[1]")) {
    sda.channelOffsetsFlag[0] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ChannelOffsets[2]")) {
    sda.channelOffsetsFlag[1] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ChannelOffsets[3]")) {
    sda.channelOffsetsFlag[2] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ChannelOffsets[4]")) {
    sda.channelOffsetsFlag[3] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ChannelSepCodes[1]")) {
    sda.channelSepCodesFlag[0] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ChannelSepCodes[2]")) {
    sda.channelSepCodesFlag[1] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ChannelSepCodes[3]")) {
    sda.channelSepCodesFlag[2] = flag;
  }
  if (!strcasecmp(keywordChars, "SDA.ChannelSepCodes[4]")) {
    sda.channelSepCodesFlag[3] = flag;
  }
  if (flag) {
    sda.writeSomethingFlag = True;
  }
  return;
}

void VlaSink::setFlags(ADAFlags& ada, Char* buffer) {
  String keyword;
  Char val;
  Bool flag = False;
  istringstream input(buffer);
  input >> keyword >> val;
  const char* keywordChars = keyword.chars();
  if (val == 'T' || val == '1') {
    flag = True;
  }
  if (!strcasecmp(keywordChars, "ADA.TimeStamp")) {
    ada.timeStampFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.AntennaID")) {
    ada.antennaIDFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.DCSaddress")) {
    ada.dcsAddressFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.AntennaControlBits")) {
    ada.antennaControlBitsFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.IFStatus")) {
    ada.ifStatusFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.NominalSensitivity")) {
    ada.nominalSensitivityFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.PeculiarDelay")) {
    ada.peculiarDelayFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.PeculiarPhase")) {
    ada.peculiarPhaseFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.TotalDelayAtEpoch")) {
    ada.totalDelayAtEpochFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.U")) {
    ada.uFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.V")) {
    ada.vFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.W")) {
    ada.wFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.Bx")) {
    ada.bxFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.By")) {
    ada.byFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.Bz")) {
    ada.bzFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.Ba")) {
    ada.baFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.FrontendTsys")) {
    ada.frontendTsysFlag = flag;
  }
  if (!strcasecmp(keywordChars, "ADA.BackendTsys")) {
    ada.backendTsysFlag = flag;
  }
  if (flag) {
    ada.writeSomethingFlag = True;
  }
  return;
}

void VlaSink::setFlags(ContinuumCDAFlags& cda, Char* buffer) {
  String keyword;
  Char val;
  Bool flag = False;
  istringstream input(buffer);
  input >> keyword >> val;
  const char* keywordChars = keyword.chars();
  if (val == 'T' || val == '1') {
    flag = True;
  }
  if (!strcasecmp(keywordChars, "CCDA.TimeStamp")) {
    cda.timeStampFlag = flag;
  }
  if (!strcasecmp(keywordChars, "CCDA.Header")) {
    cda.headerFlag = flag;
  }
  if (!strcasecmp(keywordChars, "CCDA.Data")) {
    cda.dataFlag = flag;
  }
  if (!strcasecmp(keywordChars, "CCDA.AwA")) {
    cda.AwAFlag = flag;
  }
  if (!strcasecmp(keywordChars, "CCDA.CwC")) {
    cda.CwCFlag = flag;
  }
  if (!strcasecmp(keywordChars, "CCDA.AwC")) {
    cda.AwCFlag = flag;
  }
  if (!strcasecmp(keywordChars, "CCDA.CwA")) {
    cda.CwAFlag = flag;
  }
  if (flag) {
    cda.writeSomethingFlag = True;
  }
  return;
}

void VlaSink::setFlags(SpectralLineCDAFlags& cda, Char* buffer) {
  Char keyword[80];
  Char val;
  Bool flag = False;
  istringstream iss(buffer);
  iss >> keyword >> val;
  if (val == 'T' || val == '1') {
    flag = True;
  }
  if (!strcasecmp(keyword, "SLCDA.TimeStamp")) {
    cda.timeStampFlag = flag;
  }
  if (!strcasecmp(keyword, "SLCDA.Channel")) {
    cda.channelFlag = flag;
  }
  if (!strcasecmp(keyword, "SLCDA.BaseLine")) {
    cda.baseLineFlag = flag;
  }
  if (!strcasecmp(keyword, "SLCDA.AllChannels")) {
    cda.allChannelsFlag = flag;
  }
  if (!strcasecmp(keyword, "SLCDA.IF")) {
    cda.ifFlag[0] = flag;
    for (Int i=1; i<4; i++) {
      iss >> val;
      if (val == 'T' || val == '1') {
	flag = True;
      }
      cda.ifFlag[i] = flag;
    }
  }
  if (!strcasecmp(keyword, "SLCDA.ChannelID")) {
    cda.channelCount = val;
    cda.channelID = new Int[val];
    for (Int i=0; i<val; i++) {
      iss >> cda.channelID[i];
    }
  }
  if(flag) {
    cda.writeSomethingFlag = True;
  }
  return;
}

void VlaSink::setFlags(RCAFlags& rca, Bool flag) {
  initFlags(rca, flag);
  return;
}

void VlaSink::setFlags(SDAFlags& rca, Bool flag) {
  initFlags(rca, flag);
  return;
}

void VlaSink::setFlags(ADAFlags& rca, Bool flag) {
  initFlags(rca, flag);
  return;
}

void VlaSink::setFlags(ContinuumCDAFlags& rca, Bool flag) {
  initFlags(rca, flag);
  return;
}

void VlaSink::setFlags(SpectralLineCDAFlags& rca, Bool flag) {
  initFlags(rca, flag);
  return;
}


void VlaSink::initFlags(RCAFlags& rca, Bool flag) {
  rca.logicalRecordLengthFlag    = flag;
  rca.formatTypeFlag             = flag;
  rca.formatRevisionFlag         = flag;
  rca.dateMJADFlag               = flag;
  rca.timeIATFlag                = flag;
  rca.controlProgramIDFlag       = flag;
  rca.ptr2SDAFlag                = flag;
  rca.ptr2ADAFlag                = flag;
  rca.lengthOfADAFlag            = flag;
  rca.numberOfAntennasFlag       = flag;
  rca.ptr2FirstCDAFlag           = flag;
  rca.numWordsFirstBRHFlag       = flag;
  rca.numWordsFirstBRFlag        = flag;
  rca.ptr2SecondCDAFlag          = flag;
  rca.numWordsSecondBRHFlag      = flag;
  rca.numWordsSecondBRFlag       = flag;
  rca.ptr2ThirdCDAFlag           = flag;
  rca.numWordsThirdBRHFlag       = flag;
  rca.numWordsThirdBRFlag        = flag;
  rca.ptr2FourthCDAFlag          = flag;
  rca.numWordsFourthBRHFlag      = flag;
  rca.numWordsFourthBRFlag       = flag;
  rca.recordSizePerBlockSizeFlag = flag;
  rca.writeSomethingFlag         = flag;
  return;
}

void VlaSink::initFlags(SDAFlags& sda, Bool flag) { 
  sda.subarrayIDFlag                 = flag;
  sda.sourceNameFlag                 = flag;
  sda.sourceNameQualifierFlag        = flag;
  sda.arrayConfigurationFlag         = flag;
  sda.observingProgramIDFlag         = flag;
  sda.aipsIDFlag                     = flag;
  sda.observingModeFlag              = flag;
  sda.calibratorCodeFlag             = flag;
  sda.submodeFlag                    = flag;
  sda.arrayStatusInformationFlag     = flag;
  sda.numFreqChansPerBaselineFlag    = flag;
  sda.integrationTimeFlag            = flag;
  sda.stopTimeFlag                   = flag;
  sda.startTimeFlag                  = flag;
  sda.raFlag                         = flag;
  sda.decFlag                        = flag;
  sda.apparentRAFlag                 = flag;
  sda.apparentDecFlag                = flag;
  sda.iatEndOfIntegrationFlag        = flag;
  sda.lstEndOfIntegrationFlag        = flag;
  sda.iat4GeoCalculationsFlag        = flag;
  sda.currentSurfaceRefractivityFlag = flag;
  sda.estimatedZenithAtmosPhaseFlag  = flag;
  sda.sinElevationFlag               = flag;
  sda.cosElevationFlag               = flag;
  sda.sinAzimuthFlag                 = flag;
  sda.cosAzimuthFlag                 = flag;
  sda.cosParallacticAngleFlag        = flag;
  sda.sinParallacticAngleFlag        = flag;
  sda.bandwidthCodesFlag             = flag;
  sda.frontendFilterCodesFlag        = flag;
  sda.recirculatorControlCodesFlag   = flag;
  sda.zeroSpacingFluxFlag            = flag;
  sda.uvLimitsFlag                   = flag;
  sda.arrayControlBitsFlag           = flag;
  sda.epochYearFlag                  = flag;
  sda.writeSomethingFlag             = flag;
  sda.correlatorModeFlag             = flag;
  sda.arrayProcessorOptionsFlag      = flag;
  for (Int i=0; i<4; i++) {
    sda.sslosFlag[i]                  = flag;
    sda.skyFreqFlag[i]                = flag;
    sda.radialVelocityFlag[i]         = flag;
    sda.lineRestFreqFlag[i]           = flag;
    sda.velocityReferenceFrameFlag[i] = flag;
    sda.channelOffsetsFlag[i]         = flag;
    sda.channelSepCodesFlag[i]        = flag;
    sda.weatherInfoFlag[i]            = flag;
  }
  sda.weatherInfoFlag[4] = flag;
  return;
}

void VlaSink::initFlags(ADAFlags& ada, Bool flag) {
  ada.timeStampFlag          = flag;
  ada.antennaIDFlag          = flag;
  ada.dcsAddressFlag         = flag;
  ada.antennaControlBitsFlag = flag;
  ada.ifStatusFlag           = flag;
  ada.nominalSensitivityFlag = flag;
  ada.peculiarDelayFlag      = flag;
  ada.peculiarPhaseFlag      = flag;
  ada.totalDelayAtEpochFlag  = flag;
  ada.uFlag                  = flag;
  ada.vFlag                  = flag;
  ada.wFlag                  = flag;
  ada.bxFlag                 = flag;
  ada.byFlag                 = flag;
  ada.bzFlag                 = flag;
  ada.baFlag                 = flag;
  ada.frontendTsysFlag       = flag;
  ada.backendTsysFlag        = flag;
  ada.writeSomethingFlag     = flag;
  return;
}

void VlaSink::initFlags(ContinuumCDAFlags& cda, Bool flag) {
  cda.headerFlag = flag;
  cda.writeSomethingFlag = flag;
  cda.AwAFlag = flag;
  cda.CwCFlag = flag;
  cda.AwCFlag = flag;
  cda.CwAFlag = flag;
  cda.dataFlag = flag;
  return;
}

void VlaSink::initFlags(SpectralLineCDAFlags& cda, Bool flag) {
  cda.timeStampFlag = flag;
  cda.channelFlag = flag;
  cda.allChannelsFlag = flag;
  cda.writeSomethingFlag = flag;
  cda.channelCount = 0;
  cda.channelID = 0;
  for (Int i=0; i<4; i++) {
    cda.ifFlag[i] = flag;
  }
  return;
}

// Local Variables: 
// compile-command: "gmake OPTLIB=1 VlaSink"
// End: 
