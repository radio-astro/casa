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
#ifndef FIT2DTOOL_QO_H
#define FIT2DTOOL_QO_H

#include <QtGui/QDialog>
#include <QProgressDialog>
#include <QFileDialog>
#include <casa/aipstype.h>
#include <display/Fit/Fit2DTool.ui.h>
#include <display/Fit/FindSourcesDialog.qo.h>
#include <display/Fit/PixelRangeDialog.qo.h>
#include <display/Fit/ResidualHistogramDialog.qo.h>
#include <display/Fit/Fit2DLogDialog.qo.h>
#include <display/region/QtRegionSource.qo.h>

#include <casa/cppconfig.h>

namespace casa {

	template <class T> class ImageInterface;
	class Gaussian2DFitter;
	class ColorComboDelegate;
	class RegionShape;

	class Fit2DTool : public QDialog {
		Q_OBJECT

	public:
		Fit2DTool(QWidget *parent = 0);
		void setImage( shared_ptr<const ImageInterface<Float> > image);
		bool setImageRegion( ImageRegion* imageRegion, int id );
		void deleteImageRegion( int id );
		void imageRegionSelected( int id );
		~Fit2DTool();

	signals:
		void showOverlay(String, const QString&);
		void removeOverlay(String);
		void remove2DFitOverlay( QList<RegionShape*> fitMarkers );
		void add2DFitOverlay( QList<RegionShape*> fitMarkers );
		void addResidualFitImage( String );

	public slots:
		void frameChanged( int frame );
		void newRegion( int, const QString &shape, const QString &name,
		                const QList<double> &world_x, const QList<double> &world_y,
		                const QList<int> &pixel_x, const QList<int> &pixel_y,
		                const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );
		void updateRegion( int, viewer::region::RegionChanges,
		                   const QList<double> &world_x, const QList<double> &world_y,
		                   const QList<int> &pixel_x, const QList<int> &pixel_y );

	private slots:
		void showFileDialog();
		void showFindSourcesDialog();
		void showPixelRangeDialog();
		void estimateFileChanged( const QString& fullPath );
		void pixelRangeChanged();
		void pixelRangeEnabledChanged( bool enabled );
		void pixelRangeNoneSelected( bool selected );
		void doFit();
		void fitColorChanged( const QString& colorName);
		void fitDone();
		void clearFitMarkers();
		void showResults();
		void showSaveDialog();
		void showResidualHistogramDialog();
		void residualSupportChanged( bool enable );
		void showResidualDialog();
		void displayFitChanged( bool display );
		void imageModeChanged( bool imageEnabled );

	private:
		Fit2DTool( const Fit2DTool& fitTool );
		Fit2DTool operator=( const Fit2DTool& fitTool );
		Vector<Float> populateInclude() const;
		Vector<Float> populateExclude() const;
		void populateIncludeExclude(Vector<Float>& range ) const;

		void setImageFunctionalityEnabled( bool enable );
		void resetRegion( const QList<int>& pixelX, const QList<int>& pixelY );
		bool populateSaveFile( String& saveFile );
		bool populateResidualFile( String& saveFile );
		void showFileChooserDialog(const QString& title, QFileDialog::FileMode mode, QLineEdit* destinationLineEdit );
		bool validateFile( QLineEdit* directoryLineEdit, QLineEdit* fileLineEdit,
		                   String& saveFile, const QString& purpose );
		void addViewerFitMarkers();
		void removeViewerFitMarkers();
		void clearRegions();

		//Update the widgets that depend on knowing the frame.
		void updateFrame();

		const QString REGION_LABEL;
        shared_ptr<const ImageInterface<Float> > image;
		QList<RegionShape*> fitMarkers;
		Gaussian2DFitter* fitter;
		ColorComboDelegate* fitColorDelegate;
		QProgressDialog progressBar;
		FindSourcesDialog findSourcesDialog;
		PixelRangeDialog pixelRangeDialog;
		Fit2DLogDialog logDialog;
		ResidualHistogramDialog residualHistogramDialog;
		String residualImagePath;

		Ui::Fit2DToolClass ui;
	};
}
#endif // FIT2DTOOL_QO_H
