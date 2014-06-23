// -*- C++ -*-
//# CFStore2.cc: Implementation of the CFStore2 class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
#include <synthesis/TransformMachines/CFStore2.h>
#include <synthesis/TransformMachines/CFBuffer.h>
#include <synthesis/TransformMachines/CFCell.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/SynthesisError.h>
#include <synthesis/TransformMachines/PolOuterProduct.h>

namespace casa{
  //
  //---------------------------------------------------------------
  //
  CFStore2 & CFStore2::operator=(const CFStore2& other)
  {
    if (&other != this)
      {
	storage_p.assign(other.storage_p); 
	ant1_p.assign(other.ant1_p);
	ant2_p.assign(other.ant2_p);
	pa_p.assign(other.pa_p);
      }
    return *this;
  };
  //
  //---------------------------------------------------------------
  //
  void CFStore2::show(const char *Mesg, ostream& os)
  {
    if (!null())
      {
	if (Mesg != NULL) os << Mesg << endl;
	os << "Data Shape: " << storage_p.shape() << endl;
	os << "Ant1: " << ant1_p << endl;
	os << "Ant2: " << ant2_p << endl;
	os << "PA = "; for (uInt i=0;i<pa_p.nelements();i++)
			 os << pa_p[i].get("deg") << endl;
      }
  };
  //
  //---------------------------------------------------------------
  //
  Vector<Int> CFStore2::resize(const Quantity& pa,
			       const Quantity& paTol,
			       const Int& ant1, const Int& ant2,
			       Bool retainValues)
  {
    Int paNdx, antNdx;
    //    getIndex(pa,wValue,ant1,ant2,paNdx, antNdx, wNdx);
    getIndex(pa,paTol,ant1,ant2,paNdx, antNdx);

    // If a index along any axis is negative, resize the correponding
    // vector holding the values along that axis.
    if (paNdx < 0) 
      {paNdx=pa_p.nelements();pa_p.resize(paNdx+1,retainValues);};
    if (antNdx < 0) 
      {antNdx=ant1_p.nelements();ant1_p.resize(antNdx+1,True);ant2_p.resize(antNdx+1,retainValues);}; 
    // if (wNdx < 0) 
    //   {wNdx=wValue_p.nelements();wValue_p.resize(wNdx+1,retainValues);}
    // Resize the storage_p Cube to the current size of required along
    // the 3 axis.  This is a NoOp if the axis lengths did not change.
    storage_p.resize(pa_p.nelements(), ant1_p.nelements(), retainValues);
    
    pa_p[paNdx] = pa;
    ant1_p[antNdx] = ant1;
    ant2_p[antNdx] = ant2;

    Vector<Int> pos(2);pos(0)=paNdx;pos(1)=antNdx;
    if (storage_p(pos(0),pos(1)).null())
      storage_p(pos(0),pos(1)) = new CFBuffer;
    return pos;
  };
  //
  //---------------------------------------------------------------
  //
  void CFStore2::setCFBuffer(CFBuffer* /*dataPtr*/, 
			     Quantity /*pa*/, 
			     const Int& /*ant1*/, const Int& /*ant2*/)
  {
    throw(AipsError("setCFBuffer called!"));
    // Vector<Int> pos=resize(pa,ant1,ant2);
    // storage_p(pos[0], pos[1]) = dataPtr;
  }
  //
  //---------------------------------------------------------------
  //
  CountedPtr<CFBuffer>& CFStore2::getCFBuffer(const Quantity& pa, 
					      const Quantity& paTol, 
					      const Int& ant1, const Int& ant2)
  {
    Int paNdx, antNdx;
    getIndex(pa,paTol,ant1,ant2,paNdx, antNdx);
    if ((paNdx < 0) || (antNdx < 0)) 
	throw(CFNotCached("CFStore2::getIndex: Index not found"));
      
    return getCFBuffer(paNdx, antNdx);
  }
  //
  //---------------------------------------------------------------
  //
  // Get CFBuffer by direct indexing
  CountedPtr<CFBuffer>& CFStore2::getCFBuffer(const Int& paNdx, const Int& antNdx)
  {
    return storage_p(paNdx, antNdx);
  }
  //
  //---------------------------------------------------------------
  //
  void CFStore2::getParams(Quantity& pa,
			   Int& ant1, Int& ant2, 
			   const Int& paNdx, const Int& antNdx)
  {
    pa     = pa_p[paNdx];
    ant1   = ant1_p[antNdx];
    ant2   = ant2_p[antNdx];
  }
  //
  //---------------------------------------------------------------
  //
  void CFStore2::makePersistent(const char *dir,const char *qualifier)
  {
    for (Int i=0;i<storage_p.shape()(0);i++)
      for (Int j=0;j<storage_p.shape()(1);j++)
	{
	  ostringstream name;
	  name << dir << "/" << qualifier << "CFS_" << i << "_" << j;
	  storage_p(i,j)->makePersistent(name.str().c_str());
	}
  }
  //
  //---------------------------------------------------------------
  //
  void CFStore2::primeTheCFB()
  {
    IPosition shp=getShape();
    for (Int i=0; i<shp(0); i++)
      for (Int j=0; j<shp(1); j++)
	getCFBuffer(i,j)->primeTheCache();
  }
  //
  //---------------------------------------------------------------
  //
  void CFStore2::initMaps(const VisBuffer& vb, const Matrix<Double>& freqSelection, const Double& imRefFreq)
  {
    for (Int i=0;i<storage_p.shape()(0);i++)
      for (Int j=0;j<storage_p.shape()(1);j++)
	storage_p(i,j)->initMaps(vb, freqSelection, imRefFreq);
  }
  //
  //---------------------------------------------------------------
  //
  void CFStore2::initPolMaps(PolMapType& polMap, PolMapType& conjPolMap)
  {
    IPosition shp=getShape();
    for (Int i=0; i<shp(0); i++)
      for (Int j=0; j<shp(1); j++)
	getCFBuffer(i,j)->initPolMaps(polMap, conjPolMap);
  }
  //
  //---------------------------------------------------------------
  //
  Double CFStore2::memUsage()
  {
    IPosition cfsShape = getStorage().shape();
    Double memUsed=0;
    for (Int ib=0;ib<cfsShape(0); ib++)
      for(int it=0;it<cfsShape(1); it++)
	{
	  IPosition cfbShape = getStorage()(ib,it)->getStorage().shape();
	  for(int ip=0;ip<cfbShape(2); ip++)
	    for(Int ich=0;ich<cfbShape(0);ich++)
	      for(Int iw=0;iw<cfbShape(1); iw++)
		memUsed += getStorage()(ib,it)->getStorage()(ich,iw,ip)->getStorage()->shape()
		  .product()*sizeof(Complex);
	}
    return memUsed;
  }
  //
  //---------------------------------------------------------------
  //
  Int CFStore2::nearestPA(const Quantity& pa, const Quantity& paTol)
    {
      Int n=pa_p.nelements();
      Float dpa=paTol.getValue("rad"),
	paVal = pa.getValue("rad");
      for(Int i=0;i<n;i++)
	{
	  if (fabs(pa_p(i).getValue("rad") - paVal) < dpa)
	  return i;
	}
      return -1;
    }
}; // end casa namespace



