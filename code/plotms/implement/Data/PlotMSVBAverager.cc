//# PlotMSVBAverager.cc: Implementation of PlotMSVBAverager.h
//# Copyright (C) 1996,1997,1998,1999,2000,2002,2003
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

#include <plotms/Data/PlotMSVBAverager.h>
#include <msvis/MSVis/VisSet.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/ArrayLogical.h>

#define PRTLEV_PMSVBA -1

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------
  
PlotMSVBAverager::PlotMSVBAverager(Int nAnt,Bool chanDepWt) 
  : nAnt_p(nAnt),
    nCorr_p(0),
    nChan_p(0),
    nBlnMax_p(0),
    //    chanIndepWt_p(!chanDepWt),
    chanIndepWt_p(True),
    blnOK_p(),
    blnAve_p(False),
    antAve_p(False),
    timeRef_p(0.0),
    minTime_p(0.0),
    maxTime_p(0.0),
    aveTime_p(0.0),
    aveInterval_p(0.0),
    blnWtSum_p(),
    vbWtSum_p(0.0),
    aveScan_p(0),
    doVC_p(False),
    doMVC_p(False),
    doCVC_p(False),
    doUVW_p(False),
    doWC_p(False),
    avBuf_p(),
    initialized_p(False),
    prtlev_(PRTLEV_PMSVBA)
{
// Construct from the number of antennas and the averaging interval
// Input:
// Output to private data:
//    nAnt_p               Int              No. of antennas
//

  if (prtlev()>2) cout << "PMSVBA::PMSVBA()" << endl;

  // Ensure we do weightCube if any data is done
  doWC_p=doVC_p||doMVC_p||doCVC_p;

};

//----------------------------------------------------------------------------

PlotMSVBAverager::~PlotMSVBAverager()
{
// Null default destructor
//
  if (prtlev()>2) cout << "PMSVBA::~PMSVBA()" << endl;
};

//----------------------------------------------------------------------------

