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
//# $Id: $

#include <images/Images/ImageAnalysis.h>
#include <images/Images/ImageFitter.h>
#include <images/Images/ImageMetaData.h>
#include <images/Images/ImageStatistics.h>
#include <images/Regions/RegionManager.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <components/ComponentModels/Flux.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/IO/FitterEstimatesFileParser.h>
#include <casa/Arrays/ArrayUtil.h>

#include <images/Regions/WCUnion.h>
#include <images/Regions/WCBox.h>
#include <lattices/Lattices/LCRegion.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>

#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentShape.h>

#include <components/ComponentModels/GaussianShape.h>

namespace casa {

    ImageFitter::ImageFitter(
        const String& imagename, const String& box, const String& region,
        const uInt chanInp, const String& stokes,
        const String& maskInp, const Vector<Float>& includepix,
        const Vector<Float>& excludepix, const String& residualInp,
        const String& modelInp, const String& estimatesFilename
    ) : chan(chanInp), stokesString(stokes), mask(maskInp),
		residual(residualInp),model(modelInp), includePixelRange(includepix),
		excludePixelRange(excludepix), estimates(), fixed(0) {
        itsLog = new LogIO();
        *itsLog << LogOrigin("ImageFitter", "constructor");
        _construct(imagename, box, region, estimatesFilename);
    }

    ImageFitter::~ImageFitter() {
        delete itsLog;
        delete image;
    }

    ComponentList ImageFitter::fit() const {
        *itsLog << LogOrigin("ImageFitter", "fit");
        Array<Float> residPixels;
        Array<Bool> residMask;
        Bool converged;
        uInt ngauss = estimates.nelements() > 0 ? estimates.nelements() : 1;
        Vector<String> models(ngauss);
        models.set("gaussian");
        cout << "ngauss " << models.size() << endl;
        cout << "n estiamtes " << estimates.nelements() << endl;
        ImageAnalysis myImage(image);
        Bool fit = True;
        Bool deconvolve = False;
        Bool list = True;
        // TODO make param passed to fitsky a ComponentList so this crap doesn't
        // have to be done.
        String errmsg;
        Record estimatesRecord;
        estimates.toRecord(errmsg, estimatesRecord);
        Record rec = Record(imRegion.toRecord(""));
        ComponentList compList = myImage.fitsky(
            residPixels, residMask, converged, rec,
            chan, stokesString, mask, models,
            estimatesRecord, fixed, includePixelRange,
            excludePixelRange, fit, deconvolve, list,
            residual, model
        );
        cout << " fitted comp list elements " << compList.nelements() << endl;
        Flux<Double> flux;
        Vector<Quantity> fluxQuant;
        for(uInt k=0; k<compList.nelements(); k++) {
            SkyComponent skyComp = compList.component(k);
            flux = skyComp.flux();
            cout << "flux val " << flux.value(Stokes::I) << endl;
            compList.getFlux(fluxQuant, k);
            cout << " flux from comp list " << fluxQuant << endl;
            Vector<String> polarization = compList.getStokes(k);
            cout << "stokes from comp list " << polarization << endl;
            const ComponentShape* compShape = compList.getShape(k);
            String compType = ComponentType::name(compShape->type());
            cout << "component type " << compType << endl;
            MDirection mdir = compList.getRefDirection(k);

            Quantity lat = mdir.getValue().getLat("rad");
            Quantity longitude = mdir.getValue().getLong("rad");

            Vector<Double> world(4,0), pixel(4,0);
            image->coordinates().toWorld(world, pixel);

            world[0] = longitude.getValue();
            world[1] = lat.getValue();
            if (image->coordinates().toPixel(pixel, world)) {
        	    cout << "max pixel position " << pixel << endl;
            }
            else {
        	    cerr << "unable to convert world to pixel" << endl;
            }

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
        }
        return compList;
   /*
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


//    GaussianFitter myGF(image, regionRecord);
//    const IPosition latticeShape(image.shape());
//    RegionManager *regionManager_p;
//    regionManager_p = new casa::RegionManager();
*/
    }

