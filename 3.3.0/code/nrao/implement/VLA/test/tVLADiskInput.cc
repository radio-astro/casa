//# tVLADiskInput.cc: 
//# Copyright (C) 1999,2000,2001
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

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>
#include <casa/iostream.h>

#include <casa/Inputs.h>
#include <casa/OS/Path.h>

#include <nrao/VLA/VLADiskInput.h>

#include <casa/namespace.h>

int main(int argc, char* argv[]) {
  try {
    Input inputs(1);
    inputs.create ("input", "file1.vla",
		   "Input disk file");
    inputs.readArguments (argc, argv);
 
    const Path name = inputs.getString("input");
    AlwaysAssert(name.isValid(), AipsError);
    VLADiskInput in(name);
    uInt i = 0;
    while (in.read()) {
      ByteSource& record = in.logicalRecord();
      cout << "Record " << i << " contains " 
	   << record.typeIO().byteIO().length() << " bytes"
	   << endl;
      i++; 
    }
  }
  catch (AipsError x) {
    cerr << x.getMesg() << endl;
    cout << "FAIL" << endl;
    return 1;
  } 
  cout << "OK" << endl;
}

// Local Variables: 
// compile-command: "gmake OPTLIB=1 tVLADiskInput"
// End: 