void PlotMSVBAverager::finalizeAverage()
{
// Normalize the current accumulation interval
// Output to private data:
//    avBuf_p         VisBuffer&       Averaged buffer
//  

  if (prtlev()>2) cout  << "  PMSVBA::finalizeAverage()" << endl;

  Int nBln=ntrue(blnOK_p);

  if (nBln>0 && vbWtSum_p>0.0) {

    // Divide by the weights
    Int obln=0;
    for (Int ibln=0;ibln<nBlnMax_p; ++ibln) {
      if (blnOK_p(ibln)) {
	if (!avBuf_p.flagRow()(ibln)) {
	  for (Int ichn=0;ichn<nChan_p;++ichn) {
	    for (Int icor=0;icor<nCorr_p;++icor) {
	      Float& thiswt(avBuf_p.weightCube()(icor,ichn,ibln));
	      if (!avBuf_p.flagCube()(icor,ichn,ibln) &&
		  doWC_p &&  
		  thiswt>0.0) {
		if (doVC_p) 
		  avBuf_p.visCube()(icor,ichn,obln)=
		    avBuf_p.visCube()(icor,ichn,ibln)/thiswt;
		if (doMVC_p) 
		  avBuf_p.modelVisCube()(icor,ichn,obln)=
		    avBuf_p.modelVisCube()(icor,ichn,ibln)/thiswt;
		if (doCVC_p) 
		  avBuf_p.correctedVisCube()(icor,ichn,obln)=
		    avBuf_p.correctedVisCube()(icor,ichn,ibln)/thiswt;
	      } // !flagCube & wt>0
	      if (obln<ibln) {
		avBuf_p.flagCube()(icor,ichn,obln)=avBuf_p.flagCube()(icor,ichn,ibln);
		if (doWC_p)
		  avBuf_p.weightCube()(icor,ichn,obln)=thiswt;
	      }
	    } // icor
	  } // ichn
	 
	  if (doUVW_p && blnWtSum_p(ibln)>0.0)
	    avBuf_p.uvwMat().column(obln)=
	      avBuf_p.uvwMat().column(ibln)/blnWtSum_p(ibln);

	} // flagRow
	else {
	  // if flagRow, set flagCube, too
	  avBuf_p.flagCube().xyPlane(obln)=True;
	}


	if (obln<ibln) {
	  avBuf_p.flagRow()(obln)=avBuf_p.flagRow()(ibln);
	  avBuf_p.antenna1()(obln)=avBuf_p.antenna1()(ibln);
	  avBuf_p.antenna2()(obln)=avBuf_p.antenna2()(ibln);
	}

	// increment the output row
	++obln;
      } // blnOK
    } // ibln

    // Contract the VisBuffer, if necessary
    if (nBln<nBlnMax_p) {
      avBuf_p.nRow()=nBln;
      avBuf_p.time().resize(nBln);
      avBuf_p.timeInterval().resize(nBln);
      avBuf_p.scan().resize(nBln);
      avBuf_p.antenna1().resize(nBln,true);
      avBuf_p.antenna2().resize(nBln,true);
      if (doUVW_p)
	avBuf_p.uvwMat().resize(3,nBln,True);
      if (doVC_p)
	avBuf_p.visCube().resize(nCorr_p,nChan_p,nBln,True);
      if (doMVC_p)
	avBuf_p.modelVisCube().resize(nCorr_p,nChan_p,nBln,True);
      if (doCVC_p)
	avBuf_p.correctedVisCube().resize(nCorr_p,nChan_p,nBln,True);
      if (doWC_p)
	avBuf_p.weightCube().resize(nCorr_p,nChan_p,nBln,True);

      avBuf_p.flagRow().resize(nBln,True);
      avBuf_p.flagCube().resize(nCorr_p,nChan_p,nBln,True);
    }

    // Time:
    //    aveTime_p/=vbWtSum_p;
    //    avBuf_p.time()=aveTime_p;
    avBuf_p.time()=timeRef_p+(maxTime_p-minTime_p)/2.0;  // must be center of the interval!
    avBuf_p.timeInterval()=maxTime_p-minTime_p;
    avBuf_p.scan()=aveScan_p;


    //    cout << "final amp = " << amplitude(avBuf_p.visCube()) << endl;
    //    cout << "final flagR = " << boolalpha << avBuf_p.flagRow() << endl;
    //    cout << "final flagC = " << boolalpha << avBuf_p.flagCube() << endl;

    // We need to be reinitialized to do more accumulating
    initialized_p=False;

  }
  else {


    // cout << "nBln = " << nBln << " (" << nBlnMax_p << ")" << endl;
    //cout << "vbWtSum_p = " << vbWtSum_p << endl;
    //cout << "avBuf_p.flagRow() = " << avBuf_p.flagRow() << endl;

    // Should not be able to reach here
    //  (there is always at least one good baseline, even if flagged)
    // throw(AipsError("Big problem in finalizeAverage!"));
  }
};

//----------------------------------------------------------------------------

