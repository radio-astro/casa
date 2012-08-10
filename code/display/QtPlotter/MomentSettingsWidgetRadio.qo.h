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
#ifndef MOMENTSETTINGSWIDGETRADIO_QO_H
#define MOMENTSETTINGSWIDGETRADIO_QO_H

#include <QtGui/QWidget>
#include <QMap>
#include <QProgressDialog>
#include <display/QtPlotter/ProfileTaskFacilitator.h>
#include <display/QtPlotter/MomentSettingsWidgetRadio.ui.h>
#include <images/Images/ImageMomentsProgressMonitor.h>

namespace casa {

class ImageAnalysis;
class MomentCollapseThreadRadio;
class ThresholdingBinPlotDialog;
class Converter;

//Note: ProfileTaskFacilitator abstracts out some of the common functionality
//needed for calculating moments and spectral line fitting into a single
//base class

//Note:  ImageMomentsProgressMonitor is an interface that provides this class
//with updates concerning the progress of the moment calculation task.

class MomentSettingsWidgetRadio : public QWidget, public ProfileTaskFacilitator,
	public ImageMomentsProgressMonitor
{
    Q_OBJECT

public:
    MomentSettingsWidgetRadio(QWidget *parent = 0);

    void setUnits( QString units );
    void setRange( float min, float max );
    void reset();
    ~MomentSettingsWidgetRadio();

    //Methods from the ImageMomentsProgressMonitor interface
    void setStepCount( int count );
    void setStepsCompleted( int count );
    void done();

signals:
	void updateProgress(int);
	void momentsFinished();

private slots:
	void thresholdingChanged();
	void adjustTableRows( int count );
	void collapseImage();
	void setCollapsedImageFile();
	void collapseDone();
	void graphicalThreshold();
	void symmetricThresholdChanged( int checkedState );
	void thresholdTextChanged( const QString& text );

private:
	enum SummationIndex {MAX, MEAN, MEDIAN, MIN, RMS, STDDEV, INTEGRATED, ABS_MEAN_DEV, END_INDEX};
	QMap<SummationIndex, int> momentMap;
    Ui::MomentSettingsWidgetRadio ui;
    ImageAnalysis* imageAnalysis;
    MomentCollapseThreadRadio* collapseThread;
    ThresholdingBinPlotDialog* thresholdingBinDialog;
    QString outputFileName;
    QList<QString> momentOptions;
    QProgressDialog progressBar;

    void setTableValue(int row, int col, float val );
    void getChannelMinMax( int channelIndex, QString& minStr, QString& maxStr ) const;
    void convertChannelRanges( const QString& oldUnits, const QString& newUnits );
    void convertChannelValue( const QString& channelStr, const QString& channelIdentifier,
    		Converter* converter, int row, int col, bool toPixels );
    String makeChannelInterval( float startChannelIndex,float endChannelIndex ) const;
    Vector<Int> populateMoments( Vector<QString>& momentNames );
    Vector<String> populateMethod() const;
    String populateChannels(uInt * nSelectedChannels);
    bool populateThresholds( Vector<Float>& includeThreshold, Vector<Float>& excludeThreshold );
    bool populateThreshold( Vector<Float>& threshold );
};

}

#endif // MOMENTSETTINGSWIDGETRADIO_QO_H
