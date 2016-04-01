//# tAWPFTM.cc: Tests the AWProject class of FTMachines
//# Copyright (C) 2016
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
#include <measures/Measures/Stokes.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <casa/Arrays/ArrayMath.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/Flux.h>
#include <tables/TaQL/ExprNode.h>
#include <measures/Measures/MeasTable.h>
#include <ms/MSSel/MSSelection.h>
#include <synthesis/TransformMachines2/FTMachine.h>
#include <synthesis/TransformMachines2/GridFT.h>
#include <synthesis/TransformMachines2/SetJyGridFT.h>
#include <synthesis/TransformMachines2/WProjectFT.h>
#include <synthesis/TransformMachines2/SimpleComponentFTMachine.h>
#include <msvis/MSVis/VisImagingWeight.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <casa/namespace.h>
#include <casa/OS/Directory.h>
#include <casa/Utilities/Regex.h>
#include <synthesis/TransformMachines2/test/MakeMS.h>
#include <synthesis/TransformMachines2/FTMachine.h>
#include <synthesis/TransformMachines2/AWProjectFT.h>
#include <synthesis/TransformMachines2/AWProjectWBFTNew.h>
#include <synthesis/TransformMachines2/AWConvFunc.h>
#include <synthesis/TransformMachines2/AWConvFuncEPJones.h>
#include <synthesis/TransformMachines2/ATerm.h>
#include <synthesis/TransformMachines2/EVLAAperture.h>
#include <synthesis/TransformMachines2/NoOpATerm.h>
#include <synthesis/TransformMachines2/AWVisResampler.h>
#include <ms/MSSel/MSSelection.h>

using namespace casa;
using namespace casa::refim;
using namespace casa::test;

void createAWPFTMachine(CountedPtr<FTMachine>& theFT, CountedPtr<FTMachine>& theIFT, 
			const String& telescopeName,
			MPosition& observatoryLocation,
			const Int cfBufferSize=1024,
			const Int cfOversampling=20,
			//------------------------------
			const Int wprojPlane=1,
			const Float=1.0,
			const Bool useDoublePrec=True,
			//------------------------------
			const Bool aTermOn= True,
			const Bool psTermOn= True,
			const Bool mTermOn= False,
			const Bool wbAWP= True,
			const String cfCache= "testCF.cf",
			const Bool doPointing= False,
			const Bool doPBCorr= True,
			const Bool conjBeams= True,
			const Float computePAStep=360.0,
			const Float rotatePAStep=5.0,
			const Int cache=1000000000,
			const Int tile=16,
			const String imageNamePrefix=String("img"))
  
{
  LogIO os( LogOrigin("SynthesisImager","createAWPFTMachine",WHERE));
  
  if (wprojPlane<=1)
    {
      os << LogIO::NORMAL
	 << "You are using wprojplanes=1. Doing co-planar imaging (no w-projection needed)" 
	 << LogIO::POST;
      os << LogIO::NORMAL << "Performing WBA-Projection" << LogIO::POST; // Loglevel PROGRESS
    }
  if((wprojPlane>1)&&(wprojPlane<64)) 
    {
      os << LogIO::WARN
	 << "No. of w-planes set too low for W projection - recommend at least 128"
	 << LogIO::POST;
      os << LogIO::NORMAL << "Performing WBAW-Projection" << LogIO::POST; // Loglevel PROGRESS
    }
  
  // ROMSObservationColumns msoc(mss4vi_p[0].observation());
  // String telescopeName=msoc.telescopeName()(0);
  CountedPtr<ConvolutionFunction> awConvFunc = AWProjectFT::makeCFObject(telescopeName, 
									 aTermOn,
									 psTermOn, True, mTermOn, wbAWP,
									 cfBufferSize, cfOversampling);
  //
  // Construct the appropriate re-sampler.
  //
  CountedPtr<VisibilityResamplerBase> visResampler;
  visResampler = new AWVisResampler();
  //
  // Construct and initialize the CF cache object.
  //
  CountedPtr<CFCache> cfCacheObj;
  
  //
  // Finally construct the FTMachine with the CFCache, ConvFunc and
  // Re-sampler objects.  
  //
  Float pbLimit_l=1e-3;
  theFT = new AWProjectWBFTNew(wprojPlane, cache/2, 
			       cfCacheObj, awConvFunc, 
			       visResampler,
			       /*True */doPointing, doPBCorr, 
			       tile, computePAStep, pbLimit_l, True,conjBeams,
			       useDoublePrec);
  
  cfCacheObj = new CFCache();
  cfCacheObj->setCacheDir(cfCache.data());
  cfCacheObj->setWtImagePrefix(imageNamePrefix.c_str());
  cfCacheObj->initCache2();
  
  theFT->setCFCache(cfCacheObj);
  
  
  Quantity rotateOTF(rotatePAStep,"deg");
  static_cast<AWProjectWBFTNew &>(*theFT).setObservatoryLocation(observatoryLocation);
  static_cast<AWProjectWBFTNew &>(*theFT).setPAIncrement(Quantity(computePAStep,"deg"),rotateOTF);
  
  theIFT = new AWProjectWBFTNew(static_cast<AWProjectWBFTNew &>(*theFT));
  
  //// Send in Freq info.
  // os << "Sending frequency selection information " <<  mssFreqSel_p  <<  " to AWP FTM." << LogIO::POST;
  // theFT->setSpwFreqSelection( mssFreqSel_p );
  // theIFT->setSpwFreqSelection( mssFreqSel_p );
  
}

