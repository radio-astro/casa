//# VlaDAs.h:
//# Copyright (C) 1997,1999
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
//#
//# $Id$

#ifndef NRAO_VLADAS_H
#define NRAO_VLADAS_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>

#include <casa/namespace.h>
// <summary>
// Classes define VLA Data Archive structures
//</summary>

// <use visibility=local>   or   <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SomeClass
//   <li> SomeOtherClass
//   <li> some concept
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <templating arg=T>
//    <li>
//    <li>
// </templating>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>

// These structures are created pretty much byte for byte from the Hunt,
// Sowinski, and Bottomly memo on the VLA archive format.
//
// Note modcomp words are two bytes as I (wky) painfully found out.

struct RecordControlArea {
  Int     logicalRecordLength;
  Short   formatType;
  Short   formatRevision;
  Int     dateMJAD;
  Int     timeIAT;        // 19.2 Hz interrupt counts since IAT midnight
  String  controlProgramID;
  Int     ptr2SDA;
  Int     ptr2ADA;
  Short   lengthOfADA;
  Short   numberOfAntennas;
  Int     ptr2FirstCDA;
  Short   numWordsFirstBRH;
  Short   numWordsFirstBR;
  Int     ptr2SecondCDA;
  Short   numWordsSecondBRH;
  Short   numWordsSecondBR;
  Int     ptr2ThirdCDA;
  Short   numWordsThirdBRH;
  Short   numWordsThirdBR;
  Int     ptr2FourthCDA;
  Short   numWordsFourthBRH;
  Short   numWordsFourthBR;
  Short   recordSizePerBlockSize;
};

struct RCAFlags {
  Bool  logicalRecordLengthFlag;
  Bool  formatTypeFlag;
  Bool  formatRevisionFlag;
  Bool  dateMJADFlag;
  Bool  timeIATFlag;
  Bool  controlProgramIDFlag;
  Bool  ptr2SDAFlag;
  Bool  ptr2ADAFlag;
  Bool  lengthOfADAFlag;
  Bool  numberOfAntennasFlag;
  Bool  ptr2FirstCDAFlag;
  Bool  numWordsFirstBRHFlag;
  Bool  numWordsFirstBRFlag;
  Bool  ptr2SecondCDAFlag;
  Bool  numWordsSecondBRHFlag;
  Bool  numWordsSecondBRFlag;
  Bool  ptr2ThirdCDAFlag;
  Bool  numWordsThirdBRHFlag;
  Bool  numWordsThirdBRFlag;
  Bool  ptr2FourthCDAFlag;
  Bool  numWordsFourthBRHFlag;
  Bool  numWordsFourthBRFlag;
  Bool  recordSizePerBlockSizeFlag;
  Bool  writeSomethingFlag;
};

struct SubarrayDataArea {
  Short  subarrayID;
  String sourceName;
  Short  sourceNameQualifier;
  String arrayConfiguration;
  String observingProgramID;
  Short  aipsID;
  String observingMode;
  Char   calibratorCode;
  uChar  submode;
  Short  arrayStatusInformation;
  Short  numFreqChansPerBaseline;
  Short  integrationTime;            // 19.2 Hz interrupt counts
  Float  stopTime;                   // LST Radians
  Float  startTime;                  // LST Radians
  Double ra;                         // At standard Epoch
  Double dec;                        // At standard Epoch
  Double apparentRA;                 // Now
  Double apparentDec;                // Now
  Double sslos[4];                   // Signed Sum of LOs for IFS A-D, GHz
  Double skyFreq[4];                 // at Band Center channel 0, GHz
  Double iatEndOfIntegration;        // Radians
  Double lstEndOfIntegration;        // Radians
  Double iat4GeoCalculations;        // Radians
  Float  currentSurfaceRefractivity; // n-1
  Float  estimatedZenithAtmosPhase;  // nsec
  Float  sinElevation;
  Float  cosElevation;
  Float  sinAzimuth;
  Float  cosAzimuth;
  Float  cosParallacticAngle;
  Float  sinParallacticAngle;
  Short  bandwidthCodes;
  Short  frontendFilterCodes;
  Short  recirculatorControlCodes;
  Float  zeroSpacingFlux;
  Float  uvLimits[2];                // for on-line antsol
  Int    arrayControlBits;
  Float  weatherInfo[5];
  Double radialVelocity[4];          // km/s
  Double lineRestFreq[4];            // MHz
  String velocityReferenceFrame[4];
  String correlatorMode;
  String arrayProcessorOptions;
  Short  epochYear;
  Short  channelOffsets[4];
  Short  channelSepCodes[4];         // the k in 50MHz/2**k
};