void PlotMSVBAverager::initialize(VisBuffer& vb)
{
  // Initialize the averager

  if (prtlev()>2) cout << "  PMSVBA::initialize()" << endl;

  // Assign main meta info only
  avBuf_p.assign(vb,False);  
  avBuf_p.updateCoordInfo();  // This is (simplified) CalVisBuffer version!
  
  // Zero row count
  avBuf_p.nRow();
  avBuf_p.nRow() = 0;
  
  // Immutables:
  nChan_p = vb.nChannel();
  nCorr_p = vb.nCorr();

  if (blnAve_p)
    nBlnMax_p=1;
  else if (antAve_p)
    nBlnMax_p=nAnt_p;
  else 
    nBlnMax_p = nAnt_p*(nAnt_p+1)/2;
  
  blnOK_p.resize(nBlnMax_p);
  blnOK_p=False;
  blnWtSum_p.resize(nBlnMax_p);
  blnWtSum_p=0.0;

  /*
  cout << "Shapes = " 
       << nCorr_p << " "
       << nChan_p << " "
       << nBlnMax_p << " "
       << ntrue(blnOK_p) << " "
       << endl;
  */

  // Set basic shapes
  avBuf_p.nRow()= nBlnMax_p;
  avBuf_p.nChannel() = nChan_p;
  avBuf_p.nCorr() = nCorr_p;

  // Adopt some stationary info from the input vb
  avBuf_p.spectralWindow()=vb.spectralWindow();
  avBuf_p.frequency().resize(nChan_p);
  avBuf_p.frequency()= vb.frequency();
  avBuf_p.channel().resize(nChan_p);
  avBuf_p.channel() = vb.channel();

  avBuf_p.fieldId(); // =vb.fieldId();
  
  // Resize and fill in the antenna numbers for all rows
  avBuf_p.antenna1().resize(nBlnMax_p, False);
  avBuf_p.antenna2().resize(nBlnMax_p, False);

  if (blnAve_p) {
    avBuf_p.antenna1()=-1;
    avBuf_p.antenna2()=-1;
  }
  else if (antAve_p) {
    indgen(avBuf_p.antenna1());
    avBuf_p.antenna2()=-1;
  }
  else {
    Int ibln=0;
    for (Int iant1=0; iant1<nAnt_p; ++iant1) {
      for (Int iant2=iant1; iant2<nAnt_p; ++iant2) {
	avBuf_p.antenna1()(ibln) = iant1;
	avBuf_p.antenna2()(ibln) = iant2;
	++ibln;
      }
    }
  }

  // Resize and initialize everything else
  avBuf_p.time().resize(nBlnMax_p, False); 
  avBuf_p.time().set(0.0);
  aveScan_p=vb.scan0();

  avBuf_p.flagRow().resize(nBlnMax_p, False); 
  avBuf_p.flagRow().set(True);
  avBuf_p.flagCube().resize(nCorr_p,nChan_p, nBlnMax_p,False);
  avBuf_p.flagCube().set(True);

  if (doUVW_p) {
    avBuf_p.uvwMat().resize(3,nBlnMax_p, False); 
    avBuf_p.uvwMat().set(0.0);
  }
  Complex czero(0.0);
  if (doVC_p) {
    avBuf_p.visCube().resize(nCorr_p,nChan_p, nBlnMax_p,False);
    avBuf_p.visCube().set(czero);
  }
  if (doMVC_p) {
    avBuf_p.modelVisCube().resize(nCorr_p,nChan_p, nBlnMax_p,False);
    avBuf_p.modelVisCube().set(czero);
  }
  if (doCVC_p) {
    avBuf_p.correctedVisCube().resize(nCorr_p,nChan_p, nBlnMax_p,False);
    avBuf_p.correctedVisCube().set(czero);
  }

  if (doWC_p) {
    avBuf_p.weightCube().resize(nCorr_p,nChan_p,nBlnMax_p, False); 
    avBuf_p.weightCube().set(0.0f);
  }

  minTime_p=DBL_MAX;
  maxTime_p=-DBL_MAX;
  timeRef_p=vb.time()(0);

  // we are now initialized
  initialized_p=True;

};


