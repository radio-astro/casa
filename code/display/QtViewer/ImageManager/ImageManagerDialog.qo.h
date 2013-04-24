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
#ifndef IMAGEMANAGERDIALOG_QO_H
#define IMAGEMANAGERDIALOG_QO_H

#include <QtGui/QDialog>
#include <display/QtViewer/ImageManager/ImageManagerDialog.ui.h>
#include <display/QtViewer/ImageManager/ImageTracker.h>
#include <casa/BasicSL/String.h>

namespace casa {

class QtDisplayData;
class ImageView;
class ImageScrollWidget;
class DisplayDataHolder;
class ColormapDefinition;
template <class T> class ImageInterface;

/**
 * Allows users to manipulate the images loaded in the viewer.  Includes
 * the ability to reorder images, change color display properties, etc.
 * Please see CAS-4081 for details.
 */
class ImageManagerDialog : public QDialog, public ImageTracker {
    Q_OBJECT

public:
    ImageManagerDialog(QWidget *parent = 0);
    void setImageHolders( DisplayDataHolder* displayed, DisplayDataHolder* allImages );
    QtDisplayData* getDisplayChangeDD() const;
    ~ImageManagerDialog();

    //Image Tracker Interface
    void masterImageSelected( QtDisplayData* image );
    void imageAdded( QtDisplayData* image );
    void imageRemoved( QtDisplayData* image );

signals:
	void ddClosed( QtDisplayData* dd );
	void ddOpened( const String& path, const String& dataType,
			const String& displayType/*, const QColor singleColor*/ );

public slots:
	void displayTypeChanged( ImageView* changedView );
	void displayColorsChanged(ImageView* changedView );

private slots:
	void applyColorChanges();
	void openToDisplayed();
	void displayedToOpen();
	void closeImage();
	void unDisplayImage( QtDisplayData* image );
	void displayImage( QtDisplayData* image );
	void colorRestrictionsChanged();
	void masterImageChanged( const QString& imageName );

private:
	ImageManagerDialog( const ImageManagerDialog& other );
	ImageManagerDialog operator=( const ImageManagerDialog& other );

	void updateSelectedMaster( const QString& previousSelection);
	void updateMasterList();
	void updateColorList();
	void updateSaturationList();
	void setComboIndex( QComboBox* combo, int index );
	void removeImageFromList( QList<ImageView*>& imageList );
	void initializeScrollArea( QWidget* holder, ImageScrollWidget*& scrollArea );
	bool getIntensityMinMax( ImageInterface<float>* img,
			double* intensityMin, double* intensityMax );
	bool isControlEligible( QtDisplayData* qdd ) const;
	ColormapDefinition* generateSaturationMap( double minIntensity, double maxIntensity,
			QColor baseColor );
	void updateImageList(QComboBox* combo );
	DisplayDataHolder* openHolder;
	DisplayDataHolder* allImages;
    Ui::ImageManagerDialogClass ui;
    ImageScrollWidget* openScroll;
    ImageScrollWidget* displayedScroll;
};

}

#endif // IMAGEMANAGERDIALOG_QO_H
