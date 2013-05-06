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
#ifndef FEATHERMAIN_QO_H
#define FEATHERMAIN_QO_H

#include <QtGui/QMainWindow>
#include <QThread>
#include <QProgressDialog>
#include <guitools/Feather/FeatherMain.ui.h>
#include <guitools/Feather/FileLoader.qo.h>
#include <guitools/Feather/Preferences.qo.h>
#include <guitools/Feather/PreferencesColor.qo.h>

#include <casa/BasicSL/String.h>
#include <casa/aipstype.h>

#include <synthesis/MeasurementEquations/Feather.h>
#include <pair.h>
namespace casa {

class PlotHolder;
template <class T> class ImageInterface;

class FeatherThread : public QThread {

friend class FeatherMain;

public:
	FeatherThread():featherWorker(NULL),
		saveOutput(false), fileSaved(true){}
	void setFeatherWorker( Feather* worker ){
		featherWorker = worker;
	}
	void setSaveOutput( bool save, const QString& outputPath = ""){
		saveOutput = save;
		if ( saveOutput ){
			saveFilePath = outputPath;
		}
	}
	void run();
	~FeatherThread(){}

private:

	Feather* featherWorker;

	Vector<Float> sDxWeight;
	Vector<Float> sDxAmpWeight;
	Vector<Float> sDyWeight;
	Vector<Float> sDyAmpWeight;

	Vector<Float> intxWeight;
	Vector<Float> intxAmpWeight;
	Vector<Float> intyWeight;
	Vector<Float> intyAmpWeight;

	Vector<Float> sDxCut;
	Vector<Float> sDxAmpCut;
	Vector<Float> sDyCut;
	Vector<Float> sDyAmpCut;

	Vector<Float> intxCut;
	Vector<Float> intxAmpCut;
	Vector<Float> intyCut;
	Vector<Float> intyAmpCut;

	bool saveOutput;
	bool fileSaved;
	QString saveFilePath;

};

class FeatherMain : public QMainWindow
{
    Q_OBJECT

public:
    FeatherMain(QWidget *parent = 0);
    ~FeatherMain();

signals:
	void featherFinished();

private slots:
	void openFileLoader();
	void featherImages();
	void openPreferences();
	void openPreferencesColor();
	void ySupportChanged( bool ySupport );
	//Called by the text edits when their values change.  Sends
	//the event down to the plots.
	void dishDiameterXChanged( const QString& xDiameter );
	void dishDiameterYChanged( const QString& yDiameter );
	//Called when the diameter is set through a plot.  Updates
	//the text edits with the new values.
	void dishDiameterXChanged( double value );
	void dishDiameterYChanged( double value );

	void functionColorsChanged();
	void imageFilesChanged();
	void preferencesChanged();
	void featheringDone();

private:
	FeatherMain( const FeatherMain& other );
	FeatherMain operator=( const FeatherMain& other );
	QString getFileName( QString path ) const;
	void initializeDishDiameterLimit( QLabel* diamLimitLabel );
	void clearPlots();
	bool isInputImagesChanged();
	bool generateInputImage( const String& lowResImagePath, const String& highResImagePath,
			ImageInterface<Float>*& lowResImage, ImageInterface<Float>*& highResImage );
	pair<float,float> populateDishDiameters(Bool& validDiameters);
	float populateSDFactor() const;
	bool loadImages();
	void addOriginalDataToPlots();
	void resetDishDiameters();
	void resetDishDiameter( QLineEdit* dishEdit, QLabel* diamLimit,
			float value, float defaultValue );

	const static int DISH_DIAMETER_DEFAULT;
	const static int SINGLE_DISH_FACTOR_DEFAULT;

	Ui::FeatherMainClass ui;
    FileLoader fileLoader;
    Preferences preferences;
    PreferencesColor preferencesColor;

    ImageInterface<Float>* lowResImage;
    ImageInterface<Float>* highResImage;
    QString lowResImagePath;
    QString highResImagePath;
    QString outputImagePath;

    Feather featherWorker;
    FeatherThread* thread;
    PlotHolder* plotHolder;
    QProgressDialog progressMeter;
    LogIO logger;

};
}
#endif // FEATHERMAIN_QO_H