//----------------------------------------------------------------------------
void PlotMSVBAverager::simpAccumulate (VisBuffer& vb)
{
// Accumulate a VisBuffer
// Input:
//    vb               const VisBuffer&     VisBuffer to accumulate
// Output to private data:
//    tStart_p         Double               Start time of current accumulation
//    nChan_p          Int                  No. of channels in the avg. buffer
//    avrow_p          Int                  Start row of current accumulation
//    avBuf_p          CalVisBuffer         Averaging buffer
//

  if (prtlev()>2) cout << " PMSVBA::accumulate() " << endl;

  if (!initialized_p) initialize(vb);

  //  cout << "chanIndepWt_p = " << boolalpha << chanIndepWt_p << endl;

  // Only accumulate VisBuffers with the same number of channels
  //  TBD: handle multiple shapes by separating ddis?
  if (vb.nChannel() != nChan_p || vb.nCorr() != nCorr_p) 
    throw(AipsError("PlotMSVBAverager: data shape does not conform"));

  if (vb.spectralWindow()!=avBuf_p.spectralWindow())
    avBuf_p.spectralWindow()=-1;

  /*
  cout << vb.flagCube().shape() << " " 
       << vb.weightMat().shape() << " " 
       << vb.visCube().shape() << " " 
       << avBuf_p.flagCube().shape() << " " 
       << avBuf_p.visCube().shape() << " " 
       << avBuf_p.weightCube().shape() << " " 
       << vb.nRow() << " "
       << ntrue(blnOK_p) << " "
       << endl;
  */
  Double vbWt(0.0);  // will accumulate this VBs total data weight
  Float *wt;
  for (Int ibln=0;ibln<vb.nRow();++ibln) {

    // Calculate row from antenna numbers with the hash function.
    Int ant1=vb.antenna1()(ibln);
    Int ant2=vb.antenna2()(ibln);
    Int obln = baseline(ant1,ant2);

    // This baseline occurs in input, so preserve in output
    blnOK_p(obln)=True;

    if (False) { //  || vb.nRow()==1) {
      cout << ibln << " " 
	   << ant1 << " " << ant2 << " "
	   << obln << " " 
	   << flush;
    }

    // If this row not flagged (and has unflagged cells)
    if (!vb.flagRow()(ibln) && (nfalse(vb.flagCube().xyPlane(ibln))>0) ) {
      avBuf_p.flagRow()(obln)=False;

      // Point to cell weights
      if (chanIndepWt_p)
	wt = &vb.weightMat()(0,ibln);
      else
	wt = &vb.weightSpectrum()(0,0,ibln);


      Double blnWt(0.0);
      for (Int chn=0; chn<vb.nChannel(); chn++) {
	for (Int cor=0;cor<nCorr_p;++cor,++wt) {
	  // If this cell is unflagged
	  if (!vb.flagCube()(cor,chn,ibln) && (*wt)>0.0) {
	    avBuf_p.flagCube()(cor,chn,obln)=False;
	    if (doVC_p) 
	      avBuf_p.visCube()(cor,chn,obln)+=
		( (*wt)*vb.visCube()(cor,chn,ibln) );
	    if (doMVC_p) 
	      avBuf_p.modelVisCube()(cor,chn,obln)+=
		( (*wt)*vb.modelVisCube()(cor,chn,ibln) );
	    if (doCVC_p)
	      avBuf_p.correctedVisCube()(cor,chn,obln)+=
		((*wt)*vb.correctedVisCube()(cor,chn,ibln) );
	    if (doWC_p)
	      avBuf_p.weightCube()(cor,chn,obln)+=(*wt);
	    blnWt+=(*wt);

	  } // flagCube
	} // cor 

	// Use same wt for next channel if they are chan-indep
	if (chanIndepWt_p) wt-=nCorr_p;
      } // chn

      // Don't let it be too large
      blnWt/=Double(nChan_p);
      vbWt+=blnWt;

      blnWtSum_p(obln)+=blnWt;

      // UVW
      if (doUVW_p && blnWt>0.0)
	for (uInt i=0;i<3;++i)
	  avBuf_p.uvwMat()(i,obln)+=(vb.uvwMat()(i,ibln)*blnWt);

    }  // flagRow
  }

  if (vbWt>0) {

    vbWtSum_p+=vbWt;

    Double thisTime(vb.time()(0)-timeRef_p);
    Double thisInterval(vb.timeInterval()(0));

    minTime_p=min(minTime_p,(thisTime-thisInterval/2.0));
    maxTime_p=max(maxTime_p,(thisTime+thisInterval/2.0));

    aveTime_p+=(thisTime*vbWt);
    aveInterval_p+=vb.timeInterval()(0);
 
    Int thisScan=vb.scan0();
    if (aveScan_p!=thisScan)
      aveScan_p=-1;

    // This doesn't work...
    //    Int thisField=vb.fieldId();
    //    if (avBuf_p.fieldId()!=thisField)
    //      avBuf_p.fieldId()=-1;

  }  

  //  cout << "Done with accumulate." << endl;


};

//----------------------------------------------------------------------------

