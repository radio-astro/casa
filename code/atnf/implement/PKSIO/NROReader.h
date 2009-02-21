//#---------------------------------------------------------------------------
//# NROReader.h: Base class to read NRO 45m and ASTE data.
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
//# Original: 2008/10/30, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef NRO_READER_H
#define NRO_READER_H

#include <string>
#include <stdio.h>
#include <vector>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>

#include <casa/namespace.h>

#include <atnf/PKSIO/NRODataset.h>
#include <atnf/PKSIO/NROHeader.h>

#define SCAN_HEADER_SIZE 424 

using namespace std ;

// <summary>
// Base class to read NRO 45m and ASTE data.
// </summary>

enum ENDIAN { BIG,
              LITTLE,
              UNKNOWN } ;

// Open an appropriate NROreader for a NRO 45m and ASTE dataset.
class NROReader *getNROReader( const String filename,
                               String &datatype );

// As above, but search a list of directories for it.
class NROReader *getNROReader( const String filename,
                               const Vector<String> directories,
                               int &iDir,
                               String &datatype ) ;

// 
// NROReader
// 
class NROReader
{
 public:
  // Constructor 
  NROReader( string name ) ;

  // Destructor.
  virtual ~NROReader() ;

  // Read data header
  virtual Int readHeader() = 0 ;

  // Read the next data record.
  Int readData( int i ) ;

  // Open the dataset.
  int open() ;

  // Close the input file.
  void close() ;

  // Get data header
  virtual NROHeader *getHeader() { return header_ ; } ;

  // Get scan data
  virtual NRODataset *getData() { return data_ ; } ;
  virtual int getData( int i ) ;
 
  // Get number of scan 
  Int getScanNum() { return scanNum_ ; } ;

  // Get length of scan
  Int getScanLen() { return scanLen_ ; } ;

  // Get number of data record
  Int getRowNum() { return rowNum_ ; } ;

  // Get number of channel
  Int getChannelMax() { return chmax_ ; } ;
 
  // Get spectrum
  vector< vector<double> > getSpectrum() ;
  vector<double> getSpectrum( int i ) ;

  // Get number of polarization
  Int getPolarizationNum() ;

  // Get MJD time
  double getStartTime() ;
  double getEndTime() ;
  vector<double> getStartIntTime() ;
  double getStartIntTime( int i ) ;

  // Get Antenna Position in ITRF coordinate
  virtual vector<double> getAntennaPosition() = 0 ;

  // Get Frequency Settings
  vector<double> getFrequencies( int i ) ;

  // Get IF settings
  vector<Bool> getIFs() ;

  // Get Beam settings
  vector<Bool> getBeams() ;

  // Get SRCDIRECTION in RADEC(J2000)
  Vector<Double> getSourceDirection() ;

  // Get DIRECTION in RADEC(J2000)
  Vector<Double> getDirection( int i ) ;

 protected:
  // convert time in character representation to MJD representation
  double getMJD( char *time ) ;

  // Get integer representation of ARRYT
  Int getIndex( int irow ) ;

  // convert Endian
  void convertEndian( int &value ) ;
  void convertEndian( float &value ) ;
  void convertEndian( double &value ) ;
  void convertEndian( NRODataset *d ) ;

  // memory management
  void releaseData() ;

  // filename 
  string filename_ ;

  // file pointer 
  FILE *fp_ ;

  // header
  NROHeader *header_ ;

  // data
  NRODataset *data_ ;

  // data index
  Int dataid_ ;

  // number of scan
  Int scanNum_ ;

  // length of data record (byte)
  Int scanLen_ ;

  // number of data record
  Int rowNum_ ;

  // endian of the system
  ENDIAN endian_ ;

  // check if endian of input file is same as that of system
  bool same_ ;

  // channel number for spectral data
  Int chmax_ ;
};

#endif /* NRO_READER_H */
