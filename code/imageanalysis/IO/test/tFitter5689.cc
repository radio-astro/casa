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

#include <imageanalysis/IO/FitterEstimatesFileParser.h>

#include <casa/Utilities/Assert.h>
#include <casa/OS/File.h>
#include <components/ComponentModels/GaussianShape.h>
#include <images/Images/FITSImage.h>
#include <imageanalysis/ImageAnalysis/ImageProfileFitter.h>

#include <casa/namespace.h>



int main() {
	String pixelBox("100,100,115,115");
	String channelStr( "17~27");
	int spectralAxisNumber = 3;
	int nGauss = 1;
	SpectralList spectralList;
	std::tr1::shared_ptr<ImageInterface<Float> > image( new PagedImage<Float>( "/home/uniblab/casa/trunk/test/titan2/titanline-small2.image"));
	ImageProfileFitter* fitter = new ImageProfileFitter( image, "", 0, pixelBox,
			                                 channelStr, "", "",
			                                 spectralAxisNumber,
			                                 static_cast<uInt>(nGauss), "",
			                                 spectralList);
	fitter->setPolyOrder( 0 );
	fitter->setLogfile( "/tmp/fitTest.txt" );
	Record results = fitter->fit();
	cout << "Results: "<<results;
    cout << "ok" << endl;
    return 0;
}

