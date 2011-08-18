//# LFTimeFreqCrop: A lighter flagger - for autoflag
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
#ifndef FLAGGING_LFTIMEFREQCROP_H
#define FLAGGING_LFTIMEFREQCROP_H

#include <flagging/Flagging/LFBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class LFTimeFreqCrop : public LFBase
  {
  public:  
    // default constructor 
    LFTimeFreqCrop  ();
    // default destructor
    ~LFTimeFreqCrop ();

    // Return method name
    String methodName(){return String("tfcrop");};

    // Set autoflag params
    Bool setParameters(Record &parameters);
    
    // Get default autoflag params
    Record getParameters();

    // Run the algorithm
    Bool runMethod(const VisBuffer &inVb, Cube<Float> &inVisc, Cube<Bool> &inFlagc, Cube<Bool> &inPreFlagc,
		   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP);    

    Bool getMonitorSpectrum(Vector<Float> &monspec, uInt pl, uInt bs);
    
  private:

    // TFCROP functions
  Float UMean(Vector<Float> vect, Vector<Bool> flag);
  Float calcVar(Vector<Float> vect, Vector<Bool> flag, Vector<Float> fit);
  Float UStd(Vector<Float> vect, Vector<Bool> flag, Vector<Float> fit);
  Float UStd(Vector<Float> vect, Vector<Bool> flag, Float mean);
    void FitPiecewisePoly(Vector<Float> data,Vector<Bool> flag, Vector<Float> fit);
  void PolyFit(Vector<Float> data,Vector<Bool> flag, Vector<Float> fit, uInt lim1, uInt lim2,uInt deg);
  void LineFit(Vector<Float> data,Vector<Bool> flag, Vector<Float> fit, uInt lim1, uInt lim2);

    //  void Ants(uInt bs, uInt *a1, uInt *a2);
    //uInt BaselineIndex(uInt row, uInt a1, uInt a2);

  void AllocateMemory();  

    void FitBaseAndFlag(uInt pl, uInt bs, String fittype, String direction, Cube<Float> &cleanArr);
    //    void FlagDeviationsFromBase(uInt pl, uInt bs, String direction, Cube<Float> &cleanArr);

    /*
    void FitCleanBandPass(uInt pl, uInt bs, String fittype);
    void FitCleanTimeSeries(uInt pl, uInt bs, String fittype);

    void FlagTimeSeriesAgain(uInt pl, uInt bs);
    void FlagBandPass(uInt pl, uInt bs);
    */

    //void FlagTimeSeries(uInt pl, uInt bs, String fittype);


    // Input parameters
    Double ANT_TOL, BASELN_TOL, T_TOL, F_TOL;
    Int CorrChoice, NumTime, MaxNPieces, halfWin_p, MaxDeg;
    String Expr, Column;
    Vector<Int> intSelCorr;

    String timeFitType_p, freqFitType_p, flagDimension_p, winStats_p;
 
    // Shapes per chunk

    //    uInt Nrow; // Number of rows in current chunk
    //  uInt Ncorr; // Number of correlations
    //IPosition VisCubeShp;


  Cube<Float> meanBP; 	// mean bandpass - npol x nbaselines x nchannels
  Cube<Float> cleanBP; 	// cleaned bandpasses - npol x nbaselines x nchannels
  Cube<Float> cleanTS; 	// cleaned timeseries - npol x nbaselines x ntimes

  Vector<Float> tempBP; // temporary workspace - nchannels
  Vector<Float> tempTS; // temporary workspace - ntimes
  Vector<Bool> flagBP; 	// temporary list of flags - nchannels
  Vector<Bool> flagTS; 	// temporary list of flags - ntimes
  Vector<Float> fitBP; 	// temporary fit array - nchannels
  Vector<Float> fitTS; 	// temporary fit array - ntimes
    Vector<Int> ant1,ant2;
    
  };
  
  
} //# NAMESPACE CASA - END

#endif