Int main(int argc, char **argv)
{
  Int NX=256, NY=256, cfBufferSize=512, cfOversampling=20;
  if (argc > 1) {sscanf(argv[1],"%d",&NX); NY=NX;}
  if (argc > 2) {sscanf(argv[2],"%d",&cfBufferSize);};
  if (argc > 3) {sscanf(argv[3],"%d",&cfOversampling);};
  
  try
    {
      MDirection thedir(Quantity(20.0, "deg"), Quantity(20.0, "deg"));
      String msname("Test.ms");
      MakeMS::makems(msname, thedir);
      MeasurementSet thems(msname, Table::Update);
      thems.markForDelete();
      vi::VisibilityIterator2 vi2(thems,vi::SortColumns(),True);
      vi::VisBuffer2 *vb=vi2.getVisBuffer();
      VisImagingWeight viw("natural");
      vi2.useImagingWeight(viw);
      ComponentList cl;
      SkyComponent otherPoint(ComponentType::POINT);
      otherPoint.flux() = Flux<Double>(6.66e-2, 0.0, 0.0, 0.00000);
      otherPoint.shape().setRefDirection(thedir);
      cl.add(otherPoint);
      Matrix<Double> xform(2,2);
      xform = 0.0;
      xform.diagonal() = 1.0;
      DirectionCoordinate dc(MDirection::J2000, Projection::SIN, Quantity(20.0,"deg"), Quantity(20.0, "deg"),
			     Quantity(0.5, "arcsec"), Quantity(0.5,"arcsec"),
			     xform, (NX/2.0), (NY/2.0), 999.0, 
			     999.0);
      Vector<Int> whichStokes(1, Stokes::I);
      StokesCoordinate stc(whichStokes);
      SpectralCoordinate spc(MFrequency::LSRK, 1.5e9, 1e6, 0.0 , 1.420405752E9);
      CoordinateSystem cs;
      cs.addCoordinate(dc); cs.addCoordinate(stc); cs.addCoordinate(spc);
      TempImage<Complex> im(IPosition(4,NX,NY,1,1), cs);//, "gulu.image");

      CountedPtr<FTMachine> ftm,iftm;
      //FTMachine * ftm=nullptr;
      MPosition loc;
      MeasTable::Observatory(loc, MSColumns(thems).observation().telescopeName()(0));
      createAWPFTMachine(ftm, iftm,
			 String("EVLA"),
			 loc, cfBufferSize, cfOversampling);
      {
	MSSelection thisSelection;
	thisSelection.setSpwExpr(String("*"));
	TableExprNode exprNode=thisSelection.toTableExprNode(&thems);
	
	Matrix<Double> mssFreqSel;
	mssFreqSel  = thisSelection.getChanFreqList(NULL,True);
	// Send in Freq info.
	cerr << "Sending frequency selection information " <<  mssFreqSel  <<  " to AWP FTM." << endl;
	ftm->setSpwFreqSelection( mssFreqSel );
	iftm->setSpwFreqSelection( mssFreqSel );
      }

      {
	im.set(Complex(0.0));
	Matrix<Float> weight;
	vi2.originChunks();
	vi2.origin();
	ftm->initializeToSky(im, weight, *vb);
	refim::SimpleComponentFTMachine cft;
	
	for (vi2.originChunks();vi2.moreChunks(); vi2.nextChunk())
	  {
	    for (vi2.origin(); vi2.more(); vi2.next())
	      {
		cft.get(*vb, cl);
		vb->setVisCube(vb->visCubeModel());
		ftm->put(*vb);	
	      }
	  }
	ftm->finalizeToSky();
	
	ftm->getImage(weight, True);
	
	//  cerr << "val at center " << im.getAt(IPosition(4, 50, 50, 0, 0)) << endl;
	AlwaysAssertExit(near(6.66e-2, real( im.getAt(IPosition(4, (NX/2), (NY/2), 0, 0))), 1.0e-5));
	///Let us degrid now
	im.set(Complex(0.0));
	im.putAt(Complex(999.0, 0.0),IPosition(4, (NX/2), (NY/2), 0, 0) );
	vi2.originChunks();
	vi2.origin();
	ftm->initializeToVis(im, *vb);
	for (vi2.originChunks();vi2.moreChunks(); vi2.nextChunk())
	  {
	    for (vi2.origin(); vi2.more(); vi2.next())
	      {
		//vb->setVisCubeModel(Complex(0.0,0.0));
		ftm->get(*vb);
		//		cerr << "mod " << vb->visCubeModel().shape() << " " << (vb->visCubeModel())(0,0,0) << endl;
		AlwaysAssertExit(near(999.0, abs((vb->visCubeModel())(0,0,0)), 1e-5 ));
	      }
	  }
      }
      //detach the ms for cleaning up
      thems=MeasurementSet();
    } 
  catch (AipsError x) 
    {
      cout << "Caught exception " << endl;
      cout << x.getMesg() << endl;
      return(1);
    }
  cerr <<"OK" << endl;
  exit(0);
}
