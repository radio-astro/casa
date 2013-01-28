//#---------------------------------------------------------------------------
//# NRODataset.h: Base class for NRO dataset.
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
//#---------------------------------------------------------------------------
//# Original: 2009/02/27, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef NRO_DATASET_H
#define NRO_DATASET_H

#include <string> 
#include <stdio.h>
#include <vector> 
#include <iostream>

//#include <casa/aips.h>
#include <casa/Logging/LogIO.h>
#include <casa/Containers/Record.h>
#include <atnf/PKSIO/NRODataRecord.h>
#include <casa/namespace.h>
#include <casa/iomanip.h>

#define SCAN_HEADER_SIZE 424 

// <summary>
// Base class for NRO accessor classes.
// </summary>
//
// <prerequisite>
//   <li> <linkto class=NROReader>NROReader</linkto>
//   <li> <linkto class=NRODataRecord>NRODataRecord</linkto>
// </prerequisite>
//
// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>
//
// <etymology>
// This class is a base class for classes that actually access data from NRO telescopes.
// Concrete classes are defiened for each data type (OTF format or NRO FITS) and/or 
// telescopes (45m or ASTE).
// The class have two filler method: fillHeader and fillRecord. The former reads header 
// information from the data. Since header data depends on the telescope and its configuration, 
// it is an abstract in this class and is defined in each concrete class. 
// On the other hand, the later reads each scan record (set of meta data 
// and spectral data). The method uses <linkto class=NRODataRecord>NRODataRecord</linkto> 
// to access scan record. It is implemented here since contents of scan record is 
// quite similar for several types of data.
// </etymology>
//
// <synopsis>
// Abstract class that is designed as a base class for all accessor classes.
// </synopsis>
//

class NRODataset
{
 public:
  // Constructor 
  NRODataset( std::string name ) ;

  // Destructor 
  virtual ~NRODataset() ;

  // Data initialization
  virtual void initialize() ;

  // open file
  virtual int open() ;

  // close file
  virtual void close() ;

  // Fill data header from file
  virtual int fillHeader() = 0 ;

  // Fill data record
  virtual int fillRecord( int i ) ;

