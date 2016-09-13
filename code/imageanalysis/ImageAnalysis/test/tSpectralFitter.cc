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

#include <imageanalysis/ImageAnalysis/SpectralFitter.h>

#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/FITSImage.h>
#include <images/Images/FITSQualityImage.h>
#include <casa/OS/Directory.h>
#include <casa/OS/EnvVar.h>

#include <casa/namespace.h>

#include <sys/types.h>

#include <unistd.h>
#include <iomanip>

Bool allNear (const Vector<Double>& data, const Vector<Double>& templ, Double tol=1.0e-5);

int main() {
	String *parts = new String[2];
	split(EnvironmentVariable::get("CASAPATH"), parts, 2, String(" "));
	String datadir = parts[0] + "/data/regression/viewertest/";
	delete [] parts;
	FITSQualityImage *goodQualImage = new FITSQualityImage(datadir + "3DVisTestIDL.fits", 1, 2);

	uInt retVal = 0;
	try {
		// create the analysis object
		ImageAnalysis* analysis;
		analysis = new ImageAnalysis(goodQualImage);

		// define the vector
		Vector<Double> wxv(1);
		Vector<Double> wyv(1);
		wxv = 0.523515;
		wyv = 0.52367;

		Vector<Float> z_xval;
		Vector<Float> z_yval;
		Vector<Float> z_eval;

		// get the spectral profile
		Bool ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
				"world", "wavelength", 0, 0, 0, "nm", "TOPO",
				0, 0, "0.00000000e+00Hz");
		{
			// define some variables for the fit
			Bool fitGauss(True);
			Bool fitPoly(True);
			uInt nPoly(1);
			String msg;

			// create the fitter and do a "normal" fit
			SpectralFitter *specFit=new SpectralFitter();
			ok = specFit->fit(z_xval, z_yval, z_eval, Float(556.0), Float(568.0),
					fitGauss, fitPoly, nPoly, msg);

			// ok is always True
			AlwaysAssert(ok, AipsError);

			// the fit needs to be a success
			AlwaysAssert(specFit->getStatus()==SpectralFitter::SUCCESS, AipsError);

			// check the number of iterations
			AlwaysAssert(near(specFit->getNumberIterations(), Double(17)), AipsError);

			// check the chi-square
			AlwaysAssert(near(Double(specFit->getChiSquared()), Double(124960),1.0e-05), AipsError);

			// get the lines
			const SpectralList &spcList = specFit->getList();

			// check that there are two lines
			AlwaysAssert(near(spcList.nelements(), uInt(2)), AipsError);

			// go over all elements
			Vector<Double> params, errors;
			String spTypeStr;
			for (uInt index=0; index < spcList.nelements(); index++){

				// get the element type, parameters and errors
				SpectralElement::Types spType = spcList[index]->getType();
                params.resize(spcList[index]->getOrder());
                errors.resize(spcList[index]->getOrder());
				spcList[index]->get(params);
				spcList[index]->getError(errors);

				if (spType == SpectralElement::GAUSSIAN){
					Vector<Double> refPar(3), refErr(3);
					refPar(0)=2980.73;  refPar(1)=561.674;    refPar(2)=1.14481;
					refErr(0)=22.5129;  refErr(1)=0.00982114; refErr(2)=0.010977;
					AlwaysAssert(allNear(params, refPar), AipsError);
					AlwaysAssert(allNear(errors, refErr), AipsError);
				}
				else if (spType == SpectralElement::POLYNOMIAL){
					Vector<Double> refPar(2), refErr(2);
					refPar(0)=1546.32;  refPar(1)=0.863697;
					refErr(0)=1211.75;  refErr(1)=2.15471;
					AlwaysAssert(allNear(params, refPar), AipsError);
					AlwaysAssert(allNear(errors, refErr), AipsError);
				}
			}
			//specFit->report();
			delete specFit;

		}

		// get the error profile
		ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_eval,
				"world", "wavelength", 0, 0, 0, "nm", "TOPO",
				0, 1, "0.00000000e+00Hz");
		{
			// define some variables for the fit
			Bool fitGauss(True);
			Bool fitPoly(True);
			uInt nPoly(1);
			String msg;

			// create the fitter and do the fit with the error
			SpectralFitter *specFit=new SpectralFitter();
			ok = specFit->fit(z_xval, z_yval, z_eval, Float(556.0), Float(568.0),
					fitGauss, fitPoly, nPoly, msg);

			// ok is always True
			AlwaysAssert(ok, AipsError);

			// the fit needs to be a success
			AlwaysAssert(specFit->getStatus()==SpectralFitter::SUCCESS, AipsError);

			// check the number of iterations
			AlwaysAssert(near(specFit->getNumberIterations(), Double(17)), AipsError);

			// check the chi-square
			AlwaysAssert(near(Double(specFit->getChiSquared()), Double(7.22302e+06),1.0e-05), AipsError);

			// get the lines
			const SpectralList &spcList = specFit->getList();

			// check that there are two lines
			AlwaysAssert(near(spcList.nelements(), uInt(2)), AipsError);

			// go over all elements
			Vector<Double> params, errors;
			String spTypeStr;
			for (uInt index=0; index < spcList.nelements(); index++){

				// get the element type, parameters and errors
				SpectralElement::Types spType = spcList[index]->getType();
                params.resize(spcList[index]->getOrder());
                errors.resize(spcList[index]->getOrder());
				spcList[index]->get(params);
				spcList[index]->getError(errors);

				if (spType == SpectralElement::GAUSSIAN){
					Vector<Double> refPar(3), refErr(3);
					refPar(0)=2979.51;  refPar(1)=561.675;    refPar(2)=1.14595;
					refErr(0)=21.2636;  refErr(1)=0.00955648; refErr(2)=0.0110905;
					AlwaysAssert(allNear(params, refPar), AipsError);
					AlwaysAssert(allNear(errors, refErr), AipsError);

				}
				else if (spType == SpectralElement::POLYNOMIAL){
					Vector<Double> refPar(2), refErr(2);
					refPar(0)=1599.54;  refPar(1)=0.768233;
					refErr(0)=1341.81;  refErr(1)=2.38598;
					AlwaysAssert(allNear(params, refPar), AipsError);
					AlwaysAssert(allNear(errors, refErr), AipsError);
				}
			}
			//specFit->report();
			delete specFit;
		}

		// get the the data, but in frequency (MHz) units
		ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_eval,
				"world", "frequency", 0, 0, 0, "MHz", "TOPO",
				0, 0, "0.00000000e+00Hz");
		{
			// define some variables for the fit
			Bool fitGauss(True);
			Bool fitPoly(True);
			uInt nPoly(1);
			String msg;

			// create the fitter and do the fit with the error
			SpectralFitter *specFit=new SpectralFitter();
			ok = specFit->fit(z_xval, z_yval, z_eval, Float(6.48867e+08), Float(7.1381e+08),
					fitGauss, fitPoly, nPoly, msg);


			// ok is always True
			AlwaysAssert(ok, AipsError);

			// the fit needs to be a success
			AlwaysAssert(specFit->getStatus()==SpectralFitter::SUCCESS, AipsError);

			// check the number of iterations
            AlwaysAssert(specFit->getNumberIterations() == 18, AipsError);

			// check the chi-square
			AlwaysAssert(near(Double(specFit->getChiSquared()), Double(1.33195e+12),1.0e-05), AipsError);

			// get the lines
			const SpectralList &spcList = specFit->getList();

			// check that there are two lines
			AlwaysAssert(near(spcList.nelements(), uInt(2)), AipsError);

			// go over all elements
			Vector<Double> params, errors;
			String spTypeStr;
			for (uInt index=0; index < spcList.nelements(); index++){

				// get the element type, parameters and errors
				SpectralElement::Types spType = spcList[index]->getType();
                errors.resize(spcList[index]->getOrder());
                params.resize(spcList[index]->getOrder());
				spcList[index]->get(params);
				spcList[index]->getError(errors);

				if (spType == SpectralElement::GAUSSIAN){
					Vector<Double> refPar(3), refErr(3);
					refPar(0)=-1048.06;  refPar(1)=6.89816e+08;    refPar(2)=5.52089e+06;
					refErr(0)=18.4522;  refErr(1)=87067.4; refErr(2)=93652.3;
					AlwaysAssert(allNear(params, refPar), AipsError);
					AlwaysAssert(allNear(errors, refErr), AipsError);
				}
				else if (spType == SpectralElement::POLYNOMIAL){
					Vector<Double> refPar(2), refErr(2);
					refPar(0)=1885.07;  refPar(1)=2.10936e-07;
					refErr(0)=132.992;  refErr(1)=1.976e-07;
					AlwaysAssert(allNear(params, refPar), AipsError);
					AlwaysAssert(allNear(errors, refErr), AipsError);
				}
			}
			//specFit->report();

			delete specFit;
		}

		delete analysis;
		delete goodQualImage;
        cout << "OK" << endl;
	}
	catch (AipsError x) {
		cerr << "FAIL: " << x.getMesg() << endl;
		retVal = 1;
	}
	return retVal;
}

Bool allNear (const Vector<Double>& data, const Vector<Double>& templ, Double tol){
   for (uInt i=0; i<data.nelements(); i++) {
   	if (!near(data(i), templ(i), tol)) {
   		cerr << "data differ, tol = " << tol << endl;
   		cerr << data(i) << ", " << templ(i)<< endl;
   		return False;
   	}
   }
   return True;
}

