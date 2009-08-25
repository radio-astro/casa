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
// #include <images/Images/GaussianFitter.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageStatistics.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Regions/RegionManager.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <components/ComponentModels/Flux.h>

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

#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentShape.h>
#include <casa/namespace.h>

#include <components/ComponentModels/GaussianShape.h>
#include <components/ComponentModels/DiskShape.h>
#include <components/ComponentModels/PointShape.h>



Double _stringToDouble(String& string) {
    istringstream instr(string);
    Double var;
    instr >> var;
    return var;
}


// process the 'box' command line argument and return the associated region as
// a record.
ImageRegion _processBox(const ImageInterface<Float>& image, const String& box) {
    Vector<String> boxParts = stringToVector(box);
    ImageRegion imRegion;
    if (boxParts.size() != 4) {
        return imRegion;
    }
    IPosition imShape = image.shape(); 
    Vector<Double> blc(imShape.nelements());
    Vector<Double> trc(imShape.nelements());

    for (Int i=0; i<imShape.nelements(); ++i) {
        blc[i] = 0;
        trc[i] = imShape[i] - 1;
    }
    
    // TODO: locate direction coordinates (axes) for more general case when
    // position axes are not 0 and 1
    blc[0] = _stringToDouble(boxParts[0]);
    blc[1] = _stringToDouble(boxParts[1]);
    trc[0] = _stringToDouble(boxParts[2]);
    trc[1] = _stringToDouble(boxParts[3]);

    LCBox lcBox(blc, trc, imShape);
    WCBox wcBox(lcBox, image.coordinates());
    imRegion = ImageRegion(wcBox);
    return imRegion;
} 

ImageRegion _processRegionName(const String& imagename, const String& region) {
    PagedImage<Float> image(imagename);
    ImageRegion imRegion = image.getRegion(region);
    return imRegion;
}

bool _processInputs(Int argc, char *argv[]) {
    Input input(1);
    input.version("$ID:$");
    input.create("imagename");
    input.create("box");
    input.create("region");
    input.create("ngauss");
    input.create("chan");
    input.create("stokes");
    input.readArguments(argc, argv);
    String imagename = input.getString("imagename");
    if (imagename.size() == 0) {
        cerr << "command line option imagename must be specified" << endl;
        return false;
    }
    String box = input.getString("box");
    String region = input.getString("region");
    ImageRegion imRegion;
    PagedImage<Float> image(imagename);
    Bool doRegion = False;
    if (box == "") {
        // box not specified, check for saved region
        if (region == "") {
            // neither region nor box specified, use entire image
        }
        else {
            // get the ImageRegion from the specified region
            imRegion = _processRegionName(imagename, region);
            doRegion = True;
        }

    }
    else if (box.freq(",") != 3) {
        cerr << "command line box not specified correctly" << endl;
        return false;
    }
    else {
        if (region != "") {
            cout << "both box and region specified, box will be used" << endl;
        }
        // we have been given a box by the user and it is specified correctly
        imRegion = _processBox(image, box);
        doRegion = True;

    }
    Record rec;
    if (box != "" && region != "") {
        rec = Record(imRegion.toRecord(""));
    }

    Int ngauss = input.getInt("ngauss");
    // input.getInt() will default to 0 if param not specified
    Int chan = input.getInt("chan");

    ComponentList compList;
    Array<Float> residPixels;
    Array<Bool> residMask;
    Bool converged;
    String stokesString = input.getString("stokes");
    if (stokesString == "") {
        stokesString = "I";
    }
    String mask;
    // make this ngauss when we get that far
    Vector<String> models(1);
    models[0] = "gaussian";
    Vector<String> fixedparams;
    Record estimate; 
    Vector<Float> includepix, excludepix;
    ImageAnalysis myImage(&image);
    myImage.fitsky(
        residPixels, residMask, compList, converged,
        rec,
        chan, stokesString, mask, models,
        estimate, fixedparams, includepix, excludepix
    );   

    Flux<Double> flux;
    for(Int k=0; k<compList.nelements(); ++k) {
        SkyComponent skyComp = compList.component(k);
        flux = skyComp.flux();
        cout << "flux val " << flux.value(Stokes::I) << endl;
    }
    Vector<Quantity> fluxQuant;
    compList.getFlux(fluxQuant, 0);
    cout << " flux from comp list " << fluxQuant << endl;
    Vector<String> polarization = compList.getStokes(0);
    cout << "stokes from comp list " << polarization << endl;
    const ComponentShape* compShape = compList.getShape(0);
    String compType = ComponentType::name(compShape->type());
    cout << "component type " << compType << endl;

    MDirection mdir = compList.getRefDirection(0);
    Quantity lat = mdir.getValue().getLat("rad");
    Quantity longitude = mdir.getValue().getLong("rad");

    Quantity elat = compShape->refDirectionErrorLat();
    Quantity elong = compShape->refDirectionErrorLong();
    cout << " RA " << MVTime(lat).string(MVTime::TIME, 11) << " DEC "
        << MVAngle(longitude).string(MVAngle::ANGLE_CLEAN, 11) << endl;
    cout << "RA error " << MVTime(elat).string(MVTime::TIME, 11) << " Dec error " 
        << MVAngle(elong).string(MVAngle::ANGLE, 11) << endl;

    cout << "RA error rads" << elat << " Dec error rad " << elong << endl;



    if (compShape->type() == ComponentType::GAUSSIAN) {
        // print gaussian stuff
        Quantity bmaj = (static_cast<const GaussianShape *>(compShape))->majorAxis();
        Quantity bmin = (static_cast<const GaussianShape *>(compShape))->minorAxis();
        Quantity bpa  = (static_cast<const GaussianShape *>(compShape))->positionAngle();
        Quantity emaj = (static_cast<const GaussianShape *>(compShape))->majorAxisError();
        Quantity emin = (static_cast<const GaussianShape *>(compShape))->minorAxisError();
        Quantity epa  = (static_cast<const GaussianShape *>(compShape))->positionAngleError();
        cout << "bmaj " << bmaj << " bmin " << bmin << " bpa " << bpa << endl;
        cout << "emaj " << emaj << " emin " << emin << " epa " << epa << endl;
    } 
    
    // this needs to be cleaned up, it was done as an intro to the casa dev system

    ImageInterface<Float>* imagePtr = &image;
    SubImage<Float> subim;
    if (doRegion) {
        subim = SubImage<Float>(image, imRegion, False);
        imagePtr = &subim;
    }
    LogIO logio;
    ImageStatistics<Float> stats(*imagePtr, logio, True, False);
    IPosition minpos, maxpos;
    stats.getMinMaxPos(minpos, maxpos);
    cout << " min pos " << minpos << " maxpos " << maxpos << endl; 
    Array<Double> sumsquared;
    stats.getStatistic (sumsquared, LatticeStatsBase::SUMSQ);
    cout << "sumsq " << sumsquared << endl;
    // get single channel
    IPosition start(4, 0, 0, 0, 0);
    IPosition end = imagePtr->shape() - 1;
    IPosition stride(4, 1, 1, 1, 1);
    cout << "start " << start << " end " << end << endl;
    // channel 38
    start[3] = chan;
    end[3] = chan; 
    cout << "start " << start << " end " << end << endl;

    Slicer sl(start, end, stride, Slicer::endIsLast);
    SubImage<Float> subim2(*imagePtr, sl, False);
    
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



