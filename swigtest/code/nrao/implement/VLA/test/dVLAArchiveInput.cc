//# dVLAArchiveInput.cc:
//# Copyright (C) 1999,2000,2001,2002
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
#include <iomanip.h>

#include <casa/IO/ByteIO.h>
#include <casa/IO/RegularFileIO.h>
#include <casa/Inputs.h>
#include <casa/OS/File.h>
#include <casa/OS/SymLink.h>
#include <casa/IO/FilebufIO.h>
#include <casa/IO/FiledesIO.h>
#include <nrao/VLA/VLAArchiveInput.h>
#include <nrao/VLA/VLADiskInput.h>
#include <casa/IO/TapeIO.h>
#include <casa/OS/Timer.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <casa/namespace.h>

int main(int argc, char* argv[]) {
  try {
    Input inputs(1);
    /*    inputs.create ("input", "/home/bonzo4/rmarson/file1.dat",
		   "Input Archive ie., a filename");
    inputs.readArguments (argc, argv);

    File input;
    {
      const Path name = inputs.getString("input");
      AlwaysAssert(name.isValid(), AipsError);
      input = File(name);
      AlwaysAssert(input.exists(), AipsError);
    }
    VLAArchiveInput *myrecord;
    if (input.isSymLink()) {
      SymLink link(input);
      const Path name = link.followSymLink();
      AlwaysAssert(name.isValid(), AipsError);
      input = File(name);
      AlwaysAssert(input.exists(), AipsError);
      AlwaysAssert(input.isSymLink() == False, AipsError);
       myrecord = new VLADiskInput(name);
    }      
    int fd = -1;
    ByteIO* ioPtr = 0;
    */
/*
    Lose tape forever

    if (input.isRegular()) {
      //ioPtr = new RegularFileIO(RegularFile(input));
//       fd = open(input.path().absoluteName().chars(), O_RDONLY);
//       ioPtr = new FiledesIO(fd);
    } else if (input.isCharacterSpecial()) {
      fd = TapeIO::open(input.path().absoluteName());
      ioPtr = new TapeIO(fd);
    }
*/

/*    Timer clock;
    uInt recordsRead = 0;
    uInt bytesRead = myrecord->length();
    while (!myrecord->eof()) {
      if (!myrecord->isValid()) {
 	cout << "Record " << recordsRead << " is invalid" << endl;
      }
      bytesRead += myrecord->length();
      recordsRead++; (*myrecord)++;
    }
    Double elapsed = clock.real();
    cout << setprecision(4) 
	 << "Read " << bytesRead/1024.0/1024.0 << " Mbytes ("
	 << recordsRead << " records) in " << elapsed << " secs." 
	 << " Data rate is " << bytesRead/elapsed/1024.0/1024.0 
	 << " Mbytes/sec" << endl;
    if (input.isCharacterSpecial()) {
      TapeIO::close(fd);
    }
    if (myrecord->eof()) cout << "Stopped by the end of file" << endl;
    if (myrecord->lost()) cout << "Stopped because we are lost" << endl;
    if (myrecord->badData()) cout << "Stopped as we detected bad data" <<endl;
    if (myrecord->noData()) cout << "Stopped as we detected no data" << endl;
    AlwaysAssert(myrecord->eof() == True, AipsError);
*/
  }
  catch (AipsError x) {
    cerr << x.getMesg() << endl;
    cout << "FAIL" << endl;
    return 1;
  } 
  cout << "OK" << endl;
}

// Local Variables: 
// compile-command: "gmake OPTLIB=1 dVLAArchiveInput"
// End: 
