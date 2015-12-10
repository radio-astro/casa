//# ATCAFiller.h: Definition for ATCA filler 
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
//# $Id: ATCAFiller.h,v 1.6 2004/12/13 04:44:35 mwiering Exp $

#ifndef ATNF_ATCAFILLER_H
#define ATNF_ATCAFILLER_H

//# Includes
#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Logging/LogIO.h>
//#include <tables/Tables/ArrayColumn.h>
//#include <tables/Tables/ScalarColumn.h>
#include <tables/DataMan/TiledDataStManAccessor.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>

#include <casa/namespace.h>
namespace casa { //# NAMESPACE CASA - BEGIN
class String;

class ATCAFiller {
public:

  // Construct the ATCAFiller
  ATCAFiller();

  ~ATCAFiller();

  // return the ATCA required tabledesc, this is the standard MeasurementSet
  // plus ATCA specific additions.
  static TableDesc atcaTableDesc(Bool compress);

  // make the ATCA specific MeasurementSet
  static MeasurementSet makeTable(const String& tableName, Bool compress,
    Bool cabb);

  // make the subtables with ATCA specific additions
  static void makeSubTables(MS& ms, Table::TableOption option, Bool cabb);

  // Open the measurement set and one or more RPFits files.
  // Opacity correction is not yet implemented - could go
  // in gencal or here, if gencal scheme doesn't improve
  Bool open(const String& msName, const Vector<String>& rpfitsFiles,
	          const Vector<String> & options, Int opcor=2);

 // Fill the measurement set
  Bool fill();
 
  // Select a number of fields by name.
  ATCAFiller & fields(const Vector<String> & fieldList);

  // Select a range of frequencies, lowFreq=0 => everything below higFreq,
  //  highFreq=0 => everything above lowFreq.
  ATCAFiller & freqRange(Double lowFreq, Double highFreq=0);
 
  // Select frequencies within windowWidth of specified ones.
  // (This selects on center-frequencies only, not channelfrequencies)
  ATCAFiller & frequencies(Vector<Double> freqs, Double windowWidth=1e6);

  // Select a range of scans to read. first=0 or 1 => start at first one,
  //   last=0 => read to end of file.
  ATCAFiller & scanRange(Int firstScan, Int lastScan=0);

  // Select range of channels, with optional increment.
  // We may want multiple channel ranges -> use matrix(3,n) for selection?
  ATCAFiller & chanRange(Int firstChan, Int lastChan, Int ChanInc=1);

  // Time range selection.
  ATCAFiller & timeRange(Double firstTime, Double lastTime=0);

  // Select the Freq (which one of the simult. freqs), 0=> no selection.
  // 
  ATCAFiller & freqSel(const Vector<Int>& spws); 

  // Select on bandwidth of IF 1.
  ATCAFiller & bandwidth1(Int bandwidth1);

  // Select on number of channels of IF 1
  ATCAFiller & numChan1(Int numchan1);

  // Set shadowing limit
  ATCAFiller & shadow(Double diam);

  // Deselect antennas.
  ATCAFiller & deselectAntenna(Vector<Int> antennas);

  // Smooth xy-phases with running median and 
  // flag data with discrepant xy-phase.
  ATCAFiller & xyPhaseSmooth(Int window=9, Double tolerance=10.0);

  // Smooth Tsys values with running median and recalibrate the data.
  ATCAFiller & tsysSmooth(Int window=9);
  
  // Set percentage of channels to flag at band edges
  ATCAFiller & edge(Float edge);

private:
  //# disallow all these
  //ATCAFiller();
  ATCAFiller(const ATCAFiller &);
  ATCAFiller & operator=(const ATCAFiller &);

  //for constructors
  void init();

  void storeHeader();
  void storeATCAHeader();
  void storeSysCal();

  // fill a single input file
  Bool fill1(const String & rpfitsname);

  // List the file on cout
  void list();

   // Flag data if samplerstats are bad.
  Bool samplerFlag(Int row, Double posNegTolerance=3.0, 
                   Double zeroTolerance=0.5);
  
  Int birdChan(Double refFreq, Int refChan, Double chanSpac);
  void reweight();
  void storeData();
  Int checkSpW(Int ifNumber,Bool log=True);
  void checkField();
  void checkObservation();
  // Fill the feed table (with dummy values)
  void fillFeedTable();
  void fillObservationTable();
  void fillMeasureReferences();
  Bool selected(Int ifNum);
  void listScan(Double & mjd, Int scan, Double ut);
  Bool checkCABB(const String & rpfitsname);
  void rfiFlag(Matrix<Bool> & flags);
  

  String atcaPosToStation(Vector<Double>& xyz);
  void flush();
  void unlock();

  void shadow(Int row, Bool last=False);

