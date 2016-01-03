//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#include "MomentCollapseThreadRadio.h"
#include <images/Images/ImageAnalysis.h>

#include <QDebug>
#include <QFile>

namespace casa {

MomentCollapseThreadRadio::MomentCollapseThreadRadio( ImageAnalysis* imageAnalysis ):
		analysis( imageAnalysis ){
}

bool MomentCollapseThreadRadio::isSuccess() const{
	bool success = false;
	if ( collapseResults.size() > 0 ){
		success = true;
	}
	return success;
}

void MomentCollapseThreadRadio::setData(const Vector<Int>& mments, const Int axis, Record& region,
    	const String& maskStr, const Vector<String>& methodVec,
    	const Vector<Int>& smoothaxesVec,
    	const Vector<String>& smoothtypesVec,
        const Vector<Quantity>& smoothwidthsVec,
        const Vector<Float>& includepixVec,
        const Vector<Float>& excludepixVec,
        const Double peaksnr, const Double stddev,
        const String& doppler, const String& baseName){
	moments.resize( mments.size());

	moments = mments;
	this->axis = axis;
	this->region = region;
	mask = maskStr;
	method = methodVec;
	smoothaxes = smoothaxesVec;
	smoothtypes = smoothtypesVec;
	smoothwidths = smoothwidthsVec;
	includepix = includepixVec;
	excludepix = excludepixVec;
	this->peaksnr = peaksnr;
	this->stddev = stddev;
	this->doppler = doppler;
	this->baseName = baseName;
}

std::vector<CollapseResult> MomentCollapseThreadRadio::getResults() const {
	return collapseResults;
}

void MomentCollapseThreadRadio::setChannelStr( String str ){
	channelStr = str;
}

void MomentCollapseThreadRadio::setOutputFileName( QString name ){
	outputFileName = name;
}

void MomentCollapseThreadRadio::setMomentNames( const Vector<QString>& momentNames ){
	this->momentNames = momentNames;
}

bool MomentCollapseThreadRadio::getOutputFileName( String& outName,
		int moment, const String& channelStr ) const {

	bool tmpFile = true;
	//Use a default base name
	if (outputFileName.isEmpty()){
		outName = baseName;
	}
	//Use the user specified name
	else {
		outName = outputFileName.toStdString();
		tmpFile = false;
	}

	//Append the channel and moment used to make it descriptive.
	outName = outName + "_" + String(momentNames[moment].toStdString());
	if ( channelStr != ""){
		outName = outName + "_"+channelStr;
	}
	QString uniqueFileName( outName.c_str() );

	//Add gibberish to ensure the file name is unique.
	bool fileExists = true;
	while ( fileExists ){
		QFile file( uniqueFileName );
		fileExists = file.exists();
		if ( fileExists ){
			uniqueFileName = outName.c_str() + QString::number(qrand() % 10000);
		}
		else {
			outName = uniqueFileName.toStdString();
		}
	}
	return tmpFile;
}


void MomentCollapseThreadRadio::run(){

	for ( int i = 0; i < static_cast<int>(moments.size()); i++ ){
		Vector<int> whichMoments(1);
		whichMoments[0] = moments[i];

		//Output file
		String outFile;
		bool outputFileTemporary = getOutputFileName( outFile, whichMoments[0], channelStr );

		ImageInterface<Float>* newImage = analysis->moments( moments, axis, region,
						mask, method,
						smoothaxes, smoothtypes, smoothwidths,
						includepix,excludepix,
						peaksnr, stddev, "RADIO", outFile);
		if ( newImage != NULL ){
			CollapseResult result( outFile, outputFileTemporary, newImage );
			collapseResults.push_back( result );
		}
	}
}

MomentCollapseThreadRadio::~MomentCollapseThreadRadio() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */