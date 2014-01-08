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

#ifndef IMAGEVIEW_QO_H
#define IMAGEVIEW_QO_H

#include <QtGui/QFrame>
#include <QMenu>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <measures/Measures/MDoppler.h>
#include <display/QtViewer/ImageManager/ImageView.ui.h>

class QSpacerItem;

namespace casa {

	class QtDisplayData;
	class DisplayLabel;

	/**
	 * Displays properties of an image for manipulation such as color and
	 * display type.
	 */

	class ImageView : public QFrame {
		Q_OBJECT

	public:
		ImageView(QtDisplayData* data, QWidget *parent = 0);
		typedef enum ColorCombinationMode {NO_COMBINATION, RGB, HUE_SATURATION };

		//Accessors
		QString getName() const;
		QtDisplayData* getData() const;
		QString getDataDisplayTypeName() const;
		bool isRegistered() const;
		bool isMasterHue() const;
		bool isMasterSaturation() const;
		bool isMasterCoordinate() const;
		bool isEmpty() const;
		bool isRaster() const;
		QColor getDisplayedColor() const;

		//Setters
		void setRegistered( bool selected );
		void setColorCombinationMode( ColorCombinationMode mode );
		void setMasterCoordinateImage( bool masterImage );
		void setMasterHueImage( bool masterImage );
		void setMasterSaturationImage( bool masterImage );
		//Sets whether or not this is the image currently being viewed
		//on the animator.
		void setViewedImage( bool viewed );
		void setDisplayedColor( QColor color );
		void setData( QtDisplayData* other );

		//Returns whether or not the data is eligible to set the
		//coordinate system for the display.
		bool isControlEligible() const;

		virtual QSize minimumSizeHint() const;

		static const QString DROP_ID;
		virtual ~ImageView();

	signals:
		//Register/unregister has changed.
		void imageSelected(ImageView*);
		//Raster/Contour/Vector has changed.
		void displayTypeChanged( ImageView* dd );
		//User has requested to view the display properties of this data.
		void showDataDisplayOptions( QtDisplayData* imageData );
		//Close the image.
		void close( ImageView* imageToClose );
		//This imageView has become the master coordinate image.
		void masterCoordinateImageSelected( ImageView* imageView );
		//This ImageView has become the master hue image.
		void masterHueImageSelected( ImageView* imageView );
		//This ImageView has become the master saturation image.
		void masterSaturationImageSelected( ImageView* imageView );
		//There will be no master coordinate image.
		void masterCoordinateImageClear();
		//An image has been selected to view on the image animator.
		void viewImage( ImageView* imageToView );

	protected:
		//Implemented to support drag and drop.
		virtual void mouseMoveEvent( QMouseEvent* event );


	private slots:
		//Method of specifying "rest" has changed.
		void restChanged();
		void restUnitsChanged();
		//Minimize/maximize the display
		void openCloseDisplay();
		//Display data has changed register/unregister status.
		void imageRegistrationChanged( bool selected );
		//User has changed the rest frequency
		void restFrequencyChanged();
		//Change to raster/contour/vector/marker
		void displayTypeChanged();
		//Color this image will use in RGB mode has changed.
		void rgbChanged();
		//User selected color for RGB mode has changed.
		//void otherColorChanged();
		//Display the context menu.
		void showContextMenu( const QPoint& location );
		//Show a color dialog where the user can choose a custom color.
		//void showColorDialog();
		//Show the display options for this image view.
		void showDataOptions();

	private:
		ImageView( const ImageView& other );
		ImageView operator=( const ImageView& other );

		//Initialization
		void initDisplayLabels();
		void initDisplayLabel( QWidget* holder, DisplayLabel* label );
		void initColorModeSettings();
		void initRestSettings();
		void initDisplayType();
		void setTitle();

		//Conversion
		double wavelengthFrequencyConversion( double value,
			QString oldUnits, QString newUnits ) const;
		bool isCategoryMatch( const QString& newUnits, const QString& oldUnits ) const;


		//Background color.  Master image used to set the
		//coordinate system is a slightly different color
		void setBackgroundColor( QColor color );
		QColor getBackgroundColor() const;

		//Opening/closing
		void minimizeDisplay();
		void maximizeDisplay();

		//Snapshots
		void saveSnapshot();
		void restoreSnapshot();

		//Drag and drop
		QImage* makeDragImage();
		void makeDrag( QMouseEvent* event );

		//Custom color for RGB image combination
		//void setButtonColor( QColor color );
		//QColor getButtonColor() const;
		void rgbModeChanged();

		enum DisplayType { DISPLAY_RASTER, DISPLAY_CONTOUR, DISPLAY_VECTOR, DISPLAY_MARKER, DISPLAY_NONE };
		QMap<DisplayType,QString> displayTypeMap;
		DisplayType storedDisplay;

		enum RestTypes { REST_FREQUENCY, REST_WAVELENGTH, REST_REDSHIFT };
		RestTypes restType;
		QString restUnits;

		//Available context menu choices
		QAction viewAction;
		QAction closeAction;
		QAction masterCoordinateSystemAction;
		QAction masterCoordinateSystemUndoAction;
		QAction masterHueAction;
		QAction masterSaturationAction;
		QAction rasterAction;
		QAction contourAction;
		QMenu contextMenu;

		//Regular background or master coordinate image background
		QColor normalColor;
		QColor masterCoordinateColor;

		QtDisplayData* imageData;
		QButtonGroup* displayGroup;

		//Indicators of which display properties apply to this image view.
		DisplayLabel* displayTypeLabel;
		DisplayLabel* coordinateMasterLabel;
		DisplayLabel* hueMasterLabel;
		DisplayLabel* saturationMasterLabel;
		QSpacerItem* spacerFirst;
		QSpacerItem* spacerLast;

		//Method used to combine images using colors.
		ColorCombinationMode colorMode;
		Record displayOptionsSnapshot;
		bool empty;

		//Rest frequency
		QStringList frequencyUnits;
		QStringList wavelengthUnits;
		const String REST_FREQUENCY_KEY;
		const String VALUE_KEY;

		//Opening/closing
		int minimumSize;
		const int SIZE_COLLAPSED;
		const int SIZE_EXPANDED;

		const int VIEWED_BORDER_SIZE;
		const int NOT_VIEWED_BORDER_SIZE;
		bool isViewed() const;
		bool isRGBImage() const;
		Ui::ImageViewClass ui;
	};

}

#endif // IMAGEVIEW_QO_H
