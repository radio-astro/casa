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
#include <display/QtPlotter/ProfileTaskFacilitator.h>
#include <display/QtPlotter/MomentSettingsWidgetRadio.ui.h>

namespace casa {

class ImageAnalysis;
class MomentCollapseThreadRadio;

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
	void collapseDone();

private:
	enum SummationIndex {MAX, MEAN, MEDIAN, MIN, RMS, STDDEV, INTEGRATED, ABS_MEAN_DEV, END_INDEX};
	QMap<SummationIndex, int> momentMap;
    Ui::MomentSettingsWidgetRadio ui;
    ImageAnalysis* imageAnalysis;
    MomentCollapseThreadRadio* collapseThread;
    QString outputFileName;
    QList<QString> momentOptions;
    void setTableValue(int row, int col, float val );
    String makeChannelInterval( float startChannelIndex,float endChannelIndex ) const;
    Vector<Int> populateMoments();
    Vector<String> populateMethod() const;
    String populateChannels(uInt * nSelectedChannels);
};

}

#endif // MOMENTSETTINGSWIDGETRADIO_QO_H
