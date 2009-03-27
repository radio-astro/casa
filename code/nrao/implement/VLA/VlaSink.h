//# VlaSink.h: this defines VlaSink, which ...
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

#ifndef NRAO_VLASINK_H
#define NRAO_VLASINK_H

#include <casa/aips.h>
#include <nrao/VLA/VlaDAs.h>
#include <nrao/VLA/VlaSelectInfo.h>

#include <casa/namespace.h>
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
} //# NAMESPACE CASA - END


// <summary></summary>

// <use visibility=export>

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

class VlaSink
{
public:
  VlaSink();
  virtual ~VlaSink();

  // Returns the number of bytes written,  if bytes < 0 then 
  // the time is past that of the stop time.
  Int write(const Char* buffer);
  
  // Set the constraints on what is written out
  virtual void setConstraints(const String& filename);

  // Given the logical record load the data into VLA specific structures
  // Probably should be protected but made public just in case.
  void fillFromBuffer(const Char* buffer);

protected:
  // Writes the visibility record, depends on class type
  virtual Int writeVisibilityRecord(const Char* buffer) = 0;
  
  // If the record is OK returns 1, -1 if past stop time, 0 otherwise
  Int ok();
  
  void allocateFlags();
  
  // These routines provide the modcomp to IEEE conversion
  static uShort vlaCompNibble2uShort(const Char* s, uInt nibble);
  static Char   vlaByte2Char(const Char* s);
  static uChar  vlaByte2uChar(const Char* s);
  static Short  vlaByte2Short(const Char* s);
  static uShort vlaByte2uShort(const Char* s);
  static uShort vlaI22uShort(const Char* s);
  static Short  vlaI22Short(const Char* s);
  static Int    vlaI42Int(const Char* s);
  static uInt   vlaI42uInt(const Char* s);
  static String vlaAS2String(const Char* s, uInt nBytes);
  static Double vlaDP2Double(const Char* s);
  static Float  vlaFP2Float(const Char* s);
  
  // A VLA visibility record is broken down into it's various parts, while
  // strictly speaking these aren't necessary for a MS filler they can
  // provide a bit of clarity for the maintainer. 
  AntennaDataArea     ada[27];
  SubarrayDataArea    sda;
  RecordControlArea   rca;

  // cda depends on what type of experiment we''re are loading, either
  // spectral-line or continuum
  CorrelatorDataArea* cda[4];
  VlaSelectInfo       dataSelectionCriteria;
  Int*                antIds;
  Int*                writeAntennae;

  // Flags are provided for convience not needed for Measurement Set sink
  RCAFlags*             rcaFlags;
  SDAFlags*             sdaFlags;
  ADAFlags*             adaFlags;
  ContinuumCDAFlags*    ccdaFlags;
  SpectralLineCDAFlags* slcdaFlags;

private:
  Int  getCDAOffset(Int vlaIF);
  Int  getCDAHeader(Int vlaIF);
  Int  getCDAData(Int vlaIF);

  void assignADA(AntennaDataArea& ada, const Char* buffer);
  void assignContinuum(CorrelatorDataArea& cda, const Char* buffer);
  Int  assignContinuumDatum(VLAContinuumDatum& datum, const Char* buffer);
  void assignSpectralLine(CorrelatorDataArea& cda, const Char* buffer,
			  const Int sizeBRH, const Int sizeBR);

  Bool isContinuum();
  Bool isHolography();

  void setFlags(RCAFlags& rca, Char* buffer);
  void setFlags(SDAFlags& sda, Char* buffer);
  void setFlags(ADAFlags& ada, Char* buffer);
  void setFlags(ContinuumCDAFlags& cda, Char* buffer);
  void setFlags(SpectralLineCDAFlags& cda, Char* buffer);
  
  void setFlags(RCAFlags& rca, Bool flag);
  void setFlags(SDAFlags& rca, Bool flag);
  void setFlags(ADAFlags& rca, Bool flag);
  void setFlags(ContinuumCDAFlags& rca, Bool flag);
  void setFlags(SpectralLineCDAFlags& rca, Bool flag);
  
  void initFlags(RCAFlags& rca, Bool flag);
  void initFlags(SDAFlags& sda, Bool flag);
  void initFlags(ADAFlags& ada, Bool flag);
  void initFlags(ContinuumCDAFlags& cda, Bool flag);
  void initFlags(SpectralLineCDAFlags& cda, Bool flag);
  
};

#endif
