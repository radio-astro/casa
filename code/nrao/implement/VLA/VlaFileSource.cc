//# VlaFileSource.cc:
//# Copyright (C) 1997,1999,2001,2002
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

#include <nrao/VLA/VlaFileSource.h>
#include <casa/BasicSL/String.h>
#include <casa/fstream.h>
#include <string.h> // for memcpy

const Int PHYS_RECORD_SIZE = 26624;

struct TapeHeader{      // Each VLA tape record has this tape header 
  Short current;
  Short total;
};

VlaFileSource::VlaFileSource(const String& a)
  :itsTape(new ifstream(a.chars()))
{
}

VlaFileSource::~VlaFileSource() {
  delete itsTape;
}

Int VlaFileSource::next(Char* data)
{
   TapeHeader tapeHead;
   Int LRSize = 0;
 
   itsTape->read((Char *) &tapeHead, sizeof(TapeHeader));
   if(!itsTape->eof()){
     itsTape->read((Char *) &LRSize, sizeof(LRSize));
     // int bufferAllocation = (2*LRSize > 4096) ? 2*LRSize + 2048 : 4096;
     memcpy(data, &LRSize, sizeof(LRSize));
 
     Int BeePtr(sizeof(LRSize));
     Int NumPhysRecs(tapeHead.total);
     Int en = 8;
   
     if (tapeHead.total > 1) {
       if (tapeHead.current == 1) {
	 itsTape->read(data+BeePtr, PHYS_RECORD_SIZE-en);
	 BeePtr = PHYS_RECORD_SIZE - 4;
       }
       en = 4;
       for (Int i=2; i<NumPhysRecs; i++) {
	 itsTape->read( (Char *) &tapeHead, sizeof(TapeHeader));
	 itsTape->read(data+BeePtr, PHYS_RECORD_SIZE - en);
	 BeePtr += (PHYS_RECORD_SIZE - en);
       }
       en = 4;
       itsTape->read( (Char *) &tapeHead, sizeof(TapeHeader));
     }
     Int Filler(2048*(1+((2*LRSize)%(PHYS_RECORD_SIZE))/2048) - en);
     itsTape->read(data+BeePtr, Filler); 
   }
   return LRSize;
}

Int VlaFileSource::open(const String& a) {
  itsTape = new ifstream(a.chars());
  return 0;
}

Int VlaFileSource::close() {
  itsTape->close();
  return 0;
}
// Local Variables: 
// compile-command: "gmake OPTLIB=1 VlaFileSource"
// End: 
