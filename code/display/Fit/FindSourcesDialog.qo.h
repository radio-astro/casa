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
#include <QMap>
#include <casa/Containers/Record.h>
#include <display/Fit/FindSourcesDialog.ui.h>
#include <display/Fit/ComponentListWrapper.h>
#include <display/Fit/PixelRangeDialog.qo.h>
#include <display/region/QtRegionSource.qo.h>

#include <tr1/memory>
class QFileSystemModel;

namespace casa {

	class SkyCatOverlayDD;
	class RegionBox;
	class ColorComboDelegate;

	/**
	 * Displays a dialog that allows the user to find and edit a source list
	 * that can be used as estimates for a 2D fit.
	 */

	class FindSourcesDialog : public QDialog {
		Q_OBJECT

	public:
		FindSourcesDialog(QWidget *parent = 0, bool displayModeFunctionality = true);
		void setImage( std::tr1::shared_ptr<const ImageInterface<Float> > image );
		void setChannel( int channel );
		void clearImage();

		QString getRegionString() const;
		String getPixelBox() const;
		QString getImagePixelBox() const;
		String getScreenedEstimatesFile( const String& estimatesFileName,
		                                 bool* errorWritingFile );
		const static QStringList colorNames;
		~FindSourcesDialog();

	signals:
		void showOverlay(String, const QString& );
		void removeOverlay(String path );
		void estimateFileSpecified( const QString& fullPath );

	public slots:
		void setImageMode( bool imageMode );
		bool newRegion( int id, const QString & shape, const QString &name,
		                const QList<double> & world_x, const QList<double> & world_y,
		                const QList<int> &pixel_x, const QList<int> &pixel_y,
		                const QString & linecolor, const QString & text, const QString & font,
		                int fontsize, int fontstyle );
		bool updateRegion( int id, viewer::region::RegionChanges changes,
		                   const QList<double> & world_x, const QList<double> & world_y,
		                   const QList<int> &pixel_x, const QList<int> &pixel_y );
		void setOverlayColor(const QString& colorName);

	private slots:
		void findSources();
		void deleteSelectedSource();
		void canceledFindSources();
		void saveEstimateFile();
		void directoryChanged(const QModelIndex& modelIndex );
		void validateDirectory( const QString& str );
		void cutoffModeChanged( bool noise );
		void showPixelRange();
		void pixelRangeChanged();
		void viewerDisplayChanged();

	private:
		FindSourcesDialog( const FindSourcesDialog& other );
		FindSourcesDialog& operator=( const FindSourcesDialog& other );
		void populatePixelBox();
		//Written because when a fit of a residual image was being done, the region
		//bounds were larger than the bounds in the residual image.
		void populateImageBounds();

		void resetCurrentId( int suggestedId );
		void resetSourceView();
		void setSourceResultsVisible( bool visible );
		void createTable();
		void initializeFileManagement();
		void setTableValue(int row, int col, const String& val );
		double populateCutOff(bool* valid) const;
		Record makeRegion( bool * valid );
		void resetSkyOverlay();
		void clearSkyOverlay();
		void clearRegions();
		bool writeEstimateFile( QString& filePath,
		                        bool screenEstimates = false, RegionBox* screeningBox = NULL );
		QString getRemoveOverlayPath() const;

		ComponentListWrapper skyList;
		enum SourceColumns { ID_COL, RA_COL, DEC_COL, FLUX_COL,
		                     MAJOR_AXIS_COL, MINOR_AXIS_COL, ANGLE_COL/*, FIXED_COL*/
		                   };
        std::tr1::shared_ptr<const ImageInterface<Float> > image;
		String pixelBox;
		QString skyPath;
		QString overlayColorName;
		bool imageMode;
		int channel;
		int resultIndex;
		int currentRegionId;
		QFileSystemModel* fileModel;
		QMap< int, RegionBox*> regions;
		const int DEFAULT_KEY;
		const QString SKY_CATALOG;
		ColorComboDelegate* colorDelegate;
		PixelRangeDialog pixelRangeDialog;
		Vector<int> blcVector;
		Vector<int> trcVector;
		Ui::FindSourcesDialogClass ui;
	};
}
#endif // FIND_SOURCES_DIALOG_QO_H
