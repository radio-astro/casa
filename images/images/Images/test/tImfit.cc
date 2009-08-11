//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
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
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $


#include <casa/Inputs/Input.h>
#include <images/Images/GaussianFitter.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageStatistics.h>
#include <images/Regions/RegionManager.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/UnitMap.h>

#include <casa/Arrays/ArrayUtil.h>
#include <casa/iostream.h>
#include <casa/namespace.h>

#include <images/Regions/WCUnion.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/ImageRegion.h>
#include <lattices/Lattices/LCRegion.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>

#include <casa/namespace.h>



Double _stringToDouble(String& string) {
  istringstream instr(string);
  Double var;
  instr >> var;
  return var;
}


// process the 'box' command line arguement and return the associated region as
// a record.
void _processBox(const String& box, Vector<Double>& blc, Vector<Double>& trc) {
    Vector<String> boxParts = stringToVector(box);
    if (boxParts.size() != 4) {
        return;
    }
    blc[0] = _stringToDouble(boxParts[0]);
    blc[1] = _stringToDouble(boxParts[1]);
    trc[0] = _stringToDouble(boxParts[2]);
    trc[1] = _stringToDouble(boxParts[3]);
    return;
} 

bool _processInputs(Int argc, char *argv[]) {
    Input input(1);
    input.version("$ID:$");
    input.create("imagename");
    input.create("box");
    input.readArguments(argc, argv);
    String imagename = input.getString("imagename");
    if (imagename.size() == 0) {
        cerr << "command line option imagename must be specified" << endl;
        return false;
    }
    String box = input.getString("box");
    if (box.freq(",") != 3) {
        cerr << "command line box not specified correctly" << endl;
        return false;
    }
    Vector<Double> blc(2);
    Vector<Double> trc(2);

    Vector<Quantum<Double> > wblc(2);
    Vector<Quantum<Double> > wtrc(2);
    _processBox(box, blc, trc);
/*
    for (Int k=0; k<2; ++k) {
        cout << "k " << k << " blc[k] " << blc[k] << " trc[k] " << trc[k] << endl;
        wblc[k] = Quantum<Double>(blc[k], Unit("pix"));
        wtrc[k] = Quantum<Double>(trc[k], Unit("pix"));
    }
*/
    PagedImage<Float> image(imagename);
    cout << "regions " << image.regionNames() << endl;
    CoordinateSystem coordsys = image.coordinates();
    RegionManager regManager = RegionManager(coordsys);

     

    Int dirIndex = coordsys.findCoordinate(Coordinate::DIRECTION); 
    Vector<Int> pixelAxis(2);
    pixelAxis[0] = dirIndex;
    pixelAxis[1] = dirIndex + 1;
    //ImageRegion* imRegion = regManager.wbox(wblc, wtrc, pixelAxis, coordsys);




    //delete imRegion;
    ImageRegion box1 = image.getRegion(image.regionNames()[0]);
    SubImage<Float> subim(image, box1, False);
    LogIO logio;
    ImageStatistics<Float> stats(subim, logio, True, False);
    IPosition minpos, maxpos;
    stats.getMinMaxPos(minpos, maxpos);
    cout << " min pos " << minpos << " maxpos " << maxpos << endl; 
    Array<Double> sumsquared;
    stats.getStatistic (sumsquared, LatticeStatsBase::SUMSQ);
    cout << "sumsq " << sumsquared << endl;
    // get single channel
    IPosition start(4, 0, 0, 0, 0);
    IPosition end = subim.shape() - 1;
    IPosition stride(4, 1, 1, 1, 1);
    cout << "start " << start << " end " << end << endl;
    // channel 38
    start[3] = 38;
    end[3] = 38; 
    cout << "start " << start << " end " << end << endl;

    Slicer sl(start, end, stride, Slicer::endIsLast);
    SubImage<Float> subim2(subim, sl, False);
    
    stats.setNewImage(subim2);
    stats.getMinMaxPos(minpos, maxpos);
    cout << " min pos " << minpos << " maxpos " << maxpos << endl; 

    stats.getStatistic (sumsquared, LatticeStatsBase::SUMSQ);
    cout << "sumsq " << sumsquared << endl;
  

/*
    GaussianFitter myGF(image, regionRecord);
    const IPosition latticeShape(image.shape());
    RegionManager *regionManager_p;
    regionManager_p = new casa::RegionManager();
*/
    return true;
}

Int main(Int argc, char *argv[]) {
    if (! _processInputs(argc, argv)) {
        return 1;
    }
    return 0;
}