  // simple getter
  std::string getLOFIL() { return LOFIL ; } ;
  std::string getVER() { return VER ; } ;
  std::string getGROUP() { return GROUP ; } ;
  std::string getPROJ() { return PROJ ; } ;
  std::string getSCHED() { return SCHED ; } ;
  std::string getOBSVR() { return OBSVR ; } ;
  std::string getLOSTM() { return LOSTM ; } ;
  std::string getLOETM() { return LOETM ; } ;
  int getARYNM() { return ARYNM ; } ;
  int getNSCAN() { return NSCAN ; } ;
  std::string getTITLE() { return TITLE ; } ;
  std::string getOBJ() { return OBJ ; } ;
  std::string getEPOCH() { return EPOCH ; } ;
  double getRA0() { return RA0 ; } ;
  double getDEC0() { return DEC0 ; } ;
  double getGLNG0() { return GLNG0 ; } ;
  double getGLAT0() { return GLAT0 ; } ;
  int getNCALB() { return NCALB ; } ;
  int getSCNCD() { return SCNCD ; } ;
  std::string getSCMOD() { return SCMOD ; } ;
  double getURVEL() { return URVEL ; } ;
  std::string getVREF() { return VREF ; } ;
  std::string getVDEF() { return VDEF ; } ;
  std::string getSWMOD() { return SWMOD ; } ;
  double getFRQSW() { return FRQSW ; } ;   
  double getDBEAM() { return DBEAM ; } ;   
  double getMLTOF() { return MLTOF ; } ;    
  double getCMTQ() { return CMTQ ; } ;     
  double getCMTE() { return CMTE ; } ;
  double getCMTSOM() { return CMTSOM ; } ;
  double getCMTNODE() { return CMTNODE ; } ;
  double getCMTI() { return CMTI ; } ;
  std::string getCMTTM() { return CMTTM ; } ;
  double getSBDX() { return SBDX ; } ;
  double getSBDY() { return SBDY ; } ;
  double getSBDZ1() { return SBDZ1 ; } ;
  double getSBDZ2() { return SBDZ2 ; } ;
  double getDAZP() { return DAZP ; } ;
  double getDELP() { return DELP ; } ;
  int getCHBIND() { return CHBIND ; } ;
  int getNUMCH() { return NUMCH ; } ;
  int getCHMIN() { return CHMIN ; } ;
  int getCHMAX() { return CHMAX ; } ;
  double getALCTM() { return ALCTM ; } ;
  double getIPTIM() { return IPTIM ; } ;
  double getPA() { return PA ; } ;
  int getSCNLEN() { return SCNLEN ; } ;
  int getSBIND() { return SBIND ; } ;
  int getIBIT() { return IBIT ; } ;
  std::string getSITE() { return SITE ; } ;
  std::vector<std::string> getRX() { return RX ; } ;
  std::vector<double> getHPBW() { return HPBW ; } ;
  std::vector<double> getEFFA() { return EFFA ; } ;
  std::vector<double> getEFFB() { return EFFB ; } ;
  std::vector<double> getEFFL() { return EFFL ; } ;
  std::vector<double> getEFSS() { return EFSS ; } ;
  std::vector<double> getGAIN() { return GAIN ; } ;
  std::vector<std::string> getHORN() { return HORN ; } ;
  std::vector<std::string> getPOLTP() { return POLTP ; } ;
  std::vector<double> getPOLDR() { return POLDR ; } ;
  std::vector<double> getPOLAN() { return POLAN ; } ;
  std::vector<double> getDFRQ() { return DFRQ ; } ;
  std::vector<std::string> getSIDBD() { return SIDBD ; } ;
  std::vector<int> getREFN() { return REFN ; } ;
  std::vector<int> getIPINT() { return IPINT ; } ;
  std::vector<int> getMULTN() { return MULTN ; } ;
  std::vector<double> getMLTSCF() { return MLTSCF ; } ;
  std::vector<std::string> getLAGWIND() { return LAGWIND ; } ;
  std::vector<double> getBEBW() { return BEBW ; } ;
  std::vector<double> getBERES() { return BERES ; } ;
  std::vector<double> getCHWID() { return CHWID ; } ;
  std::vector<int> getARRY() { return ARRY ; } ;
  std::vector<int> getNFCAL() { return NFCAL ; } ;
  std::vector<double> getF0CAL() { return F0CAL ; } ;
  std::vector< std::vector<double> > getFQCAL() { return FQCAL ; } ;
  std::vector< std::vector<double> > getCHCAL() { return CHCAL ; } ;
  std::vector< std::vector<double> > getCWCAL() { return CWCAL ; } ;
  std::string getCDMY1() { return CDMY1 ; } ;
  std::vector<double> getDSBFC() { return DSBFC ;} ;
  int getDataSize() { return datasize_ ; } ;
  int getRowNum() { return rowNum_ ; } ;

  // get various parameters
  NRODataRecord *getRecord( int i ) ;
  virtual std::vector< std::vector<double> > getSpectrum() ;
  virtual std::vector<double> getSpectrum( int i ) ;
  virtual int getIndex( int irow ) ;
  virtual int getPolarizationNum() ;
  virtual std::vector<double> getStartIntTime() ;
  virtual double getStartIntTime( int i ) ;
  virtual double getScanTime( int i ) ;
  virtual double getMJD( char *time ) ;
  virtual std::vector<bool> getIFs() ;
  virtual std::vector<double> getFrequencies( int i ) ;
  virtual uInt getArrayId( std::string type ) ;
  virtual uInt getPolNo( int irow ) ;

 protected:
  // fill header information
  virtual int fillHeader( int sameEndian ) = 0 ;

  // Endian conversion for int variable
  void convertEndian( int &value ) ;

  // Endian convertion for float variable
  void convertEndian( float &value ) ;

  // Endian conversion for double variable
  void convertEndian( double &value ) ;

