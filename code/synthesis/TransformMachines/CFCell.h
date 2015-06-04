// -*- C++ -*-
//# CFCell.h: Definition of the CFCell class
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
#ifndef SYNTHESIS_CFCELL_H
#define SYNTHESIS_CFCELL_H
#include <synthesis/TransformMachines/CFDefs.h>
#include <synthesis/TransformMachines/SynthesisError.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/CountedPtr.h>
#include <images/Images/ImageInterface.h>
#include <msvis/MSVis/VisBuffer.h>
//
// <summary> 
//
// A light-weight container to carray all the information required for
// a single convolution function plan.
//
//</summary>

// <prerequisite>
// </prerequisite>
//
// <etymology> 
//
//  CFCell is basic in-memory representation of a single
//  monochromatic, single polarization cross-product and single
//  w-plane convoluion function .
//
//</etymology>
//
// <synopsis> 
//
// Mostly a conveniance class to pass around related information.  
//
//</synopsis>
//
// <example>
// </example>
//
// <motivation>
//
// To avoid parameter-bloat to passing around a number of related objects seperately.
//
// </motivation>
//

namespace casa { //# NAMESPACE CASA - BEGIN
  typedef Complex TT;

  struct  CFCStruct{
    CoordinateSystem coordSys;
    TT * CFCStorage;
    Int shape[2];
    Float sampling,diameter;
    Int xSupport, ySupport;
    Double wValue, freqValue,freqIncr, conjFreq;    
    Int muellerElement, conjPoln;
    String fileName, telescopeName;
  };

  using namespace CFDefs;
  using namespace std;
  //  template <class T>
  class CFCell
  {
  void initCFCStruct(CFCStruct& cfcSt) 
  {
    cfcSt.CFCStorage=NULL;
    cfcSt.xSupport = cfcSt.ySupport=0;
    cfcSt.sampling=0.0;
    cfcSt.shape[0]=cfcSt.shape[1]=0;
  }

  public:
    //
    //========================= Administrative Parts ==========================
    //------------------------------------------------------------------
    //
    CFCell():cfShape_p(){};

    CFCell(Array<TT> &dataPtr, CoordinateSystem& cs, Float& /*samp*/)
    {
      if (storage_p.null()) storage_p = new Array<TT>(dataPtr);
      coordSys_p = cs;
      cfShape_p.assign(storage_p->shape().asVector());
    };

    ~CFCell() 
    {
      if (!storage_p.null()) 
	{
	  //cerr << "############### " << "~CFCell() called " << storage_p->shape() << endl;
	  storage_p->resize();
	}
    };

    void getAsStruct(CFCStruct& cfst) 
    {
      Bool dummy;
      cfst.CFCStorage = getStorage()->getStorage(dummy);
      cfst.coordSys = coordSys_p;
      cfst.shape[0]=cfShape_p[0];
      cfst.shape[1]=cfShape_p[1];
      cfst.sampling=sampling_p;
      cfst.xSupport=xSupport_p;
      cfst.ySupport=ySupport_p;
      cfst.wValue=wValue_p;
      cfst.freqValue=freqValue_p;
      cfst.freqIncr=freqIncr_p;
      cfst.muellerElement=muellerElement_p;
      cfst.conjFreq = conjFreq_p;
      cfst.conjPoln = conjPoln_p;
      cfst.diameter=diameter_p;
      cfst.fileName = fileName_p;
      cfst.telescopeName=telescopeName_p;
    }
    CountedPtr<Array<TT> >& getStorage() {return storage_p;}
    void makePersistent(const char *dir, const char *cfName="");
    CountedPtr<CFCell> clone();
    void setParams(const CFCell& other);
    void initCache(const Bool& releaseSpace=False) {shape_p=getShape(); cfShape_p.assign(shape_p.asVector());IPosition tt=shape_p;tt=0;tt[0]=tt[1]=0;if (releaseSpace) storage_p->resize(tt);};
    IPosition getShape() {return storage_p->shape();}
    //
    //============================= Functional Parts ============================
    //------------------------------------------------------------------
    //
    void show(const char *Mesg,ostream &os);

    IPosition shape_p;
    CountedPtr<Array<TT> > storage_p; // Nx x Ny
    CoordinateSystem coordSys_p;
    Float sampling_p, diameter_p;
    Int xSupport_p,ySupport_p, conjPoln_p;
    Double wValue_p, freqValue_p,freqIncr_p, conjFreq_p;
    //    MuellerElementType muellerElement_p;
    Int muellerElement_p;
    Quantity pa_p;
    Vector<Int> cfShape_p;
    String fileName_p,telescopeName_p;
  };
} //# NAMESPACE CASA - END
#endif
