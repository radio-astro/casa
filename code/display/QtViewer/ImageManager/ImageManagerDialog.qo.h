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
#include <display/QtViewer/ImageManager/ImageView.qo.h>
#include <casa/BasicSL/String.h>

#include <tr1/memory>

namespace casa {

	class DisplayOptionsDialog;
	class QtDisplayData;
	//class ImageView;
	class ImageScroll;
	class DisplayDataHolder;
	class ColormapDefinition;
	class Colormap;
	template <class T> class ImageInterface;

	/**
	 * Allows users to manipulate the images loaded in the viewer.  Includes
	 * the ability to reorder images, change color display properties for multiple images, etc.
	 * Please see CAS-4081 for details.
	 */
	class ImageManagerDialog : public QDialog, public ImageTracker {
		Q_OBJECT

	public:
		ImageManagerDialog(QWidget *parent = 0);
		//Sets the containers which are the sources for display data, both registered and
		//open.
		void setImageHolders( DisplayDataHolder* displayed, DisplayDataHolder* allImages );

		//Sets the image that is currently being viewed (on the Animator).  Allows the image
		//manager to indicate the current image on the display.
		void setViewedImage( int registrationIndex );
		void closeImageView( QtDisplayData* image );

		~ImageManagerDialog();

		//-----------------------------------------------------------------------
		//          Image Tracker Interface
		//-----------------------------------------------------------------------
		//Internally, a new master image has been selected.
		void masterImageSelected( QtDisplayData* image );
		//Adds an image to the manager when a new one is opened.
		void imageAdded( QtDisplayData* image, int position,
				bool autoRegister, bool masterCoordinate,
				bool masterSaturation, bool masterHue );

	signals:
		void ddClosed( QtDisplayData*& dd );
		void ddOpened( const String& path, const String& dataType,
		               const String& displayType,/*, const QColor singleColor*/
		               int insertionIndex, bool registered,
		               bool masterCoordinate, bool masterSaturation,
		               bool masterHue);
		void registerAll();
		void unregisterAll();
		void registerDD( QtDisplayData* dd, int position );
		void unregisterDD( QtDisplayData* dd );
		//Emitted when a new master image for setting the coordinate system has been
		//selected.
		void masterCoordinateChanged( QtDisplayData* oldMaster, QtDisplayData* newMaster );
		void animateToImage( int index );
		void createRGBImage( QtDisplayData* coordinateMaster, QtDisplayData* redImage, QtDisplayData* greenImage, QtDisplayData* blueImage );

	public slots:
		//The ImageView has changed types (raster, contour, vector, etc)
		void displayTypeChanged( ImageView* changedView );


	private slots:
		//Opens the display data options panel for the specific display data.
		void showDataDisplayOptions( QtDisplayData* );

		//Apply the color changes the user has specified.
		void applyColorChanges();
		//User has changed the color method for combining multiple images (RGB,Hue/Saturation,etc)
		void colorRestrictionsChanged();

		//Close/register/unregister
		void closeAll();
		void registerImages();
		void unregisterImages();
		void registrationChange( ImageView* imageView );
		void closeImage( QtDisplayData* image, bool coordinateMaster );

		//A new image has been designated as the master image for setting the
		//coordinate system.
		void masterImageChanged( QtDisplayData* newMaster );

		//The user has reordered the images in the display.
		void reorderDisplayImages( QtDisplayData* displayData, int dropIndex, bool registered,
				bool masterCoordinate, bool masterSaturation, bool masterHue, QColor rgbColor );


	private:
		ImageManagerDialog( const ImageManagerDialog& other );
		ImageManagerDialog operator=( const ImageManagerDialog& other );

		//Initialization
		void initializeScrollArea();

		//Coloring
		//Returns the transparency to use when combining images.
		//float getTransparency() const;
		//Get the min and max intensity of the image.
		bool getIntensityMinMax( std::tr1::shared_ptr<ImageInterface<float> > img,
		                         double* intensityMin, double* intensityMax );
        //Generate a color map based on a single base color (RGB mode).
		Colormap* generateColorMap( QColor baseColor);
		//Helper method which generates a color definition based on a single color.
		ColormapDefinition* generateSaturationMap( QColor baseColor );
		//Attempts to set a master hue color map into all of the images.  Used in
		//hue/saturation mode.
		bool applyMasterColor( QString& errorMessage );
		//Worker method that remaps a baseMap with a baseIntensityMin and baseIntensityMap
		//to a new color map definition with intensityMin and intensityMax.  Used in
		//hue/saturation mode.
		Colormap* generateMasterDefinition( ColormapDefinition* baseMap,
			double colorMin, double colorMax, double intensityMin, double intensityMax );
		float getColorFraction( float value, double minValue, double maxValue );
		ImageView::ColorCombinationMode getColorCombinationMode() const;

		//Enable/disable the "all" buttons based on how many images are
		//registered, unregistered, open.
		void updateAllButtons();

		//Dialog containing display options for a particular display data.
		DisplayOptionsDialog* displayOptionsDialog;
		//Holds the open images
		DisplayDataHolder* allImages;

		//Holds the registered images
		DisplayDataHolder* displayedImages;
		Ui::ImageManagerDialogClass ui;
		ImageScroll* imageScroll;
		const String SINGLE_COLOR_MAP;
		const String MASTER_COLOR_MAP;
		const int COLOR_MAP_SIZE;
	};

}

#endif // IMAGEMANAGERDIALOG_QO_H
