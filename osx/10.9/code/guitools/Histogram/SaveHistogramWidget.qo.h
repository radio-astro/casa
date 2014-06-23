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
#ifndef SAVEHISTOGRAMWIDGET_QO_H
#define SAVEHISTOGRAMWIDGET_QO_H

#include <QtGui/QDialog>
#include <guitools/Histogram/SaveHistogramWidget.ui.h>

class QFileSystemModel;

namespace casa {
class SaveHistogramWidget : public QDialog {
    Q_OBJECT

public:
    SaveHistogramWidget(QWidget *parent = 0);
    ~SaveHistogramWidget();

signals:
	void savePing( const QString& filePath, int width, int height );
	void saveAscii( const QString& filePath );

private slots:
	void validateDirectory( const QString& str );
	void directoryChanged(const QModelIndex& modelIndex );
	void suffixChanged( const QString& text );
	void save();
	void saveCanceled();

private:
	SaveHistogramWidget( const SaveHistogramWidget& widget );
	SaveHistogramWidget& operator=(const SaveHistogramWidget& widget );
	static const QString SAVE_ASCII;
	static const QString SAVE_PNG;
    QFileSystemModel* fileModel;
    Ui::SaveHistogramWidgetClass ui;
};
}
#endif // SAVEHISTOGRAMWIDGET_QO_H
