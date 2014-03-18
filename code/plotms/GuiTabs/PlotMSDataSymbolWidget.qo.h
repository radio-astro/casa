//# Copyright (C) 2009
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
//# $Id: $
#ifndef DATASYMBOLWIDGET_QO_H
#define DATASYMBOLWIDGET_QO_H

#include <QtGui/QWidget>
#include <plotms/GuiTabs/PlotMSDataSymbolWidget.ui.h>

namespace casa {

class PlotSymbolWidget;
class PlotMSPlotter;
class PMS_PP_Display;

/**
 * Represents how symbols are colored and drawn on the plot.
 */
class PlotMSDataSymbolWidget : public QWidget
{
    Q_OBJECT

public:
    PlotMSDataSymbolWidget(PlotMSPlotter *parent = 0);
    ~PlotMSDataSymbolWidget();
    void getValue(PMS_PP_Display* d, int index );
    void setValue( const PMS_PP_Display* d, int );
    void setLabelDefaults( QMap<QLabel*,QString>& map );
    void update( const PMS_PP_Display* d, int index );

signals:
	void highlightWidgetText( QLabel* source, bool highlight);
	void symbolChanged();

private slots:
	void symbolColorizeChanged();

private:
    // Symbol widgets for unflagged and flagged points, respectively.
    PlotSymbolWidget* itsSymbolWidget_;
    PlotSymbolWidget* itsMaskedSymbolWidget_;
    Ui::PlotMSDataSymbolWidgetClass ui;
};

}

#endif // DATASYMBOLWIDGET_QO_H
