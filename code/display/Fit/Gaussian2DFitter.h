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

#ifndef GAUSSIAN2DFITTER_H_
#define GAUSSIAN2DFITTER_H_

#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>
#include <display/Fit/ComponentListWrapper.h>
#include <QString>
#include <QThread>

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <tr1/memory>

namespace casa {

	template <class T> class ImageInterface;

	/**
	 * Performs a 2DGaussian fit of an image in a background thread.
	 */

	class Gaussian2DFitter : public QThread {
	public:
		Gaussian2DFitter();
		void run();
		bool isFitSuccessful() const;
		void setFitParameters( ImageTask::shCImFloat image, const String& box,
		                       int channelNum, const String& estimatesFileName, const String& residualImageFile,
		                       const Vector<Float>& includeVector, const Vector<Float>& excludeVector);
		QString getErrorMessage() const;
		QString getLogFilePath() const;
		void setWriteLogFile( bool write );
		QString getResidualImagePath() const;
		void setFilePath( String path );
		bool writeRegionFile() const;
		QList<RegionShape*> toDrawingDisplay( const ImageTask::shCImFloat image, const QString& colorName) const;
		virtual ~Gaussian2DFitter();

	private:
		Gaussian2DFitter( const Gaussian2DFitter& other );
		Gaussian2DFitter operator=( const Gaussian2DFitter& other );
		QString errorMsg;
		bool successfulFit;
		bool logFile;
		ComponentListWrapper fitResultList;
		ImageTask::shCImFloat image;
		Vector<Float> includePixs;
		Vector<Float> excludePixs;
		String pixelBox;
		String filePath;
		int channelNumber;
		String estimateFile;
		String residualImageFile;
		const QString LOG_SUFFIX;
		const QString REGION_SUFFIX;
	};

} /* namespace casa */
#endif /* GAUSSIAN2DFITTER_H_ */
