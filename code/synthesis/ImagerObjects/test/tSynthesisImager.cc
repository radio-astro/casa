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
#include <synthesis/ImagerObjects/SynthesisImager.h>
#include <synthesis/ImagerObjects/SIImageStore.h>
#include <lattices/Lattices/LatticeConcat.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageConcat.h>
#include <images/Images/SubImage.h>
#include <casa/namespace.h>
#include <images/Images/TempImage.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <ms/MeasurementSets/MSSourceIndex.h>

int main(int argc, char **argv)
{
  using namespace std;
  using namespace casa;
  try{



	  if (argc<2) {
		  cout <<"Usage: tSynthesisImager ms-table-name  [continuum, cube, cubeslice, widefield, facet]"<<endl;
		  exit(1);
	  }
	  Bool useViVb2=False;
	  String msname=String(argv[1]);
	  String imtype=String("continuum");
	  if(argc>2)
		  imtype=String(argv[2]);

	  SynthesisImager* imgr = new SynthesisImager();
	  imgr->selectData(msname, /*spw=*/"0", /*field=*/"0", /*taql=*/"", /*antenna=*/"",  /*uvdist*/"", /*scan*/"", /*obs*/"",
    		/*timestr*/"", /*usescratch*/False, /*readonly*/False);
	  cout <<"--Imager created for MeasurementSet object. " << endl;
	  MeasurementSet tab(msname);
	  MDirection phasecenter=MSFieldColumns(tab.field()).phaseDirMeas(0,0.0);
	  Quantity freqBeg=MSSpWindowColumns(tab.spectralWindow()).chanFreqQuant()(0)(IPosition(1,0));
	  Int ndataChan=MSSpWindowColumns(tab.spectralWindow()).numChan()(0);
	  Quantity freqWidth=MSSpWindowColumns(tab.spectralWindow()).chanFreqQuant()(0)(IPosition(1,ndataChan-1));
	  freqWidth-=freqBeg;
	  if(imtype==String("continuum")){
		  int nx = 100;
		  int ny = 100;
		  Quantity cellx( 30, "arcsec" );
		  Quantity celly( 30, "arcsec" );
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
		  imgr->defineImage(/*imagename*/"test_cont_image", nx, ny, cellx, celly,
			   stokes,phasecenter, nchan,
			   freqBeg, freqWidth, Vector<Quantity>(1,Quantity(1.420, "GHz")), 1);
		  /*
			   const String& ftmachine="GridFT",
			   const Projection& projection=Projection::SIN,
			   const Quantity& distance=Quantity(0,"m"),
			   const MFrequency::Types& freqFrame=MFrequency::LSRK,
			   const Bool trackSource=False, const MDirection&
			   trackDir=MDirection(Quantity(0.0, "deg"),
					       Quantity(90.0, "deg")))
		   */
	  }
	  else if(imtype==String("cube")){
		  int nx = 100;
		  int ny = 100;
		  Quantity cellx( 30, "arcsec" );
		  Quantity celly( 30, "arcsec" );
		  Vector<Int> spwids(2);
		  String stokes="I";
		  Int nchan=1;
		  cerr << "nx=" << nx << " ny=" << ny
				  << " cellx='" << cellx.getValue() << cellx.getUnit()
				  << "' celly='" << celly.getValue() << celly.getUnit()
				  << "' spwids=" << 0
				  << " field=" <<   0 << endl;
		  ////lets do a cube of ndatachan
		  nchan=ndataChan;
		  freqWidth /= Double(nchan);
		  imgr->defineImage(/*imagename*/"test_cube_image", nx, ny, cellx, celly,
				  stokes,phasecenter, nchan,
				  freqBeg, freqWidth, Vector<Quantity>(1,Quantity(1.420, "GHz")), 1);

	  }
	  else if(imtype==String("widefield")){
		  int nx = 300;
		  int ny = 300;
		  Quantity cellx( 30, "arcsec" );
		  Quantity celly( 30, "arcsec" );
		  Vector<Int> spwids(2);
		  String stokes="I";
		  Int nchan=1;
		  cerr << "nx=" << nx << " ny=" << ny
				  << " cellx='" << cellx.getValue() << cellx.getUnit()
				  << "' celly='" << celly.getValue() << celly.getUnit()
				  << "' spwids=" << 0
				  << " field=" <<   0 << endl;

		  freqWidth /= Double(nchan);
		  ///64 projplanes
		  imgr->setupImaging(1.0, False, True, 64, "SF");
		  imgr->defineImage(/*imagename*/"test_widefield_image", nx, ny, cellx, celly,
				  stokes,phasecenter, nchan,
				  freqBeg, freqWidth, Vector<Quantity>(1,Quantity(1.420, "GHz")), 1, "WProjectFT");
	  }
	  else if(imtype==String("facet")){
	  		  int nx = 300;
	  		  int ny = 300;
	  		  Quantity cellx( 30, "arcsec" );
	  		  Quantity celly( 30, "arcsec" );
	  		  Vector<Int> spwids(2);
	  		  String stokes="I";
	  		  Int nchan=1;
	  		  cerr << "nx=" << nx << " ny=" << ny
	  				  << " cellx='" << cellx.getValue() << cellx.getUnit()
	  				  << "' celly='" << celly.getValue() << celly.getUnit()
	  				  << "' spwids=" << 0
	  				  << " field=" <<   0 << endl;

	  		  freqWidth /= Double(nchan);
	  		  imgr->defineImage(/*imagename*/"test_facet_image", nx, ny, cellx, celly,
	  				  stokes,phasecenter, nchan,
	  				  freqBeg, freqWidth, Vector<Quantity>(1,Quantity(1.420, "GHz")), 2);
	  }
	  else if(imtype==String("cubeslice")){

	  		  int nx = 100;
	  		  int ny = 100;
	  		  Quantity cellx( 30, "arcsec" );
	  		  Quantity celly( 30, "arcsec" );
	  		  Vector<Int> spwids(2);
	  		  String stokes="I";
	  		  Int nchan=1;
	  		  cerr << "nx=" << nx << " ny=" << ny
	  				  << " cellx='" << cellx.getValue() << cellx.getUnit()
	  				  << "' celly='" << celly.getValue() << celly.getUnit()
	  				  << "' spwids=" << 0
	  				  << " field=" <<   0 << endl;
	  		  ////lets do a cube of ndatachan
	  		  nchan=ndataChan;
	  		  freqWidth /= Double(nchan);
	  		  imgr->defineImage(/*imagename*/"test_cubesliced_image", nx, ny, cellx, celly,
	  				  stokes,phasecenter, nchan,
	  				  freqBeg, freqWidth, Vector<Quantity>(1,Quantity(1.420, "GHz")), 1);
	  		  CountedPtr<SIImageStore> si=imgr->imageStore(0);
	  		  CountedPtr<ImageInterface<Float> > resid=si->residual();
	  		  CountedPtr<ImageInterface<Float> > psf=si->psf();
	  		  CountedPtr<ImageInterface<Float> > wgt=si->weight();
	  		  CountedPtr<ImageInterface<Float> > mod=si->model();
	  		  CountedPtr<ImageInterface<Float> > restor=si->image();
	  		  IPosition blc(4,0,0,0,0);
	  		  IPosition trc(4,99,99,0,0);
	  		  /////Imaging a channel at a time
	  		  ////you could do a chunk at a time if memory allows

	  		  ///Openmp at this level does not work the MS is unsafe
//////#pragma omp parallel default(none) firstprivate(blc, trc, nchan, msname) shared(resid, psf, wgt, mod, restor) num_threads(1)
	  		  {
//////#pragma omp for

	  		  for (Int k=0; k < nchan; ++k){
	  			  blc[3]=k;
	  			  trc[3]=k;
	  			  Slicer sl(blc, trc, Slicer::endIsLast);
	  			  SubImage<Float> *subresid= new SubImage<Float>(*resid, sl, True);
	  			  SubImage<Float> *subpsf= new SubImage<Float>(*psf, sl, True);
	  			  SubImage<Float>* subwgt= new SubImage<Float>(*wgt, sl, True);
	  			  SubImage<Float>* submod= new SubImage<Float>(*mod, sl, True);
	  			  SubImage<Float>* subrestor= new SubImage<Float>(*restor, sl, True);
	  			  CountedPtr<SIImageStore> subImStor=new SIImageStore(submod, subresid, subpsf, subwgt, subrestor);
	  			  SynthesisImager subImgr;
	  			  //can select the right channel to match subimage
	  			  subImgr.selectData(msname, /*spw=*/"0", /*field=*/"0", /*taql=*/"", /*antenna=*/"",  /*uvdist*/"", /*scan*/"", /*obs*/"",
	  			    		/*timestr*/"", /*usescratch*/False, /*readonly*/True);
	  			  subImgr.defineImage(subImStor, "GridFT");
	  			  subImgr.weight("natural");
	  			  Record rec;
	  			  subImgr.executeMajorCycle(rec, useViVb2);
	  			  subImgr.makePSF(useViVb2);
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
              return 0;
	  	  }
	  else{
		  throw(AipsError("Don't know what you are talking about"));
	  }
	  imgr->weight("natural");
	  Record rec;
	  imgr->executeMajorCycle(rec, useViVb2);
	  imgr->makePSF(useViVb2);
	  CountedPtr<SIImageStore> images=imgr->imageStore(0);
	  LatticeExprNode LEN = max( *(images->residual()) );
	  cerr << "Max of residual=" << LEN.getFloat() << endl;
	  LatticeExprNode psfmax = max( *(images->psf()) );
	  LatticeExprNode psfmin = min( *(images->psf()) );
	  cerr <<"Min max of psf "<< psfmin.getFloat() << " " << psfmax.getFloat() << endl;
	  images->dividePSFByWeight();
	  images->divideResidualByWeight();



  }catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
  }
  return 0;
};
