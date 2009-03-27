//# VlaTapeSource.cc:
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

#include <nrao/VLA/VlaTapeSource.h>
#include <string.h> // for memcpy

const Int PHYS_RECORD_SIZE = 26624;

struct TapeHeader{      // Each VLA tape record has this tape header 
  Short current;
  Short total;
};

VlaTapeSource::VlaTapeSource(const String& a)
  :itsTape(new VLATapeIO(a))
{
}

VlaTapeSource::~VlaTapeSource() {
  delete itsTape;
}

Int VlaTapeSource::open(const String& a) {
  return itsTape->open(a, O_RDONLY, 0);
}

Int VlaTapeSource::next(Char* data) {
  TapeHeader* tapeHead;
  Int LRSize = 0;
  
  Char* buffer = new Char[PHYS_RECORD_SIZE];
  Int bytes_read = itsTape->read(buffer, PHYS_RECORD_SIZE);
  if (bytes_read >= 2048){
    memcpy((Char *) &LRSize, buffer+sizeof(TapeHeader), sizeof(LRSize));
    LRSize *= 2;
    tapeHead = (TapeHeader *) buffer;
    
    // int bufferAllocation = (LRSize > 2048) ? LRSize + 2048 : 2048;
    
    memcpy(data, buffer+sizeof(TapeHeader), bytes_read - sizeof(TapeHeader));
    
    //      cerr << " Logical Record Size: " << LRSize << " bytes. ";
    
    Int BeePtr = 0;
    Int NumPhysRecs = tapeHead->total;
    Int en = 4;
    
    if (tapeHead->total > 1) {
      for (Int i=2; i<=NumPhysRecs; i++) {
	BeePtr += (PHYS_RECORD_SIZE - en);
	bytes_read = itsTape->read(buffer, PHYS_RECORD_SIZE);
	memcpy(data+BeePtr, buffer + en, bytes_read - en);
      }
    }
  } else {
    LRSize = bytes_read;
  }
  return LRSize;
}

Int VlaTapeSource::close() {
  return itsTape->close();
}
// Local Variables: 
// compile-command: "gmake OPTLIB=1 VlaTapeSource"
// End: 