    void ImageFitter::_construct(
        const String& imagename, const String& box, const String& region,
        const String& estimatesFilename
    ) {
        if (imagename.empty()) {
            *itsLog << "imagename cannot be blank" << LogIO::EXCEPTION;
        }

        // Register the functions to create a FITSImage or MIRIADImage object.
        FITSImage::registerOpenFunction();
        MIRIADImage::registerOpenFunction();

        ImageUtilities::openImage(image, imagename, *itsLog);
        if (image == 0) {
            throw(AipsError("Unable to open image " + imagename));
        }
        _doRegion(box, region);
        _checkImageParameterValidity();
        if(estimatesFilename.empty()) {
        	*itsLog << LogIO::NORMAL << "No estimates file specified, so will attempt to find and fit one gaussian."
        		<< LogIO::POST;
        }
        else {
        	FitterEstimatesFileParser parser(estimatesFilename, *image);
        	estimates = parser.getEstimates();
        	fixed = parser.getFixed();
        	Record rec;
        	String errmsg;
        	estimates.toRecord(errmsg, rec);
        	*itsLog << LogIO::NORMAL << "File " << estimatesFilename << " has " << estimates.nelements()
        		<< " specified, so will attempt to fit that many gaussians " << LogIO::POST;
        }
    }

    void ImageFitter::_checkImageParameterValidity() const {
        *itsLog << LogOrigin("ImageFitter", "_checkImageParameterValidity");
        String error;
        ImageMetaData imageProps(*image);
        if (imageProps.hasPolarizationAxis() && imageProps.hasSpectralAxis()) {
            if (! imageProps.areChannelAndStokesValid(error, chan, stokesString)) {
                *itsLog << error << LogIO::EXCEPTION;
            }
        }
    } 

    void ImageFitter::_doRegion(const String& box, const String& region) {
        if (box == "") {
            // box not specified, check for saved region
            if (region == "") {
                // neither region nor box specified, use entire 2-D plane
                IPosition imShape = image->shape();
                Vector<Int> dirNums = ImageMetaData(*image).directionAxesNumbers();
                Vector<Int> dirShape(imShape[dirNums[0]], imShape[dirNums[1]]);
                *itsLog << LogIO::NORMAL << "Neither box nor region specified, "
                    << "so entire plane of " << dirShape[0] << " x "
                    << dirShape[1] << "  will be used" << LogIO::POST;
                ostringstream boxStream;
                boxStream << "0, 0, " << dirShape[0] << ", " << dirShape[1];
                _processBox(String(boxStream));
            }
            else {
                // get the ImageRegion from the specified region
                imRegion = image->getRegion(region);
            }

        }
        else if (box.freq(",") != 3) {
            *itsLog << "box not specified correctly" << LogIO::EXCEPTION;
        }
        else {
            if (region != "") {
                *itsLog << "both box and region specified, box will be used" << LogIO::WARN;
            }
            // we have been given a box by the user and it is specified correctly
            _processBox(box);
        }
    } 

    void ImageFitter::_processBox(const String& box) {
        Vector<String> boxParts = stringToVector(box);
        if (boxParts.size() != 4) {
            return;
        }
        IPosition imShape = image->shape(); 
        Vector<Double> blc(imShape.nelements());
        Vector<Double> trc(imShape.nelements());

        for (uInt i=0; i<imShape.nelements(); ++i) {
            blc[i] = 0;
            trc[i] = imShape[i] - 1;
        }
    
        Vector<Int> dirNums = ImageMetaData(*image).directionAxesNumbers();
        blc[dirNums[0]] = String::toDouble(boxParts[0]);
        blc[dirNums[1]] = String::toDouble(boxParts[1]);
        trc[dirNums[0]] = String::toDouble(boxParts[2]);
        trc[dirNums[1]] = String::toDouble(boxParts[3]);

        LCBox lcBox(blc, trc, imShape);
        WCBox wcBox(lcBox, image->coordinates());
        imRegion = ImageRegion(wcBox);
    }
}

