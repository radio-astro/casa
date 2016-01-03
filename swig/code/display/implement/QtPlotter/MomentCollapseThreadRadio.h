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
#ifndef MOMENTCOLLAPSETHREADRADIO_H_
#define MOMENTCOLLAPSETHREADRADIO_H_

#include <QThread>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta/Quantum.h>

namespace casa {

class ImageAnalysis;
template <class T> class ImageInterface;


class CollapseResult {
public:
	CollapseResult( const String& outputName, bool tmp, ImageInterface<Float>* img ):
		outputFileName(outputName),
		temporary( tmp ),
		image(img) {}
	String getOutputFileName() const { return outputFileName; }
	bool isTemporaryOutput() const { return temporary; }
	ImageInterface<Float>* getImage() const { return image; }
private:
	String outputFileName;
	bool temporary;
	ImageInterface<Float>* image;
};




/**
 * Responsible for running the collapse algorithm in
 * the background so that we don't freeze the GUI.
 */
class MomentCollapseThreadRadio : public QThread {
public:
	MomentCollapseThreadRadio( ImageAnalysis* imageAnalysis );
	bool isSuccess() const;
	void setChannelStr( String str );
	void setMomentNames( const Vector<QString>& momentNames );
	void setOutputFileName( QString name );
	std::vector<CollapseResult> getResults() const;
	void setData(const Vector<Int>& moments, const Int axis, Record& region,
	    	const String& mask, const Vector<String>& method,
	    	const Vector<Int>& smoothaxes,
	    	const Vector<String>& smoothtypes,
	        const Vector<Quantity>& smoothwidths,
	        const Vector<Float>& includepix,
	        const Vector<Float>& excludepix,
	        const Double peaksnr, const Double stddev,
	        const String& doppler = "RADIO", const String& baseName = "");
	void run();
	~MomentCollapseThreadRadio();
private:

	bool getOutputFileName( String& outName, int moment, const String& channelStr ) const;
	ImageAnalysis* analysis;
	Vector<Int> moments;
	Vector<QString> momentNames;
	Int axis;
	Record region;
	String mask;
	String channelStr;
	Vector<String> method;
	Vector<Int> smoothaxes;
	Vector<String> smoothtypes;
	Vector<Quantity> smoothwidths;
	Vector<Float> includepix;
	Vector<Float> excludepix;
	Double peaksnr;
	Double stddev;
	String doppler;
	String baseName;
	QString outputFileName;

	std::vector<CollapseResult> collapseResults;
};


} /* namespace casa */
#endif /* MOMENTCOLLAPSETHREADRADIO_H_ */