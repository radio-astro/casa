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
#ifndef IMAGESCROLL_QO_H
#define IMAGESCROLL_QO_H

#include <display/QtViewer/ImageManager/ImageView.qo.h>
#include <display/QtViewer/ImageManager/ImageScroll.ui.h>
#include <QtGui/QWidget>

class QDragMoveEvent;
class QDragEnterEvent;
class QDropEvent;
class QMimeData;
class QSpacerItem;
class QFrame;

namespace casa {

	class QtDisplayData;

	/**
	 * Displays a list of images, allowing the user to reorder them.
	 */

	class ImageScroll : public QWidget {
		Q_OBJECT

	public:
		ImageScroll(QWidget *parent = 0);

		//Accessors
		QList<ImageView*> getViews();
		//Returncds the image in charge of defining the hue color..
		QtDisplayData* getHueMaster() const;
		//Returns the image in charge of defining the saturation color.
		QtDisplayData* getSaturationMaster() const;
		QtDisplayData* getCoordinateMaster() const;

		//Returns the number of open images.
		int getImageCount() const;
		//Returns the number of registered images
		int getRegisteredCount() const;
		//Returns the registeration index of the image with open index given by dropIndex.
		int getRegisteredIndex( int dropIndex )const;
		//Returns the open index of the image.
		int getIndex( ImageView* view );

		//Setters
		//Index of image currently being viewed in animator.
		void setViewedImage( int registrationIndex );
		void setRegisterAll( bool selectAll);
		void setColorCombinationMode( ImageView::ColorCombinationMode mode );
		//Called from the ImageManagerDialog, when a new master coordinate
		//image has been selected programmatically by the viewer.  Percolates
		//the selection downward to the ImageView so it can be displayed.
		void setMasterCoordinateImage( QString masterCoordinateImageName );

		//ImageManipulation
		void closeImages();
		void addImageView( QtDisplayData* displayData, bool registered,
				ImageView::ColorCombinationMode mode,
				int dropIndex = -1, bool masterCoordinate = false,
				bool masterSaturation = false, bool masterHue = false,
				QColor rgbColor= QColor("#D3D3D3"));
		void removeImageView( QtDisplayData* displayData );
		virtual ~ImageScroll();

	signals:
		//An display data was closed.
		void displayDataRemoved( QtDisplayData* imageData, bool );
		//A display data was added.
		void displayDataAdded( QtDisplayData* imageData );
		//Change from raster, contour,vector, marker
		void displayTypeChanged( ImageView* displayData );
		//Image was moved by the user
		void imageOrderingChanged( QtDisplayData* imageData, int dropIndex,
				bool registered, bool masterCoordinate, bool masterSaturation,
				bool masterHue, QColor rgbColor );
		//The new display data became in charge of setting the coordinate system.
		void masterCoordinateImageChanged( QtDisplayData* imageData );
		//Show the display options for this data.
		void showDataDisplayOptions( QtDisplayData* imageName );
		//Either register/unregister occured for the ImageView.
		void registrationChange( ImageView* imageView );
		void animateToImage( int index );

	protected:
		void dragEnterEvent( QDragEnterEvent* enterEvent );
		void dropEvent( QDropEvent* dropEvent );
		void dragMoveEvent( QDragMoveEvent* dragMoveEvent );
		void dragLeaveEvent( QDragLeaveEvent* leaveEvent );

	private slots:
		//Close this particular image view.
		void closeImage( ImageView* imageView, bool deleteImage = true );
		//Called by an ImageView when it has been selected to be the new
		//master coordinate image.  Percolates the event upward to the the
		//ImageManagerDialog.
		void coordinateSystemChanged( ImageView* imageData );
		//There should be no display data used to set the master coordinate
		//system.
		void masterCoordinateClear();
		//The master image used to determine hue has changed.
		void hueImageChanged( ImageView* imageData );
		//The master image used to determine color saturation has changed.
		void saturationImageChanged( ImageView* imageData );
		void viewImage( ImageView* imageView );

	private:
		ImageScroll( const ImageScroll& other );
		ImageScroll operator=( const ImageScroll& other );

		//Coordinate system master image
		void resetMasterCoordinate( ImageView* newMaster );

		//Adds an image at a particular place in the layout
		void addImage( ImageView* imageView, int dropIndex = -1 );

		//Returns the index of a particular image.
		int findImageView( QString name, bool exactMatch = true );
		QString removeSuffixes( QString name ) const;
		QString stripBold( QString name ) const;

		//Drag and drop
		int getDropIndex( int dropY );
		void insertDragMarker( int position );
		void removeDragMarker();
		int getDragMarkerLayoutIndex() const;
		ImageView* getMimeImageView( const QMimeData* mimeData );

		QSpacerItem* spacer;
		//Horizontal line showing where the image will be placed when it is reordered.
		QFrame* dropMarker;
		const int LAYOUT_SPACING;
		const int LAYOUT_MARGIN;
		QList<ImageView*> images;
		QWidget* scrollWidget;
		Ui::ImageScrollClass ui;
	};

}

#endif // IMAGESCROLL_QO_H
