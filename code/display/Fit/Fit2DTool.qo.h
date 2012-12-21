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
#include <casa/aipstype.h>
#include <display/Fit/Fit2DTool.ui.h>
#include <display/Fit/FindSourcesDialog.qo.h>
#include <display/Fit/PixelRangeDialog.qo.h>
#include <display/Fit/Fit2DLogDialog.qo.h>
#include <display/region/QtRegionSource.qo.h>

namespace casa {

template <class T> class ImageInterface;
class Gaussian2DFitter;

class Fit2DTool : public QDialog {
    Q_OBJECT

public:
    Fit2DTool(QWidget *parent = 0);
    void setImage( ImageInterface<Float>* image );
    ~Fit2DTool();

signals:
	void showOverlay(String, String, String);
	void removeOverlay(String);

public slots:
	void frameChanged( int frame );
	void newRegion( int, const QString &shape, const QString &name,
		const QList<double> &world_x, const QList<double> &world_y,
		const QList<int> &pixel_x, const QList<int> &pixel_y,
		const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );

	void updateRegion( int, viewer::Region::RegionChanges,
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
	void finishedWork();
	void doFit();
	void fitDone();

private:
	Fit2DTool( const Fit2DTool& fitTool );
	Fit2DTool operator=( const Fit2DTool& fitTool );
	Vector<Float> populateInclude() const;
	Vector<Float> populateExclude() const;
	void populateIncludeExclude(Vector<Float>& range ) const;
	String populatePixelBox() const;
	void setImageFunctionalityEnabled( bool enable );
	void resetRegion( const QList<int>& pixelX, const QList<int>& pixelY );

	QList<int> regionBoxBLC;
	QList<int> regionBoxTRC;
	ImageInterface<Float>* image;
	Gaussian2DFitter* fitter;
	QProgressDialog progressBar;
    FindSourcesDialog findSourcesDialog;
    PixelRangeDialog pixelRangeDialog;
    Fit2DLogDialog logDialog;

    Ui::Fit2DToolClass ui;
};
}
#endif // FIT2DTOOL_QO_H
