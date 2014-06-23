//# tImageFitter.cc:  test the PagedImage class
//# Copyright (C) 1994,1995,1998,1999,2000,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or(at your option)
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


#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <images/Images/FITSImage.h>
#include <casa/namespace.h>
#include <lattices/Lattices/LatticeFractile.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <casa/OS/EnvVar.h>

void writeTestString(const String& test) {
    cout << "\n" << "*** " << test << " ***" << endl;
}


int main() {
	Bool ok = True;
	try {
         String casapath = EnvironmentVariable::get("CASAPATH");
         if (casapath.empty()) {
            cerr << "CASAPATH env variable not defined. Can't find fixtures. Did you source the casainit.(c)sh file?" << endl;
            return 1;
        }

        String *parts = new String[2];
        split(casapath, parts, 2, String(" "));
        String datadir = parts[0] + "/data/regression/unittest/imageanalysis/ImageAnalysis/";
        delete [] parts;
		writeTestString("Verify fix for CAS-2195: error if image has no direction coordinate but does have linear coordiante");
		FITSImage* fits = new FITSImage(datadir + "/linearCoords.fits");
		shared_ptr<casa::ImageInterface<float> > imgPtr( fits );
		ImageAnalysis ia(imgPtr );
        {
            // CAS-2533
            PagedImage<Float>* img = new PagedImage<Float>(datadir + "/CAS-2533.im");
            shared_ptr<casa::ImageInterface<float> > imgPtr2( img );
            ImageAnalysis analysis(imgPtr2);

            Vector<casa::Double> wxv(2);
            Vector<casa::Double> wyv(2);
            Vector<casa::Float> z_xval;
            Vector<casa::Float> z_yval;

            wxv[0] = 4.63641;
            wxv[1] = 4.63639;
            wyv[0] = -0.506297;
            wyv[1] = -0.506279;

            bool ok = analysis.getFreqProfile(
                wxv, wyv, z_xval, z_yval,
                "world", "radio velocity",
                0, -1, 0, "", "LSRK"
            );
            AlwaysAssert(ok, AipsError);
            AlwaysAssert(fabs(1-z_xval[0]/137.805) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[1]/133.631) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[2]/129.457) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[3]/125.283) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[4]/121.109) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[5]/116.935) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[6]/112.761) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[7]/108.587) < 1e-5, AipsError); 

            AlwaysAssert(fabs(1-z_yval[0]/-0.146577) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[1]/-0.244666) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[2]/-0.184397) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[3]/0.0869152) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[4]/-0.43336) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[5]/-0.145391) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[6]/-0.0924785) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[7]/-0.131597) < 1e-5, AipsError); 
        }
        {
            // wavelength output
            PagedImage<Float>* img = new PagedImage<Float>(datadir + "/CAS-2533.im");
            shared_ptr<casa::ImageInterface<float> > imgPtr( img );
            ImageAnalysis analysis(imgPtr);

            Vector<casa::Double> wxv(2);
            Vector<casa::Double> wyv(2);
            Vector<casa::Float> z_xval;
            Vector<casa::Float> z_yval;

            wxv[0] = 4.63641;
            wxv[1] = 4.63639;
            wyv[0] = -0.506297;
            wyv[1] = -0.506279;

            bool ok = analysis.getFreqProfile(
                wxv, wyv, z_xval, z_yval,
                "world", "wavelength",
                0, -1, 0, "", "LSRK"
            );

            AlwaysAssert(ok, AipsError);
            AlwaysAssert(fabs(1-z_xval[0]/212.115) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[1]/212.112) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[2]/212.109) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[3]/212.106) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[4]/212.103) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[5]/212.100) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[6]/212.097) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_xval[7]/212.094) < 1e-5, AipsError); 

            AlwaysAssert(fabs(1-z_yval[0]/-0.146577) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[1]/-0.244666) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[2]/-0.184397) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[3]/0.0869152) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[4]/-0.43336) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[5]/-0.145391) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[6]/-0.0924785) < 1e-5, AipsError); 
            AlwaysAssert(fabs(1-z_yval[7]/-0.131597) < 1e-5, AipsError); 
        }
        {
            writeTestString("histograms() test");
            FITSImage* image = new FITSImage(datadir + "/histogram_test.fits");
            shared_ptr<casa::ImageInterface<float> > imgPtr( image );
            ImageAnalysis ia( imgPtr);
            Record regionRec;
            String mask;
            Vector<Int> axes(0);
            Int nbins = 25;
            Vector<Double> includepix(0);
            Bool gauss = False;
            Bool cumu = False;
            Bool log = False;
            Bool list = False;
            Vector<Int> size(2);
            size[0] = 600;
            size[1] = 450;
            Bool force = False;
            Bool disk = False;
            Bool extendMask = False;
            Record histOut = ia.histograms(
            	axes, regionRec, mask, nbins, includepix,
            	gauss, cumu, log, list, force, disk, extendMask
            );
            Array<Float> values = histOut.asArrayFloat("values");
            Array<Float> counts = histOut.asArrayFloat("counts");
            AlwaysAssert((Int)values.size() == nbins, AipsError);
            AlwaysAssert((Int)counts.size() == nbins, AipsError);
            Float expValue = -96;
            IPosition axisPath = IPosition::makeAxisPath(values.ndim());
            ArrayPositionIterator iter(values.shape(), axisPath, False);
            while (! iter.pastEnd()) {
                const IPosition pos = iter.pos();
                AlwaysAssert(values(pos) == expValue, AipsError);
                Float expCount = 0;
                if (pos[0] == 0 || pos[0] == nbins-1) {
                    expCount = 1;
                }
                else if (pos[0] == Int((nbins-1)/2)) {
                    expCount = 48;
                }
                cout << "*** count " << counts(pos) << " exp " << expCount << endl;
                AlwaysAssert(counts(pos) == expCount, AipsError);
                iter.next();
                expValue += 8;
            }
        }

        {
        	writeTestString("CAS-2359 verification");
        	CoordinateSystem csys = CoordinateUtil::defaultCoords3D();
        	TempImage<Float>* x = new TempImage<Float>(TiledShape(IPosition(3,10,10,1)), csys);
        	Array<Float> data(IPosition(3,10,10,1));
        	data.set(0);
        	shared_ptr<casa::ImageInterface<float> > imgPtr( x );
        	ImageAnalysis ia(imgPtr);
        	Vector<Double> xy;
        	Vector<Float> zxaxis, zyaxis;
        	try {
        		// should throw exception, xy.size() != 2
        		ia.getFreqProfile(xy, zxaxis, zyaxis);
        		AlwaysAssert(False, AipsError);
        	}
        	catch (AipsError x) {
        		cout << "Exception thrown as expected: " << x.getMesg() << endl;
        	}
        	xy.resize(2);
        	xy.set(1.0);
    		AlwaysAssert(! ia.getFreqProfile(xy, zxaxis, zyaxis),AipsError);
        }
        {
        	/*
            writeTestString("CAS-4230 test");
        	PagedImage<Float> image("uvtaper.cube.hsmooth.image");
        	ImageAnalysis ia (&image);
        	cout << "shape " << image.shape() << endl;
        	IPosition blc(4, 0 , 0, 0, 8210);
        	IPosition trc(4, 255, 255, 0, 8475);

        	LCBox box(blc, trc, image.shape());
        	Record statsout;
        	Record regionRec = box.toRecord("");
        	Vector<Int> axes(0);
        	String mask;
        	Vector<String> plotstats(0);
        	Vector<Float> includepix(0), excludepix(0);
        	Int nx = 1, ny = 1;
        	Bool list = False, force = False, disk = False, robust = True;
        	Bool verbose = True, extendMask = False;
        	vector<String> *messageStore = 0;

        	ia.statistics(
        		statsout, axes, regionRec, mask, plotstats,
        		includepix, excludepix, "", nx, ny, list,
        		force, disk, robust, verbose, extendMask, messageStore
        	);
        	*/
        	//FITSImage image("cas-4230.fits");
        	// LatticeFractile<Float>::unmaskedFractile (image, 0.5);

        }

        /*{
        	cout << "*** test regrid by velocity ***" << endl;
        	TempImage<Float> input(
        		TiledShape(IPosition(3, 10, 10, 10)),
        		CoordinateUtil::defaultCoords3D()
        	);
        	CoordinateSystem csysTemp = CoordinateUtil::defaultCoords3D();
        	SpectralCoordinate spec = csysTemp.spectralCoordinate();

        	spec.setRestFrequency(4.0e9);
        	spec.setReferenceValue(Vector<Double>(1, 4.0e9));
        	spec.setReferencePixel(Vector<Double>(1, 4.5));
        	csysTemp.replaceCoordinate(spec, 1);
        	TempImage<Float> templ(TiledShape(IPosition(3, 10, 10, 10)), csysTemp);
        	ImageAnalysis ia(&input);
        	ImageInterface<Float> *output = ia.regrid(
			"", &templ, "linear", True,
			Vector<Int>(1, 2), Record(),
        		"", 10, False, True, False, False, False
        	);
        	SpectralCoordinate specOut = output->coordinates().spectralCoordinate();
        	SpectralCoordinate specTemp = templ.coordinates().spectralCoordinate();
        	SpectralCoordinate specIn = input.coordinates().spectralCoordinate();
        	AlwaysAssert(
        		specOut.referencePixel()[0] == specTemp.referencePixel()[0],
        		AipsError
        	);
        	Double outRefVel, tempRefVel;
        	specOut.pixelToVelocity(outRefVel, specOut.referencePixel()[0]);
        	specTemp.pixelToVelocity(tempRefVel, specTemp.referencePixel()[0]);
        	AlwaysAssert(outRefVel == tempRefVel, AipsError);
        	Double inFreq;
        	specIn.velocityToFrequency(inFreq, outRefVel);
        	AlwaysAssert(specOut.referenceValue()[0] == inFreq, AipsError);
        	Double outVel1;
        	specOut.pixelToVelocity(outVel1, specOut.referencePixel()[0]+1);
        	Double outVelInc = outVel1 - outRefVel;
        	Double tempVel1;
        	specTemp.pixelToVelocity(tempVel1, specTemp.referencePixel()[0]+1);
        	Double tempVelInc = tempVel1 - tempRefVel;
        	AlwaysAssert(outVelInc == tempVelInc, AipsError);
        	cout << tempVelInc << " " << outVelInc << endl;
        }*/

        cout << "ok" << endl;
	}
    catch (AipsError x) {
    	ok = False;
        cerr << "Exception caught: " << x.getMesg() << endl;
    }
	return ok ? 0 : 1;
}

