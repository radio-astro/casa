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


#include <casa/BasicMath/Math.h>
#include <components/ComponentModels/ComponentList.h>
#include <images/Images/ImageFitter.h>
#include <measures/Measures/MDirection.h>
#include <components/ComponentModels/ComponentShape.h>
#include <images/Images/ImageAnalysis.h>
#include <casa/BasicSL/Constants.h>
#include <casa/OS/Directory.h>
#include <casa/namespace.h>

#include <sys/types.h>
#include <unistd.h>


void writeTestString(const String& test) {
    cout << "\n" << "*** " << test << " ***" << endl;
}

void checkImage(
		const String& gotImage, const String& expectedImage,
		const String& differenceImage
	) {
    ImageAnalysis ia;
    ia.open(gotImage);
    String expr = "\"" + gotImage + "\" - \"" + expectedImage + "\"";
    ia.imagecalc(differenceImage, expr, True);
    ia.open(differenceImage);
    Record stats;
    Vector<Int> axes(2);
    axes[0] = 0;
    axes[1] = 1;
    Record region;
    Vector<String> plotstats(0);
    ia.statistics(stats, axes, region, "", plotstats, Vector<Float>(0), Vector<Float>(0));

    Array<Double> sumArray = stats.asArrayDouble("sum");
    vector<double> sum;
    sumArray.tovector(sum);
    AlwaysAssert(sum[0] == 0, AipsError);
}

