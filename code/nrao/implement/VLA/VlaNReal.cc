//# VlaNReal.cc:
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
//# $Id: VlaNReal.cc,v 19.0.40.1 2006/02/13 02:07:03 wyoung Exp $

#include <nrao/VLA/VlaNReal.h>
#include <iostream>

// kludge for vla network until we implement multicast. 
// These functions are defined in nreal.cc
int olopen_(int *){ return -1;}
int olclose_(int *);
Int nrtread_(Int *, Char *) { std::cerr << "Wrong call nrtread_, use olread_" << std::endl; return -1;}

VlaNReal::VlaNReal()
  :itsTapeId(-99)
{
  itsTapeId = olopen_(&itsTapeId);
}

// VlaNReal::VlaNReal(const VlaNReal& other)
//   :itsTapeId(other.itsTapeId)
// {
// }

VlaNReal::~VlaNReal() {
  if (itsTapeId != -99) olclose_(&itsTapeId);
}

// VlaNReal& VlaNReal::operator=(const VlaNReal& other) {
//   if (this != &other) {
//     if (itsTapeId != -99) olclose_(&itsTapeId);
//     itsTapeId = other.itsTapeId;
//   }
//   return *this;
// }

Int VlaNReal::read(Char* buf) {
  return nrtread_(&itsTapeId, buf);
}

Int VlaNReal::open(const String& a) {
  return itsTapeId = olopen_(&itsTapeId);
}

Int VlaNReal::close() {
  return olclose_(&itsTapeId);
}
// Local Variables: 
// compile-command: "gmake OPTLIB=1 VlaNReal"
// End: 
