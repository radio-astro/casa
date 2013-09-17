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
		{

            // CAS-2533
			PagedImage<Float>* img = new PagedImage<Float>(datadir + "/CAS-2533.im");
			std::tr1::shared_ptr<casa::ImageInterface<float> > imgPtr( img );
			ImageAnalysis analysis(imgPtr);

			Vector<casa::Double> wxv(2);
			Vector<casa::Double> wyv(2);
			Vector<casa::Float> z_xval;
			Vector<casa::Float> z_yval;

			wxv[0] = 4.63641;
			wxv[1] = 4.63639;
			wyv[0] = -0.506297;
			wyv[1] = -0.506279;

			// extract the spectrum in "frequency"
			// with the default units
			bool ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "frequency",
					0, -1, 0, "", "LSRK", 0, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/1.41335) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/1.41337) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/1.41339) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/1.41341) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/1.41343) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/1.41345) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/1.41347) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/1.41349) < 1e-5, AipsError);

			// check the spectrum points
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
			// CAS-2533
			PagedImage<Float>* img = new PagedImage<Float>(datadir + "/CAS-2533.im");
			std::tr1::shared_ptr<casa::ImageInterface<float> > imgPtr( img );
			ImageAnalysis analysis(imgPtr);

			Vector<casa::Double> wxv(2);
			Vector<casa::Double> wyv(2);
			Vector<casa::Float> z_xval;
			Vector<casa::Float> z_yval;

			wxv[0] = 4.63641;
			wxv[1] = 4.63639;
			wyv[0] = -0.506297;
			wyv[1] = -0.506279;

			// extract the spectrum in "frequency"
			// with the MHz as unit
			bool ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "frequency",
					0, -1, 0, "MHz", "LSRK", 0, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/1413.35) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/1413.37) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/1413.39) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/1413.41) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/1413.43) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/1413.45) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/1413.47) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/1413.49) < 1e-5, AipsError);
		}
		{
			// CAS-2533
			PagedImage<Float>* img = new PagedImage<Float>(datadir + "/CAS-2533.im");
			std::tr1::shared_ptr<casa::ImageInterface<float> > imgPtr( img );
			ImageAnalysis analysis(imgPtr);

			Vector<casa::Double> wxv(2);
			Vector<casa::Double> wyv(2);
			Vector<casa::Float> z_xval;
			Vector<casa::Float> z_yval;

			wxv[0] = 4.63641;
			wxv[1] = 4.63639;
			wyv[0] = -0.506297;
			wyv[1] = -0.506279;

			// extract the spectrum in "frequency"
			// with the Hz as unit
			bool ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "frequency",
					0, -1, 0, "Hz", "LSRK", 0, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/1.413350e+09) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/1.413370e+09) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/1.413390e+09) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/1.413410e+09) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/1.413430e+09) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/1.413450e+09) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/1.413470e+09) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/1.413490e+09) < 1e-5, AipsError);
		}
		{
			// CAS-2533
			PagedImage<Float>* img = new PagedImage<Float>(datadir + "/CAS-2533.im");
			std::tr1::shared_ptr<casa::ImageInterface<float> > imgPtr( img );
			ImageAnalysis analysis(imgPtr);

			Vector<casa::Double> wxv(2);
			Vector<casa::Double> wyv(2);
			Vector<casa::Float> z_xval;
			Vector<casa::Float> z_yval;

			wxv[0] = 4.63641;
			wxv[1] = 4.63639;
			wyv[0] = -0.506297;
			wyv[1] = -0.506279;

			// extract the spectrum in "radio velocity"
			// with default unit
			bool ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "radio velocity",
					0, -1, 0, "", "LSRK", 0, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/137.805) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/133.631) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/129.457) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/125.283) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/121.109) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/116.935) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/112.761) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/108.587) < 1e-5, AipsError);

			// check the spectrum points
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
			std::tr1::shared_ptr<casa::ImageInterface<float> > imgPtr( img );
			ImageAnalysis analysis(imgPtr);

			Vector<casa::Double> wxv(2);
			Vector<casa::Double> wyv(2);
			Vector<casa::Float> z_xval;
			Vector<casa::Float> z_yval;

			wxv[0] = 4.63641;
			wxv[1] = 4.63639;
			wyv[0] = -0.506297;
			wyv[1] = -0.506279;

			// extract the mean spectrum in "wavelength"
			// with default unit
			bool ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "", "LSRK", 0, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/212.115) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/212.112) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/212.109) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/212.106) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/212.103) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/212.100) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/212.097) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/212.094) < 1e-5, AipsError);

			// extract the mean spectrum in "air wavelength"
			// with default unit
			ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "air wavelength",
					0, -1, 0, "", "LSRK", 0, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/212.054) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/212.051) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/212.048) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/212.045) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/212.042) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/212.039) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/212.036) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/212.033) < 1e-5, AipsError);

			// check the spectrum points
			AlwaysAssert(fabs(1-z_yval[0]/-0.146577)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[1]/-0.244666)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[2]/-0.184397)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[3]/0.0869152)   < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[4]/-0.43336)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[5]/-0.145391)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[6]/-0.0924785) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[7]/-0.131597)  < 1e-5, AipsError);
		}
		{
			// wavelength output
			PagedImage<Float>* img = new PagedImage<Float>(datadir + "/CAS-2533.im");
			std::tr1::shared_ptr<casa::ImageInterface<float> > imgPtr( img );
			ImageAnalysis analysis(imgPtr);

			Vector<casa::Double> wxv(2);
			Vector<casa::Double> wyv(2);
			Vector<casa::Float> z_xval;
			Vector<casa::Float> z_yval;

			wxv[0] = 4.63641;
			wxv[1] = 4.63639;
			wyv[0] = -0.506297;
			wyv[1] = -0.506279;

			// extract the mean spectrum in "wavelength"
			// with the unit "nm"
			bool ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "nm", "LSRK", 0, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/212.115e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/212.112e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/212.109e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/212.106e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/212.103e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/212.100e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/212.097e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/212.094e+06) < 1e-5, AipsError);

			// extract the mean spectrum in "air wavelength"
			// with the unit "nm"
			ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "air wavelength",
					0, -1, 0, "nm", "LSRK", 1, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/212.054e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/212.051e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/212.048e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/212.045e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/212.042e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/212.039e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/212.036e+06) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/212.033e+06) < 1e-5, AipsError);
		}
		{
			// wavelength output
			PagedImage<Float>* img = new PagedImage<Float>(datadir + "/CAS-2533.im");
			std::tr1::shared_ptr<casa::ImageInterface<float> > imgPtr( img );
			ImageAnalysis analysis(imgPtr);

			Vector<casa::Double> wxv(2);
			Vector<casa::Double> wyv(2);
			Vector<casa::Float> z_xval;
			Vector<casa::Float> z_yval;

			wxv[0] = 4.63641;
			wxv[1] = 4.63639;
			wyv[0] = -0.506297;
			wyv[1] = -0.506279;

			// extract the mean spectrum in "wavelength"
			// with the default unit
			bool ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "", "LSRK", 0, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/212.115) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/212.112) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/212.109) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/212.106) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/212.103) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/212.100) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/212.097) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/212.094) < 1e-5, AipsError);

			// check the spectrum points
			AlwaysAssert(fabs(1-z_yval[0]/-0.146577)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[1]/-0.244666)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[2]/-0.184397)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[3]/0.0869152)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[4]/-0.43336)   < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[5]/-0.145391)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[6]/-0.0924785) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[7]/-0.131597)  < 1e-5, AipsError);

			// extract the median spectrum in "wavelength"
			// with the default unit
			ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "", "LSRK", 1, 0);

			// check the spectrum points
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_yval[0]/-0.136524)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[1]/-0.256063)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[2]/-0.188921)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[3]/0.049296)   < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[4]/-0.465811)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[5]/-0.129467)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[6]/-0.0803559) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[7]/-0.166592)  < 1e-5, AipsError);


			// extract the sum spectrum in "wavelength"
			// with the default unit
			ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "", "LSRK", 2, 0);

			// check the spectrum points
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_yval[0]/-3.66442) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[1]/-6.11665) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[2]/-4.60994) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[3]/2.17288)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[4]/-10.834)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[5]/-3.63479) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[6]/-2.31196) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[7]/-3.28992) < 1e-5, AipsError);


			// extract the mean squared error spectrum in "wavelength"
			// with the default unit
			ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "", "LSRK", 3, 0);

			// check the spectrum points
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_yval[0]/0.0103565)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[1]/0.00934595) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[2]/0.00341176) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[3]/0.024648)   < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[4]/0.0158358)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[5]/0.00682485) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[6]/0.0231727)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[7]/0.0246372)  < 1e-5, AipsError);

			// extract the root mean squared error spectrum in "wavelength"
			// with the default unit
			ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "", "LSRK", 4, 0);

			// check the spectrum points
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_yval[0]/0.101767)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[1]/0.0966744) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[2]/0.0584102) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[3]/0.156997)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[4]/0.12584)   < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[5]/0.0826126) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[6]/0.152226)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[7]/0.156963)  < 1e-5, AipsError);

			// extract square root of the
			// sum in "wavelength"
			// with the default unit
			ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "", "LSRK", 5, 0);

			// check the spectrum points
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[0]) < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[1]) < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[2]) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[3]/sqrt(2.17288))  < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[4]) < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[5]) < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[6]) < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[7]) < 1e-5, AipsError);

			// extract average square root of the
			// sum of squares in "wavelength"
			// with the default unit
			ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "", "LSRK", 6, 0);

			// check the spectrum points
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[0])  < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[1]) < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[2]) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[3]/(sqrt(2.17288)/25.0))  < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[4])   < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[5]) < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[6])  < 1e-5, AipsError);
			AlwaysAssert(fabs(0.0-z_yval[7])  < 1e-5, AipsError);

			// extract the default combine type
			// (mean) in "wavelength"
			// with the default unit
			ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "wavelength",
					0, -1, 0, "", "LSRK", 100, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			AlwaysAssert(fabs(1-z_xval[0]/212.115) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[1]/212.112) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[2]/212.109) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[3]/212.106) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[4]/212.103) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[5]/212.100) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[6]/212.097) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_xval[7]/212.094) < 1e-5, AipsError);

			// check the spectrum points
			AlwaysAssert(fabs(1-z_yval[0]/-0.146577)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[1]/-0.244666)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[2]/-0.184397)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[3]/0.0869152)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[4]/-0.43336)   < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[5]/-0.145391)  < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[6]/-0.0924785) < 1e-5, AipsError);
			AlwaysAssert(fabs(1-z_yval[7]/-0.131597)  < 1e-5, AipsError);

		}
		{
			// Tabular Coordinate Instead of Spectral Coordinate
			PagedImage<Float>* img = new PagedImage<Float>(datadir + "/18h03m_tabular_axis.image");
			std::tr1::shared_ptr<casa::ImageInterface<float> > imgPtr( img );
			ImageAnalysis analysis(imgPtr);

			Vector<casa::Double> wxv(2);
			Vector<casa::Double> wyv(2);
			Vector<casa::Float> z_xval;
			Vector<casa::Float> z_yval;

			wxv[0] = 5.28206;
			wxv[1] = 4.99135;
			wyv[0] = 0.369376;
			wyv[1] = 0.877171;

			// extract the spectrum in "frequency"
			// with the MHz as unit
			bool ok = analysis.getFreqProfile(
					wxv, wyv, z_xval, z_yval,
					"world", "frequency",
					0, 2, 0, "Hz", "LSRK", 0, 0);

			// check the frequency/wavelength/velocity values
			AlwaysAssert(ok, AipsError);
			//cout <<"0="<<z_xval[0]<<" 1="<<z_xval[1]<<" 2="<<z_xval[2]<<" 3="<<z_xval[3]<<endl;
			AlwaysAssert(fabs(144040000-z_xval[0])/100000000 < 1e-5, AipsError);
			AlwaysAssert(fabs(151852000-z_xval[1])/100000000 < 1e-5, AipsError);
			AlwaysAssert(fabs(160055000-z_xval[2])/100000000 < 1e-5, AipsError);
			AlwaysAssert(fabs(167867000-z_xval[3])/100000000 < 1e-5, AipsError);
		}
		cout << "ok" << endl;
	}
    catch (AipsError x) {
    	ok = False;
        cerr << "Exception caught: " << x.getMesg() << endl;
    }
	return ok ? 0 : 1;
}

