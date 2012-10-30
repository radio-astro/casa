#ifndef FEATHERMAIN_QO_H
#define FEATHERMAIN_QO_H

#include <QtGui/QMainWindow>
#include <guitools/Feather/FeatherMain.ui.h>
#include <guitools/Feather/FileLoader.qo.h>
#include <guitools/Feather/Preferences.qo.h>
#include <guitools/Feather/PreferencesColor.qo.h>
#include <casa/BasicSL/String.h>

#include <synthesis/MeasurementEquations/Feather.h>
#include <pair.h>
namespace casa {

class FeatherPlotWidget;
template <class T> class ImageInterface;

class FeatherMain : public QMainWindow
{
    Q_OBJECT

public:
    FeatherMain(QWidget *parent = 0);
    ~FeatherMain();

private slots:
	void openFileLoader();
	void featherImages();
	void openPreferences();
	void openPreferencesColor();
	void ySupportChanged( bool ySupport );
	void dishDiameterXChanged( const QString& xDiameter );
	void functionColorsChanged();
	void imageFilesChanged();
	void preferencesChanged();

private:
	void layoutPlotWidgets();
	void removeWidget(QWidget* widget, QLayout* layout);
	bool generateInputImage( const String& lowResImagePath, const String& highResImagePath,
			ImageInterface<Float>* lowResImage, ImageInterface<Float>* highResImage );
	pair<float,float> populateDishDiameters() const;
	const static int DISH_DIAMETER_DEFAULT;

	Ui::FeatherMainClass ui;
    FileLoader fileLoader;
    Preferences preferences;
    PreferencesColor preferencesColor;

    FeatherPlotWidget* origXWidget;
    FeatherPlotWidget* xWidget;
    FeatherPlotWidget* origYWidget;
    FeatherPlotWidget* yWidget;

    LogIO logger;
};
}
#endif // FEATHERMAIN_QO_H
