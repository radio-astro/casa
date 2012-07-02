//# VisBuffGroupAcc.cc: Implementation of VisBuffGroupAcc.h
//# Copyright (C) 2008
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
//# $Id: VisBuffAccumulator.cc,v 19.7 2006/01/17 08:22:27 gvandiep Exp $
//----------------------------------------------------------------------------

#include <msvis/MSVis/VisBuffGroupAcc.h>
#include <casa/Exceptions/Error.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------
  
VisBuffGroupAcc::VisBuffGroupAcc(const Int& nAnt, 
				 const Int& nSpw,
				 const Int& nFld,
				 const Double& subinterval)
  : nAnt_p(nAnt), 
    nSpw_p(nSpw),
    nFld_p(nFld),
    nBuf_p(0),
    subinterval_p(subinterval), 
    prenorm_p(False),
    globalTimeStamp_p(0.0),
    VBA_p(),
    spwfldids_p(nSpw,nFld,-1)
{

  // Interval MUST be strictly greater than zero
  if (subinterval_p < DBL_EPSILON)
    subinterval_p=0.1;   // TBD: is this reasonable? 

  // NB: Enforcing no prenormalization here, for now


};

//----------------------------------------------------------------------------

VisBuffGroupAcc::~VisBuffGroupAcc()
{
// Null default destructor
//

  // Delete all VBAs.
  // TBD: encapsulate this for more general use?
  for (Int i=0;i<nBuf_p;++i)
    if (VBA_p[i]) delete VBA_p[i];

  VBA_p.resize(0);

};

//----------------------------------------------------------------------------

void VisBuffGroupAcc::accumulate (const VisBuffer& vb)
{

  Int spw=vb.spectralWindow();
  Int fld=vb.fieldId();
  Int ibuf=spwfldids_p(spw,fld);

  // Create the new VisBuffAccumulator, if needed
  if (ibuf<0) {
    ibuf=nBuf_p;
    ++nBuf_p;
    VBA_p.resize(nBuf_p,False,True);
    spwfldids_p(spw,fld)=ibuf;
    VBA_p[ibuf]= new VisBuffAccumulator(nAnt_p,subinterval_p,prenorm_p);
  }

    // ibuf should be non-negative now
    if (ibuf<0) 
      throw(AipsError("VisBuffGroupAcc: VisBuffAccumulator index failure."));


  // Accumulate the vb into the correct accumulator
  VBA_p[ibuf]->accumulate(vb);

};

//----------------------------------------------------------------------------

void VisBuffGroupAcc::finalizeAverage()
{
  globalTimeStamp_p=0.0;
  Double globalTimeStampWt(0.0);
  Double t0(-1.0);  // avoid round-off problems in time average

  for (Int ibuf=0;ibuf<nBuf_p;++ibuf) {
    // tell a VBA to finalize itself
    VBA_p[ibuf]->finalizeAverage();

    // Accumulate weighted timestamps
    Double& thistimewt(VBA_p[ibuf]->timeStampWt());
    if (thistimewt>0.0) {
      globalTimeStampWt+=(thistimewt);
      Double& thistime(VBA_p[ibuf]->timeStamp());
      if (t0<0.0) 
	t0=thistime;
      else
	globalTimeStamp_p+=(thistimewt*(thistime-t0));
    }
  }

  // Form global timestamp
  if (globalTimeStampWt>0.0)
    globalTimeStamp_p/=globalTimeStampWt;

  // Add offset back in!
  globalTimeStamp_p+=t0;

  // NB: the per-VBA timestamp weights are approximately (exactly?)
  //  the relative weights of the _data_ going into the solution.
  //  This could be a useful log message?....

};
  
void VisBuffGroupAcc::enforceAPonData(const String& apmode)
{

  // Delegate to each CalVisBuffer in turn
  for (Int ibuf=0;ibuf<nBuf_p;++ibuf)
    if (VBA_p[ibuf])
      this->operator()(ibuf).enforceAPonData(apmode);

}


CalVisBuffer& VisBuffGroupAcc::operator()(const Int& buf) 
{
  if (buf > -1 && buf < nBuf_p)
    return VBA_p[buf]->aveCalVisBuff();
  else
    throw(AipsError("VisBuffGroupAcc: operator(buf) index out-of-range."));
}

CalVisBuffer& VisBuffGroupAcc::operator()(const Int& spw,const Int& fld) 
{
  if (spw>-1 && fld > -1 && spwfldids_p(spw,fld) > 0)
    return this->operator()(spwfldids_p(spw,fld));
  else
    throw(AipsError("VisBuffGroupAcc: operator(spw,fld) index out-of-range."));
}



//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

  



} //# NAMESPACE CASA - END

