// -*- C++ -*-
//# CFStore2.h: Definition of the CFStore2 class
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
#ifndef SYNTHESIS_CFSTORE2_H
#define SYNTHESIS_CFSTORE2_H
#include <synthesis/TransformMachines/CFDefs.h>
#include <synthesis/TransformMachines/CFBuffer.h>
#include <synthesis/TransformMachines/CFCell.h>
#include <synthesis/TransformMachines/VBStore.h>
#include <synthesis/TransformMachines/SynthesisError.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Containers/OrderedMap.h>
#include <casa/Utilities/CountedPtr.h>
#include <images/Images/ImageInterface.h>
#include <synthesis/MSVis/VisBuffer.h>
namespace casa { //# NAMESPACE CASA - BEGIN
  using namespace CFDefs;
  typedef Cube<CountedPtr<CFCell > > VBRow2CFMapType;
  typedef Vector<CountedPtr<CFBuffer > > VBRow2CFBMapType;
  class CFStore2
  {
  public:
    CFStore2():storage_p(), pa_p(), mosPointingPos_p(0) {};

    // CFStore2(CFBuffer<Complex> *dataPtr, Quantity PA, Int mosPointing):
    //   storage_p(), pa_p(PA), mosPointingPos_p(mosPointing)
    // {storage_p = new CFBuffer<Complex>(*dataPtr);};

    virtual ~CFStore2() {};

    CFStore2& operator=(const CFStore2& other);
    //-------------------------------------------------------------------------
    void show(const char *Mesg=NULL,ostream &os=cerr);
    //-------------------------------------------------------------------------
    void makePersistent(const char *dir,const char *qualifier="");
    //-------------------------------------------------------------------------
    void primeTheCFB();
    //-------------------------------------------------------------------------
    void initMaps(const VisBuffer& vb, const Matrix<Double>& freqSelection, const Double& imRefFreq);
    //-------------------------------------------------------------------------
    void initPolMaps(PolMapType& polMap, PolMapType& conjPolMap);
    //-------------------------------------------------------------------------
    Bool null() {return (storage_p.size() == 0);};
    //-------------------------------------------------------------------------
    Double memUsage();
    //-------------------------------------------------------------------------
    void set(const CFStore2& other)
    {
      pa_p.assign(other.pa_p);
      ant1_p.assign(other.ant1_p);
      ant2_p.assign(other.ant2_p);
    }
    //-------------------------------------------------------------------------
    void setCFBuffer(CFBuffer *dataPtr, Quantity pa, 
		     const Int& ant1, const Int& ant2);
    //-------------------------------------------------------------------------
    CountedPtr<CFBuffer>& getCFBuffer(const Quantity& pa, 
				      const Quantity& paTol, 
				      const Int& ant1, const Int& ant2);
    //-------------------------------------------------------------------------
    // Get CFBuffer by directly indexing in the list of CFBuffers
    CountedPtr<CFBuffer>& getCFBuffer(const Int& paNdx, const Int& antNdx);
    void getParams(Quantity& pa, 
		   Int& ant1, Int& ant2, 
		   const Int& paNdx, const Int& antNdx);
    //-------------------------------------------------------------------------
    //
    // Generate a map for the given frequency and Mueller element list
    // to the index in the internal list of CFs.  This can be used in
    // tight loops to get get direct access to the required CF.
    //
    /*
    void makeVBRow2CFMap(VBRow2CFMapType& vbRow2CFMap,
			 const VisBuffer& vb, 
			 const Quantity& paTol,
			 const Vector<Int>& dataChan2ImChanMap,
			 const Vector<Int>& dataPol2ImPolMap);
    */
    //-------------------------------------------------------------------------
    Vector<Int> resize(const Quantity& pa, const Quantity& paTol, 
		       const Int& ant1,const Int& ant2, Bool retainValues=True);
    // void rememberATerm(CountedPtr<ATerm>& aTerm) {theATermUsed_p = aTerm;}

    // Int mapAntIDToAntType(const Int& ant) {return theATermUsed_p->mapAntIDToAntType(ant);};
    Matrix<CountedPtr<CFBuffer> >& getStorage() {return storage_p;}
    Vector<Int>& getAnt1List() {return ant1_p;};
    Vector<Int>& getAnt2List() {return ant2_p;};
    Vector<Quantity> getPAList() {return pa_p;};
    IPosition getShape() {return storage_p.shape();}

  protected:



    Matrix<CountedPtr<CFBuffer > > storage_p;
    Vector<Int> ant1_p, ant2_p;
    Vector<Quantity> pa_p;
    Int mosPointingPos_p;

    virtual void getIndex(const Quantity& pa, 
			  const Quantity& paTol, 
			  const Int& ant1, const Int& ant2,
			  Int& paNdx, Int& antNdx)
    {
      paNdx  = paHashFunction(pa,paTol);
      antNdx = antHashFunction(ant1,ant2);
    }

    // virtual Int wHashFunction(const Double& wValue)
    // {
    //   Int ndx=-1;
    //   for(uInt i=0;i<wValue_p.nelements(); i++)
    // 	if (wValue_p[i] == wValue)
    // 	  {ndx=i;break;}
    //   return ndx;
    // }
    virtual Int antHashFunction(const Int& ant1, const Int& ant2)
    {
      Int ndx=-1;
      for (uInt i=0;i<ant1_p.nelements(); i++)
	if ((ant1_p[i]==ant1) && (ant2_p[i]==ant2))
	  {ndx=i;break;}
      return ndx;
    };

    virtual Int paHashFunction(const Quantity& pa,const Quantity& paTol)
    {
      // for (uInt i=0; i<pa_p.nelements(); i++)
      // 	if ( pa_p[i] == pa)
      // 	  {ndx=i;break;}
      return nearestPA(pa, paTol);
    }

    virtual Int nearestPA(const Quantity& pa, const Quantity& paTol);
  };
} //# NAMESPACE CASA - END

#endif
