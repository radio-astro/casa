//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#ifndef FIND_SOURCES_DIALOG_QO_H
#define FIND_SOURCES_DIALOG_QO_H

#include <QtGui/QDialog>
#include <casa/Containers/Record.h>

#include <display/Fit/FindSourcesDialog.ui.h>
#include <display/Fit/ComponentListWrapper.h>
#include <display/region/QtRegionSource.qo.h>

class QFileSystemModel;

namespace casa {

class SkyCatOverlayDD;

/**
 * Displays a dialog that allows the user to find and edit a source list
 * that can be used as estimates for a 2D fit.
 */

class FindSourcesDialog : public QDialog
{
    Q_OBJECT

public:
    FindSourcesDialog(QWidget *parent = 0);
    void setImage( ImageInterface<Float>* image );
    ~FindSourcesDialog();
    void setChannel( int channel );
    void setPixelBox( const String& box );

signals:
	void showOverlay(String, String, String);
	void removeOverlay(String path );
	void estimateFileSpecified( const QString& fullPath );

private slots:
	void findSources();
	void deleteSelectedSource();
	void canceledFindSources();
	void saveEstimateFile();
	void directoryChanged(const QModelIndex& modelIndex );
	void validateDirectory( const QString& str );

private:
	FindSourcesDialog( const FindSourcesDialog& other );
	FindSourcesDialog& operator=( const FindSourcesDialog& other );
	bool writeEstimateFile( QString& filePath );
	void resetSourceView();
	void setSourceResultsVisible( bool visible );
	void createTable();
	void initializeFileManagement();
	void setTableValue(int row, int col, const String& val );
	Record makeRegion() const;
	void resetSkyOverlay();
	void clearSkyOverlay();

	ComponentListWrapper skyList;
	enum SourceColumns { ID_COL, RA_COL, DEC_COL, FLUX_COL };
	ImageInterface<Float>* image;
	String pixelBox;
	QString skyPath;
	int channel;
	int resultIndex;
	QFileSystemModel* fileModel;
	const QString SKY_CATALOG;
    Ui::FindSourcesDialogClass ui;
};
}
#endif // FIND_SOURCES_DIALOG_QO_H
