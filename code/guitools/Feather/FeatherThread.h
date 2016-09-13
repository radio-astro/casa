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


#ifndef FEATHERTHREAD_H_
#define FEATHERTHREAD_H_

#include <QThread>
#include <QMap>
#include <guitools/Feather/FeatheredData.h>
#include <guitools/Feather/FeatherDataType.h>

namespace casa {

class Feather;
template <class T> class ImageInterface;

/**
 * Gathers up the data from the feathering process and stores
 * it.
 */

class FeatherThread : public QThread {

friend class FeatherManager;


public:
	/**
	 * Computes First Image x Second Image.  Caller is responsible
	 * for deleting the convolved image.
	 */
	ImageInterface<float>* makeConvolvedImage(ImageInterface<float>* firstImage, ImageInterface<float>* secondImage );
	FeatherThread();
	enum DataTypes { SD_WEIGHT, INT_WEIGHT, SD_CUT, INT_CUT, DIRTY_CUT,
				SD_ORIGINAL, INT_ORIGINAL, INT_CONVOLVED_LOW, INT_CONVOLVED_LOW_WEIGHTED,
				DIRTY_ORIGINAL, DIRTY_CONVOLVED_LOW, DIRTY_CONVOLVED_LOW_WEIGHTED, LOW_CONVOLVED_HIGH,
				LOW_CONVOLVED_HIGH_WEIGHTED, /*LOW_CONVOLVED_DIRTY, LOW_CONVOLVED_DIRTY_WEIGHTED,*/ END_DATA };
	void setImages(ImageInterface<float>* lowImage, ImageInterface<float>* highImage, ImageInterface<float>* dirtyImage );
	void setFeatherWorker( Feather* worker );
	static void setLogger( LogIO* logger );
	void setRadial( bool radialPlot );
	bool isSuccess() const;
	QString getErrorMessage() const;
	void setSaveOutput( bool save, const QString& outputPath = "");
	void run();
	~FeatherThread();

private:
	FeatherThread( const FeatherThread& other );
	FeatherThread operator=( const FeatherThread& other );
	bool collectLowHighData();
	bool collectLowDirtyData();
	static ImageInterface<float>* addMissingAxes( ImageInterface<float>* firstImage );
	bool collectConvolvedData( DataTypes original, DataTypes cut);
	bool setWorkerImages( ImageInterface<float>* imageLow, ImageInterface<float>* imageHigh );
	Feather* featherWorker;
	static LogIO* logger;
	ImageInterface<float>* lowImage;
	ImageInterface<float>* highImage;
	ImageInterface<float>* dirtyImage;


	QMap<DataTypes, FeatheredData> dataMap;

	bool radial;
	bool saveOutput;
	bool fileSaved;
	bool success;
	QString saveFilePath;
	QString errorMessage;
};


} /* namespace casa */
#endif /* FEATHERTHREAD_H_ */