  // Constants
  // MaxNPol needs to agree with if_cstok in RPFITS.h
  enum{MaxNChan=16385, MaxNPol=4};
  // Data
  MeasurementSet atms_p;
  MSColumns *msc_p;

  // Filenames
  Vector<String> rpfitsFiles_p;
  Int opcor_p;
  String currentFile_p;

  // The following should be constant throughout the rpfits file
  Int nAnt_p; 

  // Number of scans seen; #SpWs, #fields stored sofar
  Int nScan_p, nSpW_p, nField_p;
  // #scanheaders stored, index into MS SpW and Field Tables for current data
  Int scanNo_p, spWId_p, fieldId_p, prev_fieldId_p, obsId_p;

  // Bools
  Bool gotAN_p; //have we got an antenna Table yet?
  Bool appendMode_p;
  Bool storedHeader_p;
  Bool skipScan_p;
  Bool skipData_p;
  Bool firstHeader_p;
  Bool listHeader_p;
  uInt fileSize_p;
  Bool eof_p;
  Bool birdie_p;   // flag birdie channels
  Bool reweight_p; // gibbs reweighting
  Bool noxycorr_p; // do not apply xy phase correction
  Bool noac_p;    // Don't load the autocorrelation data
  Int obsType_p; // the type of observation: 0= standard, 1= fastmosaic
  Bool hires_p; // transform binned data into high time res data
  Bool init_p; // Are we initialized yet?
  Bool cabb_p; // Are we reading CABB data?

  // rpfits data
  Float vis[2*MaxNPol*MaxNChan];
  Float weight[MaxNPol*MaxNChan];
  Int baseline, flg, bin, if_no, sourceno; //index into rpfits Table(not MS) 
  Float ut, u, v, w;

  // storage manager accessor
  TiledDataStManAccessor dataAccessor_p,sigmaAccessor_p,flagAccessor_p,
    flagCatAccessor_p;
  TiledDataStManAccessor modelDataAccessor_p,corrDataAccessor_p,imWtAccessor_p;

  // Column objects to access Tables

  // colXXX objects are ATCA specific columns in the MeasurementSet 
  ScalarColumn<Int> colSysCalIdAnt1, colSysCalIdAnt2;
  ScalarColumn<Float> colXYAmplitude,colTrackErrMax,colTrackErrRMS,
      colWeatherSeeMonPhase,colWeatherSeeMonRMS,colWeatherRainGauge;
  ScalarColumn<Int> colSamplerBits;
  ArrayColumn<Float> colSamplerStatsNeg, colSamplerStatsZero,
      colSamplerStatsPos;
  ArrayColumn<Float> colGTP, colSDO, colCalJy;
  ScalarColumn<Bool> colWeatherSeeMonFlag;
  // ATCA_SCAN_INFO columns & table
  ScalarColumn<Int> colScanInfoAntId, colScanInfoScanId, colScanInfoSpWId,
       colScanInfoCacal;
  ArrayColumn<Int> colScanInfoFine, colScanInfoCoarse, colScanInfommAtt;
  ArrayColumn<Float> colScanInfoSubreflector;
  ScalarColumn<String> colScanInfoCorrConfig, colScanInfoScanType,
      colScanInfoCoordType,colScanInfoPointInfo;
  ScalarColumn<Bool> colScanInfoLineMode;
  Table msScanInfo_p;
  Matrix<Float> pointingCorr_p;
  Bool newPointingCorr_p;

  // reference date
  Double mjd0_p;

  // variables to keep the state of the sysCal search & binning state
  Vector<Int> sysCalId_p;
  Float lastUT_p;
  Int lastSpWId_p;
  Bool gotSysCalId_p;
  Float lastWeatherUT_p;
  Int errCount_p;

  // Selection parameters
  Vector<String> fieldSelection_p;
  Double lowFreq_p, highFreq_p;
  Vector<Double> freqs_p;
  Double windowWidth_p;
  Int firstScan_p, lastScan_p;
  Int firstChan_p, lastChan_p;
  Double firstTime_p, lastTime_p;
  Vector<Int> spws_p;
  Int bandWidth1_p, numChan1_p;
  Vector<Int> baselines_p, antennas_p;

  // Track sources
  String sources_p;
  Int nsources_p;
  
  // Polarization order
  Matrix<Int> corrIndex_p;

  // Check for shadowing
  Float shadow_p;
  Float edge_p;
  Block<Int> rowCache_p;
  Int nRowCache_p;
  Double prevTime_p;
  
  // Flagging
  Bool autoFlag_p,flagScanType_p;
  enum {COUNT=0, FLAG, ONLINE, SCANTYPE, SYSCAL, SHADOW, NFLAG};
  Vector<Int> flagCount_p;

  // Logger
  LogIO os_p;
      
};

} //# NAMESPACE CASA - END


#endif
