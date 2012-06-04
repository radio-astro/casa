#ifndef MOMENTSETTINGSWIDGETRADIO_QO_H
#define MOMENTSETTINGSWIDGETRADIO_QO_H

#include <QtGui/QWidget>
#include <QMap>
#include <display/QtPlotter/ProfileTaskFacilitator.h>
#include <display/QtPlotter/MomentSettingsWidgetRadio.ui.h>

namespace casa {

class ImageAnalysis;

class MomentSettingsWidgetRadio : public QWidget, public ProfileTaskFacilitator
{
    Q_OBJECT

public:
    MomentSettingsWidgetRadio(QWidget *parent = 0);

    void setUnits( QString units );
    void setRange( float min, float max );
    void reset();
    ~MomentSettingsWidgetRadio();

private slots:
	void includePixelsChanged( int state );
	void excludePixelsChanged( int state );
	void adjustTableRows( int count );
	void collapseImage();
	void setCollapsedImageFile();

private:
	enum SummationIndex {MAX, MEAN, MEDIAN, MIN, RMS, STDDEV, INTEGRATED, ABS_MEAN_DEV, END_INDEX};
	QMap<SummationIndex, int> momentMap;
    Ui::MomentSettingsWidgetRadio ui;
    ImageAnalysis* imageAnalysis;
    QString outputFileName;
    QList<QString> momentOptions;
    void setTableValue(int row, int col, float val );
    String makeChannelInterval( float startChannelIndex,float endChannelIndex ) const;
    Vector<Int> populateMoments();
    Vector<String> populateMethod() const;
    String populateChannels(uInt * nSelectedChannels);
    bool getOutputFileName(String& outName, int moment, const String& channelStr ) const;

};

}

#endif // MOMENTSETTINGSWIDGETRADIO_QO_H
