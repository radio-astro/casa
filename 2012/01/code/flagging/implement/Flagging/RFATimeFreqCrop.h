//# RFATimeFreqCrop.h : This defines RFATimeFreqCrop
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
//# $Id$

#ifndef FLAGGING_RFATIMEFREQCROP_H
#define FLAGGING_RFATIMEFREQCROP_H

#include <flagging/Flagging/RFAFlagCubeBase.h> 
#include <flagging/Flagging/RFDataMapper.h> 
#include <flagging/Flagging/RFFlagCube.h> 
#include <flagging/Flagging/RFRowClipper.h> 
#include <flagging/Flagging/RFABase.h>
#include <casa/Arrays/Vector.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Fitting.h>
#include <scimath/Fitting/LinearFit.h>
#include <scimath/Fitting/GenericL2Fit.h>
#include <scimath/Mathematics/MedianSlider.h> 
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>
// RFATimeFreqCrop : 2-Dimensional Time Frequency Autoflag agent 
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> RFDataMapper
//   <li> RFFlagCubeBase
//   <li> RFCubeLattice
// </prerequisite>
//
// <synopsis>
// </synopsis>
//
// <todo asof="2002/08/06">
//   <li> 
//   <li> 
//   <li> 
//   // </todo>



//class RFATimeFreqCrop : public RFADiffMapBase
class RFATimeFreqCrop : public RFAFlagCubeBase, public RFDataMapper
{
public:
  RFATimeFreqCrop  ( RFChunkStats &ch,const RecordInterface &parm );
  virtual ~RFATimeFreqCrop ();

  virtual Bool newChunk (Int &i);
  virtual void endChunk ();
  virtual void startData (bool verbose);
  virtual IterMode iterTime (uInt itime);
  virtual IterMode iterRow  (uInt irow);
  virtual IterMode endData  ();
  virtual void startFlag (bool verbose);
  virtual void iterFlag(uInt itime);
  static const RecordInterface & getDefaults ();
  
private:
  
  Float UMean(Vector<Float> vect, Vector<Bool> flag);
  Float UStd(Vector<Float> vect, Vector<Bool> flag, Vector<Float> fit);
  Float UStd(Vector<Float> vect, Vector<Bool> flag, Float mean);
  void CleanBand(Vector<Float> data,Vector<Float> fit);
  void PolyFit(Vector<Float> data,Vector<Bool> flag, Vector<Float> fit, uInt lim1, uInt lim2,uInt deg);
  void LineFit(Vector<Float> data,Vector<Bool> flag, Vector<Float> fit, uInt lim1, uInt lim2);
  void Ants(uInt bs, uInt *a1, uInt *a2);
  uInt BaselineIndex(uInt row, uInt a1, uInt a2);

  void Display_ds9(Int xdim, Int ydim, Matrix<Float> &data, Int frame);
  void Plot_ds9(Int dim, Vector<Float> data1, Vector<Float> data2);

  void AllocateMemory();  
  void FlagZeros();
  void RunTFCrop();
  void FlagTimeSeries(uInt pl, uInt bs);
  void FitCleanBandPass(uInt pl, uInt bs);
  void FlagBandPass(uInt pl, uInt bs);
  void GrowFlags(uInt pl, uInt bs);
  RFA::IterMode ShowFlagPlots();
  void ExtendFlags();
  void FillChunkFlags();

  uInt itime;  
  VisibilityIterator &vi; 
  VisBuffer &vb;
  uInt TimeStamps;
  String msname;
  Vector<Int> ant1,ant2;
  
  Cube <Complex> *vv; 	// visCube ptr - for one timestamp
  Cube <Complex> vc; 	// visCube ptr - for one timestamp
  Cube <Bool> ff; 	// flagCube ptr - for one timestamp
  Vector <Bool> fr;     // rowflag ptr - for one timestamp
  Cube<Float> visc; 	// visCube
  Cube<Bool> flagc; 	// flagCube
  Cube<Float> meanBP; 	// mean bandpass - npol x nbaselines x nchannels
  Cube<Float> cleanBP; 	// cleaned bandpasses - npol x nbaselines x nchannels
  Vector<Bool> rowflags; // rowflags from the MS - ntime x nbaselines
  Vector<Bool> baselineflags; // flags for baselines - NumB (for internal use)

  Cube<Bool> chunkflags; // Flags for the entire chunk.
  
  Vector<Float> tempBP; // temporary workspace - nchannels
  Vector<Float> tempTS; // temporary workspace - ntimes
  Vector<Bool> flagBP; 	// temporary list of flags - nchannels
  Vector<Bool> flagTS; 	// temporary list of flags - ntimes
  Vector<Float> fitBP; 	// temporary fit array - nchannels
  Vector<Float> fitTS; 	// temporary fit array - ntimes
 
  Float T_TOL,F_TOL,ANT_TOL,BASELN_TOL; // Input parameters
  uInt FlagLevel,CorrChoice,NumTime,MaxNPieces;//Input parameters
  Bool ShowPlots,DryRun,IgnorePreflags;
  uInt nPol;
  Bool FreqLineFit;
  Bool StopAndExit;
  String Column;
  Vector<String> Expr;
 
  Matrix<Bool> RowFlags; // Row Flags
  
  IPosition cubepos,matpos; 

  uInt passcnt;
  uInt iterRowcnt;
  uInt iterTimecnt;
  uInt timecnt;

  uInt NumT; // Number of timestamps in one block
  uInt NumB; // Number of baselines.
  uInt NumAnt; // Number of antennas.
  uInt NumC; // Number of channels
  uInt NumP; // Number of polarizations
 
  FlagCubeIterator flag_iter; // Local Flag Iterator
};

    

} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <flagging/Flagging/RFATimeFreqCrop.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
