//# SIMapperMultiTerm.cc: Implementation of SIMapperMultiTerm.h
//# Copyright (C) 1997-2008
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>

#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <casa/OS/DirectoryIterator.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>

#include <casa/OS/HostInfo.h>

#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisBuffer2Adapter.h>
#include <synthesis/TransformMachines/BeamSkyJones.h>
#include <synthesis/TransformMachines/SkyJones.h>
#include <synthesis/TransformMachines/SimpleComponentFTMachine.h>
#include <synthesis/TransformMachines/SimpCompGridMachine.h>

#include <synthesis/ImagerObjects/SIMapperMultiTerm.h>
#include <synthesis/ImagerObjects/SIImageStoreMultiTerm.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SIMapperMultiTerm::SIMapperMultiTerm( CountedPtr<SIImageStore>& imagestore, 
					CountedPtr<FTMachine>& ftm, CountedPtr<FTMachine>& iftm,
					CountedPtr<VPSkyJones>& vp,
					uInt ntaylorterms)
    : SIMapperBase(vp),
      vb_p (vi::VisBuffer2::factory (vi::VbPlain, vi::VbRekeyable))
  {
    LogIO os( LogOrigin("SIMapperMultiTerm","Construct a mapper",WHERE) );
    
    nterms_p = ntaylorterms;
    
    ftms_p.resize(nterms_p);
    iftms_p.resize(2*nterms_p-1);
    
    for(uInt tix=0; tix<2*nterms_p-1; tix++)
      {
	if( tix < nterms_p ) 
	  { ftms_p[tix] = getNewFTM( ftm ); }
	iftms_p[tix] = getNewFTM( iftm );
      }
    
    itsImages=imagestore;
    
    reffreq_p = static_cast<SIImageStoreMultiTerm*> (&*itsImages)->getReferenceFrequency();
  
  os << "Make a multi-term mapper with " << nterms_p << " terms with forward " << ftms_p.nelements() << " and reverse " << iftms_p.nelements() << " and reference freq : " << reffreq_p << LogIO::POST;
  
  //  printFTTypes();
  
}

