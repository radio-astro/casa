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
#include <display/QtViewer/ImageManager/ImageView.ui.h>



namespace casa {

	class QtDisplayData;

	/**
	 * Displays properties of an image for manipulating such as color and
	 * display type.
	 */

	class ImageView : public QFrame {
		Q_OBJECT

	public:
		ImageView(QtDisplayData* data, QWidget *parent = 0);
		QString getName() const;
		QtDisplayData* getData() const;
		QString getDataDisplayTypeName() const;
		bool isImageSelected() const;
		void setImageSelected( bool selected );
		void setImageColorsEnabled( bool enabled );
		void setDisplayedColor( QColor imageColor );
		void emitDisplayColorsChanged();
		QColor getDisplayedColor() const;
		void makeDrag();
		~ImageView();

	signals:
		void imageSelected(ImageView*);
		void displayTypeChanged( ImageView* dd );
		void displayColorsChanged( ImageView* dd );

	protected:
		virtual void mouseMoveEvent( QMouseEvent* event );

	private slots:
		void openCloseDisplay();
		void showColorDialog();
		void imageSelectionChanged( bool selected );
		void displayTypeChanged();

	private:
		ImageView( const ImageView& other );
		ImageView operator=( const ImageView& other );
		void setBackgroundColor( QColor color );
		void minimizeDisplay();
		void maximizeDisplay();
		void initDisplayType();

		void setButtonColor( QColor color );
		QColor getButtonColor() const;

		enum DisplayType { DISPLAY_RASTER, DISPLAY_CONTOUR, DISPLAY_VECTOR, DISPLAY_MARKER };
		DisplayType getDataDisplayType() const;

		bool selected;
		bool minimized;
		QColor selectedColor;
		QColor normalColor;

		QtDisplayData* imageData;
		QButtonGroup* displayGroup;
		Ui::ImageViewClass ui;
	};

}

#endif // IMAGEVIEW_QO_H
