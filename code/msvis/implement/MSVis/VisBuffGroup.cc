//# VisBuffGroup.cc: Implementation of VisBuffGroup.h
//# Copyright (C) 2011
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
//----------------------------------------------------------------------------

#include <msvis/MSVis/VisBuffGroup.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>

namespace casa { //# NAMESPACE CASA - BEGIN

VisBuffGroup::VisBuffGroup() :
    nBuf_p(0),
    VB_p()
{
  endChunk_p.resize(0);
}

VisBuffGroup::~VisBuffGroup()
{
  // Null default destructor

  // Delete all VBs.
  for(uInt i = 0; i < nBuf_p; ++i)
    if(VB_p[i])
      delete VB_p[i];

  VB_p.resize(0);
}

Bool VisBuffGroup::store(const VisBuffer& vb)
{
  // Int spw=vb.spectralWindow();
  // Int fld=vb.fieldId();
  // Int ibuf=spwfldids_p(spw,fld);

  LogIO os(LogOrigin("VisBuffGroup", "store"));
  uInt ibuf = nBuf_p;
  Bool retval = False;
  
  try{
    VB_p.resize(nBuf_p + 1, False, True); // n, forceSmaller, copyElements
    endChunk_p.resize(nBuf_p + 1, True);   // n, copyElements
    VB_p[ibuf] = new VisBuffer(vb);  // The copy is detached from the VisIter.
    endChunk_p[ibuf] = False;           // until notified otherwise.
    ++nBuf_p;
    retval = True;
  }
  catch(AipsError x){
    os << LogIO::SEVERE
       << "Error " << x.getMesg() << " storing a VisBuffer."
       << LogIO::POST;
  }
  catch(...){
    os << LogIO::SEVERE
       << "Unknown exception caught while storing a VisBuffer."
       << LogIO::POST;
  }
  return retval;
}

void VisBuffGroup::endChunk()
{
  endChunk_p[nBuf_p - 1] = True;
}

VisBuffer& VisBuffGroup::operator()(const Int buf) 
{
  if(buf < 0 || buf >= static_cast<Int>(nBuf_p))
    throw(AipsError("VisBuffGroup: operator(buf) index out of range."));

  return *(VB_p[buf]);
}

} //# NAMESPACE CASA - END

