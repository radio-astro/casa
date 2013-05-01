//# LFBase.h: A lighter flagger - for autoflag
//# Copyright (C) 2000,2001
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
//# $Jan 28 2011 rurvashi Id$
#ifndef FLAGGING_LFBASE_H
#define FLAGGING_LFBASE_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <synthesis/MSVis/VisSet.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <ms/MeasurementSets/MSColumns.h>

#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta/Quantum.h>

#include <scimath/Functionals/Polynomial.h>
#include <scimath/Fitting.h>
#include <scimath/Fitting/LinearFit.h>
#include <scimath/Fitting/GenericL2Fit.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class LFBase
  {
  public:  
    // default constructor 
    LFBase  (){NumT=0; NumAnt=0; NumB=0; NumC=0; NumP=0; 
      Record rec = getParameters(); setParameters(rec); };
    // default destructor
    virtual ~LFBase (){};

    //Return method name
    virtual String methodName()=0;

    // Set autoflag params
    virtual Bool setParameters(Record &/*parameters*/){ return False;}
    
    // Get default autoflag params
    virtual Record getParameters(){return Record();};

    // Run the algorithm
    virtual Bool runMethod(const VisBuffer &inVb, 
			   Cube<Float> &inVisc, Cube<Bool> &inFlagc, Cube<Bool> &inPreFlagc,
			   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP)
    {
      vb.assign(inVb,False);
      visc.reference(inVisc); flagc.reference(inFlagc); preflagc.reference(inPreFlagc);
      NumT=numT, NumAnt=numAnt; NumB=numB; NumC=numC; NumP=numP;

      return False;
     };    

    // Extract a diagnostic spectrum vector.
    virtual Bool getMonitorSpectrum(Vector<Float> &/*monspec*/, uInt /*pl*/, uInt /*bs*/)
    {return False;};

    // Set baselineFlags...
    virtual Bool setBaselineFlag(Vector<Bool> &inBaselineFlag)
    {baselineFlag.reference(inBaselineFlag); return False;}

/* Return antenna numbers from baseline number - upper triangle storage */
 void Ants(uInt bs, uInt *a1, uInt *a2)
  {
    uInt sum=0,cnt=0;
    for(uInt i=(NumAnt);i>1;i--)
      {
	sum += i;
	if(sum<=bs) cnt++;
	else break;
      }
    *a1 = cnt;
    sum = (NumAnt)*((NumAnt)+1)/2 - ((NumAnt)-(*a1))*((NumAnt)-(*a1)+1)/2; 
    *a2 = bs - sum + (*a1);
  }
  
  /* Return baseline index from a pair of antenna numbers - upper triangle storage */
  uInt BaselineIndex(uInt /*row*/, uInt a1, uInt a2)
  {
    return ( (NumAnt)*((NumAnt)+1)/2 - ((NumAnt)-a1)*((NumAnt)-a1+1)/2 + (a2 - a1) );
  }
  
    /* Return baseline index for the autocorrelation value for an antenna */
    uInt Self(uInt ant)
    {
      return ( (NumAnt)*((NumAnt)+1)/2 - ((NumAnt)-ant)*((NumAnt)-ant+1)/2 );
    }
    
  protected:

  Bool dbg;

    // Reference to current vb.
    VisBuffer vb;

    // References to input data and flags
    Cube<Float> visc;
    Cube<Bool> flagc;
    Cube<Bool> preflagc;
    Vector<Bool> baselineFlag;

  // Shapes per chunk
  uInt NumT; // Number of timestamps in one block
  uInt NumAnt; // Number of antennas
  uInt NumB; // Number of baselines.
  uInt NumC; // Number of channels
  uInt NumP; // Number of polarizations
    
  };
  
  
} //# NAMESPACE CASA - END

#endif

