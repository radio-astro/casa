//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#include "Gaussian2DFitter.h"
#include <imageanalysis/ImageAnalysis/ImageFitter.h>
#include <display/Display/Options.h>
#include <QDebug>

namespace casa {

	Gaussian2DFitter::Gaussian2DFitter() :
		LOG_SUFFIX("Log.txt"), REGION_SUFFIX("Region.txt") {
		logFile = false;
	}

	bool Gaussian2DFitter::isFitSuccessful() const {
		return successfulFit;
	}

	QString Gaussian2DFitter::getErrorMessage() const {
		return errorMsg;
	}

	QString Gaussian2DFitter::getLogFilePath() const {
		QString logPath;
		if ( logFile ) {
			logPath = filePath.c_str()+ LOG_SUFFIX;
		} else {
			logPath = viewer::options.temporaryPath( "tmpFitLogFile" ).c_str();
		}
		return logPath;
	}

	QString Gaussian2DFitter::getResidualImagePath() const {
		return residualImageFile.c_str();
	}

	void Gaussian2DFitter::setFilePath( String path ) {
		filePath = path;
	}

	void Gaussian2DFitter::setWriteLogFile( bool write ) {
		logFile = write;
	}

	void Gaussian2DFitter::setFitParameters( ImageTask::shCImFloat image, const String& box,
	        int channelNum, const String& estimatesFileName, const String& residualImage,
	        const Vector<Float>& include, const Vector<Float>& exclude ) {
		this->image = image;
		pixelBox = box;
		channelNumber = channelNum;
		estimateFile = estimatesFileName;
		residualImageFile = residualImage;
		int includeCount = include.size();
		includePixs.resize( includeCount );
		for ( int i = 0; i < includeCount; i++ ) {
			includePixs[i] = include[i];
		}
		int excludeCount = exclude.size();
		for ( int i = 0; i < excludeCount; i++ ) {
			excludePixs[i] = exclude[i];
		}
	}

	QList<RegionShape*> Gaussian2DFitter::toDrawingDisplay(const ImageTask::shCImFloat image, const QString& colorName) const {
		return fitResultList.toDrawingDisplay( image.get(), colorName );
	}

	void Gaussian2DFitter::run() {
		successfulFit = true;
		String channelStr = String::toString(channelNumber);

		ImageFitter fitter(image, "", NULL, pixelBox, channelStr, "", "", includePixs,
		                   excludePixs, residualImageFile, "", estimateFile);
		String logFile = getLogFilePath().toStdString();
		fitter.setLogfile( logFile );

		// do the fit
		try {
			ComponentList componentList = fitter.fit();
			fitResultList.fromComponentList( componentList );

			//If the fit did not converge record an error.
			if (!fitter.converged(0)) {
				successfulFit = false;
				errorMsg = "Fit did not converge.";
			}
		} catch( AipsError& error ) {
			successfulFit = false;
			QString specificProblem( error.what() );
			qDebug() << "Unsuccessful fit: "<<specificProblem;
			errorMsg = "Fit did not converge";
		}
	}

	bool Gaussian2DFitter::writeRegionFile() const {
		QString regionFile = filePath.c_str() + REGION_SUFFIX;
		bool successfulWrite = fitResultList.toRegionFile( image.get(), channelNumber, regionFile );
		return successfulWrite;
	}

	Gaussian2DFitter::~Gaussian2DFitter() {
	}

} /* namespace casa */
