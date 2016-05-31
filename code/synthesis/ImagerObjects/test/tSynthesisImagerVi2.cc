/*
 * tSynthesisImager.cc
 *SynthesisImager.cc: test of SynthesisImager
//# Copyright (C) 2013
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
 *  Created on: Jun 27, 2013
 *      Author: kgolap
 */





#include <casa/iostream.h>
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <measures/Measures/MRadialVelocity.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Logging/LogIO.h>
#include <synthesis/ImagerObjects/SynthesisImagerVi2.h>
#include <synthesis/ImagerObjects/SIImageStore.h>
#include <imageanalysis/Utilities/SpectralImageUtil.h>
#include <lattices/Lattices/LatticeConcat.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageConcat.h>
#include <images/Images/SubImage.h>
#include <casa/namespace.h>
#include <images/Images/TempImage.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <ms/MSSel/MSSourceIndex.h>
#include <synthesis/TransformMachines2/test/MakeMS.h>

int main(int argc, char **argv)
{
  using namespace std;
  using namespace casa;
  using namespace casa::test;
  try{



    /*  if (argc<2) {
		  cout <<"Usage: tSynthesisImager ms-table-name  [continuum, cube, cubeslice, widefield, facet]  [vi2] "<<endl;
		  exit(1);
	  }
    */	 
	  String imtype=String("continuum");
	  String ftmName("");
	  if(argc>1) imtype=String(argv[1]);
	  if (argc > 2) ftmName = String(argv[2]);
	  if(imtype.contains("help")){
	    cerr << "usage: " << argv[0] << " [continuum, cube, widefield, cubeslice] [gridft, awprojecft]"<< endl;
	    exit(-1);
	  }
	  
	  MDirection thedir(Quantity(20.0, "deg"), Quantity(20.0, "deg"));
	  String msname("Test2.ms");
	  const Int numchan=10;
	  MakeMS::makems(msname, thedir, 1.0e9, 1e8, numchan, 20);
	  //MakeMS::makems(msname, thedir, 1.5e9, 1e6, numchan, 20);
	  MeasurementSet thems(msname, Table::Update);
	  thems.markForDelete();
	  MSColumns(thems).data().fillColumn(Matrix<Complex>(4,numchan, Complex(6.66e-2)));
	  MSColumns(thems).correctedData().fillColumn(Matrix<Complex>(4,numchan, Complex(6.66e-2)));
	  thems.flush();
	  SynthesisImager* imgr = new SynthesisImagerVi2();
	  imgr->selectData(msname, /*spw=*/"0",/*freqBeg*/"", /*freqEnd*/"", /*freqFrame*/MFrequency::LSRK, 
			   /*field=*/"0",  /*antenna=*/"",  /*timestr*/"", /*scan*/"", /*obs*/"", /*state*/"",/*uvdist*/"", 
			   /*taql*/"", /*usescratch*/False, /*readonly*/False);
	  cout <<"--Imager created for MeasurementSet object. " << endl;
	  MeasurementSet tab(msname);
	  MDirection phasecenter=MSFieldColumns(tab.field()).phaseDirMeas(0,0.0);
	  Quantity freqBeg=MSSpWindowColumns(tab.spectralWindow()).chanFreqQuant()(0)(IPosition(1,0));
	  Int ndataChan=MSSpWindowColumns(tab.spectralWindow()).numChan()(0);
	  Quantity freqWidth=MSSpWindowColumns(tab.spectralWindow()).chanFreqQuant()(0)(IPosition(1,ndataChan-1));
	  freqWidth-=freqBeg;
	  int nx,ny;
	  nx = ny = 1024;
	  Quantity cellx( 10.0, "arcsec" );
	  Quantity celly( 10.0, "arcsec" );
	  // Quantity cellx( 0.5, "arcsec" );
	  // Quantity celly( 0.5, "arcsec" );
	  Vector<Int> spwids(2);
	  String stokes="I";
	  Int nchan=1;
	  cerr << "nx=" << nx << " ny=" << ny
			  << " cellx='" << cellx.getValue() << cellx.getUnit()
			  << "' celly='" << celly.getValue() << celly.getUnit()
			  << "' spwids=" << 0
			  << " field=" <<   0 << endl;
	  ////lets do a cube of ndatachan
	  //nchan=ndataChan;
	  freqWidth /= Double(nchan);
	  String imageName("test_cont_image2");
	  String cfCache("test.cf");
	  SynthesisParamsImage impars;
	  SynthesisParamsGrid gridpars;
	  Vector<Quantity> qCellSize(2); qCellSize[0]=cellx; qCellSize[1]=celly;
	  Vector<Int> ims(2);ims[0]=nx, ims[1]=ny;
	  // Set up the parameters common for all modes
	  impars.cellsize=qCellSize;
	  impars.imsize=ims;
	  impars.phaseCenter=phasecenter;
	  impars.stokes=stokes;
	  impars.freqStart=freqBeg;
	  impars.freqStep=freqWidth;
	  impars.restFreq=Vector<Quantity>(1,Quantity(1.420, "GHz"));
	  impars.nchan=-1;
	  impars.imageName=imageName;

	  if (ftmName != "") gridpars.ftmachine=ftmName;
	  gridpars.facets=1;
	  //	  gridpars.imageName=imageName;
	  gridpars.aTermOn=True;
	  gridpars.psTermOn=False;
	  gridpars.mTermOn=True;
	  gridpars.wbAWP=True;
	  gridpars.cfCache=cfCache;
	  gridpars.conjBeams=True;
	  if(imtype==String("continuum")){
	    std::string cmd=string("rm -rf ")+string(imageName)+string(".*");
	    system(cmd.c_str());
	    imgr->defineImage(impars,gridpars);
	  }
	  else if(imtype==String("cube")){
		  ////lets do a cube of ndatachan
		  nchan=ndataChan;
		  freqWidth /= Double(nchan);

		  impars.nchan=nchan;
		  impars.freqStep=freqWidth;

		  imgr->defineImage(impars,gridpars);
	  }
	  else if(imtype==String("widefield")){
	    ims[0]=ims[1]=300;
	    impars.imsize=ims;
	    impars.nTaylorTerms=1;
	    gridpars.ftmachine=String("wprojectft");
	    gridpars.wprojplanes=64;
	    impars.imageName=String("test_widefield_image2");
	    imgr->defineImage(impars,gridpars);
	  }
	  else if(imtype==String("facet")){
	    ims[0]=ims[1]=300;
	    impars.imsize=ims;
	    gridpars.facets=2;
	    impars.imageName=String("test_facet_image2");
	    imgr->defineImage(impars,gridpars);
	  }
	  else if(imtype==String("cubeslice")){
	    ims[0]=ims[1]=100;
	    nchan=ndataChan;
	    freqWidth /= Double(nchan);

	    impars.imsize=ims;
	    impars.nchan=nchan;
	    impars.freqStep=freqWidth;
	    impars.imageName=String("test_cubesliced_image2");
	    //	    imgr->defineImage(impars,gridpars);

	    nx = ny = 100;
	    		  imgr->defineImage(/*imagename*/"test_cubesliced_image2", nx, ny, cellx, celly,
	    		  		  stokes,phasecenter, nchan,
	    		  		  freqBeg, freqWidth, Vector<Quantity>(1,Quantity(1.420, "GHz")), 1);
	  		  CountedPtr<SIImageStore> si=imgr->imageStore(0);
	  		  CountedPtr<ImageInterface<Float> > resid=si->residual();
	  		  CountedPtr<ImageInterface<Float> > psf=si->psf();
	  		  CountedPtr<ImageInterface<Float> > wgt=si->weight();
	  		  CountedPtr<ImageInterface<Float> > mod=si->model();
	  		  CountedPtr<ImageInterface<Float> > restor=si->image();
	  		  /////Imaging a channel at a time
	  		  ////you could do a chunk at a time if memory allows

	  		  ///Openmp at this level does not work the MS is unsafe
//////#pragma omp parallel default(none) firstprivate(blc, trc, nchan, msname) shared(resid, psf, wgt, mod, restor) num_threads(1)
	  		  {
//////#pragma omp for

	  		  for (Int k=0; k < nchan; ++k){
			    /*    std::shared_ptr<ImageInterface<Float> >subresid=std::make_shared<SubImage<Float> >(SpectralImageUtil::getChannel(*resid, k, k, True));
			    SHARED_PTR<ImageInterface<Float> >subpsf= std::make_shared<SubImage<Float> >(SpectralImageUtil::getChannel(*psf, k, k, True));
			    SHARED_PTR<ImageInterface<Float> > subwgt= std::make_shared<SubImage<Float> >(SpectralImageUtil::getChannel(*wgt, k, k, True));
			    SHARED_PTR<ImageInterface<Float> > submod=std::make_shared<SubImage<Float> >( SpectralImageUtil::getChannel(*mod, k, k, True));
			    SHARED_PTR<ImageInterface<Float> > subrestor= std::make_shared<SubImage<Float> >(SpectralImageUtil::getChannel(*restor, k, k, True));
			    String freqBeg=String::toString(SpectralImageUtil::worldFreq(subresid->coordinates(), Double(-0.5)))+"Hz";
			    String freqEnd=String::toString(SpectralImageUtil::worldFreq(subresid->coordinates(), Double(0.5)))+"Hz";
			    CountedPtr<SIImageStore> subImStor=new SIImageStore(submod, subresid, subpsf, subwgt, subrestor, nullptr, nullptr, resid->coordinates(), "");
			    */
			    SHARED_PTR<SIImageStore> subImStor=si->getSubImageStore(0, 1, k, nchan, 0,1);
			    String freqBeg=String::toString(SpectralImageUtil::worldFreq((subImStor->residual())->coordinates(), Double(-0.5)))+"Hz";
			    String freqEnd=String::toString(SpectralImageUtil::worldFreq((subImStor->residual())->coordinates(), Double(0.5)))+"Hz";
			    SynthesisImager subImgr;
	  			  //can select the right channel to match subimage
			    subImgr.selectData(msname, /*spw=*/"0", freqBeg, freqEnd, MFrequency::LSRK, /*field=*/"0",  
					       /*antenna=*/"",  /*timestr*/"", /*scan*/"", /*obs*/"", /*state*/"",/*uvdist*/"", 
					       /*taql*/"", /*usescratch*/False, /*readonly*/False, /*incrmodel*/True);

			    subImgr.defineImage(subImStor, "gridft");
			    subImgr.weight("natural");
			    Record rec;
			    subImgr.executeMajorCycle(rec);
			    subImgr.makePSF();
	  		  }
			  
	  		  }
	  		  //We can do the division at the end
	  		  si->dividePSFByWeight();
	  		  si->divideResidualByWeight();
	  		  LatticeExprNode LEN = max( *(si->residual()) );
	  		  cerr << "Max of whole residual image " << LEN.getFloat() << endl;
	  		  LatticeExprNode psfmax = max( *(si->psf()) );
	  		  LatticeExprNode psfmin = min( *(si->psf()) );
	  		  cerr <<"Min max of whole psf "<< psfmin.getFloat() << " " << psfmax.getFloat() << endl;
	  		  delete imgr;
              return 0;
	  	  }
	  else if(imtype==String("predictimage")){
	    impars.imageName=String("test_cont_image");
	    imgr->defineImage(impars,gridpars);

	    imgr->predictModel();
	    delete imgr;
	    return 0;
	  }
	  else{
		  throw(AipsError("Don't know what you are talking about"));
	  }
	  imgr->weight("natural");
	  Record rec;
	  imgr->executeMajorCycle(rec);
	  imgr->makePSF();
	  //imgr->makePSF(useViVb2);
	  CountedPtr<SIImageStore> images=imgr->imageStore(0);
	  if(images.null())
	    throw(AipsError("Did not get shared_ptr "));
	  images->dividePSFByWeight();
	  images->divideResidualByWeight();
	  {
	    //After Normalization
	    LatticeExprNode LENMaxRes = max( *(images->residual()) );
	    cerr << "Max of residual=" << LENMaxRes.getFloat() << endl;
	    LatticeExprNode psfmax = max( *(images->psf()) );
	    LatticeExprNode psfmin = min( *(images->psf()) );
	    cerr <<"Min max of psf "<< psfmin.getFloat() << " " << psfmax.getFloat() << endl;
	    AlwaysAssertExit(near(6.66e-2, LENMaxRes.getFloat(), 1.0e-5));
	  }
	  delete imgr;


  }catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
    exit(-1);
  }
  cout << "OK" << endl;
  return 0;
};