  // Endian conversion for NRODataRecord
  void convertEndian( NRODataRecord *r ) ;

  // Read char data
  int readHeader( char *v, int size ) ;

  // Read int data
  int readHeader( int &v, int b ) ;

  // Read float data
  int readHeader( float &v, int b ) ;

  // Read double data
  int readHeader( double &v, int b ) ;

  // Release DataRecord
  void releaseRecord() ;

  // show primary information
  void show() ;

  // convert frequency frame
  virtual double toLSR( double v, double t, double x, double y ) ;

  // POLNO from RX
  //uInt polNoFromRX( const char *rx ) ;
  uInt polNoFromRX( const std::string &rx ) ;

  // Type of file record
  std::string LOFIL ;

  // Version 
  std::string VER ;

  // Group name
  std::string GROUP ;

  // Project name
  std::string PROJ ;

  // Name of observation scheduling file
  std::string SCHED ;

  // Name of observer
  std::string OBSVR ;

  // Observation start time with format of "YYYYMMDDHHMMSS" (UTC)
  std::string LOSTM ;

  // observation end time with format of "YYYYMMDDHHMMSS" (UTC)
  std::string LOETM ;

  // Number of arrays (beams and IFs)
  int ARYNM ;

  // Number of scans
  int NSCAN ;

  // Title of observation
  std::string TITLE ;

  // Name of target object 
  std::string OBJ ;

  // Equinox (B1950 or J2000)
  std::string EPOCH ;

  // Right ascension [rad]
  double RA0 ;

  // Declination [rad]
  double DEC0 ;

  // Galactic longitude [rad]
  double GLNG0 ;

  // Galactic latitude [rad]
  double GLAT0 ;

  // Calibration interval
  int NCALB ;

  // Scan coordinate  (0: RADEC  1: LB  2: AZEL)
  int SCNCD ;

  // Scan sequence pattern
  std::string SCMOD ;

  // User-defined recessional velocity [m/s]
  double URVEL ;

  // Reference frame for recessional velocity  (LSR or HEL or GAL)
  std::string VREF ;

  // Definition of recessional velocity  (RAD or OPT)
  std::string VDEF ;

  // Switching mode  (POS or BEAM or FREQ)
  std::string SWMOD ;

  // Switching frequency [Hz]
  double FRQSW ;

  // Off-beam angle of beam switching [rad]
  double DBEAM ;

  // Initial inclination angle of multi-beam array 
  double MLTOF ;

  // Comet: Perihelion distance
  double CMTQ ;

  // Comet: Eccentricity
  double CMTE ;

  // Comet: Argument of perihelion
  double CMTSOM ;

  // Comet: Longitude of the ascending node 
  double CMTNODE ;

  // Comet: Orbital inclination angle
  double CMTI ;

  // Comet: Time of the perihelion passage
  std::string CMTTM ;

  // Correction for position of subreflector DX [mm] 
  double SBDX ;

  // Correction for position of subreflector DY [mm] 
  double SBDY ;

  // Correction for position of subreflector DZ1 [mm] 
  double SBDZ1 ;

  // Correction for position of subreflector DZ2 [mm] 
  double SBDZ2 ;

  // Correction for pointing on azimuth [rad]
  double DAZP ;

  // Correction for pointing on elevation [rad]
  double DELP ;

  // Number of channel binding  
  int CHBIND ;

  // Number of channel after binding
  int NUMCH ;

  // Channel range (minimum)
  int CHMIN ;

  // Channel range (maximum)
  int CHMAX ;

  // ALC time constant
  double ALCTM ;

  // Interval to get data from spectrometer
  double IPTIM ;

  // Position angle of the map
  double PA ;

  // Length of scan record [bytes]
  int SCNLEN ;

  // Range of space binding
  int SBIND ;

  // Quantization bit number (fixed to 12)
  int IBIT ;

  // Site (antenna) name  (45m or ASTE)
  std::string SITE ;

  // Dummy data
  std::string CDMY1 ;

  // Type of detector frontend 
  std::vector<std::string> RX ;

  // HPBW [rad]
  std::vector<double> HPBW ;

