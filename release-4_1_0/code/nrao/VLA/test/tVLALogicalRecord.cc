//# tVLALogicalRecord.cc: 
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
#include <casa/OS/File.h>
#include <casa/OS/SymLink.h>
#include <nrao/VLA/VLALogicalRecord.h>
#include <nrao/VLA/VLATapeInput.h>
#include <nrao/VLA/VLADiskInput.h>
#include <nrao/VLA/VLARCA.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Containers/Block.h>

#include <casa/namespace.h>

void print(VLALogicalRecord& record) {
  static uInt i = 0;
  i++;
  cout << "Record " << i << " contains " 
       << record.RCA().length() << " bytes in an array with "
       << record.RCA().nAntennas() << " antennas." 
       << endl;
  cout << "Source Name: " << record.SDA().sourceName()
       << " Project Name: " << record.SDA().obsId()
       << endl;
  const uInt nant = record.RCA().nAntennas();
  cout << "Antennae involved are";
  for (uInt k = 0; k < nant; k++) {
    cout << ":" << record.ADA(k).antId();
  }
  cout << endl;
  for (uInt k = 0; k < 2; k++) {
    if (!record.CDA(k).isValid()) {
      cout << "CDA[" << k << "] is not valid" << endl;
    } else {
      cout << "CDA[" << k << "]  contains " 
	   << record.SDA().nChannels((VLAEnum::CDA)k) << " channels & "
	   << record.SDA().npol((VLAEnum::CDA)k) << " polarisations"
	   << endl;
    }
  }
}

int main(int argc, char* argv[]) {
  try {
    Input inputs(1);
    inputs.create ("input", "/home/bonzo4/rmarson/file2.dat",
		   "Input Archive ie., a filename");
    inputs.create ("files", "0",
		   "tape files to read  0 -> next file");
    inputs.readArguments (argc, argv);

    VLALogicalRecord record;
    {
      Path name = inputs.getString("input");
      AlwaysAssert(name.isValid(), AipsError);
      File input(name);
      cout << "name is " << inputs.getString("input") << endl;
      AlwaysAssert(input.exists(), AipsError);
      if (input.isSymLink()) {
	SymLink link(input);
	name = link.followSymLink();
	AlwaysAssert(name.isValid(), AipsError);
	input = File(name);
	AlwaysAssert(input.exists(), AipsError);
	AlwaysAssert(input.isSymLink() == False, AipsError);
      }      
      if (input.isRegular()) {
	record = VLALogicalRecord(new VLADiskInput(name));
      } else if (input.isCharacterSpecial()) {
	Block<uInt> files;
	{
	  Block<Int> f = inputs.getIntArray("files");
	  files.resize(f.nelements());
	  for (uInt i = 0; i < f.nelements(); i++) {
	    AlwaysAssert(f[i] >= 0, AipsError);
	    files[i] = f[i];
	  }
	}
	cout << Vector<uInt>(files) << endl;
	record = VLALogicalRecord(new VLATapeInput(name, files));
      }
    }
    uInt i = 1;
    const uInt nComp=2000;
    //    record.read();
    while ( record.read() && record.isValid() ) {
     print(record);
      if (i < 40 && record.RCA().length() > nComp*sizeof(Int)) {
	Block<Int> currentData(nComp);
	record.logicalRecord().seek(0);
	record.logicalRecord().read(nComp, currentData.storage());
	VLALogicalRecord copy(record);
	AlwaysAssert(copy.RCA().length() == record.RCA().length(),AipsError);
	Block<Int> newData(nComp);
	copy.logicalRecord().seek(0);
	copy.logicalRecord().read(nComp, newData.storage());
	{
	  Vector<Int> data(currentData), reference(newData);
	  AlwaysAssert(allEQ(data,reference),AipsError);
	}
	copy.read(); record.read();
	print(record);
	AlwaysAssert(copy.RCA().length() == record.RCA().length(),
		     AipsError);
	copy.logicalRecord().seek(0);
	copy.logicalRecord().read(nComp, newData.storage());
	record.logicalRecord().seek(0);
	record.logicalRecord().read(nComp, currentData.storage());
	{
	  Vector<Int> data(currentData), reference(newData);
	  AlwaysAssert(allEQ(data,reference),AipsError);
	}
      }
      i++; 
      //record.read();
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
// compile-command: "gmake OPTLIB=1 tVLALogicalRecord"
// End: 
