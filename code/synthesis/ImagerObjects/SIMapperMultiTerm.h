//# SIMapper.h: Imager functionality sits here; 
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//#
//# $Id$

#ifndef SYNTHESIS_SIMAPPERMULTITERM_H
#define SYNTHESIS_SIMAPPERMULTITERM_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBufferImpl2.h>
#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/ImagerObjects/SIMapperBase.h>
#include <synthesis/ImagerObjects/SIImageStoreMultiTerm.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

  class SIMapperMultiTerm : public SIMapperBase
{
 public:
  // Default constructor

  SIMapperMultiTerm( CountedPtr<SIImageStore>& imagestore,
		     CountedPtr<FTMachine>& ftm, 
		     CountedPtr<FTMachine>& iftm,
		     CountedPtr<VPSkyJones>& vp,
		     uInt ntaylorterms=1);
  virtual ~SIMapperMultiTerm();

  ///// Major Cycle Functions

  
  /////////////////////// NEW VI/VB versions
  void initializeGrid(const vi::VisBuffer2& , Bool ){};
  void grid(const vi::VisBuffer2& , Bool , FTMachine::Type ){};
  void finalizeGrid(const vi::VisBuffer2& , const Bool ){};
  void initializeDegrid(const vi::VisBuffer2& , const Int ){};
  void degrid(vi::VisBuffer2& ){};
  

  /////////////////////// OLD VI/VB versions
  void initializeGrid(VisBuffer& vb, Bool dopsf);
  void grid(VisBuffer& vb, Bool dopsf, FTMachine::Type col);
  void finalizeGrid(VisBuffer& vb, Bool dopsf);
  void initializeDegrid(VisBuffer& vb, Int row=-1);
  void degrid(VisBuffer& vb);
  void finalizeDegrid();

  //////////////the return value is False if no valid record is being returned
  Bool getCLRecord(Record & /*rec*/, const String diskimage=""){diskimage==""; return False;};
  Bool getFTMRecord(Record& rec);

  Bool releaseImageLocks(){return itsImages->releaseLocks();}
  String getImageName(){return itsImages->getName();};
  CountedPtr<SIImageStore> imageStore(){return itsImages;};

protected:

  // Instantiate a new sub FTM
  CountedPtr<FTMachine> getNewFTM(const CountedPtr<FTMachine>& ftm);

  // Multiply Imaging weights by Taylor-function weights - during "put"
  Bool modifyVisWeights(VisBuffer& vb, uInt thisterm);
  // Multiply model visibilities by Taylor-function weights - during "get"
  Bool modifyModelVis(VisBuffer &vb, uInt thisterm);
  // Restore vb.imagingweights to the original
  void restoreImagingWeights(VisBuffer &vb);

  void printFTTypes()
  {
    cout << "** Number of FTs : " << ftms_p.nelements() << " -- " ;
    for(uInt tix=0; tix<(ftms_p).nelements(); tix++)
      cout << tix << " : " << (ftms_p[tix])->name() << "   " ;
    cout << "   and iFTs : " << iftms_p.nelements() << " -- " ;
    for(uInt tix=0; tix<(iftms_p).nelements(); tix++)
      cout << tix << " : " << (iftms_p[tix])->name() << "   " ;
    cout << endl;
  };

  //  CountedPtr<SIImageStoreMultiTerm> itsImages;
  CountedPtr<SIImageStore> itsImages;

  Block< CountedPtr<FTMachine> > ftms_p, iftms_p; 
  vi::VisBuffer2 * vb_p;
  VisBuffer ovb_p;

  uInt nterms_p;
  Double reffreq_p;

  Matrix<Float> imweights_p;
  Cube<Complex> modviscube_p;

};


} //# NAMESPACE CASA - END

#endif