struct SDAFlags {
  Bool   timeStampFlag;
  Bool   subarrayIDFlag;
  Bool   sourceNameFlag;
  Bool   sourceNameQualifierFlag;
  Bool   arrayConfigurationFlag;
  Bool   observingProgramIDFlag;
  Bool   aipsIDFlag;
  Bool   observingModeFlag;
  Bool   calibratorCodeFlag;
  Bool   submodeFlag;
  Bool   arrayStatusInformationFlag;
  Bool   numFreqChansPerBaselineFlag;
  Bool   integrationTimeFlag;
  Bool   stopTimeFlag;
  Bool   startTimeFlag;
  Bool   raFlag;
  Bool   decFlag;
  Bool   apparentRAFlag;
  Bool   apparentDecFlag;
  Bool   sslosFlag[4];
  Bool   skyFreqFlag[4];
  Bool   iatEndOfIntegrationFlag;
  Bool   lstEndOfIntegrationFlag;
  Bool   iat4GeoCalculationsFlag;
  Bool   currentSurfaceRefractivityFlag;
  Bool   estimatedZenithAtmosPhaseFlag;
  Bool   sinElevationFlag;
  Bool   cosElevationFlag;
  Bool   sinAzimuthFlag;
  Bool   cosAzimuthFlag;
  Bool   cosParallacticAngleFlag;
  Bool   sinParallacticAngleFlag;
  Bool   bandwidthCodesFlag;
  Bool   frontendFilterCodesFlag;
  Bool   recirculatorControlCodesFlag;
  Bool   zeroSpacingFluxFlag;
  Bool   uvLimitsFlag;
  Bool   arrayControlBitsFlag;
  Bool   weatherInfoFlag[5];
  Bool   radialVelocityFlag[4];
  Bool   lineRestFreqFlag[4];
  Bool   velocityReferenceFrameFlag[4];
  Bool   correlatorModeFlag;
  Bool   arrayProcessorOptionsFlag;
  Bool   epochYearFlag;
  Bool   channelOffsetsFlag[4];
  Bool   channelSepCodesFlag[4];
  Bool   writeSomethingFlag;
};

struct AntennaDataArea {
  uShort antennaID;
  uShort dcsAddress;
  Int    antennaControlBits;
  Char   ifStatus[2];
  Float  nominalSensitivity[4];// unitless
  Float  peculiarDelay[4];     // nsec
  Double peculiarPhase;        // turns
  Double totalDelayAtEpoch;    // nsec
  Float  u;                    // at center of integration for specified
  Float  v;                    // Epoch (nsec)
  Float  w;
  Double bx;                   // nsec
  Double by;         
  Double bz;
  Float  ba;                   // K-term (nsec)
  Float  frontendTsys[4];      // Kelvin
  Float  backendTsys[4];       // Kelvin
};

struct ADAFlags {
  Bool  timeStampFlag;
  Bool  antennaIDFlag;
  Bool  dcsAddressFlag;
  Bool  antennaControlBitsFlag;
  Bool  ifStatusFlag;
  Bool  nominalSensitivityFlag;
  Bool  peculiarDelayFlag;
  Bool  peculiarPhaseFlag;
  Bool  totalDelayAtEpochFlag;
  Bool  uFlag;
  Bool  vFlag;
  Bool  wFlag;
  Bool  bxFlag;
  Bool  byFlag;         
  Bool  bzFlag;
  Bool  baFlag;
  Bool  frontendTsysFlag;
  Bool  backendTsysFlag;
  Bool  writeSomethingFlag;
};

// Correlator Data comes in two flavors, continuum and spectral line
 
 
struct VLAContinuumDatum {
  Cube<Short>  header;
  Cube<Float>  dataAwA;
  Cube<Float>  dataCwC;
  Cube<Float>  dataAwC;
  Cube<Float>  dataCwA;
};

struct VLASpectralLine {
  Int               numberOfChannels;
  Vector<Short>     header;
  Cube<Complex>     data;
};
 
struct CorrelatorDataArea {
  Short              numAntennas;
  Int*               antennaIDs;
  Int*               writeAntennae;
  Bool               continuum;
  union {
    VLAContinuumDatum* continuum;
    VLASpectralLine*   spectralLine;
  } correlatorData;

// These two are replicated from the RCA for diagnostic purposes
 
  Int     dateMJAD;
  Int     timeIAT;      // 19.2 Hz interrupt counts since IAT midnight
 
};

struct ContinuumCDAFlags {
  Bool  timeStampFlag;
  Bool  headerFlag;
  Bool  dataFlag;
  Bool  AwAFlag;
  Bool  CwCFlag;
  Bool  AwCFlag;
  Bool  CwAFlag;
  Bool  writeSomethingFlag;
};

struct SpectralLineCDAFlags {
  Bool  timeStampFlag;
  Bool  channelFlag;
  Bool  writeSomethingFlag;
  Int   channelCount;
  Int*  channelID;
  Int   ifFlag[4];
  Bool  allChannelsFlag;
  Bool  baseLineFlag;
};

#endif