int main() {
    pid_t pid = getpid();
    ostringstream os;
    os << "tImageFitter_tmp_" << pid;
    String dirName = os.str();
	Directory workdir(dirName);
    try {
    	const Double DEGREES_PER_RADIAN = 180/C::pi;
        Double arcsecsPerRadian = DEGREES_PER_RADIAN*3600;
        String test;
        {
            writeTestString(
                "test fitter using all available image pixels with model with no noise"
            );
            ImageFitter fitter = ImageFitter("gaussian_model.fits");
            ComponentList compList = fitter.fit();
            Vector<Quantity> flux;
            compList.getFlux(flux,0);
            // I stokes flux test
            AlwaysAssert(near(flux(0).getValue(), 60318.5801, 1e-4), AipsError);
            // Q stokes flux test
            AlwaysAssert(flux(1).getValue() == 0, AipsError);
            MDirection direction = compList.getRefDirection(0);
            AlwaysAssert(near(direction.getValue().getLong("rad").getValue(), 0.000213318, 1e-5), AipsError);
            AlwaysAssert(near(direction.getValue().getLat("rad").getValue(), 1.939254e-5, 1e-5), AipsError);

            Vector<Double> parameters = compList.getShape(0)->parameters();

            Double majorAxis = arcsecsPerRadian*parameters(0);
            AlwaysAssert(near(majorAxis, 23.548201, 1e-7), AipsError);

            Double minorAxis = arcsecsPerRadian*parameters(1);
            AlwaysAssert(near(minorAxis, 18.838560, 1e-7), AipsError);

            Double positionAngle = DEGREES_PER_RADIAN*parameters(2);
            AlwaysAssert(near(positionAngle, 120.0, 1e-7), AipsError);
        }
        String noisyImage = "gaussian_model_with_noise.fits";
        {
            writeTestString(
                "test fitter using all available image pixels with model with noise"
            );
            ImageFitter fitter = ImageFitter(noisyImage);
            ComponentList compList = fitter.fit();
            Vector<Quantity> flux;
            compList.getFlux(flux,0);
            // I stokes flux test
            AlwaysAssert(near(flux(0).getValue(), 60340.7606, 1e-5), AipsError);
            // Q stokes flux test
            AlwaysAssert(flux(1).getValue() == 0, AipsError);
            MDirection direction = compList.getRefDirection(0);
            AlwaysAssert(near(direction.getValue().getLong("rad").getValue(), 0.000213381, 1e-5), AipsError);
            AlwaysAssert(near(direction.getValue().getLat("rad").getValue(), 1.93571e-05, 1e-5), AipsError);

            Vector<Double> parameters = compList.getShape(0)->parameters();

            Double majorAxis = arcsecsPerRadian*parameters(0);
            AlwaysAssert(near(majorAxis, 23.546913, 1e-7), AipsError);

            Double minorAxis = arcsecsPerRadian*parameters(1);
            AlwaysAssert(near(minorAxis, 18.876406, 1e-7), AipsError);

            Double positionAngle = DEGREES_PER_RADIAN*parameters(2);
            AlwaysAssert(near(positionAngle, 119.897741, 1e-7), AipsError);
        }
        {
            writeTestString(
                "test fitter using a box region with model with noise"
            );
            ImageFitter fitter = ImageFitter(noisyImage, "130,89,170,129");
            ComponentList compList = fitter.fit();
            Vector<Quantity> flux;
            compList.getFlux(flux,0);
            // I stokes flux test
            AlwaysAssert(near(flux(0).getValue(), 60323.3212, 1e-5), AipsError);
            // Q stokes flux test
            AlwaysAssert(flux(1).getValue() == 0, AipsError);
            MDirection direction = compList.getRefDirection(0);
            AlwaysAssert(near(direction.getValue().getLong("rad").getValue(), 0.000213372, 1e-5), AipsError);
            AlwaysAssert(near(direction.getValue().getLat("rad").getValue(), 1.93593e-05, 1e-5), AipsError);

            Vector<Double> parameters = compList.getShape(0)->parameters();

            Double majorAxis = arcsecsPerRadian*parameters(0);
            AlwaysAssert(near(majorAxis, 23.545291, 1e-7), AipsError);

            Double minorAxis = arcsecsPerRadian*parameters(1);
            AlwaysAssert(near(minorAxis, 18.866377, 1e-7), AipsError);

            Double positionAngle = DEGREES_PER_RADIAN*parameters(2);
            AlwaysAssert(near(positionAngle, 119.806997, 1e-7), AipsError);
        }
        {
            // test fitter using an includepix (i=0) and excludepix (i=1) range with model with noise
            for (uInt i=0; i<3; i++) {
                String mask;
                Vector<Float> includepix, excludepix;
                switch (i) {
                    case 0:
                        writeTestString("test using includepix range");
                        includepix.resize(2);
                        includepix(0) = 40;
                        includepix(1) = 121;
                        mask = "";
                        break;
                    case 1:
                        writeTestString("test using excludepix range");
                        includepix.resize(0);
                        excludepix.resize(2);
                        excludepix(0) = -10;
                        excludepix(1) = 40;
                        mask = "";
                        break;
                    case 2:
                        includepix.resize(0);
                        excludepix.resize(0);
                        mask = "\"" + noisyImage + "\">40";
                        writeTestString("test using LEL mask " + mask);
                        break;
                }
                cout << "running " <<  i << endl;
                ImageFitter fitter = ImageFitter(
                        noisyImage, "", "", 1, 0, "I", mask, includepix, excludepix
                );
                cout << "ran " << i << endl;
                ComponentList compList = fitter.fit();
                Vector<Quantity> flux;
                compList.getFlux(flux,0);
                // I stokes flux test
                cout << "flux " << flux(0).getValue() << endl;
                AlwaysAssert(near(flux(0).getValue(), 60354.3232, 1e-5), AipsError);
                // Q stokes flux test
                AlwaysAssert(flux(1).getValue() == 0, AipsError);
                MDirection direction = compList.getRefDirection(0);
                AlwaysAssert(near(direction.getValue().getLong("rad").getValue(), 0.000213391, 1e-5), AipsError);
                AlwaysAssert(near(direction.getValue().getLat("rad").getValue(), 1.93449e-05, 1e-5), AipsError);

                Vector<Double> parameters = compList.getShape(0)->parameters();

                Double majorAxis = arcsecsPerRadian*parameters(0);
                AlwaysAssert(near(majorAxis, 23.541712, 1e-7), AipsError);

                Double minorAxis = arcsecsPerRadian*parameters(1);
                AlwaysAssert(near(minorAxis, 18.882029, 1e-7), AipsError);

                Double positionAngle = DEGREES_PER_RADIAN*parameters(2);
                AlwaysAssert(near(positionAngle, 119.769648, 1e-7), AipsError);
            }
        }
        {

            workdir.create();
            String residImage = dirName + "/residualImage";
            String modelImage = dirName + "/modelImage";
            String residDiff = dirName + "/residualImage.diff";
            String modelDiff = dirName + "/modelImage.diff";
            ImageFitter fitter(
            	noisyImage, "100,100,200,200", "", 1, 0, "I", "",
            	Vector<Float>(0), Vector<Float>(0), residImage,
            	modelImage
            );
            fitter.fit();
            checkImage(
            	residImage, "gaussian_model_with_noise_resid.fits",
            	residDiff
            );
            checkImage(
             	modelImage, "gaussian_model_with_noise_model.fits",
             	modelDiff
            );
            workdir.removeRecursive();
        }
        cout << "ok" << endl;
    }
    catch (AipsError x) {
    	if(workdir.exists()) {
    		workdir.removeRecursive();
    	}
        cerr << "Exception caught: " << x.getMesg() << endl;
        return 1;
    } 
    return 0;
}