void PlotMSVBAverager::antAccumulate (VisBuffer& vb)
{
// Accumulate a VisBuffer with per-antenna averaging
// Input:
//    vb               const VisBuffer&     VisBuffer to accumulate

  if (prtlev()>2) cout << " PMSVBA::antAccumulate() " << endl;

  if (!initialized_p) initialize(vb);

  // Only accumulate VisBuffers with the same number of channels
  //  TBD: handle multiple shapes by separating ddis?
  if (vb.nChannel() != nChan_p || vb.nCorr() != nCorr_p) 
    throw(AipsError("PlotMSVBAverager: data shape does not conform"));

  if (vb.spectralWindow()!=avBuf_p.spectralWindow())
    avBuf_p.spectralWindow()=-1;

  Double vbWt(0.0);  // will accumulate this VBs total data weight
  Float *wt;
  for (Int ibln=0;ibln<vb.nRow();++ibln) {

    Int obln_i=vb.antenna1()(ibln);
    Int obln_j=vb.antenna2()(ibln);

    // These antennas occur (even if flagged) in input, so preserve in output
    blnOK_p(obln_i)=True;
    blnOK_p(obln_j)=True;
      
    // If this row not flagged (and has unflagged cells)
    if (!vb.flagRow()(ibln) && (nfalse(vb.flagCube().xyPlane(ibln))>0) ) {
      avBuf_p.flagRow()(obln_i)=False;
      avBuf_p.flagRow()(obln_j)=False;
      
      // Point to cell weights
      if (chanIndepWt_p)
	wt = &vb.weightMat()(0,ibln);
      else
	wt = &vb.weightSpectrum()(0,0,ibln);
      
      Double blnWt(0.0);
      for (Int chn=0; chn<vb.nChannel(); chn++) {
	for (Int cor=0;cor<nCorr_p;++cor,++wt) {
	  // If this cell is unflagged
	  if (!vb.flagCube()(cor,chn,ibln) && (*wt)>0.0) {
	    
	    avBuf_p.flagCube()(cor,chn,obln_i)=False;
	    avBuf_p.flagCube()(cor,chn,obln_j)=False;
	    if (doVC_p) {
	      avBuf_p.visCube()(cor,chn,obln_i)+=
		( (*wt)*vb.visCube()(cor,chn,ibln) );
	      avBuf_p.visCube()(cor,chn,obln_j)+=
		( (*wt)*conj(vb.visCube()(cor,chn,ibln)) );
	    }
	    if (doMVC_p) {
	      avBuf_p.modelVisCube()(cor,chn,obln_i)+=
		( (*wt)*vb.modelVisCube()(cor,chn,ibln) );
	      avBuf_p.modelVisCube()(cor,chn,obln_j)+=
		( (*wt)*conj(vb.modelVisCube()(cor,chn,ibln)) );
	    }
	    if (doCVC_p) {
	      avBuf_p.correctedVisCube()(cor,chn,obln_i)+=
		((*wt)*vb.correctedVisCube()(cor,chn,ibln) );
	      avBuf_p.correctedVisCube()(cor,chn,obln_j)+=
		((*wt)*conj(vb.correctedVisCube()(cor,chn,ibln)) );
	    }
	    if (doWC_p) {
	      avBuf_p.weightCube()(cor,chn,obln_i)+=(*wt);
	      avBuf_p.weightCube()(cor,chn,obln_j)+=(*wt);
	    }

	    blnWt+=(*wt);
	    
	  } // flagCube
	} // cor 
	  
	// Use same wt for next channel if they are chan-indep
	if (chanIndepWt_p) wt-=nCorr_p;

      } // chn

      // Don't let it be too large
      blnWt/=Double(nChan_p);
      vbWt+=blnWt;
	
      blnWtSum_p(obln_i)+=blnWt;
      blnWtSum_p(obln_j)+=blnWt;
      
      // UVW
      if (doUVW_p && blnWt>0.0)
	for (uInt i=0;i<3;++i) {
	  avBuf_p.uvwMat()(i,obln_i)+=(vb.uvwMat()(i,ibln)*blnWt);
	  avBuf_p.uvwMat()(i,obln_j)+=(vb.uvwMat()(i,ibln)*blnWt);
	}      
    }  // flagRow
  }

  if (vbWt>0) {

    vbWtSum_p+=vbWt;

    Double thisTime(vb.time()(0)-timeRef_p);
    Double thisInterval(vb.timeInterval()(0));

    minTime_p=min(minTime_p,(thisTime-thisInterval/2.0));
    maxTime_p=max(maxTime_p,(thisTime+thisInterval/2.0));

    aveTime_p+=(thisTime*vbWt);
    aveInterval_p+=vb.timeInterval()(0);
 
    Int thisScan=vb.scan0();
    if (aveScan_p!=thisScan)
      aveScan_p=-1;

    // This doesn't work...
    //    Int thisField=vb.fieldId();
    //    if (avBuf_p.fieldId()!=thisField)
    //      avBuf_p.fieldId()=-1;

  }  

  //  cout << "Done with accumulate2." << endl;


};

//----------------------------------------------------------------------------

Int PlotMSVBAverager::baseline(const Int& ant1, const Int& ant2)
{
// Compute row index in an accumulation interval for an
// interferometer index (ant1, ant2).
// Input:
//    ant1            const Int&      Antenna 1
//    ant2            const Int&      Antenna 2
// Output:
//    hashFunction    Int             Row offset in current accumulation
//

  if (blnAve_p)
    return 0;

  Int index;
  index = nAnt_p * ant1 - (ant1 * (ant1 - 1)) / 2 + ant2 - ant1;
  return index;
};

//----------------------------------------------------------------------------

  



} //# NAMESPACE CASA - END

