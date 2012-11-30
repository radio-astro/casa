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
#ifndef IMAGELOADER_QO_H
#define IMAGELOADER_QO_H

#include <QtGui/QDialog>
#include <guitools/Histogram/ImageLoader.ui.h>

class QFileSystemModel;
class QKeyEvent;

namespace casa {

/**
 * Loads image files when the histogram runs as a standalone
 * app.
 */

class ImageLoader : public QDialog
{
    Q_OBJECT

public:
    ImageLoader(QWidget *parent = 0);
    ~ImageLoader();
    QString getFilePath() const;

signals:
	void imageFileChanged();

protected:
    void keyPressEvent( QKeyEvent* event );

private slots:
	void fileChanged();
	void fileReset();
	void directoryChanged(const QModelIndex& modelIndex );

	void validateDirectory( const QString& str );

private:
	ImageLoader( const ImageLoader& );
	ImageLoader& operator=( const ImageLoader& );
	bool validatePath( QLineEdit* lineEdit, const QString& errorPrefix, bool file, QString& destination );
	void fileChanged( QLineEdit* destinationLineEdit, const QString& emptyWarning, bool directory );

	QString imageFile;
	QFileSystemModel* fileModel;
	bool saveOutput;
    Ui::ImageLoaderClass ui;
};

}

#endif // FILELOADER_QO_H
