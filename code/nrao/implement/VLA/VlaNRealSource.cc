//# VlaNRealSource.cc:
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
//# $Id$

#include <nrao/VLA/VlaNRealSource.h>

// VlaNRealSource::VlaNRealSource()
//   :itsTape(0)
// {
// }

// VlaNRealSource::VlaNRealSource(const VlaNRealSource& other)
//   :itsTape(new VlaNReal(*(other.itsTape)))
// {
// }

VlaNRealSource::VlaNRealSource(const String& a)
  :itsTape(new VlaNReal)
{
}

VlaNRealSource::~VlaNRealSource() {
  delete itsTape;
}

// VlaNRealSource & VlaNRealSource::operator=(const VlaNRealSource & other) {
//   if (this != &other) {
//     delete itsTape;
//     itsTape = new VlaNReal(*(other.itsTape));
//   }
//   return *this;
// }

Int VlaNRealSource::next(Char* buffer) {
  return itsTape->read(buffer);
}

Int VlaNRealSource::open(const String& a) {
  return itsTape->open(a);
}

Int VlaNRealSource::close() {
  return itsTape->close();
}
// Local Variables: 
// compile-command: "gmake OPTLIB=1 VlaNRealSource"
// End: 
