//# imageconcat  -- a simplistic command line imageconcat
//# Copyright (C) 2013
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

#include <casa/iostream.h>
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <measures/Measures/MRadialVelocity.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Logging/LogIO.h>
#include <lattices/Lattices/LatticeConcat.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageConcat.h>
#include <casa/namespace.h>

int main(int argc, char **argv)
{
  using namespace std;
  using namespace casa;

  if (argc<3) {
    cout <<String("Usage: imageconcat  \"image0 image1 image2...imageN\"  outimage")<<endl;
    exit(1);
  }
  try{

    Timer tim;
    String res[10000];
    Int nimages=split(String(argv[1]), res, 10000, String(" "));
    String outname(argv[2]);
    //cerr << "Output image will be " << outname << endl;
    Vector<String> images(nimages);
    Block<CountedPtr<PagedImage<Float> > > vim(nimages);
    for (Int k=0; k < nimages; ++k){
      images[k]=res[k];
      vim[k]=new PagedImage<Float>(images[k]);
      vim[k]->tempClose();
    }
    //cerr << "images" << images << endl;
    //PagedImage<Float> im1(argv[1]);
    CoordinateSystem cs=vim[0]->coordinates();
    //Int nchan=0;
    /* for (Int k=0; k < nimages; ++k){
      nchan+=vim[k]->shape()[3];
      //vim[k]->tempClose();
    }
    IPosition tileShape(4, vim[0]->shape()[0]/8, vim[0]->shape()[1]/8, 1, nchan/10);
    */
    //tim.mark();
    {
      ImageConcat<Float> ic(3, True);
      for (Int k=0; k < nimages; ++k){
	ic.setImage(*vim[k], True);
      }
      PagedImage<Float> out2(TiledShape(ic.shape())
			     //, tileShape)
			     , cs, outname);
      //out2.table().markForDelete();
      //out2.tempClose();
      out2.copyData(ic);
      if(ic.isMasked()){
	out2.makeMask ("mask0", True, True, False, True);
	out2.pixelMask().put(ic.getMask());
      }
     }
    //tim.show("Time taken to concatenate via image: ");
    /* for(Int k=0; k < nimages; ++k){
      //vim[k]->tempClose();
    }
   
    */
    /*
    tim.mark();
      {
	LatticeConcat<Float> lc(3, False);
	for(Int k=0; k < nimages; ++k){
	  lc.setLattice(*vim[k]);
	}
	PagedImage<Float> out(TiledShape(lc.shape(), tileShape), cs, "Output.image");
	out.copyData(lc);
      }
    tim.show("Time taken to concatenate via lattice: ");
    */
    /*
    tim.mark();
    {
      IPosition imshp=vim[0]->shape();
      
      imshp[3]=nchan;
      Int startchan=0;
      Int endchan=vim[0]->shape()[3];
      IPosition blc(4, 0, 0, 0, 0);
      IPosition trc=imshp-1;
      trc[3]=endchan-1;
      PagedImage<Float> out3(TiledShape(imshp)
			     //, tileShape)
			     , cs, "Output3.image");
      ImageRegion outreg=out3.makeMask("mask0", False, False);
      LCRegion& outmask=outreg.asMask();
      
      for (Int k=0; k < nimages; ++k){
	trc[3]=blc[3]+vim[k]->shape()[3]-1;
	//cerr << "blc trc " << blc << "  " << trc << endl;
	Slicer sl(blc, trc, Slicer::endIsLast);
	SubImage<Float> subIm(out3, sl, True);
	subIm.copyData(*vim[k]);
	if( (vim[k]->getDefaultMask()) != String("")){	
	  SubLattice<Bool> subMask(outmask, sl, True);
	  subMask.copyData(vim[k]->getRegion(vim[k]->getDefaultMask()).asMask());
	}
	//vim[k]->tempClose();
	blc[3]=trc[3]+1;
      }
    }
    tim.show("Time taken to concatenate via lattice-slice copy: ");
    */

     

  }catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
    exit(1);
  }
  exit(0);
};
