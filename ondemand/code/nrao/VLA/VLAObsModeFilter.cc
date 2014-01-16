//# VLAObsModeFilter.cc:
//# Copyright (C) 1999,2000
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

#include <nrao/VLA/VLAObsModeFilter.h>
#include <nrao/VLA/VLALogicalRecord.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/ArrayLogical.h>


VLAObsModeFilter::VLAObsModeFilter()
  :itsObsModes()
{
  DebugAssert(ok(), AipsError);

  // Permit only sensible modes for now
  itsObsModes.resize(9);
  itsObsModes(0)="  ";   // standard
  itsObsModes(1)="H ";   // holography
  itsObsModes(2)="S ";   // solar
  itsObsModes(3)="SP";   // solar (low accuracy)
  itsObsModes(4)="VA";   // self-phasing (AD)
  itsObsModes(5)="VB";   // self-phasing (BC)
  itsObsModes(6)="VL";   // self-phasing (CD)
  itsObsModes(7)="VR";   // self-phasing (AB)
  itsObsModes(8)="VX";   // phasing from prior scan

}

VLAObsModeFilter::VLAObsModeFilter(const Vector<String>& obsModes) 
  :itsObsModes(obsModes)
{
  DebugAssert(ok(), AipsError);
}

VLAObsModeFilter::VLAObsModeFilter(const VLAObsModeFilter& other) 
  : VLAFilter(),
  itsObsModes()
{
  DebugAssert(ok(), AipsError);
  itsObsModes.resize();
  itsObsModes = other.itsObsModes;
}

VLAObsModeFilter::~VLAObsModeFilter() {
  DebugAssert(ok(), AipsError);
}

VLAObsModeFilter& VLAObsModeFilter::
operator=(const VLAObsModeFilter& other) {
  if (this != &other) {
    itsObsModes.resize();
    itsObsModes=other.itsObsModes;
  }
  DebugAssert(ok(), AipsError);
  return *this;
}

Bool VLAObsModeFilter::passThru(const VLALogicalRecord& record) const {
  //  cerr << "This record obs mode: |" << record.SDA().obsMode()
  //       << "| \tallowed obs modes: |" << itsObsModes;
  if (itsObsModes.nelements()==0 ||
      anyEQ(itsObsModes,record.SDA().obsMode()) ) {
    //    cerr << " match" << endl;
    return True;
  } else {
    //    cerr << " NO match" << endl;
    return False;
  }
}

VLAFilter* VLAObsModeFilter::clone() const {
  DebugAssert(ok(), AipsError);
  VLAObsModeFilter* tmpPtr = new VLAObsModeFilter(*this);
  AlwaysAssert(tmpPtr != 0, AipsError);
  return tmpPtr;
}

Bool VLAObsModeFilter::ok() const {
  // The LogIO class is only constructed if an error is detected for
  // performance reasons. Both function static and file static variables
  // where considered and rejected for this purpose.
  if (!VLAFilter::ok()) return False; 
  return True;
}
// Local Variables: 
// compile-command: "gmake VLAObsModeFilter"
// End: 
