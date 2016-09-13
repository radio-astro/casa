//# tVLATapeInput.cc:
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

// #include <casa/IO/ByteIO.h>
// #include <casa/IO/RegularFileIO.h>
#include <casa/Inputs.h>
#include <casa/OS/Path.h>
// #include <casa/OS/File.h>
// #include <casa/OS/SymLink.h>
// #include <casa/OS/RegularFile.h>

#include <nrao/VLA/VLATapeInput.h>
// #include <nrao/VLA/VLARCA.h>
// #include <casa/IO/TapeIO.h>
// #include <casa/Arrays/Vector.h>
// #include <casa/Arrays/ArrayLogical.h>
// #include <casa/Containers/Block.h>

#include <casa/namespace.h>

int main(int argc, char* argv[]) {
  try {
    Input inputs(1);
    inputs.create ("input", "/dev/tape",
		   "Input tape device");
    inputs.create ("file", "0",
		   "file to read");
    inputs.readArguments (argc, argv);
    
//     File input;
//     {
    const Path name = inputs.getString("input");
    AlwaysAssert(name.isValid(), AipsError);
//       input = File(name);
//       AlwaysAssert(input.exists(), AipsError);
//     }
//     if (input.isSymLink()) {
//       SymLink link(input);
//       const Path name = link.followSymLink();
//       AlwaysAssert(name.isValid(), AipsError);
//       input = File(name);
//       AlwaysAssert(input.exists(), AipsError);
//       AlwaysAssert(input.isSymLink() == False, AipsError);
//     }      
//     int fd;
//     ByteIO* ioPtr = 0;
    {
      Block<Int> bi = inputs.getIntArray("file");
      Block<uInt> bui(bi.nelements());
      for (uInt i = 0; i < bi.nelements(); i++) {
	bui[i] = bi[i];
	cout << "Will read file " << bui[i] << endl;
      }
      VLATapeInput in(name, bui);
      uInt i;
      //     const uInt nComp=2000;
      uInt prevIndex = 0;
      while (in.read()) {
 	ByteSource& record = in.logicalRecord();
        /*
	if (prevIndex != in.fileIndex()) {
	  i = 1;
	  prevIndex = in.fileIndex();
	}
        i*/
 	cout << "Record " << i << " contains " 
 	     << record.typeIO().byteIO().length() << " bytes"
	  //   << " in tape file " << in.tapeFile()
	     << endl;
	i++;
      }
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
// compile-command: "gmake OPTLIB=1 tVLATapeInput"
// End: 
