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

#include <imageanalysis/ImageAnalysis/SpectralCollapser.h>
#include <imageanalysis/ImageAnalysis/ImageCollapser.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/Directory.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/FITSImage.h>
#include <images/Images/FITSQualityImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Images/TempImage.h>
#include <lattices/Lattices/LatticeUtilities.h>

namespace casa {
SpectralCollapser::SpectralCollapser(ImageInterface<Float> * &image):
		_image(image), _log(new LogIO()), _storePath(""){
	_setUp();
}

SpectralCollapser::SpectralCollapser(ImageInterface<Float> * &image, const String storePath):
		_image(image), _log(new LogIO()), _storePath(storePath){
	_setUp();
}

SpectralCollapser::~SpectralCollapser(){delete _log;}

Bool SpectralCollapser::collapse(const Vector<Float> &specVals, const Float startVal, const Float endVal,
		const String &unit, const SpectralCollapser::CollapseType &collType, SpectralCollapser::CollapseError &collError, String &outname, String &msg){

	Bool ok;
	String unit_(unit);

	*_log << LogOrigin("SpectralCollapser", "collapse");

	if (specVals.size() < 1){
		msg = String("No spectral values provided!");
		*_log << LogIO::WARN << msg << LogIO::POST;
		return False;
	}

	// in the unit, replace a "/" with "_p_"
	if (unit_.find(String("/"))!=String::npos)	{
		String::size_type slashPos=unit_.find(String("/"));
		unit_.replace(slashPos, 1, String("_p_"));
	}

	Bool ascending=True;
	if (specVals(specVals.size()-1)<specVals(0))
		ascending=False;

	Int startIndex, endIndex;
	if (ascending){
		if (endVal < specVals(0)){
			msg = String("Start value: ") + String::toString(endVal) + String(" is smaller than all spectral values!");
			*_log << LogIO::WARN << msg << LogIO::POST;
			return False;
		}
		if (startVal > specVals(specVals.size()-1)){
			msg = String("End value: ") + String::toString(startVal) + String(" is larger than all spectral values!");
			*_log << LogIO::WARN << msg << LogIO::POST;
			return False;
		}
		startIndex=0;
		while (specVals(startIndex)<startVal)
			startIndex++;

		endIndex=specVals.size()-1;
		while (specVals(endIndex)>endVal)
			endIndex--;
	}
	else {
		if (endVal < specVals(specVals.size()-1)){
			msg = String("Start value: ") + String::toString(endVal) + String(" is smaller than all spectral values!");
			*_log << LogIO::WARN << msg << LogIO::POST;
			return False;
		}
		if (startVal > specVals(0)){
			msg = String("End value: ") + String::toString(startVal) + String(" is larger than all spectral values!");
			*_log << LogIO::WARN << msg << LogIO::POST;
			return False;
		}
		startIndex=0;
		while (specVals(startIndex)<endVal)
			startIndex++;

		endIndex=specVals.size()-1;
		while (specVals(endIndex)>startVal)
			endIndex--;
	}

	String chanInp;
	chanInp = String::toString(startIndex) + "~" + String::toString(endIndex);

	String wcsInp;
	wcsInp = String::toString(startVal) + "~" + String::toString(endVal) + unit_;

	if (startIndex > endIndex){
		msg = String("Spectral window ") + wcsInp + String(" too narrow!");
		*_log << LogIO::WARN << msg << LogIO::POST;
		return False;
	}

	String dataAggStr;
    collapseTypeToString(collType, dataAggStr);

	outname = _getOutputName(wcsInp, True);

	if (_hasQualAxis){
		SubImage<Float> subData;
		SubImage<Float> subError;
		ok = _getQualitySubImgs(_image, subData, subError);
		ok = _collapse(&subData, dataAggStr, chanInp, outname);
	}
	else {
		ok = _collapse(_image, dataAggStr, chanInp, outname);
	}

	msg = String("Collapsed image: ") + outname;

	return True;
}


String SpectralCollapser::summaryHeader() const {
	ostringstream os;
	os << "Input parameters ---" << endl;
	os << "       --- imagename:           " << _image->name() << endl;
	os << "       --- storage path:        " << _storePath << endl;
	os << "       --- spectral axis:       " << _specAxis << endl;
	os << "       --- quality axis:        " << _hasQualAxis << endl;
	//os << "       --- channels:            " << _chan << endl;
	//os << "       --- stokes:              " << _stokesString << endl;
	//os << "       --- mask:                " << _mask << endl;
	return os.str();
}

void SpectralCollapser::stringToCollapseType(const String &text, SpectralCollapser::CollapseType &collType){
	if (!text.compare(String("mean")))
		collType = SpectralCollapser::PMEAN;
	else if (!text.compare(String("median")))
		collType = SpectralCollapser::PMEDIAN;
	else if (!text.compare(String("sum")))
		collType = SpectralCollapser::PSUM;
	else
		collType = SpectralCollapser::CUNKNOWN;
}

void SpectralCollapser::stringToCollapseError(const String &text, SpectralCollapser::CollapseError &collError){
	if (!text.compare(String("no error")))
		collError = SpectralCollapser::PNOERROR;
	else if (!text.compare(String("rmse")))
		collError = SpectralCollapser::PERMSE;
	else if (!text.compare(String("propagated")))
		collError = SpectralCollapser::PPROPAG;
	else
		collError = SpectralCollapser::EUNKNOWN;
}

void SpectralCollapser::collapseTypeToString(const SpectralCollapser::CollapseType &collType, String &strCollType){
	switch (collType)
	{
	case SpectralCollapser::PMEAN:
		strCollType=String("mean");
		break;
	case SpectralCollapser::PMEDIAN:
		strCollType=String("median");
		break;
	case SpectralCollapser::PSUM:
		strCollType=String("sum");
		break;
	case SpectralCollapser::CUNKNOWN:
		strCollType=String("unknown");
		break;
	default:
		strCollType=String("No Idea!");
	}
}


void SpectralCollapser::_setUp(){
	*_log << LogOrigin("SpectralCollapser", "_setUp");

	_all = CasacRegionManager::ALL;

	// get the pixel axis number of the spectral axis
	CoordinateSystem cSys = _image->coordinates();
	Int specAx = cSys.findCoordinate(Coordinate::SPECTRAL);
	if (specAx < 0)
		*_log << LogIO::EXCEPTION << "No spectral axis in image: " << _image->name() << LogIO::POST;

	Vector<Int> nPixelSpec = cSys.pixelAxes(specAx);
	_specAxis = IPosition(1, nPixelSpec(0));

	_hasQualAxis = (cSys.findCoordinate(Coordinate::QUALITY) < 0) ? False : True;
}

Bool SpectralCollapser::_getQualitySubImg(const ImageInterface<Float>* image, const Bool &getData, SubImage<Float> &qualitySub){
	Int specAx = (image->coordinates()).findCoordinate(Coordinate::QUALITY);
	Vector<Int> nPixelQual = (image->coordinates()).pixelAxes(specAx);
	uInt nAxisQual=nPixelQual(0);

	// build the appropriate slicer
	IPosition startPos(image->ndim(), 0);
	IPosition lengthPos=image->shape();
	if (!getData)
		startPos(nAxisQual) = 1;
	lengthPos(nAxisQual) = 1;
	Slicer subSlicer(startPos, lengthPos, Slicer::endIsLength);

	qualitySub = SubImage<Float>(*image, subSlicer, AxesSpecifier(False));

	return True;
}

Bool SpectralCollapser::_getQualitySubImgs(ImageInterface<Float>* image, SubImage<Float> &subData, SubImage<Float> &subError){

	FITSQualityImage *qImg = dynamic_cast<FITSQualityImage*>(image);

	// create the data image
	subData = SubImage<Float>(*(qImg->fitsData()), AxesSpecifier(False));

	// create the error image
	subError = SubImage<Float>(*(qImg->fitsError()), AxesSpecifier(False));

	return True;
}

String SpectralCollapser::_getOutputName(const String &wcsInp, const Bool &data){
	Path   imgPath(_image->name());
	Path   collImgName(_storePath);
	String imgName(imgPath.baseName());

	*_log << LogOrigin("SpectralCollapser", "_getOutputName");

	// check that the storage path is OK
	if (!collImgName.isValid ())
		*_log << LogIO::EXCEPTION << "Not a valid storage path: " << collImgName.absoluteName() << LogIO::POST;

	// strip ".fits" or ".img"
	if ((int)imgName.find(".fits") > -1)
		imgName = imgName(0, imgName.find(".fits"));
	else if ((int)imgName.find(".img") > -1)
		imgName = imgName(0, imgName.find(".img"));

	// build a new name
	imgName += "_" + wcsInp;
	collImgName.append(imgName);
	File imgFile(collImgName);

	// make sure the name is unique
	Int index=1;
	while (imgFile.exists()){
		imgFile = File(collImgName.absoluteName() + "(" + String::toString(index)+ ")");
		index++;
	}

	// return the unique name
	collImgName = imgFile.path();
	return collImgName.absoluteName();
}


Bool SpectralCollapser::_collapse(const ImageInterface<Float> *image, const String &aggString,
		const String& chanInp, const String& outname){

	ImageCollapser collapser(
			aggString,                     // String aggString
			image,                 // const ImageInterface<Float> *const image
			"",                         // const String& region
			0,                          // const Record *const regionRec
			"",                         // const String& box
			chanInp,                        // const String& chanInp
			_all,                        // const String& stokes
			"",                         // const String& maskInp
			_specAxis,                       // const IPosition& axes
			outname,                  // String& outname
			True                        // const Bool overwrite
		);
		collapser.collapse(False);
		return True;
}
}

