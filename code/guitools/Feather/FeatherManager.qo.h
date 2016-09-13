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


#ifndef FEATHERMANAGER_H_
#define FEATHERMANAGER_H_
#include <synthesis/MeasurementEquations/Feather.h>
#include <guitools/Feather/FeatheredData.h>
#include <guitools/Feather/FeatherThread.h>
#include <QObject>
namespace casa {

//FeatherWorker1 (SD, INT)
//- SD																//getFTCutSDImage
//- SD weighted and scaled											//getFeatheredCutSD
//- INT																//getFTCutIntImage
//- INT weighted and scaled											//getFeatheredCutInt

//FeatherWorker2 (SD, DIRTY)
// - DIRTY															//getFtCutIntImage
// - DIRTY, weighted and scaled										//getFeatheredCutInt

//FeatherWorker3 ( SD convolved with Int synthesized beam, INT )
//- SD convolved with INT synthesized beam							//getFTCutSDImage
//- SD convolved with INT synthesized beam; weighted and scaled		//getFeatheredCutSD

//FeatherWorker4 ( SD convolved with DIRTY beam, DIRTY )
//- SD convolved with DIRTY beam									//getFTCutSDImage
//- SD convolved with DIRTY beam, weighted and scaled				//getFeatheredCutSD

//FeatherWorker5 ( SD, INT Convolved with SD resolution (beam) )
//- INT convolved with SD resolution (beam)							//getFTCutIntImage
//- INT convolved with SD resolution (beam), weighted and scaled	//getFeatheredCutInt

//FeatherWorker6 (SD, DIRTY )
//- DIRTY (or other image)											//getFTCutIntImage
//- DIRTY (or other image), weighted and scaled						//getFeatheredCutInt

//FeatherWorker7( SD, DIRTY Convolved with SD resolution (beam) )
//- DIRTY convolved with SD resolution (beam)						//getFTCutIntImage
//- DIRTY convolved with SD resolution (beam), weighted and scaled	//getFeatheredCutInt



template <class T> class ImageInterface;
class LogIO;

/**
 * Does image/data manapulation and stores/provides
 * data.
 */

class FeatherManager : public QObject {
	Q_OBJECT
public:
	FeatherManager();
	bool isReady() const ;
	bool isSuccess() const;
	bool loadImages( const QString& lowImagePath, const QString& highImagePath, LogIO* logger );
	bool loadDirtyImage( const QString& dirtyImagePath);
	void applyFeather( bool saveOutput, const QString& outputImagePath );
	void getEffectiveDishDiameter( Float& xDiam, Float& yDiam );
	bool setEffectiveDishDiameter( float xDiam, float yDiam );
	void setSDScale( float scale );
	void setRadial( bool radialPlot );
	void setChannelsAveraged( bool averaged );
	void setChannelIndex( int index );
	bool isRadial() const;
	int getPlaneCount() const;
	int getChannelIndex() const;
	QString getError() const;
	bool isFileSaved() const;
	bool isChannelsAveraged() const;

	//Data
	FeatheredData getSDOrig();
	FeatheredData getIntOrig();
	FeatheredData getDirtyOrig();

	FeatheredData getSDWeight() const;
	FeatheredData getIntWeight() const;

	FeatheredData getSDCut() const;
	FeatheredData getIntCut() const;
	FeatheredData getDirtyCut() const;

	FeatheredData getIntConvolvedSDOrig();
	FeatheredData getIntConvolvedSDCut() const;
	FeatheredData getDirtyConvolvedSDOrig();
	FeatheredData getDirtyConvolvedSDCut() const;

	FeatheredData getSDConvolvedIntOrig();
	FeatheredData getSDConvolvedIntCut() const;
	//FeatheredData getSDConvolvedDirtyOrig();
	//FeatheredData getSDConvolvedDirtyCut() const;
	virtual ~FeatherManager();
signals:
	void featheringDone();
private slots:
	void featherDone();
private:
	FeatherManager( const FeatherManager& other );
	FeatherManager operator=( const FeatherManager& other );
	FeatheredData getConvolvedOrig( ImageInterface<float>* image ) const;
	bool generateInputImage( QString highResImagePath, QString lowResImagePath);
	bool generateDirtyImage( QString dirtyImagePath);
	int getPlaneCount( ImageInterface<float>* image ) const;
	ImageInterface<Float>* getSinglePlaneImage( ImageInterface<float>* image ) const;
	void resetBasicFeedImages();
	void resetDirtyFeedImage();
	ImageInterface<Float>* lowResImage;
	ImageInterface<Float>* highResImage;
	ImageInterface<Float>* dirtyImage;
	ImageInterface<Float>* highResFeedImage;
	ImageInterface<Float>* dirtyFeedImage;
	ImageInterface<Float>* lowResFeedImage;
	Feather* featherWorker;
	FeatherThread* thread;
	QString errorMessage;
	LogIO* logger;
	bool radialAxis;
	bool channelsAveraged;
	bool success;
	int channelIndex;
};

} /* namespace casa */
#endif /* FEATHERMANAGER_H_ */
