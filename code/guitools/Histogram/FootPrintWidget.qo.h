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
#ifndef FOOTPRINTWIDGET_QO_H
#define FOOTPRINTWIDGET_QO_H

#include <QtGui/QWidget>
#include <guitools/Histogram/FootPrintWidget.ui.h>

namespace casa {

class FootPrintWidget : public QWidget
{
    Q_OBJECT

public:
    FootPrintWidget(QWidget *parent = 0);
    ~FootPrintWidget();
    enum PlotMode {REGION_MODE,IMAGE_MODE,REGION_ALL_MODE};
public slots:
	void setPlotMode( int mode );
signals:
	void plotModeChanged( int );
private slots:
	void imageModeSelected( bool selected );
	void regionModeSelected( bool selected );
	void regionAllModeSelected( bool selected );

private:
	//Added to fix the problem that custom menu items are transparent
	//on the MAC.
	void setDefaultBackground();
    Ui::FootPrintWidgetClass ui;
};
}

#endif // FOOTPRINTWIDGET_QO_H