SIMapperMultiTerm::~SIMapperMultiTerm() 
{
  delete vb_p;
}

  CountedPtr<FTMachine> SIMapperMultiTerm::getNewFTM(const CountedPtr<FTMachine>& ftm)
  {
    return ftm->cloneFTM();

    /*
    FTMachine* newftm = ftm->cloneFTM();
    (*newftm) = (*ftm);

    return newftm;
    */
    /* 
    if(ftm->name()=="GridFT")
      {
       	return new GridFT(static_cast<const GridFT&>(*ftm)); 
      }
      else  if(ftm->name()=="AWProjectWBFT") 
      { return new AWProjectWBFT(static_cast<const AWProjectWBFT&>(*ftm)); }
    else
      {throw(AipsError("FTMachine "+ftm->name()+" is not supported with MS-MFS")); }
    */

  }
  
  // #############################################
  // #############################################
  // #######  Gridding / De-gridding functions ###########
  // #############################################
  // #############################################

   void SIMapperMultiTerm::initializeGrid(VisBuffer&  vb, Bool dopsf )
  {
    
    LogIO os( LogOrigin("SIMapperMultiTerm","initializeGrid",WHERE) );

    uInt gridnterms = nterms_p;
    if(dopsf){ gridnterms = 2*nterms_p-1; }

    //    Matrix<Float> wgt;
    for(uInt tix=0; tix<gridnterms; tix++)
      {
	initializeGridCore( vb, iftms_p[tix], *(itsImages->backwardGrid(tix)) );// , wgt );
	//  replace backgrid and wgt by taylor-specific ones.
      }

    ovb_p.assign(vb, False);
    
  }

  /////////////////OLD vi/vb version
  void SIMapperMultiTerm::grid(VisBuffer& vb, Bool dopsf, FTMachine::Type col)
   {
     LogIO os( LogOrigin("SIMapperMultiTerm","grid",WHERE) );

    uInt gridnterms = nterms_p;
    if(dopsf){ gridnterms = 2*nterms_p-1; }

    gridCore( vb, dopsf, col, iftms_p[0], -1 );

    for(uInt tix=1;tix<gridnterms;tix++)
      {
	modifyVisWeights(vb,tix);
	gridCore( vb, dopsf, col, iftms_p[tix], -1 );
	restoreImagingWeights(vb);
      }

   }


  //////////////OLD VI/VB version
  void SIMapperMultiTerm::finalizeGrid(VisBuffer& /* vb */, Bool dopsf)
  {
    LogIO os( LogOrigin("SIMapperMultiTerm","finalizeGrid",WHERE) );
    
    uInt gridnterms = nterms_p;
    if(dopsf){ gridnterms = 2*nterms_p-1; }

    for(uInt tix=0;tix<gridnterms;tix++)
      {
	//	Matrix<Float> wgt;
	finalizeGridCore(dopsf,  iftms_p[tix], 
			 (dopsf ? *(itsImages->psf(tix)) : *(itsImages->residual(tix)) ) ,
			 (useWeightImage(iftms_p[tix]))?*(itsImages->weight(tix)): *(itsImages->psf(tix)),  
			 useWeightImage(iftms_p[tix])  );
	// *(itsImages->weight(tix)) ,   wgt);  

	// replace by taylor dependent images when ready.
      }

    }


  //////////////////OLD vi/vb version
  void SIMapperMultiTerm::initializeDegrid(VisBuffer& vb, Int /*row*/)
  {
	  LogIO os( LogOrigin("SIMapperMultiTerm", "initializeDegrid",WHERE) );

	  for (uInt tix=0; tix<nterms_p; tix++)
	    {
	      initializeDegridCore( vb, ftms_p[tix], *(itsImages->model(tix)) , *(itsImages->forwardGrid(tix)) );
	      // replace image grids by multiterm ones....
	    }

  }

   void SIMapperMultiTerm::degrid(VisBuffer& vb)
    {
      LogIO os( LogOrigin("SIMapperMultiTerm","degrid",WHERE) );

      CountedPtr<ComponentFTMachine> cft;
      ComponentList cl;
      degridCore( vb, ftms_p[0], cft, cl );

      // Save the model visibilities in a local cube
      modviscube_p.assign( vb.modelVisCube() );
    
      for(uInt tix=1;tix<nterms_p;tix++) // Only nterms.... not 2nterms-1
	{
	  // Reset the model visibilities to zero
	  vb.setModelVisCube(Complex(0.0,0.0));
	  // De-grid the model onto the modelviscube (other Taylor terms)
	  degridCore( vb, ftms_p[tix], cft, cl );
	  // Multiply visibilities by taylor-weights
	  modifyModelVis(vb,tix); 
	  // Accumulate model visibilities across Taylor terms
	  modviscube_p += vb.modelVisCube();
	}
      // Set the vb.modelviscube to what has been accumulated
      vb.setModelVisCube(modviscube_p);
      
    }

  void SIMapperMultiTerm::finalizeDegrid()
  {
    LogIO os( LogOrigin("SIMapperMultiTerm","finalizeDegrid",WHERE) );
  }


  Bool SIMapperMultiTerm::getFTMRecord(Record& rec)
  {
    LogIO os( LogOrigin("SIMapperMultiTerm","getFTMRecord",WHERE) );

    AlwaysAssert( nterms_p==ftms_p.nelements() , AipsError );

    Bool rstat=True;

    for(uInt tix=0; tix<nterms_p;tix++)
      {
	Record subFTContainer;
	String error;
	rstat &= ftms_p[tix]->toRecord(error, subFTContainer,True);
	rec.defineRecord("subftm_"+String::toString(tix),subFTContainer);
      }
    rec.define("nterms",nterms_p);
    rec.define("reffreq",reffreq_p);

    return rstat;
  }


  // Multiply the imaging weights by Taylor functions - in place
  // This function MUST be called in ascending Taylor-term order
  // NOTE : Add checks to ensure this.
  Bool SIMapperMultiTerm::modifyVisWeights(VisBuffer& vb,uInt thisterm)
  {
      
      if(imweights_p.shape() != vb.imagingWeight().shape())
	imweights_p.resize(vb.imagingWeight().shape());
      imweights_p = vb.imagingWeight();
      
      Float freq=0.0,mulfactor=1.0;
      Vector<Double> selfreqlist(vb.frequency());
      
      for (Int row=0; row<vb.nRow(); row++)
	for (Int chn=0; chn<vb.nChannel(); chn++)
	  {
	    freq = selfreqlist(IPosition(1,chn));
	    mulfactor = ((freq-reffreq_p)/reffreq_p);
	    (vb.imagingWeight())(chn,row) *= pow( mulfactor,(Int)thisterm);
	    //	      sumwt_p += (vb.imagingWeight())(chn,row);
	  }
    /* // For debugging.
       else
       {
       for (Int row=0; row<vb.nRow(); row++)
       for (Int chn=0; chn<vb.nChannel(); chn++)
       {
       sumwt_p += (vb.imagingWeight())(chn,row);
       }
       }
    */
    return True;
  }
  
  // Reset the imaging weights back to their original values
  // to be called just after "put"
  void SIMapperMultiTerm::restoreImagingWeights(VisBuffer &vb)
  {
    AlwaysAssert( imweights_p.shape() == vb.imagingWeight().shape() ,AipsError);
    vb.imagingWeight() = imweights_p;
  }
  
  
  // Multiply the model visibilities by the Taylor functions - in place.
  Bool SIMapperMultiTerm::modifyModelVis(VisBuffer& vb, uInt thisterm)
  {
    Float freq=0.0,mulfactor=1.0;
    Vector<Double> selfreqlist(vb.frequency());
    
    // DComplex modcount=0.0;

    for (uInt pol=0; pol< uInt((vb.modelVisCube()).shape()[0]); pol++)
      for (uInt chn=0; chn< uInt(vb.nChannel()); chn++)
	for (uInt row=0; row< uInt(vb.nRow()); row++)
	  {
	    // modcount += ( vb.modelVisCube())(pol,chn,row);
	    freq = selfreqlist(IPosition(1,chn));
	    mulfactor = ((freq-reffreq_p)/reffreq_p);
	    (vb.modelVisCube())(pol,chn,row) *= pow(mulfactor, (Int) thisterm);
	  }
    
    // cout << "field : " << vb.fieldId() << " spw : " 
    // 	 << vb.spectralWindow() << "  --- predicted model before taylor wt mult :" 
    // 	 << thisterm << "  sumvis : " << modcount << endl;

    return True;
  }
  



} //# NAMESPACE CASA - END

