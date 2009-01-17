//# VlaAsciiSink.h:
//# Copyright (C) 1997,1999,2001
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

#ifndef NRAO_VLAASCIISINK_H
#define NRAO_VLAASCIISINK_H

#include <casa/aips.h>
#include <nrao/VLA/VlaSink.h>
#include <casa/iosfwd.h>

#include <casa/namespace.h>
//# Forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
} //# NAMESPACE CASA - END

struct RCAFlags;
struct SDAFlags;
struct AntennaDataArea;
struct ADAFlags;
struct ContinuumCDAFlags;
struct SpectralLineCDAFlags;

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


class VlaAsciiSink :public VlaSink {
public:
  VlaAsciiSink();
  VlaAsciiSink(const String& a); 
  ~VlaAsciiSink();

  Int writeVisibilityRecord(const Char* buffer);
      
private:
  //# private and inaccessible
  VlaAsciiSink(const VlaAsciiSink& other);
  VlaAsciiSink & operator=(const VlaAsciiSink& other);

  Int writeAscii(ostream& oss, const RCAFlags* flags);
  Int writeAscii(ostream& oss, const SDAFlags* flags);
  Int writeAscii(ostream& oss, const AntennaDataArea& ada,
		 const ADAFlags* flags);
  Int writeAscii(ostream& oss, const ContinuumCDAFlags* flags);
  Int writeAscii(ostream& oss, const SpectralLineCDAFlags* flags);
};
#endif
