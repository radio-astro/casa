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
#include <casa/Logging/LogIO.h>

//#include <fitsio.h>

#include <casa/namespace.h>

#include <atnf/PKSIO/NRODataset.h>
#include <atnf/PKSIO/NRODataRecord.h>

using namespace std ;

// <summary>
// Base class to read NRO 45m and ASTE data.
// </summary>
// <use visibility=global>
//
// <prerequisite>
//   <li> <linkto class=NRODataset>NRODataset</linkto>
// </prerequisite>
//
// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>
//
// <etymology>
// <linkto class=NROReader>NROReader</linkto> is a base class for all NRO reader classes.
// Reader classes have <linkto class=NRODataset>NRODataset</linkto> object to access 
// actual data. These classes are a kind of interface for Scantable data filler. 
// </etymology>
//
// <synopsis>
// Abstract class that is designed as a base class for NRO reader classes. 
// </synopsis>
//

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
  virtual Int read() = 0 ;

  // Get header information
  virtual int getHeaderInfo( Int &nchan,
                             Int &npol,
                             Int &nif,
                             Int &nbeam,
                             String &observer,
                             String &project,
                             String &obstype,
                             String &antname,
                             Vector<Double> &antpos,
                             Float &equinox,
                             String &freqref,
                             Double &reffreq,
                             Double &bw,
                             Double &utc,
                             String &fluxunit,
                             String &epoch,
                             String &poltype ) ;
                         
  // Get scan information
  virtual int getScanInfo( int irow,
                           uInt &scanno,
                           uInt &cycleno,
                           uInt &beamno,
                           uInt &polno,
                           vector<double> &freqs,   
                           Vector<Double> &restfreq,  
                           uInt &refbeamno,
                           Double &scantime,
                           Double &interval,
                           String &srcname,
                           String &fieldname,
                           Array<Float> &spectra,
                           Array<uChar> &flagtra,
                           Array<Float> &tsys,
                           Array<Double> &direction,
                           Float &azimuth,
                           Float &elevation,
                           Float &parangle,
                           Float &opacity,
                           uInt &tcalid,
                           Int &fitid,
                           uInt &focusid,
                           Float &temperature,  
                           Float &pressure,     
                           Float &humidity,     
                           Float &windvel,      
                           Float &winddir,      
                           Double &srcvel,
                           Array<Double> &propermotion,
                           Vector<Double> &srcdir,
                           Array<Double> &scanrate ) ;

  // Get scan type
  virtual string getScanType( int i ) ;

  // Get dataset
  virtual NRODataset *getDataset() { return dataset_ ; } ;

  // Get number of rows
  virtual Int getRowNum() ;

  // Get IF settings
  virtual vector<Bool> getIFs() ;

  // Get Beam settings
  virtual vector<Bool> getBeams() ;

 protected:
  // convert time in character representation to MJD representation
  virtual double getMJD( char *time ) ;
  virtual double getMJD( string strStartTime ) ;

  // Get spectrum
  virtual vector< vector<double> > getSpectrum() ;

  // Get number of polarization
  virtual Int getPolarizationNum() ;

  // Get MJD time
  virtual double getStartTime() ;
  virtual double getEndTime() ;
  virtual vector<double> getStartIntTime() ;
  //virtual double getStartIntTime( int i ) ;

  // Get Antenna Position in ITRF coordinate
  virtual vector<double> getAntennaPosition() = 0 ;

  // Get SRCDIRECTION in RADEC(J2000)
  virtual Vector<Double> getSourceDirection() ;

  // Get DIRECTION in RADEC(J2000)
  virtual Vector<Double> getDirection( int i ) ;

  // filename 
  string filename_ ;

  // dataset
  NRODataset *dataset_ ;

  // Logger
  //LogIO os ;
};

#endif /* NRO_READER_H */
