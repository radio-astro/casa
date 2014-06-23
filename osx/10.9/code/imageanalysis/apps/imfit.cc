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

#include <imageanalysis/ImageAnalysis/ImageFitter.h>


#include <casa/Inputs/Input.h>
#include <images/Images/ImageUtilities.h>
#include <casa/cppconfig.h>
#include <casa/namespace.h>

Int main(Int argc, char *argv[]) {
	Input input(1);
	input.version("$ID:$");
	input.create("imagename");
	input.create("box", "");
	input.create("region", "");
	input.create("chans", "0");
	input.create("stokes", "I");
	input.create("mask","");
	input.create("includepix", "");
	input.create("excludepix", "");
	input.create("residual", "");
	input.create("model", "");
	input.create("estimates", "");
	input.create("logfile", "");
	input.create("append","false");
	input.create("newestimates","");

	input.readArguments(argc, argv);
	String imagename = input.getString("imagename");

	String box = input.getString("box");
	String region = input.getString("region");
	String chans = input.getString("chans");
	String stokes = input.getString("stokes");
	String mask = input.getString("mask");
	String residual = input.getString("residual");
	String model = input.getString("model");
	String estimatesFilename = input.getString("estimates");
	String logfile = input.getString("logfile");
	Bool append = input.getBool("append");
	String newEstimatesFileName = input.getString("newestimates");

	LogIO log;
	Vector<String> includePixParts = stringToVector(input.getString("includepix"));
	Vector<String> excludePixParts = stringToVector(input.getString("excludepix"));
	Vector<Float> includePixelRange(includePixParts.nelements());
	Vector<Float> excludePixelRange(excludePixParts.nelements());

	ImageInterface<Float> *image;
	ImageUtilities::openImage(image, imagename);
	SPCIIF sImage(image);
	ImageFitter imFitter(
		sImage, region, 0, box, chans, stokes, mask,
		estimatesFilename, newEstimatesFileName
	);
	if (includePixelRange.nelements() == 1) {
		imFitter.setIncludePixelRange(
			std::pair<Float, Float> (includePixelRange[0], includePixelRange[0])
		);
	}
	else if (includePixelRange.nelements() == 2) {
		imFitter.setIncludePixelRange(
			std::pair<Float, Float> (includePixelRange[0], includePixelRange[1])
		);

	}
	if (excludePixelRange.nelements() == 1) {
		imFitter.setExcludePixelRange(
			std::pair<Float, Float> (excludePixelRange[0], excludePixelRange[0])
		);
	}
	else if (excludePixelRange.nelements() == 2) {
		imFitter.setExcludePixelRange(
			std::pair<Float, Float> (excludePixelRange[0], excludePixelRange[1])
		);
	}
    if(! logfile.empty()) {
    	imFitter.setLogfile(logfile);
    	imFitter.setLogfileAppend(append);
    }
    imFitter.setModel(model);
    imFitter.setResidual(residual);
    imFitter.fit();

    return 0;
}