  // Aperture efficiencies 
  std::vector<double> EFFA ;

  // Beam efficiencies 
  std::vector<double> EFFB ;

  // Antenna efficiencies 
  std::vector<double> EFFL ;

  // FSS efficiencies 
  std::vector<double> EFSS ;

  // Antenna gain 
  std::vector<double> GAIN ;

  // Type of polarization at feed horn  (R or L or H or V)
  std::vector<std::string> HORN ;

  // Type of polarization  (CIRC or LINR)
  std::vector<std::string> POLTP ;

  // Rotation direction of circular polarization
  std::vector<double> POLDR ;

  // Polarization angle of linear polarization
  std::vector<double> POLAN ;

  // Switching frequency of frequcency switching [Hz]
  std::vector<double> DFRQ ;

  // Type of sideband  (LSB or USB or DSB)
  std::vector<std::string> SIDBD ;

  // Identifier of reference synthesizer
  std::vector<int> REFN ;

  // Temperature of calibrator
  std::vector<int> IPINT ;

  // Beam id of the multi-beam detector
  std::vector<int> MULTN ;

  // Scaling factor of the multi-beam detector
  std::vector<double> MLTSCF ;

  // Type of LAG window  (NONE or HANN or HAMM or BLCK)
  std::vector<std::string> LAGWIND ;

  // Bandwidth at backend
  std::vector<double> BEBW ;

  // Spectral resolution at backend
  std::vector<double> BERES ;

  // Channel width at backend
  std::vector<double> CHWID ;

  // Array usage  (1: used  0: not used)
  std::vector<int> ARRY ;

  // Frequency calibration: Number of measurement (max 10)
  std::vector<int> NFCAL ;

  // Frequency calibration: Central frequency [Hz]
  std::vector<double> F0CAL ;

  // Frequency calibration: Measured central frequency [Hz]
  std::vector< std::vector<double> > FQCAL ;

  // Frequency calibration: Measured channel number
  std::vector< std::vector<double> > CHCAL ;

  // Frequency calibration: Measured channel width [Hz]
  std::vector< std::vector<double> > CWCAL ;

  // DSB scaling factor
  std::vector<double> DSBFC ;

  // number of scan
  int scanNum_ ;

  // number of row
  int rowNum_ ;

  // length of scan (byte)
  int scanLen_ ;

  // length of spectral data (byte)
  int dataLen_ ;

  // Data size of the header [bytes]
  int datasize_ ;

  // maximum channel number
  int chmax_ ;

  // Current data id
  int dataid_ ;

  // Data record
  NRODataRecord *record_ ;

  // input filename
  std::string filename_ ;

  // file pointer
  FILE *fp_ ;

  // OS endian
  int same_ ;

  // Logger
  //LogIO os ;

  // reference frequency for each array
  std::vector<double> refFreq_ ;

  // record to store REFPIX, REFVAL, INCREMENT pair for each array
  Record frec_ ;
} ;

// debug message output
template<class T> inline void nro_debug_output( char *name, int len, std::vector<T> &val ) 
{
  for ( int i = 0 ; i < len ; i++ ) {
    if ( i == 0 ) {
      cout << setw(8) << left << name ;
    }
    else if ( ( i % 5 ) == 0 ) {
      cout << endl << "        " ;
    }
    cout << "\'" << val[i] << "\' " ;
  }
  cout << endl ;
} 

template<class T> inline void nro_debug_output( char *name, int len1, int len2, std::vector< std::vector<T> > &val ) 
{
  for ( int i = 0 ; i < len1 ; i++ ) {
    for ( int j = 0 ; j < len2 ; j++ ) {
      if ( j == 0 ) {
        if ( i < 10 ) 
          cout << name << "0" << i << " " ;
        else 
          cout << name << i << " " ;
      }
      else if ( ( j % 5 ) == 0 ) {
        cout << endl << "        " ;
      }
      cout << "\'" << val[i][j] << "\' " ;
    }
    cout << endl ;
  }
}


#endif /* NRO_HEADER_H */
